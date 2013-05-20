
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

#define GNU_SOURCE
#include "definitionfinder.h"
#include "cexplode.h"
#include "common.h"
#include "commonformchars.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <errno.h>

char *deflist_task_keys[DEFFIND_ID_AMNT]={NULL};
unsigned short int deflist_task_values[DEFFIND_ID_AMNT]={0};
//char *deflist_msgid_keys[DEFFIND_ID_AMNT]={NULL};
//unsigned short int deflist_msgid_values[DEFFIND_ID_AMNT]={0};
char *deflist_struct_keys[DEFFIND_ID_AMNT]={NULL};
void *deflist_struct_values[DEFFIND_ID_AMNT]={NULL};

static void get_structs(FILE *structfile, int *id)
{
    char *structname;
    char tmpstructcontent[2048]={0};
    int rval;
    int i;
	while( EOF!=(rval=fscanf(structfile,"struct %a[^\n]\n",&structname)) && *id<MAX_DEFFIND_ID)
	{
        if(rval==1 && structname)
        {
            int ok=0;
            int c;
            int namelen=strlen(structname);
            if(namelen<3)
            {
                free(structname);
                continue;
            }
            if(structname[namelen-1]=='{')
            {
                namelen-=1;
                structname[namelen-1]='\0';
            }
            for(i=0;i<2046&&EOF!=(c=fgetc(structfile));i++)
            {
                tmpstructcontent[i]=(char)c;
                if('}'==c)
                {
                    tmpstructcontent[i+1]='\0';
                    ok=1;
                    break;
                }
            }
            if(ok)
            {
                deflist_struct_values[*id]=malloc(strlen(tmpstructcontent)+1);
                if(!deflist_struct_values[*id])
                {
                    DEBUGPR("Malloc FAILED!\n");
                    free(structname);
                    return;
                }
                deflist_struct_keys[*id]=structname;
                strcpy(deflist_struct_values[*id],tmpstructcontent);
                (*id)++;
                deflist_struct_keys[*id]=NULL;
            }
            
        }
        else
        {
            if(EOF==(rval=fscanf(structfile,"%a[^\n]\n",&structname)) || rval<1)
            {
                break;
            }
            free(structname);
        }
    }
}
unsigned short int look_if_known_base(char *base)
{
    int blen,kblen,i;
    char *kbases[19]= { "API_EXTERN_BASE","(API_EXTERN_BASE","AIF_OM_IBASE","AIF_TCOM_EBASE","AIF_OM_EBASE","INET_TCOM_EBASE","INET_OM_EBASE","CD_TCOM_EBASE","CD_OM_EBASE","MONITOR_EBASE","SPARE_EXT_EBASE","INET_OM_IBASE","CD_OM_IBASE","GENIO_COMMON_IBASE","GENIO_OM_IBASE","I2C_OM_IBASE","COMMON_OM_IBASE","CTRL_OM_IBASE","SPARE_EXT_IBASE"};
    unsigned short kbasenums[19]={0x2800,0x2800,0x01C0,0x0000,0x0040,0x0080,0x00C0,0x0100,0x0140,0x0190,0x0196,0x0200,0x0240,0x0280,0x02C0,0x0300,0x0380,0x03C0,0x03D0};

    if(base)
    {
        blen=strlen(base);
        for(i=0;i<19;i++)
        {
            if(blen>=(kblen=strlen(kbases[i])))
            {
                if(!memcmp(base,kbases[i],kblen))
                    return kbasenums[i];
            }
        }
    }
    return 0;
}

static unsigned short find_task_base(char *assumedbase)
{
    char *key;
    int i;
    for(i=0,key=deflist_task_keys[0];key;key=deflist_task_keys[++i])
    {
        if(!strcmp(assumedbase,key))
            return deflist_task_values[i];
    }
    return 0;
}

