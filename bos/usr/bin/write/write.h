/* @(#)15	1.4  src/bos/usr/bin/write/write.h, cmdwrite, bos411, 9428A410j 4/23/91 13:57:19 */
/*
 * COMPONENT_NAME: (CMDCOMM) user to user communication
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#define MAX_USERID_LEN 8 /* same as in struct utmp declared in utmp.h */
#define MAX_HOST_LEN 64  /* as defined in ping.c */
#define MAX_SER_LEN 10
#define DATE_LEN 30
#define MB_DATE_LEN 50
#define MAX_TTY_LEN 17 /* same as in struct utmp declared in utmp.h */

#define RELAY  '0'
#define RWRITE '1'
#define HWRITE '2'
#define QUERY  '3'

#define OK 0
#define CANCEL 1
#define MQUERY 2

#define SOK "ok"
#define SCANCEL "cancel"
#define SQUERY "query"

/* common control characters */
#define WRT_BELL	7  
#define WRT_NEWLINE	10	

/* error codes from writesrv */
#define NOTLOG 0
#define NOTTY -1
#define NOPERM -2
#define NOOPEN -3
#define MALLOC -4
#define BADHAND -5
#define SNDRPLY -6
#define GETRPLY -7 
#define NOSERVICE -8

struct ttys {
	char tty[PATH_MAX];
	struct ttys *next;
};
