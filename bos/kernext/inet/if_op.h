/* @(#)35	1.2  src/bos/kernext/inet/if_op.h, sockinc, bos411, 9428A410j 6/10/91 17:01:58 */
/* 
 * COMPONENT_NAME: (SOCKET) Socket services
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 26 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution is only permitted until one year after the first shipment
 * of 4.4BSD by the Regents.  Otherwise, redistribution and use in source and
 * binary forms are permitted provided that: (1) source distributions retain
 * this entire copyright notice and comment, and (2) distributions including
 * binaries display the following acknowledgement:  This product includes
 * software developed by the University of California, Berkeley and its
 * contributors'' in the documentation or other materials provided with the
 * distribution and in all advertising materials mentioning features or use
 * of this software.  Neither the name of the University nor the names of
 * its contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

/*
 * 802.3 LLC header
 */
struct ie2_llc_hdr {
	unsigned char	dsap;		/* DSAP				*/
	unsigned char	ssap;		/* SSAP				*/
	unsigned char	ctrl;		/* control field		*/
	unsigned char	prot_id[3];	/* protocol id			*/
	unsigned short	type;		/* type field			*/
};

#define	DSAP_INET	0xaa		/* SNAP SSAP			*/
#define	SSAP_INET	0xaa		/* SNAP DSAP			*/
#define	SSAP_RESP	0x01		/* SSAP response bit		*/
#define	CTRL_UI		0x03		/* unnumbered info		*/
#define CTRL_XID	0xaf		/* eXchange IDentifier		*/
#define	CTRL_TEST	0xe3		/* test frame			*/
#define	_802_3_TYPE_IP	0x0800		/* IP protocol */


/*
 * Optical Header.
 */
struct op_hdr {
	u_long			proc_id;	/* Dest. HW addr-like thingy */
	struct ie2_llc_hdr	snap;		/* snap header 802.2 */
};

#define OPMTU		((SOL_MAX_XMIT) - sizeof(struct op_hdr))
#define NSC
