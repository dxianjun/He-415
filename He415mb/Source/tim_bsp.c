/************************************************************************************************/
/**
* @file               tim_bsp.c
* @author             MCU Ecosystem Development Team
* @brief              Comment text fixed.
*                           
*
**************************************************************************************************
* @attention
* Copyright (c) CEC Huada Electronic Design Co.,Ltd. All rights reserved.
*
**************************************************************************************************
*/


/*------------------------------------------includes--------------------------------------------*/
#include "tim_bsp.h"
#include "common.h"

/*------------------------------------------variables-------------------------------------------*/
uint32_t g_enter_cnt = 0U;

uint32_t g_ch1_ccx_value = 0U;
uint32_t g_ch2_ccx_value = 0U;

uint32_t g_pwm_duty = 0x0U;
uint32_t g_pwm_frequency = 0x0U;

/*-------------------------------------------functions------------------------------------------*/
#if 0
#define TIM3_ARR_VALUE        (0xFFFFU)		// pwm in 

void tim3_in_gpio_init(void)
{
    std_gpio_init_t tim3_gpio_init = {0};
    
    
    std_rcc_gpio_clk_enable(RCC_PERIPH_CLK_GPIOB);
    
    /* TIM3 input capture test pin:
       PB4 -> TIM3_CH1 */
    tim3_gpio_init.pin = GPIO_PIN_4;
    tim3_gpio_init.mode = GPIO_MODE_ALTERNATE;
    tim3_gpio_init.pull = GPIO_PULLDOWN;
    tim3_gpio_init.alternate = GPIO_AF2_TIM3;
    std_gpio_init(GPIOB, &tim3_gpio_init); 
}


/**
* @brief              Comment text fixed.
* @retval             value
*/
void tim3_in_init(void)
{	
    std_tim_basic_init_t basic_init_struct = {0};
    std_tim_input_capture_init_t input_capture_struct = {0};
    
    
    std_rcc_apb1_clk_enable(RCC_PERIPH_CLK_TIM3);
    
    
    basic_init_struct.counter_mode = TIM_COUNTER_MODE_UP;
    basic_init_struct.period = TIM3_ARR_VALUE;
    basic_init_struct.clock_div = TIM_CLOCK_DTS_DIV1;
    basic_init_struct.auto_reload_preload = TIM_AUTORELOAD_PRE_DISABLE;
    std_tim_init(TIM3, &basic_init_struct);
    
    
    std_tim_slave_mode_config(TIM3, TIM_SLAVE_MODE_RESET);
    std_tim_trig_source_config(TIM3, TIM_TRIG_SOURCE_TI1FP1);
    std_tim_set_input_pol(TIM3, TIM_CHANNEL_1, TIM_INPUT_POL_RISING);
    
    
    input_capture_struct.input_capture_pol = TIM_INPUT_POL_RISING;
    input_capture_struct.input_capture_sel = TIM_INPUT_CAPTURE_SEL_DIRECTTI;
    input_capture_struct.input_capture_prescaler = TIM_INPUT_CAPTURE_PSC_DIV1;
    std_tim_input_capture_init(TIM3, &input_capture_struct, TIM_CHANNEL_1);
    
    
    input_capture_struct.input_capture_pol = TIM_INPUT_POL_FALLING;
    input_capture_struct.input_capture_sel = TIM_INPUT_CAPTURE_SEL_INDIRECTTI;
    std_tim_input_capture_init(TIM3, &input_capture_struct, TIM_CHANNEL_2);
		
    
    NVIC_SetPriority(TIM3_IRQn, NVIC_PRIO_0);
    NVIC_EnableIRQ(TIM3_IRQn);
}

uint16_t period_cnt1=0, period_cnt2=0;
uint16_t duty_cnt1=0, duty_cnt2=0;

uint8_t update_flag1=0, update_flag2=0, cc1_valid=0, cc2_valid=0;


