#if 1
#ifndef _GPIO_UART_H
#define _GPIO_UART_H
#define REG_READ(_r) (*(volatile uint32 *)(_r))
#define DR_REG_WDEV_SCH_BASE  0x3ff20c00
#define WDEVTIME                                (DR_REG_WDEV_SCH_BASE + 0x0000)
#define WDEV_NOW()               (REG_READ(WDEVTIME))


  #define GPIO_UART_IN_NUM 14
  #define GPIO_UART_IN_MUX PERIPHS_IO_MUX_MTMS_U
  #define GPIO_UART_IN_FUNC  FUNC_GPIO14


  #define GPIO_UART_BITNUM  8
  #define GPIO_UART_BAUDRATE  9600
  #define GPIO_UART_CYCLE_T (1000000/GPIO_UART_BAUDRATE)
  #define GPIO_UART_HALF_CYCLE_T  (1000000/GPIO_UART_BAUDRATE/2)
  #define GPIO_UART_MARGIN_US    10

void ICACHE_FLASH_ATTR gpio_uart_rx_init();

#endif
#endif