/*
Links used:
- https://howtomechatronics.com/tutorials/arduino/arduino-wireless-communication-nrf24l01-tutorial/
- https://projecthub.arduino.cc/hibit/using-joystick-module-with-arduino-0ffdd4

See image (for joystick):
- https://components101.com/sites/default/files/inline-images/Joystick-Module-Analog-Output.png
*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h> 
#include <CytronMotorDriver.h>

#define MAX_SPEED 255
#define HALF_SPEED 128

#define M1A 2
#define M1B 3
#define M2A 4
#define M2B 5

CytronMD motorRight(PWM_PWM, M1A, M1B);
CytronMD motorLeft(PWM_PWM, M2A, M2B);

typedef struct {
  short x;
  short y;
  bool joystickPressed;
  bool maxGasPressed;
  bool halfGasPressed;
  bool isCorrectSignal;
} DataPackage;

DataPackage data;

RF24 radio(7, 8); // CE, CSN

// Speeds are 0-255 PWM
int speedLeft = 255;
int speedRight = 255;
const byte address[6] = "00001";

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

void loop() {
  if (radio.available()) {
    radio.read(&data, sizeof(data));
    if (!data.isCorrectSignal) return;

    if (data.maxGasPressed) {
      speedLeft = MAX_SPEED;
      speedRight = MAX_SPEED;
      move(data.y, data.x);
    } else if (data.halfGasPressed) {
      speedLeft = HALF_SPEED;
      speedRight = HALF_SPEED;
      move(data.y, data.x);
    } else {
      speedLeft = 0;
      speedRight = 0;
      motorLeft.setSpeed(speedLeft);
      motorRight.setSpeed(speedRight);
    }

    char buffer[128];
    snprintf(buffer, sizeof(buffer),
      "(%d, %d), joystick button pressed: %s, max gas pressed: %s, half gas pressed: %s, is signal: %s",
       data.x, data.y, data.joystickPressed ? "true" : "false",
       data.maxGasPressed ? "true" : "false", data.halfGasPressed ? "true" : "false", data.isCorrectSignal ? "true" : "false"
    );

    Serial.println(buffer);
  }
}

void move(short y, short x) {
  // Quadrant 1
  if (y > 0 && x > 0) {
    speedRight *= abs(atan(x / (double)y)) / HALF_PI;
  // Quadrant 2
  } else if (y < 0 && x > 0) {
    speedLeft *= abs(atan(x / (double)y)) / HALF_PI;
  // Quadrant 3
  } else if (y < 0 && x < 0) {
    speedLeft *= -1;
    speedRight *= -1;

    speedRight *= abs(atan(x / (double)y)) / HALF_PI;
    Serial.println(speedRight);
  // Quadrant 4
  } else if (y > 0 && x < 0) {
    speedLeft *= -1;
    speedRight *= -1;

    speedLeft *= abs(atan(x / (double)y)) / HALF_PI;
    Serial.println(speedLeft);
  // On axes
  // Stationary
  } else if (y == 0 && x == 0) {
    speedLeft = 0;
    speedRight = 0;
  // Left turn
  } else if (y > 0 && x == 0) {
    speedRight = 0;
  // Right turn
  } else if (y < 0 && x == 0) {
    speedLeft = 0;
  // Reverse
  } else if (y == 0 && x < 0) {
    speedLeft *= -1;
    speedRight *= -1;
  }
  
  motorLeft.setSpeed(speedLeft);
  motorRight.setSpeed(speedRight);
}
