#include "solar_app.h"
#include "test.h"

void SolarApp_Start(void)
{
  char default_country[32] = "Spain";

  autoModeInputs(180.0f, 90.0f, true, -3.70f, 40.41f, default_country);
}
