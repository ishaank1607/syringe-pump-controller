// Bring-up test: Uno -> TMC2209 (STEP/DIR mode) -> NEMA 17.
// Turns one revolution each way, forever. Open the Serial Monitor at 115200 to watch.

const uint8_t STEP_PIN = 3;
const uint8_t DIR_PIN = 4;
const uint8_t EN_PIN = 5;                 // active LOW

const int STEPS_PER_REV = 1600;           // 200 full steps x 8 (driver default 1/8 microstepping)
const unsigned int HALF_PERIOD_US = 1250; // 400 steps/s = 0.25 rev/s (15 RPM)

void setup() {
  Serial.begin(115200);
  digitalWrite(EN_PIN, HIGH);             // set the level before enabling the output: no glitch
  pinMode(EN_PIN, OUTPUT);                // driver disabled until we say so
  digitalWrite(STEP_PIN, LOW);
  pinMode(STEP_PIN, OUTPUT);
  digitalWrite(DIR_PIN, LOW);
  pinMode(DIR_PIN, OUTPUT);
  Serial.println("bring-up test: STEP=D3 DIR=D4 EN=D5");
}

void turn(bool dirLevel, int steps) {
  digitalWrite(DIR_PIN, dirLevel);
  delayMicroseconds(5);                   // DIR setup time
  for (int i = 0; i < steps; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(HALF_PERIOD_US);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(HALF_PERIOD_US);
  }
}

void loop() {
  digitalWrite(EN_PIN, LOW);              // enable driver
  delay(200);                             // StealthChop needs a moment at standstill

  Serial.println("1 rev, DIR high");
  turn(HIGH, STEPS_PER_REV);
  delay(1000);

  Serial.println("1 rev, DIR low");
  turn(LOW, STEPS_PER_REV);

  digitalWrite(EN_PIN, HIGH);             // release the motor between cycles
  Serial.println("pause");
  delay(2000);
}
