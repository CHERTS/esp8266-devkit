struct sensor_reading* read_ds18b20(void);
int ds_str(char *buff,int sensornum);
void ds_init(uint32_t polltime);
int readDS(uint8_t *dsaddr);
struct sensor_reading dsreading [4];

extern uint8_t addr[4][8];
extern uint8_t numds;