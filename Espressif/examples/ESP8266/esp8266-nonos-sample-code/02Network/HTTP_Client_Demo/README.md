##SDK Version : esp8266_nonos_sdk_v2.0.0_16_07_19
##Platform : ESP-LAUNCHER BAOARD

##Operation Steps:

1. Enter path:/home/esp8266/Share, clone ESP8266 NONOS SDK to lubuntu environment by command: 
       
	git clone https://github.com/espressif/esp8266-nonos-sample-code.git
	   
2. Change the ssid and the password of the AP you want to connect to in user_main.c:
		
	os_sprintf(stationConf.ssid, "TP-LINK-FD");
	os_sprintf(stationConf.password, "aaaaaaaa");

3. Copy the example next to bin/ folder in the SDK folder. THe SDK folder should have folders inside it like : bin, examples, third party...
	   
4. Enter example folder,select the example you want to , run ./gen_misc.sh, and follow below steps to fininsh the sample code compile:
	
		Option 1 will be automatically selected, 
		Option 2 > Enter 1. 
		Option 3 > Enter Default(Just Press enter)
		Option 4 > Enter Default(Just Press enter)
		Option 5 > Enter 5.
		The bin files will generate in BIN_PATH folder which is bin/upgrade. 
	   
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

	mode : sta(18:fe:34:ed:86:7a)
	add if0
	scandone
	state: 0 -> 2 (b0)
	state: 2 -> 3 (0)
	state: 3 -> 5 (10)
	add 0
	aid 2
	cnt 

	connected with TP-LINK-FD, channel 1
	dhcp client start...
	ip:192.168.0.105,mask:255.255.255.0,gw:192.168.0.1
	http://api.openweathermap.org/data/2.5/weather?q=beijing&APPID=fd470504ddf42c64c35a4fa30c2c4120
	http_status=200
	strlen(full_response)=798
	response=HTTP/1.1 200 OK
	Server: openresty
	Date: Thu, 25 Aug 2016 08:37:51 GMT
	Content-Type: application/json; charset=utf-8
	Content-Length: 442
	Connection: close
	X-Cache-Key: /data/2.5/weather?APPID=fd470504ddf42c64c35a4fa30c2c4120&q=beijing
	Access-Control-Allow-Origin: *
	Access-Control-Allow-Credentials: true
	Access-Control-Allow-Methods: GET, POST

	{"coord":{"lon":116.4,"lat":39.91},"weather":[{"id":800,"main":"Clear","description":"clear sky","icon":"01d"}],"base":"stations","main":{"temp":304.93,"pressure":1012,"humidity":28,"temp_min":303.15,"temp_max":307.04},"visibility":10000,"wind":{"speed":4,"deg":360},"clouds":{"all":0},"dt":1472114071,"sys":{"type":1,"id":7405,"message":0.0175,"country":"CN","sunrise":1472074558,"sunset":1472122563},"id":1816670,"name":"Beijing","cod":200}<EOF>
	