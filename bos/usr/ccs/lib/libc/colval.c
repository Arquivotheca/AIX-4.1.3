static char sccsid[] = "@(#)68	1.4.2.7  src/bos/usr/ccs/lib/libc/colval.c, libcstr, bos41J, 9513A_all 3/27/95 16:28:13";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: _getcolval, _mbucoll, _mbce_lower
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#pragma alloca

#define _ILS_MACROS

#include <sys/limits.h>
#include <sys/localedef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/************************************************************************/
/*  _getcolval - determine nth collation weight of collating symbol	*/
/*									*/
/* one-two-many mapping:						*/
/*	Determine if this is the first time for this character		*/
/*	If so, return length of collation element, and the first (and	*/
/*	possibly only) collating weight for this order.  If this order  */
/*	has a one-to-many mapping set done=0 else done=1.  If this      */
/* 	routine is called again, return next collating weight if one to */
/*	many mapping.							*/
/*									*/
/************************************************************************/



int _getcolval(_LC_collate_objhdl_t hdl, int *colval, wchar_t wc, char *str, int order, char **wgt_str, int *done)
{

    int i;
    int count;
    int index;
    unsigned short * wgt_ptr;

    if (*wgt_str) { /* if not first time */
	*colval = *(unsigned short *)*wgt_str;
	if ((*wgt_str)[sizeof(short)]==NULL) {
	    *done=1;
	    *wgt_str=NULL; /* get ready for next time */
	    }
	else
	    *wgt_str+=sizeof(short);

	return (0);
	}
		
    wgt_ptr = (unsigned short *)&(__OBJ_DATA(hdl)->co_coltbl[wc].ct_wgt.n);

    /* the code that follows will be executed the first time */
    /* if the caller passed used _getcolval properly, ie:    */
    /*		int done;				     */
    /*		char *wgt=NULL;				     */
    /*           _getcolval(...,&wgt,&done);                 */

    /**********
      get the collation value for the character (wc)
    **********/
	if (*wgt_ptr == _SUB_STRING) {/* 1-many ? */
	    index = wgt_ptr[1];
	    *wgt_str = __OBJ_DATA(hdl)->co_subs[index].tgt_wgt_str[order];
	    *colval = *(unsigned short *)*wgt_str;
	    if ((*wgt_str == NULL) || ((*wgt_str)[sizeof(short)]==0)) {
		*done=1;
		*wgt_str = NULL;
		}
            else {
	        *wgt_str+=sizeof(short);
		*done=0;
		}
        }
	else { /* normal case, no 1-many mapping */
	    *colval = wgt_ptr[order];
	    *done = 1;
	    }

    /**********
      check if there are any collation elements for this character
    **********/
     if (__OBJ_DATA(hdl)->co_coltbl[wc].ct_collel != (_LC_collel_t *)NULL) {

	 i = 0;
	 while (__OBJ_DATA(hdl)->co_coltbl[wc].ct_collel[i].ce_sym != (uchar *)NULL) {
	    /**********
	      get the length of the collation element that starts with
	      this character
	    **********/
	    count = strlen(__OBJ_DATA(hdl)->co_coltbl[wc].ct_collel[i].ce_sym);
	    
	    /**********
	      if there is a match, get the collation elements value and
	      return the number of characters that make up the collation
	      value
	    **********/
	    if (strncmp(str,__OBJ_DATA(hdl)->co_coltbl[wc].ct_collel[i].ce_sym, count) == 0) {

		 if (*(__OBJ_DATA(hdl)->co_coltbl[wc].ct_collel[i].ce_wgt.n) == _SUB_STRING) {/* 1-many ? */
		      index = __OBJ_DATA(hdl)->co_coltbl[wc].ct_collel[i].ce_wgt.n[1];
		      *wgt_str = __OBJ_DATA(hdl)->co_subs[index].tgt_wgt_str[order];
		      *colval = *(unsigned short *)*wgt_str;
		      if ((*wgt_str == NULL) || ((*wgt_str)[sizeof(short)]==0)) {
			  *done=1;
			  *wgt_str = NULL;
			  }
		      else {
			  *wgt_str+=sizeof(short);
			  *done=0;
			  }
		  }
		  else { /* normal case, no 1-many mapping */
		      *colval = __OBJ_DATA(hdl)->co_coltbl[wc].ct_collel[i].ce_wgt.n[order];
		      *done = 1;
		      }

	      return(count);
	  }
	    
	    /**********
	      This collation element did not match, go to the next
	    **********/
	    i++;
	}
    }
    /**********
      no collation elements, or none that matched, return
      0 additional characters.
    **********/
    return(0);
}


#if 0

