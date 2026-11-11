#!/usr/bin/env bash
# Batería de tests de host para los módulos puros del firmware.
# Compila los módulos C-compatibles y las suites de tests, y ejecuta todo.
set -euo pipefail

cd "$(dirname "$0")"
SRC=src
BUILD=/tmp/drone_test_build
mkdir -p "$BUILD"

objs=()

# Módulos puros del firmware (compilados como C).
for f in \
    flight/pid.cpp \
    flight/mixer.cpp \
    flight/flight_control.cpp \
    flight/autotune.cpp \
    nav/nav_math.cpp \
    nav/nmea.cpp \
    nav/obstacle_logic.cpp \
    nav/mission.cpp \
    nav/autopilot.cpp \
    nav/altitude_control.cpp \
    nav/geofence.cpp \
    radio/ibus_parser.cpp \
    sensors/madgwick.cpp \
    sensors/mag_calib.cpp \
    safety/failsafe.cpp \
    safety/battery.cpp ; do
    o="$BUILD/$(echo "$f" | tr '/' '_').o"
    gcc -std=c17 -x c -I "$SRC" -c "$SRC/$f" -o "$o"
    objs+=("$o")
done

# Suites de tests.
gcc -std=c17 -x c -I "$SRC" -I tests -c tests/test_util.c -o "$BUILD/test_util.o"
objs+=("$BUILD/test_util.o")

for t in \
    tests/test_pid.c \
    tests/test_mixer.c \
    tests/test_nav_math.c \
    tests/test_nmea.c \
    tests/test_ibus.c \
    tests/test_madgwick.c \
    tests/test_mag_calib.c \
    tests/test_obstacle.c \
    tests/test_mission.c \
    tests/test_geofence.c \
    tests/test_failsafe.c \
    tests/test_battery.c \
    tests/test_autotune.c \
    tests/test_autopilot.c \
    tests/test_altitude.c \
    tests/test_integration.c ; do
    o="$BUILD/$(basename "$t" .c).o"
    gcc -std=c17 -x c -I "$SRC" -I tests -c "$t" -o "$o"
    objs+=("$o")
done

gcc -std=c17 -x c -I "$SRC" -I tests -c tests/test_main.c -o "$BUILD/test_main.o"
objs+=("$BUILD/test_main.o")

gcc "${objs[@]}" -lm -o "$BUILD/drone_host_tests"
"$BUILD/drone_host_tests"
