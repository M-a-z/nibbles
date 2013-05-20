
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

#include "bshandler.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <sched.h>
#include <sys/mman.h>
#include "udp_handler.h" 

#define MVA_MIN(foo,bar) ( ((foo)<(bar))?(foo):(bar))
#define MVA_MAX(foo,bar) ( ((foo)>(bar))?(foo):(bar))
#define REALFILESIZE(rfile) (((underlying_file*)(rfile))->w_mapped_start+ ((underlying_file*)(rfile))->w_used)

static size_t DEFAULT_SCROLL_MAP_SIZE=0;
static size_t BS_AREA_SIZE=0;
static size_t BS_R_AREA_SIZE=0;
static size_t BS_WATERMARK=0;
static size_t BS_R_WATERMARK=0;
static size_t PRINTLINEPTRAMNT=0;
static size_t G_pagesize=0;

//#define BS_AREA_SIZE 6000
//#define BS_WATERMARK 4500
#define ASSUMED_AVG_PRINT_LEN (TIMESTAMPSIZE+12)
//#define PRINTLINEPTRAMNT (BS_AREA_SIZE/ASSUMED_AVG_PRINT_LEN)

static pthread_mutex_t filemutex=PTHREAD_MUTEX_INITIALIZER;

extern int G_cancelled;
extern int G_bs1filewrneeded;
extern int G_bs2filewrneeded;
extern int G_bs1_wrsize;
extern int G_bs2_wrsize;

//static char G_bs1[BS_AREA_SIZE]={0};
//static char G_bs2[BS_AREA_SIZE]={0};
static char * G_bs1=NULL;
static char * G_bs2=NULL;
//static printptrs G_printptrs[PRINTLINEPTRAMNT];


static char *get_next_readable(printptrhandler *_this,int *size);
//static void scroll_set_offset_block(struct printptrhandler *_this,unsigned screensize,int scrolldir);
static int scroll_set_offset_block(struct printptrhandler *_this,unsigned screensize,int scrolldir);
static char *scroll_get_offset_block(printptrhandler *_this,int *size);
static char *get_writepoint(printptrhandler *_this, int *freespace);
static int update_writepoint(printptrhandler *_this,char *writepoint,size_t writelen,int *filewrbs);
static void toggle_scrollmode(printptrhandler *_this,int enable);

static void init_mem_regions()
{
    G_pagesize=sysconf(_SC_PAGE_SIZE);
    /* hack */
    BS_AREA_SIZE=3*G_pagesize;
    BS_WATERMARK=2*G_pagesize;
    BS_R_AREA_SIZE=BS_AREA_SIZE;
    BS_R_WATERMARK=BS_WATERMARK;
    DEFAULT_SCROLL_MAP_SIZE=BS_AREA_SIZE*2;
    PRINTLINEPTRAMNT=BS_AREA_SIZE/ASSUMED_AVG_PRINT_LEN;
    DEBUGPR("mem region sizes set: page size=0x%x,BS_AREA_SIZE=0x%x,BS_WATERMARK=0x%x,DEFAULT_SCROLL_MAP_SIZE=0x%x\n",G_pagesize,BS_AREA_SIZE,BS_WATERMARK,DEFAULT_SCROLL_MAP_SIZE);
}

void udp_file_flush(void *arg)
{
//    FILE *printfile=*(FILE **)arg;
    printptrhandler *_this=((udp_handler *)arg)->bufferhandler;
    printf("Flusher called!\n");
    if(!_this)
    {
        G_cancelled=1;
        return;
    }
    pthread_mutex_lock(&filemutex);
    msync(G_bs1, _this->realfile->w_used,MS_SYNC);
    ftruncate(_this->realfile->fd,REALFILESIZE(_this->realfile));

    G_cancelled=1;
}

static int expand_file(int fd, size_t expansion, size_t *filesize)
{
    VERBOSE_DEBUGPR("file expansion requested: fd=%d,expansion=%u\n",fd,expansion);
    *filesize=(size_t)lseek(fd,expansion,SEEK_END);
    if((size_t)(off_t)-1==*filesize || 1!=write(fd,"",1))
    {
        DEBUGPR("file expansion FAILED!\n");
            return -1;
    }
    VERBOSE_DEBUGPR("file expansion done, new size=0x%x\n",(unsigned int)*filesize);
    return 0;
}

