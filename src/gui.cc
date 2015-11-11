#include "gui.h"

Gui::Gui() {
  int i;
  for(i = 0; i < MAX_LOG_ENTRIES; i++) { logbuff[i] = NULL; }
  data_path = "../data/";
  font_path = "../data/fonts/";
  srand(time(NULL));
}

Gui::~Gui() {
  SDL_DestroyTexture(check_texture);
  SDL_DestroyTexture(info_card_texture);
  SDL_DestroyTexture(module_unk_texture);
  SDL_DestroyTexture(module_texture);
  SDL_DestroyTexture(base_texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  IMG_Quit();
  TTF_Quit();
  SDL_Quit();
}

int Gui::Init() {
  SDL_Surface *base_image, *module_image, *module_unk_image,  *icon_save_image, *icon_mode_image, *info_card_image;
  SDL_Surface *check_image, *slider_image;
  int flags;
  if(SDL_Init ( SDL_INIT_VIDEO ) < 0 ) {
        printf("SDL Could not initializes\n");
        return(-1);
  }
  if(TTF_Init() < 0) {
        printf("TTF Could not be initialized\n");
        return -2;
  }
  flags = SDL_WINDOW_SHOWN;
  if(_fullscreen) {
    //flags |= SDL_WINDOW_FULLSCREEN;
    flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
  }
  window = SDL_CreateWindow("UDSim", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Gui::getWidth(), Gui::getHeight(), flags); // | SDL_WINDOW_RESIZABLE);
  if(window == NULL) {
        printf("Window could not be shown\n");
  }
  renderer = SDL_CreateRenderer(window, -1, 0);
  if(_fullscreen) SDL_RenderSetLogicalSize(renderer, Gui::getWidth(), Gui::getHeight());
  base_image = Gui::load_image("udsbg.png");
  module_image = Gui::load_image("module.png");
  module_unk_image = Gui::load_image("module-unk.png");
  icon_save_image = Gui::load_image("save_icon.png");
  icon_mode_image = Gui::load_image("mode_icon.png");
  info_card_image = Gui::load_image("infocard.png");
  check_image = Gui::load_image("check.png");
  slider_image = Gui::load_image("slider.png");
  base_texture = SDL_CreateTextureFromSurface(renderer, base_image);
  module_texture = SDL_CreateTextureFromSurface(renderer, module_image);
  module_unk_texture = SDL_CreateTextureFromSurface(renderer, module_unk_image);
  info_card_texture = SDL_CreateTextureFromSurface(renderer, info_card_image);
  check_texture = SDL_CreateTextureFromSurface(renderer, check_image);
  slider_texture = SDL_CreateTextureFromSurface(renderer, slider_image);
  SDL_FreeSurface(slider_image);
  SDL_FreeSurface(check_image);
  SDL_FreeSurface(info_card_image);
  SDL_FreeSurface(module_unk_image);
  SDL_FreeSurface(module_image);
  SDL_FreeSurface(base_image);
  // Toolbar
  saveButton = new IconButton();
  saveButton->setLoc(ICON_SAVE_X, ICON_SAVE_Y, ICON_SAVE_H, ICON_SAVE_W);
  saveButton->setTexture(SDL_CreateTextureFromSurface(renderer, icon_save_image));
  SDL_FreeSurface(icon_save_image);
  modeButton = new IconButton();
  modeButton->setLoc(ICON_MODE_X, ICON_MODE_Y, ICON_MODE_H, ICON_MODE_W);
  modeButton->setTexture(SDL_CreateTextureFromSurface(renderer, icon_mode_image));
  SDL_FreeSurface(icon_mode_image);
  module_ttf = Gui::load_font("FreeSans.ttf", 10);
  if(!module_ttf) {
    cout << TTF_GetError() << endl;
    return -3;
  }
  log_ttf = Gui::load_font("FreeSans.ttf", 12);
  title_ttf = Gui::load_font("Coluna_Rounded.otf", 22);
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

void Gui::toggleFullscreen() {
  int flags =  (SDL_GetWindowFlags(window) ^ SDL_WINDOW_FULLSCREEN_DESKTOP);
  if (SDL_SetWindowFullscreen(window, flags) < 0) return;
  SDL_SetWindowSize(window, Gui::getWidth(), Gui::getHeight());
  if(flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
    SDL_RenderSetLogicalSize(renderer, Gui::getWidth(), Gui::getHeight());
    _fullscreen = true;
  } else {
    _fullscreen = false;
  }
  Gui::Redraw();
}

bool Gui::isModuleOverlapping(Module *mod) {
    vector <Module *>modules;
    int x = mod->getX();
    int y = mod->getY();
    bool overlapping = false;

    if(gd.getMode() == MODE_LEARN) {
      modules = gd.get_possible_active_modules();
    } else {
      modules = gd.get_active_modules();
    }
    for(vector<Module *>::iterator it = modules.begin(); it != modules.end(); ++it) {
      Module *xmod = *it;
      if(mod->getArbId() != xmod->getArbId()) {
        // We need to check all four courners of the module to test for overlap
        if((x >= xmod->getX() && x <= xmod->getX() + MODULE_W &&
           y >= xmod->getY() && y <= xmod->getY() + MODULE_H) ||
           (x + MODULE_W >= xmod->getX() && x + MODULE_W <= xmod->getX() + MODULE_W &&
            y >= xmod->getY() && y <= xmod->getY() + MODULE_H) ||
           (x >= xmod->getX() && x <= xmod->getX() + MODULE_W &&
            y + MODULE_H >= xmod->getY() && y + MODULE_H <= xmod->getY() + MODULE_H) ||
           (x + MODULE_W >= xmod->getX() && x + MODULE_W <= xmod->getX() + MODULE_W &&
            y + MODULE_H >= xmod->getY() && y + MODULE_H <= xmod->getY() + MODULE_H)) {
             overlapping = true;
        }
      }
    }
   return overlapping;
}

void Gui::setRandomModulePosition(Module *mod) {
  int i;
  bool good_location = false;

  mod->setX(260 + (rand() % 100));
  mod->setY(80 + rand() % 200);
  for(i = 0; !good_location && i < MAX_RANDOM_ATTEMPTS; i++) {
    if(Gui::isModuleOverlapping(mod)) {
       mod->setX(260 + (rand() % 100));
       mod->setY(80 + rand() % 200);
    } else {
      good_location = true;
    }
  }
}

void Gui::DrawModules(bool force_refresh) {
  SDL_Rect update, pos, car, trect;
  SDL_Color font_color = { 255, 255, 255, 255 };
  SDL_Surface *font = NULL;
  char id_buf[16];
  int state;
  Module *selected = NULL;
  SDL_Texture *module_color = NULL;
  vector <Module *>modules;

  if(gd.getMode() == MODE_LEARN) {
    modules = gd.get_possible_active_modules();
  } else {
    modules = gd.get_active_modules();
  }
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
      setRandomModulePosition(mod);
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
      if(mod->getPositiveResponder() > 0 || mod->getNegativeResponder() > 0) {
        module_color = module_texture;
      } else {
        module_color = module_unk_texture;
      }
      state = mod->getState();
      switch(state) {
        case STATE_IDLE:
          update.x = 0;
          SDL_RenderCopy(renderer, module_color, &update, &pos);
          SDL_QueryTexture(mod->getIdTexture(), NULL, NULL, &pos.w, &pos.h);
          pos.x += (MODULE_W / 2) - (pos.w / 2) + 2;
          pos.y += (MODULE_H / 2) - (pos.h / 2);
          SDL_RenderCopy(renderer, mod->getIdTexture(), NULL, &pos);
          break;
        case STATE_ACTIVE:
          update.x = MODULE_W;
          SDL_RenderCopy(renderer, module_color, &update, &pos);
          SDL_QueryTexture(mod->getIdTexture(), NULL, NULL, &pos.w, &pos.h);
          pos.x += (MODULE_W / 2) - (pos.w / 2) + 2;
          pos.y += (MODULE_H / 2) - (pos.h / 2);
          SDL_RenderCopy(renderer, mod->getIdTexture(), NULL, &pos);
          break;
        case STATE_MOUSEOVER:
          update.x = MODULE_W * 2;
          SDL_RenderCopy(renderer, module_color, &update, &pos);
          SDL_QueryTexture(mod->getIdTexture(), NULL, NULL, &pos.w, &pos.h);
          pos.x += (MODULE_W / 2) - (pos.w / 2) + 2;
          pos.y += (MODULE_H / 2) - (pos.h / 2);
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
    pos.w = MODULE_W;
    pos.h = MODULE_H;
    if(selected->getPositiveResponder() >0 && selected->getNegativeResponder() > 0) {
      module_color = module_texture;
    } else {
      module_color = module_unk_texture;
    }
    SDL_RenderCopy(renderer, module_color, &update, &pos);
    SDL_QueryTexture(selected->getIdTexture(), NULL, NULL, &pos.w, &pos.h);
    pos.x += (MODULE_W / 2) - (pos.w / 2) + 2;
    pos.y += (MODULE_H / 2) - (pos.h / 2);
    SDL_RenderCopy(renderer, selected->getIdTexture(), NULL, &pos);
  }
  if(force_refresh) SDL_RenderPresent(renderer);
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
  icon.x = modeButton->getState() * ICON_SAVE_W;
  SDL_RenderCopy(renderer, modeButton->getTexture(), &icon, modeButton->getLoc());
}

void Gui::Redraw() {
  SDL_RenderCopy(renderer, base_texture, NULL, NULL);
  Gui::DrawModules();
  Gui::DrawLog();
  Gui::DrawToolbar();
  Gui::DrawStatus();
  Gui::DrawInfoCard();
  SDL_RenderPresent(renderer);
}

void Gui::HandleMouseMotions(SDL_MouseMotionEvent motion) {
  int x = motion.x;
  int y = motion.y;
  bool change = false;
  bool change_toolbar = false;
  /* Check to see if hovering over a module */
  vector<Module *>modules;
  if(gd.getMode() == MODE_LEARN) {
    modules = gd.get_possible_active_modules();
  } else {
    modules = gd.get_active_modules();
  }
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
  if(modeButton->isOver(x, y)) {
    if(modeButton->getState() == ICON_STATE_IDLE) {
      modeButton->setState(ICON_STATE_HOVER);
      change_toolbar = true;
    }
  } else if(modeButton->getState() == ICON_STATE_HOVER) {
    modeButton->setState(ICON_STATE_IDLE);
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

// Card must be full extended for this to return true
bool Gui::isOverCardRegion(int x, int y) {
  bool over_card = false;
  if(module_selected != -1) {
    if(Gui::getCardState() == CARD_NOANIM) {
      if(x > CARD_REGION_X && x < CARD_REGION_X + CARD_REGION_W &&
         y > CARD_REGION_Y && y < CARD_REGION_Y + CARD_REGION_H) over_card = true;
    }
  }
  return over_card;
}

void Gui::HandleMouseClick(SDL_MouseButtonEvent button) {
  bool change = false;
  bool change_toolbar = false;
  bool change_card = false;
  unsigned int step = 0;
  if(button.button == SDL_BUTTON_LEFT) {
    if(button.state == SDL_PRESSED) {
      if(Gui::isOverCarRegion(button.x, button.y)) {
        vector<Module *>modules;
        if(gd.getMode() == MODE_LEARN) {
          modules = gd.get_possible_active_modules();
        } else {
          modules = gd.get_active_modules();
        }
        for(vector<Module *>::iterator it = modules.begin(); it != modules.end(); ++it) {
          Module *mod = *it;
          if(button.x > mod->getX() && button.x < mod->getX() + MODULE_W &&
             button.y > mod->getY() && button.y < mod->getY() + MODULE_H) {
               mod->setState(STATE_SELECTED);
               if(module_selected != mod->getArbId()) {
                 module_selected = mod->getArbId();
                 change_card = true;
                 Gui::AdvanceCard();
               }
               change = true;
          }
        }
        if(!change) { // No module was clicked.  Unselect info card
          if(module_selected != -1) {
            Gui::RetractCard();
            change_card = true;
          }
        }
      } else if (Gui::isOverToolbarRegion(button.x, button.y)) {
        if(saveButton->isOver(button.x, button.y)) {
          saveButton->setState(ICON_STATE_SELECTED);
          change_toolbar = true;
          gd.SaveConfig();
        }
        if(modeButton->isOver(button.x, button.y)) {
          modeButton->setState(ICON_STATE_SELECTED);
          change_toolbar = true;
          gd.nextMode();
        }
      } else if (Gui::isOverCardRegion(button.x, button.y)) {
        // Card Options
        if(button.x > CARD_FAKE_RESP_X && button.x < CARD_FAKE_RESP_X + CARD_FAKE_RESP_W &&
           button.y > CARD_FAKE_RESP_Y && button.y < CARD_FAKE_RESP_Y + CARD_FAKE_RESP_H) {
             Module *mod = gd.get_module(module_selected);
             mod->toggleFakeResponses();
             change_card = true;
        }
        if(button.x > CARD_IGNORE_X && button.x < CARD_IGNORE_X + CARD_IGNORE_W &&
           button.y > CARD_IGNORE_Y && button.y < CARD_IGNORE_Y + CARD_IGNORE_H) {
             Module *mod = gd.get_module(module_selected);
             mod->toggleIgnore();
             change_card = true;
        }
        if(button.x > CARD_FUZZ_VIN_X && button.x < CARD_FUZZ_VIN_X + CARD_FUZZ_VIN_W &&
           button.y > CARD_FUZZ_VIN_Y && button.y < CARD_FUZZ_VIN_Y + CARD_FUZZ_VIN_H) {
             Module *mod = gd.get_module(module_selected);
             mod->toggleFuzzVin();
             change_card = true;
        }
        if(button.x > CARD_FUZZ_LEVEL_X && button.x < CARD_FUZZ_LEVEL_X + CARD_FUZZ_LEVEL_W &&
           button.y > CARD_FUZZ_LEVEL_Y && button.y < CARD_FUZZ_LEVEL_Y + CARD_FUZZ_LEVEL_H) {
             Module *mod = gd.get_module(module_selected);
             step = CARD_FUZZ_LEVEL_W / CARD_FUZZ_LEVEL_STEPS;
             if(mod->getFuzzLevel() != (button.x - CARD_FUZZ_LEVEL_X) / step) {
               mod->setFuzzLevel( (button.x - CARD_FUZZ_LEVEL_X) / step);
               change_card = true;
             }
        }
      }
    } else if (button.state == SDL_RELEASED) {
      if(Gui::isOverCarRegion(button.x, button.y)) {
        vector<Module *>modules;
        if(gd.getMode() == MODE_LEARN) {
          modules = gd.get_possible_active_modules();
        } else {
          modules = gd.get_active_modules();
        }
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
        if(modeButton->isOver(button.x, button.y)) {
          modeButton->setState(ICON_STATE_HOVER);
          change_toolbar = true;
        }
     }
    }
  }
  if(change) Gui::DrawModules();
  if(change_toolbar) Gui::DrawToolbar();
  if(change_card) Gui::DrawInfoCard();
  if(change || change_toolbar || change_card) SDL_RenderPresent(renderer);
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
            case SDL_KEYDOWN:
                switch(event.key.keysym.sym) {
                  case SDLK_ESCAPE:
                    running = 0;
                    break;
                  case SDLK_f:
                    Gui::toggleFullscreen();
                    break;
                }
                break;
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


void Gui::setStatus(string status) {
  SDL_Color font_color = { 255, 255, 255, 255 };
  SDL_Surface *font;
  font = TTF_RenderText_Blended(log_ttf, status.c_str(), font_color);
  _status = SDL_CreateTextureFromSurface(renderer, font);
  SDL_FreeSurface(font);
  Gui::DrawStatus();
  SDL_RenderPresent(renderer);
}

void Gui::DrawStatus() {
  SDL_Rect srect, mrect;
  srect.x = STATUS_REGION_X;
  srect.y = STATUS_REGION_Y;
  srect.w = STATUS_REGION_W;
  srect.h = STATUS_REGION_H;
  SDL_RenderCopy(renderer, base_texture, &srect, &srect);
  if(_status == NULL) return;
  mrect.x = 0;
  mrect.y = 0;
  if(_status == NULL) return;
  SDL_QueryTexture(_status, NULL, NULL, &mrect.w, &mrect.h);
  if(mrect.w > srect.w) mrect.w = srect.w;
  if(mrect.h > srect.h) mrect.h = srect.h;
  srect.w = mrect.w;
  srect.h = mrect.h;
  SDL_RenderCopy(renderer, _status, &mrect, &srect);
}

void Gui::HandleAnimations() {
  _ticks = SDL_GetTicks();
  vector <Module *>mods;
  bool change = false;
  if(Gui::getCardState() == CARD_ADVANCE || Gui::getCardState() == CARD_RETRACT) {
    if(_ticks > _lastTicks + _CardAnimationTicks) {
      if(Gui::getCardState() == CARD_ADVANCE) {
        _card_current_x+= 10;
        if(_card_current_x > CARD_REGION_X) {
          _card_current_x = CARD_REGION_X;
          Gui::setCardState(CARD_NOANIM);
        }
      } else { // Retract
        _card_current_x-=10;
        if(_card_current_x < -CARD_REGION_W) {
          _card_current_x = -CARD_REGION_W;
          Gui::setCardState(CARD_NOANIM);
          module_selected = -1;
        }
      }
      _lastTicks = _ticks;
      Gui::DrawInfoCard();
      change = true;
    }
  }
  if(gd.getMode() == MODE_LEARN) {
    mods = gd.get_possible_active_modules();
  } else {
    mods = gd.get_active_modules();
  }
  for(vector<Module *>::iterator it = mods.begin(); it != mods.end(); ++it) {
    Module *mod = *it;
    if(mod->getState() == STATE_ACTIVE) {
      if(_ticks > mod->getActiveTicks() + ACTIVE_TICK_DELAY) {
        mod->setState(STATE_IDLE);
        Gui::DrawModules();
        change = true;
      }
    }
  }
  if (change) SDL_RenderPresent(renderer);
}

void Gui::AdvanceCard() {
  _card_current_x = -CARD_REGION_W;
  Gui::setCardState(CARD_ADVANCE);
}

void Gui::RetractCard() {
  _card_current_x = CARD_REGION_X;
  Gui::setCardState(CARD_RETRACT);
}

/* NOTE: This could be sped up by caching the cards during animations */
void Gui::DrawInfoCard() {
  SDL_Color font_color = { 255, 255, 255, 255 };
  SDL_Surface *title, *detail;
  SDL_Texture *title_tex, *detail_tex;
  SDL_Rect srect, titler, detailr, checkr;
  stringstream ss;
  char id_buf[16];
  srect.x = CARD_REGION_X;
  srect.y = CARD_REGION_Y;
  srect.h = CARD_REGION_H;
  srect.w = CARD_REGION_W;
  if(Gui::getCardState() == CARD_RETRACT) SDL_RenderCopy(renderer, base_texture, &srect, &srect);
  if(module_selected != -1) {
    Module *mod = gd.get_module(module_selected);
    if(mod) {
      srect.x = _card_current_x;
      SDL_RenderCopy(renderer, info_card_texture, NULL, &srect);
      memset(id_buf, 0, 16);
      snprintf(id_buf, 15, "%02X", mod->getArbId());
      title = TTF_RenderText_Blended(title_ttf, id_buf, font_color);
      titler.x = srect.x + 3;
      titler.y = srect.y + 6;
      titler.w = title->w;
      titler.h = title->h;
      titler.x += ((srect.w - 20) / 2) - (titler.w / 2);
      title_tex = SDL_CreateTextureFromSurface(renderer, title);
      SDL_RenderCopy(renderer, title_tex, NULL, &titler);
      SDL_FreeSurface(title);
      SDL_DestroyTexture(title_tex);
      if(mod->getPositiveResponder() == -1 && mod->getNegativeResponder() == -1) {
        detail = TTF_RenderText_Blended(log_ttf, "No responses (Missing module?)", font_color);
      } else {
        if(mod->getPositiveResponder() > -1) ss << " Positive ID: " << hex << mod->getPositiveResponder();
        if(mod->getNegativeResponder() > -1) ss << " Negative ID: " << hex << mod->getNegativeResponder();
        detail = TTF_RenderText_Blended(log_ttf, ss.str().c_str(), font_color);
      }
      detail_tex = SDL_CreateTextureFromSurface(renderer, detail);
      detailr.x = srect.x + 3;
      detailr.y = titler.y + 26;
      detailr.w = detail->w;
      detailr.h = detail->h;
      SDL_RenderCopy(renderer, detail_tex, NULL, &detailr);
      SDL_FreeSurface(detail);
      SDL_DestroyTexture(detail_tex);
      // Options
      if(mod->getFakeResponses()) {
        checkr.x = srect.x + CARD_FAKE_RESP_X - 6;
        checkr.y = CARD_FAKE_RESP_Y - 3;
        checkr.h = CARD_FAKE_RESP_H;
        checkr.w = CARD_FAKE_RESP_W;
        SDL_RenderCopy(renderer, check_texture, NULL, &checkr);
      }
      if(mod->getIgnore()) {
        checkr.x = srect.x + CARD_IGNORE_X - 3;
        checkr.y = CARD_IGNORE_Y - 3;
        checkr.h = CARD_IGNORE_H;
        checkr.w = CARD_IGNORE_W;
        SDL_RenderCopy(renderer, check_texture, NULL, &checkr);
      }
      if(mod->getFuzzVin()) {
        checkr.x = srect.x + CARD_FUZZ_VIN_X - 6;
        checkr.y = CARD_FUZZ_VIN_Y - 3;
        checkr.h = CARD_FUZZ_VIN_H;
        checkr.w = CARD_FUZZ_VIN_W;
        SDL_RenderCopy(renderer, check_texture, NULL, &checkr);
      }
      // Draw fuzz level
      checkr.x = srect.x + CARD_FUZZ_LEVEL_X - 7 + (mod->getFuzzLevel() * (CARD_FUZZ_LEVEL_W / CARD_FUZZ_LEVEL_STEPS));
      checkr.y = CARD_FUZZ_LEVEL_Y - 3;
      checkr.h = FUZZ_SLIDER_H;
      checkr.w = FUZZ_SLIDER_W;
      SDL_RenderCopy(renderer, slider_texture, NULL, &checkr);
    }
  }
}

