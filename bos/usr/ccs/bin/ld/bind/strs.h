/* @(#)25	1.3  src/bos/usr/ccs/bin/ld/bind/strs.h, cmdld, bos411, 9428A410j 1/28/94 11:35:54 */
#ifndef Binder_STRS
#define Binder_STRS
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

#include <loader.h>
#include <typchk.h>

#include "typedefs.h"

/***********************************************************************
 * TYPECHK structure:	Each unique type-checking hash value found in
 *			input files is stored in a TYPECHK structure.
 ***********************************************************************/
struct typechk {
    TYPECHK	*t_next;		/* Next typechk in bucket */

    union {
	TYPCHK	_typechk;
	char *_cp_typechk;		/* Pointer to typechk if
					   t_len > sizeof(TYPCHK) */
	char _c_typechk[sizeof(TYPCHK)]; /* Typchk if t_len <= sizeof(TYPCHK)
					    (and t_len != TYPCHKSZ) */
    } h;
#define t_typechk	h._typechk
#define t_cp_typechk	h._cp_typechk
#define t_c_typechk	h._c_typechk

    int16	t_len;			/* If t_len != TYPCHKSZ, preserve the
					   string but don't use it to detect
					   mismatches. */
    long	t_value;		/* Index into typchk_values[] for
					   offsets of used typchks */
};

extern unsigned char universal_hash[4];

#define OLDTYPCHKSZ 8

/* Global function for TYPECHKs */
extern TYPECHK	*put_TYPECHK(caddr_t, off_t);

/***********************************************************************
 * STR structure:  Each unique name found in input files is stored in
 *	a STR structure.  Lists of symbols are anchored here.
 ***********************************************************************/
struct str {
    char	*name;			/* Actual string (null terminated) */
    SYMBOL	*str_syms[3];
#define first_ext_sym	str_syms[0]	/* First C_EXT symbol with this name */
#define first_hid_sym	str_syms[1]	/* First C_HIDEXT symbol with name */
#define refs		str_syms[2]	/* External refs to this name */
    STR		*alternate;		/* Paired dotted name or plain name */
    off_t	str_value;		/* Offset in some section or index
					   into some internal array.  When this
					   value is in use, a flag will be
					   set. (See #defines below.) */
    short	len;			/* Length of string name, without \0 */

    uint16	flags;
#define STR_ISSYMBOL	0x01		/* Set if name is a symbol (some names
					   are used for filenames, etc.).  If
					   this flag is set, one of the
					   str_syms[] lists must be non-empty,
					   or the STR_KEEP or STR_EXPORT flag
					   must be set. */

/* The following six flags are set by symbol resolution.  If symbol
   resolution must be done a second time, the flags must be reset. */
#define STR_NO_DEF	0x02		/* No non-deleted C_EXT definition
					   exists for this symbol name. */
#define STR_RESOLVED	0x04		/* The external symbols with this name
					   have been resolved.  */
#define STR_TOC_RESOLVED 0x08		/* At least one TOC symbol with this
					   name has been resolved. */
#define STR_DS_EXPORTED 0x80		/* A descriptor with this name
					   (minus the .) has been exported. */
#define STR_ERROR	0x100		/* A resolve-time error exists for a
					   symbol with this name.  An internal
					   array indicates the type of error
					   for printing at the end of symbol
					   resolution. */
#define	STR_IMPORT	0x40		/* DO NOT CHANGE THIS VALUE
					   Kept symbol instance was imported. */

/* The following two flags are set if a symbol with this name is exported or
   is the entrypoint, respectively. */
#define	STR_EXPORT	0x10		/* DO NOT CHANGE THIS VALUE
					   Symbol with this name has been
					   exported (with the "export" sub-
					   command or with an export file). */
#define	STR_ENTRY	0x20		/* DO NOT CHANGE THIS VALUE
					   Symbol with this name is entry point
					   (specified by the "entry"
					   subcommand). */

#if STR_EXPORT != L_EXPORT || STR_ENTRY != L_ENTRY || STR_IMPORT != L_IMPORT
#error      Warning: Defines from "/usr/include/loader.h" have changed.
#endif

#define STR_SYSCALL	0x200		/* Symbol with this name has been
					   exported with "syscall" attribute.
					   This bit is reset if an export
					   for the same symbol name is
					   encountered without the "syscall"
					   attribute. */
#define STR_KEEP	0x400		/* Keep symbol with this name in
					   output file (specified with the
					   "keep" subcommand or the -u option
					   to 'ld'). */

/* The following three flags are used during SAVE processing. */
#define STR_STRING_USED	0x800		/* This string belongs in the output
					   file string table.  The str_value
					   field contains its offset within
					   the string table. */
/* When one of LOADER_USED or LOADER_IMPID_USED is defined, str_value contains
   the offset (into the loader-section string table) or import ID.  If both are
   defined, str_value is an offset into an array containing both the loader-
   section string-table offset and the import ID for the same name. This can
   only occur if a loader-section variable and an import file have the same
   name. */
#define LOADER_USED	0x1000		/* Symbol used in loader section */
#define LOADER_IMPID_USED 0x2000	/* Symbol name is import-id string */

#define LOADER_RENAME	0x4000		/* Symbol renamed for loader section */

#define STR_ER_OUTPUT	0x8000		/* An ER was generated in the output
					   symbol table for symbol with this
					   name.  */
#define STR_ER_QUEUED	STR_ER_OUTPUT	/* SHARED with above flag:  Symbol
					   with this name has been queued
					   for output in the loader section. */
};

/* Structure for hash-table chains for STRs.  Plain names (without a leading
   dot) will be in the STR structure in one of these HASH_STR structures.
   A dot name (beginning with a single '.') is allocated as a separate STR
   structure, and is found by traversing the "alternate" field of the plain
   name.

   HASH_STRs are allocated in blocks.  The first HASH_STR in each block
   contains the number of additional HASH_STRs in the block in the s.len
   field. The 'next' field of the first HASH_STR points to the next block
   of HASH_STRs.  This technique allows all names to be examined without
   scanning the hash table itself, which could be sparsely populated. */
struct hash_str {
    STR		s;
    HASH_STR	*next;			/* Next HASH_STR with same hash; OR
					   next block of HASH_STR structures.*/
#ifdef DEBUG
    short	hash;			/* Computed hash number */
#endif
};

/* Global function for STRs */
extern int total_STRS(void);
extern int hash(char *, int, int);
extern STR *putstring(char *);
extern STR *putstring_len(char *, int);	/* Second argument is maximum length
					   in case string is not terminated. */
extern STR *lookup_stringhash(char *);
extern STR *lookup_dotname(char *);	/* dot is not part of argument */

/* Unique STR used for all null strings */
extern STR	NULL_STR;

/* Root of list of all HASH_STRs */
extern HASH_STR	HASH_STR_root;

/* Shorthand notation */
#define for_all_STRs(r, i, h, str) \
    for (r = &HASH_STR_root; r; r = r->next) \
    for (i = 0, h = r+1; i < r->s.len; h++, i++) \
    for (str = &h->s; str; str = (str == &h->s) ? str->alternate : NULL)

#endif /* Binder_STRS */
