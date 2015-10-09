#ifndef UDS_CAN_H
#define UDS_CAN_H

#include <string>
#include <string.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/can.h>
#include <linux/can/raw.h>

using namespace std;

#define CANID_DELIM '#'
#define DATA_SEPERATOR '.'

class Can
{
  public:
    Can(char *);
    ~Can();
    string getIfname() { return ifname; }
    int parse_canframe(char *, struct canfd_frame *);
    unsigned char asc2nibble(char);
  private:
    string ifname;
};

#endif
