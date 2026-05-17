#ifndef EXTI_BSP_H
#define EXTI_BSP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ciu32l051_std.h"

extern uint32_t g_exti_gpio_flag;
extern volatile uint16_t ui_wheel_lh_cnt;
extern volatile uint16_t ui_wheel_rh_cnt;
extern volatile uint8_t g_req_spring;
extern volatile uint8_t g_wheel_auto_mode;

#define LED1_GPIO_PORT            GPIOB
#define LED1_PIN                  GPIO_PIN_14
#define LED1_TOGGLE()             std_gpio_toggle_pin(LED1_GPIO_PORT, LED1_PIN)

#define BUTTON_USER_PORT          GPIOC
#define BUTTON_USER_PIN           GPIO_PIN_13

#define BUTTON_USER_EXTI_PORT     EXTI_GPIOC
#define BUTTON_USER_EXTI_LINE     EXTI_LINE_GPIO_PIN13

void led_init(void);
void exti_init(void);
void EXTI4_15_IRQHandler(void);
void Exit_Hand(void);

#ifdef __cplusplus
}
#endif

#endif
