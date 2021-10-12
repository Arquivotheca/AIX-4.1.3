/* @(#)96	1.3.1.7  src/bos/kernel/sys/core.h, sysproc, bos411, 9428A410j 1/18/94 13:15:08 */
/*
 *   COMPONENT_NAME: SYSPROC
 *
 *   FUNCTIONS: CDATA_ADDR
 *		CSTACK_ADDR
 *		
 *   ORIGINS: 3, 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */


#ifndef _H_SYS_CORE
#define _H_SYS_CORE

#include <sys/user.h>

/* provides information on vm regions */
struct vm_info {
        void *vminfo_addr;              /* effective address of region */
        int vminfo_size;                /* size of memory region */
        int vminfo_offset;              /* offset in core file of region */
};

/* The core_dump structure occurs once at the beginning of a core file. */
struct core_dump
{
        char            c_signo;        /* signal number (cause of error) */
        char            c_flag;         /* flag to describe core dump type */
        ushort          c_entries;      /* number of core dump modules */
        struct ld_info  *c_tab;         /* offset to beginning of core table */
        caddr_t         c_stack;        /* offset of user stack */
        int             c_size;         /* size of user stack */
        struct mstsave  c_mst;      	/* copy of the faulting mst */
	struct user	c_u;		/* copy of the user structure */
        int             c_nmsts;    	/* number of msts in c_msts */
	struct mstsave  *c_msts;	/* offset to the other threads msts */	
	int		c_datasize;	/* size of data region */
	caddr_t		c_data;		/* offset to user data */
        int             c_vmregions;    /* number of anonymously mapped areas */
        struct vm_info  *c_vmm;         /* offset to start of vm_info table */
};

/* Core file name, header size, maximum loader table size */
#define COREFILE	"core"
#define CHDRSIZE	(sizeof(struct core_dump))
#define CTABSIZE	(sizeof(struct ld_info) + MAXPATHLEN + MAXNAMLEN)

/* Values for the c_flag field of the core_dump structure */
#define FULL_CORE       0x01            /* core contains the data sections */
#define CORE_VERSION_1  0x02            /* for AIXv4 or higher */
#define MSTS_VALID      0x04            /* the u_threads have been dumped */
#define CORE_BIGDATA	0x08		/* records big data usage */
#define UBLOCK_VALID    0x10            /* the u_block has been dumped */
#define USTACK_VALID    0x20            /* the user stack has been dumped */
#define LE_VALID        0x40            /* core contains at least 1 module */
#define CORE_TRUNC      0x80            /* core was truncated (see setrlimit) */

/* Macro to define the user's maximum file, core, stack, data sizes in bytes */
#define CFILE_SIZE	U.U_rlimit[RLIMIT_FSIZE].rlim_cur
#define CCORE_SIZE	MIN(CFILE_SIZE, U.U_rlimit[RLIMIT_CORE].rlim_cur)
#define CDATA_SIZE	U.U_dsize

/* Macro to determine the starting user address given the length and type */
#define CDATA_ADDR(x)	(PRIVORG)
#define CSTACK_ADDR(x)	(USRSTACK - x)

#endif  /* _H_SYS_CORE */
