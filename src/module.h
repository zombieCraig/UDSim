#ifndef UDS_MODULE_H
#define UDS_MODULE_H

#include <cstddef>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "canframe.h"

using namespace std;

#define STATE_IDLE      0
#define STATE_ACTIVE    1
#define STATE_MOUSEOVER 2
#define STATE_SELECTED  3

#define MODULE_H 30
#define MODULE_W 35

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
  void setState(int s) { state = s; }
  int getX() { return _x; }
  int getY() { return _y; }
  void setX(int x) { _x = x; }
  void setY(int y) { _y = y; }
  SDL_Texture *getIdTexture() { return id_texture; }
  void setIdTexture(SDL_Texture *t) { id_texture = t; }
 private:
  int arbId;
  int matched_isotp = 0;
  int missed_isotp = 0;
  bool padding = false;
  char padding_byte;
  bool responder = false;
  int state = STATE_IDLE;
  int _x = 0;
  int _y = 0;
  SDL_Texture *id_texture = NULL;
  vector<CanFrame *>can_history;
  int positive_responder_id = -1;
  int negative_responder_id = -1;
};

#endif
