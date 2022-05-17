#include <stdio.h>
#include "SDL2/SDL.h"
#include "chip8.h"

int main(int argc, char** argv){

  struct chip8 chip8;
  chip8.registers.SP = 0;
  chip8_stack_push(&chip8, 0xff);
  chip8_stack_push(&chip8, 0xaa);
  printf("%x\n", chip8_stack_pop(&chip8));
  printf("%x\n", chip8_stack_pop(&chip8));
  chip8.registers.V[0xf] = 50;
  chip8_memory_set(&chip8.memory, 50, 'Z');
  printf("%c\n", chip8_memory_get(&chip8.memory,50));

  SDL_Init(SDL_INIT_EVERYTHING);
  SDL_Window* window = SDL_CreateWindow(
    EMULATOR_WINDOW_TITLE, 
    SDL_WINDOWPOS_UNDEFINED,
    SDL_WINDOWPOS_UNDEFINED,
    CHIP_8_WIDTH * WINDOW_RESIZER,
    CHIP_8_HEIGHT * WINDOW_RESIZER,
    SDL_WINDOW_SHOWN
  );

  SDL_Renderer* renderer = SDL_CreateRenderer(window,-1, SDL_TEXTUREACCESS_TARGET);
  while(1){
    SDL_Event event;
    // closing the program
    while(SDL_PollEvent(&event)){
      if(event.type == SDL_QUIT){
        goto out;
      }
    }
    SDL_SetRenderDrawColor(renderer,0,0,0,0); // rendering black for the display
    SDL_RenderClear(renderer); // paint over screen with black
    SDL_SetRenderDrawColor(renderer,255, 255, 255, 0); // draw white rectangle
    SDL_Rect r; r.x = 0; r.y =0; r.w = 40; r.h = 40;
    SDL_RenderFillRect(renderer, &r);
    SDL_RenderPresent(renderer);
  }
out: // destroy window
  SDL_DestroyWindow(window);
  return -1;
}