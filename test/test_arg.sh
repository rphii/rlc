#!/bin/bash
set -e
clang -o test_arg.out test_arg.c ../src/arg.c \
    ../src/str.c \
    -Og -fsanitize=address -rdynamic -ggdb3 \

    #-lrphiic
    #-O3 -flto -march=native -mfpmath=sse \

#/usr/bin/time -vvv ./test_arg.out $@ || true
./test_arg.out $@ || true
rm test_arg.out

