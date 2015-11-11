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
  string idstr, idata, byte, ext, epkt;
  unsigned int c;
  pos = packet.find(',');
  ext = packet;
  // Does the packet contian extended info?
  while(pos != string::npos) {
    ext.erase(0, pos+1);
    epkt = ext;
    pos = ext.find(',');
    if(pos != string::npos) epkt.erase(pos, string::npos);
    CanFrame *newcf = new CanFrame(epkt);
    queue.push_back(newcf);
  }
  pos = packet.find('#');
  if(pos != string::npos) {
     idstr = packet;
     idata = packet;
     idstr.erase(pos, string::npos);
     idata.erase(0, pos+1);   
     ss << hex << idstr;
     ss >> can_id;
     ss.str("");
     ss.clear();
     if(!idata.empty()) {
       len = idata.size() / 2;
       if(len > 8) len = 8;
       for(i=0; i < len; i++) {
         byte = idata;
         if(i < 7) byte.erase((i*2)+2, string::npos);
         if(i > 0) byte.erase(0, i*2);
         ss << hex << byte;
         ss >> hex >> c;
         data[i] = c;
         ss.str("");
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
  if(_cf) free(_cf);
}

// Convert basic canframe to a string
string CanFrame::str() {
  stringstream pkt;
  pkt << hex << can_id << CANID_DELIM;
  int i;
  for(i=0; i < len; i++) {
    pkt << setfill('0') << setw(2) << hex << (int)data[i];
  }
  return pkt.str();
}

// estr - Extended string
// Convert canframe and extended packets (size > 8) as string
string CanFrame::estr() {
  stringstream pkt;
  pkt << CanFrame::str();
  for(vector<CanFrame *>::iterator it = queue.begin(); it != queue.end(); ++it) {
    CanFrame *e = *it;
    pkt << "," << e->str();
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
