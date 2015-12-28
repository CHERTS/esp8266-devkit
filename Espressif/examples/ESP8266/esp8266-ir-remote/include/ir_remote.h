/*
 * ir_remote.h
 *
 * Version 1.0, 06.04.2015
 * Written by Valeriy Kucherenko
 * For details, see https://github.com/valkuc/esp8266-ir-remote
 */

#ifndef __IR_REMOTE_H__
#define __IR_REMOTE_H__

/**
  * @brief  Initialize IR-remote.
  * @param  gpio_pin_num: Board GPIO pin number (example: GPIO0 - 0, GPIO2 - 2 and so on).
  *         invert_gpio: Invert logic level - logic HIGH => LOW and logic LOW => HIGH. Suitable when IR-led is pulled-up by default.
  * @retval None
  */
void ir_remote_init(uint16 gpio_pin_num, bool invert_gpio);

/**
  * @brief  Send NEC code.
  * @param  data: Code to transmit.
  *         nbits: Number of bits to transmit. Typically 32 bits.
  * @retval None
  */
void ir_remote_send_nec(uint32_t data, uint8_t nbits);

/**
  * @brief  Send Panasonic code.
  * @param  address: Protocol address.
  *         data: Code to transmit.
  * @retval None
  */
void ir_remote_send_panasonic(uint16_t address, uint32_t data);

/**
  * @brief  Send Sony code.
  * @param  data: Code to transmit.
  *         nbits: Number of bits to transmit. Typically 32 bits.
  * @retval None
  */
void ir_remote_send_sony(uint32_t data, uint8_t nbits);

/**
  * @brief  Send Samsung code.
  * @param  data: Code to transmit.
  *         nbits: Number of bits to transmit. Typically 32 bits.
  * @retval None
  */
void ir_remote_send_samsung(uint32_t data, uint8_t nbits);

/**
  * @brief  Send raw data.
  * @param  buf: Array of delays (us) to transmit.
  *         nbits: Count of elements in buffer.
  *         freq_hz: Carrier frequency (Hz).
  * @retval None
  */
void ir_remote_send_raw(uint16_t buf[], uint16_t len, uint16_t freq_hz);

#endif
