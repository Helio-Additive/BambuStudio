#include "LayerSettingsImporter.hpp"

#include <fstream>
#include <algorithm>
#include <boost/log/trivial.hpp>
#include <boost/nowide/fstream.hpp>

#include "libslic3r/Model.hpp"
#include "libslic3r/PrintConfig.hpp"

namespace Slic3r {

void LayerSettingEntry::from_json(const nlohmann::json& j)
{
    // Required fields
    j.at("z_min").get_to(z_min);
    j.at("z_max").get_to(z_max);

    // Optional fields
    if (j.contains("sparse_infill_density")) {
        has_sparse_infill_density = true;
        j.at("sparse_infill_density").get_to(sparse_infill_density);
    }

    if (j.contains("wall_loops")) {
        has_wall_loops = true;
        j.at("wall_loops").get_to(wall_loops);
    }

    if (j.contains("top_shell_layers")) {
        has_top_shell_layers = true;
        j.at("top_shell_layers").get_to(top_shell_layers);
    }

    if (j.contains("bottom_shell_layers")) {
        has_bottom_shell_layers = true;
        j.at("bottom_shell_layers").get_to(bottom_shell_layers);
    }

    if (j.contains("sparse_infill_pattern")) {
        has_sparse_infill_pattern = true;
        j.at("sparse_infill_pattern").get_to(sparse_infill_pattern);
    }

    if (j.contains("layer_height")) {
        has_layer_height = true;
        j.at("layer_height").get_to(layer_height);
    }

    if (j.contains("wall_sequence")) {
        has_wall_sequence = true;
        j.at("wall_sequence").get_to(wall_sequence);
    }

    if (j.contains("infill_wall_overlap")) {
        has_infill_wall_overlap = true;
        j.at("infill_wall_overlap").get_to(infill_wall_overlap);
    }

    if (j.contains("top_shell_thickness")) {
        has_top_shell_thickness = true;
        j.at("top_shell_thickness").get_to(top_shell_thickness);
    }

    if (j.contains("wall_generator")) {
        has_wall_generator = true;
        j.at("wall_generator").get_to(wall_generator);
    }

    if (j.contains("is_infill_first")) {
        has_is_infill_first = true;
        j.at("is_infill_first").get_to(is_infill_first);
    }

    if (j.contains("bridge_speed")) {
        has_bridge_speed = true;
        j.at("bridge_speed").get_to(bridge_speed);
    }

    if (j.contains("internal_solid_infill_speed")) {
        has_internal_solid_infill_speed = true;
        j.at("internal_solid_infill_speed").get_to(internal_solid_infill_speed);
    }

    if (j.contains("internal_solid_infill_pattern")) {
        has_internal_solid_infill_pattern = true;
        j.at("internal_solid_infill_pattern").get_to(internal_solid_infill_pattern);
    }
}

LayerSettingsData LayerSettingsData::parse_from_file(const std::string& filepath)
{
    LayerSettingsData data;

    boost::nowide::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filepath);
    }

    nlohmann::json j;
    try {
        file >> j;
    } catch (const nlohmann::json::parse_error& e) {
        throw std::runtime_error(std::string("JSON parse error: ") + e.what());
    }

    // Parse version if present
    if (j.contains("version")) {
        j.at("version").get_to(data.version);
    }

    // Parse layer_settings array
    if (!j.contains("layer_settings")) {
        throw std::runtime_error("JSON missing required 'layer_settings' array");
    }

    const auto& layers_json = j.at("layer_settings");
    if (!layers_json.is_array()) {
        throw std::runtime_error("'layer_settings' must be an array");
    }

    data.layers.reserve(layers_json.size());
    for (const auto& layer_json : layers_json) {
        LayerSettingEntry entry;
        entry.from_json(layer_json);
        data.layers.push_back(entry);
    }

    return data;
}

