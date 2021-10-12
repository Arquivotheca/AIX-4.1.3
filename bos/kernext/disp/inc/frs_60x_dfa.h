/* @(#)49	1.2  src/bos/kernext/disp/inc/frs_60x_dfa.h, dispccm, bos411, 9428A410j 7/5/94 11:32:32 */

/*
 *
 * COMPONENT_NAME: (dispcfg) Display Configuration
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/*
 *   ROM structure and macro definitions for DFA table
 */

#ifndef	_H_FRS_60X_DFA
#define	_H_FRS_60X_DFA

typedef	struct
{
	ulong_t		section_id;
	ulong_t		addr_GOB_FB_A_8;
	ulong_t		addr_GOB_FB_B_8;
	ulong_t		addr_GOB_FB_A_16;
	ulong_t		addr_GOB_FB_B_16;
	ulong_t		addr_GOB_FB_A_24;
	ulong_t		addr_GOB_FB_B_24;
	ulong_t		addr_GOB_FB_A_24_DITHER_332;
	ulong_t		addr_GOB_FB_B_24_DITHER_332;
	ulong_t		addr_GOB_CL_TYPE_A;
	ulong_t		addr_GOB_OL_TYPE_A;
	ulong_t		addr_GOB_PI_TYPE_A;
} rscan_60x_dfa_table_entry_t;


typedef	struct
{
	ulong_t				version;
	ulong_t				length;	/* Length in bytes */
	ulong_t				num_sections;
	rscan_60x_dfa_table_entry_t	basic;	/* Basic DFA table */
	rscan_60x_dfa_table_entry_t	opaque;	/* Opaque expansion */
	rscan_60x_dfa_table_entry_t	transp;	/* Transparent expansion */
} rscan_60x_dfa_table_t;

#endif /* ! _H_FRS_60X_DFA */

