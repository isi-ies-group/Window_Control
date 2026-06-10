#include "movement_task.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "global_structs.h"
#include "movement.h"

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
  /* Manual mode posts a MOVE command after g_x_val/g_z_val have been updated. */
  movementPostCommand(CMD_MOVE);
}

void requestHome(void)
{
  /* Homing is queued so the FSM thread is not blocked by step generation. */
  movementPostCommand(CMD_HOME);
}

static void movementTask(void *argument)
{
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
          /* move() consumes the latest absolute X/Z targets from the global state. */
          move(g_x_val, g_z_val);
          break;

        case CMD_HOME:
          /* Homing resets both software position globals to the mechanical zero. */
          GoHomePair(&g_x_val, &g_z_val);
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
