cmake_minimum_required(VERSION 3.10)
project(axdigi C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Include GNUInstallDirs for standard installation paths
include(GNUInstallDirs)
include_directories(include)

# Add the libtnc subdirectory
add_subdirectory(libs/libtnc)

# Main executable
add_executable(axdigi src/main.c src/alias.c)
target_link_libraries(axdigi PRIVATE tnc m)
target_include_directories(axdigi PRIVATE libs/libtnc/include)

# Installation
install(TARGETS axdigi DESTINATION ${CMAKE_INSTALL_BINDIR})
