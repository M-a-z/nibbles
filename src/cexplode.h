
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

#ifndef MVA_CEXPLODE_H
#define MVA_CEXPLODE_H

/* Some Cexplode calls support using this special item define */
#define CEXPLODE_LAST_ITEM 0xFFFFFFFF

#include <sys/types.h>

/**
 * @brief Struct for Cexplode object
 */
typedef struct CexplodeStrings
{
    int amnt;
    char **strings;
	char *separator;
	int sepwasatend;
	int startedWdelim;
	int index;
}CexplodeStrings;

/**
 * @brief enumeration for Cexplodei's error return values
 */
typedef enum ECexplodeRet
{
    ECexplodeRet_InternalFailure    = -666,
    ECexplodeRet_InvalidParams         = -667
}ECexplodeRet;

/**
* @brief Removes the previously returned piece
*
* Must not be called before calling Cexplode 
* If removed item is last piece, the "sepwasatend" flag will be set true
*
* @param CexplodeStrings *exp_obj pointer to CexplodeStrings type object, filled by call to Cexplode()
* @return ptr to string being removed at success, NULL at failure 
* @see Cexplode, Cexplode_removeNth, Cexplode_getAmnt, Cexplode_nextexists
*/
char *Cexplode_removeCurrent(CexplodeStrings *exp_obj);

/**
 * @brief Removes Nth piece from cexplode 
 * Must not be called before calling Cexplode 
 * If removed item is last piece, the "sepwasatend" flag will be set true! 
 * Note, you can use special CEXPLODE_LAST_ITEM define to remove the last item 
 *
 * @param int nro number of exploded piece to be removed from the CexplodeStrings containing results
 * @param CexplodeStrings *exp_obj pointer to CexplodeStrings type object, filled by call to Cexplode()
 * @return ptr to removed string 
 * @see Cexplode, Cexplode_removeCurrent, Cexplode_getAmnt, Cexplode_nextexists
 */
char *Cexplode_removeNth(int nro,CexplodeStrings *exp_obj);

/**
 * @brief Get the amount of pieces in exploded object 
 * Must not be called before calling Cexplode
 *
 * @param CexplodeStrings *exp_obj pointer to CexplodeStrings type object, filled by call to Cexplode()
 * @return amount of exploded pieces stored in CexplodeStrings container
 * @see Cexplode
 */
int Cexplode_getAmnt(CexplodeStrings exp_obj);

/**
 * @brief Explodes string to pieces according to delimiter. Result is stored in exp_obj and can be retrieved using functions below 
 * The results of explosion are stored in same order as they occurred in initial string, eg. if string "1 2 3 4" 
 * would be exploded with space (" ") as delimiter, Cexplode_getfirst() would return 1, Cexplode_getNth() with n being 4, would return 4.
 *
 * @param const char *string pointer to C string being exploded
 * @param const char *delim pointer to C string used as delimiter for cutting original string
 * @param CexplodeStrings *exp_obj pointer to CexplodeStrings type object, which will be filled to contain results of explosion.
 * @return amount of pieces - number smaller than 1 if an error occurs 
 * @see CexplodeStrings, Cexplode_removeCurrent, Cexplode_removeNth, Cexplode_getAmnt, Cexplode_nextexists, Cexplode_getNth, Cexplode_getfirst, Cexplode_getnext, Cexplode_getlast, Cexplode_free, Cexplode_free_allButPieces, Cexplode_getlentilllast, Cexplode_sepwasatend, Cexplode_concat
 *
 */
int Cexplode(const char *string,const char *delim, CexplodeStrings *exp_obj );

/**
 * @brief Peeks if there's another result in exp_obj. 
 * Must not be called before calling Cexplode 
 *
 * @param CexplodeStrings exp_obj CexplodeStrings type object, filled by call to Cexplode()
 * @return 1 if next piece exists (Eg. if Cexplode_getnext et al. can be safely used), 0 if there's no next result in object.
 * @see Cexplode, Cexplode_getnext
 * */ 
int Cexplode_nextexists(CexplodeStrings exp_obj);

/**
 * @brief Retrieve's Nth exploded piece - first is first (index starts from 1, not from 0)
 * Updates internal iterator, IE following call to Cexplode_getnext will retrieve index+1th piece
 * @param int index index number of result to be retrieved. first is first (index starts from 1, not from 0)
 * @param CexplodeStrings *exp_obj pointer to CexplodeStrings type object, filled by call to Cexplode()
 * @return NULL on error, othervice a pointer to result stored in Cexplode object
 * @warning Must not be called before calling Cexplode 
 * @see Cexplode, Cexplode_getfirst, Cexplode_getnext, Cexplode_getlast, Cexplode_getAmnt
 */
