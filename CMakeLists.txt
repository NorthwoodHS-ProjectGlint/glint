cmake_minimum_required(VERSION 3.16)
project(glint)
set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)
pkg_check_modules(STB REQUIRED stb)

# Add library
add_library(glint SHARED
    src/io.cpp
    src/fs.cpp
    src/exec.cpp
    src/gl.cpp
    src/hid.cpp
    src/title.cpp

    src/glad.c
)

# Include directories
target_include_directories(glint
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_include_directories(glint PRIVATE
    ${STB_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
)

target_link_libraries(glint
    ${STB_LIBRARIES}
    ${GLFW_LIBRARIES}
)

set_target_properties(glint PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Installation
install(TARGETS glint
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY include/ DESTINATION include)
configure_file(glint.pc.in glint.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/glint.pc
    DESTINATION lib/pkgconfig
)

add_subdirectory(tools/glt_execcreate)
add_subdirectory(system/bootloader)