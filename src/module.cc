#include "module.h"

Module::Module(int id) {
  arbId = id;
}

Module::~Module() {
}

/* Calculates the confidence that it is a UDS module based on seen ISOTP packets to non */
float Module::confidence() {
  float total = getMatchedISOTP() + getMissedISOTP();
  return (float)getMatchedISOTP() / total;
}

void Module::addPacket(struct canfd_frame *cf) {
  CanFrame *newcf = new CanFrame(cf);
  bool dup_found = false;
  for(vector<CanFrame *>::iterator it = can_history.begin(); it != can_history.end(); ++it) {
    CanFrame *old = *it;
    if(old->str() == newcf->str()) dup_found = true;
  }
  if(!dup_found) can_history.push_back(newcf);
}

void Module::addPacket(string packet) {
  CanFrame *newcf = new CanFrame(packet);
  bool dup_found = false;
  for(vector<CanFrame *>::iterator it = can_history.begin(); it != can_history.end(); ++it) {
    CanFrame *old = *it;
    if(old->str() == newcf->str()) dup_found = true;
  }
  if(!dup_found) can_history.push_back(newcf);
}

/* Returns the state.  If ACTIVE also IDLEs it */
/* This basically only makes it active for one tick */
int Module::getState() {
  int s = state;
  if(state == STATE_ACTIVE) state = STATE_IDLE;
  return s;  
}

/* Retrives all matching patckets give a matching byte at position */
vector <CanFrame *>Module::getPacketsByBytePos(unsigned int pos, unsigned char byte) {
  vector <CanFrame *>matches;
  for(vector<CanFrame *>::iterator it = can_history.begin(); it != can_history.end(); ++it) {
    CanFrame *frame = *it;
    if(frame->len > pos) {
      if(frame->data[pos] == byte) {
        matches.push_back(frame);
      }
    }
  }
  return matches;
}
