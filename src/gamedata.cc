#include "gamedata.h"

GameData::GameData() {

}

GameData::~GameData() {

}

Module *GameData::get_module(int id) {
  for(vector<Module>::iterator it = modules.begin(); it != modules.end(); ++it) {
    if(it->getArbId() == id) return &*it;
  }  
  return NULL;
}

vector <Module *> GameData::get_active_modules() {
  vector<Module *> active_modules;
  for(vector<Module>::iterator it = modules.begin(); it != modules.end(); ++it) {
    if(it->isResponder() == false) active_modules.push_back(&*it);
  }  
  return active_modules;
}

/* Same as get_active_modules but designed for learning mode */
vector <Module *> GameData::get_possible_active_modules() {
  vector<Module *> possible_active_modules;
  for(vector<Module>::iterator it = possible_modules.begin(); it != possible_modules.end(); ++it) {
    if(it->isResponder() == false) possible_active_modules.push_back(&*it);
  }  
  return possible_active_modules;
}


Module *GameData::get_possible_module(int id) {
  for(vector<Module>::iterator it = possible_modules.begin(); it != possible_modules.end(); ++it) {
    if(it->getArbId() == id) return &*it;
  }  
  return NULL;
}

void GameData::setMode(int m) {
  switch(m) {
    case MODE_SIM:
      Msg("Switching to Simulator mode");
      if(_gui) _gui->setStatus("Simulation Mode");
      if(mode == MODE_LEARN && possible_modules.size() > 0) { // Previous mode was learning, update
        Msg("Normalizing learned data");
        GameData::processLearned();
      }
      mode=m;
      if(_gui) _gui->DrawModules();
      break;
    case MODE_LEARN:
      Msg("Switching to Learning mode");
      if(_gui) _gui->setStatus("Learning Mode");
      mode=m;
      break;
    case MODE_ATTACK:
      Msg("Switching to Attack mode");
      if(_gui) _gui->setStatus("Attack Mode");
      if(mode == MODE_LEARN && possible_modules.size() > 0) { // Previous mode was learning, update
        Msg("Normalizing learned data");
        GameData::processLearned();
      }
      mode=m;
      if(_gui) _gui->DrawModules();
      break;
    default:
      Msg("Unknown game mode");
      break;
  }
}

void GameData::processPkt(canfd_frame *cf) {
  switch(mode) {
    case MODE_SIM:
      GameData::HandleSim(cf);
      break;
    case MODE_LEARN:
      GameData::LearnPacket(cf);
      break;
    case MODE_ATTACK:
      GameData::AttackPacket(cf);
      break;
    default:
      cout << "ERROR: Processing packets while in an unknown mode" << mode << endl;
      break;
  }
}

void GameData::HandleSim(canfd_frame *cf) {
  Module *module = GameData::get_module(cf->can_id);
  CanFrame *resFrame = NULL;
  // Flow control is special
  if(cf->data[0] == 0x30 && !module) module = GameData::get_module(cf->can_id - 1);
  if(module) {
     if(module->getIgnore()) return;
     if(!module->isResponder()) {
       module->setState(STATE_ACTIVE);
       if(_gui) _gui->DrawModules(true);
       vector<CanFrame *>response = module->getResponse(cf, false);
       if(!response.empty()) {
         resFrame = response.at(0);
         if(resFrame->data[1] > 9) {
           if (resFrame->data[0] > 8) {// Multi-byte
             canif->sendPackets(response);
           } else { // Single packet
             if(response.size() > 1) {
               // More than one possible answer, select a random one
               vector<CanFrame *>randFrame;
               randFrame.push_back(response.at(rand() % response.size()));
               canif->sendPackets(randFrame);
             } else {
               canif->sendPackets(response);
             }
           }
         } else { // <= 9 then MODE note PID
           canif->sendPackets(response);
         }
       }
     }
  }
}

