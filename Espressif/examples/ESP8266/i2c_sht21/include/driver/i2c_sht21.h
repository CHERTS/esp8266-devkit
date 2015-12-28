#ifndef __I2C_SHT21_H
#define	__I2C_SHT21_H

#include "c_types.h"
#include "ets_sys.h"
#include "osapi.h"

#define SHT21_ADDRESS 0x80

#define TRIGGER_TEMP_MEASURE_HOLD  0xE3
#define TRIGGER_HUMD_MEASURE_HOLD  0xE5
#define TRIGGER_TEMP_MEASURE_NOHOLD  0xF3
#define TRIGGER_HUMD_MEASURE_NOHOLD  0xF5
#define WRITE_USER_REG  0xE6
#define READ_USER_REG  0xE7
#define SOFT_RESET  0xFE

enum {
    GET_SHT_TEMPERATURE = 0,
    GET_SHT_HUMIDITY
};

bool SHT21_Init(void);
int16_t SHT21_GetVal(uint8 mode);
bool SHT21_SoftReset(void);

#endif
