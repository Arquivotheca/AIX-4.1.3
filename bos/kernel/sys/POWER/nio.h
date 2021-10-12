/* @(#)96	1.10  src/bos/kernel/sys/POWER/nio.h, sysios, bos411, 9428A410j 9/22/93 03:27:49 */
#ifndef _H_NIO
#define _H_NIO
/*
 * COMPONENT_NAME: (SYSIOS) Native IO Hardware definitions
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988,1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* 
 *	The following defines the interface to the native keyboard.
 */


#define NIO_BID			0x820C0060	/* NIO segment register value */
#define	NIO_KEYBOARD		0x00000050	/* base address of keyboard */
#define NIO_SLOT		0x000f0000
#define CARD_ID_REG1		(NIO_SLOT | 0x00400000)
#define CARD_ID_REG2		(NIO_SLOT | 0x00400001)
#define RESET_DIR_REG		(NIO_SLOT | 0x00400002)
#define IO_SEG_REG		0x80080020
#define IOCC_SEG_REG		0x80000080
#define IOCC_SEG_REG_VALUE	0x82000080
#define KBD_CONFIG_REG		0x57
#define COMP_RESET_REG		0x0040002c
#define COMP_RESET_REG_PPC	0x000100A0
#define MAX_SLOTS		16

#define POS_ADDR(slot, offset)	(0x00400000 | ((slot) << 16) | (offset))



/* Standard IO pos IDs
 */
#define NIO_POS0_REL1 0x5fdf		/* Not Stilwell/Gomez */
#define NIO_POS0_REL2 0xe6de		/* Stilwell/Gomez */
#define NIO_POS0_RSC1 0xfef6		/* Salmon */
#define NIO_POS0_PEGASUS 0xd9fe		/* Pegasus */

	
struct nio_key				/* native keyboard */
{
	char		command;	/* command and data latch */
	char		reserved1;	/* reserved */
	char		reserved2;	/* reserved */
	char		reserved3;	/* reserved */
	char		input;		/* input buffer */
	char		diagnostic;	/* diagnostic sense */
	char		interrupt;	/* interrupt identifier */
	char		control;	/* control commands */
	char		ctlalt;		/* ctl-alt-anything */
};

#endif /* _H_NIO */
