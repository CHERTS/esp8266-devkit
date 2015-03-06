Example of reading data from the sensor DHT22
---------------------------------------------

Repository: https://github.com/CHERTS/esp8266-dht11_22

The example uses:
1. Driver GPIO16
2. Driver DHT11_22

For a single device, connect as follows DHT22 to ESP-01:
DHT22 1 (Vcc) to Vcc (3.3 Volts) ESP-01
DHT22 2 (DATA_OUT) to GPIO2 ESP-01
DHT22 3 (NC)
DHT22 4 (GND) to GND ESP-01
Between the terminal Vcc and DATA_OUT need to connect a pullup resistor of 5 Kohms.

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
