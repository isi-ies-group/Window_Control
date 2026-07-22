#ifndef MOVEMENT_H
#define MOVEMENT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MOVEMENT_HORIZONTAL_MAX_MM 72.0f

void init_motors(void);
void move(float xmm, float zmm);
float movementClampHorizontalTarget(float zmm);
void GoHomePair(float *posX, float *posZ);
uint8_t movementLimitSwitchUpdateFromExti(uint16_t gpio_pin);
void movementLimitSwitchRefreshAll(void);
uint8_t movementAnyLimitSwitchActive(void);

#ifdef __cplusplus
}
#endif

#endif /* MOVEMENT_H */
