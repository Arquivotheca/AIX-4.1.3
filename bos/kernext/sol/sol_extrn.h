/* @(#)59	1.3  src/bos/kernext/sol/sol_extrn.h, sysxsol, bos411, 9428A410j 5/14/91 14:33:38 */
#ifndef _H_SOL_EXTRN
#define _H_SOL_EXTRN
/*
 * COMPONENT_NAME: (SYSXSOL) - Serial Optical Link Device Handler Include File
 *
 * FUNCTIONS: sol_extrn.h
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
extern struct irq_tbl irq_tbl;
extern struct cddq cddq;
extern struct sla_tbl sla_tbl;
extern struct isq isq;
extern struct imcs_addresses imcs_addresses;
extern int imcs_host;
extern uchar cck_proc[];

extern struct intr sla_handler[MAX_NUM_SLA];   

extern struct imcsdata imcsdata;

#endif /* _H_SOL_EXTRN */
