
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

#include <stdio.h>
#include "common.h"
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "tgt_commander.h"


static int send_msg(tgt_commander *_this,void *msgdata,size_t datasize,char *interface);
static int add_msg(tgt_commander *_this,char *msgname,size_t datasize,void *msgdata);
static msglist *find_msg(tgt_commander *_this,char *msgname);

static msglist *find_msg(tgt_commander *_this,char *msgname)
{
    msglist* found;

    for(found=_this->msglistitem.next;found;found=found->next/*,*msgsize=0 */)
    {
        if(found->msgname)
            if(!strcmp(found->msgname,msgname))
                break;
    }
    return found;
}
/*
static int send_stored(tgt_commander *_this,char *msgname,uint32_t ip,unsigned short port)
{
    msglist* item;
    if(!(item=_this->find_msg(_this,msgname)))
    {
        DEBUGPR("Could not find named msg '%s' for sending\n",msgname);
        return -1;
    }
    return _this->send_msg(_this,item->msgdata,item->msgsize,ifname);
}
*/
static int add_msg(tgt_commander *_this,char *msgname,size_t datasize,void *msgdata)
{
    msglist *new;
    msglist *tmp;
    new=calloc(1,sizeof(msglist));
    if(!new)
        return -1;
    strncpy(new->msgname,msgname,MSGNAME_MAX);
    new->msgsize=datasize;
    new->msgdata=msgdata;
    new->next=NULL;
    tmp=&(_this->msglistitem);
    while(tmp->next)
        tmp=tmp->next;
    tmp->next=new;
    return 0;
}

static int send_msg(tgt_commander *_this,void *msgdata,size_t datasize, char *ifname)
{
    struct ifreq ifr;
    int sock;
    struct sockaddr_ll device;
    memset(&device,0,sizeof(device));
    if(0> (sock = socket(PF_PACKET,SOCK_RAW,htons(ETH_P_ALL))))
    {
        printf("PF_PACKET sock failed, ()%s\n",strerror(errno));
        return EXIT_FAILURE;
    }
    strncpy((char *)&(ifr.ifr_name),ifname,sizeof(ifr.ifr_name));
    DEBUGPR("Sending using device: '%s'",ifname);
    if(ioctl(sock,SIOCGIFINDEX,&ifr))
    {
        /* ErrorTerror */
        ERRPR("interface '%s' recognition failed %d (%s)\n",ifname,errno,strerror(errno));
        return -1;
    }
    device.sll_family=AF_PACKET;
    device.sll_ifindex=ifr.ifr_ifindex;
    device.sll_halen=htons(6);
    if(-1==sendto(sock,msgdata,datasize, 0, (struct sockaddr *)&device, sizeof(device)))
    {
        DEBUGPR("sendto() failed: %s\n",strerror(errno));
        printf("sendto() failed: %s\n",strerror(errno));
        return -1;
    }
    close(sock);
    DEBUGPR("msg sent\n");
    return 0;
}
void uninit_commander(tgt_commander **_this_)
{
    if(_this_ && *_this_)
    {
        /* TODO: also add freeing of msglist */
        free(*_this_);
        *_this_=NULL;
    }
}
tgt_commander * init_tgt_commander()
{
    tgt_commander *_this=calloc(1,sizeof(tgt_commander));
    if(_this)
    {
        _this->send_msg=&send_msg;
        _this->add_msg=&add_msg;
        _this->find_msg=&find_msg;
        _this->uninit_commander=&uninit_commander;
    }
    return _this;
}

