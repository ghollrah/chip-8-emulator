#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"
#include <stdio.h>
#include <iostream>
#include <chrono>
#include <stdint.h>
#include <unistd.h>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>

#include "tinyfiledialogs.h"
#include "chip8.h"
#include "disassembler.h"
#include "display.h"

int w=1280;
int h=720;
GLuint texture_id;

void clear_texture(){
  
  glBindTexture(GL_TEXTURE_2D, 0);

  if(texture_id != 0){
    glDeleteTextures(1, &texture_id);
    texture_id = 0;
  }
}

bool create_texture(GLuint* pixels, GLfloat width, GLfloat height){
  
  glEnable(GL_TEXTURE_2D);

  clear_texture();

  //w = width;
  //h = height;

  glGenTextures(1, &texture_id);

  glBindTexture(GL_TEXTURE_2D, texture_id);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glBindTexture(GL_TEXTURE_2D, 0);

  GLenum error = glGetError();
  if (error!=GL_NO_ERROR){
    printf("Error initializing texture: %d\n", error);
    return false;
  }

  return true;

}

bool update_texture(GLuint* pixels){
  
  glBindTexture(GL_TEXTURE_2D, texture_id);

  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 64, 32, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

  glBindTexture(GL_TEXTURE_2D, 0);

  GLenum error = glGetError();
  if(error != GL_NO_ERROR){
    printf("Error updating texture: %d\n", error);
    return false;
  }

  return true;

}

void render(GLfloat x, GLfloat y){
  
  if(display->texture_id != 0){
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(x, y, 0.f);

    glBindTexture(GL_TEXTURE_2D, texture_id);

    glBegin(GL_QUADS);
      glTexCoord2f(0.f, 0.f); glVertex2f(0.f, 0.f);
      glTexCoord2f(1.f, 0.f); glVertex2f(w, 0.f);
      glTexCoord2f(1.f, 1.f); glVertex2f(w, h);
      glTexCoord2f(0.f, 1.f); glVertex2f(0.f, h);
    glEnd();

  }

}

GLuint get_texture_id(){
  return texture_id;
}

