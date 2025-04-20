#!/bin/bash
set -e
tcc -o test_arg.out test_arg.c ../src/arg.c \
    ../src/str.c ../src/file.c \
    -Og -rdynamic -ggdb3 \

    #-lrphiic
    #-O3 -flto -march=native -mfpmath=sse \
    #Og -fsanitize=address -rdynamic -ggdb3 \

#/usr/bin/time -vvv ./test_arg.out $@ || true

./test_arg.out $@ || true
rm test_arg.out

