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
#include <string.h>

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
  bool btn1Pressed;
  bool btn2Pressed;
  bool btn3Pressed;
  bool btn4Pressed;
  char password[6];
} DataPackage;

DataPackage data;
bool isDriving = true;

RF24 radio(7, 8); // CE, CSN

// Speeds are 0-255 PWM
int speedLeft = 255;
int speedRight = 255;
const byte address[6] = "BCIT2";

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.setChannel(108);
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

void loop() {
  if (radio.available()) {
    radio.read(&data, sizeof(data));
    if (strcmp(data.password, "BCIT2")) return;

    if (data.btn4Pressed) {
      isDriving = !isDriving;
      speedLeft = 0;
      speedRight = 0;
      motorLeft.setSpeed(speedLeft);
      motorRight.setSpeed(speedRight);
    }

    if (isDriving) {
      if (data.btn1Pressed) {
        speedLeft = MAX_SPEED;
        speedRight = MAX_SPEED;
        move(data.y, data.x);
      } else if (data.btn2Pressed) {
        speedLeft = HALF_SPEED;
        speedRight = HALF_SPEED;
        move(data.y, data.x);
      } else if (data.joystickPressed) {
        speedLeft = HALF_SPEED;
        speedRight = -HALF_SPEED;
        motorLeft.setSpeed(speedLeft);
        motorRight.setSpeed(speedRight);
      } else {
        speedLeft = 0;
        speedRight = 0;
        motorLeft.setSpeed(speedLeft);
        motorRight.setSpeed(speedRight);
      }
    } else {
      if (data.btn1Pressed) {
        // TODO: Forklift up.
      } else if (data.btn2Pressed) {
        // TODO: Forklift down.
      }
    }

    if (data.btn3Pressed) {
      // TODO: Spin drill.
    }
    

    char buffer[128];
    snprintf(buffer, sizeof(buffer),
      "(%d, %d), joystick pressed: %s, 1: %s, 2: %s, 3: %s, 4: %s, password: %s",
       data.x, data.y, data.joystickPressed ? "true" : "false",
       data.btn1Pressed ? "true" : "false", data.btn2Pressed ? "true" : "false",
       data.btn3Pressed ? "true" : "false", data.btn4Pressed ? "true" : "false",
       data.password
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
