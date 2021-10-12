/* @(#)61	1.4  src/bos/kernext/dlc/lan/lanstlst.h, sysxdlcg, bos411, 9428A410j 10/19/93 11:23:05 */
/*
 * COMPONENT_NAME: (SYSXDLCG) Generic Data Link Control
 *
 * FUNCTIONS: lanstlst.h
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
#ifndef _h_LANSTLST
#define _h_LANSTLST
/**********************************************************************/
/* station list declaration */
/**********************************************************************/

struct station_list {
    	ulong_t 	sta_hash;         /* rcv station table hash#    */
    	ulong_t 	call_retries;     /* call retries counter       */
    	ulong_t 	call_buf_addr;    /* find name buff address(old)*/
    	ulong_t 	find_buf_addr;    /* find name buff address(new)*/
    	ulong_t 	sta_cb_addr;      /* station cb address         */
    	u_char 	t1_ena;           /* t1 enable indicator        */
    	u_char 	t2_ena;           /* t2 enable indicator        */
    	u_char 	t3_ena;           /* t3 enable indicator        */
	u_char 	wakeup_needed; 	  /* local busy wakeup needed   */
    	ulong_t 	t1_ctr;           /* t1 counter                 */
    	ulong_t 	t2_ctr;           /* t2 counter                 */
    	ulong_t 	t3_ctr;           /* t3 counter                 */
    	u_char 	sapnum;           /* local sap list index       */
	u_char 	sta_active;	  /* station fully active       */
	u_char 	call_pend;	  /* call pending completion ind*/
	u_char 	in_use; 	  /* table entry in use indicate*/
        u_char  discv_allr;       /* discovery route status     */
        
};
#endif /* _h_LANSTLST */
