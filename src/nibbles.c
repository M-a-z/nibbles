
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
#include <ncurses.h>
#include <panel.h>
#include "syscomform.h"
#include "displayhandler.h"
#include "common.h"
#include "stringfilters.h"
#include "bshandler.h"
#include "udp_handler.h"
#include <getopt.h>
#include <unistd.h>
#include <sched.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include "cexplode.h"
#include <signal.h>
#include "tgt_commander.h"
#include "shitemsgparser.h"
#include "msgloadermenu.h"
#include <net/if.h>

/* We use fscanf specifier %a (to make fscanf allocate space for read string) which is gnu extension */



static struct option long_options[] =
{
    {"fcmip", required_argument,  0, 'a'},
    {"fspip", required_argument,  0, 'b'},
    {"fcmport", required_argument,  0, 'A'},
    {"fspport", required_argument,  0, 'B'},
    {"config", required_argument,  0, 'c'},
    {"debug", required_argument,  0, 'd'},
    {"file" , required_argument, 0, 'f'},
    {"size" , required_argument, 0, 's'},
    {"rotate" , required_argument, 0, 'r'},
    {"port" , required_argument, 0, 'p'},
    {"version",  no_argument, 0, 'v'},
    {"help",  no_argument, 0, 'h'},
    {"editor",  required_argument, 0, 'e'},
    {0,0,0,0}
};




typedef struct user_commands
{
    int pause;
    int end;
    int show_help;
    int show_form;
    int scroll_mode;
    char *editor;
    int scrollcmd;
}user_commands;



#define NEXTBSD(bsd) ((rcvprints *) (((char *)(bsd)) + (((rcvprints*)(bsd))->datasize) ))
//+ (ALIGN_SIZE-(((rcvprints*)(bsd))->datasize)%ALIGN_SIZE) ))


//static void * start_editor(void *params) __attribute__((unused));

pthread_mutex_t G_condmutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t G_cond=PTHREAD_COND_INITIALIZER;

static int G_NOFILTERS=0;
FILE *G_logfile=NULL;

int G_USE_FILE=0;
char G_CURRENT_UDP_FILE_NAME[1024];
//FILE *G_outfile=NULL;

static char *G_exename;
pthread_t G_tid=0;
int G_filewriter_running=0;
int G_bs1_wrsize=0;
int G_bs2_wrsize=0;
int G_bs1filewrneeded=0;
int G_bs2filewrneeded=0;

static void print_usage()
{
    EARLY_DBGPR("\nUsage:\n\n\n");
    EARLY_DBGPR("%s <options>\n\n",G_exename);
    EARLY_DBGPR(HELP_PRINT);
}

void *UDPreaderthread(void *arg)
{
    /* do UDP reading here */
    udp_handler *hand=(udp_handler *)arg;
    pthread_cleanup_push(&udp_file_flush,arg);
    
    signal(SIGTERM,&out);
    signal(SIGINT,&out);
    signal(SIGSTOP,SIG_IGN);
    while(1)
    {
        int bsfail=0;
        if(hand->waitdata(hand))
        {
            DEBUGPR("something went vituiksi\n");
            return NULL;
        }
        if((bsfail=hand->read_bs(hand)))
        {
            DEBUGPR("UDPreader - read_bs FAILED %d!\n",bsfail);
            continue;
        }
    }
    pthread_cleanup_pop(1);
    return NULL;
}
typedef struct editorthreadparam{ int *mode; char *editor; char *logname; sdisplayhandler *dh;}editorthreadparam;

static void * start_editor(void *params)
{
    char command[1000];
    editorthreadparam *p=(editorthreadparam*)params;
    snprintf(command,999,"%s '%s'",p->editor,p->logname);
    DEBUGPR("executing editor command %s\n",command);
    command[999]='\0';
    endwin();
    system(command);
	cbreak();
	keypad(stdscr, TRUE);		/* I need that nifty F1 	*/
    update_panels();
    doupdate();
    timeout(10);
    noecho();
    //p->dh->magic(p->dh);
    *(p->mode)=0;
    free(p);
    return NULL;
}

