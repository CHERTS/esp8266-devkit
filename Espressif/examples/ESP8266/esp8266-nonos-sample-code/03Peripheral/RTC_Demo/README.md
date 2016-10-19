##SDK Version : esp8266_nonos_sdk_v2.0.0_16_07_19
##Platform : ESP-LAUNCHER BAOARD

##Operation Steps:

1. Enter path:/home/esp8266/Share, clone ESP8266 NONOS SDK to lubuntu environment by command: 
       
	git clone https://github.com/espressif/esp8266-nonos-sample-code.git
	   
2. change the ssid and the password of the AP you want to connect to in user_main.c:
		
	os_sprintf(stationConf.ssid, "TP-LINK-FD");
	os_sprintf(stationConf.password, "aaaaaaaa");

3. copy the example next to bin/ folder in the SDK folder. THe SDK folder should have folders inside it like : bin, examples, third party...
	   
4. Enter example folder,select the example you want to , run ./gen_misc.sh, and follow below steps to fininsh the sample code compile:
	
		Option 1 will be automatically selected, 
		Option 2 > Enter 1. 
		Option 3 > Enter Default(Just Press enter)
		Option 4 > Enter Default(Just Press enter)
		Option 5 > Enter 5.
		Then bin files will generate in BIN_PATH folder which is bin/upgrade.
	   
5. Download bin files to ESP-LAUNCHER as below sittings.
		
		Download address of each bin files:
		blank.bin				           		  0x1FE000
		esp_init_data_default.bin			  	  0x1FC000
		boot_v1.5.bin					   		  0x00000
		user1.2048.new.5.bin			          0x01000
		
		Flash download tool settings:
		CrystalFreq: 26M
		SPI SPEED: 40MHz
		SPID MODE: QIO
		FLASH SIZE: 16Mbit-C11
			
##FOR VERIFY: 
Start the board and uart0 will print the information as follow:

	2nd boot version : 1.6
  	SPI Speed      : 40MHz
 	 SPI Mode       : QIO
	  SPI Flash Size & Map: 8Mbit(512KB+512KB)
	jump to run user1 @ 1000

	rf cal sector: 251
	rf[112] : 00
	rf[113] : 00
	rf[114] : 01

	SDK ver: 2.0.0(656edbf) compiled @ Jul 19 2016 17:58:40
	phy ver: 1055, pp ver: 10.2

	RTC TIMER: 22268 
	SYS TIMER: 176377 
	time acc: 1073645316 
	time base: 477763670 
	magic : 0x27919d42
	rtc time init...
	time base : 25046 
	==================
	RTC time test : 
	 rtc_t2-t1 : 309 
	 st2-t2 :  1823  
	cal 1  : 5.895  
	cal 2  : 5.895 
	==================

	rtc time acc  : 9567585 
	power on time :  9567  us
	power on time :  0.00  S
	------------------------
	continue ...
	count: 1
	SDK version:2.0.0(656edbf)
	mode : softAP(1a:fe:34:ed:86:7a)
	add if1
	dhcp server start:(ip:192.168.4.1,mask:255.255.255.0,gw:192.168.4.1)
	bcn 100
	RTC TIMER: 871497 
	SYS TIMER: 5231767 
	magic correct
	time acc: 1073645316 
	time base: 26669 
	magic : 0x55aaaa55
	==================
	RTC time test : 
	 rtc_t2-t1 : 309 
	 st2-t2 :  1850  
	cal 1  : 5.983  
	cal 2  : 5.983 
	==================

	rtc time acc  : 5069917189 
	power on time :  5069917  us
	power on time :  5.06  S
	------------------------
	continue ...
	count: 2
