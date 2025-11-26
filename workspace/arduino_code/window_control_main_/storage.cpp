#include <Arduino.h>
#include <Preferences.h>
#include "global_structs.h"
#include "state_machine.h"
#include "storage.h"

Preferences prefs;

void saveData() {
    prefs.begin("config", false);


    prefs.putDouble("latitude",  g_SPAInputs.latitude);
    prefs.putDouble("longitude", g_SPAInputs.longitude);
    prefs.putDouble("pan",       g_AOIInputs.pan);
    prefs.putDouble("tilt",      g_AOIInputs.tilt);
    prefs.putBool("tilt_corr",   g_AOIInputs.tilt_correction);

    prefs.putString("country",   g_country); 
    prefs.putInt("fsm_state", (int)thisSt);
    
    Serial.println("Data saved to flash:");
    Serial.print("Latitude: "); Serial.println(g_SPAInputs.latitude);
    Serial.print("Longitude: "); Serial.println(g_SPAInputs.longitude);
    Serial.print("Pan: "); Serial.println(g_AOIInputs.pan);
    Serial.print("Tilt: "); Serial.println(g_AOIInputs.tilt);
    Serial.print("Tilt correction: "); Serial.println(g_AOIInputs.tilt_correction ? "true" : "false");
    Serial.print("Country: "); Serial.println(g_country);
    Serial.print("FSM state: "); Serial.println(stateToText(thisSt));

    prefs.end();
}

void loadData() {
    prefs.begin("config", true);

    g_SPAInputs.latitude  = prefs.getDouble("latitude",  40.4168);
    g_SPAInputs.longitude = prefs.getDouble("longitude", -3.7038);
    g_AOIInputs.pan       = prefs.getDouble("pan",       0.0);
    g_AOIInputs.tilt      = prefs.getDouble("tilt",      0.0);
    g_AOIInputs.tilt_correction = prefs.getBool("tilt_corr", false);

    g_country = prefs.getString("country",  "Spain");
    thisSt = (States)prefs.getInt("fsm_state", STDBY);

    
    Serial.println("Data loaded from flash:");
    Serial.print("Latitude: "); Serial.println(g_SPAInputs.latitude);
    Serial.print("Longitude: "); Serial.println(g_SPAInputs.longitude);
    Serial.print("Pan: "); Serial.println(g_AOIInputs.pan);
    Serial.print("Tilt: "); Serial.println(g_AOIInputs.tilt);
    Serial.print("Tilt correction: "); Serial.println(g_AOIInputs.tilt_correction ? "true" : "false");
    Serial.print("Country: "); Serial.println(g_country);
    Serial.print("FSM state: "); Serial.println(stateToText(thisSt));

    prefs.end();
}
void savePos(){
    prefs.putFloat("x_pos", g_x_val);
    prefs.putFloat("z_pos", g_z_val);
}
void loadPos(){
    g_x_val = prefs.getFloat("x_pos", 1.0);
    g_z_val = prefs.getFloat("z_pos", -1.0);
}