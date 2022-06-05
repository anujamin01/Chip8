#include <stdio.h>
#include "SDL2/SDL.h"
#include "chip8.h"
#include "chip8keyboard.h"
#include "Windows.h"
// mapping desktop keys to virtual keys
const char keyboard_map[CHIP8_TOTAL_KEYS] = {
  SDLK_0,SDLK_1,SDLK_2,SDLK_3, 
  SDLK_4,SDLK_5,SDLK_6,SDLK_7,
  SDLK_8,SDLK_9,SDLK_a,SDLK_b,
  SDLK_c,SDLK_d,SDLK_e,SDLK_f,
};

int main(int argc, char** argv){
  if (argc < 2){
    printf("You must provide file to load\n");
    return -1;
  }
  const char* filename = argv[1];
  printf("The filename to load is: %s\n", filename);

  FILE* f = fopen(filename, "rb");
  if (!f){
    printf("Failed to open file\n");
    return -1;
  }

  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  fseek(f,0,SEEK_SET);

  char buf[size];
  int res = fread(buf,size,1,f);

  if (res != 1){
    printf("Failed to read from file");
    return -1;
  }

  struct chip8 chip8;
  chip8_init(&chip8);
  chip8_load(&chip8, buf, size);
  chip8_keyboard_set_map(&chip8.keyboard, keyboard_map);
  //chip8.registers.delay_timer = 255;
  //chip8.registers.sound_timer = 30;
  //chip8_screen_draw_sprite(&chip8.screen,62,30,&chip8.memory.memory[0x00],5);
  //chip8_screen_set(&chip8.screen,10,1);
  //chip8_keyboard_down(&chip8.keyboard, 0xf);
  //chip8_keyboard_up(&chip8.keyboard, 0xf);
  //bool is_down = chip8_keyboard_is_down(&chip8.keyboard, 0xf);
  //printf("%i\n", (int)is_down);
  /*
  chip8.registers.SP = 0;
  chip8_stack_push(&chip8, 0xff);
  chip8_stack_push(&chip8, 0xaa);
  printf("%x\n", chip8_stack_pop(&chip8));
  printf("%x\n", chip8_stack_pop(&chip8));
  chip8.registers.V[0xf] = 50;
  chip8_memory_set(&chip8.memory, 50, 'Z');
  printf("%c\n", chip8_memory_get(&chip8.memory,50));
  */
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
      switch(event.type){
        case SDL_QUIT:
          goto out;
        break;

        case SDL_KEYDOWN:
        {
          char key = event.key.keysym.sym;
          int vkey = chip8_keyboard_map(&chip8.keyboard,key);
          //printf("key is down %x\n", vkey);

          if (vkey != -1){
            chip8_keyboard_down(&chip8.keyboard,vkey);
          }
        }
        break;
        case SDL_KEYUP:
        {
          char key = event.key.keysym.sym;
          int vkey = chip8_keyboard_map(&chip8.keyboard,key);
          //printf("key is down %x\n", vkey);

          if (vkey != -1){
            chip8_keyboard_up(&chip8.keyboard,vkey);
          }
        }
          //printf("key is up\n");
        break;
      };
    }

    SDL_SetRenderDrawColor(renderer,0,0,0,0); // rendering black for the display
    SDL_RenderClear(renderer); // paint over screen with black
    SDL_SetRenderDrawColor(renderer,255, 255, 255, 0); // draw white rectangle

    // loop through each pixels
    for (int x = 0; x < CHIP_8_WIDTH; x++){
      for (int y = 0; y < CHIP_8_HEIGHT; y++){
        if(chip8_screen_is_set(&chip8.screen,x,y)){ // if pixel is set draw it 
          SDL_Rect r;
          r.x = x * WINDOW_RESIZER;
          r.y = y * WINDOW_RESIZER;
          r.w = WINDOW_RESIZER; 
          r.h = WINDOW_RESIZER;
          SDL_RenderFillRect(renderer, &r);
        }
      }
    }

    SDL_RenderPresent(renderer);

    if(chip8.registers.delay_timer > 0){
      Sleep(1);
      chip8.registers.delay_timer-=1;
      //printf("Delay\n");
    }

    if(chip8.registers.sound_timer > 0){
      Beep(15000, 100 * chip8.registers.sound_timer);
      chip8.registers.sound_timer=0;
    }
    unsigned short opcode = chip8_memory_get_short(&chip8.memory, chip8.registers.PC); // read short/opcode
    chip8.registers.PC +=2; // increment PC due to instruction
    chip8_exec(&chip8,opcode); // execute instruction
  }
out: // destroy window
  SDL_DestroyWindow(window);
  return -1;
}