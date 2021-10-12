static char sccsid[] = "@(#)02	1.1  src/bos/usr/ccs/lib/libsrc/srcinet.c, libsrc, bos411, 9428A410j 11/10/89 16:25:01";
/*
 * COMPONENT_NAME: (cmdsrc) System Resource Controler
 *
 * FUNCTIONS:
 *	srcaddinet,srcdelinet
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregate modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <src.h>

void srcaddinet()
{
	tellsrc(ADDINETSOCKET,"");
}
void srcdelinet()
{
	tellsrc(DELINETSOCKET,"");
}
