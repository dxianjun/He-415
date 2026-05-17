#include <stdio.h>

#include "app_service.h"
#include "app_tcp.h"
#include "exti_bsp.h"
#include "input_bsp.h"
#include "output_bsp.h"

bit16_field_t g_app_flag;

static uint8_t uc_PreOff_cnt = 0;
static uint8_t uc_Bat_Low_cnt = 0;
static uint8_t uc_Dc_In_cnt = 0;
static uint16_t ui_turn_try_set = 0;
static uint16_t ui_turn_try = 0;
static uint8_t f_Dc_In = 0;
static uint8_t f_Turn_wheel = 0;
static uint8_t f_Turn_Cont = 0;

#define V_PWR_OFF_KEY       1
#define V_PWR_OFF_BATLOW    2
#define V_PWR_OFF_WATFULL   3

#define TTMR_HAND   0
#define TWATR_HAND  1
#define TINPUT_HAND 2

#define WATER_HAND_TMO 600
#define ANGLE_180L 170
#define ANGLE_180R 170

#define WHEEL_PWM_FWD       500
#define WHEEL_PWM_TURN_SLOW 375
#define WHEEL_PWM_TURN_FAST 600
#define WHEEL_PWM_TURN_REV  250
#define FAN_PWM_ON          500
#define FAN_PWM_OFF         0
#define BT_INFO_PWR_OFF     0
#define BT_INFO_STANDBY     1
#define BT_INFO_PWR_ON      2

static void wheel_set_forward(void)   { output_wheel_dir_forward(); }
static void wheel_set_turn_left(void) { output_wheel_dir_turn_left(); }
static void wheel_set_turn_right(void){ output_wheel_dir_turn_right(); }

void Power_On(void)
{
    if (f_Pwr_On && !f_Standby) { return; }

    f_Pwr_On = 1;
    f_Standby = 0;
    f_pre_pwr_off = 0;
    f_bat_pwr_off = 0;
    f_full_pwr_off = 0;
    uc_Bat_Low_cnt = 0;
    uc_PreOff_cnt = 0;

    output_info_set(0);
    output_bat_en_set(1);
    output_pwr_lock_set(1);

    wheel_set_forward();
    output_wheel_pwm_set(WHEEL_PWM_FWD, WHEEL_PWM_FWD);
    output_fan_pwm_set(FAN_PWM_ON);

    SendToSub_Pwr(2);
    Delay_1ms(10);
    SendToSub_Pwr(2);
    Bt_Send_info(BT_INFO_PWR_ON);

    TimOut10mS[TWATR_HAND] = 0;
    printf("power on\r\n");
}

void Power_Off(void)
{
    f_Pwr_On = 0;
    f_Standby = 0;
    f_pre_pwr_off = 0;
    f_bat_pwr_off = 0;
    f_full_pwr_off = 0;
    uc_PreOff_cnt = 0;
    f_Water_Spring = 0;
    f_Water_Spring_cnt = 0;

    output_bat_en_set(0);
    output_pwr_lock_set(0);
    output_fan_pwm_set(FAN_PWM_OFF);

    SendToSub_Spring(0);
    Delay_1ms(10);
    SendToSub_Pwr(0);
    Delay_1ms(10);
    SendToSub_Pwr(0);
    Bt_Send_info(BT_INFO_PWR_OFF);

    printf("power off\r\n");
}

void Enter_Standby(void)
{
    if (!f_Pwr_On) { return; }

    f_Standby = 1;
    f_pre_pwr_off = 0;
    f_bat_pwr_off = 0;
    f_full_pwr_off = 0;
    f_Water_Spring = 0;
    f_Water_Spring_cnt = 0;

    
    wheel_set_forward();
    output_wheel_pwm_set(0, 0);
    output_fan_pwm_set(FAN_PWM_OFF);

    SendToSub_Spring(0);
    Delay_1ms(10);
    SendToSub_Water(0);
    Delay_1ms(10);
    SendToSub_Pwr(1);
    Bt_Send_info(BT_INFO_STANDBY);

    printf("enter standby\r\n");
}

void vi_Power_off(uint8_t uc_flg)
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
    uc_PreOff_cnt = 0;
}

void App_OnMotorCmd(uint8_t dat)
{
    if (!f_Pwr_On || f_Standby) { return; }

    if (dat == 0)
    {
        f_Turn_Cont = 0;
        f_Turn_wheel = 0;
        wheel_set_forward();
        output_wheel_pwm_set(WHEEL_PWM_FWD, WHEEL_PWM_FWD);
    }
    else if (dat == 1)
    {
        f_Turn_Cont = 1;
        f_Turn_wheel = 1;
        ui_turn_try = 0;
        wheel_set_turn_left();
        output_wheel_pwm_set(WHEEL_PWM_TURN_FAST, WHEEL_PWM_TURN_SLOW);
    }
    else if (dat == 2)
    {
        f_Turn_Cont = 1;
        f_Turn_wheel = 1;
        ui_turn_try = 0;
        wheel_set_turn_right();
        output_wheel_pwm_set(WHEEL_PWM_TURN_SLOW, WHEEL_PWM_TURN_FAST);
    }
    else if (dat == 3)
    {
        f_Turn_Cont = 0;
        f_Turn_wheel = 1;
        ui_turn_try_set = ANGLE_180L;
        ui_turn_try = 0;
        wheel_set_turn_left();
        output_wheel_pwm_set(WHEEL_PWM_TURN_FAST, WHEEL_PWM_TURN_REV);
    }
    else if (dat == 4)
    {
        f_Turn_Cont = 0;
        f_Turn_wheel = 1;
        ui_turn_try_set = ANGLE_180R;
        ui_turn_try = 0;
        wheel_set_turn_right();
        output_wheel_pwm_set(WHEEL_PWM_TURN_REV, WHEEL_PWM_TURN_FAST);
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
                Power_Off();
            }
        }
    }
    else
    {
        if (uc_Dc_In_cnt > 0) { uc_Dc_In_cnt--; }
        else { f_Dc_In = 0; }
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
            uc_Bat_Low_cnt--;
            if (uc_Bat_Low_cnt == 0)
            {
                output_info_set(0);
            }
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
            uc_PreOff_cnt++;

            if (f_pre_pwr_off && uc_PreOff_cnt >= 3) { Power_Off(); }
            else if (f_bat_pwr_off && uc_PreOff_cnt >= 5) { Power_Off(); }
            else if (f_full_pwr_off && uc_PreOff_cnt >= 3) { Power_Off(); }
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
            if (f_Water_Spring_cnt == 0)
            {
                f_Water_Spring = 1;
                SendToSub_Spring(1);
            }
        }
    }
}

void user_serv(void)
{
    Input_Hand();
    tcp_hand();
    Tmr_Hand();
    Turn_Tmo_Hand();
  //  Exit_Hand();
}



