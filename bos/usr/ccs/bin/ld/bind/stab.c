#if lint == 0 && CENTERLINE == 0
#pragma comment (copyright, "@(#)05	1.13  src/bos/usr/ccs/bin/ld/bind/stab.c, cmdld, bos41B, 9505A 1/20/95 13:21:51")
#endif

#define STAB_COMPACTS
/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS: init_stabs
 *		finish_stab_section
 *		put_debug_name
 *		save_unique_stabstring
 *		stab_stats (ifdef STATS only)
 *
 *   STATIC FUNCTIONS:
 *		new_stab
 *		free_stabs
 *		save_stabstring
 *		stab_hash
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ltypes.h>

#include "global.h"
#include "bind.h"
#include "error.h"
#include "stab.h"
#include "strs.h"
#include "util.h"
#include "stats.h"
#include "objects.h"

#define STABS_BUMP		500
#define STABSTR_HASH_TABLE_SIZE	2039
#define STAB_HASH_TABLE_SIZE	2039
#define SPAN			7
#define SPAN_START		20
#define HASHCONS		65599

/* Local typedefs (for static variables) */
/* Structure for saving 
   1) Unique stabstrings in .debug section.
   2) Unique subtype definitions seen in this bind.
   3) Deferred stabstrings--those with references to undefined subtypes.
*/
typedef struct stab STAB;
struct stab {
    STAB	*stab_next;
    char	*stabstring;		/* Stabstring--could point to actual
					   string or it could point to parsed,
					   fragmented definition. */
    /* NOTE:  The length fields don't have to be part of the union
       alternatives, but structure padding is avoided by putting them
       in the union. */
    union {
	struct {
	    uint16	_length;	/* Length of fragmented definition. */
#define deferred_length u._deferred._length
	    uint16	namelen;	/* Length of name for C_DECL stab */
	    SYMENT	*syment;	/* Symbol table entry in output file */
	} _deferred;
	struct {
	    uint16	_length;	/* Length of *stabstring, not counting
					   terminating null. */
#define subtype_length u._subtype._length
	    uint16	global_type;	/* Global mapping number for saved,
					   unnamed subtype. */
	    struct type_names *names;	/* Chain of names for this type, with
					   a global type for each name. */
	} _subtype;
	struct {
	    uint16	_length;	/* Length of *stabstring, not counting
					   terminating null. */
#define stab_length u._unique._length

#ifdef USE_EXPLICIT_STABSTR_OFFSET
	    /* The offset in the debug section for a saved stabstring can
	       be computed from its stabstring pointer, since a mapped
	       output file is being used.  If stabstrings are saved
	       elsewhere, the _offset field will have to be used. */
	    off_t	_offset;	/* Offset in debug sect (for output) */
#define stabstr_offset(s) ((*s).u._unique._offset)
#else
#define stabstr_offset(s) ((*s).stabstring - debug_base)
#endif
	} _unique;
    } u;
};

struct type_names {
    char		*name;
    uint16		name_len;
    uint16		stab_global_type;
    struct type_names	*chain;
};

/* Global variables for error messages */
OBJECT	*stab_obj;
#ifdef DEBUG
SRCFILE	*stab_sf;
#endif

/* Static variables */
/* Hash table for unique subtypes (if state.stabcmpct >= STABCMPCT_RENUM) */
static STAB	*stabs_types_hash[STAB_HASH_TABLE_SIZE];
/* Hash table for unique stabstrings saved in .debug section. */
static STAB	*stabstr_hash[STABSTR_HASH_TABLE_SIZE];
static STAB	*free_stab_list;
static caddr_t	debug_base;		/* Pointer into mapped output file
					   to beginning of .debug section. */
static caddr_t	stab_mem_ptr;		/* Pointer into mapped output file
					   to current position in .debug
					   section. */
static X_OFF_T	current_stab_offset = 2; /* Pointers are to string, not its
					    length field, so first valid
					    stabstr section offset is 2.
					    This value is the current
					    .debug-section-relative offset
					    */

