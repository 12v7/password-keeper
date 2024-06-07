cmake_minimum_required(VERSION 3.13)

add_executable(keyboard)

target_sources(keyboard PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/main.c
    ${CMAKE_CURRENT_LIST_DIR}/usb_descriptors.c
    ${CMAKE_CURRENT_LIST_DIR}/cdc_task.c
    ${CMAKE_CURRENT_LIST_DIR}/hid_task.c
    ${CMAKE_CURRENT_LIST_DIR}/application.c
    ${CMAKE_CURRENT_LIST_DIR}/sha256.c
)

# Make sure TinyUSB can find tusb_config.h
target_include_directories(keyboard PUBLIC
    ${CMAKE_CURRENT_LIST_DIR})

# In addition to pico_stdlib required for common PicoSDK functionality, add dependency on tinyusb_device
# for TinyUSB device support and tinyusb_board for the additional board support library used by the example
target_link_libraries(keyboard PUBLIC
    pico_stdlib
    pico_unique_id
    tinyusb_device
    tinyusb_board
)

# Enable or Disable UART
#pico_enable_stdio_uart(keyboard 1)

# Enable or Disable USB CDC
#pico_enable_stdio_usb(keyboard 0)

pico_add_extra_outputs(keyboard)