static void handle_char(int ch,stringfilter *filters,sdisplayhandler *displayhandler,char *filterstring,unsigned *filterindex,user_commands *uc,tgt_commander *commander,smsgloader *scommenu,shitemsgparser *sp,printptrhandler *bhandler)
{
    editorthreadparam *parm;
    //int fsp=0;
    pthread_attr_t attr;

    if(uc->pause)
    {
        switch(ch)
        {
            case PAUSECHAR:
                goto pausechar;
                break;
            case ENDCHAR:
                goto endchar;
                break;
            case KEY_UP:
                uc->scrollcmd=KEY_UP;
                break;
            case KEY_DOWN:
                uc->scrollcmd=KEY_DOWN;
                break;
        }
    }
    else if(uc->scroll_mode)
    {
         return;
    }
    else
        switch(ch)
        {
            case SCROLLCHAR:
            {
                parm=malloc(sizeof(editorthreadparam));
                if(!parm)
                    break;
                if(!G_USE_FILE)
                    break;
                pthread_t tid;
                parm->dh=displayhandler;
                parm->mode=&(uc->scroll_mode);
                parm->editor=uc->editor;
                parm->logname=G_CURRENT_UDP_FILE_NAME;
                uc->scroll_mode=1;

                pthread_attr_init(&attr);
                pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

                if( pthread_create(&tid,&attr,start_editor,parm))
                {
                    DEBUGPR("Failed to launch scroller thread!\n");
                    uc->scroll_mode=0;
                }
                pthread_attr_destroy(&attr);
                break;
            }
            case ENDCHAR:
        //case ENDCHAR2:
endchar:
                uc->end=1;
                break;
            case PAUSECHAR:
pausechar:
                DEBUGPR("Toggling pause %d->%d\n",uc->pause,!uc->pause);
                uc->pause=!uc->pause;
                filters->set_paused(filters,uc->pause);
                bhandler->toggle_scrollmode(bhandler,uc->pause);
                break;
            case FSTR_TOGGLE_CHAR:
                displayhandler->toggle_fstr(displayhandler);
                break;
            case DEFFIND_TOGGLE_CHAR:
                displayhandler->toggle_deffind(displayhandler);
                break;
            case HELPCHAR:
                displayhandler->toggle_help(displayhandler);
                break;
            case ERR:
                break;
            case SCOMMENU_TOGGLE_CHAR:
                displayhandler->toggle_scommenu(displayhandler);
                break;
            case FORM_TOGGLE_CHAR:
                displayhandler->toggle_scom(displayhandler);
                break;
/*            case FORM_SEND_FSP_CHAR:
                fsp=1;
*/
            case FORM_SEND_FCM_CHAR:
            {
                char ifname[IFNAMSIZ+1];
                void *msg;
                uint32_t msglen;
                char *nameptr=&(ifname[0]);
                if(!displayhandler->in_syscommode(displayhandler))
                    break;
                msg=displayhandler->get_built_msg(displayhandler,&msglen,&nameptr);
                if(!msg)
                {
                    DEBUGPR("Could not fetch built msg!\n");
                    break;
                }
                else if(commander)
                    commander->send_msg(commander,msg,msglen,ifname);
            }
            break;
            case 10:
            case KEY_ENTER:
            {
                if(displayhandler->scommenuontop(displayhandler))
                {
                    displayhandler->formhandler->fillform(displayhandler->formhandler,sp,scommenu->get_selected_hidden(scommenu));
                    ungetch(KEY_F(3));
                    break;
                }

                werase(displayhandler->filter_win);
                if(!strcmp(filterstring,"C"))
                {
                    displayhandler->clearlog(displayhandler);
                }
                /*
                if(commander)
                {
                    if(filterstring[0]==COMMAND_STARTCHAR)
                    {
                        goto clear_filterstring;
                        break;
                    }
                }
                */
                if(G_NOFILTERS)
                    break;
                DEBUGPR("adding filter!\n");
                filters->add(filters, filterstring);

                *filterindex=0;
                filterstring[0]=filterstring[1]='\0',filterstring[2]='\0',filterstring[3]='\0';
                break;
                /* apply filters */
            }
            default:
                if(displayhandler->scommenuontop(displayhandler))
                {
                    scommenu->handle_char(scommenu,ch);
                    break;
                }
                if(displayhandler->in_syscommode(displayhandler) || displayhandler->in_deffindmode(displayhandler))
                {
                    displayhandler->handle_char(displayhandler,ch);
                    break;
                }
                if(G_NOFILTERS)
                    break;
                /* add characters to filter */
                if(ch==KEY_BACKSPACE)
                {
                    DEBUGPR("Backspace detected\n");
                    if(*filterindex)
                    {    
                        filterstring[*filterindex-1]='\0';
                        *filterindex=(*filterindex)-1;
                        wmove(displayhandler->filter_win,0,*filterindex);
                        wclrtoeol(displayhandler->filter_win);
                    }
                }
                else
                {
                    if(*filterindex<1024)
                        (*filterindex)++;
                
                    filterstring[*filterindex-1]=ch;
                    filterstring[*filterindex]='\0';
                    DEBUGPR("Added char '%c' (%d) to filterstring '%s' - index %d\n",(char)ch,ch,filterstring,*filterindex-1);
                    wprintw(displayhandler->filter_win,"%c",ch);
                }
        }

}
static void display_udpdata(stringfilter *filters,sdisplayhandler *displayhandler,printptrhandler *bufferhandler,user_commands *uc)
{
    int size;
    char *blockstart;
    unsigned linesize;
    char *linestart;
    int hl;
    int i;
    int blockread;
    char * (*blockfetchfunc)(struct printptrhandler *_this,int *size);
    int y,x;
    /* Actually this is screen size */
    unsigned lines;
    

    if(uc->pause)
    {
        if(!uc->scrollcmd)
            return;
        /* This makes bufferhandler to count lines chars in scrollcmd direction and return a block that fits on screen judging this info */
        getmaxyx(displayhandler->logwin,y,x);
        /* Actually this is screen size */
        lines=y*x;
        VERBOSE_DEBUGPR("Using screen size = 0x%x\n",lines);
        bufferhandler->scroll_set_offset_block(bufferhandler,lines,uc->scrollcmd);
        uc->scrollcmd=0;
        blockfetchfunc=bufferhandler->scroll_get_offset_block;
        wclear(displayhandler->logwin);
    }
    else
        blockfetchfunc=bufferhandler->get_next_readable;

    while((blockstart=(*blockfetchfunc)(bufferhandler,&size)))
    {
        linestart=blockstart;
        for(blockread=0;blockread<size;blockread+=linesize)
        {
            for(linesize=0;linestart[linesize]!='\n' && blockread+linesize<size;linesize++)
                if(blockread==size-1)
                {
                    break;
                }
            if(blockread+linesize<size)
                linesize++;

            if(!filters->filter(filters,linestart,linesize))
            {
                if(!filters->filter(filters,linestart,linesize))
                {
                    if((hl=filters->hl(filters,linestart,linesize)))
                        displayhandler->log_start_hl(displayhandler);
                    for(i=0;i<linesize;i++)
                        waddch(displayhandler->logwin,linestart[i]);
                    if(linestart[i-1]!='\n')
                        waddch(displayhandler->logwin,'\n');
                    if(hl)
                        displayhandler->log_end_hl(displayhandler);
                }
            }
            linestart=linestart+linesize;
        }
    }
}
static int argchk(char *arg, unsigned long int lower, unsigned int upper, unsigned long int *value)
{
    char *chkptr;
    unsigned long int retval = 0;
    *value=0;
    if(NULL==arg)
    {
        EARLY_DBGPR("Non numeric arg!\n");
        return -1;
    }
    retval=strtoul(arg,&chkptr,0);
    if(*chkptr!='\0')
    {
        EARLY_DBGPR("Non numeric arg!\n");
        return -1;
    }
    if(retval>upper || retval < lower)
    {
        EARLY_DBGPR("Value %lx not in allowed range!\n",retval);
        return -1;
    }
    *value=retval;
    return 0;

}
/* I should have written scan_string() in same fashion the scan_uint and scan_ip are done... */
static void ucs_from_cfgfile(FILE *cf,user_commands *uc)
{
     char *line;
    int rval;
    unsigned nline=0;
    rewind(cf);
    DEBUGPR("Searching editor name from cfg file\n");

    while(1)
    {
        if(1==(rval=fscanf(cf,"editor=%a[^\n]\n",&line)))
        {
            nline++;
            DEBUGPR("editor %s found from cfg file line %u\n",line,nline);
            if(!uc->editor)
                uc->editor = line;
            break;
        }
        else if(EOF==rval || rval < 0)
            break;
        else if(!(rval=fscanf(cf,"%*a[^\n]\n")))
        {
            nline++;
        }
        else
            break;
    }
}
static void udplog_from_cfgfile(FILE *cf,fileargs *farg)
{
    char *line;
    unsigned val;
    int rval;
    unsigned nline=0;
    rewind(cf);
    DEBUGPR("Searching udplog name from cfg file\n");

    while(1)
    {
        if(1==(rval=fscanf(cf,"udplog=%a[^\n]\n",&line)))
        {
            nline++;
            DEBUGPR("udplog %s found from cfg file line %u\n",line,nline);
            if(!farg->filebasename)
               farg->filebasename = line;
            continue;
        }
        else if(1 == (rval=fscanf(cf,"rotate=%u\n",&val)))
        {
            nline++;
            if(!farg->fileamnt)
                farg->fileamnt=val;
        }
        else if(1 == (rval=fscanf(cf,"filesize=%u\n",&val)))
        {
            nline++;
            if(!farg->filesize)
                farg->filesize=val;
        }
        else if(EOF==rval || rval < 0)
            break;
        else if(!(rval=fscanf(cf,"%*a[^\n]\n")))
        {
            nline++;
        }
        else
            break;
    }
    //return NULL;
}

