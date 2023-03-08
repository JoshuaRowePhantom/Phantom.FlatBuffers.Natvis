cmake_minimum_required(VERSION 3.24)

find_package(flatbuffers CONFIG REQUIRED )
include("${flatbuffers_DIR}/FlatcTargets.cmake")
find_program(FLATBUFFERS_FLATC_EXECUTABLE flatc)
include("${flatbuffers_DIR}/BuildFlatBuffers.cmake")

add_subdirectory(src)
include(share/PhantomFlatBuffersNatvisTargets.cmake)
add_subdirectory(reflection)
add_subdirectory(test)
