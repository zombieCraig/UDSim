#include "gui.h"

Gui::Gui() {
  data_path = "../data/";
}

Gui::~Gui() {
  SDL_DestroyTexture(base_texture);
  SDL_FreeSurface(base_image);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  IMG_Quit();
  SDL_Quit();
}

int Gui::Init() {
  if(SDL_Init ( SDL_INIT_VIDEO ) < 0 ) {
        printf("SDL Could not initializes\n");
        return(-1);
  }
  window = SDL_CreateWindow("UDSim", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Gui::getWidth(), Gui::getHeight(), SDL_WINDOW_SHOWN); // | SDL_WINDOW_RESIZABLE);
  if(window == NULL) {
        printf("Window could not be shown\n");
  }
  renderer = SDL_CreateRenderer(window, -1, 0);
  base_image = Gui::load_image("udsbg.png");
  base_texture = SDL_CreateTextureFromSurface(renderer, base_image);
  Gui::Redraw();
  return 0;
}

SDL_Surface *Gui::load_image(string imgname) {
  return IMG_Load((Gui::getData() + imgname).c_str());
}

void Gui::Redraw() {
  SDL_RenderCopy(renderer, base_texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

int Gui::HandleEvents() {
    int running = 1;
    while( SDL_PollEvent(&event) != 0 ) {
        switch(event.type) {
            case SDL_QUIT:
                running = 0;
                break;
            case SDL_WINDOWEVENT:
            switch(event.window.event) {
                case SDL_WINDOWEVENT_ENTER:
                case SDL_WINDOWEVENT_RESIZED:
                        Gui::Redraw();
                break;
            }
        }
      SDL_Delay(3);
    }
    return running;
}

void Gui::Msg(string m) {
  if(verbose) { cout << m << endl; }
}
