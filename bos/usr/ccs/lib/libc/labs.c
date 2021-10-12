static char sccsid[] = "@(#)62	1.8  src/bos/usr/ccs/lib/libc/labs.c, libccnv, bos411, 9428A410j 6/16/90 01:05:14";
/*
 * COMPONENT_NAME: LIBCCNV labs
 *
 * FUNCTIONS: labs
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME: labs
 *                                                                    
 * FUNCTION: Return absolute value of an integer
 *                                                                    
 * NOTES:
 *
 * RETURNS: long integer absolute value of j
 *
 */

#ifdef labs
#undef labs
#endif

long int
labs(long int j)
{
	return (j >= 0 ? j : -j);
}