#ifdef STAB_COMPACTS
/* Static variables needed for compaction */
static uint32	old_symtab_index;	/* For error messages */
static uint32	*old_symtab_index_p;	/* For error messages */
static int	stab_error_occurred = 0; /* To suppress multiple error messages
					    for a single stabstring. */
static STAB	*current_stab = NULL;

static int forward_ref_seen;		/* Set when an undefined subtype is
					   seen while parsing.  If a stabstring
					   contains nested type definitions,
					   this variable is only valid for
					   the current nesting level.  */
static int type_being_defined;		/* The type whose definition we are
					   parsing.  That is, we have seen:
					   <n>=... */

#ifdef DEBUG
static int unmapped_type;
static int successful_count, deferred_count;
static int last_tbd = 0;
#define conditional_NL() {if(nl_needed){say(SAY_NL_ONLY);nl_needed=0;}}
static int	nl_needed = 0;
#endif /* DEBUG */

static char is_digits_array[256];
#define L_ISDIGIT 1
#define L_ISXDIGIT 2
#define L_ISSPACE 4
#define L_ISNOTNAME 8
#define l_isdigit(c) (is_digits_array[c]&L_ISDIGIT)
#define l_isxdigit(c) (is_digits_array[c]&L_ISXDIGIT)
#define l_isspace(c) (is_digits_array[c]&L_ISSPACE)
#define l_isname(c) (!(is_digits_array[c]&L_ISNOTNAME))

static STAB	*deferred_stabs = NULL; /* Root of list of deferred stabs */
static char	*g_input;		/* Input stab (being parsed) */
static char	*output;		/* Output stab (with mapped numbers) */
static char	*frag_base;
static char	*max_input;
static int	stabstring_name_len;

/* Defines (needed for compacting stabstrings */
#define convert2(x) (*(short *)(x))
#define write2(x,v) (*(short *)(x)=v)

/* Forward declaration */
static char *parse_stabstring(char *);

#else /* STAB_COMPACTS */
/* Dummy definitions */
void process_deferred_stabs(void){};
void reset_stab_mappings(void){};

#endif

/************************************************************************
 * Name: init_stabs
 *									*
 * Purpose: Initialize is_digits_array. Initialize file pointer to
 *	debug section in mapped output file.
 *									*
 ************************************************************************/
void
init_stabs(X_OFF_T output_offset,	/* File offset to debug section. */
	   caddr_t base)		/* Mapped address of output file  */
{
#ifdef STAB_COMPACTS
    /* Initialize character-checking array for stab compaction. */
    if (Switches.stabcmpct >= STABCMPCT_NODUPS) {
	int i;

	for (i = '0'; i <= '9'; i++)
	    is_digits_array[i] = L_ISDIGIT | L_ISXDIGIT;

	for (i = 'A'; i <= 'F'; i++) {
	    /* NOTE:  The stabstring grammar requires that letters in hex
	       numbers be upper case. */
	    is_digits_array[i] = L_ISXDIGIT;
	}
	is_digits_array['\t'] = L_ISSPACE;
	is_digits_array['\n'] = L_ISSPACE;
	is_digits_array['\v'] = L_ISSPACE;
	is_digits_array['\f'] = L_ISSPACE;
	is_digits_array['\r'] = L_ISSPACE;
	is_digits_array[' ']  = L_ISSPACE;

	is_digits_array[';'] = L_ISNOTNAME;
	is_digits_array[':'] = L_ISNOTNAME;
	is_digits_array[','] = L_ISNOTNAME;
	is_digits_array['"'] = L_ISNOTNAME;
	is_digits_array['\0'] = L_ISNOTNAME;
    }
#endif
    /* Align .debug section on halfword boundary */
    stab_mem_ptr = base + ROUND(output_offset, 2);
    debug_base = stab_mem_ptr;
}
/************************************************************************
 * Name: finish_stab_section
 *									*
 * Purpose: Write the .debug section header if any long stabstrings
 *		were written.
 *
 * Returns:	offset to next available byte in output file.
 *		If a debug section header is written, *next_scn_ptr is
 *		incremented.
 ************************************************************************/
