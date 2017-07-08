/*
 * How to connect:
 * Wire I2C: Uno/Nano: A4 = SDA, A5 = SCL
 * External interrupt: INT => Uno: int.0 = digital pin 2
 * AD0/SD0 => Uno: D12 (just a digital output)
 */
#define MPU_AD0_PIN     11
#define MPU_INT_PIN     2  // Digital pin 2, because it can be used as external interrupt source (0).
#define MPU_ADDRESS     (uint8_t) 0x69  // Last bit is AD0, should be high.

extern volatile uint8_t mpu9265_data_ready_flag;  // Flag used by ISR to signal when an interrupt has occurred.

extern uint8_t readSensors(int16_t *accelerations, int16_t *angularVelocities);
extern void mpu9265_setup();
