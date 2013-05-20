
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

#ifndef NIBBLES_MSGDEFINES_H
#define NIBBLES_MSGDEFINES_H

//#include "tgt_commander.h"
#include <inttypes.h>
#include <stdlib.h>

typedef struct eth_hdr
{
    uint8_t dmac[6];
    uint8_t smac[6];
    uint16_t ethtype;
} __attribute__((packed)) eth_hdr;

typedef struct vlan_eth_hdr
{
    uint8_t dmac[6];
    uint8_t smac[6];
    uint32_t vlan_tag;
    uint16_t ethtype;
} __attribute__((packed)) vlan_eth_hdr;

typedef struct qinq_eth_hdr
{
    uint8_t dmac[6];
    uint8_t smac[6];
    uint32_t vlan1_tag;
    uint32_t vlan2_tag;
    uint16_t ethtype;
} __attribute__((packed)) qinq_eth_hdr;


#endif
