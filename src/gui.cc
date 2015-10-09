#include "gui.h"

Gui::Gui() {
  data_path = "../data/";
  srand(time(NULL));
}

Gui::~Gui() {
  SDL_DestroyTexture(module_texture);
  SDL_DestroyTexture(base_texture);
  SDL_FreeSurface(module_image);
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
  module_image = Gui::load_image("module.png");
  base_texture = SDL_CreateTextureFromSurface(renderer, base_image);
  module_texture = SDL_CreateTextureFromSurface(renderer, module_image);
  Gui::Redraw();
  return 0;
}

SDL_Surface *Gui::load_image(string imgname) {
  return IMG_Load((Gui::getData() + imgname).c_str());
}

void Gui::DrawModules() {
  SDL_Rect update, pos;
  int state;
  vector <Module *>modules = gd.get_active_modules();
  for(vector<Module *>::iterator it = modules.begin(); it != modules.end(); ++it) {
    Module *mod = *it;
    if(mod->getX() == 0 && mod->getY() == 0) { // Initialize location
      mod->setX(260 + (rand() % 100));
      mod->setY(80 + rand() % 100);
    }
    update.x = mod->getX();
    update.y = mod->getY();
    update.w = MODULE_W;
    update.h = MODULE_H;
    SDL_RenderCopy(renderer, base_texture, &update, &update);
    memcpy(&pos, &update, sizeof(SDL_Rect));
    update.x = 0;
    update.y = 0;
    state = mod->getState();
    switch(state) {
      case STATE_IDLE:
        break;
      case STATE_ACTIVE:
        update.x = MODULE_W;
        break;
      case STATE_MOUSEOVER:
        update.x = MODULE_W * 2;
        break;
      case STATE_SELECTED:
        update.x = MODULE_W * 3;
        break;
      default:
        break;
    }
    SDL_RenderCopy(renderer, module_texture, &update, &pos);
  }
}

void Gui::Redraw() {
  SDL_RenderCopy(renderer, base_texture, NULL, NULL);
  Gui::DrawModules();
  SDL_RenderPresent(renderer);
}

void Gui::HandleMouseMotions(SDL_MouseMotionEvent motion) {
  int x = motion.x;
  int y = motion.y;
  bool change = false;
  /* Check to see if hovering over a module */
  vector<Module *>modules = gd.get_active_modules();
  for(vector<Module *>::iterator it = modules.begin(); it != modules.end(); ++it) {
    Module *mod = *it;
    if(x > mod->getX() && x < mod->getX() + MODULE_W && y > mod->getY() && y < mod->getY() + MODULE_H) {
      if(mod->getState() == STATE_SELECTED) {
        mod->setX(mod->getX() + motion.xrel);
        mod->setY(mod->getY() + motion.yrel);
        change = true;
      } else if(mod->getState() != STATE_MOUSEOVER) {
        mod->setState(STATE_MOUSEOVER);
        change = true;
      }
    } else if (mod->getState() == STATE_MOUSEOVER) {
      mod->setState(STATE_IDLE);
      change = true;
    }
  }
  if(change) {
    Gui::DrawModules();
    SDL_RenderPresent(renderer);
  }
}

bool Gui::isOverCarRegion(int x, int y) {
  if(x > CAR_REGION_X && x < CAR_REGION_X + CAR_REGION_W &&
     y > CAR_REGION_Y && y < CAR_REGION_Y + CAR_REGION_H) return true;
  return false;
}

void Gui::HandleMouseClick(SDL_MouseButtonEvent button) {
  bool change = false;
  if(button.button == SDL_BUTTON_LEFT) {
    if(button.state == SDL_PRESSED) {
      if(Gui::isOverCarRegion(button.x, button.y)) {
        vector<Module *>modules = gd.get_active_modules();
        for(vector<Module *>::iterator it = modules.begin(); it != modules.end(); ++it) {
          Module *mod = *it;
          if(button.x > mod->getX() && button.x < mod->getX() + MODULE_W &&
             button.y > mod->getY() && button.y < mod->getY() + MODULE_H) {
               mod->setState(STATE_SELECTED);
               change = true;
          }
        }
      }
    } else if (button.state == SDL_RELEASED) {
      if(Gui::isOverCarRegion(button.x, button.y)) {
        vector<Module *>modules = gd.get_active_modules();
        for(vector<Module *>::iterator it = modules.begin(); it != modules.end(); ++it) {
          Module *mod = *it;
          if(button.x > mod->getX() && button.x < mod->getX() + MODULE_W &&
             button.y > mod->getY() && button.y < mod->getY() + MODULE_H) {
               mod->setState(STATE_MOUSEOVER);
               change = true;
          }
        }
      }
    }
  }
  if(change) {
    Gui::DrawModules();
    SDL_RenderPresent(renderer);
  }
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
            case SDL_MOUSEMOTION:
                Gui::HandleMouseMotions(event.motion);
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                Gui::HandleMouseClick(event.button);
                break;
        }
      SDL_Delay(3);
    }
    return running;
}

void Gui::Msg(string m) {
  if(verbose) { cout << m << endl; }
}
