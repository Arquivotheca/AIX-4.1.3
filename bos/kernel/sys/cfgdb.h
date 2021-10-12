/* @(#)50     1.9 src/bos/kernel/sys/cfgdb.h, cmdcfg, bos411, 9428A410j 5/10/93 10:37:46 */
#ifndef _H_CFGDB
#define _H_CFGDB
/*
 * COMPONENT_NAME: (LIBCFG)  Generic library config support
 *
 * FUNCTIONS: cfgdb.h
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define	NOT_IN_USE	-1
#define	TRUE		1
#define	FALSE		0

/* Common to Predefined/Customized Object Classes */

#define		TYPESIZE	16		
#define		CLASSIZE	16	
#define		PREFIXSIZE	16
#define		DEVIDSIZE	16
#define		CATSIZE		16
#define		DDNAMESIZE	16
#define		UNIQUESIZE	48
#define		NAMESIZE	16
#define		VPDSIZE		512
#define		KEYSIZE		16
#define		LOCSIZE		16

/* Change Status Flag */

#define		NEW		0 
#define		DONT_CARE	1
#define		SAME		2
#define		MISSING		3

/* Device Status and Previous Device Status Flags */

#define		DEFINED		0
#define		AVAILABLE	1
#define		STOPPED		2
#define         DIAGNOSE        4

/* FRU Flag */

#define         NO_FRU          0
#define         SELF_FRU        1
#define         PARENT_FRU      2

/* VPD Flag */

#define		HW_VPD		0
#define		USER_VPD	1

/* Predefined/Customized Attribute Object Class */

#define		ATTRNAMESIZE	16
#define		DEFAULTSIZE	256
#define		ATTRVALSIZE	256
#define		WIDTHSIZE	16
#define		FLAGSIZE	8

/* Configuration Rules Object Class */

#define		RULESIZE		256
#define		SEQUENCE		1
#define		VERIFY_ONLY		2
#define		LD_FROM_CUSTOMIZED	3
#define		PHASE1			1
#define		PHASE2			2
#define		PHASE2MAINT		3
#define		RUNTIME_CFG		4

/* RUNTIME attribute for use in checking for phase1 execution legality */
/* WARNING - if you change this name, you have to change the cfg.add file */

#define		PHASE1_DISALLOWED	"uniquetype=runtime"

/* Customized Device Driver Object Class */

#define		RESOURCESIZE	12
#define		VALUESIZE	20

/* the following bit masks are used by bosboot, cfgmgr, & savebase to */
/*    to determine which devices belong with a specific kind of boot */
/* each "type of boot" requires a seperate bit */
/* each mask is specified as a HEX number, and may be a combination of any */
/*    of the defined masks */
/* DISK_BOOT should NEVER be changed - that's because we didn't have a */
/*		boot mask for v3.1, but we want to be compatable with it */

#define		DISK_BOOT			0x0001
#define		TAPE_BOOT			0x0002
#define		DISKETTE_BOOT		0x0004
#define		CDROM_BOOT			0x0008
#define		NETWORK_BOOT		0x0010
#define		PHASE0_BOOT			0x0020

#endif /* endif _H_CFGDB */
