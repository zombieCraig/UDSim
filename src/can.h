#ifndef UDS_CAN_H
#define UDS_CAN_H

#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>
#include <string.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/can.h>
#include <linux/can/raw.h>

#include "canframe.h"

using namespace std;

#define CANID_DELIM '#'
#define DATA_SEPERATOR '.'

class CanFrame;

class Can
{
  public:
    Can(char *);
    ~Can();
    bool Init();
    string getIfname() { return ifname; }
    int parse_canframe(char *, struct canfd_frame *);
    unsigned char asc2nibble(char);
    vector <CanFrame *>getPackets();
    void sendPackets(vector <CanFrame *>);
  private:
    string ifname;
    int _canfd;
    struct sockaddr_can _addr;
};

#endif