/**********
  get collation value for wchar_t
**********/
#define STR_BUF_SIZE 10*MB_LEN_MAX+1
int _wc_getcolval(_LC_collate_objhdl_t hdl, int *colval, wchar_t wc, wchar_t *wcs, int order, int *done)
{
/*****************************************************************************/
/* if this routine is needed it will have to be updated with changes similar */
/* to those made in _getcolval() to support one to many mapping, etc	     */
/*****************************************************************************/
    
    char strbuf[STR_BUF_SIZE];
    char *str;          /* mulit-byte string for wcs */
    char *str_ptr;
    int i;
    int j;
    int rc;
    int count;

    /**********
      get the collation value for the character (wc)
    **********/
    if (__OBJ_DATA(hdl)->co_nord < 2)
	*colval = __OBJ_DATA(hdl)->co_coltbl[wc].ct_wgt.n[order];
    else
	*colval = __OBJ_DATA(hdl)->co_coltbl[wc].ct_wgt.p[order];
    
    /**********
      check if there are any collation elements for this character
    **********/
     if (__OBJ_DATA(hdl)->co_coltbl[wc].ct_collel != (_LC_collel_t *)NULL) {

	 i = 0;
	 while (__OBJ_DATA(hdl)->co_coltbl[wc].ct_collel[i].ce_sym != (uchar *)NULL) {
	    /**********
	      get the length of the collation element that starts with
	      this character
	    **********/
	    count = strlen(__OBJ_DATA(hdl)->co_coltbl[wc].ct_collel[i].ce_sym);

	    /**********
	     get the space needed to convert 'count' number of characters of wcs
	     to a mb-string for the comparison
	    **********/
	    if (((count*MB_CUR_MAX) + 1) < STR_BUF_SIZE) 
		str = strbuf;
	    else {
	    	str = (char *) alloca((count * MB_CUR_MAX)  + 1);
	    	if (str == (char *)NULL) {
			perror("wcscoll");
			exit(-1);
	    	}
	    }

	    str_ptr = str;
	    for (j=0; j<count; j++) {
		rc = wctomb(str_ptr, *wcs);
		/***********
		  Hit a null
		**********/
		if (rc == 0)
		    continue;
		/**********
		  for an invalid character, assume it is one
		  character and continue
		**********/
		else if (rc == -1)
		    *str_ptr++ = (char)*wcs;
		else
		    str_ptr += rc;
		wcs++;
	    }
	    /**********
	      if there is a match, get the collation elements value and
	      return the number of characters that make up the collation
	      value
	    **********/
	    if (strncmp(str,__OBJ_DATA(hdl)->co_coltbl[wc].ct_collel[i].ce_sym, count) == 0) {
		if (__OBJ_DATA(hdl)->co_nord < 2)
		    *colval = __OBJ_DATA(hdl)->co_coltbl[wc].ct_collel[i].ce_wgt.n[order];
		else
		    *colval = __OBJ_DATA(hdl)->co_coltbl[wc].ct_collel[i].ce_wgt.p[order];
		return(count);
	    }
	    
	    /**********
	      This collation element did not match, go to the next
	    **********/
	    i++;
	}
    }
    /**********
      no collation elements, or none that matched, return
      0 additional characters.
    **********/
    return(0);
}

#endif


/************************************************************************/
/*  _mbucoll - determine unique collating weight of collating symbol	*/
/************************************************************************/

int _mbucoll(_LC_collate_objhdl_t phdl, char *str, char **next_char)
{
	int	ucoll;		/* collating symbol unique weight	*/
	int	wclen;		/* # bytes in first character		*/
	wchar_t	wc=0;		/* first character process code		*/
	int	dummy_done;	/* dummy return for _getcolval		*/
	char 	*dummy_wgt=NULL;/* dummy return for _getcolval		*/

	wclen = mbtowc(&wc, str, MB_CUR_MAX);
	if (wclen < 0)
		wc = *str++ & 0xff;
	else
		str += wclen;

	*next_char = str + _getcolval(phdl, &ucoll, wc, str, _UCW_ORDER, &dummy_wgt,&dummy_done);
	/* only need to call _getcolval once since we won't have one to many */
	/* mapping as unique collating weights.                              */
	return (ucoll);
}


/************************************************************************/
/* _mbce_lower    - convert multibyte collating element to lowercase	*/
/*                - return status indicating if old/new are different	*/
/*									*/
/*                - for each character in collating element		*/
/*                - convert from file code to proces code		*/
/*		  - convert process code to lower case			*/
/*		  - convert lower case process code back to file code	*/
/*		  - set status if upper/lower process code different	*/
/*                - add terminating NUL					*/
/************************************************************************/

int _mbce_lower(char *pstr, size_t n, char *plstr)
{
	char	*pend;		/* ptr to end of conversion string	*/
	int	stat;		/* return status			*/
	wchar_t	wc;		/* original string process code		*/
	wchar_t	wcl;		/* lowercase string process code	*/

	stat = 0;
	for (pend = pstr + n; pstr < pend;)
		{
		pstr += mbtowc(&wc, pstr, MB_CUR_MAX);
		wcl = towlower((wint_t)wc);
		plstr += wctomb(plstr, wcl);
		if (wcl != wc)
			stat++;
		}
	*plstr = '\0';
	return (stat);
}
