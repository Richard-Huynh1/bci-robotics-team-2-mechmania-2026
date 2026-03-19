/*
Links used:
- https://howtomechatronics.com/tutorials/arduino/arduino-wireless-communication-nrf24l01-tutorial/
- https://projecthub.arduino.cc/hibit/using-joystick-module-with-arduino-0ffdd4
*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

typedef struct {
  short x = 0;
  short y = 0;
  bool joystickPressed = false;
  bool gasPressed = false;
} DataPackage;

DataPackage data;

RF24 radio(7, 8); // CE, CSN

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

    char buffer[128];
    snprintf(buffer, sizeof(buffer), "(%d, %d), joystick button pressed: %s, gas pressed: %s", data.x, data.y, data.joystickPressed ? "true" : "false", data.gasPressed ? "true" : "false");

    Serial.println(buffer);
  }
}