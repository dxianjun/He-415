#include <stdio.h>
#include <string.h>

#include "app_tcp.h"
#include "usart_bsp.h"
#include "tim_bsp.h"
#include "app_service.h"
#include "input_bsp.h"

static uint8_t uc_Rxdata[64] = {0};
static uint8_t TCP_SEND_buf[16] = {0};


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

static void UART2_Send_String(const char *str)
{
    while (!(std_usart_get_flag(UART2, USART_FLAG_TXE))) {}
	
    while((*str)!='\0')
    {
        std_usart_tx_write_data(UART2, (uint32_t)*str);
        while (!std_usart_get_flag(UART2, USART_FLAG_TC)) {}
		str++;
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

#define HEAD_VALUE_RECV_SUB		0xB5
#define HEAD_VALUE_RECV_BT		0xB6

void parse_sub(uint8_t *str)
{
	uint8_t cmd = str[1];
	uint8_t dat = str[3];
	
	switch(cmd)
		{
		default: break;

		case CMD_SEND_ACK:
			{
			if (dat == 1)
				{
				
				}
			
			}
			break;

		case CMD_SEND_INFO:
			{
			if (dat == SS_WATER_FULL)
				{
				if (!f_Water_Full)
					{
					// 执行污水满
					f_Water_Full = 1; 
					printf("water full \n");
					vi_Full_warn(V_PWR_OFF_WATFULL);
					}
				}
			}
			break;

		case CMD_SEND_PWR:
			{
			if (dat) 
				{
				printf("uart:pwr\n"); 
				Power_On(); 
				}
			else 
				{
				printf("uart:pwr off\n"); 
				vi_Power_off(V_PWR_OFF_KEY); 
				}
			}
			break;
		}
}

static void ParseSubCmd(uint8_t *str, uint16_t len)
{
    uint16_t i;
    uint16_t cnt;
    uint8_t *p;
	
    for (i = 0; i < len; i++)
    {
        p = &str[i];
        if (*p != HEAD_VALUE_RECV_SUB) { continue; }
        if (i + 3 >= len) { break; }

        cnt = (uint16_t)p[2] + 4;
        if ((i + cnt) > len) { break; }
		
        parse_sub(p);

        i = (uint16_t)(i + cnt - 1);
    }
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

const uint8_t cmd_bt_ready[] = "AT\r\n\0";
uint8_t cmd_bt_pair[] = "AT+FTM=19667F4A76FC\r\n\0";
const uint8_t cmd_readmac[] = "AT+MAC?\r\n\0";

uint8_t bBtReadMacWaiting=false;
static void Bt_Send_ReadMac_Resp(void)
{
    UART2_Send_String((char *)cmd_readmac);
	bBtReadMacWaiting = true;
}

void parse_bt(uint8_t *str)
{
	uint8_t cmd = str[1];
	uint8_t dat = str[3];

#if 1	// 充电时，所有按键都不允许
    if (input_dc_in_is_active())
	    {
        Bt_Send_Ack(BT_ACK_ERR, BT_KEY_PWR, dat);
        return;
	    }
#endif

	switch (cmd)
		{
		default: 
			{
			Bt_Send_Ack(BT_ACK_ERR, cmd, dat);
			}
			break;
		
		case BT_KEY_PWR:
			{
			#if 0
			if (input_dc_in_is_active())
	            {
	                Bt_Send_Ack(BT_ACK_ERR, BT_KEY_PWR, dat);
	                continue;
	            }
			#endif
			
			 // 2~4 都是开机，0和1都是关机
			if (dat >= 2)
				{
				printf("bt:pwr\n");
				Power_On();
				}
			else
				{
				printf("bt:pwr off\n");
				// davidd 20250213, 按键关机改为预关机, 等5秒后, 才能真正关吸水电机并且断电;
				vi_Power_off(V_PWR_OFF_KEY);
				}	
			}
			break;

		case BT_KEY_WATER:
			{			
            if (!f_Pwr_On)
	            {
	                Bt_Send_Ack(BT_ACK_ERR, cmd, dat);
	                break;
	            }
			switch (dat)
				{
				default: 
					{
					Bt_Send_Ack(BT_ACK_ERR, cmd, dat);
					}
					break;
				
				case WATER_STOP:
					{
	                f_Water_Enable = 0;
	                f_Water_Spring_cnt = 0;
	                SendToSub_Water(0);
					Bt_Send_info(BT_INFO_WATER_STOP);
	            	}
					break;

				case WATER_SMAL:
					{
	                f_Water_Enable = 1;
	                f_Water_Spring_cnt = 0;
	                SendToSub_Water(1);
	                Bt_Send_info(BT_INFO_WATER_SMALL);
	            	}
					break;
				
				case WATER_BIG:
					{
					f_Water_Enable = 1;
	                f_Water_Spring_cnt = 1;
	                SendToSub_Spring(2);
	                Bt_Send_info(BT_INFO_WATER_BIG);
					}
					break;

				case WATER_CONT:
					{
					f_Water_Spring_cnt = 1;
	                f_Water_Enable = 1;
					f_Water_Spring = 1;
	                SendToSub_Spring(2);
					}
					break;
					
				case WATER_CONT_REL:
					{
					f_Water_Spring_cnt = 0;
	                f_Water_Enable = 1;
					f_Water_Spring = 0;
	                SendToSub_Spring(0);
					}
					break;
				}
        	}
			break;

		case BT_KEY_READ_MAC:
			{
            Bt_Send_ReadMac_Resp();
			}
			break;

		#if (WHEEL_DRIVER_BY_MOTOR)
		case BT_KEY_MOT:
			{
			if (!f_Pwr_On)
	            {
	                Bt_Send_Ack(BT_ACK_ERR, cmd, dat);
	                continue;
	            }
			
            App_OnMotorCmd(dat);
			}
			break;
		#endif
        
		}

}
static void ParseBtCmd(uint8_t *str, uint16_t len)
{
    uint16_t i;
    uint16_t cnt;

    uint8_t *p;
		
	for (i = 0; i < len; i++)
    {
        if (str[i] == HEAD_VALUE_RECV_BT) 
        	{
        	p = &str[i];
	        if (i + 3 >= len) { break; }

	        cnt = (uint16_t)p[2] + 4;
	        if ((i + cnt) > len) { break; }

	        parse_bt(p);
	        i = (uint16_t)(i + cnt - 1);
        	}
		else
			{
			if (str[i] == '+')		// 回复的信息都是以+开头，以\r\n\0结尾
				{
				if ((str[i] != '+') || (str[i+1] != 'M') || (str[i+2] != 'A') || (str[i+3] != 'C') || (str[i+4] != ':'))
					{
					continue;
					}
				printf("%s", str);
				bBtReadMacWaiting = false;
				
				cnt = 25;	// "AT+FTM=19667F4A76FC\r\n\0";
				i = (uint16_t)(i + cnt - 1);
				}
			}
    }
}


#if 1
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
#endif


