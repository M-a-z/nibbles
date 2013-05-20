
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

#include "displayhandler.h"
#include "common.h"
#include <stdlib.h>
#include <string.h>

typedef enum e_nibble_color
{
    e_nibble_color_normal   = 0,
    e_nibble_color_hl       = 1
}e_nibble_color;



static WINDOW *create_newwin(int height, int width, int starty, int startx);

static int windows_init(sdisplayhandler *_this)
{
    int sucks=0;
    _this->statuswin=create_newwin(1/*height*/, COLS-2/*width*/, 0/* y */, 0/*x*/ );
    _this->filter_win=create_newwin(1/*height*/, COLS-2/*width*/, 1/* y */, 0/*x*/ );
    _this->logwin=create_newwin(LINES-2/*height*/, COLS-2/*width*/, 2/* y */, 0/*x*/ );
//    _this->helpwin=create_newwin(LINES-6/*height*/, COLS-6/*width*/, 3/* y */, 3/*x*/ );
    _this->helpwin=create_newwin((LINES<RUNTIME_HELP_PRINT_LINES+4)?LINES:RUNTIME_HELP_PRINT_LINES+4/*height*/, (COLS<HELP_WIN_LEN)?COLS:HELP_WIN_LEN/*width*/, 3/* y */, 3/*x*/ );
	_this->syscomwin = create_newwin(LINES-3/*height*/, COLS-3/*width*/, 6/* y */, 0/*x*/ );
	_this->deffindwin = create_newwin(LINES-5/*height*/, COLS-3/*width*/, 5/* y */, 1/*x*/ );
    _this->fstrwin = create_newwin(LINES-4, COLS-4, 2,2);
	_this->menuwin = create_newwin(LINES-4/*height*/, COLS-4/*width*/, 4/* y */, 4/*x*/ );
    sucks=(int)(!_this->statuswin || !_this->filter_win || !_this->logwin || ! _this->helpwin || !_this->syscomwin || !_this->deffindwin || !_this->fstrwin || !_this->menuwin);
    DEBUGPR("windows initialized (succescode %d)...\n",sucks);
    if(!sucks)
    {
        box(_this->helpwin,0,0);
        wprintw(_this->helpwin,"(%s)",HELPWIN_TOGGLE_STR);
    }
    return sucks;
}
static void magic(sdisplayhandler *_this)
{
    wnoutrefresh(_this->helpwin);
    wnoutrefresh(_this->syscomwin);
    wnoutrefresh(_this->deffindwin);
    wnoutrefresh(_this->fstrwin);
    wnoutrefresh(_this->menuwin);
    wnoutrefresh(_this->filter_win);
    wnoutrefresh(_this->statuswin);
    wnoutrefresh(_this->logwin);
    doupdate();
}
static void ncursesmode_init(sdisplayhandler *_this)
{
	initscr();			/* Start curses mode 		*/
	cbreak();			/* Line buffering disabled, Pass on
					 * everty thing to me 		*/
    timeout(10);
   // halfdelay(1);
	keypad(stdscr, TRUE);		/* I need that nifty F1 	*/
    noecho();
    DEBUGPR("Ncursesmode inited\n");
}
static void colors_init(sdisplayhandler *_this)
{
    start_color();
    init_pair( e_nibble_color_hl, HL_TXT_COLOR, HL_BG_COLOR );
    use_default_colors();
    DEBUGPR("Colours inited\n");
}
static void handle_char(sdisplayhandler *_this, int ch)
{
    if(1==_this->in_syscommode(_this))
    {
        DEBUGPR("displayhandler passing char %d to form engine (syscommode %d)\n",ch,_this->hidden[EWinId_Syscom]);
        _this->formhandler->form_chr(_this->formhandler,ch);
    }
    else
    {
        _this->deffinder->handle_input(_this->deffinder,ch);
    }
}
static int scomform_init(sdisplayhandler *_this)
{
    if(!(_this->deffinder=init_definitionfinder(_this->deffindwin)))
    {
        DEBUGPR("Failed to create definitionfinder");
        return -1;
    }
	if(!(_this->formhandler=init_form(_this->syscomwin)))
	{
		DEBUGPR("Failed to create syscomform!\n");
        return -1;
	}
    _this->deffinder->create_form(_this->deffinder);
    _this->deffinder->add_form_values(_this->deffinder);
    _this->formhandler->display_form(_this->formhandler);
    _this->deffinder->display_form(_this->deffinder);
    return 0;
}
static int panels_init(sdisplayhandler *_this)
{
    //int i=0;
/*
    for(i=0;i<EWinId_Last;i++)
    {
        if(!(_this->panels[i]=new_panel((WINDOW *)(((char *)_this)+offsetof(sdisplayhandler,filter_win)+i*sizeof(WINDOW *)))))
        {
            DEBUGPR("Failed to init panels!\n");
            return -1;
        }
        set_panel_userptr(_this->panels[i],&_this->hidden[i]);
    }
*/
    _this->panels[EWinId_Deffind]=new_panel(_this->deffindwin);
	_this->panels[EWinId_Syscom]=new_panel(_this->syscomwin);
	_this->panels[EWinId_Help]=new_panel(_this->helpwin);
	_this->panels[EWinId_Log]=new_panel(_this->logwin);
	_this->panels[EWinId_Filter]=new_panel(_this->filter_win);
	_this->panels[EWinId_Status]=new_panel(_this->statuswin);
	_this->panels[EWinId_Fstr]=new_panel(_this->fstrwin);
	_this->panels[EWinId_Menu]=new_panel(_this->menuwin);

    hide_panel(_this->panels[EWinId_Menu]);
    hide_panel(_this->panels[EWinId_Syscom]);
    hide_panel(_this->panels[EWinId_Help]);
    hide_panel(_this->panels[EWinId_Fstr]);
   // _this->hidden[EWinId_Fstr]=1;

    set_panel_userptr(_this->panels[EWinId_Menu],&_this->hidden[EWinId_Menu]);
    set_panel_userptr(_this->panels[EWinId_Deffind],&_this->hidden[EWinId_Deffind]);
	set_panel_userptr(_this->panels[EWinId_Syscom],&_this->hidden[EWinId_Syscom]);
	set_panel_userptr(_this->panels[EWinId_Help],&_this->hidden[EWinId_Help]);
	set_panel_userptr(_this->panels[EWinId_Log],&_this->hidden[EWinId_Log]);
	set_panel_userptr(_this->panels[EWinId_Filter],&_this->hidden[EWinId_Filter]);
	set_panel_userptr(_this->panels[EWinId_Status],&_this->hidden[EWinId_Status]);
	set_panel_userptr(_this->panels[EWinId_Fstr],&_this->hidden[EWinId_Fstr]);

    return /* (!_this->panels[EWinId_Syscom] || !_this->panels[EWinId_Help] || !_this->panels[EWinId_Log] || !_this->panels[EWinId_Filter]|| !_this->panels[EWinId_Status] || !_this->panels[EWinId_Deffind] || !_this->panels[EWinId_Fstr] ) */ 0; 
}

