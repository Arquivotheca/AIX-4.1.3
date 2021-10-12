/* @(#)33	1.2  src/bos/diag/da/siosl/kbddev/dkbd.h, dakbdsl, bos411, 9428A410j 12/9/93 11:20:16 */
/*
 * COMPONENT_NAME: (dkbd.h) header file for keyboard device diagnostic
 *			application.
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include	<nl_types.h>

#ifndef _dkbd
#define _dkbd
#ifndef	CATD_ERR
#define	CATD_ERR	-1
#endif
#define ERR_FILE_OPEN	-1

#define MAXTEMP 	40 

#define TEST1		0x10
#define TEST2		0x20
#define TEST3		0x30
#define TEST4		0x40
#define TEST5		0x50
#define TEST6		0x60

#define PTR_TST1	0	/* pointer to test # 1 execute flag.         */
#define PTR_TST2 	1	/* pointer to test # 2 execute flag.         */
#define PTR_TST3 	2	/* pointer to test # 3 execute flag.         */
#define PTR_TST4 	3	/* pointer to test # 4 execute flag.         */
#define PTR_TST5 	4	/* pointer to test # 5 execute flag.         */
#define PTR_TST6 	5	/* pointer to test # 6 execute flag.         */
#define ELA_PTR 	6	/* pointer to ela rc offsets.		     */

#define MAX_BUF 	1024 
#define MAX_ERROR 	4

#define CANCEL_KEY_ENTERED	7
#define EXIT_KEY_ENTERED	8
#define TU1_IOCTL_OPEN_ERR	0x10
#define TU2_IOCTL_OPEN_ERR	0x20
#define TU3_IOCTL_OPEN_ERR	0x30
#define TU4_IOCTL_OPEN_ERR	0x40
#define TU5_IOCTL_OPEN_ERR	0x50
#define TU6_IOCTL_OPEN_ERR	0x60

#define KB101	"BFB0"
#define KB102	"BFB1"
#define KB106	"BFB2"
#define SN101	0x921
#define SN102	0x922
#define SN106	0x923
#define KDB_TYPE	0

typedef struct	TUCB_SYS	
{
	nl_catd	catd;
	long	ad_mode;
	int	kbtype;
	int	mach_fd; 
};

#endif 
