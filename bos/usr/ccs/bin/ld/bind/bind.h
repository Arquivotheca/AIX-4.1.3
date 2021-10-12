/* @(#)12	1.17  src/bos/usr/ccs/bin/ld/bind/bind.h, cmdld, bos41B, 9504A 12/7/94 15:44:49 */
#ifndef Binder_BIND
#define Binder_BIND
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

#ifdef READ_FILE
#include <stdio.h>
#endif

#include <xcoff.h>
#include <ar.h>

/* Define a conventional name for the info section if not already defined. */
#ifndef _INFO
#define _INFO ".info"
#endif

#include <sys/ltypes.h>

/* Hierarchy of structures created from information in input files:
1. IFILE : Input (source) files to bind (includes import/export files)
   2. OBJECT : either single XCOFF objects or composite objects, such as
		archives
   2a. OBJ_MEMBER_INFO : information about objects making up composite
      				objects
   2b. OBJECT_INFO : Additional information about non-composite objects
      3. SECTION : Array of information about sections in an XCOFF object
      3. SRCFILE : Source files (from ".file" symbol table entries)
	 4. CSECT : Csects contained in a source file. (XTY_SD or STY_CM)
	    5. SYMBOL : List of symbols defined in CSECT (from XTY_LD
	    		or name on csect entry)
	    5. RLD : List of rld references contained in csect.
1. STR : Name structure
   2. SYMBOL: List of external (C_EXT) symbols with this name
   2. SYMBOL: List of hidden (C_HIDEXT) symbols with this name
   3. SYMBOL: List of external references (XTY_ER) to symbol with this name
*/
#include "typedefs.h"

/************************************************************************
 *	ifile	The "ifile" structure maintains information about
 *		the physical input files being processed.
 ************************************************************************/
typedef unsigned char IFILE_TYPE_T;
typedef unsigned char OBJECT_TYPE_T;

/* Allowable object types--values must fit in 1 byte (max value 255) */
/* These defines are used for ifile types as well */
#define O_T_UNKNOWN		0 /* Composite member not read yet */
#define O_T_IGNORE		1 /* Ignore member because of error */
#define O_T_SCRIPT		2 /* "script" (import) file */
#define O_T_EXPORT		3 /* export file */
#define O_T_OBJECT		4 /* XCOFF object */
#define O_T_SHARED_OBJECT	5 /* Shared XCOFF object */
#define O_T_ARCHIVE		6 /* Archive */

/* The rest are only used for object types */
#define O_T_ARCHIVE_SYMTAB	7 /* Archive global symbol table */
#define O_T_TRUNCATED		8 /* File corrupted--can't be used */

/* Allowable ifile types--values must fit in 1 byte (max value 255) */
#define	I_T_UNKNOWN		O_T_UNKNOWN
#define	I_T_IGNORE		O_T_IGNORE
#define	I_T_SCRIPT		O_T_SCRIPT
#define	I_T_EXPORT		O_T_EXPORT
#define	I_T_OBJECT		O_T_OBJECT
#define	I_T_SHARED_OBJECT	O_T_SHARED_OBJECT
#define	I_T_ARCHIVE		O_T_ARCHIVE

struct ifile {
#ifdef READ_FILE
    union {
	FILEDES		_fd;		/* file descriptor for input file */
	FILE		*_file;		/* FILE pointer (if reading file) */
    } u;
#define i_fd u._fd
#define i_file u._file
#else
    FILEDES		i_fd;
#endif

    STR			*i_name;	/* Input file name */
    OBJECT		*i_objects;	/* List of objects in file */
    uint		i_ordinal;	/* Ordinal number of file */
    IFILE_TYPE_T	i_type;		/* Type of file */

    unsigned char	i_access;	/* How is file accessed? */
#define I_ACCESS_SHMAT 1		/* May change to I_ACCESS_ANONMMAP */
#define I_ACCESS_MMAP 2			/* May change to I_ACCESS_ANONMMAP */

#ifdef READ_FILE
#define I_ACCESS_READ 3
#define I_ACCESS_SHMAT_READ 4		/* Changes to ..._SHMAT or ..._READ */
#define I_ACCESS_MMAP_READ 5		/* Changes to ..._MMAP or ..._READ */
#endif /* READ_FILE */

#define I_ACCESS_ANONMMAP 6

#if CENTERLINE || DEBUG
#define I_ACCESS_MALLOC 7		/* Not ordinarily used.  */
#endif

