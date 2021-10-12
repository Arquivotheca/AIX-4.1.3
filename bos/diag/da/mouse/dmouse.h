/* @(#)68	1.6  src/bos/diag/da/mouse/dmouse.h, damouse, bos411, 9428A410j 5/27/94 15:14:46 */
/*
 *   COMPONENT_NAME: DAMOUSE
 *
 *   FUNCTIONS: Diagnostic header file.
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifndef	MOUSE_DEVICE
#define MOUSE_DEVICE	1

#define	MSE_ELA	10
#ifndef	CATD_ERR
#define CATD_ERR -1
#endif


#define MDD_DEVTU_01          10        /* mouse    device diag tu command 1 */
#define MDD_DEVTU_02          20        /* mouse    device diag tu command 2 */
#define MDD_DEVTU_03          30        /* mouse    device diag tu command 3 */
#define MDD_DEVTU_04          40        /* mouse    device diag tu command 4 */
#define MDD_DEVTU_05          50        /* mouse    device diag tu command 5 */
#define MDD_DEVTU_08          80        /* mouse    device diag tu command 8 */
#define	BUTTON	((char)(1))

#define PTR_DISABLE		0
#define PTR_INT_WRAP		1
#define PTR_RESOLUTION		2
#define PTR_DEFAULT		3
#define PTR_READ_DATA		4
#define PTR_SWITCH		5
#define PTR_SWITCH_MV		6

#define	ERR_FILE_OPEN	-1
#define	INVALID_RETURN	-1

#define	BUTTONS			1
#define	MOVEMENT		2
#define	WORKED_YES_NO		5

#define	SCALE_UNIT	10

#define	POS_RETRY	7
#define	RETRY		8

#define ADAP_DTX_REG	0x0048
#define ADAP_CMD_REG	0x0049

#define LEFT_BUTTON	0x10000
#define RIGHT_BUTTON	0x20000
#define MIDDLE_BUTTON	0x40000

#endif 
