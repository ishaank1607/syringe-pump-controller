#include <TMCStepper.h>
#include <SoftwareSerial.h>

const uint8_t STEP_PIN = 3;
const uint8_t DIR_PIN  = 4;
const uint8_t EN_PIN   = 5;
const uint8_t SW_RX    = 6; // Connect directly to TMC2209 UART pin
const uint8_t SW_TX    = 7; // Connect to SW_RX via a 1k Ohm resistor

#define R_SENSE 0.11f       // Standard sense resistor value
#define DRIVER_ADDRESS 0b00 // MS1 and MS2 low/unconnected

SoftwareSerial SoftSerial(SW_RX, SW_TX);
TMC2209Stepper driver(&SoftSerial, R_SENSE, DRIVER_ADDRESS);

const unsigned int HALF_PERIOD_US = 1250; // Steady top speed... HIGHER NUMBER = SLOWER

void setup() {
  Serial.begin(115200);
  SoftSerial.begin(19200); // Reliable baud rate for Arduino Uno SoftwareSerial

  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, HIGH);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  digitalWrite(DIR_PIN, LOW); // Lock direction, HIGH = PULL, LOW = PUSH

  driver.begin();
  driver.toff(4);
  driver.rms_current(350);        // 350mA RMS
  driver.microsteps(8);           // 1/8 microstepping
  driver.en_spreadCycle(true);    // Force SpreadCycle (high torque)
  driver.pwm_autoscale(false);

  // Verify UART communication
  uint8_t result = driver.test_connection();
  if (result == 0) {
    Serial.println("SUCCESS: TMC2209 UART connected! Running steady DIR HIGH.");
  } else {
    Serial.print("ERROR: UART failed! Code: ");
    Serial.println(result);
  }

  digitalWrite(EN_PIN, LOW); // Enable motor
}

void loop() {
  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(HALF_PERIOD_US);
  digitalWrite(STEP_PIN, LOW);
  delayMicroseconds(HALF_PERIOD_US);
}
