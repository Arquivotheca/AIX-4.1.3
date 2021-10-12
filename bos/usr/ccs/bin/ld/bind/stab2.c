#if lint == 0 && CENTERLINE == 0
static char sccsid[] = "@(#)11	1.12  src/bos/usr/ccs/bin/ld/bind/stab2.c, cmdld, bos41J, 9512A_all 3/21/95 10:21:27";
/* stab.c already uses #pragma;  It can't be used twice */
#endif
/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS: reset_stab_mappings
 *		process_deferred_stabs
 *
 *   STATIC FUNCTIONS:
 *		add_type_refs
 *		adjust_stab_lengths
 *		compare_subtype_to_string
 *		compare_frags
 *		compare_typedefs
 *		compare_typerefs
 *		copy_char_error
 *		copy_typeid
 *		create_local_mapping
 *		flatten_typedefs
 *		get_local_mapping
 *		parse_stabstring
 *		print_stab
 *		save_unique_subtype
 *		search
 *		stack_types
 *		stabstrings_equiv
 *		stab_error2
 *		stab_sprintf
 *
 *   STATIC PARSING FUNCTIONS
 *		copy_name
 *		copy_past
 *		copy_constant
 *		copy_dbx_string
 *		copy_typedef
 *		copy_enumspec
 *		copy_bound
 *		copy_tparamlist
 *		copy_field
 *		copy_fieldlist
 *		copy_variantpart
 *		copy_conditions
 *		copy_usage
 *		copy_namedtparamlist
 *		copy_optbasespeclist
 *		copy_extendedfieldlist
 *		copy_nameresolutionlist
 *		copy_real
 *		copy_int
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

/* A placeholder can also have a 'c' or 'u' where the 's' occurs. */
#define STAB_PLACEHOLDER "=Y0s(;"

static char char_as_string[2] = {'\0', '\0'};

/* Static variables for  STABCMPCT_RENUM */
static int	unique_type;
static int	stabstring_name_len;
static int	global_mapping = 2;	/* Reserve 1 for undefined types */
static int	use_arbitrary_mapping = 0; /* 1 if incomplete types should
					      be assigned mappings anyway */
static int	num_deferred_types;	/* Number of forward refs.  */

typedef struct local_mapping {
    union {
	STAB	*_incomplete_stab;	/* Pointer to STAB containing
					   incomplete definition
					   (if def_offset >= 0)  */
	char	*_stab_def;		/* Pointer to complete definition
					   for type.
					   (if def_offset < 0) */
    } u;
#define incomplete_stab u._incomplete_stab
#define stab_def u._stab_def
    int		def_offset;		/* If >= 0, offset withing
					   parsed_stab to definition.
					   If < 0, -def_offset is length of
					   completes stabstring */
    int		global;			/* Mapped-to type number */
    off_t	input_offset;		/* Offset in debug section of input
					   file of stabstring defining this
					   type (at outer level). We can avoid
					   parsing a stabstring at all if it
					   has the same offset as another
					   stabstring. */
    char	defined;
    char	needed;
    char	named;			/* If local_type is a named type,
					   set to 1.  */
    char	placeholder;
#ifdef DEBUG
    char	arbitrary;		/* Set if type is arbitrary or depends
					   on arbitrary type. */
    short	depends_on_count;	/* Count of dependencies */
    short	depends_on;		/* Type number that this type depends
					   on.  If global <= 0 and depends_on
					   > 0, it is valid. */
#endif
} LOCAL_MAPPING;

struct {
    int limit;
    LOCAL_MAPPING m[4096];
} *local_mapping_base[16];

#define local_mapping(n) (local_mapping_base[n>>12]->m[n&0xFFF])

#ifdef DEBUG
LOCAL_MAPPING *lm;
#endif

static STAB	*stabs_types_hash[STAB_HASH_TABLE_SIZE];

/* Forward declarations */
#ifdef DEBUG
static char *print_stab(char *s, int len, int depth);
#endif
static char *compare_subtype_to_string(char *, char *, char *, char *);

#define chkcont(i,mi,osi) \
	((i==mi-1&&(*i=='?'||*i=='\\'))?i=do_continuation(i,&osi,&mi):i)

/* Define macros for copying single characters. */
#define copy_char(i,c,prod) \
	(((*i)==c) ? (*output++=c,++i) : copy_char_error(i,c,prod))
#define copy_cur_char() ((input<max_input)?(*output++=*input++):0)
#define safe_copy_cur_char() *output++=*input++

/* Forward declarations */
static int create_local_mapping(int, int);
static int get_local_mapping(int);
static char *copy_bound(char *);
static char *copy_char_error(char *, char, char *);
static char *copy_conditions(char *);
static char *copy_constant(char *);
static char *copy_dbx_string(char *);
static char *copy_enumspec(char *);
static char *copy_extendedfieldlist(char *);
static char *copy_field(char *);
static char *copy_fieldlist(char *);
static char *copy_int(char *);
static char *copy_name(char *);
static char *copy_namedtparamlist(char *);
static char *copy_nameresolutionlist(char *);
static char *copy_optbasespeclist(char *);
static char *copy_past(char *, char);
static char *copy_real(char *);
static char *copy_tparamlist(char *);
static char *copy_typedef(char *);
static char *copy_typeid(char *, char *);
static char *copy_usage(char *);
static char *copy_variantpart(char *);

/************************************************************************
 * Name: stab_sprintf
 *									*
 * Purpose: Provide a limited-function sprintf() to avoid the overhead of
 *	the real sprintf().  The format string can only be "%d" or "%d=".
 *	The terminating null is not added.
 *									*
 ************************************************************************/
#ifdef USE_SPRINTF
#define stab_sprintf sprintf
#else
static int
stab_sprintf(char *dest,
	     char *format,
	     int arg)
{
    char buffer[10];
    int i;
    char *save_dest = dest;

    if (arg == 0) {
	*dest++ = '0';
    }
    else {
	if (arg < 0) {
	    *dest++ = '-';
	    arg = -arg;
	}
	i = 0;
	while (arg > 0) {
	    buffer[i++] = arg % 10 + '0';
	    arg /= 10;
	}
	while (i-- > 0)
	    *dest++ = buffer[i];
    }
    if (format[2] == '=')
	*dest++ = '=';
    return dest - save_dest;
}
#endif
/************************************************************************
 * Name: save_unique_subtype
 *									*
 * Purpose: See if the stabstring has been seen before.  If so, return
 *	its global mapped-to type.  Otherwise, save the string in a
 *	hash table and return a new global number.
 *									*
 ************************************************************************/
static int
save_unique_subtype(int local_type,	/* Type number in current C_FILE. */
		    char *stab_str,	/* Stabstring */
		    int stab_len,	/* Length (without final '\0') */
		    char *type_name,	/* Name of type (if any). Length of
					   type_name is stabstring_name_len. */
		    STAB ***in_place)	/* See "in_place" optimization. */
{
    static char id[] = "save_unique_subtype";
    int			h;
    int			global_type;
    STAB		*s;
    struct type_names	*s_name;
    uint16		*global_type_ptr;

    /* See if this subtype has been seen in entire link */
    h = stab_hash(stab_str, stab_len, STAB_HASH_TABLE_SIZE);
    for (s = stabs_types_hash[h]; s; s = s->stab_next) {
	if (s->subtype_length == stab_len
	    && memcmp(stab_str, s->stabstring, stab_len) == 0) {
	    /* Repeat occurrence for this subtype.  We won't need to save the
	       subtype definition, so we don't need the in_place pointer. */
	    if (in_place != NULL)
		*in_place = NULL;

	    /* Even though the subtype has already been seen, we may not
	       have inserted the local mapping into the table yet, or it
	       may have pointed to an incomplete stabstring.  In this case,
	       def_offset >= 0, and we must update stab_def to point to
	       the complete definition. */
	    if (local_mapping(local_type).def_offset >= 0) {
		local_mapping(local_type).stab_def = s->stabstring;
		local_mapping(local_type).def_offset = -stab_len;
	    }
	    goto check_name;
	}
    }

    /* New subtype */
    s = new_stab();
    s->stab_next = stabs_types_hash[h];
    stabs_types_hash[h] = s;
    s->u._subtype.global_type = 0;
    s->subtype_length = stab_len;
    s->u._subtype.names = NULL;

    /* New subtype.  If the subtype is being saved in-place,
       adjust the in_place pointer. */
    if (in_place != NULL) {
	s->stabstring = stab_str;
	**in_place = s;
    }
    else {
	/* Save the subtype in the string table.  It won't be written to
	   the output file as a separate entity. */
#ifdef DEBUG
	s->stabstring = save_string(stab_str, stab_len+1);
	s->stabstring[stab_len] = '\0';	/* Add null for dbx */
#else
	s->stabstring = save_string(stab_str, stab_len);
#endif
    }

    local_mapping(local_type).stab_def = s->stabstring;
    local_mapping(local_type).def_offset = -stab_len;

  check_name:
    if (type_name != NULL && stabstring_name_len > 0) {
	local_mapping(local_type).named = 1;
	for (s_name = s->u._subtype.names; s_name; s_name = s_name->chain) {
	    if (s_name->name_len == stabstring_name_len
		&& strncmp(type_name, s_name->name, stabstring_name_len) == 0)
		break;
#ifdef DEBUG1
	    s_name = s_name;
#endif
	}
	if (s_name == NULL) {
	    /* First occurrence of subtype with this name. */
	    s_name = emalloc(sizeof(*s_name), id);
	    s_name->chain = s->u._subtype.names;
	    s_name->stab_global_type = 0;
#ifdef DEBUG
	    s_name->name = save_string(type_name, stabstring_name_len+1);
	    s_name->name[stabstring_name_len] = '\0';
#else
	    s_name->name = save_string(type_name, stabstring_name_len);
#endif
	    s_name->name_len = stabstring_name_len;
	    s->u._subtype.names = s_name;
	}
	global_type_ptr = &s_name->stab_global_type;
    }
    else {
	global_type_ptr = &s->u._subtype.global_type;
	local_mapping(local_type).named = 0;
    }	

    /* Now, *global_type_ptr is the current global_type for this type with
       this name.  If the value is 0, we've never seen this type with this
       name before, so it will get a brand new mapping. The only exception
       is if we're redefining a named type.  In that case, we continue to
       use the old mapping.  */
    global_type = *global_type_ptr;

    if (local_mapping(local_type).global > 0) {
	/* We already had to create an arbitrary mapping for this type,
	   so even though the subtype matches another previously-defined
	   type, we can't use it. */
	global_type = local_mapping(local_type).global;
	if (*global_type_ptr == 0)
	    *global_type_ptr = global_type;
    }
    else {
	/* The underlying type has already been seen, but unnamed types
	   and each unique named type have a different mapping.
	   If there is no current global_type, the value will be 0,
	   and create_local_mapping will generate a new mapping. */
	global_type = create_local_mapping(local_type, global_type);
#ifdef DEBUG
	if (global_type == 0)
	    internal_error();
#endif
	*global_type_ptr = global_type;
    }
    return global_type;
} /* save_unique_subtype */
/************************************************************************
 * Name: adjust_stab_lengths
 *									*
 * Purpose: Update subtype lengths in a stabstring.  This procedure is
 *	called after add_type_refs has updated a stabstring.
 *									*
 ************************************************************************/
static void
adjust_stab_lengths(char *s,		/* Fragmented stabstring to adjust */
		    char *def_begin,	/* Adjustment boundaries */
		    char *def_end,
		    int delta)		/* Adjustment amount */
{
    int frag_len;
    char c;
    int def_len;

    /* Skip over first fragment */
    frag_len = convert2(s);
    s += 2 + frag_len;
    while (s < def_end) {
	c = s[2];
	s += 5;
	if (c != ' ') {
	    if (s == def_begin)
		break;
	    def_len = convert2(&s[-2]);
	    if (s < def_begin && &s[def_len] > def_end) {
		/* This definition encompasses the updated definition.
		   Adjust its length and continue by adjusting included
		   definition. */
		write2(&s[-2], def_len - delta);
	    }
	    else {
		/* This definition is not affected by the updated definition. */
		s += def_len;
	    }
	}
	/* Skip next fragment */
	frag_len = convert2(s);
	s += 2 + frag_len;
    }
} /* adjust_stab_lengths */
/************************************************************************
 * Name: add_type_refs
 *									*
 * Purpose: Process the definition for type 'local_type' by writing its
 *	subtypes (if available) into the incomplete definition.
 *									*
 ************************************************************************/
