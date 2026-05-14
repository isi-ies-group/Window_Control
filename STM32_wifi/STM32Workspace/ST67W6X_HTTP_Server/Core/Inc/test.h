#ifndef TEST_H
#define TEST_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

int dummy(int x);
void autoModeInputs(float pan, float tilt, bool tilt_correction,
                    float longitude, float latitude, const char *country);

#ifdef __cplusplus
}
#endif

#endif /* TEST_H */
