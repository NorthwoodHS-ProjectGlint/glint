cmake_minimum_required(VERSION 3.16)
project(glt_execcreate)
set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CJSON REQUIRED IMPORTED_TARGET libcjson)

# Add library
add_executable(glt_execcreate
    src/main.cpp
)


target_link_libraries(glt_execcreate
    PRIVATE PkgConfig::CJSON
)
# Installation
install (TARGETS glt_execcreate
    RUNTIME DESTINATION bin
)