static int filters_from_cfgfile(stringfilter *filters,FILE *cf)
{
    int rval;
    char *filter;
    unsigned nline=0;
    rewind(cf);
    DEBUGPR("Searching filters from cfg file\n");

    while(1)
    {
        if(1==(rval=fscanf(cf,"filter=%a[^\n]\n",&filter)))
        {
            nline++;
            DEBUGPR("Filter string %s found from cfg file line %u\n",filter,nline);
            if(filters->add(filters,filter))
            {
                DEBUGPR("Failed to add filter '%s' from cfgfile line %d\n",filter,nline);
                return -1;
            }
        }
        else if(EOF==rval)
            break;
        else if(!(rval=fscanf(cf,"%*a[^\n]\n")))
        {
            nline++;
        }
        else if(EOF==rval)
            break;
    }
    return 0;
}

uint32_t scan_ip(FILE *cf,char *scanfmt)
{
    char *line;
    int rval;
    rewind(cf);
    while(1)
    {

        if(1==(rval=fscanf(cf,scanfmt,&line)))
        {
            rval=inet_addr(line);
            free(line);
            break;
        }
        else if(EOF == rval || (rval=fscanf(cf,"%*a[^\n]\n")))
        {
            rval=0;
            break;
        }
    }
    return rval;
}
void configfile_getports(FILE *cf, unsigned short *fcmport, unsigned short *fspport)
{
    DEBUGPR("entering configfile_getport():, current fcm=0x%hx, fsp=0x%hx\n",*fcmport,*fspport);
    if(!cf || !fcmport || !fspport)
        return;
    if(!*fcmport)
    {
        rewind(cf);
        *fcmport = (unsigned short)scan_uint(cf,"fcmport=%a[^\n]\n");
    }
    if(!*fspport)
    {
        rewind(cf);
        *fspport = (unsigned short)scan_uint(cf,"fspport=%a[^\n]\n");
    }
    DEBUGPR("leaving configfile_getports():, current fcm=0x%hx, fsp=0x%hx\n",*fcmport,*fspport);

}

