
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

#include <ncurses.h>
#include <errno.h>
#include <form.h>
#include "msgdefines.h"
#include "common.h"
#include <stdlib.h>
#include <string.h>
#include "syscomform.h"
#include "cexplode.h"
#include <arpa/inet.h>
#include <net/if.h>


static void *get_msg_from_form(formal_msg *_this,uint32_t *size,char **ifname,int *hdrtype);

int lenbytype(uint32_t *type)
{
    if(*type==*((uint32_t*)"u32"))
        return 4;
    if(*type==*((uint32_t *)"u16"))
        return 2;
    if(*type==*((uint32_t*)"u8"))
        return 1;
    if(*type==*((uint32_t*)"u64"))
        return 8;
    return 0;
}

int get_msg_size(char *s_payload, CexplodeStrings *ploder)
{
    int i;
//    CexplodeStrings ploder;
    i=Cexplode(s_payload," ",ploder);
    return i;
}

void addc(char **ptr,unsigned long long value)
{
    **ptr=(char)value;
    (*ptr)++;
}
void adds(char **ptr,unsigned long long value)
{
    unsigned short v=htons((unsigned short)value);
    memcpy(*ptr,&v,sizeof(v));
    (*ptr)+=sizeof(v);
}
void addl(char **ptr,unsigned long long value)
{
    uint32_t v=htonl((uint32_t)value);
    memcpy(*ptr,&v,sizeof(v));
    (*ptr)+=sizeof(v);
}
static void mva_ntohll(unsigned long long *num)
{
    short one=1;
    char *two=(char *)&one;
    if(!*two)
        return;
    two++;
    for(one=0;*(char*)&one<4;one++)
    {
        *two=((char *)num)[7-*(char*)&one];
        ((char *)num)[7-*(char*)&one]=((char *)num)[(int)*(char*)&one];
        ((char *)num)[(int)*(char*)&one]=*two;
    }
}


void addll(char **ptr,unsigned long long value)
{
    mva_ntohll(&value);
    memcpy(*ptr,&value,sizeof(value));
    (*ptr)+=sizeof(value);
}
/*
int  add_value(char **updateptr,unsigned long long value,uint32_t *type)
{
    int i;
    int typelen=lenbytype(type);
    void (*addtype[4])(char **ptr,unsigned long long value) =
    {
        &addc,
        &adds,
        &addl,
        &addll
    };

    for(i=0;i<4;i++)
    {
        if((1<<i)==typelen)
            addtype[i](updateptr,value);
    }
}
*/
int  add_value(char **updateptr,char *value)
{
    char *endptr;
    unsigned tmp;
    tmp=strtol(value,&endptr,0);
    if(!value || !*value || (*endptr && *endptr!='\n'))
    {
        DEBUGPR("Invalid msg payload byte '%s' given!",(value)?value:"NULL");
        return -1;
    }
    if(tmp>0xff)
    {
        DEBUGPR("Too large value 0x%x given in msg payload (biggest supported value is one byte (0xff)",tmp);
        return -1;
    }
    **updateptr=(char)tmp;
    (*updateptr)++;
    return 0;
}
void *parse_payload(char *s_payload,int *payloadlen,void **msghead,size_t hdrsize)
{
    //int i;
    int ok=0;
    //char type[4];
    //unsigned long long value;
    int msgsize;
    void *msg;
    char *updateptr;
    CexplodeStrings ploder;
    char *byte;
    *payloadlen=0;
    if(!s_payload)
        return NULL;
    msgsize=get_msg_size(s_payload,&ploder);
    if(msgsize<1)
    {
        Cexplode_free(ploder);
        return NULL;
    }
    *msghead=calloc(1,msgsize+hdrsize);
    if(!*msghead)
        return NULL;
    msg=updateptr=(((char *)*msghead)+hdrsize);
    byte=Cexplode_getfirst(&ploder);
    while(byte)
    {
        if(*payloadlen==msgsize)
        {
            DEBUGPR("Invalid msgsize!! expected %d, now copied %d bytes at %s:%d",msgsize,*payloadlen,__FILE__,__LINE__);
            ok=-1;
            break;
        }
        ok+=add_value(&updateptr,byte);
        (*payloadlen)++;
        byte=Cexplode_getnext(&ploder);
    }
    if(!ok)
        *payloadlen=msgsize;
    else
    {
        free(*msghead);
        *msghead=NULL;
        return NULL;
    }
    return msg;
}

