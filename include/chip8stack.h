#ifndef CHIP8STACK_H
#define CHIP8STACK_H
#include "config.h"

struct chip8; // forward declare chip8
struct chip8_stack
{
  unsigned short stack[CHIP8_TOTAL_STACK_DEPTH];
};

void chip8_stack_push(struct chip8* chip8, unsigned short val); // push return addresses to stack
unsigned short chip8_stack_pop(struct chip8* chip8); // return value from top of stack

#endif