void GameData::LearnPacket(canfd_frame *cf) {
  Module *module = GameData::get_possible_module(cf->can_id);
  Module *possible_module = GameData::isPossibleISOTP(cf);
  int possible;
  // If module exists then we have seen this ID before
  if(module) {
    module->addPacket(cf);
    if(possible_module) {
      module->incMatchedISOTP();
    } else {
      // Still maybe an ISOTP answer, check for common syntax
      if(cf->data[0] == 0x10 && cf->len == 8) {
        module->incMatchedISOTP();
      } else if(cf->data[0] == 0x30 && cf->len == 3) {
        module->incMatchedISOTP();
      } else if(cf->data[0] >= 0x21 || cf->data[0] <= 0x30) {
        module->incMatchedISOTP();
      } else {
        module->incMissedISOTP();
      }
    }
    module->setState(STATE_ACTIVE);
    if(_gui) _gui->DrawModules(true);
  } else if(possible_module) { // Haven't seen this ID yet
    possible_module->addPacket(cf);
    possible_modules.push_back(*possible_module);
    if(_gui) _gui->DrawModules();
  }
}

void GameData::AttackPacket(canfd_frame *cf) {
  Module *module = GameData::get_module(cf->can_id);
  CanFrame *resFrame = NULL;
  // Flow control is special
  if(cf->data[0] == 0x30 && !module) module = GameData::get_module(cf->can_id - 1);
  if(module) {
     if(module->getIgnore()) return;
     if(!module->isResponder()) {
       module->setState(STATE_ACTIVE);
       if(_gui) _gui->DrawModules(true);
       vector<CanFrame *>response = module->getResponse(cf, true);
       if(!response.empty()) {
         resFrame = response.at(0);
         if(resFrame->data[1] > 9) {
           if (resFrame->data[0] > 8) {// Multi-byte
             canif->sendPackets(response);
           } else { // Single packet
             if(response.size() > 1) {
               // More than one possible answer, select a random one
               vector<CanFrame *>randFrame;
               randFrame.push_back(response.at(rand() % response.size()));
               canif->sendPackets(randFrame);
             } else {
               canif->sendPackets(response);
             }
           }
         } else { // <= 9 then MODE note PID
           canif->sendPackets(response);
         }
       }
     }
  }
}

Module *GameData::isPossibleISOTP(canfd_frame *cf) {
  int i;
  bool padding = false;
  char last_byte;
  Module *possible = NULL;
    if(cf->data[0] == cf->len - 1) { // Possible UDS request
      possible = new Module(cf->can_id);
    } else if(cf->data[0] < cf->len - 2) { // Check if remaining bytes are just padding
      padding = true;
      if(cf->data[0] == 0) padding = false;
      last_byte = cf->data[cf->data[0] + 1];
      for(i=cf->data[0] + 2; i < cf->len; i++) {
        if(cf->data[i] != last_byte) {
          padding = false;
        } else {
          last_byte = cf->data[i];
        }
      }
      if(padding == true) { // Possible UDS w/ padding
        possible = new Module(cf->can_id);
        possible->setPaddingByte(last_byte);
      }
    }
  return possible;
}

// Goes through the modules and removes ones that are less
// likely to be of use
void GameData::pruneModules() {
  vector<Module> goodModules;
  bool keep = false;

  for(vector<Module>::iterator it = modules.begin(); it != modules.end(); ++it) {
    keep = false;
    if(it->confidence() > CONFIDENCE_THRESHOLD) {
      if(it->getPositiveResponder() > -1 || it->getNegativeResponder() > -1)  keep = true;
      if(it->isResponder()) keep = true;
      if(!keep and it->getMatchedISOTP() > 0) keep = true;
    }

    if(keep) {
      goodModules.push_back(*it);
    } else {
      if(verbose) cout << "Removing module " << hex << it->getArbId() << endl;
    }
  }
  modules = goodModules;
}

