#include "icon-button.h"

IconButton::IconButton() {
  loc = (SDL_Rect *)malloc(sizeof(SDL_Rect));
  loc->x = 0;
  loc->y = 0;
  loc->w = 0;
  loc->h = 0;
}

IconButton::~IconButton() {
  free(loc);
}

void IconButton::setLoc(int x, int y, int h, int w) {
  loc->x = x;
  loc->y = y;
  loc->h = h;
  loc->w = w;
}

bool IconButton::isOver(int x, int y) {
  if(x > loc->x && x < loc->x + loc->w &&
     y > loc->y && y < loc->y + loc->h)
       return true;
  return false;
}
