/* @(#)35	1.3  src/bos/diag/da/dial/dial.h, dadial, bos411, 9428A410j 3/4/94 12:47:55 */
/*
 *   COMPONENT_NAME: DADIAL
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef DA_DIAL
#define DA_DIAL
#define	TIMEOUT		30
#define	ERR_FILE_OPEN	-1

#define DIAL_CBL_WRP	120	/* DIAL Cable wrap test			     */
#define DIAL_INT_WRAP 	130	/* DIAL local wrap test in internal device   */
#define DIAL_ORD_COMP	140	/* DIAL order compare			     */
#define	DIAL_RND_COMP	150	/* DIAL random order compare.		     */
#define	DIAL_RESET	210	/* DIAL reset.				     */

#define MAX_CBL_WRP	2	/* Max return code from LPFK cable wrap.     */ 
#define MAX_INT_WRAP 	3	/* Max return code from LPFK wrap test	     */
#define MAX_ORD_COMP	1	/* Max return code from LPFK order compare   */
#define MAX_RESET	2	/* Max return code from LPFK cable wrap.     */ 
				/* test					     */
#define	MAX_RND_COMP	1	/* Max return code from LPFK random compare  */
				/* test.				     */
 
#define IND_CBL_WRP	0	/* Index to test # 120 execute flag.	    */
#define IND_WRP_TST	1	/* Index to test # 130 execute flag.	    */
#define IND_ORDER_COMP	2	/* Index to test # 140 execute flag.	    */
#define IND_RANDOM_COMP	3	/* Index to test # 150 execute flag.	    */
#define IND_RESET	4	/* Index to test # 210 execute flag.	    */

#define PORT1		1       /* Physical Port #1 on the GIO adapter card  */
#define PORT2		2       /* Physical Port #2 on the GIO adapter card  */
#define A_PORT		3       /* Physical Port #1 and Port #2 for GIO      */

#define	QUEST		2
#define	INST		4
#define INST_QUEST	(QUEST | INST)
#define INSERT_WRAP	TRUE
#define REMOVE_WRAP	FALSE
#define	DIAL_TYPE	1
#define	HFT_TYPE	2

#define	YES		1

/* These tuXX are the error codes that are returned by exectu   */

int     tu10[] = { 1, 2, 3, 0};
int     tu20[] = { 1, 2, 0};
int     tu30[] = { 1, 2, 0};
int     tu40[] = { 1, 2, 0};
int     tu50[] = { 1, 2, 0};
int     tu60[] = { 1, 0};
int     tu70[] = { 1, 0};
int     tu80[] = { 1, 0};
int     tu90[] = { 1, 0};
int     tu100[] = { 1, 0};
int     tu110[] = { 1, 0};
int     tu120[] = { 1, 0};
int     tu130[] = { 1, 0};
int     tu140[] = { 1, 0};
int     tu150[] = { 1, 2, 0};
int     tu200[] = { 1, 2, 3, 4, 0};
int     tu210[] = { 1, 2, 3, 4, 0};
int     tu220[] = { 1, 0};


#endif 

