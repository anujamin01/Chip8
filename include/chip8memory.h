#ifndef CHIP8MEMORY_H
#define CHIP8MEMORY_H

#include "config.h"

struct chip8_memory
{
  unsigned char memory[CHIP8_MEMORY_SIZE]; // declare memory 4kb
};

void chip8_memory_set(struct chip8_memory* memory, int index, unsigned char val); // declare memory set
unsigned char chip8_memory_get(struct chip8_memory* memory, int index); // method to get value
unsigned short chip8_memory_get_short(struct chip8_memory* memory, int index);
#endif
