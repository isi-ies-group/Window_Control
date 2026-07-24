#include "test.h"

#include "global_structs.h"
#include "storage.h"

#include <string>
#include <string.h>

/*
 * Migration test/startup parameters.
 * Edit these values when the project is being tested without the WiFi web UI.
 */
#define TEST_STARTUP_APPLY_PARAMETERS      1
#define TEST_STARTUP_SAVE_CONFIG_TO_FLASH  0

#define TEST_STARTUP_PAN_DEG               180.0f
#define TEST_STARTUP_TILT_DEG              90.0f
#define TEST_STARTUP_TILT_CORRECTION       true
#define TEST_STARTUP_LONGITUDE_DEG         -3.70f
#define TEST_STARTUP_LATITUDE_DEG          40.41f
#define TEST_STARTUP_COUNTRY               "Spain"
#define TEST_STARTUP_MOVEMENT_GAIN         MOVEMENT_HYSTERESIS_DEFAULT_GAIN
#define TEST_STARTUP_MOVEMENT_OFFSET_MM    MOVEMENT_HYSTERESIS_DEFAULT_OFFSET_MM

int dummy(int x)
{
    return ++x;
}

void Test_SetAutoModeInputs(float pan,
                            float tilt,
                            bool tilt_correction,
                            float longitude,
                            float latitude,
                            const char *country)
{
    g_SPAInputs.latitude = latitude;
    g_SPAInputs.longitude = longitude;
    g_AOIInputs.pan = pan;
    g_AOIInputs.tilt = tilt;
    g_AOIInputs.tilt_correction = tilt_correction;
    g_movement_hysteresis_gain = TEST_STARTUP_MOVEMENT_GAIN;
    g_movement_hysteresis_offset_mm = TEST_STARTUP_MOVEMENT_OFFSET_MM;

    /* g_country is a fixed C buffer in the migrated application layer. */
    strncpy(g_country, (country != 0) ? country : "", sizeof(g_country) - 1U);
    g_country[sizeof(g_country) - 1U] = '\0';
}

void autoModeInputs(float pan,
                    float tilt,
                    bool tilt_correction,
                    float longitude,
                    float latitude,
                    const std::string &country)
{
    Test_SetAutoModeInputs(pan,
                           tilt,
                           tilt_correction,
                           longitude,
                           latitude,
                           country.c_str());
}

void Test_ApplyStartupParameters(void)
{
#if (TEST_STARTUP_APPLY_PARAMETERS != 0)
    /*
     * This function is intentionally called from the FSM STARTUP state.
     * It replaces the web configuration path while the migration project is
     * being tested with hard-coded inputs.
     */
    Test_SetAutoModeInputs(TEST_STARTUP_PAN_DEG,
                           TEST_STARTUP_TILT_DEG,
                           TEST_STARTUP_TILT_CORRECTION,
                           TEST_STARTUP_LONGITUDE_DEG,
                           TEST_STARTUP_LATITUDE_DEG,
                           TEST_STARTUP_COUNTRY);

#if (TEST_STARTUP_SAVE_CONFIG_TO_FLASH != 0)
    /*
     * Disabled by default to avoid rewriting flash at every boot. Enable it
     * only if the hard-coded configuration must become the stored fallback.
     */
    (void)saveData();
#endif
#endif
}
