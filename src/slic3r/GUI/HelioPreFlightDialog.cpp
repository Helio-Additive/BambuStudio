#include "HelioPreFlightDialog.hpp"
#include "HelioReleaseNote.hpp"
#include "I18N.hpp"

#include "libslic3r/Utils.hpp"
#include "slic3r/Utils/Http.hpp"
#include <boost/log/trivial.hpp>
#include "GUI.hpp"
#include "GUI_App.hpp"
#include "MainFrame.hpp"
#include "format.hpp"
#include "BitmapCache.hpp"
#include "Widgets/RoundedRectangle.hpp"
#include "Widgets/StaticLine.hpp"
#include "Plater.hpp"
#include "BackgroundSlicingProcess.hpp"

#include <wx/dcgraph.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/scrolwin.h>
#include <wx/wupdlock.h>

namespace Slic3r { namespace GUI {

// Helio dark palette theme colors
namespace {
    const wxColour HELIO_BG_BASE(7, 9, 12);
    const wxColour HELIO_CARD_BG(14, 19, 32);
    const wxColour HELIO_CARD_HIGHLIGHT(18, 26, 43);
    const wxColour HELIO_BORDER(255, 255, 255, 25);
    const wxColour HELIO_TEXT(238, 242, 255);
    const wxColour HELIO_MUTED(168, 176, 192);
    const wxColour HELIO_SUCCESS(34, 197, 94);
    const wxColour HELIO_WARNING(251, 191, 36);
    const wxColour HELIO_ERROR(239, 68, 68);
    const wxColour HELIO_INFO(59, 130, 246);
}

HelioInputDialogTheme HelioPreFlightDialog::get_theme() const
{
    HelioInputDialogTheme theme;
    theme.bg = HELIO_BG_BASE;
    theme.card = HELIO_CARD_BG;
    theme.card2 = HELIO_CARD_HIGHLIGHT;
    theme.border = HELIO_BORDER;
    theme.text = HELIO_TEXT;
    theme.muted = HELIO_MUTED;
    theme.purple = wxColour(175, 124, 255);
    theme.blue = wxColour(79, 134, 255);
    return theme;
}

HelioPreFlightDialog::HelioPreFlightDialog(wxWindow* parent)
    : DPIDialog(parent ? parent : static_cast<wxWindow*>(wxGetApp().mainframe),
                wxID_ANY,
                _L("Helio Pre-Flight Check"),
                wxDefaultPosition,
                wxSize(FromDIP(600), FromDIP(500)),
                wxCAPTION | wxCLOSE_BOX)
{
    try {
        shared_ptr = std::make_shared<int>(0);

        // Set Helio icon
        try {
            wxBitmap bmp = create_scaled_bitmap("helio_icon", this, 32);
            if (bmp.IsOk()) {
                wxIcon icon;
                icon.CopyFromBitmap(bmp);
                SetIcon(icon);
            }
        } catch (...) {
            // Icon loading failed, continue anyway
        }

        SetBackgroundColour(HELIO_BG_BASE);

        create_ui();
        run_all_checks();

    } catch (const std::exception& e) {
        BOOST_LOG_TRIVIAL(error) << "HelioPreFlightDialog constructor error: " << e.what();
        throw;
    }
}

HelioPreFlightDialog::~HelioPreFlightDialog()
{
}

void HelioPreFlightDialog::create_ui()
{
    try {
        auto* main_sizer = new wxBoxSizer(wxVERTICAL);
        auto theme = get_theme();

        // Create overall status banner
        create_overall_status_banner(main_sizer);

        // Create scrollable content area
        m_scroll_window = new wxScrolledWindow(this, wxID_ANY);
        m_scroll_window->SetBackgroundColour(HELIO_BG_BASE);
        m_scroll_window->SetScrollRate(0, 20);

        auto* content_sizer = new wxBoxSizer(wxVERTICAL);

        // Create 4 check cards
        auto sync_card = create_check_card(_L("Helio Catalog Sync"), "sync", theme);
        content_sizer->Add(sync_card, 0, wxEXPAND | wxBOTTOM, FromDIP(12));

        auto material_card = create_check_card(_L("Printer & Material Support"), "material", theme);
        content_sizer->Add(material_card, 0, wxEXPAND | wxBOTTOM, FromDIP(12));

        auto multi_card = create_check_card(_L("Multi-Material Check"), "multi", theme);
        content_sizer->Add(multi_card, 0, wxEXPAND | wxBOTTOM, FromDIP(12));

        auto blocker_card = create_check_card(_L("Blocking Conditions"), "blockers", theme);
        content_sizer->Add(blocker_card, 0, wxEXPAND | wxBOTTOM, FromDIP(12));

        m_scroll_window->SetSizer(content_sizer);
        main_sizer->Add(m_scroll_window, 1, wxEXPAND | wxALL, FromDIP(16));

        // Close button at bottom
        auto* button_sizer = new wxBoxSizer(wxHORIZONTAL);
        button_sizer->AddStretchSpacer();

        StateColor close_btn_bg(
            std::pair<wxColour, int>(HELIO_CARD_HIGHLIGHT, StateColor::Hovered),
            std::pair<wxColour, int>(HELIO_CARD_BG, StateColor::Normal));
        StateColor close_btn_border(
            std::pair<wxColour, int>(HELIO_BORDER, StateColor::Normal));
        StateColor close_btn_text(
            std::pair<wxColour, int>(HELIO_TEXT, StateColor::Normal));

        auto* close_button = new Button(this, _L("Close"));
        close_button->SetBackgroundColor(close_btn_bg);
        close_button->SetBorderColor(close_btn_border);
        close_button->SetTextColor(close_btn_text);
        close_button->SetMinSize(wxSize(FromDIP(100), FromDIP(36)));
        close_button->SetCornerRadius(FromDIP(6));
        close_button->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& e) { Close(); });

