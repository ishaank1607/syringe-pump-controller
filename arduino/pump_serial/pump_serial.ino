// Constant-rate leadscrew syringe pump, controlled over USB serial.
// TMC2209 in STEP/DIR mode. Serial Monitor: 115200 baud, line ending "Newline".
//
// Commands:
//   RUN <mL> <mL/min>    push the plunger (dispense)
//   PULL <mL> <mL/min>   retract the plunger (refill / reset)
//   STOP                 stop now and release the motor
//   ZERO                 call the current plunger position "fully back" (full syringe)
//   STATUS               print state and position
// Position is tracked from ZERO and is lost on reset. Moves past either end are refused.

const uint8_t STEP_PIN = 3;
const uint8_t DIR_PIN = 4;
const uint8_t EN_PIN = 5;                 // active LOW

const float STEPS_PER_REV = 1600.0;       // 200 full steps x 8 (driver default 1/8 microstepping)
const float LEAD_MM = 25.4 / 16;          // 1/4"-16 screw, single start (verify by measuring)
const float SYRINGE_ID_MM = 12.06;        // 5 mL syringe, typical value (measure yours)
const float SYRINGE_VOLUME_ML = 5.0;
const bool PUSH_LEVEL = HIGH;             // DIR level that pushes the plunger; flip if reversed
const float FLOW_CAL = 1.0;               // measured / commanded volume; set after weighing a test
const float MAX_HZ = 1500.0;
const bool HOLD_AFTER_MOVE = false;       // true keeps the motor energized when a move ends

const float ML_PER_STEP =
    (LEAD_MM / STEPS_PER_REV) * (PI * (SYRINGE_ID_MM / 2) * (SYRINGE_ID_MM / 2)) / 1000.0 * FLOW_CAL;
const long FULL_STEPS = (long)(SYRINGE_VOLUME_ML / ML_PER_STEP);

bool running = false;
bool pushing = true;
uint32_t stepsLeft = 0;
uint32_t nextStep = 0;                    // micros() deadline of the next step
float period = 0;                         // microseconds between steps
float frac = 0;                           // carries the sub-microsecond remainder
long pos = 0;                             // steps from "fully back"

char buf[48];
uint8_t n = 0;

void release() {
  if (!HOLD_AFTER_MOVE) digitalWrite(EN_PIN, HIGH);
}

void startMove(bool push, float volMl, float rateMlMin) {
  if (running) { Serial.println("ERR busy"); return; }
  if (volMl <= 0 || rateMlMin <= 0) { Serial.println("ERR need positive volume and rate"); return; }
  uint32_t steps = (uint32_t)(volMl / ML_PER_STEP + 0.5);
  float hz = rateMlMin / 60.0 / ML_PER_STEP;
  if (steps == 0) { Serial.println("ERR volume is less than one step"); return; }
  if (hz > MAX_HZ) { Serial.print("ERR rate needs "); Serial.print(hz, 0); Serial.println(" steps/s, over MAX_HZ"); return; }
  if (push && pos + (long)steps > FULL_STEPS) { Serial.println("ERR past the end of the syringe"); return; }
  if (!push && (long)steps > pos) { Serial.println("ERR past the start; retract less or use ZERO"); return; }

  digitalWrite(DIR_PIN, push ? PUSH_LEVEL : !PUSH_LEVEL);
  digitalWrite(EN_PIN, LOW);
  delay(200);                             // StealthChop needs a moment at standstill

  period = 1e6 / hz;
  frac = 0;
  stepsLeft = steps;
  pushing = push;
  running = true;
  nextStep = micros();
  Serial.print(push ? "RUN " : "PULL ");
  Serial.print(steps); Serial.print(" steps at "); Serial.print(hz, 2); Serial.println(" steps/s");
}

void stepIfDue() {
  if (!running || (long)(micros() - nextStep) < 0) return;
  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(5);
  digitalWrite(STEP_PIN, LOW);
  pos += pushing ? 1 : -1;
  // absolute deadlines: timing error never accumulates into flow error
  uint32_t whole = (uint32_t)period;
  nextStep += whole;
  frac += period - whole;
  if (frac >= 1.0) { nextStep++; frac -= 1.0; }
  if (--stepsLeft == 0) {
    running = false;
    release();
    Serial.println("DONE");
  }
}

void handle(char *line) {
  char *cmd = strtok(line, " ");
  char *a = strtok(NULL, " ");
  char *b = strtok(NULL, " ");
  if (!cmd) return;
  if (!strcasecmp(cmd, "RUN") && a && b) startMove(true, atof(a), atof(b));
  else if (!strcasecmp(cmd, "PULL") && a && b) startMove(false, atof(a), atof(b));
  else if (!strcasecmp(cmd, "STOP")) { running = false; digitalWrite(EN_PIN, HIGH); Serial.println("STOPPED"); }
  else if (!strcasecmp(cmd, "ZERO")) { pos = 0; Serial.println("zeroed"); }
  else if (!strcasecmp(cmd, "STATUS")) {
    Serial.print(running ? "running" : "idle");
    Serial.print(" pos="); Serial.print(pos);
    Serial.print(" steps ("); Serial.print(pos * ML_PER_STEP, 3);
    Serial.print(" mL of "); Serial.print(SYRINGE_VOLUME_ML, 1); Serial.println(")");
  }
  else Serial.println("ERR unknown command");
}

void readSerial() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (n) { buf[n] = 0; handle(buf); n = 0; }
    } else if (n < sizeof(buf) - 1) {
      buf[n++] = c;
    }
  }
}

void setup() {
  Serial.begin(115200);
  digitalWrite(EN_PIN, HIGH);             // set the level before enabling the output: no glitch
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(STEP_PIN, LOW);
  pinMode(STEP_PIN, OUTPUT);
  digitalWrite(DIR_PIN, LOW);
  pinMode(DIR_PIN, OUTPUT);
  Serial.print("pump ready: "); Serial.print(ML_PER_STEP * 1000, 3); Serial.println(" uL/step");
  Serial.println("commands: RUN, PULL, STOP, ZERO, STATUS");
}

void loop() {
  stepIfDue();
  readSerial();
}
