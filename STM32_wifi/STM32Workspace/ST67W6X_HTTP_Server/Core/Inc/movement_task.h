#ifndef MOVEMENT_TASK_H
#define MOVEMENT_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

void initMovementTask(void);

void requestMove(void);
void requestHome(void);

#ifdef __cplusplus
}
#endif

#endif /* MOVEMENT_TASK_H */
