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
	rf[112 default configuration
	MQTT_InitConnection
	MQTT_InitClient
	WIFI_INIT

	System started ...
	mode : sta(18:fe:34:ed:86:7a)
	add if0
	STATION_IDLE
	STATION_IDLE
	STATION_IDLE
	scandone
	state: 0 -> 2 (b0)
	STATION_IDLE
	state: 2 -> 3 (0)
	state: 3 -> 5 (10)
	add 0
	aid 2
	cnt 

	connected with TP-LINK-FD, channel 1
	dhcp client start...
	STATION_IDLE
	STATION_IDLE
	ip:192.168.0.105,mask:255.255.255.0,gw:192.168.0.1
	TCP: Connect to ip  192.168.0.103:1883
	MQTT: Connected to broker 192.168.0.103:1883
	MQTT: Sending, type: 1, id: 0000
	TCP: Sent
	TCP: data received 4 bytes
	MQTT: Connected to 192.168.0.103:1883
	MQTT: Connected
	MQTT: queue subscribe, topic"topic_tech", id: 1
	MQTT: queue subscribe, topic"topic_football", id: 2
	MQTT: queuing publish, length: 54, queue size(42/2048)
	MQTT: queuing publish, length: 52, queue size(98/2048)
	MQTT: Sending, type: 8, id: 0001
	TCP: Sent
	TCP: data received 5 bytes
	MQTT: Subscribe successful
	MQTT: Sending, type: 8, id: 0002
	TCP: Sent
	TCP: data received 5 bytes
	MQTT: Subscribe successful
	MQTT: Sending, type: 3, id: 0000
	TCP: Sent
	MQTT: Published
	TCP: data received 54 bytes
	Receive topic: topic_tech, data: espressif create the esp8266 wifi chip
	 
	MQTT: queuing publish, length: 74, queue size(54/2048)
	MQTT: Sending, type: 3, id: 0003
	TCP: Sent
	MQTT: Published
	TCP: data received 4 bytes
	MQTT: received MQTT_MSG_TYPE_PUBACK, finish QoS1 publish
	MQTT: Sending, type: 3, id: 0000
	TCP: Sent
	MQTT: Published
	TCP: data received 52 bytes
	MQTT: Queue response QoS: 1
	Receive topic: topic_football, data: Arsenal is a top football club
	 
	MQTT: queuing publish, length: 74, queue size(6/2048)
	MQTT: Sending, type: 4, id: 0001
	TCP: Sent
	MQTT: Sending, type: 3, id: 0000
	TCP: Sent
	MQTT: Published
	pm open,type:2 0

