/* @(#)50       1.4  src/bos/usr/ccs/bin/common/caloff.h, cmdprog, bos411, 9428A410j 6/3/91 10:30:32 */
/*
 * COMPONENT_NAME: (CMDPROG) caloff.h
 *
 * FUNCTIONS: caloff                                                         
 *
 * ORIGINS: 27 03 09 32 00 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Changes for ANSI C were developed by HCR Corporation for IBM
 * Corporation under terms of a work made for hire contract.
 */
/*
 *  @OSF_COPYRIGHT@
 */


OFFSZ offsz;

/* -------------------- caloff -------------------- */

OFFSZ caloff(){
        return (0x10000000L * 8);     /* maximum usable stack offset */
        }

NODE *lastfree;  /* pointer to last free node; (for allocator) */

