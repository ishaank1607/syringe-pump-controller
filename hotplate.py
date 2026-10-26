"""IKA RCT hotplate over USB serial. Read-only unless --set is given.

The serial settings and command names below are from memory of IKA's NAMUR-style
protocol and have NOT been checked against the RCT manual or the device. Run it
with no flags first and compare the replies with the manual's command table.
"""
import argparse
import sys
import time

import serial

PORT = "/dev/ttyUSB0"       # find it with: ls /dev/ttyUSB* /dev/ttyACM*
BAUD = 9600
BYTESIZE = serial.SEVENBITS
PARITY = serial.PARITY_EVEN
STOPBITS = serial.STOPBITS_ONE

CMD_NAME = "IN_NAME"
CMD_READ_PLATE_TEMP = "IN_PV_2"    # plate sensor; IN_PV_1 would be an external probe
CMD_READ_SETPOINT = "IN_SP_1"
CMD_SET_TEMP = "OUT_SP_1"
CMD_HEAT_ON = "START_1"
CMD_HEAT_OFF = "STOP_1"
CMD_WATCHDOG = "OUT_WD1@"          # + seconds; heater shuts off if not refreshed in time

MAX_SETPOINT_C = 100               # refuse anything higher; raise deliberately
POLL_S = 1.0
MAX_FAILS = 5


def send(ser, cmd):
    ser.write(f"{cmd}\r\n".encode("ascii"))


def query(ser, cmd):
    ser.reset_input_buffer()
    send(ser, cmd)
    return ser.readline().decode("ascii", errors="replace").strip()


def parse_value(reply):
    # replies look like "23.4 2" (value, then channel); keep the first token
    return float(reply.split()[0])


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--port", default=PORT)
    ap.add_argument("--set", type=float, metavar="C",
                    help="setpoint in C; turns heating ON (omit for read-only)")
    ap.add_argument("--watchdog", type=int, metavar="S",
                    help="refresh the device watchdog every poll (device limit usually 20-1500 s)")
    ap.add_argument("--log", metavar="FILE", help="append 'unix_time,plate_temp' lines")
    args = ap.parse_args(argv)

    if args.set is not None and not 0 <= args.set <= MAX_SETPOINT_C:
        sys.exit(f"refusing setpoint {args.set} C (allowed 0-{MAX_SETPOINT_C}, see MAX_SETPOINT_C)")

    log = open(args.log, "a") if args.log else None
    heating = False
    try:
        with serial.Serial(args.port, BAUD, bytesize=BYTESIZE, parity=PARITY,
                           stopbits=STOPBITS, timeout=1) as ser:
            print("device:", query(ser, CMD_NAME) or "<no reply - check port and serial settings>")
            try:
                if args.set is not None:
                    send(ser, f"{CMD_SET_TEMP} {args.set:.0f}")
                    send(ser, CMD_HEAT_ON)
                    heating = True
                    print("setpoint reads back as:", query(ser, CMD_READ_SETPOINT) or "<no reply>")

                fails = 0
                while True:
                    t0 = time.monotonic()
                    try:
                        temp = parse_value(query(ser, CMD_READ_PLATE_TEMP))
                        fails = 0
                        print(f"{time.strftime('%H:%M:%S')}  plate {temp:6.1f} C")
                        if log:
                            log.write(f"{time.time():.1f},{temp}\n")
                            log.flush()
                    except (ValueError, IndexError):
                        fails += 1
                        print("no/bad reply from hotplate")
                        if fails >= MAX_FAILS:
                            raise RuntimeError("lost contact with the hotplate")
                    if args.watchdog:
                        send(ser, f"{CMD_WATCHDOG}{args.watchdog}")
                    time.sleep(max(0.0, POLL_S - (time.monotonic() - t0)))
            except KeyboardInterrupt:
                pass
            finally:
                if heating:
                    try:
                        send(ser, CMD_HEAT_OFF)
                        print("heating stopped")
                    except Exception:
                        print("WARNING: could not send stop - turn the hotplate off by hand")
    finally:
        if log:
            log.close()


if __name__ == "__main__":
    main()
