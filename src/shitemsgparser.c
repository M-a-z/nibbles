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

#include "shitemsgparser.h"
#include <string.h>
#include <stdlib.h>
#include "cexplode.h"

static void *get_matching_pl(shitemsgparser *_this,void *opaq)
{
    VERBOSE_DEBUGPR("shiteparser - Get matching pl called, returning %p\n",(!(opaq)?NULL:((shitemsglist *)opaq)->payload));
    if(opaq)
        return ((shitemsglist *)opaq)->payload;
     DEBUGPR("NOOO2!!!!!! %s:%d\n",__FILE__,__LINE__);
    return opaq;
}
static int get_matching_plsize(shitemsgparser *_this,void *opaq)
{
    VERBOSE_DEBUGPR("shiteparser - Get matching plsize called, returning %d\n",(!(opaq)?0:((shitemsglist *)opaq)->plsize));
     if(opaq)
        return ((shitemsglist *)opaq)->plsize;
     DEBUGPR("NOOO!!!!!! %s:%d\n",__FILE__,__LINE__);
    return -1; 
}

static void *get_matching_hdr(shitemsgparser *_this,void *opaq, int *hdrtype)
{
    VERBOSE_DEBUGPR("shiteparser - returning hdr %p when requested\n",(!(opaq)?NULL:( &(((shitemsglist *)opaq)->hdr))));
     if(opaq)
     {
         *hdrtype=((shitemsglist *)opaq)->hdrtype;
        return &(((shitemsglist *)opaq)->hdr);
     }
    return opaq; 
}

static int loaded_msg_amnt(struct shitemsgparser *_this)
{
    VERBOSE_DEBUGPR("shiteparser - returning msgamnt %d when requested\n",_this->msgamnt);
    return _this->msgamnt;
}
static void *get_first_msgitem(shitemsgparser *_this)
{
    return (void *)_this->msgs;
}
static void *get_next_msgitem(shitemsgparser *_this,void *opaq)
{
    if(opaq)
        return (void *)((shitemsglist *)opaq)->next;
    return opaq;
}
char *get_matching_desc(shitemsgparser *_this,void *opaq)
{
    if(opaq)
        return ((shitemsglist *)opaq)->msgdesc;
    return opaq;
}

char *get_matching_name(shitemsgparser *_this,void *opaq)
{
    if(opaq)
        return ((shitemsglist *)opaq)->msgname;
    return opaq;
}
int scan_msgfile_version(FILE *msgfile)
{
    int version;
    if(!msgfile)
    {
        DEBUGPR("shiteparser NULL msgfile in version check!\n");
        return -1;
    }
    if(fscanf(msgfile,"#ThongsMsgFile %d",&version)!=1)
    {
        DEBUGPR("First line not valid '#ThongsMsgFile <version>' tag!!\n");
        return -1;
    }
    DEBUGPR("ThongsMsgFile version %d found",version);
    return version;
}

#define NAMEINFO 1
#define DMACINFO   (NAMEINFO<<1)
#define SMACINFO   (DMACINFO<<1)
#define ETYPEINFO  (SMACINFO<<1)
#define VLANINFO  (ETYPEINFO<<1)
#define QINQINFO  (VLANINFO<<1)

#define ARE_REQUIRED_FOUND(fieldinfo) \
    ( \
      ( (fieldinfo)&NAMEINFO  ) && \
      ( (fieldinfo)&DMACINFO  ) && \
      ( (fieldinfo)&SMACINFO  ) && \
      ( (fieldinfo)&ETYPEINFO )  \
    )
#define NAMEFOUND(info) ( (info)&NAMEINFO )
#define DMACFOUND(info) ( (info)&DMACINFO )
#define SMACFOUND(info) ( (info)&SMACINFO )
#define ETYPEFOUND(info) ( (info)&ETYPEINFO )
#define VLANFOUND(info) ( (info)&VLANINFO )
#define QINQFOUND(info) ( (info)&QINQINFO )

char *dublicatestr(char *str)
{
    char *p=malloc(strlen(str)+1);
    if(p)
        strcpy(p,str);
    return p;
}

#define PAYLOADINC(foo) { \
            currpsize+=(foo); \
            if(currpsize>psize) \
            { \
                psize+=1024; \
                payload=realloc(payload,psize); \
                if(!payload) \
                    return -1; \
            } \
} 


