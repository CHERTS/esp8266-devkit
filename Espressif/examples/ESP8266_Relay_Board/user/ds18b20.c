/*
 * Adaptation of Paul Stoffregen's One wire library to the ESP8266 and
 * Necromant's Frankenstein firmware by Erland Lewin <erland@lewin.nu>
 * 
 * Paul's original library site:
 *   http://www.pjrc.com/teensy/td_libs_OneWire.html
 * 
 * See also http://playground.arduino.cc/Learning/OneWire
 * 
 */
#include "espmissingincludes.h"
#include "ets_sys.h"
#include "os_type.h"
#include "mem.h"
#include "osapi.h"
#include "gpio.h"
#include <stdlib.h>
#include "dht22.h"

#define DS18B20_PIN 2

#ifdef CONFIG_CMD_DS18B20_DEBUG
#define dbg(fmt, ...) os_printf(fmt, ##__VA_ARGS__)
#else
#define dbg(fmt, ...)
#endif

/*
 * static function prototypes
 */

static int ds_search( uint8_t *addr );
static void reset_search();
static void write_bit( int v );
static void write( uint8_t v, int parasitePower );
static inline int read_bit(void);
static inline void write_bit( int v );
static void select(const uint8_t rom[8]);
void ds_init(uint32_t polltime);
static uint8_t reset(void);
static uint8_t read();
static uint8_t crc8(const uint8_t *addr, uint8_t len);
int ds_str(char *buff, int sensornum);

uint8_t addr[4][8]; //DS18b20 addresses found, max 4
uint8_t numds;  //number of DS18B20s found
uint8_t currds; //current in poll queue

struct sensor_reading dsreading [4] = {
    {.source = "DS18B20-1", .success = 0},
	{.source = "DS18B20-2", .success = 0},
	{.source = "DS18B20-3", .success = 0},
	{.source = "DS18B20-4", .success = 0},
};

/*** 
 * CRC Table and code from Maxim AN 162:
 *  http://www.maximintegrated.com/en/app-notes/index.mvp/id/162
*/
unsigned const char ICACHE_STORE_ATTR ICACHE_RODATA_ATTR dscrc_table[] = {
0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
157,195, 33,127,252,162, 64, 30, 95, 1,227,189, 62, 96,130,220,
35,125,159,193, 66, 28,254,160,225,191, 93, 3,128,222, 60, 98,
190,224, 2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89, 7,
219,133,103, 57,186,228, 6, 88, 25, 71,165,251,120, 38,196,154,
101, 59,217,135, 4, 90,184,230,167,249, 27, 69,198,152,122, 36,
248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91, 5,231,185,
140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
202,148,118, 40,171,245, 23, 73, 8, 86,180,234,105, 55,213,139,
87, 9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53};

unsigned char dowcrc = 0;

static uint8_t byte_of_aligned_array(const uint8_t* aligned_array, uint32_t index)
{
    if( (((uint32_t)aligned_array)%4) != 0 ){
        os_printf("aligned_array is not 4-byte aligned.\n");
        return 0;
    }
    uint32_t v = ((uint32_t *)aligned_array)[ index/4 ];
    uint8_t *p = (uint8_t *) (&v);
    return p[ (index%4) ];
}

unsigned char ow_crc( unsigned char x){
    dowcrc = byte_of_aligned_array(dscrc_table,dowcrc^x);
    return dowcrc;
}

//
// Write a byte. The writing code uses the active drivers to raise the
// pin high, if you need power after the write (e.g. DS18S20 in
// parasite power mode) then set 'power' to 1, otherwise the pin will
// go tri-state at the end of the write to avoid heating in a short or
// other mishap.
//
static void ICACHE_FLASH_ATTR write( uint8_t v, int power ) {
	uint8_t bitMask;

	for (bitMask = 0x01; bitMask; bitMask <<= 1) {
		write_bit( (bitMask & v)?1:0);
	}
	if ( !power) {
		// noInterrupts();
		GPIO_DIS_OUTPUT( DS18B20_PIN );
		GPIO_OUTPUT_SET( DS18B20_PIN, 0 );
		// DIRECT_MODE_INPUT(baseReg, bitmask);
		// DIRECT_WRITE_LOW(baseReg, bitmask);
		// interrupts();
	}
}