static int
add_type_refs(int local_type,
	      uint32 symtab_index,	/* For error messages */
	      char *x1)			/* NULL, if this is outer call;
					 pointer to subtype definition,
					 otherwise  */
{
    char	c;
    char	*x;
    char	*dest, *src, *cur_frag_len_field;
    char	*full_stabstring;
    int		def_len, subtype_len, new_subtype_len, frag_len, subtype;
    int		j, stabstring_len;
    int		rc = 0, rc1;
    LOCAL_MAPPING *lmt = &local_mapping(local_type);

    /* If we have nested subtyped definitions, the definition for
       local_type may have been completed by another call to this
       function, so just return in this case. */
    if (lmt->def_offset < 0)
	return 0;

    full_stabstring = lmt->incomplete_stab->stabstring;
    stabstring_len = lmt->incomplete_stab->deferred_length;

    if (x1 == NULL) {
	x = &full_stabstring[lmt->def_offset];
	symtab_index = lmt->incomplete_stab->u._deferred.syment->n_offset;

#ifdef DEBUG
	if (bind_debug & (STAB_DEBUG|DEBUG_LONG) == (STAB_DEBUG|DEBUG_LONG)) {
	    say(SAY_NO_NLS, "------at call to add_type_refs--------");
	    print_stab(full_stabstring, stabstring_len, 1);
	}
#endif
    }
    else
	x = x1;

    subtype_len = convert2(x-2);

    cur_frag_len_field = x;

    /* First fragment doesn't have to be copied */
    frag_len = convert2(x);
    src = x + 2 + frag_len;		/* Skip over frag length field */
    dest = x + 2 + frag_len;
    while (src < &x[subtype_len]) {
	/* Extract the fields from the subtype specifier */
	subtype = convert2(src);
	c = src[2];

	j = get_local_mapping(subtype);
#ifdef DEBUG
	/* This should never happen until we start handling circular types. */
	if (j == 0)
	    internal_error();
#endif
	switch(c) {
	  case '=':
	  case 'r':
	    /* Add references to nested subtype definition. This code can
	       only be executed if a nested subtype had a forward reference. */
	    rc1 = add_type_refs(subtype, symtab_index, &src[5]);
	    subtype_len = convert2(x-2); /* Refetch subtype length--it
					    could have been modified. */
	    rc += rc1;
	    if (rc1 == 0) {
		/* Subtype is completely defined, so we can write its type
		   number and definition */
		/* We must fetch frag_len before using stab_sprintf to write
		   j, because the call could write 7 bytes, if j is a 5-digit
		   number. */
		frag_len = convert2(&src[5]); /* Only 1 fragment in new def. */
		src += 7;
		dest += stab_sprintf(dest, "%d=", j);
		if (dest != src)
		    memmove(dest, src, frag_len);
		src += frag_len;
		dest += frag_len;
	    }
	    else {
		/* Some subtypes still not defined */
		/* Update length field of fragment preceding nested
		   subtype definition. */
		write2(cur_frag_len_field, dest - (cur_frag_len_field + 2));

		/* copy 5-byte subtype reference plus its updated incomplete
		   definition in one copy. */
		def_len = convert2(&src[3]); /* After recursion */
		if (src != dest)
		    memmove(dest, src, 5 + def_len);
		src += 5 + def_len;
		dest += 5 + def_len;
		cur_frag_len_field = dest;
		dest += 2;		/* Skip over next frag length field */
	    }
	    frag_len = convert2(src);
	    break;
	  case 'p':
	    /* The fragment preceding this undefined subtype is now
	       complete--write its new length. */
	    write2(cur_frag_len_field, dest - (cur_frag_len_field+2));
	    /* Placeholders are not updated. */
	    if (src != dest)
		memmove(dest, src, 8);
	    dest += 8;
	    src += 8;
	    break;
	  case ' ':
	    if (j > 0) {
		src += 5;
		/* The following stab_sprintf will write 6 bytes if j >= 10000.
		   Therefore, we must read the next frag_len here. */
		frag_len = convert2(src);
		dest += stab_sprintf(dest, "%d", j);
	    }
	    else {
		rc++;
		/* The fragment preceding this undefined subtype is now
		   complete--write its new length. */
		write2(cur_frag_len_field, dest - (cur_frag_len_field+2));

		if (src != dest)
		    memmove(dest, src, 5);
		src += 5;
		dest += 5;
		cur_frag_len_field = dest;
		dest += 2;
		frag_len = convert2(src);
	    }
	    break;
	}
	src += 2;			/* Skip over frag length field */
	if (frag_len != 0) {
	    if (src != dest)		/* Copy fragment */
		memmove(dest, src, frag_len);
	    src += frag_len;
	    dest += frag_len;
	}
    }
    /* Update last fragment length */
    frag_len = dest - (cur_frag_len_field + 2);
    write2(cur_frag_len_field, frag_len);

    /* Update complete subtype length */
    new_subtype_len = dest - x;
    write2(&x[-2], new_subtype_len);

    /* Since we modified the middle of a stabstring, we must shift
       the remaining bytes in the stabstring. */

#ifdef DEBUG
    if (dest != &full_stabstring[lmt->def_offset + new_subtype_len])
	internal_error();
    if (src != &full_stabstring[lmt->def_offset + subtype_len])
	internal_error();
#endif

    memmove(dest, src, stabstring_len - subtype_len - lmt->def_offset);
    adjust_stab_lengths(full_stabstring,
			x, &x[subtype_len], subtype_len - new_subtype_len);
    lmt->incomplete_stab->deferred_length -= subtype_len - new_subtype_len;

    if (rc == 0) {
	char *sname;

	/* We should have a single fragment, a complete subtype definition. */
	old_symtab_index = symtab_index;
#ifdef DEBUG
	use_arbitrary_mapping = 0;
#endif
	if (x1 == NULL &&
	    (stabstring_name_len
	     = lmt->incomplete_stab->u._deferred.namelen) > 0) {
	    /* Name begins after initial fragment length */
	    sname = &full_stabstring[2];
	}
	else
	    sname = NULL;
	save_unique_subtype(local_type, x+2, frag_len,
			    sname,	/* Type name */
			    NULL	/* No in-place subtype */);
#ifdef DEBUG
	use_arbitrary_mapping = 1;
#endif	
    }

    return rc;
} /* add_type_refs */
/************************************************************************/
struct stack {
    int mark;
    int x;
    int on_stack;
    int lowlink;
    int element;
} *stack;
static int count;
static int stack_ptr;
/************************************************************************
 * Name: search
 *									*
 * Purpose: Find strongly-connected components
 *									*
 ************************************************************************/
static void
search(int i)
{
    int		j, k, se, component_size;
    int		cur_offset, def_len;
    char	*x;

    DEBUG_MSG(STAB_DEBUG|DEBUG_LONG,
	       (SAY_NO_NLS, "Searching %d (slot %d)", stack[i].element, i));

    stack[i].mark = count++;
    stack[i].lowlink = stack[i].mark;
    stack[stack_ptr++].x = i;
    stack[i].on_stack = 1;

    se = stack[i].element;
    if (local_mapping(se).def_offset < 0) {
	/* Originally deferred subtype now has a complete definition. */
	DEBUG_MSG(STAB_DEBUG,
		  (SAY_NO_NLS, "Duplicate definition is complete."));
    }
    else {
	x = &local_mapping(se).incomplete_stab
	    ->stabstring[local_mapping(se).def_offset];
	def_len = convert2(x-2);

	for (cur_offset = convert2(x) + 2;
	     cur_offset < def_len;
	     cur_offset += convert2(&x[cur_offset]) + 2) {
	    int subtype = convert2(&x[cur_offset]);
	    int subtype_len = convert2(&x[cur_offset+3]);
	    j = local_mapping(subtype).global;
	    if (j < 0) {
		j= -j;
		if (stack[j].mark == 0) {
		    search(j);
		    stack[i].lowlink = min(stack[i].lowlink, stack[j].lowlink);
		}
		else if (stack[j].mark < stack[i].mark && stack[j].on_stack)
		    stack[i].lowlink = min(stack[j].mark, stack[i].lowlink);
	    }
	    cur_offset += 5 + subtype_len;
	}
    }

    if (stack[i].lowlink == stack[i].mark) {
	/* We found a strongly-connected component.  Process it. */
	component_size = 0;
	do {
	    ++component_size;
	    j = stack[--stack_ptr].x;
	    stack[j].on_stack = 0;
	    DEBUG_MSG(STAB_DEBUG, (SAY_NO_NLS, "\tStab %d (slot %d) ",
				   stack[j].element, j));
	} while (j != i);
	if (component_size > 1) {
	    /* Here's where we will handle cycles when the time comes.  For
	       now, because use_arbitrary_mapping=1, add_type_refs() will
	       never return non-zero. */
	    for (k = stack_ptr + component_size - 1; k >= stack_ptr; k--) {
		se = stack[stack[k].x].element;
		type_being_defined = se;
		if (add_type_refs(se, 0, NULL) != 0)
		    internal_error();
	    }
	}
	else {
	    /* The type definition isn't part of a cycle.  Any subtypes it
	       needs must already be mapped.
	       The only exception is a self-referential type, and because
	       of a peculiarity in the stabstring grammar, there is at
	       least one such type. */
	    se = stack[j].element;
	    type_being_defined = se;

	    /* If we handle cycles, we will want to handle a self-referential
	       subtype better.  For now, we just leave use_arbitrary_mapping=1
	       so that add_type_refs can never return non-zero. */
	    if (add_type_refs(se, 0, NULL) != 0)
		internal_error();
	}

	DEBUG_MSG(STAB_DEBUG,
		  (SAY_NO_NLS,
		   "End of SCC (component size = %d)", component_size));
    }
} /* search */
/************************************************************************
 * Name: stack_types
 *									*
 * Purpose:
 *									*
 ************************************************************************/
static void
stack_types(char *ss,
	    int slen,
	    STAB *x,
	    int *needed_subtypes_p)
{
    int def_len;
    int cur_offset;
    int	subtype;

    for (cur_offset = convert2(ss) + 2;
	 cur_offset < slen;
	 cur_offset += convert2(&ss[cur_offset]) + 2) {
	subtype = convert2(&ss[cur_offset]);
	switch(ss[cur_offset+2]) {
	  case '=':
	  case 'r':
	    def_len = convert2(&ss[cur_offset+3]);
	    stack_types(&ss[cur_offset + 5], def_len, x, needed_subtypes_p);
	    DEBUG_MSG(STAB_DEBUG, (SAY_NO_NLS|SAY_NO_NL, "\t(%sdefinition)",
				   ss[cur_offset+2]=='r' ? "re" : ""));
	    /* Skip over definition and length bytes */
	    cur_offset += 5 + def_len;
	    break;
	  case 'p':
	    DEBUG_MSG(STAB_DEBUG, (SAY_NO_NLS|SAY_NO_NL, "\t(placeholder)"));
	    /* Skip over definition and length bytes */
	    cur_offset += 5 + 3;	/* Placeholder def. is 3 bytes long */
	    break;
	  case ' ':
	    cur_offset += 5;
	    if (local_mapping(subtype).defined == 0) {
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(STAB_ERROR12,
 "%1$s: 0711-393 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n"
 "\tA stabstring refers to type %4$d, which is not defined."),
			 Main_command_name,
			 x->u._deferred.syment->n_offset,
			 get_object_file_name(stab_obj),
			 subtype);
		DEBUG_MSG(STAB_DEBUG,
			  (SAY_NO_NLS, "Mapping (for undefined type) %d->1",
			   subtype));
		local_mapping(subtype).global = 1;
		local_mapping(subtype).defined = 1;
#ifdef DEBUG
		local_mapping(subtype).arbitrary = 1;
#endif
	    }
	    break;
	}
	if (local_mapping(subtype).global == 0) {
	    stack[++(*needed_subtypes_p)].element = subtype;
	    local_mapping(subtype).global = -(*needed_subtypes_p);
	    DEBUG_MSG(STAB_DEBUG,
		      (SAY_NO_NLS, "\t%d (assigned to stack slot %d)",
		       subtype, (*needed_subtypes_p)));
	}
#ifdef DEBUG
	else {
	    if (bind_debug & STAB_DEBUG) {
		if (local_mapping(subtype).global < 0)
		    say(SAY_NO_NLS, "\t%d (already in slot %d)",
			subtype, -local_mapping(subtype).global);
		else
		    say(SAY_NO_NLS, "\t%d (already mapped to type %d)",
			subtype, local_mapping(subtype).global);
	    }
	}
#endif
    }
} /* stack_types */
/************************************************************************
 * Name: flatten_typedefs
 *									*
 * Purpose: Expand all subtype descriptors contained in the input def.
 *									*
 ************************************************************************/
static char *
flatten_typedefs(char *x,
		 int src_len,
		 char *dest)
{
    char	c;
    char	*src = x;
    int		def_len, frag_len, subtype;
    int		global_type;

    /* Copy the first fragment */
    frag_len = convert2(src);
    src += 2;
    if (frag_len > 0) {
	memcpy(dest, src, frag_len);
	src += frag_len;
	dest += frag_len;
    }

    while (src < &x[src_len]) {
	/* Write out a subtype */
	subtype = convert2(src);
	src += 2;
	global_type = get_local_mapping(subtype);
#ifdef DEBUG
	if (global_type == 0)
	    internal_error();
#endif
	switch(*src++) {
	  case '=':
	    dest += stab_sprintf(dest, "%d=", global_type);
	    def_len = convert2(src);
	    src += 2;
	    frag_len = convert2(src);
	    DEBUG_MSG(STAB_DEBUG,
		      (SAY_NO_NLS,
		    "\t(writing defining instance for type %d)%s", subtype,
		       (def_len != frag_len + 2)
		       ? "\n\t(Additional definition)" : ""));
	    if (local_mapping(subtype).def_offset >= 0)
		internal_error();
	    /* Write subtype definition */
	    if (def_len != frag_len + 2) {
		dest = flatten_typedefs(src, def_len, dest);
	    }
	    else {
		memcpy(dest,
		       local_mapping(subtype).stab_def,
		       -local_mapping(subtype).def_offset);
		dest += -local_mapping(subtype).def_offset;
	    }
	    src += def_len;
	    break;
	  case 'r':
	    /* Use unique output type. */
	    dest += stab_sprintf(dest, "%d=", global_mapping++);
	    def_len = convert2(src);
	    src += 2;
	    DEBUG_MSG(STAB_DEBUG,
		      (SAY_NO_NLS, "\t(writing redefinition of type %d)",
		       subtype));
	    /* Write subtype definition */
	    dest = flatten_typedefs(src, def_len, dest);
	    src += def_len;
	    break;
	  case ' ':
	    dest += stab_sprintf(dest, "%d", global_type);
	    src += 2;
	    break;
	  case 'p':
	    dest += stab_sprintf(dest, "%d", global_type);
	    /* Placeholder definition */
	    DEBUG_MSG(STAB_DEBUG,
		      (SAY_NO_NLS,
		       "\t(writing placeholder for type %d)", subtype));
	    strcpy(dest, STAB_PLACEHOLDER);
	    dest[3] = src[4];
	    dest += strlen(STAB_PLACEHOLDER);
	    src += 2 + 2 + 1;
	    break;
	}

	/* Copy next fragment */
	frag_len = convert2(src);
	src += 2;
	if (frag_len > 0) {
	    memcpy(dest, src, frag_len);
	    dest += frag_len;
	    src += frag_len;
	}
    }
    return dest;
} /* flatten_typedefs */
/************************************************************************
 * Name: process_deferred_stabs
 *									*
 * Purpose:
 *									*
 ************************************************************************/
