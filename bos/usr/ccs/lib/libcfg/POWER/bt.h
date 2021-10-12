/* @(#)47	1.20  src/bos/usr/ccs/lib/libcfg/POWER/bt.h, libcfg, bos41J, bai15 4/11/95 15:48:09 */
/*
 * COMPONENT_NAME: (LIBCFG) BUILD TABLES HEADER FILE
 *
 * FUNCTIONS: Compiler definitions
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

/* prevent multiple inclusion */
#ifndef _H_BT
#define _H_BT

/* range separator */
#define		RANGESEP	'-'

/* list delimiter */
#define		DELIM		','

/* defines for the attribute flags */
#define		TBADDR		'B'
#define		TMADDR		'M'
#define		TIOADDR		'O'
#define		TINTLVL		'I'
#define		TNSINTLVL	'N'
#define		TPRIORITY	'P'
#define		TDMALVL		'A'
#define		TGROUP		'G'
#define		TSHARE		'S'
#define		TWIDTH		'W'

/* This is assigned when no Priority Class attribute is specified */
#define		DEFAULT_PRIORITY_CLASS 4

/* Defines for print_resource_summary() */
#define		INTRO		0
#define		INTERIM		1
#define		FORCE		2

/* Defines for busresolve information */
#define		COMMAND_MODE_FLAG	0x80000000
#define		RESOLVE_MODE_FLAG	0x40000000

#endif /* _H_BT */

