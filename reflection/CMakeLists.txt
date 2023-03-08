cmake_minimum_required (VERSION 3.24)

flatbuffers_generate_headers(
    TARGET Phantom.FlatBuffers.Natvis.Reflection
    FLAGS --reflect-names 
    BINARY_SCHEMAS_DIR "${CMAKE_CURRENT_BINARY_DIR}"
    SCHEMAS
    reflection-phantom-natvis.fbs)

phantom_flatbuffers_generate_natvis(
    TARGET Phantom.FlatBuffers.Natvis.Reflection
    NATVIS_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_custom_target(
    Phantom.FlatBuffers.Natvis.Reflection.Target
    ALL
    SOURCES
    reflection-phantom-natvis.natvis "${CMAKE_CURRENT_SOURCE_DIR}/reflection-phantom-natvis.fbs"
)