/**
* @brief              Comment text fixed.
* @retval             value
*/
void TIM3_IRQHandler(void)
{
    
    if ((std_tim_get_flag(TIM3, TIM_FLAG_CC1)) && (std_tim_get_interrupt_enable(TIM3, TIM_INTERRUPT_CC1)))
    {
        
        if (g_enter_cnt == 0)
        {
            g_enter_cnt = 1;
            std_tim_get_ccx_value(TIM3, TIM_CHANNEL_1);
        }
        
        
        else if (g_enter_cnt == 1)
        {
        
        // std_tim_interrupt_disable(TIM3, TIM_INTERRUPT_CC1);
        g_ch1_ccx_value = std_tim_get_ccx_value(TIM3, TIM_CHANNEL_1);
		std_tim_clear_flag(TIM3, TIM_FLAG_CC1);
						
		update_flag1 = 1;
		// std_tim_interrupt_enable(TIM3, TIM_INTERRUPT_CC1 | TIM_INTERRUPT_CC2);
        }
    }
    
    
    if ((std_tim_get_flag(TIM3, TIM_FLAG_CC2)) && (std_tim_get_interrupt_enable(TIM3, TIM_INTERRUPT_CC2)))
    {
        
        // std_tim_interrupt_disable(TIM3, TIM_INTERRUPT_CC2);
        g_ch2_ccx_value = std_tim_get_ccx_value(TIM3, TIM_CHANNEL_2);
        std_tim_clear_flag(TIM3, TIM_FLAG_CC2);
    }    
}

/**
* @brief              Comment text fixed.
* @retval             value
*/
void bsp_tim3_capture_start(void)
{
    
    std_tim_interrupt_enable(TIM3, TIM_INTERRUPT_CC1 | TIM_INTERRUPT_CC2);
    
    
    std_tim_ccx_channel_enable(TIM3, TIM_CHANNEL_1);
    std_tim_ccx_channel_enable(TIM3, TIM_CHANNEL_2);
    
    
    std_tim_enable(TIM3);
}

/**
* @brief              Comment text fixed.
* @retval             value
*/
void bsp_rcc_mco_config(void)
{
    std_gpio_init_t mco_gpio_cfg = {0};
    
    
    std_rcc_gpio_clk_enable(RCC_PERIPH_CLK_GPIOA);
    
    mco_gpio_cfg.pin = GPIO_PIN_8;
    mco_gpio_cfg.mode = GPIO_MODE_ALTERNATE;
    mco_gpio_cfg.alternate = GPIO_AF0_MCO;
    mco_gpio_cfg.pull = GPIO_NOPULL;
    
    std_gpio_init(GPIOA, &mco_gpio_cfg);
    
    std_rcc_mco_config(RCC_MCO_SRC_RCH, RCC_MCO_DIV16);
}


void Pwm_in_hand(void)
{
	 if (update_flag1)
		{
		update_flag1=0;

		
		g_pwm_duty = ((g_ch2_ccx_value + 1) * 100) / (g_ch1_ccx_value + 1);
		
		
		g_pwm_frequency = (std_rcc_get_pclk1freq() / (g_ch1_ccx_value + 1));

		printf("Freq=%ld, duty=%d\r\n", g_pwm_frequency, g_pwm_duty);
		}

}
#endif
/*-----------------------------------------------------------------------------------------------------------*/
// out
/*--------------------------------------------define--------------------------------------------*/
// 48K
#define TIM_PERIOD_VALUE        1000	//(0xFFFFU)
#define TIM_PRESCALER_VALUE     (0x00U)
#define TIM_PULSE1_VALUE        (uint32_t)(TIM_PERIOD_VALUE/2)
#define TIM_PULSE2_VALUE        (uint32_t)(TIM_PERIOD_VALUE*37.5/100)
#define TIM_PULSE3_VALUE        (uint32_t)(TIM_PERIOD_VALUE/4) 
#define TIM_PULSE4_VALUE        (uint32_t)(TIM_PERIOD_VALUE*12.5/100)

