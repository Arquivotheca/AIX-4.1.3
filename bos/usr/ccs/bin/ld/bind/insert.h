/* @(#)18	1.2  src/bos/usr/ccs/bin/ld/bind/insert.h, cmdld, bos411, 9428A410j 1/28/94 11:33:31 */
#ifndef Binder_INSERT
#define Binder_INSERT
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

/* In archive.c */
/* Variable */
extern SYMBOL	*reading_archive_object_symbol;

/* Functions */
extern SYMBOL	*read_archive_member_for_symbol(SYMBOL *);
extern RETCODE	read_archive_symbols(OBJECT *);
extern void	allocate_archive_objects(IFILE *, OBJECT *, FL_HDR *, off_t);

/* In impexp.c */
extern RETCODE	read_impexp_object(OBJECT *, int, off_t);

/* In xcoff.c */
extern RETCODE	read_section_rlds(OBJECT *, int);
extern void	read_xcoff_symbols(OBJECT *, off_t, STR *);

/* In insert.c */
extern RETCODE	handle_insert(IFILE *, HEADERS *);
extern RETCODE	insert_deferred_files(void);
extern RETCODE	read_object_symbols(OBJECT *, off_t);

/* In shared.c */
extern void	read_shared_object_symbols(OBJECT *, off_t);

/* In glue.c */
extern SYMBOL	*glue_ext_symbol;

#endif /* Binder_INSERT */
