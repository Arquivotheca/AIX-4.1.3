/* @(#)52	1.2  src/bos/usr/include/netiso/tp_astring.c, sockinc, bos411, 9428A410j 3/5/94 12:41:22 */
/*
 * 
 * COMPONENT_NAME: (SOCKET) Socket services
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 26 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *****************************************************************/
/* $Header$ */
/* $Source$ */
/*	 (#)tp_astring.c	7.3 (Berkeley) 8/29/89 */
#ifndef _NFILE
#include <stdio.h>
#endif /* _NFILE */
char *tp_sstring[] = {
"ST_ERROR(0x0)",
"TP_CLOSED(0x1)",
"TP_CRSENT(0x2)",
"TP_AKWAIT(0x3)",
"TP_OPEN(0x4)",
"TP_CLOSING(0x5)",
"TP_REFWAIT(0x6)",
"TP_LISTENING(0x7)",
"TP_CONFIRMING(0x8)",
};

char *tp_estring[] = {
"TM_inact(0x0)",
"TM_retrans(0x1)",
"TM_sendack(0x2)",
"TM_notused(0x3)",
"TM_reference(0x4)",
"TM_data_retrans(0x5)",
"ER_TPDU(0x6)",
"CR_TPDU(0x7)",
"DR_TPDU(0x8)",
"DC_TPDU(0x9)",
"CC_TPDU(0xa)",
"AK_TPDU(0xb)",
"DT_TPDU(0xc)",
"XPD_TPDU(0xd)",
"XAK_TPDU(0xe)",
"T_CONN_req(0xf)",
"T_DISC_req(0x10)",
"T_LISTEN_req(0x11)",
"T_DATA_req(0x12)",
"T_XPD_req(0x13)",
"T_USR_rcvd(0x14)",
"T_USR_Xrcvd(0x15)",
"T_DETACH(0x16)",
"T_NETRESET(0x17)",
"T_ACPT_req(0x18)",
};
