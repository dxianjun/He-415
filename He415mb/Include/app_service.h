#ifndef APP_SERVICE_H
#define APP_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "tim_bsp.h"

extern bit16_field_t g_app_flag;

#define f_Dc_In				g_app_flag.bits.b0			// 默认值0: DC电源插入的标记;  1: 表示Dc电源插入了;
#define f_Pwr_On			g_app_flag.bits.b1			// 默认值0: 开关机标记, 1: 表示现在已经开机了;
#define f_Water_Full		g_app_flag.bits.b2			// 默认值0: 污水已满的标记;        	1: 表示污水满了;

#define f_Water_Enable      g_app_flag.bits.b4			// 默认值为1: 允许喷水
#define f_Water_Spring      g_app_flag.bits.b5				// 默认值0: 正在喷水的标记;        	1: 表示正在喷水;
#define f_Water_Spring_cnt  g_app_flag.bits.b6				// 默认值0: 正在持续喷水的标记;        	1: 表示正在持续喷水;

#define f_Turn_wheel			g_app_flag.bits.b8			// 默认值0: 正在转方向的标记			1: 正在转向
#define f_Turn_Cont				g_app_flag.bits.b9			// 默认值0: 持续转动的标记			1: 持续转动, adj按键时标记

#define f_pre_pwr_off			g_app_flag.bits.b10			// 默认值0: 预关机的标记, 按键关机时, 要求吸水电机继续工作, 等5秒后, 才能彻底断电
#define f_bat_pwr_off			g_app_flag.bits.b11			// 默认值0: bat信号关电机的标记, Bat低电量后, 要求工作状态通水满一样
#define f_full_pwr_off			g_app_flag.bits.b12			// 默认值0: 污水信号关电机的标记, 污水满后, 要求工作假关机, 闪灯不一样


void sys_flg_init(void);
void user_serv(void);

void Power_On(void);
void Power_Off(void);
void Enter_Standby(void);
void vi_Power_off(uint8_t uc_flg);
void vi_Full_warn(uint8_t uc_flg);

void App_OnMotorCmd(uint8_t dat);
void Turn_Tmo_Hand(void);

#ifdef __cplusplus
}
#endif

#endif


