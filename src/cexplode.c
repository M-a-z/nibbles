
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

#include "cexplode.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "common.h"
//#define DEBUGPRINTS
#define CEXPLODE_PRINT DEBUGPR


int Cexplode_nextexists(CexplodeStrings exp_obj)
{
	return !(exp_obj.amnt<exp_obj.index+1);
}

char *Cexplode_getnext(CexplodeStrings *exp_obj)
{
	//char *tmp;
	if(NULL==exp_obj)
		return NULL;
	if(exp_obj->amnt<=exp_obj->index)
	{
#ifdef DEBUGPRINTS
		CEXPLODE_PRINT("Cexplode_getnext(): amnt%d, index%d => next would overflow\n",exp_obj->amnt,exp_obj->index);
#endif
		return NULL;
	}
	exp_obj->index++;
	return exp_obj->strings[exp_obj->index-1];
/*	tmp=Cexplode_getNth(exp_obj->index+1,exp_obj);
	return tmp;
*/
}
int Cexplode_getAmnt(CexplodeStrings exp_obj)
{
    return exp_obj.amnt;
}
char *Cexplode_removeCurrent(CexplodeStrings *exp_obj)
{
    return Cexplode_removeNth(exp_obj->index+1,exp_obj);
}

char *Cexplode_removeNth(int nro,CexplodeStrings *exp_obj)
{
    char *retval;
    if(CEXPLODE_LAST_ITEM==nro)
        nro=exp_obj->amnt;
    if( NULL==exp_obj || exp_obj->amnt>nro || nro<0 )
    {
#ifdef DEBUGPRINTS
        perror("Warning, invalid args to Cexplode_removeNth()");
#endif
        return NULL;
    }
    if(exp_obj->index>=nro)
        exp_obj->index--;
	if(nro==exp_obj->amnt)
		exp_obj->sepwasatend=1;
    retval=exp_obj->strings[nro-1];
    memmove(&(exp_obj->strings[nro-1]),&(exp_obj->strings[nro]),(exp_obj->amnt-nro)*sizeof(char *));
    exp_obj->amnt--;
    exp_obj->strings[exp_obj->amnt]=NULL;
    return retval;
}

char *Cexplode_getlast(CexplodeStrings *exp_obj)
{
	if(NULL==exp_obj)
		return NULL;
	return Cexplode_getNth(exp_obj->amnt,exp_obj);
}
//TODO: TestThis!
size_t Cexplode_getlentilllast(CexplodeStrings exp_obj)
{
	int i;
	size_t retval=0,seplen;
	if(exp_obj.amnt<2)
		return 0;
	seplen=strlen(exp_obj.separator);
	for
	(
		i=0; 
		i < 
			((exp_obj.sepwasatend)?
			 	exp_obj.amnt:
				exp_obj.amnt-1
			) 
		;i++
	)
	{
		retval+=strlen(exp_obj.strings[i])+seplen;
	}
	//if text started with delim (which was cropped off) => original lenght must be increased by one delimlen
	if(exp_obj.startedWdelim)
		retval+=seplen;
	//remove last seplen to get the place before last delimiter (is this what we want? No.)
//	retval-=seplen;
	return retval;
}
//TODO: test this!!
int Cexplode_sepwasatend(CexplodeStrings exp_obj)
{
	return exp_obj.sepwasatend;
}

int Cexplode_concat(CexplodeStrings *first,CexplodeStrings *second)
{
	size_t cpylen;
	size_t newamnt=first->amnt+second->amnt;
    int i;
	first->strings=realloc(first->strings,newamnt*sizeof(char *));
	if(NULL==first->strings)
	{
		perror("Cexplode_concat realloc FAILED!\n");
		return -666;
	}
	for(i=0;i<second->amnt;i++)
	{
		cpylen=strlen(second->strings[i])+1;
		first->strings[first->amnt+i]=malloc(cpylen);
		memcpy(first->strings[first->amnt+i],second->strings[i],cpylen);
	}
	first->amnt=newamnt;
	first->sepwasatend=second->sepwasatend;
	return newamnt;
}