//
// Write a bit. Port and bit is used to cut lookup time and provide
// more certain timing.
//
static inline void ICACHE_FLASH_ATTR write_bit( int v )
{
	// IO_REG_TYPE mask=bitmask;
	//	volatile IO_REG_TYPE *reg IO_REG_ASM = baseReg;

	GPIO_OUTPUT_SET( DS18B20_PIN, 0 );
	if( v ) {
		// noInterrupts();
		//	DIRECT_WRITE_LOW(reg, mask);
		//	DIRECT_MODE_OUTPUT(reg, mask);	// drive output low
		os_delay_us(10);
		GPIO_OUTPUT_SET( DS18B20_PIN, 1 );
		// DIRECT_WRITE_HIGH(reg, mask);	// drive output high
		// interrupts();
		os_delay_us(55);
	} else {
		// noInterrupts();
		//	DIRECT_WRITE_LOW(reg, mask);
		//	DIRECT_MODE_OUTPUT(reg, mask);	// drive output low
		os_delay_us(65);
		GPIO_OUTPUT_SET( DS18B20_PIN, 1 );
		//	DIRECT_WRITE_HIGH(reg, mask);	// drive output high
		//		interrupts();
		os_delay_us(5);
	}
}

//
// Read a bit. Port and bit is used to cut lookup time and provide
// more certain timing.
//
static inline int ICACHE_FLASH_ATTR read_bit(void)
{
	//IO_REG_TYPE mask=bitmask;
	//volatile IO_REG_TYPE *reg IO_REG_ASM = baseReg;
	int r;
  
	// noInterrupts();
	GPIO_OUTPUT_SET( DS18B20_PIN, 0 );
	// DIRECT_MODE_OUTPUT(reg, mask);
	// DIRECT_WRITE_LOW(reg, mask);
	os_delay_us(3);
	GPIO_DIS_OUTPUT( DS18B20_PIN );
	// DIRECT_MODE_INPUT(reg, mask);	// let pin float, pull up will raise
	os_delay_us(10);
	// r = DIRECT_READ(reg, mask);
	r = GPIO_INPUT_GET( DS18B20_PIN );
	// interrupts();
	os_delay_us(53);

	return r;
}

//
// Do a ROM select
//
static void ICACHE_FLASH_ATTR select(const uint8_t *rom)
{
	uint8_t i;
	write(0x55, 0);           // Choose ROM
	for (i = 0; i < 8; i++) write(rom[i], 0);
}

//
// Read a byte
//
static uint8_t ICACHE_FLASH_ATTR read() {
	uint8_t bitMask;
	uint8_t r = 0;
	for (bitMask = 0x01; bitMask; bitMask <<= 1) {
		if ( read_bit()) r |= bitMask;
	}
	return r;
}

//
// Compute a Dallas Semiconductor 8 bit CRC directly.
// this is much slower, but much smaller, than the lookup table.
//
static uint8_t ICACHE_FLASH_ATTR crc8(const uint8_t *addr, uint8_t len)
{
	uint8_t crc = 0;
	
	while (len--) {
		uint8_t inbyte = *addr++;
		for (uint8_t i = 8; i; i--) {
			uint8_t mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix) crc ^= 0x8C;
			inbyte >>= 1;
		}
	}
	return crc;
}


/*
 * Parameter: <>
 */
 
 struct sensor_reading* ICACHE_FLASH_ATTR read_ds18b20(void) { 
    return &dsreading[0];
}


int ICACHE_FLASH_ATTR readDS(uint8_t *dsaddr)
{
	int i;
	uint8_t data[12];
	
	// perform the conversion
	reset();
	select( dsaddr );

	write( 0x44, 1 ); // perform temperature conversion

	os_delay_us( 1000000 );

	dbg( "Scratchpad: " );
	reset();
	select( dsaddr );
	write( 0xbe, 0 ); // read scratchpad
	
	for( i = 0; i < 9; i++ )
	{
		data[i] = read();
		dbg( "%2x ", data[i] );
	}
	dbg( "\n" );

    dowcrc = 0;
    for(int i = 0; i < 8; i++){
        ow_crc(data[i]);
    }
    
    if(data[8] != dowcrc){
        os_printf("CRC check failed: %02X %02X", data[8], dowcrc);
		return -9999;
    } 

	int HighByte, LowByte, TReading;
	LowByte = data[0];
	HighByte = data[1];
	TReading = (HighByte << 8) + LowByte;
	
	return TReading;
}

