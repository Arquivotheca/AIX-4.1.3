/* @(#)07	1.7  src/bos/usr/include/aixif/x25xlate.h, sockinc, bos411, 9428A410j 11/5/93 11:59:31 */
/*
 * COMPONENT_NAME: (CMDNET) Network commands 
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993 
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifndef _H_X25XLATE
#define _H_X25XLATE

typedef struct 
{
	u_short vc_type; 	
	short rcv_wndsiz;
	short xmit_wndsiz;
	char remote_dte[16];
	short rcv_pktsiz;
	short xmit_pktsiz;
	char opt_fac[33];	/* 32 ascii digits (16 hex bytes) + null */
	char callusr_data[33];	/* 32 ascii digits (16 hex bytes) + null */
	char rpoa_selec[40];
	short cug_indx;
	short cug_indxout;
	u_short logical_chann;
	u_short port_num;
	struct in_addr ip_addr;
	int callusr_data_len;
	int opt_fac_len;
}XLATE_ENTRY;

#define SVC_VC_TYPE	1
#define PVC_VC_TYPE	2

struct x25_xlate_req
{
	XLATE_ENTRY *tabptr;
	int num_hosts;
	int tabsiz;
};



#endif  /*_H_X25XLATE */
