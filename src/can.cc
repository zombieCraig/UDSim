#include "can.h"

Can::Can(char *interface) {
  ifname = string(interface);
}

Can::~Can() {

}

/* Pulled from can-utils lib.c */
int Can::parse_canframe(char *cs, struct canfd_frame *cf) {
        /* documentation see lib.h */

        int i, idx, dlen, len;
        int maxdlen = CAN_MAX_DLEN;
        int ret = CAN_MTU;
        unsigned char tmp;

        len = strlen(cs);
        //printf("'%s' len %d\n", cs, len);

        memset(cf, 0, sizeof(*cf)); /* init CAN FD frame, e.g. LEN = 0 */

        if (len < 4)
                return 0;

        if (cs[3] == CANID_DELIM) { /* 3 digits */

                idx = 4;
                for (i=0; i<3; i++){
                        if ((tmp = asc2nibble(cs[i])) > 0x0F)
                                return 0;
                        cf->can_id |= (tmp << (2-i)*4);
                }

        } else if (cs[8] == CANID_DELIM) { /* 8 digits */

                idx = 9;
                for (i=0; i<8; i++){
                        if ((tmp = asc2nibble(cs[i])) > 0x0F)
                                return 0;
                        cf->can_id |= (tmp << (7-i)*4);
                }
                if (!(cf->can_id & CAN_ERR_FLAG)) /* 8 digits but no errorframe?  */
                        cf->can_id |= CAN_EFF_FLAG;   /* then it is an extended frame */

        } else
                return 0;

        if((cs[idx] == 'R') || (cs[idx] == 'r')){ /* RTR frame */
                cf->can_id |= CAN_RTR_FLAG;

                /* check for optional DLC value for CAN 2.0B frames */
                if(cs[++idx] && (tmp = asc2nibble(cs[idx])) <= CAN_MAX_DLC)
                        cf->len = tmp;

                return ret;
        }

        if (cs[idx] == CANID_DELIM) { /* CAN FD frame escape char '##' */

                maxdlen = CANFD_MAX_DLEN;
                ret = CANFD_MTU;

                /* CAN FD frame <canid>##<flags><data>* */
                if ((tmp = asc2nibble(cs[idx+1])) > 0x0F)
                        return 0;

                cf->flags = tmp;
                idx += 2;
        }

        for (i=0, dlen=0; i < maxdlen; i++){

                if(cs[idx] == DATA_SEPERATOR) /* skip (optional) separator */
                        idx++;

                if(idx >= len) /* end of string => end of data */
                        break;

                if ((tmp = asc2nibble(cs[idx++])) > 0x0F)
                        return 0;
                cf->data[i] = (tmp << 4);
                if ((tmp = asc2nibble(cs[idx++])) > 0x0F)
                        return 0;
                cf->data[i] |= tmp;
                dlen++;
        }
        cf->len = dlen;

        return ret;  
}

unsigned char Can::asc2nibble(char c) {
        if ((c >= '0') && (c <= '9'))
                return c - '0';

        if ((c >= 'A') && (c <= 'F'))
                return c - 'A' + 10;

        if ((c >= 'a') && (c <= 'f'))
                return c - 'a' + 10;

        return 16; /* error */
}

bool Can::Init() {
  struct ifreq ifr;

  // Create a new raw CAN socket
  _canfd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if(_canfd < 0) {
    cout << "Can not initialize a raw CAN socket" << endl;
    return false;
  }

  _addr.can_family = AF_CAN;
  memset(&ifr.ifr_name, 0, sizeof(ifr.ifr_name));
  strncpy(ifr.ifr_name, ifname.c_str(), ifname.size());
  if (ioctl(_canfd, SIOCGIFINDEX, &ifr) < 0) {
    perror("SIOCGIFINDEX");
    return false;
  }
  _addr.can_ifindex = ifr.ifr_ifindex;

  if (bind(_canfd, (struct sockaddr *)&_addr, sizeof(_addr)) < 0) {
        perror("bind");
        false;
  }
  return true;
}

/* This currently only returns one packet but we set it up this
 * way in case we want to do buffering in the future */
vector <CanFrame *>Can::getPackets() {
  vector <CanFrame *>packets;
  struct iovec iov;
  struct msghdr msg;
  struct canfd_frame frame;
  int ret, nbytes;
  fd_set rdfs;
  char ctrlmsg[CMSG_SPACE(sizeof(struct timeval)) + CMSG_SPACE(sizeof(__u32))];
  struct timeval timeo;

  iov.iov_base = &frame;
  iov.iov_len = sizeof(frame);
  msg.msg_name = &_addr;
  msg.msg_namelen = sizeof(_addr);
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_control = &ctrlmsg;
  msg.msg_controllen = sizeof(ctrlmsg);
  msg.msg_flags = 0;

    FD_ZERO(&rdfs);
    FD_SET(_canfd, &rdfs);
    timeo.tv_sec  = 0;
    //timeo.tv_usec = 10000 * 20; // 20 ms  
    timeo.tv_usec = 10000 * 1; // 1 ms  

    if ((ret = select(_canfd+1, &rdfs, NULL, NULL, &timeo)) < 0) {
      cout << "Error: Interface is probably down" << endl;
      return packets;
    }
    if (FD_ISSET(_canfd, &rdfs)) {
      nbytes = recvmsg(_canfd, &msg, 0);
      if (nbytes < 0) {
        perror("read");
        return packets;
      }
      if ((size_t)nbytes != CAN_MTU) {
        fprintf(stderr, "read: incomplete CAN frame\n");
        return packets;
      }
      packets.push_back(new CanFrame(&frame));
    }

  return packets;
}