X_OFF_T
finish_stab_section(X_OFF_T output_offset, /* Output file
					      offset to .debug section. */
		    SCNHDR *sect_hdr,	/* Section header array. */
		    uint16 *next_scn_ptr)
{
    const X_OFF_T	debug_section_length = current_stab_offset - 2;

    if (debug_section_length == 0)
	return output_offset;
    sect_hdr += *next_scn_ptr;
    (*next_scn_ptr)++;

    (void) strncpy(sect_hdr->s_name, _DEBUG, sizeof(sect_hdr->s_name));
    sect_hdr->s_paddr = sect_hdr->s_vaddr = 0;
    sect_hdr->s_size = debug_section_length;
    sect_hdr->s_scnptr = output_offset;
    sect_hdr->s_relptr = sect_hdr->s_lnnoptr = 0;
    sect_hdr->s_nreloc = sect_hdr->s_nlnno = 0;
    sect_hdr->s_flags = STYP_DEBUG;

    return output_offset + debug_section_length; /* Compute offset to next free
						    byte in output file. */
}
/************************************************************************
 * Name: new_stab
 *									*
 * Purpose: Allocate and return a new, uninitialized stab structure
 *
 * Note:  Because STABs can be freed, a free list is checked first.
 ************************************************************************/
static STAB *
new_stab(void)
{
    char	*id = "new_stab";
    STAB	*temp_stab;
    static int	stabs_left = 0;
    static STAB	*available_stabs;

    if (free_stab_list) {
	temp_stab = free_stab_list;
	free_stab_list = free_stab_list->stab_next;
    }
    else {
	if (stabs_left == 0) {
	    available_stabs = get_memory(sizeof(STAB),STABS_BUMP,STABS_ID,id);
	    stabs_left = STABS_BUMP;
	}
	temp_stab = available_stabs++;
	stabs_left--;
    }
    STAT_use(STABS_ID, 1);
    return temp_stab;
}
/************************************************************************
 * Name: free_stabs
 *									*
 * Purpose: Frees a list of stab structures by prepending the list to
 *	the free list.
 *									*
 ************************************************************************/
static void
free_stabs(STAB *list_head,		/* Head of list to be freed. */
	   STAB *list_tail,		/* Tail of list to be freed. */
	   int count)			/* Number of elements in list. */
{
    list_tail->stab_next = free_stab_list;
    free_stab_list = list_head;
    
    STAT_free(STABS_ID, count);
}
/************************************************************************
 * Name: stab_hash
 *									*
 * Purpose: Compute hash value for a hash string.
 *									*
 * Returns:	hash_value
 *
 ************************************************************************/
static int
stab_hash(char *strng,			/* string to be hashed */
	  int len,			/* length of string */
	  int table_size)		/* size of hash table */
{
    unsigned int sum = 0;

    if (len <= SPAN_START)
	return hash(strng, len, table_size); /* Not too long */

    while (len > 4) {
	len -= 4;
	sum = (sum * HASHCONS) + *((unsigned int *)strng);
	strng += 4;
    }
    while (len > 0) {
	len --;
	sum = (sum * HASHCONS) + (unsigned int) *strng;
	strng ++;
    }
    return sum % table_size;
}
/************************************************************************
 * Name: save_stabstring
 *									*
 * Purpose: Save a stabstring in the .debug section.  The length of
 *	the stabstring is written in the first two bytes, followed
 *	by the string itself.  The string must be null-terminated to
 *	satisfy XCOFF requirements; the caller must ensure that the null
 *	byte is there.
 *
 * Note:  Stabstrings can contain null characters, so memcpy must be used
 *	to copy the string.
 *
 * Returns: The address of the saved string (not its length field)
 *
 ************************************************************************/
