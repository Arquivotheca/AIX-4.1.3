static char sccsid[] = "@(#)41	1.1  src/bos/usr/ccs/lib/libc/AFopen.c, libcgen, bos411, 9428A410j 12/14/89 17:40:20";
/*
 * LIBIN: AFopen
 *
 * ORIGIN: ISC
 *
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1985, 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 *
 * FUNCTION: Open an Attribute File.
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer to an Attribute File
 *	     structure.
 */

#include <stdio.h>
#include <stdlib.h>
#include <IN/standard.h>
#include <IN/AFdefs.h>

AFILE_t AFopen(
char *filename,
int maxrecsiz,  /* maximum size of a record (in bytes) */
int maxnumatr)  /* maximum number of attributes per record */
{       register AFILE_t af;
	register FILE *file;
	register int n;

	if ((file = fopen(filename,"r")) == NULL)
	    return(NULL);
	if ((af = (AFILE_t)malloc((size_t)sizeof(struct AFILE))) == NULL)
	{   fclose(file);
	    return(NULL);
	}
	af->AF_iop = file;
	af->AF_rsiz = maxrecsiz;
	af->AF_natr = maxnumatr;
	if ((af->AF_cbuf = (char *)malloc((size_t)maxrecsiz)) == NULL)
	{   free((void *)af);
	    fclose(file);
	    return(NULL);
	}
	if ((af->AF_dbuf = (char *)malloc((size_t)maxrecsiz)) == NULL)
	{   free((void *)af->AF_cbuf);
	    free((void *)af);
	    fclose(file);
	    return(NULL);
	}
	n = sizeof(struct ATTR) * (maxnumatr+2);
	if ((af->AF_catr = (ATTR_t)malloc((size_t)n)) == NULL)
	{   free((void *)af->AF_dbuf);
	    free((void *)af->AF_cbuf);
	    free((void *)af);
	    fclose(file);
	    return(NULL);
	}
	if ((af->AF_datr = (ATTR_t)malloc((size_t)n)) == NULL)
	{   free((void *)af->AF_catr);
	    free((void *)af->AF_dbuf);
	    free((void *)af->AF_cbuf);
	    free((void *)af);
	    fclose(file);
	    return(NULL);
	}
	af->AF_catr->AT_name = NULL;
	af->AF_catr->AT_value = NULL;
	af->AF_datr->AT_name = NULL;
	af->AF_datr->AT_value = NULL;
	return(af);
}