static FILE * msgadd_start(void)
{
    FILE *msgfile=NULL;
    char *home=getenv("HOME");
    char *filename=calloc(1,strlen(home)+ 23 );
    if(filename)
    {
        sprintf(filename,"%s/.nibbles/msgtemplates",home);
        msgfile=fopen(filename,"a");
        if(msgfile)
        {
            char *msgstartrow="\n#msg_added_by_nibbles\nPL_ENDIANESS:MAINTAIN\n";
            if(!ftell(msgfile))
            {
                char *version="#ThongsMsgFile 1";
                if(1!=fwrite(version,strlen(version),1,msgfile))
                {
                    perror("Msg file write FAILED!\n");
                    fclose(msgfile);
                    msgfile=NULL;
                }
            }
            if(1!=fwrite(msgstartrow,strlen(msgstartrow),1,msgfile))
            {
                fclose(msgfile);
                msgfile=NULL;
            }
        }
        else
            fprintf(stderr,"Failed to open '%s' (%s)\n",filename,strerror(errno));
    }
    return msgfile;
}
int write_msg_to_file(char *msgname, FILE *msgfile, char *msgdata, uint32_t msgsize, int hdrtype)
{
    size_t hdrsizes[]={ sizeof(eth_hdr),sizeof(vlan_eth_hdr),sizeof(qinq_eth_hdr) };
    eth_hdr *hdr=(eth_hdr *)msgdata;
    char *positionptr;

    if(hdrsizes[hdrtype]>msgsize)
         return -1;

    if(0>fprintf(msgfile, "NAME:%s\n", msgname))
        return -1;

    if
    (
        0 > fprintf
        (
            msgfile, 
            "DMAC:%02x:%02x:%02x:%02x:%02x:%02x\n",
            hdr->dmac[0],
            hdr->dmac[1],
            hdr->dmac[2],
            hdr->dmac[3],
            hdr->dmac[4],
            hdr->dmac[5]
        )
    )
        return -1;

    if
    (
        0 > fprintf
        (
            msgfile, 
            "SMAC:%02x:%02x:%02x:%02x:%02x:%02x\n",
            hdr->smac[0],
            hdr->smac[1],
            hdr->smac[2],
            hdr->smac[3],
            hdr->smac[4],
            hdr->smac[5]
        )
    )
        return -1;

   
    switch(hdrtype)
    {
        case 0:
        {
            if(0>fprintf(msgfile, "ETYPE:0x%x\n",ntohs(hdr->ethtype)))
                return -1;
            break;
        }
        case 1:
        {
            vlan_eth_hdr *vhdr=(vlan_eth_hdr *)msgdata;
            if(0>fprintf(msgfile, "VLAN:0x%x\n",ntohl(vhdr->vlan_tag)))
                return -1;
            if(0>fprintf(msgfile, "ETYPE:0x%x\n",ntohs(vhdr->ethtype)))
                return -1;
            break;
        }
        case 2:
        {
            qinq_eth_hdr *qhdr=(qinq_eth_hdr *)msgdata;
            if(0>fprintf(msgfile, "QINQ:0x%x\n",ntohl(qhdr->vlan1_tag)))
                return -1;
            if(0>fprintf(msgfile, "VLAN:0x%x\n",ntohl(qhdr->vlan2_tag)))
                return -1;
            if(0>fprintf(msgfile, "ETYPE:0x%hx\n",ntohs(qhdr->ethtype)))
                return -1;
            break;
        }

    }

    fprintf(msgfile, "PAYLOAD:\n");
    for(positionptr=msgdata+hdrsizes[hdrtype];positionptr<msgdata+msgsize;positionptr++)
        if(0>fprintf(msgfile, "u8:0x%02x:foo\n",(unsigned)*(unsigned char *)positionptr))
            return -1;
    fprintf(msgfile, "DAOLYAP\n");
    return 0;
}
static int savemsg(formal_msg *_this)
{
    uint32_t size;
    int rval=0;
    FILE *msgfile;
    char ifname[IFNAMSIZ+1]={0};
    char *ifptr=&(ifname[0]);
    int hdrtype=0;
//    DEBUGPR("Msg storing not yet done! %s:%d\n",__FILE__,__LINE__);
    char *msg=get_msg_from_form(_this,&size,&ifptr,&hdrtype);
    char *errstring="msg parsing FAILED";
    char *msgname=field_buffer(_this->savenamefield,0);

    if(!msg)
        goto msg_get_failed;
    errstring="Can't write msgfile";
    msgfile=msgadd_start();
    if(!msgfile)
        goto msgfile_open_fail;
    errstring="error while writing msgfile may be corrupted!";
    if(write_msg_to_file(msgname,msgfile,msg,size,hdrtype))
        goto file_write_failed;
    fclose(msgfile);
    free(msg);
    set_field_buffer(_this->savenamefield,0,"Saved!");
    if(0)
    {
file_write_failed:
        fclose(msgfile);
msgfile_open_fail:
        free(msg);
msg_get_failed:
        set_field_buffer(_this->savenamefield,0,errstring);
        rval=-1;
    }
    return rval;
}

