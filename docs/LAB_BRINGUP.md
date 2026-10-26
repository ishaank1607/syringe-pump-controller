# Lab bring-up checklist

Goal: confirm the driver, motor and wiring work using the Arduino Uno, then the hotplate, then the Raspberry Pi.
Nothing in this repo has been run on the real hardware yet. Everything below is the plan, and the numbers marked
"estimate" need to be checked with a meter or against the manuals.

## Bring
- NEMA 17 motor (XY42STH34-0354A), Adafruit TMC2209 breakout, 12 V supply, jumper wires, small screwdriver
  for the terminal blocks, multimeter. The Uno, breadboards and USB cable stay in the lab.

## Stage 0: before any power
1. **Coil pairs.** With the multimeter, Red-Yellow should read about 34 ohm, and Green-Gray about 34 ohm.
   Between the two pairs it should read open. If not, re-sort the wires before going further.
2. **Screw lead.** Mark the nut, turn the screw 10 turns, and measure the travel. Expect about 15.9 mm
   (1/4"-16 is 1.5875 mm per turn). If it differs, set `LEAD_MM` to measured travel divided by 10.
3. **Syringe bore.** Measure the plunger travel `L` in mm between the 0 and 5 mL marks, then
   `SYRINGE_ID_MM = 2 * sqrt(5000 / (pi * L))`. The default of 12.06 mm is a typical 5 mL value, not measured.

## Stage 1: Uno wiring (motor power OFF, motor not connected)
| TMC2209 | Connects to |
|---|---|
| VIO | Uno 5V |
| GND | Uno GND **and** the 12 V supply minus |
| STEP | D3 |
| DIR | D4 |
| EN | D5 (active low) |
| VM | 12 V supply plus (leave off for now) |
| A1 / A2 | Red / Yellow |
| B1 / B2 | Green / Gray |

Only one controller (Uno or Pi) may be connected to STEP, DIR and EN at a time. Never connect Uno outputs to Pi pins.

Upload `arduino/bringup/bringup.ino` (Tools > Board > Arduino Uno, pick the COM port). With VM off, the
yellow STEP LED and the red and green DIR LEDs on the driver should light.

## Stage 2: motor
1. Turn the driver's current pot to the low end. The motor is rated 0.35 A. The datasheet says current scales with
   VREF, and my estimate is about 0.45-0.5 V for roughly 0.35 A. Check Adafruit's guide for where to measure VREF.
2. Connect the motor **before** powering VM. Never unplug it while VM is on.
3. Power VM at 12 V. Expect one revolution each way, a pause, and a repeat.
4. If the motor only buzzes or shakes, a coil pair is crossed. If it runs backward, swap one coil's two wires.
5. If the driver or motor gets very hot, cut power and lower the pot. If the supply wires are long, add a 100 uF
   or larger capacitor across VM and GND at the board.

## Stage 3: pump sketch
Upload `arduino/pump_serial/pump_serial.ino`. Serial Monitor: 115200 baud, line ending "Newline".
Edit `LEAD_MM` and `SYRINGE_ID_MM` first if Stage 0 gave different numbers.

| Command | Meaning |
|---|---|
| `STATUS` | state and position |
| `ZERO` | call the current plunger position "fully back" |
| `RUN 0.5 3` | push 0.5 mL at 3 mL per minute (about 10 s, 2.8 screw turns) |
| `PULL 0.5 3` | retract |
| `STOP` | stop and release the motor |

Test with no syringe attached. If the plunger direction is wrong, flip `PUSH_LEVEL`.
Then dispense into a beaker and weigh it to set `FLOW_CAL` (measured over commanded volume).
Moves past either end of the stroke, and rates above 1500 steps per second, are refused.

## Stage 4: hotplate (read-only first)
`hotplate.py` needs Python and pyserial (`pip install pyserial` on Windows). Set `PORT` to the hotplate's
COM port (for example `COM3`). Run it with no flags first. The command names and the 9600 baud 7E1 serial
settings are from memory of IKA's protocol and are NOT verified for the RCT. Compare the replies with the
manual before using `--set`. Set the hotplate's own safety circuit above your highest process temperature.

## Stage 5: Raspberry Pi (later)
`stepper.py` and `pump.py` drive the same driver from the Pi (STEP GPIO17, DIR GPIO27, EN GPIO22, VIO to 3.3 V).
See `README.md`. The Pi has not been reachable over a network yet, and USB boot is unconfirmed.
