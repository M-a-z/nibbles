
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
#include "stringfilters.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cexplode.h"

static void set_paused(stringfilter *_this,int paused);
static void print_statsinfo(stringfilter *_this);

static inline int do_we_have_match(size_t filterlen, char *filter, size_t txtlen, char *txt)
{
    int i;
    int j;
    if(!filterlen || !txtlen)
        return 0;
    for(i=0;txtlen-i > filterlen; i++)
    {
        for(j=0;j<filterlen;j++)
        {
            if(filter[j]!=txt[i+j])
                break;
        }
        if(j==filterlen)
            return 1;
    }
    return 0;
}

int do_hl(stringfilter *_this,char *txt, int txtlen)
{
    int i;
    for(i=0;i<_this->hlamnt;i++)
        if
        (
            do_we_have_match
            (
                _this->hllens[i], 
                _this->hlstrings[i], 
                txtlen, 
                txt
            )
        )
            return 1;
    return 0;
}
static void set_fstrwin(stringfilter *_this,WINDOW *fstr)
{
    int x,x1,y,y1;
    getbegyx(fstr,y,x);
    getmaxyx(fstr,y1,x1);
    _this->fstringwin=derwin(fstr,y1-y-4,x1-x-4,2,2);
    box(fstr,0,0);
    wprintw(fstr,"(%s)",FSTR_TOGGLE_STR);
}
void set_statsinfo(stringfilter *_this,WINDOW *statuswin,unsigned ports)
{
    _this->statwin=statuswin;
    _this->portamnt=ports;
}
static void set_paused(stringfilter *_this,int paused)
{
    _this->paused=paused;
    print_statsinfo(_this);
}
static void print_statsinfo(stringfilter *_this)
{
    wmove(_this->statwin,0,0);
    //wclear(_this->statwin);
    werase(_this->statwin);
    wprintw(_this->statwin,STATUSWIN_PRINT_FMT,_this->portamnt,_this->includeamnt,_this->excludeamnt,_this->hlamnt,(_this->paused)?"PAUSED":"LIVEDISPLAY");
}
static int do_filter(stringfilter *_this,char *txt, int txtlen)
{
    int i;
    int found;

    found = !_this->includeamnt;
    for(i=0;i<_this->includeamnt && !found;i++)
    {
        /* Include only if match found */
        found=do_we_have_match(_this->inclens[i], _this->includestrings[i], txtlen, txt);
    }
    if(!found)
        return 1;
    /* This far looks like string should be printed, see if exclude rule matches */
    for(i=0;i<_this->excludeamnt;i++)
    {
        if(do_we_have_match(_this->exclens[i], _this->excludestrings[i], txtlen, txt))
            return 1;
    }
    /* string passed filters -> return 0 */
    return 0;
}
static void clearfilters(stringfilter *_this)
{
    //int i;
    /*
    int includeamnt;
    int excludeamnt;
    pthread_mutex_t inclock;
    pthread_mutex_t exclock;
    unsigned inclens[INCL_FILTER_MAX_AMNT];
    unsigned exclens[EXCL_FILTER_MAX_AMNT];
    char *includestrings[INCL_FILTER_MAX_AMNT];
    char *excludestrings[EXCL_FILTER_MAX_AMNT];
    int (*add)(struct stringfilter *_this, char *filterstr);
    int (*filter)(struct stringfilter *,char *,char*);
    void (*clearfilters)(struct stringfilter *)
    */
    if(!_this)
        return;
    if(_this->includeamnt)
    {
        memset(_this->inclens,0,_this->includeamnt*sizeof(unsigned));
        memset(_this->includestrings,0,_this->includeamnt*sizeof(char *));
    }
    if(_this->excludeamnt)
    {
        memset(_this->exclens,0,_this->excludeamnt*sizeof(unsigned));
        memset(_this->excludestrings,0,_this->excludeamnt*sizeof(char *));
    }
    if(_this->hlamnt)
    {
        memset(_this->hllens,0,_this->hlamnt*sizeof(unsigned));
        memset(_this->hlstrings,0,_this->hlamnt*sizeof(char *));
    }
    _this->includeamnt=_this->excludeamnt=_this->hlamnt=0;
#if 0
    for(i=0;i<EXCL_FILTER_MAX_AMNT;i++)
    {
        _this->exclens[i]=0;
        /* I know we leak here, but due to the lazy trim in add_filter, we lose original pointer and cannot reliably free() pieces.. */
        /* TODO: leave as FIXME for someone who is troubled with this =) */
        _this->excludestrings[i]=NULL;
    }
    for(i=0;i<INCL_FILTER_MAX_AMNT;i++)
    {
        _this->includestrings[i]=NULL;
        _this->inclens[i]=0;
    }
#endif
}
static int add_filter(stringfilter *_this,char *filterstr)
{
    int i;
    int filters=0;
    int newincsamnt=0;
    int newexcsamnt=0;
    int newhlamnt=0;
    char *newincs[INCL_FILTER_MAX_AMNT]={NULL,NULL};
    char *newexcs[EXCL_FILTER_MAX_AMNT]={NULL,NULL};
    char *newhls[HL_FILTER_MAX_AMNT]={NULL,NULL};
    CexplodeStrings ploder;
    char *tmp;
    if(!filterstr || !_this)
        return -1;
    filters=Cexplode(filterstr, ",", &ploder );
    if(filters<1)
        return -1;
    while((tmp=Cexplode_getnext(&ploder)))
    {
        /* TODO: FIXME this "lazy trim" makes freeing filters impossible (we lose original pointer...)*/
        while(' '==*tmp && '\0' != *tmp)
        {
            *tmp='\0';
            tmp++;
        }
        switch(*tmp)
        {
            case '\0':
                break;
            case '+':
                if(*(tmp+1))
                {
                    newincs[newincsamnt]=tmp+1;
                    newincsamnt++;
                }
                break;
            case '-':
                if(*(tmp+1))
                {
                    newexcs[newexcsamnt]=tmp+1;
                    newexcsamnt++;
                }
                break;
            case '!':
                if(*(tmp+1))
                {
                    newhls[newhlamnt]=tmp+1;
                    newhlamnt++;
                }
                break;

            case 'c':
                if(!strcmp("clear",tmp))
                {
                    _this->clearfilters(_this);
                    if(_this->fstringwin)
                        werase(_this->fstringwin);
                    print_statsinfo(_this);
                    return 0;
                }
            default:
                Cexplode_free(ploder);
                return -1;
        }
    }
    if(_this->includeamnt<INCL_FILTER_MAX_AMNT)
    {
//        pthread_mutex_lock(&_this->inclock);
        for(i=0;i<newincsamnt;i++)
        {
            if(_this->includeamnt+1<INCL_FILTER_MAX_AMNT)
            {
                _this->includestrings[_this->includeamnt]=newincs[i];
                _this->inclens[_this->includeamnt]=strlen(newincs[i]);
                _this->includeamnt++;
                if(_this->fstringwin)
                    wprintw(_this->fstringwin,"INC\t '%s'\n",newincs[i]);
            }
            else
                break;
        }
//        pthread_mutex_unlock(&_this->inclock);
    }
    if(_this->excludeamnt<EXCL_FILTER_MAX_AMNT)
    {
//        pthread_mutex_lock(&_this->exclock);
        for(i=0;i<newexcsamnt;i++)
        {
            if(_this->excludeamnt+1<EXCL_FILTER_MAX_AMNT)
            {
                _this->excludestrings[_this->excludeamnt]=newexcs[i];
                _this->exclens[_this->excludeamnt]=strlen(newexcs[i]);
                _this->excludeamnt++;
                if(_this->fstringwin)
                    wprintw(_this->fstringwin,"EXC\t '%s'\n",newexcs[i]);
            }
            else
                break;
        }
//        pthread_mutex_unlock(&_this->exclock);
    }
    if(_this->hlamnt<HL_FILTER_MAX_AMNT)
    {
//        pthread_mutex_lock(&_this->exclock);
        for(i=0;i<newhlamnt;i++)
        {
            if(_this->hlamnt+1<HL_FILTER_MAX_AMNT)
            {
                _this->hlstrings[_this->hlamnt]=newhls[i];
                _this->hllens[_this->hlamnt]=strlen(newhls[i]);
                _this->hlamnt++;
                if(_this->fstringwin)
                    wprintw(_this->fstringwin,"HIL\t '%s'\n",newhls[i]);
            }
            else
                break;
        }
//        pthread_mutex_unlock(&_this->exclock);
    }
    print_statsinfo(_this);

    return 0;
/*
    int includeamnt;
    int excludeamnt;
    unsigned inclens[INCL_FILTER_MAX_AMNT];
    unsigned exclens[EXCL_FILTER_MAX_AMNT];
    char *includestrings[INCL_FILTER_MAX_AMNT];
    char *excludestrings[EXCL_FILTER_MAX_AMNT];
    int (*add)(struct stringfilter *_this, char *filterstr);
    int (*filter)(stringfilter *,char *,char*);
*/
        
}

stringfilter *filterinit()
{
    stringfilter *_this;
    _this=calloc(1,sizeof(stringfilter));
    if(!_this)
        return NULL;
/* Calls to filter + filter add are all done from main threads context => no need to sync */
//    pthread_mutex_init(&_this->inclock,NULL);
//    pthread_mutex_init(&_this->exclock,NULL);
    _this->set_fstrwin=&set_fstrwin;
    _this->add=&add_filter;
    _this->filter=&do_filter;
    _this->hl=&do_hl;
    _this->clearfilters=&clearfilters;
    _this->set_statsinfo=&set_statsinfo;
    _this->set_paused=&set_paused;

    return _this;
}