#ifndef O_CLOEXEC
    #define O_CLOEXEC 0
#endif

static int prepare_udp_log(underlying_file *_this)
{
    //int tmp;
    _this->fd=open(_this->filename,O_CLOEXEC | O_TRUNC | O_CREAT|O_RDWR, S_IRUSR | S_IWUSR);
    if(-1==_this->fd)
        goto err_open;
    if(expand_file(_this->fd,BS_AREA_SIZE,&_this->filesize))
        goto err_expand;
    if(MAP_FAILED==(G_bs1 = mmap(NULL, BS_AREA_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED,_this->fd, 0)))
        goto err_expand;
    if( MAP_FAILED==(G_bs2 = mmap(NULL, BS_AREA_SIZE, PROT_READ, MAP_SHARED,_this->fd, 0)))
        goto umap;
    VERBOSE_DEBUGPR("realfile prepared, mapped writes to %p, reads to %p\n",G_bs1,G_bs2);
    if(0)
    {
umap:
        munmap(G_bs1,BS_AREA_SIZE);
err_expand:
        close(_this->fd);
        unlink(_this->filename);
err_open:
        DEBUGPR("Failed to use %s\n",_this->filename);
    return -1;
    }
    return 0;
}

underlying_file *realfile_init(char *filename,size_t max_filesize,int rotateamnt)
{
    underlying_file *_this=NULL;
    if(max_filesize && max_filesize<BS_AREA_SIZE)
    {
        DEBUGPR("minimum UDP file size id %u",BS_AREA_SIZE);
    }
    else
        _this=calloc(1,sizeof(underlying_file));
    if(_this)
    {
        _this->filename=filename;
        _this->max_filesize=max_filesize;
        _this->rotateamnt=rotateamnt;
        if(prepare_udp_log(_this))
        {
            free(_this);
            _this=NULL;
        }
        else
        {
//            _this->r_used=_this->w_used=_this->w_mapped_start=_this->r_mapped_start=_this->w_used=_this->r_used=0;
//            calloc should zero the area
            VERBOSE_DEBUGPR("realfile_init successfully done\n");
        }
    }
    return _this;
}

