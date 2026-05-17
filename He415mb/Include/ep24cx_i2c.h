#ifndef EP24CX_I2C_H
#define EP24CX_I2C_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "ciu32l051_std.h"

typedef struct
{
    uint16_t m_sign;
    uint16_t m_flg;
    uint16_t m_Custcode;
} SysData;

#define DATA_SYS_CNT    4
#define DATA_EPROM_CNT  6

#define SYSDATA_START    0
#define SYS_DATA_OFFSET  2
#define SYS_ADDR_OFFSET  4

#define SIGNADDR 0
#define SIGNAL   0xAA55

extern SysData gSysData;

bool ReadEpData(uint16_t raddr, uint8_t *rdata, uint16_t uNum);
bool WriteEpData(uint16_t waddr, uint8_t *wdata, uint16_t uNum);

void Init_SysData(void);
void Save_CustomCode(void);
void clr_Custcode(void);

#ifdef __cplusplus
}
#endif

#endif