    unsigned		i_rebind:1;	/* Set if REBIND used to read file */
    unsigned		i_reinsert:1;	/* Set if REINSERT used to read file */
    unsigned		i_library:1;	/* Set if LIBRARY used to read file */
    unsigned		i_keepfile:1;	/* Set to keep all symbols in file */
    unsigned		i_closed:1;	/* Set if file is closed for good */
    caddr_t		i_map_addr;	/* ptr to beginning of file in mapped
					   memory, or ptr to complete file copy
					   in area (allocated by malloc()), if
					   i_access == I_ACCESS_MALLOC) */
    off_t		i_filesz;	/* File size returned by stat() */
    IFILE		*i_next, *i_auxnext; /* Links */
};

/************************************************************************
 *	object	The "object" structure maintains information
 *		about individual xcoff objects being processed.
 *	Note:	More than one "object" can be contained in a "file".  For
 *		now, only archives can contain more than one object.
 ************************************************************************/
struct object
{
	OBJECT		*o_next;	/* Pointer to next object in bind */
	IFILE		*o_ifile;	/* IFILE containing object */
	size_t		o_size;		/* Object size */
#ifndef DEBUG
	union {
		OBJECT_INFO	*info;	/* Pointer to object_info if object is
					   O_T_OBJECT. */
		SRCFILE	*srcf;		/* SRCFILEs for shared-objects or
					   script (import) files */
		SYMBOL	*_gst_syms;	/* List of symbols in global symbol
					   table. Only for archive objects. */
		struct {		/* Saved header info for shared-
					   objects before section is read. */
			uint16 num_sects;
			uint16 opt_hdr_size;
		} u;
	} u;
#	define o_info u.info
#	define o_srcfiles u.srcf
#	define o_gst_syms u._gst_syms
#	define o_num_sections u.u.num_sects
#	define o_opt_hdr_size u.u.opt_hdr_size
#else
	/* Don't use unions */
	OBJECT_INFO	*o_info;
	SRCFILE		*o_srcfiles;
	SYMBOL		*o_gst_syms;
	uint16		o_num_sections;
	uint16		o_opt_hdr_size;
#endif /* DEBUG */
	OBJ_MEMBER_INFO	*o_member_info;	/* If NULL, object is not subobject
					   Otherwise, info about this member */
	ulong		o_num_subobjects; /* If 0, object is not composite,
					     Otherwise, number of subobjects */

	OBJECT_TYPE_T	o_type;		/* Type of this object */
	OBJECT_TYPE_T	o_contained_in_type; /* Type of composite of which this
						object is a member--(for now,
						only legal composite is
						O_T_ARCHIVE). */
};

/* If additional composite objects are to be supported, new obj_member_info
   types will have to be defined. */
struct obj_member_info {
	STR		*o_member;	/* archive member name */
	off_t		o_ohdr_off;	/* Offset (in ifile) to archive hdr. */
	OBJECT		*o_parent;	/* Containing object */
};

/* Information about XCOFF objects.  Allocated only when object needs to be
   processed.  Unread archive members will not have this information. */
