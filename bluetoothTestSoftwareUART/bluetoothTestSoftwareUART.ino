#include <SoftwareSerial.h>
#include <CircularBuffer.h>
#include <BluetoothCommandParser.h>

#define BLUETOOTH_RX_PIN 4
#define BLUETOOTH_TX_PIN 3
#define LED_PIN 13

SoftwareSerial bluetoothSerial(BLUETOOTH_RX_PIN, BLUETOOTH_TX_PIN);
char blinkState = 0;
BluetoothCommandParser btcmdparser;
int blinkPeriod = 1000;
unsigned long lastBlink;

void setup() {
  pinMode(BLUETOOTH_RX_PIN, INPUT);
  pinMode(BLUETOOTH_TX_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600);
  bluetoothSerial.begin(9600);
  lastBlink = millis();
}

void toggleLED() {
  blinkState = 1 - blinkState;
  digitalWrite(LED_PIN, blinkState); 
}

void loop() {
  unsigned long now = millis();
  while (Serial.available()) {
    char pcByte = Serial.read();
    bluetoothSerial.write(pcByte);
  }
  while (bluetoothSerial.available()) {
    char bluetoothByte = bluetoothSerial.read();
    Serial.write(bluetoothByte);
    btcmdparser.feed(bluetoothByte);
    if (btcmdparser.isValid()) {
      Serial.print("BT CMD: ");
      Serial.print(btcmdparser.key(), HEX);
      Serial.print(btcmdparser.onoff(), HEX);
      Serial.print("\n");
      if (btcmdparser.key() == 5 && btcmdparser.onoff() == 0) {
        blinkPeriod = blinkPeriod / 2;
      } else if (btcmdparser.key() == 6 && btcmdparser.onoff() == 0) {
        blinkPeriod = blinkPeriod * 2;
      }
      btcmdparser.reset();      
    }
  }
  if (now - lastBlink >= blinkPeriod) {
    toggleLED();
    lastBlink = now;
  }
}

