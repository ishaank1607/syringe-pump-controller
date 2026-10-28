# Lab bring-up checklist

Goal: confirm the driver, motor and wiring work using the Arduino Uno, then the hotplate, then the Raspberry Pi.
Nothing in this repo has been run on the real hardware yet. Everything below is the plan, and the numbers marked
"estimate" need to be checked with a meter or against the manuals.

## What you can do before the parts order arrives
On order: 12 V DC supply, barrel socket with leads, Elegoo Uno R3, microSD card and reader, micro-HDMI cable,
jumper wires. Until they arrive, use the lab's Uno and do:

1. [Lab desktop setup](#lab-desktop-setup): get both sketches into the Arduino IDE and compile them.
2. [Stage 0](#stage-0-before-any-power): measurements, no power.
3. [Stage 1](#stage-1-uno-wiring-motor-power-off-motor-not-connected): logic test. No 12 V supply needed.
4. [Stage 1b](#stage-1b-pump-sketch-logic-test-no-motor): pump sketch commands and limits, no motor.
5. [Stage 2](#stage-2-motor) only if the lab has a bench DC supply. Otherwise wait for the 12 V supply.

## Lab desktop setup
Only these files are needed for the Uno work. Read this checklist on GitHub in the browser.

| File | Use |
|---|---|
| `arduino/bringup/bringup.ino` | Logic test and first motor spin |
| `arduino/pump_serial/pump_serial.ino` | Pump commands and limit tests |

The Python files are for the Pi and the hotplate, later.

1. **Get the sketches.** On GitHub, open each `.ino` file and click **Copy raw file**. In the Arduino IDE, use
   **File > New Sketch**, paste over the template, and save as `bringup` or `pump_serial`. The IDE needs each sketch
   in a folder of the same name, and saving this way does that. (Or use **Code > Download ZIP** and open the
   extracted `.ino` files.)
2. **Compile.** **Tools > Board > Arduino AVR Boards > Arduino Uno**, then **Verify** (checkmark) each sketch.
   Both should end with "Done compiling". An error usually means a line was lost in the copy-paste.
3. **Pick the port.** Plug in the Uno over USB and choose it under **Tools > Port** (for example `COM4`).
   If unsure which port it is, unplug the Uno and see which entry disappears.

**Shared desktop:** the repo is private, so log into GitHub in a private/incognito window, don't let the browser
save the password, and log out when done. If you change a constant in the lab (`LEAD_MM`, `SYRINGE_ID_MM`,
`PUSH_LEVEL`, `FLOW_CAL`), write the value down and commit it from your own computer later. Don't put git
credentials on the lab desktop.

If the IDE isn't installed and you can't install software there, ask your mentor. Arduino's web Cloud Editor is
an option but also needs a small helper app installed.

## Bring
The Uno and breadboards stay in the lab. Check what the lab already has before carrying things in.

**For the Uno motor test**
- NEMA 17 motor (XY42STH34-0354A) and the Adafruit TMC2209 breakout
- 12 V DC supply, 1 A or more (on order: 5.5 x 2.1 mm, center positive, plus a female barrel socket with
  red/black 18 AWG leads). Until it arrives, a lab bench supply works for Stage 2.
- Jumper wires: male-to-male for the Uno and breadboard, plus a few male-to-female for the Pi later
- Small flat screwdriver for the terminal blocks and the current pot
- Multimeter (coil resistance and VREF)
- The Uno's USB-B cable, if it isn't with the board

**For the Stage 0 measurements**
- The leadscrew, its nut, and the 5 mL syringe
- Calipers or a ruler

**Nice to have**
- 100 uF electrolytic capacitor, 25 V or higher, for across VM and GND
- Tape or labels for marking the coil pairs

**When the Pi comes in**
- The Lexar USB stick (the Pi's boot drive) and a 5 V, 3 A USB-C supply with cable
- A working way to reach the Pi over a network. The laptop's Ethernet port was unreliable, so plan this first.

**Check before you go**
- **Header pins.** The breakout's headers are not soldered yet. Resting the board on loose pins is OK for the
  Stage 1 logic-only test, but wait for soldering before Stage 2 (see the warning there).
- **Motor wire ends.** The driver's terminal blocks need bare wire. If the motor leads end in a plug, bring a
  matching connector or a cable with bare ends.

## Stage 0: before any power
1. **Coil pairs.** With the multimeter, Red-Yellow should read about 34 ohm, and Green-Gray about 34 ohm.
   Between the two pairs it should read open. If not, re-sort the wires before going further.
2. **Screw lead.** Mark the nut, turn the screw 10 turns, and measure the travel. Expect about 15.9 mm
   (1/4"-16 is 1.5875 mm per turn). If it differs, set `LEAD_MM` to measured travel divided by 10.
3. **Syringe bore.** Measure the plunger travel `L` in mm between the 0 and 5 mL marks, then
   `SYRINGE_ID_MM = 2 * sqrt(5000 / (pi * L))`. The default of 12.06 mm is a typical 5 mL value, not measured.

## Stage 1: Uno wiring (motor power OFF, motor not connected)
No 12 V supply is needed. The Uno runs from USB, and the driver's logic side (VIO) runs from the Uno's 5V pin.
This stage only checks that STEP, DIR and EN reach the driver.

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
yellow STEP LED and the red and green DIR LEDs on the driver should light, in a pattern of one revolution each
way, a pause, and a repeat. Unsoldered headers are fine for this stage.

## Stage 1b: pump sketch logic test (no motor)
Same wiring as Stage 1, VM and motor still disconnected. Upload `arduino/pump_serial/pump_serial.ino` and open the
Serial Monitor at 115200 baud, line ending "Newline". This checks the firmware logic only.

| Send | Expect |
|---|---|
| On upload | `pump ready: 0.113 uL/step` and the command list |
| `STATUS` | `idle pos=0 steps (0.000 mL of 5.0)` |
| `ZERO` then `RUN 0.5 3` | `RUN 4412 steps at 441.16 steps/s` (last digits may differ slightly), STEP LED blinks about 10 s, then `DONE` |
| `RUN 6 1` | `ERR past the end of the syringe` (more than the 5 mL stroke) |
| `RUN 1 20` | `ERR rate needs ... steps/s, over MAX_HZ` (ceiling is about 10 mL/min) |
| `ZERO` then `PULL 0.5 3` | `ERR past the start; retract less or use ZERO` |
| `RUN 2 1`, then `STATUS` | `running` (a second `RUN` now gives `ERR busy`) |
| `STOP` | `STOPPED`, and `STATUS` shows the partial position |

If the lab has an oscilloscope, probe STEP during `RUN 1 1`: it should read about 147 Hz (1 mL/min).

## Stage 2: motor

> **No 12 V supply yet?** A lab bench DC supply is a good first-spin source. Set it to 12 V with the current
> limit at about 0.5 A, so a wiring mistake can't draw much. Never power VM from the Uno's 5V pin.

> **Solder the headers first.** With 12 V on VM, a header pin that loses contact can damage the driver or the Uno.
> If GND or VIO drops out, current can find a path through the signal pins, and a loose EN pin can float and
> enable the motor unexpectedly. If you must run before soldering, press the board down firmly, don't touch it
> while powered, and keep the run short.

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
