static char sccsid[] = "@(#)49	1.1  src/bos/usr/ccs/lib/lib300/point.c, libt300, bos411, 9428A410j 9/30/89 15:42:16";
/*
 * COMPONENT_NAME: libplot
 *
 * FUNCTIONS: point
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

point(xi,yi){
		move(xi,yi);
		label(".");
		return;
}