struct object_info {
	SRCFILE		*_srcfiles;	/* List of src files (C_FILE) in OBJ */
	SYMBOL		*_ext_refs;	/* List of external refs for OBJECT */
	int16		_num_sections;	/* # of sections (from XCOFF hdr) */
	uint16		_flags;	/* Flags */
#define OBJECT_HAS_DEBUG	1
#define OBJECT_HAS_TYPECHK	2
#define OBJECT_HAS_EXCEPT	4
#define OBJECT_HAS_INFO		8
#define OBJECT_NOT_READ		16
#define OBJECT_GLUE		32
#define OBJECT_USED		64	/* For -m (member) option */
#define OBJECT_HAS_BINCL_EINCL	128
	uint16		_debug_sect_i;	/* 0-based index for .debug section */
	uint16		_except_sect_i;	/* 0-based index for .except section */
	uint32		_num_symbols;	/* Number of symbols in symbol table */
	off_t		_symtab_offset;	/* Offset (in IFILE, not OBJECT)
					   to symbol table */
	caddr_t		_strtab_base;	/* String table (in memory) */
	uint32		_strtab_len;	/* Length of string table */
	union {
		SECTION	*_section_info;	/* Information about sections */
		off_t	_sections_offset;/* Offset (in OBJECT) to sect. hdrs */
	} u;
	ITEM		*_syms_lookup;	/* An ordered map from original symbol
					   table indexes to SYMBOLS (used to
					   look up symbols by symbol table
					   index). */

#	define oi_srcfiles	o_info->_srcfiles
#	define oi_ext_refs	o_info->_ext_refs
#	define oi_num_sections	o_info->_num_sections
#	define oi_flags		o_info->_flags
#	define oi_debug_sect_i	o_info->_debug_sect_i
#	define oi_except_sect_i	o_info->_except_sect_i
#	define oi_num_symbols	o_info->_num_symbols
#	define oi_symtab_offset	o_info->_symtab_offset
#	define oi_strtab_base	o_info->_strtab_base
#	define oi_strtab_len	o_info->_strtab_len
#	define oi_section_info	o_info->u._section_info
#	define oi_sections_offset o_info->u._sections_offset
#	define oi_syms_lookup	o_info->_syms_lookup
};

/***********************************************************************
 *	section	Contains information about sections in an XCOFF object
 ***********************************************************************/
struct section {
    uint16	sect_type;		/* Section type */
    uint16	sect_flags;
#define SECT_CODE		1	/* For TEXT, DATA, or BSS section */
#define SECT_RAWDATA		2	/* For TEXT or DATA section. */
#define SECT_OVERFLOW_NEEDED	4	/* Set in flags for primary section
					   header.  Reset when corresponding
					   overflow section header found. */
#define SECT_OVERFLOW_FOUND	8
#define SECT_TOC_REFERENCED	16	/* Set if any TOC-relative reference
					   was made to a symbol in this
					   section. */
    uint32	sect_size;		/* Size of section (from s_size) */
    union {
	caddr_t sect_base;		/* Address (in memory) of a section's
					   raw data.  If the file is being
					   read, this area is allocated.
					   Otherwise, it's the mapped,
					   memory address of the section.
					   Used for TYPCHK sections. */
	off_t	sect_offset;		/* Offset (in IFILE, not OBJECT) to a
					   section's raw data.
					   Used for DEBUG and EXCEPT sects. */
	off_t	raw_offset;		/* Offset (within IFILE) to address 0
					   of the section's raw data. Adding
					   a CSECT's address to raw_offset
					   yields the offset (within the IFILE)
					   to the raw data for the csect.
					   Used when SECT_RAWDATA is set. */
    } u;
    uint	csect_count;		/* Used if SECT_CODE is set. */
    /*  The remaining sections are only used for TEXT and DATA sections, except
	that l_reloc_count is initialized for all sections. */
    uint32	l_reloc_count;		/* Number of relocation items */
    off_t	l_reloc_base;		/* Offset (in IFILE) to reloc entries*/
    SYMBOL	*l_toc_anchor;		/* SYMBOL for TOC anchor, if any */
    CSECT_HEAP	l_csect_heap;		/* Heap for looking up csects by
					   address.  */
    off_t	l_linenum_last;		/* Offset (in OBJECT) to 1 past last
					   line number entry */
};

/***********************************************************************
 *	SRCFILE	Contains information about source files in an XCOFF
 *		object (as indicated by C_FILE symbol table entries).
 ***********************************************************************/