std::string LayerSettingsData::validate() const
{
    if (layers.empty()) {
        return "No layer settings defined";
    }

    for (size_t i = 0; i < layers.size(); ++i) {
        const auto& layer = layers[i];

        // Check z_min < z_max
        if (layer.z_min >= layer.z_max) {
            return "Layer " + std::to_string(i + 1) + ": z_min (" +
                   std::to_string(layer.z_min) + ") must be less than z_max (" +
                   std::to_string(layer.z_max) + ")";
        }

        // Check non-negative z values
        if (layer.z_min < 0) {
            return "Layer " + std::to_string(i + 1) + ": z_min cannot be negative";
        }

        // Validate sparse_infill_density bounds
        if (layer.has_sparse_infill_density) {
            if (layer.sparse_infill_density < 0 || layer.sparse_infill_density > 100) {
                return "Layer " + std::to_string(i + 1) + ": sparse_infill_density (" +
                       std::to_string(layer.sparse_infill_density) + ") must be between 0 and 100";
            }
        }

        // Validate wall_loops bounds
        if (layer.has_wall_loops) {
            if (layer.wall_loops < 0 || layer.wall_loops > 10) {
                return "Layer " + std::to_string(i + 1) + ": wall_loops (" +
                       std::to_string(layer.wall_loops) + ") must be between 0 and 10";
            }
        }

        // Validate top_shell_layers bounds
        if (layer.has_top_shell_layers) {
            if (layer.top_shell_layers < 0 || layer.top_shell_layers > 20) {
                return "Layer " + std::to_string(i + 1) + ": top_shell_layers (" +
                       std::to_string(layer.top_shell_layers) + ") must be between 0 and 20";
            }
        }

        // Validate bottom_shell_layers bounds
        if (layer.has_bottom_shell_layers) {
            if (layer.bottom_shell_layers < 0 || layer.bottom_shell_layers > 20) {
                return "Layer " + std::to_string(i + 1) + ": bottom_shell_layers (" +
                       std::to_string(layer.bottom_shell_layers) + ") must be between 0 and 20";
            }
        }

        // Validate layer_height bounds
        if (layer.has_layer_height) {
            if (layer.layer_height < 0.04 || layer.layer_height > 0.6) {
                return "Layer " + std::to_string(i + 1) + ": layer_height (" +
                       std::to_string(layer.layer_height) + ") must be between 0.04 and 0.6 mm";
            }
        }

        // Validate infill pattern if specified
        if (layer.has_sparse_infill_pattern) {
            if (LayerSettingsImporter::pattern_string_to_enum(layer.sparse_infill_pattern) < 0) {
                return "Layer " + std::to_string(i + 1) + ": unknown sparse_infill_pattern '" +
                       layer.sparse_infill_pattern + "'";
            }
        }

        // Validate wall_sequence if specified
        if (layer.has_wall_sequence) {
            if (LayerSettingsImporter::wall_sequence_string_to_enum(layer.wall_sequence) < 0) {
                return "Layer " + std::to_string(i + 1) + ": unknown wall_sequence '" +
                       layer.wall_sequence + "' (valid: inner_outer, outer_inner, inner_outer_inner)";
            }
        }

        // Validate infill_wall_overlap bounds (negative values create gap, positive creates overlap)
        if (layer.has_infill_wall_overlap) {
            if (layer.infill_wall_overlap < -100 || layer.infill_wall_overlap > 100) {
                return "Layer " + std::to_string(i + 1) + ": infill_wall_overlap (" +
                       std::to_string(layer.infill_wall_overlap) + ") must be between -100 and 100";
            }
        }

        // Validate top_shell_thickness bounds
        if (layer.has_top_shell_thickness) {
            if (layer.top_shell_thickness < 0) {
                return "Layer " + std::to_string(i + 1) + ": top_shell_thickness cannot be negative";
            }
        }

        // Validate wall_generator if specified
        if (layer.has_wall_generator) {
            if (LayerSettingsImporter::wall_generator_string_to_enum(layer.wall_generator) < 0) {
                return "Layer " + std::to_string(i + 1) + ": unknown wall_generator '" +
                       layer.wall_generator + "' (valid: classic, arachne)";
            }
        }

        // Validate bridge_speed bounds
        if (layer.has_bridge_speed) {
            if (layer.bridge_speed < 0) {
                return "Layer " + std::to_string(i + 1) + ": bridge_speed cannot be negative";
            }
        }

        // Validate internal_solid_infill_speed bounds
        if (layer.has_internal_solid_infill_speed) {
            if (layer.internal_solid_infill_speed < 0) {
                return "Layer " + std::to_string(i + 1) + ": internal_solid_infill_speed cannot be negative";
            }
        }

        // Validate internal_solid_infill_pattern if specified
        if (layer.has_internal_solid_infill_pattern) {
            if (LayerSettingsImporter::pattern_string_to_enum(layer.internal_solid_infill_pattern) < 0) {
                return "Layer " + std::to_string(i + 1) + ": unknown internal_solid_infill_pattern '" +
                       layer.internal_solid_infill_pattern + "'";
            }
        }
    }

    // Check for overlapping ranges
    std::vector<LayerSettingEntry> sorted_layers = layers;
    std::sort(sorted_layers.begin(), sorted_layers.end(),
        [](const LayerSettingEntry& a, const LayerSettingEntry& b) {
            return a.z_min < b.z_min;
        });

    for (size_t i = 1; i < sorted_layers.size(); ++i) {
        if (sorted_layers[i].z_min < sorted_layers[i-1].z_max) {
            return "Overlapping height ranges detected: [" +
                   std::to_string(sorted_layers[i-1].z_min) + "-" +
                   std::to_string(sorted_layers[i-1].z_max) + "] and [" +
                   std::to_string(sorted_layers[i].z_min) + "-" +
                   std::to_string(sorted_layers[i].z_max) + "]";
        }
    }

    return "";  // Valid
}

