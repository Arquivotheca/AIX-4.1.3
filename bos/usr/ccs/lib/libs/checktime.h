/* @(#)87	1.1  src/bos/usr/ccs/lib/libs/checktime.h, libs, bos411, 9428A410j 10/4/93 11:17:19 */
/*
 *   COMPONENT_NAME: LIBS
 *
 *   FUNCTIONS: FINISHENTRY
 *		INITENTRY
 *		PARTIALRANGE
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/**********************************************************
*
* Common definitions for chectime.c
*
***********************************************************/


/* STATE definitions for the DFA */
#define S1	0
#define M1	1
#define M2	2
#define M3	3
#define H1	4
#define H2	5
#define H3	6
#define H4	7 
#define H5	8 
#define J1	9 
#define J2	10
#define J3	11
#define E1	12

#define INVALID -1		/* Invalid state */

#define NUMSTATES	13	/* Needs to change if num of states changes */

/* Token definitions */
#define COMMA		0
#define EOI		1		
#define HYPHEN		2
#define COLON		3
#define EXCLAIM		4
#define MONTHDAY	5
#define DAY		6
#define HOUR		7

#define NUMTOKENS	8	/* Needs to change if num of tokens changes */

/* Structure for tokens */
struct token {
	int type;
	int val1;
	int val2;
};

#define MAXENTRIES    200     /* Max restriction entries for one port/user */
#define ALLOW           0     /* An ALLOW entry (no '!')                   */
#define DENY            1     /* A DENY entry                              */
#define NONE           -2     /* For checklog()                            */
#define ERROR          -1     /* An error during parsing                   */
#define INITVAL	       -1     /* Initial value for logtime struct entries  */

#define BIGBUFSIZ	1000  /* Large buffer                              */

/* One half of a range (start or end) */
struct logtime {
	int month;
	int daynum;
	int day;
	int hour;
};

/* Entry record, specifies a full range */
struct entry_rec {
	int               type;   /* ALLOW or DENY */
	struct logtime    start;  /* Start of range */
	struct logtime    end;    /* End of range */
};

/* Macro to initialize an entry */
/* Default entry type is ALLOW  */
#define INITENTRY(e) {                                \
                     entries[e].type        =ALLOW;   \
                     entries[e].start.month =INITVAL; \
                     entries[e].start.daynum=INITVAL; \
                     entries[e].start.day   =INITVAL; \
                     entries[e].start.hour  =INITVAL; \
                     entries[e].end.month   =INITVAL; \
                     entries[e].end.daynum  =INITVAL; \
                     entries[e].end.day     =INITVAL; \
                     entries[e].end.hour    =INITVAL; \
                     }

/* Macro to copy relevant entry start info to entry end info */
#define FINISHENTRY(e) {                                       \
   entries[e].end.month  =entries[e].start.month;              \
   entries[e].end.daynum =entries[e].start.daynum;             \
   entries[e].end.day    =entries[e].start.day;                \
                       }

#define PARTIALRANGE(e1)                                         \
         ( (entries[e1].start.month ==entries[e1].end.month) &&  \
           (entries[e1].start.daynum==entries[e1].end.daynum) && \
           (entries[e1].start.day   ==entries[e1].end.day) )

/**
 ** USERFORMAT and DBFORMAT are passed to output() to specify which
 ** type of output to construct.
 **/
#define USERFORMAT      1   /* Specifies user readable output from output() */
#define DBFORMAT        2   /* Specifies database (normalized) output from  *
                             * output()                                     */

