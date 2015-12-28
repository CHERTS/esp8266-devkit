enum sensor_type{
	SENSOR_DHT11,SENSOR_DHT22
};

struct sensor_reading{
	float temperature;
	float humidity;
	const char* source;
	uint8_t sensor_id[16];
	BOOL success;
};


struct sensor_reading * readDHT(void);
void DHTInit(enum sensor_type, uint32_t polltime);

int dht_temp_str(char *buff);
int dht_humi_str(char *buff);
