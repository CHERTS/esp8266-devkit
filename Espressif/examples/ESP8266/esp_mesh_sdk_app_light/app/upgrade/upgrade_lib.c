#include "ets_sys.h"
#include "spi_flash.h"

//#include "net80211/ieee80211_var.h"
//#include "lwip/mem.h"
#include "mem.h"

#include "upgrade.h"

#include "user_interface.h"

#define ESP_DBG os_printf

struct upgrade_param {
    uint32 fw_bin_addr;

    uint16 fw_bin_sec;
    uint16 fw_bin_sec_num;

    uint16 fw_bin_sec_earse;

    uint8 extra;

    uint8 save[4];

    uint8 *buffer;
};

LOCAL struct upgrade_param *upgrade;

extern SpiFlashChip *flashchip;

/******************************************************************************
 * FunctionName : system_upgrade_internal
 * Description  : a
 * Parameters   :
 * Returns      :
*******************************************************************************/
LOCAL bool ICACHE_FLASH_ATTR
system_upgrade_internal(struct upgrade_param *upgrade, uint8 *data, uint16 len)
{
    bool ret = false;
    if(data == NULL || len == 0)
    {
    	return true;
    }
    upgrade->buffer = (uint8 *)os_zalloc(len + upgrade->extra);
    os_delay_us(1000);
    os_memcpy(upgrade->buffer, upgrade->save, upgrade->extra);
    os_memcpy(upgrade->buffer + upgrade->extra, data, len);

    len += upgrade->extra;
    upgrade->extra = len & 0x03;
    len -= upgrade->extra;

    os_memcpy(upgrade->save, upgrade->buffer + len, upgrade->extra);

    do {
        if (upgrade->fw_bin_addr + len >= (upgrade->fw_bin_sec + upgrade->fw_bin_sec_num) * SPI_FLASH_SEC_SIZE) {
            break;
        }

        if (len > SPI_FLASH_SEC_SIZE) {

        } else {
//			os_printf("%x %x\n",upgrade->fw_bin_sec_earse,upgrade->fw_bin_addr);
            /* earse sector, just earse when first enter this zone */
            if (upgrade->fw_bin_sec_earse != (upgrade->fw_bin_addr + len) >> 12) {
                upgrade->fw_bin_sec_earse = (upgrade->fw_bin_addr + len) >> 12;
//                spi_flash_erase_sector(upgrade->fw_bin_sec_earse);
//				os_printf("%x\n",upgrade->fw_bin_sec_earse);
            }
        }
		os_printf("-------------------\r\n");
		os_printf("flash write: 0x%08x ; len: %d \r\n",upgrade->fw_bin_addr,len);
		#if 0
		os_printf("data: \r\n");
		int i;
		for(i=0;i<len;i++) os_printf("%02x ",*((uint8*)((upgrade->buffer)+i)));
		
		os_printf("-------------------\r\n");
		#endif
        if (spi_flash_write(upgrade->fw_bin_addr, (uint32 *)upgrade->buffer, len) != SPI_FLASH_RESULT_OK) {
            break;
        }

        ret = true;
        upgrade->fw_bin_addr += len;
    } while (0);

    os_free(upgrade->buffer);
    upgrade->buffer = NULL;
    return ret;
}

/******************************************************************************
 * FunctionName : system_upgrade
 * Description  : a
 * Parameters   :
 * Returns      :
*******************************************************************************/
bool ICACHE_FLASH_ATTR
system_upgrade(uint8 *data, uint16 len)
{
    bool ret;

    ret = system_upgrade_internal(upgrade, data, len);

    return ret;
}

/******************************************************************************
 * FunctionName : system_upgrade_init
 * Description  : a
 * Parameters   :
 * Returns      :
*******************************************************************************/
void ICACHE_FLASH_ATTR
system_upgrade_init(void)
{
    uint32 user_bin1_start, user_bin2_start;
    uint8 spi_size_map = system_get_flash_size_map();
	ESP_DBG("system_get_flash_size_map: %d \r\n",system_get_flash_size_map());

    if (upgrade == NULL) {
        upgrade = (struct upgrade_param *)os_zalloc(sizeof(struct upgrade_param));
    }

    system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
	ESP_DBG("-------------------------\r\n");
	ESP_DBG("FLG SET IDLE: %d \r\n",system_upgrade_flag_check());

    user_bin1_start = 1;

    if (spi_size_map == FLASH_SIZE_8M_MAP_512_512 ||
    		spi_size_map == FLASH_SIZE_16M_MAP_512_512 ||
    		spi_size_map == FLASH_SIZE_32M_MAP_512_512) {
		user_bin2_start = 129;	// 512/4 + 1
		upgrade->fw_bin_sec_num = 123;	// 512/4 - 1 - 4
	} else if (spi_size_map == FLASH_SIZE_16M_MAP_1024_1024 ||
			spi_size_map == FLASH_SIZE_32M_MAP_1024_1024) {
		user_bin2_start = 257;	// 1024/4 + 1
		upgrade->fw_bin_sec_num = 251;	// 1024/4 - 1 - 4
	} else {
		user_bin2_start = 65;	// 256/4 + 1
		upgrade->fw_bin_sec_num = 59;	// 256/4 - 1 - 4
	}

    upgrade->fw_bin_sec = (system_upgrade_userbin_check() == USER_BIN1) ? user_bin2_start : user_bin1_start;
    ESP_DBG("system_upgrade_userbin_check: %d \r\n",system_upgrade_userbin_check());
	ESP_DBG("upgrade->fw_bin_sec_num: %d \r\n",upgrade->fw_bin_sec_num);
	ESP_DBG("-------------------------\r\n");
	
    upgrade->fw_bin_addr = upgrade->fw_bin_sec * SPI_FLASH_SEC_SIZE;
}

/******************************************************************************
 * FunctionName : system_upgrade_deinit
 * Description  : a
 * Parameters   :
 * Returns      :
*******************************************************************************/
void ICACHE_FLASH_ATTR
system_upgrade_deinit(void)
{
	if (upgrade != NULL) {
		os_free(upgrade);
		upgrade = NULL;
	}else {
		return;
	}
}
