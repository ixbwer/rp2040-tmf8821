# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

set(PICO_BOARD pico CACHE STRING "Board type")
cmake_minimum_required(VERSION 3.13)

file(GLOB TMF8828_A_FILES "tmf8828_a/*")
file(GLOB ST7789_FILES "st7789/*")
add_executable(hello
    hello.cpp
    ${TMF8828_A_FILES}
    ${ST7789_FILES}
)

include_directories(tmf8828_a)
include_directories(st7789)
target_link_libraries(hello 
    pico_stdlib 
    hardware_i2c
    hardware_spi
    hardware_gpio
    pico_multicore
    )
# set PICO_SDK_PATH
set(PICO_SDK_PATH "../pico-sdk")
include(pico_sdk_import.cmake)

project(hello C CXX ASM)
pico_sdk_init()
pico_enable_stdio_usb(hello 1)
pico_enable_stdio_uart(hello 1)
pico_add_extra_outputs(hello)