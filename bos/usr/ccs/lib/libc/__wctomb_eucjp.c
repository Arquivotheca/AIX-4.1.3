static char sccsid[] = "@(#)56	1.3.1.1  src/bos/usr/ccs/lib/libc/__wctomb_eucjp.c, libccppc, bos411, 9428A410j 5/25/92 13:44:25";
/*
 * COMPONENT_NAME: (LIBCCCPPC) LIBC Code-Point/Process-Code Conversion Functions
 *
 * FUNCTIONS: __wctomb_eucjp
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
#include <sys/errno.h>
/*
  Converts a process code to a string of characters for the JP_EUC codeset

  The algorithm for this conversion is:
  PC <= 0x009f:                 s[0] = PC
  PC >= 0x0100 and PC <=0x015d: s[0] = 0x8e
                                s[1] = PC - 0x005f
  PC >= 0x015e and PC <=0x303b: s[0] = ((PC - 0x015e) >> 7) + 0xa1
                                s[1] = ((PC - 0x015e) & 0x007f) + 0xa1
  PC >= 0x303c and PC <=0x5f19: s[0] = 0x8f
                                s[1] = ((PC - 0x303c) >> 7) + 0xa1
                                s[2] = ((PC - 0x303c) & 0x007f) + 0xa1

  |  process code   |   s[0]    |   s[1]    |   s[2]    |
  +-----------------+-----------+-----------+-----------+
  | 0x0000 - 0x009f | 0x00-0x9f |    --     |    --     |
  | 0x00a0 - 0x00ff |   --      |    --     |    --     |
  | 0x0100 - 0x015d | 0x8e      | 0xa1-0xfe |    --     |
  | 0x015e - 0x303b | 0xa1-0xfe | 0xa1-0xfe |    --     |
  | 0x303c - 0x5f19 | 0x8f      | 0xa1-0xfe | 0xa1-0xfe |
  +-----------------+-----------+-----------+-----------+

*/

int __wctomb_eucjp(_LC_charmap_objhdl_t hdl, char *s, wchar_t pwc) 
{
    /**********
      if s is NULL or points to a NULL return 0
    **********/
    if (s == (char *)NULL)
	return(0);

    /**********
      Single Byte PC <= 9f
    **********/

    if (pwc <= 0x9f) {
	s[0] = (char) pwc;
	return(1);
    }

    /**********
      Double byte: 0x0100 - 0x015d
    **********/
    else if ((pwc >= 0x0100) && (pwc <= 0x015d)) {
	s[0] = 0x8e;
	s[1] = pwc - 0x5f;
	return(2);
    }

    /**********
      Double byte 0x015e - 0x303b
    **********/
    else if ((pwc >=0x015e) && (pwc <= 0x303b)) {
	s[0] = (char) (((pwc - 0x015e) >> 7) + 0x00a1);
	s[1] = (char) (((pwc - 0x015e) & 0x007f) + 0x00a1);
	return(2);
    }
    /**********
      Triple byte 0x303c - 0x5f19
    **********/
    else if ((pwc >=0x303c) && (pwc <= 0x5f19)) {
	s[0] = 0x8f;
	s[1] = (char) (((pwc - 0x303c) >> 7) + 0x00a1);
	s[2] = (char) (((pwc - 0x303c) & 0x007f) + 0x00a1);
	return(3);
    }

    /**********
      otherwise it is an invaild process code
    **********/
    errno = EILSEQ;
    return(-1);
}
