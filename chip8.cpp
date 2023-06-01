#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#define _OPEN_SYS_ITOA_EXT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "chip8.h"
#include "time.h"
#include <chrono>
#include <random>

#define TRUE 1
#define FALSE 0

/* memory map for chip8 emulator
   0x000 - 0x1FF: chip8 interpreter
   0x050 - 0x0A0: used for built in 4x5 pixel fontset
   0x200 - 0xFFF: instructions for ROM are stored starting at 0x200, everything else is free to use (RAM) */


/* chip-8 fontset */
uint8_t fontset[80] = {
  0xF0, 0x90, 0x90, 0x90, 0xF0, //0
  0x20, 0x60, 0x20, 0x20, 0x70, //1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
  0x90, 0x90, 0xF0, 0x10, 0x10, //4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
  0xF0, 0x10, 0x20, 0x40, 0x40, //7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
  0xF0, 0x90, 0xF0, 0x90, 0x90, //A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
  0xF0, 0x80, 0x80, 0x90, 0xF0, //C
  0xE0, 0x90, 0x90, 0x90, 0xE0, //D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
  0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

chip8::chip8(){
  /* PC starts at 0x200 */
  pc = 0x200;
  sp = 0;
  index = 0;
  opcode = 0;

  /* clear display, stack registers, memory */
  memset(V, 0, sizeof(V));
  memset(stack, 0, sizeof(stack));
  memset(mem, 0, sizeof(mem));
  memset(graphics, 0, sizeof(graphics));
  memset(keypad, 0, sizeof(keypad));
  
  /* load fontset */
  for (int i = 0; i < 80; i++) {
    mem[i] = fontset[i];
  }

  //instr = "";

  /* reset timers */
  delay_timer = 0;
  sound_timer = 0;

  drawflag = true;

  srand(time(NULL));
}


chip8::~chip8() {}


/* load a file given in command line arguments */
bool chip8::load(std::string filename) {
  std::ifstream file;
  size_t size;
  char* buffer;

  file.open(filename, std::fstream::binary);

  if (!file.is_open()) {
    printf("Error: Could not open file\n");
    return false;
  }

  char c;
  int j = 512;

  /* read through file and load contents to memory. if file is too large, return false */
  for (int i = 0x200; file.get(c); i++) {
    if (j >= 4096) {
      printf("Error: File is too large.\n");
      return false;
    }
    mem[i] = (uint8_t)c;
    j++;
  }

  return true;
}

void chip8::emulate() {

  /* fetch opcode using bitwise OR operation
     add 8 bits to mem[pc] and OR with mem[pc+1] to merge bytes */
  opcode = mem[pc] << 8 | mem[pc + 1];
  //printf("%X\n", opcode);
  int x = (opcode & 0x0F00) >> 8;
  int y = (opcode & 0x00F0) >> 4;
  int NN = opcode & 0x00FF;

  char *addr = (char*)malloc(5*sizeof(char));
  char buff[] = "V[] ";
  char res[10];
  /* decode opcode */
  switch (opcode & 0xF000) {

    case 0x0000: 
      switch (opcode & 0x000F) {
        
        /* clear display */
        case 0x0000:
          memset(graphics, 0, sizeof(graphics));
          drawflag = true;
          pc += 2;
          instr = "CLR ";
          break;

        /* return from subroutine */
        case 0x000E: 
          /* needs error checking */
          --sp;
          pc = stack[sp];
          pc += 2;
          instr = "RET ";
          break;

        default:
          printf("Invalid opcode: 0x%X\n", opcode);

      }
      break;

    /* jump to address at NNN */
    case 0x1000: 
      pc = opcode & 0x0FFF;
      instr = "JMP";
      break;

    /* call subroutine at NNN */
    case 0x2000: 
      /* store location in stack, increment stack pointer, set pc to address at NNN */
      stack[sp] = pc;
      sp++;
      pc = opcode & 0x0FFF;
      instr = "CALL";
      //addr = hex_to_string(pc);
      break;

    /* skips next instruction if VX == NN */
    case 0x3000: 
      if (V[x] == NN){
        pc += 4;
        instr = "SE";
      }
      else
        pc += 2;

      
      break;

    /* skips next instruction if VX != NN */
    case 0x4000: 
      if (V[x] != NN){
        pc += 4;
        instr = "SNE";
      }
      else
        pc += 2;
      break;

    /* skips next instruction if VX == VY */
    case 0x5000: 
      if (V[x] == V[y]){
        pc += 4;
        instr = "SE";
      }
      else
        pc += 2;
      break;


    /* sets V[x] to NN */
    case 0x6000: 
      V[x] = NN;
      pc += 2;
      instr = "LD ";
      break;


    /* adds NN to V[x] */
    case 0x7000: 
      V[x] += NN;
      pc += 2;
      //instr = "ADD";
      break;

    case 0x8000: 
      switch (opcode & 0x000F) {
        /* set V[x] to V[y] */
        case 0x0000: 
          V[x] = V[y];
          pc += 2;
          instr = "LD";
          break;

        /* sets V[x] to V[x] OR V[y] */
        case 0x0001: 
          V[x] |= V[y];
          pc += 2;
          instr = "OR";
          break;

        /* sets V[x] to V[x] AND V[y] */
        case 0x0002: 
          V[x] &= V[y];
          pc += 2;
          instr = "AND";
          break;

        /* sets V[x] to V[x] XOR V[y] */
        case 0x0003: 
          V[x] ^= V[y];
          pc += 2;
          instr = "XOR";
          break;

        /* adds V[y] to V[x], set V[f] to 1 if there is a carry, 0 else */
        case 0x0004: 
          if (V[x] + V[y] > 255){
            V[15] = 1;
          }
          else
            V[15] = 0;
          V[x] += V[y];
          pc += 2;
          instr = "ADD";
          break;

        /* subtracts V[y] from V[x], set V[f] to 0 if there is a borrow, 1 else */
        case 0x0005: 
          if (V[x] < V[y])
            V[15] = 0;
          else
            V[15] = 1;
          V[x] -= V[y];
          pc += 2;
          instr = "SUB";
          break;

        /* stores lsb of V[x] in V[f], V[x] shifts to right by 1 */
        case 0x0006: 
          V[15]=0x0;
          if (V[x] & 0x1 == 1)
            V[15] = 0x1;
          V[x] >>= 1;
          pc += 2;
          instr = "SHR";
          break;

        /* sets V[x] to V[y] - V[x]. V[f] is 0 when there is a borrow, 1 else */
        case 0x0007: 
          if (V[y] < V[x])
            V[15] = 0;
          else
            V[15] = 1;
          V[x] = V[y] - V[x];
          pc += 2;
          instr = "SUBN";
          break;

        /* stores msb of V[x] in V[f], V[x] shifts to left by 1 */
        case 0x000E: 
          V[15] = V[x] >> 7;
          V[x] <<= 1;
          pc += 2;
          instr = "SHL";
          break;

        default:
          printf("Invalid opcode (0x8000)\n");
      }
      break;

    /* skips next instruction if VX != VY */
    case 0x9000: 
      if (V[x] != V[y]){
        pc += 4;
        instr = "SNE";
      }
      else
        pc += 2;
      break;

    /* sets index to address at NNN */
    case 0xA000: 
      index = opcode & 0x0FFF;
      pc += 2;
      instr = "LD";
      break;

    /* jumps to address NNN + V[0] */
    case 0xB000: 
      pc = (opcode & 0x0FFF) + V[0];
      instr = "JMP";
      break;

    /* sets V[x] to the result of a bitwise AND operation on a random number and NN */
    case 0xC000: 
      V[x] = (rand() % 256) & (opcode & 0x00FF);
      pc += 2;
      instr = "RND";
      break;

    /* draws a sprite at coordinate (V[x], V[y]) that has a width of 8 px and a height of n+1 px. Each row of 8 pixels is bit-coded starting from memory location index (I), I value does not change after the execution of this instruction. V[f] is set to 1 when any pixels are set/unset and 0 if that does not happen. */
    case 0xD000:{ 
      uint16_t _x = V[x];
      uint16_t _y = V[y];
      uint16_t height = opcode & 0x000F;
      uint16_t pixel;

      V[15] = 0;
      for (int y_val = 0; y_val < height; y_val++) {
        pixel = mem[index + y_val];
        for (int x_val = 0; x_val < 8; x_val++) {
          if ((pixel & (0x80 >> x_val)) != 0) {
            if (graphics[_x + x_val + ((_y + y_val) * 64)] == 1)
              V[15] = 1;
            graphics[_x + x_val + ((_y + y_val) * 64)] ^= 1;
          }
        }
      }
      drawflag = true;
      pc += 2;
      break;
    }
  
    /* skips next instruction if the key stored in V[x] is pressed */
    case 0xE000: 
      switch (opcode & 0x00FF) {
        /* skips next instruction if the key stored in V[x] is pressed */
        case 0x009E: 
          if (keypad[V[x]] != 0)
            pc += 4;
          else
            pc += 2;
          break;

        /* skips next instruction if key stored in Vx is not pressed */
        case 0x00A1: 
          if (keypad[V[x]] == 0)
            pc += 4;
          else
            pc += 2;
          break;

        default:
          printf("\nUnknown opcode: %.4X\n", opcode);
      }
      break;


    case 0xF000: 
      switch (opcode & 0x00FF) {
        /* sets Vx to value of delay timer */
        case 0x0007: 
          V[x] = delay_timer;
          pc += 2;
          break;

        case 0x000A:{ 
          bool flag = false;
          for (int i = 0; i < 16; i++) {
            if (keypad[i] != 0) {
              V[x] = i;
              flag = true;
            }
          }
          if (flag == false)
            return;
          pc += 2;
          
        }
          break;



        /* sets delay timer to Vx */
        case 0x0015: 
          delay_timer = V[x];
          pc += 2;
          break;

        /* sets sound timer to V[x] */
        case 0x0018: 
          sound_timer = V[x];
          pc += 2;
          break;

        /* adds Vx to index. Vf is not affected */
        case 0x001E: 
          if (index + V[x] > 0xFFF) {
            V[0xF] = 1;
          }
          else
            V[0xF] = 0;

          index += V[x];
          pc += 2;
          break;

        /* sets index to the location of sprite for character in Vx, characters 0-F in hex are represented by 4x5 font */
        case 0x0029: 
          index = V[x] * 0x5; //16*5=80
          pc += 2;
          break;

        /* stores bcd representation of Vx w/ the msb at address index, middle bit at index+1, lsb at index+2 */
        case 0x0033: 
          mem[index] = V[x] / 100;
          mem[index + 1] = (V[x] / 10) % 10;
          mem[index + 2] = V[x] % 10;
          pc += 2;
          break;

        /* stores V0 to Vx in memory starting at address index */
        case 0x0055:
          for (int i = 0; i <= x; i++)
            mem[index + i] = V[i];
          index += x + 1;
          pc += 2;
          break;
        

        /* fills V0 to Vx w/ values in memory starting at address index */
        case 0x0065: 
          for (int i = 0; i <= x; i++)
            V[i] = mem[index + i];
          index += x + 1;
          pc += 2;
          break;
        
        default:
          printf("Invalid opcode 0x%X", opcode);
      }
      break;

    default:
      printf("Invalid opcode: 0x%X", opcode);


  }

  /* update timers */
  if (delay_timer > 0)
    --delay_timer;

  free(addr);

}

void chip8::reset_rom(){
  delay_timer = 0;
  sound_timer = 0;
  pc = 0x200;

  for (int i=0; i<16; i++){
    V[i] = 0;
    stack[i] = 0;
    keypad[i] = 0;
  }
  for (int i=0; i<4096; i++){
    mem[i] = 0;
  }
  for(int i=0; i<2048; i++){
    graphics[i] = 0;
  }
  
}
