add_executable(blink
        main.c
)

target_link_libraries(blink 
        pico_stdlib
        freertos
        hardware_pio
        pico_multicore
        hardware_i2c
        pico_time
        #pico_lwip
        #hardware_rtc
        hardware_pwm 
        )

pico_enable_stdio_usb(blink 1)
pico_enable_stdio_uart(blink 0)        

pico_add_extra_outputs(blink)

add_subdirectory(inc)
