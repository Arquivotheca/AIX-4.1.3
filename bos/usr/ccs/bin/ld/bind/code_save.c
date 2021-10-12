#if lint == 0 && CENTERLINE == 0
#pragma comment (copyright, "@(#)88	1.54  src/bos/usr/ccs/bin/ld/bind/code_save.c, cmdld, bos41B, 9504A 1/6/95 08:27:12")
#endif

/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS: collect_and_save_csects
 *
 *   STATIC FUNCTIONS:
 *		assign_typchks
 *		call_dfs2
 *		check_special_names
 *		dfs2
 *		dbopt1_print
 *		group_csect_by_smclass
 *		order_glue_csects
 *		prebind
 *		process_csect
 *		rel_ovfl
 *		save_csects
 *		do_fixup
 *		do_relocation
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/param.h>
#include <sys/debug.h>

#include "global.h"
#include "bind.h"
#include "error.h"
#include "strs.h"

#include "dump.h"
#include "save.h"
#include "match.h"
#include "ifiles.h"
#include "symbols.h"
#include "objects.h"
#include "util.h"

/* Union declaration */
union instruction_t {
    uint32 i;
    char bytes[4];
    struct {
	unsigned int opcode: 6;
	signed int li : 24;
	unsigned int aa : 1;
	unsigned int lk : 1;
    } branch;
    struct {
	unsigned int opcode: 6;
	signed int li : 26;		/* For branch-displacement updating
					   without having to worry about
					   shifting deltas. */
    } branch2;
    struct {
	unsigned int opcode : 6;
	unsigned int rt : 5;
	unsigned int ra : 5;
	signed int d: 16;
    } dform;
};

/* Opcode definitions */
#define OP_ADDI		14
#define OP_ADDIS	15
#define OP_BRANCH	18		/* b, bl, ba, bla */
#define OP_LWZ		32
#define OP_STW		36

#define REAL_MAX_BRANCH_OFFSET ((1<<25)-4)
#define REAL_MIN_BRANCH_OFFSET (-(1<<25))

/* Structure definitions */
struct abs_info {
    int	is;
    int	was;
};

/* Defines for fixup code */
/* If the next define is 1, we have at least one kind of fixup code that will
   save the original instruction before the fixup code.  Regardless of the
   value, a dummy branch is saved at the beginning of the entire fixup section,
   and a check is made when undoing fixup code for the original instruction. */
#define FIXUP_SAVE 0

/* Define maximum length of fixup sequences. Cases for which fixup code are
   needed for data-in-TOC are counted in resolve().  Cases for which fixup
   code is needed for TOC overflow are counted in group_csect_by_smclass().
   If a single instance of fixup code is used for both data-in-TOC and TOC
   overflow, the maximum code length is not MAX_TOCOVERFLOW_FIXUP_LENGTH +
   MAX_TOCDATA_FIXUP_LENGTH, but is MAX_OVERALL_FIXUP_LENGTH.  An
   adjustment for this is made when TOC overflow cases are detected. */
#define MAX_TOCDATA_FIXUP_LENGTH	(5+FIXUP_SAVE)
#define MAX_TOCOVERFLOW_FIXUP_LENGTH	(4+FIXUP_SAVE)
#define MAX_OVERALL_FIXUP_LENGTH	(6+FIXUP_SAVE)

/* The FIXUP csect has overhead of 1 word for a dummy branch and 3
   words for a traceback table. */
#define FIXUP_OVERHEAD_WORDS 4
#define FIXUP_OVERHEAD_BYTES 16
#define FIXUP_TRACEBACK_WORDS 3
#define FIXUP_TRACEBACK_BYTES 12

/* Definitions for TOC overflow */
#define REAL_ARCH_LIMIT 65536
#ifdef DEBUG
static long	ARCH_LIMIT = REAL_ARCH_LIMIT;
#else
#define ARCH_LIMIT	65536
#endif

/* Static variables */
static CSECT	**temp_queue;
static int	toc_size;
static int	last;
static int	pre_TOC_index = SC_DS;
static uint32	raw_text_target_org;
static uint32	raw_data_target_org;
static long	toc_anchor_address;

static long	neg_offset;
static long	cur_fixup_ptr;
static CSECT 	*text_fixup_CSECT = NULL;

/* Mapping from section number to section number, and from section number
   to major section.  The first half of the mapping is initialized to the
   identity mapping. This information is needed by the mapping code in
   group_csect_by_smclass(). */
static struct {
    short subsect;
    short section;
} mappings[MAXSECTIONS] = {
    /* Undefined symbols or imported symbols */
    {SC_EXTERN, MS_EXTERN},

    /* .text */
    {SC_CODE_LO, MS_TEXT},
    {SC_PR, MS_TEXT},
    {SC_RO, MS_TEXT},
    {SC_TB, MS_TEXT},
    {SC_TI, MS_TEXT},
    {SC_DB, MS_TEXT},
    {SC_TFIXUP, MS_TEXT},
    {SC_CODE_HI, MS_TEXT},

    /* .data */
    {SC_DATA_LO, MS_DATA}, {SC_RW, MS_DATA}, {SC_DS, MS_DATA},
    /* TOC entries */
    {SC_TCOVRFL, MS_DATA}, {SC_TC0, MS_DATA},
    {SC_TC, MS_DATA}, {SC_TC_EXT, MS_DATA},
    /* Other .data sections */
    {SC_UA, MS_DATA}, {SC_DFIXUP, MS_DATA}, {SC_DATA_HI, MS_DATA},

    /* .bss */
    {SC_BS, MS_BSS}, {SC_UC, MS_BSS}, {SC_CM_RW, MS_BSS}, {SC_BSS_HI, MS_BSS}
    };

/* A list of all text sections: The list is terminated with -2. */
static short text_sections[MAXSECTIONS+1] = {
    SC_CODE_LO, SC_PR, SC_RO, SC_TB, SC_TI, SC_DB, SC_TFIXUP, SC_CODE_HI, -2};

/* A list of all data sections: The list is terminated with -2. */
static short data_sections[MAXSECTIONS+1] = {
    SC_DATA_LO, SC_RW, SC_DS,
    SC_TCOVRFL, SC_TC0, SC_TC, SC_TC_EXT,
    SC_UA, SC_DFIXUP, SC_DATA_HI, -2};

#ifdef DEBUG
static int dfs2_depth;
#endif

/* Static variables for special external variable names */
static STR *ptrgl_name;
static STR *PTRGL_name;

/* Global variables */
int		Queue_size;
CSECT		**Queue = NULL;
SYMBOL		**unresolved_queue;
int		last_unresolved = 0;
int		first_text_index, first_data_index, last_data_index;
SECT_INFO	sect_info[MAXSECTIONS];

#define make_int_switches() \
      (Switches.dbg_opt1 << 0) \
    | (Switches.dbg_opt2 << 1) \
    | (Switches.dbg_opt5 << 2) \
    | (Switches.dbg_opt7 << 3) \
    | (Switches.execute  << 4) \
    | (Switches.loadabs  << 5) \
    | (Switches.bigtoc   << 6) \
    | (Switches.verbose  << 7) \
    | (Switches.strip    << 8) \
    | (Switches.fold     << 9)

#define SWITCHES_DBG_OPT1 1
#define SWITCHES_DBG_OPT2 2
#define SWITCHES_DBG_OPT5 4
#define SWITCHES_DBG_OPT7 8
#define SWITCHES_EXECUTE  16
#define SWITCHES_LOADABS  32
#define SWITCHES_BIGTOC   64
#define SWITCHES_VERBOSE  128
#define SWITCHES_STRIP    256
#define SWITCHES_FOLD     512

/* Forward declaration */
static void	check_special_names(void);
static void	do_relocation(CSECT *, uint32, const int);
static X_OFF_T	save_csects(const int);
static RETCODE	group_csect_by_smclass(CSECT **, int, const int);
static void	prebind(caddr_t, CSECT *, RLD *, SYMBOL *, struct abs_info *,
			const int);
static int	rel_ovfl(caddr_t, CSECT *, RLD *,
			 int32, const int32, const int);
static int	do_fixup(uint32, unsigned long, CSECT *, RLD *, int32,
			 const int);

/************************************************************************
 * Name: dfs2			REORDER Processor for CSECTs		*
 *									*
 * Purpose: Traverse CSECT graph in depth-first-sort order.
 *
 *	Each visited CSECT is saved in queue[].  In addition, each
 *	unresolved symbol is saved in unresolved_queue[] for generating
 *	the proper symbol table.
 *									*
 * Returns: Nothing
 *									*
 ************************************************************************/
static void
dfs2(SYMBOL *sym,
     CSECT **queue)
{
    int		cs_is_descriptor;
    CSECT	*cs;
    RLD		*r;
    SYMBOL 	*er;
#ifdef DEBUG
    SYMBOL *save_sym = sym;		/* So dbx can see original argument. */
#endif

#ifdef DEBUG
    if (sym == NULL) {
	internal_error();
	return;
    }
#endif

    if (sym->s_flags & S_SAVE)
	return;

#ifdef DEBUG
    if ((bind_debug & (SAVE_DEBUG|DEBUG_LONG)) == (SAVE_DEBUG|DEBUG_LONG)) {
	int i;
	for (i = dfs2_depth; i > 0; i--)
	    say(SAY_NO_NLS | SAY_NO_NL, ".");
	say(SAY_NO_NLS, "dfs2([%s]%s%s%s{0x%x})",
	    show_sym(sym, NULL),
	    (sym->s_flags & S_HIDEXT) ? "<" : "",
	    sym->s_name->name,
	    (sym->s_flags & S_HIDEXT) ? ">" : "",
	    sym);
    }
#endif

    sym->s_flags |= S_SAVE;		/* Mark symbol */

    if (sym->s_name->flags & STR_IMPORT) {
	imp_symbols++;
#ifdef DEBUG
	if (sym->s_name->flags & STR_EXPORT)
	    imp_exp_symbols++;		/* Count passed-through exports */
	if (sym->s_smclass == XMC_XO)
	    XO_imports++;
#endif
    }

    cs = sym->s_csect;
    if (cs->c_save)
	return;				/* This csect visited */
    cs->c_save = 1;			/* Mark csect */
    queue[last++] = cs;			/* Enqueue csect */

    if (imported_symbol(sym))		/* No successors for imported syms */
	return;

#ifdef DEBUG
    dfs2_depth++;
#endif

    cs_is_descriptor = (cs->c_symbol.s_smclass == XMC_DS);
    /* Edges for DFS are RLDs for csect */
    for (r = cs->c_first_rld; r; r = r->r_next) {

#ifdef DEBUG
	/* Any symbol reachable from a saved symbol must have already been
	   visited during resolve processing.  In addition, all referenced
	   names must have STR_RESOLVED set. */
	if (!(r->r_sym->s_flags & S_VISITED)
	    || ((r->r_flags & RLD_RESOLVE_BY_NAME)
		&& !(r->r_sym->s_name->flags & STR_RESOLVED)))
	    internal_error();
#endif

	if (r->r_flags & RLD_RESOLVE_BY_NAME) {
	    if (r->r_sym->s_name->flags & STR_NO_DEF)
		sym = NULL;
	    else {
		sym = r->r_sym->s_name->first_ext_sym;

		/* If glue code and the real routine both exist, any descriptor
		   must refer to the real code.  NOTE:  No check is made to be
		   sure the name on the descriptor and the routine name
		   match. */
		if (cs_is_descriptor
		    && (sym->s_flags & S_LOCAL_GLUE))
		    sym = sym->s_synonym; /* This should never assign NULL. */
	    }

	    if (sym == NULL) {
		/* We have an unresolved reference.  Mark the ER symbol so
		   that the ER will be written if a symbol table is generated.
		   Make sure such a name is only saved once. */
		if (r->r_flags & RLD_TOCDATA_REF) {
		    /* This means r->r_sym->s_smclass == XMC_TD.  The first ER
		       with this name may not be the one saved, so we modify the
		       storage-mapping class of the ER that will be saved. */
		    r->r_sym->s_name->refs->s_smclass = XMC_TD;
		    r->r_flags &= ~RLD_OVERFLOW_FIXUP_OK;
		}
		if (!(r->r_sym->s_name->flags & STR_ER_QUEUED)) {
		    /* Queue first ER with name */
		    er = r->r_sym->s_name->refs;
		    r->r_sym->s_name->flags |= STR_ER_QUEUED;
		    DEBUG_MSG(SAVE_DEBUG, (SAY_NO_NLS,
					   "Queueing undefined symbol %s",
					   r->r_sym->s_name->name));
		    er->s_flags |= S_SAVE;
		    /* Save symbol to put in loader section symbol table. */
		    unresolved_queue[last_unresolved++] = er;
		}
		continue;
	    }
	}
	else {
	    sym = r->r_sym->s_resolved;
	    if (sym == NULL)
		continue;
	}
	dfs2(sym, queue);
    }
#ifdef DEBUG
    dfs2_depth--;
#endif
} /* dfs2 */
/************************************************************************
 * Name: call_dfs2
 *									*
 * Purpose: 	Static routine to call dfs2().
 *		(Used as a parameter for match.)
 *									*
 ************************************************************************/
static RETCODE
call_dfs2(STR *name)
{
    /* Only resolved names are allowed. */
    if ((name->flags & (STR_RESOLVED | STR_NO_DEF)) == STR_RESOLVED
	&& (name->first_ext_sym->s_flags & (S_MARK|S_SAVE)) == S_MARK)
	dfs2(name->first_ext_sym, temp_queue);

    return RC_OK;
}
/************************************************************************
 * Name: process_csect
 *									*
 * Purpose: 	Go through list of RLDs for each CSECT saved by noreorder
 *		processing to check for data-in-TOC stuff.
 *									*
 ************************************************************************/
static void
process_csect(CSECT *cs,
	      CSECT **queue)
{
    RLD		*r;
    SYMBOL 	*sym, *er;

    if (cs->c_save)
	return;				/* This csect visited */

    cs->c_save = 1;			/* Mark csect */
    queue[last++] = cs;			/* Enqueue csect */

    if (cs->c_secnum == N_IMPORTS) {
	/* Check all symbols for passed-through exports and XO-imports */

	/* We won't be visiting all marked symbols in an
	   import file, so we must be sure that all
	   "MARKed" symbols are "SAVEed" as well. */
	for (sym = &cs->c_symbol; sym; sym = sym->s_next_in_csect)
	    if (sym->s_flags & S_MARK) {
		sym->s_flags |= S_SAVE;
		imp_symbols++;
#ifdef DEBUG
		/* Count passed-through imports */
		if (sym->s_name->flags & STR_EXPORT)
		    imp_exp_symbols++;
		if (sym->s_smclass == XMC_XO)
		    XO_imports++;
#endif
	    }
    }
    else {
	/* This code duplicates a part of dfs2(), saving information for
	   references to undefined symbols. */
	for (r = cs->c_first_rld; r; r = r->r_next) {

#ifdef DEBUG
	    /* Any symbol reachable from a saved symbol must have already been
	       visited during resolve processing.  In addition, all referenced
	       names must have STR_RESOLVED set. */
	    if (!(r->r_sym->s_flags & S_VISITED)
		|| ((r->r_flags & RLD_RESOLVE_BY_NAME)
		    && !(r->r_sym->s_name->flags & STR_RESOLVED)))
		internal_error();
#endif

	    /* Get referenced symbol, if external. */
	    if (r->r_flags & RLD_RESOLVE_BY_NAME) {
		if (r->r_sym->s_name->flags & STR_NO_DEF)
		    sym = NULL;
		else {
		    sym = r->r_sym->s_name->first_ext_sym;

		    /* If glue code and the real routine both exist, any
		       descriptor must refer to the real code.
		       NOTE:  No check is made to be sure the name on the
		       descriptor and the routine name match. */
		    if (cs->c_symbol.s_smclass == XMC_DS
			&& sym
			&& (sym->s_flags & S_LOCAL_GLUE))
			sym = sym->s_synonym; /* Should never assign NULL. */
		}
		if (sym == NULL) {
		    /* We have an unresolved reference.  Mark the ER symbol so
		       that the ER will be written if a symbol table is
		       generated.  Make sure such a name is only saved once. */
		    if (r->r_flags & RLD_TOCDATA_REF) {
			/* This means r->r_sym->s_smclass == XMC_TD.  The first
			   ER with this name may not be the one saved, so we
			   modify the storage-mapping class of the ER that will
			   be saved. */
			r->r_sym->s_name->refs->s_smclass = XMC_TD;
			r->r_flags &= ~RLD_OVERFLOW_FIXUP_OK;
		    }
		    if (!(r->r_sym->s_name->flags & STR_ER_QUEUED)) {
			/* Queue first ER with name */
			er = r->r_sym->s_name->refs;
			r->r_sym->s_name->flags |= STR_ER_QUEUED;
			DEBUG_MSG(SAVE_DEBUG,
				  (SAY_NO_NLS,
				   "Queueing %s", r->r_sym->s_name->name));
			er->s_flags |= S_SAVE;
			/* save symbol to put in loader section symbol table */
			unresolved_queue[last_unresolved++] = er;
		    }
		}
	    }
	}
    }
} /* process_csect */
/************************************************************************
 * Name: order_glue_csects
 *									*
 * Purpose: Order CSECTs coming from glue code if REORDER_BY_ADDRESS is
 *	used.  This routine is needed because the normal heaps don't
 *	exist for glue code.
 *									*
 * Returns: Nothing
 *									*
 ************************************************************************/
static void
order_glue_csects(OBJECT *obj)
{
    char	*id = "order_glue_csects";
    int		i;
    long	sf_inpndx;
    SRCFILE	*sf;
    CSECT	*cs;
    CSECT_HEAP	*bss_heaps;

    bss_heaps = emalloc(obj->oi_num_sections * sizeof(*bss_heaps), id);
    for (i = 0; i < obj->oi_num_sections; i++)
	if (obj->oi_section_info[i].sect_flags & SECT_CODE)
	    bss_heaps[i] = emalloc((1 + obj->oi_section_info[i].csect_count)
				   * sizeof(*bss_heaps[i]), id);

    sf = obj->oi_srcfiles;
    sf_inpndx = sf->sf_inpndx;

    for (sf = obj->oi_srcfiles; sf; ) {
	if (sf->sf_inpndx == sf_inpndx) {
	    /* Initialize (or reinitialize) heaps. */
	    for (i = 0; i < obj->oi_num_sections; i++) {
		if (obj->oi_section_info[i].sect_flags & SECT_CODE)
		    bss_heaps[i][0].heap_index = 0;
	    }
	}
	for (cs = sf->sf_csect; cs; cs = cs->c_next) {
	    if (obj->oi_section_info[cs->c_secnum-1].sect_flags & SECT_CODE) {
		if (cs->c_mark)
		    bss_heaps[cs->c_secnum-1]
			[++bss_heaps[cs->c_secnum-1][0].heap_index].csect = cs;
	    }
	}

	sf = sf->sf_next;
	if (sf == NULL || sf->sf_inpndx == sf_inpndx) {
	    /* One instance of glue code is complete--queue the csects. */
	    for (i = 0; i < obj->oi_num_sections; i++)
		if (obj->oi_section_info[i].sect_flags & SECT_CODE) {
		    makeheap(bss_heaps[i]);
		    while (cs = deletemin(bss_heaps[i]))
			process_csect(cs, temp_queue);
		}
	}
    }
    for (i = 0; i < obj->oi_num_sections; i++)
	if (obj->oi_section_info[i].sect_flags & SECT_CODE)
	    efree(bss_heaps[i]);

    efree(bss_heaps);
} /* order_glue_csects */
/************************************************************************
 * Name: collect_and_save_csects
 *
 * Purpose: Put the CSECTs in the chosen order and write them to the output
 *		file.  This routine finds all csects to be saved and puts
 *		them in a list.  Then it calls group_csect_by_smclass(), which
 *		sorts the csects by storage-mapping class.  Finally, it calls
 *		save_csects(), which writes the csects to the output file.
 *
 * Returns:	The offset of the next free byte in the output file.
 *		0 If TOC overflow occurred.
 *
 ************************************************************************/
