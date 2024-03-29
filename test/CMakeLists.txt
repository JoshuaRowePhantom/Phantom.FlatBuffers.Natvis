cmake_minimum_required (VERSION 3.24)

find_package(gtest REQUIRED)

add_executable(
    Phantom.FlatBuffers.Natvis.Test
    DebuggerTest.cpp
)

flatbuffers_generate_headers(
    TARGET Phantom.FlatBuffers.Natvis.Test.FlatBuffers
    FLAGS --reflect-names --gen-name-strings --gen-compare --cpp-std c++17 --gen-object-api --scoped-enums --gen-mutable
    BINARY_SCHEMAS_DIR "${CMAKE_CURRENT_BINARY_DIR}"
    SCHEMAS
    DebuggerTestFlatBuffers.fbs)

target_link_libraries(
    Phantom.FlatBuffers.Natvis.Test
    PRIVATE
    flatbuffers::flatbuffers
    GTest::gtest
    GTest::gtest_main
    Phantom.FlatBuffers.Natvis.Test.FlatBuffers
)

phantom_flatbuffers_generate_natvis(
    TARGET Phantom.FlatBuffers.Natvis.Test.FlatBuffers
    NATVIS_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
)

set_target_properties(
    Phantom.FlatBuffers.Natvis.Test
    PROPERTIES
    CXX_STANDARD 20
)

gtest_discover_tests(
    Phantom.FlatBuffers.Natvis.Test)
