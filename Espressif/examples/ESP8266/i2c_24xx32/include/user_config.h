#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#define user_procTaskPrio        0
#define user_procTaskQueueLen    1

#define sleepms(x) os_delay_us(x*1000);

// Device start address 0x50 to 0x57
// 0  b  1  0  1  0  0  A1  A0  R/W
//      MSB                     LSB
#define DEVICEADDRESS 0x50

// Set to a higher number if you want to start at a higher address.
#define MIN_ADDRESS 0x00
// Maximum address (inclusive) in address space.
#define MAX_ADDRESS 0x0000FFFF

#endif