static void get_tasks(FILE *file, int *id)
{
    CexplodeStrings tokenizer; 
    int pieces;
    char *task;
    char *base;
    int rval;
    char *junk;
    while( EOF!=(rval=fscanf(file,"#define %a[^\n]\n",&junk)) && *id<MAX_DEFFIND_ID)
    {
        unsigned short taskid=0;
        char *tmp;
        if(!rval)
        {
            if((rval=fscanf(file,"%*a[^\n]\n")))
                return;
            continue;
        }
        for(tmp=junk;tmp&&*tmp;tmp++)
            if(*tmp=='\t')
                *tmp=' ';
        if(1<=(pieces=Cexplode(junk," ",&tokenizer)))
        {
            if(1==pieces)
            {
                free(junk);
                Cexplode_free(tokenizer);
                continue;
            }
            if(!(task=Cexplode_getfirst(&tokenizer)))
            {
                free(junk);
                Cexplode_free(tokenizer);
                continue;
            }
            while((base=Cexplode_getnext(&tokenizer)))
            {
                unsigned short tmp=0;
                char *endp;
                tmp=strtol(base,&endp,0);
                if(*endp)
                {
                    tmp+=look_if_known_base(base);
                    if(!tmp)
                        tmp+=find_task_base(base);
                }
                taskid+=tmp;
            }
            deflist_task_keys[*id]=malloc(strlen(task)+1);
            if(!deflist_task_keys[*id])
            {
                DEBUGPR("Alloc FAILED!\n");
                free(junk);
                Cexplode_free(tokenizer);
                return;
            }
            strcpy(deflist_task_keys[*id],task);
            deflist_task_values[*id]=taskid;
            (*id)++;
            deflist_task_keys[*id]=NULL;
            Cexplode_free(tokenizer);
        }   
    }
}




void read_ids_from_file(char *filename, int *nextid,void (*filehandler)(FILE *,int *))
{
    FILE *file;
    file=fopen(filename,"r");
    if(file)
    {
        VERBOSE_DEBUGPR("Opened file %s\n",filename);
       (*filehandler)(file,nextid); 
       fclose(file);
    }
    else
        DEBUGPR("Could not open file %s\n",filename);
}
void get_idsfrom_files_in(char *dir_name, int *id,void (*filehandler)(FILE *,int *))
{
    DIR *dir;
    struct dirent *ent;
    dir=opendir(dir_name);
    if(dir)
    {
        VERBOSE_DEBUGPR("Opened dir %s\n",dir_name);
        while((ent=readdir(dir)) && *id<MAX_DEFFIND_ID)
        {
            char *newname;
            int namelen=strlen(ent->d_name)+strlen(dir_name)+2;
            if(DT_DIR==ent->d_type)
            {
                if(strcmp(ent->d_name,"..") && strcmp(ent->d_name,".svn") && strcmp(ent->d_name,"."))
                {
                    newname=malloc(namelen);
                    if(!newname)
                        return;
                    sprintf(newname,"%s/%s",dir_name,ent->d_name);
                    get_idsfrom_files_in(newname,id,filehandler);
                    free(newname);
                }
            }
            else
            {
                newname=malloc(namelen);
                if(!newname)
                    return;
                sprintf(newname,"%s/%s",dir_name,ent->d_name);
                read_ids_from_file(newname,id,filehandler);
                free(newname);
            }
        }
    }
    else
        DEBUGPR("Could not open dir %s %d (%s)\n",dir_name,errno,strerror(errno));

}