        button_sizer->Add(close_button, 0, wxALL, FromDIP(8));
        main_sizer->Add(button_sizer, 0, wxEXPAND);

        SetSizer(main_sizer);
        SetMinSize(wxSize(FromDIP(600), FromDIP(500)));
        Layout();
        Fit();

    } catch (const std::exception& e) {
        BOOST_LOG_TRIVIAL(error) << "HelioPreFlightDialog::create_ui error: " << e.what();
        throw;
    }
}

void HelioPreFlightDialog::create_overall_status_banner(wxBoxSizer* parent_sizer)
{
    m_overall_status_panel = new wxPanel(this, wxID_ANY);
    m_overall_status_panel->SetBackgroundColour(HELIO_CARD_BG);

    auto* banner_sizer = new wxBoxSizer(wxHORIZONTAL);

    // Status icon
    m_overall_status_icon = new wxStaticBitmap(m_overall_status_panel, wxID_ANY, wxNullBitmap);
    banner_sizer->Add(m_overall_status_icon, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(12));

    // Status text
    m_overall_status_label = new Label(m_overall_status_panel, _L("Running checks..."));
    m_overall_status_label->SetFont(Label::Body_14);
    m_overall_status_label->SetForegroundColour(HELIO_TEXT);
    banner_sizer->Add(m_overall_status_label, 1, wxALIGN_CENTER_VERTICAL);

    m_overall_status_panel->SetSizer(banner_sizer);
    parent_sizer->Add(m_overall_status_panel, 0, wxEXPAND | wxALL, FromDIP(16));
}

