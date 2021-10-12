/* @(#)95	1.1  src/bos/usr/lbin/tty/mon-cxma/tables.h, sysxtty, bos411, 9428A410j 6/23/94 15:28:04 */
/*
 * COMPONENT_NAME: (sysxtty) System Extension for tty support
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 */

#ifndef _TABLES_H_
#define _TABLES_H_

#define	S2400	0
#define	S4800	1
#define S9600	2
#define	S14_4K	3
#define	S19_2K	4
#define	S38_4K	5
#define	S57_6K	6
#define	S64K	7
#define	S76_8K	8
#define	S115K	9
#define	S155K	10
#define	S230K	11
#define	S460K	12
#define	S920K	13
#define	S921K	14
#define	S1_2M	15
#define	S1_843M	16
#define	S2_458M	17
#define	S3_686M	18
#define	S7_373M	19
#define	S10M	20

#define DIRECT_CONNECT 0
#define SYNC_CONNECT 1

#define WIRE4 0
#define WIRE8 1

#define CLOCK_INTERNAL 0
#define CLOCK_SELF 1
#define CLOCK_RS422 2
#define CLOCK_RS232 3

extern char *speeds[];
extern char *clocks[];
extern char *wires[];
extern char *connections[];
extern struct table {
	int speed;
	int wire;
	int clock;
	int connection;
} table[];

#endif /* _TABLES_H_ */
