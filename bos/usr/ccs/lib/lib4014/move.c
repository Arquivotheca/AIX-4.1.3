static char sccsid[] = "@(#)94	1.1  src/bos/usr/ccs/lib/lib4014/move.c, libt4014, bos411, 9428A410j 9/30/89 15:50:02";
/*
 * COMPONENT_NAME: libplot
 *
 * FUNCTIONS: move
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

move(xi,yi){
	putch(035);
	cont(xi,yi);
}
