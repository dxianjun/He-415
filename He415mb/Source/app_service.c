#include <stdio.h>

#include "app_service.h"
#include "app_tcp.h"
#include "exti_bsp.h"
#include "input_bsp.h"
#include "output_bsp.h"

bit16_field_t g_app_flag;
volatile bit_field_t  vf_sys_flag;

static uint8_t uc_PreOff_cnt = 0;
static uint8_t uc_Bat_Low_cnt = 0;
static uint8_t uc_Dc_In_cnt = 0;
static uint16_t ui_turn_try_set = 0;
static uint16_t ui_turn_try = 0;

#define WATER_HAND_TMO 600
#define ANGLE_180L 170
#define ANGLE_180R 170

#define WHEEL_PWM_FWD       900
#define WHEEL_PWM_TURN_SLOW 375
#define WHEEL_PWM_TURN_FAST 600

#define FAN_PWM_ON          220		// 吸水电机 220;
#define FAN_PWM_OFF         0

volatile uint16_t ui_wheel_l_pwm=WHEEL_PWM_FWD, ui_wheel_r_pwm=WHEEL_PWM_FWD;

static void wheel_set_forward(void)   { output_wheel_dir_forward(); }
static void wheel_set_turn_left(void) { output_wheel_dir_turn_left(); }
static void wheel_set_turn_right(void){ output_wheel_dir_turn_right(); }

void Power_On(void)
{
    if (f_Pwr_On) { return; }
	if (f_Dc_In||input_dc_in_is_active()) return;
	
    f_Pwr_On = 1;
    f_pre_pwr_off = 0;
    f_bat_pwr_off = 0;
    f_full_pwr_off = 0;
	f_Water_Full = 0; 
	
    uc_Bat_Low_cnt = 0;
    uc_PreOff_cnt = 0;

    output_info_set(0);
    output_bat_en_set(1);
    output_pwr_lock_set(1);

    wheel_set_forward();
	ui_wheel_l_pwm = WHEEL_PWM_FWD;
	ui_wheel_r_pwm = WHEEL_PWM_FWD;
	
    output_wheel_pwm_set(WHEEL_PWM_FWD, WHEEL_PWM_FWD);
    output_fan_pwm_set(FAN_PWM_ON);

	// 通知电机板开机
    SendToSub_Pwr(SET_PWR_ON);
    Delay_1ms(10);
    SendToSub_Pwr(SET_PWR_ON);

	// 通知蓝牙模块
    Bt_Send_info(BT_INFO_PWR_ON);

    TimOut10mS[TWATR_HAND] = 0;
	f_Water_Spring = 0;
    printf("power on\r\n");
}

void Power_Off(void)
{
    f_Pwr_On = 0;
    f_pre_pwr_off = 0;
	
    // 充电时, 充电板一直处于工作状态(放电状态/同时充放的状态); 如果逻辑有漏洞, 要求结构修改, 单独拉P10出来;
	if (f_bat_pwr_off)
		{
		f_bat_pwr_off = 0;
		}
	else if (f_full_pwr_off)
		{
		f_full_pwr_off = 0;
		}
	else
		{
		output_bat_en_set(0);
		}
	
    uc_PreOff_cnt = 0;
    f_Water_Spring = 0;
    f_Water_Spring_cnt = 0;

	// 关机锁定脚解锁
    output_pwr_lock_set(0);
	// 电机彻底停下来
	ui_wheel_l_pwm = 0; ui_wheel_r_pwm = 0;
	output_wheel_pwm_set(ui_wheel_l_pwm, ui_wheel_r_pwm);
	
    output_fan_pwm_set(FAN_PWM_OFF);

// 	 通知电机板关机，就包含了通知喷水电机停止的功能
//    SendToSub_Spring(0);
//    Delay_1ms(10);
    SendToSub_Pwr(SET_PWR_OFF);
    Delay_1ms(10);
    SendToSub_Pwr(SET_PWR_OFF);
	
    Bt_Send_info(BT_INFO_PWR_OFF);

    printf("power off\r\n");
}

// davidd 20260129, 水满只在motor板处理灯闪，不在主板这边关机；
void vi_Full_warn(uint8_t uc_flg)
{
	if (V_PWR_OFF_WATFULL == uc_flg)
		{

		}
}

