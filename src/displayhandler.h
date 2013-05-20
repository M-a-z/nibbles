
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

#ifndef DISPLAYHANDLER_H
#define DISPLAYHANDLER_H
#include <ncurses.h>
#include <form.h>
#include <panel.h>
//        syscomform.
#include "syscomform.h"
#include "definitionfinder.h"



#define NORMAL_BG_COLOR COLOR_WHITE
#define NORMAL_TXT_COLOR COLOR_BLACK

#define HL_BG_COLOR COLOR_YELLOW
#define HL_TXT_COLOR COLOR_CYAN

typedef enum EWinId
{
    EWinId_Filter = 0,
    EWinId_Status ,
    EWinId_Log ,
    EWinId_Help ,
    EWinId_Syscom ,
    EWinId_Deffind,
    EWinId_Fstr,
    EWinId_Menu,
    EWinId_Last
}EWinId;


typedef struct sdisplayhandler
{
    definitionfinder *deffinder;
    formal_msg *formhandler;
    int hidden[EWinId_Last];
    WINDOW *filter_win;
    WINDOW *logwin;
    WINDOW *statuswin;
    WINDOW *helpwin;
    WINDOW *syscomwin;
    WINDOW *deffindwin;
    WINDOW *fstrwin;
    WINDOW *menuwin;
    PANEL *panels[EWinId_Last];
    void (*magic)(struct sdisplayhandler *);
    int (*scommenuontop)(struct sdisplayhandler *);
    void (*ncursesmode_init)(struct sdisplayhandler *); ///< 1st step to do after preparing disphandler
    void (*colors_init)(struct sdisplayhandler *); ///< 2nd step to do after preparing disphandler
    int (*windows_init)(struct sdisplayhandler *);   ///< 3rd step to do after preparing disphandler
    int (*scomform_init)(struct sdisplayhandler *); ///< 4th step to do after preparing disphandler
    int (*panels_init)(struct sdisplayhandler *); ///< 5th step to do after preparing disphandler
    void (*set_late_properties)(struct sdisplayhandler *); 
    void (*toggle_help)(struct sdisplayhandler *);
    void (*toggle_scom)(struct sdisplayhandler *);
    void (*toggle_fstr)(struct sdisplayhandler *);
    void (*toggle_deffind)(struct sdisplayhandler *);
    void (*toggle_scommenu)(struct sdisplayhandler *);
    int (*in_deffindmode)(struct sdisplayhandler *);
    int (*in_syscommode)(struct sdisplayhandler *);
    void *(*get_built_msg)(struct sdisplayhandler *,uint32_t *,char **);
    void (*handle_char)(struct sdisplayhandler *,int);
    void (*log_end_hl)(struct sdisplayhandler *);
    void (*log_start_hl)(struct sdisplayhandler *);
    void (*clearlog)(struct sdisplayhandler *);
}sdisplayhandler;

sdisplayhandler * sdisplayhandler_init();



#endif