static char *
save_stabstring(char *stab,
		uint16 stab_len)	/* Length (including null) */
{
    char	*r;

    r = (char *)stab_mem_ptr;
    stab_mem_ptr += stab_len + 2;	/* Update file pointer. */

    memcpy(r, &stab_len, 2);		/* Copy length */

    /* The return value is address of stab string, not its length */
    r += 2;
    memcpy(r, stab, stab_len);		/* Copy terminating null, too */
    return r;
}
/************************************************************************
 * Name: save_unique_stabstring
 *									*
 * Purpose:  Save an instance of a stabstring for comparisons
 *									*
 * Parameters:
 *	flags:	If SUS_TO_DEBUG bit is set, a new stabstring will be
 *		copied to the .debug section.  Otherwise, the new string
 *		is already in the .debug section, and stab_mem_ptr points
 *		to it.
 *		If SUS_IS_DELETABLE is set, we return -1 if the stabstring
 *		is a duplicate.  Otherwise, we return 0 for duplicates.
 * RETURNS:
 *	-1: Symbol table entry should be deleted (only if STABCMPCT_NODUPS
 *			is defined).
 *	 0: Symbol table entry needed, but stabstring fits in symbol itself,
 *		or it was a duplicate and n_offset field has been filled in.
 *	 1: Symbol table entry needed.  Stabstring is new.
 ************************************************************************/
int
save_unique_stabstring(SYMENT *syment,	/* Symbol table entry for symbol */
		       char *stab_str,	/* Must be null-terminated */
		       int stab_len,	/* Length without terminating '\0' */
		       int flags)
{
    int		h;
    STAB	*s;

    if (stab_len <= SYMNMLEN
#ifdef STABCMPCT_NODUPSYMS
	&& !(flags & SUS_IS_DELETABLE)
#endif
	) {
	/* Short stab strings go in the symbol itself, except that if we're
	   eliminating duplicate C_DECLs, we check for duplicates first. */

	/* We don't want a terminating null here. */
	memset(syment->n_name, 0, SYMNMLEN);
	memcpy(syment->n_name, stab_str, stab_len);
	return 0;
    }

    /* Stabstring won't fit in symbol, or we're checking C_DECL for dups. */
    syment->n_zeroes = 0;

    h = stab_hash(stab_str, stab_len, STABSTR_HASH_TABLE_SIZE);
    for (s = stabstr_hash[h]; s; s = s->stab_next) {
	if (s->stab_length == stab_len
	    && memcmp(stab_str, s->stabstring, stab_len) == 0) {
	    /* We found a duplicate */
#ifdef STABCMPCT_NODUPSYMS
	    /* If we're doing an unnamed C_DECL, we delete it. */
	    if (flags & SUS_IS_DELETABLE)
		return -1;
	    if (stab_len <= SYMNMLEN) {
		/* Symbol fits in name--we don't need offset */
		memset(syment->n_name, 0, SYMNMLEN);
		memcpy(syment->n_name, stab_str, stab_len);
	    }
	    else
#endif
		syment->n_offset = stabstr_offset(s);
	    return 0;			/* Duplicate stabstring*/
	}
    }

    /* New stab */
    s = new_stab();
    s->stab_next = stabstr_hash[h];	/* Add to chain for bucket */
    stabstr_hash[h] = s;
    s->stab_length = stab_len;
#ifdef STABCMPCT_NODUPSYMS
    if (stab_len <= SYMNMLEN) {
	/* This should only happen if we're deleting duplicate C_DECLs */
	/* Save string in the string table--not in the working debug section,
	   because it will never be part of the .debug section.  We just need
	   to save it for comparison purposes. */
#ifdef DEBUG
	/* Terminating '\0' doesn't need to be saved, but it's nice to have
	   it there for debugging. The stabstring must be null-terminated, so
	   the null is simply copied. */
	s->stabstring = save_string(stab_str, stab_len+1);
#else
	s->stabstring = save_string(stab_str, stab_len);
#endif
	memset(syment->n_name, 0, SYMNMLEN);
	memcpy(syment->n_name, stab_str, stab_len);
#ifdef DEBUG
	s->stabstr_offset = -1;
#endif
	return 0;
    }
#endif /* STABCMPCT_NODUPS */

#ifdef USE_EXPLICIT_STABSTR_OFFSET
    s->stabstr_offset = current_stab_offset;
#endif
    syment->n_offset = current_stab_offset;
    if (flags & SUS_TO_DEBUG)
	s->stabstring = save_stabstring(stab_str, stab_len + 1);
    else
	s->stabstring = stab_mem_ptr + 2; /* Already in .debug sect. */
    current_stab_offset += stab_len
	+ 1				/* Add 1 for terminating '\0' */
	    + 2				/* and 2 for length field.*/;
    return 1;
} /* save_unique_stabstring */
/************************************************************************
 * Name: put_debug_name
 *									*
 * Purpose:	Take a stabstring, process it if necessary, and save it
 *		in the .debug section if necessary.  Fill in the offset for
 *		the new symbol table entry.
 *									*
 * Parameters:
 *	stab_len:	Length of stabstring (not counting terminating '\0')
 *
 * RETURNS:	1, if symbol table entry is used.
 *		0, if symbol table entry can be overwritten
 *
 ************************************************************************/
