#!/bin/bash
set -e
gcc -o test_arg.out test_arg.c ../src/arg.c \
    ../src/str2.c ../src/file.c ../src/array.c -lm \
    -Og -rdynamic -ggdb3 -fsanitize=address \

    #-lrphiic
    #-O3 -flto -march=native -mfpmath=sse \
    #Og -fsanitize=address -rdynamic -ggdb3 \

#/usr/bin/time -vvv ./test_arg.out $@ || true

./test_arg.out $@ || true
rm test_arg.out

