add_executable(sample_wave
        sample_wave.c
        )

# pull in common dependencies
target_link_libraries(sample_wave 
    pico_stdlib 
    hardware_uart 
    hardware_gpio
)

pico_enable_stdio_uart(sample_wave 0)   # Disable output via UART
pico_enable_stdio_usb(sample_wave 1)    # Diable output via USB

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(sample_wave)

# add url via pico_set_program_url
example_auto_set_url(sample_wave)
