/*
 * ir_remote_def.h
 *
 * Version 1.0, 06.04.2015
 * Written by Valeriy Kucherenko
 * For details, see https://github.com/valkuc/esp8266-ir-remote
 *
 * IR code defines are based on https://github.com/shirriff/Arduino-IRremote by Ken Shirriff
 */

#ifndef __IR_REMOTE_DEF_H__
#define __IR_REMOTE_DEF_H__

#define CLOCK_DIV_1 	0
#define CLOCK_DIV_16 	4
#define CLOCK_DIV_256 	8

#define TM_LEVEL_INT 	1
#define TM_EDGE_INT 	0

#define FRC1_ENABLE_TIMER	BIT7
#define PWM_1S				1000000

// 0xffffffff/(80000000/1)=35
// 0xffffffff/(80000000/16)=35A
// 0xffffffff/(80000000/256)=35AF
#define US_TO_RTC_TIMER_TICKS(t)	((t) ? (((t) > 0x35AF) ? (((t)>>2) * (TIMER_CLK_FREQ/250000) + ((t)&0x3) * (TIMER_CLK_FREQ / PWM_1S))  : (((t) *TIMER_CLK_FREQ) / PWM_1S)) : 0)

#define FREQ_TO_TICKS(x)	US_TO_RTC_TIMER_TICKS((PWM_1S / (x)))


#define TOPBIT 0x80000000

#define NEC_FREQUENCY	38400
#define NEC_HDR_MARK	9000
#define NEC_HDR_SPACE	4500
#define NEC_BIT_MARK	562
#define NEC_ONE_SPACE	1687
#define NEC_ZERO_SPACE	562

#define PANASONIC_FREQUENCY		35000
#define PANASONIC_HDR_MARK 		3502
#define PANASONIC_HDR_SPACE 	1750
#define PANASONIC_BIT_MARK 		502
#define PANASONIC_ONE_SPACE 	1244
#define PANASONIC_ZERO_SPACE 	400

#define SONY_FREQUENCY	40000
#define SONY_HDR_MARK	2400
#define SONY_HDR_SPACE	600
#define SONY_ONE_MARK	1200
#define SONY_ZERO_MARK	600

#define SAMSUNG_FREQUENCY	38400
#define SAMSUNG_HDR_MARK  	5000
#define SAMSUNG_HDR_SPACE 	5000
#define SAMSUNG_BIT_MARK  	560
#define SAMSUNG_ONE_SPACE 	1600
#define SAMSUNG_ZERO_SPACE  560

#endif