int
put_debug_name(SYMENT *syment,		/* output symbol */
	       char *stab_str,		/* Stabstring--it must be
					   null-terminated. */
	       uint16 stab_len,		/* Length (not including '\0')  */
	       uint32 *old_symtab_indx_p, /* For continued stabstrings */
	       uint32 old_symtab_indx	/* For error messages */
#ifdef STABCMPCT_NODUPSYMS
	       , int is_deletable	/* If 1, this is unnamed C_DECL,
					   which can be deleted. */
#endif
)
{
    int		h;
    char	*stabstring_base;
    int		frag_len;

#ifdef DEBUG
    static int total_parsings = 0;
#endif

    DEBUG_MSG(STAB_DEBUG|DEBUG_LONG, (SAY_NO_NLS, "Stab: %s", stab_str));

    switch(Switches.stabcmpct) {
      case STABCMPCT_NONE:
	/* Stabs go directly to output, without checking for duplicates.
	   The stabstring must have been in a debug section (or this function
	   is not called), so stab_str is already null-terminated, and its
	   terminating null gets copied when save_stabstring() is called. */
	(void) save_stabstring(stab_str, ++stab_len); /* Add 1 for '\0' */
	syment->n_offset = current_stab_offset;	/* Offset is to stabstring,
						   not length */
	/* Include 2 for length field */
	current_stab_offset += stab_len + 2; /* Get offset to next string */
	return 1;			/* Symbol table entry used */

      case STABCMPCT_NODUPS:
	/* No parsing needed--check for dups */
	h = save_unique_stabstring(syment, stab_str, stab_len, SUS_TO_DEBUG);

#if STABCMPCT_NODUPSYMS || DEBUG
	switch(h) {
#ifdef STABCMPCT_NODUPSYMS
	  case -1:			/* Duplicate stabstring. */
	    return 0;			/* No symbol table entries used. */
#endif
#ifdef DEBUG
	  case 1:
	    DEBUG_MSG(STAB_DEBUG|DEBUG_LONG,
		      (SAY_NO_NLS, "current_stab_offset = %d",
		       current_stab_offset - (stab_len + 3)));
	    break;
	  case 0:
	    DEBUG_MSG(STAB_DEBUG,
		      (SAY_NO_NLS, "Duplicate stabstring--offset = %d",
		       syment->n_offset));
	    break;
#endif
	}
#endif
	return 1;			/* Symbol table entry used. */

      default:
	break;
    }

#ifdef STAB_COMPACTS
    /* Save parameters in static variables for error messages. */
    old_symtab_index = old_symtab_indx;
    old_symtab_index_p = old_symtab_indx_p;

    /* Allocate structure ahead of time so its address can be saved in
       local_mapping[] if necessary. */
    if (current_stab == NULL)
	current_stab = new_stab();

    g_input = stab_str;			/* Set input pointer for parsing. */
    max_input = &g_input[stab_len];	/* Guard against overrun */
    forward_ref_seen = 0;		/* Initialize */

    /* When we parse the stabstring, we copy the parsed output directly to
       the .debug section, reserving 2 bytes for the length field.  If there
       are no forward references in the stabstring, the stabstring won't have
       to be copied because it is already in the .debug section.  Otherwise,
       the partial stabstring is saved for later processing. */
    frag_base				/* Base for current fragment.  A
					   fragment extends until a new
					   subtype definition is seen, or
					   until a reference to an
					   undefined subtype is seen. */
	= stabstring_base		/* Base pointer for this stabstring--
					   never updated, even if nested
					   typedefs or undefined subtypes
					   are seen. */
	    = output			/* Output pointer for parsing. */
		= stab_mem_ptr + 2;	/* Leave 2 bytes for length--
					   stab_mem_ptr is not updated
					   unless a complete stabstring can be
					   written. */

#ifdef DEBUG
    unmapped_type = 0;
    type_being_defined = --total_parsings;
#else
    type_being_defined = 0;		/* Used for error messages.  0 means
					   no current subtype.  */
#endif

    stab_error_occurred = 0;		/* Reset */
    g_input = parse_stabstring(g_input);
    *output = '\0';			/* Terminate stabstring */

#ifdef DEBUG
    if (g_input > max_input)
	internal_error();		/* Overrun */
#endif

    if (!forward_ref_seen) {
	/* The parse was successful: stab_mem_ptr now points to an updated
	   stabstring, with all referenced subtypes remapped. */
#ifdef DEBUG
	successful_count++;
#endif
	stab_len = output - stabstring_base; /* Don't count final null. */
	/* When a stabstring is written to the .debug section, the terminating
	   null byte must be included in the length. */
	write2(&stabstring_base[-2], stab_len+1);

	switch(save_unique_stabstring(syment, stabstring_base, stab_len,
#ifdef STABCMPCT_NODUPSYMS
				      is_deletable ? SUS_IS_DELETABLE :
#endif
				      0
				      )) {
	  case -1:			/* Duplicate C_DECL stabstring. */
	    return 0;			/* No symbol table entries used. */

	  case 0:
#ifdef DEBUG
	    if (bind_debug & STAB_DEBUG)
		if (stab_len > SYMNMLEN)
		    say(SAY_NO_NLS, "Compacting duplicate stabstring");
#endif
	    break;
	  case 1:
	    stab_mem_ptr = output + 1;	/* Point past terminating null
					   of current stabstring. */
	    break;
	}

	DEBUG_MSG(STAB_DEBUG|DEBUG_LONG,
		  (SAY_NO_NLS, "current_stab_offset = %d",
		   current_stab_offset));
	return 1;			/* Symbol table entry used. */
    }

    frag_len = output - frag_base;	/* Compute length of last fragment */

    /* The stabstring contained a reference to a type that has not been
       completely defined yet.  We must save the stabstring for later
       processing. */
    /* Write the length of the last fragment */
    write2(&frag_base[-2], frag_len);

    /* Compute length of stabstring to save.  Include 2 bytes for the
       initial length field. */
    stab_len = output - stabstring_base + 2;
    /* Now we must save the stabstring for later processing, when
       all subtypes will be mapped. */
#ifdef DEBUG
    ++deferred_count;
    if (bind_debug & STAB_DEBUG) {
	conditional_NL();
	if (!(bind_debug & DEBUG_LONG)) {
	    say(SAY_NO_NLS | SAY_NO_NL, "Stab/w frefs (to %d): ",
		unmapped_type);		/* More forward refs could exist. */
	    say(SAY_NO_NLS, "%-.50s", stab_str);
	}
    }
#endif	    
    current_stab->stab_next = deferred_stabs;
    deferred_stabs = current_stab;
    current_stab->u._deferred.syment = syment; /* Symbol for back patching */
    current_stab->u._deferred.namelen = stabstring_name_len;
    syment->n_offset = old_symtab_index; /* Save for error messages when
					    processing deferred stabs. */

    /* We must copy the fragmented stabstring from the .debug section
       to working memory. We save a terminating null, so add_type_defs()
       can use sprintf() when updating the stabstring in-place later on. */
    current_stab->stabstring = save_string(stab_mem_ptr, stab_len+1);
    current_stab->deferred_length = stab_len;
    current_stab = NULL;		/* Force allocation next time. */
    return 1;				/* We have to keep the symbol */
#else
    internal_error();
    return 1;
#endif /* STAB_COMPACTS */
} /* put_debug_name */
#ifdef STATS
#define MAX_CHAIN_EXAMINED 40
/************************************************************************
 * Name: stab_stats
 *									*
 * Purpose: Print information about effectiveness of
 *		stabstring hashing function
 *									*
 ************************************************************************/
