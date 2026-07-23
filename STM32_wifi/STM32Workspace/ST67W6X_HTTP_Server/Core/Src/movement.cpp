#include "movement.h"

#include <stdint.h>
#include <stdlib.h>

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "global_structs.h"
#include "movement_alarm.h"

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

/* Compensated motor-step counters; logical position stays in g_x_val/g_z_val. */
static long CurrentStep1 = 0;
static long CurrentStep2 = 0;

typedef struct
{
  volatile uint8_t yli;
  volatile uint8_t yle;
  volatile uint8_t yri;
  volatile uint8_t yre;
  volatile uint8_t ylyb;
  volatile uint8_t yleb;
  volatile uint8_t yrib;
  volatile uint8_t yreb;
  volatile uint8_t zl;
  volatile uint8_t zr;
  volatile uint8_t zli;
  volatile uint8_t zri;
} LimitSwitchState;

/*
 * What: EXTI callbacks keep the current endstop level for movement decisions.
 * How: rising edges store 1, falling edges store 0, and phase starts resync from GPIO.
 * Why: a released endstop must allow movement again without waiting for a polling read.
 */
static LimitSwitchState limitSwitchState = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};

static void BackoffAll(int steps, long speed_us);
static void SecondTouchPair(long speed_us);
static uint8_t read_limit_pin(GPIO_TypeDef *port, uint16_t pin);
static bool yli_limit_active(void);
static bool yle_limit_active(void);
static bool yri_limit_active(void);
static bool yre_limit_active(void);
static bool ylyb_limit_active(void);
static bool yleb_limit_active(void);
static bool yrib_limit_active(void);
static bool yreb_limit_active(void);
static bool zl_limit_active(void);
static bool zr_limit_active(void);
static bool zli_limit_active(void);
static bool zri_limit_active(void);
static bool vertical_top_left_limit_active(void);
static bool vertical_top_right_limit_active(void);
static bool vertical_bottom_left_limit_active(void);
static bool vertical_bottom_right_limit_active(void);
static bool vertical_limit_active(void);
static bool vertical_far_limit_active(void);
static bool horizontal_exterior_left_limit_active(void);
static bool horizontal_exterior_right_limit_active(void);
static bool horizontal_interior_left_limit_active(void);
static bool horizontal_interior_right_limit_active(void);
static bool horizontal_limit_active(void);
static bool horizontal_far_limit_active(void);
static bool all_vertical_limits_active(void);
static bool all_horizontal_limits_active(void);
static bool limit_should_stop_axis(bool moving_positive, bool released_once, bool home_limit_active, bool far_limit_active);
static void movement_alarm_rearm_inactive_vertical_limits(void);
static void movement_alarm_rearm_inactive_horizontal_limits(void);
static void movement_alarm_count_vertical_stop(void);
static void movement_alarm_count_horizontal_stop(void);

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

static long movement_compensated_target_steps(float logical_mm,
                                              long steps_per_mm,
                                              float gain,
                                              float offset_mm,
                                              float default_gain,
                                              float default_offset_mm)
{
  float compensated_mm;
  float compensated_steps;

  if (steps_per_mm <= 0)
  {
    return 0;
  }

  if (logical_mm != logical_mm)
  {
    logical_mm = 0.0f;
  }

  if (gain != gain)
  {
    gain = default_gain;
  }

  if (offset_mm != offset_mm)
  {
    offset_mm = default_offset_mm;
  }

  /*
   * What: convert a logical target into the compensated motor coordinate.
   * How: applies compensation once to the absolute target: motor_mm = target*(1+gain) - offset.
   * Why: offset/gain must not be added to every small movement or automatic mode will accumulate error.
   */
  compensated_mm = (logical_mm * (1.0f + gain)) - offset_mm;
  if (compensated_mm <= 0.0f)
  {
    return 0;
  }

  compensated_steps = compensated_mm * (float)steps_per_mm;
  return (long)(compensated_steps + 0.5f);
}

