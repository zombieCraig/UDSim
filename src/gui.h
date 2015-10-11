#ifndef UDSSIM_GUI_H
#define UDSSIM_GUI_H

#include <vector>
#include <iostream>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "gamedata.h"
#include "module.h"

using namespace std;

#define MAX_LOG_ENTRIES 23
// The region of the vehicle on the screen
#define CAR_REGION_X 245
#define CAR_REGION_Y 58
#define CAR_REGION_H 327
#define CAR_REGION_W 165
// Log window region
#define LOG_REGION_X 450
#define LOG_REGION_Y 40
#define LOG_REGION_H 446
#define LOG_REGION_W 270

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
  void setFontPath(string f) { font_path = f; }
  string getData() { return data_path; }
  string getFontPath() { return font_path; }
  SDL_Surface *load_image(string);
  TTF_Font *load_font(string, int);
  void Redraw();
  int HandleEvents();
  void Msg(string);

  private:
  void DrawModules();
  void DrawLog();
  void HandleMouseMotions(SDL_MouseMotionEvent);
  void HandleMouseClick(SDL_MouseButtonEvent);
  bool isOverCarRegion(int, int);
  int width = 720;
  int height = 486;
  int verbose = 0;
  string data_path;
  string font_path;
  string logbuff[MAX_LOG_ENTRIES];
  SDL_Window *window = NULL;
  SDL_Surface *screen = NULL;
  SDL_Renderer *renderer = NULL;
  SDL_Surface *base_image = NULL;
  SDL_Texture *base_texture = NULL;
  SDL_Surface *module_image = NULL;
  SDL_Texture *module_texture = NULL;
  SDL_Event event;
  TTF_Font *module_ttf = NULL;
  TTF_Font *log_ttf = NULL;
};

#endif
