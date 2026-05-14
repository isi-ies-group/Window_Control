#include "solar_app.h"
#include "global_structs.h"
#include "gps.h"
#include "test.h"

void SolarApp_Start(void)
{
  autoModeInputs(180.0f, 90.0f, true, -3.70f, 40.41f, "Spain");
  setManualTime(2026, 7, 2, 12, 0, 0);
  auto_on = true;
}
