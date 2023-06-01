#pragma once
#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>
#include <string>

class display{

public:
  void clear_texture();
  bool create_texture(GLuint* pixels, GLfloat width, GLfloat height);
  bool update_texture(GLuint* pixels);
  void render(GLfloat x, GLfloat y);
  GLuint get_texture_id();

  //int w;
  //int h;
  //GLuint texture_id;

  uint8_t keymap[16] = {
  SDL_SCANCODE_X,
  SDL_SCANCODE_1,
  SDL_SCANCODE_2,
  SDL_SCANCODE_3,
  SDL_SCANCODE_Q,
  SDL_SCANCODE_W,
  SDL_SCANCODE_E,
  SDL_SCANCODE_A,
  SDL_SCANCODE_S,
  SDL_SCANCODE_D,
  SDL_SCANCODE_Z,
  SDL_SCANCODE_C,
  SDL_SCANCODE_4,
  SDL_SCANCODE_R,
  SDL_SCANCODE_F,
  SDL_SCANCODE_V,
  };


public:
  display();
  ~display();

};
