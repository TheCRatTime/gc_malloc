cmake_minimum_required(VERSION 3.10)

project(gcmalloc C)

include(CTest)

enable_testing()

add_executable(gc_test tests/gc_test.c)
target_include_directories(gc_test PRIVATE include)
add_test(NAME gc_memory_leak COMMAND gc_test)
