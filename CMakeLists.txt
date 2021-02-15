# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initalize pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "/home/neggie/pico/pico-sdk")

# Pull in Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(fbconsole-test C CXX)

# Initialise the Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(fbconsole-test
        main.cpp
        fb_setup.cpp
        ili9341.cpp
        FBConsole.cpp)

pico_set_program_name(fbconsole-test "fbconsole-test")
pico_set_program_version(fbconsole-test "0.1")

pico_enable_stdio_uart(fbconsole-test 1)
pico_enable_stdio_usb(fbconsole-test 1)

# Add the standard library to the build
target_link_libraries(fbconsole-test pico_stdlib)

# Add any user requested libraries
target_link_libraries(fbconsole-test
        hardware_spi
        )

pico_add_extra_outputs(fbconsole-test)

