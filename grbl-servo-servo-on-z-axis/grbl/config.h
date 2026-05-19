/*
  config.h - compile time configuration
  Part of Grbl

  Copyright (c) 2012-2016 Sungeun K. Jeon for Gnea Research LLC
  Copyright (c) 2009-2011 Simen Svale Skogsrud

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef config_h
#define config_h
#include "grbl.h" // For Arduino IDE compatibility.

#define DEFAULTS_GENERIC
#define CPU_MAP_ATMEGA328P // Arduino Uno CPU

// #define BAUD_RATE 230400
#define BAUD_RATE 115200

#define CMD_RESET 0x18 // ctrl-x.
#define CMD_STATUS_REPORT '?'
#define CMD_CYCLE_START '~'
#define CMD_FEED_HOLD '!'

#define CMD_SAFETY_DOOR 0x84
#define CMD_JOG_CANCEL  0x85
#define CMD_DEBUG_REPORT 0x86
#define CMD_FEED_OVR_RESET 0x90
#define CMD_FEED_OVR_COARSE_PLUS 0x91
#define CMD_FEED_OVR_COARSE_MINUS 0x92
#define CMD_FEED_OVR_FINE_PLUS  0x93
#define CMD_FEED_OVR_FINE_MINUS  0x94
#define CMD_RAPID_OVR_RESET 0x95
#define CMD_RAPID_OVR_MEDIUM 0x96
#define CMD_RAPID_OVR_LOW 0x97
#define CMD_SPINDLE_OVR_RESET 0x99
#define CMD_SPINDLE_OVR_COARSE_PLUS 0x9A
#define CMD_SPINDLE_OVR_COARSE_MINUS 0x9B
#define CMD_SPINDLE_OVR_FINE_PLUS 0x9C
#define CMD_SPINDLE_OVR_FINE_MINUS 0x9D
#define CMD_SPINDLE_OVR_STOP 0x9E
#define CMD_COOLANT_FLOOD_OVR_TOGGLE 0xA0
#define CMD_COOLANT_MIST_OVR_TOGGLE 0xA1

// CHANGED: Disabled — no limit switches on pen plotter
//#define HOMING_INIT_LOCK

#define HOMING_CYCLE_0 ((1<<X_AXIS)|(1<<Y_AXIS))
// #define HOMING_CYCLE_0 (1<<X_AXIS)
// #define HOMING_CYCLE_1 (1<<Y_AXIS)

#define N_HOMING_LOCATE_CYCLE 1

// #define HOMING_SINGLE_AXIS_COMMANDS
// #define HOMING_FORCE_SET_ORIGIN

// CHANGED: Reduced from 2 to save flash
#define N_STARTUP_LINE 1

#define N_DECIMAL_COORDVALUE_INCH 4
#define N_DECIMAL_COORDVALUE_MM   3
#define N_DECIMAL_RATEVALUE_INCH  1
#define N_DECIMAL_RATEVALUE_MM    0
#define N_DECIMAL_SETTINGVALUE    3
#define N_DECIMAL_RPMVALUE        0

// #define LIMITS_TWO_SWITCHES_ON_AXES
// #define USE_LINE_NUMBERS

// CHANGED: Disabled to save flash
//#define MESSAGE_PROBE_COORDINATES

// #define ENABLE_M7
// #define ENABLE_SAFETY_DOOR_INPUT_PIN

#define SAFETY_DOOR_SPINDLE_DELAY 4.0
#define SAFETY_DOOR_COOLANT_DELAY 1.0

// CoreXY — keep enabled
#define COREXY

// #define INVERT_CONTROL_PIN_MASK CONTROL_MASK
// #define INVERT_LIMIT_PIN_MASK ((1<<X_LIMIT_BIT)|(1<<Y_LIMIT_BIT))
// #define INVERT_SPINDLE_ENABLE_PIN
// #define INVERT_COOLANT_FLOOD_PIN
// #define INVERT_COOLANT_MIST_PIN
// #define FORCE_INITIALIZATION_ALARM

// CHANGED: Disabled to save flash
//#define CHECK_LIMITS_AT_INIT

// #define DEBUG

#define DEFAULT_FEED_OVERRIDE           100
#define MAX_FEED_RATE_OVERRIDE          200
#define MIN_FEED_RATE_OVERRIDE           10
#define FEED_OVERRIDE_COARSE_INCREMENT   10
#define FEED_OVERRIDE_FINE_INCREMENT      1

#define DEFAULT_RAPID_OVERRIDE  100
#define RAPID_OVERRIDE_MEDIUM    50
#define RAPID_OVERRIDE_LOW       25

#define DEFAULT_SPINDLE_SPEED_OVERRIDE    100
#define MAX_SPINDLE_SPEED_OVERRIDE        200
#define MIN_SPINDLE_SPEED_OVERRIDE         10
#define SPINDLE_OVERRIDE_COARSE_INCREMENT  10
#define SPINDLE_OVERRIDE_FINE_INCREMENT     1

#define RESTORE_OVERRIDES_AFTER_PROGRAM_END

// CHANGED: Disabled report fields to save flash
//#define REPORT_FIELD_BUFFER_STATE
//#define REPORT_FIELD_PIN_STATE
#define REPORT_FIELD_CURRENT_FEED_SPEED
#define REPORT_FIELD_WORK_COORD_OFFSET
// CHANGED: Disabled to save flash
//#define REPORT_FIELD_OVERRIDES
// CHANGED: Disabled to save flash
//#define REPORT_FIELD_LINE_NUMBERS

#define REPORT_OVR_REFRESH_BUSY_COUNT 20
#define REPORT_OVR_REFRESH_IDLE_COUNT 10
#define REPORT_WCO_REFRESH_BUSY_COUNT 30
#define REPORT_WCO_REFRESH_IDLE_COUNT 10

#define ACCELERATION_TICKS_PER_SECOND 100

// CHANGED: Disabled to save ~200 bytes — not needed at pen plotter speeds
//#define ADAPTIVE_MULTI_AXIS_STEP_SMOOTHING

// #define MAX_STEP_RATE_HZ 30000

//#define DISABLE_LIMIT_PIN_PULL_UP
//#define DISABLE_PROBE_PIN_PULL_UP
//#define DISABLE_CONTROL_PIN_PULL_UP

#define TOOL_LENGTH_OFFSET_AXIS Z_AXIS

// Variable spindle — required for servo control, keep enabled
#define VARIABLE_SPINDLE

// Servo Z position control — keep enabled
#define SPINDLE_RPM_CONTROLLED_BY_Z_POS

#ifdef SPINDLE_RPM_CONTROLLED_BY_Z_POS
#define Z_MM_FOR_MAX_SPINDLE_RPM -30.000
#define Z_MM_FOR_MIN_SPINDLE_RPM  30.000
#endif

// #define SPINDLE_PWM_MIN_VALUE 5
// #define USE_SPINDLE_DIR_AS_ENABLE_PIN
// #define SPINDLE_ENABLE_OFF_WITH_ZERO_SPEED
// #define REPORT_ECHO_LINE_RECEIVED

#define MINIMUM_JUNCTION_SPEED 0.0
#define MINIMUM_FEED_RATE 1.0

// CHANGED: Increased from 12 to 25 — reduces trig calls, saves flash
#define N_ARC_CORRECTION 25

#define ARC_ANGULAR_TRAVEL_EPSILON 5E-7

#define DWELL_TIME_STEP 50

// #define STEP_PULSE_DELAY 10
// #define BLOCK_BUFFER_SIZE 16
// #define SEGMENT_BUFFER_SIZE 6
// #define LINE_BUFFER_SIZE 80
// #define RX_BUFFER_SIZE 128
// #define TX_BUFFER_SIZE 100
// #define ENABLE_SOFTWARE_DEBOUNCE
// #define SET_CHECK_MODE_PROBE_TO_START
// #define HARD_LIMIT_FORCE_STATE_CHECK
// #define HOMING_AXIS_SEARCH_SCALAR  1.5
// #define HOMING_AXIS_LOCATE_SCALAR  10.0

// CHANGED: Disabled EEPROM restore commands to save flash
//#define ENABLE_RESTORE_EEPROM_WIPE_ALL
//#define ENABLE_RESTORE_EEPROM_DEFAULT_SETTINGS
//#define ENABLE_RESTORE_EEPROM_CLEAR_PARAMETERS

// CHANGED: Disabled to save flash
//#define ENABLE_BUILD_INFO_WRITE_COMMAND

#define FORCE_BUFFER_SYNC_DURING_EEPROM_WRITE

#define FORCE_BUFFER_SYNC_DURING_WCO_CHANGE

// #define ALLOW_FEED_OVERRIDE_DURING_PROBE_CYCLES
// #define PARKING_ENABLE

#define PARKING_AXIS Z_AXIS
#define PARKING_TARGET -5.0
#define PARKING_RATE 500.0
#define PARKING_PULLOUT_RATE 100.0
#define PARKING_PULLOUT_INCREMENT 5.0

// #define ENABLE_PARKING_OVERRIDE_CONTROL
// #define DEACTIVATE_PARKING_UPON_INIT

#define DISABLE_LASER_DURING_HOLD

// #define ENABLE_PIECEWISE_LINEAR_SPINDLE

#define N_PIECES 4
#define RPM_MAX  11686.4
#define RPM_MIN  202.5
#define RPM_POINT12  6145.4
#define RPM_POINT23  9627.8
#define RPM_POINT34  10813.9
#define RPM_LINE_A1  3.197101e-03
#define RPM_LINE_B1  -3.526076e-1
#define RPM_LINE_A2  1.722950e-2
#define RPM_LINE_B2  8.588176e+01
#define RPM_LINE_A3  5.901518e-02
#define RPM_LINE_B3  4.881851e+02
#define RPM_LINE_A4  1.203413e-01
#define RPM_LINE_B4  1.151360e+03

// #define ENABLE_DUAL_AXIS
#define DUAL_AXIS_SELECT  X_AXIS
#define DUAL_AXIS_HOMING_FAIL_AXIS_LENGTH_PERCENT  5.0
#define DUAL_AXIS_HOMING_FAIL_DISTANCE_MAX  25.0
#define DUAL_AXIS_HOMING_FAIL_DISTANCE_MIN  2.5
#define DUAL_AXIS_CONFIG_PROTONEER_V3_51

#endif