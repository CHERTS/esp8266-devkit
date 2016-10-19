SDK Version : esp8266_nonos_sdk_v2.0.0_16_07_19
Platform : ESP-LAUNCHER BOARD

1.change the ssid and the password of the AP you want to connect to in user_main.c .
	os_sprintf(stationConf.ssid, "TP-LINK-FD");
	os_sprintf(stationConf.password, "aaaaaaaa");
	
2. Copy the examples next to bin/ folder in the SDK folder. THe SDK folder should have folders inside it like : bin, examples, third party...

3. Enter example folder, select the example you want to , run ./gen_misc.sh, and follow the tips and steps.
 
4.After executing:
	Option 1 > Enter 1, 
	Option 2 > Enter 1. 
	Option 3 > Enter Default(Just Press enter)
	Option 4 > Enter Default(Just Press enter)
	Option 5 > Enter 5.
	Then bin files will generate in BIN_PATH folder which is bin/upgrade.


5.Download bin files to ESP-LAUNCHER as below sittings.

	Download address of each bin files
	blank.bin						0x1FE000
	esp_init_data_default.bin				0x1FC000
	boot_v1.6.bin						0x00000
	user1.2048.new.5.bin					0x01000
	
	Flash download tool settings.
	CrystalFreq: 26M
	SPI SPEED: 40MHz
	SPID MODE: QIO
	FLASH SIZE: 16Mbit-C11


6.If the parameter of the function "wifi_fpm_do_sleep" is 0xFFFFFFF, the board would be sleeping until the the gpio voltage is right, else the board would wakeup automatically when the sleeping time(parameter of wifi_fpm_do_sleep) is costed.In both cases, the callback function will be called automatically.In this example, we create a tcp server after the board wakeup and you can check the received infomation by serial.When the board is sleeping in forced light-mode,the current would be about 8 mA.
  
 
