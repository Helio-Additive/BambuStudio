#ifndef slic3r_GUI_HelioPreFlightDialog_hpp_
#define slic3r_GUI_HelioPreFlightDialog_hpp_

#include <wx/wx.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/scrolwin.h>
#include <map>
#include <string>
#include <chrono>
#include <memory>

#include "GUI_Utils.hpp"
#include "wxExtensions.hpp"
#include "HelioDragon.hpp"
#include "Widgets/Label.hpp"
#include "Widgets/Button.hpp"
#include "Widgets/ScrolledWindow.hpp"

namespace Slic3r { namespace GUI {

// Forward declaration
struct HelioInputDialogTheme;
class Plater;

class HelioPreFlightDialog : public DPIDialog
{
public:
    HelioPreFlightDialog(wxWindow* parent = nullptr);
    ~HelioPreFlightDialog();

    void on_dpi_changed(const wxRect& suggested_rect) override;

    enum CheckStatus {
        STATUS_PASS,      // ✓ All good
        STATUS_WARNING,   // ⚠ Issue but non-blocking
        STATUS_FAIL,      // ✗ Blocking issue
        STATUS_SYNCING    // Syncing in progress
    };

    struct CheckResult {
        std::string name;
        CheckStatus status;
        std::string message;
        bool show_resync_button;
    };

private:
    // UI Components
    wxScrolledWindow* m_scroll_window{nullptr};
    wxPanel* m_overall_status_panel{nullptr};
    Label* m_overall_status_label{nullptr};
    wxStaticBitmap* m_overall_status_icon{nullptr};
    std::map<std::string, wxPanel*> m_check_cards;
    std::map<std::string, Label*> m_check_status_labels;
    std::map<std::string, Label*> m_check_message_labels;
    std::map<std::string, Label*> m_check_hint_labels;
    std::map<std::string, wxStaticBitmap*> m_check_icons;

    // Check results
    std::map<std::string, CheckResult> m_results;

    // UI elements
    Button* m_resync_button{nullptr};
    bool m_is_syncing{false};

    // Shared state for callback safety
    std::shared_ptr<int> shared_ptr{nullptr};

    // Theme helper
    HelioInputDialogTheme get_theme() const;

    // UI Creation
    void create_ui();
    void create_overall_status_banner(wxBoxSizer* parent_sizer);
    wxPanel* create_check_card(const wxString& check_name, const wxString& check_id, const HelioInputDialogTheme& theme);

    // Check execution
    void run_all_checks();
    CheckResult check_sync_state();
    CheckResult check_single_material_support();
    CheckResult check_multi_material();
    CheckResult check_blocking_conditions();

    // Actions
    void on_resync_clicked();

    // UI Update
    void update_check_card(const std::string& check_id, const CheckResult& result);
    void update_overall_status();
    void set_overall_status_banner(const wxString& message, const wxColour& color);

    // Helper functions
    wxBitmap get_status_icon(CheckStatus status);
    wxColour get_status_color(CheckStatus status);
    Plater* get_plater();
};

}} // namespace Slic3r::GUI

#endif
