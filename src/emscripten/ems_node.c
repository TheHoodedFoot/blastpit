// api_example.c
#include <emscripten.h>
#include <stdio.h>

EMSCRIPTEN_KEEPALIVE
void sayHi() { printf("Hi!\n"); }

EMSCRIPTEN_KEEPALIVE
int daysInWeek() { return 7; }
