#include "hardware/i2c.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>     // ใช้ฟังก์ชัน printf แบบกำหนดเอง
#include <string.h>
#include <math.h>
#include "pico/binary_info.h"

// กำหนดขา SDA และ SCL สำหรับการเชื่อมต่อ I2C
#define I2C_MODULE     i2c1
#define SDA_PIN  6
#define SCL_PIN  7

/* 
   โค้ดตัวอย่างเพื่อควบคุม LCD ขนาด 16x2 ผ่าน I2C bridge chip (เช่น PCF8574)

   หมายเหตุ: แผง LCD ต้องสามารถทำงานที่ 3.3V ไม่ใช่ 5V เนื่องจาก Pico GPIO
   (และ I2C) ไม่สามารถใช้งานที่ 5V ได้

   คุณจะต้องใช้ level shifter บนสาย I2C หากต้องการใช้งานบอร์ดที่ 5V

   การเชื่อมต่อบนบอร์ด Raspberry Pi Pico, บอร์ดอื่นอาจแตกต่างกัน

   GPIO 16 (พิน 20) -> SDA บนบอร์ด LCD bridge
   GPIO 17 (พิน 21) -> SCL บนบอร์ด LCD bridge
   3.3V (พิน 36) -> VCC บนบอร์ด LCD bridge
   GND (พิน 38)  -> GND บนบอร์ด LCD bridge
*/

// คำสั่งต่าง ๆ สำหรับ LCD
const int LCD_CLEARDISPLAY = 0x01;      // ล้างหน้าจอ
const int LCD_RETURNHOME = 0x02;        // ไปที่ตำแหน่งเริ่มต้น
const int LCD_ENTRYMODESET = 0x04;      // ตั้งค่าโหมดการป้อนข้อมูล
const int LCD_DISPLAYCONTROL = 0x08;    // ควบคุมการแสดงผล
const int LCD_CURSORSHIFT = 0x10;       // เลื่อนเคอร์เซอร์
const int LCD_FUNCTIONSET = 0x20;       // ตั้งค่าฟังก์ชัน
const int LCD_SETCGRAMADDR = 0x40;      // ตั้งค่า CGRAM address
const int LCD_SETDDRAMADDR = 0x80;      // ตั้งค่า DDRAM address

// ธงสำหรับโหมดการป้อนข้อมูล
const int LCD_ENTRYSHIFTINCREMENT = 0x01; // เพิ่มตำแหน่งเมื่อป้อนข้อมูล
const int LCD_ENTRYLEFT = 0x02;           // เคอร์เซอร์ไปทางซ้าย

// ธงสำหรับการควบคุมการแสดงผลและเคอร์เซอร์
const int LCD_BLINKON = 0x01;             // เปิดการกระพริบของเคอร์เซอร์
const int LCD_CURSORON = 0x02;            // เปิดเคอร์เซอร์
const int LCD_DISPLAYON = 0x04;           // เปิดการแสดงผล

// ธงสำหรับการเลื่อนการแสดงผลและเคอร์เซอร์
const int LCD_MOVERIGHT = 0x04;           // เลื่อนไปทางขวา
const int LCD_DISPLAYMOVE = 0x08;         // เลื่อนการแสดงผล

// ธงสำหรับการตั้งค่าฟังก์ชัน
const int LCD_5x10DOTS = 0x04;            // ขนาด 5x10 dots
const int LCD_2LINE = 0x08;               // 2 บรรทัด
const int LCD_8BITMODE = 0x10;            // โหมด 8 บิต

// ธงสำหรับการควบคุม backlight
const int LCD_BACKLIGHT = 0x08;

const int LCD_ENABLE_BIT = 0x04;          // ธงสำหรับการเปิดใช้งาน

// ที่อยู่บัส LCD แสดงตัวอย่างที่อยู่ 0x27
static int addr = 0x27;

// โหมดสำหรับฟังก์ชัน lcd_send_byte
#define LCD_CHARACTER  1
#define LCD_COMMAND    0

#define MAX_LINES      2
#define MAX_CHARS      16


char line0_buff [16];
char line1_buff [16];

void i2c_write_byte(uint8_t val);
void lcd_toggle_enable(uint8_t val);
void lcd_toggle_enable(uint8_t val);
void lcd_send_byte(uint8_t val, int mode);
void lcd_clear(void);
void lcd_set_cursor(int line, int position);
static void inline lcd_char(char val);
void lcd_string(const char *s);
void lcd_wireLine(const char *s);
void put_buff_to_lcd();
int lcd_buff_printf(int line, int position, const char *format, ...);
int lcd_printf(int line, int position, const char *format, ...);
void lcd_init();



/* ฟังก์ชันช่วยสำหรับการส่งข้อมูลบิตเดียว */
void i2c_write_byte(uint8_t val) {
#ifdef I2C_MODULE
    i2c_write_blocking(I2C_MODULE, addr, &val, 1, 0);
#endif
}

/* ฟังก์ชันเพื่อสลับการเปิดใช้งานของ LCD */
void lcd_toggle_enable(uint8_t val) {
    // สลับพิน enable บน LCD display
    // เราไม่สามารถทำสิ่งนี้เร็วเกินไปมิฉะนั้นจะไม่ทำงาน
#define DELAY_US 400
    sleep_us(DELAY_US);                // รอ 600 ไมโครวินาที
    i2c_write_byte(val | LCD_ENABLE_BIT); // ส่งบิตเปิดใช้งาน
    sleep_us(DELAY_US);                // รอ 600 ไมโครวินาที
    i2c_write_byte(val & ~LCD_ENABLE_BIT); // ปิดบิตเปิดใช้งาน
    sleep_us(DELAY_US);                // รอ 600 ไมโครวินาที
}