wxPanel* HelioPreFlightDialog::create_check_card(const wxString& check_name, const wxString& check_id, const HelioInputDialogTheme& theme)
{
    auto* card = new wxPanel(m_scroll_window, wxID_ANY);
    card->SetBackgroundColour(theme.card);

    auto* card_sizer = new wxBoxSizer(wxVERTICAL);

    // Header with icon and name
    auto* header_sizer = new wxBoxSizer(wxHORIZONTAL);

    // Status icon
    auto* icon = new wxStaticBitmap(card, wxID_ANY, get_status_icon(STATUS_PASS));
    header_sizer->Add(icon, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(8));
    m_check_icons[check_id.ToStdString()] = icon;

    // Check name
    auto* name_label = new Label(card, check_name);
    name_label->SetFont(Label::Body_14);
    name_label->SetForegroundColour(theme.text);
    header_sizer->Add(name_label, 1, wxALIGN_CENTER_VERTICAL);

    // Resync button (initially hidden, only for sync card)
    if (check_id == "sync") {
        StateColor resync_btn_bg(
            std::pair<wxColour, int>(HELIO_CARD_HIGHLIGHT, StateColor::Hovered),
            std::pair<wxColour, int>(HELIO_CARD_BG, StateColor::Normal));

        m_resync_button = new Button(card, _L("Resync"));
        m_resync_button->SetBackgroundColor(resync_btn_bg);
        m_resync_button->SetBorderColor(theme.border);
        m_resync_button->SetTextColor(StateColor(std::pair<wxColour, int>(theme.text, StateColor::Normal)));
        m_resync_button->SetMinSize(wxSize(FromDIP(80), FromDIP(28)));
        m_resync_button->SetCornerRadius(FromDIP(4));
        m_resync_button->SetFont(Label::Body_12);
        m_resync_button->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& e) { on_resync_clicked(); });
        m_resync_button->Hide();

        header_sizer->Add(m_resync_button, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(8));
    }

    card_sizer->Add(header_sizer, 0, wxEXPAND | wxALL, FromDIP(12));

    // Separator line
    auto* line = new StaticLine(card, false);
    line->SetLineColour(theme.border);
    card_sizer->Add(line, 0, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(12));

    // Status message
    auto* message_label = new Label(card, _L("Checking..."));
    message_label->SetFont(Label::Body_13);
    message_label->SetForegroundColour(theme.muted);
    card_sizer->Add(message_label, 0, wxEXPAND | wxALL, FromDIP(12));
    m_check_message_labels[check_id.ToStdString()] = message_label;

    card->SetSizer(card_sizer);
    m_check_cards[check_id.ToStdString()] = card;

    return card;
}

void HelioPreFlightDialog::run_all_checks()
{
    // Run all 4 checks synchronously
    auto sync_result = check_sync_state();
    update_check_card("sync", sync_result);

    auto material_result = check_single_material_support();
    update_check_card("material", material_result);

    auto multi_result = check_multi_material();
    update_check_card("multi", multi_result);

    auto blocker_result = check_blocking_conditions();
    update_check_card("blockers", blocker_result);

    update_overall_status();
}

HelioPreFlightDialog::CheckResult HelioPreFlightDialog::check_sync_state()
{
    try {
        const auto& supported_printers = HelioQuery::global_supported_printers;
        const auto& supported_materials = HelioQuery::global_supported_materials;

        if (m_is_syncing) {
            return {"Sync State", STATUS_SYNCING, "Syncing catalog...", false};
        }

        if (supported_printers.empty() || supported_materials.empty()) {
            return {"Sync State", STATUS_WARNING,
                    "Helio catalog not loaded. Click Resync to update.", true};
        }

        return {"Sync State", STATUS_PASS,
                wxString::Format("Catalog synced (%zu printers, %zu materials)",
                                supported_printers.size(),
                                supported_materials.size()).ToStdString(),
                false};

    } catch (const std::exception& e) {
        BOOST_LOG_TRIVIAL(error) << "check_sync_state error: " << e.what();
        return {"Sync State", STATUS_WARNING, "Error checking sync state", true};
    }
}

