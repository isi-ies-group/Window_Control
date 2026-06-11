#include "movement.h"

#include <stdint.h>
#include <stdlib.h>

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

#ifndef YLI_Pin
#define YLI_Pin YLY_Pin
#define YLI_GPIO_Port YLY_GPIO_Port
#endif

#ifndef MXRI_Z_DIR_Pin
#define MXRI_Z_DIR_Pin MXLE_Y_DIRG7_Pin
#define MXRI_Z_DIR_GPIO_Port MXLE_Y_DIRG7_GPIO_Port
#endif

/* STM32 pin aliases kept close to the original ESP32 movement numbering. */
#define STEP1_Pin        MXLI_X_STEP_Pin
#define STEP1_Port       MXLI_X_STEP_GPIO_Port
#define DIR1_Pin         MXLI_X_DIR_Pin
#define DIR1_Port        MXLI_X_DIR_GPIO_Port

#define STEP2_Pin        MXLE_Y_STEP_Pin
#define STEP2_Port       MXLE_Y_STEP_GPIO_Port
#define DIR2_Pin         MXLE_Y_DIR_Pin
#define DIR2_Port        MXLE_Y_DIR_GPIO_Port

#define STEP3_Pin        MXRI_Z_STEP_Pin
#define STEP3_Port       MXRI_Z_STEP_GPIO_Port
#define DIR3_Pin         MXRI_Z_DIR_Pin
#define DIR3_Port        MXRI_Z_DIR_GPIO_Port

#define STEP4_Pin        MXRE_A_STEP_Pin
#define STEP4_Port       MXRE_A_STEP_GPIO_Port
#define DIR4_Pin         MXRE_A_DIR_Pin
#define DIR4_Port        MXRE_A_DIR_GPIO_Port

#define STEP5_Pin        ZR_Z_STEP_Pin
#define STEP5_Port       ZR_Z_STEP_GPIO_Port
#define DIR5_Pin         ZR_Z_DIR_Pin
#define DIR5_Port        ZR_Z_DIR_GPIO_Port

#define STEP6_Pin        ZL_A_STEP_Pin
#define STEP6_Port       ZL_A_STEP_GPIO_Port
#define DIR6_Pin         ZL_A_DIR_Pin
#define DIR6_Port        ZL_A_DIR_GPIO_Port

#define ENABLE_X_Pin     Vertical_ENABLE_Pin
#define ENABLE_X_Port    Vertical_ENABLE_GPIO_Port
#define ENABLE_Z_Pin     Horizontal_ENABLE_Pin
#define ENABLE_Z_Port    Horizontal_ENABLE_GPIO_Port

/* CNC shield enables are active low: RESET enables, SET disables. */
#define ENABLE_ACTIVE     GPIO_PIN_RESET
#define ENABLE_INACTIVE   GPIO_PIN_SET

static const long HOMING_SPEED_FAST = 500;
static const long HOMING_SPEED_SLOW = 1000;
static const long MAX_X_HOMING_STEPS = 1500;
static const long MAX_Z_HOMING_STEPS = 1200;
static const long DIR_CHANGE_DELAY_US = 10000;
static const int BACKOFF_STEPS = 30;
static const long VERTICAL_STEPS_PER_MM = 25;
static const long HORIZONTAL_STEPS_PER_MM = 20;
static const long SECOND_TOUCH_EXTRA_MM = 2;
static const long MAX_VERTICAL_SECOND_TOUCH_STEPS =
  BACKOFF_STEPS + (VERTICAL_STEPS_PER_MM * SECOND_TOUCH_EXTRA_MM);
static const long MAX_HORIZONTAL_SECOND_TOUCH_STEPS =
  BACKOFF_STEPS + (HORIZONTAL_STEPS_PER_MM * SECOND_TOUCH_EXTRA_MM);
static const long Speed = 600;

/* Software position counters; they are only reliable after a valid homing cycle. */
static long CurrentStep1 = 0;
static long CurrentStep2 = 0;

