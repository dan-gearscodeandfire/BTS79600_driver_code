/**
 * @file Motor_IBT2_MCPWM.ino
 * @brief Bidirectional DC motor control using ESP32 MCPWM and an IBT-2 (BTS7960) driver.
 *
 * Hardware:
 *  - ESP32 (Arduino core)
 *  - IBT-2 / BTS7960 43A H-bridge motor driver
 *  - DC motor, separate motor power supply (6–27 V depending on motor)
 *
 * Logic level: 3.3 V (ESP32) — compatible with IBT-2 input pins.
 *
 * ---------------------------------------------------------------------------
 * Wiring: ESP32  ↔  IBT-2 (BTS7960) input port
 * ---------------------------------------------------------------------------
 *  IBT-2 pin | Label | Connection
 *  ----------+-------+-----------------------------------------
 *      1     | RPWM  | ESP32 GPIO 4  (MCPWM0A, "forward" PWM)
 *      2     | LPWM  | ESP32 GPIO 15 (MCPWM0B, "reverse" PWM)
 *      3     | R_EN  | +5 V (always HIGH to enable forward side)
 *      4     | L_EN  | +5 V (always HIGH to enable reverse side)
 *      5     | R_IS  | (optional) ESP32 ADC input for current/fault monitoring
 *      6     | L_IS  | (optional) ESP32 ADC input for current/fault monitoring
 *      7     | VCC   | 5 V logic supply (can also feed 5 V back to MCU if needed)
 *      8     | GND   | Common ground shared with ESP32 GND
 *
 *  Motor power:
 *    - Connect motor supply (e.g. 12 V) to IBT-2 V+ and GND (power side).
 *    - Connect motor leads to the IBT-2 motor output terminals.
 *
 * NOTE:
 *  - Both R_EN and L_EN must be held HIGH for the bridge to drive the motor.
 *  - ESP32 GND and IBT-2 GND must be connected together.
 */

#include "driver/mcpwm.h"   // ESP32 hardware MCPWM driver

/**
 * @class MyMotor
 * @brief Simple wrapper for controlling a DC motor via a BTS7960/IBT-2 driver.
 *
 * The class uses two MCPWM outputs:
 *  - One PWM channel for "forward" direction (RPWM).
 *  - One PWM channel for "reverse" direction (LPWM).
 *
 * Public API:
 *  - MyMotor(int forwardPin, int reversePin)
 *  - void setSpeed(int speed);
 *
 * Speed convention:
 *  -100 .. -1  : reverse, magnitude is duty cycle percentage
 *      0       : motor off (both PWMs 0%)
 *   1 .. 100   : forward, magnitude is duty cycle percentage
 */
class MyMotor {
public:
  /**
   * @brief Construct a new MyMotor object and configure MCPWM.
   *
   * @param forwardPin GPIO number used for the forward PWM (RPWM).
   * @param reversePin GPIO number used for the reverse PWM (LPWM).
   *
   * The constructor:
   *  - Binds forwardPin to MCPWM0A and reversePin to MCPWM0B.
   *  - Sets up MCPWM Unit 0 / Timer 0 at 1 kHz, active high.
   *  - Initializes both channels at 0% duty.
   */
  MyMotor(int forwardPin, int reversePin) {
    this->forwardPin = forwardPin;
    this->reversePin = reversePin;

    // Bind GPIOs to MCPWM outputs
    // MCPWM0A -> forward direction (RPWM)
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, forwardPin);

