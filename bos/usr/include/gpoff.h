/* @(#)13	1.10  src/bos/usr/include/gpoff.h, cmdld, bos411, 9428A410j 3/29/93 08:39:16 */
/* gpoff.h	5.3 - 87/05/07 - 19:07:14 */
/*
 * COMPONENT_NAME: LIBIN
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27, 9
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_GPOFF
#define _H_GPOFF 1

#include <sys/limits.h>
#include <sys/dir.h>
#include <nlist.h>

struct exec {
	unsigned char   a_magic[2]; /* magic number */
	unsigned char   a_flags;    /* flags, see below */
	unsigned char   a_cpu;      /* cpu id */
	unsigned char   a_hdrlen;   /* length of header */
	unsigned char   a_unused;   /* reserved for future use */
	unsigned short  a_version;  /* version stamp */
	long            a_text;     /* size of text segment */
	long            a_data;     /* size of data segment */
	long            a_bss;      /* size of bss segment */
	long            a_entry;    /* entry point */
	long            a_misc;     /* misc., e.g. initial stack pointer */
	long            a_syms;     /* symbol table size */
				    /* SHORT FORM ENDS HERE */
	long            a_trsize;   /* text relocation size */
	long            a_drsize;   /* data relocation size */
	long            a_tbase;    /* text relocation base */
	long            a_dbase;    /* data relocation base */
	long            a_lnums;    /* size of line number section */
	long            a_toffs;    /* offset of text from start of file */
};

#define A_MAGIC0    (unsigned char)0x01
#define A_MAGIC1    (unsigned char)0x03
#define BADMAG(X) ((X).a_magic[0] != A_MAGIC0 || (X).a_magic[1] != A_MAGIC1)

/* Shared library text image magic numbers */

#define A_SLMAG0    (unsigned char)0x01	
#define A_SLMAG1    (unsigned char)0x04	

/* CPU Id of TARGET machine (byte order coded in low order 2 bits): */

#define A_NONE   0x00    /* unknown */

#define A_I8086  0x04    /* intel i8086/88 */

#define A_VAX    0x08    /* dec vax */

#define A_NS32K  0x0C    /* national semiconductor 32xxx */
#ifdef ns32k
#define A_SELF	 A_NS32K
#endif

#define A_PDP11  0x06    /* dec pdp11 */

#define A_Z8K2   0x07    /* zilog z8002 */

#define A_M68K   0x0B    /* motorola m68000 */

#define A_370    0x0F    /* ibm 370 architecture */

#define A_AIWS   0x13
#define A_SELF	 A_AIWS

#define A_I80186 0x14    /* intel i80186/88 */

#define A_M68K20 0x1B    /* motorola m68020 */

#define A_I80286 0x24    /* intel i80286 */

#define A_I80386 0x34    /* intel i80386 */


#define A_BLR(cputype)  ((cputype&0x01)!=0) /* TRUE if bytes left-to-right */
#define A_WLR(cputype)  ((cputype&0x02)!=0) /* TRUE if words left-to-right */

/* Flags: */
#define A_TOFF  0x01    /* text offset */
#define A_STRS	0x02	/* string table present */
#define A_HDREXT 0x08   /* extended header present */
#define A_EXEC  0x10    /* executable */
#define A_SEP   0x20    /* separate I/D */
#define A_PURE  0x40    /* pure text */
#define A_SHLIB	0x80	/* file uses shared libraries */

/* Offsets of various things: */
#define A_MINHDR	32
#define A_TEXTPOS(X)    (((X).a_flags&A_TOFF)?(X).a_toffs:(long)(X).a_hdrlen)
#define A_DATAPOS(X)    (A_TEXTPOS(X) + (X).a_text)
#define A_HASRELS(X)	((X).a_hdrlen>(unsigned char)A_MINHDR)
#define A_HASEXT(X)	((X).a_hdrlen>(unsigned char)(A_MINHDR+8))
#define A_HASLNS(X)	((X).a_hdrlen>(unsigned char)(A_MINHDR+16))
#define A_HASTOFF(X)	((X).a_hdrlen>(unsigned char)(A_MINHDR+20))
#define A_TRELPOS(X)    (A_DATAPOS(X) + (X).a_data)
#define A_DRELPOS(X)    (A_TRELPOS(X) + (X).a_trsize)
#define A_SYMPOS(X)     (A_TRELPOS(X) + \
		     	    (A_HASRELS(X) ? ((X).a_trsize + (X).a_drsize): 0))
