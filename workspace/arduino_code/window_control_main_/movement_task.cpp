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

static void movementTask(void *pvParameters) {
    for (;;) {
        if (pendingCmd != CMD_NONE) {
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
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

void initMovementTask() {
    xTaskCreatePinnedToCore(
        movementTask,
        "movementTask",
        4096,
        NULL,
        1,
        &movementTaskHandle,
        1
    );
}

void requestMove() {
    pendingCmd = CMD_MOVE;
}

void requestHome() {
    pendingCmd = CMD_HOME;
}

void requestAntiBacklash() {
    pendingCmd = CMD_ANTIBACKLASH;
}

void requestAdjustZ() {
    pendingCmd = CMD_ADJUSTZ;
}