void
process_deferred_stabs(void)
{
    static char *id = "process_deferred_stabs";
    char	*dest;
    int		def_len, stab_len;
    int		cur_offset;
    int		subtype, global_type;
    int		frag_len;
    int		needed_subtypes;
    int		i;
#ifdef DEBUG
    int		cannot_delete = 0;
#endif
    STAB	*x, *last_x;

#ifdef DEBUG
    if (bind_debug & STAB_DEBUG) {
	if (successful_count != 0 || deferred_count != 0) {
	    conditional_NL();
	    say(SAY_NO_NLS,
    "\nStarting deferred stabs for %s: %d were good initially, %d deferred",
		stab_sf->sf_name->name, successful_count, deferred_count);
	}
    }
    last_tbd = 0;
#endif

    if (deferred_stabs == NULL)
	return;
    type_being_defined = 0;

    /* Allocate a stack for finding strongly-connected components--
       we don't use stack[0]. */
    stack = emalloc(sizeof(*stack) * (num_deferred_types+1), id);
    for (i = 1; i <= num_deferred_types; i++) {
	stack[i].element = 0;
	stack[i].mark = 0;
	stack[i].on_stack = 0;
	stack[i].lowlink = 0;
    }

    needed_subtypes = 0;
    for (x = deferred_stabs; x; x = x->stab_next) {
	DEBUG_MSG(STAB_DEBUG,		/* Print first fragment only. */
		  (SAY_NO_NLS, "Processing deferred stab beginning: %-*s",
		   convert2(x->stabstring), &x->stabstring[2]));
	stack_types(x->stabstring, x->deferred_length, x, &needed_subtypes);
    }

    DEBUG_MSG(STAB_DEBUG,
	      (SAY_NO_NLS, "needed_subtypes = %d, num_deferred_types = %d",
	       needed_subtypes, num_deferred_types));

    /* Now find the strongly-connected graphs */
    count = 1;
    DEBUG_MSG(STAB_DEBUG, (SAY_NO_NLS, "Beginning search routine"));

    use_arbitrary_mapping = 1;
    for (i = 1; i <= needed_subtypes; i++)
	if (stack[i].mark == 0)
	    search(i);
    use_arbitrary_mapping = 0;

    /* Go through the deferred stabstrings and write them out to the .debug
       section.  Their dependent types must have been defined
       (possibly arbitrarily). */
    type_being_defined = 0;
    for (x = deferred_stabs; x; x = x->stab_next) {
	last_x = x;
#ifdef DEBUG
	if (bind_debug & STAB_DEBUG) {
	    if (bind_debug & DEBUG_LONG) {
		say(SAY_NO_NLS, "Completing deferred stab");
		print_stab(x->stabstring, x->deferred_length, 1);
	    }
	    else {
		say(SAY_NO_NLS, "Completing deferred stab beginning: %*s",
		    convert2(x->stabstring), &x->stabstring[2]);
	    }
	}
#endif

	dest = stab_mem_ptr + 2;	/* Leave 2 bytes for length */
	dest = flatten_typedefs(x->stabstring, x->deferred_length, dest);
	*dest++ = '\0';

	/* Compute length (with null) */
	stab_len = dest - (stab_mem_ptr + 2);
	switch(save_unique_stabstring(x->u._deferred.syment,
				      stab_mem_ptr + 2,
				      stab_len - 1,
				      SUS_NONE)) {
	  case 1:			/* New stabstring */
	    write2(stab_mem_ptr, stab_len);
	    DEBUG_MSG(STAB_DEBUG|DEBUG_LONG,
		      (SAY_NO_NLS, "current_stab_offset = %d",
		       current_stab_offset - (stab_len + 2)));
	    stab_mem_ptr = dest;
	    break;
	  case 0:			/* Stabstring does not need to be
					   saved, because it fit in the symbol
					   table entry, or it's a duplicate. */
#ifdef DEBUG
	    if (bind_debug & STAB_DEBUG)
		if (stab_len - 1 > SYMNMLEN)
		    say(SAY_NO_NLS, "Compacting duplicate stabstring");
#endif
	    break;
	  case -1:
	    /* Duplicate, deletable stabsting */
	    /* It's too late to delete the stab entry. Just make it NULL. */
#ifdef DEBUG
	    ++cannot_delete;
	    if (bind_debug & STAB_DEBUG)
		say(SAY_NO_NLS, "Would delete stab %*s",
		    convert2(x->stabstring), &x->stabstring[2]);
#endif
	    ;				/* Skip */
	}
    }
    free_stabs(deferred_stabs, last_x, num_deferred_types);
    deferred_stabs = NULL;
    num_deferred_types = 0;

    use_arbitrary_mapping = 0;

#ifdef DEBUG
    if (bind_debug & STAB_DEBUG) {
	int base, any = 0;
	for (base = 0; base < 16; base++) {
	    if (local_mapping_base[base]) {
		for (i = base << 12; i<=local_mapping_base[base]->limit; i++) {
		    if (local_mapping(i).arbitrary) {
			if (!any) {
			    say(SAY_NO_NLS, "Arbitrary mappings");
			    any = 1;
			}
			say(SAY_NO_NLS, "\t%d->%d",
			    i, local_mapping(i).global);
		    }
		}
	    }
	}
    }
    if (cannot_delete)
	say(SAY_NO_NLS, "%d C_DECLs would have been deleted", cannot_delete);
#endif

    efree(stack);
} /* process_deferred_stabs */
/************************************************************************
 * Name: reset_stab_mappings
 *									*
 * Purpose:
 *									*
 ************************************************************************/
void
reset_stab_mappings(void)
{
    int base;

    /* We assume most object files will have stabstrings numbered from 1.
       Therefore, we don't free the first array. */
    for (base = 1; base < 16; base++)
	if (local_mapping_base[base]) {
	    efree(local_mapping_base[base]);
	    local_mapping_base[base] = NULL;
	}

    if (local_mapping_base[0])
	local_mapping_base[0]->limit = -1;

    num_deferred_types = 0;
#ifdef DEBUG
    deferred_count = 0;
    successful_count = 0;

    if (Switches.stabcmpct >= STABCMPCT_RENUM
	&& (bind_debug & STAB_DEBUG)) {
	conditional_NL();
	say(SAY_NO_NLS,
	    "Starting stabs for %s", stab_sf->sf_name->name);
    }
#endif
} /* reset_stab_mappings */
#ifdef STAB_MAIN
/************************************************************************
 * Name: main
 *									*
 * Purpose:
 *									*
 ************************************************************************/
main() {
    char input_buffer[10000];
    char output_buffer[10000];

    while (gets(input_buffer)) {
	stab_mem_ptr = output_buffer;
	output = output_buffer;
	(void) parse_stabstring(input_buffer);
	*output = '\0';
	printf("%s\n", output_buffer);
    }
}
#endif
/************************************************************************
 * Name: stab_error
 *									*
 * Purpose: Print an error message for a stabstring
 *
 ************************************************************************/
#define stab_error(i,m,n,p) stab_error2(i,m,n,p,NULL)
#define STAB_ERROR_NO_NEXT_CHAR 0
#define STAB_ERROR_SHOW_NEXT_CHARACTER 1
static char *
stab_error2(
	    char *input,		/* Current input pointer */
#ifdef NO_NLS
	    char *n,			/* Message number */
#else
	    int	n,			/* Message string */
#endif
	    int show_next_input_char,
	    char *msg_parm1,
	    char *msg_parm2)
{
    int len;

    if (stab_error_occurred == 1)
	return input;
    stab_error_occurred = 1;

    bind_err(SAY_NORMAL, RC_SEVERE, n,
	     Main_command_name, old_symtab_index,
	     get_object_file_name(stab_obj),
	     msg_parm1, msg_parm2);
    if (input >= max_input) {
	if (show_next_input_char == STAB_ERROR_SHOW_NEXT_CHARACTER) {
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(STAB_END, "\tEnd of stabstring reached."));
	}
    }
    else {
	if (show_next_input_char) {
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     isprint(*input)
		     ? NLSMSG(STAB_CHARACTER, "\tCharacter %c seen.")
		     : NLSMSG(STAB_ASCII, "\tASCII %d seen."),
		     *input);
	}
	if (max_input - input > 0) {
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(STAB_REMAINDER,
			    "\tRemainder of stabstring is:\n%s"),
		     input);

	    /* Copy remaining portion of stabstring. */
	    len = max_input - input;
	    memcpy(output, input, len);
	    output += len;
	    input += len;
	}
    }
    return input;
} /* stab_error2 */
/************************************************************************
 * Name: stabstrings_equiv
 *									*
 * Purpose: Compare two stabstrings for equality.
 *	Returns: 1, if stabstrings are equal
 *		 0, otherwise
 *									*
 ************************************************************************/
static int
stabstrings_equiv(int local_type,
		  char *src,
		  int src_len)
{
    if (local_mapping(local_type).def_offset < 0) {
	/* Earlier definition is complete.  We can just compare strings */
	if (src_len == -local_mapping(local_type).def_offset
	    && memcmp(local_mapping(local_type).incomplete_stab,
		      src,
		      src_len) == 0)
	    return 1;			/* Definitions are equal */
    }
    else {
	char *frag = &local_mapping(local_type).incomplete_stab
	    ->stabstring [local_mapping(local_type).def_offset];
	if (compare_subtype_to_string(frag,
				      &frag[convert2(&frag[-2])],
				      src,
				      &src[src_len]))
	    return 1;
    }
    return 0;
} /* stabstrings_equiv */
/************************************************************************
 * Name: copy_typeid
 *									*
 * Purpose: Copy a typeid, including its definition if any.
 *									*
 ************************************************************************/
