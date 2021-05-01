#!/bin/bash

INCLUDES="-I../../test/src \
-I../../core/math/src \
"

SRC="../../test/src/basic_test.c \
motor_mixer_test.c \
../src/motor_mixer.c \
"

g++ -g ${INCLUDES} ${SRC} && ./a.out