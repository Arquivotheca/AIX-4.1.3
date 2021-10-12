/* @(#)73       1.5  src/bos/usr/bin/que/qadm/qcadm.h, cmdque, bos411, 9428A410j 3/21/91 14:53:34 */
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
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

/*----Misc defines */
#define QCONFIG CONFIG
#define DEVICES "device"

/*----This is a dummy name which doesn't happen, so we use it as a marker */
/*    It is always ignored by readln() in qccom.c */
#define DUMMY "dummy\177"

/*----Types of lines in qconfig file */
#define TYPBAD	0		/* Line is bad */
#define TYPCOMM 1		/* Line is a comment line */
#define TYPNAME	2		/* Line is a queue or device name */
#define TYPASSG	3		/* Line is an assignment xxx = yyy */

/*----Data structures */
struct clause {			/* Linked list of clauses for queue or device */
	char		ctext[MAXLINE];		/* string of text */
	boolean		cflag;			/* useful tagging flag */
	struct clause	*cnext;			/* pointer to next one in list */
	};

struct quelist {                /* Linked list of user selected job numbers */
	struct quelist  *next;        /* pointer to next in linked list */
	boolean         flag;         /* multi use flag for integrity, etc.*/
	char            qname[QNAME +1]; /* name of queue */
	char            dname[DNAME +1]; /* name of device */
	struct clause   *clauses;     /* pointer to list of clauses */
};

struct parms {
	struct quelist	*queues;	/* list of queues entered by user */
	boolean		deflt; 		/* indicates that queue entered is default */
	};