HelioPreFlightDialog::CheckResult HelioPreFlightDialog::check_single_material_support()
{
    try {
        Plater* plater = get_plater();
        if (!plater) {
            return {"Material Support", STATUS_WARNING, "Unable to access printer/material settings", false};
        }

        // Get filament count
        DynamicPrintConfig& config = wxGetApp().preset_bundle->filaments.get_edited_preset().config;
        auto* filament_type_opt = config.option<ConfigOptionStrings>("filament_type");

        if (!filament_type_opt || filament_type_opt->values.empty()) {
            return {"Material Support", STATUS_WARNING, "No materials loaded", false};
        }

        // Only check if single material
        if (filament_type_opt->values.size() > 1) {
            return {"Material Support", STATUS_PASS,
                    "Multi-material detected (see Multi-Material Check)", false};
        }

        // Single material - check printer + material support
        DynamicPrintConfig& printer_config = wxGetApp().preset_bundle->printers.get_edited_preset().config;
        std::string printer_model = printer_config.opt_string("printer_model");
        std::string filament_type = filament_type_opt->values[0];

        const auto& supported_printers = HelioQuery::global_supported_printers;
        const auto& supported_materials = HelioQuery::global_supported_materials;

        // Check printer
        std::string printer_lower = printer_model;
        std::transform(printer_lower.begin(), printer_lower.end(), printer_lower.begin(), ::tolower);

        bool printer_found = false;
        for (const auto& supported : supported_printers) {
            std::string supported_lower = supported.name;
            std::transform(supported_lower.begin(), supported_lower.end(), supported_lower.begin(), ::tolower);
            if (printer_lower.find(supported_lower) != std::string::npos) {
                printer_found = true;
                break;
            }
        }

        // Check material
        std::string filament_lower = filament_type;
        std::transform(filament_lower.begin(), filament_lower.end(), filament_lower.begin(), ::tolower);

        bool material_found = false;
        for (const auto& supported : supported_materials) {
            std::string supported_lower = supported.name;
            std::transform(supported_lower.begin(), supported_lower.end(), supported_lower.begin(), ::tolower);
            if (filament_lower.find(supported_lower) != std::string::npos ||
                supported_lower.find(filament_lower) != std::string::npos) {
                material_found = true;
                break;
            }
        }

        if (!printer_found && !material_found) {
            return {"Material Support", STATUS_WARNING,
                    wxString::Format("Printer '%s' and material '%s' may need mapping. Update lists or choose supported options.",
                                    printer_model, filament_type).ToStdString(),
                    true};
        } else if (!printer_found) {
            return {"Material Support", STATUS_WARNING,
                    wxString::Format("Printer '%s' not found. Choose supported printer or update lists.",
                                    printer_model).ToStdString(),
                    true};
        } else if (!material_found) {
            return {"Material Support", STATUS_WARNING,
                    wxString::Format("Material '%s' not found. Choose supported material or update lists.",
                                    filament_type).ToStdString(),
                    true};
        }

        return {"Material Support", STATUS_PASS,
                wxString::Format("Printer '%s' and material '%s' are supported",
                                printer_model, filament_type).ToStdString(),
                false};

    } catch (const std::exception& e) {
        BOOST_LOG_TRIVIAL(error) << "check_single_material_support error: " << e.what();
        return {"Material Support", STATUS_WARNING, "Error checking support", false};
    }
}

HelioPreFlightDialog::CheckResult HelioPreFlightDialog::check_multi_material()
{
    try {
        Plater* plater = get_plater();
        if (!plater) {
            return {"Multi-Material", STATUS_PASS, "Single material (default)", false};
        }

        DynamicPrintConfig& config = wxGetApp().preset_bundle->filaments.get_edited_preset().config;
        auto* filament_type_opt = config.option<ConfigOptionStrings>("filament_type");

        if (!filament_type_opt || filament_type_opt->values.size() <= 1) {
            return {"Multi-Material", STATUS_PASS, "Single material mode", false};
        }

        int material_count = filament_type_opt->values.size();
        return {"Multi-Material", STATUS_WARNING,
                wxString::Format("Using %d materials. Helio supports one material. Continue — you may see prompts to confirm or adjust.",
                                material_count).ToStdString(),
                false};

    } catch (const std::exception& e) {
        BOOST_LOG_TRIVIAL(error) << "check_multi_material error: " << e.what();
        return {"Multi-Material", STATUS_PASS, "Unable to check material count", false};
    }
}