static void hl_begin(WINDOW *win)
{
    wattrset( win, COLOR_PAIR(e_nibble_color_hl) );
}
static void log_start_hl(sdisplayhandler *_this)
{
    hl_begin(_this->logwin);
}
static void hl_end(WINDOW *win)
{
    wattrset( win, COLOR_PAIR(e_nibble_color_normal) );
}
static void log_end_hl(sdisplayhandler *_this)
{
    hl_end(_this->logwin);
}

static int get_visibility(PANEL *panel)
{
    if(*(int *)panel_userptr(panel))
    {
        if(panel == panel_below(NULL))
            return 1;
        else
            return 2;
    }
    return 0;
}
static int scommenuontop(sdisplayhandler *_this)
{
    return (1&get_visibility(_this->panels[EWinId_Menu]));
}
static int in_deffindmode(sdisplayhandler *_this)
{
    return get_visibility(_this->panels[EWinId_Deffind]);
}

static int in_syscommode(sdisplayhandler *_this)
{
    return get_visibility(_this->panels[EWinId_Syscom]);
}
static void toggle_panel_visibility(PANEL *panel)
{
    int *show;
    if(*(show=(int *)panel_userptr(panel)))
        hide_panel(panel);
    else
    {
        top_panel(panel);
        show_panel(panel);
    }
    *show=(!*show);
}
static void toggle_scommenu(sdisplayhandler *_this)
{
    toggle_panel_visibility(_this->panels[EWinId_Menu]);
}

static void toggle_deffind(sdisplayhandler *_this)
{
    _this->deffinder->display_form(_this->deffinder);
    toggle_panel_visibility(_this->panels[EWinId_Deffind]);
}
static void toggle_fstr(sdisplayhandler *_this)
{
    toggle_panel_visibility(_this->panels[EWinId_Fstr]);
}
static void toggle_help(sdisplayhandler *_this)
{
    toggle_panel_visibility(_this->panels[EWinId_Help]);
}
static void toggle_scom(sdisplayhandler *_this)
{
    _this->formhandler->display_form(_this->formhandler);
    toggle_panel_visibility(_this->panels[EWinId_Syscom]);
}
static void write_help(WINDOW *hw)
{
    wprintw(hw,RUNTIME_HELP_PRINT);
}
static void *get_built_msg(sdisplayhandler *_this, uint32_t *msglen, char **ifname)
{
    int msgtype;
    return _this->formhandler->get_msg_from_form(_this->formhandler,msglen,ifname,&msgtype);
}
static void set_late_properties(sdisplayhandler *_this)
{
    int y,x,y2,x2;

    getbegyx(_this->helpwin,y,x);
    getmaxyx(_this->helpwin,y2,x2);
    scrollok(_this->logwin,TRUE);
//    write_help(_this->helpwin);
    write_help(derwin(_this->helpwin,y2-y,x2-x,2,2));
}
static void clearlog(sdisplayhandler *_this)
{
    werase(_this->logwin);
}

sdisplayhandler * sdisplayhandler_init()
{
    sdisplayhandler *_this;
    _this=calloc(1,sizeof(sdisplayhandler));
    if(_this)
    {
        memset(_this,0,sizeof(sdisplayhandler));
        _this->scommenuontop=&scommenuontop;
        _this->clearlog=&clearlog;
        _this->in_deffindmode=&in_deffindmode;
        _this->toggle_scommenu=&toggle_scommenu;
        _this->toggle_deffind=&toggle_deffind;
        _this->toggle_fstr=&toggle_fstr;
        _this->log_end_hl=&log_end_hl;
        _this->log_start_hl=&log_start_hl;
        _this->handle_char=&handle_char;
        _this->get_built_msg=&get_built_msg;
        _this->in_syscommode=&in_syscommode;
        _this->toggle_help=&toggle_help;
        _this->toggle_scom=&toggle_scom;
        _this->set_late_properties=&set_late_properties;
        _this->windows_init=&windows_init;
        _this->ncursesmode_init=&ncursesmode_init;
        _this->colors_init=&colors_init;
        _this->scomform_init=&scomform_init;
        _this->panels_init=&panels_init;
        _this->magic=&magic;
    }
    return _this;
}
static WINDOW *create_newwin(int height, int width, int starty, int startx)
{	WINDOW *local_win;
    local_win = newwin(height, width, starty, startx);
	return local_win;
}

