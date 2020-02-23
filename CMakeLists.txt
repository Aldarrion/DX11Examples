cmake_minimum_required(VERSION 3.12)

# General settings
set(CMAKE_CONFIGURATION_TYPES "Release;Debug;RelWithDebInfo")

set(CMAKE_CXX_STANDARD 14)

# Basic tutorials
add_subdirectory(Tutorial1)
add_subdirectory(Tutorial2)


# Framework library
set(PROJ_NAME Examples)
project(${PROJ_NAME})

file(GLOB_RECURSE FRAMEWORK_HEADERS "Framework/include/*.h")
file(GLOB_RECURSE FRAMEWORK_SOURCES "Framework/src/*.cpp")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Framework/include" PREFIX "Framework" FILES ${FRAMEWORK_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Framework/src" PREFIX "Framework" FILES ${FRAMEWORK_SOURCES})

add_library(Framework ${FRAMEWORK_HEADERS} ${FRAMEWORK_SOURCES})

target_include_directories(Framework PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Framework/include")
target_include_directories(Framework PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Extern/include")

# Examples executable
file(GLOB_RECURSE EXAMPLES_SOURCES "Examples/src/*.cpp")
file(GLOB_RECURSE EXAMPLES_HEADERS "Examples/src/*.h")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Examples/src" PREFIX "Examples" FILES ${EXAMPLES_HEADERS} ${EXAMPLES_SOURCES})

add_executable(${PROJ_NAME} WIN32 ${EXAMPLES_SOURCES} ${EXAMPLES_HEADERS})

target_include_directories(${PROJ_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Extern/include")
target_include_directories(${PROJ_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Examples/include")
target_include_directories(${PROJ_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Framework/include")

target_link_libraries(${PROJ_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/Extern/lib/64/assimp-vc140-mt.lib")
target_link_libraries(${PROJ_NAME} d3dcompiler d3d11)
target_link_libraries(${PROJ_NAME} Framework)

set_property(TARGET ${PROJ_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Examples")

add_custom_command(TARGET ${PROJ_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/Extern/lib/64/assimp-vc140-mt.dll" "${CMAKE_CURRENT_BINARY_DIR}/Debug"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/Extern/lib/64/assimp-vc140-mt.dll" "${CMAKE_CURRENT_BINARY_DIR}/Release"
)

set(FTIME_TRACE 0 CACHE BOOL "-ftime-trace for clang")
if (FTIME_TRACE)
    target_compile_options(Framework PRIVATE -ftime-trace)
    target_compile_options(${PROJ_NAME} PRIVATE -ftime-trace)
endif()

if(MSVC)
    add_definitions(/MP)
endif()

# Solution name
project (DX11Examples)
