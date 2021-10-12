/* @(#)30	1.3  src/bos/diag/util/udiagmon/diagd.h, dsaudiagmon, bos411, 9428A410j 1/21/94 13:41:56 */
/*	COMPONENT_NAME: DSAUDIAGMON
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* Notational conventions: 
G_<variable>  => Global symbols
S_<variable>  => Static symbols
*/

#define HOURS		24
#define MAX_DEV_ALLOC	5

#ifdef  LOG_DATA
#define DBGPR(x)	fprintf(G_fd, x);
#define DBGPR1(x,y)	fprintf(G_fd, x, y);
#define DBGPR2(x,y,z)	fprintf(G_fd, x, y, z);
#else
#define DBGPR(x)	
#define DBGPR1(x,y)
#define DBGPR2(x,y,z)
#endif


struct hrmin {		/* hour:minutes structure 			*/
short hour;
short minutes;
};

struct ptl {
char name[NAMESIZE];	/* name of the resource to be tested 		*/
short periodic;		/* time at which this resource be tested 	*/
};

struct min_list {	/* binary tree of tests for specified minutes. 	*/
struct hrmin hm;	/* minutes-job entry has access to hour:minutes */
short ndev;		/* number of devices in the list to be tested 	*/
char **devname;		/* list of devices to be tested 		*/
struct min_list *left;	/* left tree 					*/
struct min_list *right; /* right tree 					*/
};

