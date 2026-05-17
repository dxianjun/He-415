#include "exti_bsp.h"

uint32_t g_exti_gpio_flag = 0;
volatile uint16_t ui_wheel_lh_cnt = 0;
volatile uint16_t ui_wheel_rh_cnt = 0;
volatile uint8_t g_req_spring = 0;

#define WHEEL_PULSE_CNT 18

void exti_init(void)
{
    std_exti_init_t exti_init_config = {0};
    std_gpio_init_t button_init_config = {0};

    std_rcc_gpio_clk_enable(RCC_PERIPH_CLK_GPIOB);
    std_rcc_gpio_clk_enable(RCC_PERIPH_CLK_GPIOC);
    std_rcc_gpio_clk_enable(RCC_PERIPH_CLK_GPIOD);
    std_rcc_gpio_clk_enable(RCC_PERIPH_CLK_GPIOA);
    std_rcc_gpio_clk_enable(RCC_PERIPH_CLK_GPIOF);

    /* DC检测 PF6: 高电平表示DC插入 */
    button_init_config.pin = GPIO_PIN_6;
    button_init_config.mode = GPIO_MODE_INPUT;
    button_init_config.pull = GPIO_PULLDOWN;
    std_gpio_init(GPIOF, &button_init_config);

    /* 电池低电检测 PA4: 低电平表示低电量 */
    button_init_config.pin = GPIO_PIN_4;
    button_init_config.mode = GPIO_MODE_INPUT;
    button_init_config.pull = GPIO_PULLUP;
    std_gpio_init(GPIOA, &button_init_config);

    /* 低电量通知输出 PB4 */
    button_init_config.pin = GPIO_PIN_4;
    button_init_config.mode = GPIO_MODE_OUTPUT;
    button_init_config.pull = GPIO_NOPULL;
    button_init_config.output_type = GPIO_OUTPUT_PUSHPULL;
    std_gpio_init(GPIOB, &button_init_config);
    std_gpio_reset_pin(GPIOB, GPIO_PIN_4);

    button_init_config.pin = BUTTON_USER_PIN;
    button_init_config.mode = GPIO_MODE_INPUT;
    button_init_config.pull = GPIO_NOPULL;
    std_gpio_init(BUTTON_USER_PORT, &button_init_config);

    exti_init_config.line_id = BUTTON_USER_EXTI_LINE;
    exti_init_config.mode = EXTI_MODE_INTERRUPT;
    exti_init_config.trigger = EXTI_TRIGGER_FALLING;
    exti_init_config.gpio_id = BUTTON_USER_EXTI_PORT;
    std_exti_init(&exti_init_config);

    button_init_config.pin = GPIO_PIN_12;
    button_init_config.pull = GPIO_PULLUP;
    std_gpio_init(GPIOB, &button_init_config);

    exti_init_config.line_id = EXTI_LINE_GPIO_PIN12;
    exti_init_config.gpio_id = EXTI_GPIOB;
    std_exti_init(&exti_init_config);

    button_init_config.pin = GPIO_PIN_14;
    std_gpio_init(GPIOB, &button_init_config);
    exti_init_config.line_id = EXTI_LINE_GPIO_PIN14;
    std_exti_init(&exti_init_config);

    button_init_config.pin = GPIO_PIN_15;
    std_gpio_init(GPIOB, &button_init_config);
    exti_init_config.line_id = EXTI_LINE_GPIO_PIN15;
    std_exti_init(&exti_init_config);

    NVIC_SetPriority(EXTI4_15_IRQn, NVIC_PRIO_3);
    NVIC_EnableIRQ(EXTI4_15_IRQn);

    button_init_config.pin = GPIO_PIN_0;
    std_gpio_init(GPIOD, &button_init_config);

    exti_init_config.line_id = EXTI_LINE_GPIO_PIN0;
    exti_init_config.gpio_id = EXTI_GPIOD;
    std_exti_init(&exti_init_config);

    NVIC_SetPriority(EXTI0_1_IRQn, NVIC_PRIO_3);
    NVIC_EnableIRQ(EXTI0_1_IRQn);
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

    if (std_exti_get_pending_status(EXTI_LINE_GPIO_PIN12))
    {
        std_exti_clear_pending(EXTI_LINE_GPIO_PIN12);
        g_exti_gpio_flag = 1;
    }

    if (std_exti_get_pending_status(EXTI_LINE_GPIO_PIN14))
    {
        std_exti_clear_pending(EXTI_LINE_GPIO_PIN14);
        g_exti_gpio_flag = 1;
        ui_wheel_lh_cnt++;
    }

    if (std_exti_get_pending_status(EXTI_LINE_GPIO_PIN15))
    {
        std_exti_clear_pending(EXTI_LINE_GPIO_PIN15);
        g_exti_gpio_flag = 1;
        ui_wheel_rh_cnt++;
    }

    if (std_exti_get_pending_status(EXTI_LINE_GPIO_PIN0))
    {
        std_exti_clear_pending(EXTI_LINE_GPIO_PIN0);
        g_exti_gpio_flag = 1;
    }

    ui_wheel_min = (ui_wheel_lh_cnt < ui_wheel_rh_cnt) ? ui_wheel_lh_cnt : ui_wheel_rh_cnt;
    ui_wheel_max = (ui_wheel_lh_cnt > ui_wheel_rh_cnt) ? ui_wheel_lh_cnt : ui_wheel_rh_cnt;

    if (ui_wheel_max >= WHEEL_PULSE_CNT)
    {
        if (ui_wheel_max <= (uint16_t)(ui_wheel_min + 2))
        {
            g_req_spring = 1;
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

void Exit_Hand(void)
{
    if (g_exti_gpio_flag == 1)
    {
        LED1_TOGGLE();
        g_exti_gpio_flag = 0;
    }
}