X_OFF_T
collect_and_save_csects(void)
{
    static char	*id = "collect_and_save_csects";
    /* Put switches in const, automatic variable for better optimization. */
    const int	int_switches = make_int_switches();

    char	*arg;
    int		i;
    int		j1;
    int		rc;
    long	put_at_end = 0;
    FILE	*order_file;
    CSECT	*cs;
    CSECT_HEAP	*bss_heaps;
    SYMBOL	*sym;
    HASH_STR	*sroot, *sh;
    OBJECT	*obj;
    SRCFILE	*sf;
    STR		*s;

#ifdef DEBUG
    {
	/* Read debug variable from environment */
	char *s = getenv("ARCH_LIMIT");
	if (s)
	    ARCH_LIMIT = strtoul(s, NULL, 0);
    }
#endif

    /* Set up mapping arrays mappings[], text_sections[], and data_sections[]
       for use by group_csect_by_smclass().  These arrays are initialized for
       the default case, and only have to be updated for special cases.
       Commented-out statements are part of the original initialization.

       There is no bss_sections[] array.  The set of bss sections is fixed. */

    switch(Switches.mapping) {
      case MAPPING_NORMAL:
	break;

      case MAPPING_TEXTINDATA:
	/* Map all code sections to the .data section */
	for (i = SC_CODE_LO + 1; i < SC_CODE_HI; i++)
	    mappings[i].section = MS_DATA;

	/* Reinitialize list of text and data csects. */
	/* text_sections[0] = SC_CODE_LO; */
	text_sections[1] = SC_CODE_HI;
	text_sections[2] = -2;

	/*data_sections[0] = SC_DATA_LO;*/
	data_sections[1] = SC_PR;
	data_sections[2] = SC_RO;
	data_sections[3] = SC_TB;
	data_sections[4] = SC_TI;
	data_sections[5] = SC_DB;
	data_sections[6] = SC_RW;
	data_sections[7] = SC_DS;
	data_sections[8] = SC_TCOVRFL;
	data_sections[9] = SC_TC0;
	data_sections[10] = SC_TC;
	data_sections[11] = SC_TC_EXT;
	data_sections[12] = SC_UA;
	data_sections[13] = SC_DFIXUP;
	data_sections[14] = SC_DATA_HI;
	data_sections[15] = -2;
	break;

      case MAPPING_DATAINTEXT:
	/* Reset major_mappings for main data sections.  The TOC sections
	   are always in the .data section. */
	mappings[SC_RW].section = MS_TEXT;
	mappings[SC_DS].section = MS_TEXT;
	mappings[SC_UA].section = MS_TEXT;

	/* Reinitialize list of text and data csects. */
	/* These are the defaults:
	text_sections[0] = SC_CODE_LO;
	text_sections[1] = SC_PR;
	text_sections[2] = SC_RO;
	text_sections[3] = SC_TB;
	text_sections[4] = SC_TI;
	text_sections[5] = SC_DB;
	*/
	text_sections[6] = SC_RW;
	text_sections[7] = SC_DS;
	text_sections[8] = SC_UA;
	text_sections[9] = SC_TFIXUP;
	text_sections[10] = SC_CODE_HI;
	text_sections[11] = -2;

	/* data_sections[0] = SC_DATA_LO; */
	pre_TOC_index = SC_DATA_LO;
	data_sections[1] = SC_TCOVRFL;
	data_sections[2] = SC_TC0;
	data_sections[3] = SC_TC;
	data_sections[4] = SC_TC_EXT;
	data_sections[5] = SC_DATA_HI;
	data_sections[6] = -2;
	break;
    }

    if (int_switches & SWITCHES_FOLD) {
	if (Switches.mapping == MAPPING_TEXTINDATA)
	    mappings[SC_PR].subsect = SC_RW;
	/* All "text" sections get mapped as SC_PR, except that SC_TFIXUP is
	   always at the end. */
	mappings[SC_RO].subsect = mappings[SC_PR].subsect;
	mappings[SC_TB].subsect = mappings[SC_PR].subsect;
	mappings[SC_TI].subsect = mappings[SC_PR].subsect;
	mappings[SC_DB].subsect = mappings[SC_PR].subsect;

	if (Switches.mapping == MAPPING_DATAINTEXT)
	    mappings[SC_RW].subsect = SC_PR;
	/* All "data" sections get mapped as SC_RW, except that SC_DFIXUP is
	   always at the end. */
	mappings[SC_DS].subsect = mappings[SC_RW].subsect;
	mappings[SC_UA].subsect = mappings[SC_RW].subsect;

	/* All .bss sections get mapped as SC_BS */
	mappings[SC_CM_RW].subsect = SC_BS;
	mappings[SC_UC].subsect = SC_BS;
    }

    temp_queue = emalloc(total_csects_allocated() * sizeof(*temp_queue), id);
    unresolved_queue
	= emalloc(total_ers_allocated() * sizeof(*unresolved_queue), id);

    check_special_names();

    last = 0;				/* Initialize count of csects. */

    if (Bind_state.order_file) {
	/* WARNING:  This is not production-level code.  There is no error
	   checking here. */
	STR	*order_name;
	CSECT	*order_cs;
	SYMBOL	*order_sym;
	char	*nl;
	order_file = fopen(Bind_state.order_file, "r");

	while (1) {
	    char line_buf[1024];
	    if (fgets(line_buf, 1024-1, order_file) == NULL)
		break;
	    if (NULL == (nl = strchr(line_buf, '\n')))
		continue;		/* Symbol name too long */
	    *nl = '\0';			/* Strip newline. */
	    if (line_buf[0] == '=') {
		if (strcmp(line_buf, "= =") == 0) {
		    put_at_end = ftell(order_file);
		    continue;
		}
	    }
	    order_name = lookup_stringhash(line_buf);
	    if (order_name == NULL || (order_name->flags & STR_NO_DEF))
		continue;
	    order_sym = order_name->first_ext_sym;
	    switch(order_sym->s_smtype) {
	      case XTY_SD:
	      case XTY_CM:
	      case XTY_LD:
		order_cs = order_sym->s_csect;
		break;
	      default:
		continue;
	    }
	    if (!order_cs->c_mark || order_cs->c_save)
		continue;
	    if (put_at_end) {
		order_cs->c_save = 1;
		order_cs->c_major_sect = 255;
	    }
	    else
		process_csect(order_cs, temp_queue);
	}
    }

    switch(Switches.reorder) {
      case REORDER_BY_ADDRESS:
	/************************************************
	 * CSECTs are sorted by address order. The algorithm is:
	 * For each object file o (in command line order)
	 * 	For each section s in o (in order)
	 *		For each csect c in s in address order
	 *			If c is marked, queue c in temp_queue[].
	 *
	 * The address order is determined for .text and .data csects from the
	 * heaps saved when the files were read.  For .bss sections, the heaps
	 * must be created.  Csects from glue files have to be handled
	 * as do generated csects.
	 ************************************************/
	for (obj = first_object(); obj; obj = obj->o_next) {
	    switch(obj->o_type) {
	      case O_T_OBJECT:
		if (obj->oi_flags & OBJECT_NOT_READ)
		    continue;
		if (obj->oi_flags & OBJECT_GLUE) {
		    order_glue_csects(obj);
		    continue;
		}

		/* We didn't create bss heaps before, because heaps were
		   only required for looking up RLD items, and BSS csects
		   don't have RLD items.  Therefore, we must create heaps now
		   to sort the BSS csects. */
		bss_heaps
		    = emalloc(obj->oi_num_sections * sizeof(*bss_heaps), id);
		for (i = 0; i < obj->oi_num_sections; i++) {
		    if (obj->oi_section_info[i].sect_type == STYP_BSS) {
			bss_heaps[i]
			    = emalloc((1 + obj->oi_section_info[i].csect_count)
				      * sizeof(*bss_heaps[i]), id);
			bss_heaps[i][0].heap_index = 0;
		    }
		}

		/* Put each saved BSS section in the heap. */
		for (sf = obj->oi_srcfiles; sf; sf = sf->sf_next)
		    for (cs = sf->sf_csect; cs; cs = cs->c_next) {
			if (cs->c_secnum == N_GENERATED) {
			    /* All generated symbols will end up being mapped
			       at the beginning of the symbols for an object
			       file. These CSECTS are not in the structure
			       used by deletemin() calls. */
			    if (cs->c_mark)
				process_csect(cs, temp_queue);
			}
			else if (obj->oi_section_info[cs->c_secnum-1].sect_type
			    == STYP_BSS) {
			    if (cs->c_mark)
				bss_heaps[cs->c_secnum-1]
				    [++bss_heaps[cs->c_secnum-1][0].heap_index
				     ].csect = cs;
			}
		    }

		for (i = 0; i < obj->oi_num_sections; i++) {
		    /* The newly-established BSS heaps, and any other sections
		       without RLDs must made into heaps */
		    if (obj->oi_section_info[i].sect_type == STYP_BSS) {
			makeheap(bss_heaps[i]);
			while (cs = deletemin(bss_heaps[i]))
			    process_csect(cs, temp_queue);
			efree(bss_heaps[i]);
		    }
		    else if (obj->oi_section_info[i].l_reloc_count != 0) {
			/* Must be a .text or .data section */
			j1 = obj->oi_section_info[i].l_csect_heap[0].heap_index;
			/* We may not have accessed all csects, if some of
			   them didn't have RLDs.  So we make sure all csects
			   are sorted now. */
			while (j1-- > 0)
			    (void)
				deletemin(obj->oi_section_info[i].l_csect_heap);

			for (j1 = obj->oi_section_info[i].csect_count;
			     j1 > 0;
			     j1--) {
			    cs = obj->oi_section_info[i].l_csect_heap[j1].csect;
			    if (cs->c_mark)
				process_csect(cs, temp_queue);
			}
		    }
		    else if (obj->oi_section_info[i].sect_flags & SECT_CODE) {
			/* Section had no RLDs, so we make its heap now. */
			if (obj->oi_section_info[i].csect_count) {
			    makeheap(obj->oi_section_info[i].l_csect_heap);
			    while (cs =
				deletemin(obj->oi_section_info[i].l_csect_heap))
				if (cs->c_mark)
				    process_csect(cs, temp_queue);
			}
		    }
		}
		efree(bss_heaps);
		break;

	      case O_T_SHARED_OBJECT:
		/* Only 1 srcfile and CSECT in shared object */
		if (obj->o_srcfiles->sf_csect->c_mark)
		    process_csect(obj->o_srcfiles->sf_csect, temp_queue);
		break;

	      case O_T_SCRIPT:		/* Import file */
		for (sf = obj->o_srcfiles; sf; sf = sf->sf_next)
		    for (cs = sf->sf_csect; cs; cs = cs->c_next)
			if (cs->c_mark)
			    process_csect(cs, temp_queue);
		break;
	    }
	}
	break;

      case REORDER_DFS:
	/************************************************
	 * CSECT Reordering by DFS -- Default Case
	 ************************************************/
	/* Look at entrypoint.  In an ordinary program, this call queues
	   all marked csects. */
	if (Bind_state.entrypoint_sym)
	    dfs2(Bind_state.entrypoint_sym, temp_queue);

	/* Now make sure we keep all marked symbols.  Specifically, exported
	   and kept symbols are found in this loop.  */
	for (sroot = &HASH_STR_root; sroot; sroot = sroot->next) {
	    for (i = 0, sh = sroot+1; i < sroot->s.len; sh++, i++) {
		/* Name must have been resolved in order for any symbol with
		   this name to have been marked, and a definition for the
		   symbol must exist.  In addition, the resolved symbol
		   must have been marked. */
		s = &sh->s;

		/* Check plain name */
		if ((s->flags & (STR_RESOLVED | STR_NO_DEF)) == STR_RESOLVED
		    && (s->first_ext_sym->s_flags & (S_MARK|S_SAVE)) == S_MARK)
		    dfs2(s->first_ext_sym, temp_queue);

		/* Check dot name */
		if ((s = s->alternate) != NULL
		    && (s->flags & (STR_RESOLVED | STR_NO_DEF)) == STR_RESOLVED
		    && (s->first_ext_sym->s_flags & (S_MARK|S_SAVE)) == S_MARK)
		    dfs2(s->first_ext_sym, temp_queue);
	    }
	}
	/* Make sure the TOC anchor is kept. It may have been implicitly
	   referenced. */
	if (Bind_state.o_toc_sym)
	    dfs2(Bind_state.o_toc_sym, temp_queue);

	break;
    }

    if (put_at_end) {
	STR	*order_name;
	CSECT	*order_cs;
	SYMBOL	*order_sym;
	char	*nl;

	fseek(order_file, put_at_end, SEEK_SET);

	while (1) {
	    char line_buf[1024];
	    if (fgets(line_buf, 1024-1, order_file) == NULL)
		break;
	    if (NULL == (nl = strchr(line_buf, '\n')))
		continue;		/* Symbol name too long */
	    *nl = '\0';			/* Strip newline. */
	    order_name = lookup_stringhash(line_buf);
	    if (order_name == NULL || (order_name->flags & STR_NO_DEF))
		continue;
	    order_sym = order_name->first_ext_sym;
	    switch(order_sym->s_smtype) {
	      case XTY_SD:
	      case XTY_CM:
	      case XTY_LD:
		order_cs = order_sym->s_csect;
		break;
	      default:
		continue;
	    }
	    if (order_cs->c_major_sect == 255) {
		order_cs->c_save = 0;
		process_csect(order_cs, temp_queue);
	    }
	}
    }

    if (last == 0)
	if (int_switches & SWITCHES_EXECUTE)
	    bind_err(SAY_NORMAL, RC_ERROR, NLSMSG(SAVE_NULL,
  "%1$s: 0711-244 ERROR: No csects or exported symbols have been saved."),
		     Main_command_name);
	else
	    bind_err(SAY_NORMAL, RC_WARNING, NLSMSG(SAVE_NULL2,
    "%1$s: 0711-245 WARNING: No csects or exported symbols have been saved."),
		     Main_command_name);

    if (group_csect_by_smclass(temp_queue, last, int_switches) != RC_OK)
	return 0;			/* TOC overflow */

    return save_csects(int_switches);
} /* collect_and_save_csects */
/************************************************************************
 * Name: group_csect_by_smclass
 *									*
 * Purpose: Sort referenced CSECTS by storage-mapping class, preserving
 *	as a secondary sorting order the order generated by
 *	collect_and_save_csects().
 *
 * Algorithm:  Use a bucket sort, based on the types and storage-mapping
 *	classes of the csects.  Rather than use a separate array as the queue
 *	for each bucket, the queues are implemented as linked lists stored
 *	in the single array tqueue[].  The head and tail of each list (as
 *	an index into tqueue[]) are kept in the sect_info[] array.
 *
 *	Step 1 creates the lists.  At the same time, the maximum alignment
 *	for any csect in the section is computed and a preliminary calculation
 *	of the size of the TOC is made (to check for TOC overflow).
 *
 *	Step 2 takes the linked lists and places the CSECTs, in order,
 *	into the array queue2[] (which is later copied to the global
 *	variable Queue[], to be used by later phases).  The heads and tails
 *	fields in sect_info[] are redefined: heads is now the index into
 *	queue2[] of the first csect of the appropriate section (which are
 *	now contiguous in queue2[]), or -1 if no csects of the section
 *	exist; tails is one more than the index of the last csect of the
 *	section, or if no csect, the last csect in the previous non-empty
 *	section.
 *
 *	Step 2 is complicated by the fact that the TOC may have to be split
 *	into 2 sections if negative offsets are allowed from the TOC anchor
 *	and if the maximum positive offset has been exceeded.
 *
 * Returns:	RC_SEVERE if TOC overflow occurred.
 *		RC_OK otherwise.
 *									*
 ************************************************************************/
