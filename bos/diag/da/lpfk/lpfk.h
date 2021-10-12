/* @(#)52	1.3  src/bos/diag/da/lpfk/lpfk.h, dalpfk, bos411, 9428A410j 3/4/94 08:51:52 */
/*
 *   COMPONENT_NAME: DALPFK
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


#ifndef DA_LPF
#define DA_LPF

#define	ERR_FILE_OPEN	-1

#define LPFK_CBL_WRP	60	/* LPFK Cable wrap test			     */
#define LPFK_INT_WRAP 	70	/* LPFK local wrap test in internal device   */
#define LPFK_LIGHT_ON	80	/* LPFK turn lights ON.			     */
#define LPFK_LIGHT_OFF 	90	/* LPFK turn lights OFF.		     */
#define LPFK_ORD_COMP	100	/* LPFK order compare			     */
#define	LPFK_RND_COMP	110	/* LPFK random order compare.		     */
#define	LPFK_RESET	200	/* LPFK reset.				     */

#define MAX_CBL_WRP	2	/* Max return code from LPFK cable wrap.     */ 
#define MAX_INT_WRAP 	3	/* Max return code from LPFK wrap test	     */
#define MAX_LIGHT_ON	1	/* Max return code from LPFK lights ON	     */
#define MAX_LIGHT_OFF 	1	/* Max return code from LPFK lights OFF	     */
#define MAX_ORD_COMP	1	/* Max return code from LPFK order compare   */
#define MAX_RESET	2	/* Max return code from LPFK cable wrap.     */ 
				/* test					     */
#define	MAX_RND_COMP	1	/* Max return code from LPFK random compare  */
				/* test.				     */
 
#define IND_CBL_WRP	0	/* Index to test # 60 execute flag.	    */
#define IND_WRP_TST	1	/* Index to test # 70 execute flag.	    */
#define IND_LIGHT_ON	2	/* Index to test # 80 execute flag.	    */
#define IND_LIGHT_OFF	3	/* Index to test # 90 execute flag.	    */
#define IND_ORDER_COMP	4	/* Index to test # 100 execute flag.	    */
#define IND_RANDOM_COMP	5	/* Index to test # 110 execute flag.	    */
#define IND_RESET	6	/* Index to test # 200 execute flag.	    */

#define FALSE		0      /* logical false.                              */
#define TRUE		1      /* logical true.                               */
#define	QUEST		2
#define	INST		4
#define INST_QUEST	(QUEST | INST)
#define INSERT_WRAP	TRUE
#define REMOVE_WRAP	FALSE
#define	HFT_TYPE	1
#define LPF_TYPE	2

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
