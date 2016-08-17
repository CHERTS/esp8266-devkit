/*
	The hello world c++ demo
*/

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <gpio.h>

#define DELAY 1000 /* milliseconds */

// =============================================================================================
// C includes and declarations
// =============================================================================================
extern "C"
{
#include "driver/uart.h"
#include <user_interface.h>

void *pvPortMalloc( size_t xWantedSize );
void vPortFree( void *pv );
void *pvPortZalloc(size_t size);

// declare lib methods
extern int ets_uart_printf(const char *fmt, ...);
void ets_timer_disarm(ETSTimer *ptimer);
void ets_timer_setfn(ETSTimer *ptimer, ETSTimerFunc *pfunction, void *parg);
void ets_timer_arm_new(ETSTimer *ptimer,uint32_t milliseconds, bool repeat_flag, bool);

#define os_malloc   pvPortMalloc
#define os_free     vPortFree
#define os_zalloc   pvPortZalloc

}//extern "C"

// =============================================================================================
// These methods shall be defined anywhere.
// They are required for C++ compiler
// =============================================================================================
void *operator new(size_t size)
{
   return os_malloc(size);
}

void *operator new[](size_t size)
{
   return os_malloc(size);
}

void operator delete(void * ptr)
{
   os_free(ptr);
}

void operator delete[](void * ptr)
{
   os_free(ptr);
}

extern "C" void __cxa_pure_virtual(void) __attribute__ ((__noreturn__));
extern "C" void __cxa_deleted_virtual(void) __attribute__ ((__noreturn__));
extern "C" void abort()
{
   while(true); // enter an infinite loop and get reset by the WDT
}

void __cxa_pure_virtual(void) {
  abort();
}

void __cxa_deleted_virtual(void) {
  abort();
}

// =============================================================================================
// Example class. It show that the global objects shall be initialyzed by user
// (see user_init method)
// =============================================================================================

class A
{
	int a;
	int b;

public:
	A()
	: a(5)
	, b(6)
	{
	}

	A( int k, int m )
	: a(k)
	, b(m)
	{
	}

	void print()
	{
		ets_uart_printf( "a = %d, b = %d\n", a, b );
	}
};

// =============================================================================================
// global objects
// =============================================================================================

LOCAL os_timer_t hello_timer;
A a;


// =============================================================================================
// Pointers to the constructors of the global objects
// (defined in the linker script eagle.app.v6.ld)
// =============================================================================================

extern void (*__init_array_start)(void);
extern void (*__init_array_end)(void);

// Initialyzer of the global objects
static void do_global_ctors(void)
{
    void (**p)(void);
    for (p = &__init_array_start; p != &__init_array_end; ++p)
            (*p)();
}

// =============================================================================================
// User code
// =============================================================================================

LOCAL void ICACHE_FLASH_ATTR hello_cb(void *arg)
{
	static int counter = 0;
	ets_uart_printf("Hello World #%d!\r\n", counter++);
	a.print();
}


/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABBBCDDD
 *                A : rf cal
 *                B : at parameters
 *                C : rf init data
 *                D : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
extern "C" uint32 ICACHE_FLASH_ATTR user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 8;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}

extern "C" void ICACHE_FLASH_ATTR user_rf_pre_init(void)
{
}

extern "C" void ICACHE_FLASH_ATTR user_init(void)
{
	do_global_ctors();
	// Configure the UART
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	ets_uart_printf("System init...\r\n");

	// Set up a timer to send the message
	// os_timer_disarm(ETSTimer *ptimer)
	os_timer_disarm(&hello_timer);
	// os_timer_setfn(ETSTimer *ptimer, ETSTimerFunc *pfunction, void *parg)
	os_timer_setfn(&hello_timer, (os_timer_func_t *)hello_cb, (void *)0);
	// void os_timer_arm(ETSTimer *ptimer,uint32_t milliseconds, bool repeat_flag)
	os_timer_arm(&hello_timer, DELAY, 1);

	ets_uart_printf("System init done.\r\n");
}
