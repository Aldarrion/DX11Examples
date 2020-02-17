set(PROJ_NAME Examples)

project(${PROJ_NAME})

file(GLOB_RECURSE EXAMPLES_SOURCES "src/*.cpp")
file(GLOB_RECURSE EXAMPLES_HEADERS "src/*.h")

add_executable(${PROJ_NAME} WIN32 ${EXAMPLES_SOURCES} ${EXAMPLES_HEADERS})

target_include_directories(${PROJ_NAME} PUBLIC "include")

target_link_libraries(${PROJ_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/lib/64/assimp-vc140-mt.lib")
target_link_libraries(${PROJ_NAME} d3dcompiler d3d11)

set_property(TARGET ${PROJ_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

add_custom_command(TARGET ${PROJ_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/lib/64/assimp-vc140-mt.dll" "${CMAKE_CURRENT_BINARY_DIR}/Debug"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/lib/64/assimp-vc140-mt.dll" "${CMAKE_CURRENT_BINARY_DIR}/Release"
)

