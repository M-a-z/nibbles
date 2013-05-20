
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

#ifndef UDP_HANDLER_H
#define UDP_HANDLER_H

#include <sys/socket.h>
#include <netinet/ip.h>
#include "common.h"
#include "stringfilters.h"
#include "bshandler.h"

typedef struct fileargs
{
    char *filebasename;
    FILE *fptr;
    unsigned filesize;
    unsigned fileamnt;
    printptrhandler *phand;

}fileargs;


typedef struct portlist
{
    struct portlist *next;
    unsigned short port;
}portlist;

typedef struct sockstruct
{
    int sockamnt;
    int *sockarray;
}sockstruct;




typedef struct udp_handler
{
//    portlist cmdports;
    portlist ports;
    sockstruct st;
    printptrhandler *bufferhandler;
    fd_set rfds;
    int (*waitdata)(struct udp_handler *_this);
    int (*read_bs)(struct udp_handler *_this);
    void (*prepare_printbuffer)(struct udp_handler *_this,printptrhandler *bhandler);
    int (*add_port)(struct udp_handler *_this,unsigned short port);
    int (*start_sockets)(struct udp_handler *_this);
    int (*get_portamnt)(struct udp_handler *_this);
    int (*read_portcfgfile)(struct udp_handler *_this,FILE *cf);
}udp_handler;

udp_handler *init_udphandler();

#endif