static  void ICACHE_FLASH_ATTR pollDSCb(void * arg)
{
	if(currds>numds)
		currds=0;
	int TReading = readDS(addr[currds]);
	if(TReading!=-9999) {
		dsreading[currds].temperature=TReading;
		dsreading[currds].success = 1;
	} else {
		dsreading[currds].success = 0;	
	}
	currds++;	
}


int ICACHE_FLASH_ATTR ds_str(char *buff,int sensornum) {
	int Treading,Whole,Fract,SignBit;
	Treading = dsreading[sensornum].temperature;
	SignBit = Treading & 0x8000;  // test most sig bit
	if (SignBit) // negative
		Treading = (Treading ^ 0xffff) + 1; // 2's comp
	
	Whole = Treading >> 4;  // separate off the whole and fractional portions
	Fract = (Treading & 0xf) * 100 / 16;

	if (SignBit) // negative
		Whole*=-1;
		
	return os_sprintf( buff,"%d.%d", Whole, Fract < 10 ? 0 : Fract);
}

// global search state
static unsigned char ROM_NO[8];
static uint8_t LastDiscrepancy;
static uint8_t LastFamilyDiscrepancy;
static uint8_t LastDeviceFlag;

void ICACHE_FLASH_ATTR ds_init(uint32_t polltime)
{
	//set gpio2 as gpio pin
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
	  
	//disable pulldown
	PIN_PULLDWN_DIS(PERIPHS_IO_MUX_GPIO2_U);
	  
	//enable pull up R
	PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO2_U);  
	  
	// Configure the GPIO with internal pull-up
	// PIN_PULLUP_EN( gpio );

	GPIO_DIS_OUTPUT( DS18B20_PIN );

	reset_search();

	int r;
	os_memset(addr,sizeof(addr),0);
	
	numds=0;
	for(int i=0;i<4;i++) {
		r = ds_search( addr[i] );
		if(r==0) break;
		os_printf( "Found Device @ %02x %02x %02x %02x %02x %02x %02x %02x\n", 
				addr[i][0], addr[i][1], addr[i][2], addr[i][3], addr[i][4], addr[i][5], 
				addr[i][6], addr[i][7] );
		if( crc8( addr[i], 7 ) != addr[i][7] ) {
			os_printf( "CRC mismatch, crc=%xd, addr[7]=%xd\n",
					crc8( addr[i], 7 ), addr[i][7] );
			break;
		}
		switch( addr[i][0] )
		{
		case 0x10:
			dbg( "Device is DS18S20 family.\n" );
			numds++;

		case 0x28:
			dbg( "Device is DS18B20 family.\n" );
			numds++;

		default:
			dbg( "Device is unknown family.\n" );
			break;
		}
		
	}
	
	pollDSCb(NULL);
	
	os_printf("DS18B20 Setup; poll interval of %d sec; %d sensors found\n", (int)polltime/1000, numds);
  
	static ETSTimer dsTimer;
	os_timer_setfn(&dsTimer, pollDSCb, NULL);
	os_timer_arm(&dsTimer, polltime, 1);
  
}

static void ICACHE_FLASH_ATTR reset_search()
{
	// reset the search state
	LastDiscrepancy = 0;
	LastDeviceFlag = FALSE;
	LastFamilyDiscrepancy = 0;
	for(int i = 7; ; i--) {
		ROM_NO[i] = 0;
		if ( i == 0) break;
	}
}

