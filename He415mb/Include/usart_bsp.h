#ifndef USART_BSP_H
#define USART_BSP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ciu32l051_std.h"

#define BUFF_SIZE 64

extern uint8_t UartRxBuf[];
extern uint8_t UartRxBuf2[];
extern uint8_t UartRxBuf3[];
extern uint8_t UartRxBuf4[];

extern volatile uint32_t rx_ptr;
extern volatile uint32_t rx_ptr2;
extern volatile uint32_t rx_ptr3;
extern volatile uint32_t rx_ptr4;

extern volatile uint32_t rx_len;
extern volatile uint8_t rx_irq_flag;

void uart_gpio_init(void);
void uart_init(void);
void usart_init_int(void);

void USART1_IRQHandler(void);
void UART2_IRQHandler(void);
void UART3_4_IRQHandler(void);

void uart2_4_test(void);

#ifdef __cplusplus
}
#endif

#endif