static void *get_msg_from_form(formal_msg *_this,uint32_t *size, char **ifname,int *hdrtype)
{
    char *convsuccess;
    /* 0 == normal ethernet,
     * 1 == vlan,
     * 2 == qinq
     */
    int ethhdrtype=0;
    size_t hdrsize[]={ sizeof(eth_hdr),sizeof(vlan_eth_hdr),sizeof(qinq_eth_hdr)};
    char *s_dmac,*s_smac,*s_etype,*s_vlan,*s_qinq,*s_payload,*s_ifname;
    uint8_t smac[6];
    uint8_t dmac[6];
    uint16_t etype;
    uint32_t vlan;
    uint32_t qinq;
//    unsigned short len,flags;
    char *errstring=NULL;
    void *msg=NULL;
    int payloadlen=0;
    void *payload=NULL;
    int i;
//    scommsghdr *hdr;
    //size_t etype_offset=offsetof(eth_hdr,ethtype);
    eth_hdr *ehdr;
    //vlan_eth_hdr *vhdr;
    //qinq_eth_hdr *qhdr;
    uint32_t *etypeptr;


    form_driver(_this->syscomform,REQ_END_LINE);
    s_dmac=field_buffer(_this->dmacfield,0);
    s_smac=field_buffer(_this->smacfield,0);
    s_etype=field_buffer(_this->ethtypefield,0);
    s_vlan=field_buffer(_this->vlanfield,0);
    s_qinq=field_buffer(_this->qinqfield,0);
    s_payload=field_buffer(_this->payloadfield,0);
    s_ifname=field_buffer(_this->ifnamefield,0);
    strncpy(*ifname,s_ifname,IFNAMSIZ);
    for(i=0;i<IFNAMSIZ && s_ifname[i];i++)
        if(s_ifname[i]==' ' && !(s_ifname[i]='\0'))
            break;

    errstring="Empty field in form!\n";
    if(!s_dmac || !s_smac || !s_etype)
        goto err_out;
    errstring="destination mac conversion FAILED!\n";
    if(get_mac(s_dmac,dmac))
        goto err_out;

    errstring="source mac conversion FAILED!\n";
    if(get_mac(s_smac,smac))
        goto err_out;

    qinq=strtol(s_qinq, &convsuccess, 0);
    if(*s_qinq && (!*convsuccess || *convsuccess==' ' ))
        ethhdrtype=2;

    vlan=strtol(s_vlan, &convsuccess, 0);
    errstring="Valid qinq tag but invalid VLAN tag!\n";
    if(*s_vlan && (!*convsuccess || *convsuccess==' ' ))
    {
        ethhdrtype= (ethhdrtype==2)?2:1;
    }
    else
        if(ethhdrtype==2)
            goto err_out;
    
    errstring="ethertype conversion FAILED!\n";
    etype=strtol(s_etype, &convsuccess, 0);
    if(!*s_etype || (*convsuccess && *convsuccess!=' ' ))
        goto err_out;

    if(s_payload && *s_payload!=' ')
    {
        DEBUGPR("Payload %s given \n",s_payload);
        payload=parse_payload(s_payload,&payloadlen,&msg,hdrsize[ethhdrtype]);
    }
    else
        msg=calloc(hdrsize[ethhdrtype],1);
    if(!payload)
        payloadlen=0;

    errstring="ALLOC FAILED\n"; 
    if(!msg)
        goto err_out;
    ehdr=(eth_hdr *)msg;
//    vhdr=(eth_hdr *)msg;
//    qhdr=(eth_hdr *)msg;
    memcpy(ehdr->dmac,dmac,6);
    memcpy(ehdr->smac,smac,6);
    etypeptr=(uint32_t *)&(ehdr->ethtype);
    switch(ethhdrtype)
    {
        case 2:                      
            *etypeptr=(uint32_t)htonl(qinq);
            etypeptr++;
        case 1:
            *etypeptr=(uint32_t)htonl(vlan);
            etypeptr++;
        case 0:
            *((uint16_t *)etypeptr)=htons(etype);
            *size=(hdrsize[ethhdrtype]+payloadlen);
            break;
    }
    *hdrtype=ethhdrtype;
    if(0)
    {
err_out:
        if(errstring)
            DEBUGPR(errstring);
        DEBUGPR("Could not get message from form!\n");
        return NULL;
    }
    return msg;
}

