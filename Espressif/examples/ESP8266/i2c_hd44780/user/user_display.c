#include "user_display.h"

volatile PageData _PageData[DISPLAY_PAGE_MAX];

volatile uint8 display_page = 1;
volatile uint8 startup_done = 0;
static volatile os_timer_t refresh_timer;


void ICACHE_FLASH_ATTR 
display_draw_page(uint8 page)
{
    page--;
    LCD_print(_PageData[page].line1);
    LCD_setCursor(0,1);
    LCD_print(_PageData[page].line2);
    LCD_setCursor(0,2);
    LCD_print(_PageData[page].line3);
    LCD_setCursor(0,3);
    LCD_print(_PageData[page].line4);
}
void ICACHE_FLASH_ATTR 
display_redraw(void)
{
    char page_buffer[20];

    //Clear the lcd
    LCD_clear();

    //Draw the page from the buffer
    display_draw_page(display_page);

    //Draw page counter 
    LCD_setCursor(17,3);
    os_sprintf(page_buffer, "%i/%i",display_page,DISPLAY_PAGE_MAX);
    LCD_print(page_buffer);
}

void ICACHE_FLASH_ATTR
display_refresh(uint8 start)
{
    //Refresh the display
    os_timer_disarm(&refresh_timer);
    os_timer_setfn(&refresh_timer, (os_timer_func_t *)display_refresh, 0);
    os_timer_arm(&refresh_timer, 5000, 0);
    if (start == 1) {
        display_redraw();
        return;
    }
    display_next_page();
}

void ICACHE_FLASH_ATTR 
display_next_page(void)
{
    display_page++;
    if (display_page > DISPLAY_PAGE_MAX) display_page = 1;
    display_redraw();

    //Reset refresh timer
    os_timer_disarm(&refresh_timer);
    os_timer_setfn(&refresh_timer, (os_timer_func_t *)display_refresh, 0);
    os_timer_arm(&refresh_timer, 5000, 0);
}

void ICACHE_FLASH_ATTR 
display_prev_page(void)
{
    display_page--;
    if (display_page == 0) display_page = DISPLAY_PAGE_MAX;
    display_redraw();

    //Reset refresh timer
    os_timer_disarm(&refresh_timer);
    os_timer_setfn(&refresh_timer, (os_timer_func_t *)display_refresh, 0);
    os_timer_arm(&refresh_timer, 5000, 0);
}

void ICACHE_FLASH_ATTR
display_data(uint8 page, uint8 line, char data[20])
{
    page--;
    switch (line)
    {
        case 1:
            os_memcpy(&_PageData[page].line1, data, 20);
        break; 
        case 2:
            os_memcpy(&_PageData[page].line2, data, 20);
        break; 
        case 3:
            os_memcpy(&_PageData[page].line3, data, 20);
        break; 
        case 4:
            os_memcpy(&_PageData[page].line4, data, 20);
        break;
    }
    if (startup_done == 0 && line == 4) {
        LCD_noBlink();
        display_redraw();
        os_timer_disarm(&refresh_timer);
        os_timer_setfn(&refresh_timer, (os_timer_func_t *)display_refresh, 0);
        os_timer_arm(&refresh_timer, 5000, 1);
        startup_done = 1;
    }
}

void ICACHE_FLASH_ATTR
display_init(void)
{
    i2c_init();
    while (!LCD_init())
    {
        os_printf("LCD not found\n\r");
    }
    LCD_setCursor(0,2);
    LCD_print("Home status Display");

    LCD_setCursor(0,3);
    LCD_print("Connecting ");
    LCD_blink();
}