struct srcfile {
	SRCFILE		*sf_next;	/* Next file in XCOFF object */
	STR		*sf_name;	/* Name of source file */
	int32		sf_inpndx;	/* Input symbol table index of C_FILE*/
#define SF_GENERATED_INPNDX INT_MAX	/* Value used for generated srcfile */
	CSECT		*sf_csect;	/* Chain of csects for this file */
	OBJECT		*sf_object;	/* Containing XCOFF object */
#ifdef _CPUTYPE_FEATURE
	unsigned short	sf_flags;
#define SF_USED 0x0001
	unsigned char	sf_cputype;	/* CPU type */
#endif
};

/***********************************************************************
 *	SYMBOL:	Information about symbols defined or referred to in the
 *	XCOFF object.
 ***********************************************************************/
struct symbol {
	STR		*s_name;	/* Symbol name */
	SYMBOL		*s_next_in_csect; /* Next symbol in CSECT (or OBJECT)*/
	SYMBOL		*s_synonym;	/* Pointer to next symbol with same
					   name in appropriate chain (either
					   external, hidden, or external
					   reference) */
	uint32		s_addr;		/* Address of symbol (as specified in
					   symbol table). */
	int32		s_inpndx;	/* Original input symbol index value
					   in xcoff object file (or, if
					   S_INPNDX_MOD is set, offset into
					   internal array containing input
					   and output indices).  */
#define INPNDX_FIXUP		-5
#define INPNDX_GENERATED	-6
#define INPNDX_IMPORT		-7
#define INPNDX_IMPORT_TD	-8	/* To mark imported symbol with
					   a XMC_TD reference. */
#define INPNDX_ARCHIVE		-9

	long		s_number;	/* Ordinal number of symbol
					   (for messages). Symbols can be
					   renumbered if a GEN command is
					   executed after SAVE. */
	uint8		s_smclass;	/* Symbol storage-mapping class */
			/*	Storage Mapping Class (from syms.h)
				READ ONLY CLASSES: XMC_PR, XMC_RO, XMC_DB,
					XMC_GL, XMC_XO, XMC_SV, XMC_TI, XMC_TB
				READ WRITE CLASSES: XMC_RW, XMC_TC0, XMC_TC,
					XMC_DS, XMC_UA, XMC_BS, XMC_UC	*/
	uint8		s_smtype;	/* Symbol type */
				/* From (syms.h): XTY_ER, XTY_SD, XTY_LD,
				   XTY_CM */
/* Define new values for internal use--These are impossible values for a real
symbol table entry, because only 3 bits are allowed for a real entry. */
/* Special values used temporarily when order csects with special names */
#define XTY_CODE_LO 8
#define XTY_CODE_HI 9
#define XTY_DATA_LO 10
#define XTY_DATA_HI 11
#define XTY_BSS_HI 12
#define XTY_AR 13			/* Symbol from archive global symtab */
#define XTY_IS 14			/* Symbol imported from shared lib. */
#define XTY_IF 15			/* Symbol imported from import file */
#define imported_symbol(s) ((s)->s_smtype >= XTY_IS)