static char *
copy_typeid(char *input,
	    char *typename)
{
    char	*type_def_base;
    STAB	**stab_hash_ptr, *temp_hash;
    char	*saved_frag_base;
    char	*save_input;
    int		use_hash_ptr;
    int		digits;
    int		frag_len, def_len, temp_len;
    int		global_type;
    int		local_type;
    int		saved_forward_ref_seen;
    int		saved_unique_type;
    int		saved_type_being_defined;
    int		type_len;
    int		stab_redefined;

    DEBUG_MSG(STAB_DEBUG, (SAY_NO_NLS, "Entering copy_typeid, stab = %.60s",
			   input));

    save_input = input;			/* Used for redefinition error */
    if (*input == '-') {
	/* Predefined types are negative (or 0) and are
	   copied directly to the output */
	safe_copy_cur_char();		/* Copy sign */
	while (l_isdigit(*input))
	    safe_copy_cur_char();	/* Copy digits */
      check_for_redefine:
	if (*input == '=') {
	    if (stab_error_occurred == 0) {
		stab_error_occurred = 1;
		bind_err(SAY_NO_NL, RC_SEVERE,
			 NLSMSG(STAB_ERROR11,
 "%1$s: 0711-392 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n"
 "\tStabstring redefines predefined type %5$.*4$s."),
			 Main_command_name,
			 old_symtab_index,
			 get_object_file_name(stab_obj),
			 input-save_input, /* Length of bad number */
			 save_input);	/* Pointer to bad number */
	    }
	    /* Copy remaining portion of stabstring. */
	    temp_len = max_input - input;
	    memcpy(output, input, temp_len);
	    output += temp_len;
	    input += temp_len;
	}
	return input;
    }

    local_type = 0;
    digits = 0;
    while (l_isdigit(*input)) {
	digits = 1;
	local_type = 10 * local_type + (*input++ - '0');
    }

    if (digits == 0) {
	if (typename != NULL) {
	    input = stab_error(input, NLSMSG(STAB_ERROR5,
 "%1$s: 0711-386 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n"
 "\tType number expected."),
		       STAB_ERROR_SHOW_NEXT_CHARACTER, NULL);
	}
	else
	    input = copy_typedef(input);
	return input;
    }

    if (local_type == 0) {
	*output++ = '0';		/* Write '0' to output */
	goto check_for_redefine;
    }

    if (*input != '=') {
	/* Using a previously-defined type--check for mapped value */
	global_type = get_local_mapping(local_type);
	/* If local_type is not yet mapped, then
	   a) "forward_ref_seen" will be set,
	   b) 0 will be returned, and
	   c) eventually, stabstring being processed will
		be saved, to be processed later.
	*/
	if (global_type > 0)
	    output += stab_sprintf(output, "%d", global_type);
	else {
	    /* Previous fragment is complete now */
	    frag_len = output - frag_base;
	    write2(&frag_base[-2], frag_len);

	    /* Write 5-byte subtype reference information */
	    write2(output, local_type);
	    output[2] = ' ';		/* Not a definition */
	    output[3] = '\0';		/* No definition here--length is 0 */
	    output[4] = '\0';
	    output += 7;		/* Skip over 5-byte subtype, and leave
					   room for next frag_len */
	    frag_base = output;		/* Next fragment begins here */
	}
	return input;
    }

    /* Previous fragment is complete for now.  It will be extended if this
       new subtype can be parsed with complete mappings. */
    frag_len = output - frag_base;
    write2(&frag_base[-2], frag_len);
    saved_frag_base = frag_base;	/* Save, in case fragment will be
					   extended */

    /* Set up the 5-byte subtype reference in case
       the subtype cannot be completly translated. */
    write2(output, local_type);
    output[2] = '=';
    /* output[3] and output[4] will contain type-def-len for subtype, if it
       contains references to undefined subtypes */
    output += 5;			/* Skip over 5-byte subtype */
    type_def_base = output;
    output += 2;			/* Leave room for next frag_len */
    frag_base = output;

    create_local_mapping(local_type, -2);	/* Extend mapping. */
    if (local_mapping(local_type).defined
	&& local_mapping(local_type).placeholder == 0) {
	stab_redefined = 1;
#ifdef DEBUG
	local_mapping(local_type).depends_on_count = 0;	/* Reset */
	if (bind_debug & STAB_DEBUG)
	    say(SAY_NO_NLS, "Redefining type %d. ", local_type);
#endif
    }
    else {
	stab_redefined = 0;
	local_mapping(local_type).defined = 1;
	local_mapping(local_type).incomplete_stab = current_stab;
	local_mapping(local_type).def_offset = type_def_base - stab_mem_ptr;
    }

    saved_type_being_defined = type_being_defined;
    saved_forward_ref_seen = forward_ref_seen;
    saved_unique_type = unique_type;

    type_being_defined = local_type;
    forward_ref_seen = 0;
    unique_type = 0;

    input++;				/* Skip '=' */
    /* Copy optional type attributes */
    while (*input == '@') {
	safe_copy_cur_char();
	input = copy_past(input, ';');
    }
    input = copy_typedef(input);

    /* Write the length of the last fragment (writing 0 is okay) */
    frag_len = output - frag_base;	/* Length of last fragment */
    write2(&frag_base[-2], frag_len);

    type_being_defined = saved_type_being_defined;

    if (forward_ref_seen) {
	/* We can't use the newly-found type for comparisons, because it
	   contains a forward reference.  Instead, we save the stabstring,
	   to be processed later.
	   We must set the length of the definition in the 5-byte area. */
	++num_deferred_types;

	def_len = output - type_def_base;
	write2(&type_def_base[-2], def_len);
	output += 2;			/* Leave room for next fragment len. */
	frag_base = output;		/* Set up frag_base in case anything
					   follows.  */
	if (forward_ref_seen == 'p') {
	    /* Definition is a placeholder. */
	    if (stab_redefined) {
#ifdef DEBUG
		conditional_NL();
#endif
		bind_err(SAY_NORMAL, RC_SEVERE,
			 NLSMSG(STAB_MULTIPLE,
 "%1$s: 0711-377 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n"
 "\tType %4$d has multiple definitions."),
			 Main_command_name, old_symtab_index,
			 get_object_file_name(stab_obj),
			 local_type);
	    }
	    else
		local_mapping(local_type).placeholder = 1;
	    type_def_base[-3] = forward_ref_seen;
	}
	else {
	    local_mapping(local_type).placeholder = 0;
	    if (stab_redefined) {
		/* We have a redefinition.  See if the types are the same. */
		/* Since this type had a forward reference, the
		   earlier definition must have a forward reference as
		   well, or the types cannot be the same. */
		if (local_mapping(local_type).def_offset < 0 ||
		    compare_typedefs(&local_mapping(local_type).
				     incomplete_stab->stabstring
				 	[local_mapping(local_type).def_offset],
				     type_def_base) != 0) {
#ifdef DEBUG
		    conditional_NL();
#endif
		    bind_err(SAY_NORMAL, RC_SEVERE,
			     NLSMSG(STAB_MULTIPLE,
 "%1$s: 0711-377 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n"
 "\tType %4$d has multiple definitions."),
			     Main_command_name, old_symtab_index,
			     get_object_file_name(stab_obj),
			     local_type);
		    type_def_base[-3] = 'r';
		}
	    }
	}

	return input;
    }
    forward_ref_seen = saved_forward_ref_seen; /* Restore value */

#ifdef DEBUG
    /* Subtype translation is complete, so it consists of a single fragment.
       Null subtypes are not possible */
    if (frag_len == 0)
	internal_error();
#endif

    type_len = output - frag_base;

    /* If we have defined a subtype for the first time,
       and if the subtype is defined in the usual way as:
       <name>:t<nn>=.....
       then the definition for the subtype (as maintained by
       save_unique_subtype()) does not have to be copied to allocated
       memory but can point to the saved stabstring in the .debug
       section.  This will only work if the current stabstring is
       longer than 8 bytes and has no forward references anywhere. */
    stab_hash_ptr = NULL;		/* Initialize */
    use_hash_ptr = ((input == max_input) /* Subtype definition ended at
					    stabstring end. */
		    && forward_ref_seen == 0
		    && typename);
    if (stab_redefined
	&& !stabstrings_equiv(local_type, frag_base, type_len)) {
	bind_err(SAY_NORMAL, RC_SEVERE,
		 NLSMSG(STAB_MULTIPLE,
 "%1$s: 0711-377 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n"
 "\tType %4$d has multiple definitions."),
		 Main_command_name, old_symtab_index,
		 get_object_file_name(stab_obj),
		 local_type);
	global_type = global_mapping++;	/* Generate a new mapping */
    }
    else {
	if (unique_type) {
	    global_type = create_local_mapping(local_type, 0);
	    /* We don't call save_unique_subtype(), because we will
	       never reuse global_type. */
	    if (use_hash_ptr) {
		/* We set stab_hash_ptr to a non-NULL value for
		   the adjustment below.  Because we aren't calling
		   save_unique_subtype(), temp_hash won't be assigned
		   a valid value. */
		stab_hash_ptr = &temp_hash;
		local_mapping(local_type).stab_def = frag_base;
	    }
	    else {
		/* We can't save the subtype definition in place. We
		   copy it to allocated memory. */
#ifdef DEBUG
		local_mapping(local_type).stab_def
		    = save_string(frag_base, type_len+1);
		/* Add null for dbx */
		local_mapping(local_type).stab_def[type_len] = '\0';
#else
		local_mapping(local_type).stab_def
		    = save_string(frag_base, type_len);
#endif
	    }
	    local_mapping(local_type).def_offset = -type_len;
	}
	else if (use_hash_ptr) {
	    stab_hash_ptr = &temp_hash;
	    global_type = save_unique_subtype(local_type, frag_base, type_len,
					      typename, &stab_hash_ptr);
	}
	else
	    global_type = save_unique_subtype(local_type, frag_base, type_len,
					      typename, NULL);
    }

    /* Write mapped subtype over placeholder */
    output = frag_base - 7;
    output += stab_sprintf(output, "%d=", global_type);
    /* Copy the fragment */
    memmove(output, frag_base, type_len);
    /* If we saved a new subtype in place, we must adjust its
       string pointer.  If stab_hash_ptr isn't NULL, it points to temp_hash.
       If unique_type is not NULL, temp_hash doesn't point to anything.
       Otherwise, it points to the subtype STAB to update. */
    if (stab_hash_ptr) {
	char *temp_output;

	if ((output + type_len) - (stab_mem_ptr + 2) <= SYMNMLEN) {
#ifdef DEBUG
	    DEBUG_MSG(STAB_DEBUG, (SAY_NO_NLS, "Can't do in place."));
	    temp_output = save_string(output, type_len + 1);
	    temp_output[type_len] = '\0'; /* Add null for dbx */
#else
	    temp_output = save_string(output, type_len);
#endif
	}
	else
	    temp_output = output;
	local_mapping(local_type).stab_def = temp_output;
	if (!unique_type)
	    temp_hash->stabstring = temp_output;
    }
    output += type_len;
    unique_type = saved_unique_type;
    frag_base = saved_frag_base;	/* We can continue fragment */
    return input;
} /* copy_typeid */
/************************************************************************
 * Name: create_local_mapping
 *									*
 * Purpose: Create a new mapping from a local (input-file-specific) type
 *	number to a global type number.
 *									*
 * Parms/Returns:							*
 *	local_index: Type number whose mapping we'll create.
 *	trial_index: If -2, just initialize the local mapping array to
 *		include at least "local_index" elements.
 *		If 0, use the next available global mapping type number
 *		Otherwise, trial index will be used as the global mapping,
 *		and it must match a previous mapping, or an error occurs.
 *
 *	Returns: The global mapped-to type number
 *									*
 ************************************************************************/
static int
create_local_mapping(int local_index,
		     int trial_index)
{
    int m;
    int base = local_index >> 12;

    if (local_mapping_base[base] == NULL) {
	local_mapping_base[base] = emalloc(sizeof(*local_mapping_base[base]),
					   "create_local_mapping");
#ifdef DEBUG
	if (base == 0)
	    lm = &local_mapping_base[0]->m[0]; /* For use in dbx. */
#endif
	local_mapping_base[base]->limit = local_index;
	m = local_index & (~0xFFF);
	goto init_map;
    }
    else {
	if (local_index > local_mapping_base[base]->limit) {
	    m = local_mapping_base[base]->limit + 1;
	    local_mapping_base[base]->limit = local_index;
	  init_map:
	    for (; m <= local_index; m++) {
		local_mapping(m).global = 0;
		local_mapping(m).needed = 0;
		local_mapping(m).named = 0;
		local_mapping(m).placeholder = 0;
		local_mapping(m).defined = 0;
		local_mapping(m).incomplete_stab = NULL;
		local_mapping(m).input_offset = 0;
#ifdef DEBUG
		local_mapping(m).depends_on = 0;
		local_mapping(m).depends_on_count = 0;
		local_mapping(m).arbitrary = 0;
#endif
	    }
	}
    }

    if (trial_index == -2)
	return 0;		/* Extend array only */

    if (local_mapping(local_index).global <= 0) {
	if (trial_index == 0)
#ifdef ID_MAP
	    trial_index = local_index;
#else
	    trial_index = global_mapping++;
#endif
	local_mapping(local_index).global = trial_index;
#ifdef DEBUG
	if (use_arbitrary_mapping) {
	    local_mapping(local_index).arbitrary = 2;
	    if (bind_debug & STAB_DEBUG)
		say(SAY_NO_NLS, "Mapping (arbitrary): %d->%d",
		    local_index, trial_index);
	}
	else if (bind_debug & STAB_DEBUG)
	    say(SAY_NO_NLS, "Mapping: %d->%d", local_index, trial_index);
#endif
    }
    else {
	/* Mapping already exists. */
	if (trial_index != 0
	    && local_mapping(local_index).global != trial_index) {
#ifdef DEBUG
	    if (bind_debug & STAB_DEBUG) {conditional_NL();}
#endif
	    bind_err(SAY_NORMAL, RC_SEVERE,
		     NLSMSG(STAB_BAD_MAP,
 "%1$s: 0711-396 INTERNAL ERROR: Symbol table entry %2$d, object file %3$s\n"
 "\tConflict in local mapping: old %4$d; new %5$d."),
		     Main_command_name,
		     old_symtab_index,
		     get_object_file_name(stab_obj),
		     local_mapping(local_index).global,
		     trial_index);
	    trial_index = local_mapping(local_index).global;
	}
    }
    return trial_index;
} /* create_local_mapping */
/************************************************************************
 * Name: get_local_mapping
 *									*
 * Purpose: Given the local type index "old_index," return its global index.
 *	If no mapping exists and use_arbitrary_mapping is set,
 *		call create_local_mapping() to assign a new global mapping.
 *	Otherwise, set forward_ref_exists and return 0.
 *									*
 ************************************************************************/
static int
get_local_mapping(int old_index)
{
    int new_index;

    if (local_mapping_base[old_index >> 12]
	&& old_index <= local_mapping_base[old_index >> 12]->limit
	&& local_mapping(old_index).global > 0)
	new_index = local_mapping(old_index).global;
    else {
#ifdef DEBUG
	if ((bind_debug&(STAB_DEBUG|DEBUG_LONG)) == (STAB_DEBUG|DEBUG_LONG)) {
	    conditional_NL();
	    if (type_being_defined < 0)
		say(SAY_NO_NLS,
		    "Current stabstring needs types %d", old_index);
	    else
		say(SAY_NO_NLS,"Type %d needs types %d",
		    type_being_defined, old_index);
	    last_tbd = type_being_defined;
	}
#endif
	if (use_arbitrary_mapping)
	    new_index = create_local_mapping(old_index, 0);
	else {
	    create_local_mapping(old_index, -2); /* Extend mapping array */
	    local_mapping(old_index).needed = 1;
	    forward_ref_seen = 1;
#ifdef DEBUG
	    unmapped_type = old_index; /* Save some needed type */
	    if (type_being_defined > 0) {
		local_mapping(type_being_defined).depends_on = old_index;
		++local_mapping(type_being_defined).depends_on_count;
	    }
#endif
	    return 0;
	}
    }

#ifdef DEBUG
    if (bind_debug & STAB_DEBUG) {
	if (type_being_defined > 0
	    && local_mapping(old_index).arbitrary > 0
	    && local_mapping(type_being_defined).arbitrary == 0) {
	    local_mapping(type_being_defined).arbitrary = 1;
	    conditional_NL();
	    say(SAY_NO_NLS, "STABS: Type %d depends on arbitrary type %d.",
		type_being_defined, old_index);
	}
    }
#endif
    return new_index;
} /* get_local_mapping */
/************************************************************************
 * Name: compare_frags
 *
 * Function:  Compare 2 stabstring fragments.  This is not a general-purpose
 *		comparision routine, because the second fragment can be
 *		from a "more-defined" stabstring, so it can be longer
 *		than the first fragment.  In this case, typeref-fragment
 *		pairs from the first argument are scanned to complete
 *		the comparision.
 *									*
 * RETURNS: 0 match
 *	    1 no match
 *									*
 ************************************************************************/
static int
compare_frags(char **pf1,
	      char **pf2)
{
    int subtype;
    int global_type;
    int number;

    char *f1 = *pf1;
    char *f2 = *pf2;
    int f1_len = convert2(f1);
    int f2_len = convert2(f2);

    f1 += 2;
    f2 += 2;
    if (f1_len == f2_len) {
	/* Fragments are the same length. */
	if (memcmp(f1, f2, f1_len) != 0)
	    return 1;			/* Fragments differ. */
	*pf1 = f1 + f1_len;
	*pf2 = f2 + f2_len;
	return 0;			/* Identical */
    }
    /* Lengths are unequal.  The second argument length must
       be longer than the first. */
    if (f2_len < f1_len)
	return 1;
    while (f2_len > 0) {
	if (memcmp(f1, f2, f1_len) != 0)
	    return 1;
	f1 += f1_len;
	f2 += f1_len;
	f2_len -= f1_len;

	/* The last fragment of f1 must match the remaining part of the
	   fragment of f2, so we have a match. */
	if (f2_len == 0)
	    break;

	/* f1 points to a type_ref.  f2 must point to the mapped instance
	   of the same type. */
	subtype = convert2(f1);
	/* The mapping for 'subtype' must already exist, or its mapped type
	   couldn't be part of the current fragment for f2. */
	if ((global_type = local_mapping(subtype).global) <= 0)
	    return 1;

	/* Get the mapped type found in f2.  We must convert the string
	   back to a number. */
	if (!l_isdigit(*f2))
	    return 1;
	number = 0;
	while (l_isdigit(*f2)) {
	    number = number * 10 + *f2++ - '0';
	    --f2_len;
	}
	if (number != global_type)
	    return 1;
	if (f1[2] != ' ') {
	    /* A definition, redefinition, or placeholder */
	    return 1;			/* This is too hard. */
	}
	f1 += 5;
	f1_len = convert2(f1);
	if (f1_len > f2_len)
	    return 1;
	f1 += 2;
    }
    *pf1 = f1;
    *pf2 = f2;
    return 0;
} /* compare_frags */
/************************************************************************
 * Name: compare_typerefs
 *									*
 * Purpose:
 *									*
 * RETURNS:	0 match
 *		1 no match
 *									*
 ************************************************************************/
