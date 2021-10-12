/* @(#)26	1.8  src/bos/usr/ccs/bin/ld/bind/symbols.h, cmdld, bos411, 9428A410j 1/28/94 11:33:48 */
#ifndef Binder_SYMBOLS
#define Binder_SYMBOLS
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

/* Symbol table access routines */
#ifdef ALIGNPTRS
extern SYMENT	*aligned_sym_ptr(caddr_t, int);
extern AUXENT	*aligned_aux_ptr(caddr_t, int);
#else
#define aligned_sym_ptr(b,n) (SYMENT*)((int)(b)+(n)*SYMESZ)
#define aligned_aux_ptr(b,n) (AUXENT*)((int)(b)+(n)*SYMESZ)
#endif

/* String table access routines */
extern void	set_strtab_base(OBJECT *);
extern STR	*get_sym_name(OBJECT *, SYMENT *, int);
extern STR	*get_sym_name2(OBJECT *, SYMENT *, int, int);
extern STR	*get_aux_name(OBJECT *, AUXENT *, int, int);

/* TYPECHK routine variables */
extern int	hash_section;		/* First .typchk section, in case
					   x_snhash is not set properly in
					   symbol table entries.  */
/* TYPECHK routines */
extern TYPECHK	*create_TYPECHK(AUXENT *, OBJECT *, int, STR *);

/* SRCFILE routines */
extern SRCFILE	*get_init_SRCFILE(OBJECT *, STR *);

/* SYMBOL routines */
extern SYMBOL	*get_SYMBOLs(int);
extern SYMBOL	*create_SYMBOL(OBJECT *, CSECT *, SYMBOL *, int, STR *,
			       SYMENT *, AUXENT *, const int);
extern SYMBOL	*create_imported_SYMBOL(STR *, CSECT *, SYMBOL *,
					unsigned char, unsigned int);
extern SYMBOL	*create_er_SYMBOL(STR *, int, OBJECT *, AUXENT *);
extern SYMBOL	*get_ER(void);
extern SYMBOL	*create_global_archive_SYMBOL(STR *, OBJECT *);
extern int	total_symbols_allocated(void);
extern int	total_ers_allocated(void);

/* CSECT routines */
extern CSECT	*get_CSECTs(int);
extern CSECT	*get_init_CSECT(void);
extern CSECT	*create_CSECT(SYMENT *, AUXENT *);
extern void	free_csect(CSECT *);
extern int	total_csects_allocated(void);

/* RLD routines */
extern int	total_rlds_allocated(void);
extern RLD	*get_RLDs(int);

#endif /* Binder_SYMBOLS */
