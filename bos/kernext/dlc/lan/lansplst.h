/* @(#)60	1.4  src/bos/kernext/dlc/lan/lansplst.h, sysxdlcg, bos411, 9428A410j 10/19/93 11:22:38 */
/*
 * COMPONENT_NAME: (SYSXDLCG) Generic Data Link Control
 *
 * FUNCTIONS: lansplst.h
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
#ifndef _h_LANSPLST
#define _h_LANSPLST
/**********************************************************************/
/* sap list declaration */
/**********************************************************************/
struct sap_list {
	ulong_t 	addn_retries;           /* add name retries counter   */
    	ulong_t 	addn_buf_addr;          /* add name buff address (old)*/
    	ulong_t 	find_self_addr;         /* add name buff address (new)*/
    	ulong_t 	sap_cb_addr;            /* sap control block address  */
    	ulong_t 	t1_ctr;                 /* t1 counter                 */
    	ulong_t 	t1_ena;                 /* t1 enable indicator        */
#ifdef cw
    	unsigned   :24;            /* reserved                   */
	u_char   fill           ;    /* reserved              */
#endif
	u_char   sap_active     ;    /* sap fully active      */
	u_char   addn_ready     ;    /* sap ready to add name */
	u_char   addn_echo      ;    /* "add name" will echo  */
	u_char   find_self_echo ;    /* "find self" will echo */
	u_char   addn_pend      ;    /* add name pending      */
	u_char   listen_pend    ;    /* listen pending        */
	u_char   in_use         ;    /* table entry in use    */
/* LEHb defect 43788 */
	u_char   wakeup_needed  ;    /* netd wakeup needed    */
/* LEHe */
};
#endif /* _h_LANSPLST */
