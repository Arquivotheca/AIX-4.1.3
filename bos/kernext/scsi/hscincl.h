/* @(#)62	1.2.1.3  src/bos/kernext/scsi/hscincl.h, sysxscsi, bos411, 9428A410j 5/26/94 15:21:22 */
#ifndef _H_HSCINCL
#define _H_HSCINCL
/*
 * COMPONENT_NAME: (SYSXSCSI) IBM SCSI Adapter Driver
 *
 * FUNCTIONS:	NONE
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/************************************************************************/
/*                                                                      */
/* COMPONENT:   SYSXSCSI                                                */
/*                                                                      */
/* NAME:        hscincl.h                                               */
/*                                                                      */
/* FUNCTION:    IBM SCSI Adapter Driver Source File                     */
/*                                                                      */
/*      This adapter driver is the interface between a SCSI device      */
/*      driver and the actual SCSI adapter.  It executes commands       */
/*      from multiple drivers which contain generic SCSI device         */
/*      commands, and manages the execution of those commands.          */
/*      Several ioctls are defined to provide for system management     */
/*      and adapter diagnostic functions.                               */
/*                                                                      */
/* STYLE:                                                               */
/*                                                                      */
/*      To format this file for proper style, use the indent command    */
/*      with the following options:                                     */
/*                                                                      */
/*      -bap -ncdb -nce -cli0.5 -di8 -nfc1 -i4 -l78 -nsc -nbbb -lp      */
/*      -c4 -nei -nip                                                   */
/*                                                                      */
/*      Following formatting with the indent command, comment lines     */
/*      longer than 80 columns will need to be manually reformatted.    */
/*      To search for lines longer than 80 columns, use:                */
/*                                                                      */
/*      cat <file> | untab | fgrep -v sccsid | awk "length >79"         */
/*                                                                      */
/*      The indent command may need to be run multiple times.  Make     */
/*      sure that the final source can be indented again and produce    */
/*      the identical file.                                             */
/*                                                                      */
/************************************************************************/

#define	TM_SCSI		1
/* comment out the following to remove special target mode tracing */
/* #define	TM_TRACE	1	*/

/* DEBUGGING AIDS: */
#ifdef TM_DEBUG
#include <stdio.h>
#define DEBUG_0(x,A)		{if(tm_trace >= x){printf(A);}}
#define DEBUG_1(x,A,B)		{if(tm_trace >= x){printf(A,B);}}
#define DEBUG_2(x,A,B,C)	{if(tm_trace >= x){printf(A,B,C);}}
#define DEBUG_3(x,A,B,C,D)	{if(tm_trace >= x){printf(A,B,C,D);}}
#define DEBUG_4(x,A,B,C,D,E)	{if(tm_trace >= x){printf(A,B,C,D,E);}}
#define DEBUG_5(x,A,B,C,D,E,F)	{if(tm_trace >= x){printf(A,B,C,D,E,F);}}
#define DEBUG_6(x,A,B,C,D,E,F,G) {if(tm_trace >= x){printf(A,B,C,D,E,F,G);}}
#define DEBUGELSE		else
#else
#define DEBUG_0(x,A)
#define DEBUG_1(x,A,B)
#define DEBUG_2(x,A,B,C)
#define DEBUG_3(x,A,B,C,D)
#define DEBUG_4(x,A,B,C,D,E)
#define DEBUG_5(x,A,B,C,D,E,F)
#define DEBUG_6(x,A,B,C,D,E,F,G)
#define DEBUGELSE
#endif	TM_DEBUG

/* INCLUDED SYSTEM FILES */
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/syspest.h>
#include <sys/dma.h>
#include <sys/sysdma.h>
#include <sys/ioacc.h>
#include <sys/intr.h>
#include <sys/malloc.h>
#include <sys/buf.h>
#include <sys/uio.h>
#include <sys/user.h>
#include <sys/file.h>
#include <sys/pin.h>
#include <sys/sleep.h>
#include <sys/ioctl.h>
#include <sys/systm.h>
#include <sys/errno.h>
#include <sys/except.h>
#include <sys/param.h>
#include <sys/lockl.h>
#include <sys/priv.h>
#include <sys/watchdog.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dump.h>
#include <sys/xmem.h>
#include <sys/time.h>
#include <sys/errids.h>
#include <sys/ddtrace.h>
#include <sys/trchkid.h>
#include <sys/trcmacros.h>
#include <sys/adspace.h>
#include <sys/scsi.h>
#include <sys/lockname.h>

#include <sys/lock_alloc.h>

#include <sys/hscsidd.h>
/* END OF INCLUDED SYSTEM FILES	 */
#endif /* _H_HSCINCL */
