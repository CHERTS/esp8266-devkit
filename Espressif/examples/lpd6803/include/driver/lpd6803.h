#ifndef LPD6803_H
#define LPD6803_H

#define numLEDs 7
#define LPD6803_START 0
#define LPD6803_HEADER 1
#define LPD6803_DATA 2
#define LPD6803_DONE 3

#define LPD6803_MODE_NONE 0
#define LPD6803_MODE_RUNNING_PIXEL 1
#define LPD6803_MODE_RUNNING_LINE 2

#define sleepms(x) os_delay_us(x*1000);

void ICACHE_FLASH_ATTR lpd6803_LedOut();
void ICACHE_FLASH_ATTR lpd6803_setPixelColor(uint16_t n, uint8_t b, uint8_t r, uint8_t g);
void ICACHE_FLASH_ATTR lpd6803_setPixelColorByColor(uint16_t n, uint16_t c);
void ICACHE_FLASH_ATTR lpd6803_init();
unsigned int ICACHE_FLASH_ATTR lpd6803_Color(uint8_t b, uint8_t r, uint8_t g);
unsigned int ICACHE_FLASH_ATTR lpd6803_Wheel(uint8_t WheelPos);
void ICACHE_FLASH_ATTR lpd6803_loop();

//Running Pixel mode functions
void ICACHE_FLASH_ATTR lpd6803_startRunningPixel(uint16_t color);
void ICACHE_FLASH_ATTR lpd6803_RunningPixel_loop();

//Running Line mode functions
void ICACHE_FLASH_ATTR lpd6803_startRunningLine(uint16_t color);
void ICACHE_FLASH_ATTR lpd6803_RunningLine_loop();

#endif
