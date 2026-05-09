/*
Links used:
- https://www.cytron.io/tutorial/getting-started-mdd3a-makerdrive-motor-driver-using-arduino
- https://howtomechatronics.com/tutorials/arduino/arduino-wireless-communication-nrf24l01-tutorial/
- https://projecthub.arduino.cc/hibit/using-joystick-module-with-arduino-0ffdd4
- https://howtomechatronics.com/how-it-works/how-servo-motors-work-how-to-control-servos-using-arduino/

See image (for joystick):
- https://components101.com/sites/default/files/inline-images/Joystick-Module-Analog-Output.png
*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h> 
#include <CytronMotorDriver.h>
#include <Servo.h>
#include <string.h>

#define MAX_SPEED 255
#define HALF_SPEED 128
#define QUARTER_SPEED 64

CytronMD motorRight(PWM_PWM, 2, 3);
CytronMD motorLeft(PWM_PWM, 4, 5);
CytronMD motorDrill(PWM_PWM, 9, 10);
CytronMD motorForklift(PWM_PWM, 11, 12);

Servo platformServo;
Servo drillServo;

bool platformTilted = false;
bool drillDown = false;

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

void move(short y, short x);
void moveForklift(short x);

void setup() {
  Serial.begin(9600);

  platformServo.attach(45);
  drillServo.attach(44);

  platformServo.write(0);
  drillServo.write(0);

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
      motorDrill.setSpeed(0);
      motorForklift.setSpeed(0);
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
      } else if (data.btn3Pressed) {
        speedLeft = QUARTER_SPEED;
        speedRight = QUARTER_SPEED;
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
      moveForklift(data.x);
      // Tilt platform
      if (data.btn1Pressed) {
        if (platformTilted) {
          platformServo.write(0);
        } else {
          platformServo.write(55);
        }
        platformTilted = !platformTilted;
      }
      // Spin drill
      if (data.btn2Pressed) {
        motorDrill.setSpeed(-HALF_SPEED);
      } else {
        motorDrill.setSpeed(0);
      }
      // Drill up/down
      if (data.btn3Pressed) {
        if (drillDown) {
          drillServo.write(0);
        } else {
          drillServo.write(180);
        }
        drillDown = !drillDown;
      }
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

void moveForklift(short x) {
  // Forklift up
  if (x > 0) {
    motorForklift.setSpeed(MAX_SPEED);
  // Forklift down
  } else if (x < 0) {
    motorForklift.setSpeed(-MAX_SPEED);
  // Stop forklift
  } else {
    motorForklift.setSpeed(0);
  }
}
