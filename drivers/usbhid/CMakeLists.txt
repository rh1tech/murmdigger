# CMakeLists.txt - USB HID Keyboard Driver
#
# Copyright (c) 2026 Mikhail Matveev <xtreme@rh1.tech>
# https://rh1.tech
#
# SPDX-License-Identifier: GPL-3.0-or-later

add_library(usbhid INTERFACE)

if(USB_HID_ENABLED)
    target_sources(usbhid INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/hid_app.c
        ${CMAKE_CURRENT_LIST_DIR}/usbhid_wrapper.c
    )

    target_include_directories(usbhid INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
    )

    target_link_libraries(usbhid INTERFACE
        tinyusb_host
        tinyusb_board
    )

    target_compile_definitions(usbhid INTERFACE
        USB_HID_ENABLED=1
    )
endif()