void
stab_stats(void)
{
    int		h_used = 0;
    int		max_chain_length = 0;
    int		named_subtypes;
    int		max_named_subtypes;
    int		subtype_chains;
    int		total_subtype_chain_len;
    int		chain_lengths[MAX_CHAIN_EXAMINED];
    int		h_syms = 0, hk;
    int		i, j;
    float	h_ratio,        /* ratio actual/random */
		h_randist,      /* random distribution value */
		h_actdist,      /* actual distribution value */
		hkf,
		h_fsyms;
    STAB	*s;
    STAB	*s1;

    /* Stabstrings hash table */
    for (i = 0; i < MAX_CHAIN_EXAMINED; i++)
	chain_lengths[i] = 0;

    /* Hashing function diagnostic information */
    h_actdist = 0.0;
    for (i=0, h_syms=0, h_used=0; i < STABSTR_HASH_TABLE_SIZE; ++i) {
	if (stabstr_hash[i])
	    h_used++;
	/* calculate distribution of hash entries */
	hk = 0;
	for (s=stabstr_hash[i]; s; s = s->stab_next) {
	    ++hk;
	    ++h_syms;
	}
	if (hk >= MAX_CHAIN_EXAMINED)
	    ++chain_lengths[MAX_CHAIN_EXAMINED-1];
	else
	    ++chain_lengths[hk];
	max_chain_length = max(max_chain_length, hk);
	hkf = (float)hk;
	h_actdist = h_actdist + hkf * (hkf+1.0)/2.0;
    }

    say(SAY_NO_NLS | SAY_NL_ONLY);
    if (h_syms == 0)
	say(SAY_NORMAL, NLSMSG(STAB_NO_STATS,
		      "%s: STABSTRING statistics:  No stabstrings."),
	    Command_name);
    else {
	say(SAY_NORMAL, NLSMSG(STAB_STATS_HDR, "%s: STABSTRING statistics:"),
	    Command_name);
	h_fsyms = (float)h_syms;
	h_randist=(h_fsyms/(float)(2 * STABSTR_HASH_TABLE_SIZE))
	    * (h_fsyms+(float)(2 * STABSTR_HASH_TABLE_SIZE - 1));
	h_ratio = h_actdist / h_randist;

	/* Static count information */
	say(SAY_NO_NL, NLSMSG(HASH_INFO,
"\tBuckets used=%1$d of %2$d Maximum-bucket-size=%3$d Strings=%4$d\n"
"\tActual distribution=%5$.1f Random distribution=%6$.1f Ratio=%7$.3f\n"
	"\tBucket sizes: "),
	    h_used, STABSTR_HASH_TABLE_SIZE, max_chain_length, h_syms,
	    h_actdist, h_randist, h_ratio);

	j = min(MAX_CHAIN_EXAMINED-1, max_chain_length);
	hk = 0;
	h_syms = 0;
	for (i = 1; i < j;  i++)
	    say(SAY_NO_NLS | SAY_NO_NL, "%d,", chain_lengths[i]);
	say(SAY_NO_NLS, "%d", chain_lengths[i]);
    }

    /* Parsed stabs stats */
    if (Switches.stabcmpct <= STABCMPCT_NODUPS)
	return;
    for (i = 0; i < MAX_CHAIN_EXAMINED; i++)
	chain_lengths[i] = 0;

    max_chain_length = 0;
    max_named_subtypes = 0;
    total_subtype_chain_len = 0;
    subtype_chains = 0;

    /* Hashing function diagnostic information */
    h_actdist = 0.0;
    for (i=0, h_syms=0, h_used=0; i < STAB_HASH_TABLE_SIZE; ++i) {
	if (stabs_types_hash[i])
	    h_used++;
	/* calculate distribution of hash entries */
	hk = 0;
	s1 = stabs_types_hash[i];
	named_subtypes = 0;
	{
	    struct type_names *t1;
	    for (t1 = s1->u._subtype.names; t1; t1 = t1->chain)
		++named_subtypes;
	}
	if (named_subtypes > 0) {
	    ++subtype_chains;
	    total_subtype_chain_len += named_subtypes;
	    if (max_named_subtypes < named_subtypes)
		max_named_subtypes = named_subtypes;
	}

	for ( ; s1; s1 = s1->stab_next) {
	    ++hk;
	    ++h_syms;
	}
	if (hk >= MAX_CHAIN_EXAMINED)
	    ++chain_lengths[MAX_CHAIN_EXAMINED-1];
	else
	    ++chain_lengths[hk];
	max_chain_length = max(max_chain_length, hk);
	hkf = (float)hk;
	h_actdist = h_actdist + hkf * (hkf+1.0)/2.0;
    }

    if (h_syms == 0)
	return;

    say(SAY_NO_NLS | SAY_NL_ONLY);
    h_fsyms = (float)h_syms;
    h_randist=(h_fsyms/(float)(2 * STAB_HASH_TABLE_SIZE))
	* (h_fsyms+(float)(2 * STAB_HASH_TABLE_SIZE - 1));
    h_ratio = h_actdist / h_randist;

    /* Static count information */
    say(SAY_NO_NL, NLSMSG(HASH_INFO,
"\tBuckets used=%1$d of %2$d Maximum-bucket-size=%3$d Strings=%4$d\n"
"\tActual distribution=%5$.1f Random distribution=%6$.1f Ratio=%7$.3f\n"
			  "\tBucket sizes: "),
	h_used, STAB_HASH_TABLE_SIZE, max_chain_length, h_syms,
	h_actdist, h_randist, h_ratio);

    j = min(MAX_CHAIN_EXAMINED-1, max_chain_length);
    hk = 0;
    h_syms = 0;
    for (i = 1; i < j;  i++)
	say(SAY_NO_NLS | SAY_NO_NL, "%d,", chain_lengths[i]);
    say(SAY_NO_NLS, "%d", chain_lengths[i]);
    say(SAY_NO_NLS,
#ifdef DEBUG
	"\tTotal named subtypes: %d\n"
	"\tMaximum chain length %d\n"
	"\tAverage chain length for named subtypes: %7.3f",
#else
	"\t%d\t%d\t%7.3f",
#endif
	subtype_chains,
	max_named_subtypes,
	(float)total_subtype_chain_len/(float)subtype_chains);
}
#endif

#ifdef STAB_COMPACTS
#include "stab2.c"
#endif
