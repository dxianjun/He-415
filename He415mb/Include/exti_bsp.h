#ifndef EXTI_BSP_H
#define EXTI_BSP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ciu32l051_std.h"
#include "tim_bsp.h"

//extern uint32_t g_exti_gpio_flag;
extern volatile uint16_t ui_wheel_lh_cnt;
extern volatile uint16_t ui_wheel_rh_cnt;

//extern volatile uint8_t g_wheel_auto_mode;


#define LED1_GPIO_PORT            GPIOB
#define LED1_PIN                  GPIO_PIN_14
#define LED1_TOGGLE()             std_gpio_toggle_pin(LED1_GPIO_PORT, LED1_PIN)

#define BUTTON_USER_PORT          GPIOC
#define BUTTON_USER_PIN           GPIO_PIN_13

#define BUTTON_USER_EXTI_PORT     EXTI_GPIOC
#define BUTTON_USER_EXTI_LINE     EXTI_LINE_GPIO_PIN13

#if 1	// davidd 20250507 轮子改成90%, 对应285Hz
// 285Hz, 
// #define WHEEL_FB_FREQ	570	// 285*2 = 570
#define WHEEL_FB_FREQ_OVRR 571  // WHEEL_FB_FREQ+1
#define WHEEL_FB_FREQ_LOW  569	// WHEEL_FB_FREQ-1
#else	
// 145Hz对应290
#define WHEEL_FB_FREQ	290		// 145*2 = 290
#define WHEEL_FB_FREQ_OVRR 291  // WHEEL_FB_FREQ+1
#define WHEEL_FB_FREQ_LOW  289	// WHEEL_FB_FREQ-1
#endif


void led_init(void);
void exti_init(void);
void EXTI4_15_IRQHandler(void);
void Exit_Hand(void);

#ifdef __cplusplus
}
#endif

#endif


