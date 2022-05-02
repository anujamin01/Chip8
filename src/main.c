#include <stdio.h>
#include "SDL2/SDL.h"

int main(int argc, char** argv){

  SDL_Init(SDL_INIT_EVERYTHING);
  SDL_Window* window = SDL_CreateWindow(
    "Chip8 Window", 
    SDL_WINDOWPOS_UNDEFINED,
    SDL_WINDOWPOS_UNDEFINED,
    640, 320, SDL_WINDOW_SHOWN
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
  return 0;
}