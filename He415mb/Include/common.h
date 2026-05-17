#ifndef COMMON_H
#define COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "ciu32l051_std.h"
#include "stdint.h"

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



#define LED1_GPIO_PORT                     GPIOB
#define LED1_PIN                           GPIO_PIN_14
#define LED1_TOGGLE()                      std_gpio_toggle_pin(LED1_GPIO_PORT, LED1_PIN)

#define WDG_ReloadCounter std_iwdg_refresh()

// 定义是根据速度喷水, 还是自己喷水(freerun)
#define WHEEL_DRIVER_BY_MOTOR	0	// 0:手推式(手动的), 喷水速度跟着轮子速度变化; 1: 自动式(自走式), 定时6秒喷一次, 轮子速度定在145Hz	
#define FREERUN_WATER 		WHEEL_DRIVER_BY_MOTOR	


void system_clock_config(void);

void gpio_init(void);

void iwdg_init(void);
void led_init(void);

#ifdef __cplusplus
}
#endif

#endif


