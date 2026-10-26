"""Constant-rate leadscrew syringe pump, built on stepper.py.

Fill in the three hardware constants below before running. Use --dry-run first,
then run once with no syringe attached and check the direction and the travel.
"""
import argparse
import math
import sys
from time import perf_counter, sleep

from stepper import STEPS_PER_REV, direction, motor_off, motor_on, step

LEAD_MM = None             # leadscrew travel per revolution (e.g. 8 for a T8x8 screw)
SYRINGE_ID_MM = None       # syringe inner diameter
SYRINGE_VOLUME_ML = None   # full stroke; assumes you start with the plunger fully back

PUSH_CLOCKWISE = True      # flip if the plunger moves the wrong way
FLOW_CAL = 1.0             # measured / commanded volume; set after weighing a test dispense
MAX_HZ = 1500              # same ceiling as stepper.move()
STEP_PULSE_S = 20e-6


def ml_per_step():
    area_mm2 = math.pi * (SYRINGE_ID_MM / 2) ** 2
    return (LEAD_MM / STEPS_PER_REV) * area_mm2 / 1000 * FLOW_CAL


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--volume-ml", type=float, required=True)
    ap.add_argument("--rate-ml-min", type=float, required=True)
    ap.add_argument("--hold", action="store_true",
                    help="keep the motor energized afterward instead of releasing it")
    ap.add_argument("--dry-run", action="store_true", help="print the plan and exit")
    args = ap.parse_args(argv)

    if None in (LEAD_MM, SYRINGE_ID_MM, SYRINGE_VOLUME_ML):
        sys.exit("Set LEAD_MM, SYRINGE_ID_MM and SYRINGE_VOLUME_ML at the top of pump.py first.")
    if args.volume_ml <= 0 or args.rate_ml_min <= 0:
        sys.exit("volume and rate must be positive")
    if args.volume_ml > SYRINGE_VOLUME_ML:
        sys.exit(f"{args.volume_ml} mL is more than the {SYRINGE_VOLUME_ML} mL stroke")

    mps = ml_per_step()
    steps = round(args.volume_ml / mps)
    hz = args.rate_ml_min / 60 / mps
    if hz > MAX_HZ:
        sys.exit(f"{hz:.0f} steps/s is above MAX_HZ={MAX_HZ}; lower the rate")

    print(f"{mps * 1000:.3f} uL/step, {steps} steps at {hz:.2f} steps/s, "
          f"about {steps / hz / 60:.1f} min")
    if args.dry_run:
        return

    completed = False
    try:
        motor_on()
        direction.value = 1 if PUSH_CLOCKWISE else 0
        sleep(0.001)  # DIR setup time

        interval = 1 / hz
        t_next = perf_counter()
        worst_lag = 0.0
        report_every = max(1, steps // 10)
        for i in range(steps):
            step.on()
            sleep(STEP_PULSE_S)
            step.off()
            # absolute deadlines, so timing error doesn't accumulate into flow error
            t_next += interval
            delay = t_next - perf_counter()
            if delay > 0:
                sleep(delay)
            else:
                worst_lag = max(worst_lag, -delay)
            if (i + 1) % report_every == 0:
                print(f"{100 * (i + 1) / steps:.0f}%  ({(i + 1) * mps:.3f} mL)")
        completed = True
        print(f"done; worst step lateness {worst_lag * 1000:.2f} ms")
    finally:
        if not (completed and args.hold):
            motor_off()


if __name__ == "__main__":
    main()
