/* @(#)19	1.7  src/bos/kernel/sys/POWER/vminfo.h, sysvmm, bos411, 9428A410j 5/10/91 16:47:00 */
#ifndef _H_VMINFO
#define _H_VMINFO

/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/types.h>

/* Information shown by vmstat command.
 */
struct vminfo {
#define	vmmfirst	pgexct	/* first variable of vminfo
				   should be maintained as
				   first vminfo variable	        */	
	uint	pgexct; 	/* count of page faults     		*/
	uint	pgrclm; 	/* count of page reclaims 		*/
	uint	lockexct;	/* count of lockmisses	    		*/
	uint	backtrks;    	/* count of backtracks	    		*/
	uint	pageins;	/* count of pages paged in  		*/
	uint	pageouts;	/* count of pages paged out 		*/
	uint	pgspgins;	/* count of page ins from paging space	*/
	uint	pgspgouts;	/* count of page outs from paging space */
	uint	numsios;	/* count of start I/Os	    		*/
	uint	numiodone;	/* count of iodones	    		*/
	uint	zerofills;      /* count of zero filled pages 		*/
	uint	exfills;        /* count of exec filled pages		*/
	uint	scans;      	/* count of page scans by clock 	*/
	uint	cycles;      	/* count of clock hand cycles		*/
	uint	pgsteals;      	/* count of page steals	   		*/
	uint	freewts;      	/* count of free frame waits		*/
	uint	extendwts;      /* count of extend XPT waits		*/
	uint	pendiowts;      /* count of pending I/O waits  		*/
	uint	pings;		/* count of ping-pongs: source => alias */
	uint	pongs;		/* count of ping-pongs: alias => source */
	uint	pangs;		/* count of ping-pongs: alias => alias  */
	uint	dpongs;		/* count of ping-pongs: alias page delete */
	uint	wpongs;		/* count of ping-pongs: alias page writes */
	uint	cachef;		/* count of ping-pong cache flushes	*/
	uint	cachei;		/* count of ping-pong cache invalidates	*/
#define	vmmlast	cachei		/* last variable of vminfo 
				   should be maintained as
				   last vminfo variable	        */	
};

extern struct vminfo vmminfo;

/* Macros for referencing variables in vmminfo.
 */
#define vmpf_pgexct	vmminfo.pgexct
#define vmpf_pgrclm	vmminfo.pgrclm
#define vmpf_lockexct	vmminfo.lockexct
#define vmpf_backtrks	vmminfo.backtrks
#define vmpf_pageins	vmminfo.pageins
#define vmpf_pageouts	vmminfo.pageouts
#define vmpf_pgspgins	vmminfo.pgspgins
#define vmpf_pgspgouts	vmminfo.pgspgouts
#define vmpf_numsios	vmminfo.numsios
#define vmpf_numiodone	vmminfo.numiodone
#define vmpf_zerofills	vmminfo.zerofills
#define vmpf_exfills	vmminfo.exfills
#define vmpf_scans	vmminfo.scans
#define vmpf_cycles	vmminfo.cycles
#define vmpf_pgsteals	vmminfo.pgsteals
#define vmpf_freewts	vmminfo.freewts
#define vmpf_extendwts	vmminfo.extendwts
#define vmpf_pendiowts	vmminfo.pendiowts
#define vmpf_pings	vmminfo.pings
#define vmpf_pangs	vmminfo.pangs
#define vmpf_pongs	vmminfo.pongs
#define vmpf_dpongs	vmminfo.dpongs
#define vmpf_wpongs	vmminfo.wpongs
#define vmpf_cachef	vmminfo.cachef
#define vmpf_cachei	vmminfo.cachei

/* Paging space information returned by swapqry() system call
 */
struct pginfo
{
	dev_t	devno;		/* device number		*/
	uint	size;		/* size in PAGESIZE blocks	*/
	uint	free;		/* # of free PAGESIZE blocks	*/
	uint	iocnt;		/* number of pending i/o's	*/
};

#endif /* _H_VMINFO */
