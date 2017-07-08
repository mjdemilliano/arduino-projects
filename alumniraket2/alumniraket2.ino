#include <CircularBuffer.h>
#include <BluetoothCommandParser.h>
#include <SoftwareSerial.h>
#include <mpu9265.h>

#define MOTOR_PIN_A 9
#define MOTOR_PIN_B 10

#define BLUETOOTH_RX_PIN 3
#define BLUETOOTH_TX_PIN 4

#define STEP_TIME 1000
#define STEP_SIZE 40
#define MAX_SPEED 250

#define LED_PIN 13

//#define DEBUG_BLUETOOTH 1
#define LOG_IMU_DATA 1
#define READ_IMU

int speed = 0; // negative value means running backwards. maximum is 255
unsigned long checkpointTime = STEP_TIME;
int direction = 1;
int active = 0;
int ledStatus = 0;
BluetoothCommandParser btcmdparser;
int16_t g_accelerations[3];
int16_t g_angularVelocities[3];
uint32_t num_times_measured = 0;

SoftwareSerial bluetoothSerial(BLUETOOTH_RX_PIN, BLUETOOTH_TX_PIN);

void setup() {
  Serial.begin(19200);
  bluetoothSerial.begin(9600);
  pinMode(MOTOR_PIN_A, OUTPUT);
  pinMode(MOTOR_PIN_B, OUTPUT);
  pinMode(BLUETOOTH_RX_PIN, INPUT);
  pinMode(BLUETOOTH_TX_PIN, OUTPUT);
  mpu9265_setup();
}

void setSpeed(int speed) {
  // If going forward, B = low, and A = high means that the motor moves, so low PWM duty cycle means
  // lower speed.
  // If going backward, B = high, and A = high means that the motor stops, so high PWM duty cycle means
  // lower speed.
  if (speed >= 0) {
    analogWrite(MOTOR_PIN_A, speed);
    digitalWrite(MOTOR_PIN_B, 0);
  } else {
    digitalWrite(MOTOR_PIN_A, 0);
    analogWrite(MOTOR_PIN_B, -speed);
  }
}

void stopMotor() {
  digitalWrite(MOTOR_PIN_A, 0);
  digitalWrite(MOTOR_PIN_B, 0);
}

void loop() {
  char c = 0;
  char btbyte = 0;
  bool havebtcmd = false;
  uint8_t result;
  
#if 0
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
#endif
  
  // Handle commands.
  if (Serial.available()) {
    c = Serial.read();
  } else if (bluetoothSerial.available()) {
    btbyte = bluetoothSerial.read();
    //btcmdparser.feed(btbyte);
    c = btbyte;
#ifdef DEBUG_BLUETOOTH
    Serial.print("BT>uC: 0x");
    Serial.print(btbyte, HEX);
    Serial.print("\n");
#endif
    havebtcmd = btcmdparser.parseKeyCommand() == BluetoothCommandParser::Valid;
  }
  if (c != 0 || havebtcmd) {
    // Toggle LED to indicate we received a command.
    ledStatus = 1 - ledStatus;
    digitalWrite(LED_PIN, ledStatus);
  
    // Process commands.
    if (havebtcmd) {
      // Map Bluetooth command to digit.
      if (btcmdparser.keyCommand().key == 5 && btcmdparser.keyCommand().onoff == 0) {
        c = 'f'; // increase speed
      } else if (btcmdparser.keyCommand().key == 6 && btcmdparser.keyCommand().onoff == 0) {
        c = 'b'; // decrease speed
      } else if (btcmdparser.keyCommand().key == 7 && btcmdparser.keyCommand().onoff == 0) {
        c = 'p'; // stop
      } else if (btcmdparser.keyCommand().key == 8 && btcmdparser.keyCommand().onoff == 0) {
        c = 's'; // start
      } else if (btcmdparser.keyCommand().key == 1 && btcmdparser.keyCommand().onoff == 0) {
        c = 'r'; // reverse
      }
      btcmdparser.reset();
 #ifdef DEBUG_BLUETOOTH
      if (c) {
        Serial.print("BT>uC: ");
        Serial.print(c);
        Serial.print("\n");
      }
 #endif
    }
    if (c) {        
      if (c == 'f') {
        active = 1;
        speed = min(speed + STEP_SIZE, 250);
        setSpeed(speed);
      } else if (c == 'b') {
        active = 1;
        speed = max(speed - STEP_SIZE, -250);
        setSpeed(speed);
      } else if (c == 'r') {
        active = 1;
        direction = -1 * direction;
        setSpeed(speed);
      } else if (c == 's') {
        active = 1;
        setSpeed(speed);
      } else if (c == 'p') {
        active = 0;
        stopMotor();
      }
      Serial.print(active ? ">" : ".");
      Serial.print(" ");
      Serial.print(speed);
      Serial.print("\n");
      bluetoothSerial.print(active ? ">" : ".");
      bluetoothSerial.print(" ");
      bluetoothSerial.print(speed);
      bluetoothSerial.print("\n");
    }
  }
#ifdef READ_IMU
  if (mpu9265_data_ready_flag) {
    mpu9265_data_ready_flag = 0;
    result = readSensors(g_accelerations, g_angularVelocities);
    num_times_measured++;
#ifdef LOG_IMU_DATA
    if (result == 0)
    {
      bluetoothSerial.print(millis());
      bluetoothSerial.print("\t");
      bluetoothSerial.print(num_times_measured);
      bluetoothSerial.print("\t");
      bluetoothSerial.print(g_accelerations[0]);
      bluetoothSerial.print("\t");
      bluetoothSerial.print(g_accelerations[1]);
      bluetoothSerial.print("\t");
      bluetoothSerial.print(g_accelerations[2]);
      bluetoothSerial.print("\t");
      bluetoothSerial.print(g_angularVelocities[0]);
      bluetoothSerial.print("\t");
      bluetoothSerial.print(g_angularVelocities[1]);
      bluetoothSerial.print("\t");
      bluetoothSerial.print(g_angularVelocities[2]);
      bluetoothSerial.print("\r\n");
    }
#endif
  }
#endif
}
