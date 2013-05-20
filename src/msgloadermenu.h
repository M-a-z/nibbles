
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

#ifndef MSGLOADERMENU_H
#define MSGLOADERMENU_H

#include <ncurses.h>
#include <menu.h>
#include "shitemsgparser.h"


typedef struct smsgloader
{
    int state;
    WINDOW *msgloaderwin;
    WINDOW *derwin;
    ITEM   **loaditems;
    void   **itemhiddens;
    MENU   *loadmenu;
    int itemamnt;
    //shitemsgparser *sp;
    int (*msgloader_loaditems)(struct smsgloader *,shitemsgparser *);
    void (*display_menu)(struct smsgloader *_this);
    void (*release_menu)(struct smsgloader *_this);
    void (*handle_char)(struct smsgloader *,int);
    void *(*get_selected_hidden)(struct smsgloader *);
}smsgloader;

smsgloader *init_smsgloader(WINDOW *);

#endif
