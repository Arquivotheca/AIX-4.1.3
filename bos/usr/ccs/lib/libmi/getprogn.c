static char sccsid[] = "@(#)85	1.1  src/bos/usr/ccs/lib/libmi/getprogn.c, cmdpse, bos411, 9428A410j 5/7/91 13:08:21";
/*
 *   COMPONENT_NAME: CMDPSE
 *
 *   ORIGINS: 27 63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/** Copyright (c) 1990  Mentat Inc.
 ** getprogn.c 2.1, last change 11/14/90
 **/


#include <pse/common.h>

#ifndef	DEF_PROGRAM_NAME
#define	DEF_PROGRAM_NAME	""
#endif

	char	noshare * program_name;

char *
get_program_name ()
{
	if (program_name  &&  *program_name)
		return program_name;
	return DEF_PROGRAM_NAME;
}