static RETCODE
group_csect_by_smclass(CSECT **queue,
		       int q_size,
		       const int int_switches)
{
    char	*id = "group_csect_by_smclass";

    int		i;
    int		next_tq_index, prev_tq_index;
    int		tq_index, SC_TCOVRFL_heads, saved_SC_TC_tail;
    int		s_index;
    int		text_fixup_CSECT_size;	/* In words */
    int		TOC_fixup_needed = 0;

    long	cur_toc_offset, TC_EXT_offset;
    CSECT	**queue2;
    struct cs_queue {
	CSECT *cs;			/* Pointer to CSECT entry itself. */
	int next;			/* Pointer to next CSECT in
					   per-section list, or -1 if this is
					   the last member of the list. */
    } *tqueue;				/* Storage for maintaining lists of
					   CSECTS.*/

    /* Add three extra elements, one for text fixup, one for data fixup, and
       one in case we must save an unreferenced TOC anchor. */
    tqueue = emalloc((q_size+3) * sizeof(*tqueue), id);
    queue2 = emalloc((q_size+3) * sizeof(*queue2), id);

    /* Step 1:  Create chains of CSECTs for each section, as determined
       by the types and storage-mapping classes of each csect. */

    /* Initialize */
    for (i = 0; i < MAXSECTIONS; i++) {
	sect_info[1].max_align = 0;
	sect_info[i].heads = -1;
    }

    for (i = 0; i < q_size; ++i) {
	CSECT	*cs = queue[i];
	int	s;
	STR	*name;
	SYMBOL	*sym, *er;

	/*******************************************************************
	 * Identify section into which a csect will be mapped.
	 *
	 * Returns: Dictated by "name" or type/stg of containing csect. The
	 *	defaults are shown here, but can be modified by
	 *	setopt mapping' and 'setopt fold' options.
	 *		SC_EXTERN	ER
	 *		SC_CODE_LO	"_text","$CODELO" if SD/PR
	 *		SC_PR		SD/PR-XO-SV-GL
	 *		SC_RO		SD/RO
	 *		SC_TB		SD/TB
	 *		SC_TI		SD/TI
	 *		SC_DB		SD/DB
	 *		SC_TFIXUP	Never returned
	 *		SC_CODE_HI	"_etext","$CODEHI" if SD/DB
	 *		SC_DATA_LO	"_data","$DATALO" if SD/RW
	 *		SC_RW		SD/RW
	 *		SC_DS		SD/DS
	 *		SC_TCOVRFL	SD/TD (Used by data-in-toc csects)
	 *		SC_TC0		SD/TC0
	 *		SC_TC		SD/TC
	 *		SC_TC_EXT	Never returned
	 *		SC_UA		SD/UA
	 *		SC_DFIXUP	Never returned
	 *		SC_DATA_HI	"_edata" if SD/UA
	 *		SC_BS		CM/BS
	 *		SC_US		CM/UC
	 *		SC_CM_RW	CM/RW
	 *		SC_BSS_HI	"_end","end","$DATAHI" if CM/BS
	 *
	 * Side Effects: Sets OBJECT_USED flag for symbols from OBJECT files,
	 *		except for TOC symbols.
	 *
	 *********************************************************************/
	switch(cs->c_secnum) {
	  case N_IMPORTS:
	    if (cs->c_TD_ref) {
		/* Some symbol in this import file has an XMC_TD reference.
		   We must find the symbols. */
		for (sym = &cs->c_symbol; sym; sym = sym->s_next_in_csect) {
		    if (sym->s_inpndx == INPNDX_IMPORT_TD) {
			sym->s_smclass = XMC_TD;
			sym->s_inpndx = INPNDX_IMPORT;
			/* Find all object files making XMC_TD references to
			   this symbol. */
			for (er = sym->s_name->refs; er; er = er->er_synonym) {
			    if ((er->er_flags & S_VISITED)
				&& er->er_smclass == XMC_TD)
				bind_err(SAY_NORMAL, RC_WARNING,
					 NLSMSG(LOC_SYM_IMPORTED,
	    "%1$s: 0711-773 WARNING: Object %2$s, imported symbol %3$s\n"
	    "\tSymbol was expected to be local. Extra instructions\n"
	    "\tare being generated to reference the symbol."),
					 Main_command_name,
					 get_object_file_name(er->er_object),
					 sym->s_name->name);
			}
		    }
		}
	    }
	    s = SC_EXTERN;
	    break;
	  case N_UNDEF:
	    s = SC_EXTERN;
	    break;

	  default:
	    name = cs->c_symbol.s_name;

	    if (cs->c_TD_ref) {
		if (cs->c_symbol.s_smclass != XMC_TD) {
		    if (int_switches & SWITCHES_VERBOSE) {
			bind_err(SAY_NORMAL, RC_WARNING,
				 NLSMSG(TOCDATA_REF,
 "%1$s: 0711-774 WARNING: Symbol %2$s\n"
 "\tStorage-mapping class of symbol changed to XMC_TD because at least\n"
 "\tone reference expects the symbol to be in the TOC."),
				 Main_command_name,
				 cs->c_symbol.s_name->name);
		    }
		    cs->c_symbol.s_smclass = XMC_TD;
		}
	    }
	    else {
		if (cs->c_symbol.s_smclass == XMC_TD) {
		    /* Data-in-toc symbol is not referenced as such.
		       Change it to RW */
		    cs->c_symbol.s_smclass = XMC_RW;
		    if (int_switches & SWITCHES_VERBOSE)
			bind_err(SAY_NORMAL, RC_WARNING,
				 NLSMSG(TOCDATA_NOREF,
	"%1$s: 0711-775 WARNING: Symbol %2$s\n"
	"\tStorage-mapping class of symbol changed to XMC_RW because no\n"
	"\treferences expect the symbol to be in the TOC."),
				 Main_command_name,
				 cs->c_symbol.s_name->name);
		}
	    }

	    /* For most storage-mapping classes, the mappings array is used.
	       In a few cases, the storage-mapping class can never be remapped,
	       so a hard-coded return value can be used. */
	    switch (cs->c_symbol.s_smtype) {
	      case XTY_CODE_LO:
		cs->c_symbol.s_smtype = XTY_SD; /* Restore */
		s = SC_CODE_LO;
		break;
	      case XTY_CODE_HI:
		cs->c_symbol.s_smtype = XTY_SD; /* Restore */
		s = SC_CODE_HI;
		break;
	      case XTY_DATA_LO:
		cs->c_symbol.s_smtype = XTY_SD; /* Restore */
		s = SC_DATA_LO;
		break;
	      case XTY_DATA_HI:
		cs->c_symbol.s_smtype = XTY_SD; /* Restore */
		s = SC_DATA_HI;
		break;
	      case XTY_BSS_HI:
		cs->c_symbol.s_smtype = XTY_CM; /* Restore */
		s = SC_BSS_HI;
		break;

	      case XTY_CM:
		switch(cs->c_symbol.s_smclass) {
		  case XMC_BS:	s = SC_BS; break;
		  case XMC_UC:	s = mappings[SC_UC].subsect; break;
		  case XMC_RW:	s = mappings[SC_CM_RW].subsect; break;
		  case XMC_TD:
		    /* Alternate use of SC_TCOVRFL: These data-in-toc symbols
		       are merged with other TOC symbols before any TOC
		       overflow area is set up. Note that these symbols will
		       be mapped into the .data section, so initialization to 0
		       will have to be provided. */
		    s = SC_TCOVRFL;
		    break;
		  default:
		    internal_error();
		}
		break;

	      case XTY_SD:
		switch (cs->c_symbol.s_smclass) {
		  case XMC_PR:
		  case XMC_XO:
		  case XMC_GL:	s = mappings[SC_PR].subsect; break;
		  case XMC_RO:	s = mappings[SC_RO].subsect; break;
		  case XMC_TB:	s = mappings[SC_TB].subsect; break;
		  case XMC_TI:	s = mappings[SC_TI].subsect; break;
		  case XMC_DS:	s = mappings[SC_DS].subsect; break;
		  case XMC_DB:	s = mappings[SC_DB].subsect; break;
		  case XMC_RW:	s = mappings[SC_RW].subsect; break;
		  case XMC_UA:	s = mappings[SC_UA].subsect; break;

		    /* We do not want to set OBJECT_USED for TOC entries or for
		       the TOC anchor, so we can't simple break; */
		  case XMC_TC0:	s = SC_TC0; goto got_s;
		  case XMC_TC:	s = SC_TC;  goto got_s;

		  case XMC_TD:
		    /* Alternate use of SC_TCOVRFL: These data-in-toc symbols
		       are merged with other TOC symbols before any TOC
		       overflow area is set up. */
		    s = SC_TCOVRFL;
		    break;

		  case XMC_SV:
		  default:
		    internal_error();
		}
		break;
	      default:
		internal_error();
	    }
	    cs->c_srcfile->sf_object->oi_flags |= OBJECT_USED;
	}
      got_s:

	cs->c_major_sect = mappings[s].section;

	tqueue[i].cs = cs;
	tqueue[i].next = -1;

	sect_info[s].max_align = max(sect_info[s].max_align, cs->c_align);

	/* Save chain of csects per section */
	if (sect_info[s].heads == -1)
	    sect_info[s].heads = i;
	else
	    tqueue[sect_info[s].tails].next = i;

	sect_info[s].tails = i;
    }

    /* Append the TOC chain to the XMC_TD chain, so that data-in-TOC entries
       appear first. */
    if (sect_info[SC_TCOVRFL].heads != -1) {
	tqueue[sect_info[SC_TCOVRFL].tails].next = sect_info[SC_TC].heads;
	sect_info[SC_TC].heads = sect_info[SC_TCOVRFL].heads;
	sect_info[SC_TCOVRFL].heads = -1;

	sect_info[SC_TC].max_align = max(sect_info[SC_TC].max_align,
					 sect_info[SC_TCOVRFL].max_align);
    }

    /* Now take care of mapping the TOC entries. */
    sect_info[SC_TC0].max_align = max(sect_info[SC_TC0].max_align,
				      sect_info[SC_TC].max_align);

    /* Is the toc size greater than the architectural limit?  It is ok
       for the toc to be larger than the architecture limit if this is an
       ld -r (i.e. non-executable) object.  Many of the toc entries may be
       garbage collected during the final bind.  */
    if (!(int_switches & SWITCHES_EXECUTE)) {
	/* Assign offsets to all SC_TC elements. */
	toc_size = 0;
	for (tq_index = sect_info[SC_TC].heads;
	     tq_index != -1;
	     tq_index = tqueue[tq_index].next) {
	    CSECT *cs = tqueue[tq_index].cs;
	    toc_size = ROUND(toc_size, 1 << cs->c_align);
	    cs->c_new_addr = toc_size;
	    toc_size += cs->c_len;
	}
    }
    else {
	switch(Switches.target) {
	  case POWER_TARGET:
	    break;

	  default:
	    internal_error();		/* Unknown architecture?? */
	}

	/* Assign offsets to SC_TC elements, up to ARCH_LIMIT/2 */
	cur_toc_offset = 0;
	toc_size = 0;
	for (tq_index = sect_info[SC_TC].heads;
	     tq_index != -1;
	     tq_index = tqueue[tq_index].next) {
	    CSECT *cs = tqueue[tq_index].cs;

	    cur_toc_offset = ROUND(toc_size, 1 << cs->c_align);
	    /* Make sure all bytes of the csect are within the 32K (or
	       ARCH_LIMIT/2) limit of positive offsets from the TOC.
	       In addition, don't put a XMC_TD right at the end of the
	       positive region, and don't put a zero-length csect at
	       the 32K boundary. */
	    if (cur_toc_offset + cs->c_len >= ARCH_LIMIT/2
		&& (cur_toc_offset + cs->c_len > ARCH_LIMIT/2
		    || cs->c_symbol.s_smclass == XMC_TD
		    || cs->c_len == 0))
		break;
	    cs->c_new_addr = cur_toc_offset;
	    toc_size = cur_toc_offset + cs->c_len;
	    prev_tq_index = tq_index;
	}

	/* TOC handling:
	   If the toc needs negative offsets, compute them now. */
	if (tq_index != -1) {
	    CSECT *cs;

	    sect_info[SC_TCOVRFL].heads = tq_index;
	    saved_SC_TC_tail = sect_info[SC_TC].tails;
	    sect_info[SC_TC].tails = prev_tq_index;
	    tqueue[prev_tq_index].next = -1;
	    TC_EXT_offset = toc_size;

	    /* Now, see if the remaining TOC entries will all fit on
	       the TC_OVRFL list. We lay out the next set of TOC
	       entries in the negative direction. We also reverse the list,
	       so the first TC entry in the SC_TCOVRFL list will have the
	       most negative offset. */
	    sect_info[SC_TCOVRFL].tails = tq_index;
	    SC_TCOVRFL_heads = -1;

	    neg_offset = 0;
	    for ( ; tq_index != -1; tq_index = next_tq_index) {
		cs = tqueue[tq_index].cs;
		neg_offset -= cs->c_len;

		/* Round down to aligned negative offset */
#define ROUNDDOWN(X,Y) ((X)&~((Y)-1))
		neg_offset = ROUNDDOWN(neg_offset, (1 << cs->c_align));
		if (neg_offset < -ARCH_LIMIT/2) {
		    neg_offset = tqueue[SC_TCOVRFL_heads].cs->c_new_addr;
		    break;
		}
		cs->c_new_addr = neg_offset;
		next_tq_index = tqueue[tq_index].next;
		tqueue[tq_index].next = SC_TCOVRFL_heads;
		SC_TCOVRFL_heads = tq_index;
	    }
	    sect_info[SC_TCOVRFL].heads = SC_TCOVRFL_heads;

	    /* If tq_index != -1, we probably have TOC overflow.  Nevertheless,
	       it is possible that the remaining csects will now fit in the
	       positive half of the TOC.  Try this now. */
	    for ( ; tq_index != -1; tq_index = tqueue[tq_index].next) {
		cs = tqueue[tq_index].cs;
		cur_toc_offset = ROUND(TC_EXT_offset, 1 << cs->c_align);
		if (cur_toc_offset + cs->c_len >= ARCH_LIMIT/2
		    && (cur_toc_offset + cs->c_len > ARCH_LIMIT/2
			|| cs->c_symbol.s_smclass == XMC_TD
			|| cs->c_len == 0))
		    break;

		DEBUG_MSG(SAVE_DEBUG,
			  (SAY_NO_NLS,
			   "Additional TOC fits in positive half."));
		cs->c_new_addr = cur_toc_offset;
		TC_EXT_offset = cur_toc_offset + cs->c_len;
		tqueue[prev_tq_index].next = tq_index;
		prev_tq_index = tq_index;
		sect_info[SC_TC].tails = tq_index;
	    }

	    if (tq_index == -1) {
		toc_size = -tqueue[SC_TCOVRFL_heads].cs->c_new_addr
		    + TC_EXT_offset;
	    }
	    else {
		/* TOC Overflow */
		/* Everything else goes in TOC extension--
		   fixup code will be needed. */
		sect_info[SC_TC_EXT].heads = tq_index;
		/* Assign additional TOC offsets.  These will all be
		   larger than ARCH_LIMIT/2. */
		for ( ; tq_index != -1; tq_index = tqueue[tq_index].next) {
		    CSECT *cs = tqueue[tq_index].cs;
		    TC_EXT_offset = ROUND(TC_EXT_offset, 1 << cs->c_align);
		    cs->c_new_addr = TC_EXT_offset;
		    TC_EXT_offset += cs->c_len;
		}

		/* Compute TOC size. */
		toc_size = ARCH_LIMIT/2 + TC_EXT_offset;

		if (!(int_switches & SWITCHES_BIGTOC)) {
		    /* Total TOC size greater than architectural limit */
		    bind_err(SAY_NORMAL, RC_SEVERE, NLSMSG(SAVE_TOCOVRF,
  "%1$s: 0711-781 ERROR: TOC overflow. TOC size: %2$d\tMaximum size: %3$d"),
			     Main_command_name, toc_size, ARCH_LIMIT);
		    return RC_SEVERE;
		}
		else {
		    bind_err(SAY_NORMAL, RC_WARNING,
			     NLSMSG(SAVE_TOCOVRF2,
 "%1$s: 0711-783 WARNING: TOC overflow. TOC size: %2$d\tMaximum size: %3$d\n"
 "\tExtra instructions are being generated for each reference to a TOC\n"
 "\tsymbol if the symbol is in the TOC overflow area."),
			     Main_command_name, toc_size, ARCH_LIMIT);
		    TOC_fixup_needed = 1;
		}
	    }
	}
    }

    /* Step 2:  Take chains and place them in the array queue2[] */
    last = 0;
    text_fixup_CSECT_size = 0;
    /* Handle EXTERNs and the .text section */
    for (i = SC_EXTERN, s_index = 0; i != -2; i = text_sections[s_index++]) {
	int tq_indx = sect_info[i].heads;
	if (tq_indx != -1) {
	    sect_info[i].heads = last;
	    for ( ; tq_indx != -1; tq_indx = tqueue[tq_indx].next) {
		queue2[last++] = tqueue[tq_indx].cs;
		if (TOC_fixup_needed) {
		    /* This can only be executed if Switches.execute is set. */
		    RLD *rld;
		    SYMBOL *sym;
		    for (rld = tqueue[tq_indx].cs->c_first_rld;
			 rld;
			 rld = rld->r_next) {
			if (rld->r_flags & RLD_OVERFLOW_FIXUP_OK) {
			    if (rld->r_flags & RLD_RESOLVE_BY_NAME) {
				sym = rld->r_sym->s_name->first_ext_sym;
				/* Symbol must be defined because otherwise,
				   OVERFLOW_FIXUP_OK wouldn't be set. */
				if (sym->s_smclass != XMC_TC
				    && !sym->s_csect->c_TD_ref)
				    rld->r_flags &= ~RLD_OVERFLOW_FIXUP_OK;
				else if ((int32) sym->s_csect->c_new_addr
					 >= ARCH_LIMIT/2) {
				    rld->r_flags |= RLD_FIXUP_OK;
				    text_fixup_CSECT_size
					+= MAX_TOCOVERFLOW_FIXUP_LENGTH;
				}
			    }
			    else {
				if ((sym = rld->r_sym->s_resolved)
				    && (int32) sym->s_csect->c_new_addr
				    >= ARCH_LIMIT/2) {
				    rld->r_flags |= RLD_FIXUP_OK;
				    if (rld->r_flags & RLD_TOCDATA_FIXUP) {
					/* Both kinds of fixup are needed.  We
					   added MAX_TOCDATA_FIXUP_LENGTH
					   words for the data-in-TOC instance.
					   We subtract this value and add
					   MAX_OVERALL_FIXUP_LENGTH, the maximum
					   number of instructions needed for
					   both kinds of fixup. */
					text_fixup_CSECT_size
					    += (MAX_OVERALL_FIXUP_LENGTH
						- MAX_TOCDATA_FIXUP_LENGTH);
				    }
				    else
					text_fixup_CSECT_size
					    += MAX_TOCOVERFLOW_FIXUP_LENGTH;
				}
			    }
			}
		    }
		}
	    }
	}
	sect_info[i].tails = last;
    }

    if (int_switches & SWITCHES_EXECUTE)
	text_fixup_CSECT_size
	    += Bind_state.num_data_in_toc_fixups * MAX_TOCDATA_FIXUP_LENGTH;

    if (text_fixup_CSECT_size > 0) {
	SYMBOL *sym;
	int x;

	/* We have already copied SC_TEXTHI csects.  We need to shift them
	   to make room for the fixup csect. */
	for (x = last; x > sect_info[SC_TFIXUP].tails; x--)
	    queue2[x] = queue2[x-1];
	++last;

	/* Add the text-fixup csect. */
	text_fixup_CSECT = get_CSECTs(1);
	text_fixup_CSECT->c_TD_ref = 0;
	text_fixup_CSECT->c_mark
	    = text_fixup_CSECT->c_save = 1;
	text_fixup_CSECT->c_secnum = N_FIXUP;
	text_fixup_CSECT->c_align = 2;
	text_fixup_CSECT->c_major_sect = MS_TEXT;
	text_fixup_CSECT->c_first_rld = NULL;
	text_fixup_CSECT->c_addr
	    = text_fixup_CSECT->c_new_addr = 0;
	text_fixup_CSECT->c_srcfile = queue2[sect_info[SC_PR].heads]->c_srcfile;
	text_fixup_CSECT->c_next = NULL;
	sym = &text_fixup_CSECT->c_symbol;
	sym->s_name = &NULL_STR;
	sym->s_next_in_csect = NULL;
	sym->s_synonym = NULL;
	sym->s_addr = 0;
	sym->s_inpndx = INPNDX_FIXUP;
	sym->s_number = -1;
	sym->s_smclass = XMC_PR;
	sym->s_smtype = XTY_SD;
	sym->s_flags = S_MARK | S_SAVE | S_PRIMARY_LABEL | S_HIDEXT;
	sym->s_csect = text_fixup_CSECT;
	queue2[x] = text_fixup_CSECT;
	sect_info[SC_TFIXUP].heads = sect_info[SC_TFIXUP].tails++;
	++sect_info[SC_CODE_HI].tails;

	/* Leave room for traceback table (3 words)
	   and initial dummy branch (1 word). */
	text_fixup_CSECT->c_len
	    = 4 * (text_fixup_CSECT_size + FIXUP_OVERHEAD_WORDS);

	if (int_switches & SWITCHES_VERBOSE)
	    say(SAY_NORMAL,
		NLSMSG(FIXUP_CSECT_SIZE, "%1$s: Fixup csect size: %2$d"),
		Command_name, text_fixup_CSECT->c_len);
    }

    /* Handle the .data section */
    for (i = data_sections[s_index=0]; i != -2; i = data_sections[++s_index]) {
	int tq_indx = sect_info[i].heads;
	if (tq_indx != -1) {
	    sect_info[i].heads = last;
	    for ( ; tq_indx != -1; tq_indx = tqueue[tq_indx].next) {
		queue2[last++] = tqueue[tq_indx].cs;
		/* If we want to support fixup code in the data section,
		   here's where we need to check RLDs */
	    }
	}
	sect_info[i].tails = last;
    }

    /* Handle BSS sections */
    for (i = SC_BSS_LO; i < MAXSECTIONS; ++i) {
	int tq_indx = sect_info[i].heads;
	if (tq_indx != -1) {
	    sect_info[i].heads = last;
	    for ( ; tq_indx != -1; tq_indx = tqueue[tq_indx].next)
		queue2[last++] = tqueue[tq_indx].cs;
	}
	sect_info[i].tails = last;
    }

    first_text_index = sect_info[SC_CODE_LO-1].tails;
    first_data_index = sect_info[SC_DATA_LO-1].tails;
    pre_TOC_index = sect_info[pre_TOC_index].tails;
    last_data_index = sect_info[SC_DATA_HI].tails - 1;

    efree(tqueue);
    efree(queue);
    Queue = queue2;
    Queue_size = last;			/* last could be greater than q_size
					   if fixup csects were added. */
    return RC_OK;
} /* group_csect_by_smclass */
/************************************************************************
 * Name: check_special_names
 *									*
 * Purpose: Look up special names and modify the symbol types of csects
 *	containing special symbols.  In addition, prevent these symbols from
 *	being moved into the TOC.
 *
 * Returns: Nothing
 *									*
 ************************************************************************/
