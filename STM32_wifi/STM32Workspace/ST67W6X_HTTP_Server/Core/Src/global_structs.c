#include "global_structs.h"

SPAInputs g_SPAInputs = {0};
AOIInputs g_AOIInputs = {0};
InterpolInputs g_InterpolInputs = {0};

char g_country[32] = "Spain";
bool auto_on = false;
bool manual_time = false;
volatile int auto_counter = 0;

float g_x_val = 0.0f;
float g_z_val = 0.0f;
float g_x_target = 0.0f;
float g_z_target = 0.0f;
float g_interp_x_val = 0.0f;
float g_interp_z_val = 0.0f;
float g_query_aoit = 0.0f;
float g_query_aoil = 0.0f;
time_t g_sunrise_epoch = 0;
time_t g_sunset_epoch = 0;
float g_movement_hysteresis_gain = MOVEMENT_HYSTERESIS_DEFAULT_GAIN;
float g_movement_hysteresis_offset_mm = MOVEMENT_HYSTERESIS_DEFAULT_OFFSET_MM;
float g_vertical_movement_hysteresis_gain = VERTICAL_MOVEMENT_HYSTERESIS_DEFAULT_GAIN;
float g_vertical_movement_hysteresis_offset_mm = VERTICAL_MOVEMENT_HYSTERESIS_DEFAULT_OFFSET_MM;

volatile bool g_any_movement_alarm = false;
volatile uint32_t Vertical_top_right_alarm = 0U;
volatile uint32_t Vertical_top_left_alarm = 0U;
volatile uint32_t horizontal_interior_left_alarm = 0U;
volatile uint32_t horizontal_interior_right_alarm = 0U;
volatile uint32_t vertical_bottom_left_alarm = 0U;
volatile uint32_t vertical_bottom_right_alarm = 0U;
volatile uint32_t horizontal_exterior_left_alarm = 0U;
volatile uint32_t horizontal_exterior_right_alarm = 0U;