printptrhandler *init_printptrhandler(char *filename,size_t max_filesize,int rotateamnt)
{
    printptrhandler *_this=calloc(1,sizeof(printptrhandler));
    init_mem_regions();
    if(_this)
    {
        _this->realfile=realfile_init(filename,max_filesize,rotateamnt);
        if(!_this->realfile)
        {
            free(_this);
            _this=NULL;
        }
        else
        {
//            _this->writebs=_this->usedbs=&(G_bs1[0]);
            _this->get_writepoint=&get_writepoint;
            _this->update_writepoint=&update_writepoint;
            _this->get_next_readable=&get_next_readable;
            _this->scroll_set_offset_block=&scroll_set_offset_block;
            _this->scroll_get_offset_block=&scroll_get_offset_block;
            _this->toggle_scrollmode=&toggle_scrollmode;
            VERBOSE_DEBUGPR("init_printptrhandler() successfully completed\n");
        }
    }
    return _this;
}
/* Returns pointer to next free write position, and amount of free space */
/* Note, you should call update_writepoint() before calling get_writepoint() again */
static char *get_writepoint(printptrhandler *_this, int *freespace)
{
    int data_crossing_page_boundary;
//    char *wp=NULL;
    
    if(0<=(data_crossing_page_boundary=_this->realfile->w_used-BS_WATERMARK))
    {
        VERBOSE_DEBUGPR("writer mapping next block\n");
        msync(G_bs1, _this->realfile->w_used, MS_SYNC );
        /* expand file and change map window accordingly */
        if(data_crossing_page_boundary>=G_pagesize)
        {
            /* This was built using assumption that writer never exceeds BS_AREA_SIZE before we remap stuff. Eg, data_crossing_page_boundary should never exceed BS_AREA_SIZE - BS_WATERMARK which is currently pagesize wide block */
            DEBUGPR("Problems to be expected!!!!!! %s:%d\n",__FILE__,__LINE__);
            /* I am unsure if this saves our asses, but let's try read and map whole BS_AREA_SIZE more */
            if(expand_file(_this->realfile->fd,BS_AREA_SIZE,&_this->realfile->filesize))
            {
                DEBUGPR("Failed to expand file memory mapping!\n");
                out(0);
            }
            if(MAP_FAILED==(G_bs1=mmap(G_bs1, BS_AREA_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED,
                                  _this->realfile->fd, _this->realfile->w_mapped_start+BS_AREA_SIZE)))
            {
                DEBUGPR("Failed to change memory mapping!\n");
                out(0);
            }
            _this->realfile->w_mapped_start+=BS_AREA_SIZE;
            _this->realfile->w_used=0;
            *freespace=BS_AREA_SIZE;
        }
        if(expand_file(_this->realfile->fd,BS_WATERMARK,&_this->realfile->filesize))
        {
            DEBUGPR("Failed to expand file memory mapping!\n");
            out(0);
        }
        if(MAP_FAILED==(G_bs1=mmap(G_bs1, BS_AREA_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED,
                                  _this->realfile->fd, _this->realfile->w_mapped_start+BS_WATERMARK)))
        {
            DEBUGPR("Failed to change memory mapping!\n");
            out(0);
        }
        _this->realfile->w_mapped_start+=BS_WATERMARK;
        *freespace=BS_AREA_SIZE-data_crossing_page_boundary;
        _this->realfile->w_used=data_crossing_page_boundary;
    }
    else
        *freespace=BS_AREA_SIZE-_this->realfile->w_used;
    pthread_mutex_lock(&filemutex);
    return G_bs1+_this->realfile->w_used;
}
/* Advances internal book keeping according to lenght user says he wrote */
/* Returns 0 if buffer watermark was not exceeded, number of buffer (buffer 1 or buffer 2) to flush in file othervice */
static int update_writepoint(printptrhandler *_this,char *writepoint,size_t writelen,int *filewrbs)
{
    msync(G_bs1+_this->realfile->w_used,_this->realfile->w_used+writelen , MS_ASYNC );
    pthread_mutex_unlock(&filemutex); 
    VERBOSE_DEBUGPR("Wrote block: offset from mapping start %u (mapping in file at %u). Block size %u\n",_this->realfile->w_used,_this->realfile->w_mapped_start, writelen);

    /* Update to next write position */
    _this->writebs+=writelen;
    _this->realfile->w_used+=writelen;
    return 0;
}
/* Basically just map close to DEFAULT_SCROLL_MAP_SIZE bytes from end of file to scroll window. (pagesize aligned)
 * Ensure there is enough file. Set info in realfile struct according to mapping.
 * Also set state varible (which may actually not be used??)
 */
void toggle_scrollmode(printptrhandler *_this,int enable)
{
    int tmp;
    size_t filesize=REALFILESIZE(_this->realfile);
    size_t map_size;
    size_t map_f_offset;

    if(enable)
    {
        VERBOSE_DEBUGPR("Enabling scrollmode\n");
        /* map scrollfile */
        for
        (
            map_f_offset=0;
            (
                tmp=
                (int)filesize-((int)map_f_offset+(int)DEFAULT_SCROLL_MAP_SIZE)
            )
            >
                0;
            map_f_offset+=G_pagesize
        );
        map_size=DEFAULT_SCROLL_MAP_SIZE+tmp;

        if
        (
            MAP_FAILED==
            (
                _this->realfile->s_map_ptr=
                mmap
                (
                    NULL,
                    map_size, 
                    PROT_READ, 
                    MAP_SHARED, 
                    _this->realfile->fd, 
                    map_f_offset
                )
            )
        )
        {
            DEBUGPR("Mapping scroll win FAILED!");
            _this->realfile->s_map_ptr=NULL;
            return;
        }
        _this->realfile->s_mapped_start=map_f_offset;
        _this->realfile->s_used=map_size;
        _this->realfile->scroll_map_size=map_size;
        _this->realfile->scrollmode=1;
        VERBOSE_DEBUGPR("Mapped scroll win");
    }
    else
    {
        VERBOSE_DEBUGPR("Disabling scrollmode\n");
        if(_this->realfile->s_map_ptr && _this->realfile->scrollmode)
            munmap(_this->realfile->s_map_ptr,_this->realfile->scroll_map_size);
        else
        {
            DEBUGPR("Already Unmapped scroll win!?!?!?!?!");
        }
        _this->realfile->scrollmode=0;
        VERBOSE_DEBUGPR("Unmapped scroll win");
        _this->realfile->s_mapped_start=0;
        _this->realfile->s_used=0;
        _this->realfile->scroll_map_size=0;
        _this->realfile->scrollpoint=NULL;
        _this->realfile->s_map_ptr=NULL;
    }
}

