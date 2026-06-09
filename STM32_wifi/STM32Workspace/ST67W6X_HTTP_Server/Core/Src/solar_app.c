#include "solar_app.h"
#include "global_structs.h"
#include "autoMode.h"
#include "gps.h"
#include "state_machine.h"
#include "test.h"

void SolarApp_Start(void)
{
  autoModeInputs(180.0f, 90.0f, true, -3.70f, 40.41f, "Spain");
  setManualTime(2026, 7, 2, 12, 0, 0);
  auto_on = false;
  initFSM();
}

void SolarApp_Process(void)
{
  if (auto_on)
  {
    autoMode();
    auto_counter++;
  }
}