HelioPreFlightDialog::CheckResult HelioPreFlightDialog::check_blocking_conditions()
{
    try {
        std::vector<std::string> blockers;

        // 1. Check PAT
        std::string pat = HelioQuery::get_helio_pat();
        if (pat.empty()) {
            blockers.push_back("PAT not configured");
        }

        // 2. Check if sliced
        Plater* plater = get_plater();
        if (plater) {
            BackgroundSlicingProcess& process = plater->background_process();
            GCodeProcessorResult* gcode_result = process.get_current_gcode_result();
            if (!gcode_result || gcode_result->moves.empty()) {
                blockers.push_back("Model not sliced");
            }
        }

        // 3. Check by-object printing
        if (plater) {
            // Check if complete_objects is enabled (by-object printing)
            DynamicPrintConfig& config = wxGetApp().preset_bundle->prints.get_edited_preset().config;
            auto* complete_objects_opt = config.option<ConfigOptionBool>("complete_objects");
            if (complete_objects_opt && complete_objects_opt->value) {
                blockers.push_back("By-object printing enabled (not supported)");
            }
        }

        // 4. Check if Helio task is already running
        // TODO: Add check for running Helio task when the flag is accessible
        // For now, we'll skip this check

        if (blockers.empty()) {
            return {"Blocking Conditions", STATUS_PASS, "No blockers detected", false};
        }

        std::string message = "Found " + std::to_string(blockers.size()) + " blocker(s): ";
        for (size_t i = 0; i < blockers.size(); ++i) {
            if (i > 0) message += "; ";
            message += blockers[i];
        }

        return {"Blocking Conditions", STATUS_FAIL, message, false};

    } catch (const std::exception& e) {
        BOOST_LOG_TRIVIAL(error) << "check_blocking_conditions error: " << e.what();
        return {"Blocking Conditions", STATUS_WARNING, "Error checking blockers", false};
    }
}

void HelioPreFlightDialog::on_resync_clicked()
{
    try {
        m_is_syncing = true;

        // Update sync card to show syncing state
        CheckResult syncing_result = {"Sync State", STATUS_SYNCING, "Syncing catalog...", false};
        update_check_card("sync", syncing_result);

        // Hide resync button while syncing
        if (m_resync_button) {
            m_resync_button->Hide();
            m_check_cards["sync"]->Layout();
        }

        // Get API credentials
        std::string helio_api_url = HelioQuery::get_helio_api_url();
        std::string helio_api_key = HelioQuery::get_helio_pat();

        if (helio_api_key.empty()) {
            m_is_syncing = false;
            CheckResult error_result = {"Sync State", STATUS_WARNING, "PAT not configured. Set PAT first.", true};
            update_check_card("sync", error_result);
            if (m_resync_button) m_resync_button->Show();
            return;
        }

        // Make weak pointer for callback safety
        std::weak_ptr<int> weak = shared_ptr;

        // Request support lists (this will populate the global lists)
        HelioQuery::request_all_support_machine(helio_api_url, helio_api_key);
        HelioQuery::request_all_support_materials(helio_api_url, helio_api_key);

        // Simulate async completion (in real scenario, this would be in the callback)
        // For now, we'll just set a flag and re-run checks
        CallAfter([this, weak]() {
            if (weak.expired()) return;

            m_is_syncing = false;

            // Re-run all checks
            run_all_checks();
        });

    } catch (const std::exception& e) {
        BOOST_LOG_TRIVIAL(error) << "on_resync_clicked error: " << e.what();
        m_is_syncing = false;

        CheckResult error_result = {"Sync State", STATUS_WARNING, "Sync failed. Try again.", true};
        update_check_card("sync", error_result);
        if (m_resync_button) m_resync_button->Show();
    }
}

