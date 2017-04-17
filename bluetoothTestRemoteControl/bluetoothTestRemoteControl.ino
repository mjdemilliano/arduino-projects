#include <SoftwareSerial.h>
#include <CircularBuffer.h>
#include <BluetoothCommandParser.h>

#define BLUETOOTH_RX_PIN 11
#define BLUETOOTH_TX_PIN 10
#define LED_PIN 13

SoftwareSerial bluetoothSerial(BLUETOOTH_RX_PIN, BLUETOOTH_TX_PIN);
char blinkState = 0;
BluetoothCommandParser btcmdparser;
int blinkPeriod = 1000;
int dutyCycle = 255;
unsigned long lastBlink;

void setup() {
  pinMode(BLUETOOTH_RX_PIN, INPUT);
  pinMode(BLUETOOTH_TX_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(19200);
  bluetoothSerial.begin(9600);
  lastBlink = millis();
}

void toggleLED() {
  blinkState = 1 - blinkState;
  analogWrite(LED_PIN, blinkState ? dutyCycle : 0);
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
    if (btcmdparser.parseKeyCommand() == BluetoothCommandParser::Valid) {
      Serial.print("\nBT CMD: ");
      Serial.print(btcmdparser.keyCommand().key, HEX);
      Serial.print(btcmdparser.keyCommand().onoff, HEX);
      Serial.print("\n");
      if (btcmdparser.keyCommand().key == 5 && btcmdparser.keyCommand().onoff == 0) {
        blinkPeriod = blinkPeriod / 2;
      } else if (btcmdparser.keyCommand().key == 6 && btcmdparser.keyCommand().onoff == 0) {
        blinkPeriod = blinkPeriod * 2;
      }
      btcmdparser.reset();
    }
    if (btcmdparser.parseQuaternionData() == BluetoothCommandParser::Valid) {
      /*
      Serial.print("\nBT Q DATA: ");
      Serial.print(btcmdparser.quaternionData().x, 4);
      Serial.print(" ");
      Serial.print(btcmdparser.quaternionData().y, 4);
      Serial.print(" ");
      Serial.print(btcmdparser.quaternionData().z, 4);
      Serial.print(" ");
      Serial.print(btcmdparser.quaternionData().w, 4);
      Serial.print("\n");
      */
      btcmdparser.reset();
      blinkPeriod = min(1, max(0, (btcmdparser.quaternionData().x + 0.5))) * 3000;
      //dutyCycle = min(1, max(0, (btcmdparser.quaternionData().y + 0.5))) * 255;
      Serial.print("Tblink = ");
      Serial.print(blinkPeriod);
      Serial.print(" @ ");
      Serial.print(dutyCycle);
      Serial.print("/255\n");
    }
  }
  if (now - lastBlink >= blinkPeriod) {
    toggleLED();
    lastBlink = now;
  }
}

