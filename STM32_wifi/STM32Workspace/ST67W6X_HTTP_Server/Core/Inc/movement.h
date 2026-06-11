#ifndef MOVEMENT_H
#define MOVEMENT_H

#ifdef __cplusplus
extern "C" {
#endif

void init_motors(void);
void move(float xmm, float zmm);
void GoHomePair(float *posX, float *posZ);

#ifdef __cplusplus
}
#endif

#endif /* MOVEMENT_H */
