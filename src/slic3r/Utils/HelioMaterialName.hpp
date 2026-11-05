#ifndef slic3r_HelioMaterialName_hpp_
#define slic3r_HelioMaterialName_hpp_

#include <string>
#include <string_view>

namespace Slic3r {

// Matching-only normalization. Menu swatches are bitmaps, but imported/custom
// names may also contain literal square glyphs. Never alter the preset lookup key.
inline std::string helio_material_name_for_matching(std::string_view name)
{
    name = name.substr(0, name.find('@'));
    const auto trim = [&name]() {
        constexpr std::string_view spaces[] = {" ", "\t", "\r", "\n", "\f", "\v", "\xC2\xA0", "\xE3\x80\x80"};
        bool changed;
        do {
            changed = false;
            for (const auto space : spaces) {
                if (name.substr(0, space.size()) == space) {
                    name.remove_prefix(space.size());
                    changed = true;
                }
                if (name.size() >= space.size() && name.substr(name.size() - space.size()) == space) {
                    name.remove_suffix(space.size());
                    changed = true;
                }
            }
        } while (changed);
    };
    trim();
    // U+25A0/A1, U+25AA/AB, U+25FB..FE, U+2B1B/1C, U+1F7E5..EB.
    constexpr std::string_view swatches[] = {
        "\xE2\x96\xA0", "\xE2\x96\xA1", "\xE2\x96\xAA", "\xE2\x96\xAB",
        "\xE2\x97\xBB", "\xE2\x97\xBC", "\xE2\x97\xBD", "\xE2\x97\xBE",
        "\xE2\xAC\x9B", "\xE2\xAC\x9C", "\xF0\x9F\x9F\xA5", "\xF0\x9F\x9F\xA6",
        "\xF0\x9F\x9F\xA7", "\xF0\x9F\x9F\xA8", "\xF0\x9F\x9F\xA9",
        "\xF0\x9F\x9F\xAA", "\xF0\x9F\x9F\xAB"
    };
    for (;;) {
        bool removed = false;
        for (const auto swatch : swatches) {
            if (name.substr(0, swatch.size()) != swatch) continue;
            name.remove_prefix(swatch.size());
            // Optional Unicode text/emoji presentation selector on this swatch.
            if (name.substr(0, 3) == "\xEF\xB8\x8E" || name.substr(0, 3) == "\xEF\xB8\x8F")
                name.remove_prefix(3);
            trim();
            removed = true;
            break;
        }
        if (!removed) break;
    }
    return std::string(name);
}

} // namespace Slic3r
#endif
