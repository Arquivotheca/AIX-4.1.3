/* @(#)28	1.5  src/bos/diag/da/tok/trn.h, datok, bos411, 9428A410j 9/24/93 14:27:31 */
/*
 *   COMPONENT_NAME: DATOK
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


/*#define NOTUS*/
/*#define TUDEBUG*/

#define MAX_BUF 1024
#define MAX_BUCKET 18

#ifndef CATD_ERR
#define CATD_ERR -1
#endif

#define CAT_NAME "dtoken.cat"
#define MAX_MNU 7
#define S_MENU_NUM 0x850001
#define SA_MENU_NUM 0x850002
#define TI_MENU_NUM 0x850003
#define NU_MENU_NUM 0x850004
#define WT_MENU_NUM 0x850005
#define CC_MENU_NUM 0x850006
#define CCT_MENU_NUM 0x850007
#define DC_MENU_NUM 0x850008
#define AW_MENU_NUM 0x850009
#define DW_MENU_NUM 0x85000a
#define AC_MENU_NUM 0x85000b
#define NB_MENU_NUM 0x85000c
#define EAONB_MENU_NUM 0x85000d
#define ENB_MENU_NUM 0x85000e
#define HW_MENU_NUM 0x85000f
 
#define TRUE 1
#define FALSE 0
#define YES 1
#define NO 2
#define NONTELE 1
#define TELE 2
#define DONT_KNOW 3
#define uchar unsigned char

#define SW_ERR 0x9999
#define BAD_NET 0x7777
#define ELA 2

#define MAXTEMP 40
 
#define	NO_ERROR	0
#define NOT_GOOD	-1
#define	NOT_CONFIG 	-1
#define	OPEN_ERROR	0x99