std::string LayerSettingsData::validate_against_object(const ModelObject* object) const
{
    if (!object) {
        return "No object provided";
    }

    // Get object bounding box height
    BoundingBoxf3 bb = object->bounding_box();
    double object_height = bb.size().z();

    // Find max z in settings
    double max_z = 0;
    for (const auto& layer : layers) {
        max_z = std::max(max_z, layer.z_max);
    }

    // Warning if max height exceeds object (with 1% tolerance)
    if (max_z > object_height * 1.01) {
        BOOST_LOG_TRIVIAL(warning) << "Layer settings max height (" << max_z
                                   << " mm) exceeds object height (" << object_height << " mm)";
        // Return warning but don't block - this is non-fatal
    }

    return "";  // Valid
}

int LayerSettingsImporter::pattern_string_to_enum(const std::string& pattern)
{
    // Map pattern names to enum values
    // Using lowercase for comparison
    std::string lower_pattern = pattern;
    std::transform(lower_pattern.begin(), lower_pattern.end(), lower_pattern.begin(), ::tolower);

    static const std::map<std::string, int> pattern_map = {
        {"concentric",      ipConcentric},
        {"rectilinear",     ipRectilinear},
        {"grid",            ipGrid},
        {"line",            ipLine},
        {"cubic",           ipCubic},
        {"triangles",       ipTriangles},
        {"stars",           ipStars},
        {"gyroid",          ipGyroid},
        {"honeycomb",       ipHoneycomb},
        {"adaptivecubic",   ipAdaptiveCubic},
        {"adaptive_cubic",  ipAdaptiveCubic},
        {"monotonic",       ipMonotonic},
        {"monotonicline",   ipMonotonicLine},
        {"monotonic_line",  ipMonotonicLine},
        {"alignedrectilinear", ipAlignedRectilinear},
        {"3dhoneycomb",     ip3DHoneycomb},
        {"hilbertcurve",    ipHilbertCurve},
        {"hilbert_curve",   ipHilbertCurve},
        {"archimedean",     ipArchimedeanChords},
        {"octagram",        ipOctagramSpiral},
        {"lightning",       ipLightning},
        {"crosshatch",      ipCrossHatch},
        {"cross_hatch",     ipCrossHatch},
        {"zigzag",          ipZigZag},
    };

    auto it = pattern_map.find(lower_pattern);
    if (it != pattern_map.end()) {
        return it->second;
    }
    return -1;  // Unknown pattern
}

int LayerSettingsImporter::wall_sequence_string_to_enum(const std::string& sequence)
{
    // Map wall sequence names to WallSequence enum values
    std::string lower_seq = sequence;
    std::transform(lower_seq.begin(), lower_seq.end(), lower_seq.begin(), ::tolower);

    static const std::map<std::string, int> sequence_map = {
        {"inner_outer",         static_cast<int>(WallSequence::InnerOuter)},
        {"innerouter",          static_cast<int>(WallSequence::InnerOuter)},
        {"inner outer",         static_cast<int>(WallSequence::InnerOuter)},
        {"outer_inner",         static_cast<int>(WallSequence::OuterInner)},
        {"outerinner",          static_cast<int>(WallSequence::OuterInner)},
        {"outer inner",         static_cast<int>(WallSequence::OuterInner)},
        {"inner_outer_inner",   static_cast<int>(WallSequence::InnerOuterInner)},
        {"innerouterinner",     static_cast<int>(WallSequence::InnerOuterInner)},
        {"inner outer inner",   static_cast<int>(WallSequence::InnerOuterInner)},
    };

    auto it = sequence_map.find(lower_seq);
    if (it != sequence_map.end()) {
        return it->second;
    }
    return -1;  // Unknown sequence
}

int LayerSettingsImporter::wall_generator_string_to_enum(const std::string& generator)
{
    // Map wall generator names to PerimeterGeneratorType enum values
    std::string lower_gen = generator;
    std::transform(lower_gen.begin(), lower_gen.end(), lower_gen.begin(), ::tolower);

    static const std::map<std::string, int> generator_map = {
        {"classic",     static_cast<int>(PerimeterGeneratorType::Classic)},
        {"arachne",     static_cast<int>(PerimeterGeneratorType::Arachne)},
    };

    auto it = generator_map.find(lower_gen);
    if (it != generator_map.end()) {
        return it->second;
    }
    return -1;  // Unknown generator
}

