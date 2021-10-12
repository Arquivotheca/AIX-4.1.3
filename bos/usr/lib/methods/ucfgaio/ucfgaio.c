static char sccsid[] = "@(#)52	1.3  src/bos/usr/lib/methods/ucfgaio/ucfgaio.c, cfgmethods, bos411, 9428A410j 1/5/94 11:48:06";
/*
 * COMPONENT_NAME: (CFGMETHODS) unconfigure method for aio extension
 *
 * FUNCTIONS: main
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <cf.h>

/*
 * NAME: main
 * 
 * FUNCTION: This process is executed to "unconfigure" an aio extension .
 * 
 * EXECUTION ENVIRONMENT:
 *
 *	This process is invoked when an aio extension is to be 
 *	"unconfigured". 
 *
 * NOTES: Interface:
 *	ucfgaio  -l aio0 <phase>
 *
 * RETURNS: E_UNLOADEXT
 */

main()
{
	exit(E_UNLOADAIO);
}
