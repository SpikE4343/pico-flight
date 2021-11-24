#!/bin/bash

INCLUDES="-I../../../../test/src \
-I../../core/math/src \
"

SRC="../../../../test/src/basic_test.c \
fixed_test.c \
"

g++ -g ${INCLUDES} ${SRC} && ./a.out