#ifndef _USER_SWITCH_H
#define _USER_SWITCH_H
#include "os_type.h"
#include "ets_sys.h"
#include "osapi.h"
#include "c_types.h"
#include "gpio.h"

#define SWITCH_HOLD_MUX PERIPHS_IO_MUX_MTMS_U
#define SWITCH_HOLD_NUM 14
#define SWITCH_HOLD_FUNC  FUNC_GPIO14
#define SWITCH_HOLD_PIN GPIO_Pin_14

#define _SWITCH_GPIO_HOLD()  GPIO_OUTPUT_SET(SWITCH_HOLD_NUM,0x1);
#define _SWITCH_GPIO_RELEASE() GPIO_OUTPUT_SET(SWITCH_HOLD_NUM,0x0);


#define SWITCH_INPUT_CHANNLE_NUM  4


#define SWITCH_INPUT_01_IO_MUX PERIPHS_IO_MUX_MTDI_U
#define SWITCH_INPUT_01_IO_NUM  12
#define SWITCH_INPUT_01_IO_FUNC FUNC_GPIO12
#define SWITCH_INPUT_01_IO_PIN GPIO_Pin_12

#define SWITCH_INPUT_02_IO_MUX PERIPHS_IO_MUX_MTCK_U
#define SWITCH_INPUT_02_IO_NUM 13
#define SWITCH_INPUT_02_IO_FUNC FUNC_GPIO13
#define SWITCH_INPUT_02_IO_PIN  GPIO_Pin_13

#define SWITCH_INPUT_03_IO_MUX PERIPHS_IO_MUX_GPIO4_U
#define SWITCH_INPUT_03_IO_NUM 4
#define SWITCH_INPUT_03_IO_FUNC FUNC_GPIO4 
#define SWITCH_INPUT_03_IO_PIN GPIO_Pin_4

#define SWITCH_INPUT_04_IO_MUX PERIPHS_IO_MUX_GPIO5_U 
#define SWITCH_INPUT_04_IO_NUM 5
#define SWITCH_INPUT_04_IO_FUNC FUNC_GPIO5
#define SWITCH_INPUT_04_IO_PIN GPIO_Pin_5




#define GPIO_Pin_0              (BIT(0))  /* Pin 0 selected */
#define GPIO_Pin_1              (BIT(1))  /* Pin 1 selected */
#define GPIO_Pin_2              (BIT(2))  /* Pin 2 selected */
#define GPIO_Pin_3              (BIT(3))  /* Pin 3 selected */
#define GPIO_Pin_4              (BIT(4))  /* Pin 4 selected */
#define GPIO_Pin_5              (BIT(5))  /* Pin 5 selected */
#define GPIO_Pin_6              (BIT(6))  /* Pin 6 selected */
#define GPIO_Pin_7              (BIT(7))  /* Pin 7 selected */
#define GPIO_Pin_8              (BIT(8))  /* Pin 8 selected */
#define GPIO_Pin_9              (BIT(9))  /* Pin 9 selected */
#define GPIO_Pin_10             (BIT(10)) /* Pin 10 selected */
#define GPIO_Pin_11             (BIT(11)) /* Pin 11 selected */
#define GPIO_Pin_12             (BIT(12)) /* Pin 12 selected */
#define GPIO_Pin_13             (BIT(13)) /* Pin 13 selected */
#define GPIO_Pin_14             (BIT(14)) /* Pin 14 selected */
#define GPIO_Pin_15             (BIT(15)) /* Pin 15 selected */
#define GPIO_Pin_All            (0xFFFF)  /* All pins selected */

