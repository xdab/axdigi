cmake_minimum_required(VERSION 3.10)
project(axdigi C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Include GNUInstallDirs for standard installation paths
include(GNUInstallDirs)
include_directories(include)

# Add the libtnc subdirectory
add_subdirectory(libs/libtnc)

file(GLOB SOURCES "src/*.c")
add_executable(axdigi ${SOURCES})
target_link_libraries(axdigi PRIVATE tnc m)
target_include_directories(axdigi PRIVATE libs/libtnc/include)

# ax_test: unit tests
add_executable(ax_test test/main_test.c test/test.c src/alias.c src/deduplicator.c src/digipeater.c src/packet.c)
target_link_libraries(ax_test PRIVATE tnc m)
target_include_directories(ax_test PRIVATE libs/libtnc/include)

# Installation
install(TARGETS axdigi DESTINATION ${CMAKE_INSTALL_BINDIR})
