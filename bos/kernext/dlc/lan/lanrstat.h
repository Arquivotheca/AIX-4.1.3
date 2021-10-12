/* @(#)59	1.3  src/bos/kernext/dlc/lan/lanrstat.h, sysxdlcg, bos411, 9428A410j 10/19/93 11:22:10 */
/*
 * COMPONENT_NAME: (SYSXDLCG) Generic Data Link Control
 *
 * FUNCTIONS:  lanrstat.h
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
#ifndef _h_LANRSTAT
#define _h_LANRSTAT
/**********************************************************************/
/* receive station table declaration  */
/**********************************************************************/
struct RCV_STA_TBL {
	union {
		char	cmp_string[8];	    /* compare string         */
		struct {
      		char cmp_string_raddr[6];   /* remote station address */
      		u_char cmp_string_lsap;      /* local sap received     */
      		u_char cmp_string_rsap;      /* remote sap received    */
		} s1;
      		struct {
		ulong_t cmp_w8765;            /* raddr bytes 8-7-6-5    */
      		ulong_t cmp_w4321;            /* raddr bytes 4-3 & saps */
		} s2;
	} u_cs;
    	u_char	call_pend;                  /* call pending indicator */
    	u_char	in_use;                     /* table entry in use ind */
    	char	fill[2];                    /* alignment              */
    	ulong_t	sta_list_slot;              /* station list slot #    */
};
#endif /* _h_LANRSTAT */
