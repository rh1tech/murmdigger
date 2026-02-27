cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(murmdigger C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# Board variant (M1 or M2)
if(NOT DEFINED BOARD_VARIANT)
    set(BOARD_VARIANT "M2")
endif()

# CPU speed
if(NOT DEFINED CPU_SPEED)
    set(CPU_SPEED 252)
endif()

# Game sources (platform-independent)
set(GAME_SOURCES
    src/main.c
    src/game.c
    src/digger.c
    src/monster.c
    src/bags.c
    src/drawing.c
    src/sprite.c
    src/sound.c
    src/scores.c
    src/input.c
    src/keyboard.c
    src/record.c
    src/ini.c
    src/newsnd.c
    src/soundgen.c
    src/spinlock.c
    src/digger_math.c
    src/alpha.c
    src/title_gz.c
    src/cgagrafx.c
    src/digger_obj.c
    src/monster_obj.c
    src/bullet_obj.c
)

# RP2350-specific sources
set(RP2350_SOURCES
    src/rp2350_main.c
    src/rp2350_vid.c
    src/rp2350_kbd.c
    src/rp2350_snd.c
    src/rp2350_timer.c
    drivers/audio.c
    drivers/HDMI.c
)

add_executable(murmdigger
    ${GAME_SOURCES}
    ${RP2350_SOURCES}
)

# Compile definitions
target_compile_definitions(murmdigger PRIVATE
    _RP2350
    BOARD_${BOARD_VARIANT}
    CPU_CLOCK_MHZ=${CPU_SPEED}
)

# PS/2 keyboard driver (C++ library with PIO programs)
add_subdirectory(drivers/ps2kbd)

# Generate PIO header for I2S audio
pico_generate_pio_header(murmdigger
    ${CMAKE_CURRENT_LIST_DIR}/drivers/audio_i2s.pio
)

# Include directories
target_include_directories(murmdigger PRIVATE
    src
    drivers
)

# Link libraries
target_link_libraries(murmdigger
    pico_stdlib
    pico_multicore
    hardware_pio
    hardware_dma
    hardware_clocks
    hardware_vreg
    hardware_gpio
    hardware_irq
    hardware_resets
    hardware_sync
    hardware_flash
    ps2kbd
)

# Use custom linker script
pico_set_linker_script(murmdigger ${CMAKE_CURRENT_LIST_DIR}/memmap.ld)

# Generate UF2 and other output formats
pico_add_extra_outputs(murmdigger)

# Enable USB serial console for debug output
pico_enable_stdio_usb(murmdigger 1)
pico_enable_stdio_uart(murmdigger 0)