	uint16		s_flags;	/* Flags */
/* The following five flags are set when symbols are read initially.  They are
   never reset. */
#define S_XMC_XO	0x01		/* Symbol is imported at a fixed
					   address.  Usually its storage-
					   mapping class will be XMC_XO, but
					   it may be converted to XMC_TD. */
#define S_PRIMARY_LABEL 0x02		/* This is the symbol for the XTY_SD
					   symbol table entry. */
#define S_HIDEXT	0x04		/* Symbol type is C_HIDEXT.  If not set,
					   symbol type is C_EXT. */
#define	S_DUPLICATE	0x08		/* Symbol is contained in a csect
					   containing a duplicate symbol, or
					   the symbol is a duplicate itself. */
#define	S_DUPLICATE2	0x10		/* This symbol is a duplicate symbol.
					   If this bit is set, S_DUPLICATE
					   must be set as well.
					   Set when symbols are read.
					   Never reset. */
#define S_ARCHIVE_SYMBOL 0x20		/* Symbol is from an archive member */

/* The following six flags are set by resolve processing.  If resolve is
   called a second time, these flags must be reset. */
#define S_MARK		0x40		/* Mark bit for RESOLVE command */
#define S_ISTOC		0x80		/* Set if symbol is eligible for TOC
					   resolution.  (That is, it is a
					   hidden symbol with storage mapping
					   class==XMC_TC, length==4, and
					   exactly one RLD item to a global
					   symbol). */
#define S_VISITED	0x100		/* Symbol was visited during resolve().
					   A symbol is visited when the
					   RLD referencing the symbol is
					   processed.  When global symbols of
					   a given name are combined, only 1
					   symbol has S_MARK bit set.  The
					   S_VISITED bit will be set for the
					   other referenced symbols. */
#define S_RESOLVED_OK	0x200		/* The s_resolved field has been filled
					   in and can be used.  This can only
					   be set for internal symbols. */
#define S_TYPECHK_IMPLIED 0x400		/* The s_typechk field was picked up
					   from another symbol with the same
					   name. */
#define S_TYPECHK_USED	0x800		/* Symbol had typechk error--and error
					   message has already been printed. */
#define S_TOC_PRINTED	S_TYPECHK_USED	/* Internal TOC symbol was replaced,
					   and a message was already printed
					   for this symbol. */

/* The following flags are set by the addgl command.  They are never reset. */
#define S_LOCAL_GLUE	0x1000		/* Glue code added for local symbol */

/* The following two flags are set during save processing. */
#define S_SAVE		0x2000		/* Second mark bit for SAVE command. */
#define S_INPNDX_MOD	0x4000		/* s_inpndx is offset into internal
					   array containing input symndx and
					   output symndx. */

/* The following flag is used by a command that needs to use
   the s_number field temporarily.  It is used by the 'gen' command. */
#define S_NUMBER_USURPED 0x8000		/* s_number field used temporarily for
					   printing. */

#ifdef DEBUG
	/* Don't use unions if DEBUG is set */
	OBJECT		*s_object;
	CSECT		*s_csect;
	TYPECHK		*s_typechk;
	SYMBOL		*s_resolved;
	SYMBOL		*s_prev_in_gst;
#else
	union {
		OBJECT	*o;		/* External refs only and archive
					   symbols from global symbol table:
					   containing OBJECT  */
		CSECT	*csect;		/* Other symbols : Containing csect */
#		define s_csect u.csect
#		define s_object u.o
	} u;
	union {
		TYPECHK	*t;		/* Pointer to typechk string */
		SYMBOL	*r;		/* Points to resolved symbol if this
					   is a TOC hidden symbol */
		SYMBOL *_prev_in_gst;	/* Back link in chain of archive
					   symbols. */
#		define s_typechk u1.t
#		define s_resolved u1.r
#		define s_prev_in_gst u1._prev_in_gst
	} u1;
#endif
};
/***********************************************************************
 *	csect	The CSECT (control section) structure for
 *		processing csects read from object files
 ***********************************************************************/
struct csect {				/* Note:  Includes XTY_CM */
	uint32		c_len;		/* Length of csect */
	unsigned	c_TD_ref:1;	/* If 1, some symbol in csect has had
					   a XMC_TD reference. */
	unsigned	c_mark:1;	/* Mark bit for RESOLVE */
	unsigned	c_save:1;	/* Mark bit for SAVE */
	unsigned	c_align:5;	/* Csect alignment value */
	uint8		c_major_sect;	/* Mapped section (text/data/bss) */
	int16		c_secnum;	/* Section number (in input file) */
	/* Define dummy section numbers */
#define N_IMPORTS -10
#define N_GENERATED -11
#define N_UNKNOWN -12
#define N_FIXUP -13
	RLD		*c_first_rld;	/* First name used in csect */
	uint32		c_addr;		/* Address of symbol */
	uint32		c_new_addr;	/* Address of symbol in output OBJ */
	SRCFILE 	*c_srcfile;	/* Containing source file */
	CSECT		*c_next;	/* Next CSECT in SRCFILE */
	SYMBOL		c_symbol;	/* Symbol definition for csect.
					   NOTE: this is not a pointer.  */
};

/***********************************************************************
 *	ext_refs are also recorded in symbol structures
 ***********************************************************************/
