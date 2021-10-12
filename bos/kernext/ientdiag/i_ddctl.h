/* @(#)95	1.2  src/bos/kernext/ientdiag/i_ddctl.h, diagddient, bos411, 9428A410j 4/5/93 18:35:13 */
/*
 * COMPONENT_NAME: (SYSXIENT) Ethernet Device Driver - Integrated Eth. adapter
 *
 * FUNCTIONS: ddctl.h
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
typedef volatile struct {
   int		state;
   dev_t	devno;			/* content minor numbers */
   dds_t 	*dds_ptr;		/* dds pointers for each adapter */
} dd_ctrl_t;
