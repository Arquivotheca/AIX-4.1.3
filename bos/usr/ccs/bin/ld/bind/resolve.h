/* @(#)21	1.5  src/bos/usr/ccs/bin/ld/bind/resolve.h, cmdld, bos411, 9428A410j 5/12/94 13:21:49 */
#ifndef Binder_RESOLVE
#define Binder_RESOLVE
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
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifdef DEBUG
extern int	dfs_depth;
#endif

extern struct resolve_info {
    STR *name;				/* Name that needs error message */
    int flags;				/* Type of message(s) needed */

/* Flags for types of messages about symbols. */
#define ERR_HID_UNRESOLVED	0x001
#define ERR_TOC_RESOLVE		0x002
#define ERR_TYPECHK		0x004
#define ERR_UNRESOLVED		0x008
#define ERR_EXPORT		0x010	/* Do not change value */
#define ERR_CM_BUMP		0x020
#define ERR_ALIGN_BUMP		0x040
#define ERR_DUPLICATE		0x080
#define ERR_SMCLASS_CHANGED	0x100
/*				0x200 */
#define ERR_KEEP		0x400	/* Do not change value */

#define ANY_UNRESOLVED (ERR_UNRESOLVED|ERR_HID_UNRESOLVED)
#if ERR_EXPORT != STR_EXPORT || STR_KEEP != ERR_KEEP
#error      Warning: Defines from strs.h have changed.
#endif

} *resolve_names;
extern int	resolve_names_flags;	/* Logical OR of all flags in
					   resolve_names[] array. */

extern void	dfs(SYMBOL *sym);
extern void	display_symbol(SYMBOL *, int);
extern void	display_resolve_errors(int);
extern void	free_resolve_names(void);

#endif /* Binder_RESOLVE */
