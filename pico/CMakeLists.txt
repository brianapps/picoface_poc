# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NEVER EDIT THE NEXT LINES for Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 1.5.1)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
include(${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
# ====================================================================================

set(WIFI_ENABLE 0 CACHE BOOL "Enable wifi server, needs pico-w board.")

if("${WIFI_ENABLE}")
        message(STATUS "Wifi is enabled")
        set(PICO_BOARD pico_w CACHE STRING "Board type" FORCE)
        set(WIFI_SSID CACHE STRING "The WIFI SSID")
        set(WIFI_PASSWORD CACHE STRING "The WIFI Password")
        if (NOT WIFI_SSID)
                message(FATAL_ERROR "WIFI SSID not supplied, use -DWIFI_SSID=<ssid>.")
        endif()
        if (NOT WIFI_PASSWORD)
                message(FATAL_ERROR "WIFI PASSWORD not supplied, use -DWIFI_PASSWORD=<password>.")
        endif()
        set(WIFI_LIBS pico_cyw43_arch_lwip_poll)
else()
        message(STATUS "Wifi is disabled")
        set(PICO_BOARD pico CACHE STRING "Board type" FORCE)
endif()


# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(picoface C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

add_subdirectory(littlefs-lib)
add_subdirectory(FatFs_SPI)


add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/nmirom.bin ${CMAKE_CURRENT_BINARY_DIR}/nmirom.sym
        COMMAND sjasmplus --sym=${CMAKE_CURRENT_BINARY_DIR}/nmirom.sym ${CMAKE_CURRENT_SOURCE_DIR}/../nmirom/nmirom.asm
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../nmirom/nmirom.asm
)


add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/nmi.h ${CMAKE_CURRENT_BINARY_DIR}/nmi.c
        COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/asm_to_c_file.py
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/nmirom.bin ${CMAKE_CURRENT_BINARY_DIR}/nmirom.sym
)


# Add executable. Default name is the project name, version 0.1

add_executable(picoface 
        picoface.cxx
        lz4.c
        http_server.cxx
        usb_command_handler.cxx
        file_wrapper.cxx
        sd_config.c
        piohandler.S
        ${CMAKE_CURRENT_BINARY_DIR}/nmi.c
        ${CMAKE_CURRENT_BINARY_DIR}/nmi.h)

pico_set_program_name(picoface "picoface")
pico_set_program_version(picoface "0.1")

# Generate PIO header
pico_generate_pio_header(picoface ${CMAKE_CURRENT_LIST_DIR}/picoface.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(picoface 0)
pico_enable_stdio_usb(picoface 1)

# Add the standard library to the build
target_link_libraries(picoface
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(picoface PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)


if("${WIFI_ENABLE}")
        target_compile_definitions(picoface PRIVATE WIFI_ENABLE)
endif()


target_compile_definitions(picoface PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
        )

# Add any user requested libraries
target_link_libraries(picoface 
        hardware_pio
        pico_multicore
        FatFs_SPI
      
        "${WIFI_LIBS}"
        )

target_link_libraries(picoface littlefs-lib)

pico_add_extra_outputs(picoface)

