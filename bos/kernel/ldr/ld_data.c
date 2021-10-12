static char sccsid[] = "@(#)60  1.7  src/bos/kernel/ldr/ld_data.c, sysldr, bos411, 9428A410j 6/8/94 12:52:49";
/*
 * COMPONENT_NAME: (SYSLDR) Program Management
 *
 * FUNCTIONS: loader data
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/adspace.h>
#include	<sys/fp_io.h>
#include	<sys/ldr.h>
#include	<sys/malloc.h>
#include	<sys/pseg.h>
#include	<sys/seg.h>
#include	<sys/syspest.h>
#include	<sys/vmuser.h>
#include	<sys/xcoff.h>
#include	<sys/lockl.h>
#include	<sys/sleep.h>
#include	"ld_data.h"


/* This file contains definitions for global loader data
 *
 */

BUGVDEF(ld_debug,BUGNFO);		/* debugging control */

struct loader_anchor	kernel_anchor;  /* anchor of the list of
					   loaded kernel extensions */

struct loader_entry  *kernel_exports;   /* current kernel name spaces */

struct loader_entry  *syscall_exports;  /* current syscall name spaces */

struct library_anchor *library_anchor;	/* anchor of the list of library
					 * modules */

vmhandle_t library_data_handle;		/* handle for data library segment */
vmhandle_t library_text_handle;		/* handle for text library segment */

uint	shlibseg_full = 0;		/* flag to indicate that library
					 * segment has been overflowed */

heapaddr_t pp_ovfl_heap;		/* address of per-process overflow
					 * heap.  this is the same for every
					 * process */

char *kernel_filename = "\0/unix";	/* special file name for kernel */

int ld_loader_read = EVENT_NULL;	/* event list for processes waiting
					 * for another process to finish
					 * reading in a file */

