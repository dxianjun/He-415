#include "common.h"

void system_clock_config(void)
{
    std_flash_set_latency(FLASH_LATENCY_2CLK);

    std_rcc_rch_enable();
    while(!std_rcc_get_rch_ready());

    std_rcc_pll_config(RCC_PLLSRC_RCH, RCC_PLLM_MUL12, RCC_PLLN_DIV2, RCC_PLL_DIV2);
    std_rcc_pll_enable();
    while(!std_rcc_get_pll_ready());

    std_rcc_pll_output_enable();

    std_rcc_set_sysclk_source(RCC_SYSCLK_SRC_PLLCLK);
    while(std_rcc_get_sysclk_source() != RCC_SYSCLK_SRC_STATUS_PLLCLK);

    std_rcc_set_ahbdiv(RCC_HCLK_DIV1);
    std_rcc_set_apb1div(RCC_PCLK1_DIV1);
    std_rcc_set_apb2div(RCC_PCLK2_DIV1);

    SystemCoreClockUpdate();
}


