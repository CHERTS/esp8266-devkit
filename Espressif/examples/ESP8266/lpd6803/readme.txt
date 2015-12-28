# esp8266-lpd6803

This is a port of Adafruit's LPD6803 Library on ESP8266 platform.
Original library for Arduino can be found here:
https://github.com/adafruit/LPD6803-RGB-Pixels

# How to connect hardware

Connect GPIO_0 of ESP8266 to DATA and GPIO_2 to CLCK.

# How to work with library

First of all, you must define number of LEDs on your strip.
It can be done in file lpd6803.h, definition name: numLEDs.
This step is very important to have a correct LEDs addressing.

You must call lpd6803_init() function, to initialize GPIO outputs and variables from user_init() function.
It is highly reccomended to call lpd6803_show() after lpd6803_init(). This will make all LEDs inactive (black).

LPD6803 library on ESP8266 is using two timers.
Main timer is for data transmission, another timer is to run predefined modes (optional).
Main timer must be initialized in a such way:

    os_timer_disarm(&lpd6803_timer);
    os_timer_setfn(&lpd6803_timer, (os_timer_func_t *) lpd6803_LedOut, NULL);
    os_timer_arm_us(&lpd6803_timer, 40, 1);
    
Optional timer for predefined modes can be initialized like this:

    os_timer_disarm(&some_timer);
    os_timer_setfn(&some_timer, (os_timer_func_t *) lpd6803_loop, NULL);
    os_timer_arm(&some_timer, 200, 1);
	
By changing 200ms to another value, you can increase or decrease speed of predefined mode.

# Predefined modes

* Running pixel, can be started by
lpd6803_startRunningPixel(lpd6803_Color(255, 0, 0))
* Running line, can be started by lpd6803_startRunningLine(lpd6803_Color(255, 0, 0))

# Documentation
http://www.adafruit.com/datasheets/LPD6803.pdf