#define A_LINPOS(x)     (A_SYMPOS(x) + (x).a_syms)
#define A_NAMEPOS(x)    (A_LINPOS(x) + (A_HASLNS(x) ? ((x).a_lnums) : 0))

struct  reloc
{       long            r_vaddr;    /* virtual address of reference */
	unsigned short  r_symndx;   /* internal segnum or extern symbol num */
	unsigned short  r_type;     /* relocation type */
};
#define RELOC struct reloc
#define RELOCSZ sizeof(struct reloc)

/* r_type values: */
#define R_ABS      0
#define R_RELBYTE  2
#define R_PCRBYTE  3
#define R_RELWORD  4
#define R_PCRWORD  5
#define R_RELLONG  6
#define R_PCRLONG  7
#define R_REL3BYTE 8
#define R_KBRANCH  9
#define R_SEG86    10
#define R_SEG286   11
#define R_KCALL    12

/* r_symndx for internal segments */
#define S_ABS      0xffff	/* ((unsigned short)-1) */
#define S_TEXT     0xfffe	/* ((unsigned short)-2) */
#define S_DATA     0xfffd	/* ((unsigned short)-3) */
#define S_BSS      0xfffc	/* ((unsigned short)-4) */

/* symbol table entry */
struct syment {
    union {
	char		_n_name[8];	/* non-flex version */
	struct {
	    long	_n_zeroes;	/* flexname == 0 */
	    long	_n_offset;	/* offset into string table */
	    } _n_n;
	char		*_n_nptr[2];	/* allows for overlaying */
    } _n;
    long            n_value;        /* symbol value */
    unsigned char   n_sclass;       /* storage class */
    unsigned char   n_numaux;       /* number of auxiliary entries */
	union
	{
		unsigned short	_n_type; /* language base and derived type */
	} _n_tylc;
};

/* Include files <syms.h> and <nlist.h> also define n_type. */
#ifndef	n_type
#define n_type		_n_tylc._n_type
#endif /* n_type */

#define SYMENT struct syment
#define SYMESZ sizeof(struct syment)

#define n_name          _n._n_name
#define n_nptr		_n._n_nptr[1]
#define n_zeroes	_n._n_n._n_zeroes
#define n_offset	_n._n_n._n_offset

/* low bits of storage class (section) */
#define N_SECT      07  /* section mask */
#define N_UNDF      00  /* undefined */
#define N_ABS       01  /* absolute */
#define N_TEXT      02  /* text */
#define N_DATA      03  /* data */
#define N_BSS       04  /* bss */
#define N_COMM      05  /* (common) */

/* high bits of storage class */
#define N_CLASS   0370  /* storage class mask */
#define C_NULL    0000
#define C_AUTO    0010  /* automatic variable */
#define C_EXT     0020  /* external symbol */
#define C_STAT    0030  /* static */
#define C_REG     0040  /* register variable */
#define C_EXTDEF  0050  /* external definition */
#define C_LABEL   0060  /* label */
#define C_ULABEL  0070  /* undefined label */
#define C_MOS     0100  /* member of structure */
#define C_ARG     0110  /* function argument */
#define C_STRTAG  0120  /* structure tag */
#define C_MOU     0130  /* member of union */
#define C_UNTAG   0140  /* union tag */
#define C_TPDEF   0150  /* type definition */
#define C_USTATIC 0160  /* undefined static */
#define C_ENTAG   0170  /* enumeration tag */
#define C_MOE     0200  /* member of enumeration */
#define C_REGPARM 0210  /* register parameter */
#define C_FIELD   0220  /* bit field */
#define C_BEGCOM  0230  /* beginning of common section */
#define C_COMMEM  0240  /* member of common section */
#define C_ENDCOM  0250  /* end of common section */
#define C_BLOCK   0300  /* ".bb" or ".eb" */
#define C_FCN     0310  /* ".bf" or ".ef" */
#define C_EOS     0320  /* end of structure */
#define C_FILE    0330  /* file name */