void HelioPreFlightDialog::update_check_card(const std::string& check_id, const CheckResult& result)
{
    m_results[check_id] = result;

    // Update icon
    auto icon_it = m_check_icons.find(check_id);
    if (icon_it != m_check_icons.end()) {
        icon_it->second->SetBitmap(get_status_icon(result.status));
    }

    // Update message
    auto message_it = m_check_message_labels.find(check_id);
    if (message_it != m_check_message_labels.end()) {
        message_it->second->SetLabelText(wxString::FromUTF8(result.message));
        message_it->second->SetForegroundColour(get_status_color(result.status));
    }

    // Show/hide resync button for sync card
    if (check_id == "sync" && m_resync_button) {
        if (result.show_resync_button) {
            m_resync_button->Show();
        } else {
            m_resync_button->Hide();
        }
    }

    // Refresh the card
    auto card_it = m_check_cards.find(check_id);
    if (card_it != m_check_cards.end()) {
        card_it->second->Layout();
        card_it->second->Refresh();
    }
}

void HelioPreFlightDialog::update_overall_status()
{
    bool has_fail = false;
    bool has_warning = false;

    for (const auto& [check_id, result] : m_results) {
        if (result.status == STATUS_FAIL) {
            has_fail = true;
            break;
        } else if (result.status == STATUS_WARNING) {
            has_warning = true;
        }
    }

    if (has_fail) {
        set_overall_status_banner(_L("✗ Blocking issues detected"), HELIO_ERROR);
    } else if (has_warning) {
        set_overall_status_banner(_L("⚠ Warnings detected (non-blocking)"), HELIO_WARNING);
    } else {
        set_overall_status_banner(_L("✓ All checks passed"), HELIO_SUCCESS);
    }
}

void HelioPreFlightDialog::set_overall_status_banner(const wxString& message, const wxColour& color)
{
    if (m_overall_status_label) {
        m_overall_status_label->SetLabelText(message);
        m_overall_status_label->SetForegroundColour(color);
        m_overall_status_label->Refresh();
    }

    if (m_overall_status_panel) {
        m_overall_status_panel->Refresh();
    }
}

wxBitmap HelioPreFlightDialog::get_status_icon(CheckStatus status)
{
    const int icon_size = FromDIP(16);
    wxBitmap bmp(icon_size, icon_size);
    wxMemoryDC dc(bmp);

    dc.SetBackground(*wxBLACK_BRUSH);
    dc.Clear();

    wxColour color;
    switch (status) {
        case STATUS_PASS:
            color = HELIO_SUCCESS;
            break;
        case STATUS_WARNING:
            color = HELIO_WARNING;
            break;
        case STATUS_FAIL:
            color = HELIO_ERROR;
            break;
        case STATUS_SYNCING:
            color = HELIO_INFO;
            break;
        default:
            color = HELIO_MUTED;
            break;
    }

    dc.SetBrush(wxBrush(color));
    dc.SetPen(wxPen(color));
    dc.DrawCircle(icon_size / 2, icon_size / 2, icon_size / 2 - 1);

    dc.SelectObject(wxNullBitmap);
    return bmp;
}

wxColour HelioPreFlightDialog::get_status_color(CheckStatus status)
{
    switch (status) {
        case STATUS_PASS:
            return HELIO_SUCCESS;
        case STATUS_WARNING:
            return HELIO_WARNING;
        case STATUS_FAIL:
            return HELIO_ERROR;
        case STATUS_SYNCING:
            return HELIO_INFO;
        default:
            return HELIO_MUTED;
    }
}

Plater* HelioPreFlightDialog::get_plater()
{
    return wxGetApp().plater();
}

void HelioPreFlightDialog::on_dpi_changed(const wxRect& suggested_rect)
{
    // Recreate icons with new DPI
    for (auto& [check_id, result] : m_results) {
        auto icon_it = m_check_icons.find(check_id);
        if (icon_it != m_check_icons.end()) {
            icon_it->second->SetBitmap(get_status_icon(result.status));
        }
    }

    Layout();
    Fit();
}

}} // namespace Slic3r::GUI
