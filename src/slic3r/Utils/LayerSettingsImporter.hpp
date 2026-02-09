#ifndef slic3r_LayerSettingsImporter_hpp_
#define slic3r_LayerSettingsImporter_hpp_

#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

#include "libslic3r/Slicing.hpp"

namespace Slic3r {

class ModelObject;
class DynamicPrintConfig;

// Represents a single layer height range with its settings
struct LayerSettingEntry {
    double z_min = 0.0;
    double z_max = 0.0;

    // Optional settings - use std::optional-like pattern with has_* flags
    bool has_sparse_infill_density = false;
    int sparse_infill_density = 0;  // 0-100

    bool has_wall_loops = false;
    int wall_loops = 0;

    bool has_top_shell_layers = false;
    int top_shell_layers = 0;

    bool has_bottom_shell_layers = false;
    int bottom_shell_layers = 0;

    bool has_sparse_infill_pattern = false;
    std::string sparse_infill_pattern;

    bool has_layer_height = false;
    double layer_height = 0.0;

    bool has_wall_sequence = false;
    std::string wall_sequence;  // "inner_outer", "outer_inner", "inner_outer_inner"

    bool has_infill_wall_overlap = false;
    int infill_wall_overlap = 0;  // -100 to 100 percent

    bool has_top_shell_thickness = false;
    double top_shell_thickness = 0.0;  // mm, 0 = disabled

    bool has_wall_generator = false;
    std::string wall_generator;  // "classic", "arachne"

    bool has_is_infill_first = false;
    bool is_infill_first = false;

    bool has_bridge_speed = false;
    double bridge_speed = 0.0;  // mm/s

    bool has_internal_solid_infill_speed = false;
    double internal_solid_infill_speed = 0.0;  // mm/s

    bool has_internal_solid_infill_pattern = false;
    std::string internal_solid_infill_pattern;

    void from_json(const nlohmann::json& j);
};

// Container for parsed JSON data
struct LayerSettingsData {
    std::string version;
    std::vector<LayerSettingEntry> layers;

    // Parse from JSON file
    static LayerSettingsData parse_from_file(const std::string& filepath);

    // Validate the parsed data
    // Returns empty string if valid, error message otherwise
    std::string validate() const;

    // Validate against a specific object's dimensions
    // Returns empty string if valid, warning/error message otherwise
    std::string validate_against_object(const ModelObject* object) const;
};

class LayerSettingsImporter {
public:
    // Apply parsed settings to a ModelObject's layer_config_ranges
    // Returns true on success, false on failure
    // error_msg is populated on failure
    // default_layer_height: Required - used when JSON entry doesn't specify layer_height
    static bool apply_to_model_object(
        ModelObject* object,
        const LayerSettingsData& settings,
        double default_layer_height,
        std::string& error_msg,
        bool replace_existing = true
    );

    // Convert sparse_infill_pattern string to InfillPattern enum value
    // Returns -1 if pattern name is not recognized
    static int pattern_string_to_enum(const std::string& pattern);

    // Convert wall_sequence string to WallSequence enum value
    // Returns -1 if sequence name is not recognized
    // Valid values: inner_outer, outer_inner, inner_outer_inner
    static int wall_sequence_string_to_enum(const std::string& sequence);

    // Convert wall_generator string to PerimeterGeneratorType enum value
    // Returns -1 if generator name is not recognized
    // Valid values: classic, arachne
    static int wall_generator_string_to_enum(const std::string& generator);

private:
    // Create a DynamicPrintConfig from a LayerSettingEntry
    // default_layer_height is used if entry doesn't specify layer_height
    static DynamicPrintConfig create_config_from_entry(const LayerSettingEntry& entry, double default_layer_height);
};

} // namespace Slic3r

#endif // slic3r_LayerSettingsImporter_hpp_