static void
check_special_names(void)
{
    int i, j;
    STR	*name;
    SYMBOL *sym;
    CSECT *cs;

    /* Set up structure containing reserved names that have to be mapped
       specially. */
    static char *text_lo_names[] = {"_text", "$CODELO"};
    static char *text_hi_names[] = {"_etext", "$CODEHI"};
    static char *data_lo_names[] = {"_data", "$DATALO"};
    static char *data_hi_names[] = {"_edata"};
    static char *bss_hi_names[] = {"_end", "end", "DATAHI"};
    static struct special_names_info {
	char **names;			/* Names to look fo */
	int num_names;
	uint8 smtype;			/* smtype of matching symbol */
	uint8 smclass;			/* smclass of matching symbol */
	uint8 new_smtype;		/* New, temporary smtype for
					   matching symbol. */
    } special_names[] = {
#define shorthand(y) y,sizeof(y)/sizeof(y[0])
	/* NOTE:  If the smtype fields of these entries are changed, the code
	   that retores the s_smtype field will have to be changed as well. */
	{shorthand(text_lo_names), XTY_SD, XMC_PR, XTY_CODE_LO},
	{shorthand(text_hi_names), XTY_SD, XMC_DB, XTY_CODE_HI},
	{shorthand(data_lo_names), XTY_SD, XMC_RW, XTY_DATA_LO},
	{shorthand(data_hi_names), XTY_SD, XMC_UA, XTY_DATA_HI},
	{shorthand(bss_hi_names),  XTY_CM, XMC_RW, XTY_BSS_HI},
#undef shorthand
    };

    ptrgl_name  = lookup_stringhash("._ptrgl");
    PTRGL_name  = lookup_stringhash(".$PTRGL");

    for (i = 0; i < sizeof(special_names)/sizeof(special_names[0]); i++) {
	for (j = 0; j < special_names[i].num_names; j++) {
	    if ((name = lookup_stringhash(special_names[i].names[j]))
		&& !(name->flags & STR_NO_DEF)) {
		sym = name->first_ext_sym;
		cs = sym->s_csect;
		if (cs->c_mark
		    && cs->c_symbol.s_smtype == special_names[i].smtype
		    && cs->c_symbol.s_smclass == special_names[i].smclass) {
		    if (cs->c_TD_ref) {
			bind_err(SAY_NORMAL, RC_SEVERE,
				 NLSMSG(TD_RESERVED,
	"%1$s: 0711-777 SEVERE ERROR Symbol %2$s\n"
	"\tThe symbol has a reserved name and cannot be mapped into the TOC."),
				 Main_command_name, name->name);
			cs->c_TD_ref = 0; /* Disable movement into TOC. */
		    }
		    else
			cs->c_symbol.s_smtype = special_names[i].new_smtype;
		}
	    }
	}
    }
} /* check_special_names */
/************************************************************************
 * Name: dbopt1_print
 *									*
 * Purpose: Print out csect address (if dbgopt1 is set)
 *
 * Returns: Nothing
 *									*
 ************************************************************************/
static void
dbopt1_print(int i,
	     CSECT *cs,
	     int d2)			/* non-zero if dbgopt2 is set */
{
    SYMBOL *sym = &cs->c_symbol;

    say(SAY_NO_NL | SAY_NO_NLS, "%d", i);
    say(SAY_NO_NL,
	NLSMSG(DBOPT1_ADDRESSES, "\tOld address: %1$.8x New: %2$.8x "),
	cs->c_addr, cs->c_new_addr);
    say(SAY_NO_NL | SAY_NO_NLS, "[%d]", cs->c_symbol.s_number);

    if (cs->c_symbol.s_flags & S_HIDEXT) {
	/* For an unnamed hidden csect, find first non-null label name
	   in csect, if any. */
	while (sym && sym->s_name == &NULL_STR)
	    sym = sym->s_next_in_csect;

	if (sym && sym != &cs->c_symbol)
	    say(SAY_NO_NL, NLSMSG(DBOPT1_WITH_LABEL, "with label "));
    }

    if (sym == NULL)
	say(SAY_NO_NLS, "<>");
    else {
	if (sym->s_flags & S_HIDEXT)
	    say(SAY_NO_NLS, "<%s>", sym->s_name->name);
	else
	    say(SAY_NO_NLS, "%s", sym->s_name->name);
    }

    if (d2) {
	/* Don't print label of csect itself. */
	for (sym = cs->c_symbol.s_next_in_csect;
	     sym;
	     sym = sym->s_next_in_csect) {
	    say(SAY_NO_NL,
		NLSMSG(DBOPT1_ADDRESSES, "\tOld address: %1$.8x New: %2$.8x "),
		sym->s_addr, sym->s_addr + (cs->c_new_addr - cs->c_addr));
	    say(SAY_NO_NLS, "[%d]%s", sym->s_number, sym->s_name->name);
	}
    }
} /* dbopt1_print */
/************************************************************************
 * Name: assign_typchks
 *									*
 * Purpose: Assign offset into the .typchk section for symbols that will
 *	appear in the output symbol table
 *
 * Returns:
 *									*
 ************************************************************************/
static void
assign_typchks(CSECT *cs)
{
    SYMBOL	*sym;
    TYPECHK	*t;

    for (sym = &cs->c_symbol; sym; sym = sym->s_next_in_csect) {
	if (!(sym->s_flags & S_HIDEXT) && (t = sym->s_typechk)) {
	    if (t->t_value == -1 || typchk_values[t->t_value].sect_val == 0) {
		if (t->t_value == -1) {
		    t->t_value = num_used_typchks;
		    typchk_values[num_used_typchks].ldr_val = 0;
		    typchk_values[num_used_typchks++].t = t;
		}

		typchk_values[t->t_value].sect_val = typchk_sect_offset + 2;
		typchk_sect_offset += 2 + t->t_len;
	    }
	}
    }
}
/************************************************************************
 * Name: save_csects
 *									*
 * Purpose:  Write the CSECTs to be saved to the output file, relocating
 *	references as we go.
 *
 * Returns: Status code
 *									*
 ************************************************************************/