typedef struct
{
  volatile uint8_t yli;
  volatile uint8_t yle;
  volatile uint8_t yri;
  volatile uint8_t yre;
  volatile uint8_t zl;
  volatile uint8_t zr;
} LimitSwitchState;

/* EXTI callbacks keep this cache updated; movement reads it without doing ISR work. */
static LimitSwitchState limitSwitchState = {0U, 0U, 0U, 0U, 0U, 0U};
static volatile uint8_t limitSwitchStateReady = 0U;

static void BackoffAll(int steps, long speed_us);
static void SecondTouchPair(long speed_us);
static void ensure_limit_switch_state(void);
static uint8_t read_limit_pin(GPIO_TypeDef *port, uint16_t pin);
static bool yli_limit_active(void);
static bool yle_limit_active(void);
static bool yri_limit_active(void);
static bool yre_limit_active(void);
static bool zl_limit_active(void);
static bool zr_limit_active(void);
static bool vertical_limit_active(void);
static bool horizontal_limit_active(void);
static bool limit_should_stop_axis(bool moving_positive, bool released_once, bool limit_active);

static void delay_us(uint32_t us)
{
  uint32_t start = DWT->CYCCNT;
  uint32_t cycles = (HAL_RCC_GetHCLKFreq() / 1000000U) * us;

  while ((DWT->CYCCNT - start) < cycles)
  {
  }
}

