/* @(#)77	1.9  src/bos/kernel/sys/POWER/m_types.h, sysproc, bos411, 9428A410j 4/2/93 13:52:17 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef _H_M_TYPES
#define _H_M_TYPES

typedef struct label_t {		/* kernel jump buffer */
	struct label_t *prev;		/* chain to previous */
	ulong_t           iar;		/* resume address */
	ulong_t           stack;		/* stack pointer */
	ulong_t           toc;		/* toc pointer */
	ulong_t           cr;             /* non-volatile part of cr */
	ulong_t           intpri;		/* priority level of the process */
	ulong_t           reg[19];	/* non-volatile regs (13..31) */
} label_t;

typedef long		vmid_t;		/* virtual memory object ID */
typedef ulong_t		vmhandle_t;	/* virtual memory handle */

typedef struct vmaddr_t {		/* long-form virtual address */
	vmhandle_t	srval;		/* segment reg contents */
	caddr_t		offset;		/* offset within segment */
} vmaddr_t;

typedef struct adspace_t{		/* address space mapping */
	ulong_t		alloc;		/* allocation flags */
	vmhandle_t	srval[16];	/* contents of all seg regs */
} adspace_t;

#endif /*_H_M_TYPES*/

