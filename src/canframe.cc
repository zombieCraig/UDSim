#include "canframe.h"

CanFrame::CanFrame(struct canfd_frame *cf) {
  int i;
  can_id = cf->can_id;
  len = cf->len;
  for(i=0; i < len; i++) { data[i] = cf->data[i]; }
  _cf = cf;
}

// Expects format 123#010203
CanFrame::CanFrame(string packet) {
  int i,pos;
  stringstream ss;
  string idstr, data, byte;
  pos = packet.find('#');
  if(pos != string::npos) {
     idstr = packet;
     data = packet;
     idstr.erase(pos-1, string::npos);
     data.erase(0, pos+1);   
     ss << hex << idstr;
     ss >> can_id;
     ss.clear();
     if(!data.empty()) {
       len = data.size() / 2;
       if(len > 8) len = 8;
       for(i=0; i < len; i++) {
         byte = data;
         if(i > 0) byte.erase(0, i*2);
         if(i < 7) byte.erase((i*2)+2, string::npos);
         ss << hex << byte;
         ss >> data[i];
         ss.clear();
       }
     } else {
       len = 0;
     }
  }
}

CanFrame::CanFrame() {
}

CanFrame::~CanFrame() {
  free(_cf);
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

struct canfd_frame *CanFrame::toFrame() {
  int i;
  if (_cf == NULL) {
    _cf = (struct canfd_frame *)malloc(sizeof(struct canfd_frame));
    _cf->can_id = can_id;
    _cf->len = len;
    for(i=0; i < len; i++) {
      _cf->data[i] = data[i];
    }
  }
  return _cf;
}
