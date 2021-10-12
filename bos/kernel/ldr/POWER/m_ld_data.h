/* @(#)15	1.6  src/bos/kernel/ldr/POWER/m_ld_data.h, sysldr, bos411, 9428A410j 8/16/93 17:40:22 */

#ifndef _H_M_LD_DATA
#define _H_M_LD_DATA


/*
 * COMPONENT_NAME: (SYSLDR) Program Management
 *
 * FUNCTIONS: Platform dependent data for loader
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

struct m_data {
	int		libtext_ralloc_rc;	/* Flag to indicate if segment
						   13 was allocated */
	vmhandle_t	libtext_save_sr;	/* Saved value of segment
						   register 13 if allocated */
	int		libdata_ralloc_rc;	/* Flag to indicate if segment
						   15 was allocated */
	vmhandle_t	libdata_save_sr;	/* Saved value of segment
						   register 15 if allocated */
	vmhandle_t	tmp_save_sr;		/* Temporary segment register
						   save area */
	};


#endif  /* _H_M_LD_DATA */
