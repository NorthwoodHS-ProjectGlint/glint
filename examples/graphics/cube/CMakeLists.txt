cmake_minimum_required(VERSION 3.16)
project(cube_example)
set(CMAKE_CXX_STANDARD 17)

# Find dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLINT REQUIRED glint)
pkg_check_modules(GLFW REQUIRED glfw3)

# Build the shared library
add_library(${PROJECT_NAME} SHARED
    src/main.cpp
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${GLINT_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${GLINT_LIBRARIES}
    ${GLFW_LIBRARIES}
)

# Package into .glt file
add_custom_target(run_glt_${PROJECT_NAME} ALL
    COMMAND glt_execcreate $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_CURRENT_SOURCE_DIR}/.titleconfig
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)