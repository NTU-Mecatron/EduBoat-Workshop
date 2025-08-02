/**
 * @file boat_cfg.h
 * @author Scott CJX (scottcjx.w@gmail.com)
 * @brief
 * @version 0.1
 * @date 2025-08-02
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef _BOAT_CFG
#define _BOAT_CFG

#include <stdint.h>

#define THRUSTER_ESC_PIN (13)
#define STEERING_SERVO_PIN (10)

// reversed ID
const uint8_t BOATIDSEL_PINS[] = {01, 02, 42, 41, 40 ,39};

#define servo_offset 0
#define MIN_SPEED 1100
#define MIN_ALLOWABLE_SPEED 1350
#define MAX_SPEED 1900
#define MAX_ALLOWABLE_SPEED 1650
#define STOP_SPEED 1500

typedef struct reqval_t
{
    uint16_t target_val;
    uint16_t curr_val;
    uint16_t min_val;
    uint16_t no_val;
    uint16_t max_val;
    uint16_t rateOfChange;
} reqval_t;

#endif /* _BOAT_CFG */
