find_package(flatbuffers CONFIG REQUIRED )
include("${flatbuffers_DIR}/FlatcTargets.cmake")
find_program(FLATBUFFERS_FLATC_EXECUTABLE flatc)
include("${flatbuffers_DIR}/BuildFlatBuffers.cmake")
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
