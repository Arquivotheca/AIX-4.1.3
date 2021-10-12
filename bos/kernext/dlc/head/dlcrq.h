/* @(#)72	1.3  src/bos/kernext/dlc/head/dlcrq.h, sysxdlcg, bos411, 9428A410j 10/19/93 11:16:34 */
/*
 * COMPONENT_NAME: (SYSXDLCG) Generic Data Link Control
 *
 * FUNCTIONS: common header file for Head Code
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _h_DLCRQ
#define _h_DLCRQ
/**********************************************************************/
/*  IBM CONFIDENTIAL                                                  */
/*  Copyright International Business Machines Corp. 1985, 1988        */
/*  Unpublished Work                                                  */
/*  All Rights Reserved                                               */
/*  Licensed Material -- Program Property of IBM                      */
/*                                                                    */
/*  Use, Duplication or Disclosure by the Government is subject to    */
/*  restrictions as set forth in paragraph (b)(3)(B) of the Rights in */
/*  Technical Data and Computer Software clause in DAR 7-104.9(a).    */
/**********************************************************************/
/*
	This file contains structure definitions for the DLC ring queue
	routines.
*/

#define DLC_READ 0x9000

struct ring_head {
	ulong in;            	/* Ring Queue IN  pointer     */
	ulong out;           	/* Ring Queue OUT pointer     */
	ulong end;           	/* Ring Queue END pointer     */
}; 

struct que_entry {
	ulong entry[6];		/* ring queue entries 6 words */
};

struct ring_queue {
	struct 	ring_head	ring_head;
	struct 	que_entry	que_entry;
};
#endif /* h_DLCRQ */
