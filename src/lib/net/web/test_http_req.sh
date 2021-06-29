#!/bin/bash

INCLUDES="-I../../test/src \
"

SRC="../../test/src/basic_test.c \
http.c \
"

DEFINES="-DTEST=1 \
"

gcc -g ${INCLUDES} ${DEFINES} ${SRC} -o test_http_req.out && ./test_http_req.out