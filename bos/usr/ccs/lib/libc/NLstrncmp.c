static char sccsid[] = "@(#)98	1.2.1.2  src/bos/usr/ccs/lib/libc/NLstrncmp.c, libcnls, bos411, 9428A410j 3/10/94 11:24:21";
/*
 * COMPONENT_NAME: (LIBCNLS) LIBC National Language Support
 *
 * FUNCTIONS: NLstrncmp
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 , 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
#include <sys/localedef.h>
#include <stdlib.h>
#include <string.h>

/*
 *
 * FUNCTION: NLstrncmp
 *	    
 *
 * PARAMETERS: 
 *
 *
 * RETURN VALUE: 
 *
 *
 */
int NLstrncmp(const char *str1, const char *str2, int n)
{
    char *str1_ptr;
    char *str2_ptr;
    int  rc;

    /**********
      malloc the space for the temp strings
    **********/
    str1_ptr = (char *) malloc(n+1);
    str2_ptr = (char *) malloc(n+1);

    if (str1_ptr == (char *)NULL || str2_ptr == (char *)NULL ) {
	perror(NLstrncmp);
	return(0);
    }

    /*********
      copy only the first n characters
      into the temp strings
    **********/
    strncpy(str1_ptr, str1, n);
    strncpy(str2_ptr, str2, n);

    /**********
      always add the terminating NULL
    **********/
    str1_ptr[n] = '\0';
    str2_ptr[n] = '\0';

    /*********
      collate the strings, free the space then return
    **********/
    rc = strcoll(str1_ptr, str2_ptr);
    free(str1_ptr);
    free(str2_ptr);

    return( rc );
	

}
