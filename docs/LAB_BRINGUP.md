# Lab bring-up checklist

Goal: confirm the driver, motor and wiring work using the Arduino Uno, then the hotplate, then the Raspberry Pi.
Nothing in this repo has been run on the real hardware yet. Everything below is the plan, and the numbers marked
"estimate" need to be checked with a meter or against the manuals.

## Order of work
1. [Lab desktop setup](#lab-desktop-setup): get both sketches into the Arduino IDE and compile them.
2. [Stage 0](#stage-0-before-any-power): measurements, no power.
3. [Stage 1](#stage-1-uno-wiring-motor-power-off-motor-not-connected): logic test. No 12 V supply needed.
4. [Stage 1b](#stage-1b-pump-sketch-logic-test-no-motor): pump sketch commands and limits, no motor.
5. [Stage 2](#stage-2-motor): 12 V power and the first motor spin.
6. [Stage 3](#stage-3-pump-sketch) to [Stage 5](#stage-5-raspberry-pi): pump, hotplate, then the Pi.

## Parts and where they go
| Part | Goes to |
|---|---|
| 12 V DC adapter (5.5 x 2.1 mm, center positive) | Plugs into the barrel socket |
| Barrel socket with red/black 18 AWG leads | Driver's terminal block, two leftmost screws: red to **+**, black to **−** |
| Elegoo Uno R3 (or the lab's Uno) | Driver's STEP, DIR, EN, VDD and GND header pins, through the breadboard |
| NEMA 17 motor | Driver's terminal block, four right screws (2B 2A 1A 1B) |
| Shaft coupler | Motor shaft to leadscrew |
| microSD card + USB reader | Flashed on your own computer with Imager, then into the Pi |
| Micro-HDMI to HDMI cable | Pi's HDMI0 port (the one next to USB-C) to a monitor |
| Jumper wires | Male-to-male for the Uno, male-to-female for the Pi header |

## Full wiring (Uno)
```
 12 V adapter --> barrel socket --red---> +    \  terminal block,
                                --black-> -    /  left two screws

 Uno 5V  -----> VDD   \
 Uno GND -----> GND    |
 Uno D3  -----> STEP   |  driver header pins
 Uno D4  -----> DIR    |  (through the breadboard)
 Uno D5  -----> EN    /

 Motor Red / Yellow --> 1A / 1B   \  terminal block,
 Motor Green / Gray --> 2A / 2B   /  right four screws (2B 2A 1A 1B)

 Uno USB --> lab desktop (power + Serial Monitor)
```
The Uno GND goes to the header GND pin and the 12 V black lead to the terminal block's **−**. The board joins
them internally, which gives the Uno and the motor supply a common ground (check with the meter's continuity
mode in Stage 0). The motor supply never connects to the Uno itself.

**Board labels.** The names above are what the Adafruit board prints. Other guides and the TMC2209 datasheet
use different names for the same things:

| On this board | Datasheet / other guides | What it is |
|---|---|---|
| **+** / **−** (terminal block) | VM / GND, VMOT | Motor supply, 12 V |
| **VDD** (header) | VIO, VCC_IO | Logic supply: 5 V from the Uno, 3.3 V from the Pi |
| **1A 1B** / **2A 2B** | A1 A2 / B1 B2 (OA1 OA2 / OB1 OB2 on the chip) | Motor coil 1 / coil 2 |
| **S**, **F**, **B** LEDs | | Step, and the two direction LEDs |
| **UART** | PDN_UART | Not used here |
| **MS1**, **MS2**, **DIAG**, **INDEX**, **SPRD** jumper | | Leave unconnected / untouched (1/8 microstepping) |
| **ILimit 0->2A** pot | | Motor current limit |

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
   If unsure which port it is, unplug the Uno and see which entry disappears. The Elegoo Uno uses the same
   **Arduino Uno** board setting. If it shows no port at all, it may use a CH340 USB chip, which needs Elegoo's
   CH340 driver on Windows.

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
- 12 V DC adapter (5.5 x 2.1 mm, center positive) and the barrel socket with red/black leads
- Jumper wires: male-to-male for the Uno and breadboard, plus a few male-to-female for the Pi later
- Small flat screwdriver for the terminal blocks and the current pot
- Multimeter (coil resistance and VREF)
- The Uno's USB-B cable, if it isn't with the board

**For the Stage 0 measurements**
- The leadscrew, its nut, and the 5 mL syringe
- Calipers or a ruler

**Nice to have**
- 100 uF electrolytic capacitor, 25 V or higher, for across the terminal block's + and −
- Tape or labels for marking the coil pairs

**When the Pi comes in**
- The Pi with the flashed microSD card in it, and a 5 V, 3 A USB-C supply
- Micro-HDMI to HDMI cable, plus a USB keyboard if the lab has no spare one
- Male-to-female jumper wires

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
4. **Ground link.** Continuity mode between the terminal block's **−** and the header **GND** pin: it should beep.
   If it doesn't, also run a jumper from the Uno GND to the **−** screw.

## Stage 1: Uno wiring (motor power OFF, motor not connected)
No 12 V supply is needed. The Uno runs from USB, and the driver's logic side (VDD) runs from the Uno's 5V pin.
This stage only checks that STEP, DIR and EN reach the driver.

| TMC2209 | Connects to |
|---|---|
| VDD | Uno 5V |
| GND (header) | Uno GND |
| STEP | D3 |
| DIR | D4 |
| EN | D5 (active low) |
| **+** / **−** (terminal block) | 12 V red / black leads (leave off for now) |
| 1A / 1B | Red / Yellow |
| 2A / 2B | Green / Gray |

Only one controller (Uno or Pi) may be connected to STEP, DIR and EN at a time. Never connect Uno outputs to Pi pins.

Upload `arduino/bringup/bringup.ino` (Tools > Board > Arduino Uno, pick the COM port). With the 12 V off,
the driver's **S** (step) LED should glow steadily for about 4 s (one revolution, 400 steps/s is too fast to see
as blinking), then **F** / **B** (direction) swap and S glows another 4 s, then all dark for the pause, and repeat.
Unsoldered headers are fine for this stage.

**If S stays dark:** with `bringup.ino` running, set the meter to DC volts, black probe on GND, and touch the red
probe to the top of the driver's STEP pin. It should read about 2.5 V while stepping (a 50% square wave averages
to half of 5 V). Then check Uno D3 the same way, and VDD (should be 5 V).
- D3 reads 2.5 V but the driver's STEP pin doesn't: bad contact between the jumper, breadboard or header. Reseat
  or press the board down; this is where unsoldered headers bite.
- D3 reads 0 V: wrong pin (D3, not A3) or the sketch isn't running (check the Serial Monitor at 115200).
- STEP reads 2.5 V but S is dark: check VDD and GND reach the board.

## Stage 1b: pump sketch logic test (no motor)
Same wiring as Stage 1, 12 V and motor still disconnected. Upload `arduino/pump_serial/pump_serial.ino` and open the
Serial Monitor at 115200 baud, line ending "Newline". This checks the firmware logic only.

The **S** LED will **not** visibly light here. This sketch holds STEP high for only a few microseconds per step
(under 0.5% of the time), which is too short to see. The serial replies are the check. Use `bringup.ino`
(Stage 1) to check S, or an oscilloscope on STEP.

| Send | Expect |
|---|---|
| On upload | `pump ready: 0.113 uL/step` and the command list |
| `STATUS` | `idle pos=0 steps (0.000 mL of 5.0)` |
| `ZERO` then `RUN 0.5 3` | `RUN 4412 steps at 441.16 steps/s` (last digits may differ slightly), then `DONE` about 10 s later |
| `RUN 6 1` | `ERR past the end of the syringe` (more than the 5 mL stroke) |
| `RUN 1 20` | `ERR rate needs ... steps/s, over MAX_HZ` (ceiling is about 10 mL/min) |
| `ZERO` then `PULL 0.5 3` | `ERR past the start; retract less or use ZERO` |
| `RUN 2 1`, then `STATUS` | `running` (a second `RUN` now gives `ERR busy`) |
| `STOP` | `STOPPED`, and `STATUS` shows the partial position |

If the lab has an oscilloscope, probe STEP during `RUN 1 1`: it should read about 147 Hz (1 mL/min).

## Stage 2: motor

> Never power the **+** terminal from the Uno's 5V pin. A lab bench supply set to 12 V with a 0.5 A current limit also works
> in place of the adapter, and is the gentlest option for a first spin.

> **Solder the headers first.** With 12 V on the terminal block, a header pin that loses contact can damage the driver or the Uno.
> If GND or VDD drops out, current can find a path through the signal pins, and a loose EN pin can float and
> enable the motor unexpectedly. If you must run before soldering, press the board down firmly, don't touch it
> while powered, and keep the run short.

**Prepare the 12 V leads (adapter unplugged from the wall)**
1. Cut the socket's leads to a workable length (they come 6 ft long) and strip about 5 mm off each.
2. Plug the adapter into the wall and into the socket. With the multimeter on DC volts, confirm **red reads
   +12 V relative to black**. If it reads -12 V, the leads are swapped: use black as + from here on, and mark it.
3. Unplug the adapter from the wall again.

The 18 AWG leads are too thick for breadboard holes and will damage them. Clamp them only in the driver's
power terminal block.

**First spin**
1. Turn the driver's current pot to the low end. The motor is rated 0.35 A. The datasheet says current scales with
   VREF, and my estimate is about 0.45-0.5 V for roughly 0.35 A. Check Adafruit's guide for where to measure VREF.
2. Connect the motor to the motor terminal block **before** applying 12 V. Never unplug it while 12 V is on.
3. Clamp the + lead into **+** and the - lead into **−** on the terminal block. Recheck both.
4. Upload `bringup.ino` with the Uno on USB, then plug the adapter into the wall. Expect one revolution each
   way, a pause, and a repeat.
5. If the motor only buzzes or shakes, a coil pair is crossed. If it runs backward, swap one coil's two wires.
6. If the driver or motor gets very hot, cut power and lower the pot. If the supply wires are long, add a 100 uF
   or larger capacitor across **+** and **−** at the board (stripe side to **−**).

To power down: unplug the adapter first, then the Uno's USB.

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

## Stage 5: Raspberry Pi
`stepper.py` and `pump.py` drive the same driver from the Pi. The Pi boots from the microSD card, and the
Python files ride along on the card's boot partition, so no USB stick is needed.

**Flash the microSD (on your own computer)**
1. Put the card in the USB reader and open Raspberry Pi Imager.
2. Choose Raspberry Pi 4, Raspberry Pi OS (64-bit), and the microSD card. Double-check it is the card, not
   another drive.
3. In the settings: hostname `pumppi`, your username and password, SSH on. Skip Wi-Fi.
4. Write. When it finishes, unplug and replug the reader. Windows shows a small drive named **bootfs**: copy
   `stepper.py`, `pump.py` and `hotplate.py` onto it, then eject. Ignore any prompt to format the card's other
   partition (that is the Pi's Linux filesystem; formatting it erases the install).

**First boot (in the lab)**
1. Card into the Pi's slot on the underside. Micro-HDMI into **HDMI0** (next to USB-C), keyboard into a USB port.
2. Connect the 5 V, 3 A USB-C supply last. Log in at the console.
3. Copy the files from the boot partition to your home folder: `cp /boot/firmware/*.py ~/`
4. Check pyserial is there: `python3 -c "import serial"` (no output means it is).

**Rewire the driver for the Pi (all power off)**
1. Disconnect the Uno's STEP, DIR, EN, VDD and GND wires from the driver completely.
2. Wire the Pi with male-to-female jumpers:

| TMC2209 | Pi pin |
|---|---|
| VDD | 3.3V (pin 1) |
| GND | GND (pin 6) |
| STEP | GPIO17 (pin 11) |
| DIR | GPIO27 (pin 13) |
| EN | GPIO22 (pin 15) |

The 12 V leads and the motor stay on the terminal block as before. VDD must be 3.3 V, never 5 V, on the Pi.
3. Boot the Pi, plug in the 12 V adapter, and run `python3 stepper.py`: one revolution each way.
4. Then `python3 pump.py --dry-run` with your volume and rate to check the numbers before a real run.
   Set `LEAD_MM`, `SYRINGE_ID_MM` and `SYRINGE_VOLUME_ML` in `pump.py` first (they start as None).

See `README.md` for the Pi notes (jitter, idle heat, microstepping).
