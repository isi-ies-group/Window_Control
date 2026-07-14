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

/* MXRI direction must come from CubeMX; fail loudly if the GPIO label is lost. */
#ifndef MXRI_Z_DIR_Pin
#error "MXRI_Z_DIR_Pin is missing. Check the MXRI_Z_DIR GPIO label in the .ioc file."
#endif

#ifndef MXRI_Z_DIR_GPIO_Port
#error "MXRI_Z_DIR_GPIO_Port is missing. Check the MXRI_Z_DIR GPIO label in the .ioc file."
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
static const long DIR_CHANGE_DELAY_US = 10000;
static const int BACKOFF_STEPS = 30;
static const long VERTICAL_STEPS_PER_MM = 25;
static const long HORIZONTAL_STEPS_PER_MM = 20;
static const long HORIZONTAL_HYSTERESIS_REFERENCE_MM = 75;
static const long HORIZONTAL_HYSTERESIS_EXTRA_MM = 4;
static const long FIRST_TOUCH_EXTRA_MM = 30;
static const long BASE_MAX_X_HOMING_STEPS = 1500;
static const long BASE_MAX_Z_HOMING_STEPS = 1200;
static const long MAX_X_HOMING_STEPS =
  BASE_MAX_X_HOMING_STEPS + (VERTICAL_STEPS_PER_MM * FIRST_TOUCH_EXTRA_MM);
static const long MAX_Z_HOMING_STEPS =
  BASE_MAX_Z_HOMING_STEPS + (HORIZONTAL_STEPS_PER_MM * FIRST_TOUCH_EXTRA_MM);
static const long SECOND_TOUCH_EXTRA_MM = 70;
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

/*
 * What: EXTI callbacks keep the current endstop level for movement decisions.
 * How: rising edges store 1, falling edges store 0, and phase starts resync from GPIO.
 * Why: a released endstop must allow movement again without waiting for a polling read.
 */
static LimitSwitchState limitSwitchState = {0U, 0U, 0U, 0U, 0U, 0U};

static void BackoffAll(int steps, long speed_us);
static void SecondTouchPair(long speed_us);
static uint8_t read_limit_pin(GPIO_TypeDef *port, uint16_t pin);
static bool yli_limit_active(void);
static bool yle_limit_active(void);
static bool yri_limit_active(void);
static bool yre_limit_active(void);
static bool zl_limit_active(void);
static bool zr_limit_active(void);
static bool vertical_limit_active(void);
static bool horizontal_limit_active(void);
static bool all_vertical_limits_active(void);
static bool all_horizontal_limits_active(void);
static bool limit_should_stop_axis(bool moving_positive, bool released_once, bool limit_active);

/* Busy-wait for short motor pulse delays using the DWT cycle counter. */
static void delay_us(uint32_t us)
{
  uint32_t start = DWT->CYCCNT;
  uint32_t cycles = (HAL_RCC_GetHCLKFreq() / 1000000U) * us;

  while ((DWT->CYCCNT - start) < cycles)
  {
  }
}

/* Enable the DWT cycle counter used by delay_us(). */
static void dwt_delay_init(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/* Return the absolute value of a signed long without pulling in extra helpers. */
static long abs_long(long value)
{
  return (value < 0) ? -value : value;
}

static long horizontal_hysteresis_extra_steps(long logical_steps)
{
  const long reference_steps = HORIZONTAL_STEPS_PER_MM * HORIZONTAL_HYSTERESIS_REFERENCE_MM;
  const long extra_steps_at_reference = HORIZONTAL_STEPS_PER_MM * HORIZONTAL_HYSTERESIS_EXTRA_MM;

  if ((logical_steps <= 0) || (reference_steps <= 0))
  {
    return 0;
  }

  return ((logical_steps * extra_steps_at_reference) + (reference_steps / 2L)) / reference_steps;
}

float movementClampHorizontalTarget(float zmm)
{
  if (zmm < 0.0f)
  {
    return 0.0f;
  }

  if (zmm > MOVEMENT_HORIZONTAL_MAX_MM)
  {
    return MOVEMENT_HORIZONTAL_MAX_MM;
  }

  return zmm;
}

/* Small wrapper around HAL_GPIO_WritePin() to keep movement code compact. */
static void write_pin(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState state)
{
  HAL_GPIO_WritePin(port, pin, state);
}

/* Small wrapper around HAL_GPIO_ReadPin() to keep limit reads consistent. */
static GPIO_PinState read_pin(GPIO_TypeDef *port, uint16_t pin)
{
  return HAL_GPIO_ReadPin(port, pin);
}

/* Convert an active-high endstop GPIO level into a cached boolean value. */
static uint8_t read_limit_pin(GPIO_TypeDef *port, uint16_t pin)
{
  return (read_pin(port, pin) == GPIO_PIN_SET) ? 1U : 0U;
}

/*
 * What: update the endstop state touched by an EXTI edge.
 * How: samples the GPIO level, so rising marks active and falling marks released.
 * Why: the movement task can stop or re-allow pulses from the latest interrupt state.
 */
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

  return 1U;
}

