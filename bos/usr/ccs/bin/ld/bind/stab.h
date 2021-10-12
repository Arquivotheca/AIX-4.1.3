/* @(#)23	1.5  src/bos/usr/ccs/bin/ld/bind/stab.h, cmdld, bos41B, 9505A 1/17/95 17:49:10 */
#ifndef Binder_STAB
#define Binder_STAB
/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

extern void	init_stabs(X_OFF_T, caddr_t);
extern int	put_debug_name(SYMENT *, char *, uint16, uint32*, uint32
#ifdef STABCMPCT_NODUPSYMS
			       , int
#endif
			       );

extern int	save_unique_stabstring(SYMENT *, char *, int, int);
/* Define bits for last parameter of save_unique_stabstring */
#define SUS_NONE 0
#define SUS_TO_DEBUG 1
#define SUS_IS_DELETABLE 2

extern X_OFF_T	finish_stab_section(X_OFF_T, SCNHDR *, uint16 *);
extern char	*do_continuation(char *, uint32 *, char **);

extern OBJECT	*stab_obj;

#ifdef DEBUG
extern SRCFILE	*stab_sf;
#endif

extern void	process_deferred_stabs(void);
extern void	reset_stab_mappings(void);

#endif /* Binder_STAB */
