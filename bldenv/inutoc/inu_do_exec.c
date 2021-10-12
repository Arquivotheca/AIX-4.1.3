static char     sccsid[] = "@(#)29      1.1  src/bldenv/inutoc/inu_do_exec.c, cmdinstl, bos412, GOLDA411a 9/8/94 17:06:42";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*-----------------------------------------------------------------------
 *  The only reason this file exists is so that the 3.2 version of inutoc
 *  can be compiled without errors about WNOHANG.
 *-----------------------------------------------------------------------*/

inu_do_exec(char *command)
{
	return(system(command));
}
