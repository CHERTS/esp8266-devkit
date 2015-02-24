/*
 * Written By Hans
 * This is my first version of the driver for BH1750
 * I used few codes i found online to produce this
 * Big thanks who ever wrote the I2c Drivers
 */
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "c_types.h"
#include "user_interface.h"
#include "driver/bh1750.h"
#include "driver/i2c.h"

#define BH1750address 0x23 //This is the Address for when add line Low
//#define BH1750address 0x5C //This is the Address for when add line High

int ICACHE_FLASH_ATTR GetLight()
{
    uint8 dtmp1, dtmp2;
    uint16 res;
    int data;

    i2c_init();
    if (BH1750Init())
    {
    	i2c_start();
    	uint8 readAdd = BH1750address << 1; //shift the address
    	readAdd = readAdd +0x1;				//add the read bit

    	i2c_writeByte(readAdd);
    	if (!i2c_check_ack())
    	{
    		i2c_stop();
    		return -1;
    	}
    	dtmp1=i2c_readByte();	//read MSB
    	i2c_send_ack(1);
    	dtmp2 = i2c_readByte();	//read LSB
    	i2c_send_ack(0);        //NACK READY FOR STOP
    	i2c_stop();

    	//start doing the calculation
    	res = ((dtmp1<<8)|dtmp2);
    	data = (res*10)/12;		//CAL VALUE
    	return data;
    }
    else
    	return -1;
}


bool ICACHE_FLASH_ATTR BH1750Init()
{
    i2c_start();
    i2c_writeByte(BH1750address << 1);
    if (!i2c_check_ack())
    {
        i2c_stop();
        return 0;
    }
    i2c_writeByte(0x10);
    if (!i2c_check_ack())
    {
        i2c_stop();
        return 0;
    }
    i2c_stop();
    return 1;
}