#define GPIO_PIN_REG_0          PERIPHS_IO_MUX_GPIO0_U
#define GPIO_PIN_REG_1          PERIPHS_IO_MUX_U0TXD_U
#define GPIO_PIN_REG_2          PERIPHS_IO_MUX_GPIO2_U
#define GPIO_PIN_REG_3          PERIPHS_IO_MUX_U0RXD_U
#define GPIO_PIN_REG_4          PERIPHS_IO_MUX_GPIO4_U
#define GPIO_PIN_REG_5          PERIPHS_IO_MUX_GPIO5_U
#define GPIO_PIN_REG_6          PERIPHS_IO_MUX_SD_CLK_U
#define GPIO_PIN_REG_7          PERIPHS_IO_MUX_SD_DATA0_U
#define GPIO_PIN_REG_8          PERIPHS_IO_MUX_SD_DATA1_U
#define GPIO_PIN_REG_9          PERIPHS_IO_MUX_SD_DATA2_U
#define GPIO_PIN_REG_10         PERIPHS_IO_MUX_SD_DATA3_U
#define GPIO_PIN_REG_11         PERIPHS_IO_MUX_SD_CMD_U
#define GPIO_PIN_REG_12         PERIPHS_IO_MUX_MTDI_U
#define GPIO_PIN_REG_13         PERIPHS_IO_MUX_MTCK_U
#define GPIO_PIN_REG_14         PERIPHS_IO_MUX_MTMS_U
#define GPIO_PIN_REG_15         PERIPHS_IO_MUX_MTDO_U

#define GPIO_PIN_REG(i) \
    (i==0) ? GPIO_PIN_REG_0:  \
    (i==1) ? GPIO_PIN_REG_1:  \
    (i==2) ? GPIO_PIN_REG_2:  \
    (i==3) ? GPIO_PIN_REG_3:  \
    (i==4) ? GPIO_PIN_REG_4:  \
    (i==5) ? GPIO_PIN_REG_5:  \
    (i==6) ? GPIO_PIN_REG_6:  \
    (i==7) ? GPIO_PIN_REG_7:  \
    (i==8) ? GPIO_PIN_REG_8:  \
    (i==9) ? GPIO_PIN_REG_9:  \
    (i==10)? GPIO_PIN_REG_10: \
    (i==11)? GPIO_PIN_REG_11: \
    (i==12)? GPIO_PIN_REG_12: \
    (i==13)? GPIO_PIN_REG_13: \
    (i==14)? GPIO_PIN_REG_14: \
    GPIO_PIN_REG_15

#define GPIO_PIN_ADDR(i)        (GPIO_PIN0_ADDRESS + i*4)

#define GPIO_ID_IS_PIN_REGISTER(reg_id) \
    ((reg_id >= GPIO_ID_PIN0) && (reg_id <= GPIO_ID_PIN(GPIO_PIN_COUNT-1)))

#define GPIO_REGID_TO_PINIDX(reg_id) ((reg_id) - GPIO_ID_PIN0)
#if 0 
typedef enum {
    GPIO_PIN_INTR_DISABLE = 0,
    GPIO_PIN_INTR_POSEDGE = 1,
    GPIO_PIN_INTR_NEGEDGE = 2,
    GPIO_PIN_INTR_ANYEDGE = 3,
    GPIO_PIN_INTR_LOLEVEL = 4,
    GPIO_PIN_INTR_HILEVEL = 5
} GPIO_INT_TYPE;
#endif

typedef enum {
    GPIO_Mode_Input = 0x0,
    GPIO_Mode_Out_OD,
    GPIO_Mode_Output ,
    GPIO_Mode_Sigma_Delta ,
} GPIOMode_TypeDef;

typedef enum {
    GPIO_PullUp_DIS = 0x0,
    GPIO_PullUp_EN  = 0x1,
} GPIO_Pullup_IF;

typedef struct {
    uint16           GPIO_Pin;
    GPIOMode_TypeDef GPIO_Mode;
    GPIO_Pullup_IF   GPIO_Pullup;
    GPIO_INT_TYPE    GPIO_IntrType;
} GPIO_ConfigTypeDef;

#define GPIO_PIN_DRIVER_LSB                 2
#define GPIO_PIN_DRIVER_MASK                (0x00000001<<GPIO_PIN_DRIVER_LSB)
#define GPIO_AS_INPUT(gpio_bits)    gpio_output_set(0, 0, 0, gpio_bits)
#define GPIO_AS_OUTPUT(gpio_bits)   gpio_output_set(0, 0, gpio_bits, 0)
#define GPIO_SIGMA_DELTA_ADDRESS            0x68
#define SIGMA_DELTA_ENABLE                      BIT16
#define SIGMA_DELTA_ENABLE_S                    16
#define SIGMA_DELTA_PRESCALAR                   0x000000ff
#define SIGMA_DELTA_PRESCALAR_S                 8
#define SIGMA_DELTA_TARGET                      0x000000ff
#define SIGMA_DELTA_TARGET_S                    0

void user_SwitchInit();
void user_SwitchReact();

#endif
