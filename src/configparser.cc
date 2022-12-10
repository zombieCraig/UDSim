#include "configparser.h"

ConfigParser::ConfigParser() {

}

ConfigParser::~ConfigParser() {

}

/* Ideally I wanted to use regex here but since g++ < 4.9 doesn't support
   regex I am resorting to hackery.  I don't like the boost library
   and frankly I'm just annoyed that GNU doesn't do regex (really?)
*/
bool ConfigParser::parse(string config) {
  ifstream conf;
  string line;
  stringstream ss;
  Module *mod = NULL;
  int id;
  int pos;
  filename = config;
  conf.open(filename);
  while(getline(conf, line)) {
    // Remove comment character ';'
    pos = line.find(';');
    if(pos != string::npos) {
      line.erase(pos, string::npos);
    }
    if(!line.empty()) {
      if(line.at(0) == '[') { // Module def
        line.erase(0,1);
        line.erase(line.end()-1);
        ss << hex << line;
        ss >> id;
        if(id > 0) {
          if(_state == CONF_STATE_MODULE || _state == CONF_STATE_PACKETS) {
            // Finished a module, add it
            gd.modules.push_back(*mod);
          }
          _state = CONF_STATE_MODULE;
          mod = new Module(id);
          ss.clear();
        }
      }
      if(line == "{Packets}") _state = CONF_STATE_PACKETS;
      pos = line.find('#');
      if(pos != string::npos) {
        switch(_state) {
          case CONF_STATE_PACKETS:
            mod->addPacket(line);
            break;
        }
      }
      pos = line.find('=');
      if(pos != string::npos) {
        switch(_state) {
          case CONF_STATE_GLOBALS:
            ConfigParser::parseGlobals(line);
            break;
          case CONF_STATE_MODULE:
            ConfigParser::parseModule(mod, line, pos);
            break;
        }
      }
    }
  }
  if(_state == CONF_STATE_MODULE || _state == CONF_STATE_PACKETS) {
    // Add final module
    gd.modules.push_back(*mod);
  }
  conf.close();
  return true;
}

void ConfigParser::parseGlobals(string line) {

}

void ConfigParser::parseModule(Module *mod, string line, int pos) {
  stringstream ss;
  string field, value;
  int x, y, id, fuzz_level;
  bool responder, ignore, fuzz_vin;
  char d1; // dummy
  field = line;
  value = line;
  field.erase(pos - 1, string::npos);
  value.erase(0, pos + 1);
  ss << value;
  if(field == "pos") {
    ss >> x >> d1 >> y;
    mod->setX(x);
    mod->setY(y);
  } else if(field == "responder") {
    ss >> responder;
    mod->setResponder(responder);
  } else if(field == "positiveID") {
    id = gd.string2hex(value);
    mod->setPositiveResponderID(id);
  } else if(field == "negativeID") {
    id = gd.string2hex(value);
    mod->setNegativeResponderID(id);
  } else if(field == "ignore") {
    ss >> ignore;
    mod->setIgnore(ignore);
  } else if(field == "fuzz_vin") {
    ss >> fuzz_vin;
    mod->setFuzzVin(fuzz_vin);
  } else if(field == "fuzz_level") {
    ss >> fuzz_level;
    mod->setFuzzLevel(fuzz_level);
  }else {
    cout << "config error: Unknown field " << field << endl;
  }
  ss.clear();
}
