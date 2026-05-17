#ifndef APP_TCP_H
#define APP_TCP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ciu32l051_std.h"

typedef enum
{
    SS_NONE = 0,
    SS_VK_OFF,
    SS_BAT_LOW,
    SS_WATER_FULL
} ss_state_t;

#define MAX_BUF_NUM 2
#define MAX_BUF_SIZE 64

typedef enum
{
	DATA_NONE = 0,
	DATA_FROM_APP,
	DATA_FROM_BT,
	DATA_FROM_SUB
}UartDataType_TypeDef;


typedef struct
{
UartDataType_TypeDef UartDataType;		/* Specifies the Uart data type	*/
uint16_t UartData_Len;					/* Specifies the Uart data package length	*/						
}UartData_TypeDef;


#define MOTOR_DUTY	100

/*
主板下发	包头		cmd len data	结束
		A5		cmd xxx datax	0xFF
*/

#define CMD_SET_PWR		0x01
#define CMD_SET_SPEED	0x02
#define CMD_SET_WATER	0x03
#define CMD_SET_STATE	0x04
#define CMD_SET_SPRING	0x05


/*
上传		cmd	len	data	
应答	01	0	无	
上报消息	02	1	0/3			// 0：正常状态 3：水满状态"
*/
#define CMD_SEND_ACK	0x01
#define CMD_SEND_INFO	0x02
#define CMD_SEND_PWR	0x03

#define SET_PWR_OFF			0
#define SET_PWR_PRE_OFF		1
#define SET_PWR_ON			2

#define WATER_STOP			0
#define WATER_SMAL			1
#define WATER_BIG			2
#define WATER_CONT			3
#define WATER_CONT_REL		4	// 持续喷结束

#define MOT_STOP	0		// 结束旋转，并不是停止走动
#define MOT_L_CONT	1
#define MOT_R_CONT	2
#define MOT_L_180	3
#define MOT_R_180	4


#define BT_KEY_PWR      0x01
#define BT_KEY_WATER    0x02
#define BT_KEY_MOT      0x03
#define BT_KEY_READ_MAC 0x20
#define BT_SEND_ACK     0x10
#define BT_SEND_INFO    0x11

#define BT_ACK_OK       0
#define BT_ACK_ERR      1

#define BT_INFO_PWR_OFF 	0
#define BT_INFO_STANDBY 	1
#define BT_INFO_PWR_ON  	2

#define BT_INFO_WATER_STOP 	3
#define BT_INFO_WATER_SMALL 4
#define BT_INFO_WATER_BIG 	5

#define V_PWR_OFF_KEY       1
#define V_PWR_OFF_BATLOW    2
#define V_PWR_OFF_WATFULL   3

void tcp_hand(void);

void SendToSub_Pwr(uint8_t uc_pwr);
void SendToSub_Water(uint8_t uc_water);
void SendToSub_State(uint8_t uc_state);
void SendToSub_Spring(uint8_t uc_spring);
void SendToSub_Speed(uint8_t uc_speed);

void Bt_Send_info(uint8_t uc_info);
void Bt_Send_Ack(uint8_t ack, uint8_t cmd, uint8_t para);
void Bt_Setting(void);

#ifdef __cplusplus
}
#endif

#endif


