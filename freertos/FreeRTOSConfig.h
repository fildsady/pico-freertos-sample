#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* Use Pico SDK ISR handlers */
#define vPortSVCHandler         isr_svcall
#define xPortPendSVHandler      isr_pendsv
#define xPortSysTickHandler     isr_systick

#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configUSE_TICKLESS_IDLE                 0
#define configCPU_CLOCK_HZ                      133000000
#define configTICK_RATE_HZ                      ( ( TickType_t ) 1000 ) 
#define configMAX_PRIORITIES                    5
#define configMINIMAL_STACK_SIZE                256
#define configMAX_TASK_NAME_LEN                 16
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_TASK_NOTIFICATIONS            1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES   3
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1
#define configQUEUE_REGISTRY_SIZE               10
#define configUSE_QUEUE_SETS                    1
#define configUSE_TIME_SLICING                  1
#define configUSE_NEWLIB_REENTRANT              0
#define configENABLE_BACKWARD_COMPATIBILITY     1
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 5
#define configSTACK_DEPTH_TYPE                  uint32_t
#define configMESSAGE_BUFFER_LENGTH_TYPE        size_t

/* Memory allocation related definitions. */
#define configSUPPORT_STATIC_ALLOCATION         0
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configAPPLICATION_ALLOCATED_HEAP        0
#define configTOTAL_HEAP_SIZE                   (128*1024) // ขนาด heap ทั้งหมด

/* Hook function related definitions. */
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configCHECK_FOR_STACK_OVERFLOW          0
#define configUSE_MALLOC_FAILED_HOOK            0
#define configUSE_DAEMON_TASK_STARTUP_HOOK      0

/* Run time and task stats gathering related definitions. */
#define configGENERATE_RUN_TIME_STATS           0
#define configUSE_TRACE_FACILITY                0
#define configUSE_STATS_FORMATTING_FUNCTIONS    0

/* Co-routine related definitions. */
#define configUSE_CO_ROUTINES                   0
#define configMAX_CO_ROUTINE_PRIORITIES         1

/* Software timer related definitions. */
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               ( configMAX_PRIORITIES - 1 ) 
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            1024

/* การกำหนดค่าพฤติกรรมการแทรกขัดจังหวะ */
#if FREE_RTOS_KERNEL_SMP // กำหนดโดย RP2040 SMP port ของ FreeRTOS
#define configNUMBER_OF_CORES                   2   // จำนวน core ของ CPU
#define configTICK_CORE                         0   // กำหนด core สำหรับ tick
#define configRUN_MULTIPLE_PRIORITIES           1   // รันหลายลำดับความสำคัญพร้อมกัน
#define configUSE_CORE_AFFINITY                 1   // ใช้ core affinity
#endif

/* เฉพาะ RP2040 */
#define configSUPPORT_PICO_SYNC_INTEROP         1   // สนับสนุนการทำงานร่วมกับ Pico sync
#define configSUPPORT_PICO_TIME_INTEROP         1   // สนับสนุนการทำงานร่วมกับ Pico time

/* Define to trap errors during development. */
#define configASSERT( x )

/* การกำหนดค่าการรวมฟังก์ชัน API */
#define INCLUDE_vTaskPrioritySet                1   // รวมฟังก์ชัน vTaskPrioritySet
#define INCLUDE_uxTaskPriorityGet               1   // รวมฟังก์ชัน uxTaskPriorityGet
#define INCLUDE_vTaskDelete                     1   // รวมฟังก์ชัน vTaskDelete
#define INCLUDE_vTaskSuspend                    1   // รวมฟังก์ชัน vTaskSuspend
#define INCLUDE_vTaskDelayUntil                 1   // รวมฟังก์ชัน vTaskDelayUntil
#define INCLUDE_vTaskDelay                      1   // รวมฟังก์ชัน vTaskDelay
#define INCLUDE_xTaskGetSchedulerState          1   // รวมฟังก์ชัน xTaskGetSchedulerState
#define INCLUDE_xTaskGetCurrentTaskHandle       1   // รวมฟังก์ชัน xTaskGetCurrentTaskHandle
#define INCLUDE_uxTaskGetStackHighWaterMark     1   // รวมฟังก์ชัน uxTaskGetStackHighWaterMark
#define INCLUDE_xTaskGetIdleTaskHandle          1   // รวมฟังก์ชัน xTaskGetIdleTaskHandle
#define INCLUDE_eTaskGetState                   1   // รวมฟังก์ชัน eTaskGetState
#define INCLUDE_xTimerPendFunctionCall          1   // รวมฟังก์ชัน xTimerPendFunctionCall
#define INCLUDE_xTaskAbortDelay                 1   // รวมฟังก์ชัน xTaskAbortDelay
#define INCLUDE_xTaskGetHandle                  1   // รวมฟังก์ชัน xTaskGetHandle
#define INCLUDE_xTaskResumeFromISR              1   // รวมฟังก์ชัน xTaskResumeFromISR
#define INCLUDE_xQueueGetMutexHolder            1   // รวมฟังก์ชัน xQueueGetMutexHolder

/* A header file that defines trace macro can be included here. */

#endif /* FREERTOS_CONFIG_H */
