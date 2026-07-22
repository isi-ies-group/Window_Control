
#include "test.h"
#include "global_structs.h"

#include <string.h>

int dummy(int x)
{
  return ++x;
}

void autoModeInputs(float pan, float tilt, bool tilt_correction,
                    float longitude, float latitude, const char *country)
{
  g_SPAInputs.latitude = latitude;
  g_SPAInputs.longitude = longitude;
  g_AOIInputs.pan = pan;
  g_AOIInputs.tilt = tilt;
  g_AOIInputs.tilt_correction = tilt_correction;

  (void)strncpy(g_country, country, sizeof(g_country) - 1U);
  g_country[sizeof(g_country) - 1U] = '\0';
}
