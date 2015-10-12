#ifndef UDS_GAMEDATA_H
#define UDS_GAMEDATA_H

#include <cstddef>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include "module.h"
#include "gui.h"
#include "can.h"

using namespace std;

#define MODE_SIM   0
#define MODE_LEARN 1

#define CONFIDENCE_THRESHOLD 0.6 // 60%

class Gui;

class GameData
{
  public:
    GameData();
    ~GameData();
    Module *get_module(int);
    Module *get_possible_module(int);
    vector<Module> modules;
    vector<Module> possible_modules;
    vector<Module *> get_active_modules();
    void setMode(int);
    int getMode() { return mode; }
    int setVerbose(int v) { verbose = v; }
    int getVerbose() { return verbose; }
    void setCan(Can *c) { canif = c; }
    Can *getCan() { return canif; }
    void processPkt(canfd_frame *);
    string frame2string(canfd_frame *);
    void setGUI(Gui *g) { _gui = g; }
    void Msg(string);
    bool SaveConfig();
    int string2hex(string);
    int string2dex(string);
  private:
    void HandleSim(canfd_frame *);
    void LearnPacket(canfd_frame *);
    Module *isPossibleISOTP(canfd_frame *);
    void processLearned();
    int mode = MODE_SIM;
    int verbose = 0;
    Can *canif;
    Gui *_gui;
};

#endif
