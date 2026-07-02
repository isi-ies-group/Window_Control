#include "solar_app.h"
#include "global_structs.h"
#include "autoMode.h"
#include "gps.h"
#include "movement_task.h"
#include "state_machine.h"
#include "storage.h"

void SolarApp_Start(void)
{
  /*
   * What: start the application-level services that are not owned by the ST WiFi demo.
   * How: restore persistent globals first, then initialize the FSM with those values.
   * Why: the FSM must see the recovered state/position/config instead of boot defaults.
   */
  (void)Storage_LoadAll();

  /*
   * What: home the mechanics at every boot before the FSM starts running modes.
   * How: initializes the movement worker and queues one HOME command immediately.
   * Why: after reset or shutdown wakeup the stored state is useful, but position must be re-referenced first.
   */
  initMovementTask();
  requestHome();

  initFSM();

  /*
   * What: request GPS time synchronization immediately after boot.
   * How: the GPS task keeps this request pending until it parses valid date/time NMEA.
   * Why: restored flash time is only a fallback; valid GPS time must win automatically.
   */
  GPS_Task_RequestTimeSync();
}

void SolarApp_Process(void)
{
  /*
   * What: keep the old polling automode hook available for simple tests.
   * How: calls autoMode() only when auto_on is true.
   * Why: current production flow uses the FSM task, but this preserves compatibility.
   */
  if (auto_on)
  {
    autoMode();
    auto_counter++;
  }
}
