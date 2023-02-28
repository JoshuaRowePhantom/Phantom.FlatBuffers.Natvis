cmake_minimum_required (VERSION 3.24)

add_executable(
    Phantom.FlatBuffers.Natvis
    main.cpp
)

target_link_libraries(
    Phantom.FlatBuffers.Natvis
    flatbuffers::flatbuffers
    Boost::program_options
)

set_target_properties(
    Phantom.FlatBuffers.Natvis
    PROPERTIES
    CXX_STANDARD 20
)