/*
 * What: seed the EXTI cache with the real electrical state of all endstops.
 * How: reads every GPIO directly before a movement phase starts.
 * Why: if a switch is already pressed before motion begins, no new rising edge will occur.
 */
void movementLimitSwitchRefreshAll(void)
{
  limitSwitchState.yli = read_limit_pin(YLI_GPIO_Port, YLI_Pin);
  limitSwitchState.yle = read_limit_pin(YLE_GPIO_Port, YLE_Pin);
  limitSwitchState.yri = read_limit_pin(YRI_GPIO_Port, YRI_Pin);
  limitSwitchState.yre = read_limit_pin(YRE_GPIO_Port, YRE_Pin);
  limitSwitchState.zl = read_limit_pin(ZL_GPIO_Port, ZL_Pin);
  limitSwitchState.zr = read_limit_pin(ZR_GPIO_Port, ZR_Pin);
}

/* Report whether at least one cached endstop input is currently active. */
uint8_t movementAnyLimitSwitchActive(void)
{
  /*
   * What: report if any endstop is active for debug/LED feedback.
   * How: uses the same EXTI-fed state that movement uses.
   * Why: the LED must reflect the current interrupt state, including falling-edge releases.
   */
  return yli_limit_active() ||
         yle_limit_active() ||
         yri_limit_active() ||
         yre_limit_active() ||
         zl_limit_active() ||
         zr_limit_active();
}

/* Return the EXTI-fed state for the left/internal vertical endstop. */
static bool yli_limit_active(void)
{
  return (limitSwitchState.yli != 0U);
}

/* Return the EXTI-fed state for the left/external vertical endstop. */
static bool yle_limit_active(void)
{
  return (limitSwitchState.yle != 0U);
}

/* Return the EXTI-fed state for the right/internal vertical endstop. */
static bool yri_limit_active(void)
{
  return (limitSwitchState.yri != 0U);
}

/* Return the EXTI-fed state for the right/external vertical endstop. */
static bool yre_limit_active(void)
{
  return (limitSwitchState.yre != 0U);
}

/* Return the EXTI-fed state for the left horizontal endstop. */
static bool zl_limit_active(void)
{
  return (limitSwitchState.zl != 0U);
}

/* Return the EXTI-fed state for the right horizontal endstop. */
static bool zr_limit_active(void)
{
  return (limitSwitchState.zr != 0U);
}

/* Report whether any vertical-axis endstop is active. */
static bool vertical_limit_active(void)
{
  return yri_limit_active() ||
         yre_limit_active() ||
         yli_limit_active() ||
         yle_limit_active();
}

/* Report whether any horizontal-axis endstop is active. */
static bool horizontal_limit_active(void)
{
  return zl_limit_active() ||
         zr_limit_active();
}

/* Report whether every vertical motor has reached its own homing endstop. */
static bool all_vertical_limits_active(void)
{
  return yli_limit_active() &&
         yle_limit_active() &&
         yri_limit_active() &&
         yre_limit_active();
}

/* Report whether both horizontal skates have reached their own homing endstop. */
static bool all_horizontal_limits_active(void)
{
  return zl_limit_active() &&
         zr_limit_active();
}

/* Decide if a movement must stop because its homing-side limit is active. */
static bool limit_should_stop_axis(bool moving_positive, bool released_once, bool limit_active)
{
  /* Current limit switches are homing-side limits; allow a positive move to release them. */
  return limit_active && ((!moving_positive) || released_once);
}

