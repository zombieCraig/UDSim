#include "canframe.h"

CanFrame::CanFrame(struct canfd_frame *cf) {
  int i;
  can_id = cf->can_id;
  len = cf->len;
  for(i=0; i < 8; i++) { data[i] = cf->data[i]; }
}

CanFrame::~CanFrame() {

}

string CanFrame::str() {
  stringstream pkt;
  pkt << hex << can_id << CANID_DELIM;
  int i;
  for(i=0; i < len; i++) {
    pkt << setfill('0') << setw(2) << hex << (int)data[i];
  }
  return pkt.str();
}
