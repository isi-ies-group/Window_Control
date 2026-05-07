#include "solar_app.h"
#include "test.h"
#include "autoMode.h"
#include <string.h>
void SolarApp_Start(void)
{
    autoModeInputs(180.0f, 90.0f, true, -3.70, 40.41, "Spain");
}
