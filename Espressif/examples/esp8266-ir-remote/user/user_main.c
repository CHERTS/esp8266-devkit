#include "esp_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ir_remote.h"

void ICACHE_FLASH_ATTR send_code_task(void *pvParameters)
{
	while (1)
	{
		ir_remote_send_nec(0x5EA1F807, 32); // power on/off code for Yamaha RX-700
		vTaskDelay(2000 / portTICK_RATE_MS);
	}
}

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR user_init(void)
{
	// IR led is connected to GPIO2 and pulled-up with 2k2 resistor.
	ir_remote_init(2, true);

    xTaskCreate(send_code_task, "send_code_task", 256, NULL, 2, NULL);
}

