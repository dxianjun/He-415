#include "main.h"
#include "string.h"

#include "rtc_bsp.h"
#include "usart_bsp.h"
#include "tim_bsp.h"
#include "exti_bsp.h"
#include "app_service.h"
#include "i2c_driver.h"
#include "ep24cx_i2c.h"

uint8_t FirmVers[20] = "V4.01_23.01.01";
static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";

void build_FirmVers(void)
{
    int month, day, year;
    uint8_t idx;
    char s_month[5];
    char s_Data[12];

    sprintf(s_Data, __DATE__);
    sscanf(s_Data, "%s %d %d", s_month, &day, &year);

    month = (strstr(month_names, s_month) - month_names) / 3 + 1;
    year = year - 2000;

    idx = 6;
    FirmVers[idx] = (uint8_t)(year / 10 + '0');
    FirmVers[idx + 1] = (uint8_t)(year % 10 + '0');
    FirmVers[idx + 3] = (uint8_t)(month / 10 + '0');
    FirmVers[idx + 4] = (uint8_t)(month % 10 + '0');
    FirmVers[idx + 6] = (uint8_t)(day / 10 + '0');
    FirmVers[idx + 7] = (uint8_t)(day % 10 + '0');

    printf("Soft_Vers: %s\r\n", FirmVers);
}

#define HARDWAREVERSION "HE415_MB_V4.01"

void PrintData2(uint8_t* databuf, uint8_t len)
{
    uint8_t tmp;
    for (tmp = 0; tmp < len; tmp++)
    {
        printf("%02x ", (uint16_t)databuf[tmp]);
    }
    printf("\r\n");
}

void iwdg_init(void)
{
    std_rcc_rcl_enable();
    while (std_rcc_get_rcl_ready() != 1) {}

    std_iwdg_write_access_enable();
    std_iwdg_set_overflow_period(IWDG_OVERFLOW_PERIOD_2048);
    std_iwdg_start();
}

int main(void)
{
    system_clock_config();
    
    tim8_init();

    uart_gpio_init();
    uart_init();
    usart_init_int();

    printf("*********%s********** \r\n", HARDWAREVERSION);
    printf("\r\nbuild(" __DATE__ " - " __TIME__ ") \r\n");
    build_FirmVers();

    led_init();
    std_delay_init();
    exti_init();

    tim4_out_gpio_init();
    tim4_out_init();
    bsp_tim4_output_start();

    tim5_out_gpio_init();
    tim5_out_init();
    bsp_tim5_output_start();

    iwdg_init();
    sys_flg_init();
		
		I2C_Configuration();
    Init_SysData();

    while (1)
    {
        TimFlg_Hand();
        std_iwdg_refresh();
        ScheduleHandle();
    }
}
