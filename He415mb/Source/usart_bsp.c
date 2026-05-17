#include <stdio.h>

#include "usart_bsp.h"
#include "tim_bsp.h"

uint8_t UartRxBuf[BUFF_SIZE] = {0};
uint8_t UartRxBuf2[BUFF_SIZE] = {0};
uint8_t UartRxBuf3[BUFF_SIZE] = {0};
uint8_t UartRxBuf4[BUFF_SIZE] = {0};

volatile uint32_t rx_ptr = 0, rx_ptr2 = 0, rx_ptr3 = 0, rx_ptr4 = 0;
volatile uint32_t rx_len = 0;
volatile uint8_t rx_irq_flag = 0;

void uart_gpio_init(void)
{
    std_gpio_init_t usart_gpio_init = {0};

    std_rcc_gpio_clk_enable(RCC_PERIPH_CLK_GPIOA);
    std_rcc_gpio_clk_enable(RCC_PERIPH_CLK_GPIOB);

    usart_gpio_init.pin = GPIO_PIN_1 | GPIO_PIN_2;
    usart_gpio_init.mode = GPIO_MODE_ALTERNATE;
    usart_gpio_init.pull = GPIO_PULLUP;
    usart_gpio_init.alternate = GPIO_AF1_USART1;
    std_gpio_init(GPIOA, &usart_gpio_init);

    usart_gpio_init.pin = GPIO_PIN_6 | GPIO_PIN_7;
    usart_gpio_init.alternate = GPIO_AF1_UART2;
    std_gpio_init(GPIOB, &usart_gpio_init);

    usart_gpio_init.pin = GPIO_PIN_0 | GPIO_PIN_2;
    usart_gpio_init.alternate = GPIO_AF4_UART3;
    std_gpio_init(GPIOB, &usart_gpio_init);

    usart_gpio_init.pin = GPIO_PIN_1 | GPIO_PIN_3;
    usart_gpio_init.alternate = GPIO_AF4_UART4;
    std_gpio_init(GPIOA, &usart_gpio_init);
}

void uart_init(void)
{
    std_usart_init_t usart_init = {0};

    std_rcc_apb1_clk_enable(RCC_PERIPH_CLK_UART2);
    std_rcc_apb1_clk_enable(RCC_PERIPH_CLK_UART3);
    std_rcc_apb1_clk_enable(RCC_PERIPH_CLK_UART4);

    usart_init.baudrate = 115200;
    usart_init.direction = USART_DIRECTION_SEND_RECEIVE;
    usart_init.hardware_flow = USART_FLOWCONTROL_NONE;
    usart_init.parity = USART_PARITY_NONE;
    usart_init.stopbits = USART_STOPBITS_1;
    usart_init.wordlength = USART_WORDLENGTH_8BITS;
    (void)std_usart_init(UART3, &usart_init);

    usart_init.baudrate = 9600;
    (void)std_usart_init(UART2, &usart_init);
    (void)std_usart_init(UART4, &usart_init);

    std_usart_enable(UART2);
    std_usart_enable(UART3);
    std_usart_enable(UART4);
}

void usart_init_int(void)
{
    std_usart_cr1_interrupt_enable(UART2, USART_CR1_INTERRUPT_RXNE);
    NVIC_SetPriority(UART2_IRQn, NVIC_PRIO_1);
    NVIC_EnableIRQ(UART2_IRQn);

    std_usart_cr1_interrupt_enable(UART3, USART_CR1_INTERRUPT_RXNE);
    std_usart_cr1_interrupt_enable(UART4, USART_CR1_INTERRUPT_RXNE);
    NVIC_SetPriority(UART3_4_IRQn, NVIC_PRIO_1);
    NVIC_EnableIRQ(UART3_4_IRQn);
}

