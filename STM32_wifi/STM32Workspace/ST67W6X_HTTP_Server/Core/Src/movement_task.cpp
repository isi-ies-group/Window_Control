#include "movement_task.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "freertos_tickless.h"

#include "global_structs.h"
#include "movement.h"
#include "storage.h"

#define MOVEMENT_QUEUE_LENGTH      5U
#define MOVEMENT_TASK_STACK_SIZE   4096U
#define MOVEMENT_TASK_PRIORITY     22U

typedef enum
{
  CMD_NONE = 0,
  CMD_MOVE,
  CMD_HOME
} MoveCmd;

static QueueHandle_t movementQueue = NULL;
static TaskHandle_t movementTaskHandle = NULL;
/* Motors are initialized only when the first movement command is requested. */
static bool movementInitialized = false;

static void movementTask(void *argument);
static void movementPostCommand(MoveCmd cmd);

void initMovementTask(void)
{
  /*
   * What: create the movement worker and its command queue on demand.
   * How: initializes motor GPIO/timing once, then creates a FreeRTOS queue and task.
   * Why: movement pulses are blocking, so they must not run inside the HTTP/FSM task.
   */
  if (!movementInitialized)
  {
    /* GPIO/DWT motor setup is delayed so WiFi startup does not touch movement hardware. */
    init_motors();
    movementInitialized = true;
  }

  if (movementQueue == NULL)
  {
    /* The queue serializes movement commands so only one runs at a time. */
    movementQueue = xQueueCreate(MOVEMENT_QUEUE_LENGTH, sizeof(MoveCmd));
  }

  if ((movementQueue != NULL) && (movementTaskHandle == NULL))
  {
    /* FreeRTOS expects stack size in words; the project constants are kept in bytes. */
    (void)xTaskCreate(movementTask,
                      "Movement_Task",
                      MOVEMENT_TASK_STACK_SIZE >> 2U,
                      NULL,
                      MOVEMENT_TASK_PRIORITY,
                      &movementTaskHandle);
  }
}

void requestMove(void)
{
  /*
   * What: request one absolute X/Z movement.
   * How: posts CMD_MOVE after the web/FSM layer has updated g_x_val and g_z_val.
   * Why: the caller stays responsive while movement_task generates the motor pulses.
   */
  movementPostCommand(CMD_MOVE);
}

void requestHome(void)
{
  /*
   * What: request homing from any web/FSM state.
   * How: posts CMD_HOME to the movement queue.
   * Why: homing can take time and must not block WiFi request processing.
   */
  movementPostCommand(CMD_HOME);
}

static void movementTask(void *argument)
{
  /*
   * What: execute movement commands serially.
   * How: waits on the queue, runs move() or GoHomePair(), then saves the final position.
   * Why: only this task should own long motor operations and position persistence.
   */
  MoveCmd cmd;

  (void)argument;

  for (;;)
  {
    /* Sleep here until the FSM/web layer posts a movement command. */
    if (xQueueReceive(movementQueue, &cmd, portMAX_DELAY) == pdTRUE)
    {
      switch (cmd)
      {
        case CMD_MOVE:
          /* What: protect motor movement from low-power entry.
           * How: block tickless before move(), then release it when the position has been stored.
           * Why: STEP pulses must keep deterministic timing while the motors are moving.
           */
          DisableSuppressTicksAndSleep(1UL << CFG_TICKLESS_MOVEMENT_ID);
          /* move() consumes the latest absolute X/Z targets from the global state. */
          move(g_x_val, g_z_val);
          (void)savePos();
          EnableSuppressTicksAndSleep(1UL << CFG_TICKLESS_MOVEMENT_ID);
          break;

        case CMD_HOME:
          /* What: protect homing from low-power entry.
           * How: block tickless during GoHomePair() and release it after saving the home position.
           * Why: end-stop detection and STEP pulses must not be interrupted by Sleep/Stop.
           */
          DisableSuppressTicksAndSleep(1UL << CFG_TICKLESS_MOVEMENT_ID);
          /* Homing resets both software position globals to the mechanical zero. */
          GoHomePair(&g_x_val, &g_z_val);
          (void)savePos();
          EnableSuppressTicksAndSleep(1UL << CFG_TICKLESS_MOVEMENT_ID);
          break;

        case CMD_NONE:
        default:
          break;
      }
    }
  }
}

static void movementPostCommand(MoveCmd cmd)
{
  /*
   * What: send a movement command to the worker task.
   * How: lazily creates the task if needed and sends the command without blocking.
   * Why: HTTP/FSM code can request movement safely without knowing task internals.
   */
  if ((movementQueue == NULL) || (movementTaskHandle == NULL))
  {
    /* Lazy creation lets the WiFi app boot without creating movement resources. */
    initMovementTask();
  }

  if (movementQueue != NULL)
  {
    /* Non-blocking send: if the queue is full, this command is silently dropped. */
    (void)xQueueSend(movementQueue, &cmd, 0U);
  }
}
