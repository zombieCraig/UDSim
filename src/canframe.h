#ifndef CANFRAME_H
#define CANFRAME_H

#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>

#include "can.h"

using namespace std;

class CanFrame {
  public:
    CanFrame(struct canfd_frame *);
    CanFrame(string);
    CanFrame();
    ~CanFrame();
    unsigned int can_id;
    unsigned char len;
    unsigned char data[8];
    string str();
    string estr();
    struct canfd_frame *toFrame();
    vector<CanFrame *>queue;
  private:
    struct canfd_frame *_cf = NULL;
};

#endif
