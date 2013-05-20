
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

#include "common.h"
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

extern pthread_mutex_t G_condmutex;
extern pthread_cond_t G_cond;
extern pthread_t G_tid;
int G_cancelled=0;
extern int G_USE_FILE;
extern int G_filewriter_running;
extern int G_bs1filewrneeded;
extern int G_bs2filewrneeded;
extern int G_bs1_wrsize;
extern int G_bs2_wrsize;

int get_mac(char *filter,uint8_t *mac)
{
    int rval;
    unsigned m0,m1,m2,m3,m4,m5;
    short i=1;
    char *pi=(char *)&i;
    if(!filter )
    {
        printf("NULL mac at %s:%d\n",__FILE__,__LINE__);
        return -1;
    }
    if(!*pi)
        rval=sscanf(filter,"%x:%x:%x:%x:%x:%x",&m5,&m4,&m3,&m2,&m1,&m0 );
    else
        rval=sscanf(filter,"%x:%x:%x:%x:%x:%x",&m0,&m1,&m2,&m3,&m4,&m5 );
    if(6!=rval || 255<m0 || 255<m1 || 255<m2 || 255<m3 || 255<m4 || 255<m5)
    {
        printf("Invalid strip mac '%s' given\n",filter);
        return EXIT_FAILURE;
    }
    mac[0]=(unsigned char)m0;
    mac[1]=(unsigned char)m1;
    mac[2]=(unsigned char)m2;
    mac[3]=(unsigned char)m3;
    mac[4]=(unsigned char)m4;
    mac[5]=(unsigned char)m5;
    
    return 0;
}


uint32_t scan_uint(FILE *cf,char *scanfmt)
{
    char *line;
    int rval;
    while(1)
    {

        if(1==(rval=fscanf(cf,scanfmt,&line)))
        {
            rval=atoi(line);
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
void out(int sig)
{
    /* This is not atomic. */
    if(G_logfile)
    {
        fflush(G_logfile);
        fclose(G_logfile);
    }
    endwin();
    printf("Got sig %d\n",sig);
    if(2==G_USE_FILE)
    {
        if(G_tid)
        {
            printf("Cancelling!\n");
            pthread_cancel(G_tid);
            while(!G_cancelled)
                sleep(1);
        }
    }
    printf("Thanks for watching N.I.B.B.L.E.S version %s\n",VERSION);
    fflush(stdout);
    if(SIGINT==sig || SIGTERM == sig)
    {
        signal(sig,SIG_DFL);
        kill(getpid(),sig);
    }
    else
        exit(sig);
}
/*
void schedule_filewrite(int filewrite,int bs)
{
    int dosignal=1;
    if(G_filewriter_running)
    {
        pthread_mutex_lock(&G_condmutex);
        switch(bs)
        {
            case 1:
                G_bs1_wrsize=filewrite;
                if(G_bs1filewrneeded)
                    dosignal=0;
                G_bs1filewrneeded=1;
                break;
            case 2:
                G_bs2_wrsize=filewrite;
                if(G_bs2filewrneeded)
                    dosignal=0;
                G_bs2filewrneeded=1;
                break;
            default:
                    dosignal=0;
                break;
        }
        if(dosignal)
            pthread_cond_signal(&G_cond);
        pthread_mutex_unlock(&G_condmutex);
    }
}
*/


