static char sccsid[] = "@(#)48	1.14.1.3  src/bos/usr/ccs/lib/libc/NLxcol.c, libcnls, bos411, 9428A410j 3/11/94 10:55:10";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NLxcol, NCxcol, NLxcolu, NCxcolu
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdlib.h>

/*
 * NAME: _NLxcol
 *
 * FUNCTION: Find  collating value or replacement string for character
 *           beginning at str.                                  
 *
 * RETURN VALUE DESCRIPTION: -1 if 1-to-n collation is used. Otherwise,
 *                           return collating value. 
 */
/*  Find collating value or replacement string for character beginning
 *  at str.  If no replacement string exists, return collating value and
 *  set wrstr to NULL.  Else return -1 and set wrstr to point to
 *  replacement string.
 */
int _NLxcol(int pc, unsigned char **str, wchar_t **wrstr)
{
    int uniq;
    return(_NLxcolu(pc, str, wrstr, &uniq));
}

/*
 * NAME: _NCxcol
 *
 * FUNCTION: Find  collating value or replacement character for character
 *           beginning at str.                                  
 *
 * NOTE: Like _NLxcol, but interprets an input string of type wchar_t
 *       instead of char.
 * 
 * RETURN VALUE DESCRIPTION: -1 if 1-to-n collation is used. Otherwise,
 *                           return collating value. 
 */
int _NCxcol(int pc, wchar_t **wstr, wchar_t **wrstr)
{
    int uniq;
    return(_NCxcolu(pc, wstr, wrstr, &uniq));
}
 

/*
 * NAME: _NLxcolu
 *
 * FUNCTION: Find  collation value and unique collation value for character 
 * beginning at str.
 *
 * RETURN VALUE DESCRIPTION: -1 if 1-to-n collation is used. Otherwise,
 *                           return colluniq value. 
 */
/*  Find collating value and unique collation value or replacement string 
 *  for character beginning at str.
 *  If no replacement string exists, return collation value and
 *  set wrstr to NULL.  Else return -1 and set wrstr to point to
 *  replacement string.
 */
int _NLxcolu(int pc, unsigned char **str, wchar_t **wrstr, wchar_t *uniq)
{
    int subs;
    int colval;
    wchar_t wc=0x00;
    int rc;
    int len;
    int mb_len;
    int done;
    char *wgt_str=NULL;
    char *dummy;

    *wrstr = 0x00;
    *uniq = 0x00;
    
    if (pc < 0)
	pc *= -1;
    
/* see comments about 1-to-many mapping after #endif */
#if 0
    /**********
      check for sub strings
    **********/
    for (subs=0; subs< __OBJ_DATA(__lc_collate)->co_nsubs; subs++) {
	/**********
	  check if this sub is a regular expression, if not,
	  see if the first character matches
	**********/
	if (__OBJ_DATA(__lc_collate)->co_nord < 2) {
	    if (__OBJ_DATA(__lc_collate)->co_subs[subs].ss_act.n[0] 
		& _SUBS_REGEXP)
		continue;
	}
	else {
	    if (__OBJ_DATA(__lc_collate)->co_subs[subs].ss_act.p[0] 
		& _SUBS_REGEXP)
		continue;
	}

	if ((mb_len=mbtowc(&wc, __OBJ_DATA(__lc_collate)->co_subs[subs].ss_src, 
			   MB_CUR_MAX)) == -1)
	    return(0);
	if ((pc == wc) &&
	   (strncmp(&__OBJ_DATA(__lc_collate)->co_subs[subs].ss_src[mb_len],*str,
           (len = strlen(&__OBJ_DATA(__lc_collate)->co_subs[subs].ss_src[mb_len]))) == 0)
	){
	    /**********
	      get the space for the wchars
	    **********/
	    *str += len;
	    if((*wrstr=(wchar_t *)malloc((len=strlen(__OBJ_DATA(__lc_collate)->co_subs[subs].ss_tgt) *
					 sizeof(wchar_t) + 1))) == (wchar_t *)NULL)
		return(0);

	    (void)mbstowcs(*wrstr,__OBJ_DATA(__lc_collate)->co_subs[subs].ss_tgt,
			   len);
	    return(-1);
	}
    }

#endif

/* we really need to return a 1-to-many mapping string in wrstr.  */
/* however this information is not available in 4.1 locales, the  */
/* best we can do is to return a string of collation weights,     */
/* however, since this would confuse calling programs (which      */
/* we have no control over and can not fix) we are just returning */
/* the first weight in a possible 1-to-many mapping.              */

    /**********
      not part of a replacement string, check for collation elements
    **********/
    uniq = _mbucoll(__lc_collate, str, &dummy);
    *str += _getcolval(__lc_collate, &colval, pc, *str, 0,&wgt_str, &done);

    return(colval);
}

/*
 * NAME: _NCxcolu
 *
 * FUNCTION: Find collating value and unique collation value, or replacement 
 * character for character beginning at str.                                  
 *
 * NOTE: Like _NLxcolu, but interprets an input string of type wchar_t
 *       instead of char.
 * 
 * RETURN VALUE DESCRIPTION: -1 if 1-to-n collation is used. Otherwise,
 *                           return collation value. 
 */
int _NCxcolu(int pc, wchar_t **wstr, wchar_t **wrstr, wchar_t *uniq)
{
    int subs;
    int colval;
    wchar_t wc=0x00;
    char *strptr;
    char *str;
    int len;
    int count=0;
    int rc;

    *wrstr = 0x00;
    *uniq = 0x00;
    /**********
      Get the space for the mb string
    **********/
    if ((str=(char *)malloc((len=wcslen(*wstr)*MB_CUR_MAX))) == (char *)NULL)
	return(0);

    /**********
      convert the input wchar str to mb
    **********/
    if (wcstombs(str, *wstr, len) == -1) {
	free(str);
	return(0);
    }

    /**********
      get the value
    **********/
    strptr = str;
    colval = _NLxcolu(pc, &str, wrstr,uniq);
    /********
      set the wchar string to the value after
      the collation characters
    **********/
    for (strptr=strptr; strptr < str && *strptr;) {
	if ((rc = mbtowc(&wc, strptr, MB_CUR_MAX)) == -1) {
	    free(str);
	    return(0);
	}
	count++;
	strptr += rc;
    }
    *wstr += count;
    return(colval);
}