static int
compare_typerefs(char **pf1,
		 char **pf2)
{
    char *f1 = *pf1;
    char *f2 = *pf2;

    if (convert2(f1) != convert2(f2))
	return 1;
    /* If neither typeref is a definition, we must increment the pointers
       past the reference. If both are definitions, they must be identical
       definitions, or we will have reported an error for a redefinition.
       Another error would not be appropriate here.
       If only typeref is a definition, we interpret this as a match..

       So in all cases, it makes sense to skip over the definitions.
       Since convert2(&f1[3]) == 0 for a non-definition, we can use
       the two statements below in all cases. */
    *pf1 = f1 + 5 + convert2(&f1[3]);
    *pf2 = f2 + 5 + convert2(&f2[3]);
    return 0;
} /* compare_typerefs */
/************************************************************************
 * Name: compare_typedefs
 *									*
 * FUNCTION:	Compare two type definitions.  This is not a general-purpose
 *		routine.  The current definition must be "more-defined" than
 *		the previous definition.
 *
 * RETURNS:	0 match
 *		1 no match
 *									*
 ************************************************************************/
static int
compare_typedefs(char *f1,			/* Previous def */
		 char *f2)			/* Current def */
{
    char *f1_max, *f2_max;

    /* Note:  The current def can have longer fragments than the
       previous definition. */
    f1_max = f1 + convert2(&f1[-2]);
    f2_max = f2 + convert2(&f2[-2]);

    if (compare_frags(&f1, &f2) != 0)
	return 1;

    while (f1 < f1_max && f2 < f2_max) {
	if (compare_typerefs(&f1, &f2) != 0)
	    return 1;
	/* A typeref must be followed by a fragment, even if it's 0-length. */
	if (compare_frags(&f1, &f2) != 0)
	    return 1;
    }
    if (f1 != f1_max || f2 != f2_max)
	return 1;
    return 0;
} /* compare_typedefs */
/************************************************************************
 * Name: compare_subtype_to_string
 *									*
 * Purpose: Compare a subtype (made up of fragments) to a complete
 *	stabstring.  The string can have extra characters.
 *
 * RETURNS: pointer to first unmatched character in 'str' if the subtype
 *	is equal to a prefix of 'str';
 *	    NULL, otherwise
 ************************************************************************/
static char *
compare_subtype_to_string(char *src,
			  char *src_end,
			  char *str,
			  char *str_end)
{
    char *save_str = str;
    int frag_len;
    int def_len;
    int subtype;
    int n;

    /* Compare first fragment */
    frag_len = convert2(src);
    src += 2;
    if (&str[frag_len] > str_end
	|| memcmp(src, str, frag_len) != 0)
	return NULL;			/* no match */
    str += frag_len;
    src += frag_len;

    while(src < src_end) {
	/* Compare subtype */
	subtype = convert2(src);
	n = 0;
	while (str < str_end && l_isdigit(*str)) {
	    n = n * 10 + *str++ - '0';
	}
	if (n != local_mapping(subtype).global)
	    return NULL;
	switch(src[2]) {
	  case '=':
	  case 'r':
	    /* The subtype definition is here.

	       If the string has the subtype definition as well, we
	       compare the definitions.

	       Otherwise, we just skip over the subtype definition. */
	    def_len = convert2(&src[3]);
	    src += 5;
	    if (*str == '=') {
		str = compare_subtype_to_string(src, &src[def_len],
						str+1, str_end);
		if (str == NULL)
		    return NULL;
	    }
	    src += def_len;		/* Skip over embedded def. */
	    break;
	  case 'p':
	    /* A placeholder is here.

	       If the string has a placeholder as well, or no
	       embedded definition, we okay.

	       If the string has the subtype definition as well, we
	       compare the definitions.

	       Otherwise, we just skip over the subtype definition. */
	    src += 5;
	    if (*str == '=') {
		++str;
		if (str_end - str >= 5
		    && str[0] == 'Y'
		    && str[1] == '0'
		    && str[2] == src[0]
		    && str[4] == '('
		    && str[5] == ';')
		    str += 5;
		else
		    return NULL;
	    }
	    ++src;			/* Skip over ClassType in placeholder */
	    break;
	  case ' ':
	    src += 5;
	    if (*str == '=') {
		/* If the string has an embedded definition but the
		   fragment doesn't, we must compare the string to the
		   definition of the type, which must be complete. */
		str = compare_subtype_to_string
		    (local_mapping(subtype).stab_def,
		     &local_mapping(subtype).stab_def
		     [-local_mapping(subtype).def_offset],
		     str+1, str_end);
		if (str == NULL)
		    return NULL;
	    }
	    break;
	}

	/* Compare next fragment */
	frag_len = convert2(src);
	src += 2;
	if (&str[frag_len] > str_end
	    || memcmp(src, str, frag_len) != 0)
	    return NULL;		/* No match */
	str += frag_len;
	src += frag_len;
    }
#ifdef DEBUG
    if (src > src_end)
	internal_error();
#endif
    return src;
} /* compare_subtype_to_string */
/************************************************************************
 * Name: copy_char_error
 *									*
 * Purpose: Print an error, because the next character is not what was
 *	expected.
 *									*
 ************************************************************************/
static char *
copy_char_error(char *input,
		char c,
		char *prod)
{
    char_as_string[0] = c;
    return stab_error2(input,
		       NLSMSG(STAB_ERROR14,
 "%1$s: 0711-397 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n"
 "\tCharacter %4$s expected in production %5$s."),
		       STAB_ERROR_SHOW_NEXT_CHARACTER,
		       char_as_string, prod);
}
/************************************************************************
 * Name: parse_stabstring
 *									*
 * Purpose: Parse a stab string
 *
 * Parameters: input: char * to input stabstring.
 * Implied parameter: output: char * to output buffer, where translated
 *				stabstring should go.
 *									*
 * PRODUCTIONS:
 *	Stabstring:	NAME ':' Class
 *			':' Class
 *	Class:		'c' '=' Constant ';'
 *			NamedType
 *			Parameter
 *			Procedure
 *			Variable
 *			Label
 *	NamedType:	't' TypeID
 *			'T' TypeID
 *	Parameter:	'a' TypeID
 *			'p' TypeID
 *			'v' TypeID
 *			'C' TypeID
 *			'D' TypeID
 *			'R' TypeID
 *	Procedure:	Proc
 *			Proc ',' NAME ':' NAME
 *	Variable:	TypeID
 *			'd' TypeID
 *			'r' TypeID
 *			'G' TypeID
 *			'S' TypeID
 *			'V' TypeID
 *			'Y'
 *			'Z' TypeID ':' NAME
 *	Proc:		'f' TypeID
 *			'g' TypeID
 *			'm' TypeID
 *			'J' TypeID
 *			'F' TypeID
 *			'I'
 *			'P'
 *			'Q'
 *	Label:		'L'
 *
 ************************************************************************/
static char *
parse_stabstring(char *input)
{
    static char prod[] = "Stabstring";
    char *temp_stabstring_name;

    temp_stabstring_name = input;

    /* Copy NAME if it exists */
    if (*input != ':') {
	do {
	    safe_copy_cur_char();
	} while (*input && *input != ':');

	if (*input != ':') {
	    return stab_error(input,
			      NLSMSG(STAB_ERROR2,
 "%1$s: 0711-383 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n"
 "\tColon (:) marking beginning of stabstring not found."),
			      STAB_ERROR_NO_NEXT_CHAR, NULL);
	}
	stabstring_name_len = input - temp_stabstring_name;

	DEBUG_MSG(STAB_DEBUG, (SAY_NO_NLS, "%d Parsing Stabstring named %-*.*s",
			       old_symtab_index,
			       stabstring_name_len, stabstring_name_len,
			       temp_stabstring_name));
    }
    else {
	DEBUG_MSG(STAB_DEBUG, (SAY_NO_NLS, "%d Parsing unnamed Stabstring",
			       old_symtab_index));
	stabstring_name_len = 0;
    }

    safe_copy_cur_char();

    if (input >= max_input) {
	return stab_error(input,
			  NLSMSG(STAB_ERROR8,
 "%1$s: 0711-389 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n"
 "\tUnexpected end of stabstring in production %4$s."),
		   STAB_ERROR_NO_NEXT_CHAR, prod);
    }

    switch(safe_copy_cur_char()) {
      case 'c':				/* Constant */
	input = copy_char(input, '=', prod);
	input = copy_constant(input);
	break;

	/* NamedType */
      case 't':				/* User-defined type */
      case 'T':				/* User-defined, named type */
	input = copy_typeid(input, temp_stabstring_name);
	break;

	/* Procedure */
      case 'f':				/* Function: Private */
      case 'g':				/* generic function (fortran) */
      case 'm':				/* Function: Module (modula2) */
      case 'J':				/* Function: Internal */
      case 'F':				/* Function: External */
	input = copy_typeid(input, NULL);
	/* Fall through */

      case 'I':				/* Procedure: Internal */
      case 'P':				/* Procedure: External */
      case 'Q':				/* Procedure: Private */
	if (*input == ',') {
	    safe_copy_cur_char();
	    input = copy_name(input);
	    input = copy_char(input, ':', prod);
	    input = copy_name(input);
	}
	break;

      case '-':
      case '0':case '1':case '2':case '3':case '4':
      case '5':case '6':case '7':case '8':case '9':
	--input,--output;		/* Back up to previous character */
	input = copy_typeid(input, NULL);
	break;

	/* Parameter */
      case 'a':				/* Parameter: passed by ref in reg. */
      case 'p':				/* Parameter: passed by value on stk.*/
      case 'v':				/* Parameter: passed by ref on stack */
      case 'C':				/* Parm: const. passed by val on stk */
      case 'D':				/* Parameter: passed by val in FPR */
      case 'R':				/* Parameter: passed by val in GPR */

	/* Variable */
      case 'd':				/* Variable: floating register */
      case 'r':				/* Variable: register */
      case 'G':				/* Variable: external (global) */
      case 'S':				/* Variable: static global */
      case 'V':				/* Variable: static local */
	input = copy_typeid(input, NULL);
	break;

      case 'L':			/* Label */
      case 'Y':			/* Fortran pointer variable */
	break;

      case 'Z':			/* Fortran pointee variable */
	input = copy_typeid(input, NULL);
	input = copy_char(input, ':', prod);
	input = copy_name(input);
	break;

      case '*':				/* This isn't really kosher, but
					   Fortran sticks a typedef here. */
	input = copy_typedef(input);
	break;

      default:
	--input;
	input = stab_error(input,
			   NLSMSG(STAB_ERROR1,
 "%1$s: 0711-382 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n"
 "\tUnexpected character in production %4$s."),
			   STAB_ERROR_SHOW_NEXT_CHARACTER,
			   prod);
    }
    if (input < max_input) {
	input = stab_error(input,
			   NLSMSG(STAB_ERROR10,
 "%1$s: 0711-391 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n"
 "\tExtra characters at end of stabstring."),
			   STAB_ERROR_NO_NEXT_CHAR, NULL);
    }
    return input;
} /* parse_stabstring */
#ifdef DEBUG
/************************************************************************
 * Name: print_stab
 *									*
 * Purpose:
 *									*
 ************************************************************************/
static char *
print_stab(char *s,
	   int len,
	   int depth)
{
    char *max_s = &s[len];
    int frag_len, def_len, subtype, j;
    char c;
    int d;

    /* First fragment is always there */
    frag_len = convert2(s);
    s += 2;
    for (d = depth-1; d > 0; d--)
	say(SAY_NO_NLS | SAY_NO_NL, "\t");
    if (frag_len == 0)
	say(SAY_NO_NLS, "''");
    else
	say(SAY_NO_NLS, "(%d)'%*.*s'", frag_len, frag_len, frag_len, s);

    s += frag_len;			/* Skip over frag length field */
    while (s < max_s) {
	/* Extract the fields from the subtype specifier */
	subtype = convert2(s);
	c = s[2];
	def_len = convert2(&s[3]);
	s += 5;

	j = local_mapping(subtype).global;
	for (d = depth-1; d > 0; d--)
	    say(SAY_NO_NLS | SAY_NO_NL, "\t");
	if (j < 0)
	    say(SAY_NO_NLS, "Subtype: %d%c->?""? (slot %d)", subtype, c, -j);
	else
	    say(SAY_NO_NLS, "Subtype: %d%c->%d", subtype, c, j);

	if (c == '=')
	    s = print_stab(s, def_len, depth+1);

	frag_len = convert2(s);
	s += 2;				/* Skip over frag length field */

	if (frag_len != 0) {
	    for (d = depth-1; d > 0; d--)
		say(SAY_NO_NLS | SAY_NO_NL, "\t");
	    say(SAY_NO_NLS, "(%d)'%*.*s'", frag_len, frag_len, frag_len, s);
	    s += frag_len;
	}
    }
    return s;
} /* print_stab */
#endif
/* All routines past here are strictly for parsing--no tricky stuff.
   The only exceptions in copy_bound(), where special handling is
   needed to support dbx; and in copy_typedef(), where special
   handling is needed for placeholder type definitions. */
/************************************************************************
 * Name: copy_name
 *									*
 * Purpose: Copy a name from the input to the output.  The stabstring
 *	grammar doesn't allow a name to be null, but we allow it here
 *	for flexibility (and because some versions of the CSet compiler
 *	generate stabstrings with null names).
 *									*
 ************************************************************************/
