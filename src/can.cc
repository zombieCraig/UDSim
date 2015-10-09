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

