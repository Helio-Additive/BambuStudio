#include <catch2/catch.hpp>

#include "libslic3r/Model.hpp"
#include "libslic3r/Format/3mf.hpp"
#include "libslic3r/Format/bbs_3mf.hpp"
#include "miniz/miniz.h"
#include <fstream>
#include "libslic3r/Format/STL.hpp"

#include <boost/filesystem/operations.hpp>

using namespace Slic3r;

SCENARIO("Reading 3mf file", "[3mf]") {
    GIVEN("umlauts in the path of the file") {
        Model model;
        WHEN("3mf model is read") {
        	std::string path = std::string(TEST_DATA_DIR) + "/test_3mf/Geräte/Büchse.3mf";
        	DynamicPrintConfig config;
            ConfigSubstitutionContext ctxt{ ForwardCompatibilitySubstitutionRule::Disable };
            bool ret = load_3mf(path.c_str(), config, ctxt, &model, false);
            THEN("load should succeed") {
                REQUIRE(ret);
            }
        }
    }
}

SCENARIO("Export+Import geometry to/from 3mf file cycle", "[3mf]") {
    GIVEN("world vertices coordinates before save") {
        // load a model from stl file
        Model src_model;
        std::string src_file = std::string(TEST_DATA_DIR) + "/test_3mf/Prusa.stl";
        load_stl(src_file.c_str(), &src_model);
        src_model.add_default_instances();

        ModelObject* src_object = src_model.objects.front();

        // apply generic transformation to the 1st volume
        Geometry::Transformation src_volume_transform;
        src_volume_transform.set_offset({ 10.0, 20.0, 0.0 });
        src_volume_transform.set_rotation({ Geometry::deg2rad(25.0), Geometry::deg2rad(35.0), Geometry::deg2rad(45.0) });
        src_volume_transform.set_scaling_factor({ 1.1, 1.2, 1.3 });
        src_volume_transform.set_mirror({ -1.0, 1.0, -1.0 });
        src_object->volumes.front()->set_transformation(src_volume_transform);

        // apply generic transformation to the 1st instance
        Geometry::Transformation src_instance_transform;
        src_instance_transform.set_offset({ 5.0, 10.0, 0.0 });
        src_instance_transform.set_rotation({ Geometry::deg2rad(12.0), Geometry::deg2rad(13.0), Geometry::deg2rad(14.0) });
        src_instance_transform.set_scaling_factor({ 0.9, 0.8, 0.7 });
        src_instance_transform.set_mirror({ 1.0, -1.0, -1.0 });
        src_object->instances.front()->set_transformation(src_instance_transform);

        WHEN("model is saved+loaded to/from 3mf file") {
            // save the model to 3mf file
            std::string test_file = std::string(TEST_DATA_DIR) + "/test_3mf/prusa.3mf";
            store_3mf(test_file.c_str(), &src_model, nullptr, false);

            // load back the model from the 3mf file
            Model dst_model;
            DynamicPrintConfig dst_config;
            {
                ConfigSubstitutionContext ctxt{ ForwardCompatibilitySubstitutionRule::Disable };
                load_3mf(test_file.c_str(), dst_config, ctxt, &dst_model, false);
            }
            boost::filesystem::remove(test_file);

            // compare meshes
            TriangleMesh src_mesh = src_model.mesh();
            TriangleMesh dst_mesh = dst_model.mesh();

            bool res = src_mesh.its.vertices.size() == dst_mesh.its.vertices.size();
            if (res) {
                for (size_t i = 0; i < dst_mesh.its.vertices.size(); ++i) {
                    res &= dst_mesh.its.vertices[i].isApprox(src_mesh.its.vertices[i]);
                }
            }
            THEN("world vertices coordinates after load match") {
                REQUIRE(res);
            }
        }
    }
}

SCENARIO("2D convex hull of sinking object", "[3mf]") {
    GIVEN("model") {
        // load a model
        Model model;
        std::string src_file = std::string(TEST_DATA_DIR) + "/test_3mf/Prusa.stl";
        load_stl(src_file.c_str(), &model);
        model.add_default_instances();

        WHEN("model is rotated, scaled and set as sinking") {
            ModelObject* object = model.objects.front();
            object->center_around_origin(false);

            // set instance's attitude so that it is rotated, scaled and sinking
            ModelInstance* instance = object->instances.front();
            instance->set_rotation(X, -M_PI / 4.0);
            instance->set_offset(Vec3d::Zero());
            instance->set_scaling_factor({ 2.0, 2.0, 2.0 });

            // calculate 2D convex hull
            Polygon hull_2d = object->convex_hull_2d(instance->get_transformation().get_matrix());

            // verify result
            Points result = {
                { -91501496, -15914144 },
                { 91501496, -15914144 },
                { 91501496, 4243 },
                { 78229680, 4246883 },
                { 56898100, 4246883 },
                { -85501496, 4242641 },
                { -91501496, 4243 }
            };

            // Allow 1um error due to floating point rounding.
            bool res = hull_2d.points.size() == result.size();
            if (res)
                for (size_t i = 0; i < result.size(); ++ i) {
                    const Point &p1 = result[i];
                    const Point &p2 = hull_2d.points[i];
                    if (std::abs(p1.x() - p2.x()) > 1 || std::abs(p1.y() - p2.y()) > 1) {
                        res = false;
                        break;
                    }
                }

            THEN("2D convex hull should match with reference") {
                REQUIRE(res);
            }
        }
    }
}


