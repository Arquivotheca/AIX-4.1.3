static char sccsid[] = "@(#)74	1.2.1.4  src/bos/usr/ccs/lib/libc/strncollen.c, libcnls, bos411, 9428A410j 7/15/93 14:49:42";
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: strncollen()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/localedef.h>
#include <string.h>

/*
 * FUNCTION: Computes the collating length of 'len' characters in the specified 
 *	     string.
 *
 * PARAMETERS: (Uses file codes )
 *           char *s   -  string
 *           int   len -  length of string.
 *
 * RETURN VALUE DESCRIPTIONS: Returns the logical length of an NLS string
 *           as described above(>=0).
 */
int strncollen (const char *s, const int len)
{
	char *new_str;
	int  rc;

	/*********
	  get space for new string
	**********/
	if ((new_str = malloc(len+1)) == (char *)NULL)
		return(0);

	/***********
	  copy 'len' bytes to the new string
	**********/
	strncpy(new_str, s, len);

	/***********
	  ...  and null terminate, since strncpy() won't.
	**********/
	*(new_str + len) = '\0';

	/**********
	  get the total number of collation values for this string
	**********/
	rc = strxfrm((char *)NULL, new_str, 0);

	/**********
	  free up new_str
	**********/
	free(new_str);

	/* For C locale there is no transformation so strxfrm returns *
         * the length of the string which is the same as number of    *
	 * collation elements in the string.			      *
	 */
	if( ! strcmp(setlocale(LC_COLLATE,NULL),"C") ) 
		return rc;

	return( (rc/((__OBJ_DATA(__lc_collate)->co_nord+1)*2))-1 );
}
