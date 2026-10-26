# Pump System

Drives a NEMA 17 stepper motor from a Raspberry Pi 4 through an Adafruit TMC2209 driver breakout (step/dir mode, no UART).

Constant-rate syringe pump for precursor delivery (5 mL syringe, 1/4"-16 leadscrew), with an IKA RCT hotplate read over serial.
Nothing here has been run on the real hardware yet. **Start with `docs/LAB_BRINGUP.md`.**

| Path | What it is |
|---|---|
| `docs/LAB_BRINGUP.md` | Step-by-step bring-up checklist for the lab (Uno first, then hotplate, then Pi) |
| `arduino/bringup/bringup.ino` | Uno test: one revolution each way, forever |
| `arduino/pump_serial/pump_serial.ino` | Uno constant-rate pump with serial commands and stroke limits |
| `stepper.py`, `pump.py` | Raspberry Pi versions (gpiozero) |
| `hotplate.py` | IKA RCT over USB serial (pyserial), read-only unless `--set` is given |

The Pi wiring and notes below apply to `stepper.py` and `pump.py`. For the Uno wiring, see `docs/LAB_BRINGUP.md`.

## Wiring

| TMC2209 breakout | Connects to |
|---|---|
| VM | Motor supply +, 12V is typical. Do not power it from the Pi. |
| GND | Supply - **and** a Pi GND pin (common ground is required) |
| VIO | Pi 3.3V (pin 1), so the logic levels match |
| STEP | GPIO17 (pin 11) |
| DIR | GPIO27 (pin 13) |
| EN | GPIO22 (pin 15), active low |
| A1 / A2 | Coil 1: Red / Yellow |
| B1 / B2 | Coil 2: Green / Gray |

- **Motor:** XY42STH34-0354A, 1.8 deg (200 steps/rev), 12V rated, 0.35A per phase, 34 ohm, 33 mH, bipolar 4-wire. Datasheet colors: A = Red, C = Yellow (coil 1), B = Gray, D = Green (coil 2).
- **Verify the coil pairs:** use a multimeter. Each pair (Red-Yellow, Green-Gray) should read about 34 ohm, and across pairs it should be open. Red and Yellow must share a coil (both on A1/A2); same for Green and Gray (B1/B2). Which wire goes to A1 versus A2 only sets direction. If the motor spins the wrong way, swap one coil's two wires or flip the DIR logic.
- **Supply voltage:** use 12V. At 34 ohm, 12V can only push about 0.35A at standstill, which matches the motor rating. At 24V the motor would rely entirely on the driver's current limit, so avoid it until that limit is set correctly.
- **Speed and torque:** this motor has high inductance (33 mH), so torque drops quickly as speed rises, and it will be weaker at 12V/0.35A than the datasheet's 24V/0.5A pull-out curve. `max_hz` in `stepper.py` defaults to 1500 steps/s (about 56 RPM at 1/8 microstepping). Raise it gradually. Check holding and running torque against what the pump needs.
- **Wiring order:** connect the motor before applying VM power. Don't unplug it while the supply is on.
- **Current limit:** the board has an onboard potentiometer. Fully clockwise is about 2A (Adafruit's figure), and the datasheet says current scales linearly with the VREF voltage (2.5V = 100%). This motor is only 0.35A, so the pot should sit near the low end: about 0.45V on VREF is roughly 0.35A. The datasheet advises against VREF below about 0.5V (noise), so aim for 0.45-0.5V and let the 12V supply and the 34 ohm winding cap the current. Verify the setting with a multimeter before relying on it. If you can't probe VREF, start the pot near the low end of its travel and check the motor stays cool.
- **Microstepping:** with MS1 and MS2 left unconnected, the default is 1/8 step (1600 steps per revolution on a 1.8 deg motor). Tie MS1 high (3.3V) for 1/32, MS2 high for 1/64, or both high for 1/16. Match `MICROSTEPS` in `stepper.py` to whatever you pick.

## Run

`gpiozero` ships with Raspberry Pi OS, so nothing to install.

```
python3 stepper.py
```

This turns the motor one revolution one way, pauses, then one revolution back.

## Notes

- **Jitter:** `sleep()`-based timing in Python isn't precise. The TMC2209 interpolates each STEP to 256 microsteps based on the previous step interval, and the datasheet says this only works perfectly with a jitter-free step rate, so jitter can make motion slightly rough. It's fine for testing and slow moves. At high speeds you may hear roughness or lose steps. If so, switch to `pigpio` wave chains or `lgpio`'s `tx_pwm` for hardware-assisted timing.
- **Speed:** if it stalls or buzzes, lower `max_hz`.
- **Idle heat:** the motor stays energized while EN is low and will get warm. Call `motor_off()` when idle. The TMC2209 can also drop to roughly half current at standstill if PDN_UART is held low (datasheet 1.7). Check Adafruit's pinout for how your board handles that pin.
- **First move:** `motor_on()` waits 200 ms before returning. StealthChop2 tunes its current regulation while the motor sits at run current, and needs more than 130 ms of standstill before the first move.
- **Supply decoupling:** the board has a 22 uF capacitor on VM. The datasheet recommends at least 100 uF near the driver and warns that fast supply ramps or ringing can damage it. If your supply wires are long, or you switch the supply on live, add a 100 uF or larger electrolytic across VM and GND at the board. Never unplug the motor while powered, since live unplugging produces voltage spikes (datasheet 3.7).
- **Possible next step:** the TMC2209 can also be configured over UART (run current, microstepping, StallGuard) via the `PDN_UART` pin. Needs Pi serial enabled.
