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

#define BT_KEY_PWR      0x01
#define BT_KEY_WATER    0x02
#define BT_KEY_MOT      0x03
#define BT_KEY_READ_MAC 0x20
#define BT_SEND_ACK     0x10
#define BT_SEND_INFO    0x11

#define SET_PWR_OFF     0
#define SET_PWR_ON      2

#define BT_ACK_OK       0
#define BT_ACK_ERR      1

#define BT_INFO_PWR_OFF 0
#define BT_INFO_STANDBY 1
#define BT_INFO_PWR_ON  2
#define BT_INFO_WATER_S 3
#define BT_INFO_WATER_B 4

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

static void UART2_Send_Buf(const uint8_t *buf, uint8_t len)
{
    uint8_t i;
    while (!(std_usart_get_flag(UART2, USART_FLAG_TXE))) {}
    for (i = 0; i < len; i++)
    {
        std_usart_tx_write_data(UART2, (uint32_t)buf[i]);
        while (!std_usart_get_flag(UART2, USART_FLAG_TC)) {}
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

void Bt_Send_info(uint8_t uc_info)
{
    TCP_SEND_buf[0] = 0xA6;
    TCP_SEND_buf[1] = BT_SEND_INFO;
    TCP_SEND_buf[2] = 1;
    TCP_SEND_buf[3] = uc_info;
    TCP_SEND_buf[4] = 0xFF;
    UART2_Send_Buf(TCP_SEND_buf, 5);
}

void Bt_Send_Ack(uint8_t ack, uint8_t cmd, uint8_t para)
{
    TCP_SEND_buf[0] = 0xA6;
    TCP_SEND_buf[1] = BT_SEND_ACK;
    TCP_SEND_buf[2] = 3;
    TCP_SEND_buf[3] = ack;
    TCP_SEND_buf[4] = cmd;
    TCP_SEND_buf[5] = para;
    TCP_SEND_buf[6] = 0xFF;
    UART2_Send_Buf(TCP_SEND_buf, 7);
}

void Bt_Setting(void)
{
    /* 保留V3接口，后续如需AT配置可在此扩展 */
}

static void Bt_Send_ReadMac_Resp(void)
{
    TCP_SEND_buf[0] = 0xA6;
    TCP_SEND_buf[1] = BT_KEY_READ_MAC;
    TCP_SEND_buf[2] = 0;
    TCP_SEND_buf[3] = 0xFF;
    UART2_Send_Buf(TCP_SEND_buf, 4);
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

        if (cmd == BT_KEY_PWR)
        {
            if (cnt != 5)
            {
                Bt_Send_Ack(BT_ACK_ERR, cmd, dat);
                i = (uint16_t)(i + cnt - 1);
                continue;
            }

            /* 充电状态下禁止开关机命令，回发ACK */
            if (std_gpio_get_input_pin(GPIOA, GPIO_PIN_12))
            {
                Bt_Send_Ack(BT_ACK_ERR, BT_KEY_PWR, dat);
                i = (uint16_t)(i + cnt - 1);
                continue;
            }

            if (dat == 0)
            {
                vi_Power_off(V_PWR_OFF_KEY);
            }
            else if (dat == 1)
            {
                Enter_Standby();
            }
            else
            {
                Power_On();
            }
        }
        else if (cmd == BT_KEY_WATER)
        {
            if (cnt != 5)
            {
                Bt_Send_Ack(BT_ACK_ERR, cmd, dat);
                i = (uint16_t)(i + cnt - 1);
                continue;
            }

            if (!f_Pwr_On || f_Standby)
            {
                Bt_Send_Ack(BT_ACK_ERR, cmd, dat);
                i = (uint16_t)(i + cnt - 1);
                continue;
            }

            if (dat == 0)
            {
                f_Water_Enable = 0;
                f_Water_Spring_cnt = 0;
                SendToSub_Water(0);
                Bt_Send_info(BT_INFO_PWR_ON);
            }
            else if (dat == 1)
            {
                f_Water_Enable = 1;
                f_Water_Spring_cnt = 0;
                SendToSub_Water(1);
                Bt_Send_info(BT_INFO_WATER_S);
            }
            else if (dat == 2)
            {
                f_Water_Enable = 1;
                f_Water_Spring_cnt = 1;
                SendToSub_Spring(2);
                Bt_Send_info(BT_INFO_WATER_B);
            }
            else if (dat == 3)
            {
                f_Water_Spring_cnt = 1;
                f_Water_Enable = 1;
                SendToSub_Spring(2);
                Bt_Send_info(BT_INFO_WATER_B);
            }
            else if (dat == 4)
            {
                f_Water_Spring_cnt = 0;
                f_Water_Enable = 1;
                SendToSub_Spring(0);
                Bt_Send_info(BT_INFO_PWR_ON);
            }
            else
            {
                Bt_Send_Ack(BT_ACK_ERR, cmd, dat);
            }
        }
        else if (cmd == BT_KEY_MOT)
        {
            if (cnt != 5 || dat > 4 || !f_Pwr_On || f_Standby)
            {
                Bt_Send_Ack(BT_ACK_ERR, cmd, dat);
                i = (uint16_t)(i + cnt - 1);
                continue;
            }
            App_OnMotorCmd(dat);
        }
        else if (cmd == BT_KEY_READ_MAC)
        {
            if (cnt != 4)
            {
                Bt_Send_Ack(BT_ACK_ERR, cmd, 0);
                i = (uint16_t)(i + cnt - 1);
                continue;
            }
            Bt_Send_ReadMac_Resp();
        }
        else
        {
            Bt_Send_Ack(BT_ACK_ERR, cmd, dat);
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