/* 
 * When scroll is going out of mapped region, we call this to map offset more to desired direction. Note that offset should be multiples of pagesize
 * Check that we're not going beyond file
 * Fix s_used pointer to point at the same line as before, but reflecting new offset mapping.
 */


static void remap_scrollwin(underlying_file *_this,int offsetchange)
{
    int tmp;
    if
    (
        (int)_this->s_mapped_start+offsetchange < 0 || 
        _this->s_mapped_start+offsetchange > REALFILESIZE(_this) 
    )
    {
        DEBUGPR("Can't remap: _this->s_mapped_start+offsetchange = %d, REALFILESIZE = %u\n",_this->s_mapped_start+offsetchange,REALFILESIZE(_this));
        return;
    }
    VERBOSE_DEBUGPR("Remapping scroll_win: offset %d\n",offsetchange);

    _this->scroll_map_size=
        (0 < ( tmp = (int) (_this->s_mapped_start+offsetchange+_this->scroll_map_size) - 
               REALFILESIZE(_this))
        )
        ?
            _this->scroll_map_size-tmp
        :
            _this->scroll_map_size;

    if
    (
        MAP_FAILED==
        (
            _this->s_map_ptr=
            mmap
            (
                _this->s_map_ptr,
                _this->scroll_map_size, 
                PROT_READ, 
                MAP_SHARED | MAP_FIXED, 
                _this->fd, 
                _this->s_mapped_start+offsetchange
            )
        )
    )
    {
        DEBUGPR("Mapping scroll win FAILED!");
        _this->s_map_ptr=NULL;
        return;
    }
    VERBOSE_DEBUGPR("Old values before remap: map_start = %d, used=%d\n",_this->s_mapped_start,_this->s_used);
    _this->s_mapped_start+=offsetchange;
    _this->s_used-=offsetchange;
    VERBOSE_DEBUGPR("New values after remap: map_start = %d, used=%d\n",_this->s_mapped_start,_this->s_used);
}
/*
 * This searches for next/previous '\n' from mapped file, and then sets 's_used' pointer to this char.
 * If there is less than 'screensize' bytes of file mapped upwards from this '\n', then try changing
 * mapping offset by G_pagesize. set scrollptr to 'screensize' bytes before '\n' (or beginning of file
 * if there's not enough file to map)
 */
