/* @(#)44  1.4  src/bos/usr/bin/alog/alog.h, cmdalog, bos411, 9428A410j 5/21/93 12:34:47 */
/*
 *   COMPONENT_NAME: CMDALOG
 *
 *   FUNCTIONS: MSGSTR
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* alog.h -- Header file for alog utility		*/

#include	<stdio.h>
#include	<signal.h>
#include	<fcntl.h>
#include 	<sys/types.h>
#include 	<cf.h>
#include 	<sys/cfgdb.h>
#include 	<sys/cfgodm.h>
#include 	<locale.h>
#include 	<alog_msg.h>

nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_ALOG,Num,Str)

/*-------------------------------
 * This define controls rounding of
 * log size for expanding or shrinking
 * the log. Set to 4096 for arbitrary
 * log sizes.
 *-------------------------------*/
#define	DEF_SIZE  4096	       /* log size Define		*/
#define	ALOG_MAGIC  0xf9f3f9f4 /* magic number for alog files   */

/* The log file header contains information about the log file */
struct	bl_head                /* log file header */
	{
	int	magic;         /* magic number */
	int	top;           /* top of log */
	int	current;       /* current position in log */
	int	bottom;        /* bottom of log */
	int	size;          /* size of log */
	};