#if 1
/**
* @brief              Comment text fixed.
* @retval             value
*/
void tim3_out_gpio_init(void)
{
    std_gpio_init_t tim3_gpio_init = {0};
    
    
    std_rcc_gpio_clk_enable(RCC_PERIPH_CLK_GPIOB);
    std_rcc_gpio_clk_enable(RCC_PERIPH_CLK_GPIOC);
        
    /* TIM3 output test pins:
       PB4 -> TIM3_CH1
       PB5 -> TIM3_CH2
       PC7 -> TIM3_CH3
       PC8 -> TIM3_CH4 */
    tim3_gpio_init.pin = GPIO_PIN_4 | GPIO_PIN_5;
    tim3_gpio_init.mode = GPIO_MODE_ALTERNATE;
    tim3_gpio_init.pull = GPIO_NOPULL;
    tim3_gpio_init.alternate = GPIO_AF2_TIM3;
    std_gpio_init(GPIOB, &tim3_gpio_init); 

    tim3_gpio_init.pin = GPIO_PIN_7 | GPIO_PIN_8;
    tim3_gpio_init.mode = GPIO_MODE_ALTERNATE;
    tim3_gpio_init.pull = GPIO_NOPULL;
    tim3_gpio_init.alternate = GPIO_AF1_TIM3;
    std_gpio_init(GPIOC, &tim3_gpio_init);     
}


/**
* @brief              Comment text fixed.
* @retval             value
*/
void tim3_out_init(void)
{
    std_tim_basic_init_t basic_init_struct = {0};
    std_tim_output_compare_init_t oc_config_struct = {0};
    
    
    std_rcc_apb1_clk_enable(RCC_PERIPH_CLK_TIM3);
    
    
    basic_init_struct.prescaler = TIM_PRESCALER_VALUE;
    basic_init_struct.counter_mode = TIM_COUNTER_MODE_UP;
    basic_init_struct.period = TIM_PERIOD_VALUE-1;
    basic_init_struct.clock_div = TIM_CLOCK_DTS_DIV1;
    basic_init_struct.auto_reload_preload = TIM_AUTORELOAD_PRE_DISABLE;
    std_tim_init(TIM3, &basic_init_struct);
    
    
    oc_config_struct.output_compare_mode = TIM_OUTPUT_MODE_PWM1;
    oc_config_struct.output_pol = TIM_OUTPUT_POL_HIGH;
    oc_config_struct.pulse = TIM_PULSE1_VALUE;
    std_tim_output_compare_init(TIM3, &oc_config_struct, TIM_CHANNEL_1);
    
    
    oc_config_struct.pulse = TIM_PULSE2_VALUE;
    std_tim_output_compare_init(TIM3, &oc_config_struct, TIM_CHANNEL_2);
    
    
    oc_config_struct.pulse = TIM_PULSE3_VALUE;
    std_tim_output_compare_init(TIM3, &oc_config_struct, TIM_CHANNEL_3);
    
    
    oc_config_struct.pulse = TIM_PULSE4_VALUE;
    std_tim_output_compare_init(TIM3, &oc_config_struct, TIM_CHANNEL_4);
}

/**
* @brief              Comment text fixed.
* @retval             value
*/
void bsp_tim3_output_start(void)
{
    
    std_tim_ccx_channel_enable(TIM3, TIM_CHANNEL_1);
    std_tim_ccx_channel_enable(TIM3, TIM_CHANNEL_2);
    std_tim_ccx_channel_enable(TIM3, TIM_CHANNEL_3);
    std_tim_ccx_channel_enable(TIM3, TIM_CHANNEL_4);

    
    std_tim_enable(TIM3);    

}
#endif

