#include "chip8.h"
#include "memory.h"
#include <assert.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include "SDL2/SDL.h"
// characters read as sprites
const char chip8_default_character_set[] = {
  0xF0,0x90,0x90,0x90,0xF0, //character 0
  0x20,0x60,0x20,0x20,0x70, //character 1
  0xF0,0x10,0xF0,0x80,0xF0, //character 2
  0xF0,0x10,0xF0,0x10,0xF0, //character 3
  0x90,0x90,0xF0,0x10,0x10, //character 4
  0xF0,0x80,0xF0,0x10,0xF0, //character 5
  0xF0,0x80,0xF0,0x90,0xF0, //character 6
  0xF0,0x10,0x20,0x40,0x40, //character 7
  0xF0,0x90,0xF0,0x90,0xF0, //character 8
  0xF0,0x90,0xF0,0x10,0xF0, //character 9
  0xF0,0x90,0xF0,0x90,0x90, //character A
  0xE0,0x90,0xE0,0x90,0xE0, //character B
  0xF0,0x80,0x80,0x80,0xF0, //character C
  0xE0,0x90,0x90,0x90,0xE0, //character D
  0xF0,0x80,0xF0,0x80,0xF0, //character E
  0xF0,0x80,0xF0,0x80,0x80  //character F
};
void chip8_init(struct chip8* chip8){
  memset(chip8, 0, sizeof(struct chip8));
  memcpy(&chip8->memory.memory, chip8_default_character_set, sizeof(chip8_default_character_set));
}
void chip8_load(struct chip8* chip8, const char* buf, size_t size){
  assert(size+CHIP8_PROGRAM_LOAD_ADDRESS < CHIP8_MEMORY_SIZE);
  memcpy(&chip8->memory.memory[CHIP8_PROGRAM_LOAD_ADDRESS],buf,size);
  chip8->registers.PC = CHIP8_PROGRAM_LOAD_ADDRESS;
}

static char chip8_wait_for_key_press(struct chip8* chip8){
  SDL_Event event;
  while(SDL_WaitEvent(&event)){
    if (event.type != SDL_KEYDOWN){
      continue;
    }
    char c = event.key.keysym.sym;
    char chip8_key = chip8_keyboard_map(&chip8->keyboard, c);
    if(chip8_key != -1){
      return chip8_key;
    }
  }
  return -1;
}
static void chip8_exec_extended_F(struct chip8* chip8, unsigned short opcode){
  unsigned char x = (opcode >> 8) & 0x000f;
  switch (opcode & 0x00ff){

    //Fx07 - LD Vx, DTSet Vx = delay timer value.
    case 0x07:
      chip8->registers.V[x] = chip8->registers.delay_timer;
    break;

    //Fx0A - LD Vx, K Wait for a key press, store the value of the key in Vx.
    case 0x0A:
    {
      char pressed_key = chip8_wait_for_key_press(chip8);
      chip8->registers.V[x] = pressed_key;
    }
    break;

    //Fx15 - LD DT, Vx Set delay timer = Vx.
    case 0x15:
      chip8->registers.delay_timer = chip8->registers.V[x];
    break;

    //Fx18 - LD ST, Vx Set sound timer = Vx.
    case 0x18:
      chip8->registers.sound_timer = chip8->registers.V[x];
    break;

    //Fx1E - ADD I, Vx Set I = I + Vx.
    case 0x1E:
      chip8->registers.I += chip8->registers.V[x];
    break;

    //Fx29 - LD F, Vx Set I = location of sprite for digit Vx.
    case 0x29:
      chip8->registers.I = chip8->registers.V[x] * CHIP8_DEFAULT_SPRITE_HEIGHT;
    break;

    //Fx33 - LD B, Vx Store BCD representation of Vx in memory locations I, I+1, and I+2.
    case 0x33:
      {
        unsigned char hundreds = chip8->registers.V[x]/100;
        unsigned char tens = chip8->registers.V[x]/10 % 10;
        unsigned char units = chip8->registers.V[x] % 10;
        chip8_memory_set(&chip8->memory, chip8->registers.I, hundreds);
        chip8_memory_set(&chip8->memory, chip8->registers.I+1, tens);
        chip8_memory_set(&chip8->memory, chip8->registers.I+2, units);
      }
    break;

    //Fx55 - LD [I], Vx Store registers V0 through Vx in memory starting at location I.
    case 0x55:
    {
      for(int i = 0; i <= x; i++){
        chip8_memory_set(&chip8->memory, chip8->registers.I+i, chip8->registers.V[i]);
      }
    }
    break;

    //Fx65 - LD Vx, [I] Read registers V0 through Vx from memory starting at location I.
    case 0x65:
    {
      for(int i = 0; i <= x; i++){
        chip8->registers.V[i] = chip8_memory_get(&chip8->memory, chip8->registers.I+i);
      }
    }
    break;
  }
}

