#include "Arduino.h"
#include <Wire.h>

#include "mpu9265.h"

//int16_t g_accelerations[3];
//int16_t g_angularVelocities[3];

uint8_t use_interrupt_trigger = 1;
volatile uint8_t mpu9265_data_ready_flag = 0;  // Flag used by ISR to signal when an interrupt has occurred.
volatile uint32_t num_times_interrupt_fired = 0;

void ISR_MPU_interrupt();

uint8_t readByteFromMPU(uint8_t registerAddress, uint8_t *status)
{
  uint8_t result = 0xFF;
  uint8_t resultcode;
  Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(registerAddress);
  Wire.endTransmission(false);
  resultcode = Wire.requestFrom(MPU_ADDRESS, (uint8_t) 1);
  if (Wire.available())
  {
    result = Wire.read();
  }
  if (status)
  {
    *status = resultcode;
  }
  return result;
}

uint8_t readBytesFromMPU(uint8_t registerStartAddress, uint8_t numBytes, uint8_t *data)
{
  uint8_t numBytesReceived;
  uint8_t index;
  if (!data)
    return 0;
  Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(registerStartAddress);
  Wire.endTransmission(false);
  numBytesReceived = Wire.requestFrom(MPU_ADDRESS, numBytes);
  for (index = 0; index < numBytesReceived; ++index)
  {
    data[index] = Wire.read();
  }
  return numBytesReceived;
}

uint8_t writeByteToMPU(uint8_t registerAddress, uint8_t value)
{
  Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(registerAddress);
  Wire.write(value);
  return Wire.endTransmission();
}

/** Reads sensors: 3 accelerations (X, Y, Z) and 3 angular velocities (gX, gY, gZ).
 *
 *   accelerations -- address of int16_t array with 3 elements.
 *   angularVelocities -- address of int16_t array with 3 elements.
 *
 *  Returns 0 when successful, 1 otherwise.
 */
uint8_t readSensors(int16_t *accelerations, int16_t *angularVelocities)
{
  uint8_t numBytesReceived;
  uint8_t data[14];
  uint8_t status;
  if (!accelerations || !angularVelocities)
    return 1;
  numBytesReceived = readBytesFromMPU(0x3B, 14, data);  // 14 = 6 (accelerometers) + 2 (temperature) + 6 (gyros)
  if (numBytesReceived == 14)
  {
    accelerations[0] = ((int16_t) data[0] << 8) | data[1];
    accelerations[1] = ((int16_t) data[2] << 8) | data[3];
    accelerations[2] = ((int16_t) data[4] << 8) | data[5];
    angularVelocities[0] = ((int16_t) data[8] << 8) | data[9];
    angularVelocities[1] = ((int16_t) data[10] << 8) | data[11];
    angularVelocities[2] = ((int16_t) data[12] << 8) | data[13];
    status = 0;
  }
  else
  {
    status = 1;
  }
  return status;
}

void mpu9265_setup() {
  uint8_t result;

  pinMode(MPU_AD0_PIN, OUTPUT);
  digitalWrite(MPU_AD0_PIN, HIGH); // Address will be 0b1101001 = 0x69

  pinMode(MPU_INT_PIN, INPUT);

  Wire.begin();

  delay(10);
  result = writeByteToMPU(0x6B, 0x00); // Wake up.
  delay(10);

  // Set configuration
  result = writeByteToMPU(26, 0x1); // DLPF_CFG = 1, gyro badnwidth 184 Hz, sample rate 1 kHz.

  if (use_interrupt_trigger)
  {
    result = writeByteToMPU(25, 100-1);  // Set MPU sample rate divider. Internal rate: 1 kHz; rate = 1 kHz / (div + 1) = 10 Hz => div = 100-1.
    result = writeByteToMPU(55, 1 << 4);  // Configure MPU so that INT clears automatically without having to read INT status register.
    //Serial.println(result);
    result = writeByteToMPU(56, 1);  // Configure MPU that when data is ready, INT pin is toggled.
    //attachInterrupt(digitalPinToInterrupt(2), ISR_MPU_interrupt, RISING);
    attachInterrupt(0, ISR_MPU_interrupt, RISING);
  }
}

void ISR_MPU_interrupt()
{
  mpu9265_data_ready_flag = 1;
  num_times_interrupt_fired++;
}
