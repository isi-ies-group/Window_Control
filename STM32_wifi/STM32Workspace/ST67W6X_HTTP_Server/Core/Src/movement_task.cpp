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
  CMD_HOME,
  CMD_ANTIBACKLASH,
  CMD_ADJUSTZ
} MoveCmd;

static QueueHandle_t movementQueue = NULL;
static TaskHandle_t movementTaskHandle = NULL;

static void movementTask(void *argument);
static void movementPostCommand(MoveCmd cmd);

void initMovementTask(void)
{
  init_motors();

  if (movementQueue == NULL)
  {
    movementQueue = xQueueCreate(MOVEMENT_QUEUE_LENGTH, sizeof(MoveCmd));
  }

  if ((movementQueue != NULL) && (movementTaskHandle == NULL))
  {
    /* ST stack sizes are normally documented in bytes; FreeRTOS expects words. */
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
  movementPostCommand(CMD_MOVE);
}

void requestHome(void)
{
  movementPostCommand(CMD_HOME);
}

void requestAntiBacklash(void)
{
  movementPostCommand(CMD_ANTIBACKLASH);
}

void requestAdjustZ(void)
{
  movementPostCommand(CMD_ADJUSTZ);
}

static void movementTask(void *argument)
{
  MoveCmd cmd;

  (void)argument;

  for (;;)
  {
    if (xQueueReceive(movementQueue, &cmd, portMAX_DELAY) == pdTRUE)
    {
      switch (cmd)
      {
        case CMD_MOVE:
          move(g_x_val, g_z_val);
          break;

        case CMD_HOME:
          GoHomePair(&g_x_val, &g_z_val);
          break;

        case CMD_ANTIBACKLASH:
          antiBacklashZ(5, 40, 750);
          break;

        case CMD_ADJUSTZ:
          adjustmentZ();
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
  if (movementQueue != NULL)
  {
    (void)xQueueSend(movementQueue, &cmd, 0U);
  }
}
