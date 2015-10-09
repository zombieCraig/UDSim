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
  can_history.push_back(newcf);
}

/* Returns the state.  If ACTIVE also IDLEs it */
/* This basically only makes it active for one tick */
int Module::getState() {
  int s = state;
  if(state == STATE_ACTIVE) state = STATE_IDLE;
  return s;  
}
