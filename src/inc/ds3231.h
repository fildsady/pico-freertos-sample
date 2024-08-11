#ifndef DS3231_H
#define DS3231_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define DS3231_ADDRESS 0x68
#define DS3231_CONTROL_REG 0x0E
#define DS_I2C_MODULE i2c0
#define DS_SCA_PIN 8
#define DS_SCL_PIN 9

typedef struct {
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t day;
    uint8_t date;
    uint8_t month;
    uint16_t year;
} ds3231_datetime_t;

static uint8_t bcd_to_bin(uint8_t val) { return val - 6 * (val >> 4); }
static uint8_t bin_to_bcd(uint8_t val) { return val + 6 * (val / 10); }

void ds3231_init_with_time(ds3231_datetime_t t);
void ds3231_set_sqw_1hz();
void i2c_read_register(uint8_t device_address, uint8_t register_address, uint8_t *data, size_t length);
void i2c_write_register(uint8_t device_address, uint8_t register_address, uint8_t *data, size_t length);
int ds3231_init();
void ds3231_set_sqw_1hz();
float ds3231_get_temp();
int ds3231_get_datetime(ds3231_datetime_t *datetime);
int ds3231_set_datetime(const ds3231_datetime_t *datetime);
//===================================================================================================
void i2c_read_register(uint8_t device_address, uint8_t register_address, uint8_t *data, size_t length) {
    i2c_write_blocking(i2c_default, device_address, &register_address, 1,1);
    i2c_read_blocking(i2c_default, device_address, data, length, 0);
}

void i2c_write_register(uint8_t device_address, uint8_t register_address, uint8_t *data, size_t length) {
    uint8_t buffer[length + 1];
    buffer[0] = register_address;
    memcpy(&buffer[1], data, length);
    i2c_write_blocking(i2c_default, device_address, buffer, length + 1, 0);
}

int ds3231_init() {
    i2c_init(DS_I2C_MODULE, 400 * 1000); // 400 kHz
    gpio_set_function(DS_SCA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(DS_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(DS_SCA_PIN);
    gpio_pull_up(DS_SCL_PIN);
    ds3231_set_sqw_1hz();
    return 1;
}

void ds3231_set_sqw_1hz() {
    uint8_t control_reg;

    // อ่านค่ารีจิสเตอร์ Control ปัจจุบัน
    i2c_read_register(DS3231_ADDRESS, DS3231_CONTROL_REG, &control_reg, 1);

    // ตั้งค่า RS2 และ RS1 เป็น 0 และ INTCN เป็น 0
    control_reg &= ~(1 << 3); // RS1 = 0
    control_reg &= ~(1 << 4); // RS2 = 0
    control_reg &= ~(1 << 2); // INTCN = 0

    // เขียนค่ากลับไปยังรีจิสเตอร์ Control
    i2c_write_register(DS3231_ADDRESS, DS3231_CONTROL_REG, &control_reg, 1);
}

int ds3231_get_datetime(ds3231_datetime_t *datetime) {
    uint8_t buf[7];
    uint8_t reg = 0x00; // Start reading at the first register
    if (i2c_write_blocking(DS_I2C_MODULE, DS3231_ADDRESS, &reg, 1, 1) != 1 ||
        i2c_read_blocking(DS_I2C_MODULE, DS3231_ADDRESS, buf, 7, 0) != 7) {
        return 0;
    }

    datetime->sec = bcd_to_bin(buf[0] & 0x7F); // Mask for seconds
    datetime->min = bcd_to_bin(buf[1] & 0x7F); // Mask for minutes
    datetime->hour = bcd_to_bin(buf[2] & 0x3F); // Mask for 24-hour format
    datetime->day = bcd_to_bin(buf[3] & 0x07); // Mask for day of the week (1-7)
    datetime->date = bcd_to_bin(buf[4] & 0x3F); // Mask for date
    datetime->month = bcd_to_bin(buf[5] & 0x1F); // Mask for month
    datetime->year = bcd_to_bin(buf[6]) + 2000; // Year 2000+

    return 1;
}

int ds3231_set_datetime(const ds3231_datetime_t *datetime) {
    uint8_t buf[8];
    buf[0] = 0x00; // Start writing at the first register
    buf[1] = bin_to_bcd(datetime->sec);
    buf[2] = bin_to_bcd(datetime->min);
    buf[3] = bin_to_bcd(datetime->hour);
    buf[4] = bin_to_bcd(datetime->day);
    buf[5] = bin_to_bcd(datetime->date);
    buf[6] = bin_to_bcd(datetime->month);
    buf[7] = bin_to_bcd(datetime->year - 2000);

    return i2c_write_blocking(DS_I2C_MODULE, DS3231_ADDRESS, buf, 8, 0) == 8;
}

// ฟังก์ชันสำหรับการตั้งค่า RTC
void ds3231_init_with_time(ds3231_datetime_t t){
   ds3231_init();
   ds3231_set_datetime(&t);
}
float ds3231_get_temp() {
    uint8_t msb, lsb;
    uint8_t reg = 0x11; // Temperature register
    i2c_write_blocking(DS_I2C_MODULE, DS3231_ADDRESS, &reg, 1, 1);
    i2c_read_blocking(DS_I2C_MODULE, DS3231_ADDRESS, &msb, 1, 0);
    reg = 0x12;
    i2c_write_blocking(DS_I2C_MODULE, DS3231_ADDRESS, &reg, 1, 1);
    i2c_read_blocking(DS_I2C_MODULE, DS3231_ADDRESS, &lsb, 1, 0);
    return msb + (lsb >> 6) * 0.25;
}

#endif