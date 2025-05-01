#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <coap-simple.h>
#include <MsgPack.h>

#include "cfg.h"

#include <stdint.h>

#define JOY_L_X_PIN 12
#define JOY_L_Y_PIN 13
#define JOY_L_SW_PIN 14
#define JOY_R_X_PIN 18
#define JOY_R_Y_PIN 17
#define JOY_R_SW_PIN 16

static struct Message {//----------------------------------------------define you data here
  uint8_t x;
  uint8_t y;
  
  MSGPACK_DEFINE(x, y); // -----------------------------------------Field order must match
} message;

const uint8_t BOATIDSEL_PINS[] = {39, 40, 41, 42, 02, 01};
uint8_t boatID = 0xFF;
char AP_SSID[16];
char AP_PASSWORD[16];

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
}

WiFiUDP udp;
Coap coap(udp);
MsgPack::Packer packer;

void send_message() {  
  //-------------------------------------------------------------------set your data here
  // Set Message Data here

  packer.clear();
  packer.serialize(message);
  // Serial.println("Sending Message Pack");

  // Send via CoAP
  IPAddress server_ip(192, 168, boatID, 1);
  coap.put(server_ip, 5683, "message", (const char*)packer.data(), packer.size());
}

void setup() {
  Serial.begin(115200);
  setBoatID();
  
  // // Connect to AP
  setWifi();
  WiFi.begin(AP_SSID, AP_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to AP");

  pinMode(JOY_L_Y_PIN, INPUT);
  pinMode(JOY_R_X_PIN, INPUT);
}

uint16_t x, y;

void loop() {
  x = analogRead(JOY_R_X_PIN); // Read joystick Y-axis value
  y = analogRead(JOY_L_Y_PIN); // Read joystick X-axis value
  message.x = map(x, 0, 4095, 0, 255); // Map to 0-255 range
  message.y = map(y, 0, 4095, 0, 255); // Map to 0-255 range
  
  send_message();
  delay(25);
}
