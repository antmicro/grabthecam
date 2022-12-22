get_filename_component(_dir "${CMAKE_CURRENT_LIST_DIR}" PATH)
get_filename_component(_prefix "${_dir}/../.." ABSOLUTE)
set(grabthecam_INCLUDE_DIRS "${_prefix}/include")
include("${_prefix}/lib/cmake/grabthecam/grabthecam-targets.cmake")

message(STATUS "grabthecam found. Headers: ${grabthecam_INCLUDE_DIRS}")