static char *
copy_name(char *input)
{
    while (l_isname(*input))
	safe_copy_cur_char();
    return input;
}
/************************************************************************
 * Name: copy_past
 *									*
 * Purpose: Copy the input to the output until the desired character
 *	has been copied.
 *									*
 ************************************************************************/
static char *
copy_past(char *input,
	  char c)
{
    while (input < max_input && *input != c)
	safe_copy_cur_char();
    if (*input) {
	safe_copy_cur_char();
    }
    else {
	char_as_string[0] = c;
	input = stab_error(input, NLSMSG(STAB_ERROR17,
 "%1$s: 0711-379 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n"
 "\tExpected character %4$s not found before end of stabstring."),
			   0,		/* Don't show next character */
			   char_as_string);
    }
    return input;
}
/************************************************************************
 * Name: copy_constant
 *									*
 * PRODUCTIONS:
 *	Constant	: 'b' OrdValue/INTEGER
 *			| 'c' OrdValue/INTEGER
 *			| 'e' TypeID ',' OrdValue/INTEGER
 *			| 'i' INTEGER
 *			| 'r' REAL
 *			| 's' STRING
 *			| 'C' REAL ',' REAL
 *			| 'S' TypeID ',' NumElements ',' NumBits ',' BitPattern
 *
 ************************************************************************/
static char *
copy_constant(char *input)
{
    static char prod[] = "Constant";

    DEBUG_MSG(STAB_DEBUG, (SAY_NO_NLS,
			   "Entering %s, stab = %.60s", prod, input));

    switch(safe_copy_cur_char()) {
      case 'b':
      case 'c':
      case 'i':
	input = copy_int(input);	/* OrdValue or INTEGER */
	break;

      case 'e':
	input = copy_typeid(input, NULL);
	input = copy_char(input, ',', prod);
	input = copy_int(input);	/* OrdValue */
	break;

      case 'r':
	input = copy_real(input);
	break;

      case 's':
	input = copy_dbx_string(input);
	break;

      case 'S':
	input = copy_typeid(input, NULL);
	input = copy_char(input, ',', prod);
	input = copy_int(input);	/* NumElements */
	input = copy_char(input, ',', prod);
	input = copy_int(input);	/* NumBits */
	input = copy_char(input, ',', prod);
	/* Copy BitPattern */
	if (!l_isxdigit(*input)) {
	    char_as_string[1] = 'S';
	    return stab_error2(input,
			       NLSMSG(STAB_ERROR7,
 "%1$s: 0711-388 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n"
 "\tUnexpected character after %4$s in %5$s."),
			       STAB_ERROR_SHOW_NEXT_CHARACTER,
			       char_as_string, prod);
	}
	do {
	    safe_copy_cur_char();	/* Copy hex digits */
	} while (l_isxdigit(*input));
	break;

      case 'C':
	input = copy_real(input);
	input = copy_char(input, ',', prod);
	input = copy_real(input);
	break;

      default:
	--input;
	input = stab_error(input, NLSMSG(STAB_ERROR1,
 "%1$s: 0711-382 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n"
 "\tUnexpected character in production %4$s."),
		   STAB_ERROR_SHOW_NEXT_CHARACTER, prod);
    }
    return input;
} /* copy_constant */
/************************************************************************
 * Name: copy_dbx_string
 *									*
 * Purpose: Copy a string from the input to the output.  Such a string
 *	is quoted with either single or double quotes.
 *									*
 ************************************************************************/
static char *
copy_dbx_string(char *input)
{
    char quote;

    if (*input != '\'' && *input != '"') {
	input = stab_error(input, NLSMSG(STAB_ERROR6,
 "%1$s: 0711-387 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n"
 "\tSingle or double quote expected."),
		   STAB_ERROR_SHOW_NEXT_CHARACTER, NULL);
	return input;
    }
    quote = *input;
    safe_copy_cur_char();		/* Copy beginning quote */
    while (input < max_input && *input != quote) {
	/* Check for valid escape.  A backslash followed by another backslash
	   or the quote character is an escape sequence, so two characters
	   should be copied.  Otherwise, copy the slash literally. */
	if (*input == '\\' && (input[1] == '\\' || input[1] == quote))
	    safe_copy_cur_char();
	copy_cur_char();
    }
    if (*input == quote)
	safe_copy_cur_char();			/* Copy closing quote */

    /* Check for the exception.  If the final string character is a backslash,
       we allow it to be unescaped.  We've already copied the string, so
       no additional action is needed. */
    else if (!(input[-1] == quote && input[-2] == '\\')) {
	char_as_string[0] = quote;
	input = stab_error(input, NLSMSG(STAB_ERROR13,
 "%1$s: 0711-394 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n"
 "\tClosing %4$s not found."),
			   STAB_ERROR_SHOW_NEXT_CHARACTER,
			   char_as_string);
    }
    return input;
}
/************************************************************************
 * Name: copy_typedef
 *									*
 * PRODUCTIONS:
 *	TypeDef		: INTEGER <opt_semi>
 *			| 'b' TypeID ';' <cont> NumBytes
 *			| 'c' TypeID ';' <cont> NumBits
 *			| 'd' TypeID
 *			| 'e' EnumSpec ';'
 *			| 'g' TypeID ';' <cont> NumBits
 *			| 'i' NAME ':' NAME ';'
 *			| 'i' NAME ':' NAME ',' TypeID ';'
 *			| 'k' TypeID
 *			| 'l' ';' <cont>
 *			| 'm' OptVBaseSpec OptMultiBaseSpec TypeID ':' TypeID
 *				':' TypeID ';'
 *			| 'n' TypeID ';' <cont> NumBytes
 *			| 'o' NAME ';'
 *			| 'o' NAME ',' TypeID
 *			| 'w' TypeID
 *			| 'z' TypeID ';' <cont> NumBytes
 *			| 'C' Usage
 *			| 'I' NumBytes ';' <cont> PicSize
 *			| 'K' CobolFileDesc ';'
 *			| 'M' TypeID ';' <cont> Bound
 *			| 'N'
 *			| 'S' TypeID
 *			| '*' TypeID
 *			| '&' TypeID
 *			| 'V' TypeID
 *			| 'Z'
 *			| Array
 *			| Subrange
 *			| ProcedureType
 *			| Record
 *	Array		: 'a' TypeID ';' <cont> TypeID
 *			| 'A' TypeID
 *			| 'D' INTEGER ',' TypeID
 *			| 'E' INTEGER ',' TypeID
 *			| 'O' INTEGER ',' TypeID
 *			| 'P' TypeID ';' <cont> TypeID
 *	Subrange	: 'r' TypeID ';' <cont> Bound ';' <cont> Bound
 *	ProcedureType	: 'f' TypeID ';'
 *			| 'f' TypeID ',' NumParams ';' TParamList ';'
 *			| 'p' NumParams ';' TParamList ';'
 *			| 'F' TypeID ',' NumParams ';' NamedTParamList ';'
 *			| 'R' NumParams ';' NamedTParamList <opt_semi>
 *	Record		: 's' NumBytes <cont> FieldList <opt_semi>
 *			| 'u' NumBytes <cont> FieldList <opt_semi>
 *			| 'v' NumBytes <cont> FieldList VariantPart <opt_semi>
 *			| 'Y' NumBytes ClassKey OptPBV OptBaseSpecList '('
 *				ExtendedFieldList OptNameResolutionList ';'
 *			| 'G' optRedefinition n NumBits <cont> FieldList ';'
 *			| 'G' optRedefinition c NumBits <cont> CondFieldList
 *				',' <cont> FieldList ';'
 *	OptVBaseSpec	: ** empty **
 *			| 'v'
 *	ClassKey	: 's' | 'u' | 'c'
 *	CobolFileDesc	: Organization AccessMethod NumBytes/INTEGER
 *	Organization	: 'i' | 'l' | 'r' | 's'
 *	AccessMethod	: 'd' | 'o' | 'r' | 's'
 *	OptRedefinition	: ** empty **
 *			| 'r' NAME ','
 *	OptNameResolutionList	: ** empty **
 *			| ')' NameResolutionList
 *	OptMultiBaseSpec: 'm'
 *			| ** empty **
 *	OptPBV		: ** empty **
 *			| 'V'
 *	CondFieldList	: Conditions ',' <cont> FieldList
 *			| ',' <cont> FieldList ';'
 *
 ************************************************************************/
static char *
copy_typedef(char *input)
{
    int	c;
    char char_as_string[2];
    static char prod[] = "TypeDef";

    DEBUG_MSG(STAB_DEBUG, (SAY_NO_NLS,
			   "Entering %s, stab = %.60s", prod, input));

    if (l_isdigit(*input) || *input == '-') {
	input = copy_typeid(input, NULL); /* Definition shouldn't be here */
	if (*input == ';')
	    safe_copy_cur_char();
	return input;
    }
    if (input >= max_input) {
	return stab_error(input, NLSMSG(STAB_ERROR8,
 "%1$s: 0711-389 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n"
 "\tUnexpected end of stabstring in production %4$s."),
			  0, prod);
    }

    switch(c = (safe_copy_cur_char())) {
      case 'b':
      case 'c':
      case 'g':
      case 'n':
      case 'z':
	input = copy_typeid(input, NULL);
	input = copy_char(input, ';', prod);
	chkcont(input, max_input, old_symtab_index);
	input = copy_int(input);	/* NumBits or NumBytes */
	break;

      case 'd':				/* File of type TypeID */
      case 'k':				/* C++ "const" type */
      case 'w':				/* wide character */
      case 'S':				/* Set */
      case '*':				/* Pointer */
      case '&':				/* C++ Reference type */
      case 'V':				/* C++ volatile type */
      case 'A':				/* Array */
	input = copy_typeid(input, NULL);
	break;

      case 'e':
	input = copy_enumspec(input);
	input = copy_char(input, ';', prod);
	break;

      case 'i':
	input = copy_name(input);
	input = copy_char(input, ':', prod);
	input = copy_name(input);
	if (*input == ',') {
	    safe_copy_cur_char();
	    input = copy_typeid(input, NULL);
	}
	input = copy_char(input, ';', prod);
	break;

      case 'l':
	input = copy_char(input, ';', prod);
	chkcont(input, max_input, old_symtab_index);
	break;

      case 'm':
	if (*input == 'v')
	    safe_copy_cur_char();	/* OptVBaseSpec */
	if (*input == 'm')
	    safe_copy_cur_char();	/* OptMultiBaseSpec */
	input = copy_typeid(input, NULL);
	input = copy_char(input, ':', prod);
	input = copy_typeid(input, NULL);
	input = copy_char(input, ':', prod);
	input = copy_typeid(input, NULL);
	input = copy_char(input, ';', prod);
	break;

      case 'o':
	input = copy_name(input);
	if (*input == ';') {
	    safe_copy_cur_char();
	}
	else if (*input == ',') {
	    safe_copy_cur_char();
	    input = copy_int(input);
	}
	else {
	    char_as_string[0] = 'o';
	    input = stab_error2(input,
				NLSMSG(STAB_ERROR9,
 "%1$s: 0711-390 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n"
 "\tSyntax error after %4$s in production %5$s."),
				STAB_ERROR_SHOW_NEXT_CHARACTER,
				char_as_string, prod);
	}
	break;

      case 'N':			/* Pascal string pointer */
      case 'Z':			/* C++ ... (ellipses) parameter type */
	break;

      case 'C':
	input = copy_usage(input);
	break;

      case 'I':
	input = copy_int(input);	/* NumBytes */
	input = copy_char(input, ';', prod);
	chkcont(input, max_input, old_symtab_index);
	input = copy_int(input);	/* PicSize */
	break;

      case 'K':				/* CobolFileDesc */
	copy_cur_char();		/* Organization: i|l|r|s not checked */
	copy_cur_char();		/* AccessMethod: d|o|r|s not checked */
	input = copy_int(input);	/* NumBytes */
	input = copy_char(input, ';', prod);
	break;

      case 'M':
	input = copy_typeid(input, NULL);
	input = copy_char(input, ';', prod);
	chkcont(input, max_input, old_symtab_index);
	input = copy_bound(input);
	break;

      case 'a':				/* Array */
      case 'P':				/* Array */
	input = copy_typeid(input, NULL);
	input = copy_char(input, ';', prod);
	chkcont(input, max_input, old_symtab_index);
	input = copy_typeid(input, NULL);
	break;

      case 'D':				/* Array */
      case 'E':				/* Array */
      case 'O':				/* Array */
	input = copy_int(input);
	input = copy_char(input, ',', prod);
	input = copy_typeid(input, NULL);
	break;

      case 'r':				/* Subrange: base type, bounds */
	input = copy_typeid(input, NULL);
	input = copy_char(input, ';', prod);
	chkcont(input, max_input, old_symtab_index);
	input = copy_bound(input);
	input = copy_char(input, ';', prod);
	chkcont(input, max_input, old_symtab_index);
	input = copy_bound(input);
	break;

      case 'f':				/* procedure type */
	input = copy_typeid(input, NULL);
	if (*input == ',') {
	    safe_copy_cur_char();
	    goto copy1;
	}
	else if (*input == ';') {
	    safe_copy_cur_char();
	}
	else {
	    char_as_string[0] = 'f';
	    input = stab_error2(input,
				NLSMSG(STAB_ERROR9,
 "%1$s: 0711-390 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n"
 "\tSyntax error after %4$s in production %5$s."),
				STAB_ERROR_SHOW_NEXT_CHARACTER,
				char_as_string, prod);
	}
	break;

      case 'p':				/* Procedure type */
      copy1:
	input = copy_int(input);	/* NumParams */
	input = copy_char(input, ';', prod);
	input = copy_tparamlist(input);
	input = copy_char(input, ';', prod);
	break;

      case 'F':				/* Procedure type */
	input = copy_typeid(input, NULL);
	input = copy_char(input, ',', prod);
	/* Fall through */

      case 'R':				/* Procedure type */
	input = copy_int(input);	/* NumParams */
	input = copy_char(input, ';', prod);
	input = copy_namedtparamlist(input);
	if (*input == ';')
	    safe_copy_cur_char();
	break;

      case 's':				/* Record */
      case 'u':				/* Record */
	input = copy_int(input);	/* NumBytes */
	chkcont(input, max_input, old_symtab_index);
	input = copy_fieldlist(input);
	if (*input == ';')
	    safe_copy_cur_char();
	break;

      case 'v':				/* Record */
	input = copy_int(input);	/* NumBytes */
	chkcont(input, max_input, old_symtab_index);
	input = copy_fieldlist(input);
	input = copy_variantpart(input);
	if (*input == ';')
	    safe_copy_cur_char();
	break;

      case 'G':
	if (*input == 'r') {		/* optRedefinition */
	    safe_copy_cur_char();
	    input = copy_name(input);
	    input = copy_char(input, ',', prod);
	}
	c = *input;

	if (c != 'c' && c != 'n') {
	    char_as_string[0] = 'G';
	    input = stab_error2(input,
				NLSMSG(STAB_ERROR9,
 "%1$s: 0711-390 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n"
 "\tSyntax error after %4$s in production %5$s."),
				STAB_ERROR_SHOW_NEXT_CHARACTER,
				char_as_string, prod);
	    break;
	}
	safe_copy_cur_char();
	input = copy_int(input);	/* NumBits */
	chkcont(input, max_input, old_symtab_index);
	if (c == 'c') {
	    input = copy_conditions(input);
	    input = copy_char(input, ',', prod);
	    chkcont(input, max_input, old_symtab_index);
	}
	input = copy_fieldlist(input);
	/* BAD GRAMMAR */
	if (*input == ';')
	    input = copy_char(input, ';', prod);
	break;

      case 'Y':				/* C++ class */
	/* Check for PLACEHOLDER declaration, which is one
	   of "0s(;", "0u(;", or  "0c(;". */
	if (input[0] == '0'
	    && input[2] == '('
	    && input[3] == ';') {
	    output[-1] = input[1];	/* Overwrite 'Y' with ClassKey.
					   Nothing else is saved. */
	    forward_ref_seen = 'p';
	    return input + 4;
	}

	input = copy_int(input);	/* NumBytes */
	copy_cur_char();		/* ClassKey: should be s,u,or c, but
					   we don't check. */
	if (*input == 'V')		/* OptPBV */
	    safe_copy_cur_char();
	if (*input != '(')
	    input = copy_optbasespeclist(input);
	input = copy_char(input, '(', prod);
	input = copy_extendedfieldlist(input);
	if (*input == ')') {		/* OptNameResolutionList */
	    safe_copy_cur_char();
	    input = copy_nameresolutionlist(input);
	}
	input = copy_char(input, ';', prod);
	break;

      default:
	--input,--output;		/* Make input point to bad character */

	input = stab_error(input, NLSMSG(STAB_ERROR1,
 "%1$s: 0711-382 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n"
 "\tUnexpected character in production %4$s."),
			   STAB_ERROR_SHOW_NEXT_CHARACTER, prod);
    }
    return input;
} /* copy_typedef */
/************************************************************************
 * Name: copy_enumspec
 *									*
 * PRODUCTIONS:
 *	EnumSpec	: EnumList
 *			| TypeID ':' EnumList
 *	EnumList	: Enum
 *			| EnumList Enum
 *	Enum		: NAME ':' OrdValue/INTEGER ',' <cont>
 *									*
 ************************************************************************/
