#!/bin/bash
set -e
clang -o test_str.out test_str.c ../src/str.c \
    -Og -fsanitize=address -rdynamic -ggdb3 \

    #-O3 -flto -march=native -mfpmath=sse \

#/usr/bin/time -vvv ./test_str.out $@ || true
./test_str.out $@ || true
rm test_str.out


