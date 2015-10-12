#include <iostream>
#include <getopt.h>
#include "udsim.h"

using namespace std;

/* Globals */
GameData gd;

void Usage(string msg) {
 cout << msg << endl;
 cout << "Usage: udsim [options] <can-if>" << endl;
 cout << "     -c <config file>    Configuration file for simulator" << endl;
 cout << "     -l <logfile>        Parse candump log to generate learning DB" << endl;
 cout << "     -v                  Increase verbosity" << endl;
 cout << endl;
}

int main(int argc, char *argv[]) {
  int running = 1, opt, res;
  int verbose = 0;
  bool process_log = false;
  Gui gui;
  LogParser log;

  cout << "UDSim " << VERSION << endl;

  while ((opt = getopt(argc, argv, "vc:l:h?")) != -1) {
    switch(opt) {
      case 'v':
        verbose++;
        break;
      case 'c':
        break;
      case 'l':
        log.setLogFile(optarg);
        process_log = true;
        gd.setMode(MODE_LEARN);
        break;
      default:
        Usage("Help Menu");
        exit(1);
        break;
    }
  }

  if (optind >= argc) {
    Usage("You must specify at least one can device");
    exit(2);
  }

  gd.setCan(new Can(argv[optind]));

  gui.setVerbose(verbose);
  res=gui.Init();
  if(res < 0) exit(3);
  gd.setGUI(&gui);

  gd.setVerbose(verbose);
  while(running) {
    running = gui.HandleEvents();
    if(process_log) {
      if(log.Eof()) {
       process_log = false;
       gd.setMode(MODE_SIM);
       gui.Redraw();
      } else {
       gui.Msg(log.processNext());
      }
    }
  }

}
