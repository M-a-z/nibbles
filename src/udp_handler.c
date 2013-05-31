
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

#ifndef _GNU_SOURCE
    #define  _GNU_SOURCE
#endif
#include "udp_handler.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "common.h"

static int prepare_socket(unsigned port);

static int start_sockets(udp_handler *_this)
{
    int i;
    unsigned short *ports,*strop;
    portlist *tmp,*tmp2;
    //st->sockamnt=0;
    //head=&(_this->ports);
    if(_this->ports.port)
    {
        ports=calloc(_this->ports.port,sizeof(unsigned short));
        if(!ports)
        {
            EARLY_DBGPR("Malloc FAILED! 1\n");
            return -1;
        }
        for(tmp=_this->ports.next,strop=ports;tmp;tmp=tmp2)
        {
            *strop=tmp->port;
            strop++;
            _this->st.sockamnt++;
            tmp2=tmp->next;
            free(tmp);
        }
        _this->st.sockarray=malloc(_this->st.sockamnt*sizeof(int));
        if(!(_this->st.sockarray))
        {
            EARLY_DBGPR("Malloc FAILED 2!\n");
            return -1;
        }

    }
    else
    {
        _this->st.sockamnt=3;
        _this->st.sockarray=malloc(_this->st.sockamnt*sizeof(int));
        ports=malloc(_this->st.sockamnt*sizeof(unsigned short));
    
        if(!ports||!_this->st.sockarray)
        {
            EARLY_DBGPR("Malloc FAILED 3!\n");
            return -1;
        }
        ports[0]=51001;
        ports[1]=51035;
        ports[2]=51003;
    }
    for(i=0;i<_this->st.sockamnt;i++)
    {
        _this->st.sockarray[i]=prepare_socket(ports[i]);
        if(0>_this->st.sockarray[i])
        {
            EARLY_DBGPR("Failed to bind sockets!\n");
            _this->st.sockamnt=0;
            free(_this->st.sockarray);
            free(ports);
            _this->st.sockarray=NULL;
            return -1;
        }
        else
            DEBUGPR("Listening port %hu\n",ports[i]);
    }
    free(ports);
    return 0;
}


static int prepare_socket(unsigned port)
{
    int sock;
    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));

    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);
    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    
    sock=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(0>sock)
    {
        EARLY_DBGPR("Failed to open socket\n");
        EARLY_DBGPR("%s\n",strerror(errno));
        return -1;
    }
    if(bind(sock,(const struct sockaddr *)&addr,sizeof(addr)))
    {
        EARLY_DBGPR("bind FAILED! %s\n",strerror(errno));
        close(sock);
        return -1;
    }
    DEBUGPR("bound port %hu to socket %d\n",port,sock);
    return sock;
}



static int waitdata(udp_handler *_this)
{
    int i;
    int greatestsock=0;
    int retval;

    FD_ZERO(&(_this->rfds));

    for(i=0;i<_this->st.sockamnt;i++)
    {
        if(0<_this->st.sockarray[i])
        {
            if(greatestsock<_this->st.sockarray[i])
                greatestsock=_this->st.sockarray[i];
            FD_SET(_this->st.sockarray[i], &(_this->rfds));
        }
    }
    VERBOSE_DEBUGPR("falling in select\n");
reselect:
    retval = select(greatestsock+1, &(_this->rfds), NULL, NULL, NULL);
    fflush(G_logfile);
    VERBOSE_DEBUGPR("select returned %d\n",retval);
    if (retval == -1)
    {
        if(errno == EINTR)
            goto reselect;
        if(errno == EAGAIN)
            goto reselect;
        else
            DEBUGPR("select FAILED! (%s)\n",strerror(errno));
        return -1;
    }
    return 0;
}

