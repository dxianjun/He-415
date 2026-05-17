#include <stdio.h>
#include <string.h>

#include "usart_bsp.h"
#include "tim_bsp.h"
#include "exti_bsp.h"

uint8_t UartRxBuf[BUFF_SIZE] = {0};
uint8_t UartRxBuf2[BUFF_SIZE] = {0};
uint8_t UartRxBuf3[BUFF_SIZE] = {0};
uint8_t UartRxBuf4[BUFF_SIZE] = {0};

volatile uint32_t rx_ptr = 0, rx_ptr2 = 0, rx_ptr3 = 0, rx_ptr4 = 0;
volatile uint32_t rx_len = 0;
volatile uint8_t rx_irq_flag = 0;

static uint8_t uc_Rxdata[64] = {0};
static uint8_t TCP_SEND_buf[16] = {0};

static bit16_field_t g_app_flag;
#define f_Pwr_On            g_app_flag.bits.b0
#define f_pre_pwr_off       g_app_flag.bits.b1
#define f_bat_pwr_off       g_app_flag.bits.b2
#define f_full_pwr_off      g_app_flag.bits.b3
#define f_Water_Enable      g_app_flag.bits.b4
#define f_Water_Spring      g_app_flag.bits.b5
#define f_Water_Spring_cnt  g_app_flag.bits.b6

static uint8_t uc_PreOff_cnt = 0;
static uint8_t uc_waterSet = 1;
static uint8_t uc_Dc_In_cnt = 0;
static uint8_t uc_Bat_Low_cnt = 0;
static uint16_t ui_turn_try_set = 0;
static volatile uint16_t ui_turn_try = 0;
static volatile uint8_t f_Dc_In = 0;
static volatile uint8_t f_Bat_Low = 0;
static volatile uint8_t f_Turn_wheel = 0;
static volatile uint8_t f_Turn_Cont = 0;

#define CMD_SET_PWR     0x01
#define CMD_SET_SPEED   0x02
#define CMD_SET_WATER   0x03
#define CMD_SET_STATE   0x04
#define CMD_SET_SPRING  0x05

#define CMD_SEND_ACK    0x01
#define CMD_SEND_INFO   0x02
#define CMD_SEND_PWR    0x03

#define SET_PWR_OFF     0
#define SET_PWR_ON      2

#define V_PWR_OFF_KEY       1
#define V_PWR_OFF_BATLOW    2
#define V_PWR_OFF_WATFULL   3
#define TURN_ADJ            1
#define TURN_180            2
#define ANGLE_180L          170
#define ANGLE_180R          170

#define TTMR_HAND   0
#define TWATR_HAND  1
#define WATER_HAND_TMO 600

static uint8_t *mydatacpy(uint8_t* dataDest, const uint8_t* dataScr, uint16_t len)
{
    uint8_t* temp = dataDest;
    uint16_t i;
    for (i = 0; i < len; i++)
    {
        *temp++ = *dataScr++;
    }
    return dataDest;
}

