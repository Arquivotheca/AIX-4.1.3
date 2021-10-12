static char sccsid[] = "@(#)50	1.6  src/bos/usr/ccs/lib/libc/NLfgetfile.c, libcnls, bos411, 9428A410j 6/16/90 01:26:41";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NLfgetfile
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME: NLgetfile 
 *
 * FUNCTION: Fake NLgetfile() to fool loader into resolving dummy 
 *	     reference
 *
 * RETURN VALUE DESCRIPTION: None 
 */
/*
 * Fake NLgetfile() to fool loader into resolving dummy reference.
 *
 */

int
NLgetfile(name)
char *name;
{
}
