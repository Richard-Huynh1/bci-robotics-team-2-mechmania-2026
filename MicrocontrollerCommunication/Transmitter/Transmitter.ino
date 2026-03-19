/*
Links used:
- https://howtomechatronics.com/tutorials/arduino/arduino-wireless-communication-nrf24l01-tutorial/
- https://projecthub.arduino.cc/hibit/using-joystick-module-with-arduino-0ffdd4
*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define ANALOG_X_PIN A2
#define ANALOG_Y_PIN A3
#define ANALOG_BUTTON_PIN A4

#define ANALOG_X_CORRECTION 128
#define ANALOG_Y_CORRECTION 128

#define JOYSTICK_DEADZONE 5  // minimum change to trigger send

#define GAS_PIN 2

typedef struct {
  short x;
  short y;
  bool joystickPressed;
  bool gasPressed;
} DataPackage;

DataPackage lastSent = {0, 0, false, false};  // Keep track of last values

RF24 radio(7, 8);  // CE, CSN

const byte address[6] = "00001";

void setup() {
  pinMode(ANALOG_BUTTON_PIN, INPUT_PULLUP); 
  pinMode(GAS_PIN, INPUT_PULLUP);
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
}

void loop() {
  DataPackage current;
  current.x = readAnalogAxisLevel(ANALOG_X_PIN) - 128;
  current.y = readAnalogAxisLevel(ANALOG_Y_PIN) - 128;
  current.joystickPressed = isAnalogButtonPressed(ANALOG_BUTTON_PIN);
  current.gasPressed = digitalRead(GAS_PIN) != HIGH;

  // Check if joystick moved beyond deadzone or button changed
  bool xChanged = abs(current.x - lastSent.x) >= JOYSTICK_DEADZONE;
  bool yChanged = abs(current.y - lastSent.y) >= JOYSTICK_DEADZONE;
  bool gasChanged = current.gasPressed != lastSent.gasPressed;
  bool joystickChanged = current.joystickPressed != lastSent.joystickPressed;

  if (xChanged || yChanged || joystickChanged || gasChanged) {
    bool ok = radio.write(&current, sizeof(current));
    if (!ok) {
      Serial.println("Send failed!");
    }
    lastSent = current;
  }

  delay(5);
}

byte readAnalogAxisLevel(int pin) {
  return map(analogRead(pin), 0, 1023, 0, 255);
}

bool isAnalogButtonPressed(int pin) {
  return digitalRead(pin) == 0;
}
