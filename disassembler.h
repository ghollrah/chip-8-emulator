#pragma once

#include "chip8.h"

class disassembler{

public:
  void convert_register(char (&result)[9], uint8_t reg);
  void hex_to_string(char (&result)[15], uint16_t addr);
  uint8_t get_key_hex(SDL_Scancode curr_key);
  void create_instruction(char (&result)[40], char* instr, char* format, uint16_t n1, uint16_t n2);
  void disassemble(char (&instructions)[40], uint16_t instr);

public:
  disassembler();
  ~disassembler();
};
