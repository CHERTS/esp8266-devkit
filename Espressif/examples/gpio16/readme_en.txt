Driver for GPIO
---------------

Official repo: https://github.com/CHERTS/esp8266-gpio16

Wiring 2 buttons to the ESP-01:
Button 1 is connected to GPIO0 (Pin 3) ESP-01
Button 2 is connected to GPIO2 (Pin 4) ESP-01

GPIO table
----------

Pin 0 = GPIO16
Pin 1 = GPIO5
Pin 2 = GPIO4
Pin 3 = GPIO0
Pin 4 = GPIO2
Pin 5 = GPIO14
Pin 6 = GPIO12
Pin 7 = GPIO13
Pin 8 = GPIO15
Pin 9 = GPIO3
Pin 10 = GPIO1
Pin 11 = GPIO9
Pin 12 = GPIO10

[*] D0(GPIO16) can only be used as gpio read/write. no interrupt supported. no pwm/i2c/ow supported.

(c) 2014-2015 Mikhail Grigorev <sleuthhound@gmail.com>