char *Cexplode_getNth(int index,CexplodeStrings *exp_obj);

/**
 * @brief Get's the first exploded piece. Same as Cexplode_getNth(1,*exp_obj);
 * @param CexplodeStrings *exp_obj pointer to CexplodeStrings type object, filled by call to Cexplode()
 * @return NULL on error, othervice a pointer to result stored in Cexplode object
 * @warning Must not be called before calling Cexplode
 * @see Cexplode, Cexplode_getNth, Cexplode_getnext, Cexplode_getlast
 */
char *Cexplode_getfirst(CexplodeStrings *exp_obj);

/**
 * @brief Get's next piece. Returns NULL if no more pieces are around 
 * @param CexplodeStrings *exp_obj pointer to CexplodeStrings type object, filled by call to Cexplode()
 * @return NULL on error, othervice a pointer to result stored in Cexplode object
 * @warning Must not be called before calling Cexplode 
 * @see Cexplode, Cexplode_getNth, Cexplode_getfirst, Cexplode_getlast
 */
char *Cexplode_getnext(CexplodeStrings *exp_obj);
/**
 * @brief Gets last exploded piece 
 * @param CexplodeStrings *exp_obj pointer to CexplodeStrings type object, filled by call to Cexplode()
 * @return NULL on error, othervice a pointer to result stored in Cexplode object
 * @warning Must not be called before calling Cexplode
 * @see Cexplode, Cexplode_getNth, Cexplode_getnext, Cexplode_getfirst
 */
char *Cexplode_getlast(CexplodeStrings *exp_obj);

/**
 * @brief Frees resources allocated by call to Cexplode() - BEWARE frees also splitted pieces
 * @param CexplodeStrings exp_obj CexplodeStrings type object, filled by call to Cexplode()
 * @warning Must not be called before calling Cexplode 
 * @warning BEWARE frees also splitted pieces, in which the returned pointers by Cexplode_get* points.
 * @see Cexplode_free_allButPieces, Cexplode, Cexplode_getNth, Cexplode_getnext, Cexplode_getfirst, Cexplode_getlast
 * */
void Cexplode_free(CexplodeStrings exp_obj);

/**
 * @brief Frees resources allocated by call to Cexplode() - does not free splitted pieces
 * @param CexplodeStrings exp_obj CexplodeStrings type object, filled by call to Cexplode()
 * @warning Must not be called before calling Cexplode
 * @see Cexplode_free, Cexplode, Cexplode_getNth, Cexplode_getnext, Cexplode_getfirst, Cexplode_getlast
 */
void Cexplode_free_allButPieces(CexplodeStrings exp_obj);

/**
 * @brief Gets the amount of chars from the start of the original string to the beginning of last found delimiter
 * @param CexplodeStrings exp_obj CexplodeStrings type object, filled by call to Cexplode()
 * @return amount of chars from the start of the original string to the beginning of last found delimiter
 * @warning Must not be called before calling Cexplode 
 * @see Cexplode, Cexplode_sepwasatend
 * */
size_t Cexplode_getlentilllast(CexplodeStrings exp_obj);

/**
 * @brief returns 1 if last chars in original string were the separator - else returns 0
 * @param CexplodeStrings exp_obj CexplodeStrings type object, filled by call to Cexplode()
 * @return 1 if last chars in original string were the separator - else returns 0
 * @warning Must not be called before calling Cexplode 
 * @see Cexplode, Cexplode_getlentilllast
 */
int Cexplode_sepwasatend(CexplodeStrings exp_obj);

/**
 * @brief Concatenates two exp_objs into one. Modifies the first argument to contain new exp_obj.
 * Does not modify second argument 
 * @param CexplodeStrings *first pointer to CexplodeStrings type object, filled by call to Cexplode() to be combined with another CexplodeStrings object. This will contain new CexplodeStrings object holding results for both of the original CexplodeStrings objects.
 * @param CexplodeStrings *second ointer to CexplodeStrings type object, filled by call to Cexplode() to be combined with another CexplodeStrings object - this will not be modified during call.
 * @return the amount of pieces in new exp_obj - negative number upon error.
 * @warning Must not be called before calling Cexplode for both first and second argument.
 */
int Cexplode_concat(CexplodeStrings *first,CexplodeStrings *second);




#endif //MVA_CEXPLODE_H

