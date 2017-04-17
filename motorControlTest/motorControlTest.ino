#define MOTOR_PIN_A 3
#define MOTOR_PIN_B 5

int speed = 0; // negative value means running backwards. maximum is 255

void setup() {
  Serial.begin(19200);
  pinMode(MOTOR_PIN_A, OUTPUT);
  pinMode(MOTOR_PIN_B, OUTPUT);
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'f') {
      speed = min(speed + 25, 250);
    } else if (c == 'b') {
      speed = max(speed - 25, -250);
    }
    Serial.print(speed);
    Serial.print("\n");
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
}
