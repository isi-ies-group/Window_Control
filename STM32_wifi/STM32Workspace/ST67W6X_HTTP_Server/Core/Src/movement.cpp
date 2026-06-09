#include "movement.h"

#include <stdlib.h>

#define MOVEMENT_X_STEPS_PER_MM       25.0f
#define MOVEMENT_Z_STEPS_PER_MM       20.0f
#define MOVEMENT_HOMING_SPEED_SLOW    1000L

/* Hardware GPIO pulse generation will be added once the STM32 pinout is fixed. */
static long current_step_x = 0;
static long current_step_z = 0;

static long step_mxle = 0;
static long step_mxli = 0;
static long step_mxre = 0;
static long step_mxri = 0;
static long step_mzl = 0;
static long step_mzr = 0;

static long mm_to_steps(float mm, float steps_per_mm)
{
  float scaled = mm * steps_per_mm;

  if (scaled >= 0.0f)
  {
    return (long)(scaled + 0.5f);
  }

  return (long)(scaled - 0.5f);
}

void init_motors(void)
{
  /* Pin setup and anti-backlash startup sequence will live here after GPIO mapping. */
  current_step_x = 0;
  current_step_z = 0;
  step_mxle = 0;
  step_mxli = 0;
  step_mxre = 0;
  step_mxri = 0;
  step_mzl = 0;
  step_mzr = 0;
}

void move(float xmm, float zmm)
{
  /* Temporary position model: later this function will generate STEP/DIR pulses. */
  current_step_x = mm_to_steps(xmm, MOVEMENT_X_STEPS_PER_MM);
  current_step_z = mm_to_steps(zmm, MOVEMENT_Z_STEPS_PER_MM);
}

void GoHomePair(float *posX, float *posZ)
{
  /* Temporary homing model: real end-stop probing will be added with EXTI inputs. */
  current_step_x = 0;
  current_step_z = 0;

  if (posX != NULL)
  {
    *posX = 0.0f;
  }

  if (posZ != NULL)
  {
    *posZ = 0.0f;
  }
}

void SecondTouchPair(long speed_us)
{
  /* Placeholder for the second homing touch once limit switches are mapped. */
  (void)speed_us;
}

void BackoffAll(int steps, long speed_us)
{
  /* Placeholder for homing backoff pulses. */
  (void)steps;
  (void)speed_us;
}

void adjustmentZ(void)
{
  /* Placeholder for the initial Z mechanical adjustment sequence. */
}

void antiBacklashZ(int cycles, int steps, long speed_us)
{
  /* Placeholder for the Z backlash compensation sequence. */
  (void)cycles;
  (void)steps;
  (void)speed_us;
}

void move_external_vertical_right(float mm)
{
  step_mxre = mm_to_steps(mm, MOVEMENT_X_STEPS_PER_MM);
}

void move_external_vertical_left(float mm)
{
  step_mxle = mm_to_steps(mm, MOVEMENT_X_STEPS_PER_MM);
}

void move_internal_vertical_right(float mm)
{
  step_mxri = mm_to_steps(mm, MOVEMENT_X_STEPS_PER_MM);
}

void move_internal_vertical_left(float mm)
{
  step_mxli = mm_to_steps(mm, MOVEMENT_X_STEPS_PER_MM);
}

void move_horizontal_left(float mm)
{
  step_mzl = mm_to_steps(mm, MOVEMENT_Z_STEPS_PER_MM);
}

void move_horizontal_right(float mm)
{
  step_mzr = mm_to_steps(mm, MOVEMENT_Z_STEPS_PER_MM);
}

long computeHorizontalSteps(float delta_mm)
{
  return mm_to_steps(delta_mm, MOVEMENT_Z_STEPS_PER_MM);
}

long computeVerticalSteps(float delta_mm)
{
  return mm_to_steps(delta_mm, MOVEMENT_X_STEPS_PER_MM);
}
