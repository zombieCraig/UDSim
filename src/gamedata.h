#ifndef UDS_GAMEDATA_H
#define UDS_GAMEDATA_H

#include <cstddef>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <list>
#include "module.h"
#include "gui.h"
#include "can.h"

using namespace std;

#define MODE_SIM    0
#define MODE_LEARN  1
#define MODE_ATTACK 2

#define CONFIDENCE_THRESHOLD 0.6 // 60%

#define CAN_DELAY 1000 * 10

class Gui;
class Module;

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
    vector<Module *> get_possible_active_modules();
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
    void nextMode();
    void launchPeach();
    void processCan();
    int string2hex(string);
    int string2int(string);
  private:
    void HandleSim(canfd_frame *);
    void LearnPacket(canfd_frame *);
    void AttackPacket(canfd_frame *);
    void pruneModules();
    Module *isPossibleISOTP(canfd_frame *);
    void processLearned();
    int mode = MODE_SIM;
    int verbose = 0;
    int _lastTicks = 0;
    Can *canif = NULL;
    Gui *_gui = NULL;
};

#endif
