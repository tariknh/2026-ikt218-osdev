gcc \
    ../src/utils/split_int_test.c \
    ../src/utils/split_int.c \
    ../include/unity/unity.c \
    -I ../include/libc/ \
    -I ../include/unity/ \
    -o test.out
./test.out

gcc \
    ../include/unity/unity.c \
    ../src/interrupts/interrupts.c \
    ../src/interrupts/interrupts_test.c \
    -I ../include/libc/ \
    -I ../include/unity/ \
    -I ../src/interrupts/ \
    -o test.out \
    -masm=intel
./test.out

rm test.out