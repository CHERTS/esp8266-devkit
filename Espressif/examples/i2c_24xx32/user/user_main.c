/*
 *  Test for read and write EEPROM ATMEL AT24C512
 *  http://www.atmel.com/Images/doc1116.pdf
 *  and EEPROM Microchip 24AA512
 *  http://ww1.microchip.com/downloads/en/DeviceDoc/21754M.pdf
 * 
 *  For a single device, connect as follows:
 *  EEPROM 4 (GND) to GND
 *  EEPROM 8 (Vcc) to Vcc (3.3 Volts)
 *  EEPROM 5 (SDA) to ESP Pin GPIO2
 *  EEPROM 6 (SCL) to ESP Pin GPIO0 (see i2c.h)
 *  EEPROM 7 (WP)  to GND
 *  EEPROM 1 (A0)  to GND
 *  EEPROM 2 (A1)  to GND
 *  EEPROM 3 (NC)  to NC
 *
 */

#include "ets_sys.h"
#include "driver/i2c.h"
#include "driver/uart.h"
#include "osapi.h"
#include "os_type.h"
#include "user_interface.h"
#include "user_config.h"

os_event_t user_procTaskQueue[user_procTaskQueueLen];
extern int ets_uart_printf(const char *fmt, ...);
int (*console_printf)(const char *fmt, ...) = ets_uart_printf;
static void user_procTask(os_event_t *events);

uint32_t time;
uint32_t finishTime;
uint32_t errors = 0;
uint32_t address = 0;
uint8_t loop_size;
uint32_t wdata, rdata;

static void ICACHE_FLASH_ATTR
user_procTask(os_event_t *events)
{
    os_delay_us(5000);
}

static void ICACHE_FLASH_ATTR writeByByteTest()
{
	time = system_get_time();
	errors = 0;
	address = 0;
	console_printf("-----------------------------------------------\r\n");
	console_printf("Write byte test:\r\n");
	console_printf("Start address: %d, End address: %d\r\n", MIN_ADDRESS, MAX_ADDRESS);
	console_printf("Writing data: ");
	for (address = MIN_ADDRESS; address <= MAX_ADDRESS; address++)
	{
		wdata = address % loop_size;
		if(!eeprom_writeByte(DEVICEADDRESS, address, wdata))
			console_printf("Failed write, address: %d, data: %d\r\n", address, (uint8_t)wdata);
		else
		{
			if (!(address % 500)) {
				//console_printf( "Address: %d, data: %d\r\n", address, (uint8_t)wdata);
				console_printf("..500..");
			}
		}
		sleepms(3);
	}
	finishTime = system_get_time() - time;
	console_printf("DONE!\r\n");
	console_printf("Total Time (seconds): %d\r\n", (uint32_t)(finishTime / 1000000));
	console_printf("Write operations per second: %d\r\n", (uint32_t)(MAX_ADDRESS / (finishTime / 1000000)));
	console_printf("-----------------------------------------------\r\n");
	console_printf("\r\n");
}

static void ICACHE_FLASH_ATTR readByByteTest()
{
	time = system_get_time();
	errors = 0;
	address = 0;
	console_printf("-----------------------------------------------\r\n");
	console_printf("Read byte test:\r\n");
	console_printf("Start address: %d, End address: %d\r\n", MIN_ADDRESS, MAX_ADDRESS);
	console_printf("Reading data:");
	for (address = MIN_ADDRESS; address <= MAX_ADDRESS; address++)
	{
		rdata = eeprom_readByte(DEVICEADDRESS, address);
		wdata = address % loop_size;
		if (rdata != (uint8_t)wdata)
		{
			console_printf("Address: %d", address);
			console_printf(" Should be: %d", (uint8_t)wdata);
			console_printf(" Read val: %d\r\n", rdata);
			errors++;
		}
		if (!(address % 500)) console_printf("..500..");
		sleepms(3);
	}
	finishTime = system_get_time() - time;
	console_printf("DONE\r\n");
	console_printf("Total time (seconds): %d\r\n", (uint32_t)(finishTime / 1000000));
	console_printf("Read operations per second: %d\r\n", (uint32_t)(MAX_ADDRESS / (finishTime / 1000000)));
	console_printf("Total errors: %d\r\n", errors);
	console_printf("-----------------------------------------------\r\n");
	console_printf("\r\n");
}