#define er_name		s_name
#define er_typechk	s_typechk
#define er_inpndx	s_inpndx
#define er_synonym	s_synonym
#define er_smclass	s_smclass
#define er_flags	s_flags
#define er_next_in_object s_next_in_csect
#define er_object	s_object
#define er_number	s_number

/************************************************************************
 *	rld	The RLD (Relocation dictionary) items
 ************************************************************************/
struct rld
{
	SYMBOL		*r_sym;		/* Pointer to referenced symbol */
	CSECT		*r_csect;	/* Pointer to referencing csect */
	RLD		*r_next;	/* Pointer to next RLD referenced
					   in referencing Csect */
	uint32		r_addr;		/* Address of relocatable item */
	long		r_number;	/* Counter for RLDs */
	uint8		r_length;	/* Bit length of relocatable item */
	uint8		r_reltype;	/* Relocation type */
	uint16		r_flags;
#define RLD_RESOLVE_BY_NAME	0x01	/* Resolve symbol by name (should be
					   set if RLD refers to ER or to an
					   external (C_EXT) csect or label). */
#define RLD_EXT_REF		0x02	/* Reference is to XTY_ER */
#define RLD_TOCDATA_FIXUP	0x04	/* Reference needs fixup code. */
#define RLD_CANCELED		0x08	/* RLD not needed in loader section */
#define RLD_TOC_RELATIVE	0x10	/* r_reltype is R_TOC, R_TRL, or R_TRLA.
					   This bit is only set during resolve
					   for visited RLDs. */
/* Do not change the following three values */
#define RLD_FIXUP_USED		0x20	/* Referenced instruction fixed up */
#define RLD_WAS_FIXED_UP	0x40	/* Input object has fixed-up code. */
#define RLD_SIGNED		0x80	/* Relocatable item is signed */
#if RLD_WAS_FIXED_UP != R_FIXUP || RLD_SIGNED != R_SIGN \
	    || (RLD_FIXUP_USED << 1) != RLD_WAS_FIXED_UP
#error      Warning: Defines from "/usr/include/reloc.h" have changed.
#endif
#define RLD_TOCDATA_REF		0x100	/* Reference is to XMC_TD symbol. The
					   RLD must point to an XTY_ER symbol.
					   The RLD will be associated with a
					   generated XMC_TC csect, generated
					   because of a TOC-data reference to
					   an undefined symbol. */
#define RLD_OVERFLOW_FIXUP_OK	0x200	/* TOC overflow fixup is allowed for
					   this reference.  This should be set
					   whenever RLD_TOC_RELATIVE is set,
					   but is not set for symbols that
					   shouldn't be moved into the TOC,
					   such as symbols with reserved names
					   or symbols in csects with multiple
					   labels. */
#define RLD_FIXUP_OK		0x400	/* Space has been allocated in the
					   fixup area for this RLD.  If fixup
					   code is actually generated,
					   RLD_FIXUP_USED should be set. */
};
/* Initial values for RLD items come from the "struct reloc" entries in
   an object file.
   RLD			struct reloc
   ===			============
   r_sym		r_symndx (pointer to SYMBOL referred to by symbol table
					index is stored)
   r_addr		r_vaddr (copied directly)
   r_csect		r_vaddr (Address is used to determine which csect
					contains the relocatable item)
   r_length		r_rsize (bits 3-7)
   RLD_SIGNED bit	r_rsize (bit 0)
   r_reltype		r_rtype (copied directly)
*/
/************************************************************************
 *	ITEM	Utility data structure
 ************************************************************************/
struct item {
    ulong	item_sym_index;
    SYMBOL	*item_symbol;
};
#define item_key item_sym_index
/************************************************************************
 *	HEADERS	Utility union for examining file headers.
 ************************************************************************/
typedef union {
    uint16	magic;
    FILHDR	xcoff_hdr;
    FL_HDR	archive_hdr;
} HEADERS;

/************************************************************************
 * Temporary XCOFF auxiliary header, copied to output file if appropriate.
 ************************************************************************/
extern AOUTHDR			temp_aout_hdr;

#endif /* Binder_BIND */