static void parse_ids(definitionfinder *_this, char *root_folder,int *id,void (*filehandler)(FILE *,int *))
{
//    int id=0;
    get_idsfrom_files_in(root_folder,id,filehandler);
}
static void display_form(definitionfinder *_this)
{
    post_form(_this->defform);
}
static int add_form_values(definitionfinder *_this)
{
    DEBUGPR("Adding form filters to defform\n");
//    set_field_type(_this->msgidnamef,TYPE_ENUM,deflist_msgid_keys,1,0);
    set_field_type(_this->taskidnamef,TYPE_ENUM,deflist_task_keys,1,0);
    set_field_type(_this->structnamef,TYPE_ENUM,deflist_struct_keys,1,0);
//    set_field_buffer(_this->expl_msgid,0,"Msg name");
    set_field_buffer(_this->expl_taskid,0,"Definition name");
    set_field_buffer(_this->expl_struct,0,"Struct type");
    set_field_buffer(_this->expl_eval,0,"CTRL+F to evaluates form");
    return 0;
}
void display_matching_structs(definitionfinder *_this)
{
    char *key;
    int ret;
    int i;
    if(current_field(_this->defform)==_this->structnamef)
        if((ret=form_driver(_this->defform,REQ_VALIDATION)))
        {
            DEBUGPR("form_driver returned %d when validate was requested!\n",ret);
            return;
        }
    for(i=0;(key=deflist_struct_keys[i]);i++)
    {
        int j;
        char *fieldbuff;
        int len=strlen(key);
        fieldbuff=field_buffer(_this->structnamef,0);
        if(len<=strlen(field_buffer(_this->structnamef,0)))
        {
            if(!memcmp(key,fieldbuff,len))
            {
                int rv;
                field_opts_on(_this->structtypef,  O_ACTIVE);
                if((rv=set_current_field(_this->defform,_this->structtypef)))
                {
                    DEBUGPR
                    (
                        "set_current_field returned %d, E_OK=%d, E_BAD_ARG=%d,E_NOT_CONN=%d,E_BAD_STATE=%d,E_INV_FIELD=%d,E_REQ_DEN=%d,E_SYS_ERR=%d",
                        rv,
                        E_OK,
                        E_BAD_ARGUMENT,
                        E_NOT_CONNECTED,
                        E_BAD_STATE,
                        E_INVALID_FIELD,
                        E_REQUEST_DENIED,
                        E_SYSTEM_ERROR
                    );
                }
                form_driver(_this->defform,REQ_OVL_MODE);
                form_driver(_this->defform,REQ_CLR_FIELD);
                for(j=0;((char *)deflist_struct_values[i])[j];j++)
                {
                    if('\n'!=((char *)deflist_struct_values[i])[j])
                        form_driver(_this->defform,((char *)deflist_struct_values[i])[j]);
                    else
                        form_driver(_this->defform,REQ_NEW_LINE);
    //                set_field_buffer(_this->structtypef,0,deflist_struct_values[i]);
                }
                form_driver(_this->defform,REQ_INS_MODE);
                field_opts_off(_this->structtypef,  O_ACTIVE);
            }
        }
        key=deflist_struct_keys[i+1];
    }

}
void display_matching_tasknames(definitionfinder *_this)
{
    char *key;
    int ret;
    int i;
    if(current_field(_this->defform)==_this->taskidnamef)
        if((ret=form_driver(_this->defform,REQ_VALIDATION)))
        {
            DEBUGPR("form_driver returned %d when validate was requested!\n",ret);
            return;
        }
    for(i=0;(key=deflist_task_keys[i]);i++)
    {
        char *fieldbuff;
        int len=strlen(key);
        fieldbuff=field_buffer(_this->taskidnamef,0);
        if(len<=strlen(field_buffer(_this->taskidnamef,0)))
        {
            if(!memcmp(key,fieldbuff,len))
            {
                char tmp[100];
                sprintf(tmp,"0x%hx",deflist_task_values[i]);
                set_field_buffer(_this->taskidvaluef,0,tmp);
            }
        }
        key=deflist_task_keys[i+1];
    }
}
/*
void display_matching_msgnames(definitionfinder *_this)
{
    char *key;
    int ret;
    int i;
    if(current_field(_this->defform)==_this->msgidnamef)
        if((ret=form_driver(_this->defform,REQ_VALIDATION)))
        {
            DEBUGPR("form_driver returned %d when validate was requested!\n",ret);
            return;
        }
    for(i=0;(key=deflist_msgid_keys[i]);i++)
    {
        char *fieldbuff;
        int len=strlen(key);
        fieldbuff=field_buffer(_this->msgidnamef,0);
        if(len<=strlen(field_buffer(_this->msgidnamef,0)))
        {
            if(!memcmp(key,fieldbuff,len))
            {
                char tmp[100];
                sprintf(tmp,"0x%hx",deflist_msgid_values[i]);
                set_field_buffer(_this->msgidvaluef,0,tmp);
            }
        }
        key=deflist_msgid_keys[i+1];
    }
}
*/
static void display_matches(definitionfinder *_this)
{
    /*
    if(field_status(_this->msgidnamef))
    {
        display_matching_msgnames(_this);
        set_field_status(_this->msgidnamef,0);
    }
    */
    if(field_status(_this->taskidnamef))
    {
        display_matching_tasknames(_this);
        set_field_status(_this->taskidnamef,0);
    }
    if(field_status(_this->structnamef))
    {
        display_matching_structs(_this);
        set_field_status(_this->structnamef,0);
    }
}
static void create_form(definitionfinder *_this)
{
    /*
    _this->msgidnamef   = new_field(DEFF_IDFIELD_HEIGHT ,DEFF_MSGNAMEFIELD_LEN,     DEFF_MSG_ROW,    DEFF_MSGNAMEFIELD_COL,     0,0);
    _this->msgidvaluef  = new_field(DEFF_IDFIELD_HEIGHT ,DEFF_MSGIDFIELD_LEN,       DEFF_MSG_ROW,    DEFF_MSGIDFIELD_COL,       0,0);
    _this->expl_msgid = new_field(DEFF_IDFIELD_HEIGHT ,DEFF_MSGNAMEFIELD_LEN,     DEFF_MSG_ROW-1,DEFF_MSGNAMEFIELD_COL,     0,0);
    */
    /* generic explanation */
    _this->expl_eval    = new_field(DEFF_IDFIELD_HEIGHT ,DEFF_STRUCTNAMEFIELD_LEN,  DEFF_GEN_EXPL_ROW,    DEFF_STRUCTNAMEFIELD_COL,  0,0);
    /* Task Explanation */
    _this->expl_taskid  = new_field(DEFF_IDFIELD_HEIGHT ,DEFF_TASKNAMEFIELD_LEN,  DEFF_TASK_EXPL_ROW,   DEFF_TASKNAMEFIELD_COL,    0,0);
    /* Task input and outputs */
    _this->taskidnamef  = new_field(DEFF_IDFIELD_HEIGHT ,DEFF_TASKNAMEFIELD_LEN,    DEFF_TASK_ROW,   DEFF_TASKNAMEFIELD_COL,    0,0);
    _this->taskidvaluef = new_field(DEFF_IDFIELD_HEIGHT ,DEFF_TASKIDFIELD_LEN,      DEFF_TASK_ROW,   DEFF_TASKIDFIELD_COL,      0,0);
    /* Struct explanation */
    _this->expl_struct  = new_field(DEFF_IDFIELD_HEIGHT ,DEFF_STRUCTNAMEFIELD_LEN,  DEFF_STRUCT_EXPL_ROW, DEFF_STRUCTNAMEFIELD_COL,  0,0);
    /* Struct input */
    _this->structnamef  = new_field(DEFF_IDFIELD_HEIGHT ,DEFF_STRUCTNAMEFIELD_LEN,  DEFF_STRUCT_ROW, DEFF_STRUCTNAMEFIELD_COL,  0,0);
    /* Struct output */
    _this->structtypef  = new_field(DEFF_STRDATA_HEIGHT ,DEFF_STRUCTTYPEFIELD_LEN,  DEFF_STRDATA_ROW,DEFF_STRUCTTYPEFIELD_COL,  0,0);

    if( (!_this->expl_eval) || (!_this->expl_taskid ) || (!_this->taskidnamef ) || (!_this->taskidvaluef ) || (!_this->expl_struct ) || (!_this->structnamef) || (!_this->structtypef))
    {
        DEBUGPR("Failed to create definition finder fields\n");
        out(-1);
    }

    _this->nullfield    = NULL;

    field_opts_off(_this->expl_eval,  O_ACTIVE);
    field_opts_off(_this->expl_taskid,  O_ACTIVE);
    field_opts_off(_this->expl_struct,  O_ACTIVE);
/*
    field_opts_off(_this->expl_msgid,  O_ACTIVE);
    set_field_back(_this->msgidnamef,   A_UNDERLINE);
    field_opts_off(_this->msgidnamef,   O_AUTOSKIP);
    field_opts_off(_this->msgidvaluef,  O_ACTIVE);
    field_opts_off(_this->msgidvaluef,  O_AUTOSKIP);
*/
    set_field_back(_this->taskidnamef,  A_UNDERLINE);
    field_opts_off(_this->taskidnamef,  O_AUTOSKIP);

    field_opts_off(_this->taskidvaluef, O_ACTIVE);
    field_opts_off(_this->taskidvaluef, O_AUTOSKIP);

    set_field_back(_this->structnamef,  A_UNDERLINE);
    field_opts_off(_this->structnamef,  O_AUTOSKIP);

    //set_field_back(_this->structtypef,  A_UNDERLINE);
//    field_opts_off(_this->structtypef,  O_ACTIVE);
  //  field_opts_off(_this->structtypef,  O_AUTOSKIP);
   // field_opts_off(_this->structtypef,  O_NL_OVERLOAD);
//    field_opts_off(_this->structtypef,  O_STATIC);

//    _this->defform=new_form(&(_this->msgidnamef));
    _this->defform=new_form(&(_this->taskidnamef));
    scale_form(_this->defform,&(_this->rows),&(_this->cols));
    wresize(_this->dw,_this->rows+4,_this->cols+4);

    keypad(_this->dw,TRUE);
    
    set_form_win(_this->defform,_this->dw);
    set_form_sub(_this->defform,derwin(_this->dw,_this->rows,_this->cols,2,2));
    DEBUGPR("Initialized defform and fields\n");
    /*
    mvwprintw(_this->formwin,HDR_EXPSTARTLINE,IDFIELD_STARTCOL,"msg id");
    mvwprintw(_this->formwin,HDR_EXPSTARTLINE,RCVRFIELD_STARTCOL,"Receiver");
    mvwprintw(_this->formwin,HDR_EXPSTARTLINE,SNDRFIELD_STARTCOL,"Sender");
    mvwprintw(_this->formwin,HDR_EXPSTARTLINE,FLAGFIELD_STARTCOL,"Flags");
    mvwprintw(_this->formwin,HDR_EXPSTARTLINE,LENFIELD_STARTCOL,"Len");
    mvwprintw(_this->formwin,PAYLOAD_STARTLINE,PAYLOADFIELD_STARTCOL,"Payload as <type>:<value>, type can be u8,u16,u32 or u64");
    */
    box(_this->dw,0,0);
    wprintw(_this->dw,"(%s)",DEFFIND_TOGGLE_STR);
}