/* Enable or disable all vertical motors through the active-low enable pin. */
static void enable_vertical(bool enabled)
{
  write_pin(ENABLE_X_Port, ENABLE_X_Pin, enabled ? ENABLE_ACTIVE : ENABLE_INACTIVE);
}

/* Enable or disable all horizontal motors through the active-low enable pin. */
static void enable_horizontal(bool enabled)
{
  write_pin(ENABLE_Z_Port, ENABLE_Z_Pin, enabled ? ENABLE_ACTIVE : ENABLE_INACTIVE);
}

/* Set vertical motor directions for movement away from the homing side. */
static void set_vertical_dir_positive(void)
{
  write_pin(DIR1_Port, DIR1_Pin, GPIO_PIN_RESET); /* MXLI */
  write_pin(DIR2_Port, DIR2_Pin, GPIO_PIN_RESET); /* MXLE */
  write_pin(DIR3_Port, DIR3_Pin, GPIO_PIN_RESET); /* MXRI */
  write_pin(DIR4_Port, DIR4_Pin, GPIO_PIN_RESET); /* MXRE */
}

/* Set vertical motor directions for movement toward the homing side. */
static void set_vertical_dir_negative(void)
{
  write_pin(DIR1_Port, DIR1_Pin, GPIO_PIN_SET); /* MXLI */
  write_pin(DIR2_Port, DIR2_Pin, GPIO_PIN_SET); /* MXLE */
  write_pin(DIR3_Port, DIR3_Pin, GPIO_PIN_SET); /* MXRI */
  write_pin(DIR4_Port, DIR4_Pin, GPIO_PIN_SET); /* MXRE */
}

/* Set horizontal motor directions for movement away from the homing side. */
static void set_horizontal_dir_positive(void)
{
  write_pin(DIR5_Port, DIR5_Pin, GPIO_PIN_RESET); /* ZR */
  write_pin(DIR6_Port, DIR6_Pin, GPIO_PIN_SET);   /* ZL */
}

/* Set horizontal motor directions for movement toward the homing side. */
static void set_horizontal_dir_negative(void)
{
  write_pin(DIR5_Port, DIR5_Pin, GPIO_PIN_SET);   /* ZR */
  write_pin(DIR6_Port, DIR6_Pin, GPIO_PIN_RESET); /* ZL */
}

/* Drive all vertical step pins to the same level. */
static void set_vertical_step(GPIO_PinState state)
{
  write_pin(STEP1_Port, STEP1_Pin, state);
  write_pin(STEP2_Port, STEP2_Pin, state);
  write_pin(STEP3_Port, STEP3_Pin, state);
  write_pin(STEP4_Port, STEP4_Pin, state);
}

/* Drive both horizontal step pins to the same level. */
static void set_horizontal_step(GPIO_PinState state)
{
  write_pin(STEP5_Port, STEP5_Pin, state);
  write_pin(STEP6_Port, STEP6_Pin, state);
}

/* Prepare DWT timing, disable drivers and reset software position counters. */
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