#if 1
/**
* @brief              Comment text fixed.
* @retval             value
*/
void tim4_out_gpio_init(void)
{
    std_gpio_init_t tim3_gpio_init = {0};
    
    std_rcc_gpio_clk_enable(RCC_PERIPH_CLK_GPIOA);
        
    /* Hardware PWM mapping:
       PA5 -> TIM4_CH4 (suction motor PWM)
       PA7 -> TIM4_CH1 (left wheel speed PWM) */
    tim3_gpio_init.pin = GPIO_PIN_5;
    tim3_gpio_init.mode = GPIO_MODE_ALTERNATE;
    tim3_gpio_init.pull = GPIO_NOPULL;
    tim3_gpio_init.alternate = GPIO_AF2_TIM4;
    std_gpio_init(GPIOA, &tim3_gpio_init);   
	
	tim3_gpio_init.pin = GPIO_PIN_7;
	tim3_gpio_init.alternate = GPIO_AF4_TIM4;
    std_gpio_init(GPIOA, &tim3_gpio_init);  
	
}


/**
* @brief              Comment text fixed.
* @retval             value
*/
void tim4_out_init(void)
{
    std_tim_basic_init_t basic_init_struct = {0};
    std_tim_output_compare_init_t oc_config_struct = {0};
    
    
    std_rcc_apb1_clk_enable(RCC_PERIPH_CLK_TIM4);
    
    
    basic_init_struct.prescaler = TIM_PRESCALER_VALUE;
    basic_init_struct.counter_mode = TIM_COUNTER_MODE_UP;
    basic_init_struct.period = TIM_PERIOD_VALUE-1;
    basic_init_struct.clock_div = TIM_CLOCK_DTS_DIV1;
    basic_init_struct.auto_reload_preload = TIM_AUTORELOAD_PRE_DISABLE;
    std_tim_init(TIM4, &basic_init_struct);

	// 左轮, 低电平速度快
    oc_config_struct.output_compare_mode = TIM_OUTPUT_MODE_PWM2;
    oc_config_struct.output_pol = TIM_OUTPUT_POL_HIGH;
    oc_config_struct.pulse = 0;
    std_tim_output_compare_init(TIM4, &oc_config_struct, TIM_CHANNEL_1);

    // 吸水电机
    oc_config_struct.output_compare_mode = TIM_OUTPUT_MODE_PWM1;
    oc_config_struct.pulse = 0;	// 50%
    std_tim_output_compare_init(TIM4, &oc_config_struct, TIM_CHANNEL_4);
}

/**
* @brief              Comment text fixed.
* @retval             value
*/
void bsp_tim4_output_start(void)
{
    
    std_tim_ccx_channel_enable(TIM4, TIM_CHANNEL_1);
    std_tim_ccx_channel_enable(TIM4, TIM_CHANNEL_4);

    
    std_tim_enable(TIM4);    
}
#endif

#if 1
/**
* @brief              Comment text fixed.
* @retval             value
*/
void tim5_out_gpio_init(void)
{
    std_gpio_init_t tim3_gpio_init = {0};
    
    
    std_rcc_gpio_clk_enable(RCC_PERIPH_CLK_GPIOA);
        
    /* Hardware PWM mapping:
       PA6 -> TIM5_CH1 (right wheel speed PWM) */
    tim3_gpio_init.pin = GPIO_PIN_6;
    tim3_gpio_init.mode = GPIO_MODE_ALTERNATE;
    tim3_gpio_init.pull = GPIO_NOPULL;
    tim3_gpio_init.alternate = GPIO_AF5_TIM5;
    std_gpio_init(GPIOA, &tim3_gpio_init);   

}


/**
* @brief              Comment text fixed.
* @retval             value
*/
void tim5_out_init(void)
{
    std_tim_basic_init_t basic_init_struct = {0};
    std_tim_output_compare_init_t oc_config_struct = {0};
    
    std_rcc_apb1_clk_enable(RCC_PERIPH_CLK_TIM5);
    
    
    basic_init_struct.prescaler = TIM_PRESCALER_VALUE;
    basic_init_struct.counter_mode = TIM_COUNTER_MODE_UP;
    basic_init_struct.period = TIM_PERIOD_VALUE-1;
    basic_init_struct.clock_div = TIM_CLOCK_DTS_DIV1;
    basic_init_struct.auto_reload_preload = TIM_AUTORELOAD_PRE_DISABLE;
    std_tim_init(TIM5, &basic_init_struct);
    
    // 左轮, 低电平速度快, pwm2方式为计数到了，才输出有效电平
    oc_config_struct.output_compare_mode = TIM_OUTPUT_MODE_PWM2;
    oc_config_struct.output_pol = TIM_OUTPUT_POL_HIGH;
    oc_config_struct.pulse = 0;
    std_tim_output_compare_init(TIM5, &oc_config_struct, TIM_CHANNEL_1);

}

