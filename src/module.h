#ifndef UDS_MODULE_H
#define UDS_MODULE_H

#include <cstddef>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "gamedata.h"
#include "canframe.h"

using namespace std;

class GameData;

extern GameData gd;

#define STATE_IDLE      0
#define STATE_ACTIVE    1
#define STATE_MOUSEOVER 2
#define STATE_SELECTED  3

#define MODULE_TYPE_UNKNOWN 0
#define MODULE_TYPE_GM      1

#define MODULE_H 30
#define MODULE_W 35

#define ACTIVE_TICK_DELAY 100
#define DEFAULT_VIN "PWN3D OP3N G4R4G3"

class Module
{
 public:
  Module(int id);
  ~Module();
  void setArbId(int i) {arbId = i;}
  int getArbId() { return arbId; }
  void incMatchedISOTP() {matched_isotp++;}
  void incMissedISOTP() {missed_isotp++;}
  int getMatchedISOTP() { return matched_isotp; }
  int getMissedISOTP() { return missed_isotp; }
  void setPaddingByte(char b) {padding = true; padding_byte = b; }
  float confidence();
  void setPositiveResponderID(int i) { positive_responder_id = i; }
  int getPositiveResponder() { return positive_responder_id; }
  void setNegativeResponderID(int i) { negative_responder_id = i; }
  int getNegativeResponder() { return negative_responder_id; }
  void setResponder(bool v) { responder = v; }
  bool isResponder() { return responder; }
  void addPacket(struct canfd_frame *);
  void addPacket(string);
  vector <CanFrame *>getHistory() { return can_history; }
  vector <CanFrame *>getPacketsByBytePos(unsigned int, unsigned char);
  int getState();
  void setState(int s);
  int getX() { return _x; }
  int getY() { return _y; }
  void setX(int x) { _x = x; }
  void setY(int y) { _y = y; }
  SDL_Texture *getIdTexture() { return id_texture; }
  void setIdTexture(SDL_Texture *t) { id_texture = t; }
  vector <CanFrame *>getResponse(struct canfd_frame *);
  void setType(int t) { _type = t; }
  int getType() { return _type; }
  void toggleFakeResponses() { _fake_responses ? _fake_responses = false : _fake_responses = true; }
  void setFakeResponses(bool t) { _fake_responses = t; }
  bool getFakeResponses() { return _fake_responses; }
  unsigned char calc_vin_checksum(char *, int);
  vector <CanFrame *>fetchHistory(struct canfd_frame *);
  vector <CanFrame *>showCurrentData(struct canfd_frame *);
  vector <CanFrame *>vehicleInfoRequest(struct canfd_frame *);
  CanFrame *createPacket(int, char *, int);
  void setActiveTicks(int i) { _activeTicks = i; }
  int getActiveTicks() { return _activeTicks; }
 private:
  int arbId;
  int matched_isotp = 0;
  int missed_isotp = 0;
  bool padding = false;
  char padding_byte;
  bool responder = false;
  int state = STATE_IDLE;
  int _activeTicks = 0;
  int _x = 0;
  int _y = 0;
  int _type = 0;
  SDL_Texture *id_texture = NULL;
  vector<CanFrame *>can_history;
  vector<CanFrame *>_queue;
  int positive_responder_id = -1;
  int negative_responder_id = -1;
  bool _fake_responses = false;
};

#endif
