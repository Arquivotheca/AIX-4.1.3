/* @(#)85	1.7  src/bos/kernel/sys/dbkersym.h, sysdb, bos411, 9428A410j 6/16/90 00:25:58 */

/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS:
 *
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_DBKERSYM
#define _H_DBKERSYM

#include <sys/seg.h>
#include <sys/low.h>
/*
 * Debugger's module id and associated data areas 
 */
extern ulong debmid;			/* Debugger's module ID 	*/
extern caddr_t debstart;		/* Starting address		*/
extern int deblen;			/* Length 			*/

#define DEBSID KERNELSEGVAL		/* Debugger's seg value		*/
#define DEBIOCN 0x20			/* IOCN reserved for Debug */

/* Debugger PSW area */
#ifdef _IBMRT
extern struct psw dbpsw;
#endif 

/*
 * The Debugger's entry parameters, loaded by debglue.
 */
extern struct dbparm {
	caddr_t flsadr;			/* FLS pointer 			*/
	int inited;			/* 1=Initialized.     		*/
	int savestkp;			/* stack ptr. saved area 	*/
	caddr_t mckadr;			/* Machine chk handler adr	*/
	caddr_t pckadr;			/* Program chk handler adr	*/
	ulong	fromid;			/* FROMID for db/vtrm path	*/
	ulong	pathid;			/*   the PATHID			*/
	} dbparm;

/*
 * Date for pinning/unpinning the beast
 */
extern struct dbpindata {
	ulong firstpage;		/* 1st page to pin 		*/
	ulong numpages;			/* no. of pages 		*/
	} dbpindata;

/* Paths for screen restoration */
extern ulong Sdvtpid;			/* Debugger to Scr Mgr path 	*/
extern ulong Sdvtfid;			/* From pchk to VTRM        	*/

/*
 * Machine check/Program check int. save area 03.01.
 */
extern caddr_t kermciar;		/* Save the VRM's mch. chk @ 	*/
extern caddr_t kerpciar;		/* Save the VRM's pgm. chk @ 	*/
#endif /* _H_DBKERSYM */
