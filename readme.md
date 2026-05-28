# HE-415 固件工程说明

## 1. 工程概览
本仓库是 `HE-415` 主控板（CIU32L051）固件工程，当前版本以 `V4.01` 为基础，已完成从 `V3.01` 迁移的核心业务拆分：
- 业务调度：`app_service.c/.h`
- 通讯协议：`app_tcp.c/.h`
- 软件 I2C：`i2c_driver.c/.h`
- EEPROM 接口：`ep24cx_i2c.c/.h`

主流程在 `main.c`：初始化时钟、串口、EXTI、PWM、IWDG、I2C/EEPROM 后，循环执行 `TimFlg_Hand()` + `ScheduleHandle()`。

## 2. 目录结构（核心）
```text
He415/
|- Drivers/
|  |- CMSIS/
|  `- CIU32L051_Lib/
|- He415mb/
|  |- Include/
|  |  |- main.h
|  |  |- common.h
|  |  |- tim_bsp.h / exti_bsp.h / usart_bsp.h / rtc_bsp.h
|  |  |- app_service.h
|  |  |- app_tcp.h
|  |  |- i2c_driver.h
|  |  `- ep24cx_i2c.h
|  |- Source/
|  |  |- main.c
|  |  |- common.c
|  |  |- tim_bsp.c
|  |  |- exti_bsp.c
|  |  |- usart_bsp.c
|  |  |- rtc_bsp.c
|  |  |- app_service.c
|  |  |- app_tcp.c
|  |  |- i2c_driver.c
|  |  `- ep24cx_i2c.c
|  `- MDK/
|     `- ciu32L051.uvprojx
|- 硬件脚位定义说明.md
`- readme.md
```

## 3. 软件架构与职责
- `main.c`
  - 启动初始化：时钟、UART、EXTI、TIM4/TIM5 PWM、看门狗。
  - 进入主循环：定时标志处理 + 应用调度。
- `app_service.c`
  - 电源状态控制（开机/关机/低电/满水/按键关机流程）。
  - 轮子方向与速度策略（前进、左右转、180 转向）。
  - 充电检测（DC_IN）与低电检测（BAT_DET）逻辑。
- `app_tcp.c`
  - 蓝牙与下位板协议解析。
  - 关键报文：电源、水泵/弹簧、电机控制、状态同步。
- `tim_bsp.c`
  - PWM 输出配置，当前业务使用：TIM4_CH4、TIM4_CH1、TIM5_CH1。
- `exti_bsp.c`
  - GPIO 输入输出初始化。
  - 轮速 EXTI 计数（自动/手动两组输入）。
- `i2c_driver.c + ep24cx_i2c.c`
  - PB8/PB9 软件 I2C 与 EEPROM 读写。

## 4. 当前关键引脚（与代码一致）
- 电源/检测
  - `PA4`：BAT_DET（低电检测，IPU）
  - `PA12`：DC_IN（充电检测，IPD）
  - `PA10`：BAT_EN（PP 输出，初始化低）
  - `PB4`：Info_out（OD 输出）
  - `PB5`：PWR_LOCK（OD 输出，开机拉高）
- 电机与方向
  - `TIM4_CH4`：吸水电机 PWM（PA5）
  - `TIM5_CH1`：右轮 PWM（PA6）
  - `TIM4_CH1`：左轮 PWM（PA7）
  - `PB11`：左轮方向
  - `PB13`：右轮方向
- 轮速 EXTI
  - 左轮自动：`PB12 / EXTI_LINE_GPIO_PIN12`
  - 左轮手动：`PB15 / EXTI_LINE_GPIO_PIN15`
  - 右轮自动：`PB14 / EXTI_LINE_GPIO_PIN14`
  - 右轮手动：`PD0 / EXTI_LINE_GPIO_PIN0`
- EEPROM（软件 I2C）
  - `PB8`：SCL
  - `PB9`：SDA

## 5. 编译与下载
- 工程文件：`He415mb/MDK/ciu32L051.uvprojx`
- IDE：Keil MDK（ARMCC/ARMClang，按现有工程配置）
- 启动文件：`He415mb/MDK/startup_ciu32l051.s`

建议流程：
1. 打开 `uvprojx`。
2. 选择目标配置（当前默认可编译通过）。
3. Build/Rebuild。
4. 通过 JLink 下载验证。

## 6. 迁移说明（V3.01 -> V4.01）
已完成的迁移策略：
- 业务逻辑不再堆到 `bsp` 文件中，按模块拆分到独立 `.c/.h`。
- `systick` 能力使用现有 `bsp_timer`/定时基准，不重复迁移旧实现。
- I2C 与 EEPROM 已独立成模块，便于后续替换硬件 I2C 或更换存储器件。

## 7. 后续维护建议
- 增加 `.gitignore`，排除 `He415mb/MDK/Objects/`、`Listings/`、`*.axf/*.o/*.d` 等构建产物。
- 将 `硬件脚位定义说明.md` 与原理图版本绑定（变更时同步更新）。
- 对 `app_tcp.c` 协议字段补充注释和帧格式表，减少后续联调成本。

