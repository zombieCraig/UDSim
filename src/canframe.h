#ifndef CANFRAME_H
#define CANFRAME_H

#include <string>
#include <sstream>
#include <iomanip>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>

#include "can.h"

using namespace std;

class CanFrame {
  public:
    CanFrame(struct canfd_frame *);
    ~CanFrame();
    unsigned int can_id;
    unsigned char len;
    unsigned char data[8];
    string str();
};

#endif