static long horizontal_compensated_target_steps(float logical_mm)
{
  return movement_compensated_target_steps(logical_mm,
                                           HORIZONTAL_STEPS_PER_MM,
                                           g_movement_hysteresis_gain,
                                           g_movement_hysteresis_offset_mm,
                                           MOVEMENT_HYSTERESIS_DEFAULT_GAIN,
                                           MOVEMENT_HYSTERESIS_DEFAULT_OFFSET_MM);
}

static long vertical_compensated_target_steps(float logical_mm)
{
  return movement_compensated_target_steps(logical_mm,
                                           VERTICAL_STEPS_PER_MM,
                                           g_vertical_movement_hysteresis_gain,
                                           g_vertical_movement_hysteresis_offset_mm,
                                           VERTICAL_MOVEMENT_HYSTERESIS_DEFAULT_GAIN,
                                           VERTICAL_MOVEMENT_HYSTERESIS_DEFAULT_OFFSET_MM);
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

    case YLYB_Pin:
      limitSwitchState.ylyb = read_limit_pin(YLYB_GPIO_Port, YLYB_Pin);
      break;

    case YLEB_Pin:
      limitSwitchState.yleb = read_limit_pin(YLEB_GPIO_Port, YLEB_Pin);
      break;

    case YRIB_Pin:
      limitSwitchState.yrib = read_limit_pin(YRIB_GPIO_Port, YRIB_Pin);
      break;

    case YREB_Pin:
      limitSwitchState.yreb = read_limit_pin(YREB_GPIO_Port, YREB_Pin);
      break;

    case ZL_Pin:
      limitSwitchState.zl = read_limit_pin(ZL_GPIO_Port, ZL_Pin);
      break;

    case ZR_Pin:
      limitSwitchState.zr = read_limit_pin(ZR_GPIO_Port, ZR_Pin);
      break;

    case ZLI_Pin:
      limitSwitchState.zli = read_limit_pin(ZLI_GPIO_Port, ZLI_Pin);
      break;

    case ZRI_Pin:
      limitSwitchState.zri = read_limit_pin(ZRI_GPIO_Port, ZRI_Pin);
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
  limitSwitchState.ylyb = read_limit_pin(YLYB_GPIO_Port, YLYB_Pin);
  limitSwitchState.yleb = read_limit_pin(YLEB_GPIO_Port, YLEB_Pin);
  limitSwitchState.yrib = read_limit_pin(YRIB_GPIO_Port, YRIB_Pin);
  limitSwitchState.yreb = read_limit_pin(YREB_GPIO_Port, YREB_Pin);
  limitSwitchState.zl = read_limit_pin(ZL_GPIO_Port, ZL_Pin);
  limitSwitchState.zr = read_limit_pin(ZR_GPIO_Port, ZR_Pin);
  limitSwitchState.zli = read_limit_pin(ZLI_GPIO_Port, ZLI_Pin);
  limitSwitchState.zri = read_limit_pin(ZRI_GPIO_Port, ZRI_Pin);
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
         ylyb_limit_active() ||
         yleb_limit_active() ||
         yrib_limit_active() ||
         yreb_limit_active() ||
         zl_limit_active() ||
         zr_limit_active() ||
         zli_limit_active() ||
         zri_limit_active();
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

/* Return the EXTI-fed state for the left/internal vertical far endstop. */
static bool ylyb_limit_active(void)
{
  return (limitSwitchState.ylyb != 0U);
}

/* Return the EXTI-fed state for the left/external vertical far endstop. */
static bool yleb_limit_active(void)
{
  return (limitSwitchState.yleb != 0U);
}

/* Return the EXTI-fed state for the right/internal vertical far endstop. */
static bool yrib_limit_active(void)
{
  return (limitSwitchState.yrib != 0U);
}

/* Return the EXTI-fed state for the right/external vertical far endstop. */
static bool yreb_limit_active(void)
{
  return (limitSwitchState.yreb != 0U);
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

/* Return the EXTI-fed state for the left horizontal far endstop. */
static bool zli_limit_active(void)
{
  return (limitSwitchState.zli != 0U);
}

/* Return the EXTI-fed state for the right horizontal far endstop. */
static bool zri_limit_active(void)
{
  return (limitSwitchState.zri != 0U);
}

/* Report whether the left vertical top/reference side is active. */
static bool vertical_top_left_limit_active(void)
{
  return yli_limit_active() ||
         yle_limit_active();
}

/* Report whether the right vertical top/reference side is active. */
static bool vertical_top_right_limit_active(void)
{
  return yri_limit_active() ||
         yre_limit_active();
}

/* Report whether the left vertical bottom/far side is active. */
static bool vertical_bottom_left_limit_active(void)
{
  return ylyb_limit_active() ||
         yleb_limit_active();
}

/* Report whether the right vertical bottom/far side is active. */
static bool vertical_bottom_right_limit_active(void)
{
  return yrib_limit_active() ||
         yreb_limit_active();
}

/* Report whether any vertical-axis endstop is active. */
static bool vertical_limit_active(void)
{
  return vertical_top_right_limit_active() ||
         vertical_top_left_limit_active();
}

/* Report whether any vertical far-side endstop is active. */
static bool vertical_far_limit_active(void)
{
  return vertical_bottom_left_limit_active() ||
         vertical_bottom_right_limit_active();
}

/* Report whether the left horizontal exterior/reference endstop is active. */
static bool horizontal_exterior_left_limit_active(void)
{
  return zl_limit_active();
}

/* Report whether the right horizontal exterior/reference endstop is active. */
static bool horizontal_exterior_right_limit_active(void)
{
  return zr_limit_active();
}

/* Report whether the left horizontal interior/far endstop is active. */
static bool horizontal_interior_left_limit_active(void)
{
  return zli_limit_active();
}

/* Report whether the right horizontal interior/far endstop is active. */
static bool horizontal_interior_right_limit_active(void)
{
  return zri_limit_active();
}

/* Report whether any horizontal-axis endstop is active. */
static bool horizontal_limit_active(void)
{
  return horizontal_exterior_left_limit_active() ||
         horizontal_exterior_right_limit_active();
}

/* Report whether any horizontal far-side endstop is active. */
static bool horizontal_far_limit_active(void)
{
  return horizontal_interior_left_limit_active() ||
         horizontal_interior_right_limit_active();
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

/* Decide if a movement must stop because the limit in that direction is active. */
static bool limit_should_stop_axis(bool moving_positive, bool released_once, bool home_limit_active, bool far_limit_active)
{
  if (moving_positive)
  {
    return far_limit_active || (home_limit_active && released_once);
  }

  return home_limit_active;
}

/* Rearm vertical alarm latches once their physical switches are released. */
static void movement_alarm_rearm_inactive_vertical_limits(void)
{
  if (!vertical_top_left_limit_active())
  {
    MovementAlarm_Update(MOVEMENT_ALARM_VERTICAL_TOP_LEFT, 0U);
  }

  if (!vertical_top_right_limit_active())
  {
    MovementAlarm_Update(MOVEMENT_ALARM_VERTICAL_TOP_RIGHT, 0U);
  }

  if (!vertical_bottom_left_limit_active())
  {
    MovementAlarm_Update(MOVEMENT_ALARM_VERTICAL_BOTTOM_LEFT, 0U);
  }

  if (!vertical_bottom_right_limit_active())
  {
    MovementAlarm_Update(MOVEMENT_ALARM_VERTICAL_BOTTOM_RIGHT, 0U);
  }
}

/* Rearm horizontal alarm latches once their physical switches are released. */
static void movement_alarm_rearm_inactive_horizontal_limits(void)
{
  if (!horizontal_exterior_left_limit_active())
  {
    MovementAlarm_Update(MOVEMENT_ALARM_HORIZONTAL_EXTERIOR_LEFT, 0U);
  }

  if (!horizontal_exterior_right_limit_active())
  {
    MovementAlarm_Update(MOVEMENT_ALARM_HORIZONTAL_EXTERIOR_RIGHT, 0U);
  }

  if (!horizontal_interior_left_limit_active())
  {
    MovementAlarm_Update(MOVEMENT_ALARM_HORIZONTAL_INTERIOR_LEFT, 0U);
  }

  if (!horizontal_interior_right_limit_active())
  {
    MovementAlarm_Update(MOVEMENT_ALARM_HORIZONTAL_INTERIOR_RIGHT, 0U);
  }
}

/* Count only vertical endstops that actually stopped a normal move(). */
static void movement_alarm_count_vertical_stop(void)
{
  MovementAlarm_Update(MOVEMENT_ALARM_VERTICAL_TOP_LEFT,
                       vertical_top_left_limit_active() ? 1U : 0U);
  MovementAlarm_Update(MOVEMENT_ALARM_VERTICAL_TOP_RIGHT,
                       vertical_top_right_limit_active() ? 1U : 0U);
  MovementAlarm_Update(MOVEMENT_ALARM_VERTICAL_BOTTOM_LEFT,
                       vertical_bottom_left_limit_active() ? 1U : 0U);
  MovementAlarm_Update(MOVEMENT_ALARM_VERTICAL_BOTTOM_RIGHT,
                       vertical_bottom_right_limit_active() ? 1U : 0U);
}

/* Count only horizontal endstops that actually stopped a normal move(). */
static void movement_alarm_count_horizontal_stop(void)
{
  MovementAlarm_Update(MOVEMENT_ALARM_HORIZONTAL_EXTERIOR_LEFT,
                       horizontal_exterior_left_limit_active() ? 1U : 0U);
  MovementAlarm_Update(MOVEMENT_ALARM_HORIZONTAL_EXTERIOR_RIGHT,
                       horizontal_exterior_right_limit_active() ? 1U : 0U);
  MovementAlarm_Update(MOVEMENT_ALARM_HORIZONTAL_INTERIOR_LEFT,
                       horizontal_interior_left_limit_active() ? 1U : 0U);
  MovementAlarm_Update(MOVEMENT_ALARM_HORIZONTAL_INTERIOR_RIGHT,
                       horizontal_interior_right_limit_active() ? 1U : 0U);
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

  /* Manual X/Z inputs are logical absolute targets; motor counters use compensated targets. */
  long targetStepsX = vertical_compensated_target_steps(xmm);
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
      movement_alarm_rearm_inactive_vertical_limits();

      if (limit_should_stop_axis(moving_positive,
                                 released_once,
                                 vertical_limit_active(),
                                 vertical_far_limit_active()))
      {
        movement_alarm_count_vertical_stop();
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

      if (limit_should_stop_axis(moving_positive,
                                 released_once,
                                 vertical_limit_active(),
                                 vertical_far_limit_active()))
      {
        movement_alarm_count_vertical_stop();
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
  long targetStepsZ = horizontal_compensated_target_steps(zmm);
  long diffZ = targetStepsZ - CurrentStep2;

  if (diffZ != 0)
  {
    long steps = abs_long(diffZ);
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

    for (long i = 0; i < steps; i++)
    {
      movement_alarm_rearm_inactive_horizontal_limits();

      if (limit_should_stop_axis(moving_positive,
                                 released_once,
                                 horizontal_limit_active(),
                                 horizontal_far_limit_active()))
      {
        movement_alarm_count_horizontal_stop();
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

      if (limit_should_stop_axis(moving_positive,
                                 released_once,
                                 horizontal_limit_active(),
                                 horizontal_far_limit_active()))
      {
        movement_alarm_count_horizontal_stop();
        break;
      }

      if ((i % 100L) == 0L)
      {
        vTaskDelay(pdMS_TO_TICKS(1));
      }
    }

    write_pin(STEP5_Port, STEP5_Pin, GPIO_PIN_RESET);
    write_pin(STEP6_Port, STEP6_Pin, GPIO_PIN_RESET);
    if (driven_steps == steps)
    {
      CurrentStep2 = targetStepsZ;
    }
    else
    {
      CurrentStep2 += moving_positive ? driven_steps : -driven_steps;
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