static void chip8_exec_extended_8(struct chip8* chip8, unsigned short opcode){
  unsigned char x = (opcode >> 8) & 0x000f;
  unsigned char y = (opcode >> 4) & 0x000f;
  unsigned char final_bits = opcode & 0x000f;
  unsigned short temp = 0;
  switch(final_bits){
    case 0x00: //8xy0 LD Vx,Vy ;Vx = Vy
      chip8->registers.V[x] = chip8->registers.V[y];
    break;

    case 0x01: //8xy1 OR Vx,Vy ;Vx | Vy
      chip8->registers.V[x] |= chip8->registers.V[y];
    break;

    case 0x02: //8xy2 AND Vx,Vy ;Vx & Vy
      chip8->registers.V[x] &= chip8->registers.V[y];
    break;

    case 0x03: //8xy3 XOR Vx,Vy ;Vx & Vy
      chip8->registers.V[x] ^= chip8->registers.V[y];
    break;
    
    case 0x04: //8xy4 ADD Vx,Vy ;Vx += Vy set VF = carry
      temp = chip8->registers.V[x] + chip8->registers.V[y];
      chip8->registers.V[0xf] = false;
      if (temp > 0xff){
        chip8->registers.V[0xf] = true;  
      }
        chip8->registers.V[x] = temp;  
    break;

    case 0x05: //8xy5 LD Vx,Vy ;Vx -= Vy set VF = NOT borrow
      chip8->registers.V[0xf] = false;
      if (chip8->registers.V[x] > chip8->registers.V[y]){
        chip8->registers.V[0xf] = true;
      }
      chip8->registers.V[x] -= chip8->registers.V[y];
    break;

    case 0x06: //8xy6 SHR Vx {,Vy} ; Vx SHR 1
      chip8->registers.V[0xf] = chip8->registers.V[x] & 0x01;
      chip8->registers.V[x] /= 2;
    break;
    
    case 0x07: //8xy7 SUBN Vx,Vy ;Vx = Vy - Vx set VF = NOT borrow
      chip8->registers.V[0xf] = chip8->registers.V[y] > chip8->registers.V[x];
      chip8->registers.V[x] = chip8->registers.V[y]-chip8->registers.V[x];
    break;

    case 0x0E: //8xye SHL Vx {,Vy} ;Vx = Vx SHL 1
      chip8->registers.V[0xf] = chip8->registers.V[x] & 0b10000000;
      chip8->registers.V[x] *= 2;
    break;
  }  
}
static void chip8_exec_extended(struct chip8* chip8, unsigned short opcode){
  unsigned short nnn = opcode & 0x0fff;
  unsigned char x = (opcode >> 8) & 0x000f;
  unsigned char y = (opcode >> 4) & 0x000f;
  unsigned char kk = opcode & 0x00ff;
  unsigned char n = opcode & 0x000f;
  switch(opcode & 0xf000){
    case 0x1000: // JP address ie jump to location nnn
      chip8->registers.PC = nnn;
    break;

    case 0x2000: // CALL addr ie call to nnn
      chip8_stack_push(chip8, chip8->registers.PC);
      chip8->registers.PC = nnn;
    break;

    case 0x3000: // SE Vx, byte; Skip next instruction if Vx == kk
      if(chip8->registers.V[x] == kk){
        chip8->registers.PC+=2;
      }
    break;
    case 0x4000: // SE Vx, byte; Skip next instruction if Vx != kk
      if(chip8->registers.V[x] != kk){
        chip8->registers.PC+=2;
      }
    break;

    case 0x5000: ///5xy0 -SE,Vx,Vy; skip next instruction if Vx=Vy
      if(chip8->registers.V[x] == chip8->registers.V[y]){
        chip8->registers.PC+=2;
      }
    break;

    case 0x6000: //6xkk - LD Vx, byte; Vx = kk 
      chip8->registers.V[x] = kk;
    break;
    case 0x7000: //7xkk - ADD Vx, byte; Vx += kk 
      chip8->registers.V[x] += kk;
    break;

    case 0x8000:
      chip8_exec_extended_8(chip8,opcode);
    break;

    //9xy0 - SNE, Vx, Vy skip next instruction if Vx != Vy
    case 0x9000:
    if (chip8->registers.V[x] != chip8->registers.V[y]){
      chip8->registers.PC+=2;
    }
    break;

    case 0xA000: // Annn - LD I, addr set I = nnn
      chip8->registers.I = nnn;
    break;

    case 0xB000: // Annn - JP V0, addr Jump to location nnn + V0
      chip8->registers.PC = nnn + chip8->registers.V[0x0];
    break;

    //Cxkk - RND Vx, byte  Set Vx = random byte AND kk.
    case 0xC000:
      srand(clock()); 
      chip8->registers.V[x] = (rand() % 255) & kk;
    break;

    case 0xD000: // Dxyn - DRW Vx, Vy, nibble. Draws sprite to the screen
    { 
      const char* sprite = (const char*) &chip8->memory.memory[chip8->registers.I];
      chip8->registers.V[0x0f] = chip8_screen_draw_sprite(&chip8->screen, chip8->registers.V[x], chip8->registers.V[y], sprite, n);
    }
    break;

    case 0xE000:
    {
      //Ex9E - SKP Vx Skip next instruction if key with the value of Vx is pressed.
      switch(opcode & 0x00ff){
        case 0x9e:
          if (chip8_keyboard_is_down(&chip8->keyboard, chip8->registers.V[x])){
            chip8->registers.PC+=2;
          }
        break;

        //ExA1 - SKNP Vx Skip next instruction if key with the value of Vx is not pressed.
        case 0xa1:
          if (!chip8_keyboard_is_down(&chip8->keyboard, chip8->registers.V[x])){
            chip8->registers.PC+=2;
          }
        break;
      }
    }
    break;

    case 0xF000:
      chip8_exec_extended_F(chip8, opcode);
    break;
  }
}

void chip8_exec(struct chip8* chip8, unsigned short opcode){
  switch(opcode){
    case 0x00E0: // clear display CLS
      chip8_screen_clear(&chip8->screen);
    break;
    case 0x00EE: // RET return from subroutine
      chip8->registers.PC = chip8_stack_pop(chip8); // move program counter
    break;
    default:
      chip8_exec_extended(chip8,opcode); // use for rest of instructions
  }
}