/* language base and derived type (used by symbolic debugger): */
/* Base type is low 4 bits: */
#define T_NULL      0
#define T_ARG       1       /* (used internally by compiler) */
#define T_CHAR      2       /* character */
#define T_SHORT     3       /* short integer */
#define T_INT       4       /* integer */
#define T_LONG      5       /* long integer */
#define T_FLOAT     6       /* floating point */
#define T_DOUBLE    7       /* double word */
#define T_STRUCT    8       /* structure  */
#define T_UNION     9       /* union  */
#define T_ENUM      10      /* enumeration  */
#define T_MOE       11      /* member of enumeration */
#define T_UCHAR     12      /* unsigned character */
#define T_USHORT    13      /* unsigned short */
#define T_UINT      14      /* unsigned integer */
#define T_ULONG     15      /* unsigned long */
/* Derived types are replicated 2 bits at a time up to 6 times: */
#define DT_NON      0       /* no derived type */
#define DT_PTR      1       /* pointer */
#define DT_FCN      2       /* function */
#define DT_ARY      3       /* array */

#include <linenum.h>

/* CPU characteristics entry, returned by AOgetdesc */
typedef struct cpuchar {
	char *name;		/* ascii name of cpu */
	char id;		/* cpu id -- lookup key */
	char flags;             /* values from below */
	char intsize;		/* size of int on this cpu */
	char rnd;		/* small rounding boundary */
	short pad;		/* large padding (page) boundary */
	long seg;		/* segment size */
	long misc;		/* default miscellaneous value */
} AOdesc;

#define AO_sep 0x01		/* cpu supports separate i/d space */
#define AO_shr 0x02             /* cpu supports sharable but non-separate */

#define DIMNUM 4
#define FILNMLEN MAXNAMLEN	/* limits.h */

/*
 *	AUXILIARY ENTRY FORMAT
 */

union auxent {
    struct {
    	long x_tagndx;	/* str, un, or enum tag indx */
    	union {
	    struct {
	        unsigned short x_lnno;	/* declaration line number */
	        unsigned short x_size;}	/* str, union, array size */
              x_lnsz;
	    long x_fsize;}	/* size of function */
    	  x_misc;
    	union {
	    struct {   		/* if ISFCN, tag, or .bb */
    	    	long x_lnnoptr;	/* ptr to fcn line # */
    	    	long x_endndx;}	/* entry ndx past block end */
    	      x_fcn;
    	    struct {   		/* if ISARY, up to 4 dimen. */
    	    	unsigned short x_dimen[DIMNUM];}
    	      x_ary;}
    	  x_fcnary;}
      x_sym;
    struct {char x_fname[FILNMLEN];} x_file;};

#define	AUXENT	union auxent
#define	AUXESZ	sizeof(AUXENT)


/* Extended header information */

struct exthdr  {
    unsigned short ax_size;        /* total size of extension */
    unsigned short ax_type;        /* type of extension */
    unsigned short ax_flags;       /* e.g. execution model */
    unsigned short ax_nsegs;       /* number of segment entries */
};

/* Segment descriptor table for extended headers - i8086/i286 */

struct segent {
    unsigned short as_type;     /* segment type */
    unsigned short as_flags;    /* segment attributes */
    unsigned short as_num;      /* segment number */
    unsigned short as_nlnno;    /* # lineno entries */
    long as_filep;              /* position (offset) in file */
    long as_psize;              /* size of segment in file */
    long as_vsize;              /* virtual size */
    long as_rsvd1;              /* reserved */
    long as_rsvd2;              /* reserved */
    long as_lnptr;              /* position of lineno entries */
};

/* Shared library table entry */

struct slent {
    unsigned long sl_off, sl_addr;
};

/* as_type (strict subset of STL): */
#define AST_NULL 0
#define AST_TEXT 1  /* code segment */
#define AST_DATA 2  /* data segment */

/* as_flags (strict subset of STL): */
#define ASF_HUGE    0x0002         /* segment contains huge model data */
#define ASF_BSS     0x0004         /* segment contains implicit bss */
#define ASF_SHARE   0x0008         /* segment is sharable */
#define ASF_EXPDOWN 0x0010         /* segment expands down */
#define ASF_SEG     0x8000         /* always on for segments */

/* ax_type: */
#define AXT_INTEL   1
#define AXT_SHLIB   2

/* ax_flags (for ax_type == AXT_INTEL): */
#define AXF_SSS     0x0001         /* separate stack segment */
#define AXF_MCS     0x0002         /* multiple code segments */
#define AXF_MDS     0x0004         /* multiple data segments */
#define AXF_HDS     0x0008         /* huge data present */
#define AXF_OVLY    0x0010         /* code overlay */
#define AXF_FPH     0x0080         /* Floating point hardware required */
#define AXF_ABS     0x0400         /* Absolute addresses present */

#endif /* _H_GPOFF */
