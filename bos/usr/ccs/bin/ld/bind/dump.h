/* @(#)14	1.9  src/bos/usr/ccs/bin/ld/bind/dump.h, cmdld, bos411, 9428A410j 5/12/94 10:50:13 */
#ifndef Binder_DUMP
#define Binder_DUMP
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

/* Save infomation generating XREF symbol maps. */
struct save_stuff {
    RLD *rld;
    int link;
    long number;
} *saved_stuff;

extern RETCODE	dump_csect(CSECT *, int);
extern void	dump_rld(RLD *);

extern void	minidump_symbol(SYMBOL *, int, int, SYMBOL *);

#define MINIDUMP_NAME_LEN	25	/* Usual length of name */
#define HASH_FIELD_WIDTH 22		/* Length of printed hash string  */

#define MINIDUMP_HASH		0x0001
#define MINIDUMP_HASH_PAD	0x0002
#define MINIDUMP_SYMNUM		0x0004
#define MINIDUMP_SYMNUM_DBOPT11	0x0008
#define MINIDUMP_INPNDX		0x0010
#define MINIDUMP_LONG_INPNDX	0x0020
#define MINIDUMP_ADDRESS	0x0040
#define MINIDUMP_TYPE		0x0080
#define MINIDUMP_SMCLASS	0x0100
#define MINIDUMP_LEN_ALIGN	0x0200
#define MINIDUMP_CSECT_LEN_ALIGN	0x0400
#define MINIDUMP_SOURCE_INFO	0x2000

/* Flag variable for controlling some dump routines. */
extern int	dump_controls;
#define DUMP_GET_SMTYPE 1		/* If not set, get_smtype will return
					   "--" for internal-only smtypes. */
#define DUMP_SHOW_INPNDX 2		/* If set, show_inpndx will print
					   IMPORT instead of GENERATED for
					   generated, imported symbols. */

/* Routines to convert types to printable strings */
extern char	*get_sclass(unsigned char);
extern char	*get_smtype(unsigned char);
extern char	*get_smclass(unsigned char);
extern char	*get_reltype_name(unsigned char);

/* Routines to print a structure number or address */
extern char	*show_er(SYMBOL *, char *);
extern char	*show_sym(SYMBOL *, char *);
extern char	*show_rld(RLD *, char *);

extern char	*language_name(unsigned short);
extern int	show_inpndx(SYMBOL *, char *);

#endif /*  Binder_DUMP */
