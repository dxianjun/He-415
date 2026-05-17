#include <stdio.h>
#include <string.h>

#include "app_tcp.h"
#include "usart_bsp.h"
#include "tim_bsp.h"
#include "app_service.h"

static uint8_t uc_Rxdata[64] = {0};
static uint8_t TCP_SEND_buf[16] = {0};

#define CMD_SET_PWR     0x01
#define CMD_SET_SPEED   0x02
#define CMD_SET_WATER   0x03
#define CMD_SET_STATE   0x04
#define CMD_SET_SPRING  0x05

#define CMD_SEND_INFO   0x02
#define CMD_SEND_PWR    0x03

#define SET_PWR_OFF     0
#define SET_PWR_ON      2

#define V_PWR_OFF_KEY       1
#define V_PWR_OFF_WATFULL   3

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

static void ParseSubCmd(uint8_t *str, uint16_t len)
{
    uint16_t i;
    uint16_t cnt;
    uint8_t *p;

    for (i = 0; i < len; i++)
    {
        p = &str[i];
        if (*p != 0xB5) { continue; }
        if (i + 3 >= len) { break; }

        cnt = (uint16_t)p[2] + 4;
        if ((i + cnt) > len) { break; }

        if (p[1] == CMD_SEND_INFO)
        {
            if (p[3] == SS_WATER_FULL)
            {
                printf("water full\r\n");
                vi_Power_off(V_PWR_OFF_WATFULL);
            }
        }
        else if (p[1] == CMD_SEND_PWR)
        {
            if (p[3]) { Power_On(); }
            else { vi_Power_off(V_PWR_OFF_KEY); }
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
        if (*p != 0xB6) { continue; }
        if (i + 3 >= len) { break; }

        cnt = (uint16_t)p[2] + 4;
        if ((i + cnt) > len) { break; }

        cmd = p[1];
        dat = p[3];

        if (cmd == 1)
        {
            if (dat >= 2) { Power_On(); }
            else { vi_Power_off(V_PWR_OFF_KEY); }
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
                f_Water_Enable = 0;
                f_Water_Spring_cnt = 0;
                SendToSub_Water(0);
            }
            else if (dat == 1)
            {
                f_Water_Enable = 1;
                f_Water_Spring_cnt = 0;
                SendToSub_Water(1);
            }
            else if (dat == 2)
            {
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
            App_OnMotorCmd(dat);
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
            memset(UartRxBuf2, 0, BUFF_SIZE);
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
            memset(UartRxBuf4, 0, BUFF_SIZE);
            rx_ptr4 = 0;
            ParseSubCmd(uc_Rxdata, uc_len);
            memset(uc_Rxdata, 0, sizeof(uc_Rxdata));
        }
    }
}