static char *
copy_enumspec(char *input)
{
    static char prod[] = "EnumSpec";

    DEBUG_MSG(STAB_DEBUG, (SAY_NO_NLS,
			   "Entering %s, stab = %.60s", prod, input));

    if (*input == '-' || l_isdigit(*input)) {
	input = copy_typeid(input, NULL);
	input = copy_char(input, ':', prod);
    }
    while (input < max_input && *input != ';') {
	input = copy_name(input);
	input = copy_char(input, ':', prod);
	input = copy_int(input);	/* OrdValue */
	input = copy_char(input, ',', prod);
	chkcont(input, max_input, old_symtab_index);
    }
    return input;
}
/************************************************************************
 * Name: copy_bound
 *									*
 * PRODUCTIONS:
 *	Bound		: INTEGER
 *			| Boundtype INTEGER
 *			| 'J'
 *	Boundtype	: 'A' | 'S' | 'T' | 'a' | 't'
 *									*
 ************************************************************************/
static char *
copy_bound(char *input)
{
    static char prod[] = "Bound";

    DEBUG_MSG(STAB_DEBUG, (SAY_NO_NLS,
			   "Entering %s, stab = %.60s", prod, input));

    switch(*input) {
      case 'S':
	/* In dbx, the run-time address of the dynamic bound is saved in
	   the structure containing this type.  Since different instances
	   of this type could have the save static-section offsets but
	   different run-time addresses, combining types with this
	   bound causes a problem.  Therefore, all bounds of this type are
	   kept distinct. */
	unique_type = 1;
	/* Fall through */
      case 'A':
      case 'T':
      case 'a':
      case 't':
	safe_copy_cur_char();
	/* Fall through */

      case '0':case '1':case '2':case '3':case '4':
      case '5':case '6':case '7':case '8':case '9':
      case '-':
	input = copy_int(input);
	break;

      case 'J':
	safe_copy_cur_char();
	break;
      default:
	input = stab_error(input, NLSMSG(STAB_ERROR1,
 "%1$s: 0711-382 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n"
 "\tUnexpected character in production %4$s."),
			   STAB_ERROR_SHOW_NEXT_CHARACTER, prod);
    }
    return input;
} /* copy_bound */
/************************************************************************
 * Name: copy_tparamlist
 *									*
 * PRODUCTIONS:
 *	TParamList	: TParam
 *			| TParamList TParam
 *	TParam		: TypeID ',' PASSBY ';' <cont>
 *
 * Note:  TParamList is always followed by ';'.
 ************************************************************************/
static char *
copy_tparamlist(char *input)
{
    static char prod[] = "TParamList";

    while (input < max_input && *input != ';') {
	input = copy_typeid(input, NULL);
	input = copy_char(input, ',', prod);
	input = copy_int(input);	/* PassBy */
	input = copy_char(input, ';', prod);
	chkcont(input, max_input, old_symtab_index);
    }
    return input;
}
/************************************************************************
 * Name: copy_field
 *									*
 * PRODUCTIONS:
 *	Field		: NAME : TypeID , BITOFFSET , NUMBITS ; <cont>
 *
 ************************************************************************/
static char *
copy_field(char *input)
{
    static char prod[] = "Field";

    DEBUG_MSG(STAB_DEBUG, (SAY_NO_NLS,
			   "Entering %s, stab = %.60s", prod, input));

    input = copy_name(input);
    input = copy_char(input, ':', prod);
    input = copy_typeid(input, NULL);
    input = copy_char(input, ',', prod);
    input = copy_int(input);		/* BitOffset */
    input = copy_char(input, ',', prod);
    input = copy_int(input);		/* NumBits */
    input = copy_char(input, ';', prod);
    chkcont(input, max_input, old_symtab_index);
    return input;
}
/************************************************************************
 * Name: copy_fieldlist
 *									*
 * PRODUCTIONS:
 *	FieldList	: Field
 *			| FieldList Field
 *
 * Note:  FieldList can be followed by
 *		[, which begins VariantPart
 *		;, which terminates structures, unions,
 *		(, which begins the next VField
 *		], which terminates VariantPart
 *		\0, which terminates the stabstring, in case the final
 *			; was omitted.
 *									*
 ************************************************************************/
static char *
copy_fieldlist(char *input)
{
    while (input < max_input
	   && *input != ';'
	   && *input != '['
	   && *input != ']'
	   && *input != '(') {
	input = copy_field(input);
    }
    return input;
}
/************************************************************************
 * Name: copy_variantpart
 *									*
 * PRODUCTIONS:
 *	VariantPart	: [ Vtag VFieldList ]
 *	VTag		: ( Field
 *			| ( NAME : optTypeId ; <cont> // Looks like Field
 *	VFieldList	: VList
 *			| VFieldList VList
 *	VList		: VField
 *			| VField VariantPart
 *	VField		: ( VRangeList : ;
 *			| ( VRangeList : FieldList
 *	VRangeList	: VRange
 *			| VRangeList , VRange
 *	VRange		: b OrdValue/INTEGER
 *			| c OrdValue/INTEGER
 *			| e TypeID , OrdValue/INTEGER
 *			| i INTEGER
 *			| r TypeID ; <cont> Bound ; <cont> Bound
 *
 ************************************************************************/
static char *
copy_variantpart(char *input)
{
    static char prod[] = "VariantPart";

    DEBUG_MSG(STAB_DEBUG, (SAY_NO_NLS,
			   "Entering %s, stab = %.60s", prod, input));

    input = copy_char(input, '[', prod);
    input = copy_char(input, '(', prod);
    /* GRAMMAR CHANGE */
    if (*input != ':')
	input = copy_name(input);
    input = copy_char(input, ':', prod);
    if (l_isdigit(*input) || *input == '-') {
	input = copy_typeid(input, NULL);
	/* GRAMMAR CHANGE */
	if (*input == ',') {
	    /* Must be Field--get rest of production */
	    safe_copy_cur_char();
	    input = copy_int(input);	/* BitOffset */
	    input = copy_char(input, ',', prod);
	    input = copy_int(input);	/* NUMBITS */
	}
    }
    input = copy_char(input, ';', prod);
    chkcont(input, max_input, old_symtab_index);

    /* Copy VFieldList */
    while (*input == '(') {
	/* Copy vlist */
	safe_copy_cur_char();		/* Copy '(' */
	while (1) {
	    /* VRange */
	    switch(*input) {
	      case 'b':
	      case 'c':
	      case 'i':
		safe_copy_cur_char();
		input = copy_int(input);
		break;

	      case 'e':
		safe_copy_cur_char();
		input = copy_typeid(input, NULL);
		input = copy_char(input, ',', prod);
		input = copy_int(input);
		break;

	      case 'r':
		safe_copy_cur_char();
		input = copy_typeid(input, NULL);
		input = copy_char(input, ';', prod);
		chkcont(input, max_input, old_symtab_index);
		input = copy_bound(input);
		input = copy_char(input, ';', prod);
		chkcont(input, max_input, old_symtab_index);
		input = copy_bound(input);
		break;

	      default:
		input = stab_error(input, NLSMSG(STAB_ERROR1,
 "%1$s: 0711-382 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n"
 "\tUnexpected character in production %4$s."),
				   STAB_ERROR_SHOW_NEXT_CHARACTER, "VRange");
	    }

	    if (*input != ',')
		break;
	    safe_copy_cur_char();
	}
	input = copy_char(input, ':', prod);
	input = copy_fieldlist(input);
	if (*input == ';')
	    safe_copy_cur_char();
	else if (*input == '[')
	    input = copy_variantpart(input);
    }
    input = copy_char(input, ']', prod);
    return input;
} /* copy_variantpart */
/************************************************************************
 * Name: copy_conditions
 *									*
 * PRODUCTIONS:
 *	Conditions	: Condition
 *			  Conditions Condition
 *	Condition	: NAME ':' INTEGER '=' 'q' ConditionType ',' ValueList
 *					';' <cont>
 *	ConditionType	: ConditionPrimitive , KanjiChar/INTEGER
 *	ConditionPrimitive	: 'n' Sign DecimalSite/INTEGER
 *				| 'a'
 *				| 'f'
 *	Sign		: '+' | '-'
 *	ValueList	: Value
 *		 	| ValueList Value
 *	Value		: INTEGER ':' ARBITRARY <cont>
 *									*
 ************************************************************************/
static char *
copy_conditions(char *input)
{
    int len;
    static char prod[] = "Conditions";

    DEBUG_MSG(STAB_DEBUG, (SAY_NO_NLS,
			   "Entering %s, stab = %.60s", prod, input));

    while (input < max_input && *input != ';' && *input != ',') {
	input = copy_name(input);
	input = copy_char(input, ':', prod);
	input = copy_int(input);
	input = copy_char(input, '=', prod);
	input = copy_char(input, 'q', prod);
	if (*input == 'n') {		/* ConditionPrimitive */
	    safe_copy_cur_char();
	    copy_cur_char();		/* Sign: +|-|0 (not checked) */
	    input = copy_int(input);	/* DecimalSite */
	}
	else
	    copy_cur_char();		/* ConditionPrimitive: a|f */
	input = copy_char(input, ',', prod);
	input = copy_int(input);	/* KANJICHAR */
	input = copy_char(input, ',', prod);
	/* Copy ValueList */
	while (input < max_input && *input != ';') {
	    /* Length of string to follow */
	    if (!l_isdigit(*input)) {
		return stab_error(input,
				  NLSMSG(STAB_ERROR16,
 "%1$s: 0711-399 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n"
 "\tLength expected in production %4$s."),
				  STAB_ERROR_SHOW_NEXT_CHARACTER, "Value");
	    }

	    len = 0;
	    do {
		len = 10 * len + *input - '0';
		safe_copy_cur_char();
	    } while (l_isdigit(*input));

	    input = copy_char(input, ':', prod);
	    if (len <= max_input - input) {	/* Copy 'len' characters */
		memcpy(output, input, len);
		output += len;
		input += len;
	    }
	    else {
		return stab_error(input,
				  NLSMSG(STAB_ERROR8,
 "%1$s: 0711-389 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n"
 "\tUnexpected end of stabstring in production %4$s."),
				  STAB_ERROR_NO_NEXT_CHAR, prod);
	    }
	    chkcont(input, max_input, old_symtab_index);
	}
	input = copy_char(input, ';', prod); /* end of Condition */
	chkcont(input, max_input, old_symtab_index);
    }
    return input;
} /* copy_conditions */
/************************************************************************
 * Name: copy_usage
 *									*
 * PRODUCTIONS:
 *	Usage		: optRedefinition PICStorageType NumBits/INTEGER ,
 *				EditDescription , PicSize/INTEGER ;
 *			| optRedefinition PICStorageType NumBits/INTEGER ,
 *				EditDescription , PicSize/INTEGER
 *				, <cont> Condition ;
 *	optRedefinition	: 'r' NAME ','
 *			| ** empty **
 *	PICStorageType	: a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|s|t
 *	EditDescription	: STRING
 *			| INTEGER
 *									*
 ************************************************************************/