/* Move to absolute X/Z targets while stopping each axis on active endstops. */
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

      /*
       * What: generate one manual-move STEP pulse for the four vertical drivers.
       * How: force STEP1-STEP4 low, wait, then force STEP1-STEP4 high in the same cycle.
       * Why: explicit per-pin writes make the manual pulse train easier to verify on hardware.
       */
      write_pin(STEP1_Port, STEP1_Pin, GPIO_PIN_RESET);
      write_pin(STEP2_Port, STEP2_Pin, GPIO_PIN_RESET);
      write_pin(STEP3_Port, STEP3_Pin, GPIO_PIN_RESET);
      write_pin(STEP4_Port, STEP4_Pin, GPIO_PIN_RESET);
      delay_us((uint32_t)Speed);

      write_pin(STEP1_Port, STEP1_Pin, GPIO_PIN_SET);
      write_pin(STEP2_Port, STEP2_Pin, GPIO_PIN_SET);
      write_pin(STEP3_Port, STEP3_Pin, GPIO_PIN_SET);
      write_pin(STEP4_Port, STEP4_Pin, GPIO_PIN_SET);
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

    write_pin(STEP1_Port, STEP1_Pin, GPIO_PIN_RESET);
    write_pin(STEP2_Port, STEP2_Pin, GPIO_PIN_RESET);
    write_pin(STEP3_Port, STEP3_Pin, GPIO_PIN_RESET);
    write_pin(STEP4_Port, STEP4_Pin, GPIO_PIN_RESET);
    CurrentStep1 += moving_positive ? moved_steps : -moved_steps;
    if (moved_steps == steps)
    {
      CurrentStep1 = targetStepsX;
    }
    enable_vertical(false);
  }

  vTaskDelay(pdMS_TO_TICKS(1));

  /* The horizontal axis uses the same absolute-target model with its own counter. */
  zmm = movementClampHorizontalTarget(zmm);
  long targetStepsZ = (long)(zmm * (float)HORIZONTAL_STEPS_PER_MM);
  long diffZ = targetStepsZ - CurrentStep2;

  if (diffZ != 0)
  {
    long steps = abs_long(diffZ);
    long drive_steps = steps + horizontal_hysteresis_extra_steps(steps);
    long driven_steps = 0;
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

    for (long i = 0; i < drive_steps; i++)
    {
      if (limit_should_stop_axis(moving_positive, released_once, horizontal_limit_active()))
      {
        break;
      }

      /*
       * What: generate one manual-move STEP pulse for the two horizontal drivers.
       * How: force STEP5-STEP6 low, wait, then force STEP5-STEP6 high in the same cycle.
       * Why: keeps the manual X/Z pulse style identical and easy to probe.
       */
      write_pin(STEP5_Port, STEP5_Pin, GPIO_PIN_RESET);
      write_pin(STEP6_Port, STEP6_Pin, GPIO_PIN_RESET);
      delay_us((uint32_t)Speed);

      write_pin(STEP5_Port, STEP5_Pin, GPIO_PIN_SET);
      write_pin(STEP6_Port, STEP6_Pin, GPIO_PIN_SET);
      delay_us((uint32_t)Speed);
      driven_steps++;

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

    write_pin(STEP5_Port, STEP5_Pin, GPIO_PIN_RESET);
    write_pin(STEP6_Port, STEP6_Pin, GPIO_PIN_RESET);
    if (driven_steps == drive_steps)
    {
      CurrentStep2 = targetStepsZ;
    }
    else
    {
      long logical_steps_done = (driven_steps < steps) ? driven_steps : steps;
      CurrentStep2 += moving_positive ? logical_steps_done : -logical_steps_done;
    }
    enable_horizontal(false);
  }
}

/*
 * What: run a complete homing cycle and reset software position to zero.
 * How: first touch stops each whole axis when any endstop on that axis triggers,
 *      backs off, then second touch homes each motor/patin independently by EXTI state.
 * Why: the first touch finds the reference area safely, and the second touch avoids
 *      pushing a switch that has already been reached.
 */