/**
* @brief              Comment text fixed.
* @retval             value
*/
void bsp_tim5_output_start(void)
{
    std_tim_ccx_channel_enable(TIM5, TIM_CHANNEL_1);
    std_tim_enable(TIM5);    
}
#endif


#if 1	// comment fixed
#define TIM8_ARR_VALUE           (100 - 1)

/**
* @brief              Comment text fixed.
* @retval             value
*/
void tim8_init(void)
{
    std_tim_basic_init_t basic_init_struct = {0};
    uint32_t tmp_psc_value;
    
    
    std_rcc_apb1_clk_enable(RCC_PERIPH_CLK_TIM8);

	// comment fixed
	// comment fixed
    tmp_psc_value = (uint32_t)(((SystemCoreClock) / 800000) - 1);
    // comment fixed
    
    basic_init_struct.prescaler = tmp_psc_value;
    basic_init_struct.counter_mode = TIM_COUNTER_MODE_UP;
    basic_init_struct.period = TIM8_ARR_VALUE;
    basic_init_struct.clock_div = TIM_CLOCK_DTS_DIV1;
    basic_init_struct.auto_reload_preload = TIM_AUTORELOAD_PRE_DISABLE;
    std_tim_init(TIM8, &basic_init_struct);    
	 
    std_tim_interrupt_enable(TIM8, TIM_INTERRUPT_UPDATE);
    std_tim_enable(TIM8);

		
    NVIC_SetPriority(TIM8_IRQn, NVIC_PRIO_0);
    NVIC_EnableIRQ(TIM8_IRQn);   
}

void TimingDelay_Isr(void);

/**
* @brief              Comment text fixed.
* @retval             value
*/
void TIM8_IRQHandler(void)
{
    
    if ((std_tim_get_interrupt_enable(TIM8, TIM_INTERRUPT_UPDATE)) && (std_tim_get_flag(TIM8, TIM_FLAG_UPDATE)))
    {
        std_tim_clear_flag(TIM8, TIM_FLAG_UPDATE);

		TimingDelay_Isr();
		LED1_TOGGLE();
    }
}


static __IO uint32_t l_ustick;			// comment fixed
static __IO uint32_t l_tick;			// comment fixed
static __IO uint32_t l_tick_second;		// comment fixed
static __IO uint32_t l_tick_Dly;		// comment fixed

volatile unsigned short TimCnt0_1mS;	

volatile bit_field_t TimFlg;

extern void Turn_Tmo_Hand(void);
/**
  * @brief              Comment text fixed.
  * @param              parameter
  * @retval             value
  * @attention          comment
  */
