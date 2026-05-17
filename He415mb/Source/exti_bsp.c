#include "exti_bsp.h"

uint32_t g_exti_gpio_flag = 0;
volatile uint16_t ui_wheel_lh_cnt = 0;
volatile uint16_t ui_wheel_rh_cnt = 0;

volatile uint8_t g_wheel_auto_mode = 0;		// 1: auto, 0:manual

#define WHEEL_PULSE_CNT 18

void exti_gpio_init(void)
{
    std_gpio_init_t button_init_config = {0};
	
    std_rcc_gpio_clk_enable(RCC_PERIPH_CLK_GPIOB);
    std_rcc_gpio_clk_enable(RCC_PERIPH_CLK_GPIOC);
    std_rcc_gpio_clk_enable(RCC_PERIPH_CLK_GPIOD);
    std_rcc_gpio_clk_enable(RCC_PERIPH_CLK_GPIOA);

    button_init_config.pin = BUTTON_USER_PIN;
    button_init_config.mode = GPIO_MODE_INPUT;
    button_init_config.pull = GPIO_NOPULL;
    std_gpio_init(BUTTON_USER_PORT, &button_init_config);

    /* PB12/PB14/PB15/PD0: wheel speed inputs (IPU + EXTI) */
    button_init_config.pin = GPIO_PIN_12;
    button_init_config.pull = GPIO_PULLUP;
    std_gpio_init(GPIOB, &button_init_config);

    button_init_config.pin = GPIO_PIN_14;
    std_gpio_init(GPIOB, &button_init_config);

    button_init_config.pin = GPIO_PIN_15;
    std_gpio_init(GPIOB, &button_init_config);

    button_init_config.pin = GPIO_PIN_0;
    std_gpio_init(GPIOD, &button_init_config);

}

void exti_init_config(void)
{
	std_exti_init_t exti_init_config = {0};
	
    exti_init_config.line_id = BUTTON_USER_EXTI_LINE;
    exti_init_config.mode = EXTI_MODE_INTERRUPT;
    exti_init_config.trigger = EXTI_TRIGGER_FALLING;
    exti_init_config.gpio_id = BUTTON_USER_EXTI_PORT;
    std_exti_init(&exti_init_config);

    exti_init_config.line_id = EXTI_LINE_GPIO_PIN12;
    exti_init_config.gpio_id = EXTI_GPIOB;
    std_exti_init(&exti_init_config);

    exti_init_config.line_id = EXTI_LINE_GPIO_PIN14;
    std_exti_init(&exti_init_config);

    exti_init_config.line_id = EXTI_LINE_GPIO_PIN15;
    std_exti_init(&exti_init_config);	

    NVIC_SetPriority(EXTI4_15_IRQn, NVIC_PRIO_3);
    NVIC_EnableIRQ(EXTI4_15_IRQn);
	
    exti_init_config.line_id = EXTI_LINE_GPIO_PIN0;
    exti_init_config.gpio_id = EXTI_GPIOD;
    std_exti_init(&exti_init_config);

    NVIC_SetPriority(EXTI0_1_IRQn, NVIC_PRIO_3);
    NVIC_EnableIRQ(EXTI0_1_IRQn);

}

void exti_init(void)
{
	exti_gpio_init();
	exti_init_config();
}

void EXTI4_15_IRQHandler(void)
{
    uint16_t ui_wheel_min;
    uint16_t ui_wheel_max;

    if (std_exti_get_pending_status(EXTI_LINE_GPIO_PIN13))
    {
        std_exti_clear_pending(EXTI_LINE_GPIO_PIN13);
        g_exti_gpio_flag = 1;
    }
	
    /* left wheel manual: PB15; left wheel auto: PB12
       right wheel manual: PD0; right wheel auto: PB14 */

	#if WHEEL_DRIVER_BY_MOTOR
    if (std_exti_get_pending_status(EXTI_LINE_GPIO_PIN15))
    {
        std_exti_clear_pending(EXTI_LINE_GPIO_PIN15);
        if (g_wheel_auto_mode == 0)
        {
            ui_wheel_lh_cnt++;
        }
    }

    if (std_exti_get_pending_status(EXTI_LINE_GPIO_PIN0))
    {
        std_exti_clear_pending(EXTI_LINE_GPIO_PIN0);
        if (g_wheel_auto_mode == 0)
        {
            ui_wheel_rh_cnt++;
        }
    }
	#else
    if (std_exti_get_pending_status(EXTI_LINE_GPIO_PIN14))
    {
        std_exti_clear_pending(EXTI_LINE_GPIO_PIN14);
        if (g_wheel_auto_mode != 0)
        {
            ui_wheel_rh_cnt++;
        }
    }


    if (std_exti_get_pending_status(EXTI_LINE_GPIO_PIN12))
    {
        std_exti_clear_pending(EXTI_LINE_GPIO_PIN12);
        if (g_wheel_auto_mode != 0)
        {
            ui_wheel_lh_cnt++;
        }
    }
	#endif
	
    ui_wheel_min = (ui_wheel_lh_cnt < ui_wheel_rh_cnt) ? ui_wheel_lh_cnt : ui_wheel_rh_cnt;
    ui_wheel_max = (ui_wheel_lh_cnt > ui_wheel_rh_cnt) ? ui_wheel_lh_cnt : ui_wheel_rh_cnt;

    if (ui_wheel_max >= WHEEL_PULSE_CNT)
    {
        if (ui_wheel_max <= (uint16_t)(ui_wheel_min + 2))
        {
            f_req_spring = 1;
        }
        ui_wheel_lh_cnt = 0;
        ui_wheel_rh_cnt = 0;
    }
}

void led_init(void)
{
    std_gpio_init_t led_gpio_init = {0};

    std_rcc_gpio_clk_enable(RCC_PERIPH_CLK_GPIOB);

    led_gpio_init.pin = LED1_PIN;
    led_gpio_init.mode = GPIO_MODE_OUTPUT;
    led_gpio_init.pull = GPIO_PULLUP;
    led_gpio_init.output_type = GPIO_OUTPUT_PUSHPULL;
    std_gpio_init(LED1_GPIO_PORT, &led_gpio_init);
}

#if 0
void Exit_Hand(void)
{
    if (g_exti_gpio_flag == 1)
    {
        LED1_TOGGLE();
        g_exti_gpio_flag = 0;
    }
}
#endif