int read_payload(shitemsglist *msgs,FILE *file,int convert,int *line)
{
    char *type,*lines;
    unsigned long long value;
    int rval;
    char *endp;
    int i;

    char *payload;
    size_t psize=1024;
    size_t currpsize=0;
    
    VERBOSE_DEBUGPR("shiteparser - parsing payload for msg %s\n",(!msgs->msgname)?"NULL":msgs->msgname);

    payload=calloc(1,psize);
    
    while(1)
    {
        rval=fscanf(file,"%a[^\n]\n",&lines);
        if(1!=rval)
            return -1;
        (*line)++;
        if(lines[0]=='#')
        {
            free(lines);
            continue;
        }
        if(!strcmp("DAOLYAP",lines))
        {
            char *p=calloc(1,currpsize);
            if(!p)
                return -1;
            memcpy(p,payload,currpsize);
            free(payload);
            msgs->payload=p;
            msgs->plsize=currpsize;
            VERBOSE_DEBUGPR("shiteparser - payload for msg %s successfully parsed!\n",(!msgs->msgname)?"NULL":msgs->msgname);
            return 0;
        }
        for(i=0;lines[i]!='\0'&&lines[i]!=':';i++);
        if(lines[i]=='\0' || '\0'==lines[i+1])
            return -1;
        lines[i]='\0';
        type=lines;
        value=strtoll(&(lines[i+1]),&endp,0);
        if(*endp && *endp!=':')
            return -1;
        if(!strcmp(type,"u8") || !strcmp(type,"i8"))
        {
            size_t curindex=currpsize;
            /* NOTE! This macro may return. It also changes local vars. It may do other EVIL things. 
             * If you touch this func (or debug this), ensure this macro follows changes 
             */
            PAYLOADINC(1);
            payload[curindex]=(unsigned char)value;
        }
        else if(!strcmp(type,"u16") || !strcmp(type,"i16"))
        {
            unsigned short v=(unsigned short)value;
            size_t curindex=currpsize;
            PAYLOADINC(2);
            if(!convert)
                memcpy(((char *)payload)+curindex,&v,sizeof(unsigned short));
            else
            {
                ((char *)payload)[curindex]=(unsigned char)(0xff00&v>>8);
                ((char *)payload)[curindex+1]=(unsigned char)(0xff&v);
            }
        }
        else if(!strcmp(type,"u32") || !strcmp(type,"i32"))
        {
            uint32_t v=(uint32_t)value;
            size_t curindex=currpsize;
            PAYLOADINC(4);
            if(!convert)
                memcpy(((char *)payload)+curindex,&v,sizeof(v));
            else
            {
                ((char *)payload)[curindex]=(unsigned char)(0xff000000&v>>24);
                ((char *)payload)[curindex+1]=(unsigned char)(0xff0000&v>>16);
                ((char *)payload)[curindex+2]=(unsigned char)(0xff00&v>>8);
                ((char *)payload)[curindex+3]=(unsigned char)(0xff&v);
            }
        }
        else if(!strcmp(type,"u64") || !strcmp(type,"i64"))
        {
            uint32_t *v1=(uint32_t *)&value;
            uint32_t *v2=(((uint32_t *)&value)+1);

            size_t curindex=currpsize;
            PAYLOADINC(8);
            if(!convert)
                memcpy(((char *)payload)+curindex,&value,sizeof(value));
            else
            {
                ((char *)payload)[curindex]=(unsigned char)((0xff000000&(*v2))>>24);
                ((char *)payload)[curindex+1]=(unsigned char)((0xff0000&(*v2))>>16);
                ((char *)payload)[curindex+2]=(unsigned char)((0xff00&(*v2))>>8);
                ((char *)payload)[curindex+3]=(unsigned char)(0xff&(*v2));
                ((char *)payload)[curindex+4]=(unsigned char)((0xff000000&(*v1))>>24);
                ((char *)payload)[curindex+5]=(unsigned char)((0xff0000&(*v1))>>16);
                ((char *)payload)[curindex+6]=(unsigned char)((0xff00&(*v1))>>8);
                ((char *)payload)[curindex+7]=(unsigned char)(0xff&(*v1));
            }
        }
        else
        {
            DEBUGPR("shiteparser - Unknown payload value type %s\n",type);
            free(type);
            return -1;
        }
        free(type);
    }

}

