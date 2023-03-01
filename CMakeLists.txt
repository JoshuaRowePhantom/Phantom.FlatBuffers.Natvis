cmake_minimum_required (VERSION 3.24)

add_subdirectory(src)
include(share/PhantomFlatBuffersNatvisTargets.cmake)
add_subdirectory(test)