/* ส่งข้อมูลบิตไปยัง LCD โดยการแยกเป็นสองส่วน */
void lcd_send_byte(uint8_t val, int mode) {
    uint8_t high = mode | (val & 0xF0) | LCD_BACKLIGHT; // ค่าส่วนสูง
    uint8_t low = mode | ((val << 4) & 0xF0) | LCD_BACKLIGHT; // ค่าส่วนต่ำ

    i2c_write_byte(high);           // ส่งค่าส่วนสูง
    lcd_toggle_enable(high);        // สลับเปิดใช้งาน
    i2c_write_byte(low);            // ส่งค่าส่วนต่ำ
    lcd_toggle_enable(low);         // สลับเปิดใช้งาน
}

/* ล้างหน้าจอ LCD */
void lcd_clear(void) {
    lcd_send_byte(LCD_CLEARDISPLAY, LCD_COMMAND);
}

/* ตั้งค่าตำแหน่งเคอร์เซอร์บน LCD */
void lcd_set_cursor(int line, int position) {
    int val = (line == 0) ? 0x80 + position : 0xC0 + position;
    lcd_send_byte(val, LCD_COMMAND);
}

/* ส่งตัวอักษรเดียวไปยัง LCD */
static void inline lcd_char(char val) {
    lcd_send_byte(val, LCD_CHARACTER);
}

/* ส่งสตริงไปยัง LCD */
void lcd_string(const char *s) {
    while (*s) {
        lcd_char(*s++); // ส่งตัวอักษรไปยัง LCD
    }
}

void lcd_wireLine(const char *s) {
    int i = 16;
    while (i--) {
        if(*s == '\0'){
         lcd_char(0x20); 
         *s++;
        } else{
            lcd_char(*s++); // ส่งตัวอักษรไปยัง LCD
        }
    }
}

void lcd_clear_screen(){

    lcd_set_cursor(0,0);
    int i = 32;
    while (i--) {
        lcd_char(0x20); 
    }
}

void lcd_clear_buff_all(){
   memset(line0_buff,0x20,sizeof(line0_buff));
   memset(line1_buff,0x20,sizeof(line1_buff));
}

void lcd_clear_buff_line(uint8_t line){

    if(line==0)
    {
        memset(line0_buff,0x20,sizeof(line0_buff));
    }
    if(line==1)
    {
        memset(line1_buff,0x20,sizeof(line1_buff));
    }

}

void put_buff_to_lcd()
{
    lcd_set_cursor(0,0);
    lcd_wireLine(line0_buff);
    lcd_set_cursor(1,0);
    lcd_wireLine(line1_buff);
}

int lcd_buff_printf(int line, int position, const char *format, ...)
{
    size_t len;
    va_list ap;
    va_start(ap, format);
   
    if (line == 0){
            len = vsnprintf(&line0_buff[position], sizeof(line0_buff), format, ap); // สร้างข้อความที่ต้องการพิมพ์
    }
    if (line == 1){
            len = vsnprintf(&line1_buff[position], sizeof(line1_buff), format, ap); // สร้างข้อความที่ต้องการพิมพ์
    }      
    va_end(ap);
    return len;
}

/* ฟังก์ชัน printf แบบกำหนดเองเพื่อพิมพ์ข้อความบน LCD */
int lcd_printf(int line, int position, const char *format, ...) {
    char buf[32];   
    size_t len;
    va_list ap;
    va_start(ap, format);
    len = vsnprintf(buf, sizeof(buf), format, ap); // สร้างข้อความที่ต้องการพิมพ์
    lcd_set_cursor(line, position); // ตั้งค่าตำแหน่งเคอร์เซอร์
    lcd_string(buf); // ส่งข้อความไปยัง LCD
    va_end(ap);
    return len;
}

/* การเริ่มต้น LCD */
void lcd_init() {

    // ตั้งค่า I2C สำหรับการใช้งาน LCD
    i2c_init(I2C_MODULE, 550 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);
    // ทำให้ I2C pins สามารถเข้าถึงได้จาก picotool

    bi_decl(bi_2pins_with_func(SDA_PIN, SCL_PIN, GPIO_FUNC_I2C));
    lcd_send_byte(0x03, LCD_COMMAND); // ส่งคำสั่งเริ่มต้น
    lcd_send_byte(0x03, LCD_COMMAND);
    lcd_send_byte(0x03, LCD_COMMAND);
    lcd_send_byte(0x02, LCD_COMMAND);

    lcd_send_byte(LCD_ENTRYMODESET | LCD_ENTRYLEFT, LCD_COMMAND); // ตั้งค่าโหมดการป้อนข้อมูล
    lcd_send_byte(LCD_FUNCTIONSET | LCD_2LINE, LCD_COMMAND); // ตั้งค่าฟังก์ชัน LCD
    lcd_send_byte(LCD_DISPLAYCONTROL | LCD_DISPLAYON, LCD_COMMAND); // เปิดการแสดงผล
    lcd_clear(); // ล้างหน้าจอ
}