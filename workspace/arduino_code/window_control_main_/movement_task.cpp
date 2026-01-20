#include "movement_task.h"
#include "movement.h"
#include "global_structs.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

enum MoveCmd {
    CMD_NONE,
    CMD_MOVE,
    CMD_HOME,
    CMD_ANTIBACKLASH,
    CMD_ADJUSTZ
};

static TaskHandle_t movementTaskHandle = NULL;
static volatile MoveCmd pendingCmd = CMD_NONE;
static QueueHandle_t movementQueue;

static void movementTask(void *pvParameters) {
        
    for (;;) {
        if (xQueueReceive(movementQueue, &cmd, portMAX_DELAY) == pdTRUE) {
            MoveCmd cmd = pendingCmd;
            pendingCmd = CMD_NONE;

            switch (cmd) {
                case CMD_MOVE:
                    move(g_x_val, g_z_val);
                    break;

                case CMD_HOME:
                    GoHomePair(g_x_val, g_z_val);
                    break;

                case CMD_ANTIBACKLASH:
                    antiBacklashZ(5, 40, 750);
                    break;

                case CMD_ADJUSTZ:
                    adjustmentZ();
                    break;

                default:
                    break;
            }
        }
        else {
            vTaskDelay(1 / portTICK_PERIOD_MS);
        }
    }
}

void initMovementTask() {
    movementQueue = xQueueCreate(5, sizeof(MoveCmd));
    xTaskCreatePinnedToCore(
        movementTask,
        "movementTask",
        4096,
        NULL,
        3,
        &movementTaskHandle,
        0
    );
}

void requestMove() {
    MoveCmd cmd = CMD_MOVE;
    xQueueSend(movementQueue, &cmd, 0);
}

void requestHome() {
    MoveCmd cmd = CMD_HOME;
    xQueueSend(movementQueue, &cmd, 0);
}

void requestAntiBacklash() {
    MoveCmd cmd = CMD_ANTIBACKLASH;
    xQueueSend(movementQueue, &cmd, 0);
}

void requestAdjustZ() {
    MoveCmd cmd = CMD_ADJUSTZ;
    xQueueSend(movementQueue, &cmd, 0);
}

