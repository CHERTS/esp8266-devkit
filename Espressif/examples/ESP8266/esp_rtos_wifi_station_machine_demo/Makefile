# Main settings includes
include	../settings.mk

# Individual project settings (Optional)
SDK_BASE	= c:/Espressif/ESP8266_RTOS_SDK
#BOOT		= new
#APP		= 1
#SPI_SPEED	= 40
#SPI_MODE	= QIO
#SPI_SIZE_MAP	= 2
#ESPPORT		= COM2
#ESPBAUD		= 256000

# Basic project settings
CFLAGS = -DICACHE_FLASH -D__STDC_NO_ATOMICS__=1 -DESP8266_RTOS -D__STDC_VERSION__=201112L -DFREERTOS_ARCH_ESP8266
MODULES	= user
LIBS	= cirom crypto espconn espnow freertos gcc hal lwip main mesh mirom net80211 nopoll phy pp mbedtls openssl smartconfig wpa wps

# Root includes
include	../common_rtos.mk
