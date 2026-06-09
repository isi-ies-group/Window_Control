#ifndef MOVEMENT_H
#define MOVEMENT_H

#ifdef __cplusplus
extern "C" {
#endif

void init_motors(void);

void move(float xmm, float zmm);

void move_external_vertical_right(float mm);
void move_external_vertical_left(float mm);
void move_internal_vertical_right(float mm);
void move_internal_vertical_left(float mm);
void move_horizontal_left(float mm);
void move_horizontal_right(float mm);

void GoHomePair(float *posX, float *posZ);
void SecondTouchPair(long speed_us);
void BackoffAll(int steps, long speed_us);

void adjustmentZ(void);
void antiBacklashZ(int cycles, int steps, long speed_us);

long computeHorizontalSteps(float delta_mm);
long computeVerticalSteps(float delta_mm);

#ifdef __cplusplus
}
#endif

#endif /* MOVEMENT_H */