static void handle_input(definitionfinder *_this, int ch)
{
    switch(ch)
    {
        case KEY_EVALUATE_DEFFORM:
            _this->display_matches(_this);
            break;
        case KEY_BACKSPACE:
            form_driver(_this->defform,REQ_PREV_CHAR);
            form_driver(_this->defform,REQ_DEL_CHAR);
            break;

        case BELOW_FIELD:
            form_driver(_this->defform,REQ_DOWN_FIELD);
            form_driver(_this->defform,REQ_END_LINE);
            break;
        case UPPER_FIELD:
            form_driver(_this->defform,REQ_UP_FIELD);
            form_driver(_this->defform,REQ_END_LINE);
            break;
        case NEXT_FIELD:
            form_driver(_this->defform,REQ_NEXT_FIELD);
            form_driver(_this->defform,REQ_END_LINE);
            break;
        case PREV_FIELD:
            form_driver(_this->defform,REQ_PREV_FIELD);
            form_driver(_this->defform,REQ_END_LINE);
            break;
        default:
            form_driver(_this->defform,ch);
            break;
    }
}

definitionfinder * init_definitionfinder(WINDOW* dw)
{
    definitionfinder *_this;
    int id=0;
    char *home=getenv("HOME");
    char ptr[4096];
    ptr[4095]='\0';
    _this=calloc(1,sizeof(definitionfinder));
    if(_this)
    {
        _this->dw=dw;
        _this->display_form=&display_form;
        _this->create_form=&create_form;
        _this->add_form_values=&add_form_values;
        _this->display_matches=&display_matches;
        _this->handle_input=&handle_input;
    }
    if(home)
    {
        snprintf(ptr,4095,"%s%s",home,"/.nibbles/Interface");
        parse_ids(_this,ptr,&id,&get_tasks);
        _this->last_taskid=id;
        DEBUGPR("parsed %d taskIds\n",id);
        id=0;
        snprintf(ptr,4095,"%s%s",home,"/.nibbles/Interface");
        parse_ids(_this,ptr,&id,&get_structs);
        _this->last_struct=id;
        DEBUGPR("parsed %d structss\n",id);
    }
    else
    {
        parse_ids(_this,"Interface",&id,&get_tasks);
        _this->last_taskid=id;
        DEBUGPR("parsed %d taskIds\n",id);
        id=0;
        parse_ids(_this,"Interface",&id,&get_structs);
        _this->last_struct=id;
        DEBUGPR("parsed %d structss\n",id);
    }
    return _this;
}