static char *
copy_usage(char *input)
{
    static char prod[] = "Usage";

    DEBUG_MSG(STAB_DEBUG, (SAY_NO_NLS,
			   "Entering %s, stab = %.60s", prod, input));

    if (*input == 'r') {		/* Check for optRedefinition */
	safe_copy_cur_char();
	input = copy_name(input);
	input = copy_char(input, ',', prod);
    }
    copy_cur_char();			/* PICStoragetype (not checked) */
    input = copy_int(input);		/* NumBits */
    input = copy_char(input, ',', prod);
    if (l_isdigit(*input))		/* EditDescription */
	input = copy_int(input);
    else
	input = copy_dbx_string(input);
    input = copy_char(input, ',', prod);
    input = copy_int(input);		/* PicSize */
    if (*input == ',') {		/* Condition included */
	safe_copy_cur_char();
	chkcont(input, max_input, old_symtab_index);
	input = copy_conditions(input);
    }
    input = copy_char(input, ';', prod); /* end of Usage */
    return input;
} /* copy_usage */
/************************************************************************
 * Name: copy_namedtparamlist
 *									*
 * PRODUCTIONS:
 *	NamedTParamList	: ** empty **
 *			| NamedTPList
 *	NamedTPList	: NamedTParam
 *			| NamedTPList NamedTParam
 *	NamedTParam	: NAME : TypeID , PASSBY InitBody ; <cont>
 *			| : TypeID , PASSBY InitBody ; <cont>
 *	PassBy		: INTEGER
 *	InitBody	: STRING
 *			| ** empty **
 *
 * Note:  This production is always followed by a semicolon.
 *									*
 ************************************************************************/
static char *
copy_namedtparamlist(char *input)
{
    static char prod[] = "NamedTParamList";

    DEBUG_MSG(STAB_DEBUG, (SAY_NO_NLS,
			   "Entering %s, stab = %.60s", prod, input));

    while(*input && *input != ';') {
	if (*input != ':')
	    input = copy_name(input);
	input = copy_char(input, ':', prod);
	input = copy_typeid(input, NULL);
	input = copy_char(input, ',', prod);
	input = copy_int(input);	/* PassBy */
#if 0
	/* InitBody not processed by dbx */
	if (*input != ';')
	    input = copy_dbx_string(input);
#endif
	input = copy_char(input, ';', prod);
	chkcont(input, max_input, old_symtab_index);
    }
    return input;
} /* copy_namedtparamlist */
/************************************************************************
 * Name: copy_optbasespeclist
 *									*
 * PRODUCTIONS:
 *	OptBaseSpecList		: ** empty ** (empty alternative handled
 *						by caller)
 *				| BaseSpecList
 *	BaseSpecList		: BaseSpec
 *				| BaseSpecList ',' BaseSpec
 *	BaseSpec		: VirtualAccessSpec <cont> BaseClassOffset ':'
 *					ClassTypeId
 *	BaseClassOffset		: INTEGER
 *	ClassTypeId		: TypeID
 *	VirtualAccessSpec	: v AccessSpec                  * virtual *
 *				| AccessSpec
 *	AccessSpec		: i                             * private *
 *				| o                             * protected *
 *				| u                             * public *
 *
 ************************************************************************/
static char *
copy_optbasespeclist(char *input)
{
    static char prod[] = "OptBaseSpecList";

    DEBUG_MSG(STAB_DEBUG, (SAY_NO_NLS,
			   "Entering %s, stab = %.60s", prod, input));

    while(1) {
	if (*input == 'v')		/* Virtual */
	    safe_copy_cur_char();
	copy_cur_char();		/* AccessSpec: i|o|u (not checked) */
	chkcont(input, max_input, old_symtab_index);
	input = copy_int(input);	/* BaseClassOffset */
	input = copy_char(input, ':', prod);
	input = copy_typeid(input, NULL); /* ClassTypeID */
	if (*input != ',')
	    break;
	safe_copy_cur_char();
    }
    return input;
} /* copy_optbasespeclist */
/************************************************************************
 * Name: copy_extendedfieldlist
 *									*
 * PRODUCTIONS:
 *	ExtendedFieldList	: ** empty **
 *				| ExtendedFieldList ExtendedField
 *	ExtendedField		: (AnonSpec) GenSpec (VirtualSpec) AccessSpec
 *				  AnonSpec DataMember
 *				| (AnonSpec) GenSpec VirtualSpec AccessSpec
 *				  (AnonSpec) OptVirtualFuncIndex MemberFunction
 *				| (AnonSpec) (GenSpec) (VirtualSpec) AccessSpec
 *				  AnonSpec NestedClass
 *				| AnonSpec FriendClass
 *				| AnonSpec FriendFunction
 *	GenSpec			: 'c' | **EMPTY**
 *	AccessSpec		: 'i' <cont> | 'o' <cont> | 'u' <cont>
 *	AnonSpec		: 'a' | **EMPTY**
 *	VirtualSpec		: 'v' 'p'		* pure virtual *
 *				| 'v'			* virtual *
 *				| ** empty **
 *	DataMember		: MemberAttrs : Field
 *	MemberAttrs		: IsStatic IsVtblPtr IsVBasePtr
 *	IsStatic		: 's' | **EMPTY**
 *	IsVtblPtr		: 'p' INTEGER NAME | **EMPTY**
 *	IvVbasePtr		: 'b' | 'r' | **EMPTY**
 *	MemberFunction		: '[' FuncType MemberFuncAttrs ':' NAME
 *					 ':' TypeID ';' <cont>
 *	MemberFuncAttrs		: IsStatic IsInline IsConst IsVolatile
 *	IsInline		: 'i' | **EMPTY**
 *	IsConst			: 'k' | **EMPTY**
 *	IsVolatile		: 'V' | **EMPTY**
 *	NestedClass		: 'N' TypeID ';' <cont>
 *	FriendClass		: '(' TypeID ';' <cont>
 *	FriendFunction		: ']' NAME ':' TypeID ';' <cont>
 *	OptVirtualFuncIndex	: INTEGER | **EMPTY**
 *	FuncType		: 'f' | 'c' | 'd'
 *
 ************************************************************************/
static char *
copy_extendedfieldlist(char *input)
{
    static char prod[] = "ExtendedFieldList";

    DEBUG_MSG(STAB_DEBUG, (SAY_NO_NLS,
			   "Entering %s, stab = %.60s", prod, input));

    while (input < max_input && *input != ';' && *input != ')') {
	DEBUG_MSG(STAB_DEBUG, (SAY_NO_NLS,
			       "In %s, stab = %.60s", prod, input));

	if (*input == 'a')
	    safe_copy_cur_char();

	switch(*input) {
	  case '(':			/* FriendClass */
	    safe_copy_cur_char();
	    input = copy_typeid(input, NULL);
	    input = copy_char(input, ';', prod);
	    chkcont(input, max_input, old_symtab_index);
	    continue;

	  case ']':			/* FriendFunction */
	    safe_copy_cur_char();
	    input = copy_name(input);
	    input = copy_char(input, ':', prod);
	    input = copy_typeid(input, NULL);
	    input = copy_char(input, ';', prod);
	    chkcont(input, max_input, old_symtab_index);
	    continue;

	  default:
	    break;
	}

	/* GenSpec */
	if (*input == 'c')
	    safe_copy_cur_char();

	/* VirtualSpec */
	if (*input == 'v') {
	    safe_copy_cur_char();
	    if (*input == 'p') {
		safe_copy_cur_char();
	    }
	}

	copy_cur_char();		/* AccessSpec: i|o|u (not checked) */
	chkcont(input, max_input, old_symtab_index);

	if (*input == 'a')
	    safe_copy_cur_char();	/* AnonSpec */

	if (*input == 'N') {		/* Nested Class */
	    /* If virtual_spec or gen_spec was seen, dbx reports an error.
	       We don't do that check here. */
	    safe_copy_cur_char();
	    input = copy_typeid(input, NULL);
	    input = copy_char(input, ';', prod);
	    chkcont(input, max_input, old_symtab_index);
	}
	else {
	    if (l_isdigit(*input))
		input = copy_int(input); /* OptVirtualFuncIndex */
	    if (*input == '[') {	/* MemberFunction */
		safe_copy_cur_char();
		copy_cur_char();	/* FuncType: f|c|d (not checked) */
		while (1) {
		    /* MemberFuncAttrs */
		    switch(*input) {
		      case 's':		/* IsStatic */
		      case 'i':		/* IsInline */
		      case 'k':		/* IsConst */
		      case 'V':		/* IsVolatile */
			safe_copy_cur_char();
			break;
		      default:
			goto leave_loop;
		    }
		}
	      leave_loop:
		input = copy_char(input, ':', prod);
		input = copy_name(input);
		input = copy_char(input, ':', prod);
		input = copy_typeid(input, NULL);
		input = copy_char(input, ';', prod);
		chkcont(input, max_input, old_symtab_index);
	    }
	    else {
		/* DataMember */
		while (1) {
		    /* MemberAttrs */
		    switch(*input) {
		      case 's':		/* IsStatic */
		      case 'b':		/* IsVBasePtr */
		      case 'r':		/* IsVBasePtr (self-pointer) */
			safe_copy_cur_char();
			break;
		      case 'p':		/* IsVolatile */
			safe_copy_cur_char();
			input = copy_int(input);
			input = copy_name(input);
			goto leave_loop2;
		      default:
			goto leave_loop2;
		    }
		}
	      leave_loop2:
		input = copy_char(input, ':', prod);
		input = copy_field(input);
	    }
	}
    }
    return input;
} /* copy_extendedfieldlist */
/************************************************************************
 * Name: copy_nameresolutionlist
 *									*
 * PRODUCTIONS:
 *	NameResolutionList	: NameResolution
 *				| NameResolution ',' NameResolutionList
 *	NameResolution		: MemberName ':' ClassTypeID
 *				| MemberName ':'
 *	MemberName		: NAME
 *	ClassTypeID		: TypeID
 *
 ************************************************************************/
static char *
copy_nameresolutionlist(char *input)
{
    static char prod[] = "NameResolutionList";

    DEBUG_MSG(STAB_DEBUG, (SAY_NO_NLS,
			   "Entering %s, stab = %.60s", prod, input));

    while (1) {
	input = copy_name(input);
	input = copy_char(input, ':', prod);
	if (l_isdigit(*input) || *input == '-')
	    input = copy_typeid(input, NULL);
	if (*input != ',')
	    break;
	safe_copy_cur_char();
    }
    return input;
}
/************************************************************************
 * Name: copy_real
 *									*
 * Purpose: Copy a signed floating (real) number
 *	dbx uses scanf() to find a real.  We don't do that, because we
 *	don't need the converted value.
 *									*
 * PRODUCTION:
 *	REAL:	[+-][0-9]+(.[0-9]*)([eEqQ](+-)[0-9]+)
 *		(+-)INF
 *		QNAN
 *		SNAN
 * Note: Optional white space is allowed before the number
 ************************************************************************/
static char *
copy_real(char *input)
{
    /* Skip leading white space */
    while (l_isspace(*input))
	safe_copy_cur_char();

    /* Look for SNAN, QNAN, [+-], I, or a digit */
    switch(*input) {
      case 'S':
      case 'Q':
	if (strcmp(&input[1], "NAN") == 0) {
	    safe_copy_cur_char();
	  copy3:
	    safe_copy_cur_char();
	    safe_copy_cur_char();
	    safe_copy_cur_char();
	    return input;
	}
	else
	    goto no_real;

      case '0': case '1':   case '2':   case '3':   case '4':
      case '5': case '6':   case '7':   case '8':   case '9':
	break;

      case '+':
      case '-':
	safe_copy_cur_char();
	if (strcmp(input, "INF") == 0)
	    goto copy3;
	if (!(l_isdigit(*input)))
	    goto no_real;
	break;

      case 'I':
	if (strcmp(&input[1], "NF") == 0) /* Check for INF */
	    goto copy3;
	goto no_real;

      default:
      no_real:
	return stab_error(input, NLSMSG(STAB_ERROR4A,
 "%1$s: 0711-381 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n"
 "\tReal expected."),
			  STAB_ERROR_SHOW_NEXT_CHARACTER, NULL);
    }

    /* Get whole portion */
    do {
	safe_copy_cur_char();		/* Copy digits */
    } while (l_isdigit(*input));

    /* Get fraction */
    if (*input == '.') {
	safe_copy_cur_char();
	while (l_isdigit(*input))
	    safe_copy_cur_char();
    }

    /* Get exponent */
    switch(*input) {
      case 'e':
      case 'E':
      case 'q':
      case 'Q':
	safe_copy_cur_char();		/* Copy exponent character */
	break;
      default:
	return input;
    }

    if (*input == '+' || *input == '-')
	safe_copy_cur_char();		/* Copy optional sign */
    if (!l_isdigit(*input)) {
	return stab_error(input, NLSMSG(STAB_ERROR15,
 "%1$s: 0711-398 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n"
 "\tExponent expected."),
			  STAB_ERROR_SHOW_NEXT_CHARACTER, NULL);
    }
    do {
	safe_copy_cur_char();		/* Copy exponent */
    } while (l_isdigit(*input));
    return input;
} /* copy_real */
/************************************************************************
 * Name: copy_int
 *									*
 * Purpose: Copy a signed integer
 *
 * SYNTAX:	-?[0-9]+
 *									*
 ************************************************************************/
static char *
copy_int(char *input)
{
    if (*input == '-')
	safe_copy_cur_char();		/* Optional sign */
    if (!l_isdigit(*input)) {
	return stab_error(input, NLSMSG(STAB_ERROR4,
 "%1$s: 0711-385 STABSTRING ERROR: Symbol table entry %2$d, object file %3$s\n"
 "\tInteger expected."),
			  STAB_ERROR_SHOW_NEXT_CHARACTER, NULL);
    }
    do {
	safe_copy_cur_char();		/* Copy digits */
    } while (l_isdigit(*input));
    return input;
}