void configfile_getips(FILE *cf, uint32_t *fcmip, uint32_t *fspip)
{
    DEBUGPR("entering configfile_getips():, current fcm=0x%x, fsp=0x%x\n",*fcmip,*fspip);
    if(!cf || !fcmip || !fspip)
        return;
    if(!*fcmip)
        *fcmip = scan_ip(cf,"fcmip=%a[^\n]\n");
    if(!*fspip)
        *fspip = scan_ip(cf,"fspip=%a[^\n]\n");
    DEBUGPR("leaving configfile_getips():, current fcm=0x%x, fsp=0x%x\n",*fcmip,*fspip);

}

int main(int argc, char *argv[])
{	
    int c;
	int ch;
    int index;
    //pthread_t tid;
    sockstruct samant;
    stringfilter *filters;
    char *dbglogname=NULL;
    char *udplogname=NULL;
    //FILE *udplogfile=NULL;
    char *cfgfile=NULL;
    FILE *cf=NULL;
    //FILE **arg2;
    char filterstring[1025]={0};
    unsigned filterindex=0;

    sdisplayhandler *displayhandler;
    user_commands uc;
    printptrhandler *bhandler;
    udp_handler *udphandler;
    tgt_commander *commander;
    shitemsgparser *msgfileparser=NULL;
    smsgloader *msgloader;
    char *ptr;
    char *home=getenv("HOME");
    fileargs farg;
    memset(&farg,0,sizeof(farg));
    displayhandler=sdisplayhandler_init();
    if(!displayhandler)
    {
        EARLY_DBGPR("Failed to allocate displayhandler\n");
        return -1;
    }

    commander=init_tgt_commander();
    if(!commander)
    {
        EARLY_DBGPR("Failed to init tgt commander!\n");
    }
    
    memset(&uc,0,sizeof(uc));
    udphandler=init_udphandler();
    if(!udphandler)
    {
        EARLY_DBGPR("Failed to allocate udphandler\n");
        return -1;
    }


    G_exename=argv[0];
    while(-1 != (c = getopt_long(argc, argv, OPTSTRING,long_options,&index)))
    {
        switch(c)
        {
            case 'e':
            {
                uc.editor=optarg;
                break;
            }
            case 'c':
                if(optarg)
                    cfgfile=optarg;
                break;
            case 'd':
                if(optarg)
                    dbglogname=optarg;
                break;
            case 'p':
                /* port */
                if(optarg)
                {
                    unsigned long tmp;
                    if(argchk(optarg, 1, 0xffff,&tmp))
                    {
                        EARLY_DBGPR("invalid port argument (%s)\n",optarg);
                        return -1;
                    }
                    if(udphandler->add_port(udphandler,(unsigned short)tmp))
                        return -1;

                }
                break;
            case 'v':
                EARLY_DBGPR("%s version %s\n",argv[0],VERSION);
                return 0;
                break;
            case '?':
            case 'h':
                EARLY_DBGPR("%s version %s\n",argv[0],VERSION);
                print_usage();
                return 0;
                break;
            case 'f':
            {
                if(optarg)
                {
                    farg.filebasename=udplogname=optarg;
                }
            }
            break;
            case 'r':
            {
                unsigned long tmp;
                if(argchk(optarg,2,0xffffffff,&tmp))
                {
                    EARLY_DBGPR("Invalid arg -r %s\n",optarg);
                    return -1;
                }
                farg.fileamnt=(unsigned)tmp;
            }
            break;
            case 's':
            {
                unsigned long tmp;
                if(argchk(optarg,0,0xffffffff,&tmp))
                {
                    EARLY_DBGPR("Invalid arg -s %s\n",optarg);
                    return -1;
                }
                farg.filesize=(unsigned)tmp;
            }
            break;
            default:
                break;
        }
    }

    if(dbglogname)
        if(!(G_logfile=fopen(dbglogname,"w")))
        {
            EARLY_DBGPR("Failed to open logfile '%s' (%s)!\n",dbglogname,strerror(errno));
            return -1;
        }
    if(!cfgfile)
    {
        if(home)
        {
            ptr=calloc(1,strlen(home)+25);
            if(ptr)
                snprintf(ptr,strlen(home)+24,"%s%s",home,"/.nibbles/default.conf");
            cfgfile=ptr;
        /* Try seeing if we have default cfg file */
        }
    }
    if(cfgfile)
    {
        if(!(cf=fopen(cfgfile,"r")))
        {
            EARLY_DBGPR("Failed to open cfgfile '%s' (%s)!\n",cfgfile,strerror(errno));
        }
        else
            if(udphandler->read_portcfgfile(udphandler,cf))
                out(-1);
    }

    if(cf)
    {
        udplog_from_cfgfile(cf,&farg);
        ucs_from_cfgfile(cf,&uc);
        if(!uc.editor)
            uc.editor="gedit";
    }
    if(!farg.filebasename)
    {
        printf("nibbles ultrabra is file based version, -f <filename> or filename=<filename> at ~/.nibbles/default.conf is REQUIRED!\n");
        return -1;
    }
    if(farg.fileamnt || farg.filesize)
    {
        printf("nibbles ultrabra does not support -s<filesize> or -r<fileamnt> - sorry. If you NEED those, feel free to try standard nibbles\n");
        return -1;
    }
    strncpy(G_CURRENT_UDP_FILE_NAME,farg.filebasename,1023);
    bhandler=init_printptrhandler(farg.filebasename,0,0);
    if(!bhandler)
    {
        EARLY_DBGPR("Failed to allocate stuff!\n");
        return -1;
    }

    udphandler->prepare_printbuffer(udphandler,bhandler);     

        G_USE_FILE=2;
    if(udphandler->start_sockets(udphandler))
    {
        return -1;
    }
    if(home)
    {
        ptr=calloc(1,strlen(home)+ 23 /* /.nibbles/msgtemplates+'\0' */ );
        if(ptr)
        {
            sprintf(ptr,"%s/.nibbles/msgtemplates",home);
            msgfileparser = init_shitemsgparser(ptr);
        }
    }
    filters=filterinit(); 
    if(!filters)
    {
        DEBUGPR("Failed to init filters!\n");
        G_NOFILTERS=1;
    }

    signal(SIGSTOP,SIG_IGN);
    signal(SIGTERM,&out);
    signal(SIGINT,&out);
    
    
    DEBUGPR("Enabling ncurses mode...\n");
    displayhandler->ncursesmode_init(displayhandler);
    DEBUGPR("Initializing colours...\n");
    displayhandler->colors_init(displayhandler);
    DEBUGPR("Initializing windows...\n");
    if(displayhandler->windows_init(displayhandler))
    {
        DEBUGPR("Failed to create windows!\n");
        out(-1);
    }
    if(displayhandler->panels_init(displayhandler))
    {
        DEBUGPR("Failed to create panels to handle overlapping windows\n");
        out(-1);
    }
    msgloader=init_smsgloader(displayhandler->menuwin);
    if(!msgloader)
    {
        DEBUGPR("Failed to init msgloader!\n");
        out(-1);
    }
    if(msgfileparser)
        if(msgloader->msgloader_loaditems(msgloader,msgfileparser))
        {
            DEBUGPR("Something went wituiksi!\n");
            out(-1);
        }

    if(displayhandler->scomform_init(displayhandler))
    {
        DEBUGPR("Failed to create form for syscom msgs\n");
        out(-1);
    }

    if(pthread_create(&G_tid,NULL,&UDPreaderthread,  udphandler  ))
    {
        DEBUGPR("Failed to launch UDPlistener thread\n");
        out(0);
    }

    //DEBUGPR("Windows created - statuswin %p, filterwin %p, logwin %p,\n",statuswin,filter_win,logwin);
    filters->set_fstrwin(filters,displayhandler->fstrwin);
    filters->set_statsinfo(filters,displayhandler->statuswin,udphandler->get_portamnt(udphandler));
    wprintw(displayhandler->statuswin,STATUSWIN_PRINT_FMT,samant.sockamnt,0,0,0,"LIVEDISPLAY");
    displayhandler->set_late_properties(displayhandler);
    if(cf && filters_from_cfgfile(filters,cf))
        out(-1);
    if(cf)
        fclose(cf);

//    msgloader->(msgloader,displayhandler->menuwin);
    msgloader->display_menu(msgloader);
    while(!uc.end)
    {
        if(!uc.scroll_mode)
            ch=getch();
        else
            ch=0;
        handle_char(ch,filters,displayhandler,filterstring,&filterindex,&uc,commander,msgloader,msgfileparser,bhandler);
        display_udpdata(filters,displayhandler,bhandler,&uc);
//        if(!uc.scroll_mode && uc.pause<2)
//        {
//            uc.pause*=2;
        if(!uc.scroll_mode)
        {
    		update_panels();
            doupdate();
        }
//        }
    }
    out(0);
	return 0;
}


