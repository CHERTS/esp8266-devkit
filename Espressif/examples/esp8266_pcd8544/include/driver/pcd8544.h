/*
 * pcd8544.h
 *
 *  PCD8544 bit-banging driver ported from http://playground.arduino.cc/Code/PCD8544
 *
 *  Created on: Jan 7, 2015
 *      Author: Eadf
 */

#ifndef INCLUDE_DRIVER_PCD8544_H_
#define INCLUDE_DRIVER_PCD8544_H_

#include "c_types.h"

// the number of chars per line
#define PCD8544_LCD_CHARS_PER_LINE 12

typedef struct {
  uint8_t lcdVop;
  uint8_t tempCoeff;
  uint8_t biasMode;
  bool inverse;

  int8_t resetPin;
  int8_t scePin;
  int8_t dcPin;
  int8_t sdinPin;
  int8_t sclkPin;
} PCD8544_Settings;

void PCD8544_lcdImage(uint8_t *image);
void PCD8544_lcdClear(void);
void PCD8544_lcdCharacter(char character);
/**
 * Writes a null terminated string
 */
void PCD8544_lcdPrint(char *characters);
/**
 * debug prints a binary to console
 */
void PCD8544_printBinary(uint32_t data);
void PCD8544_gotoXY(int x, int y);
void PCD8544_drawLine(void);
/**
 * print ' ' a number of times, negative 'spaces' value prints nothing.
 */
void PCD8544_lcdPad(int16_t spaces);

/**
 * Sets the contrast [0x00 - 0x7f].
 * Useful, visible range is about 40-60.
 */
void PCD8544_setContrast(uint8_t val);

void PCD8544_initLCD(PCD8544_Settings *settings);
/**
 */
void PCD8544_init(PCD8544_Settings *settings);

#endif /* INCLUDE_DRIVER_PCD8544_H_ */