#define PRINT_COM UART3
#if defined(__GNUC__)
int _write(int fd, char* ptr, int len)
{
    uint32_t i;
    (void)fd;
    for (i = 0; i < (uint32_t)len; i++)
    {
        std_usart_tx_write_data(PRINT_COM, (uint32_t)ptr[i]);
        while (!std_usart_get_flag(PRINT_COM, USART_FLAG_TC)) {}
    }
    return len;
}
#else
int fputc(int ch, FILE *f)
{
    (void)f;
    std_usart_tx_write_data(PRINT_COM, (uint32_t)ch);
    while (!std_usart_get_flag(PRINT_COM, USART_FLAG_TC)) {}
    return ch;
}
#endif

void USART1_IRQHandler(void)
{
}

void UART2_IRQHandler(void)
{
    if ((std_usart_get_cr1_interrupt_enable(UART2, USART_CR1_INTERRUPT_RXNE)
      || std_usart_get_cr3_interrupt_enable(UART2, USART_CR3_INTERRUPT_ERR))
      && std_usart_get_flag(UART2, USART_FLAG_ORE))
    {
        std_usart_clear_flag(UART2, USART_CLEAR_ORE);
    }

    if (std_usart_get_cr1_interrupt_enable(UART2, USART_CR1_INTERRUPT_RXNE))
    {
        if (std_usart_get_flag(UART2, USART_FLAG_RXNE))
        {
            if (rx_ptr2 < BUFF_SIZE)
            {
                UartRxBuf2[rx_ptr2++] = (uint8_t)std_usart_rx_read_data(UART2);
            }
            TimOut1mS[TTCPIN2] = 0;
        }
    }
}

void UART3_4_IRQHandler(void)
{
    if ((std_usart_get_cr1_interrupt_enable(UART3, USART_CR1_INTERRUPT_RXNE)
      || std_usart_get_cr3_interrupt_enable(UART3, USART_CR3_INTERRUPT_ERR))
      && std_usart_get_flag(UART3, USART_FLAG_ORE))
    {
        std_usart_clear_flag(UART3, USART_CLEAR_ORE);
    }

    if (std_usart_get_cr1_interrupt_enable(UART3, USART_CR1_INTERRUPT_RXNE))
    {
        if (std_usart_get_flag(UART3, USART_FLAG_RXNE))
        {
            if (rx_ptr3 < BUFF_SIZE)
            {
                UartRxBuf3[rx_ptr3++] = (uint8_t)std_usart_rx_read_data(UART3);
            }
            TimOut1mS[TTCPIN3] = 0;
        }
    }

    if ((std_usart_get_cr1_interrupt_enable(UART4, USART_CR1_INTERRUPT_RXNE)
      || std_usart_get_cr3_interrupt_enable(UART4, USART_CR3_INTERRUPT_ERR))
      && std_usart_get_flag(UART4, USART_FLAG_ORE))
    {
        std_usart_clear_flag(UART4, USART_CLEAR_ORE);
    }

    if (std_usart_get_cr1_interrupt_enable(UART4, USART_CR1_INTERRUPT_RXNE))
    {
        if (std_usart_get_flag(UART4, USART_FLAG_RXNE))
        {
            if (rx_ptr4 < BUFF_SIZE)
            {
                UartRxBuf4[rx_ptr4++] = (uint8_t)std_usart_rx_read_data(UART4);
            }
            TimOut1mS[TTCPIN4] = 0;
        }
    }
}

uint8_t g_send_buffer2[8] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};
uint8_t g_send_buffer4[8] = {0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18};

void uart2_4_test(void)
{
    uint8_t i;

    while(!(std_usart_get_flag(UART2,USART_FLAG_TXE))){}
    for(i = 0; i < 8; i++)
    {
        std_usart_tx_write_data(UART2,(uint32_t)g_send_buffer2[i]);
        while(!std_usart_get_flag(UART2, USART_FLAG_TC));
    }

    while(!(std_usart_get_flag(UART4,USART_FLAG_TXE))){}
    for(i = 0; i < 8; i++)
    {
        std_usart_tx_write_data(UART4,(uint32_t)g_send_buffer4[i]);
        while(!std_usart_get_flag(UART4, USART_FLAG_TC));
    }
}
