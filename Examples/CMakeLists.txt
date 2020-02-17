cmake_minimum_required(VERSION 3.12)

project (Examples)

file(GLOB_RECURSE EXAMPLES_SOURCES "src/*.cpp")
file(GLOB_RECURSE EXAMPLES_HEADERS "src/*.h")

add_executable(Examples ${EXAMPLES_SOURCES} ${EXAMPLES_HEADERS})
