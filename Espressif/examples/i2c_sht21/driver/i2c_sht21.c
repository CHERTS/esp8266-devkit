/* reaper7 */

#include "driver/i2c.h"
#include "driver/i2c_sht21.h"

bool ICACHE_FLASH_ATTR
SHT21_SoftReset()
{
  //Soft reset the SHT21
  i2c_start();
  i2c_writeByte(SHT21_ADDRESS);
  if (!i2c_check_ack()) {
    //os_printf("-%s-%s slave not ack... return \r\n", __FILE__, __func__);
    i2c_stop();
    return(0);
  }
  i2c_writeByte(SOFT_RESET);
  if (!i2c_check_ack()) {
    //os_printf("-%s-%s slave not ack... return \r\n", __FILE__, __func__);
    i2c_stop();
    return(0);
  }
  i2c_stop();
  return(1);
}

bool ICACHE_FLASH_ATTR
SHT21_Init()
{
  return(SHT21_SoftReset());
}

int16_t ICACHE_FLASH_ATTR
SHT21_GetVal(uint8 mode)
{
  i2c_start(); //Start i2c
  i2c_writeByte(SHT21_ADDRESS);
  if (!i2c_check_ack()) {
    //os_printf("-%s-%s slave not ack... return \r\n", __FILE__, __func__);
    i2c_stop();
    return(0);
  }

  if (mode==GET_SHT_TEMPERATURE)
    i2c_writeByte(TRIGGER_TEMP_MEASURE_NOHOLD);
  else if (mode==GET_SHT_HUMIDITY)
    i2c_writeByte(TRIGGER_HUMD_MEASURE_NOHOLD);
  else
    return 0;
    
  if (!i2c_check_ack()) {
    //os_printf("-%s-%s slave not ack... return \r\n", __FILE__, __func__);
    i2c_stop();
    return(0);
  }
  
  os_delay_us(20);
  
  i2c_stop();
  
  os_delay_us(70000);
  
  uint8 ack = 0;
  while (!ack) {
    i2c_start();
    i2c_writeByte(SHT21_ADDRESS+1);
    ack = i2c_check_ack();
    if (!ack) i2c_stop();
  }
  //os_printf("-%s-%s get ack \r\n", __FILE__, __func__);

  uint8 msb = i2c_readByte();
  i2c_send_ack(1);
  uint8 lsb = i2c_readByte();
  i2c_send_ack(0);
  i2c_stop();
     
  uint16_t _rv = msb << 8;
  _rv += lsb;
  _rv &= ~0x0003; 
 
  //os_printf("-%s-%s RAW:%d \r\n", __FILE__, __func__,_rv);

  float rv = _rv;
  
  if (mode==GET_SHT_TEMPERATURE) {
    rv *= 175.72;
    rv /= 65536;
    rv -= 46.85;
  } else if (mode==GET_SHT_HUMIDITY) {
    rv *= 125.0;
    rv /= 65536;
    rv -= 6.0; 
  }
  
  return (int16_t)(rv*10);
}