void uart_gpio_init(void)
{
    std_gpio_init_t usart_gpio_init = {0};

    std_rcc_gpio_clk_enable(RCC_PERIPH_CLK_GPIOA);
    std_rcc_gpio_clk_enable(RCC_PERIPH_CLK_GPIOB);
    std_rcc_gpio_clk_enable(RCC_PERIPH_CLK_GPIOF);

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

static void UART4_Send_Buf(const uint8_t *buf, uint8_t len)
{
    uint8_t i;
    while (!(std_usart_get_flag(UART4, USART_FLAG_TXE))) {}
    for (i = 0; i < len; i++)
    {
        std_usart_tx_write_data(UART4, (uint32_t)buf[i]);
        while (!std_usart_get_flag(UART4, USART_FLAG_TC)) {}
    }
}

void SendToSub_Pwr(uint8_t uc_pwr)
{
    TCP_SEND_buf[0] = 0xA5;
    TCP_SEND_buf[1] = CMD_SET_PWR;
    TCP_SEND_buf[2] = 1;
    TCP_SEND_buf[3] = uc_pwr;
    TCP_SEND_buf[4] = 0xFF;
    UART4_Send_Buf(TCP_SEND_buf, 5);
}

void SendToSub_Water(uint8_t uc_water)
{
    TCP_SEND_buf[0] = 0xA5;
    TCP_SEND_buf[1] = CMD_SET_WATER;
    TCP_SEND_buf[2] = 1;
    TCP_SEND_buf[3] = uc_water;
    TCP_SEND_buf[4] = 0xFF;
    UART4_Send_Buf(TCP_SEND_buf, 5);
}

void SendToSub_State(uint8_t uc_state)
{
    TCP_SEND_buf[0] = 0xA5;
    TCP_SEND_buf[1] = CMD_SET_STATE;
    TCP_SEND_buf[2] = 1;
    TCP_SEND_buf[3] = uc_state;
    TCP_SEND_buf[4] = 0xFF;
    UART4_Send_Buf(TCP_SEND_buf, 5);
}

void SendToSub_Spring(uint8_t uc_spring)
{
    TCP_SEND_buf[0] = 0xA5;
    TCP_SEND_buf[1] = CMD_SET_SPRING;
    TCP_SEND_buf[2] = 1;
    TCP_SEND_buf[3] = uc_spring;
    TCP_SEND_buf[4] = 0xFF;
    UART4_Send_Buf(TCP_SEND_buf, 5);
}

void SendToSub_Speed(uint8_t uc_speed)
{
    TCP_SEND_buf[0] = 0xA5;
    TCP_SEND_buf[1] = CMD_SET_SPEED;
    TCP_SEND_buf[2] = 1;
    TCP_SEND_buf[3] = uc_speed;
    TCP_SEND_buf[4] = 0xFF;
    UART4_Send_Buf(TCP_SEND_buf, 5);
}

static void Power_On(void)
{
    if (f_Pwr_On)
    {
        return;
    }
    f_Pwr_On = 1;
    f_pre_pwr_off = 0;
    f_bat_pwr_off = 0;
    f_full_pwr_off = 0;
    uc_PreOff_cnt = 0;

    SendToSub_Pwr(SET_PWR_ON);
    Delay_1ms(10);
    SendToSub_Pwr(SET_PWR_ON);

    TimOut10mS[TWATR_HAND] = 0;
    printf("power on\r\n");
}

static void Power_Off(void)
{
    f_Pwr_On = 0;
    f_pre_pwr_off = 0;
    f_bat_pwr_off = 0;
    f_full_pwr_off = 0;
    uc_PreOff_cnt = 0;
    f_Water_Spring = 0;
    f_Water_Spring_cnt = 0;

    SendToSub_Spring(0);
    Delay_1ms(10);
    SendToSub_Pwr(SET_PWR_OFF);
    Delay_1ms(10);
    SendToSub_Pwr(SET_PWR_OFF);

    printf("power off\r\n");
}

static void vi_Power_off(uint8_t uc_flg)
{
    f_Pwr_On = 0;
    uc_PreOff_cnt = 0;

    if (uc_flg == V_PWR_OFF_KEY)
    {
        f_pre_pwr_off = 1;
        f_bat_pwr_off = 0;
        f_full_pwr_off = 0;
        SendToSub_State(SS_VK_OFF);
    }
    else if (uc_flg == V_PWR_OFF_BATLOW)
    {
        f_pre_pwr_off = 0;
        f_bat_pwr_off = 1;
        f_full_pwr_off = 0;
        SendToSub_State(SS_BAT_LOW);
    }
    else if (uc_flg == V_PWR_OFF_WATFULL)
    {
        f_pre_pwr_off = 0;
        f_bat_pwr_off = 0;
        f_full_pwr_off = 1;
    }
}

void sys_flg_init(void)
{
    g_app_flag.val = 0;
    f_Water_Enable = 1;
    uc_waterSet = 1;
    uc_PreOff_cnt = 0;
}

static void ParseSubCmd(uint8_t *str, uint16_t len)
{
    uint16_t i;
    uint16_t cnt;
    uint8_t cmd;
    uint8_t *p;

    for (i = 0; i < len; i++)
    {
        p = &str[i];
        if (*p != 0xB5)
        {
            continue;
        }

        if (i + 3 >= len)
        {
            break;
        }

        cnt = (uint16_t)p[2] + 4;
        if ((i + cnt) > len)
        {
            break;
        }

        cmd = p[1];
        if (cmd == CMD_SEND_INFO)
        {
            if (p[3] == SS_WATER_FULL)
            {
                printf("water full\r\n");
                vi_Power_off(V_PWR_OFF_WATFULL);
            }
        }
        else if (cmd == CMD_SEND_PWR)
        {
            if (p[3])
            {
                Power_On();
            }
            else
            {
                vi_Power_off(V_PWR_OFF_KEY);
            }
        }

        i = (uint16_t)(i + cnt - 1);
    }
}

static void ParseBtCmd(uint8_t *str, uint16_t len)
{
    uint16_t i;
    uint16_t cnt;
    uint8_t cmd;
    uint8_t dat;
    uint8_t *p;

    for (i = 0; i < len; i++)
    {
        p = &str[i];
        if (*p != 0xB6)
        {
            continue;
        }
        if (i + 3 >= len)
        {
            break;
        }

        cnt = (uint16_t)p[2] + 4;
        if ((i + cnt) > len)
        {
            break;
        }

        cmd = p[1];
        dat = p[3];
        if (cmd == 1)
        {
            if (dat >= 2)
            {
                Power_On();
            }
            else
            {
                vi_Power_off(V_PWR_OFF_KEY);
            }
        }
        else if (cmd == 2)
        {
            if (!f_Pwr_On)
            {
                i = (uint16_t)(i + cnt - 1);
                continue;
            }

            if (dat == 0)
            {
                uc_waterSet = 0;
                f_Water_Enable = 0;
                f_Water_Spring_cnt = 0;
                SendToSub_Water(0);
            }
            else if (dat == 1)
            {
                uc_waterSet = 1;
                f_Water_Enable = 1;
                f_Water_Spring_cnt = 0;
                SendToSub_Water(1);
            }
            else if (dat == 2)
            {
                uc_waterSet = 2;
                f_Water_Enable = 1;
                f_Water_Spring_cnt = 1;
                SendToSub_Spring(2);
            }
            else if (dat == 3)
            {
                f_Water_Spring_cnt = 1;
                f_Water_Enable = 1;
                SendToSub_Spring(2);
            }
            else if (dat == 4)
            {
                f_Water_Spring_cnt = 0;
                f_Water_Enable = 1;
                SendToSub_Spring(0);
            }
        }
        else if (cmd == 3)
        {
            if (!f_Pwr_On)
            {
                i = (uint16_t)(i + cnt - 1);
                continue;
            }
            if (dat == 0)
            {
                f_Turn_Cont = 0;
                f_Turn_wheel = 0;
                std_tim_set_ccx_value(TIM4, TIM_CHANNEL_1, 500);
                std_tim_set_ccx_value(TIM5, TIM_CHANNEL_1, 500);
            }
            else if (dat == 1)
            {
                f_Turn_Cont = 1;
                f_Turn_wheel = 1;
                ui_turn_try = 0;
                std_tim_set_ccx_value(TIM4, TIM_CHANNEL_1, 600);
                std_tim_set_ccx_value(TIM5, TIM_CHANNEL_1, 375);
            }
            else if (dat == 2)
            {
                f_Turn_Cont = 1;
                f_Turn_wheel = 1;
                ui_turn_try = 0;
                std_tim_set_ccx_value(TIM4, TIM_CHANNEL_1, 375);
                std_tim_set_ccx_value(TIM5, TIM_CHANNEL_1, 600);
            }
            else if (dat == 3)
            {
                f_Turn_Cont = 0;
                f_Turn_wheel = 1;
                ui_turn_try_set = ANGLE_180L;
                ui_turn_try = 0;
                std_tim_set_ccx_value(TIM4, TIM_CHANNEL_1, 600);
                std_tim_set_ccx_value(TIM5, TIM_CHANNEL_1, 250);
            }
            else if (dat == 4)
            {
                f_Turn_Cont = 0;
                f_Turn_wheel = 1;
                ui_turn_try_set = ANGLE_180R;
                ui_turn_try = 0;
                std_tim_set_ccx_value(TIM4, TIM_CHANNEL_1, 250);
                std_tim_set_ccx_value(TIM5, TIM_CHANNEL_1, 600);
            }
        }

        i = (uint16_t)(i + cnt - 1);
    }
}

void tcp_hand(void)
{
    uint8_t uc_len;

    if (TimOut1mS[TTCPIN2] >= 10)
    {
        if (rx_ptr2 > 3)
        {
            uc_len = (uint8_t)rx_ptr2;
            TimOut1mS[TTCPIN2] = 0;
            mydatacpy(uc_Rxdata, UartRxBuf2, rx_ptr2);
            memset(UartRxBuf2, 0, sizeof(UartRxBuf2));
            rx_ptr2 = 0;
            ParseBtCmd(uc_Rxdata, uc_len);
            memset(uc_Rxdata, 0, sizeof(uc_Rxdata));
        }
    }

    if (TimOut1mS[TTCPIN4] >= 10)
    {
        if (rx_ptr4 >= 4)
        {
            uc_len = (uint8_t)rx_ptr4;
            TimOut1mS[TTCPIN4] = 0;
            mydatacpy(uc_Rxdata, UartRxBuf4, rx_ptr4);
            memset(UartRxBuf4, 0, sizeof(UartRxBuf4));
            rx_ptr4 = 0;
            ParseSubCmd(uc_Rxdata, uc_len);
            memset(uc_Rxdata, 0, sizeof(uc_Rxdata));
        }
    }
}

static void Tmr_Hand(void)
{
    if (!f_Pwr_On)
    {
        if (TimOut10mS[TTMR_HAND] >= 100)
        {
            TimOut10mS[TTMR_HAND] = 0;

            if (f_pre_pwr_off)
            {
                uc_PreOff_cnt++;
                if (uc_PreOff_cnt >= 3)
                {
                    Power_Off();
                }
            }
            else if (f_bat_pwr_off)
            {
                uc_PreOff_cnt++;
                if (uc_PreOff_cnt >= 5)
                {
                    Power_Off();
                }
            }
            else if (f_full_pwr_off)
            {
                uc_PreOff_cnt++;
                if (uc_PreOff_cnt >= 3)
                {
                    Power_Off();
                }
            }
        }
        return;
    }

    if (f_Water_Enable)
    {
        if (g_req_spring)
        {
            g_req_spring = 0;
            TimOut10mS[TWATR_HAND] = 0;
            if (f_Water_Spring_cnt == 0)
            {
                f_Water_Spring = 1;
                SendToSub_Spring(1);
            }
        }

        if (TimOut10mS[TWATR_HAND] >= WATER_HAND_TMO)
        {
            TimOut10mS[TWATR_HAND] = 0;
            if (uc_waterSet != 0 && f_Water_Spring_cnt == 0)
            {
                f_Water_Spring = 1;
                SendToSub_Spring(1);
            }
        }
    }
}

static void Turn_Tmo_Hand(void)
{
    if (f_Turn_wheel && (f_Turn_Cont == 0))
    {
        ui_turn_try++;
        if (ui_turn_try >= ui_turn_try_set)
        {
            f_Turn_wheel = 0;
            std_tim_set_ccx_value(TIM4, TIM_CHANNEL_1, 500);
            std_tim_set_ccx_value(TIM5, TIM_CHANNEL_1, 500);
        }
    }
}

static void Input_Hand(void)
{
    if (TimOut10mS[TTMR_HAND] < 5)
    {
        return;
    }
    TimOut10mS[TTMR_HAND] = 0;

    if (std_gpio_get_input_pin(GPIOF, GPIO_PIN_6))
    {
        uc_Dc_In_cnt++;
        if (uc_Dc_In_cnt >= 2)
        {
            uc_Dc_In_cnt = 2;
            if (!f_Dc_In)
            {
                f_Dc_In = 1;
                f_Bat_Low = 0;
                uc_Bat_Low_cnt = 0;
                Power_Off();
            }
        }
    }
    else
    {
        if (uc_Dc_In_cnt > 0)
        {
            uc_Dc_In_cnt--;
        }
        else
        {
            f_Dc_In = 0;
        }
    }

    if (!f_Dc_In)
    {
        if (!std_gpio_get_input_pin(GPIOA, GPIO_PIN_4))
        {
            if (uc_Bat_Low_cnt < 201)
            {
                uc_Bat_Low_cnt++;
            }
            if (uc_Bat_Low_cnt == 200)
            {
                f_Bat_Low = 1;
                vi_Power_off(V_PWR_OFF_BATLOW);
            }
        }
        else if (uc_Bat_Low_cnt > 0)
        {
            uc_Bat_Low_cnt--;
        }
    }
}

void ScheduleHandle(void)
{
    Input_Hand();
    tcp_hand();
    Tmr_Hand();
    Turn_Tmo_Hand();
    Exit_Hand();
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
