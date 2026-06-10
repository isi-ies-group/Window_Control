#include "state_machine.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "autoMode.h"
#include "eph_input_mode.h"
#include "global_structs.h"
#include "manual_mode.h"
#include "movement_task.h"

#define FSM_QUEUE_LENGTH      8U
#define FSM_TASK_STACK_SIZE   4096U
#define FSM_TASK_PRIORITY     23U
#define FSM_AUTO_PERIOD_MS    5000U

/* Current and next FSM states. Start in standby until persistent memory exists. */
States thisSt = STDBY;
States nextSt = STDBY;

static QueueHandle_t fsmQueue = NULL;      /* Events queue */
static TaskHandle_t fsmTaskHandle = NULL;  /* Avoid task replication */

static void fsmTask(void *argument);
static bool stateTransitionIsValid(States currentState, States newState);

void initFSM(void)
{
  /* FSM initialization stays inside this module: state, queue and task. */
  thisSt = STDBY;
  nextSt = STDBY;
  auto_on = false;

  if (fsmQueue == NULL)
  {
    fsmQueue = xQueueCreate(FSM_QUEUE_LENGTH, sizeof(Events));
  }

  if ((fsmQueue != NULL) && (fsmTaskHandle == NULL))
  {
    /* ST defines stack sizes in bytes, but xTaskCreate expects stack words. */
    (void)xTaskCreate(fsmTask,
                      "FSM_Task",
                      FSM_TASK_STACK_SIZE >> 2U,
                      NULL,
                      FSM_TASK_PRIORITY,
                      &fsmTaskHandle);
  }
}

bool fsmPostEvent(Events event)
{
  /* Public entry point for web/server code: HTTP only posts events. */
  if (fsmQueue == NULL)
  {
    return false;
  }

  return (xQueueSend(fsmQueue, &event, 0U) == pdTRUE);
}

States fsmGetState(void)
{
  /* Useful for status reporting in the web page. */
  return thisSt;
}

const char *stateToText(States state)
{
  switch (state)
  {
    case STDBY:
      return "STDBY";

    case AUTO_MODE:
      return "AUTO_MODE";

    case MANUAL:
      return "MANUAL";

    case EPH_INPUT:
      return "EPH_INPUT";

    default:
      return "UNKNOWN";
  }
}

States fsmProcess(Events event, bool auto_running)
{
  States requestedState = thisSt;

  /* Kept for compatibility with the first FSM version; thisSt is the source of truth now. */
  (void)auto_running;

  switch (event)
  {
    case end_eph_input:
    case end_manual:
      requestedState = STDBY;
      break;

    case begin_eph_input:
    case submit_eph_input:
      requestedState = EPH_INPUT;
      break;

    case begin_manual:
    case submit_manual_goto:
      requestedState = MANUAL;
      break;

    case toggle_auto_mode:
      requestedState = (thisSt == AUTO_MODE) ? STDBY : AUTO_MODE;
      break;

    default:
      requestedState = thisSt;
      break;
  }

  changeState(requestedState);

  return thisSt;
}

void changeState(States newState)
{
  /* Keep auto_on synchronized for existing web status code. */
  if (stateTransitionIsValid(thisSt, newState))
  {
    nextSt = newState;
    thisSt = nextSt;
    auto_on = (thisSt == AUTO_MODE);
  }
}

static void fsmTask(void *argument)
{
  Events event;
  TickType_t last_auto_tick = xTaskGetTickCount();

  /* FreeRTOS task signature requires this argument, even if we do not use it. */
  (void)argument;

  for (;;)
  {
    /* Wait for web/application events without burning CPU. */
    if (xQueueReceive(fsmQueue, &event, pdMS_TO_TICKS(100U)) == pdTRUE)
    {
      States previousState = thisSt;
      States currentState = fsmProcess(event, auto_on);

      if (event == submit_manual_goto)
      {
        /* HTTP has already loaded g_x_val/g_z_val; movement_task runs the blocking pulses. */
        manualModeGoto(g_x_val, g_z_val);
        requestMove();
      }
      else if (event == submit_eph_input)
      {
        /* Ephemeris mode calculates AOI and updates g_x_val/g_z_val. */
        ephInputMode();
      }

      if ((previousState != AUTO_MODE) && (currentState == AUTO_MODE))
      {
        last_auto_tick = xTaskGetTickCount();
      }
    }

    if (thisSt == AUTO_MODE)
    {
      TickType_t now = xTaskGetTickCount();

      /* Automatic mode runs periodically while the FSM remains in AUTO_MODE. */
      if ((now - last_auto_tick) >= pdMS_TO_TICKS(FSM_AUTO_PERIOD_MS))
      {
        autoMode();
        auto_counter++;
        last_auto_tick = now;
      }
    }
    else
    {
      last_auto_tick = xTaskGetTickCount();
    }
  }
}

static bool stateTransitionIsValid(States currentState, States newState)
{
  /* For this first integration all declared states are accepted. */
  (void)currentState;

  return (newState == STDBY) ||
         (newState == AUTO_MODE) ||
         (newState == MANUAL) ||
         (newState == EPH_INPUT);
}
