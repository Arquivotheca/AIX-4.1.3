/* @(#)82	1.29  src/bos/kernel/sys/POWER/vmuser.h, sysvmm, bos41J, 9521B_all 5/15/95 16:58:48 */
#ifndef _H_VMUSER
#define _H_VMUSER

/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27 83
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

/*
 * FUNCTION: definitions for users within the kernel of VMM
 */

/*
 * parameters for vms_create
 */
#define V_WORKING	0x1
#define V_CLIENT	0x2
#define V_PERSISTENT	0x4
#define V_JOURNAL	0x8
#define V_DEFER 	0x10
#define V_SYSTEM	0x20
#define V_LOGSEG	0x40
#define V_UREAD 	0x80
#define V_PTASEG	0x100
#define V_PRIVSEG	0x200
#define V_INTRSEG	0x400
#define V_MAPPING	0x800
#define V_PSEARLYALLOC  0x1000 
#define V_SHRLIB	0x2000
#define V_SPARSE	0x4000

/*
 * parameters for vm_write
 */

#define V_FORCE 	1
#define V_NOFORCE	0

/*
 * parameters for vm_release
 */

#define V_NOKEEP	0
#define V_KEEP		1
#define V_CHECKXPT	2

/*
 * options for v_getxpt
 */
#define V_TOUCH 	1
#define V_NOTOUCH	0

/*
 * parameters for vm_protect
 */

#define KERKEY		0
#define UBLKKEY		0
#define FILEKEY		1
#define UTXTKEY		1
#define UDATAKEY	2
#define RDONLY		3

/*
 * parameters for vm_mount and PDT routines
 */
#define D_PAGING	1
#define D_FILESYSTEM	2
#define D_REMOTE	4
#define D_LOGDEV	8
#define D_SERVER	16

/* 
 * maximum file size in bytes 
 */
#ifdef _IBMRT
#define MAXFSIZE	((uint)(1 << 28))
#endif

#ifdef _POWER
#define MAXFSIZE	((uint)(1 << 31))
#endif

/*
 * return codes used within VMM
 * Note that both VM_WAIT and VM_NOWAIT
 * have a definition in ml (cf vmvcs.s)
 */
#define VM_OK		 0
#define VM_WAIT 	-1
#define VM_ECC		-2
#define VM_XMEMCNT	-3
#define VM_EXTMEM	-4
#define VM_NOTIN	-5
#define VM_NOPFAULT	-6
#define VM_UNHIDE	-7
#define VM_NOTMINE	-8
#define VM_NOWAIT 	-9
#define VM_NOSPACE      -10
#define VM_SPARSE	-11
#define VM_NOXPT	-12

/*
 * flag to check if paging space is defined
 *	0 - paging space defined
 *	nonzero - paging space not defined
 */
extern int ps_not_defined;

#endif /* _H_VMUSER */
