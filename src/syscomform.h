
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

#include <inttypes.h>
#include <ncurses.h>
#include <form.h>
#include "commonformchars.h"
#include "shitemsgparser.h"

#ifndef _SYSCOMFORM_H
#define _SYSCOMFORM_H

#define GENERIC_EXP_TEXT "hit CTRL+B to send via"
#define DMAC_EXP_TEXT    "dmac 1a:2b:.."
#define SMAC_EXP_TEXT    "smac 1a:2b:.."
#define QINQ_EXP_TEXT    "QinQ"
#define VLAN_EXP_TEXT    "VLAN"
#define ETHTYPE_EXP_TEXT "Etype"
#define PAYLOAD_EXP_TEXT "payload (bytes as 0xAB 0xBA ...)"
#define SAVEMSG_EXP_TEXT "Message name, (F12 saves)"

#define GENERIC_EXP_STARTLINE   1
#define SAVENAME_EXPSTARTLINE   (GENERIC_EXP_STARTLINE+2)
#define SAVENAMEFIELD_STARTLINE (SAVENAME_EXPSTARTLINE+1)
#define HDRFIELD_EXP_STARTLINE  (SAVENAMEFIELD_STARTLINE+2)
#define HDRFIELD_STARTLINE      (HDRFIELD_EXP_STARTLINE+1)
#define HDRFIELD2_EXP_STARTLINE  (HDRFIELD_STARTLINE+2)
#define HDRFIELD2_STARTLINE      (HDRFIELD2_EXP_STARTLINE+2)
#define PAYLOAD_EXPSTARTLINE    (HDRFIELD2_STARTLINE+2)       
#define PAYLOAD_STARTLINE       (PAYLOAD_EXPSTARTLINE+1)

#define LEFT_FIELD_STARTCOL 2

#define GENERIC_EXP_WIDTH       25
#define SAVENAMEFIELD_WIDTH     60
//#define IDFIELD_WIDTH           10
#define DMACFIELD_WIDTH           17
#define SMACFIELD_WIDTH           17
#define ETHTYPEFIELD_WIDTH         6
#define VLANFIELD_WIDTH         12
#define QINQFIELD_WIDTH         12
#define PAYLOADFIELD_WIDTH      46

#define SAVENAMEFIELD_STARTCOL  (LEFT_FIELD_STARTCOL)
#define DMACFIELD_STARTCOL      (LEFT_FIELD_STARTCOL)
#define QINQFIELD_STARTCOL      (LEFT_FIELD_STARTCOL)
#define PAYLOADFIELD_STARTCOL   (LEFT_FIELD_STARTCOL)
#define GENERIC_EXP_STARTCOL    20
#define SMACFIELD_STARTCOL      ((DMACFIELD_STARTCOL+DMACFIELD_WIDTH)+2)
#define VLANFIELD_STARTCOL      ((QINQFIELD_STARTCOL+QINQFIELD_WIDTH)+2)
#define ETHTYPEFIELD_STARTCOL   ((VLANFIELD_STARTCOL+VLANFIELD_WIDTH)+2)    

#define DMACFIELD_HEIGHT        1
#define SMACFIELD_HEIGHT        1
#define VLANFIELD_HEIGHT        1
#define QINQFIELD_HEIGHT        1
#define ETHTYPEFIELD_HEIGHT         1
#define SAVENAMEFIELD_HEIGHT    1
#define PAYLOADFIELD_HEIGHT     10


typedef struct formal_msg
{
    WINDOW *formwin;
    FORM *syscomform;
    FIELD *dmacfield;
    FIELD *smacfield;
    FIELD *qinqfield;
    FIELD *vlanfield;
    FIELD *ethtypefield;
    FIELD *payloadfield;
    FIELD *savenamefield;
    FIELD *ifnamefield;
    FIELD *expl_send;
    FIELD *expl_dmac;
    FIELD *expl_smac;
    FIELD *expl_qinq;
    FIELD *expl_vlan;
    FIELD *expl_etype;
    FIELD *expl_pl;
    FIELD *expl_sna;
    FIELD *nullfield;
    int rows;
    int cols;
    void (*fillform)(struct formal_msg *,shitemsgparser *,void *);
    void (*display_form)(struct formal_msg *);
    void (*form_chr)(struct formal_msg *, int);
    void *(*get_msg_from_form)(struct formal_msg *,uint32_t *,char **,int *);
    void (*clear_form)(struct formal_msg *);
    void (*fill_header_fields_from_struct)(struct formal_msg *,eth_hdr *,int);
    void (*fill_payload_field_from_bin)(struct formal_msg *,char *,size_t);
    int (*savemsg)(struct formal_msg *);
}formal_msg;



formal_msg * init_form(WINDOW *formwin);

#endif

