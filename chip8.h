#ifndef CHIP_8_H
#define CHIP_8_H

#include <stdint.h>
#include <string>

//using namespace std;

class chip8 {
public:
	uint16_t opcode;
  uint16_t pc;
  uint16_t sp;
  uint16_t index;
	uint8_t delay_timer, sound_timer;
	uint8_t V[16]; // 16 8-bit registers V0-VF
	uint8_t mem[4096];
	uint16_t stack[16];
	void initialize();

public:
	uint32_t graphics[64 * 32];
	uint8_t keypad[16]{};
	bool drawflag;
  std::string instr;

	chip8();
	~chip8();

	void emulate();
	bool load(std::string filename);
	void render();
	void reset_rom();
};

#endif //CHIP_8_H