static X_OFF_T
save_csects(const int int_switches)
{
    int		i, j,
		scn_idx;
    uint	align_factor;
    int		csect_seen = 0;
    int		text_needed, data_needed, bss_needed;
    int		data_follows_text;
    int		s_index;
    int		limit, limits[2];
    uint32	new_address;
    long	text_size, data_size, bss_size;
    long	text_address, data_address; /* Running addresses */
    long	text_base_address, data_base_address, bss_base_address;
    long	offset;
    long	max_align = 0;
    unsigned long pad, pad1, pad2, fudge_factor;
    uint32	raw_target_org, raw_target_orgs[2];
    IFILE	*ifile;
    OBJECT	*obj;

    init_typchk_info();			/* Initialize even if "strip" option
					   selected, because the loader section
					   may need the info. */

    if (int_switches & SWITCHES_DBG_OPT1) {
	say(SAY_NORMAL, NLSMSG(DBOPT1_ORDERING, "CSECT ordering:"));

	/* Symbols in the Queue array are in section order,
	   starting with imported symbols */
	for (i = 0; i < first_text_index; i++) {
	    say(SAY_NO_NLS | SAY_NO_NL, "%d\t", i);
	    if (*Queue[i]->c_srcfile->sf_name->name)
		say(SAY_NORMAL,
		    NLSMSG(DBOPT1_IMPORTS,
			   "Imported symbols in %1$s (from file %2$s)"),
		    Queue[i]->c_srcfile->sf_name->name,
		    get_object_file_name(Queue[i]->c_srcfile->sf_object));
	    else
		say(SAY_NORMAL, NLSMSG(DBOPT1_DEFERRED_IMPORTS,
				       "Deferred Imports (from file %s)"),
		    get_object_file_name(Queue[i]->c_srcfile->sf_object));
	}
    }

    Bind_state.out_writing = 1;	/* So signal catcher knows we're writing */

    /* Now compute the number of section headers we'll need. */
    if (int_switches & SWITCHES_EXECUTE) {
	/* If the file is to be executable, all 3 primary section headers
	   must exist. */
	text_needed = 1;
	data_needed = 1;
	bss_needed = 1;

	Filehdr->f_nscns =
#ifdef GENERATE_PAD_HEADERS
	    7;				/* 3 sections + 3 pads + loader */
#else
	    4;				/* 3 sections + loader */
#endif
    }
    else {
	text_needed = data_needed = bss_needed = 0;
	for (i = text_sections[s_index=0];
	     i != -2;
	     i = text_sections[++s_index]) /* Check for text csects */
	    if (sect_info[i].heads != -1) {
		text_needed = 1;
		break;
	    }
	for (i = data_sections[s_index=0];
	     i != -2;
	     i = data_sections[++s_index]) /* Check for data csects */
	    if (sect_info[i].heads != -1) {
		data_needed = 1;
		break;
	    }
	for (i = SC_BSS_LO; i <= SC_BSS_HI; i++) /* Check for bss csects */
	    if (sect_info[i].heads != -1) {
		bss_needed = 1;
		break;
	    }

	/* Compute upper bound on number of section headers needed */
	Filehdr->f_nscns =
#ifdef GENERATE_PAD_HEADERS
	    2 * text_needed		/* Pad + text */
	    + 2 * data_needed		/* Pad + data */
	    + 2 * (!!(int_switches & SWITCHES_EXECUTE))	/* pad + loader */
#else
	    text_needed			/* text */
	    + data_needed		/* data */
	    + (!!(int_switches & SWITCHES_EXECUTE)) /* loader */
#endif
	    + bss_needed;		/* bss */
    }

    if (!(int_switches & SWITCHES_STRIP)) {
	/* The #if 0 code results in the smallest object files, but will
	   change the size of unstripped object files.
	   This is not allowed in the 4.1.B binder.

	   This code can be used in 4.2. */
#if 0
	Filehdr->f_nscns += text_needed; /* overflow for text section */
	if (data_needed && total_rlds_allocated() >= 65535)
	    ++Filehdr->f_nscns;		/* overflow for data section */
#else
	Filehdr->f_nscns
	    += text_needed + data_needed /* overflow */
		+ 1 + 1 + 1;		/* .except + .typchk + .debug */
#endif
	if (Bind_state.sections_seen
	    & (STYP_INFO | STYP_EXCEPT | STYP_DEBUG | STYP_TYPCHK)) {
	    int obj_sections = 0;
	    /* If any input object had one of these special sections, then
	       see which of these sections were used in any used object. */
	    for (obj = first_object(); obj; obj = obj->o_next) {
		if (obj->o_type == O_T_OBJECT
		    && (obj->oi_flags & OBJECT_USED))
		    obj_sections |= (obj->oi_flags
				     & (OBJECT_HAS_DEBUG
					| OBJECT_HAS_TYPECHK
					| OBJECT_HAS_EXCEPT
					| OBJECT_HAS_INFO));
	    }

	    Filehdr->f_nscns +=
#if 0
		+ (!!(obj_sections & OBJECT_HAS_EXCEPT)) /* .except */
		+ (!!(obj_sections & OBJECT_HAS_TYPECHK)) /* .typchk */
		+ (!!(obj_sections & OBJECT_HAS_DEBUG)) /* .debug */
#endif
		+ (!!(obj_sections & OBJECT_HAS_INFO)); /* .info */
	}
    }

#ifdef GENERATE_PAD_HEADERS
    /* Insert name of pad section in static pad section */
    (void) strcpy(pad_section.s_name, _PAD);
#endif

    scn_idx = 0;			/* index of next available header */

    /* Compute next available offset in output file. */
    offset = FILHSZ + Filehdr->f_opthdr + Filehdr->f_nscns * SCNHSZ;

    /* Compute alignment for .text section. */
    if (text_needed) {
	/* Compute the maximum alignment needed by any csect in the section. */
	max_align = 0;
	for (i = text_sections[s_index=0];
	     i != -2;
	     i = text_sections[++s_index])
	    max_align = max(max_align, sect_info[i].max_align);
	temp_aout_hdr.o_algntext = max_align; /* Save maximum alignment */
    }

    /* Compute alignment for .data and .bss sections. If -D-1 was specified,
       we'll need to use the alignment computed here to align the text section
       too. */
    if (data_needed || bss_needed) {
	/* Compute the maximum alignment needed by any csect in the sections. */
	max_align = 0;
	for (i = data_sections[s_index=0];
	     i != -2;
	     i = data_sections[++s_index])
	    max_align = max(max_align, sect_info[i].max_align);

	for (i = SC_BSS_LO; i <= SC_BSS_HI; i++) /* Check bss csects */
	    max_align = max(max_align, sect_info[i].max_align);
	temp_aout_hdr.o_algndata = max_align; /* Save maximum alignment */
    }

    data_follows_text = (!(Bind_state.flags & FLAGS_DATA_PAGE_ADDR)
			 && temp_aout_hdr.o_data_start == -1);

    /* Compute starting address for .text section. */
    if (text_needed) {
	if (data_follows_text) {
	    /* Compute alignment based on all three sections. */
	    max_align = max(temp_aout_hdr.o_algndata, temp_aout_hdr.o_algntext);
	}
	else
	    max_align = temp_aout_hdr.o_algntext;

	pad1 = 1 << max_align;		/* Compute 2^max_align */

	pad = Bind_state.tpad_align;	/* From PAD command */

	switch(pad) {
	  case 0:			/* Calculate proper alignment */
	    if (int_switches & SWITCHES_EXECUTE) {
		/* We should be sure that the file offset for the text section
		   is aligned as specified by tpad_align, or as needed so that
		   the beginning is aligned as specified for any contained
		   csect. */
		fudge_factor = 0;
		if (Bind_state.flags & FLAGS_TEXT_PAGE_ADDR) {
		    /* Special case for AIX:  If the -bpT option is used, pad
		       to a doubleword boundary.  Otherwise, the first 4 bytes
		       of the text section will be unused (assuming the usual
		       alignment of 2^3 for code).  This works fine, but might
		       be confusing. */
		    if (offset & 0x7)
			fudge_factor = 8 - (offset & 0x7);
		    align_factor = (uint)offset - temp_aout_hdr.o_text_start
			- ((uint)offset & (Pagesize-1));
		}
		else
		    align_factor = (uint)offset - temp_aout_hdr.o_text_start;
		DEBUG_MSG(OUTPUT_DEBUG,
			  (SAY_NO_NLS,
		   "Text pad based on max alignment %d (with fudge factor %d)",
			   max_align, fudge_factor));
		pad2 = pad1 - (align_factor % pad1);
		if (pad2 == pad1)
		    pad2 = 0;
		pad2 += fudge_factor;
	    }
	    else
		pad2 = 0;
	    break;

	  case 1:			/* No padding */
	    pad2 = 0;
	    DEBUG_MSG(OUTPUT_DEBUG,
		      (SAY_NO_NLS,
		       "No text section padding, as specified."));
	    break;

	  default:
	    pad2 = pad - (offset % pad);
	    if (pad2 == pad)
		pad2 = 0;
	    DEBUG_MSG(OUTPUT_DEBUG,
		      (SAY_NO_NLS, "Text pad set to %d as specified", pad));
	}

	/* Add padding if needed. */
	if (pad2 > 0) {
#ifdef GENERATE_PAD_HEADERS
	    /* Initialize .pad section header */
	    Scnhdr[scn_idx] = pad_section; /* Copy dummy pad section header */
	    Scnhdr[scn_idx].s_scnptr = offset;
	    Scnhdr[scn_idx].s_size = pad2;
	    ++scn_idx;
#endif
	    offset += pad2;
	    DEBUG_MSG(OUTPUT_DEBUG,
		      (SAY_NO_NLS,
		       "Padding .text section %d bytes. New offset is %d.",
		       pad2, offset));
	}

	if (Bind_state.flags & FLAGS_TEXT_PAGE_ADDR)
	    temp_aout_hdr.o_text_start += (offset & (Pagesize-1));
	text_base_address = temp_aout_hdr.o_text_start;
	text_address = text_base_address;

	/* Check for bad alignment. */
	if (int_switches & SWITCHES_EXECUTE) {
	    /* For this error message, we only worry about the alignment
	       of .text csects, even if -D-1 was specified. */
	    if (((uint)offset - temp_aout_hdr.o_text_start)
		% (1 << temp_aout_hdr.o_algntext) > 0) {
		bind_err(SAY_NORMAL, RC_WARNING,
			 NLSMSG(BAD_TEXT_ALIGN,
	"%s: 0711-356 WARNING: The .text section may not be aligned properly."),
			 Main_command_name);
	    }
	}

	/* Assign addresses to TEXT csects */
	for (i = first_text_index; i < first_data_index; i++) {
	    CSECT *cs = Queue[i];

	    csect_seen = 1;
	    text_address = ROUND(text_address, 1 << cs->c_align);
	    cs->c_new_addr = text_address;
	    text_address += cs->c_len;

	    if (!(int_switches & SWITCHES_STRIP) /* And any typecheck seen */)
		/* We compute typecheck values now so we can know ahead of time
		   how big the .typchk section should be.  Thus, we will be
		   able to write the debug strings directly to the file. */
		assign_typchks(cs);

	    if (int_switches & SWITCHES_DBG_OPT1)
		dbopt1_print(i, cs, int_switches & SWITCHES_DBG_OPT2);
	} /* for TEXT csects */

	if ((int_switches & SWITCHES_EXECUTE)
	    && text_address == text_base_address)
	    text_address += 4;	/* Loader doesn't like 0-length text section */

	text_size = text_address - text_base_address;
	temp_aout_hdr.o_tsize = text_size;

	/* Initialize .text section header */
	(void) strncpy(Scnhdr[scn_idx].s_name, _TEXT, sizeof(Scnhdr->s_name));
	Scnhdr[scn_idx].s_flags = STYP_TEXT;
	Scnhdr[scn_idx].s_size = text_size;
	if (text_size == 0) {
	    /* This should never happen, because if Switches.execute is 1,
	       text_size is at least 4, and if Switches.execute is 0, we won't
	       be doing this code for text csects. */
	    Scnhdr[scn_idx].s_scnptr = 0;
	}
	else
	    Scnhdr[scn_idx].s_scnptr = offset;
	offset += text_size;

	/* Virtual Address and Physical Address */
	Scnhdr[scn_idx].s_vaddr = Scnhdr[scn_idx].s_paddr = text_base_address;
	raw_text_target_org = Scnhdr[scn_idx].s_scnptr-Scnhdr[scn_idx].s_paddr;
	temp_aout_hdr.o_sntext = ++scn_idx;	/* 1-based */
    }
    else {
	/* Compute text_address in case -D-1 was specified. */
	text_address = temp_aout_hdr.o_text_start;
	if (Bind_state.flags & FLAGS_TEXT_PAGE_ADDR)
	    text_address += (offset & (Pagesize-1));
	text_size = 0;
	temp_aout_hdr.o_text_start = 0;
	/* Fields o_tsize and o_algntext are initialized to 0. */
    }

    /* Compute starting addresses for .data and .bss sections. */
    if (data_needed || bss_needed) {
	pad1 = 1 << temp_aout_hdr.o_algndata; /* Compute 2^algndata */

	pad = Bind_state.dpad_align;	/* From PAD command */

	switch(pad) {
	  case 0:			/* Calculate proper alignment */
	    if (int_switches & SWITCHES_EXECUTE) {
		/* We should be sure that the file offset for the data section
		   is aligned as specified by dpad_align, or as needed so that
		   the beginning is aligned as specified for any contained
		   csect. */
		if (data_follows_text) {
		    pad2 = 0;		/* Text section was aligned strictly
					   enough for data too. */
		    DEBUG_MSG(OUTPUT_DEBUG,
			      (SAY_NO_NLS,
		       "No data pad--text section already aligned for data"));
		}
		else {
		    align_factor = (uint)offset - temp_aout_hdr.o_data_start;
		    if (Bind_state.flags & FLAGS_DATA_PAGE_ADDR)
			align_factor -= (offset & (Pagesize-1));
		    DEBUG_MSG(OUTPUT_DEBUG,
			      (SAY_NO_NLS, "Data pad based on max alignment %d",
			       temp_aout_hdr.o_algndata));

		    pad2 = pad1 - (align_factor % pad1);
		    if (pad2 == pad1)
			pad2 = 0;
		}
	    }
	    else
		pad2 = 0;
	    break;

	  case 1:			/* No padding */
	    pad2 = 0;
	    DEBUG_MSG(OUTPUT_DEBUG,
		      (SAY_NO_NLS,
		       "No data section padding, as specified."));
	    break;

	  default:
	    pad2 = pad - (offset % pad);
	    if (pad2 == pad)
		pad2 = 0;
	    DEBUG_MSG(OUTPUT_DEBUG,
		      (SAY_NO_NLS, "Data pad set to %d as specified", pad));
	}

	/* Add padding if needed. */
	if (pad2 > 0) {
#ifdef GENERATE_PAD_HEADERS
	    /* Initialize .pad section header */
	    Scnhdr[scn_idx] = pad_section; /* Copy dummy pad section header */
	    Scnhdr[scn_idx].s_scnptr = offset;
	    Scnhdr[scn_idx].s_size = pad2;
	    ++scn_idx;
#endif
	    offset += pad2;
	    DEBUG_MSG(OUTPUT_DEBUG,
		      (SAY_NO_NLS,
		       "Padding .data section %d bytes. New offset is %d.",
		       pad2, offset));
	}

	if (Bind_state.flags & FLAGS_DATA_PAGE_ADDR)
	    temp_aout_hdr.o_data_start += (offset & (Pagesize-1));
	else if (temp_aout_hdr.o_data_start == -1) {
	    /* Take into account any padding bytes when assigning addresses
	       to the data section. */
	    temp_aout_hdr.o_data_start = text_address + pad2;
	}
	data_base_address = temp_aout_hdr.o_data_start;
	data_address = data_base_address;

	/* Check for bad alignment. */
	if (int_switches & SWITCHES_EXECUTE)
	    if (((uint)offset - temp_aout_hdr.o_data_start) % pad1 > 0) {
		bind_err(SAY_NORMAL, RC_WARNING,
			 NLSMSG(BAD_DATA_ALIGN,
	"%s: 0711-357 WARNING: The .data section may not be aligned properly."),
			 Main_command_name);
	    }

	i = first_data_index;
	if (data_needed) {
	    /* Assign addresses to DATA csects up to the TOC anchor */
	    for ( ; i < pre_TOC_index; i++) {
		CSECT *cs = Queue[i];

		csect_seen = 1;
		data_address = ROUND(data_address, 1 << cs->c_align);
		cs->c_new_addr = data_address;
		data_address += cs->c_len;

		if (!(int_switches & SWITCHES_STRIP) /* And typecheck seen */)
		    assign_typchks(cs);

		if (int_switches & SWITCHES_DBG_OPT1)
		    dbopt1_print(i, cs, int_switches & SWITCHES_DBG_OPT2);
	    } /* for DATA csects (up to TOC anchor) */

	    /* Compute the address of the TOC anchor, if it exists. */
	    if (sect_info[SC_TC0].heads != -1) {
		CSECT *cs = Queue[sect_info[SC_TC0].heads];

		csect_seen = 1;
		cs->c_new_addr = 0;

		data_address = ROUND(data_address,
				     1 << sect_info[SC_TC0].max_align);
		toc_anchor_address = data_address - neg_offset;
		data_address += toc_size;

#ifdef DEBUG
		if (Bind_state.o_toc_sym != &cs->c_symbol)
		    internal_error();
#endif

		for ( ; i < sect_info[SC_TC_EXT].tails; i++) {
		    CSECT *cs = Queue[i];

		    /* We already assigned relative TOC addresses,
		       so make them absolute.*/
		    cs->c_new_addr += toc_anchor_address;

		    if (int_switches & SWITCHES_DBG_OPT1)
			dbopt1_print(i, cs, int_switches & SWITCHES_DBG_OPT2);
		} /* for TOC csects */
	    }

	    /* Assign addresses for remaining DATA csects */
	    for ( ; i < sect_info[SC_DATA_HI].tails; i++) {
		CSECT *cs = Queue[i];

		csect_seen = 1;
		data_address = ROUND(data_address, 1 << cs->c_align);
		cs->c_new_addr = data_address;
		data_address += cs->c_len;

		if (!(int_switches & SWITCHES_STRIP) /* And typecheck seen */)
		    assign_typchks(cs);

		if (int_switches & SWITCHES_DBG_OPT1)
		    dbopt1_print(i, cs, int_switches & SWITCHES_DBG_OPT2);
	    } /* for remaining DATA csects */

	    data_size = data_address - data_base_address;
	    temp_aout_hdr.o_dsize = data_size;

	    /* Initialize .data section header */
	    (void) strncpy(Scnhdr[scn_idx].s_name, _DATA,
			   sizeof(Scnhdr->s_name));
	    Scnhdr[scn_idx].s_flags = STYP_DATA;
	    Scnhdr[scn_idx].s_size = data_size;
	    if (data_size == 0)
		Scnhdr[scn_idx].s_scnptr = 0;
	    else
		Scnhdr[scn_idx].s_scnptr = offset;
	    offset += data_size;

	    /* Virtual Address and Physical Address */
	    Scnhdr[scn_idx].s_vaddr
		= Scnhdr[scn_idx].s_paddr
		    = data_base_address;
	    raw_data_target_org
		= Scnhdr[scn_idx].s_scnptr - Scnhdr[scn_idx].s_paddr;
	    temp_aout_hdr.o_sndata = ++scn_idx;	/* 1-based */
	}
	else
	    data_size = 0;

	if (bss_needed) {
	    /* Setup for .bss section--
	       This section follows immediately after .data */
	    bss_base_address = data_address;

	    /* Assign addresses to BSS csects */
	    for (; i < Queue_size; i++) {
		CSECT *cs = Queue[i];

		csect_seen = 1;
		data_address = ROUND(data_address, 1 << cs->c_align);
		cs->c_new_addr = data_address;
		data_address += cs->c_len;

		if (!(int_switches & SWITCHES_STRIP) /* And typecheck seen */)
		    assign_typchks(cs);

		if (int_switches & SWITCHES_DBG_OPT1)
		    dbopt1_print(i, cs, int_switches & SWITCHES_DBG_OPT2);
	    } /* for BSS csects */

	    bss_size = data_address - bss_base_address;
	    temp_aout_hdr.o_bsize = bss_size;

	    /* Initialize .bss section header */
	    (void) strncpy(Scnhdr[scn_idx].s_name, _BSS,
			   sizeof(Scnhdr->s_name));
	    Scnhdr[scn_idx].s_flags = STYP_BSS;
	    Scnhdr[scn_idx].s_size = bss_size;

	    /* Virtual Address and Physical Address */
	    Scnhdr[scn_idx].s_paddr = bss_base_address;
	    Scnhdr[scn_idx].s_vaddr = Scnhdr[scn_idx].s_paddr;
	    temp_aout_hdr.o_snbss = ++scn_idx;	/* 1-based */
	}
	else
	    bss_size = 0;
    }
    else {
	data_size = 0;
	bss_size = 0;
	temp_aout_hdr.o_data_start = 0;
	/* Fields o_dsize, o_bsize, and o_algndata are initialized to 0. */
    }

    say(SAY_NORMAL, NLSMSG(SAVE_SIZES,
	  "%1$s: Section sizes = %2$d+%3$d+%4$d (0x%2$X+0x%3$X+0x%4$X hex)"),
	Command_name, text_size, data_size, bss_size);

    next_scn = scn_idx;

    if (csect_seen == 0)		/* No CSECTs (although passed through
					   imports could exist. */
	return offset;

    /* Copy TEXT and DATA csects from input files to output file and update
       addresses according to the relocation entries. */
    limits[0] = first_data_index - 1;
    limits[1] = last_data_index;
    raw_target_orgs[0] = raw_text_target_org + (uint32)Shmaddr;
    raw_target_orgs[1] = raw_data_target_org + (uint32)Shmaddr;

    if (text_fixup_CSECT) {
	cur_fixup_ptr = (long)text_fixup_CSECT->c_new_addr;
	*(uint32 *)(raw_target_orgs[0] + cur_fixup_ptr)
	    = OP_BRANCH << 26; /* Dummy branch */
	cur_fixup_ptr += 4;
    }

    for (i = first_text_index, j = 0; j < 2; ++j) {
	raw_target_org = raw_target_orgs[j];
	limit = limits[j];
	for (; i <= limit; i++) {
	    CSECT *cs = Queue[i];

	    if (cs->c_len == 0)		/* 0-length */
		continue;

	    new_address = cs->c_new_addr;

	    if (cs->c_secnum <= 0) {
		switch(cs->c_secnum) {
		  case N_GENERATED:
		    /* Generated CSECT (must be a descriptor??) */
		    /* Write 0s */
		    memset((void *)(raw_target_org + new_address), 0,
			   cs->c_len);
		    break;

		  case N_FIXUP:
		    /* fixup csect--generated during relocation */
		    continue;

		  case N_UNDEF:
		  case N_IMPORTS:
		  case N_UNKNOWN:
		  default:
		    internal_error();
		    break;
		}
	    }
	    else if (cs->c_symbol.s_smtype == XTY_CM) {
		/* A BSS symbol was moved into the data section because it
		   had a data-in-TOC reference.  It is initialized to 0.
		   If the symbol was in a data section, any previous
		   initialization should have been 0. */

#ifdef DEBUG
		if (cs->c_symbol.s_smclass != XMC_TD || cs->c_first_rld != NULL)
		    internal_error();

		/* Check to make sure initialization was 0. */
		obj = cs->c_srcfile->sf_object;
		if (obj->oi_section_info[cs->c_secnum-1].sect_type
		    == STYP_DATA) {
		    int i;
		    ifile = obj->o_ifile;
		    ifile_reopen_remap(ifile);

#ifdef READ_FILE
		    if (obj->o_ifile->i_access == I_ACCESS_READ) {
			if (fseek_read(obj->o_ifile,
				       obj->oi_section_info[cs->c_secnum-1]
				       .u.raw_offset + cs->c_addr,
				       (void *)(raw_target_org + new_address),
				       cs->c_len) != 0)
			    return offset;
		    }
		    else
#endif
			memcpy((void *)(raw_target_org + new_address),
			       obj->oi_section_info[cs->c_secnum-1].u.raw_offset
			       + ifile->i_map_addr + cs->c_addr,
			       cs->c_len);
		    for (i = 0; i < cs->c_len; i++)
			if (*(char *)(raw_target_org + new_address + i) != '\0')
			    internal_error();
		}
#endif

		/* Initialize to 0. */
		memset((void *)(raw_target_org + new_address), 0, cs->c_len);
	    }
	    else {
		obj = cs->c_srcfile->sf_object;

		if (obj->oi_section_info[cs->c_secnum-1].sect_type == STYP_BSS)
		    internal_error();	/* Nothing to copy for BSS */
		ifile = obj->o_ifile;

		ifile_reopen_remap(ifile);

#ifdef READ_FILE
		if (obj->o_ifile->i_access == I_ACCESS_READ) {
		    if (fseek_read(obj->o_ifile,
				   obj->oi_section_info[cs->c_secnum-1]
				   .u.raw_offset + cs->c_addr,
				   (void *)(raw_target_org + new_address),
				   cs->c_len) != 0)
			return offset;
		}
		else
#endif
		    memcpy((void *)(raw_target_org + new_address),
			   obj->oi_section_info[cs->c_secnum-1].u.raw_offset
			   + ifile->i_map_addr + cs->c_addr,
			   cs->c_len);
	    }

	    /* relocate address constants */
	    do_relocation(cs, raw_target_org, int_switches);
	}
    }					/* for all .text and .data csects */

    if (text_fixup_CSECT) {
	long len;
#ifdef CENTERLINE
	struct tbtable_short tbtable;
	memset(&tbtable, 0, sizeof(tbtable));
#else
	struct tbtable_short tbtable = {0};
#endif

	/* Write the traceback table */
	/* Write word of 0s as marker for traceback table. */
	memset((void *)(raw_text_target_org + (uint32)Shmaddr + cur_fixup_ptr),
	       0, 4);
	tbtable.version = 0;
	tbtable.lang = TB_ASM;
	tbtable.fixup = 1;
	tbtable.tocless = 1;
	memcpy((void *)(raw_text_target_org + (uint32)Shmaddr
			+ cur_fixup_ptr + 4), &tbtable, 8);
	cur_fixup_ptr += 12;
	len = (long)cur_fixup_ptr - (long)text_fixup_CSECT->c_new_addr;
	if (int_switches & SWITCHES_VERBOSE)
	    say(SAY_NORMAL,
		NLSMSG(FIXUP_SIZE, "%1$s: Bytes written to fixup csect: %2$d"),
		Command_name, len);
	if (len > text_fixup_CSECT->c_len)
	    internal_error();
    }

    Bind_state.out_writing = 0;

    say(SAY_NORMAL,
	NLSMSG(SAVE_TOC_SIZE, "%1$s: Size of TOC: %2$d (0x%2$X hex)"),
	Command_name, toc_size);

    return offset;
} /* save_csects */
/************************************************************************
 * Name: do_relocation
 *									*
 * Purpose: Relocates the code
 *
 ************************************************************************/
