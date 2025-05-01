/**
 * @file main_combined.cpp
 * @brief Integrated sketch: CoAP server, motor + servo control, and config
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <coap-simple.h>
#include <MsgPack.h>
#include <ESP32Servo.h>
#include "ESC.h"

const uint8_t BOATIDSEL_PINS[] = {39, 40, 41, 42, 02, 01};
uint8_t boatID = 0xFF;

// ===== CONFIG SECTION (cfg.h) ===== //
#define AP_CHANNEL 1

#define THRUSTER_ESC_PIN (10)
#define STEERING_SERVO_PIN (9)

#define MIN_SPEED 1100
// #define MIN_ALLOWABLE_SPEED MIN_SPEED
#define MIN_ALLOWABLE_SPEED 1350
#define MAX_SPEED 1900
// #define MAX_ALLOWABLE_SPEED MAX_SPEED
#define MAX_ALLOWABLE_SPEED 1650
#define STOP_SPEED 1500

char AP_SSID[16];
char AP_PASSWORD[16];

typedef struct reqval_t
{
    uint16_t target_val;
    uint16_t curr_val;
    uint16_t min_val;
    uint16_t no_val;
    uint16_t max_val;
    uint16_t rateOfChange;
} reqval_t;

static struct Message
{
    uint8_t x;
    uint8_t y;

    MSGPACK_DEFINE(x, y);
} message;

void setBoatID()
{
    boatID = 0;
    for (int i = 0; i < 6; i++)
    {
        pinMode(BOATIDSEL_PINS[i], INPUT_PULLUP);
        digitalRead(BOATIDSEL_PINS[i]) ? boatID |= (1 << i) : boatID &= ~(1 << i);
    }
}

void setWifi()
{
    snprintf(AP_SSID, sizeof(AP_SSID), "boat%03d", boatID);
    snprintf(AP_PASSWORD, sizeof(AP_PASSWORD), "%02d%02d%02d%02d", boatID, boatID, boatID, boatID);

    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IPAddress(192, 168, boatID, 1), IPAddress(192, 168, boatID, 1), IPAddress(255, 255, 255, 0));
    WiFi.softAP(AP_SSID, AP_PASSWORD, AP_CHANNEL);
    Serial.println(AP_SSID);
    Serial.print("AP SSID: ");
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
}

// ===== GLOBAL VARIABLES ===== //
WiFiUDP udp;
Coap coap(udp);
MsgPack::Unpacker unpacker;

Servo steeringServo;
ESC thrusterEsc(THRUSTER_ESC_PIN, MIN_SPEED, MAX_SPEED, STOP_SPEED);

reqval_t esc_pwm_reqval;
reqval_t servo_pwm_reqval;

hw_timer_t *timer = NULL;

unsigned long lastCbTime = 0;
#define COAP_TIMEOUT 500 // 1 second timeout for CoAP response

// ===== VELOCITY CALLBACK ===== //
void cmdVel_cb()
{
    esc_pwm_reqval.target_val = map(message.y, 0, 255, MIN_ALLOWABLE_SPEED, MAX_ALLOWABLE_SPEED);
    servo_pwm_reqval.target_val = map(message.x, 0, 255, 10, 170);
    lastCbTime = millis(); // Update the last callback time
}

#define servo_offset -25

// ===== PWM UPDATE ===== //
void update_val()
{
    // Check if the CoAP message was received within the timeout period
    if (millis() - lastCbTime > COAP_TIMEOUT)
    {
        esc_pwm_reqval.target_val = esc_pwm_reqval.no_val;
        servo_pwm_reqval.target_val = servo_pwm_reqval.no_val;
    }

    if (esc_pwm_reqval.curr_val != esc_pwm_reqval.target_val)
    {
        esc_pwm_reqval.curr_val += (esc_pwm_reqval.curr_val > esc_pwm_reqval.target_val) ? -esc_pwm_reqval.rateOfChange : esc_pwm_reqval.rateOfChange;
        esc_pwm_reqval.curr_val = constrain(esc_pwm_reqval.curr_val, esc_pwm_reqval.min_val, esc_pwm_reqval.max_val);
    }
    thrusterEsc.speed(esc_pwm_reqval.curr_val);

    if (servo_pwm_reqval.curr_val != servo_pwm_reqval.target_val)
    {
        static uint8_t diff;
        diff = abs(servo_pwm_reqval.curr_val - servo_pwm_reqval.target_val);

        if (diff > 2 * servo_pwm_reqval.rateOfChange)
        {
            servo_pwm_reqval.curr_val += (servo_pwm_reqval.curr_val > servo_pwm_reqval.target_val) 
                ? -servo_pwm_reqval.rateOfChange 
                : servo_pwm_reqval.rateOfChange;

            servo_pwm_reqval.curr_val = constrain(
                servo_pwm_reqval.curr_val,
                servo_pwm_reqval.min_val,
                servo_pwm_reqval.max_val
            );
        }
    }

    // Apply offset and constrain the final value before writing to the servo
    static uint8_t servo_output;
    servo_output = constrain(
        servo_pwm_reqval.curr_val + servo_offset,
        servo_pwm_reqval.min_val,
        servo_pwm_reqval.max_val
    );

    steeringServo.write(servo_output);
}

void IRAM_ATTR onTimer()
{
    update_val();
}

void setupTimer()
{
    timer = timerBegin(0, 80, true); // 1us per tick
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 100000, true); // 100ms = 10Hz
    timerAlarmEnable(timer);
}

// ===== CoAP Message Handler ===== //
void handleMessage(CoapPacket &packet, IPAddress ip, int port)
{
    try
    {
        unpacker.feed(packet.payload, packet.payloadlen);
        unpacker.deserialize(message);
        Serial.printf("Received - x: %d, y: %d\r\n", message.x, message.y);
        cmdVel_cb();
    }
    catch (...)
    {
        Serial.println("Invalid sensor data!");
        coap.sendResponse(ip, port, packet.messageid, "ERR");
    }
}

// ===== SETUP ===== //
void setup()
{
    Serial.begin(115200);
    setBoatID();
    Serial.printf("Boat ID: %d\r\n", boatID);

    // esc_pwm_reqval = {STOP_SPEED, STOP_SPEED, MIN_SPEED, STOP_SPEED, MAX_SPEED, 50};
    esc_pwm_reqval = {STOP_SPEED, STOP_SPEED, MIN_SPEED, STOP_SPEED, MAX_SPEED, 10};
    servo_pwm_reqval = {90, 90, 0, 90, 180, 5};

    // Start Wi-Fi AP
    setWifi();

    // Start CoAP
    coap.server(handleMessage, "message");
    coap.start();
    Serial.println("CoAP server started");

    steeringServo.attach(STEERING_SERVO_PIN, 200, 2900);
    delay(1000);
    thrusterEsc.arm();
    delay(2000);
    thrusterEsc.speed(STOP_SPEED);

    // Start ESC/Servo control
    setupTimer();
}

// ===== LOOP ===== //
void loop()
{
    coap.loop();
}
