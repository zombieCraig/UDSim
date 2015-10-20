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

void Module::setState(int s) {
  state = s;
  if (state == STATE_ACTIVE) _activeTicks = SDL_GetTicks();
}

int Module::getState() {
  return state;  
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

unsigned char Module::calc_vin_checksum(char *vin, int size) {
  char w[17] = { 8, 7, 6, 5, 4, 3, 2, 10, 0, 9, 8, 7, 6, 5, 4, 3, 2 };
  int i;
  int checksum = 0;
  int num;
  for(i=0; i < size; i++) {
    if(vin[i] == 'I' || vin[i] == 'O' || vin[i] == 'Q') {
      num = 0;
    } else {
      if(vin[i] >= '0' && vin[i] <='9') num = vin[i] - '0';
      if(vin[i] >= 'A' && vin[i] <='I') num = (vin[i] - 'A') + 1;
      if(vin[i] >= 'J' && vin[i] <='R') num = (vin[i] - 'J') + 1;
      if(vin[i] >= 'S' && vin[i] <='Z') num = (vin[i] - 'S') + 2;
    }
    checksum += num * w[i];
  }
  checksum = checksum % 11;
  if (checksum == 10) return 'X';
  return ('0' + checksum);
}

CanFrame *Module::createPacket(int id,char *data, int len) {
  CanFrame *cf = new CanFrame();
  int i, counter;
  int left = len;
  cf->can_id = id;
  if(len < 7) {
    cf->len = len + 1;
    cf->data[0] = len;
    for(i=1; i < len; i++) cf->data[i] = data[i-1];
    return cf;
  } else {
    cf->len = 8;
    cf->data[0] = 0x10;
    cf->data[1] = len;
    for(i=2; i < 8; i++) cf->data[i] = data[i-2];
    left -= 6;
    counter = 0x21;
    while(left > 0) {
      CanFrame *cfq = new CanFrame();
      cfq->can_id = id;
      cfq->data[0] = counter;
      if(left > 7) {
        cfq->len = 8;
        for(i=1; i < 8; i++) cfq->data[i] = data[len - left + i-1];
        _queue.push_back(cfq);
        counter++;
        if(counter >= 0x30) counter = 0x21;
        left-=7;
      } else {
        cfq->len = left + 1;
        for(i=1; i < left + 1; i++) cfq->data[i] = data[len - left + i-1];
        _queue.push_back(cfq);
        left = 0;
      }
    }
  }
  return cf;
}

// TODO: make this work :)
vector <CanFrame *>Module::fetchHistory(struct canfd_frame *cf) {
  vector <CanFrame *>resp;
  if(getPositiveResponder() > -1 || getPositiveResponder() > -1) {
    resp = Module::getPacketsByBytePos(1, cf->data[0]);
    if(resp.size() > 0) {
      cout << "TODO: Search history for match" << endl;
    }
  }
  return resp;
}

// Mode $01
vector <CanFrame *>Module::showCurrentData(struct canfd_frame *cf) {
  vector <CanFrame *>resp;
  char *buf;
  bool found = false;
  int target = cf->can_id + 9;
  stringstream ss;
  if(Module::getPositiveResponder() > -1) target = Module::getPositiveResponder();
  if(cf->len < 3) return resp;
  // Check  to see if we have seen this yet
  resp = Module::fetchHistory(cf);
  if (resp.size() > 0) found = true;
  if(!found && Module::getFakeResponses()) {
    switch(cf->data[2]) {
      case 0x00: // PIDS
      case 0x20:
      case 0x40:
      case 0x60:
      case 0x80:
      case 0xA0:
      case 0xC0:
        buf = (char *)malloc(6);
        buf[0] = cf->data[1] + 0x40;
        buf[1] = cf->data[2];
        buf[2] = 0xBF;
        buf[3] = 0xBF;
        buf[4] = 0xB9;
        buf[5] = 0x93;
        resp.push_back(Module::createPacket(target, buf, 6));
        free(buf);
        //ss << "PID supported " << (unsigned char)cf->data[2] << "-" << (unsigned char)cf->data[2] + 0x1F;
        //gd.Msg(ss.str());
        break;
      case 0x01: // MIL
        buf = (char *)malloc(6);
        buf[0] = cf->data[1] + 0x40;
        buf[1] = cf->data[2];
        buf[2] = 0x00;
        buf[3] = 0x07;
        buf[4] = 0xE5;
        buf[5] = 0xE5;
        resp.push_back(Module::createPacket(target, buf, 6));
        free(buf);
        gd.Msg("MIL and DTC Status");
        break;
      default:
        //ss << "Requested unsupported data 0x" << hex << cf->data[2];
        //gd.Msg(ss.str());
        break;
    }
  }
  return resp;
}

// Mode $09
vector <CanFrame *>Module::vehicleInfoRequest(struct canfd_frame *cf) {
  vector <CanFrame *>resp;
  CanFrame *test;
  bool found = false;
  int target = cf->can_id + 9;
  stringstream ss;
  char *buf;
  char default_vin[] = DEFAULT_VIN;
  if(Module::getPositiveResponder() > -1) target = Module::getPositiveResponder();
  if(cf->len < 3) return resp;
  // Check  to see if we have seen this yet
  resp = Module::fetchHistory(cf);
  if (resp.size() > 0) found = true;
  if(!found && Module::getFakeResponses()) {
    switch(cf->data[2]) {
      case 0x00: // Pids
        buf = (char *)malloc(6);
        buf[0] = cf->data[1] + 0x40;
        buf[1] = cf->data[2];
        buf[2] = 0x55;
        buf[3] = 0x0;
        buf[4] = 0x0;
        buf[5] = 0x0;
        resp.push_back(Module::createPacket(target, buf, 6));
        free(buf);
        gd.Msg("Vehicle PID Request");
        break;
      case 0x02: // VIN
        buf = (char *)malloc(strlen(DEFAULT_VIN) + 3);
        buf[0] = cf->data[1] + 0x40;
        buf[1] = cf->data[2];
        buf[2] = 1;
        memcpy(&buf[3], default_vin, strlen(DEFAULT_VIN));
        test = Module::createPacket(target, buf, strlen(DEFAULT_VIN) + 3);
        resp.push_back(test);
        free(buf);
        break;
      default:
        //ss << "Requested unsupported info 0x" << (unsigned char)cf->data[2];
        //gd.Msg(ss.str());
        break;
    }
  }
  return resp;
}

/* To generate a response we check these steps:
 * 1) Check to see if we had seen a similar request
 *    1) Any possible positive responses
 *    2) Any possible negative responses
 * 3) Any generic answers
 */
vector <CanFrame *>Module::getResponse(struct canfd_frame *cf) {
  vector <CanFrame *>resp;
  stringstream ss;
  if(cf->data[0] == 0x30) {  // Flow Control
    if(_queue.size() > 0) {
      resp = _queue;
      _queue.clear();
    }
  } else if (cf->len > 1) {
    switch(cf->data[1]) {
      // Modes
      case 0x01:
        ss << hex << cf->can_id << ": Mode Show Current Data";
        resp = Module::showCurrentData(cf);
        break;
      case 0x02:
        ss << hex << cf->can_id << ": Mode Show Freeze Frame";
        break;
      case 0x03:
        ss << hex << cf->can_id << ": Mode Read DTC";
        break;
      case 0x04:
        ss << hex << cf->can_id << ": Mode Clear DTC";
        break;
      case 0x05:
        ss << hex << cf->can_id << ": Mode Non-CAN Test Results";
        break;
      case 0x06:
        ss << hex << cf->can_id << ": Mode CAN Test Results";
        break;
      case 0x07:
        ss << hex << cf->can_id << ": Mode Read Pending DTCs";
        break;
      case 0x08:
        ss << hex << cf->can_id << ": Mode Control Operations";
        break;
      case 0x09:
        ss << hex << cf->can_id << ": Mode Vehicle Information";
        resp = Module::vehicleInfoRequest(cf);
        break;
      case 0x0A:
        ss << hex << cf->can_id << ": Mode Read Perm DTCs";
        break;
      // UDS
      case 0x10:
        if(_type = MODULE_TYPE_GM) {
          ss << hex << cf->can_id << ": (GMLAN) Initiate Diagnostic";
        } else {
          ss << hex << cf->can_id << ": Diagnostic Control";
        }
        break;
      case 0x11:
        ss << hex << cf->can_id << ": ECU Reset";
        break;
      case 0x12:
        ss << hex << cf->can_id << ": (GMLAN) Read Failure Record";
        break;
      case 0x14:
        ss << hex << cf->can_id << ": Clear DTC";
        break;
      case 0x19:
        ss << hex << cf->can_id << ": Read DTC";
        break;
      case 0x1A:
        ss << hex << cf->can_id << ": (GMLAN) Read DID by ID";
        break;
      case 0x20:
        ss << hex << cf->can_id << ": (GMLAN) Restart Communications";
        break;
      case 0x22:
        ss << hex << cf->can_id << ": Read Data by ID";
        break;
      case 0x23:
        ss << hex << cf->can_id << ": Read Memory by Address";
        break;
      case 0x24:
        ss << hex << cf->can_id << ": Read Scaling by ID";
        break;
      case 0x27:
        ss << hex << cf->can_id << ": Security Access";
        break;
      case 0x28:
        ss << hex << cf->can_id << ": (GMLAN) Stop Communications";
        break;
      case 0x2A:
        ss << hex << cf->can_id << ": Read Data by ID Periodic";
        break;
      case 0x2C:
        ss << hex << cf->can_id << ": Define Data ID";
        break;
      case 0x2E:
        ss << hex << cf->can_id << ": Write Data by ID";
        break;
      case 0x2F:
        ss << hex << cf->can_id << ": IO Control by ID";
        break;
      case 0x31:
        ss << hex << cf->can_id << ": Routine Control";
        break;
      case 0x34:
        ss << hex << cf->can_id << ": Request Download";
        break;
      case 0x35:
        ss << hex << cf->can_id << ": Request Upload";
        break;
      case 0x36:
        ss << hex << cf->can_id << ": Transfer Data";
        break;
      case 0x37:
        ss << hex << cf->can_id << ": Request Transfer Exit";
        break;
      case 0x38: 
        ss << hex << cf->can_id << ": Request Transfer File";
        break;
      case 0x3D:
        ss << hex << cf->can_id << ": Write Memory by Address";
        break;
      case 0x3E:
        ss << hex << cf->can_id << ": Tester Present";
        break;
      case 0x83:
        ss << hex << cf->can_id << ": Access Timing";
        break;
      case 0x84:
        ss << hex << cf->can_id << ": Secured Data Transfer";
        break;
      case 0x85:
        ss << hex << cf->can_id << ": Control DTC Settings";
        break;
      case 0x86:
        ss << hex << cf->can_id << ": Response on Event";
        break;
      case 0x87:
        ss << hex << cf->can_id << ": Link Control";
        break;
      case 0xA2:
        ss << hex << cf->can_id << ": (GMLAN) Programmed State";
        break;
      case 0xA5:
        ss << hex << cf->can_id << ": (GMLAN) Programing Mode";
        break;
      case 0xA9:
        ss << hex << cf->can_id << ": (GMLAN) Read Diag Info";
        break;
      case 0xAA:
        ss << hex << cf->can_id << ": (GMLAN) Read Data by ID";
        break;
      case 0xAE:
        ss << hex << cf->can_id << ": (GMLAN) Device Control";
        break;
      default:
        ss << hex << cf->can_id << ": Unknown request " << hex << (unsigned int)cf->data[1];
        break;
    }
  }
  if(ss.str().size() > 0) gd.Msg(ss.str());
  return resp;
}