uint8_t get_key_hex(SDL_Scancode curr_key){
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

int main(int c, char** argv){

  /*GLuint fbo;
  GLuint fbo2;
  GLuint texture_fbo;
  GLuint rbo;
*/
  bool debug_window = true;

  uint32_t pixels[2048];

  w=1280;
  h=720;
  int horizontal_scale = 1280 / 64;
  int vertical_scale = 720 / 32;
  int tile_width = horizontal_scale;
  int tile_height = vertical_scale;


  SDL_Renderer* renderer;
  SDL_Texture* texture;

  

  chip8* chip_8 = new chip8();
  disassembler* d = new disassembler();
  display* display = new display();

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER != 0)){
    printf("Error: %s\n", SDL_GetError());
    return -1;
  }

  const char* gl_version = "#version 130";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

  /* create window with graphics context */
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  SDL_DisplayMode curr;
  SDL_GetCurrentDisplayMode(0, &curr);

  SDL_WindowFlags wf = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
  
  SDL_Window* window = SDL_CreateWindow("Chip-8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, wf);
  
  if(window != nullptr){
    printf("Successfully initialized SDL window\n");
  }
  else{
    SDL_Quit();
    printf("Error: could not initialize SDL window %s\n", SDL_GetError());
  }

  /* initialize SDL_GL context */
  SDL_GLContext gl_context = SDL_GL_CreateContext(window);
  
  if(gl_context != NULL){
    printf("Successfully initialized SDL OpenGL context\n");
  }
  else{
    SDL_DestroyWindow(window);
    SDL_Quit();
    printf("Could not initialize SDL OpenGL Context. %s\n", SDL_GetError());
  }

  SDL_GL_MakeCurrent(window, gl_context);
  SDL_GL_SetSwapInterval(0);

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC);

  if(renderer != nullptr){
    printf("Successfully initialized SDL renderer\n");
  }
  else{
    SDL_DestroyWindow(window);
    SDL_Quit();
    printf("Could not initialize SDL renderer. %s\n", SDL_GetError());
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;

  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  ImGui::StyleColorsDark();

  ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
  ImGui_ImplOpenGL3_Init();
  
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.6f, 1.00f);


  /* create a texture to render */
  create_texture(chip_8->graphics, 64, 32);

  load:
    if (!chip_8->load((std::string)argv[1])) {
      printf("Error loading ROM\n");
      return 2;
    }  

  bool run = true;
  int mem_block;
  uint16_t keys = 0;
  /* main execution loop */
  while(run){
    SDL_Event e;

    while(SDL_PollEvent(&e)){
    
    /* SDL Polling events for ImGui */
      ImGui_ImplSDL2_ProcessEvent(&e);
      SDL_Scancode curr_key = e.key.keysym.scancode;
      uint8_t curr_key_hex;
      

			if (e.type == SDL_QUIT) {
				run = false;
				exit(0);
			}

			if (e.type == SDL_KEYDOWN) {
				if (e.key.keysym.sym == SDLK_ESCAPE)
					exit(0);

				if (e.key.keysym.sym == SDLK_F1)
					goto load;

				/*for (int i = 0; i < 16; i++) {
					if (curr_key == keymap[i]){
              chip_8->keypad[i] = 1;
              printf("%d\n",i);
          }
            
        }*/
        
		  }

			
			if (e.type == SDL_KEYUP) {
				/*for (int i = 0; i < 16; i++) {
					if (curr_key == keymap[i]){
						chip_8->keypad[i] = 0;
            printf("%d\n", i);
          }
				}*/
        curr_key_hex = d->get_key_hex(curr_key);
        chip_8->keypad[curr_key_hex] = 0;
			}
    }
    
    /* start ImGui frame */
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();


    /* begin emulation */
    chip_8->emulate();
    char instructions[40] = "";
    uint16_t instr = chip_8->opcode;
    d->disassemble(instructions, instr);

    /* draw pixels to screen */
		if (chip_8->drawflag) {
			chip_8->drawflag = false;

			for (int i = 0; i < 2048; i++) {
				uint8_t pixel = chip_8->graphics[i];
				pixels[i] = (0x00FFFFFF * pixel) | 0xFF000000;
			} 
    }

    update_texture(pixels);
    render(0,0);

   // ImGuiWindowFlags wf1 = (disable_mouse_wheel ? ImGuiWindowFlags_NoScrollWithMouse : 0) | (disable_menu ? 0 : ImGuiWindowFlags_MenuBar);

    
    /* ImGui windows */
    {
            
      ImGui::SetNextWindowPos(ImVec2(0,0), ImGuiCond_Once);
      ImGui::SetNextWindowSize(ImVec2(0.7*w, 0.6*h), ImGuiCond_Once);
      
      ImGui::Begin("Chip-8 Emulator", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
      
      //ImGui::BeginChild("Child1", ImVec2(screen_w*screen_factor,0), false, wf1);
      ImGui::Image((void*)(intptr_t)get_texture_id(), ImVec2(0.7*w, 0.6*h));
      //next_pos = ImGui::GetWindowSize();

      ImGui::End();
    }
      /* CPU dump */
    { 
      ImGui::SetNextWindowPos(ImVec2(0, 0.6*h), ImGuiCond_Once);
      ImGui::SetNextWindowSize(ImVec2(0.25*w, 0.4*h), ImGuiCond_Once);
      ImGui::Begin("CPU dump", NULL, ImGuiWindowFlags_NoResize);
      ImGui::Text("PC: 0x%04x", chip_8->pc);
      ImGui::Text("OP: 0x%04x", chip_8->opcode);
      //ImGui::Text("Stack Pointer: 0x%01x", chip_8->sp);
      ImGui::Text("IR: 0x%04x", chip_8->index);
      //ImGui::Text("Delay Timer: 0x%02x", chip_8->delay_timer);
      ImGui::NewLine();
      ImGui::Text("Registers:");
      for (int i=0; i<16; i++){
        if (i != 0 && i % 2 != 0)
          ImGui::SameLine();
        ImGui::Text("V%01x: 0x%02x", i, chip_8->V[i]);
      }
      
      ImGui::End(); 
    } 
      /* stack */
    {
      //int pos = ImGui::GetWindowPos();
      ImGui::SetNextWindowPos(ImVec2(0.25*w, 0.6*h), ImGuiCond_Once);
      ImGui::SetNextWindowSize(ImVec2(0.25*w, 0.4*h), ImGuiCond_Once);
      ImGui::Begin("Stack", NULL, ImGuiWindowFlags_NoResize);
      for (int i=0; i<16; i++){
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(63,195,128)));
        ImGui::Text("%X: ", i);
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::Text("%04X", chip_8->stack[i]);
        
      }
      ImGui::End();
    }
    /* memory */
    {
      ImGui::SetNextWindowPos(ImVec2(0.5*w, 0.6*h), ImGuiCond_Once);
      ImGui::SetNextWindowSize(ImVec2(0.25*w, 0.4*h), ImGuiCond_Once);
      ImGui::Begin("Memory", NULL, ImGuiWindowFlags_NoResize);
      for(int i=0; i<4096; i++){
        if(i % 8 == 0){
          mem_block = i;
          ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(178,34,34)));
          ImGui::Text("%04X", mem_block);
          ImGui::PopStyleColor();
          continue;
        }
        else{
          ImGui::SameLine();
          ImGui::Text("%02X", chip_8->mem[i]);
        }
        
      }
      ImGui::End();

    }
    /* disassembler */
    {
      ImGui::SetNextWindowPos(ImVec2(0.75*w, 0.6*h), ImGuiCond_Once);
      ImGui::SetNextWindowSize(ImVec2(0.25*w, 0.4*h), ImGuiCond_Once);
      ImGui::Begin("Disassembler", NULL, ImGuiWindowFlags_AlwaysVerticalScrollbar);
      
      for(int i=0; i<4096; i++){
        char addr[5];
        sprintf(addr, "%X", i);
328           }
        
        char ins[5];
        sprintf(ins, "%X", instr);

        ImGui::Text("0x%s | 0x%s | %s\n", addr, ins, instructions); 
      }
      ImGui::End();
    }
    /* open file dialog */
    {
      ImGui::SetNextWindowPos(ImVec2(0.7*w, 0), ImGuiCond_Once);
      ImGui::SetNextWindowSize(ImVec2(0.3*w, 0.6*h), ImGuiCond_Once);
      ImGui::Begin("ROM info", NULL, ImGuiWindowFlags_NoResize);
      
 
      if(ImGui::Button("Load ROM", ImVec2(ImGui::GetContentRegionAvail().x, 0))){
        char const * filter_patterns[2] = {"*.ch8", "*.c8"};
        char * open_file;

        open_file = tinyfd_openFileDialog("Load ROM", "", 2, filter_patterns, NULL, 0);
        chip_8->reset_rom();        
	      if (!chip_8->load((std::string)open_file)) {
		      printf("Error loading ROM\n");
		      return 2;
        }
      }

      ImGui::End();
    }

    //ImGui::EndFrame();
    ImGui::Render();
    glViewport(0,0,(int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    /*if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable){
      
      SDL_Window* backup_window = SDL_GL_GetCurrentWindow();
      SDL_GLContext backup_context = SDL_GL_GetCurrentContext();
      //ImGui::UpdatePlatformWindows();
      //ImGui::RenderPlatformWindowsDefault();
      SDL_GL_MakeCurrent(backup_window, backup_context);
    }*/

    SDL_GL_SwapWindow(window);

    usleep(2000);
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();


}