static int scroll_set_offset_block(struct printptrhandler *_this,unsigned screensize,int scrolldir)
{
    int i,tmp;
    int remaphappened __attribute__((unused)) = 0;
    if(!_this->realfile->scrollmode)
        return -1;

    switch(scrolldir)
    {
        case 0:
            return 0;
        case KEY_UP:
            VERBOSE_DEBUGPR("KEY_UP scroll req: Old _this->realfile->s_used = %d\n",_this->realfile->s_used);
reup:
            for(i=-1;_this->realfile->s_used && (int)_this->realfile->s_used+i > 0 && _this->realfile->s_map_ptr[_this->realfile->s_used+i] != '\n';i--);
            VERBOSE_DEBUGPR("Searched '\n', found? - i=%d %s\n",i,(remaphappened)?"YAY! We Just Remapped":" ");
            if( (int)((int)_this->realfile->s_used+i-screensize)<0)
            {
                if(_this->realfile->s_mapped_start)
                {
                    remap_scrollwin(_this->realfile,-G_pagesize);
                    VERBOSE_DEBUGPR("KEY_UP scroll req: _this->realfile->s_used = %d after remap\n",_this->realfile->s_used);
                    remaphappened=1;
                    goto reup;
                }
                else
                {
                    /* Looks like we have hit the head of file, cannot map more. So let's just adjust the this->realfile->s_used */
                    VERBOSE_DEBUGPR("SCROLL UP req, filetop hit. Cannot remap more scroll window\n");
                    if(_this->realfile->s_used)
                        if(_this->realfile->s_used+i)
                            _this->realfile->s_used+=i;
                    VERBOSE_DEBUGPR("KEY_UP scroll req: Setting _this->realfile->s_used = %d\n",_this->realfile->s_used);
                }
            }
            else
            {
                VERBOSE_DEBUGPR("Definition of enough: s_used=%d, i=%d, screensize=%d, s_used+i-screensize=%d\n",_this->realfile->s_used,i,screensize,(int)_this->realfile->s_used+i-screensize);
                _this->realfile->s_used+=i;
                VERBOSE_DEBUGPR("KEY_UP scroll req: enough file left. Setting _this->realfile->s_used = %d\n",_this->realfile->s_used);
            }
            break;
        case KEY_DOWN:
            VERBOSE_DEBUGPR("KEY_DOWN scroll req: Old _this->realfile->s_used = %d\n",_this->realfile->s_used);
redown:
            for(i=1;_this->realfile->s_used+i<_this->realfile->scroll_map_size && _this->realfile->s_map_ptr[_this->realfile->s_used+i] != '\n';i++);
            if(_this->realfile->s_used+i>_this->realfile->scroll_map_size)
            {
                if(_this->realfile->s_mapped_start+_this->realfile->scroll_map_size < REALFILESIZE(_this->realfile))
                {
                    VERBOSE_DEBUGPR("SCROLL DOWN req, remapping more scroll window\n");
                    remap_scrollwin(_this->realfile,+G_pagesize);
                    VERBOSE_DEBUGPR("KEY_DOWN scroll req: _this->realfile->s_used = %d after remap\n",_this->realfile->s_used);
                    goto redown;
                }
                else
                {
                    /* Looks like we have hit the bottom of file, cannot map more. So let's just adjust the this->realfile->s_used */
                    VERBOSE_DEBUGPR("SCROLL DOWN req, filebottom hit. Cannot remap more scroll window\n");
                    _this->realfile->s_used=_this->realfile->scroll_map_size-1;
                    VERBOSE_DEBUGPR("KEY_DOWN scroll req: Setting _this->realfile->s_used = %d\n",_this->realfile->s_used);
                }
            }
            else
            {
                _this->realfile->s_used+=i;
                VERBOSE_DEBUGPR("KEY_DOWN scroll req: enough file left. Setting _this->realfile->s_used = %d\n",_this->realfile->s_used);
            }
            break;
        default:
            VERBOSE_DEBUGPR("Unknown command at %s\n",__FUNCTION__);
            return -1;
    }
    _this->realfile->uglyinternalstatevariable=1;
    _this->realfile->scrollpoint=_this->realfile->s_map_ptr+(((tmp=(int)_this->realfile->s_used-(int)screensize)>0)?tmp:0);
    VERBOSE_DEBUGPR("scroll_set_offset_block, dir %s. Setting scrollpoint %p, (_this->realfile->s_used = %d, screensize = %d\n",(KEY_UP)?"UP":"DOWN",_this->realfile->scrollpoint,(int)_this->realfile->s_used,screensize);
    return 0;
}
/* Return block selected by scroll_set_offset_block 
 * Block start should be pointed by scrollpoint, and end of block should be at 's_used' offset from start of mapped scroll window.
 */
static char * scroll_get_offset_block(printptrhandler *_this,int *size)
{
    if(_this->realfile->uglyinternalstatevariable)
    {
        _this->realfile->uglyinternalstatevariable=0;
        *size=(int)( (_this->realfile->s_map_ptr+_this->realfile->s_used) - _this->realfile->scrollpoint);
        VERBOSE_DEBUGPR("Returning scrollpoint %p, size %u\n",_this->realfile->scrollpoint,*size);
    }
    else
    {
        *size=0;
        return NULL;
    }
    return _this->realfile->scrollpoint;
}

