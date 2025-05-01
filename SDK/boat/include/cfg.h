/**
 * @file cfg.h
 * @brief Configuration and shared definitions
 */

 #ifndef _LIB_CFG
 #define _LIB_CFG
 
 #include <stdint.h>
 
 #define ID_BOAT 01
 
 // Macro stringification
 #define STR_HELPER(x) #x
 #define STR(x) STR_HELPER(x)
 
 #define AP_SSID "boat-" STR(ID_BOAT)
 #define AP_PASSWORD STR(ID_BOAT) STR(ID_BOAT) STR(ID_BOAT) STR(ID_BOAT)
 #define AP_CHANNEL 1
 
 #define THRUSTER_ESC_PIN (13)
 #define STEERING_SERVO_PIN (10)
 
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
 
 static struct Message {
     uint8_t x;
     uint8_t y;
 
     MSGPACK_DEFINE(x, y);
 } message;
 
 #endif /* _LIB_CFG */
 