static void
do_relocation(CSECT *cs,
	      uint32 raw_target_org,
	      const int int_switches)
{
    int			ovfl;
    int32		old_delta, new_delta;
    struct abs_info	abs_state;
    unsigned long	rld_off;
    RLD			*rld, *old_rld;
    SYMBOL		*new_sym, *old_sym, *save_old_sym;

    DEBUG_MSG(RELOC_DEBUG,
	      (SAY_NO_NLS, "Starting relocation for csect %s\n",
	       cs->c_symbol.s_name->name));

    if (int_switches & SWITCHES_DBG_OPT7)
	dump_csect(cs, 0 /* No extra debugging information */);

    for (rld = cs->c_first_rld; rld; rld = rld->r_next) {
	if ((int_switches & SWITCHES_DBG_OPT7)
#ifdef DEBUG
	    || (bind_debug & RELOC_DEBUG)
#endif
	    ) {
	    say(SAY_NL_ONLY);
	    dump_rld(rld);
	}

	/*
	The old address of a symbol is:
		old_sym->s_addr;

	The new address of a symbol is:
		new_sym->s_csect->c_new_addr +
	   		(new_sym->s_addr - new_sym->s_csect->c_addr)

	The address of an unresolved symbol is 0.

	For most references, we set
		old_delta = &old_sym	AND
		new_delta = &new_sym

	For TOC-relative references, we use:
		old_delta = &old_sym - &old_TOC		AND
		new_delta = &new_sym - &new_TOC

	For relative branches, we use:
		old_delta = &old_sym, if the old branch is absolute	OR
		old_delta = &old_sym - &(old branch instruction), if the
						old branch is relative; AND
		new_delta = &new_sym if the new branch is absolute	OR
		new_delta = &new_sym - &(new branch instruction), if the
						new branch is relative.

	The address calculations are not used for R_REF, R_GL, and R_TCL
	relocation types.
	*/

	if (!(rld->r_flags & RLD_TOCDATA_FIXUP))
	    old_sym = rld->r_sym;
	else {
	    old_sym = rld->r_sym->s_csect->c_first_rld->r_sym;
	    if (!(int_switches & SWITCHES_EXECUTE)) {
		/* If we're not creating an executable, we just restore
		   the original RLD.  Because it's too late to get rid of
		   the generated TOC entry, it will be in the output file,
		   but it may not be referenced */
		rld->r_flags &= ~RLD_TOCDATA_FIXUP; /* Reset flag */
		rld->r_flags |= (RLD_RESOLVE_BY_NAME | RLD_EXT_REF);
		rld->r_sym = old_sym;
		/* new_sym will be initialized below. */
	    }
	    else {
		/* When we set the RLD_TOCDATA_FIXUP, we changed the reference
		   from an XMC_TD symbol to a TOC symbol.  The new symbol is
		   the TOC entry. */
		new_sym = rld->r_sym->s_resolved;
	    }
	}

	old_delta = old_sym->s_addr;

	/* If fixup code had been generated for data-in-TOC, then the
	   instruction we're relocating actually refers to the previously
	   generated TC symbol, to which we have no direct reference.  In this
	   case, the relocation is undone by the do_relocation() routine without
	   any need for old_sym, so we don't bother searching for the old TC
	   symbol. */

	if (!(rld->r_flags & RLD_TOCDATA_FIXUP)) {
	    if (rld->r_flags & RLD_RESOLVE_BY_NAME) {
		if (old_sym->s_name->flags & STR_NO_DEF) {
		    new_sym = NULL;
		}
		else {
		    new_sym = old_sym->s_name->first_ext_sym;
		    /* If we added local glue, we have "two" new symbols.  One
		       for the actual code, and the other for the glue code.
		       The glue code symbol will be first in the first_ext_sym
		       chain.  The real code will be the following symbol.
		       Thus, the exported symbol will be the glue code, but
		       all other references to the symbol will be to the
		       actual code. */
		    if (!(rld->r_flags & RLD_EXT_REF)
			&& (new_sym->s_flags & S_LOCAL_GLUE)
			&& new_sym->s_synonym /* ????? */
			&& cs->c_symbol.s_smclass == XMC_DS)
			new_sym = new_sym->s_synonym;
		}
	    }
	    else {
		/* Referenced symbol is internal symbol */
#ifdef DEBUG
		if (!(old_sym->s_flags & S_RESOLVED_OK))
		    internal_error();
#endif
		new_sym = old_sym->s_resolved;
	    }
	}
	if (new_sym == NULL)
	    new_delta = 0;
	else {
	    new_delta = new_sym->s_csect->c_new_addr
		+ (new_sym->s_addr - new_sym->s_csect->c_addr);
	}

	rld_off = rld->r_addr + (cs->c_new_addr - cs->c_addr);
	switch(rld->r_reltype) {
	  case R_REF:			/* Nothing to reference. */
	    break;

	  case R_GL:		/* TOC of non-local symbol */
	    memset((void *)(raw_target_org + rld_off), 0, 4);
	    break;

	  case R_TCL:		/* TOC of local symbol */
#ifdef DEBUG
	    if (sect_info[SC_TC0].heads == -1)
		internal_error();
#endif
	    memcpy((void *)(raw_target_org + rld_off),
		   (void *)toc_anchor_address, 4);
	    break;

	  case R_BA:		/* Branch absolute--not modifiable */
	  case R_RBA:		/* Branch absolute--modifiable */
	  case R_BR:		/* Branch relative--not modifiable */
	  case R_RBR:		/* Branch relative--modifiable */
	    prebind((caddr_t)(raw_target_org + rld_off),
		    cs, rld, new_sym, &abs_state, int_switches);

	    if (!abs_state.was) {
		/* Old branch address */
		old_delta -= rld->r_addr;
	    }
	    if (!abs_state.is) {
		/* New branch address */
		new_delta -= rld->r_addr + (cs->c_new_addr - cs->c_addr);
	    }
	    if ((old_delta & 0x3) || (new_delta & 0x3)) {
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(BRANCH_UNALIGN,
 "%1$s: 0711-799 SEVERE ERROR: Object %2$s, section %3$d\n"
 "\tThe target of the branch instruction at address %4$x is not aligned\n"
 "\ton a word boundary. The target address is being truncated."),
		     Main_command_name,
		     get_object_file_name(cs->c_srcfile->sf_object),
		     cs->c_secnum,
		     rld->r_addr);

		old_delta &= ~0x3;
		new_delta &= ~0x3;
	    }
	    goto do_reloc;

	  case R_REL:
	    if (rld->r_length == 26)
		prebind((caddr_t)(raw_target_org + rld_off),
			cs, rld, new_sym, &abs_state, int_switches);
	    old_delta -= rld->r_csect->c_addr;
	    new_delta -= rld->r_csect->c_new_addr;

	    goto do_reloc;

	  case R_TOC:
	  case R_TRL:
	  case R_TRLA:
	    if (old_sym->s_smtype != XTY_ER) {
		/* If no TOC anchor exists in the same section as old_sym
		   (which is an error), then resolve will have initialized
		   l_toc_anchor to point to a dummy symbol. */
		old_delta -= old_sym->s_csect->c_srcfile->sf_object
		    ->oi_section_info
			[old_sym->s_csect->c_secnum-1].l_toc_anchor->s_addr;
	    }
	    if (new_sym) {
#ifdef DEBUG
		if (sect_info[SC_TC0].heads == -1)
		    internal_error();
#endif
		new_delta -= toc_anchor_address;
	    }
	    goto do_reloc;

	  case R_NEG:
	    old_delta = -old_delta;
	    new_delta = -new_delta;
	    /* Fall through */

	  case R_POS:
	  case R_RL:
	  case R_RLA:
	  do_reloc:
	    /* relocate and overflow check */
	    ovfl = rel_ovfl((caddr_t)(raw_target_org + rld_off), cs, rld,
			    old_delta, new_delta, int_switches);
	    if (!(int_switches & SWITCHES_EXECUTE))
		break;			/* No overflow messages and no fixup
					   code if we're not creating an
					   executable file. */

	    /* A reference to an imported or undefined data-in-TOC symbol
	       must always be fixed, whether overflow occurred or not. */
	    if (rld->r_flags & RLD_TOCDATA_FIXUP)
		goto call_do_fixup;

#ifdef DEBUG
	    /* If we set an arbitrary TOC size to test TOC overflow, then
	       ovfl might not be 1, but we'll still want to generate fixup
	       code for these artifically created overflows. */
	    if (new_delta >= ARCH_LIMIT/2
		&& rld->r_length == 16
		&& (rld->r_flags & RLD_OVERFLOW_FIXUP_OK)
		&& new_sym
		&& (new_sym->s_smclass == XMC_TC
		    || new_sym->s_smclass == XMC_TD))
		ovfl = 1;
#endif

	    if (ovfl) {
		/* We had relocation overflow.  Most kinds of TOC overflow
		   can be fixed up as long as new_delta >= ARCH_LIMIT/2.
		   (If new_delta < ARCH_LIMIT/2,  space in the fixup csect
		   isn't allocated.)

		   Even if we're handling TOC overflow, we will have
		   relocation overflow if a TOC item (XMC_TD or XMC_TC) begins
		   at an offset less than 2^15 but the effective address
		   computed by the overflowing instruction is beyond the
		   addressable range.  This is a programming error, and
		   will not be fixed up.  The condition
		   "new_delta >= ARCH_LIMIT/2" detects this case.

		   In addition, RLD_OVERFLOW_FIXUP_OK might still be set
		   for a reserved name, which couldn't be moved into the
		   TOC.  In this case, the storage-mapping class of the
		   new symbol will be neither XMC_TC nor XMC_TD and the
		   symbol will not have been mapped into the TOC.  This
		   case is not fixed up either.

		   Last, for fixup to occur, the address of the field being
		   relocated must be on an odd half-word boundary and the
		   RLD bit-length must be 16. */
		if (rld->r_length == 16
		    && (rld->r_flags & RLD_OVERFLOW_FIXUP_OK)
		    && new_delta >= ARCH_LIMIT/2
		    && new_sym
		    && (new_sym->s_smclass == XMC_TC
			|| new_sym->s_smclass == XMC_TD)) {
#ifdef DEBUG
		    /* We should never be here if we didn't allocate space
		       in the fixup region for this RLD. */
		    if (!(rld->r_flags & RLD_FIXUP_OK))
			internal_error();
#endif
		  call_do_fixup:
		    if (do_fixup(raw_target_org, rld_off, cs, rld,
				 new_delta, int_switches) == 1)
			goto report_overflow; /* Fixup failed */
		}
		else {
		    SYMBOL	*sym, *winning_symbol;
		    int		diff;
		    /* For the name of the symbol making the reference, we
		       find the label before and closest to the address of the
		       RLD.  Because labels aren't necessarily in address
		       order, we have to check all labels. If the first label
		       is hidden and at the same address as the csect, we'll
		       never use the csect name.  If multiple labels are at
		       the same address, the first label (in symbol table
		       order) will be used. */

		  report_overflow:
		    sym = &cs->c_symbol;
		    /* Ignore hidden name on csect if first label is at same
		       address. */
		    if (sym->s_next_in_csect
			&& (sym->s_flags & S_HIDEXT)
			&& sym->s_next_in_csect->s_addr == sym->s_addr)
			sym = sym->s_next_in_csect;
		    winning_symbol = sym; /* Default in case of corrupted
					     labels. */
		    diff = cs->c_len;
		    for ( ; sym; sym = sym->s_next_in_csect) {
			if (rld->r_addr - sym->s_addr > 0
			    && rld->r_addr - sym->s_addr < diff) {
			    winning_symbol = sym;
			    diff = rld->r_addr - sym->s_addr;
			}
		    }
		    bind_err(SAY_NORMAL, RC_SEVERE,
			     NLSMSG(SAVE_RELFLDOVRF,
    "%1$s: 0711-780 SEVERE ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n"
    "\tRelocation overflow in reference to: %5$s (entry %6$d)\n"
    "\tAddress: 0x%7$08x;    RLD type: %8$s;   RLD length: %9$d"),
			     Main_command_name,
			     winning_symbol->s_name->name,
			     winning_symbol->s_inpndx,
			     get_object_file_name(cs->c_srcfile->sf_object),
			     rld->r_sym->s_name->name,
			     rld->r_sym->s_inpndx,
			     rld->r_addr,
			     get_reltype_name(rld->r_reltype),
			     rld->r_length);
		}
	    }
#if 0
	    else {
		/* It is possible for RLD_FIXUP_OK to be set here.
		   This case can occur if the effective address computed by
		   an instruction does not result in TOC overflow, but
		   new_delta >= ARCH_LIMIT/2.  We will have allocated space
		   in the fixup csect, but no fixup code will be needed.
		   This case is harmless. */
	    }
#endif
	    break;

	  default:
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(RELOC_BAD_RLD,
	    "%1$s: 0711-782 SEVERE ERROR: Object %2$s, section %3$d\n"
	    "\tThe RLD at address %4$x has an unrecognized RLD type 0x%5$x.\n"
	    "\tThe RLD is not being processed."),
		     Main_command_name,
		     get_object_file_name(cs->c_srcfile->sf_object),
		     cs->c_secnum,
		     rld->r_addr,
		     rld->r_reltype);
	}
    } /* RLD loop */

    if (int_switches & SWITCHES_DBG_OPT7)
	say(SAY_NL_ONLY);

} /* do_relocation */
/************************************************************************
 * Name: prebind
 *									*
 * Purpose:
 *
 * Returns:
 *									*
 ************************************************************************/
static void
prebind(caddr_t mem_ptr,
	CSECT *cs,
	RLD *rld,
	SYMBOL *new_sym,		/* New instance of symbol--could
					   be NULL if symbol is undefined. */
	struct abs_info *abs_info,
	const int int_switches)
{
    int			inst;
    union instruction_t	*word = (union instruction_t *)mem_ptr;

    if (word->branch.opcode != OP_BRANCH) {
	SYMBOL	*sym, *winning_symbol;
	int		diff;
	/* For the name of the symbol making the reference, we
	   find the label before and closest to the address of the
	   RLD.  Because labels aren't necessarily in address
	   order, we have to check all labels. If the first label
	   is hidden and at the same address as the csect, we'll
	   never use the csect name.  If multiple labels are at
	   the same address, the first label (in symbol table
	   order) will be used. */
	sym = &cs->c_symbol;
	/* Ignore hidden name on csect if first label is at same
	   address. */
	if (sym->s_next_in_csect
	    && (sym->s_flags & S_HIDEXT)
	    && sym->s_next_in_csect->s_addr == sym->s_addr)
	    sym = sym->s_next_in_csect;
	winning_symbol = sym; /* Default in case of corrupted labels. */
	diff = cs->c_len;
	for ( ; sym; sym = sym->s_next_in_csect) {
	    if (rld->r_addr - sym->s_addr > 0
		&& rld->r_addr - sym->s_addr < diff) {
		winning_symbol = sym;
		diff = rld->r_addr - sym->s_addr;
	    }
	}
	bind_err(SAY_NORMAL, RC_SEVERE,
		 NLSMSG(SAVE_BAD_BRANCH,
 "%1$s: 0711-787 ERROR: Symbol %2$s (entry %3$d) in object %4$s:\n"
 "\tThe instruction at address 0x%5$08x is not a branch. It is 0x%6$08x.\n"
 "\tRLD type: %7$s;   RLD length: %8$d;    Referenced symbol: %9$s"),
		 Main_command_name,
		 winning_symbol->s_name->name,
		 winning_symbol->s_inpndx,
		 get_object_file_name(cs->c_srcfile->sf_object),
		 rld->r_addr,
		 word->i,
		 get_reltype_name(rld->r_reltype),
		 rld->r_length,
		 rld->r_sym->s_name->name);
	return;
    }

    abs_info->was = word->branch.aa;
    /* If we are doing a final link, we can convert branch instructions
       from absolute to relative or vice versa depending on their targets.
       We do not convert if we are not doing a final link (i.e.,
       Switches.execute == 0) or the referenced symbol is not defined.
       Finally, we do not convert if -bdbg:loadabs was used. */
    if ((new_sym != NULL
	 && ((int_switches & (SWITCHES_EXECUTE | SWITCHES_LOADABS))
	     == SWITCHES_EXECUTE)))
	abs_info->is = new_sym->s_smclass == XMC_XO;
    else
	abs_info->is = abs_info->was;	/* No conversion */

    switch(rld->r_reltype) {
      case R_REL:
	/* An R_REL RLD should never result in a conversion from BA to B. */
	abs_info->is = abs_info->was;
	goto no_conversion;

	/* Defect 142012:  R_BA is treated just like R_RBA.  Error message
	   795 is not updated to avoid changing the message catalog. */
      case R_BA:			/* Branch absolute--not modifiable */
      case R_RBA:			/* Branch absolute--modifiable */
	if (!abs_info->was) {
	    /* This should never happen. */
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(SAVE_BA_BAD,
	    "%1$s: 0711-795 SEVERE ERROR: Object %2$s, section %3$d\n"
	    "\tThe relocation type of the RLD for address 0x%4$x is R_BA,\n"
	    "\tbut the instruction is not an absolute branch.\n"
	    "\tThe referenced symbol is %5$s"),
		     Main_command_name,
		     get_object_file_name(cs->c_srcfile->sf_object),
		     cs->c_secnum,
		     rld->r_addr,
		     rld->r_sym->s_name->name);
	}
	break;

	/* Defect 142012:  R_BR is treated just like R_RBR.  Error message
	   797 is not updated to avoid changing the message catalog. */
      case R_BR:			/* Branch relative--not modifiable */
      case R_RBR:			/* Branch relative--modifiable */
	if (abs_info->was) {
	    /* This should never happen. */
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(SAVE_BR_BAD,
	    "%1$s: 0711-797 SEVERE ERROR: Object %2$s, section %3$d\n"
	    "\tThe relocation type of the RLD for address 0x%4$x is R_BR,\n"
	    "\tbut the instruction is an absolute branch.\n"
	    "\tThe referenced symbol is %5$s"),
		     Main_command_name,
		     get_object_file_name(cs->c_srcfile->sf_object),
		     cs->c_secnum,
		     rld->r_addr,
		     rld->r_sym->s_name->name);
	}
	break;
    }

#ifdef DEBUG
    if (bind_debug & RELOC_DEBUG) {
	uint8 new_reltype = abs_info->is ? R_RBA : R_RBR;
	if (rld->r_reltype != new_reltype)
	    say (SAY_NO_NLS, "Converting RLD type.");
    }
#endif
    rld->r_reltype = abs_info->is ? R_RBA : R_RBR; /* Convert, maybe. */
    /* Update and copy back instruction. */
#ifdef DEBUG
    if (bind_debug & RELOC_DEBUG)
	if (word->branch.aa != abs_info->is)
	    say (SAY_NO_NLS, "Converting instruction");
#endif
    word->branch.aa = abs_info->is;

  no_conversion:

    if (!word->branch.lk)		/* link bit not set */
	return;

    /* Potential bug:  What if the branch is at the end of the csect? */
    word = (union instruction_t *)(mem_ptr + 4); /* Point to next word. */

    /* If out of module call or Pointer to function call */
    /* Then the instuction following the branch must be changed */
    /* to load (i.e. restore) the address of the TOC from the stack */

    if (new_sym == NULL /* Unresolved--assume out-of-module call */
	|| new_sym->s_smclass == XMC_GL	/* Call is through glue code */
	|| new_sym->s_name == ptrgl_name
	|| new_sym->s_name == PTRGL_name
 	/* Call is to POINTER-TO-FUNCTION entry by name */
	/* This does not allow user to provide his own  */
	/* Pointer-to-function routine with another unique name */
	) {
	for (inst = 0; inst < Bind_state.num_nops; inst++) {
	    /* See if the following instruction is a no-op. */
	    if (word->i == Bind_state.nops[inst])
		goto change_to_ltoc;
	}
	/* Word was not NOP--check for reload of TOC */
	if (int_switches & SWITCHES_EXECUTE) {
	    for (inst = 0; inst < Bind_state.num_loadtocs; inst++) {
		if (word->i == Bind_state.loadtocs[inst]) {
		    if (inst > 0) {
		      change_to_ltoc:
			word->i = Bind_state.loadtocs[0]; /*Use preferred form*/
		    }
		    goto LOADTOC_OK;
		}
	    }
	    if (new_sym && new_sym->s_smclass == XMC_GL) {
		bind_err(SAY_NORMAL, RC_WARNING,
			 NLSMSG(SAVE_BAD_TOCLOAD,
 "%1$s: 0711-768 WARNING: Object %2$s, section %3$d, function %4$s:\n"
 "\tThe branch at address 0x%5$x is not followed by a recognized no-op\n"
 "\tor TOC-reload instruction. The unrecognized instruction is 0x%6$X."),
			 Main_command_name,
			 get_object_file_name(cs->c_srcfile->sf_object),
			 cs->c_secnum,
			 new_sym->s_name->name,
			 rld->r_addr,
			 word->i);
	    }
	}
      LOADTOC_OK:
	;
    }
    else {
	/* Check for a NOP, and change to preferred form. */
	for (inst = 0; inst < Bind_state.num_nops; inst++) {
	    if (word->i == Bind_state.nops[inst]) {
		if (inst > 0) {
		  change_to_nop:
		    word->i = Bind_state.nops[0]; /* Use preferred form */
		}
		goto NOP_OK;
	    }
	}
	/* change a LOAD into a NOP */
	for (inst = 0; inst < Bind_state.num_loadtocs; inst++) {
	    if (word->i == Bind_state.loadtocs[inst])
		goto change_to_nop;
	}
      NOP_OK:
	;
    }
} /* prebind */
/************************************************************************
 * Name: rel_ovfl
 *									*
 * Purpose:  modify 'bits' of storage byte-aligned and padded
 *			on the left beginning at 'ptr'.  Modify by
 *			adding 'value' to current value and
 *			compensating for overflow.
 *
 * Returns:	1 if relocation overflow occurred.  0 otherwise
 *									*
 ************************************************************************/
