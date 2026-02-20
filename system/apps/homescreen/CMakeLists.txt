cmake_minimum_required(VERSION 3.16)
project(home_screen)
set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)

add_library(home_screen SHARED
    src/main.cpp
    src/homescreen.cpp
)

target_include_directories(home_screen PRIVATE
    ${GLFW_INCLUDE_DIRS}
)

target_link_libraries(home_screen
    glint
    ${GLFW_LIBRARIES}
)

add_custom_target(run_glt ALL
    COMMAND glt_execcreate $<TARGET_FILE:home_screen> ${CMAKE_CURRENT_SOURCE_DIR}/.titleconfig
    DEPENDS home_screen
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)