static char sccsid[] = "@(#)55	1.4  src/bos/usr/ccs/lib/libc/mbswidth.c, libcnls, bos411, 9428A410j 6/30/93 11:38:55";
/*
 * COMPONENT_NAME: (LIBCCHR) LIBC Character Classification Funcions
 *
 * FUNCTIONS: mbswidth
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <mbstr.h>
#include <stdlib.h>

/*
 *
 * FUNCTION: mbswidth
 *	    
 *
 * PARAMETERS: 
 *
 *
 * RETURN VALUE: 
 *
 *
 */

int mbswidth(const char *s, size_t n)
{
    int len;
    wchar_t *wcs;
    int rc;
    char *str_ptr;
    char wcsbuf[512];
    char strbuf[256];

    if ((s == (char *)NULL) || (*s == '\0'))
	return ((int)NULL);

    /**********
      get the space for the process code.  There cannot be more process
      codes than characters
    **********/
    if ((n + 1) * sizeof(wchar_t) <= sizeof(wcsbuf)) {
	wcs = wcsbuf;
    }
    else if ((wcs = (wchar_t *) malloc ((n+1) * sizeof(wchar_t))) == (wchar_t *)NULL) {
	perror("mbswidth:malloc");
	return ((int)NULL);
    }

    /**********
      get space for a temp string
    **********/
    if ((n + 1) * sizeof(char) <= sizeof (strbuf)) {
	str_ptr = strbuf;
    }
    else if ((str_ptr = (char *) malloc (n+1)) == (char *)NULL) {
	perror("mbswidth:malloc");
	return((int)NULL);
    }

    /**********
      copy s into the temp string
    **********/
    strncpy(str_ptr, s, n);
    str_ptr[n] = '\0';

    rc = mbstowcs(wcs, str_ptr, n+1);

    /**********
      was there an invalid character found
    **********/ 
    if (rc == -1)
	len = -1;
    else
	len = wcswidth(wcs, rc+1);

    /*********
      free up the malloced space
    ********/
    if (wcs != wcsbuf)
	free(wcs);
    if (str_ptr != strbuf)
	free(str_ptr);

    return(len);
}
	    

    
    
    
    
