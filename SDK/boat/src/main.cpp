/**
 * @file main_combined.cpp
 * @brief Integrated sketch: CoAP server, motor + servo control, and config
 */

#include <Arduino.h>
#include <ESP32Servo.h>
#include "ESC.h"

const uint8_t DIPSW_PINS[] = {01, 02, 42, 41, 40 ,39};
const uint8_t PWM_PINS[] = {14, 13, 12, 11, 10, 09};

#define MIN_SPEED 1100
#define STOP_SPEED 1500
#define MAX_SPEED 1900

#define MIN_ALLOWABLE_SPEED MIN_SPEED
#define MAX_ALLOWABLE_SPEED MAX_SPEED

ESC thrusterEscs[6];

// ===== SETUP ===== //
void setup()
{
    Serial.begin(115200);

    delay(1000);
    for (int i = 0; i < 6; i++)
    {
        thrusterEscs[i] = new ESC(PWM_PINS[i], MIN_SPEED, MAX_SPEED, STOP_SPEED);
        thrusterEscs[i].arm();
        delay(250);
    }
    delay(2000);

    for (int i = 0; i < 6; i++)
    {
        thrusterEscs[i].speed(STOP_SPEED);
    }
}

static uint8_t val = STOP_SPEED;
const uint8_t ddx = 25;
const uint8_t update_delay = 100; // 10hz

void loop()
{    
    for (;val > MIN_ALLOWABLE_SPEED; i -= ddx)
    {
        for (int i = 0; i < 6; i++)
        {
            thrusterEscs[i].speed(val);
        }
        delay(update_delay);
    }

    for (;val < STOP_SPEED; i += ddx)
    {
        for (int i = 0; i < 6; i++)
        {
            thrusterEscs[i].speed(val);
        }
        delay(update_delay);
    }

    delay(1000);
    
    for (;val < MAX_ALLOWABLE_SPEED; i += ddx)
    {
        for (int i = 0; i < 6; i++)
        {
            thrusterEscs[i].speed(val);
        }
        delay(update_delay);
    }

    for (;val > STOP_SPEED; i -= ddx)
    {
        for (int i = 0; i < 6; i++)
        {
            thrusterEscs[i].speed(val);
        }
        delay(update_delay);
    }

    delay(1000);
}
