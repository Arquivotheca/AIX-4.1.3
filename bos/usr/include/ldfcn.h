/* @(#)47	1.12  src/bos/usr/include/ldfcn.h, cmdaout, bos411, 9428A410j 7/28/92 16:36:03 */
/*	src/bos/usr/include/ldfcn.h, cmdaout, bos411, 9428A410j - 92/07/28 - 16:36:03  	*/
/*
 * COMPONENT_NAME: CMDAOUT
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 3, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_LDFCN
#define _H_LDFCN

#if !defined(AIXV3AR) && !defined(PORTAR) && !defined(PORT5AR) && \
defined(_POWER)
#define AIXV3AR
#endif /* Set archive format */
 
#ifndef LDFILE
struct	ldfile {
	int	_fnum_;		/* so each instance of an LDFILE is unique */
	FILE	*ioptr;		/* system I/O pointer value */
	long	offset;		/* absolute offset to the start of the file */
#ifdef AIXV3AR
	long	ar_off;		/* archive header offset for file (or zero) */
	long	ar_end_off;	/* offset of first ar hdr after the normal  */
				/* headers.				    */
#endif /* AIXV3AR */
	FILHDR	header;		/* the file header of the opened file */
	unsigned short	type;		/* indicator of the type of the file */
};

/*
 *	provide a structure "type" definition, and the associated
 *	"attributes"
 */

#define	LDFILE		struct ldfile
#define IOPTR(x)	x->ioptr
#define OFFSET(x)	x->offset
#define TYPE(x)		x->type
#define	HEADER(x)	x->header
#define LDFSZ		sizeof(LDFILE)
#ifdef AIXV3AR
#define	AR_OFF(x)	x->ar_off
#define	AR_END_OFF(x)	x->ar_end_off
#endif /* AIXV3AR */

/*
 *	define various values of TYPE(ldptr)
 */

#define LDTYPE	B16MAGIC	/* defined in terms of the filehdr.h include file */
#define TVTYPE	TVMAGIC		/* ditto */
#if u3b5
#define USH_ARTYPE	ARTYPE
#else
#define USH_ARTYPE	(unsigned short) ARTYPE
#endif
#if defined(PORTAR) || defined(PORT5AR) || defined(AIXV3AR)
#define ARTYPE	0177545
#else
#define ARTYPE	ARMAG
#endif

/*
 *	define symbolic positioning information for FSEEK (and fseek)
 */

#define BEGINNING	0
#define CURRENT		1
#define END		2

/*
 *	define a structure "type" for an archive header
 */

#if defined(PORTAR) || defined(PORT5AR)
typedef struct
{
	char	ar_name[16];
	long	ar_date;
	int	ar_uid;
	int	ar_gid;
	long	ar_mode;
	long	ar_size;
} archdr;

#define	ARCHDR	archdr
#else
#ifdef AIXV3AR
typedef struct
{
	long	ar_size;
	long	ar_nxtmem;
	long	ar_prvmem;
	long	ar_date;
	long	ar_uid;
	long	ar_gid;
	long	ar_mode;
	int	ar_namlen;
	char	ar_name[256];
} archdr;
#define	ARCHDR	archdr
#else
#define	ARCHDR	struct ar_hdr	/* ARCHIVE is defined in ts.h */
#endif
#endif
#define ARCHSZ	sizeof(ARCHDR)

/*
 *	define some useful symbolic constants
 */

#define SYMTBL	0	/* section nnumber and/or section name of the Symbol Table */

#define	SUCCESS		1
#define	CLOSED		1
#define	FAILURE		0
#define	NOCLOSE		0
#define	BADINDEX	-1L

#define	OKFSEEK		0

/*
 *	define macros to permit the direct use of LDFILE pointers with the
 *	standard I/O library procedures
 */

#ifdef _NO_PROTO
extern LDFILE *ldopen();
extern LDFILE *ldaopen();
#else
extern LDFILE *ldopen(char *,LDFILE *);
extern LDFILE *ldaopen(char *,LDFILE *);
#endif

#define GETC(ldptr)		getc(IOPTR(ldptr))
#define GETW(ldptr)		getw(IOPTR(ldptr))
#define FEOF(ldptr)		feof(IOPTR(ldptr))
#define FERROR(ldptr)		ferror(IOPTR(ldptr))
#define FGETC(ldptr)		fgetc(IOPTR(ldptr))
#define FGETS(s,n,ldptr)	fgets(s,n,IOPTR(ldptr))
#define FILENO(ldptr)		fileno(IOPTR(ldptr))
#define FREAD(p,s,n,ldptr)	fread(p,s,n,IOPTR(ldptr))
#define FSEEK(ldptr,o,p)	fseek(IOPTR(ldptr),(p==BEGINNING)?(OFFSET(ldptr)+o):o,p)
#define FTELL(ldptr)		ftell(IOPTR(ldptr))
#define FWRITE(p,s,n,ldptr)	fwrite(p,s,n,IOPTR(ldptr))
#define REWIND(ldptr)		rewind(IOPTR(ldptr))
#define SETBUF(ldptr,b)		setbuf(IOPTR(ldptr),b)
#define UNGETC(c,ldptr)		ungetc(c,IOPTR(ldptr))
#define STROFFSET(ldptr)	(HEADER(ldptr).f_symptr + HEADER(ldptr).f_nsyms * 18) /* 18 == SYMESZ */
#endif
 
#endif /* _H_LDFCN */