void GameData::processLearned() {
  if(verbose) cout << "Identified " << possible_modules.size() << " possible modules" << endl;
  modules = possible_modules;
  if(verbose) cout << "Locating responders" << endl;
  Module *responder = NULL;
  for(vector<Module>::iterator it = modules.begin(); it != modules.end(); ++it) {
     if(it->isResponder() == false) {
       responder = GameData::get_module(it->getArbId() + 0x300);
       if(responder) { // GM style positive response
         it->setPositiveResponderID(responder->getArbId());
         responder->setResponder(true);
       }
       responder = GameData::get_module(it->getArbId() + 0x400);
       if(responder) { // GM style negative response
         it->setNegativeResponderID(responder->getArbId());
         responder->setResponder(true);
       }
       responder = GameData::get_module(it->getArbId() + 0x20);
       if(responder && it->foundResponse(responder)) { // Renault/Dacia response
         it->setPositiveResponderID(responder->getArbId());
         it->setNegativeResponderID(responder->getArbId());
         responder->setResponder(true);
       }
       responder = GameData::get_module(it->getArbId() + 0x08);
       if(responder && it->foundResponse(responder)) { // Standard response
         it->setPositiveResponderID(responder->getArbId());
         it->setNegativeResponderID(responder->getArbId());
         responder->setResponder(true);
       }
       responder = GameData::get_module(it->getArbId() + 0x01);
       if(responder && it->foundResponse(responder)) { // check for flow control
         vector<CanFrame *>pkts = responder->getPacketsByBytePos(0, 0x30);
         if(pkts.size() > 0) responder->setResponder(true);
       }
     }
  }
  GameData::pruneModules();
  // Cleanup - After pruning we can space out modules more
  possible_modules = modules; // Clear up possible to known modules
  for(vector<Module>::iterator it = modules.begin(); it != modules.end(); ++it) {
    Module *mod = &*it;
    if(_gui->isModuleOverlapping(mod)) _gui->setRandomModulePosition(mod);
  }
  _gui->Redraw();
  stringstream m;
  m << "Identified " << GameData::get_active_modules().size() << " Active modules";
  GameData::Msg(m.str());
}

string GameData::frame2string(canfd_frame *cf) {
  stringstream pkt;
  if(cf->len < 0 || cf->len > 8) { 
    return "ERROR: CAN packet with imporoper length";
  }
  pkt << hex << cf->can_id << CANID_DELIM;
  int i;
  for(i=0; i < cf->len; i++) {
    pkt << setfill('0') << setw(2) << hex << (int)cf->data[i];
  }
  return pkt.str();
}

void GameData::Msg(string mesg) {
  if(_gui == NULL) return;
  _gui->Msg(mesg);
}

bool GameData::SaveConfig() {
  ofstream configFile;
  configFile.open("config_data.cfg");
  // Globals
  // Modules
  configFile << endl;
  for(vector<Module>::iterator it = modules.begin(); it != modules.end(); ++it) {
    configFile << "[" << hex << it->getArbId() << "]" << endl;
    configFile << "pos = " << dec << it->getX() << "," << it->getY() << endl;
    configFile << "responder = " << it->isResponder() << endl;
    if(!it->isResponder()) {
      if(it->getPositiveResponder() != -1) configFile << "positiveID = " << hex << it->getPositiveResponder() << endl;
      if(it->getNegativeResponder() != -1) configFile << "negativeID = " << hex << it->getNegativeResponder() << endl;
    }
    if(it->getIgnore()) configFile << "ignore = " << it->getIgnore() << endl;
    if(it->getFuzzVin()) configFile << "fuzz_vin = " << it->getFuzzVin() << endl;
    if(it->getFuzzLevel() > 0) configFile << "fuzz_level = " << it->getFuzzLevel() << endl;
    configFile << "{Packets}" << endl;
    vector <CanFrame *>frames = it->getHistory();
    for(vector<CanFrame *>::iterator it2 = frames.begin(); it2 != frames.end(); ++it2) {
      CanFrame *frame = *it2;
      configFile << frame->estr() << endl;
    }
    configFile << endl;
  }
  configFile.close();
  Msg("Saved config_data.cfg");
  return true;
}

