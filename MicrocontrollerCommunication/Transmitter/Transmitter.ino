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

#define JOYSTICK_DEADZONE 40  // Minimum change to trigger send

#define BUTTON_PIN_1 0  // Full gas/forklift up
#define BUTTON_PIN_2 1  // Half gas/forklift down
#define BUTTON_PIN_3 2  // Spin drill
#define BUTTON_PIN_4 3  // Switch modes

typedef struct {
  short x;
  short y;
  bool joystickPressed;  // Rotate in one place
  bool btn1Pressed;
  bool btn2Pressed;
  bool btn3Pressed;
  bool btn4Pressed;
  char password[6];
} DataPackage;

DataPackage lastSent = {0, 0, false, false, false, false, false, "BCIT2"};  // Keep track of last values

RF24 radio(7, 8);  // CE, CSN

const byte address[6] = "BCIT2";

void setup() {
  Serial.begin(9600);
  pinMode(ANALOG_BUTTON_PIN, INPUT_PULLUP); 
  pinMode(BUTTON_PIN_1, INPUT_PULLUP);
  pinMode(BUTTON_PIN_2, INPUT_PULLUP);
  pinMode(BUTTON_PIN_3, INPUT_PULLUP);
  pinMode(BUTTON_PIN_4, INPUT_PULLUP);
  radio.begin();
  radio.setChannel(108);
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.setRetries(15, 15);
  radio.stopListening();
}

void loop() {
  DataPackage current;
  current.x = readAnalogAxisLevel(ANALOG_X_PIN) - 128;
  current.y = readAnalogAxisLevel(ANALOG_Y_PIN) - 128;
  current.joystickPressed = isAnalogButtonPressed(ANALOG_BUTTON_PIN);
  current.btn1Pressed = digitalRead(BUTTON_PIN_1) == LOW;
  current.btn2Pressed = digitalRead(BUTTON_PIN_2) == LOW;
  current.btn3Pressed = digitalRead(BUTTON_PIN_3) == LOW;
  current.btn4Pressed = digitalRead(BUTTON_PIN_4) == LOW;
  strcpy(current.password, "BCIT2");

  if (-5 <= current.x && current.x <= 5) {
    current.x = 0;
  }
  if (-5 <= current.y && current.y <= 5) {
    current.y = 0;
  }

  // Check if joystick moved beyond deadzone or button changed
  bool xChanged = abs(current.x - lastSent.x) >= JOYSTICK_DEADZONE;
  bool yChanged = abs(current.y - lastSent.y) >= JOYSTICK_DEADZONE;
  bool btn1Changed = current.btn1Pressed != lastSent.btn1Pressed;
  bool btn2Changed = current.btn2Pressed != lastSent.btn2Pressed;
  bool btn3Changed = current.btn3Pressed != lastSent.btn3Pressed;
  bool btn4Changed = current.btn4Pressed != lastSent.btn4Pressed;
  bool joystickChanged = current.joystickPressed != lastSent.joystickPressed;

  if (xChanged || yChanged || joystickChanged
    || btn1Changed || btn2Changed || btn3Changed || btn4Changed)
  {
    bool ok = radio.write(&current, sizeof(current));
    if (!ok) {
      Serial.println("Send failed!");
    }
    lastSent = current;
  }

  delay(100);
}

byte readAnalogAxisLevel(int pin) {
  return map(analogRead(pin), 0, 1023, 0, 255);
}

bool isAnalogButtonPressed(int pin) {
  return digitalRead(pin) == 0;
}
