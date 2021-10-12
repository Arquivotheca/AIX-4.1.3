/* @(#)22	1.7  src/bos/usr/ccs/bin/ld/bind/save.h, cmdld, bos41B, 9505A 1/17/95 17:47:10 */
#ifndef Binder_SAVE
#define Binder_SAVE
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

#define LOADER_VERSION 1	/* Version number for loader section header */

extern caddr_t	Shmaddr;		/* Global shmat address for output */
extern int	Queue_size;		/* Size of Queue[] */
extern int	first_text_index, first_data_index, last_data_index;
extern int	last_unresolved;	/* Size of unresolved_queue[] */
extern int	imp_symbols;		/* # of imported symbols */
#ifdef DEBUG
extern int	imp_exp_symbols;	/* # of imported and exported syms */
extern int	XO_imports;		/* # of imported, saved, XO symbols */
#endif
extern long	*symtab_index;
extern long	num_used_typchks;	/* Number of parm typecheck strings */
extern X_OFF_T	typchk_sect_offset;	/* Current offset for .typchk section*/
extern uint16	next_scn;		/* Next free section in output file */

extern CSECT	**Queue;		/* Queue of csects to be saved. */
extern SYMBOL	**unresolved_queue;	/* Queue of unresolved symbols */

/* Pointers to parts of output XCOFF file */
extern FILHDR	*Filehdr;		/* XCOFF header */
extern SCNHDR	*Scnhdr;		/* Section headers */

#ifdef GENERATE_PAD_HEADERS
extern SCNHDR	pad_section;		/* Initialized section hdr for PADs */
#endif

/* Offsets used when writing typchk values to the output file. */
extern struct typchk_values_t {
    TYPECHK	*t;
    X_OFF_T	ldr_val;		/* Offset within .loader section */
    X_OFF_T	sect_val;		/* Offset within .typchk section */
} *typchk_values;

/* Routines in symtab_save.c */
extern X_OFF_T	generate_symbol_table(long, X_OFF_T);
extern void	fixup_symbol_table(void);
extern void	init_typchk_info(void);
extern X_OFF_T	write_info_section(X_OFF_T);
extern X_OFF_T	write_line_numbers(X_OFF_T, long *);
extern X_OFF_T	write_exception_section(X_OFF_T);
extern X_OFF_T	write_symbol_table(X_OFF_T);

/* Routines in code_save.c */
extern X_OFF_T	collect_and_save_csects(void);

/*
 * Code section mappings -
 *	.text
 *	(1:_text) (2:PR,XO,SV,GL) (3:RO) (4:TB) (5:TI) (6:DB) (7:TFX) (8:_etext)
 *	.data
 *	(9:_data) (10:RW) (11:DS)
 *		(12:TCOVRFL) (13:TOC) (14:TC) (15:TCX)
 *		(16:UA) (17:DFX) (18:_edata)
 *	.bss
 *	(18:COMMONLO) (19:BS) (20:UC) (21:CM-RW) (22:_end or END)
 *
 * Note:  7:TFX, 17:DFX, and 15:TCX are used for handling TOC overflow.
 */

/* Major section codes. */
#define	MS_TEXT		0
#define	MS_DATA		1
#define	MS_BSS		2
#define MS_EXTERN	3
#define MS_MAX		3

/* (Minor) Section codes */
#define SC_EXTERN	0

#define	SC_CODE_LO	1	/* _text			*/
#define	SC_PR		2	/* PR, GL, SV, or XO		*/
#define	SC_RO		3	/* RO				*/
#define	SC_TB		4	/* TB				*/
#define	SC_TI		5	/* TI				*/
#define	SC_DB		6	/* DB				*/
#define SC_TFIXUP	7	/* Text fixup code		*/
#define	SC_CODE_HI	8	/* _etext			*/

#define	SC_DATA_LO	9	/* _data			*/
#define	SC_RW		10	/* RW				*/
#define	SC_DS		11	/* DS				*/

#define	SC_TCOVRFL	12	/* Toc Ovrflo (negative offsets)*/
#define	SC_TC0		13	/* TOC (toc anchor)		*/
#define	SC_TC		14	/* TC (positive offsets)	*/
#define SC_TC_EXT	15	/* TOC extension (overflow)	*/

#define	SC_UA		16	/* UA				*/
#define SC_DFIXUP	17	/* Data section fixup code	*/
#define	SC_DATA_HI	18	/* _edata			*/

#define	SC_BSS_LO	19	/*	 			*/
#define	SC_BS		19	/* BS				*/
#define	SC_UC		20	/* UC				*/
#define	SC_CM_RW	21	/* RW - CM			*/
#define	SC_BSS_HI	22	/* end or _end			*/

#define	MAXSECTIONS	23	/* maximum number of sections */

/* Structure for keeping track of CSECTs being saved for each minor section */
typedef struct sect_info {
    int max_align;			/* Max alignment for section */
    int heads;				/* Heads of lists of csects for each
					   possible section (or -1 if list
					   is empty).
					   OR after sections are sorted, the
					   index of the first csect for this
					   section (or -1). */
    int tails;				/* Tails of lists of csects for each
					   possible section (undefined if list
					   is empty).
					   OR after sections are sorted, the
					   index of the section following this
					   one (or one more than the number
					   of csects in queue).  This value is
					   always valid and is never -1. */
} SECT_INFO;
extern SECT_INFO sect_info[MAXSECTIONS];

extern short sect_mappings[MS_MAX+1];	/* Mapping from major sections to
					   output file sections */

#endif /* Binder_SAVE */