void vi_Power_off(uint8_t uc_flg)
{
    f_Pwr_On = 0;
    uc_PreOff_cnt = 0;
	
	// davidd 20260114, 增加按键关机清0，解决关机后再来，计数累加的问题；
	ui_wheel_lh_cnt = 0;
	ui_wheel_rh_cnt = 0;

	// 速度降下来
	ui_wheel_l_pwm = 300; ui_wheel_r_pwm = 300;
	output_wheel_pwm_set(ui_wheel_l_pwm, ui_wheel_r_pwm);

	TimOut10mS[TTMR_HAND] = 0;
	uc_PreOff_cnt = 0;	// 假关机的时间计数清0
	
	if (uc_flg == V_PWR_OFF_KEY)
		{
		f_pre_pwr_off = 1;
		f_bat_pwr_off = 0;
		f_full_pwr_off = 0;

		// 污水满时，按下关机键，不用等，直接关机
		if (f_Water_Full)
			{
			f_Water_Full = 0;
			Power_Off();
			}
		else
			{
			SendToSub_State(SS_VK_OFF);
			}
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
	uc_PreOff_cnt = 0;

	vf_sys_flag.val = 0;
}

void App_OnMotorCmd(uint8_t dat)
{
    if (!f_Pwr_On) { return; }

	switch (dat)
		{
		default: break;

		// 这里的停止是停止转弯，不是真的停止电机
		case MOT_STOP:
			{
			f_Turn_Cont = 0;
	        f_Turn_wheel = 0;
	        wheel_set_forward();
	        output_wheel_pwm_set(WHEEL_PWM_FWD, WHEEL_PWM_FWD);
			}
			break;
		
		case MOT_L_CONT:
			{
			f_Turn_Cont = 1;
	        f_Turn_wheel = 1;
	        ui_turn_try = 0;
	        wheel_set_forward();	// 左边速度降下来
	        output_wheel_pwm_set(WHEEL_PWM_TURN_SLOW, ui_wheel_r_pwm);
			}
			break;

		case MOT_R_CONT:
			{
			f_Turn_Cont = 1;
	        f_Turn_wheel = 1;
	        ui_turn_try = 0;
	        wheel_set_forward();	// 右边速度降下来
	        output_wheel_pwm_set(ui_wheel_l_pwm, WHEEL_PWM_TURN_SLOW);
			}
			break;

		case MOT_L_180:
			{
			f_Turn_Cont = 0;
	        f_Turn_wheel = 1;
	        ui_turn_try_set = ANGLE_180L;
	        ui_turn_try = 0;
	        wheel_set_turn_left();
			// output_wheel_pwm_set(WHEEL_PWM_TURN_FAST, WHEEL_PWM_TURN_REV);
			}
			break;

		case MOT_R_180:
			{
			f_Turn_Cont = 0;
	        f_Turn_wheel = 1;
	        ui_turn_try_set = ANGLE_180R;
	        ui_turn_try = 0;
	        wheel_set_turn_right();
	        // output_wheel_pwm_set(WHEEL_PWM_TURN_REV, WHEEL_PWM_TURN_FAST);
			}
			break;
		}
}

void Turn_Tmo_Hand(void)
{
	if (f_Turn_wheel && (f_Turn_Cont == 0))
		{
		ui_turn_try++;
		if (ui_turn_try >= ui_turn_try_set)
			{
			f_Turn_wheel = 0;
			wheel_set_forward();
			output_wheel_pwm_set(WHEEL_PWM_FWD, WHEEL_PWM_FWD);
			}
		}
}

static void Input_Hand(void)
{
	if (TimOut10mS[TINPUT_HAND] < 5) { return; }
	TimOut10mS[TINPUT_HAND] = 0;

	if (input_dc_in_is_active())
		{
		uc_Dc_In_cnt++;
		if (uc_Dc_In_cnt >= 2)
			{
			uc_Dc_In_cnt = 2;
			if (!f_Dc_In)
				{
				f_Dc_In = 1;
				uc_Bat_Low_cnt = 0;

				output_info_set(0);
				printf("Dc In stop\n");
				Power_Off();
				}
			}
		}
	else
		{
		if (uc_Dc_In_cnt > 0) 
			{ uc_Dc_In_cnt--; }
		else 
			{
			if (f_Dc_In)
				{
				f_Dc_In = 0; 
				printf("Dc off\n");
				}
			}
		}

	if (!f_Dc_In)
		{
		if (input_bat_low_is_active())
			{
			if (uc_Bat_Low_cnt < 201) { uc_Bat_Low_cnt++; }
			if (uc_Bat_Low_cnt == 200)
				{
				output_info_set(1);
				vi_Power_off(V_PWR_OFF_BATLOW);
				}
			}
		else if (uc_Bat_Low_cnt > 0)
			{
			if (uc_Bat_Low_cnt > 50)		// 1秒
				{
				uc_Bat_Low_cnt = 50;
				}
			else
				{
				uc_Bat_Low_cnt--;
				if (uc_Bat_Low_cnt == 0)
					{
					// output_info_set(0);
					}
				}
			}
		}
}

static void Tmr_Hand(void)
{
	// 关机和插入电源时, 不执行定时操作

	if (f_Dc_In||input_dc_in_is_active()) return;

	if (!f_Pwr_On)
		{
		if (TimOut10mS[TTMR_HAND] >= 100)
			{
			TimOut10mS[TTMR_HAND] = 0;
			uc_PreOff_cnt++;

			if (f_pre_pwr_off && uc_PreOff_cnt >= 3) { Power_Off(); }
			else if (f_bat_pwr_off && uc_PreOff_cnt >= 5) { Power_Off(); }
			else if (f_full_pwr_off && uc_PreOff_cnt >= 3) { Power_Off(); }
			}
		
		return;
		}

	if (f_Pwr_On && f_Water_Enable)
		{
		// 主板发喷水消息，停止喷水在电机板定时
		#if (FREERUN_WATER==1)	
		if (TimOut10mS[TWATR_HAND] >= WATER_HAND_TMO)
			{
			TimOut10mS[TWATR_HAND] = 0;
			if (f_Water_Spring_cnt == 0)
				{
				f_Water_Spring = 1;
				SendToSub_Spring(1);
				}
			}
		#else
		if (f_req_spring)
			{
			f_req_spring = 0;
			TimOut10mS[TWATR_HAND] = 0;
			if (f_Water_Spring_cnt == 0)
				{
				f_Water_Spring = 1;
				SendToSub_Spring(1);
				}
			}
		#endif
		}
	else
		{
		if (f_Water_Spring)
			{
			f_Water_Spring = 0;
			SendToSub_Spring(0); 	
			}
		}
}

void user_serv(void)
{
	WDG_ReloadCounter;
    Input_Hand();

	WDG_ReloadCounter;
    Tmr_Hand();
	

	WDG_ReloadCounter;
    tcp_hand();


	WDG_ReloadCounter;
    
  //  Exit_Hand();
}



