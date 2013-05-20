
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

#ifndef STRINGFILTERS_H
#define STRINGFILTERS_H
#include <ncurses.h>
#include <stdio.h>

#define EXCL_FILTER_MAX_AMNT 200
#define INCL_FILTER_MAX_AMNT 200
#define HL_FILTER_MAX_AMNT 200
#define STATUSWIN_PRINT_FMT "Status: Listen %d\t\tFilters: Inc %d, Exc %d, Hl %d\t\t\t%s\n"

typedef struct stringfilter
{
    int paused;
    int includeamnt;
    int excludeamnt;
    int hlamnt;
    WINDOW *statwin;
    WINDOW *fstringwin;
    int portamnt;
#if 0 
    //calls to this struct all come from main thread
    pthread_mutex_t inclock;
    pthread_mutex_t exclock;
    pthread_mutex_t hlclock;
#endif
    unsigned inclens[INCL_FILTER_MAX_AMNT];
    unsigned exclens[EXCL_FILTER_MAX_AMNT];
    unsigned hllens[HL_FILTER_MAX_AMNT];
    char *includestrings[INCL_FILTER_MAX_AMNT];
    char *excludestrings[EXCL_FILTER_MAX_AMNT];
    char *hlstrings[HL_FILTER_MAX_AMNT];
    void (*set_paused)(struct stringfilter *,int paused);
    void (*set_statsinfo)(struct stringfilter *,WINDOW *,unsigned );
    int (*add)(struct stringfilter *_this, char *filterstr);
    int (*filter)(struct stringfilter *,char *,int);
    int (*hl)(struct stringfilter *,char *,int);
    void (*clearfilters)(struct stringfilter *);
    void (*set_fstrwin)(struct stringfilter *,WINDOW *);
}stringfilter;


stringfilter *filterinit();


#endif //STRINGFILTERS_H
