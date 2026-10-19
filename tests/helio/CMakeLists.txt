# Also buildable independently without the desktop GUI dependencies:
# cmake -S tests/helio -B /tmp/helio-tests && cmake --build /tmp/helio-tests
# ctest --test-dir /tmp/helio-tests --output-on-failure
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    cmake_minimum_required(VERSION 3.13)
    project(HelioMaterialNameTests LANGUAGES CXX)
    enable_testing()
endif()
add_executable(helio_material_name_tests material_name_tests.cpp)
target_compile_features(helio_material_name_tests PRIVATE cxx_std_17)
target_include_directories(helio_material_name_tests PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../../src")
add_test(NAME helio_material_name_tests COMMAND helio_material_name_tests)

add_executable(helio_warpage_metadata_tests warpage_metadata_tests.cpp)
target_compile_features(helio_warpage_metadata_tests PRIVATE cxx_std_17)
target_include_directories(helio_warpage_metadata_tests PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../../src")
add_test(NAME helio_warpage_metadata_tests COMMAND helio_warpage_metadata_tests)
