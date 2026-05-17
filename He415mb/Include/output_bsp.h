#ifndef BSP_OUTPUT_H
#define BSP_OUTPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ciu32l051_std.h"

typedef enum
{
    BSP_WHEEL_DIR_FORWARD = 0,
    BSP_WHEEL_DIR_BACKWARD
} bsp_wheel_dir_t;

void output_gpio_init(void);
void output_info_set(uint8_t active);
void output_bat_en_set(uint8_t active);
void output_pwr_lock_set(uint8_t active);
void output_left_wheel_dir_set(bsp_wheel_dir_t dir);
void output_right_wheel_dir_set(bsp_wheel_dir_t dir);
void output_wheel_dir_forward(void);
void output_wheel_dir_turn_left(void);
void output_wheel_dir_turn_right(void);
void output_wheel_pwm_set(uint16_t left_pwm, uint16_t right_pwm);
void output_fan_pwm_set(uint16_t pwm);

#ifdef __cplusplus
}
#endif

#endif