static void clear_form(formal_msg *_this)
{
    int i;
    for(i=0;i<7;i++)
    {
        form_driver(_this->syscomform,REQ_CLR_FIELD);
        form_driver(_this->syscomform,REQ_NEXT_FIELD);
        form_driver(_this->syscomform,REQ_END_LINE);
    }
}

static void form_chr(formal_msg *_this, int ch)
{
    switch(ch)
    {
        case SYSCOM_SAVEMSG_KEY:
            _this->savemsg(_this);
            break;
        case CLEAR_FORM:
            _this->clear_form(_this);
            break;
        case BELOW_FIELD:
            form_driver(_this->syscomform,REQ_DOWN_FIELD);
            form_driver(_this->syscomform,REQ_END_LINE);
            break;
        case UPPER_FIELD:
            form_driver(_this->syscomform,REQ_UP_FIELD);
            form_driver(_this->syscomform,REQ_END_LINE);
            break;
        case NEXT_CHAR:
            form_driver(_this->syscomform,REQ_NEXT_CHAR);
            break;
        case PREV_CHAR:
            form_driver(_this->syscomform,REQ_PREV_CHAR);
           break; 
        case NEXT_FIELD2:
        case NEXT_FIELD:
            form_driver(_this->syscomform,REQ_NEXT_FIELD);
            form_driver(_this->syscomform,REQ_END_LINE);
            break;
        case PREV_FIELD:
            form_driver(_this->syscomform,REQ_PREV_FIELD);
            form_driver(_this->syscomform,REQ_END_LINE);
            break;
        case KEY_BACKSPACE:
            form_driver(_this->syscomform,REQ_PREV_CHAR);
            form_driver(_this->syscomform,REQ_DEL_CHAR);
            break;
        default:
            form_driver(_this->syscomform,ch);
        break;

    }
}
static void display_form(formal_msg *_this)
{
    post_form(_this->syscomform);
}
static void fill_payload_field_from_bin(formal_msg *_this,char *payload,size_t payloadsize)
{
    char *ptr=calloc(5,payloadsize+1);
    int i;
    if(!ptr)
        return;
    for(i=0;i<payloadsize;i++,payload++,ptr+=5)
    {
        sprintf(ptr,"0x%02x ",(unsigned int) (unsigned char)*payload);
    }
    set_field_buffer(_this->payloadfield,0,ptr-(5*payloadsize));
    free(ptr-(5*payloadsize));
}
static void fill_header_fields_from_struct(formal_msg *_this,eth_hdr *msghdr,int msgtype)
{
    char tmp[100];
    tmp[99]='\0';
    snprintf
    (
        tmp,
        99,
        "%02x:%02x:%02x:%02x:%02x:%02x",
        msghdr->dmac[0],
        msghdr->dmac[1],
        msghdr->dmac[2],
        msghdr->dmac[3],
        msghdr->dmac[4],
        msghdr->dmac[5]
    );
    set_field_buffer(_this->dmacfield,0,tmp);
    snprintf
    (
        tmp,
        99,
        "%02x:%02x:%02x:%02x:%02x:%02x",
        msghdr->smac[0],
        msghdr->smac[1],
        msghdr->smac[2],
        msghdr->smac[3],
        msghdr->smac[4],
        msghdr->smac[5]
    );
    set_field_buffer(_this->smacfield,0,tmp);
    set_field_buffer(_this->vlanfield,0,"XXXXXXXXXX");
    set_field_buffer(_this->qinqfield,0,"XXXXXXXXXX");
    if(2==msgtype)
    {
        qinq_eth_hdr *qhdr=(qinq_eth_hdr *)msghdr;
        snprintf(tmp,99,"0x%04hx",qhdr->ethtype);
        set_field_buffer(_this->ethtypefield,0,tmp);
        snprintf(tmp,99,"0x%08hx",qhdr->vlan2_tag);
        set_field_buffer(_this->vlanfield,0,tmp);
        snprintf(tmp,99,"0x%08hx",qhdr->vlan1_tag);
        set_field_buffer(_this->qinqfield,0,tmp);
    }
    else if(1==msgtype)
    {
        vlan_eth_hdr *vhdr=(vlan_eth_hdr *)msghdr;
        snprintf(tmp,99,"0x%04hx",vhdr->ethtype);
        set_field_buffer(_this->ethtypefield,0,tmp);
        snprintf(tmp,99,"0x%08hx",vhdr->vlan_tag);
        set_field_buffer(_this->vlanfield,0,tmp);
    }
    else
    {
        snprintf(tmp,99,"0x%04hx",msghdr->ethtype);
        set_field_buffer(_this->ethtypefield,0,tmp);
    }
}

