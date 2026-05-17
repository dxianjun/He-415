/************************************************************************************************/
/**
* @file               tim_bsp.h
* @author             MCU Ecosystem Development Team
* @brief              TIM BSP头文件
*                           
*
**************************************************************************************************
* @attention
* Copyright (c) CEC Huada Electronic Design Co.,Ltd. All rights reserved.
*
**************************************************************************************************
*/


/* 避免头文件重复引用 */
#ifndef TIM_BSP_H
#define TIM_BSP_H

#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------------includes--------------------------------------------*/
#include "ciu32l051_std.h"

#include "stdint.h"


/********** 位域（位段）类型定义 **********/
typedef struct {
    unsigned char  b0 : 1;
    unsigned char  b1 : 1;
    unsigned char  b2 : 1;
    unsigned char  b3 : 1;
    unsigned char  b4 : 1;
    unsigned char  b5 : 1;
    unsigned char  b6 : 1;
    unsigned char  b7 : 1;
} bits_t;

typedef union
{
    bits_t        bits;
    unsigned char val;
} bit_field_t;

typedef struct {
    unsigned short  b0 : 1;
    unsigned short  b1 : 1;
    unsigned short  b2 : 1;
    unsigned short  b3 : 1;
    unsigned short  b4 : 1;
    unsigned short  b5 : 1;
    unsigned short  b6 : 1;
    unsigned short  b7 : 1;
    unsigned short  b8 : 1;
    unsigned short  b9 : 1;
    unsigned short  b10 : 1;
    unsigned short  b11 : 1;
    unsigned short  b12 : 1;
    unsigned short  b13 : 1;
    unsigned short  b14 : 1;
    unsigned short  b15 : 1;
} bits16_t;

typedef union
{
    bits16_t   bits;
    unsigned short val;
} bit16_field_t;


#define TNORMAL 	0

#define TTCPIN1		1
#define TTCPIN2		2
#define TTCPIN3		3
#define TTCPIN4		4
#define	TTCPHND		5

#define MAX1MS		6

#define TTPWM_CH1	1
#define TTPWM_CH2	2
#define MAX10MS		3

#define MAX1S		1


extern unsigned short TimOut1mS[MAX1MS]; 
extern unsigned short TimOut10mS[MAX10MS]; 
extern unsigned short TimOut1S[MAX1S]; 

#define TIMOUT_KEYIN_10MS			3
#define TIMOUT_KEYJITTER_10MS		8		// min 8
#define TIMOUT_KEYCP_10MS			20		// min 16
#define TIMOUT_KEYSCP_10MS			200		// min 100
#define TIMOUT_KEYCPH_10MS			100		// min 100


#define TIMOUT_WKOFF_1S			10

extern volatile bit_field_t TimFlg;

#define Tim0_1ms_flg 	TimFlg.bits.b0
#define Tim1ms_flg 		TimFlg.bits.b1
#define Tim10ms_flg 	TimFlg.bits.b2
#define Tim1s_flg 		TimFlg.bits.b3
#define f_cal_ok		TimFlg.bits.b4


void TimFlg_Hand(void);
void delay_clk(uint32_t nclk);
void Delay_125us(uint32_t nTime);
void Delay_1ms(uint32_t nTime);



/*-------------------------------------------functions------------------------------------------*/
void tim3_in_gpio_init(void);
void tim3_in_init(void);
void TIM3_IRQHandler(void);
void bsp_tim3_capture_start(void);
void bsp_rcc_mco_config(void);
void Pwm_in_hand(void);

void tim3_out_gpio_init(void);
void bsp_tim3_output_start(void);
void tim3_out_init(void);    

void tim4_out_gpio_init(void);
void bsp_tim4_output_start(void);
void tim4_out_init(void);   
	
void tim5_out_gpio_init(void);
void bsp_tim5_output_start(void);
void tim5_out_init(void);  

void tim8_init(void);


#ifdef __cplusplus
}
#endif

#endif /* TIM_BSP_H */
