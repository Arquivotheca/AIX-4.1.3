static char sccsid[] = "@(#)53	1.1  src/bos/usr/lib/nls/loc/locale/zh_CN/__wctomb_euccn.c, ils-zh_CN, bos41B, 9504A 12/20/94 10:25:46";
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: __wctomb_euccn
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <stdlib.h>
#include <ctype.h>
#include <sys/errno.h>

#include "ucs_eucCN.h"
#include "eucCN_ucs.h"

/*
  Converts a process code to a string of characters for the IBM_eucCN codeset


  The algorithm for this conversion is:

  |  process code   |   s[0]    |   s[1]    |
  +-----------------+-----------+-----------+
  | 0x0000 - 0x007f | 0x00-0x7f |    --     |
  | 0x0080 - 0x04ff | 0xa1-0xfe | 0xa1-0xfe |
  | 0x2000 - 0x32ff | 0xa1-0xfe | 0xa1-0xfe |
  | 0x4e00 - 0x9fa8 | 0xa1-0xfe | 0xa1-0xfe |
  | 0xe000 - 0xe0a3 | 0xa2-0xa9 | 0xa1-0xfe |
  | 0xe0a4 - 0xe0a3 | 0xaa-0xaf | 0xa1-0xfe |
  | 0xe000 - 0xe291 | 0xf8-0xfe | 0xa1-0xfe |
  | 0xff00 - 0xfffd | 0xa1-0xfe | 0xa1-0xfe |
  +-----------------+-----------+-----------+

*/


int __wctomb_euccn(_LC_charmap_objhdl_t handle, char *s, wchar_t pwc)
{
    unsigned short indx;
    short start, end, middle;

    /**********
      if s is NULL or points to a NULL return 0
    **********/
    if (s == (char *)NULL)
        return(0);

    /**********
      Single Byte PC <= 7f
    **********/

    if (pwc <= 0x7f) {
        s[0] = (char) pwc;
        return(1);
    }

    /**********
      Double byte 0x4e00 - 0x9fa8, GB characters in CJK 
      Including the seven specially imlementated characters
    **********/
    else if ((pwc >= 0x4e00) && (pwc <= 0x9fa8)) {
	indx = (unsigned short)( pwc - 0x4e00 );
	*(unsigned short *)s = ucs_gb[indx];

	if( *(unsigned short *)s == 0x0000) {
    	    errno = EILSEQ;
            return(-1);
	}
	else  return(2);
    }

    /**********
      Double byte 0x0080 - 0x04ff
		  0x2000 - 0x32ff
		  0xff00 - 0xfffd
      UCS codes map to Non Chinese glyphs in GB and IBM-sbdCN character set.
    **********/

    else if ((pwc >= 0x0080 && pwc <= 0x04ff) || (pwc >= 0x2000 && pwc <= 0x32ff) || (pwc >= 0xff00 && pwc <= 0xfffd))	{
	start = N_C_HEADER;
	end = N_C_TRAILER;
	middle = ( end + start ) / 2;
	
 	while((pwc != non_Chinese_ucs_gb[middle][0]) && ( start <= end)) {
	    if(pwc > non_Chinese_ucs_gb[middle][0]) {
		start = middle + 1;
		middle = (start + end) / 2;
	    }
	    else if(pwc < non_Chinese_ucs_gb[middle][0]) {
		end = middle - 1;
		middle = ( end + start ) / 2;
	    }
	}
	
    	/**********
      	otherwise it is an invaild process code
    	**********/
	if(start > end)  {
    		errno = EILSEQ;
    		return(-1);
	}
	else  {
		*(unsigned short *)s = non_Chinese_ucs_gb[middle][1];
		return(2);
	}
    }
	
    /**********
      Double byte 0xe000 - 0xe56e
    **********/
    else if ((pwc >= 0xe000) && (pwc <= 0xe0a3)) {
	indx = (unsigned short)(pwc - 0xe000);
	*(unsigned short *)s = udc_index[indx]; 
	return(2);
    }

    else if ((pwc >= 0xe0a4) && (pwc <= 0xe2d7)) {
   	s[0] = (char) (((pwc - 0xe0a4) / 94 ) + 0x00aa);
        s[1] = (char) (((pwc - 0xe0a4) % 94 ) + 0x00a1);
        return(2);
    }

    else if ((pwc >= 0xe2d8) && (pwc <= 0xe2dc)) {
  	*(unsigned short *)s = 0xd7fa + (pwc - 0xe2d8);
	return(2);
    }

    else if ((pwc >= 0xe2dd) && (pwc <= 0xe54f)) {
   	s[0] = (char) (((pwc - 0xe2dd) / 94 ) + 0x00f8);
        s[1] = (char) (((pwc - 0xe2dd) % 94 ) + 0x00a1);
        return(2);
    }
    else if( pwc == 0x0d17 ) {
	s[0] = 0xa1; s[1] = 0xd7; return(2);
    }

    /**********
      otherwise it is an invaild process code
    **********/
    errno = EILSEQ;
    return(-1);

}
