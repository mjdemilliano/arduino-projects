#include <CircularBuffer.h>
#include <BluetoothCommandParser.h>
#include <SoftwareSerial.h>

#define MOTOR_PIN_A 9
#define MOTOR_PIN_B 10

#define BLUETOOTH_RX_PIN 3
#define BLUETOOTH_TX_PIN 4

#define STEP_TIME 1000
#define STEP_SIZE 20
#define MAX_SPEED 250

#define LED_PIN 13

int speed = 0; // negative value means running backwards. maximum is 255
unsigned long checkpointTime = STEP_TIME;
int direction = 1;
int active = 1;
int ledStatus = 0;
BluetoothCommandParser btcmdparser;

SoftwareSerial bluetoothSerial(BLUETOOTH_RX_PIN, BLUETOOTH_TX_PIN);

void setup() {
  Serial.begin(19200);
  pinMode(MOTOR_PIN_A, OUTPUT);
  pinMode(MOTOR_PIN_B, OUTPUT);
  pinMode(
}

void setSpeed(int speed) {
  if (speed >= 0) {
    analogWrite(MOTOR_PIN_A, speed);
    digitalWrite(MOTOR_PIN_B, 0);
  } else {
    digitalWrite(MOTOR_PIN_A, 0);
    analogWrite(MOTOR_PIN_B, -speed);
  }
}

void stopMotor() {
  analogWrite(MOTOR_PIN_A, 0);
  analogWrite(MOTOR_PIN_B, 0);
}

void loop() {
  if (millis() > checkpointTime) {
    ledStatus = 1 - ledStatus;
    digitalWrite(LED_PIN, ledStatus);
  
    checkpointTime += STEP_TIME;
    speed = speed + direction * STEP_SIZE;
    if (direction == 1) {
      if (speed > MAX_SPEED) {
        direction = -1;
        speed -= STEP_SIZE;
      }
    } else if (direction == -1) {
      if (speed < 0) {
        direction = 1;
        speed += STEP_SIZE;
      }
    }
    setSpeed(speed);
    Serial.print(speed);
    Serial.print("\n");
  }
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'f') {
      speed = min(speed + 25, 250);
      setSpeed(speed);
    } else if (c == 'b') {
      speed = max(speed - 25, -250);
      setSpeed(speed);
    } else if (c == 'r') {
      direction = -1 * direction;
    } else if (c == 's') {
      setSpeed(speed);
    } else {
      stopMotor();
    }
    Serial.print(speed);
    Serial.print("\n");
    // If going forward, B = low, and A = high means that the motor moves, so low PWM duty cycle means
    // lower speed.
    // If going backward, B = high, and A = high means that the motor stops, so high PWM duty cycle means
    // lower speed.
  }    
}