static int
rel_ovfl(caddr_t ptr,			/* In-memory address of instruction
					   to be relocated. */
	 CSECT *cs,			/* Csect being relocated */
	 RLD *rld,			/* RLD from CSECT cs */
	 int32 old_delta,
	 const int32 new_delta,
	 const int int_switches)
{
    static char	*id = "reloc";

    uint32	mask, val_mask, temp1;
    signed int	src_value, tgt_value, tst_value, temp;
    uint32	u_src_value, u_tgt_value, u_tst_value;
    int		rel_rc;
    int		sign = (rld->r_flags & RLD_SIGNED) != 0; /* Must be 1 or 0 */
    int		bits = rld->r_length;
    const int32	delta = new_delta - old_delta;
    long	fixup_ptr;
    size_t	bytes;
    union instruction_t *iptr;
    union instruction_t source, target;
    union instruction_t old_fixup[MAX_OVERALL_FIXUP_LENGTH + 1];

    if (!sign && bits < 32 && Switches.target == POWER_TARGET)
	sign = 1;

    if (rld->r_flags & RLD_WAS_FIXED_UP) {
	OBJECT	*obj;
	int	copy, branch;
	IFILE	*ifile;

	/* We must restore original instruction */
	/* Get branch instruction to find address of fixup code. */
	memcpy(source.bytes, ptr - 2, 4);
	if (source.branch.opcode != OP_BRANCH) {
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(FIXUP_BAD,
 "%1$s: 0711-788 SEVERE ERROR: Object %2$s, RLD address 0x%3$x in section %4$d:\n"
 "\tThe fixed-up instruction at address 0x%5$x is not a branch.\n"
 "\tIts opcode is %6$d."),
		     Main_command_name,
		     get_object_file_name(cs->c_srcfile->sf_object),
		     rld->r_addr, cs->c_secnum,
		     rld->r_addr - 2, source.branch.opcode);
	    return 0;
	}
	fixup_ptr = source.branch.li * 4;

	obj = cs->c_srcfile->sf_object;
	ifile = obj->o_ifile;

#ifdef READ_FILE
	if (obj->o_ifile->i_access == I_ACCESS_READ) {
	    /* Note:  We assume that we can read the required instructions from
	       the fixup code.  If we're reading fixup code, the object must
	       have a symbol table, so there should always be enough bytes to
	       satisfy the read. */
	    if (fseek_read(obj->o_ifile,
			   obj->oi_section_info[cs->c_secnum-1].u.raw_offset
			   + rld->r_addr - 2 + fixup_ptr - 4,
			   &old_fixup[0],
			   (MAX_OVERALL_FIXUP_LENGTH + 1) * 4) != 0)
		return 0;
	}
	else
#endif
	    memcpy(&old_fixup[0],
		   obj->oi_section_info[cs->c_secnum-1].u.raw_offset
		   + ifile->i_map_addr
		   + rld->r_addr - 2 + fixup_ptr - 4,
		   (MAX_OVERALL_FIXUP_LENGTH + 1) * 4);

	if (old_fixup[0].branch.opcode != OP_BRANCH) {
	    memcpy(ptr - 2, &old_fixup[0], 4); /* Original instruction was
						  saved before fixup code. */
	}
	else {

	    /* WARNING:  The code used to restore the original instruction is
	       based on the kinds of fixup code generated.  This code is very
	       sensitive to the fixup code.  If more cases are added to fixup
	       code, this code may have to be changed as well. */

	    switch(old_fixup[1].dform.opcode) {
	      case OP_ADDIS:
		if (old_fixup[3].dform.opcode == OP_BRANCH) {
		    DEBUG_MSG(FIXUP_DEBUG,
			      (SAY_NO_NLS,
			       "rel_ovfl: --O Undoing TOC overflow"));
		    /* Original fixup: TOC overflow */
		    copy = 2;
		    branch = 3;
		}
		else if (old_fixup[1].dform.ra == old_fixup[1].dform.rt) {
		    DEBUG_MSG(FIXUP_DEBUG,
			      (SAY_NO_NLS,
			       "rel_ovfl: S-O Undoing TOC overflow (STORE)"));
		    /* Original fixup: TOC overflow for STORE */
		    copy = 2;
		    branch = 4;
		}
		else {
		    DEBUG_MSG(FIXUP_DEBUG,
			      (SAY_NO_NLS,
		       "rel_ovfl: -DO Undoing TOC overflow and data-in-TOC"));
		    /* Original fixup: both TOC overflow and data-in-TOC */
		    copy = 3;
		    branch = 4;
		    /* The fixup code has .Dl and offset in two different
		       instructions.  We can undo the original relocation
		       by ignoring .Dl and setting old_delta to 0.  See
		       the fixup code sequences in do_fixup(). */
		    old_delta = 0;
		    /* old_fixup[3].dform.d += old_fixup[2].dform.d; */
		}
		old_fixup[copy].dform.ra = old_fixup[1].dform.ra;
		break;

	      case OP_LWZ:
		DEBUG_MSG(FIXUP_DEBUG,
			  (SAY_NO_NLS, "rel_ovfl: -D- Undoing data-in-TOC"));
		/* Original fixup: data-in-TOC */
		old_fixup[2].dform.ra = old_fixup[1].dform.ra;
		    /* The fixup code has .Dl and offset in two different
		       instructions.  We can undo the original relocation
		       by ignoring .Dl and setting old_delta to 0.  See
		       the fixup code sequences in do_fixup(). */
		old_delta = 0;
		/* old_fixup[2].dform.d += old_fixup[1].dform.d; */
		copy = 2;
		branch = 3;
		break;

	      case OP_STW:
		if (old_fixup[2].dform.opcode == OP_ADDIS) {
		    DEBUG_MSG(FIXUP_DEBUG,
			      (SAY_NO_NLS,
	       "rel_ovfl: SDO Undoing TOC overflow and data-in-TOC (STORE)"));
		    /* Original fixup: both TOC overflow and data-in-TOC
		       for STORE. */
		    copy = 4;
		    branch = 6;
		}
		else {
		    DEBUG_MSG(FIXUP_DEBUG,
			      (SAY_NO_NLS,
			       "rel_ovfl: SD- Undoing data-in-TOC (STORE)"));
		    /* Original fixup: data-in-TOC for STORE. */
		    copy = 3;
		    branch = 5;
		}
		old_fixup[copy].dform.ra = old_fixup[2].dform.ra;
		    /* The fixup code has .Dl and offset in two different
		       instructions.  We can undo the original relocation
		       by ignoring .Dl and setting old_delta to 0.  See
		       the fixup code sequences in do_fixup(). */
		old_delta = 0;
		/* old_fixup[copy].dform.d += old_fixup[copy-1].dform.d; */
		break;

	      default:
		goto bad_fixup_code;
	    }
	    if (old_fixup[branch].dform.opcode != OP_BRANCH) {
	      bad_fixup_code:
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(FIXUP_BAD2,
 "%1$s: 0711-776 SEVERE ERROR: Object %2$s, RLD address 0x%3$x in section %4$d:\n"
 "\tThe generated instructions (at address 0x%5$s) for the fixed-up\n"
 "\tinstruction at address 0x%6$x are not recognized.\n"
 "\tThe instruction will not be restored or relocated."),
			 Main_command_name,
			 get_object_file_name(cs->c_srcfile->sf_object),
			 rld->r_addr, cs->c_secnum,
			 rld->r_addr - 2 + fixup_ptr,
			 rld->r_addr - 2);
		return 0;
	    }

	    if (int_switches & SWITCHES_DBG_OPT7)
		say(SAY_NO_NLS,
		    "    fixup: Restored instruction: %08x",
		    old_fixup[copy].i);

	    memcpy(ptr - 2, &old_fixup[copy], 4); /*  Copy back to output */
	}
    }

    if (old_delta == 0 && new_delta == 0) {
	if (int_switches & SWITCHES_DBG_OPT7)
	    say(SAY_NO_NLS,
		"    %s: old_delta = new_delta = 0\tNo relocation.", id);
	return 0;
    }

    if (int_switches & SWITCHES_DBG_OPT7)
	say(SAY_NO_NLS,
	    "    %s: sign=%d old_delta=%08X (%d)\tnew=%08X (%d)",
	    id, sign, old_delta, old_delta, new_delta, new_delta);

    switch(bits * 2 + sign) {
      case 2 * 32:
      case 2 * 32 + 1:
	/********************************
	 * no test for 32-bit overflow	*
	 ********************************/
	u_src_value = *(uint32 *)ptr;
	u_tgt_value = u_src_value + delta;

	if (int_switches & SWITCHES_DBG_OPT7)
	    say(SAY_NO_NLS,
		"    %s: source= %08X target= %08X rel_delta= %08X (%d)",
		id, u_src_value, u_tgt_value, delta, delta);

	*(uint32 *)ptr = u_tgt_value;	/* Write new value */
	return 0;

      case 2 * 16 + 1:
	src_value = (signed short)(*(signed short *)ptr - old_delta);
	tst_value = src_value + new_delta;
	tgt_value = (signed short)tst_value; /* Truncate & sign extend */
	rel_rc = (tgt_value != tst_value);
	if (int_switches & SWITCHES_DBG_OPT7) {
#ifdef AIX_REL3
	    /* Use two calls to say() to avoid bug in printf with multiple,
	       repeated positional parameters. */
	    say(SAY_NO_NLS,
		"    %1$s: src_instr = %2$08X "
				"source= %3$04hX (%3$d)\tbase_value= (%4$d)",
		id,
		*(uint32 *)(ptr-2),	/* Source instruction */
		*(int16 *)ptr,		/* Source displacement */
		src_value);
	    say(SAY_NO_NLS,
		"    %1$s: tgt_instr = %2$04X%3$04hX "
				"target= %3$04hX (%3$d)\ttst_value = (%4$d)",
		id, *(uint16 *)(ptr-2), tgt_value, tst_value);
#else
	    say(SAY_NO_NLS,
		"    %1$s: src_instr = %2$08X "
				"source= %3$04hX (%3$d)\tbase_value= (%4$d)\n"
		"    %1$s: tgt_instr = %5$04X%6$04hX "
				"target= %6$04hX (%6$d)\ttst_value = (%7$d)",
		id,
		*(uint32 *)(ptr-2),	/* Source instruction */
		*(int16 *)ptr,		/* Source displacement */
		src_value,
		*(uint16 *)(ptr-2), tgt_value, tst_value);
#endif
	}
	*(signed short *)ptr = tgt_value; /* Write new value */
	break;

      case 2 * 26 + 1:
	iptr = (union instruction_t *)ptr;
	temp1 = *(uint32 *)ptr;		/* Save original instruction */
	if (int_switches & SWITCHES_EXECUTE) {
	    temp = iptr->branch.li << 2;
	    iptr->branch2.li -= old_delta;

	    src_value = iptr->branch2.li; /* Sign-extend value. */
	    tst_value = src_value + new_delta;
	    iptr->branch2.li = tst_value; /* Truncate value. */
	    rel_rc = (tst_value != iptr->branch2.li); /* Check for overflow */

	    if (int_switches & SWITCHES_DBG_OPT7)
		say(SAY_NO_NLS,
    "    %1$s: src_instr= %2$08X src= %3$07X (%4$d)\tbase_value= (%5$d)\n"
    "    %1$s: tgt_instr= %6$08X tgt= %7$07X (%8$d)\ttst_value = (%9$d)",
		    id,
		    temp1,			temp & 0x03FFFFFC,
		    temp & ~3,			src_value & ~3,
		    *(uint32 *)ptr,		iptr->branch2.li & 0x03FFFFFC,
		    iptr->branch2.li & ~3,	tst_value & ~3);
	}
	else {
	    /* We don't need a check for overflow if we're not creating an
	       executable, so we can simple add and trunctate. */
	    rel_rc = 0;
	    src_value = iptr->branch2.li;
	    iptr->branch2.li = src_value + delta;
	    if (int_switches & SWITCHES_DBG_OPT7)
		say(SAY_NO_NLS,
 "    %1$s: src_instr= %2$08X src= %3$07X (%4$d)\trel_delta=(%5$d)\n"
 "    %1$s: tgt_instr= %6$08X tgt= %7$07X (%8$d)",
		    id, temp1,
		    src_value & 0x03FFFFFC, src_value & ~3,
		    delta, *(uint32 *)ptr,
		    iptr->branch2.li & 0x03FFFFFC, iptr->branch2.li & ~3);
	}
	break;

      default:
	source.i = 0;
	bytes = (bits + 7) / 8;		/* # of bytes affected */
	memcpy(&source.bytes[4 - bytes], ptr, bytes);

	mask = -1 << bits;		/* setup mask for # of bits */
	val_mask = ~mask;

	if (int_switches & SWITCHES_EXECUTE) {
	    /* Subtract old_delta without any overflow checking */
	    temp = (source.i - old_delta) & val_mask;

	    /* Now add new_delta, with overflow checking */
	    if (sign) {
		/* drag the sign bit across */
		src_value = (temp << (32 - bits)) >> (32 - bits);
		tst_value = src_value + new_delta;
		/* drag the sign bit across again */
		tgt_value = (tst_value << (32 - bits)) >> (32 - bits);
		rel_rc = (tgt_value != tst_value);
		/* keep old high order bits and new low order bits */
		target.i = (source.i & mask) | (tgt_value & val_mask);
		if (int_switches & SWITCHES_DBG_OPT7) {
		    say(SAY_NO_NLS,
    "    %s: source= %0*X (%d)\tbase_value= (%d)\n"
    "    %s: target= %0*X (%d)\ttest_value= (%d)",
			id, (bits + 3) / 4, source.i & val_mask,
			(source.i << (32 - bits)) >> (32 - bits), temp,
			id, (bits + 3) / 4, target.i & val_mask,
			tgt_value, tst_value);
		}
	    }
	    else {
		u_src_value = temp;
		u_tst_value = u_src_value + new_delta;
		u_tgt_value = u_tst_value & val_mask;
		rel_rc = (u_tgt_value != u_tst_value);
		/* keep old high order bits and new low order bits */
		target.i = (source.i & mask) | u_tgt_value;
		if (int_switches & SWITCHES_DBG_OPT7) {
		    say(SAY_NO_NLS,
    "    %s: source= %0*X (%u)  base_value= (%u)\n"
    "    %s: target= %0*X (%u)  test_value= (%u)",
			id, (bits + 3) / 4,
			source.i & val_mask, source.i & val_mask,
			u_src_value,
			id, (bits + 3) / 4,
			u_tgt_value, u_tgt_value,
			u_tst_value);
		}
	    }
	}
	else {
	    /* We don't need a check for overflow if we're not creating an
	       executable, so we can simply add and trunctate. */
	    target.i = ((source.i + delta) & val_mask) | (source.i & mask);
	    if (int_switches & SWITCHES_DBG_OPT7) {
		int nibbles = (bits + 3) / 4;
		if (sign)
		    say(SAY_NO_NLS,
	    "    %s: source = %0*X (%d)\ttarget = %0*X (%d)\trel_delta= (%d)",
			id,
			nibbles, source.i & val_mask,
			(source.i << (32 - bits)) >> (32 - bits),
			nibbles, target.i & val_mask,
			(target.i << (32 - bits)) >> (32 - bits),
			delta);
		else
		    say(SAY_NO_NLS,
"    %1$s: source = %3$0*2$X (%3$u)\ttarget = %4$0*2$X (%4$u)\trel_delta= (%5$d)",
			id, nibbles, source.i & val_mask,
			target.i & val_mask, delta);
	    }
	}
	memcpy(ptr, &target.bytes[4 - bytes], bytes); /* copy back */
    } /* switch */

    return rel_rc;
} /* rel_ovfl */
/************************************************************************
 * Name: do_fixup
 *									*
 * Purpose:  Generate fixup code for TOC overflow cases and imported
 *		data-in-TOC cases.
 *
 * 	When this function is called, relocation has already been
 *	performed and the displacement field of the overflowing
 *	instruction contains the TOC-relative address of some byte of the
 *	new symbol (that is, the new instance of the referenced symbol).
 *	If the referenced byte is not the first byte of the referenced
 *	symbol, this symbol offset must be preserved in the fixup code.
 *	In particular, for data-in-TOC fixup, the referenced symbol is the
 *	generated TOC entry, but the symbol offset is within the imported
 *	symbol, not the TOC entry.
 *
 *	If the original instruction in an input file is not a reference to
 *	an imported data-in-TOC symbol, then its form is
 *		OP	Rx,.D+offset(Rt)
 *	and at function entry, the instruction will have been relocated to:
 *		OP	Rx,.DL+offset(Rt)
 *	where .D is the TOC-relative address of the old symbol
 *	and .DL+offset is the low-order 16 bits of the 32-bit displacement
 *	to the appropriate byte of the new symbol. The input parameter
 *	'new_addr' is the 32-bit TOC-relative address of  the new symbol.
 *
 *	If the original instruction is a reference to an imported
 *	data-in-TOC symbol, it will look like
 *		OP	Rx,offset(Rt)
 *	and at function entry, the instruction will have been relocated to:
 *		OP	Rx,.D+offset(Rt)
 *	where .D is the TOC-relative address of the generated TOC entry that
 *	will contain the address of the new symbol.  If relocation overflow
 *	occurs in this case, then .D+offset will be the low-order 16 bits of
 *	the correct displacement.  Either way, 'new_addr' is the 32-bit
 *	TOC-relative address of the generated TOC entry.
 *
 *	This function must not be called unless new_addr >= ARCH_LIMIT/2
 *	or (rld->r_flags & RLD_TOCDATA_FIXUP) is 1.  In addition,
 *	rld->r_length should be 16 and rld->r_addr should specify a word
 *	address.
 *
 *	Note:  If offset > 0, it is possible for relocation overflow
 *	to occur even though new_addr < ARCH_LIMIT/2. In this case, do_fixup
 *	is not called and no fixup code is generated.
 *
 * Returns: 0 if fixup was successful;
 *	    1 if fixup was unsuccessful because the fixup csect is too
 *		far away;
 *	    2 for other unsuccessful cases.  When 2 is returned,
 *		an error message will have already been printed.
 *									*
 ************************************************************************/
static int
do_fixup(uint32 ptr,			/* In-memory address of virtual 0
					   of the current section in the
					   output file. */
	 unsigned long off,		/* Virtual address of instruction
					   being relocated. */
	 CSECT *cs,			/* Csect being relocated */
	 RLD *rld,			/* RLD for relocated instruction */
	 int32 new_addr,		/* TOC-relative address of
					   beginning of new symbol. */
	 int int_switches)
{
#define GPR_DSA 1				/* Stack pointer register */
#define WORK_DISP 16

    int		high_order_bits, ovfl;
    int		toc_reg, rwork;
    int16	offset;
    int32	x_new_addr;
    int32	b_offset;
    const int	bits = rld->r_length;
    const int	imported_tocdata = (rld->r_flags & RLD_TOCDATA_FIXUP) ? 2 : 0;

    union instruction_t original, temp;
#ifdef CENTERLINE
    /* Code_center won't allow initialized unions */
    union instruction_t branch;
    union instruction_t work;
#else
    union instruction_t branch = {(OP_BRANCH << 26)};
    union instruction_t work = {(GPR_DSA << 16) | WORK_DISP};
#endif

#define WRITE_INSTRUCTION(n, i) memcpy((void *)((n-1)*4 + ptr + cur_fixup_ptr),\
				       &i, 4)
