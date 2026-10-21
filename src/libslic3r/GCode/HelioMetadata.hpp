#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <iterator>
#include <string_view>
#include <system_error>
#include <fast_float/fast_float.h>

namespace Slic3r { namespace HelioMetadata {

inline bool parse_warpage_fields(std::string_view fields, std::array<float, 9>& values)
{
    static constexpr std::array<std::string_view, 9> keys = { "wdm", "wdx", "wdy", "wdz", "wr", "wtg", "wts", "whs", "wls" };
    bool parsed = false;

    while (!fields.empty()) {
        const size_t           delimiter = fields.find(',');
        std::string_view       field     = fields.substr(0, delimiter);
        if (!field.empty() && field.front() == '(')
            field.remove_prefix(1);
        if (!field.empty() && field.back() == ')')
            field.remove_suffix(1);
        const size_t           separator = field.find('=');
        if (separator != std::string_view::npos) {
            const auto key = std::find(keys.begin(), keys.end(), field.substr(0, separator));
            if (key != keys.end()) {
                const std::string_view number = field.substr(separator + 1);
                float                  value;
                const auto [end, error] = fast_float::from_chars(number.data(), number.data() + number.size(), value);
                if (error == std::errc() && end == number.data() + number.size() && std::isfinite(value)) {
                    values[std::distance(keys.begin(), key)] = value;
                    parsed = true;
                }
            }
        }
        if (delimiter == std::string_view::npos)
            break;
        fields.remove_prefix(delimiter + 1);
    }
    return parsed;
}

// Do not accept a numeric prefix of a malformed header value.
inline bool parse_percentile(std::string_view number, float& value)
{
    float parsed;
    const auto result = fast_float::from_chars(number.data(), number.data() + number.size(), parsed);
    if (result.ec != std::errc() || result.ptr != number.data() + number.size() || !std::isfinite(parsed))
        return false;
    value = parsed;
    return true;
}

}} // namespace Slic3r::HelioMetadata