static void dwt_delay_init(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static long abs_long(long value)
{
  return (value < 0) ? -value : value;
}

static void write_pin(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState state)
{
  HAL_GPIO_WritePin(port, pin, state);
}

static GPIO_PinState read_pin(GPIO_TypeDef *port, uint16_t pin)
{
  return HAL_GPIO_ReadPin(port, pin);
}

static uint8_t read_limit_pin(GPIO_TypeDef *port, uint16_t pin)
{
  return (read_pin(port, pin) == GPIO_PIN_SET) ? 1U : 0U;
}

uint8_t movementLimitSwitchUpdateFromExti(uint16_t gpio_pin)
{
  switch (gpio_pin)
  {
    case YLI_Pin:
      limitSwitchState.yli = read_limit_pin(YLI_GPIO_Port, YLI_Pin);
      break;

    case YLE_Pin:
      limitSwitchState.yle = read_limit_pin(YLE_GPIO_Port, YLE_Pin);
      break;

    case YRI_Pin:
      limitSwitchState.yri = read_limit_pin(YRI_GPIO_Port, YRI_Pin);
      break;

    case YRE_Pin:
      limitSwitchState.yre = read_limit_pin(YRE_GPIO_Port, YRE_Pin);
      break;

    case ZL_Pin:
      limitSwitchState.zl = read_limit_pin(ZL_GPIO_Port, ZL_Pin);
      break;

    case ZR_Pin:
      limitSwitchState.zr = read_limit_pin(ZR_GPIO_Port, ZR_Pin);
      break;

    default:
      return 0U;
  }

  limitSwitchStateReady = 1U;
  return 1U;
}

void movementLimitSwitchRefreshAll(void)
{
  limitSwitchState.yli = read_limit_pin(YLI_GPIO_Port, YLI_Pin);
  limitSwitchState.yle = read_limit_pin(YLE_GPIO_Port, YLE_Pin);
  limitSwitchState.yri = read_limit_pin(YRI_GPIO_Port, YRI_Pin);
  limitSwitchState.yre = read_limit_pin(YRE_GPIO_Port, YRE_Pin);
  limitSwitchState.zl = read_limit_pin(ZL_GPIO_Port, ZL_Pin);
  limitSwitchState.zr = read_limit_pin(ZR_GPIO_Port, ZR_Pin);
  limitSwitchStateReady = 1U;
}

uint8_t movementAnyLimitSwitchActive(void)
{
  ensure_limit_switch_state();

  return (limitSwitchState.yli != 0U) ||
         (limitSwitchState.yle != 0U) ||
         (limitSwitchState.yri != 0U) ||
         (limitSwitchState.yre != 0U) ||
         (limitSwitchState.zl != 0U) ||
         (limitSwitchState.zr != 0U);
}

static void ensure_limit_switch_state(void)
{
  if (limitSwitchStateReady == 0U)
  {
    movementLimitSwitchRefreshAll();
  }
}

static bool yli_limit_active(void)
{
  ensure_limit_switch_state();
  return (limitSwitchState.yli != 0U);
}

static bool yle_limit_active(void)
{
  ensure_limit_switch_state();
  return (limitSwitchState.yle != 0U);
}

static bool yri_limit_active(void)
{
  ensure_limit_switch_state();
  return (limitSwitchState.yri != 0U);
}

static bool yre_limit_active(void)
{
  ensure_limit_switch_state();
  return (limitSwitchState.yre != 0U);
}

static bool zl_limit_active(void)
{
  ensure_limit_switch_state();
  return (limitSwitchState.zl != 0U);
}

static bool zr_limit_active(void)
{
  ensure_limit_switch_state();
  return (limitSwitchState.zr != 0U);
}

static bool vertical_limit_active(void)
{
  return yri_limit_active() ||
         yre_limit_active() ||
         yli_limit_active() ||
         yle_limit_active();
}

static bool horizontal_limit_active(void)
{
  return zl_limit_active() ||
         zr_limit_active();
}

static bool limit_should_stop_axis(bool moving_positive, bool released_once, bool limit_active)
{
  /* Current limit switches are homing-side limits; allow a positive move to release them. */
  return limit_active && ((!moving_positive) || released_once);
}

static void enable_vertical(bool enabled)
{
  write_pin(ENABLE_X_Port, ENABLE_X_Pin, enabled ? ENABLE_ACTIVE : ENABLE_INACTIVE);
}

static void enable_horizontal(bool enabled)
{
  write_pin(ENABLE_Z_Port, ENABLE_Z_Pin, enabled ? ENABLE_ACTIVE : ENABLE_INACTIVE);
}

static void set_vertical_dir_positive(void)
{
  write_pin(DIR1_Port, DIR1_Pin, GPIO_PIN_RESET); /* MXLI */
  write_pin(DIR2_Port, DIR2_Pin, GPIO_PIN_RESET); /* MXLE */
  write_pin(DIR3_Port, DIR3_Pin, GPIO_PIN_RESET); /* MXRI */
  write_pin(DIR4_Port, DIR4_Pin, GPIO_PIN_RESET); /* MXRE */
}

static void set_vertical_dir_negative(void)
{
  write_pin(DIR1_Port, DIR1_Pin, GPIO_PIN_SET); /* MXLI */
  write_pin(DIR2_Port, DIR2_Pin, GPIO_PIN_SET); /* MXLE */
  write_pin(DIR3_Port, DIR3_Pin, GPIO_PIN_SET); /* MXRI */
  write_pin(DIR4_Port, DIR4_Pin, GPIO_PIN_SET); /* MXRE */
}

static void set_horizontal_dir_positive(void)
{
  write_pin(DIR5_Port, DIR5_Pin, GPIO_PIN_RESET); /* ZR */
  write_pin(DIR6_Port, DIR6_Pin, GPIO_PIN_SET);   /* ZL */
}

static void set_horizontal_dir_negative(void)
{
  write_pin(DIR5_Port, DIR5_Pin, GPIO_PIN_SET);   /* ZR */
  write_pin(DIR6_Port, DIR6_Pin, GPIO_PIN_RESET); /* ZL */
}

static void set_vertical_step(GPIO_PinState state)
{
  write_pin(STEP1_Port, STEP1_Pin, state);
  write_pin(STEP2_Port, STEP2_Pin, state);
  write_pin(STEP3_Port, STEP3_Pin, state);
  write_pin(STEP4_Port, STEP4_Pin, state);
}

static void set_horizontal_step(GPIO_PinState state)
{
  write_pin(STEP5_Port, STEP5_Pin, state);
  write_pin(STEP6_Port, STEP6_Pin, state);
}

void init_motors(void)
{
  dwt_delay_init();
  movementLimitSwitchRefreshAll();

  enable_horizontal(false);
  enable_vertical(false);

  set_vertical_dir_negative();
  set_horizontal_dir_negative();

  set_vertical_step(GPIO_PIN_RESET);
  set_horizontal_step(GPIO_PIN_RESET);

  CurrentStep1 = 0;
  CurrentStep2 = 0;
}

void move(float xmm, float zmm)
{
  movementLimitSwitchRefreshAll();

  /* Manual X/Z inputs are treated as absolute position targets, not relative moves. */
  long targetStepsX = (long)(xmm * (float)VERTICAL_STEPS_PER_MM);
  long diffX = targetStepsX - CurrentStep1;

  if (diffX != 0)
  {
    long steps = abs_long(diffX);
    long moved_steps = 0;
    bool moving_positive = (diffX > 0);
    bool released_once = !vertical_limit_active();

    enable_vertical(true);

    if (moving_positive)
    {
      set_vertical_dir_positive();
    }
    else
    {
      set_vertical_dir_negative();
    }

    for (long i = 0; i < steps; i++)
    {
      if (limit_should_stop_axis(moving_positive, released_once, vertical_limit_active()))
      {
        break;
      }

      set_vertical_step(GPIO_PIN_RESET);
      delay_us((uint32_t)Speed);

      set_vertical_step(GPIO_PIN_SET);
      delay_us((uint32_t)Speed);
      moved_steps++;

      if (!vertical_limit_active())
      {
        released_once = true;
      }

      if (limit_should_stop_axis(moving_positive, released_once, vertical_limit_active()))
      {
        break;
      }

      if ((i % 100L) == 0L)
      {
        vTaskDelay(pdMS_TO_TICKS(1));
      }
    }

    set_vertical_step(GPIO_PIN_RESET);
    CurrentStep1 += moving_positive ? moved_steps : -moved_steps;
    if (moved_steps == steps)
    {
      CurrentStep1 = targetStepsX;
    }
    enable_vertical(false);
  }

  vTaskDelay(pdMS_TO_TICKS(1));

  /* The horizontal axis uses the same absolute-target model with its own counter. */
  long targetStepsZ = (long)(zmm * (float)HORIZONTAL_STEPS_PER_MM);
  long diffZ = targetStepsZ - CurrentStep2;

  if (diffZ != 0)
  {
    long steps = abs_long(diffZ);
    long moved_steps = 0;
    bool moving_positive = (diffZ > 0);
    bool released_once = !horizontal_limit_active();

    enable_horizontal(true);

    if (moving_positive)
    {
      set_horizontal_dir_positive();
    }
    else
    {
      set_horizontal_dir_negative();
    }

    for (long i = 0; i < steps; i++)
    {
      if (limit_should_stop_axis(moving_positive, released_once, horizontal_limit_active()))
      {
        break;
      }

      set_horizontal_step(GPIO_PIN_RESET);
      delay_us((uint32_t)Speed);

      set_horizontal_step(GPIO_PIN_SET);
      delay_us((uint32_t)Speed);
      moved_steps++;

      if (!horizontal_limit_active())
      {
        released_once = true;
      }

      if (limit_should_stop_axis(moving_positive, released_once, horizontal_limit_active()))
      {
        break;
      }

      if ((i % 100L) == 0L)
      {
        vTaskDelay(pdMS_TO_TICKS(1));
      }
    }

    set_horizontal_step(GPIO_PIN_RESET);
    CurrentStep2 += moving_positive ? moved_steps : -moved_steps;
    if (moved_steps == steps)
    {
      CurrentStep2 = targetStepsZ;
    }
    enable_horizontal(false);
  }
}

void GoHomePair(float *posX, float *posZ)
{
  bool xHomingReached = false;
  bool zHomingReached = false;
  long safeSteps = 0;

  movementLimitSwitchRefreshAll();

  if (yri_limit_active() &&
      yre_limit_active() &&
      yli_limit_active() &&
      yle_limit_active() &&
      zl_limit_active() &&
      zr_limit_active())
  {
    CurrentStep1 = 0;
    CurrentStep2 = 0;

    if (posX != NULL)
    {
      *posX = 0.0f;
    }

    if (posZ != NULL)
    {
      *posZ = 0.0f;
    }

    return;
  }

  if (vertical_limit_active())
  {
    xHomingReached = true;
  }

  if (horizontal_limit_active())
  {
    zHomingReached = true;
  }

  enable_vertical(true);
  enable_horizontal(true);

  set_vertical_dir_negative();
  set_horizontal_dir_negative();

  safeSteps = 0;
  while (!xHomingReached && (safeSteps < MAX_X_HOMING_STEPS))
  {
    set_vertical_step(GPIO_PIN_RESET);
    delay_us((uint32_t)Speed);

    set_vertical_step(GPIO_PIN_SET);
    delay_us((uint32_t)Speed);

    safeSteps++;

    if (vertical_limit_active())
    {
      xHomingReached = true;
    }
  }

  set_vertical_step(GPIO_PIN_RESET);

  safeSteps = 0;
  while (!zHomingReached && (safeSteps < MAX_Z_HOMING_STEPS))
  {
    set_horizontal_step(GPIO_PIN_RESET);
    delay_us((uint32_t)HOMING_SPEED_SLOW);

    set_horizontal_step(GPIO_PIN_SET);
    delay_us((uint32_t)HOMING_SPEED_SLOW);

    safeSteps++;

    if (horizontal_limit_active())
    {
      zHomingReached = true;
    }
  }

  set_horizontal_step(GPIO_PIN_RESET);

  if (xHomingReached && zHomingReached)
  {
    BackoffAll(BACKOFF_STEPS, HOMING_SPEED_SLOW);
    SecondTouchPair(HOMING_SPEED_SLOW);
  }

  enable_vertical(false);
  enable_horizontal(false);

  CurrentStep1 = 0;
  CurrentStep2 = 0;

  if (posX != NULL)
  {
    *posX = 0.0f;
  }

  if (posZ != NULL)
  {
    *posZ = 0.0f;
  }
}

static void SecondTouchPair(long speed_us)
{
  bool verticalDone = false;
  bool zLeftDone = false;
  bool zRightDone = false;
  long safeSteps = 0;

  set_vertical_dir_negative();
  set_horizontal_dir_negative();

  delay_us((uint32_t)DIR_CHANGE_DELAY_US);

  enable_vertical(true);
  enable_horizontal(true);

  while (!verticalDone && (safeSteps < MAX_VERTICAL_SECOND_TOUCH_STEPS))
  {
    bool moveMXLI;
    bool moveMXLE;
    bool moveMXRI;
    bool moveMXRE;

    if (yli_limit_active() &&
        yle_limit_active() &&
        yri_limit_active() &&
        yre_limit_active())
    {
      verticalDone = true;
      break;
    }

    moveMXLI = !yli_limit_active();
    moveMXLE = !yle_limit_active();
    moveMXRI = !yri_limit_active();
    moveMXRE = !yre_limit_active();

    set_vertical_step(GPIO_PIN_RESET);
    delay_us((uint32_t)speed_us);

    if (moveMXLI) write_pin(STEP1_Port, STEP1_Pin, GPIO_PIN_SET);
    if (moveMXLE) write_pin(STEP2_Port, STEP2_Pin, GPIO_PIN_SET);
    if (moveMXRI) write_pin(STEP3_Port, STEP3_Pin, GPIO_PIN_SET);
    if (moveMXRE) write_pin(STEP4_Port, STEP4_Pin, GPIO_PIN_SET);

    delay_us((uint32_t)speed_us);
    safeSteps++;
  }

  set_vertical_step(GPIO_PIN_RESET);

  safeSteps = 0;
  while ((!zLeftDone || !zRightDone) && (safeSteps < MAX_HORIZONTAL_SECOND_TOUCH_STEPS))
  {
    bool moveZL;
    bool moveZR;

    if (zl_limit_active()) zLeftDone = true;
    if (zr_limit_active()) zRightDone = true;

    moveZL = !zLeftDone;
    moveZR = !zRightDone;

    set_horizontal_step(GPIO_PIN_RESET);
    delay_us((uint32_t)speed_us);

    if (moveZR) write_pin(STEP5_Port, STEP5_Pin, GPIO_PIN_SET);
    if (moveZL) write_pin(STEP6_Port, STEP6_Pin, GPIO_PIN_SET);

    delay_us((uint32_t)speed_us);
    safeSteps++;
  }

  set_horizontal_dir_positive();
  delay_us((uint32_t)speed_us);

  set_horizontal_step(GPIO_PIN_SET);
  delay_us((uint32_t)speed_us);
  set_horizontal_step(GPIO_PIN_RESET);

  set_vertical_dir_positive();
  write_pin(DIR5_Port, DIR5_Pin, GPIO_PIN_RESET);
  write_pin(DIR6_Port, DIR6_Pin, GPIO_PIN_RESET);

  enable_vertical(false);
  enable_horizontal(false);
}

static void BackoffAll(int steps, long speed_us)
{
  set_vertical_dir_positive();
  set_horizontal_dir_positive();

  for (int i = 0; i < steps; i++)
  {
    set_vertical_step(GPIO_PIN_RESET);
    delay_us((uint32_t)speed_us);

    write_pin(STEP1_Port, STEP1_Pin, GPIO_PIN_SET);
    write_pin(STEP2_Port, STEP2_Pin, GPIO_PIN_RESET);
    write_pin(STEP3_Port, STEP3_Pin, GPIO_PIN_RESET);
    write_pin(STEP4_Port, STEP4_Pin, GPIO_PIN_RESET);
    delay_us((uint32_t)speed_us);

    write_pin(STEP1_Port, STEP1_Pin, GPIO_PIN_RESET);
    write_pin(STEP2_Port, STEP2_Pin, GPIO_PIN_SET);
    write_pin(STEP3_Port, STEP3_Pin, GPIO_PIN_RESET);
    write_pin(STEP4_Port, STEP4_Pin, GPIO_PIN_RESET);
    delay_us((uint32_t)speed_us);

    write_pin(STEP1_Port, STEP1_Pin, GPIO_PIN_RESET);
    write_pin(STEP2_Port, STEP2_Pin, GPIO_PIN_RESET);
    write_pin(STEP3_Port, STEP3_Pin, GPIO_PIN_SET);
    write_pin(STEP4_Port, STEP4_Pin, GPIO_PIN_RESET);
    delay_us((uint32_t)speed_us);

    write_pin(STEP1_Port, STEP1_Pin, GPIO_PIN_RESET);
    write_pin(STEP2_Port, STEP2_Pin, GPIO_PIN_RESET);
    write_pin(STEP3_Port, STEP3_Pin, GPIO_PIN_RESET);
    write_pin(STEP4_Port, STEP4_Pin, GPIO_PIN_SET);
    delay_us((uint32_t)speed_us);
  }

  set_vertical_step(GPIO_PIN_RESET);

  for (int i = 0; i < steps; i++)
  {
    set_horizontal_step(GPIO_PIN_RESET);
    delay_us((uint32_t)speed_us);

    write_pin(STEP5_Port, STEP5_Pin, GPIO_PIN_SET);
    write_pin(STEP6_Port, STEP6_Pin, GPIO_PIN_RESET);
    delay_us((uint32_t)speed_us);

    write_pin(STEP5_Port, STEP5_Pin, GPIO_PIN_RESET);
    write_pin(STEP6_Port, STEP6_Pin, GPIO_PIN_SET);
    delay_us((uint32_t)speed_us);
  }

  set_horizontal_step(GPIO_PIN_RESET);
}
