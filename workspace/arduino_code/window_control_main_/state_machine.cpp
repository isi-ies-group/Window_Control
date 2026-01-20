#include "state_machine.h"
#include "autoMode.h"
#include "global_structs.h"
#include "matrices.h"
#include "storage.h"
#include <time.h>
#include "movement_task.h"
#include "commonlib.h"
#include <Arduino.h>
#include "gps.h"
#include "spa_func.h"
#include "stdint.h"

States thisSt;
States nextSt;
States prevSt;
unsigned long start = millis();


void initFSM() {
	Serial.print("Machine initialized. Current State: ");
	loadState();
	thisSt = STDBY;
	switch (thisSt) {
		case STDBY:       
			Serial.println("STDBY"); 
			break;
		case CONFIG:      
			Serial.println("CONFIG"); 
			break;
		case MANUAL:      
			Serial.println("MANUAL"); 
			break;
		case SLEEP:       
			Serial.println("SLEEP"); 
			break;
		case AWAKENING:   
			Serial.println("AWAKENING"); 
			break;
		case EPH_INPUT:   
			Serial.println("EPH_INPUT");
			break;
		case AUTO_MODE:   
			Serial.println("AUTO_MODE");
			break;
		default:          
			Serial.println("UNKNOWN");
			break;
	}
	if (thisSt == SLEEP && esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER) {
    Serial.println("[BOOT] Wakeup from sleep");

    auto_on   = rtc_auto_mode_on;
    g_sunrise_epoch = rtc_sunrise_epoch;
    g_sunset_epoch  = rtc_sunset_epoch;

    thisSt = AWAKENING;
		saveState();
	}
}	

String stateToText(States s) {
    switch (s) {
      case STDBY: return "STDBY";
      case CONFIG: return "CONFIG";
      case MANUAL: return "MANUAL";
      case EPH_INPUT: return "EPH_INPUT";
      case AUTO_MODE: return "AUTO_MODE";
      case SLEEP: return "SLEEP";
      case AWAKENING: return "AWAKENING";
      default: return "UNKNOWN";
    }
}


void runMachine() {
	int64_t sleep_us = 0;
	time_t now;
	time(&now);
	if (thisSt != prevSt) {
		switch (thisSt) {
			case CONFIG:
				saveState();
				Serial.println("[FSM]: CONFIG");
				break;
			case STDBY:
				auto_on = false;
				saveState();
				Serial.println("[FSM]: STDBY");
				break;

			case MANUAL:
				Serial.println("[FSM]: MANUAL");
				saveState();
				requestHome();
				break;

			case SLEEP:
				Serial.println("[FSM]: SLEEP");
				saveState();
				rtc_auto_mode_on   = auto_on;
				rtc_sunrise_epoch = g_sunrise_epoch;
				rtc_sunset_epoch  = g_sunset_epoch;


				sleep_us = (rtc_sunrise_epoch - now) * 1000000LL;

				if (sleep_us > 0) {
					Serial.println("[SLEEP] Deep sleep");
					esp_sleep_enable_timer_wakeup(sleep_us);
					delay(50);
					esp_deep_sleep_start();
				} 
				
				else {
					Serial.println("[SLEEP] Sunrise already passed, skipping sleep");
				}

				break;			

			case AWAKENING:
				Serial.println("[FSM]: AWAKENING");
				saveState();
				loadData();
				if (syncTimeFromGPSQuick()) {
						Serial.println("[AWAKE] Time synced from GPS");
						time_t now;
						struct tm time_info;
						time(&now);
						localtime_r(&now, &time_info);
						updateSPAInputsFromTime(&time_info, &g_SPAInputs);
						printLocalTime();
						SPA_f();
					} 
				else {
						Serial.println("[AWAKE] GPS time sync failed");
				}			
				changeState(fsmProcess(woke_up, rtc_auto_mode_on));
				break;

			case EPH_INPUT:
				saveState();
				Serial.println("[FSM]: EPH_INPUT");
				requestHome();
				break;

			case AUTO_MODE:
				saveState();
				auto_on = true;
				Serial.println("[FSM]: AUTO_MODE");
				requestHome();
				start = millis();
			break;
		}

		prevSt = thisSt;
	}

	if (thisSt == AUTO_MODE) {
		time_t now;
    time(&now);

    if (rtc_sunset_epoch > 0 && now >= rtc_sunset_epoch) {
        Serial.println("[AUTO] Sunset reached → going to sleep");
        changeState(fsmProcess(go_sleep, auto_on));
        return;
    }

		if (millis() - start >= 5000) {
			auto_on = true;
			autoMode();
			start = millis();       
		}
	}
}


States fsmProcess(Events event, bool auto_running){
	if (event == begin_config) return CONFIG;
	if (event == end_config) return STDBY;
	if (event == end_eph_input) return STDBY;
	if (event == end_manual) return STDBY;
	if (event == begin_eph_input) return EPH_INPUT;
	if (event == begin_manual) return MANUAL;
	if (event == toggle_auto_mode && auto_running == false) return AUTO_MODE;
	if (event == toggle_auto_mode && auto_running == true) return STDBY;
	if (event == go_sleep) return SLEEP;
	if (event == wake_up) return AWAKENING;
	if (event == woke_up) 
		if (auto_running == true) return AUTO_MODE;
		else return STDBY;
}


void changeState(States newSt) {
    bool valid = true;

    switch (thisSt) {
			case STDBY:
				if (newSt == AUTO_MODE || newSt == CONFIG || newSt == MANUAL || newSt == SLEEP
						|| newSt == EPH_INPUT)
					nextSt = newSt;
				else valid = false;
				break;
			case CONFIG:
					nextSt = newSt;
				break;						
			case AUTO_MODE:
				if (newSt == STDBY || newSt == SLEEP)
					nextSt = newSt;
				else valid = false;
				break;
			case SLEEP:
				if (newSt == AWAKENING)
					nextSt = newSt;
				else valid = false;
				break;
			case AWAKENING:
				if (newSt == STDBY || newSt == AUTO_MODE)
					nextSt = newSt;
				else valid = false;
			break;
			case EPH_INPUT:
					nextSt = newSt;
			break;		
			case MANUAL:
					nextSt = newSt;
			break;
	}

    if (valid) thisSt = nextSt;
}
