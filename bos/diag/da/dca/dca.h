/* @(#)29	1.4  src/bos/diag/da/dca/dca.h, dadca, bos411, 9428A410j 1/5/93 09:16:44 */
/*
 *   COMPONENT_NAME: DADCA
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



#define SW_ERR 0x9999
#define NODEV 5
#define CHECK_KBD 99

#define MAX_BUF 1024
#define MAX_BUCKET 8

#ifndef CATD_ERR
#define CATD_ERR -1
#endif

#define S_MENU_NUM 0x854001
#define SA_MENU_NUM 0x854002
#define TI_MENU_NUM 0x854003
#define SBC_MENU_NUM 0x854004
#define ICC_MENU_NUM 0x854005
#define ICD_MENU_NUM 0x854006
#define ICA_MENU_NUM 0x854007
#define PC_MENU_NUM 0x854008
#define ICP_MENU_NUM 0x854009

#define CAT_NAME "ddca.cat"
#define MAX_MNU 7
 
#define TRUE 1
#define FALSE 0
#define YES 1
#define NO 2
#define uchar unsigned char
#define	NOT_CONFIG	-1
#define	OPEN_ERROR	0x99
#define	CONFIG_FRU	6
#define	OPEN_FRU	7
