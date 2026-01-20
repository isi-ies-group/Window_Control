#include "movement_task.h"
#include "movement.h"
#include "global_structs.h"
#include <Arduino.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

enum MoveCmd {
    CMD_NONE,
    CMD_MOVE,
    CMD_HOME,
    CMD_ANTIBACKLASH,
    CMD_ADJUSTZ
};

typedef struct {
    MoveCmd cmd;
    float x;
    float z;
} MovementMsg;

static TaskHandle_t movementTaskHandle = NULL;
static QueueHandle_t movementQueue = NULL;

static void movementTask(void *pvParameters) {

     MovementMsg msg;

    for (;;) {
        if (xQueueReceive(movementQueue, &msg, portMAX_DELAY) == pdTRUE) {

            switch (msg.cmd) {
                case CMD_MOVE:
                    move(msg.x, msg.z);
                    g_x_val = msg.x;
                    g_z_val = msg.z;
                    Serial.print("Requested Move case: x z val:: x z msg");
                    Serial.println(g_x_val, 6);
                    Serial.println(g_z_val, 6);
                    Serial.println(msg.x, 6);
                    Serial.println(msg.z, 6);

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
    }
}

void initMovementTask() {
    movementQueue = xQueueCreate(5, sizeof(MovementMsg));

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
    MovementMsg msg;
    msg.cmd = CMD_MOVE;
    msg.x   = g_x_val;
    msg.z   = g_z_val;
    	
	Serial.print("RequestedMove: x z val:: x z msg");
	Serial.println(g_x_val, 6);
	Serial.println(g_z_val, 6);
    Serial.println(msg.x, 6);
	Serial.println(msg.z, 6);


    xQueueSend(movementQueue, &msg, portMAX_DELAY);
}

void requestHome() {
    MovementMsg msg;
    msg.cmd = CMD_HOME;
    msg.x   = g_x_val;
    msg.z   = g_z_val;
    xQueueSend(movementQueue, &msg, portMAX_DELAY);
}

void requestAntiBacklash() {
    MovementMsg msg;
    msg.cmd = CMD_ANTIBACKLASH;
    msg.x   = 0;
    msg.z   = 0;
    xQueueSend(movementQueue, &msg, portMAX_DELAY);
}


void requestAdjustZ() {
    MovementMsg msg;
    msg.cmd = CMD_ADJUSTZ;
    msg.x   = 0;
    msg.z   = 0;
    xQueueSend(movementQueue, &msg, portMAX_DELAY);
}



