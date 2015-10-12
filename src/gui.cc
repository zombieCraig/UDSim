#include "gui.h"

Gui::Gui() {
  int i;
  for(i = 0; i < MAX_LOG_ENTRIES; i++) { logbuff[i] = NULL; }
  data_path = "../data/";
  font_path = "../data/fonts/";
  srand(time(NULL));
}

Gui::~Gui() {
  SDL_DestroyTexture(module_texture);
  SDL_DestroyTexture(base_texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  IMG_Quit();
  TTF_Quit();
  SDL_Quit();
}

int Gui::Init() {
  SDL_Surface *base_image, *module_image, *icon_save_image;
  if(SDL_Init ( SDL_INIT_VIDEO ) < 0 ) {
        printf("SDL Could not initializes\n");
        return(-1);
  }
  if(TTF_Init() < 0) {
        printf("TTF Could not be initialized\n");
        return -2;
  }
  window = SDL_CreateWindow("UDSim", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Gui::getWidth(), Gui::getHeight(), SDL_WINDOW_SHOWN); // | SDL_WINDOW_RESIZABLE);
  if(window == NULL) {
        printf("Window could not be shown\n");
  }
  renderer = SDL_CreateRenderer(window, -1, 0);
  base_image = Gui::load_image("udsbg.png");
  module_image = Gui::load_image("module.png");
  icon_save_image = Gui::load_image("save_icon.png");
  base_texture = SDL_CreateTextureFromSurface(renderer, base_image);
  module_texture = SDL_CreateTextureFromSurface(renderer, module_image);
  SDL_FreeSurface(module_image);
  SDL_FreeSurface(base_image);
  // Toolbar
  saveButton = new IconButton();
  saveButton->setLoc(ICON_SAVE_X, ICON_SAVE_Y, ICON_SAVE_H, ICON_SAVE_W);
  saveButton->setTexture(SDL_CreateTextureFromSurface(renderer, icon_save_image));
  SDL_FreeSurface(icon_save_image);
  module_ttf = Gui::load_font("FreeSans.ttf", 10);
  if(!module_ttf) {
    cout << TTF_GetError() << endl;
    return -3;
  }
  log_ttf = Gui::load_font("FreeSans.ttf", 12);
  Gui::Redraw();
  return 0;
}

SDL_Surface *Gui::load_image(string imgname) {
  return IMG_Load((Gui::getData() + imgname).c_str());
}

TTF_Font *Gui::load_font(string fontname, int size) {
  SDL_RWops *file = SDL_RWFromFile((Gui::getFontPath() + fontname).c_str(), "rb");
  if(!file) {
     cout << "ERROR: " << SDL_GetError();
  }
  return TTF_OpenFontRW(file, 1, size);
}

void Gui::DrawModules() {
  SDL_Rect update, pos, car, trect;
  SDL_Color font_color = { 255, 255, 255, 255 };
  SDL_Surface *font = NULL;
  char id_buf[16];
  int state;
  Module *selected = NULL;
  vector <Module *>modules = gd.get_active_modules();
  update.x = 0;
  update.y = 0;
  update.w = MODULE_W;
  update.h = MODULE_H;
  car.x = CAR_REGION_X;
  car.y = CAR_REGION_Y;
  car.w = CAR_REGION_W;
  car.h = CAR_REGION_H;
  SDL_RenderCopy(renderer, base_texture, &car, &car);
  for(vector<Module *>::iterator it = modules.begin(); it != modules.end(); ++it) {
    Module *mod = *it;
    if(mod->getX() == 0 && mod->getY() == 0) { // Initialize location
      mod->setX(260 + (rand() % 100));
      mod->setY(80 + rand() % 100);
    }
    pos.x = mod->getX();
    pos.y = mod->getY();
    pos.w = MODULE_W;
    pos.h = MODULE_H;
    if(mod->getIdTexture() == NULL) {
      memset(id_buf, 0, 16);
      snprintf(id_buf, 15, "%02X", mod->getArbId()); 
      font = TTF_RenderText_Blended(module_ttf, id_buf, font_color);
      mod->setIdTexture(SDL_CreateTextureFromSurface(renderer, font));
      SDL_FreeSurface(font);
    }
    state = mod->getState();
    switch(state) {
      case STATE_IDLE:
        update.x = 0;
        SDL_RenderCopy(renderer, module_texture, &update, &pos);
        SDL_RenderCopy(renderer, mod->getIdTexture(), NULL, &pos);
        break;
      case STATE_ACTIVE:
        update.x = MODULE_W;
        SDL_RenderCopy(renderer, module_texture, &update, &pos);
        SDL_RenderCopy(renderer, mod->getIdTexture(), NULL, &pos);
        break;
      case STATE_MOUSEOVER:
        update.x = MODULE_W * 2;
        SDL_RenderCopy(renderer, module_texture, &update, &pos);
        SDL_RenderCopy(renderer, mod->getIdTexture(), NULL, &pos);
        break;
      case STATE_SELECTED:
        /* Draw selected last in case dragged over another module */
        selected = mod;
        break;
      default:
        break;
    }
  }
  if(selected) {
    update.x = MODULE_W * 3;
    pos.x = selected->getX();
    pos.y = selected->getY();
    SDL_RenderCopy(renderer, module_texture, &update, &pos);
    SDL_RenderCopy(renderer, selected->getIdTexture(), NULL, &pos);
  }
}

void Gui::DrawToolbar() {
  SDL_Rect toolbar, icon;
  toolbar.x = TOOLBAR_REGION_X;
  toolbar.y = TOOLBAR_REGION_Y;
  toolbar.w = TOOLBAR_REGION_W;
  toolbar.h = TOOLBAR_REGION_H;
  SDL_RenderCopy(renderer, base_texture, &toolbar, &toolbar);
  icon.x = saveButton->getState() * ICON_SAVE_W;
  icon.y = 0;
  icon.w = ICON_SAVE_W;
  icon.h = ICON_SAVE_H;
  SDL_RenderCopy(renderer, saveButton->getTexture(), &icon, saveButton->getLoc());
}

void Gui::Redraw() {
  SDL_RenderCopy(renderer, base_texture, NULL, NULL);
  Gui::DrawModules();
  Gui::DrawLog();
  Gui::DrawToolbar();
  SDL_RenderPresent(renderer);
}

void Gui::HandleMouseMotions(SDL_MouseMotionEvent motion) {
  int x = motion.x;
  int y = motion.y;
  bool change = false;
  bool change_toolbar = false;
  /* Check to see if hovering over a module */
  vector<Module *>modules = gd.get_active_modules();
  if(Gui::isOverCarRegion(x, y)) {
    for(vector<Module *>::iterator it = modules.begin(); it != modules.end(); ++it) {
      Module *mod = *it;
      if(x > mod->getX() && x < mod->getX() + MODULE_W && y > mod->getY() && y < mod->getY() + MODULE_H) {
        if(mod->getState() == STATE_SELECTED) {
          mod->setX(mod->getX() + motion.xrel);
          mod->setY(mod->getY() + motion.yrel);
          if(mod->getX() < CAR_REGION_X) mod->setX(CAR_REGION_X);
          if(mod->getY() < CAR_REGION_Y) mod->setY(CAR_REGION_Y);
          if(mod->getX() > CAR_REGION_X + CAR_REGION_W - MODULE_W) mod->setX(CAR_REGION_X + CAR_REGION_W - MODULE_W);
          if(mod->getY() > CAR_REGION_Y + CAR_REGION_H - MODULE_H) mod->setY(CAR_REGION_Y + CAR_REGION_H - MODULE_H);
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
  }
  if(saveButton->isOver(x, y)) {
    if(saveButton->getState() == ICON_STATE_IDLE) {
      saveButton->setState(ICON_STATE_HOVER);
      change_toolbar = true;
    }
  } else if(saveButton->getState() == ICON_STATE_HOVER) {
    saveButton->setState(ICON_STATE_IDLE);
    change_toolbar = true;
  }
  if(change) Gui::DrawModules();
  if(change_toolbar) Gui::DrawToolbar();
  if(change || change_toolbar) SDL_RenderPresent(renderer);
}

bool Gui::isOverCarRegion(int x, int y) {
  if(x > CAR_REGION_X && x < CAR_REGION_X + CAR_REGION_W &&
     y > CAR_REGION_Y && y < CAR_REGION_Y + CAR_REGION_H) return true;
  return false;
}

bool Gui::isOverToolbarRegion(int x, int y) {
  if(x > TOOLBAR_REGION_X && x < TOOLBAR_REGION_X + TOOLBAR_REGION_W &&
     y > TOOLBAR_REGION_Y && y < TOOLBAR_REGION_Y + TOOLBAR_REGION_H) return true;
  return false;
}

void Gui::HandleMouseClick(SDL_MouseButtonEvent button) {
  bool change = false;
  bool change_toolbar = false;
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
      } else if (Gui::isOverToolbarRegion(button.x, button.y)) {
        if(saveButton->isOver(button.x, button.y)) {
          saveButton->setState(ICON_STATE_SELECTED);
          change_toolbar = true;
          gd.SaveConfig();
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
      } else if(Gui::isOverToolbarRegion(button.x, button.y)) {
        if(saveButton->isOver(button.x, button.y)) {
          saveButton->setState(ICON_STATE_HOVER);
          change_toolbar = true;
        }
     }
    }
  }
  if(change) Gui::DrawModules();
  if(change_toolbar) Gui::DrawToolbar();
  if(change || change_toolbar) SDL_RenderPresent(renderer);
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

void Gui::DrawLog() {
  SDL_Rect logrect, trect, pos;
  int i;
  logrect.x = LOG_REGION_X;
  logrect.y = LOG_REGION_Y;
  logrect.w = LOG_REGION_W;
  logrect.h = LOG_REGION_H;
  SDL_RenderCopy(renderer, base_texture, &logrect, &logrect);
  for(i = 0; i < MAX_LOG_ENTRIES; i++) {
    if(logbuff[i] != NULL) {
      trect.x = 0;
      trect.y = 0;
      SDL_QueryTexture(logbuff[i], NULL, NULL, &trect.w, &trect.h);
      if(trect.w > LOG_REGION_W) trect.w = LOG_REGION_W;
      pos.x = LOG_REGION_X;
      pos.y = LOG_REGION_Y + LOG_REGION_H - (trect.h * i) - trect.h;
      pos.h = trect.h;
      pos.w = trect.w;
      SDL_RenderCopy(renderer, logbuff[i], &trect, &pos);
    }
  }
}

void Gui::Msg(string m) {
  int i;
  SDL_Color font_color = { 255, 255, 255, 255 };
  SDL_Surface *font;
  if(verbose) { cout << m << endl; }
  if(logbuff[MAX_LOG_ENTRIES-1] != NULL) SDL_DestroyTexture(logbuff[MAX_LOG_ENTRIES-1]);
  for(i = MAX_LOG_ENTRIES-1; i > 0; i--) {
    logbuff[i] = logbuff[i-1];
  }
  font = TTF_RenderText_Blended(log_ttf, m.c_str(), font_color);
  logbuff[0] = SDL_CreateTextureFromSurface(renderer, font);
  SDL_FreeSurface(font);
  Gui::DrawLog();
  SDL_RenderPresent(renderer);
}