// The standard member must stay printable even for consumers that do not
// understand Helio's optional preview metadata (including printer firmware).
TEST_CASE("Bambu 3MF keeps annotated preview separate from printer payload", "[3mf][helio]")
{
    namespace fs = boost::filesystem;
    const bool with_preview = GENERATE(false, true);
    const fs::path dir = fs::temp_directory_path() / fs::unique_path("helio-3mf-%%%%-%%%%");
    fs::create_directories(dir);
    struct Cleanup {
        fs::path path;
        ~Cleanup() { boost::system::error_code ec; fs::remove_all(path, ec); }
    } cleanup{dir};
    const std::string printable = "G1 X10 Y20 E1\n";
    const std::string preview = "G1 X10 Y20 E1 ; (wr=0.7)\n";
    const auto print_path = (dir / "print.gcode").string();
    const auto preview_path = (dir / "preview.gcode").string();
    std::ofstream(print_path, std::ios::binary) << printable;
    std::ofstream(preview_path, std::ios::binary) << preview;

    Model model;
    DynamicPrintConfig config;
    config.apply(FullPrintConfig::defaults());
    PlateData plate;
    plate.plate_index = 0;
    plate.is_sliced_valid = true;
    plate.gcode_file = print_path;
    if (with_preview)
        plate.preview_gcode_file = preview_path;
    const auto archive_path = (dir / "test.gcode.3mf").string();
    StoreParams params;
    params.path = archive_path.c_str();
    params.model = &model;
    params.config = &config;
    params.plate_data_list = {&plate};
    params.strategy = SaveStrategy::WithGcode | SaveStrategy::SkipModel | SaveStrategy::SkipAuxiliary | SaveStrategy::Silence;
    REQUIRE(store_bbs_3mf(params));

    mz_zip_archive zip{};
    REQUIRE(mz_zip_reader_init_file(&zip, archive_path.c_str(), 0));
    struct CloseZip {
        mz_zip_archive* zip;
        ~CloseZip() { mz_zip_reader_end(zip); }
    } close_zip{&zip};
    auto read_member = [&](const char* name) {
        size_t size = 0;
        void* bytes = mz_zip_reader_extract_file_to_heap(&zip, name, &size, 0);
        REQUIRE(bytes != nullptr);
        std::string result(static_cast<const char*>(bytes), size);
        mz_free(bytes);
        return result;
    };
    CHECK(read_member("Metadata/plate_1.gcode") == printable);
    const auto metadata = read_member("Metadata/model_settings.config");
    if (with_preview) {
        CHECK(read_member("Metadata/plate_1_helio_preview.gcode") == preview);
        CHECK(metadata.find("key=\"helio_preview_gcode_file\" value=\"Metadata/plate_1_helio_preview.gcode\"") != std::string::npos);
    } else {
        CHECK(mz_zip_reader_locate_file(&zip, "Metadata/plate_1_helio_preview.gcode", nullptr, 0) < 0);
        CHECK(metadata.find("helio_preview_gcode_file") == std::string::npos);
    }
    CHECK(metadata.find("key=\"gcode_file\" value=\"Metadata/plate_1.gcode\"") != std::string::npos);

    Model imported_model;
    DynamicPrintConfig imported_config;
    PlateDataPtrs imported_plates;
    struct ReleasePlates {
        PlateDataPtrs* plates;
        ~ReleasePlates() { release_PlateData_list(*plates); }
    } release_plates{&imported_plates};
    Semver version;
    std::ifstream input(archive_path, std::ios::binary);
    REQUIRE(load_gcode_3mf_from_stream(input, &imported_config, &imported_model, &imported_plates, &version));
    REQUIRE(imported_plates.size() == 1);
    CHECK(imported_plates.front()->gcode_file == "Metadata/plate_1.gcode");
    CHECK(imported_plates.front()->preview_gcode_file ==
        (with_preview ? "Metadata/plate_1_helio_preview.gcode" : ""));
}
