from gpiozero import OutputDevice
from time import sleep

STEP_PIN, DIR_PIN, EN_PIN = 17, 27, 22

FULL_STEPS_PER_REV = 200   # 1.8 deg motor
MICROSTEPS = 8             # must match MS1/MS2 wiring (default 1/8)
STEPS_PER_REV = FULL_STEPS_PER_REV * MICROSTEPS

step = OutputDevice(STEP_PIN)
direction = OutputDevice(DIR_PIN)
enable = OutputDevice(EN_PIN, initial_value=True)  # EN is active LOW: high = disabled


def motor_on():
    enable.off()   # pull EN low -> driver enabled
    # StealthChop2 auto-tunes current regulation while stationary at run current
    # (datasheet 6.1, needs >130 ms standstill before the first move)
    sleep(0.2)


def motor_off():
    enable.on()    # release the motor (no holding torque)


def move(steps, clockwise=True, start_hz=300, max_hz=1500, ramp_steps=400):
    """Move `steps` microsteps with a linear speed ramp up and down.

    Motor is XY42STH34-0354A (12V, 0.35A, 34 ohm, 33 mH). High inductance means
    torque falls off quickly with speed, so max_hz is kept conservative:
    1500 steps/s at 1/8 microstepping is ~0.94 rev/s (~56 RPM). Raise it slowly.
    """
    direction.value = 1 if clockwise else 0
    sleep(0.001)  # DIR setup time

    ramp = min(ramp_steps, steps // 2)
    for i in range(steps):
        # distance from nearest end of the move -> accel/decel profile
        edge = min(i, steps - 1 - i)
        if ramp and edge < ramp:
            hz = start_hz + (max_hz - start_hz) * edge / ramp
        else:
            hz = max_hz
        half_period = 1 / (2 * hz)
        step.on()
        sleep(half_period)
        step.off()
        sleep(half_period)


if __name__ == "__main__":
    try:
        motor_on()
        move(STEPS_PER_REV, clockwise=True)    # 1 rev one way
        sleep(0.5)
        move(STEPS_PER_REV, clockwise=False)   # 1 rev back
    finally:
        motor_off()
