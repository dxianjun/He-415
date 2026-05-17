# Source Functions

Generated from: `He415mb/Source`

## app_service.c
- `wheel_set_forward`
- `wheel_set_turn_left`
- `wheel_set_turn_right`
- `Power_On`
- `Power_Off`
- `vi_Full_warn`
- `vi_Power_off`
- `sys_flg_init`
- `App_OnMotorCmd`
- `Turn_Tmo_Hand`
- `Input_Hand`
- `Tmr_Hand`
- `user_serv`

## app_tcp.c
- `UART4_Send_Buf`
- `UART2_Send_Buf`
- `SendToSub_Pwr`
- `SendToSub_Water`
- `SendToSub_State`
- `SendToSub_Spring`
- `SendToSub_Speed`
- `Bt_Send_info`
- `Bt_Send_Ack`
- `Bt_Setting`
- `Bt_Send_ReadMac_Resp`
- `parse_sub`
- `ParseSubCmd`
- `parse_bt`
- `ParseBtCmd`
- `tcp_hand`

## common.c
- `system_clock_config`

## ep24cx_i2c.c
- `ReadEpData`
- `WriteEpData`
- `Save_CustomCode`
- `clr_Custcode`
- `InitEprom_SysData`
- `GetEpromSign`
- `Init_SysData`

## exti_bsp.c
- `exti_gpio_init`
- `exti_init_config`
- `exti_init`
- `EXTI4_15_IRQHandler`
- `led_init`
- `Exit_Hand`

## i2c_driver.c
- `I2C_delay`
- `Delay1ms_i2c`
- `SDA_IO_SET`
- `I2c_gpio_init`
- `I2C_Start`
- `I2C_Stop`
- `I2C_SendAck`
- `I2C_SendNoAck`
- `I2C_ChkAck`
- `I2C_WriteByte`
- `I2C_ReadByte`
- `I2C_WriteBytes`
- `I2C_ReadBytes`
- `I2C_SendAddr`
- `i2c_gpio_init`
- `I2C_Configuration`
- `I2C_PolledMasterWrite`
- `I2C_PolledMasterRead`
- `I2C_PolledMasterNWrite`
- `I2C_PolledMasterNRead`
- `I2C_WriteData`

## input_bsp.c
- `input_gpio_init`
- `input_dc_in_is_active`
- `input_bat_low_is_active`

## main.c
- `build_FirmVers`
- `PrintData2`
- `iwdg_init`
- `main`

## output_bsp.c
- `output_gpio_init`
- `output_info_set`
- `output_bat_en_set`
- `output_pwr_lock_set`
- `output_left_wheel_dir_set`
- `output_right_wheel_dir_set`
- `output_wheel_dir_forward`
- `output_wheel_dir_turn_left`
- `output_wheel_dir_turn_right`
- `output_wheel_pwm_set`
- `output_fan_pwm_set`

## rtc_bsp.c
- `rtc_clock_config`
- `RTC_TAMP_IRQHandler`
- `bsp_rtc_datetime_config`
- `bsp_rtc_tamp_in_config`
- `bsp_rtc_get_timestamp`
- `Rtc_Init`

## tim_bsp.c
- `tim3_in_gpio_init`
- `tim3_in_init`
- `TIM3_IRQHandler`
- `bsp_tim3_capture_start`
- `bsp_rcc_mco_config`
- `Pwm_in_hand`
- `tim3_out_gpio_init`
- `tim3_out_init`
- `bsp_tim3_output_start`
- `tim4_out_gpio_init`
- `tim4_out_init`
- `bsp_tim4_output_start`
- `tim5_out_gpio_init`
- `tim5_out_init`
- `bsp_tim5_output_start`
- `tim8_init`
- `TIM8_IRQHandler`
- `TimingDelay_Isr`
- `TimFlg_Hand`
- `get_tick`
- `get_tick_second`
- `delay_clk`
- `Delay_125us`
- `Delay_1ms`

## usart_bsp.c
- `uart_gpio_init`
- `uart_init`
- `usart_init_int`
- `_write`
- `fputc`
- `USART1_IRQHandler`
- `UART2_IRQHandler`
- `UART3_4_IRQHandler`
- `uart2_4_test`

