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
 cout << "     -f                  Fullscreen" << endl;
 cout << "     -v                  Increase verbosity" << endl;
 cout << endl;
}

int main(int argc, char *argv[]) {
  int running = 1, opt, res;
  int verbose = 0;
  bool process_log = false;
  Gui gui;
  LogParser log;
  ConfigParser conf;

  cout << "UDSim " << VERSION << endl;

  while ((opt = getopt(argc, argv, "vfc:l:h?")) != -1) {
    switch(opt) {
      case 'v':
        verbose++;
        break;
      case 'c':
        if(!conf.parse(optarg)) {
          exit(10);
        }
        break;
      case 'l':
        log.setLogFile(optarg);
        process_log = true;
        gd.setMode(MODE_LEARN);
        break;
      case 'f':
        gui.setFullscreen(true);
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
  if(!gd.getCan()->Init()) {
    cout << "Failed to initialize CAN.  Aborting." << endl;
    return 20;
  }

  gui.setVerbose(verbose);
  res=gui.Init();
  if(res < 0) exit(3);
  gd.setGUI(&gui);

  if(!process_log) gd.setMode(MODE_SIM);

  gd.setVerbose(verbose);
  while(running) {
    running = gui.HandleEvents();
    gui.HandleAnimations();
    if(process_log) {
      if(log.Eof()) {
       process_log = false;
       gd.setMode(MODE_SIM);
       gui.Redraw();
      } else {
       gui.Msg(log.processNext());
      }
    } else {
      gd.processCan();
    }
  }

}
