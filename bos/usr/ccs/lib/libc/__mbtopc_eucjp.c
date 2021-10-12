static char sccsid[] = "@(#)57	1.3.1.1  src/bos/usr/ccs/lib/libc/__mbtopc_eucjp.c, libccppc, bos411, 9428A410j 5/25/92 13:43:35";
/*
 * COMPONENT_NAME: (LIBCCCPPC) LIBC Code-Point/Process-Code Conversion Functions
 *
 * FUNCTIONS: __mbtopc_eucjp
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991 , 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*
 *
 * FUNCTION: 
 *	    
 *
 * PARAMETERS: 
 *
 *
 * RETURN VALUE: 
 *
 *
 */
#include <stdlib.h>
#include <ctype.h>
/*
  Converts a multi-byte string to process code for the JP_EUC codeset

  The algorithm for this conversion is:
  s[0] < 0x9f:  PC = s[0]
  s[0] = 0x8e:  PC = s[1] + 0x5f;
  s[0] = 0x8f   PC = (((s[1] - 0xa1) << 7) | (s[2] - 0xa1) + 0x303c)
  s[0] > 0xa1   PC = (((s[0] - 0xa1) << 7) | (s[1] - 0xa1) + 0x15e)

  |  process code   |   s[0]    |   s[1]    |   s[2]    |
  +-----------------+-----------+-----------+-----------+
  | 0x0000 - 0x009f | 0x00-0x9f |    --     |    --     |
  | 0x00a0 - 0x00ff |   --      |    --     |    --     |
  | 0x0100 - 0x015d | 0x8e      | 0xa1-0xfe |    --     |
  | 0x015e - 0x303b | 0xa1-0xfe | 0xa1-0xfe |    --     |
  | 0x303c - 0x5f19 | 0x8f      | 0xa1-0xfe | 0xa1-0xfe |
  +-----------------+-----------+-----------+-----------+
  
  This algorithm compresses all of code points to process codes less
  than 0x5f19.
*/

size_t __mbtopc_eucjp(_LC_charmap_objhdl_t hdl, wchar_t *pwc, char *s, size_t maxlen, 
		      int *err)
{
    wchar_t dummy;

    /**********
      if s is NULL or points to a NULL return 0
    **********/
    if (s == (char *)NULL)
	return(0);

    /**********
      if pwc is null, then set it to dummy
    **********/
    if (pwc == (wchar_t *)NULL)
	pwc = &dummy;
    
    /**********
      assume it is a bad character
    **********/
    *err = 0;
    
    /**********
      single byte (<=0x8d)
    **********/
    if ((unsigned char)s[0] <= 0x8d) {
	if (maxlen < 1) {
	    *err = 1;
	    return(0);
	}
	else {
	    *pwc = (wchar_t) s[0];
	    return(1);
	}
    }

    /**********
      Double Byte 8e[a1-fe]
    **********/
    else if ((unsigned char)s[0] == 0x8e) {
	if (maxlen >= 2) {
	    if ((unsigned char)s[1] >=0xa1 && (unsigned char)s[1] <=0xfe) {
	        *pwc = (wchar_t) ((unsigned char)s[1] + 0x5f);
	        return(2);
	    }
	}
	else {
	    *err = 2;
	    return(0);
	}
    }
    /**********
      Triple Byte 8f[a1-fe][a1-fe]
    **********/
    else if ((unsigned char)s[0] == 0x8f) {
	if (maxlen >= 3) {
	    if (((unsigned char)s[1] >=0xa1 && (unsigned char)s[1] <=0xfe) &&
		((unsigned char)s[2] >=0xa1 && (unsigned char)s[2] <= 0xfe)) {
	        *pwc = (wchar_t) ((((unsigned char)s[1] - 0xa1) << 7) |
		       (wchar_t) ((unsigned char)s[2] - 0xa1)) + 0x303c;
	        return(3);
	    }
	}
	else {
	    *err = 3;
	    return(0);
        }
    }

    /**********
      Single Byte [90-9f]
    **********/
    else if ((unsigned char)s[0] <= 0x9f) {
	if (maxlen < 1) {
	    *err = 1;
	    return(0);
	}
	else {
	    *pwc = (wchar_t) s[0];
	    return(1);
	}
    }
    
    /**********
      Double Byte [a1-fe][a1-fe]
    **********/
    else if ((unsigned char)s[0] >= 0xa1 && (unsigned char)s[0] <= 0xfe) {
	if (maxlen >= 2) {
	    if  ((unsigned char)s[1] >=0xa1 && (unsigned char)s[1] <= 0xfe) {
		*pwc = (wchar_t) ((((unsigned char)s[0] - 0xa1) << 7) |
		       (wchar_t) ((unsigned char)s[1] - 0xa1)) + 0x15e;
		return(2);
	    }
	}
	else {
	    *err = 2;
	    return(0);
	}
    }

    /**********
      If we are here invalid mb character
    **********/
    *err = -1;
    return(0);
    
}
    
		



