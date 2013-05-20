
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

#ifndef BSHANDLER_H
#define BSHANDLER_H

#include "common.h"

typedef struct printptrs
{
    int linelen;
    char *printptr;
    char *buffinuse;
}printptrs;

typedef struct underlying_file
{
    int uglyinternalstatevariable;
    char *filename; // basename
    size_t max_filesize; //maximum filesize (further dev)
    int rotateamnt;      //amount of logs to keep (further dev)
    int rotateindex;     //which log is currently being written (further dev)
    size_t filesize;     //size of actual file.
    int scrollmode;       //displaying scroll window, not window advancing according to writes
    char *s_map_ptr;
    size_t s_mapped_start; //spot of file which is mapped for scrolling
    size_t s_used;         //point inside mapped scroll area - end of displayed scroll text.
    size_t scroll_map_size; //size of mapped scoll memory
    char *scrollpoint;      //point inside mapped scroll area - starting point of displayed scroll area
    size_t r_mapped_start; //startpoint of mapped region (map size is BS_AREA_SIZE)
    size_t r_used;         //currently written 
    size_t w_mapped_start; //startpoint of mapped region (map size is BS_AREA_SIZE)
    size_t w_used;         //currently written 
    int fd;              //file descriptor for currently open file.
}underlying_file;

typedef struct printptrhandler
{
    underlying_file *realfile;
    int write_printptr_index;
    int read_printptr_index;
//    char *usedbs; //pointer to used buffer's start
    char *writebs; //next writepoint - updated by update_writepoint
    char * (*get_writepoint)(struct printptrhandler *_this, int *freespace);
    int (*update_writepoint)(struct printptrhandler *_this,char *writepoint,size_t writelen,int *filewrbs);
    char * (*get_next_readable)(struct printptrhandler *_this,int *size);
    char * (*scroll_get_offset_block)(struct printptrhandler *_this,int *size);
    void (*toggle_scrollmode)(struct printptrhandler *_this,int enable);
    int (*scroll_set_offset_block)(struct printptrhandler *_this,unsigned screensize,int scrolldir);
}printptrhandler;

//printptrhandler *init_printptrhandler();
printptrhandler *init_printptrhandler(char *filename,size_t max_filesize,int rotateamnt);
char *getbsbase(int bsnum);
void udp_file_flush(void *arg);

#endif
