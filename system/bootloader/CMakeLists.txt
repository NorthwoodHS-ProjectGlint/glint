cmake_minimum_required(VERSION 3.16)
project(bootloader)
set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)

add_executable(bootloader
    src/boot.cpp
    src/setup.cpp
)

target_include_directories(bootloader PRIVATE
    ${GLFW_INCLUDE_DIRS}
)

target_include_directories(bootloader PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(bootloader
    glint
    ${GLFW_LIBRARIES}
)