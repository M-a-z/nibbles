
/****************************************************************/
/*                                                              *
 *                  LICENSE for nibbles                         *
 *                                                              *
 *  It is allowed to use this program for free, as long as:     *
 *                                                              *
 *      -You use this as permitted by local laws.               *
 *      -You do not use it for malicious purposes like          *
 *      harming networks by messing up arp tables etc.          *
 *      -You understand and accept that any harm caused by      *
 *      using this program is not program author's fault.       *
 *      -You let me know if you liked this, my mail             *
 *      Mazziesaccount@gmail.com is open for comments.          *
 *                                                              *
 *  It is also allowed to redistribute this program as long     *
 *  as you maintain this license and information about          *
 *  original author - Matti Vaittinen                           *
 *  (Mazziesaccount@gmail.com)                                  *
 *                                                              *
 *  Modifying this program is allowed as long as:               *
 *                                                              *
 *      -You maintain information about original author/SW      *
 *      BUT also add information that the SW you provide        *
 *      has been modified. (I cannot provide support for        *
 *      modified SW.)                                           *
 *      -If you correct bugs from this software, you should     *
 *      send corrections to me also (Mazziesaccount@gmail.com   *
 *      so I can include fixes to official version. If I stop   *
 *      developing this software then this requirement is no    *
 *      longer valid.                                           *
 *                                                              *
 ****************************************************************/

#ifndef TGT_COMMANDER_H
#define TGT_COMMANDER_H
#include <inttypes.h>
#include <arpa/inet.h>


#define MSGNAME_MAX 32
#define FCM_DEFAULT_TESTPORT 15005
#define FSP_DEFAULT_TESTPORT 15005

typedef struct msglist
{
    struct msglist *next;
    char   msgname[MSGNAME_MAX+1];
    size_t msgsize;
    void   *msgdata;
}msglist;

typedef struct tgt_commander
{
    msglist msglistitem;
    int (*send_msg)(struct tgt_commander *,void *,size_t, char *);
    int (*add_msg)(struct tgt_commander *,char *,size_t,void *); /* add prefilled msg(s) TODO: implement some dynamic fields */
    msglist *(*find_msg)(struct tgt_commander *,char *);
    void (*uninit_commander)(struct tgt_commander **);
    /* TODO: Add functions to store partially filled msgs, and a function to send those partial msgs with fields filled */
}tgt_commander;


tgt_commander * init_tgt_commander();
int init_msgs(tgt_commander *commander,uint32_t fcmip, uint32_t fspip,unsigned short fcmnode, unsigned short fspnode);



#endif

