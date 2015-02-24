# esp8266_pcd8544
PCD8544 LCD driver for esp8266 (Nokia 5110 &amp; 3110 display)

This is a direct port of code found at [arduino playground.](http://playground.arduino.cc/Code/PCD8544)

The interface requires 5 available GPIO outputs so an ESP-01 will not work. 

This is how the code is hooked up by default:

PCD8544| ESP8266
-------|------------------
RST Pin 1 | GPIO4
CE  Pin 2 | GPIO5
DC  Pin 3 | GPIO12
Din Pin 4 | GPIO13
Clk Pin 5 | GPIO14

Some ESP-12 have GPIO4 & GPIO5 reversed.
All of the pins are configurable, you just set the pins you want to use in the setting struct.

I don't know if it is required but i put 1KΩ resistors on each GPIO pin, and it does not seem to cause any problems. 

Take a look at [esp_mqtt_lcd](https://github.com/eadf/esp_mqtt_lcd) to see an example on how this project can be used as a library module (git subtree) in your own project.

###Required:

esp_iot_sdk_v0.9.4_14_12_19 ( v0.9.5 breaks everything ) 

Actually, i have not tested this with v0.9.5. I tested a clean sdk 0.9.5 install with one of the basic examples (could have been blinky). It compiled and uploaded fine but the esp had a infinite crash loop with some message about "MEM CHK FAIL" on the console. So i threw the whole sdk out. I will try to upgrade the sdk again once [mqtt](https://github.com/tuanpmt/esp_mqtt) upgrades to 0.9.5+.
