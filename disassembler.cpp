#include "imgui.h"                                                                              
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"
#include <stdio.h>
#include <iostream>
#include <chrono>
#include <stdint.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include "chip8.h"
#include "disassembler.h"

chip8* chip_8 = new chip8();

disassembler::disassembler(){

}

disassembler::~disassembler(){}


void disassembler::convert_register(char (&result)[9], uint8_t reg){
  char converted_reg[3] = "";

  sprintf(converted_reg, "%X", reg);

  strcpy(result, "V[");
  strcpy(result+2, converted_reg);
  int idx = 0;
  while(converted_reg[idx] != '\0') idx++;
  
  strcpy(result+2+idx, "]");
  
  
}

void disassembler::hex_to_string(char (&result)[15], uint16_t addr){
  
  sprintf(result, "%X", addr);

  /* when decoding, change 0 to '0' we will print a 4 character hex opcode each time*/
  if(result[1] == '\0'){
    char temp = result[0];
    
    result[0] = '0';
    result[1] = '0';
    result[2] = '0';
    result[3] = temp;
    result[4] = '\0';
  }
  else if (result[2] == '\0'){
    char temp0 = result[0];
    char temp1 = result[1];

    result[0] = '0';
    result[1] = '0';
    result[2] = temp0;
    result[3] = temp1;
    result[4] = '\0';
  }
  else if (result[3] == '\0'){
    char temp0 = result[0];
    char temp1 = result[1];
    char temp2 = result[2];

    result[0] = '0';
    result[1] = temp0;
    result[2] = temp1;
    result[3] = temp2;
    result[4] = '\0';
  }

  //std::string res = buffer;
  //printf("%s\n", buffer);

}

uint8_t disassembler::get_key_hex(SDL_Scancode curr_key){ 
  switch(curr_key){
    case SDL_SCANCODE_1:
      return 0x1;
    case SDL_SCANCODE_2:
      return 0x2;
    case SDL_SCANCODE_3:
      return 0x3;
    case SDL_SCANCODE_4:
      return 0xC;
    case SDL_SCANCODE_Q:
      return 0x4;
    case SDL_SCANCODE_W:
      return 0x5;
    case SDL_SCANCODE_E:
      return 0x6;
    case SDL_SCANCODE_R:
      return 0xD;
    case SDL_SCANCODE_A:
      return 0x7;
    case SDL_SCANCODE_S:
      return 0x8;
    case SDL_SCANCODE_D:
      return 0x9;
    case SDL_SCANCODE_F:
      return 0xE;
    case SDL_SCANCODE_Z:
      return 0xA;
    case SDL_SCANCODE_X:
      return 0x0;
    case SDL_SCANCODE_C:
      return 0xB;
    case SDL_SCANCODE_V:
      return 0xF;
    default:
      return 45;

  }
}



void disassembler::create_instruction(char (&result)[40], char* instr, char* format, uint16_t n1, uint16_t n2){
  
  strcpy(result, instr);

  int idx = 0;
  while(result[idx] != '\0')
    idx++;
  
  result[idx] = ' ';

  if(format == "n,n"){
    result[idx] = '\0';
  }
  else if(format == "a,n"){
    char converted_addr[15];
    hex_to_string(converted_addr, n1);
    strcpy(result+idx+1, converted_addr);
  }
  else if(format == "r,a"){
    char converted_addr[15], converted_reg[9];
    hex_to_string(converted_addr, n1);
    convert_register(converted_reg, n2);
    strcpy(result+idx+1, converted_reg);
    while(result[idx] != '\0') idx++;
    strcpy(result+idx, ", ");
    strcpy(result+idx+2, converted_addr);
  }
  else if(format == "r,r"){
    char reg1[9], reg2[9];
    convert_register(reg1, n1);
    convert_register(reg2, n2);
    strcpy(result+idx+1, reg1);
    while(result[idx] != '\0') idx++;
    strcpy(result+idx, ", ");
    strcpy(result+idx+2, reg2);
  }
  else if(format == "r,n"){
    char reg1[9];
    convert_register(reg1, n1);
    strcpy(result+idx+1, reg1);
  }
  

}