void GoHomePair(float *posX, float *posZ)
{
  bool xHomingReached = false;
  bool zHomingReached = false;
  long safeSteps = 0;

  movementLimitSwitchRefreshAll();

  if (all_vertical_limits_active() && all_horizontal_limits_active())
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

  xHomingReached = vertical_limit_active();
  zHomingReached = horizontal_limit_active();

  enable_vertical(true);
  enable_horizontal(true);

  set_vertical_dir_negative();
  set_horizontal_dir_negative();

  safeSteps = 0;
  while (!xHomingReached && (safeSteps < MAX_X_HOMING_STEPS))
  {
    /*
     * What: first vertical touch.
     * How: all vertical motors pulse together until any vertical endstop interrupt is active.
     * Why: this coarse pass only finds the home area before the precise second touch.
     */
    if (vertical_limit_active())
    {
      xHomingReached = true;
      break;
    }

    set_vertical_step(GPIO_PIN_RESET);
    delay_us((uint32_t)HOMING_SPEED_SLOW);

    set_vertical_step(GPIO_PIN_SET);

    delay_us((uint32_t)HOMING_SPEED_SLOW);

    safeSteps++;

    if (vertical_limit_active())
    {
      xHomingReached = true;
    }

    if ((safeSteps % 100L) == 0L)
    {
      vTaskDelay(pdMS_TO_TICKS(1));
    }
  }

  set_vertical_step(GPIO_PIN_RESET);
  if (xHomingReached)
  {
    enable_vertical(false);
  }

  safeSteps = 0;
  while (!zHomingReached && (safeSteps < MAX_Z_HOMING_STEPS))
  {
    /*
     * What: first horizontal touch.
     * How: both horizontal patins pulse together until either horizontal endstop is active.
     * Why: this coarse pass matches the ESP32 behavior before the precise second touch.
     */
    if (horizontal_limit_active())
    {
      zHomingReached = true;
      break;
    }

    set_horizontal_step(GPIO_PIN_RESET);
    delay_us((uint32_t)HOMING_SPEED_SLOW);

    set_horizontal_step(GPIO_PIN_SET);

    delay_us((uint32_t)HOMING_SPEED_SLOW);

    safeSteps++;

    if (horizontal_limit_active())
    {
      zHomingReached = true;
    }

    if ((safeSteps % 100L) == 0L)
    {
      vTaskDelay(pdMS_TO_TICKS(1));
    }
  }

  set_horizontal_step(GPIO_PIN_RESET);
  if (zHomingReached)
  {
    enable_horizontal(false);
  }

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

/*
 * What: perform the slow second touch after backing off the switches.
 * How: each loop reads every endstop before each STEP pulse and only pulses motors
 *      whose own endstop is still inactive.
 * Why: the final reference must be gentle and independent so an active switch cannot
 *      be pushed again while another motor is still searching.
 */
static void SecondTouchPair(long speed_us)
{
  bool verticalDone = false;
  long safeSteps = 0;

  movementLimitSwitchRefreshAll();

  set_vertical_dir_negative();
  set_horizontal_dir_negative();

  delay_us((uint32_t)DIR_CHANGE_DELAY_US);

  enable_vertical(true);

  while (!verticalDone && (safeSteps < MAX_VERTICAL_SECOND_TOUCH_STEPS))
  {
    bool moveMXLI;
    bool moveMXLE;
    bool moveMXRI;
    bool moveMXRE;

    movementLimitSwitchRefreshAll();

    if (all_vertical_limits_active())
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
  enable_vertical(false);

  enable_horizontal(true);
  safeSteps = 0;
  while (safeSteps < MAX_HORIZONTAL_SECOND_TOUCH_STEPS)
  {
    bool moveZL;
    bool moveZR;

    movementLimitSwitchRefreshAll();

    if (all_horizontal_limits_active())
    {
      break;
    }

    /*
     * What: second horizontal touch with falling-edge recovery.
     * How: each patin is pulsed only while its current EXTI state says "not pressed".
     * Why: if a switch releases or bounces low, the next pulse is allowed again.
     */
    moveZL = !zl_limit_active();
    moveZR = !zr_limit_active();

    if (!moveZL && !moveZR)
    {
      break;
    }

    set_horizontal_step(GPIO_PIN_RESET);
    delay_us((uint32_t)speed_us);

    if (moveZR) write_pin(STEP5_Port, STEP5_Pin, GPIO_PIN_SET);
    if (moveZL) write_pin(STEP6_Port, STEP6_Pin, GPIO_PIN_SET);

    delay_us((uint32_t)speed_us);
    safeSteps++;
  }

  set_horizontal_step(GPIO_PIN_RESET);
  set_vertical_step(GPIO_PIN_RESET);

  enable_horizontal(false);
}

/*
 * What: move all axes away from the endstops before the second touch.
 * How: all motors in the same axis receive simultaneous STEP pulses while directions
 *      are set away from home.
 * Why: simultaneous backoff keeps left/right and ZL/ZR aligned instead of advancing
 *      one motor more than the others during the release movement.
 */
static void BackoffAll(int steps, long speed_us)
{
  enable_vertical(true);
  enable_horizontal(true);

  set_vertical_dir_positive();
  set_horizontal_dir_positive();

  for (int i = 0; i < steps; i++)
  {
    set_vertical_step(GPIO_PIN_RESET);
    delay_us((uint32_t)speed_us);

    set_vertical_step(GPIO_PIN_SET);
    delay_us((uint32_t)speed_us);
  }

  set_vertical_step(GPIO_PIN_RESET);

  for (int i = 0; i < steps; i++)
  {
    set_horizontal_step(GPIO_PIN_RESET);
    delay_us((uint32_t)speed_us);

    set_horizontal_step(GPIO_PIN_SET);
    delay_us((uint32_t)speed_us);
  }

  set_horizontal_step(GPIO_PIN_RESET);

  enable_vertical(false);
  enable_horizontal(false);
  movementLimitSwitchRefreshAll();
}
