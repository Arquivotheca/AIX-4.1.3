/* @(#)58	1.3  src/bos/kernext/dlc/lan/lanmem.h, sysxdlcg, bos411, 9428A410j 10/19/93 11:21:41 */
/*
 * COMPONENT_NAME: (SYSXDLCG)  Generic Data Link Control
 *
 * FUNCTIONS: common header file for Generic Data Link Control
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
char *ptr_corr;   			/* correlator vector          */
char *ptr_nsa; 				/* nsa vector              */
char *ptr_mac; 				/* mac vector              */
char *ptr_lsap; 			/* lsap vector             */
char *ptr_resp; 			/* response vector         */
char *ptr_tid; 				/* target id vector        */
char *ptr_sid; 				/* source id vector        */
char *ptr_s_nsa; 			/* source id nsa vector    */
char *ptr_t_nsa; 			/* target id nsa vector    */
char *ptr_object; 			/* object name vector      */
