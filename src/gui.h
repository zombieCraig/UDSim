#ifndef UDSSIM_GUI_H
#define UDSSIM_GUI_H

#include <iostream>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

using namespace std;

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
  int width = 720;
  int height = 486;
  int verbose = 0;
  string data_path;
  SDL_Window *window = NULL;
  SDL_Surface *screen = NULL;
  SDL_Renderer *renderer = NULL;
  SDL_Surface *base_image = NULL;
  SDL_Texture *base_texture = NULL;
  SDL_Event event;
};

#endif
