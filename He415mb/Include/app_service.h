#ifndef APP_SERVICE_H
#define APP_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "tim_bsp.h"

extern bit16_field_t g_app_flag;

#define f_Pwr_On            g_app_flag.bits.b0
#define f_pre_pwr_off       g_app_flag.bits.b1
#define f_bat_pwr_off       g_app_flag.bits.b2
#define f_full_pwr_off      g_app_flag.bits.b3
#define f_Water_Enable      g_app_flag.bits.b4
#define f_Water_Spring      g_app_flag.bits.b5
#define f_Water_Spring_cnt  g_app_flag.bits.b6
#define f_Standby           g_app_flag.bits.b7

void sys_flg_init(void);
void user_serv(void);

void Power_On(void);
void Power_Off(void);
void Enter_Standby(void);
void vi_Power_off(uint8_t uc_flg);

void App_OnMotorCmd(uint8_t dat);

#ifdef __cplusplus
}
#endif

#endif