void disassembler::disassemble(char (&instructions)[40], uint16_t instr){
  
  /* converts instructions into a readable format for debugging purposes
  i.e. JMP 03DC, LD V4 0A6D */

  uint8_t first_nibble = instr >> 12;
  uint8_t x = (instr & 0x0F00) >> 8;
  uint8_t y = (instr & 0x00F0) >> 4;
  uint16_t N = instr & 0xF;
  uint16_t NN = instr & 0xFF;
  uint16_t NNN = instr & 0xFFF;

  switch(first_nibble){
    case (0x0):
      switch(instr & 0xFFF){
        case (0x0E0):
          create_instruction(instructions, "CLS", "n,n", NULL, NULL);
          break;
        
        case (0x0EE):
          create_instruction(instructions, "RET", "n,n", NULL, NULL);
          break;
      }

    case (0x1):
      create_instruction(instructions, "JMP", "a,n", NNN, NULL);
      break;
    
    case (0x2):
      create_instruction(instructions, "CALL", "a,n", NNN, NULL);
      break;

    case (0x3):
      create_instruction(instructions, "SE", "r,a", x, NN);
      break;

    case (0x4):
      create_instruction(instructions, "SNE", "r,a", x, NN);
      break;

    case (0x5):
      switch(N){
        case(0x0): 
          create_instruction(instructions, "SE", "r,r", x, y);
          break;
      }
      break;

    case (0x6):
      create_instruction(instructions, "LD", "r,a", x, NN);
      break;

    case (0x7):
      create_instruction(instructions, "ADD", "r,a", x, NN);
      break;

    case (0x8):
      switch(N){
        case (0x0):
          create_instruction(instructions, "LD", "r,r", x, y);
          break;

        case (0x1):
          create_instruction(instructions, "OR", "r,r", x, y);
          break;

        case (0x2):
          create_instruction(instructions, "AND", "r,r", x, y);
          break;

        case (0x3):
          create_instruction(instructions, "XOR", "r,r", x, y);
          break;

        case (0x4):
          create_instruction(instructions, "ADD", "r,r", x, y);
          break;

        case (0x5):
          create_instruction(instructions, "SUB", "r,r", x, y);
          break;

        case (0x6):
          create_instruction(instructions, "SHR", "r,n", x, NULL);
          break;

        case (0x7):
          create_instruction(instructions, "SUBN", "r,r", x, y);
          break;

        case (0xE):
          create_instruction(instructions, "SHL", "r,n", x, NULL);
          break;
      }
      break;

      case (0x9):
        switch(N){
          case (0x0):
            create_instruction(instructions, "SNE", "r,r", x, y);
            break;
        }
        break;

      case (0xA):{
        char result[15];
        uint16_t index = chip_8->index;
        hex_to_string(result, index);

        strcpy(instructions, "LD I(0x");
        strcpy(instructions+7, result);

        int idx = 0;
        while(result[idx] != '\0') idx++;

        strcpy(instructions+7+idx, "), ");

        char result2[15];
        hex_to_string(result2, NNN);
        strcpy(instructions+7+idx+3, result2);
      }
        
        break;

      case (0xB):
        create_instruction(instructions, "JMP", "r,a", 0, NNN);
        break;

      case(0xC):
        create_instruction(instructions, "RND", "r,a", x, NN);
        break;

      case(0xD):{
        create_instruction(instructions, "DRW", "r,r", x, y);
        int idx = 0;
        
        while(instructions[idx] != '\0')
          idx++;
        
        instructions[idx] = ',';
        instructions[idx+1] = ' ';
        
        char result[4] = "";
        sprintf(result, "%d", N);
        strcpy(instructions + idx + 2, result);
      }
        break;
      case (0xE):
        switch(NN){
          case (0x9E):
            create_instruction(instructions, "SKP", "r,n", x, NULL);
            break;

          case (0xA1):
            create_instruction(instructions, "SKNP", "r,n", x, NULL);
            break;
        }
        break;

      case (0xF):
        switch(NN){
          case (0x07):{
            char result[9];
            convert_register(result,x);
            strcpy(instructions, "LD ");
            strcpy(instructions+3, result);

           /* int idx = 0;
            while(result[idx] != '\0')
              idx++;
           */

            strcpy(instructions+3+strlen(result), "DT [");
            
            char delay_timer[4];
            sprintf(delay_timer, "%d", chip_8->delay_timer);
            
            strcpy(instructions+3+strlen(result)+4+strlen(delay_timer), delay_timer);
            instructions[3+strlen(result)+4+strlen(delay_timer)] = ']';
            instructions[3+strlen(result)+4+strlen(delay_timer)+1] = '\0';
            

          }
            break;
        
          case (0x0A):{
            char result[9];
            convert_register(result,x);
            strcpy(instructions, "LD ");
            strcpy(instructions+3, result);

            strcpy(instructions+3+strlen(result), ", key");

            instructions[3+strlen(result)+5] = '\0';
          }
            break;
          
          case (0x15):{
            char result[9];
            convert_register(result,x);
            strcpy(instructions, "LD ");
            strcpy(instructions+3, "DT [");

            char res[4];
            sprintf(res, "%d", chip_8->delay_timer);

            strcpy(instructions+7, res);

            strcpy(instructions+7+strlen(res), "], ");

            strcpy(instructions+7+strlen(res)+3, result);

            instructions[7+strlen(res)+3+strlen(result)] = '\0';
          }
            break;

          case (0x18):{
            char result[9];
            convert_register(result,x);
            strcpy(instructions, "LD ");
            strcpy(instructions+3, "ST [");

            char res[4];
            sprintf(res, "%d", chip_8->sound_timer);

            strcpy(instructions+7, res);

            strcpy(instructions+7+strlen(res), "], ");

            strcpy(instructions+7+strlen(res)+3, result);

            instructions[7+strlen(res)+3+strlen(result)] = '\0';
  
          }
            break;

          case (0x1E):{
            char result[9];
            convert_register(result,x);
            strcpy(instructions, "ADD I (0x");
            
            char index_result[5];

            sprintf(index_result, "%X", chip_8->index);
            
            strcpy(instructions+9, index_result);
            strcpy(instructions+9+strlen(index_result), "), ");
            strcpy(instructions+9+strlen(index_result)+3, result);
            
          }
            break;

          case (0x29):{
            char result[9];
            char result_register[5];

            convert_register(result, x);
            sprintf(result_register, "%X", chip_8->V[x]);

            strcpy(instructions, "LD F, ");
            strcpy(instructions+6, result);
            strcpy(instructions+6+strlen(result), " (");
            strcpy(instructions+6+strlen(result)+2, result_register);

            instructions[6+strlen(result)+2+strlen(result_register)]=')';
            instructions[6+strlen(result)+2+strlen(result_register)+1]='\0';
            
          }
            break;

          case (0x33):{
            char result[9];

            convert_register(result, x);
            strcpy(instructions, "LD B, ");
            strcpy(instructions+6, result);

          }
            break;

          case (0x55):{
            char result_reg[5];
            sprintf(result_reg, "%X", x);
            strcpy(instructions, "LD [I], V");
            strcpy(instructions+9, result_reg);
          }
            break;

          case (0x65):{
            char result_reg[5];
            sprintf(result_reg, "%X", x);
            strcpy(instructions, "LD V");
            strcpy(instructions+4, result_reg);
            strcpy(instructions+4+strlen(result_reg), "[I]");
          }
            break;
        
        default:
          break;
            
        }
        break;
      default:
        break;
  }
 // printf("%s\n", instructions);
}
