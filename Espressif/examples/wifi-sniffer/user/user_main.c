#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"
#include "driver/uart.h"

os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static volatile os_timer_t channelHop_timer;

static void loop(os_event_t *events);
static void promisc_cb(uint8 *buf, uint16 len);

void channelHop(void *arg)
{
    // 1 - 13 channel hopping
    uint8 new_channel = wifi_get_channel() % 12 + 1;
    os_printf("** hop to %d **\n", new_channel);
    wifi_set_channel(new_channel);
}

static void ICACHE_FLASH_ATTR
promisc_cb(uint8 *buf, uint16 len)
{
    os_printf("-> %3d: %d", wifi_get_channel(), len);
    printmac(buf,  4);
    printmac(buf, 10);
    printmac(buf, 16);
    os_printf("\n");
}

//Main code function
static void ICACHE_FLASH_ATTR
loop(os_event_t *events)
{
    os_delay_us(10);
}

void user_rf_pre_init(void)
{
}

//Init function 
void ICACHE_FLASH_ATTR user_init()
{
    uart_init(115200, 115200);
    os_delay_us(100);

    uart0_sendStr("*** Monitor mode test ***\r\n");
    
    os_printf(" -> Promisc mode setup ... ");
    wifi_set_promiscuous_rx_cb(promisc_cb);
    wifi_promiscuous_enable(1);
    os_printf("done.\n");

    os_printf(" -> Timer setup ... ");
    os_timer_disarm(&channelHop_timer);
    os_timer_setfn(&channelHop_timer, (os_timer_func_t *) channelHop, NULL);
    os_timer_arm(&channelHop_timer, CHANNEL_HOP_INTERVAL, 1);
    os_printf("done.\n");
    
    os_printf(" -> Set opmode ... ");
    wifi_set_opmode( 0x1 );
    os_printf("done.\n");

    //Start os task
    system_os_task(loop, user_procTaskPrio,user_procTaskQueue, user_procTaskQueueLen);

    os_printf(" -> Init finished!\n\n");
}
