#include "chip8memory.h"
#include <assert.h>

static void chip8_memory_is_in_bounds(int index){
  assert(index >= 0 && index < CHIP8_MEMORY_SIZE);
}
void chip8_memory_set(struct chip8_memory* memory, int index, unsigned char val){ // declare memory set

  chip8_memory_is_in_bounds(index); // check if we have a valid memory index and throw exception o/w
  memory->memory[index] = val;
}

unsigned char chip8_memory_get(struct chip8_memory* memory, int index){ // method to get value
  chip8_memory_is_in_bounds(index); // check if we have a valid memory index and throw exception o/w
  return memory->memory[index];
}