DynamicPrintConfig LayerSettingsImporter::create_config_from_entry(const LayerSettingEntry& entry, double default_layer_height)
{
    DynamicPrintConfig config;

    // IMPORTANT: Always set layer_height - the slicing code requires it
    // Use the value from JSON if specified, otherwise use the default
    double layer_height = entry.has_layer_height ? entry.layer_height : default_layer_height;
    config.set_key_value("layer_height", new ConfigOptionFloat(layer_height));

    // Also set extruder to 0 (use object default) as the existing code does
    config.set_key_value("extruder", new ConfigOptionInt(0));

    if (entry.has_sparse_infill_density) {
        config.set_key_value("sparse_infill_density",
            new ConfigOptionPercent(entry.sparse_infill_density));
    }

    if (entry.has_wall_loops) {
        config.set_key_value("wall_loops",
            new ConfigOptionInt(entry.wall_loops));
    }

    if (entry.has_top_shell_layers) {
        config.set_key_value("top_shell_layers",
            new ConfigOptionInt(entry.top_shell_layers));
    }

    if (entry.has_bottom_shell_layers) {
        config.set_key_value("bottom_shell_layers",
            new ConfigOptionInt(entry.bottom_shell_layers));
    }

    if (entry.has_sparse_infill_pattern) {
        int pattern_value = pattern_string_to_enum(entry.sparse_infill_pattern);
        if (pattern_value >= 0) {
            config.set_key_value("sparse_infill_pattern",
                new ConfigOptionEnum<InfillPattern>(static_cast<InfillPattern>(pattern_value)));
        }
    }

    if (entry.has_wall_sequence) {
        int sequence_value = wall_sequence_string_to_enum(entry.wall_sequence);
        if (sequence_value >= 0) {
            config.set_key_value("wall_sequence",
                new ConfigOptionEnum<WallSequence>(static_cast<WallSequence>(sequence_value)));
        }
    }

    if (entry.has_infill_wall_overlap) {
        config.set_key_value("infill_wall_overlap",
            new ConfigOptionPercent(entry.infill_wall_overlap));
    }

    if (entry.has_top_shell_thickness) {
        config.set_key_value("top_shell_thickness",
            new ConfigOptionFloat(entry.top_shell_thickness));
    }

    if (entry.has_wall_generator) {
        int generator_value = wall_generator_string_to_enum(entry.wall_generator);
        if (generator_value >= 0) {
            config.set_key_value("wall_generator",
                new ConfigOptionEnum<PerimeterGeneratorType>(static_cast<PerimeterGeneratorType>(generator_value)));
        }
    }

    if (entry.has_is_infill_first) {
        config.set_key_value("is_infill_first",
            new ConfigOptionBool(entry.is_infill_first));
    }

    if (entry.has_bridge_speed) {
        config.set_key_value("bridge_speed",
            new ConfigOptionFloatsNullable({entry.bridge_speed}));
    }

    if (entry.has_internal_solid_infill_speed) {
        config.set_key_value("internal_solid_infill_speed",
            new ConfigOptionFloatsNullable({entry.internal_solid_infill_speed}));
    }

    if (entry.has_internal_solid_infill_pattern) {
        int pattern_value = pattern_string_to_enum(entry.internal_solid_infill_pattern);
        if (pattern_value >= 0) {
            config.set_key_value("internal_solid_infill_pattern",
                new ConfigOptionEnum<InfillPattern>(static_cast<InfillPattern>(pattern_value)));
        }
    }

    return config;
}

bool LayerSettingsImporter::apply_to_model_object(
    ModelObject* object,
    const LayerSettingsData& settings,
    double default_layer_height,
    std::string& error_msg,
    bool replace_existing)
{
    if (!object) {
        error_msg = "No object provided";
        return false;
    }

    // Validate settings
    std::string validation_error = settings.validate();
    if (!validation_error.empty()) {
        error_msg = validation_error;
        return false;
    }

    // Validate against object (non-fatal warnings)
    settings.validate_against_object(object);

    // Clear existing ranges if requested
    if (replace_existing) {
        object->layer_config_ranges.clear();
    }

    // Apply each layer setting
    for (const auto& entry : settings.layers) {
        t_layer_height_range range(entry.z_min, entry.z_max);
        DynamicPrintConfig config = create_config_from_entry(entry, default_layer_height);

        // Assign to layer_config_ranges
        object->layer_config_ranges[range].assign_config(config);
    }

    BOOST_LOG_TRIVIAL(info) << "Imported " << settings.layers.size()
                            << " layer settings from JSON";

    return true;
}

} // namespace Slic3r
