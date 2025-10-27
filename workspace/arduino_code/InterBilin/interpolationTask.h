#ifndef spaTask_h
#define spaTask_h

#ifdef __cplusplus
extern "C" {
#endif

#include "interpolation.h"
#define N 86


typedef struct {
  const float (*matrix_X)[N];
  const float (*matrix_Z)[N];
  float AOIt;
  float AOIl;
} InterpolInputs;

void InterpolationTask(void *pvParameters);



#ifdef __cplusplus
}
#endif

#endif