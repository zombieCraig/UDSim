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
#include "icon-button.h"

using namespace std;

#define MAX_LOG_ENTRIES 22
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
// Toolbar region
#define TOOLBAR_REGION_X 450
#define TOOLBAR_REGION_Y 0
#define TOOLBAR_REGION_H 50
#define TOOLBAR_REGION_W 270
// Status region
#define STATUS_REGION_X 2
#define STATUS_REGION_Y 466
#define STATUS_REGION_H 20
#define STATUS_REGION_W 240
// Save Icon
#define ICON_SAVE_X 453
#define ICON_SAVE_Y 0
#define ICON_SAVE_H 50
#define ICON_SAVE_W 50
// Mode Icon
#define ICON_MODE_X 506
#define ICON_MODE_Y 0
#define ICON_MODE_H 50
#define ICON_MODE_W 50

class GameData;

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
  void setStatus(string);
  void Redraw();
  int HandleEvents();
  void Msg(string);

  private:
  void DrawModules();
  void DrawLog();
  void DrawToolbar();
  void DrawStatus();
  void HandleMouseMotions(SDL_MouseMotionEvent);
  void HandleMouseClick(SDL_MouseButtonEvent);
  bool isOverCarRegion(int, int);
  bool isOverToolbarRegion(int, int);
  int width = 720;
  int height = 486;
  int verbose = 0;
  string data_path;
  string font_path;
  IconButton *saveButton = NULL;
  IconButton *modeButton = NULL;
  SDL_Texture *logbuff[MAX_LOG_ENTRIES];
  SDL_Window *window = NULL;
  SDL_Surface *screen = NULL;
  SDL_Renderer *renderer = NULL;
  SDL_Texture *base_texture = NULL;
  SDL_Texture *module_texture = NULL;
  SDL_Texture *_status;
  SDL_Event event;
  TTF_Font *module_ttf = NULL;
  TTF_Font *log_ttf = NULL;
};

#endif
