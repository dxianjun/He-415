#ifndef I2C_DRIVER_H
#define I2C_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "ciu32l051_std.h"

#define I2C_GPIO_PORT  GPIOB
#define I2C_SCL_PIN    GPIO_PIN_8
#define I2C_SDA_PIN    GPIO_PIN_9

void i2c_gpio_init(void);
void I2C_Configuration(void);

bool I2C_PolledMasterWrite(uint8_t device_addr, uint8_t regADD, uint8_t* write_data, uint8_t num_bytes);
bool I2C_PolledMasterRead(uint8_t device_addr, uint8_t regADD, uint8_t* read_data, uint8_t num_bytes);

bool I2C_PolledMasterNWrite(uint8_t device_addr, uint8_t* write_data, uint8_t num_bytes);
bool I2C_PolledMasterNRead(uint8_t device_addr, uint8_t* read_data);

bool I2C_WriteData(unsigned char Cmd, unsigned char write_data);

#ifdef __cplusplus
}
#endif

#endif
