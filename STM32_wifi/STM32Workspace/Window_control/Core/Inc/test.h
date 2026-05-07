#ifndef TEST_H
#define TEST_H

#include <string.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

int dummy(int);
void autoModeInputs(float pan, float tilt, bool tilt_correction, float longitude, float latitude, char country[32]);

#ifdef __cplusplus
}
#endif

#endif
