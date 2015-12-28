PCD8544 LCD driver example for esp8266 (Nokia 5110 & 3110 display)

https://github.com/eadf/esp8266_pcd8544_example

The driver is a direct port of code found at arduino playground.

Good news, the interface no longer requires 5 available GPIO outputs so an ESP-01 will indeed work. (But only if the RX pin of the esp is used.)

This is how the code is hooked up by default:
-----------------------
| PCD8544   | ESP8266 |
|-------------------- |
| RST Pin 1 | GPIO4   |
| CE Pin 2  | GPIO5   |
| DC Pin 3  | GPIO12  |
| Din Pin 4 | GPIO13  |
| Clk Pin 5 | GPIO14  |
-----------------------

Some ESP-12 have GPIO4 & GPIO5 reversed.

The RST pin is optional, set it to a negative value and tie PCD8544 reset to ESP reset via a resistor.

The CE pin is optional, set it to a negative value and tie PCD8544 CE pin to GND via a resistor.

All of the pins are configurable, you just set the pins you want to use in the setting struct.

I don't know if it is required but i put 1K? resistors on each GPIO pin, and it does not seem to cause any problems.

Take a look at esp_mqtt_lcd to see another example on how the pcd8544 driver can be used as a library module (git subtree) in your own project.

The makefile is copied from esp_mqtt.