    // MCPWM0B -> reverse direction (LPWM)
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, reversePin);

    // Base MCPWM configuration structure
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 1000;              // 1 kHz switching frequency
    pwm_config.cmpr_a   = 0.0;                // Channel A starts at 0% duty
    pwm_config.cmpr_b   = 0.0;                // Channel B starts at 0% duty
    pwm_config.counter_mode = MCPWM_UP_COUNTER;   // Simple up-counting mode
    pwm_config.duty_mode    = MCPWM_DUTY_MODE_0;  // Active-high output

    // Apply configuration to Unit 0, Timer 0
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
  }

  /**
   * @brief Set motor speed and direction.
   *
   * @param speed Value from -100 to 100.
   *
   *  - speed > 0 : forward, duty = speed %
   *  - speed < 0 : reverse, duty = |speed| %
   *  - speed = 0 : motor off (both PWM channels 0%)
   *
   * The method clamps input to [-100, 100] to avoid invalid duty values.
   */
  void setSpeed(int speed) {
    // Constrain to valid range
    if (speed < -100) speed = -100;
    if (speed > 100)  speed = 100;

    // Duty cycle is always positive; direction is encoded in sign of speed
    float dutyCycle = abs(speed);  // 0–100 %

    if (speed > 0) {
      // ---------------------------
      // Forward direction (RPWM)
      // ---------------------------
      mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, dutyCycle); // Forward PWM
      mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 0.0);       // Reverse off

      Serial.print("Forward Speed: ");
      Serial.print(speed);
      Serial.println("%");
    }
    else if (speed < 0) {
      // ---------------------------
      // Reverse direction (LPWM)
      // ---------------------------
      mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 0.0);       // Forward off
      mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, dutyCycle); // Reverse PWM

      Serial.print("Reverse Speed: ");
      Serial.print(speed);
      Serial.println("%");
    }
    else {
      // ---------------------------
      // Stop: both channels off
      // ---------------------------
      mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 0.0);
      mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 0.0);

      Serial.println("Motor Stopped");
    }
  }

private:
  int forwardPin;   ///< GPIO used for forward PWM (RPWM).
  int reversePin;   ///< GPIO used for reverse PWM (LPWM).
};

// -----------------------------------------------------------------------------
// Global motor instance
// -----------------------------------------------------------------------------

// Using GPIO 4 as forward (RPWM) and GPIO 15 as reverse (LPWM).
// These must be wired to IBT-2 pins RPWM and LPWM respectively.
MyMotor motor(4, 15);

// -----------------------------------------------------------------------------
// Arduino setup() and loop()
// -----------------------------------------------------------------------------

/**
 * @brief Arduino setup function.
 *
 * Initializes the serial port for debug output.
 * The motor driver and MCPWM are initialized by the MyMotor constructor.
 */
void setup() {
  Serial.begin(115200);
  Serial.println("Starting Motor Test...");
}

/**
 * @brief Arduino main loop.
 *
 * Performs a continuous test pattern:
 *  1. Ramp forward from 0% to 100% duty.
 *  2. Hold at full forward.
 *  3. Ramp down to 0%.
 *  4. Ramp reverse from 0% to -100% duty.
 *  5. Hold at full reverse.
 *  6. Ramp back to 0%.
 *
 * Each step updates the motor speed using MyMotor::setSpeed().
 */
void loop() {
  // ---------------------------------------------------------------------------
  // FORWARD CYCLE: ramp 0 -> +100 %
  // ---------------------------------------------------------------------------
  for (int speed = 0; speed <= 100; speed += 10) {
    motor.setSpeed(speed);
    delay(500);  // Adjust for ramp rate (smaller = smoother/faster ramp)
  }

  delay(2000);   // Hold at 100% forward

  // ---------------------------------------------------------------------------
  // RAMP DOWN: +100 % -> 0 %
  // ---------------------------------------------------------------------------
  for (int speed = 100; speed >= 0; speed -= 10) {
    motor.setSpeed(speed);
    delay(500);
  }

  delay(2000);   // Hold stopped

  // ---------------------------------------------------------------------------
  // REVERSE CYCLE: 0 -> -100 %
  // ---------------------------------------------------------------------------
  for (int speed = 0; speed >= -100; speed -= 10) {
    motor.setSpeed(speed);
    delay(500);
  }

  delay(2000);   // Hold at 100% reverse

  // ---------------------------------------------------------------------------
  // RAMP DOWN: -100 % -> 0 %
  // ---------------------------------------------------------------------------
  for (int speed = -100; speed <= 0; speed += 10) {
    motor.setSpeed(speed);
    delay(500);
  }

  delay(2000);   // Hold stopped before repeating cycle
}
