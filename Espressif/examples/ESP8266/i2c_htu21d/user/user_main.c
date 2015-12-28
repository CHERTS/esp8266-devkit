/*
 *  Examples of work with humidity and temperature sensor HTU21D
 *  https://www.sparkfun.com/products/12064
 *
 */

#include "ets_sys.h"
#include "driver/i2c.h"
#include "driver/uart.h"
#include "osapi.h"
#include "os_type.h"
#include "user_interface.h"
#include "user_config.h"

extern int ets_uart_printf(const char *fmt, ...);

#define HTDU21D_ADDRESS 0x40  //Unshifted 7-bit I2C address for the sensor

#define TRIGGER_TEMP_MEASURE_HOLD 0xE3
#define TRIGGER_HUMD_MEASURE_HOLD 0xE5
#define TRIGGER_TEMP_MEASURE_NOHOLD 0xF3
#define TRIGGER_HUMD_MEASURE_NOHOLD 0xF5
#define WRITE_USER_REG 0xE6
#define READ_USER_REG 0xE7
#define SOFT_RESET 0xFE

os_event_t user_procTaskQueue[user_procTaskQueueLen];
static void user_procTask(os_event_t *events);
static volatile os_timer_t sensor_timer;

void sensor_timerfunc(void *arg)
{
    uint8 ack;
    uint8 i;
    uint8 pData[4];
    uint8 address;
    uint8 real_address;
	char temper[80];

    //Read data
    ets_uart_printf("Sensor pull: ");
    i2c_start(); //Start i2c
    i2c_writeByte(HTDU21D_ADDRESS << 1); //write address 0x40
    ack = i2c_check_ack(); // Get ack from slave
    if (!ack) {
    	ets_uart_printf("addr not ack when tx write cmd\r\n");
        i2c_stop();
        return;
    }
    else
    	ets_uart_printf("ack ");
    os_delay_us(10);
    i2c_writeByte(TRIGGER_TEMP_MEASURE_NOHOLD);
    ack = i2c_check_ack();
    if (!ack)
    	ets_uart_printf("ack ");
    os_delay_us(100);
    ack = 1;
    while (ack) {
        i2c_start();
        address = HTDU21D_ADDRESS << 1;
        address |= 1;
        i2c_writeByte(address);
        ack = i2c_check_ack();
    }
    ets_uart_printf("ack ");

    uint16_t t = i2c_readByte();
    i2c_send_ack(1);
    t <<= 8;
    t |= i2c_readByte();
    i2c_send_ack(1);

    uint8_t crc = i2c_readByte();
    i2c_send_ack(1);
    i2c_stop();

    float temp = t;
    temp *= 175.72;
    temp /= 65536;
    temp -= 46.85;

	os_sprintf(temper, "Temperature = %d\r\n", (uint16)temp);
    ets_uart_printf(temper);
    ets_uart_printf("Done\r\n");
}

static void ICACHE_FLASH_ATTR
user_procTask(os_event_t *events)
{
    os_delay_us(5000);
}

void user_rf_pre_init(void)
{
}

void user_init(void)
{
    //Init uart
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
    os_delay_us(1000);

    ets_uart_printf("Booting...\r\n");

    // i2c
    i2c_init();

    //Soft reset the HTU
    i2c_start();
    i2c_writeByte(HTDU21D_ADDRESS << 1);
    i2c_check_ack();
    i2c_writeByte(SOFT_RESET);
    i2c_check_ack();
    i2c_stop();

    //Disarm timer
    os_timer_disarm(&sensor_timer);

    //Setup timer
    os_timer_setfn(&sensor_timer, (os_timer_func_t *)sensor_timerfunc, NULL);

    //Arm timer for every 5 sec.
    os_timer_arm(&sensor_timer, 5000, 1);

    system_os_task(user_procTask, user_procTaskPrio,user_procTaskQueue, user_procTaskQueueLen);
}

