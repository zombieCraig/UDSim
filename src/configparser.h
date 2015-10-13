#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <regex>

#include "gamedata.h"

using namespace std;

extern GameData gd;

#define CONF_STATE_GLOBALS 0
#define CONF_STATE_MODULE  1
#define CONF_STATE_PACKETS 2

class ConfigParser {
  public:
    ConfigParser();
    ~ConfigParser();
    bool parse(string);
    string getFilename() { return filename; }
  private:
    string filename;
    void parseGlobals(string);
    void parseModule(Module *, string, int);
    int _state = CONF_STATE_GLOBALS;
};

#endif