/* Returns pointer to next unread line and lenght of the line. Also marks line as read then.
 * Returns NULL if no lines are read */
static char *get_next_readable(printptrhandler *_this,int *size)
{
    /*
    int next_readindex=(_this->read_printptr_index+1)%PRINTLINEPTRAMNT;
    */
    char *retptr;
    size_t writepoint=REALFILESIZE(_this->realfile);
    size_t readpoint=_this->realfile->r_used+_this->realfile->r_mapped_start;
    int read_over_pageboundary;
    /* If write and read indexes are same, then theres no unread prints */
    if(readpoint>=writepoint)
    {
        *size=0;
        return NULL;
    }
    /* If we have read somewhere between 
     * BS_R_WATERMARK - BS_R_AREA_SIZE from the current mapping, 
     * then we need to map more 
     */
    if(0<=(read_over_pageboundary=_this->realfile->r_used-BS_R_WATERMARK))
    {
        /* map more - see if we have read whole BS_R_AREA_SIZE. 
         * If so, then we need to map full BS_R_AREA_SIZE and 
         * set 'already_read' to 0 (Eg. whole new mapping is unread)
         */
        VERBOSE_DEBUGPR("reader mapping next block\n");

        //if(read_over_pageboundary<G_pagesize)
        //{
            /* if we had not read whole BS_R_AREA_SIZE but somewhere 
             * between BS_R_WATERMARK - BS_R_AREA_SIZE, then we just 
             * map BS_R_WATERMARK more, and set the amount 
             * we had read over BS_R_WATERMARK as 'already_read' 
             */
            if(MAP_FAILED==(G_bs2=mmap(G_bs2, BS_R_AREA_SIZE, PROT_READ, MAP_SHARED | MAP_FIXED, _this->realfile->fd, _this->realfile->r_mapped_start+BS_R_WATERMARK)))
            {
                DEBUGPR("Failed to change memory mapping!\n");
                out(0);
            }
            _this->realfile->r_mapped_start+=BS_R_WATERMARK;
            _this->realfile->r_used=read_over_pageboundary;
        /*}
        else
        {
            if(MAP_FAILED==(G_bs2=mmap(G_bs2, BS_R_AREA_SIZE, PROT_READ, MAP_SHARED | MAP_FIXED, _this->realfile->fd, _this->realfile->r_mapped_start+BS_R_AREA_SIZE)))
            {
                DEBUGPR("Failed to change memory mapping!\n");
                out(0);
            }
            _this->realfile->r_mapped_start+=BS_R_AREA_SIZE;
            _this->realfile->r_used=0;
        }*/
    }
    /* Allright, now we should have ensured there is stuff to read in mapped memory */
    /* Let's see if we have full mapped area to read */
    if(_this->realfile->r_mapped_start<_this->realfile->w_mapped_start)
    {
        /* all right, writer has proceeded over our current read mapping - whole mapped block should be readable */
        *size=BS_R_WATERMARK-_this->realfile->r_used;
    }
    else
    {
        /* writer has not necessarily written whole readblock -> we should just read as much as there is data for reading. */
        *size=writepoint-readpoint;
    }
    /* Let's return read ptr to first spot that's not yet read */
    VERBOSE_DEBUGPR("Returning readable block offset from mapping start %u (mapping in file at %u). Block size %u\n",_this->realfile->r_used,_this->realfile->r_mapped_start, *size);
    retptr=G_bs2+_this->realfile->r_used;
    /* Let's mark returned size as read */
    _this->realfile->r_used+=*size;
    if(writepoint<_this->realfile->r_mapped_start+*size)
    {
        int tmp;
        VERBOSE_DEBUGPR("JALLU FOUND! write offset at %u, read goes to %u\n",writepoint,_this->realfile->r_mapped_start+*size);
        tmp=_this->realfile->r_mapped_start+*size - writepoint;
        *size=*size-tmp;
        _this->realfile->r_used-=tmp;
        VERBOSE_DEBUGPR("DummyJalluFix - readsize decreased to %d.\n",*size);
    }
    return retptr;
}


