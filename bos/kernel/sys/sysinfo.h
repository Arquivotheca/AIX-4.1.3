/* @(#)28	1.15  src/bos/kernel/sys/sysinfo.h, sysios, bos411, 9428A410j 2/28/94 12:50:57 */
/* sysinfo.h	5.2 87/01/09 18:25:51 */
#ifndef _H_SYSINFO
#define _H_SYSINFO
/*
 * COMPONENT_NAME: (SYSIOS) IO subsystems
 *
 * ORIGINS: 3, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*  NOTE:  An assembly-language version of these structures exists
 *            in "sysinfo.m4".  BE SURE to update that file whenever
 *            <sys/sysinfo.h> is changed!!!
 */

#include <sys/types.h>
#include <sys/iostat.h>
#ifdef _KERNSYS
#include <sys/ppda.h>
#endif

struct sysinfo {
#define	CPU_NTIMES	4 	/* number of cpu times */
	time_t	cpu[CPU_NTIMES];   /* this array is updated every clock tick,
			     and keys off the state of the current running
			     process */
#define	CPU_IDLE	0 /* slot incremented whenever the 'wait' process
			     is the current running process */
#define	CPU_USER	1 /* slot incremented whenever the current running
			     process is executing in user mode */
#define CPU_KERNEL	2 /* slot incremented whenever the current running 
			     process is executing in kernel mode */
#define	CPU_WAIT	3 /* This slot is incremented whenever the current 
			     running process is waiting for a block i/o 
			     request to complete. Currently, whenever a 
			     process becomes blocked, it is put to sleep 
			     and a new process is made the current running 
			     process (i.e. processes no longer maintain 
			     control of the cpu when they become blocked). */
#define	sysfirst	bread	/* first sysinfo variable
			  	   sysfirst define should be maintained as 
				   first non-array sysinfo variable */
	long	bread;
	long	bwrite;
	long	lread;
	long	lwrite;
	long	phread;
	long	phwrite;
	long	pswitch;  /* field is incremented whenever 'curproc' (i.e.
			     current running process) changes. It is possible
			     that this field can wrap. */
	long	syscall;
	long	sysread;
	long	syswrite;
	long	sysfork;  /* field is incremented by one whenever a 'fork'
			     is done */
	long	sysexec;  /* field is incremented by one whenever a 'exec'
			     is done */
	long	runque;   /* every second the process table is scanned to
			     determine the number of processes that are
			     ready to run. If that count is > 0 the
			     current number of ready processes is added 
			     to 'runque' (i.e. 'runque' is a cummulative
			     total of ready processes at second intervals). */
	long	runocc;   /* whenever 'runque' is updated, 'runocc'
			     is incremented by 1 (can be used to compute
			     the simple average of ready processes). */
	long	swpque;   /* every second the process table is scanned to 
			     determine the number of processes that are
			     inactive because they are waiting to be paged
			     in. If that count is > 0 then the current number
			     of processes waiting to be paged in is added
			     to 'swpque' (i.e. 'swpque' is a cummulative
			     total of processes waiting to be swapped in
			     at second intervals). */
	long	swpocc;   /* whenever 'swpque' is updated, 'swpocc' is 
			     incremented by one (can be used to compute
			     the simple average of processes waiting to be 
			     paged in).*/
	long	iget;
	long	namei;
	long	dirblk;
	long	readch;
	long	writech;
	long	rcvint;
	long	xmtint;
	long	mdmint;
	struct	ttystat ttystat;
#define	rawch	ttystat.rawinch
#define	canch	ttystat.caninch
#define	outch	ttystat.rawoutch
	long	msg;
	long	sema;
	long    ksched;   /* field is incremented by one whenever a kernel
                             process is created */
	long    koverf;   /* field is incremented by one whenever an  attempt
			     is made to create a kernel process and:
				- the user has forked to their maximum limit
					       - OR -
				- the configuration limit of processes has been
				  reached */
	long    kexit;    /* field is incremented by one immediately after the
			     kernel process becomes a zombie */
	long    rbread;         /** remote read requests       **/
	long    rcread;         /** reads from remote cache    **/
	long    rbwrt;          /** remote writes              **/
	long    rcwrt;          /** cached remote writes       **/
	long	devintrs;	/* device interrupts */
	long	softintrs;	/* software interrupts */
	long	traps;		/* traps */
#define	syslast		traps	/* last sysinfo variable
				   syslast define should be maintained as 
				   last non-array sysinfo variable */
};

extern struct sysinfo sysinfo;

struct syswait {
	short	iowait;
	short	physio;
};

extern struct syswait syswait;

struct syserr {
	long	inodeovf;
	long	fileovf;
	long	textovf;
	long	procovf;
	long	sbi[5];
#define	SBI_SILOC	0
#define	SBI_CRDRDS	1
#define	SBI_ALERT	2
#define	SBI_FAULT	3
#define	SBI_TIMEO	4
};

extern struct syserr syserr;

struct cpuinfo {
	time_t	cpu[CPU_NTIMES];
	long	pswitch;
	long	syscall;
	long	sysread;
	long	syswrite;
	long	sysfork;
	long	sysexec;
	long	readch;
	long	writech;
	long	iget;
	long	namei;
	long	dirblk;
	long	msg;
	long	sema;
};

#ifdef _KERNSYS
extern struct cpuinfo cpuinfo[MAXCPU];
#endif

#endif /* _H_SYSINFO */
