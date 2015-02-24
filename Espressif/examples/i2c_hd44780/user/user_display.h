#ifndef __USER_DISPLAY_H__
#define __USER_DISPLAY_H__

#include "ets_sys.h"
#include "driver/i2c.h"
#include "driver/i2c_hd44780.h"
#include "osapi.h"
#include "os_type.h"
#include "user_interface.h"
#include "user_config.h"
#include "user_display.h"

#define DISPLAY_PAGE_MAX 5

typedef struct {
      char line1[20];
      char line2[20];
      char line3[20];
      char line4[20];
} PageData;

void display_redraw(void);
void display_next_page(void);
void display_prev_page(void);
void display_data(uint8 page, uint8 line, char data[20]);
void display_init(void);

#endif