#define D_FORM(i, op, Rt, Ra, D)\
    {i.dform.opcode = op; i.dform.rt = Rt; i.dform.ra = Ra; i.dform.d = D;}

    /* Primary opcodes */
#define D_NEVER 0			/* To document D-form instructions that
					   are never modified. */
#define D_LOAD		1		/* Target can be used as temporary */
#define D_NOLOAD	2		/* Target cannot be used as temporary */
#define D_MOD		3

    static char opcodes[64] = {
	0, 0,				/* INVALID 00-01 */
	D_NEVER, D_NEVER,		/* "tdi" 02	"twi" 03 */
	0, 0, 0,			/* 04-06 */
	D_NEVER, D_NEVER, D_NEVER,	/* "mulli" 07; "subfic" 08; "dozi" 09 */
	D_NEVER, D_NEVER,		/* "cmpli" 10;	"cmpi"  11; */
	D_MOD,				/* "addic" 12 */
	D_MOD,				/* "addic." 13 */
	D_LOAD,				/* "addi(cal)" 14 */
	D_NEVER,			/* "addis(cau)" 15 */
	0, 0, 0,			/* "bc" 16; "sc" 17; "b" 18 */
	0,				/* expanded opcode 19 */
	0, 0, 0,			/* "rlimi" 20; "rlinm" 21; "rlmi" 22 */
	0,				/* "rlnm"  23 */
	D_NEVER, D_NEVER, D_NEVER,	/* "ori" 24; "oris" 25; "xori" 26 */
	D_NEVER, D_NEVER, D_NEVER,	/* "xoris" 27;"andi." 28;"andis." 29 */
	0,				/* expanded opcode 30 */
	0,				/* expanded opcode 31 */
	D_LOAD,				/* "lwz" 32 */
	D_NEVER,			/* "lwzu" 33 */
	D_LOAD,		D_NEVER,	/* "lbz" 34	"lbzu" 35 */
	D_NOLOAD,	D_NEVER,	/* "stw" 36	"stwu" 37 */
	D_NOLOAD,	D_NEVER,	/* "stb" 38	"stbu" 39 */
	D_LOAD,		D_NEVER,	/* "lhz" 40	"lhzu" 41 */
	D_LOAD,		D_NEVER,	/* "lha" 42	"lhau" 43 */
	D_NOLOAD,	D_NEVER,	/* "sth" 44	"sthu" 45 */
	D_NEVER,	D_NEVER,	/* "lmw" 46	"stmw" 47 */
	D_NOLOAD,	D_NEVER,	/* "lfs" 48	"lfsu" 49 */
	D_NOLOAD,	D_NEVER,	/* "lfd" 50	"lfdu" 51 */
	D_NOLOAD,	D_NEVER,	/* "stfs" 52	"stfsu" 53 */
	D_NOLOAD,	D_NEVER,	/* "stfd" 54	"stfdu" 55 */
	D_NOLOAD,	D_NEVER,	/* "lfq" 56	"lfqu" 57 */
	D_NOLOAD,			/* "ld" "ldu" "lwa" 58 */
	0,				/* expanded opcode 59 */
	0,				/* "stfq" (POWER2) 60 */
	0,				/* "stfqu" (POWER2) 61 */
	0,				/* "std" "stdu" 62 */
	0				/* expanded opcode 63 */
	};

#ifdef CENTERLINE
    /* Code_center won't allow initialized unions */
    branch.branch.opcode = OP_BRANCH;
    work.i = (GPR_DSA << 16) | WORK_DISP;
#endif

    /* Copy entire instruction */
    memcpy(&original, (void *)(off + ptr - 2), 4);

    if (int_switches & SWITCHES_DBG_OPT7)
	say(SAY_NO_NLS, "    fixup: Instruction: %08x", original.i);

    if (opcodes[original.dform.opcode] == 0) {
	bind_err(SAY_NORMAL, RC_SEVERE,
		 NLSMSG(RELOC_CANNOT_FIXUP2,
 "%1$s: 0711-794 SEVERE ERROR: Object %2$s, RLD address 0x%3$x in section %4$d:\n"
 "\tThe instruction at address 0x%5$x cannot be fixed up:\n"
 "\tThe opcode is %6$d."),
		 Main_command_name,
		 get_object_file_name(cs->c_srcfile->sf_object),
		 rld->r_addr, cs->c_secnum,
		 rld->r_addr - 2, original.dform.opcode);
	return 2;			/* Failure */
    }

    toc_reg = original.dform.ra;
    if (bits != 16) {
	bind_err(SAY_NORMAL, RC_SEVERE,
		 NLSMSG(RELOC_CANNOT_FIXUP1,
 "%1$s: 0711-793 SEVERE ERROR: Object %2$s, RLD address 0x%3$x in section %4$d:\n"
 "\tThe instruction at address 0x%5$x cannot be fixed up:\n"
 "\tThe bit length of the reference is %6$d, not 16."),
		 Main_command_name,
		 get_object_file_name(cs->c_srcfile->sf_object),
		 rld->r_addr, cs->c_secnum,
		 rld->r_addr - 2,
		 bits);
	return 2;			/* Failure */
    }

    /* Compute offset into referenced symbol. */
    offset = (int16)((int32)original.dform.d - new_addr);

    if (imported_tocdata)
	x_new_addr = new_addr;
    else
	x_new_addr = new_addr + offset;

    if (x_new_addr >= ARCH_LIMIT/2) {
	ovfl = 1;
	high_order_bits = (x_new_addr + REAL_ARCH_LIMIT/2)/REAL_ARCH_LIMIT;
    }
    else {
	ovfl = 0;
	high_order_bits = 0;
    }

    /* In some cases, ADDI can be transformed in-place to a LWZ instruction.
       Otherwise, fixup code is required. */
    if (original.dform.opcode == OP_ADDI
	&& ovfl == 0			/* Must be data-in-TOC */
	&& offset == 0) {
	DEBUG_MSG(FIXUP_DEBUG, (SAY_NO_NLS, "ADDI in-place"));
	original.dform.opcode = OP_LWZ; /* Convert instruction to LOAD */
	rld->r_flags &= ~RLD_TOCDATA_FIXUP; /* Reset fixup flag--this
					       transformation is permanent. */
	/* Transformation in place */
	/* TRANSFORM:	ADDI	Rx,D(Rt)
	   TO:		LWZ	Rx,.D(Rt) */
	memcpy((void *)(ptr + off - 2), &original, 4); /* Copy back */
	return 0;			/* Success */
    }

#if FIXUP_SAVE != 0
    /* Save original instruction before fixup code. This will only be needed
     if we start generating fixup code that can't be undone by examining the
     code itself, and even then, it may not be needed for all fixup
     sequences. */
    memcpy((void *)(ptr + cur_fixup_ptr), &original, 4);
    cur_fixup_ptr += 4;
#endif

    b_offset = cur_fixup_ptr - (off - 2);

#ifdef DEBUG
    if ((b_offset & 3) || b_offset < 0)
	internal_error();
#endif
    if (b_offset > REAL_MAX_BRANCH_OFFSET)
	return 1;			/* Failure */

    branch.branch.li = b_offset/4;
    memcpy((void *)(ptr + off - 2), &branch, 4); /* Copy fixup branch back */
    rld->r_flags |= RLD_FIXUP_USED;	/* Set the flag once the branch to
					   fixup code is written. */

    /* In the descriptions of the transformations below, the following
       notation is used:
	Rx	RT or RS field of D-form instruction
	Rt	Base register (RA) of instruction.  This will usually be 2,
		the TOC register
	D	Offset within the TOC to a (data-in-TOC) variable
	.D	Offset within the TOC to a TOC entry
	offset	Offset of the referenced byte (within its symbol)
	.Dl	Low-order 16 bits of .D
	.Du	High-order 16 bits of .D

	If we have a data-in-TOC access to a non-imported variable and
	TOC overflow, the usual TOC-overflow transformation is effected.
	To be completely accurate, comments in the code should use
	D, Dl, and Du, instead of .D, .Dl, and .Du

	Six possible transformations are possible. Fields from the original
	instruction are shown in uppercase, while generated instruction
	fields are in lowercase.  To undo the transformation, the uppercase
	fields must be restored.  We never need to get .du explicitly, because
	it is handled by 2's-complement arithmetic.

	WARNING:  These 6 sequences are the only ones detected by the code
	used during rebinding to restore the original instruction.  If these
	sequences are modified, or if new sequences are added, the code
	in rel_ovfl() used to restore the original instruction will have to
	be modified as well.

	Original Instruction		Generated instruction sequence
	--------------------		------------------------------
    RX can be used as a base register.

	L_OP	RX,.D+OFFSET(RT)	addis	rx,.du(RT)	TOC overflow
	       				L_OP	RX,.DL+OFFSET(rx)

	L_OP	RX,D+OFFSET(RT)		lwz	rx,.d(RT)	Data-in-TOC
	       				L_OP	RX,OFFSET(rx)

	L_OP	RX,D+OFFSET(RT)		addis	rx,.du(RT)	Both
	       				lwz	rx,.dl(rx)
	       				L_OP	RX,OFFSET(rx)

    RX cannot be used as a base register because the instruction is not a load,
    RX == 0, or the instruction is a floating-point load.

	OP	RX,.D+OFFSET(RT)	addis	rt,.du(RT)	TOC overflow
	       				OP	RX,.DL+OFFSET(rt)
	       				addis	rt,-.du(rt)

	OP	RX,D+OFFSET(RT)		stw	rw,16(rdsa)	Data-in-TOC
					lwz	rw,.d(RT)
		       			OP	RX,OFFSET(rw)
					lwz	rw,16(rdsa)

	OP	RX,D+OFFSET(RT)		stw	rw,16(rdsa)	Both
					addis	rw,.du(RT)
					lwz	rw,.dl(rw)
	       				OP	RX,OFFSET(rw)
					lwz	rw,16(rdsa)
    */
    switch(opcodes[original.dform.opcode]) {
      case D_LOAD:
	switch(imported_tocdata + ovfl) {
	  case 0:
	    internal_error();
	    break;

	  case 1:			/* TOC overflow */
	    if (original.dform.rt == 0)
		goto use_tocreg;

	    DEBUG_MSG(FIXUP_DEBUG, (SAY_NO_NLS, "do_fixup: --O (Case 1)"));

	    /* TRANSFORM:	L_OP	Rx,.D+offset(Rt)

	       TO:	(1)	ADDIS	Rx,.Du(Rt)
	       		(2)	L_OP	Rx,.Dl+offset(Rx) */

	    /* Instruction 1:	ADDIS	Rx,.Du(Rt) */
	    D_FORM(temp, OP_ADDIS, original.dform.rt, toc_reg, high_order_bits);
	    WRITE_INSTRUCTION(1, temp); /* Copy addis */

	    /* Instruction 2:	L_OP	Rx,.Dl+offset(Rt) */
	    original.dform.ra = original.dform.rt;
	    WRITE_INSTRUCTION(2, original); /* Copy load */

	    cur_fixup_ptr += 8;		/* 2 instructions generated. */
	    break;

	  case 2:			/* Data-in-TOC */
	    if (original.dform.rt == 0)
		goto use_work_area;

	    DEBUG_MSG(FIXUP_DEBUG, (SAY_NO_NLS, "do_fixup: -D- (Case 2)"));

	    /* TRANSFORM:	L_OP	Rx,D+offset(Rt)
	       AFTER RELOCATION:L_OP	Rx,.D+offset(Rt)

	       TO:	(1)	LWZ	Rx,.D(Rt)
			(2)	L_OP	Rx,offset(Rx) */

	    /* Instruction 1:	LWZ	Rx,.D(Rt) */
	    D_FORM(temp, OP_LWZ, original.dform.rt, toc_reg, new_addr);
	    WRITE_INSTRUCTION(1, temp);

	    /* Instruction 2:	L_OP	Rx,offset(Rx) */
	    original.dform.ra = original.dform.rt;
	    original.dform.d = offset;
	    WRITE_INSTRUCTION(2, original);

	    cur_fixup_ptr += 8;		/* 2 instructions generated. */
	    break;

	  case 3:			/* Data-in-TOC and TOC overflow */
	    if (original.dform.rt == 0)
		goto use_work_area2;

	    DEBUG_MSG(FIXUP_DEBUG, (SAY_NO_NLS, "do_fixup: -DO (Case 3)"));

	    /* TRANSFORM:	L_OP	Rx,D+offset(Rt)
	       AFTER RELOCATION:L_OP	Rx,.D+offset(Rt)

	       TO:	(1)	ADDIS	Rx,.Du(Rt)
			(2)	LWZ	Rx,.Dl(Rx)
			(3)	L_OP	Rx,offset(Rx) */

	    /* Instruction 1:	ADDIS	Rx,.Du(Rt) */
	    D_FORM(temp, OP_ADDIS, original.dform.rt, toc_reg, high_order_bits);
	    WRITE_INSTRUCTION(1, temp);

	    /* Instruction 2:	LWZ	Rx,.Dl(Rx) */
	    D_FORM(temp, OP_LWZ,original.dform.rt,original.dform.rt,new_addr);
	    WRITE_INSTRUCTION(2, temp);

	    /* Instruction 3:	L_OP	Rx,offset(Rx) */
	    original.dform.ra = original.dform.rt;
	    original.dform.d = offset;
	    WRITE_INSTRUCTION(3, original);

	    cur_fixup_ptr += 12;	/* 3 instructions generated. */
	    break;
	}
	break;				/* Case D_LOAD */

      case D_NOLOAD:
	switch(imported_tocdata + ovfl) {
	  case 0:
	    internal_error();
	    break;

	  case 1:			/* TOC overflow */
	  use_tocreg:

	    DEBUG_MSG(FIXUP_DEBUG, (SAY_NO_NLS, "do_fixup: S-O (Case 4)"));

	    /* TRANSFORMATION:	OP	Rx,.D+offset(Rt)

	       TO:	(1)	ADDIS	Rt,.Du(Rt)
			(2)	OP	Rx,.Dl+offset(Rt)
			(3)	ADDIS	Rt,-.Du(Rt) */

	    /* Instruction 1:	ADDIS	Rt,.Du(Rt) */
	    D_FORM(temp, OP_ADDIS, toc_reg, toc_reg, high_order_bits);
	    WRITE_INSTRUCTION(1, temp);

	    /* Instruction 2:	OP	Rx,.Dl+offset(Rt) */
	    WRITE_INSTRUCTION(2, original);

	    /* Instruction 3:	ADDIS	Rt,-.Du(Rt) */
	    temp.dform.d = -high_order_bits;
	    WRITE_INSTRUCTION(3, temp);

	    cur_fixup_ptr += 12;		/* 3 instructions generated. */
	    break;

	  case 2:			/* Data-in-TOC */
	  use_work_area:

	    DEBUG_MSG(FIXUP_DEBUG, (SAY_NO_NLS, "do_fixup: SD- (Case 5)"));

	    for (rwork = 3;
		 original.dform.ra != rwork && original.dform.rt == rwork;
		 rwork++)
		/* Skip */;

	    /* TRANSFORM:	OP	Rx,D+offset(Rt)
	       AFTER RELOCATION:OP	Rx,.D+offset(Rt)

	       TO:	(1)	STW	Rw,16(R_DSA)
			(2)	LWZ	Rw,.D(Rt)
			(3)	OP	Rx,offset(Rw)
			(4)	LWZ	Rw,16(R_DSA) */

	    /* Instruction 1:	STW	Rw,16(RDSA) */
	    work.dform.opcode = OP_STW;
	    work.dform.rt = rwork;
	    WRITE_INSTRUCTION(1, work);

	    /* Instruction 2:	LWZ	Rw,.D(Rt) */
	    D_FORM(temp, OP_LWZ, rwork, toc_reg, new_addr);
	    WRITE_INSTRUCTION(2, temp);

	    /* Instruction 3:	OP	Rx,offset(Rw) */
	    original.dform.d = offset;
	    original.dform.ra = rwork;
	    WRITE_INSTRUCTION(3, original);

	    /* Instruction 4:	LWZ	Rw,16(RDSA) */
	    work.dform.opcode = OP_LWZ;
	    WRITE_INSTRUCTION(4, work);

	    cur_fixup_ptr += 16;	/* 4 instructions generated. */
	    break;

	  case 3:			/* Data-in-TOC and TOC overflow */
	  use_work_area2:
	    DEBUG_MSG(FIXUP_DEBUG, (SAY_NO_NLS, "do_fixup: SDO (Case 6)"));

	    for (rwork = 3;
		 original.dform.ra != rwork && original.dform.rt == rwork;
		 rwork++)
		/* Skip */;

	    /* TRANSFORM:	OP	Rx,D+offset(Rt)
	       AFTER RELOCATION:OP	Rx,.D+offset(Rt)

	       TO:	(1)	STW	Rw,16(R_DSA)
			(2)	ADDIS	Rw,.Du(Rt)
			(3)	LWZ	Rw,.Dl(Rw)
			(4)	OP	Rx,offset(Rw)
			(5)	LWZ	Rw,16(R_DSA) */

	    /* Instruction 1:	STW	Rw,16(RDSA) */
	    work.dform.opcode = OP_STW;
	    work.dform.rt = rwork;
	    WRITE_INSTRUCTION(1, work);

	    /* Instruction 2:	ADDIS	Rw,.Du(Rt) */
	    D_FORM(temp, OP_ADDIS, rwork, toc_reg, high_order_bits);
	    WRITE_INSTRUCTION(2, temp);

	    /* Instruction 3:	LWZ	Rw,.Dl(Rt) */
	    D_FORM(temp, OP_LWZ, rwork, rwork, new_addr);
	    WRITE_INSTRUCTION(3, temp);

	    /* Instruction 4:	OP	Rx,offset(Rw) */
	    original.dform.d = offset;
	    original.dform.ra = rwork;
	    WRITE_INSTRUCTION(4, original);

	    /* Instruction 5:	LWZ	Rw,16(RDSA) */
	    work.dform.opcode = OP_LWZ;
	    WRITE_INSTRUCTION(5, work);

	    cur_fixup_ptr += 20;	/* 5 instructions generated. */
	}
	break;				/* case D_NOLOAD */

      default:
	internal_error();
    }

    /* Generate return branch */
    b_offset = (off - 2) + 4 - cur_fixup_ptr;

#ifdef DEBUG
    if ((b_offset & 3) || b_offset > 0)
	internal_error();
#endif
    if (b_offset < REAL_MIN_BRANCH_OFFSET)
	return 1;			/* Failure */
    branch.branch.li = b_offset/4;
    memcpy((void *)(ptr + cur_fixup_ptr), &branch, 4); /* Copy branch back */
    cur_fixup_ptr += 4;

    return 0;				/* Success */
} /* do_fixup */
