/* @(#)40       1.3  src/bos/kernext/mps_tok/mps_cmd.h, sysxmps, bos411, 9432A411a 8/4/94 21:59:44 */
/*
 *   COMPONENT_NAME: sysxmps
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_MPS_CMD
#define _H_MPS_CMD

/*****************************************************************************/
/*                  Modify Receive Options Command Structure                 */
/*****************************************************************************/
typedef struct {
	uchar	cmd;		/* MODIFY.RECEIVE.OPTIONS "0x17"            */
	uchar   rsv_1;          /* Reserved                                 */
	uchar	retcode;        /* Set by adapter on return                 */
	uchar	rsv_3;          /* Reserved                                 */
	ushort	recv_op;        /* New setting for the receive options      */
	uchar	passwd[8];      /* Access password                          */
} mod_recv_op;


/*****************************************************************************/
/*                  Set Group Command Structure                              */
/*****************************************************************************/
typedef struct {
	uchar	g_addr[6];      /* Group address                            */
	ushort	num;            /* Number of sequential address             */
	uchar	type;           /* Selects the group address type           */
	uchar	location;
} g_addr;

/*****************************************************************************/
/*                  Set Function Command Structure                           */
/*****************************************************************************/
typedef struct {
	uchar	cmd;		/* SET.GROUP.ADDRESS or RESET.GROUP.ADDRESS */
	uchar   rsv_1;          /* Reserved                                 */
	uchar	retcode;        /* Set by adapter on return                 */
	uchar	rsv_3[3];       /* Reserved                                 */
	uchar	g_addr[6];      /* Group address                            */
	ushort	num;            /* Number of sequential address             */
	uchar	type;           /* Selects the group address type           */
	uchar	rsv_16;         /* Reserved                                 */
} g_address;

/*****************************************************************************/
/*                  Set Function Command Structure                           */
/*****************************************************************************/
typedef	struct {
	uchar	cmd;		/* SET.FUNCTIONAL.ADDRESS                   */
	uchar   rsv_1;          /* Reserved                                 */
	uchar	retcode;        /* Set by adapter on return                 */
	uchar	rsv_3[3];       /* Reserved                                 */
	uchar	f_addr[4];      /* functional address                       */
} f_address;

#endif /* _H_MPS_CMD */