static void ICACHE_FLASH_ATTR eeprom_erase(uint8_t devaddr, uint32_t startaddr, uint32_t length)
{
    uint8_t msgff[16] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };
	uint32_t i;
	uint32_t stopaddr  = startaddr + length;
    for (i = startaddr; i < stopaddr; i += sizeof(msgff))
    {
        if(!eeprom_writePage(DEVICEADDRESS,  i, msgff, sizeof(msgff)))
    		console_printf("Failed to clean up the memory address %03x, block size %d\r\n",  i, sizeof(msgff));
    	else
    		console_printf("Memory address %03x, block size %d erased.\r\n", i, sizeof(msgff));
    }
}


static void ICACHE_FLASH_ATTR eeprom_dump(uint8_t devaddr, uint32_t addr, uint32_t length)
{
	uint32_t i;
	int j;
	char buffer[16];

	uint32_t startaddr = addr;
	uint32_t stopaddr  = addr + length;

    for (i = startaddr; i < stopaddr; i += 16) {
        console_printf("%03x: ", i);
        os_sprintf(buffer, "%s", eeprom_readPage(devaddr, i, 16), 16);
        for(j = 0; j < 16; j++) {
            if (j == 8) {
            	console_printf(" ");
            }
            console_printf("%02x ", buffer[j]);
        }
        console_printf(" | ");
        for(j = 0; j < 16; j++) {
        	console_printf("%c ", isprint(buffer[j]) ? buffer[j] : '.');
        }
        console_printf("|\r\n");
    }
}

void user_rf_pre_init(void)
{
}

void user_init(void)
{
	int i;
    uint8_t buffer[16];
    loop_size = rand();
    // Data to write
    uint8_t msg1[] = "Test Message #1";
    uint8_t msg2[] = "Test Message #2";
    uint8_t msg3[] = "Hello world!";

	//Init uart
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
    sleepms(1);

    console_printf("Booting...\r\n");
	console_printf("-----------------------------------------------\r\n");
    console_printf("AT24C512 Library Benchmark\r\n");
	console_printf("-----------------------------------------------\r\n");
    // i2c init
    i2c_init();

    // Erase memory
    eeprom_erase(DEVICEADDRESS, 0, 224);
	console_printf("-----------------------------------------------\r\n");

    // Write some stuff to EEPROM
	if(!eeprom_writePage(DEVICEADDRESS, 0x00, msg1, sizeof(msg1)))
		console_printf("Failed write, address: %d, data: %s\r\n", 0x00, msg1);
	else
		console_printf("Message 1 stored in the memory.\r\n");
	if(!eeprom_writePage(DEVICEADDRESS, 0x40, msg2, sizeof(msg2)))
		console_printf("Failed write, address: %d, data: %s\r\n", 0x40, msg2);
	else
		console_printf("Message 2 stored in the memory.\r\n");
	if(!eeprom_writePage(DEVICEADDRESS, 0x80, msg3, sizeof(msg3)))
		console_printf("Failed write, address: %d, data: %s\r\n",  0x80, msg3);
	else
		console_printf("Message 3 stored in the memory.\r\n");
	console_printf("-----------------------------------------------\r\n");

    // Read the first page in EEPROM memory, a byte at a time
	console_printf("EEPROM read byte, starting at 0x000:\r\n");
    for (i = 0; i < sizeof(msg1)-1; i++) {
    	uint8_t b = eeprom_readByte(DEVICEADDRESS, i);
    	console_printf("0x%02x ", b);
    }
    console_printf("\r\n");
    for (i = 0; i < sizeof(msg1)-1; i++) {
    	uint8_t b = eeprom_readByte(DEVICEADDRESS, i);
    	console_printf("%c ", isprint(b) ? b : '.');
    }
    console_printf("\r\n");
	console_printf("-----------------------------------------------\r\n");

    console_printf("EEPROM read buffer, starting at 0x000:\r\n");
    os_sprintf(buffer, "%s", eeprom_readPage(DEVICEADDRESS, 0, sizeof(buffer)), sizeof(buffer));
    //First print the hex bytes on this row
    for (i = 0; i < sizeof(buffer)-1; i++) {
        char outbuf[6];
    	console_printf("0x%02x ", buffer[i]);
    }
    console_printf("\r\n");
    for (i = 0; i < sizeof(buffer)-1; i++) {
        char outbuf[6];
    	console_printf("%c ", isprint(buffer[i]) ? buffer[i] : '.');
    }
    console_printf("\r\n");
	console_printf("-----------------------------------------------\r\n");

    // Now dump 224 bytes
    console_printf("EEPROM dump:\r\n");
    eeprom_dump(DEVICEADDRESS, 0, 224);

    // Start write benchmark test
    writeByByteTest();
    // Start read benchmark test
    readByByteTest();

    system_os_task(user_procTask, user_procTaskPrio,user_procTaskQueue, user_procTaskQueueLen);
}

