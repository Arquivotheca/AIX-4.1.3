/* @(#)96	1.10  src/bos/kernel/sys/POWER/debug.h, cmddbx, bos411, 9428A410j 9/16/93 13:31:52 */
/*
 * COMPONENT_NAME: SYSPROC
 *
 * FUNCTIONS: none
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
#ifndef _H_debug
#define _H_debug

/* Traceback table, one per procedure usually.              */
/* Table is marked by word-aligned word of zeroes in instruction space    */
/* Traceback table is also referred to as "procedure-end table." */

struct tbtable_short {
	unsigned version:8; 	/* traceback format version               */
	unsigned lang:8;  	/* See language values below              */
	unsigned globallink:1;	/* Set if routine is global linkage 	  */
	unsigned is_eprol:1;	/* Set if is out-of-line epilog/prologue  */
	unsigned has_tboff:1;	/* Set if offset from start of proc stored */
	unsigned int_proc:1;	/* Set if routine is internal		  */
	unsigned has_ctl:1;	/* Set if routine involves controlled storage */
	unsigned tocless:1;	/* Set if routine contains no TOC 	  */
	unsigned fp_present:1;  /* Set if routine performs FP operations  */
	unsigned log_abort:1;   /* Set if routine logs or aborts FP ops   */
	unsigned int_hndl:1;    /* Set if routine is interrupt handler    */
	unsigned name_present:1;/* Set if name is present in proc table */
	unsigned uses_alloca:1; /* Set if alloca used to allocate storage */
	unsigned cl_dis_inv:3;  /* On-condition directives, see below */
	unsigned saves_cr:1;    /* Set if procedure saves the condition reg */
	unsigned saves_lr:1;    /* Set if procedure saves the link reg */
	unsigned stores_bc:1;   /* Set if procedure stores the backchain */
	unsigned fixup:1;	/* Set if code is generated fixup code. */
	unsigned fpr_saved:6;   /* Number of FPRs saved, max of 32 */
	unsigned spare3:2;      /* Spare bits */
	unsigned gpr_saved:6;   /* Number of GPRs saved, max of 32 */
	unsigned fixedparms:8;  /* Number of fixed point parameters */
	unsigned floatparms:7;  /* Number of floating point parameters */
	unsigned parmsonstk:1;  /* Set if all parameters placed on stack */
};

/*
 * Optional portions of procedure-end table.  
 *
 * Optional portions exist in the following order independently, not as
 * a structure or an union. Whether or not portions exist is determinable
 * from bit-fields within the base procedure-end table.  
 *
 * parminfo      exists if fixedparms or floatparms != 0.
 * tb_offset     exists if has_tboff bit is set.
 * hand_mask     exists if int_hndl bit is set.
 * ctl_info      exists if has_ctl bit is set.
 * ctl_info_disp exists if ctl_info exists.
 * name_len      exists if name_present bit is set.
 * name          exists if name_len exists.
 * alloca_reg    exists if uses_alloca bit is set.
 */
struct tbtable_ext {
	unsigned int parminfo;  /* Order and type encoding of parameters:
				 * Left-justified bit-encoding as follows:
				 * '0'  ==> fixed parameter
				 * '10' ==> single-precision float parameter
				 * '11' ==> double-precision float parameter */
	unsigned int tb_offset; /* Offset from start of code to tb table */
	int hand_mask;		/* What interrupts are handled by */
	int ctl_info;		/* Number of CTL anchors, followed by */ 
	int ctl_info_disp[1];	/* Actually ctl_info_disp[ctl_info] */
				/* Displacements into stack of each anchor */
	short name_len;		/* Length of procedure name */
	char name[1];		/* Actually char[name_len] (no NULL) */
	char alloca_reg;	/* Register for alloca automatic storage */
};

struct tbtable {
	struct tbtable_short tb;
	struct tbtable_ext tb_ext;
};

/* Language indicators */
#define TB_C		 0
#define TB_FORTRAN	 1
#define TB_PASCAL	 2
#define TB_ADA		 3
#define TB_PL1		 4
#define TB_BASIC	 5
#define TB_LISP		 6
#define TB_COBOL	 7
#define TB_MODULA2	 8
#define TB_CPLUSPLUS	 9
#define TB_RPG		10	
#define TB_PL8		11
#define TB_ASM		12

/* On-condition directives */
#define WALK_ONCOND	0	/* Walk the stack without restoring state */
#define DISCARD_ONCOND	1	/* Walk the stack and discard */
#define INVOKE_ONCOND	2	/* Invoke a specific system routine */

#endif	/*_H_debug*/
