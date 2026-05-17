/************************************************************************************************/
/**
* @file               rtc_bsp.c
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
#include <stdio.h>
#include "rtc_bsp.h"


/*------------------------------------------variables-------------------------------------------*/

__IO uint8_t g_timestamp_flag = 0;

/*------------------------------------------functions-------------------------------------------*/

/**
* @brief              Comment text fixed.
* @retval             value
*/
void rtc_clock_config(void)
{    
    
    std_rcc_apb1_clk_enable(RCC_PERIPH_CLK_RTC);
    
    
    std_rcc_apb1_clk_enable(RCC_PERIPH_CLK_PMU);
    std_pmu_vaon_write_enable();     

    
    std_rcc_lxtal_drive_mode_config(RCC_LXTAL_DRIVE_MODE_ENHANCE);
    std_rcc_lxtal_drive_config(RCC_LXTAL_DRIVE_LEVEL2);
    std_rcc_lxtal_enable(RCC_LXTAL_ON);
    while(!std_rcc_get_lxtal_ready());
                    
    
    std_rcc_set_rtcclk_source(RCC_RTC_ASYNC_CLK_SRC_LXTAL);
    
    
    std_rcc_rtc_enable();
}



/**
* @brief              Comment text fixed.
* @retval             value
*/
void RTC_TAMP_IRQHandler(void)
{
    
    if(std_tamp_get_interrupt_enable(TAMP_INTERRUPT_TAMP_IN) 
    && std_tamp_get_interrupt_flag(TAMP_INTERRUPT_FLAG_TAMP_IN))
    {
        
        std_tamp_clear_flag(TAMP_CLEAR_FLAG_TAMP_IN);
        
        g_timestamp_flag = 1;
    }
}

/**
* @brief              Comment text fixed.
* @retval             value
*/
void bsp_rtc_datetime_config(void)
{
    std_rtc_time_t rtc_time = {0};
    std_rtc_date_t rtc_date = {0};
    std_status_t status = STD_ERR;
    
    
    rtc_date.weekday = RTC_WEEKDAY_TUESDAY;
    rtc_date.month = RTC_MONTH_OCTOBER;
    rtc_date.day = 0x16;
    rtc_date.year = 0x26;

    status = std_rtc_date_init(&rtc_date);
    while (status != STD_OK)
    {
        
    }

    
    rtc_time.hours = 0x10;
    rtc_time.minutes = 0x05;
    rtc_time.seconds = 0x05;

    status = std_rtc_time_init(&rtc_time);
    while (status != STD_OK)
    {
        
    }
}

/**
* @brief              Comment text fixed.
* @retval             value
*/
void bsp_rtc_tamp_in_config(void)
{
    
    std_rtc_write_protection_disable();

    
    std_rtc_output_disable();
    
    
    std_rtc_write_protection_enable();

    
    std_tamp_disable(TAMP_SOURCE_TAMP_IN);
    
    
    std_tamp_set_trigger(TAMP_TRIGGER_FALLING_EDGE);
        
    
    std_tamp_set_filter(TAMP_FILTER_DISABLE);
        
    
    std_tamp_pullup_enable();
    
    
    std_tamp_interrupt_enable(TAMP_INTERRUPT_TAMP_IN);
    
    
    std_tamp_enable(TAMP_SOURCE_TAMP_IN);
    
    
    NVIC_SetPriority(RTC_TAMP_IRQn, NVIC_PRIO_0); 
    
    NVIC_EnableIRQ(RTC_TAMP_IRQn); 
}

/**
* @brief              Comment text fixed.
* @param              parameter
* @param              parameter
* @retval             value
*/
void bsp_rtc_get_timestamp(uint8_t *timestamp_date, uint8_t *timestamp_time)
{
    
    while(!std_rtc_get_flag(RTC_FLAG_TIMESTAMP));
    
    
    timestamp_time[0] = std_rtc_timestamp_get_hour();
    timestamp_time[1] = std_rtc_timestamp_get_minute();
    timestamp_time[2] = std_rtc_timestamp_get_second();

    
    timestamp_date[0] = std_rtc_timestamp_get_day();
    timestamp_date[1] = std_rtc_timestamp_get_month();
    
    
    std_rtc_clear_flag(RTC_CLEAR_TIMESTAMP);
}


void Rtc_Init(void)
{
	uint8_t ts_time[3], ts_date[2];
	
	rtc_clock_config();

	

	
	bsp_rtc_datetime_config();

	
	bsp_rtc_tamp_in_config();

	
	while (g_timestamp_flag == 0);

	
	//bsp_rtc_get_timestamp(ts_date, ts_time); 

	
	printf("Timestamp Time = %02d:%02d:%02d\r\n",  
	   std_rtc_convert_bcd2bin(ts_time[0]), 
	   std_rtc_convert_bcd2bin(ts_time[1]), 
	   std_rtc_convert_bcd2bin(ts_time[2]));

	
	printf("Timestamp Date = %2d-%2d\r\n",	
	   std_rtc_convert_bcd2bin(ts_date[0]), 
	   std_rtc_convert_bcd2bin(ts_date[1]));

}