int Cexplode(const char *string,const char *delim, CexplodeStrings *exp_obj )
{
    int stringL = 0;
    int delimL  = 0;
    int index;
    int pieces=0;
    int string_start=0;
    char **tmp=NULL;
	
    //Sanity Checks:
    if(NULL==string || NULL==delim || NULL == exp_obj)
    {
#ifdef DEBUGPRINTS
        perror("Invalid params given to Cexplode!\n");
#endif
        return ECexplodeRet_InvalidParams;
    }
	exp_obj->amnt=exp_obj->index=0;
	exp_obj->sepwasatend=0;
	exp_obj->startedWdelim=0;

    stringL = strlen(string);
    delimL  = strlen(delim);

	exp_obj->separator=malloc(delimL+1);
	if(exp_obj->separator==NULL)
	{
		CEXPLODE_PRINT("Malloc Failed at %s:%d tried %d bytes",__FILE__,__LINE__,delimL+1);
		return ECexplodeRet_InternalFailure;
	}
	memcpy(exp_obj->separator,delim,delimL);
	exp_obj->separator[delimL]='\0';
    if(delimL>=stringL)
    {
#ifdef DEBUGPRINTS
        CEXPLODE_PRINT("Delimiter longer than string => No pieces can be found! (returning original string)\n");
#endif
        tmp=malloc(sizeof(char *));
        if(NULL==tmp)
        {
        	perror("Cexplode: Malloc failed!\n");
            return ECexplodeRet_InternalFailure;
        }
        //alloc also for \0
        tmp[0]=malloc(sizeof(char *)*(stringL+1)); 
        if(NULL==tmp[0])
        {
            perror("Cexplode: Malloc failed!\n");
            return ECexplodeRet_InternalFailure;
        }
        memcpy(tmp[0],string,stringL+1); 
		exp_obj->amnt=1;
		exp_obj->strings=tmp;
		return 1;
    }

    for(index=0;index<stringL-delimL;index++)
    {
        if(string[index]==delim[0])
        {
            //Check if delim was actually found
            if( !memcmp(&(string[index]),delim,delimL) )
            {
                //token found
                //let's check if token was at the beginning:
                if(index==string_start)
                {
                    string_start+=delimL;
                    index+=delimL-1;
					exp_obj->startedWdelim=1;
                    continue;
                }
                //if token was not at start, then we should add it in CexplodeStrings
                pieces++;   
                if(NULL==tmp)
                    tmp=malloc(sizeof(char *));
                else
                    tmp=realloc(tmp,sizeof(char *)*pieces);
                if(NULL==tmp)
                {
                    perror("Cexplode: Malloc failed!\n");
                    return ECexplodeRet_InternalFailure;
                }
                //alloc also for \0
                tmp[pieces-1]=malloc(sizeof(char *)*(index-string_start+1)); 
                if(NULL==tmp[pieces-1])
                {
                    perror("Cexplode: Malloc failed!\n");
                    return ECexplodeRet_InternalFailure;
                }
                memcpy(tmp[pieces-1],&(string[string_start]),index-string_start); 

                tmp[pieces-1][index-string_start]='\0'; 
                string_start=index+delimL;
                index+=(delimL-1);
            }//delim found
        }//first letter in delim found from string
    }//for loop

    if(memcmp(&(string[index]),delim,delimL))
	{
    	index+=delimL;
	}
	else
	{
		//Token was last piece in string
		exp_obj->sepwasatend=1;
	}
    if(index!=string_start)
    {
		pieces++;
	    if(NULL==tmp)
	        tmp=malloc(sizeof(char *));
	    else
	        tmp=realloc(tmp,sizeof(char *)*pieces);
	    if(NULL==tmp)
	    {
	        perror("Cexplode: Malloc failed!\n");
	        return ECexplodeRet_InternalFailure;
	    }
	        tmp[pieces-1]=malloc(sizeof(char *)*(index-string_start+1));
	    if(NULL==tmp[pieces-1])
	    {
	        perror("Cexplode: Malloc failed!\n");
	        return ECexplodeRet_InternalFailure;
	    }
	    memcpy(tmp[pieces-1],&(string[string_start]),index-string_start);
	    tmp[pieces-1][index-string_start]='\0'; //MazFix 1
    }
    exp_obj->amnt=pieces;
    exp_obj->strings=tmp;
    return pieces;
}


char *Cexplode_getNth(int index,CexplodeStrings *_exp_obj)
{
	if(_exp_obj->amnt==0)
	{
#ifdef DEBUGPRINTS
		CEXPLODE_PRINT("Cexplode_getNth: amnt = 0\n");
#endif
		return NULL;
	}
    if(_exp_obj->amnt<index)
    {
        return NULL;
    }
	_exp_obj->index=index;
    return _exp_obj->strings[index-1];
}

char *Cexplode_getfirst(CexplodeStrings *exp_obj)
{
    return Cexplode_getNth(1,exp_obj);
}

void Cexplode_free_allButPieces(CexplodeStrings exp_obj)
{
    //int i=0;
	
	exp_obj.sepwasatend=0;
	exp_obj.startedWdelim=0;
	exp_obj.index=0;
	if(NULL!=exp_obj.separator)
		free(exp_obj.separator);
	if(NULL!=exp_obj.strings)
    	free(exp_obj.strings);
	exp_obj.amnt=0;
	exp_obj.separator=NULL;
	exp_obj.strings=NULL;
}

void Cexplode_free(CexplodeStrings exp_obj)
{
    int i=0;
	
	exp_obj.sepwasatend=0;
	exp_obj.startedWdelim=0;
	exp_obj.index=0;
    for(;i<exp_obj.amnt;i++)
        free(exp_obj.strings[i]);
	if(NULL!=exp_obj.separator)
		free(exp_obj.separator);
	if(NULL!=exp_obj.strings)
    	free(exp_obj.strings);
	exp_obj.amnt=0;
	exp_obj.separator=NULL;
	exp_obj.strings=NULL;
}


