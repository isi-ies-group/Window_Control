#include "test.h"
#include "global_structs.h"
#include "main.h"
#include <string.h>
#include <stdio.h>

extern RTC_TimeTypeDef sTime;
extern RTC_DateTypeDef sDate;


int dummy(int x){
return ++x;
}

void autoModeInputs(float pan, float tilt, bool tilt_correction, float longitude, float latitude, char country[32]){

    g_SPAInputs.latitude = latitude;
    g_SPAInputs.longitude = longitude;
    g_AOIInputs.pan = pan;
    g_AOIInputs.tilt = tilt;
    g_AOIInputs.tilt_correction = tilt_correction;
    strncpy(g_country, country, sizeof(g_country)-1);
    g_country[sizeof(g_country)-1] = '\0';

}