void GameData::launchPeach() {
  ofstream peachXML;
  peachXML.open("fuzz_can.xml");
  peachXML << "<?xml version=\"1.0\" encoding=\"utf-8\"?>" << endl;
  peachXML << "<Peach xmlns=\"http://peachfuzzer.com/2012/Peach\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" << endl;
  peachXML << "        xsi:schemaLocation=\"http://peachfuzzer.com/2012/Peach /peach/peach.xsd\">" << endl;
  // TODO Use learned information to populate DataModels
  peachXML << "        <DataModel name=\"CANPacket\">" << endl;
  peachXML << "                <Blob name=\"sample1\" valueType=\"hex\" value=\"00 00 03 33 03 01 02 03 00 00 00 00 00 00 00 00\"/>" << endl;
  peachXML << "        </DataModel>" << endl;
  peachXML << endl;
  peachXML << "        <StateModel name=\"TheState\" initialState=\"Initial\">" << endl;
  peachXML << "                <State name=\"Initial\">" << endl;
  peachXML << "                        <Action type=\"output\">" << endl;
  peachXML << "                                <DataModel ref=\"CANPacket\"/>" << endl;
  peachXML << "                        </Action>" << endl;
  peachXML << "                </State>" << endl;
  peachXML << "        </StateModel>" << endl;
  peachXML << "        <Agent name=\"TheAgent\">" << endl;
  peachXML << "        </Agent>" << endl;
  peachXML << "        <Test name=\"Default\">" << endl;
  peachXML << "                <Agent ref=\"TheAgent\"/>" << endl;
  peachXML << "                <StateModel ref=\"TheState\"/>" << endl;
  peachXML << "                <Publisher class=\"CAN\">" << endl;
  peachXML << "                        <Param name=\"Interface\" value=\"" << canif->getIfname() << "\"/>" << endl;
  peachXML << "                </Publisher>" << endl;
  peachXML << "                <Logger class=\"File\">" << endl;
  peachXML << "                        <Param name=\"Path\" value=\"logs\"/>" << endl;
  peachXML << "                </Logger>" << endl;
  peachXML << "        </Test>" << endl;
  peachXML << "</Peach>" << endl;
  peachXML.close();
  Msg("Created fuzz_can.xml");
}

void GameData::nextMode() {
  switch(mode) {
    case MODE_SIM:
      GameData::setMode(MODE_LEARN);
      break;
    case MODE_LEARN:
      GameData::setMode(MODE_ATTACK);
      break;
    case MODE_ATTACK:
      GameData::setMode(MODE_SIM);
      break;
  }
}

int GameData::string2hex(string s) {
  stringstream ss;
  int h;
  ss << hex << s;
  ss >> h;
  return h;
}

int GameData::string2int(string s) {
  stringstream ss;
  int i;
  ss << dec << s;
  ss >> i;
  return i;
}

void GameData::processCan() {
  struct canfd_frame cf;
  int i;
  if(!canif) return;
  int ticks = SDL_GetTicks();
  if(_lastTicks + CAN_DELAY > ticks) {
    vector <CanFrame *>frames = canif->getPackets();
    for(vector <CanFrame *>::iterator it=frames.begin(); it != frames.end(); ++it) {
      CanFrame *pkt = *it;
      if(verbose) Msg(pkt->str());
      cf.can_id = pkt->can_id;
      cf.len = pkt->len;
      for(i=0; i < pkt->len; i++) {
        cf.data[i] = pkt->data[i];
      }
      GameData::processPkt(&cf);
    }
    _lastTicks = ticks;
  }
}
