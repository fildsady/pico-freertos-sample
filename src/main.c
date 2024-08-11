/*
 * LED blink with FreeRTOS
 */

#include <FreeRTOS.h>
#include <task.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "inc/i2c_lcd.h"
#include "pico/time.h"
#include "inc/ds3231.h"
#include "queue.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

#define DATA_PIN 10
#define CLOCK_PIN 11
#define LATCH_PIN 12

#define LED_PIN_1 25

// Pin assignment
#define CLK_PIN 27
#define DIO_PIN 26

struct led_task_arg {
    int gpio;
    int delay;
};
// ตัวแปร global สำหรับคิว
QueueHandle_t timeQueue;

TaskHandle_t LCDTaskHandle = NULL; // Handle for the time task
TaskHandle_t GetTimeTaskHandle = NULL; // Handle for the time task

float ds3231_temp = 0;

// ตัวแปร global สำหรับเก็บค่าเวลาและสถานะ LED
ds3231_datetime_t global_datetime;
char global_led_status[4];

const char* days_of_week[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};

// โครงสร้างสำหรับส่งข้อมูลเวลา
typedef struct {
    char time[20];
    ds3231_datetime_t datetime;
} time_data_t;


void get_time_task(void *pvParameters) {
    ds3231_datetime_t now;
    time_data_t time_data;

    gpio_init(LED_PIN_1);
    gpio_set_dir(LED_PIN_1, GPIO_OUT);
     

    while (1) {
        ds3231_get_datetime(&now);
        ds3231_temp = ds3231_get_temp();
        snprintf(time_data.time, sizeof(time_data.time), "%02d:%02d:%02d", now.hour, now.min, now.sec);
        time_data.datetime = now;

        //xQueueSend(timeQueue, &time_data, pdMS_TO_TICKS(10));
        if (xQueueSend(timeQueue, &time_data, pdMS_TO_TICKS(10)) != pdPASS) {
            //printf("Failed to send time to queue\n");
        }

        gpio_put(LED_PIN_1,1);
        vTaskDelay(pdMS_TO_TICKS(50)); 
        gpio_put(LED_PIN_1,0);
        vTaskDelay(pdMS_TO_TICKS(950)); 
    }
}

void lcd_task(void *p) {
    time_data_t time_data;

    while (1) {
        // รับข้อมูลเวลาจาก timeQueue
        if (xQueueReceive(timeQueue, &time_data, pdMS_TO_TICKS(10)) == pdPASS) {
            ds3231_datetime_t t = time_data.datetime;
            lcd_buff_printf(0, 0, "%02d/%02d/%02d %s", t.date, t.month, t.year-2000, days_of_week[t.day]);
            lcd_buff_printf(1, 0, "%02d:%02d:%02d %.1fc", t.hour, t.min, t.sec,ds3231_temp);
        }
        // รับข้อมูลสถานะ LED จาก ledStatusQueue
        put_buff_to_lcd();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}



void led_task(void *p)
{
    struct led_task_arg *a = (struct led_task_arg *)p;

    gpio_init(a->gpio);
    gpio_set_dir(a->gpio, GPIO_OUT);
    while (1) {
        gpio_put(a->gpio, 1);
        vTaskDelay(pdMS_TO_TICKS(a->delay));
        gpio_put(a->gpio, 0);
        vTaskDelay(pdMS_TO_TICKS(a->delay));
    }
}

int main()
{
    //=============================================================================
    stdio_init_all();  // Initialize standard I/O
    lcd_init();
    ds3231_init();
    //=============================================================================

    printf("Start LED blink\n");

    timeQueue = xQueueCreate(10, sizeof(time_data_t));

xTaskCreate(get_time_task,"GET TIME",configMINIMAL_STACK_SIZE,NULL,1,&GetTimeTaskHandle);
    xTaskCreate(lcd_task,"LCD DISPLAY",configMINIMAL_STACK_SIZE,NULL,0,&LCDTaskHandle);

    struct led_task_arg arg1 = { 15, 1000 };
    xTaskCreate(led_task, "LED_Task 1", 256, &arg1, 1, NULL);

    struct led_task_arg arg2 = { 14, 200 };
    xTaskCreate(led_task, "LED_Task 2", 256, &arg2, 1, NULL);

    struct led_task_arg arg3 = { 13, 300 };
    xTaskCreate(led_task, "LED_Task 3", 256, &arg3, 1, NULL);

    vTaskStartScheduler();

    while (1)
        ;
}
