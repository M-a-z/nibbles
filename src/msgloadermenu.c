
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

#include <menu.h>
#include <string.h>
#include "msgloadermenu.h"
#include <stdlib.h>
#include <unistd.h>

void handle_char(smsgloader *_this,int ch)
{
    if(_this->loadmenu)
        switch(ch)
        {
            case KEY_DOWN:
                menu_driver(_this->loadmenu, REQ_DOWN_ITEM);
                break;
            case KEY_UP:
                menu_driver(_this->loadmenu, REQ_UP_ITEM);
                break;
            case KEY_LEFT:
                menu_driver(_this->loadmenu, REQ_LEFT_ITEM);
                break;
            case KEY_RIGHT:
                menu_driver(_this->loadmenu, REQ_RIGHT_ITEM);
                break;
            case KEY_NPAGE:
                menu_driver(_this->loadmenu, REQ_SCR_DPAGE);
                break;
            case KEY_PPAGE:
                menu_driver(_this->loadmenu, REQ_SCR_UPAGE);
                break;
        }
}

/* glue items to menu and post menu */
static void display_menu(smsgloader *_this)
{
    int rows,cols;
    VERBOSE_DEBUGPR("displaying scom menu requested\n");
    if(_this->loadmenu)
    {
        DEBUGPR("Error!!!!! loadmenu already dsiplayed??\n");
        return;
    }
    _this->loadmenu=new_menu(_this->loaditems);
    scale_menu(_this->loadmenu,&rows,&cols);
    wresize(_this->msgloaderwin,rows+5,cols+5);
    box(_this->msgloaderwin,0,0);
    wprintw(_this->msgloaderwin,"(%s)",SCOMMENU_TOGGLE_STR);
    set_menu_win(_this->loadmenu,_this->msgloaderwin);
    _this->derwin=derwin(_this->msgloaderwin,rows,cols,2,2);
    set_menu_sub(_this->loadmenu,_this->derwin);
    set_menu_mark(_this->loadmenu," * ");
    post_menu(_this->loadmenu);
}
/* unpost menu and unglue items allowing them to be freed / changed in sp */
static void release_menu(smsgloader *_this)
{
    int i;
    VERBOSE_DEBUGPR("releasing scom menu requested\n");
    if(!_this->loadmenu)
    {
        DEBUGPR("Trying to free menu which is not allocated!\n");
        return;
    }
    unpost_menu(_this->loadmenu);
    for(i=0;i<_this->itemamnt;i++)
        free_item(_this->loaditems[i]);
    free(_this->loaditems);
    _this->loaditems=NULL;
    free_menu(_this->loadmenu);
    _this->loadmenu=NULL;
    if(_this->derwin)
        delwin(_this->derwin);
}
static void *get_selected_hidden(smsgloader *_this)
{

    VERBOSE_DEBUGPR("scommenu - retrieving msglistitemhandle %p for msg item\n",item_userptr(current_item(_this->loadmenu)));
    return item_userptr(current_item(_this->loadmenu));
}
/* Load items from sp struct to menu */
static int msgloader_loaditems(smsgloader *_this,shitemsgparser *sp)
{
    int i;
    VERBOSE_DEBUGPR("scom menu loading items\n");
    /* Release and unpost menu before trying to load new items to sp */
    if(_this->loaditems)
        _this->release_menu(_this);

    if(sp->load_msgs(sp))
    {
        char *nomsgs;
        DEBUGPR("Failed to load messages (no message file in ./nibbles ?)!\n");
        nomsgs=malloc(20);
        strcpy(nomsgs,"no messages");
        if(!nomsgs)
        {
            DEBUGPR("ALLOC FAILED! %s:%d\n",__FILE__,__LINE__);
            return -1;
        }
        _this->loaditems=calloc(2,sizeof(ITEM *));
        if(!_this->loaditems)
        {
            free(nomsgs);
            DEBUGPR("ALLOC FAILED! %s:%d\n",__FILE__,__LINE__);
            return -1;
        }
        _this->loaditems[0]=new_item(nomsgs,nomsgs);
        if(!_this->loaditems[0])
        {
            free(_this->loaditems);
            _this->loaditems=NULL;
            DEBUGPR("ALLOC FAILED! %s:%d\n",__FILE__,__LINE__);
            return -1;
        }
        _this->loaditems[1]=NULL;
        _this->itemamnt=1;
    }
    else
    {
        /* This forms a tight binding between data hold by sp, and data in menu... It is nasty I know, but on the other hand, I do not want to allocate space for dublicate information.. So lets just try to invent a way to sync menu showing&access and message data updates... */
        void *msglistitemhandle;
        int itemamnt=sp->loaded_msg_amnt(sp);
        char *msgname;
        VERBOSE_DEBUGPR("scommenu - messages loaded to sp\n");
        _this->loaditems=calloc(itemamnt+1,sizeof(ITEM *));
        if(!_this->loaditems)
        {
            DEBUGPR("Calloc FAILED! %s:%d\n",__FILE__,__LINE__);
            return -1;
        }
        _this->itemhiddens=calloc(itemamnt,sizeof(void *));
        if(!_this->itemhiddens)
        {
            free(_this->loaditems);
            DEBUGPR("Calloc FAILED! %s:%d\n",__FILE__,__LINE__);
            return -1;
        }
        msglistitemhandle=sp->get_first_msgitem(sp);
        msgname=sp->get_matching_name(sp,msglistitemhandle);
        for(i=0;i<itemamnt && (msgname=sp->get_matching_name(sp,msglistitemhandle));i++,msglistitemhandle=sp->get_next_msgitem(sp,msglistitemhandle))
        {
            VERBOSE_DEBUGPR("scommenu - creating msg item %d for msg '%s'\n",i,msgname);
            _this->loaditems[i]=new_item(msgname,sp->get_matching_desc(sp,msglistitemhandle));
            VERBOSE_DEBUGPR("scommenu - storing msglistitemhandle %p for msg item\n",msglistitemhandle);
            set_item_userptr(_this->loaditems[i],msglistitemhandle);
        }
        _this->loaditems[i]=NULL;
        if(i<itemamnt)
        {
            DEBUGPR("Odd,Item amount (%d) and amount of found message names (%d) not same!\n",itemamnt,i);
        }
        _this->itemamnt=i;
        VERBOSE_DEBUGPR("scommenu - FOUND %d messages!\n",_this->itemamnt);
    }
    return 0;
}

smsgloader *init_smsgloader(WINDOW *menuwin)
{
    smsgloader *_this=calloc(1,sizeof(smsgloader));
    if(_this)
    {
        _this->get_selected_hidden=&get_selected_hidden;
        _this->handle_char=&handle_char;
        _this->msgloaderwin=menuwin;
        _this->msgloader_loaditems=&msgloader_loaditems;
        _this->display_menu=&display_menu;
        _this->release_menu=&release_menu;
    }
    return _this;
}
