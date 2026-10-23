#ifndef slic3r_HelioMaterialCatalog_hpp_
#define slic3r_HelioMaterialCatalog_hpp_

#include "HelioMaterialName.hpp"
#include "nlohmann/json.hpp"

namespace Slic3r {

// Third-party materials need not have a Bambu Studio alias. Always fall back
// to the catalog name when the alias is missing, null, or unusable.
inline std::string helio_material_catalog_matching_name(const nlohmann::json& material)
{
    const auto aliases = material.find("alternativeNames");
    if (aliases != material.end() && aliases->is_object()) {
        const auto alias = aliases->find("bambustudio");
        if (alias != aliases->end() && alias->is_string()) {
            const auto name = helio_material_name_for_matching(alias->get<std::string>());
            if (!name.empty()) return name;
        }
    }
    const auto name = material.find("name");
    return name != material.end() && name->is_string()
        ? helio_material_name_for_matching(name->get<std::string>()) : std::string();
}

} // namespace Slic3r
#endif
