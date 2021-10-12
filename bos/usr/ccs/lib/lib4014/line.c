static char sccsid[] = "@(#)92	1.1  src/bos/usr/ccs/lib/lib4014/line.c, libt4014, bos411, 9428A410j 9/30/89 15:49:47";
/*
 * COMPONENT_NAME: libplot
 *
 * FUNCTIONS: line
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

line(x0,y0,x1,y1){
	move(x0,y0);
	cont(x1,y1);
}