void TimingDelay_Isr(void)
{
	TimCnt0_1mS++;
	Tim0_1ms_flg = 1;
	l_ustick++;

	
	if(TimCnt0_1mS % 8 ==0)
		{
		Tim1ms_flg=1;
		
		// tick 
		l_tick++;
		l_tick_Dly++;
		
		if(TimCnt0_1mS % 80 ==0)
			{
			Tim10ms_flg=1;

			Turn_Tmo_Hand();
			
			if(TimCnt0_1mS % 8000 == 0)
				{
				Tim1s_flg = 1;
				
				l_tick_second++;

				#if (WHEEL_DRIVER_BY_MOTOR==1)
				
				// 1秒种, 根据频率, 调整速度; davidd 20250306
				if ((f_Turn_wheel==0)&&(f_Pwr_On))
					{
					// 开机并且非转弯时, 根据速度调节轮子的Pwm
					if (ui_wheel_lh_cnt > WHEEL_FB_FREQ_OVRR)
						{
						// 限制最小值为30%
						if (ui_wheel_l_pwm > 300) ui_wheel_l_pwm--;

						TIM4->CC1 = (uint32_t)ui_wheel_l_pwm;
						}
					else if (ui_wheel_lh_cnt < WHEEL_FB_FREQ_LOW)
						{
						// 限制最大值为99%
						if (ui_wheel_l_pwm < 990)  ui_wheel_l_pwm++;

						TIM4->CC1 = (uint32_t)ui_wheel_l_pwm;
						}

					if (ui_wheel_rh_cnt > WHEEL_FB_FREQ_OVRR)
						{
						// 限制最小值为30%
						if (ui_wheel_r_pwm > 300) ui_wheel_r_pwm--;

						TIM5->CC1 = (uint32_t)ui_wheel_r_pwm;
						}
					else if (ui_wheel_rh_cnt < WHEEL_FB_FREQ_LOW)
						{
						// 限制最大值为99%
						if (ui_wheel_r_pwm < 990)  ui_wheel_r_pwm++;
						
						TIM5->CC1 = (uint32_t)ui_wheel_r_pwm;
						}
					}

				ui_wheel_lh_cnt = 0;
				ui_wheel_rh_cnt = 0;

				#endif

				
				if (TimCnt0_1mS >= 40000)
					{
					TimCnt0_1mS = 0;
					}
				}
			}
		}	

	// comment fixed
	// Pwm_out_adjust();
}

unsigned short TimOut1mS[MAX1MS]={0};
unsigned short TimOut10mS[MAX10MS]={0}; 
unsigned short TimOut1S[MAX1S]={0}; 

// comment fixed
void TimFlg_Hand(void)
{
	uint8_t i;
	if (Tim0_1ms_flg)
		{
		Tim0_1ms_flg = 0;
		}
	
	if (Tim1ms_flg)
		{
		Tim1ms_flg = 0;

		for (i=0;i<MAX1MS;i++)
			{
			TimOut1mS[i]++;
			}
		}

	if (Tim10ms_flg)
		{
		Tim10ms_flg = 0;

		for (i=0;i<MAX10MS;i++)
			{
			TimOut10mS[i]++;
			}
		}

	if (Tim1s_flg)
		{
		Tim1s_flg = 0;

		for (i=0;i<MAX1S;i++)
			{
			TimOut1S[i]++;
			}
		}
}

uint32_t get_tick(void)
{
	return l_tick;
}

uint32_t get_tick_second(void)
{
	return l_tick_second;
}

void delay_clk(uint32_t nclk)
{
	for (uint32_t i=0;i<nclk;i++);
}


void Delay_125us(__IO uint32_t nTime)
{
	uint32_t  nTimeH=0;
	
		// comment fixed
		nTimeH = nTime/100;
		while (nTimeH--)
			{
			//WDG_ReloadCounter;
			l_ustick = 0;
			while (l_ustick < 100);
			}
	
		// comment fixed
		//WDG_ReloadCounter;
		nTimeH = nTime%100;
		
		l_ustick = 0;
		while (l_ustick < nTimeH);

}


/**
  * @brief              Comment text fixed.
  * @param  
  * Comment text fixed.
  * @retval             value
  */

void Delay_1ms(__IO uint32_t nTime)
{
	uint32_t  nTimeH=0;
	// uint32_t  l_Try;
	
	// comment fixed
	nTimeH = nTime/100;
	while (nTimeH--)
		{
		WDG_ReloadCounter;
		l_tick_Dly = 0;
		while (l_tick_Dly < 100);
		}

	// comment fixed
	WDG_ReloadCounter;
	nTimeH = nTime%100;
	
	l_tick_Dly = 0;
	while (l_tick_Dly < nTimeH);
	
#if 0
	if (nTime%100 >= 50)
		WDG_ReloadCounter;
#endif
}


#endif


