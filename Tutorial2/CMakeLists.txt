project (Tutorial2)

add_executable(Tutorial2 WIN32 "Tutorial02.cpp")

target_link_libraries(Tutorial2 d3d11 d3dcompiler)

set_property(TARGET Tutorial2 PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

