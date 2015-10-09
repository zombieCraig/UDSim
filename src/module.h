#ifndef UDS_MODULE_H
#define UDS_MODULE_H

#include <cstddef>
#include <vector>
#include <stdlib.h>
#include <string.h>
//#include <net/if.h>
//#include <linux/can.h>
//#include <linux/can/raw.h>

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
  void setPositiveResponse(Module *m) { positive_responder = m; }
  void setNegativeResponse(Module *m) { negative_responder = m; }
  void setResponder(bool v) { responder = v; }
  bool isResponder() { return responder; }
  void addPacket(struct canfd_frame *);
  int getState();
  void setState(int s) { state = s; }
  int getX() { return _x; }
  int getY() { return _y; }
  void setX(int x) { _x = x; }
  void setY(int y) { _y = y; }
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
  vector<CanFrame *>can_history;
  Module *positive_responder = NULL;
  Module *negative_responder = NULL;
};

#endif