void fillform(formal_msg *_this,shitemsgparser *sp,void *msgitemhandle)
{
    DEBUGPR("Filling form from saved msgs not yet done %s:%d!",__FILE__,__LINE__);
     void *bulk_hdr;
     int hdrtype;
    size_t payloadsize;
    void *payload;
    if(!msgitemhandle)
        return;
    bulk_hdr=sp->get_matching_hdr(sp,msgitemhandle,&hdrtype);
    payloadsize=sp->get_matching_plsize(sp,msgitemhandle);
    _this->fill_header_fields_from_struct(_this,bulk_hdr,hdrtype);
    if(payloadsize)
    {
        payload=sp->get_matching_pl(sp,msgitemhandle);
        _this->fill_payload_field_from_bin(_this,payload,payloadsize);
    }
    return;
}

formal_msg * init_form(WINDOW *formwin)
{
    formal_msg *_this;
    _this=calloc(1,sizeof(formal_msg));
    if(_this)
    {
        _this->fillform= &fillform;
        _this->fill_payload_field_from_bin=&fill_payload_field_from_bin;
        _this->fill_header_fields_from_struct=&fill_header_fields_from_struct;
        _this->clear_form   =&clear_form;
        _this->form_chr     =&form_chr;
        _this->display_form =&display_form;
        _this->get_msg_from_form = &get_msg_from_form;
        _this->savemsg = &savemsg;

        _this->dmacfield      =new_field( DMACFIELD_HEIGHT      ,DMACFIELD_WIDTH,     HDRFIELD_STARTLINE,      DMACFIELD_STARTCOL,       0,0);
        _this->smacfield    =new_field( SMACFIELD_HEIGHT    ,SMACFIELD_WIDTH,   HDRFIELD_STARTLINE,      SMACFIELD_STARTCOL,     0,0);
        _this->vlanfield    =new_field( VLANFIELD_HEIGHT    ,VLANFIELD_WIDTH,   HDRFIELD2_STARTLINE,     VLANFIELD_STARTCOL,     0,0);
        _this->qinqfield    =new_field( QINQFIELD_HEIGHT    ,QINQFIELD_WIDTH,   HDRFIELD2_STARTLINE,     QINQFIELD_STARTCOL,     0,0);
        _this->ethtypefield =new_field( ETHTYPEFIELD_HEIGHT ,ETHTYPEFIELD_WIDTH,HDRFIELD2_STARTLINE,     ETHTYPEFIELD_STARTCOL,      0,0);
        _this->payloadfield =new_field( PAYLOADFIELD_HEIGHT ,PAYLOADFIELD_WIDTH,PAYLOAD_STARTLINE,       PAYLOADFIELD_STARTCOL,  0,0);
        _this->savenamefield=new_field( SAVENAMEFIELD_HEIGHT,SAVENAMEFIELD_WIDTH,SAVENAMEFIELD_STARTLINE,SAVENAMEFIELD_STARTCOL, 0,0);
        _this->ifnamefield  =new_field( DMACFIELD_HEIGHT    ,DMACFIELD_WIDTH    ,GENERIC_EXP_STARTLINE  ,ETHTYPEFIELD_STARTCOL, 0,0);

        _this->expl_send    =new_field( DMACFIELD_HEIGHT      ,GENERIC_EXP_WIDTH, GENERIC_EXP_STARTLINE,   DMACFIELD_STARTCOL,     0,0);
        _this->expl_dmac      =new_field( DMACFIELD_HEIGHT      ,DMACFIELD_WIDTH,     HDRFIELD_EXP_STARTLINE,  DMACFIELD_STARTCOL,       0,0);
        _this->expl_smac    =new_field( SMACFIELD_HEIGHT      ,SMACFIELD_WIDTH,   HDRFIELD_EXP_STARTLINE,  SMACFIELD_STARTCOL,     0,0);
        _this->expl_qinq    =new_field( QINQFIELD_HEIGHT      ,QINQFIELD_WIDTH,   HDRFIELD2_EXP_STARTLINE,  QINQFIELD_STARTCOL,     0,0);
        _this->expl_vlan    =new_field( VLANFIELD_HEIGHT      ,VLANFIELD_WIDTH,   HDRFIELD2_EXP_STARTLINE,  VLANFIELD_STARTCOL,     0,0);
        _this->expl_etype     =new_field( ETHTYPEFIELD_HEIGHT      ,ETHTYPEFIELD_WIDTH,    HDRFIELD2_EXP_STARTLINE,  ETHTYPEFIELD_STARTCOL,      0,0);
        _this->expl_pl      =new_field( DMACFIELD_HEIGHT      ,PAYLOADFIELD_WIDTH,PAYLOAD_EXPSTARTLINE,    PAYLOADFIELD_STARTCOL,  0,0);
        _this->expl_sna     =new_field( DMACFIELD_HEIGHT      ,SAVENAMEFIELD_WIDTH,SAVENAME_EXPSTARTLINE,  SAVENAMEFIELD_STARTCOL, 0,0);
        _this->nullfield   = NULL;

//        set_field_back(_this->savedfield, A_UNDERLINE);
        set_field_back(_this->dmacfield, A_UNDERLINE);
        set_field_back(_this->smacfield, A_UNDERLINE);
        set_field_back(_this->vlanfield, A_UNDERLINE);
        set_field_back(_this->qinqfield, A_UNDERLINE);
        set_field_back(_this->ethtypefield, A_UNDERLINE);
        set_field_back(_this->payloadfield, A_UNDERLINE);
        set_field_back(_this->savenamefield, A_UNDERLINE);
        set_field_back(_this->ifnamefield, A_UNDERLINE);

        field_opts_off(_this->expl_sna,  O_ACTIVE);
        field_opts_off(_this->expl_send,  O_ACTIVE);
        field_opts_off(_this->expl_dmac,  O_ACTIVE);
        field_opts_off(_this->expl_smac,O_ACTIVE);
        field_opts_off(_this->expl_qinq,O_ACTIVE);
        field_opts_off(_this->expl_vlan,O_ACTIVE);
        field_opts_off(_this->expl_etype, O_ACTIVE);
        field_opts_off(_this->expl_pl,  O_ACTIVE);

//        field_opts_off(_this->dmacfield,      O_AUTOSKIP);
//        field_opts_off(_this->smacfield,    O_AUTOSKIP);
//        field_opts_off(_this->vlanfield,    O_AUTOSKIP);
//        field_opts_off(_this->qinqfield,    O_AUTOSKIP);
//        field_opts_off(_this->ethtypefield,     O_AUTOSKIP);
//        field_opts_off(_this->payloadfield, O_AUTOSKIP);
//        field_opts_off(_this->savenamefield, O_AUTOSKIP);

        _this->formwin=formwin;

        _this->syscomform=new_form(&(_this->dmacfield));
        scale_form(_this->syscomform,&(_this->rows),&(_this->cols));
        wresize(_this->formwin,_this->rows+4,_this->cols+4);

    
        keypad(_this->formwin,TRUE);

        set_form_win(_this->syscomform,_this->formwin);
        set_form_sub(_this->syscomform,derwin(_this->formwin,_this->rows,_this->cols,2,2));
        box(_this->formwin,0,0);
        wprintw(_this->formwin,"(%s)",SYSCOM_TOGGLE_STR);
        
        set_field_buffer(_this->vlanfield ,0,"XXXXXXXXXX"); 
        set_field_buffer(_this->qinqfield ,0,"XXXXXXXXXX"); 
        set_field_buffer(_this->ifnamefield ,0,"eth0"); 
        set_field_buffer(_this->expl_send ,0,GENERIC_EXP_TEXT); 
        set_field_buffer(_this->expl_dmac ,0,DMAC_EXP_TEXT); 
        set_field_buffer(_this->expl_smac ,0,SMAC_EXP_TEXT); 
        set_field_buffer(_this->expl_qinq ,0,QINQ_EXP_TEXT); 
        set_field_buffer(_this->expl_vlan ,0,VLAN_EXP_TEXT); 
        set_field_buffer(_this->expl_etype ,0,ETHTYPE_EXP_TEXT); 
        set_field_buffer(_this->expl_pl ,0,PAYLOAD_EXP_TEXT); 
        set_field_buffer(_this->expl_sna ,0,SAVEMSG_EXP_TEXT); 
    }
    return _this;
}


