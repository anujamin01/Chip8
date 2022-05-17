#include "chip8stack.h"
#include "chip8.h"
#include <assert.h>
static void chip8_stack_in_bounds(struct chip8* chip8){
  assert(chip8->registers.SP < CHIP8_TOTAL_STACK_DEPTH); // check if < 16 (16 registers)
}

void chip8_stack_push(struct chip8* chip8, unsigned short val){
  chip8_stack_in_bounds(chip8);
  chip8->stack.stack[chip8->registers.SP] = val; // update address on top of stack
  chip8->registers.SP+=1; // increment stack pointer
}

unsigned short chip8_stack_pop(struct chip8* chip8){
  chip8->registers.SP-=1; //decrease stack pointer 
  chip8_stack_in_bounds(chip8);
  unsigned short val = chip8->stack.stack[chip8->registers.SP];
  return val;
}