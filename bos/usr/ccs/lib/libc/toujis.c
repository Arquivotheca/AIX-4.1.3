static char sccsid[] = "@(#)39	1.2  src/bos/usr/ccs/lib/libc/toujis.c, libcnls, bos411, 9428A410j 6/8/91 15:59:38";
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: toujis
 *
 * ORIGINS: 10
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include "NLctype.h"
/**********************************************************************/
/*								      */
/*  NAME							      */
/*	toujis							      */
/*								      */
/* SYNOPSIS							      */
/*	int							      */
/*	toujis (c)						      */
/*	register c;						      */
/*								      */
/* DESCRIPTION							      */
/*	toujis sets all of the parameter bits that are not 16 bit     */
/*      Ujis code to zero.                                            */
/*								      */
/* DIAGNOSTICS							      */
/*	NONE				                              */
/*								      */
/**********************************************************************/

int toujis (int c)
{
	return (c & 0xffff);
}
