static char sccsid[] = "@(#)40	1.1  src/bos/usr/ccs/lib/lib300/close.c, libt300, bos411, 9428A410j 9/30/89 15:41:02";
/*
 * COMPONENT_NAME: libplot
 *
 * FUNCTIONS: closevt, closepl
 *
 * ORIGINS: 4,10,27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <stdio.h>
closevt(){
	closepl();
}
closepl(){
	fflush(stdout);
	reset();
}
