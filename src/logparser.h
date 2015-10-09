#ifndef LOGPARSER_H
#define LOGPARSER_H

/*
 * Parses candump log format
*/

#include <stdio.h>
#include <string>
#include <string.h>
// Needed for constant references
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/can.h>
#include <linux/can/raw.h>

#include "gamedata.h"

extern GameData gd;

using namespace std;

class LogParser
{
  public:
    LogParser();
    ~LogParser();
    string getLogFile() {return logfile; }
    void setLogFile(string l) {logfile = l; }
    bool Eof() { return log_eof; }
    string processNext();
  private:
    string logfile;
    bool log_eof = false;
    bool log_opened = false;
    FILE *logfp;
};

#endif
