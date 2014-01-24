
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

#ifndef NIBBLE_COMMIN_H
#define NIBBLE_COMMIN_H

#include <stdio.h>
#include <ncurses.h>
#include <inttypes.h>

//#define debug

#define ALIGN_SIZE 4
#define VERSION "\n0.3 - RC ultrabra   \n (? Y ?)\n  )   ("
#define OPTSTRING "d:e:p:f:c:r:s:vh?"
#define EARLY_DBGPR printf
/* /altGR + s */
#define SCROLLCHAR (159)
/* ctrl + p */
#define PAUSECHAR (16)
#define HELPWIN_TOGGLE_STR  "F2"
#define HELPCHAR            (KEY_F(2))
#define SYSCOM_TOGGLE_STR   "F3"
#define FORM_TOGGLE_CHAR    (KEY_F(3))
#define DEFFIND_TOGGLE_STR  "F4"
#define DEFFIND_TOGGLE_CHAR (KEY_F(4))
#define FSTR_TOGGLE_STR     "F5"
#define FSTR_TOGGLE_CHAR    (KEY_F(5))
#define SCOMMENU_TOGGLE_STR "F6"
#define SCOMMENU_TOGGLE_CHAR    (KEY_F(6))
/* ESC */
#define ENDCHAR (0x1B) // ESC
/* ctrl+b */
#define FORM_SEND_FCM_CHAR (2)

#define COMMAND_STARTCHAR '*'

#define HELP_PRINT \
"Args:\n"\
"     NOTE: IPs must be given in basic dotted notation\n\n" \
"    -d --debug             write debug log in file specified as parameter\n" \
"    -p --port              listen port given as parameter. You can give multiple -p args\n" \
"    -c --config            use file given as parameter to get default configurations\n" \
"    -f --file              write prints in file given as parameter\n" \
"    -r --rotate            specify how many files to keep if -f and -s is given\n" \
"    -s --size              specify how many kilobytes to write in one file (inaccurate!) if -f is given\n" \
"    -v --version           display version and exit\n" \
"    -h --help              to get this help\n" \
"    -e --editor            editor which is used to open log\n" \
"    -?                     to get this help\n\n" \
"    Ncurses Inspired Buggy Basic Log Extractor Shell - N.I.B.B.L.E.S is used to listen and filter UDP prints\n" \
"    Nibbles can also be used to find definitions && send and build syscom messages.\n" \
"    You can add 'exclude' 'include' and 'highlight' filters at runtime, by typing -<excludethis1>,-<excludethis2>,+<includethis1>,!<highlightthis> and pressing enter. (or -<excludethis1> followed by enter, -<excludethis2> followed by enter and so on). N.I.B.B.L.E will then scan incoming UDP prints for filters, and display prints as follows:\n" \
"    If string in print line matches exclude filter, it will not be displayed.\n" \
"    If include filter(s) are set, and no string in line matches any include filters, line is not displayed\n" \
"    Othervice line is displayed.\n" \
"    If string in print line passes include and exlude filters (is displayed), and if it matches highlight filter, then line is printed and coloured.\n" \
"    Filtering does not affect logging.\n\n"

#define HELP_WIN_LEN 100
#define RUNTIME_HELP_PRINT_LINES 16
#define RUNTIME_HELP_PRINT \
    " -<str1>,<str2>,..\t\texclude lines with <str1> or <str2> or ...\n" \
    " +<str1>,<str2>,..\t\tinclude only lines with <str1> or <str2> or ...\n" \
    " !<str1>,<str2>,..\t\thighlight lines with <str1> or <str2> or ...\n" \
    " clear            \t\tclear all inc, exc and highlight filters\n" \
    " C                \t\tclear logwindow\n" \
    " ctrl+p           \t\tpause/resume\n" \
    " F2               \t\ttoggle this help\n" \
    " F3               \t\ttoggle ethernet packet sender\n" \
    " F4               \t\ttoggle definition finder\n" \
    " F5               \t\ttoggle installed filters view\n" \
    " ctrl+b           \t\tsend ethernet packet when F3 view is on\n" \
    " ctrl+f           \t\tFind values for definitions when definition finder view on\n" \
    " altGR+s          \t\tOpen collected log file on editor. NOTE: does not flush prints to file!\n" \
    " esc              \t\tquit\n"




#define TIMESTAMPSIZE 26
#define ERRPR DEBUGPR
#define DEBUGPR(foo,args...) { if(NULL!=G_logfile){ fprintf(G_logfile,(foo), ## args ); fflush(G_logfile); } }
#ifdef debug
#define VERBOSE_DEBUGPR DEBUGPR
#else
#define VERBOSE_DEBUGPR(foo,args...) ;
#endif

extern FILE *G_logfile;
void schedule_filewrite(int filewrite,int bs);
void out(int sig);
uint32_t scan_uint(FILE *cf,char *scanfmt);
int get_mac(char *filter,uint8_t *mac);

#endif

