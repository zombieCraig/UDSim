#ifndef UDSSIM_GUI_H
#define UDSSIM_GUI_H

#include <vector>
#include <iostream>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "gamedata.h"
#include "module.h"

using namespace std;

// The region of the vehicle on the screen
#define CAR_REGION_X 245
#define CAR_REGION_Y 58
#define CAR_REGION_H 327
#define CAR_REGION_W 165

extern GameData gd;

class Gui {
  public:
  Gui();
  ~Gui();
  void setWidth(int w) { width = w; }
  void setHeight(int h) { height = h; }
  int getWidth() { return width; }
  int getHeight() { return height; }
  int Init();
  void setVerbose(int v) { verbose = v; }
  void setData(string s) { data_path = s; }
  string getData() { return data_path; }
  SDL_Surface *load_image(string);
  void Redraw();
  int HandleEvents();
  void Msg(string);

  private:
  void DrawModules();
  void HandleMouseMotions(SDL_MouseMotionEvent);
  void HandleMouseClick(SDL_MouseButtonEvent);
  bool isOverCarRegion(int, int);
  int width = 720;
  int height = 486;
  int verbose = 0;
  string data_path;
  SDL_Window *window = NULL;
  SDL_Surface *screen = NULL;
  SDL_Renderer *renderer = NULL;
  SDL_Surface *base_image = NULL;
  SDL_Texture *base_texture = NULL;
  SDL_Surface *module_image = NULL;
  SDL_Texture *module_texture = NULL;
  SDL_Event event;
};

#endif
