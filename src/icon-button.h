#ifndef ICONBUTTON_H
#define ICONBUTTON_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

using namespace std;

#define ICON_STATE_IDLE     0
#define ICON_STATE_HOVER    1
#define ICON_STATE_SELECTED 2

class IconButton {
  public:
    IconButton();
    ~IconButton();
    void setLoc(int, int, int, int);
    SDL_Rect *getLoc() { return loc; }
    void setTexture(SDL_Texture *t) { texture = t; }
    SDL_Texture *getTexture() { return texture; }
    bool isOver(int, int);
    void setState(int s) { _state = s; }
    int getState() { return _state; }
  private:
    SDL_Rect *loc;
    SDL_Texture *texture = NULL;
    int _state = ICON_STATE_IDLE;
};

#endif
