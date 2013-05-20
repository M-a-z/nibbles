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

#ifndef SHITEMSGPARSER_H
#define SHITEMSGPARSER_H

#include "msgdefines.h"
#include <stdio.h>
#include "common.h"

#define SHITEFNAME_MAX 255
#define HTYPE_QINQ 2
#define HTYPE_VLAN 1
#define HTYPE_ENET 0

typedef struct shitemsglist
{
    struct shitemsglist *next;
    char *msgname;
    char *msgdesc;
    int hdrtype;
    int plsize;
    struct qinq_eth_hdr hdr;
    void *payload;
}shitemsglist;
typedef struct shitemsgparser
{
    char filename[SHITEFNAME_MAX];
    FILE *msgfile;
    shitemsglist *msgs;
    int msgamnt;
    int (*load_msgs)(struct shitemsgparser *);
    void *(*get_first_msgitem)(struct shitemsgparser *);
    void *(*get_next_msgitem)(struct shitemsgparser *,void *);
    int (*get_matching_plsize)(struct shitemsgparser *,void *);
    void *(*get_matching_pl)(struct shitemsgparser *,void *);
    void *(*get_matching_hdr)(struct shitemsgparser *,void *,int *);
    char *(*get_matching_name)(struct shitemsgparser *,void *);
    char *(*get_matching_desc)(struct shitemsgparser *,void *);
    void (*release_shitemsglist)(struct shitemsgparser *);
    int (*loaded_msg_amnt)(struct shitemsgparser *);
}shitemsgparser;

shitemsgparser *init_shitemsgparser(char *filename);

#endif
