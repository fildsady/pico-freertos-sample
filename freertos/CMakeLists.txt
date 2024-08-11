#set(FREERTOS_KERNEL_PATH ${PICO_SDK_PATH}/lib/tinyusb/lib/FreeRTOS/FreeRTOS/Source/)
set(FREERTOS_KERNEL_PATH FreeRTOS-Kernel)

add_library(freertos
    ${FREERTOS_KERNEL_PATH}/event_groups.c
    ${FREERTOS_KERNEL_PATH}/list.c
    ${FREERTOS_KERNEL_PATH}/queue.c
    ${FREERTOS_KERNEL_PATH}/stream_buffer.c
    ${FREERTOS_KERNEL_PATH}/tasks.c
    ${FREERTOS_KERNEL_PATH}/timers.c
    ${FREERTOS_KERNEL_PATH}/portable/MemMang/heap_4.c
    ${FREERTOS_KERNEL_PATH}/portable/GCC/ARM_CM0/port.c
   # port.c
)


target_include_directories(freertos PUBLIC
    .
    ${FREERTOS_KERNEL_PATH}/include
    ${FREERTOS_KERNEL_PATH}/portable/GCC/ARM_CM0
)