int get_message_from_file(shitemsgparser *_this,FILE *msgfile)
{
    char /**junk,*/*type,*value;
    //,*comment;
    int rval,line=0,funcret=0xabba;
    unsigned int requiredInfosFound=0;
    //CexplodeStrings ploder;
    int convert=0;
    shitemsglist *msgs,*tmpmsgs;
    msgs=calloc(1,sizeof(shitemsglist));

    VERBOSE_DEBUGPR("shiteparser - Parsing messages from file %s\n",_this->filename);
    if(!msgs)
    {
        DEBUGPR("MSG alloc FAILED! %s:%d\n",__FILE__,__LINE__);    
        return -1;
    }

    while(1)
    {
        if((rval=fscanf(msgfile,"%a[^:]:%a[^\n]\n",&type,&value)))
            line++;
        if(rval>0 && EOF != rval)
            if('#'==type[0])
            {
                free(type);
                if(2==rval)
                    free(value);
                continue;
            }

        if(rval==1)
        {
            int c=0;
            /* \n was propably not consumed since latter value was not filled. */
            while(EOF != (c=fgetc(msgfile)))
                if('\n'==(char)c)
                    break;
            if(!strcmp("PAYLOAD",type))
            {
                if(! ARE_REQUIRED_FOUND(requiredInfosFound) )
                {
                    DEBUGPR("shiteparser - no all required fields found before PAYLOAD tag\n");
                    free(type);
                    return -1;
                }
                requiredInfosFound=0;
                if(read_payload(msgs,msgfile,convert,&line))
                {
                    DEBUGPR("shiteparser - failed to parse payload!\n");
                    free(type);
                    return -1;
                }
                convert=0;
                if(!_this->msgs)
                    _this->msgs=msgs;
                else
                {
                    for(tmpmsgs=_this->msgs;tmpmsgs->next;tmpmsgs=tmpmsgs->next);
                        tmpmsgs->next=msgs;
                }
    //            msgs=calloc(1,sizeof(shitemsglist));
                funcret=0;
                break;
            }
            else
            {
                /* Unlknown tag in msg file! */
                DEBUGPR("Unknown tag %s at line %d in msg file %s\n",type,line,_this->filename);
            }
            free(type);
        }
        else if(2==rval)
        {
            if(!strcmp(type,"PL_ENDIANESS"))
            {
                funcret=1;
                if(!strcmp(value,"CONVERT"))
                    convert=1;
            }
            else if(!strcmp(type,"NAME"))
            {
                funcret=1;
                VERBOSE_DEBUGPR("shiteparser - MSG %s found from %s\n",value,_this->filename);
                if(NAMEFOUND(requiredInfosFound))
                {
                    DEBUGPR("dublicate name tag %s at line %d in msg file %s\n",type,line,_this->filename);
                    free(value);
                    free(type);
                    continue;
                }
                msgs->msgname=dublicatestr(value);
                requiredInfosFound|=NAMEINFO;
            }
            else if(!strcmp(type,"DMAC"))
            {
                //char *endp;
                funcret=1;
                if(DMACFOUND(requiredInfosFound))
                {
                    DEBUGPR("dublicate dmac tag %s at line %d in msg file %s\n",type,line,_this->filename);
                    free(value);
                    free(type);
                    continue;
                }
                requiredInfosFound|=DMACINFO;
                if(get_mac(value,msgs->hdr.dmac))
                {
                    DEBUGPR("Invalid mac '%s', expected format 1a:2b:3c:4d:5e:6f\n",value);
                    free(value);
                    free(type);
                    continue;
                }
            }
            else if(!strcmp(type,"SMAC"))
            {
//                char *endp;
                funcret=1;
                if(SMACFOUND(requiredInfosFound))
                {
                    DEBUGPR("dublicate smac tag %s at line %d in msg file %s\n",type,line,_this->filename);
                    free(value);
                    free(type);
                    continue;
                }
                requiredInfosFound|=SMACINFO;
                if(get_mac(value,msgs->hdr.smac))
                {
                    DEBUGPR("Invalid mac '%s', expected format 1a:2b:3c:4d:5e:6f\n",value);
                    free(value);
                    free(type);
                    continue;
                }
            }
            else if(!strcmp(type,"QINQ"))
            {
                char *endp;
                funcret=1;
                if(QINQFOUND(requiredInfosFound))
                {
                    DEBUGPR("dublicate qinq tag %s at line %d in msg file %s\n",type,line,_this->filename);
                    free(value);
                    free(type);
                    continue;
                }
                if(VLANFOUND(requiredInfosFound) || ETYPEFOUND(requiredInfosFound) )
                    DEBUGPR("ERROR: qinq tag %s at line %d in msg file %s found AFTER etype or vlan - overwriting...\n",type,line,_this->filename);
                msgs->hdr.vlan1_tag=strtol(value,&endp,0);
                msgs->hdrtype=HTYPE_QINQ;
                requiredInfosFound|=QINQINFO;
            }
            else if(!strcmp(type,"VLAN"))
            {
                char *endp;
                funcret=1;
                if(VLANFOUND(requiredInfosFound))
                {
                    DEBUGPR("dublicate vlan tag %s at line %d in msg file %s\n",type,line,_this->filename);
                    free(value);
                    free(type);
                    continue;
                }
                if(ETYPEFOUND(requiredInfosFound) )
                    DEBUGPR("ERROR: vlan tag %s at line %d in msg file %s found AFTER etype - overwriting...\n",type,line,_this->filename);
                if(HTYPE_QINQ==msgs->hdrtype)
                    msgs->hdr.vlan2_tag=strtol(value,&endp,0);
                else
                {
                    msgs->hdrtype=HTYPE_VLAN;
                    msgs->hdr.vlan1_tag=strtol(value,&endp,0);
                }
                requiredInfosFound|=VLANINFO;
            }
            else if(!strcmp(type,"ETYPE"))
            {
                char *endp;
                funcret=1;
                if(ETYPEFOUND(requiredInfosFound))
                {
                    DEBUGPR("dublicate etype tag %s at line %d in msg file %s\n",type,line,_this->filename);
                    free(value);
                    free(type);
                    continue;
                }
                if(HTYPE_QINQ==msgs->hdrtype)
                    msgs->hdr.ethtype=strtol(value,&endp,0);
                else if(HTYPE_VLAN==msgs->hdrtype)
                    *((uint16_t*)&(msgs->hdr.vlan2_tag))=strtol(value,&endp,0);
                else
                {
                    msgs->hdrtype=HTYPE_ENET;
                    *((uint16_t*)&(msgs->hdr.vlan1_tag))=strtol(value,&endp,0);
                }
                requiredInfosFound|=ETYPEINFO;
            }
            free(value);
            free(type);
        }
        else if(EOF == rval)
        {
           break; 
        }
        else if(!rval)
        {
            if((rval=fscanf(msgfile,"%a[^\n]\n",&type)))
                line++;
            if(1==rval)
            {
                if('#'==type[0])
                {
                    int tlen=strlen(type);
                    if(tlen>11 && type[0]=='M' && type[1]=='S'&& type[2]=='G'&& type[3]=='C'&& type[4]=='O'&& type[5]=='M'&& type[6]=='M'&& type[7]=='E'&& type[8]=='N' && type[8]=='T')
                        msgs->msgdesc=dublicatestr(&type[10]);
                }
            }
        }
        else
            return rval;
    }
//    free(msgs);
    return funcret;
}
static void release_shitemsglist(shitemsgparser *_this)
{
    shitemsglist *m,*ml=_this->msgs;
    while(ml)
    {
        m=ml->next;
        free(ml);
        ml=m;
    }
    _this->msgs=NULL;
}
static int load_msgs(shitemsgparser *_this)
{
    int i;
    if(_this->msgs)
        _this->release_shitemsglist(_this);
    if(!_this->msgfile)
    {
        _this->msgfile=fopen(_this->filename,"r");
    }
    if(_this->msgfile)
    {
        if(1!=scan_msgfile_version(_this->msgfile))
        {
            fclose(_this->msgfile);
            _this->msgfile=NULL;
            return -1;
        }
        for(i=0;!get_message_from_file(_this,_this->msgfile);i++)
            VERBOSE_DEBUGPR("shiteparser - msg found and added!\n");
        _this->msgamnt=i;
        fclose(_this->msgfile);
        _this->msgfile=NULL;
    }
    else
    {
        return -1;
    }
    return 0;
}
shitemsgparser *init_shitemsgparser(char *filename)
{
    shitemsgparser *_this;
    _this=calloc(1,sizeof(shitemsgparser));
    if(_this)
    {
        snprintf(_this->filename,SHITEFNAME_MAX-1,"%s",filename);
        _this->loaded_msg_amnt=&loaded_msg_amnt;
        _this->release_shitemsglist=&release_shitemsglist;
        _this->load_msgs=&load_msgs;
        _this->get_first_msgitem=&get_first_msgitem;
        _this->get_next_msgitem=&get_next_msgitem;
        _this->get_matching_plsize=&get_matching_plsize;
        _this->get_matching_pl=&get_matching_pl;
        _this->get_matching_hdr=&get_matching_hdr;
        _this->get_matching_name=&get_matching_name;
        _this->get_matching_desc=&get_matching_desc;
    }
    return _this;
}
