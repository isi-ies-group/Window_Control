#include "interpolation.h"
#define N 86


typedef struct {
  const float (*matrix_X)[N];
  const float (*matrix_Z)[N];
  double AOIt;
  double AOIl;
} InterpolInputs;

void InterpolationTask(void *pvParameters);
