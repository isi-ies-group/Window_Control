#include <Arduino.h>
#include <Preferences.h>
#include "global_structs.h"
#include "state_machine.h"
#include "storage.h"

Preferences prefs;

void saveData() {
    prefs.begin("config", false);  // "config" = namespace NVS

    prefs.putDouble("latitude",  g_SPAInputs.latitude);
    prefs.putDouble("longitude", g_SPAInputs.longitude);
    prefs.putDouble("pan",       g_AOIInputs.pan);
    prefs.putDouble("tilt",      g_AOIInputs.tilt);
    prefs.putBool("tilt_corr",   g_AOIInputs.tilt_correction);

    prefs.putString("country",   "Spain"); 

    prefs.putInt("fsm_state", (int)thisSt);
    prefs.end();
    Serial.println("Flash data saved");
}

void loadData() {
    prefs.begin("config", true);

    g_SPAInputs.latitude  = prefs.getDouble("latitude",  40.4168);
    g_SPAInputs.longitude = prefs.getDouble("longitude", -3.7038);
    g_AOIInputs.pan       = prefs.getDouble("pan",       0.0);
    g_AOIInputs.tilt      = prefs.getDouble("tilt",      0.0);
    g_AOIInputs.tilt_correction = prefs.getBool("tilt_corr", false);

    String country = prefs.getString("country", "Spain");
    thisSt = (States)prefs.getInt("fsm_state", STDBY);

    prefs.end();
    Serial.println("Flash data loaded");
}
