#include "ets_sys.h"
#include "osapi.h"
#include "driver/uart.h"

static void ICACHE_FLASH_ATTR UARTTxd(char TxChar) {
	// Wait until there is room in the FIFO
	while (true)
	{
		uint32 fifo_cnt = READ_PERI_REG(UART_STATUS(0)) & (UART_TXFIFO_CNT<<UART_TXFIFO_CNT_S);
		if ((fifo_cnt >> UART_TXFIFO_CNT_S & UART_TXFIFO_CNT) < 126) {
			break;
		}
	}
	// Send the character
	WRITE_PERI_REG(UART_FIFO(0), TxChar);
}

static void ICACHE_FLASH_ATTR UARTPutChar(char TxChar) {
	// Convert \n -> \r\n
	if (TxChar == '\n') {
		UARTTxd('\r');
		UARTTxd('\n');
	} else if (TxChar == '\r') {
	} else {
		UARTTxd(TxChar);
	}
}


void UARTInit(UartBautRate uart0_br) {
	// Enable TxD pin
	PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0TXD_U);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);
	
	// Set baud rate and other serial parameters to uart0_br,n,8,1
	uart_div_modify(0, UART_CLK_FREQ/uart0_br);
	WRITE_PERI_REG(UART_CONF0(0), (STICK_PARITY_DIS)|(ONE_STOP_BIT << UART_STOP_BIT_NUM_S)|(EIGHT_BITS << UART_BIT_NUM_S));

	// Reset tx & rx fifo
	SET_PERI_REG_MASK(UART_CONF0(0), UART_RXFIFO_RST|UART_TXFIFO_RST);
	CLEAR_PERI_REG_MASK(UART_CONF0(0), UART_RXFIFO_RST|UART_TXFIFO_RST);
	// Clear pending interrupts
	WRITE_PERI_REG(UART_INT_CLR(0), 0xffff);

	// Install our own putchar handler
	os_install_putc1((void *)UARTPutChar);
}
