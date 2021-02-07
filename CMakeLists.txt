cmake_minimum_required(VERSION 3.0)
project(leakchek)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#
# leakcheck
#

add_library(leakcheck_static STATIC
    src/leakcheck/leakcheck.c
)

target_include_directories(leakcheck_static PUBLIC
    src
)

add_library(leakcheck SHARED
    src/leakcheck/api.c)

target_link_libraries(leakcheck
    "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/leakcheck.map"
    leakcheck_static
    dl)

install(TARGETS leakcheck LIBRARY DESTINATION lib)
install(FILES leakcheck DESTINATION bin)

#
# leakcheck_simple
#

add_library(leakcheck_simple_static STATIC
    src/leakcheck_simple/leakcheck_simple.c
)

target_include_directories(leakcheck_simple_static PUBLIC
    src
)

add_library(leakcheck_simple SHARED
    src/leakcheck_simple/api.c)

target_link_libraries(leakcheck_simple
    "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/leakcheck.map"
    leakcheck_simple_static
    dl)

install(TARGETS leakcheck_simple LIBRARY DESTINATION lib)
install(FILES leakcheck_simple DESTINATION bin)

#
# test executables
#

add_executable(test_no_alloc test/test_no_alloc.c)
add_executable(test_calloc test/test_calloc.c)
add_executable(test_malloc test/test_malloc.c)
add_executable(test_realloc test/test_realloc.c)
add_executable(test_invalid_free test/test_invalid_free.c)
add_executable(test_missing_free test/test_missing_free.c)

add_executable(test_mallinfo test/test_mallinfo.c)