// Perform the onewire reset function.  We will wait up to 250uS for
// the bus to come high, if it doesn't then it is broken or shorted
// and we return a 0;
//
// Returns 1 if a device asserted a presence pulse, 0 otherwise.
//
static uint8_t ICACHE_FLASH_ATTR reset(void)
{
	//	IO_REG_TYPE mask = bitmask;
	//	volatile IO_REG_TYPE *reg IO_REG_ASM = baseReg;
	int r;
	uint8_t retries = 125;

	// noInterrupts();
	// DIRECT_MODE_INPUT(reg, mask);
	GPIO_DIS_OUTPUT( DS18B20_PIN );
	
	// interrupts();
	// wait until the wire is high... just in case
	do {
		if (--retries == 0) return 0;
		os_delay_us(2);
	} while ( !GPIO_INPUT_GET( DS18B20_PIN ));

	// noInterrupts();
	GPIO_OUTPUT_SET( DS18B20_PIN, 0 );
	// DIRECT_WRITE_LOW(reg, mask);
	// DIRECT_MODE_OUTPUT(reg, mask);	// drive output low
	// interrupts();
	os_delay_us(480);
	// noInterrupts();
	GPIO_DIS_OUTPUT( DS18B20_PIN );
	// DIRECT_MODE_INPUT(reg, mask);	// allow it to float
	os_delay_us(70);
	// r = !DIRECT_READ(reg, mask);
	r = !GPIO_INPUT_GET( DS18B20_PIN );
	// interrupts();
	os_delay_us(410);

	return r;
}

/* pass array of 8 bytes in */
static int ICACHE_FLASH_ATTR ds_search( uint8_t *newAddr )
{
	uint8_t id_bit_number;
	uint8_t last_zero, rom_byte_number;
	uint8_t id_bit, cmp_id_bit;
	int search_result;

	unsigned char rom_byte_mask, search_direction;

	// initialize for search
	id_bit_number = 1;
	last_zero = 0;
	rom_byte_number = 0;
	rom_byte_mask = 1;
	search_result = 0;

	// if the last call was not the last one
	if (!LastDeviceFlag)
	{
		// 1-Wire reset
		if (!reset())
		{
			// reset the search
			LastDiscrepancy = 0;
			LastDeviceFlag = FALSE;
			LastFamilyDiscrepancy = 0;
			return FALSE;
		}

		// issue the search command
		write(0xF0, 0);

		// loop to do the search
		do
		{
			// read a bit and its complement
			id_bit = read_bit();
			cmp_id_bit = read_bit();
	 
			// check for no devices on 1-wire
			if ((id_bit == 1) && (cmp_id_bit == 1))
				break;
			else
			{
				// all devices coupled have 0 or 1
				if (id_bit != cmp_id_bit)
					search_direction = id_bit;  // bit write value for search
				else
				{
					// if this discrepancy if before the Last Discrepancy
					// on a previous next then pick the same as last time
					if (id_bit_number < LastDiscrepancy)
						search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
					else
						// if equal to last pick 1, if not then pick 0
						search_direction = (id_bit_number == LastDiscrepancy);

					// if 0 was picked then record its position in LastZero
					if (search_direction == 0)
					{
						last_zero = id_bit_number;

						// check for Last discrepancy in family
						if (last_zero < 9)
							LastFamilyDiscrepancy = last_zero;
					}
				}

				// set or clear the bit in the ROM byte rom_byte_number
				// with mask rom_byte_mask
				if (search_direction == 1)
					ROM_NO[rom_byte_number] |= rom_byte_mask;
				else
					ROM_NO[rom_byte_number] &= ~rom_byte_mask;

				// serial number search direction write bit
				write_bit(search_direction);

				// increment the byte counter id_bit_number
				// and shift the mask rom_byte_mask
				id_bit_number++;
				rom_byte_mask <<= 1;

				// if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
				if (rom_byte_mask == 0)
				{
					rom_byte_number++;
					rom_byte_mask = 1;
				}
			}
		}
		while(rom_byte_number < 8);  // loop until through all ROM bytes 0-7

		// if the search was successful then
		if (!(id_bit_number < 65))
		{
			// search successful so set LastDiscrepancy,LastDeviceFlag,search_result
			LastDiscrepancy = last_zero;

			// check for last device
			if (LastDiscrepancy == 0)
				LastDeviceFlag = TRUE;

			search_result = TRUE;
		}
	}

	// if no device found then reset counters so next 'search' will be like a first
	if (!search_result || !ROM_NO[0])
	{
		LastDiscrepancy = 0;
		LastDeviceFlag = FALSE;
		LastFamilyDiscrepancy = 0;
		search_result = FALSE;
	}
	for (int i = 0; i < 8; i++) newAddr[i] = ROM_NO[i];
	return search_result;
}

