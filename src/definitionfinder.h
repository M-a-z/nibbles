
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

#ifndef DEFINITIONFINDER_H
#define DEFINITIONFINDER_H
#include <ncurses.h>
#include <form.h>
//#include <commonformchars.h>

#define MAX_DEFFIND_ID 8191
#define DEFFIND_ID_AMNT 8192
//#define MAX_DEFFIND_ID 4095
//#define DEFFIND_ID_AMNT 4096
#define KEY_EVALUATE_DEFFORM (6) /*ctrl+f */
/* Basic heeight for oneliners */
#define DEFF_IDFIELD_HEIGHT 1
//#define DEFF_MSG_ROW 2
//#define DEFF_MSGNAMEFIELD_COL 2
//#define DEFF_MSGNAMEFIELD_LEN 60
//#define DEFF_MSGIDFIELD_COL 64
//#define DEFF_MSGIDFIELD_LEN 6

/* Generic explanation on top */
#define DEFF_GEN_EXPL_ROW 0
/* Task explanation two lines below */
#define DEFF_TASK_EXPL_ROW (DEFF_GEN_EXPL_ROW+2)
/* Actual task input right below explanation */
#define DEFF_TASK_ROW (DEFF_TASK_EXPL_ROW+1)

/* Struct explanation two rows ubnder task row */
#define DEFF_STRUCT_EXPL_ROW (DEFF_TASK_ROW+2)
/* struct row right below explanation */
#define DEFF_STRUCT_ROW (DEFF_STRUCT_EXPL_ROW+1)
/* struct data row right below struct name input */
#define DEFF_STRDATA_ROW (DEFF_STRUCT_ROW+1)

#define DEFF_TASKNAMEFIELD_COL 2 
#define DEFF_TASKNAMEFIELD_LEN 60
#define DEFF_TASKIDFIELD_COL 64
#define DEFF_TASKIDFIELD_LEN 6
#define DEFF_STRUCTNAMEFIELD_COL 2
#define DEFF_STRUCTNAMEFIELD_LEN 40
#define DEFF_STRUCTTYPEFIELD_COL 2
#define DEFF_STRUCTTYPEFIELD_LEN 80
#define DEFF_STRDATA_HEIGHT 34

//#define DEFF_MSGIDFIELD_COL

typedef struct definitionfinder
{
    WINDOW *dw;
    FORM *defform;
//    FIELD *msgidnamef;
//    FIELD *msgidvaluef;
    FIELD *taskidnamef;
    FIELD *taskidvaluef;
    FIELD *structnamef;
    FIELD *structtypef;
    FIELD *expl_eval;
//    FIELD *expl_msgid;
    FIELD *expl_taskid;
    FIELD *expl_struct;
    FIELD *nullfield;
    int rows;
    int cols;
//    int last_msgid;
    int last_taskid;
    int last_struct;
    void (*create_form)(struct definitionfinder *);
    void (*display_matches)(struct definitionfinder *);
    int (*add_form_values)(struct definitionfinder *);
    void (*handle_input)(struct definitionfinder *,int ch);
    void (*display_form)(struct definitionfinder *);
}definitionfinder;
definitionfinder * init_definitionfinder(WINDOW* dw);

#endif

