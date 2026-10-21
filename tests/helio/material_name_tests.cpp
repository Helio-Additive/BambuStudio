#include "slic3r/Utils/HelioMaterialName.hpp"
#include "slic3r/Utils/HelioMaterialCatalog.hpp"
#include <iostream>
#include <utility>
#include <vector>

int main()
{
    using Slic3r::helio_material_name_for_matching;
    const std::vector<std::pair<std::string, std::string>> cases = {
        {"Xioneer VXL111", "Xioneer VXL111"},
        {"  Xioneer VXL111 @BBL H2D Pro 0.4 nozzle", "Xioneer VXL111"},
        {u8"□Xioneer VXL111", "Xioneer VXL111"},
        {u8"■ Xioneer VXL111", "Xioneer VXL111"},
        {u8"🟧Xioneer VXL111 @BBL H2D Pro", "Xioneer VXL111"},
        {u8"⬜\uFE0F\u00A0Xioneer VXL111", "Xioneer VXL111"},
        {u8"▪\uFE0E\u3000Xioneer VXL111", "Xioneer VXL111"},
        {u8" \t🟥 🟦🟩🟨🟪🟫 ⬛ □ Xioneer VXL111 ", "Xioneer VXL111"},
        {u8"▫◻◼◽◾PLA", "PLA"},
        {"3D-Fuel PCTG Pro", "3D-Fuel PCTG Pro"},
        {"PA6-CF (Black) / 1.75mm", "PA6-CF (Black) / 1.75mm"},
        {u8"国产 PLA+", u8"国产 PLA+"},
        {u8"Brand □ PLA", u8"Brand □ PLA"},
        {"[White] PLA", "[White] PLA"},
        {"#FFFFFF PLA", "#FFFFFF PLA"},
        {u8"★ PLA", u8"★ PLA"},
        {"", ""},
        {" \t\n", ""},
        {u8"□\uFE0F 🟧 @Printer", ""},
        {std::string("\xF0\x9F", 2), std::string("\xF0\x9F", 2)},
    };
    for (const auto& test : cases) {
        const auto actual = helio_material_name_for_matching(test.first);
        if (actual != test.second || helio_material_name_for_matching(actual) != actual) {
            std::cerr << "Failed normalization: " << test.first << '\n';
            return 1;
        }
    }
    if (helio_material_name_for_matching(u8"🟧Xioneer VXL111 @BBL H2D") !=
        helio_material_name_for_matching(u8"□ Xioneer VXL111")) {
        std::cerr << "Decorated preset and catalog names did not match\n";
        return 1;
    }
    using Slic3r::helio_material_catalog_matching_name;
    using nlohmann::json;
    const std::vector<json> missing_aliases = {
        json::object(),
        {{"alternativeNames", nullptr}},
        {{"alternativeNames", json::object()}},
        {{"alternativeNames", {{"bambustudio", nullptr}}}},
        {{"alternativeNames", {{"bambustudio", ""}}}},
        {{"alternativeNames", {{"bambustudio", " \t "}}}},
        {{"alternativeNames", {{"bambustudio", u8"□"}}}},
        {{"alternativeNames", {{"bambustudio", 123}}}},
        {{"alternativeNames", "invalid"}},
    };
    for (auto material : missing_aliases) {
        material["id"] = "test-material";
        material["name"] = "Xioneer VXL111";
        material["feedstock"] = "FILAMENT";
        if (helio_material_catalog_matching_name(material) !=
            helio_material_name_for_matching("Xioneer VXL111 @Bambu Lab H2D Pro 0.4 nozzle")) {
            std::cerr << "Catalog fallback failed: " << material.dump() << '\n';
            return 1;
        }
    }
    if (helio_material_catalog_matching_name({{"name", "Catalog name"},
            {"alternativeNames", {{"bambustudio", u8"🟧 Bambu alias"}}}}) != "Bambu alias" ||
        !helio_material_catalog_matching_name({{"name", nullptr}}).empty() ||
        !helio_material_catalog_matching_name(json::object()).empty()) {
        std::cerr << "Alias precedence or invalid-name handling failed\n";
        return 1;
    }
    std::cout << "Catalog fallback: 9 cases plus alias precedence and missing names passed\n";
    std::cout << cases.size() << " normalization cases and decorated-name matching passed\n";
}
