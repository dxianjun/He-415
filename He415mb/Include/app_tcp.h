#ifndef APP_TCP_H
#define APP_TCP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ciu32l051_std.h"

typedef enum
{
    SS_NONE = 0,
    SS_VK_OFF,
    SS_BAT_LOW,
    SS_WATER_FULL
} ss_state_t;

void tcp_hand(void);

void SendToSub_Pwr(uint8_t uc_pwr);
void SendToSub_Water(uint8_t uc_water);
void SendToSub_State(uint8_t uc_state);
void SendToSub_Spring(uint8_t uc_spring);
void SendToSub_Speed(uint8_t uc_speed);

#ifdef __cplusplus
}
#endif

#endif
