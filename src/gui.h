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
// Info Card region
#define CARD_REGION_X 0
#define CARD_REGION_Y 50
#define CARD_REGION_H 410
#define CARD_REGION_W 240
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

// Card Options
#define CARD_FAKE_RESP_X 20
#define CARD_FAKE_RESP_Y 420
#define CARD_FAKE_RESP_H 20
#define CARD_FAKE_RESP_W 20

// Animation states
#define CARD_NOANIM  0
#define CARD_ADVANCE 1
#define CARD_RETRACT 2

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
  void DrawModules(bool b = false);
  void DrawLog();
  void DrawToolbar();
  void DrawStatus();
  void DrawInfoCard();
  int HandleEvents();
  void HandleAnimations();
  void Msg(string);
  void setCardState(int s) { _card_state = s; }
  int getCardState() { return _card_state; }
  void AdvanceCard();
  void RetractCard();

  private:
  void HandleMouseMotions(SDL_MouseMotionEvent);
  void HandleMouseClick(SDL_MouseButtonEvent);
  bool isOverCarRegion(int, int);
  bool isOverToolbarRegion(int, int);
  bool isOverCardRegion(int, int);
  int width = 720;
  int height = 486;
  int verbose = 0;
  int module_selected = -1;
  unsigned int _ticks = 0;
  unsigned int _lastTicks = 0;
  unsigned int _CardAnimationTicks = 10;
  int _card_current_x = -CARD_REGION_W;
  int _card_state = CARD_NOANIM;
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
  SDL_Texture *module_unk_texture = NULL;
  SDL_Texture *info_card_texture = NULL;
  SDL_Texture *_status = NULL;
  SDL_Texture *check_texture = NULL;
  SDL_Event event;
  TTF_Font *module_ttf = NULL;
  TTF_Font *log_ttf = NULL;
  TTF_Font *title_ttf = NULL;
};

#endif
