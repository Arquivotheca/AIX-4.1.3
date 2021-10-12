/* @(#)94       1.32.1.7  src/bos/kernel/sys/space.h, sysios, bos411, 9428A410j 3/11/94 15:50:02 */
/*
 *   COMPONENT_NAME: (SYSIOS) IO subsystems
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#ifndef _H_SPACE
#define _H_SPACE

#include <sys/proc.h>

#include <sys/sysinfo.h>
struct sysinfo sysinfo;
struct syswait syswait;
struct syserr syserr;
struct cpuinfo cpuinfo[MAXCPU];

#include <sys/var.h>
struct var v = {
/* the following fields with RW in comment can be updated during runtime via the sysconfig
   system call.  The fields with RO in comment cannot be updated using sysconfig.
 */
	0,			/* var_vers field 		*/
	0,			/* var_gen field  		*/	
	SYSCONFIG_VARSIZE,	/* var size for SYSCONFIG  	*/
	0,			/* RW: v_bufhw field  		*/
	2048,			/* RW: v_mbufhw field  		*/
	CHILD_MAX,		/* RW: v_maxup field  		*/
	0,			/* RW: v_iostrun field		*/
	0,			/* RW: v_leastpriv field 	*/
	0,			/* RW: v_autost field  		*/
	0,			/* RW: v_maxpout field 		*/
	0,			/* RW: v_minpout field 		*/
	0,			/* RW: v_memscrub field 	*/
	0,			/* RO: v_lock field   		*/
	(char *)0,		/* RO: ve_lock field  		*/
	0,			/* RO: v_file field   		*/
	(char *)0,		/* RO: ve_file field  		*/
	NPROC,			/* RO: v_proc field	  	*/
	(char *)(&proc[0]),	/* RO: ve_proc field  		*/
	NCLIST,			/* RO: v_clist field  		*/
	NTHREAD,		/* RO: v_thread field 		*/
	(char *)(&thread[0]),	/* RO: ve_thread field		*/
	(char *)(&proc[0]),	/* RO: vb_proc field  		*/
	(char *)(&thread[0]),	/* RO: vb_thread field		*/
	0,			/* RO: v_ncpus	  		*/
	0,			/* RO: v_ncpus_cfg	  	*/
	0,			/* RW: v_fullcore field 	*/
	{ (char)0,(char)0,(char)0,(char)0 } /* RW: v_initlvl field */
};

#endif /* _H_SPACE */
