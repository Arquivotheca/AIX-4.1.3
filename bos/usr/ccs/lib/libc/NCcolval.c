static char sccsid[] = "@(#)90	1.8.1.3  src/bos/usr/ccs/lib/libc/NCcolval.c, libcnls, bos411, 9428A410j 3/10/94 11:20:49";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NCcolval
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/localedef.h>
#include <NLctype.h>

/*
 *
 *  Find the (not necessarily unique) collation value of a character.
 *
 */

/*  Find collating value for character chr.  If NCcollate returns a negative
 *  value, go through the collating descriptors until find the one for the
 *  individual character and use that (not necessarily unique) collation 
 *  value.  For 1-n collation, use the collation value of the first 
 *  character in the replacement string.
 */
int NCcolval (wchar_t chr)
{

    int colval;
    char *wgt_str=NULL;
    char *tmp_str=NULL;
    int done;
   
    (void)_getcolval(__lc_collate,&colval,chr,tmp_str,0,&wgt_str,&done);

    return(colval);
}
