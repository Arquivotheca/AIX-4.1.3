/* @(#)86	1.1  src/bos/diag/da/niokta/dkeytaba.h, dakbd, bos411, 9428A410j 2/28/94 10:36:56 */
/*
 *   COMPONENT_NAME: DAKBD
 *
 *   FUNCTIONS: Diagnostic header file.
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef tablet_adap
#define tablet_adap

#ifndef		CATD_ERR		/* catalog error return code      */
#define		CATD_ERR	-1      /* catalog error return code for  */
					/* the old NLS code used by RIOS  */
#endif 

#define TEST1   	0x10 /* NIO Keyboard Fuse test              */
#define TEST2   	0x20 /* NIO Keyboard read SRAM test         */
#define TEST3   	0x30 /* NIO Keyboard UART test              */
#define TEST4		0x40 /* NIO Tablet Port wrap test.          */

#define PTR_TST_1       0	/* pointer to test # 0 execute flag.      */
#define PTR_TST_2	1	/* pointer to test # 1 execute flag.      */
#define PTR_TST_3	2	/* pointer to test # 2 execute flag.      */
#define PTR_TST_4 	3	/* pointer to test # 3 execute flag.      */

#define	INSERT_WRAP	TRUE
#define	REMOVE_WRAP	FALSE
#define	ERR_FILE_OPEN	-1
#define KEYTABA_LED	0x821
#define BAD		-1
#endif 