static int read_bs(udp_handler *_this)
{
    int readsocks=0;
    int i;
    int rcvd;
    int retval=0;
    char rcvbuff[1501];
    time_t tim;
    rcvbuff[1500]='\0';

    for(i=0;i<_this->st.sockamnt;i++)
    {
        if(FD_ISSET(_this->st.sockarray[i], &(_this->rfds)))
        {
            readsocks++;

rerecv:
            if(0>(rcvd=recv(_this->st.sockarray[i],rcvbuff,1500,0)))
            {
                int err=errno;
                if(EINTR!=err)
                {
                    DEBUGPR("Recv error! %s\n", strerror(err)); 
                    retval=_this->st.sockarray[i];
                }
                else
                    goto rerecv;

            }
            else if(rcvd)
            {
                int index;
                char *linestart=0;
                
                linestart=rcvbuff;
                char timestamp[TIMESTAMPSIZE];
                tim=time(NULL);
                if(!ctime_r(&tim,&(timestamp[0])))
                {
                    memset(&timestamp,' ',sizeof(timestamp));
                }
                timestamp[TIMESTAMPSIZE-1]='\0';
                timestamp[TIMESTAMPSIZE-2]='\0';
                for(index=0;index<=rcvd;index++)
                {
                    if(index>1 && (rcvbuff[index-1]=='\n' && rcvbuff[index-2]=='\r'))
                    {
                        rcvbuff[index-1]='\0';
                        rcvbuff[index-2]='\n';
                    }
                    if('\0'==rcvbuff[index])
                    {

                        if(&(rcvbuff[index])>=linestart+1 && index>2)
                        {
                            int filewrbs;
                            char *wrpoint;
                            int freespace;
                            int textlen;
                            int filewrite;
                            wrpoint=_this->bufferhandler->get_writepoint(_this->bufferhandler,&freespace);
                            if(!wrpoint)
                            {
                                DEBUGPR("Wituiks män!\n");       
                                out(-1);
                            }
                            if(freespace<(textlen=snprintf(wrpoint,freespace,"<%s> %s",timestamp,linestart)))
                            {
                                /* We do not want to have NULL but linefeed in buffer due to file dumping */
                                textlen=freespace;
                                wrpoint[freespace-1]='\n';
                            }
                            else
                            {
                                /* We do not want to perserve NULL in this case either */
//                                textlen-=1;
                            }
                            /* let's put line in buffer */
                            if((filewrite=_this->bufferhandler->update_writepoint(_this->bufferhandler,wrpoint,textlen,&filewrbs)))
                            {
                            //    schedule_filewrite(filewrite,filewrbs);
                            }
                        }
                        /* set next line to start from character after '\0' */
                        linestart=&(rcvbuff[index+1]);
                    }
                }
            }
            else
                perror("rcvd returned 0!\n");
        }
    }
    return retval;
}

void prepare_printbuffer(udp_handler *_this,printptrhandler *bhandler)
{
    _this->bufferhandler=bhandler;
}

static int add_port(struct udp_handler *_this,unsigned short num)
{
    portlist *old;
    portlist *port=malloc(sizeof(portlist));
    if(!port)
    {
        EARLY_DBGPR("Malloc FAILED 4!\n");
        return -1;
    }
    old=&(_this->ports);
    port->port=num;
    port->next=NULL;
    while(NULL!=old->next)
        old=old->next;
    old->next=port;
    if(_this->ports.port==0xffff)
    {
        EARLY_DBGPR("Only 0xffff ports can be configured\n");
        return -1;
    } 
    _this->ports.port++;
    return 0;


}
int get_portamnt(udp_handler *_this)
{
    return _this->st.sockamnt;
}
static int read_portcfgfile(udp_handler *_this,FILE *cf)
{
    unsigned short port;
    /* If commandline port was givem ignore config file ports */
    if(_this->ports.port)
        return 0;
    rewind(cf);
    DEBUGPR("Searching ports from cfg file\n");
    while((port=(unsigned short)scan_uint(cf,"port=%a[^\n]\n")))
    {
        if(_this->add_port(_this,port))
        {
            DEBUGPR("Failed to add port %hu from cfgfile",port);
            return -1;
        }
    }
    return 0;
}

udp_handler *init_udphandler()
{
    udp_handler *_this=calloc(1,sizeof(udp_handler));
    if(_this)
    {
        memset(_this,0,sizeof(udp_handler));
        _this->waitdata=&waitdata;
        _this->read_bs=&read_bs;
        _this->prepare_printbuffer=&prepare_printbuffer;
        _this->add_port=&add_port;
        _this->get_portamnt=&get_portamnt;
        _this->start_sockets=&start_sockets;
        _this->read_portcfgfile=&read_portcfgfile;
    }
    return _this;
}

