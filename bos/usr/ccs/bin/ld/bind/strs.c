#if lint == 0 && CENTERLINE == 0
#pragma comment (copyright, "@(#)06	1.10  src/bos/usr/ccs/bin/ld/bind/strs.c, cmdld, bos411, 9428A410j 1/28/94 13:06:24")
#endif

/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS: total_STRS
 *		hash
 *		lookup_stringhash
 *		lookup_dotname
 *		putstring
 *		putstring_len
 *		str_stats
 *		TYPECHK_stats
 *
 *   STATIC FUNCTIONS:
 *		hash_maxlen
 *		new_TYPECHK
 *		new_init_HASH_STR
 *		new_init_STR
 *		TYPECHK_hash
 *		put_TYPECHK
 *
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

#include <stdlib.h>
#include <string.h>
#include <sys/ltypes.h>

#include "global.h"
#include "bind.h"
#include "error.h"
#include "strs.h"
#include "util.h"
#include "stats.h"

/* Parameters for hashing algorithm */
#define HASHCONS		65599
#define STR_HASH_TABLE_SIZE	677
#define TYPECHK_HASH_TABLE_SIZE	677

/* Parameters for STR  and TYPECHK routines */
#define STR_BUMP	512
#define TYPECHK_BUMP	256

#define MAX_STR_LEN	65535

/* Static variables */
static HASH_STR	*strings_hash[STR_HASH_TABLE_SIZE];
static TYPECHK	*typechks_hash[TYPECHK_HASH_TABLE_SIZE];
static int	STRS_in_use = 0;
static STR	INITIALIZE_STR = {NULL, {NULL, NULL, NULL}, NULL, 0, 0, 0};

/* Global variables */
STR NULL_STR = {
	"",				/* Name */
	{(SYMBOL *)NULL, (SYMBOL *)NULL, (SYMBOL *)NULL}, /* Defs and refs */
	(STR *)NULL,			/* Alternate (dot-name or plain-name)*/
	0, 0, 0				/* Value, length, flags */
	};
HASH_STR HASH_STR_root = {{NULL, {NULL}, NULL, 0, 0, 0}, NULL};
STR STR_root = {NULL, {NULL}, NULL, 0, 0, 0};

static unsigned char null_hash[4] = {'\0', '\0', '\0', '\0'};
unsigned char universal_hash[4] = {' ', ' ', ' ', ' '};

/************************************************************************
 * Name: total_STRS
 *
 * Returns: Total number of STRs allocated
 ************************************************************************/
int
total_STRS(void)
{
    return STRS_in_use;
}
/************************************************************************
 * Name: hash								*
 *									*
 * Generate a hash value for a given string.
 *
 * NOTE:  The hashing algorithm works for strings that contain nulls or
 *	  that are not null-terminated.
 ************************************************************************/
int
hash(char *strng,			/* string to be hashed */
     int len,				/* length of string */
     int table_size)			/* size of hash table */
{
    unsigned int	sum = 0;

    while (len-- > 0)
	sum = (sum * HASHCONS) + (unsigned int) *strng++;
    return sum % table_size;
}
/************************************************************************
 * Name: hash								*
 *									*
 * Generate a hash value for a given string.
 *
 * Note:  This routine only hashes strings up to *maxlen characters or
 *		to the first null.  The actual length hashed (not counting
 *		the null) is assigned to *maxlen.
 ************************************************************************/
static int
hash_maxlen(char *strng,		/* string to be hashed */
	    int *maxlen,		/* maximum length of string--actual
					   length is assigned  */
	    int table_size)		/* size of hash table */
{
    unsigned int	byte;
    int			maxl, l;
    unsigned int	sum = 0;

    maxl = l = *maxlen;
    for (; l && (byte = (unsigned int)*strng++); l--)
	sum = (sum * HASHCONS) + byte;
    *maxlen = maxl - l;
    return sum % table_size;
}
/************************************************************************
 * Name: lookup_stringhash
 *									*
 * Returns: The STR structure for the name "name"
 *	If no definition for "name" exists, return NULL.
 *
 * NOTE:  Names beginning with exactly one '.' are treated in a special
 *	way.  A symbol table entry for the name without the dot is created,
 *	and the "alternate" field is used to point to the symbol for the
 *	name with the dot.
 *
 ************************************************************************/
STR *
lookup_stringhash(char *name)
{
    int l = MAX_STR_LEN;		/* Max possible length */
    int h;
    HASH_STR *sh;

    if (name[0] == '\0')
	return &NULL_STR;
    else if (name[0] == '.' && name[1] != '.') /* Exactly one '.'? */
	return lookup_dotname(name+1);

    h = hash_maxlen(name, &l, STR_HASH_TABLE_SIZE);
    for (sh = strings_hash[h]; sh; sh = sh->next)
	if (sh->s.len == l && memcmp(name, sh->s.name, l) == 0)
	    return &sh->s;		/* Found it */

    return NULL;
}
/************************************************************************
 * Name: lookup_dotname
 *									*
 * Returns: The STR structure for the name "name" with a '.' prepended.
 *	If no definition for ".name" exists, return NULL.
 *
 * This function is just like lookup_stringhash, except for the '.'  The
 *	'.' should not be a part of the name parameter.
 ************************************************************************/
STR *
lookup_dotname(char *name)
{
    int l = MAX_STR_LEN - 1;		/* Max possible length */
    int h;
    HASH_STR *sh;

    if (name[0] == '\0')
	return NULL_STR.alternate;

    h = hash_maxlen(name, &l, STR_HASH_TABLE_SIZE);
    for (sh = strings_hash[h]; sh; sh = sh->next)
	if (sh->s.len == l && memcmp(name, sh->s.name, l) == 0)
	    return sh->s.alternate;	/* Found it */

    return NULL;
}
/************************************************************************
 * Name: new_init_HASH_STR
 *									*
 * Returns: Return a new HASH_STR structure, with the STR structure
 * initialized.
 *
 ************************************************************************/
static HASH_STR *
new_init_HASH_STR(void)
{
    char *id = "new_init_HASH_STR";

    static int HASH_STRs_left = 0;
    static HASH_STR *available_HASH_STRs;
    static HASH_STR *current_HASH_STR_block = &HASH_STR_root;

    if (HASH_STRs_left == 0) {
	/* Allocate new block of HASH_STRs */
	available_HASH_STRs
	    = get_memory(sizeof(HASH_STR), STR_BUMP+1, STRS_ID, id);
	current_HASH_STR_block->next = available_HASH_STRs;
	available_HASH_STRs->next = NULL;
	available_HASH_STRs->s.len = 0;
	HASH_STRs_left = STR_BUMP;
	current_HASH_STR_block = available_HASH_STRs++;
	HASH_STRs_left = STR_BUMP;
    }
    STAT_use(STRS_ID, 1);
    STRS_in_use++;
    HASH_STRs_left--;
    current_HASH_STR_block->s.len++;

    available_HASH_STRs->s = INITIALIZE_STR;

    return available_HASH_STRs++;
}
/************************************************************************
 * Name: new_init_STR
 *									*
 * Returns: Return a new, initialized STR structure.
 *
 ************************************************************************/
static STR *
new_init_STR(void)
{
    char *id = "new_STR";

    static int STRs_left = 0;
    static STR *available_STRs;
    static STR *current_STR_block = &STR_root;

    if (STRs_left == 0) {
	/* Allocate new block of STRs */
	available_STRs = get_memory(sizeof(STR), STR_BUMP+1, STRS_ID, id);
	current_STR_block->alternate = available_STRs;
	available_STRs->alternate = NULL;
	available_STRs->len = 0;
	STRs_left = STR_BUMP;
	current_STR_block = available_STRs++;
	STRs_left = STR_BUMP;
    }
    STAT_use(STRS_ID, 1);
    STRS_in_use++;
    STRs_left--;
    current_STR_block->len++;

    *available_STRs = INITIALIZE_STR;

    return available_STRs++;
}
/************************************************************************
 * Name: Save a new string in a STR structure, if new.  Otherwise,
 *	return the existing STR structure for the name.
 *									*
 * Returns: Return a new, initialized STR structure.
 *
 ************************************************************************/
STR *
putstring(char *strng)			/* Name (null-terminated) */
{
    int		h;
    int		dot_name;
    int		use_plain_name;
    int		str_len;
    STR		*s_plain, *s_dot;
    HASH_STR	*sh;

    if (strng[0] == '\0')
	return &NULL_STR;

    str_len = MAX_STR_LEN;		/* Max string length is 0xFFFF */

    dot_name = 0;
    use_plain_name = 0;
    if (strng[0] == '.') {
	if (strng[1] == '.')
	    use_plain_name = 1;		/* Don't prepend saved plain name with
					   a dot. */
	else {
	    dot_name = 1;
	    str_len--;
	    strng++;
	}
    }

    h = hash_maxlen(strng, &str_len, STR_HASH_TABLE_SIZE);

    if (str_len == MAX_STR_LEN - dot_name && strng[str_len] != '\0') {
	bind_err(SAY_NORMAL, RC_ERROR,
		 NLSMSG(STRING_TOO_LONG,
			"%1$s: 0711-950 ERROR: A name is too long.\n"
			"\tNames longer than 65535 characters are truncated.\n"
			"\tThe name begins: %2$-45.45s"),
		 Main_command_name, strng);
    }

    for (sh = strings_hash[h]; sh; sh = sh->next) {
	if (sh->s.len == str_len && memcmp(strng, sh->s.name, str_len) == 0) {
	    s_plain = &sh->s;
	    if (dot_name)
		if (s_plain->alternate != NULL)
		    return s_plain->alternate; /* duplicate */
		else
		    goto handle_dot_name;
	    return s_plain;		/* duplicate */
	}
    }

    /* New string */
    sh = new_init_HASH_STR();
#ifdef DEBUG
    sh->hash = h;
#endif
    /* Add HASH_STR to chain of HASH_STRs with this hash value */
    sh->next = strings_hash[h];
    strings_hash[h] = sh;

    s_plain = &sh->s;
    s_plain->len = str_len;
    if (use_plain_name)
	s_plain->name = save_string(strng, str_len + 1 /* for '\0' */);
    else
	s_plain->name = 1 + save_dotted_string(strng,
					       str_len + 1 /* for '\0' */);

    /* Write terminating null in case string was longer than
       MAX_STR_LEN chars.  (Usually, the null will already be there.)  */
    s_plain->name[str_len] = '\0';
    /* Keep track of maximum number of strings that could possibly be
       written to symbol table string area in output file */
    if (str_len > SYMNMLEN)
	Bind_state.num_long_strings++;

    if (dot_name) {
      handle_dot_name:
	/* The plain name STR exists; we use its name for the dotted name. */
	s_dot = s_plain->alternate = new_init_STR();
	s_dot->alternate = s_plain;
	s_dot->len = str_len + 1;
	s_dot->name = s_plain->name - 1;
	if (str_len+1 > SYMNMLEN)
	    Bind_state.num_long_strings++;
	return s_dot;
    }
    else
	return s_plain;

} /* putstring */
/************************************************************************
 * Name: Save a new string in a STR structure, if new.  Otherwise,
 *	return the existing STR structure for the name.
 *									*
 * Returns: Return a new, initialized STR structure.
 *
 * This function is identical to putstring, except that the length of the
 *	name is limited.
 ************************************************************************/
STR *
putstring_len(char *strng,
	      int max_len)
{
    int		h;
    int		dot_name;
    int		use_plain_name;
    int		str_len = max_len;
    STR		*s_plain, *s_dot;
    HASH_STR	*sh;

    if (str_len == 0 || strng[0] == '\0')
	return &NULL_STR;

    str_len = min(MAX_STR_LEN, str_len); /* Max string length is 0xFFFF */

    dot_name = 0;
    use_plain_name = 0;
    if (strng[0] == '.') {
	if (strng[1] == '.')
	    use_plain_name = 1;		/* Don't prepend saved plain name with
					   a dot. */
	else {
	    dot_name = 1;
	    str_len--;
	    strng++;
	}
    }

    h = hash_maxlen(strng, &str_len, STR_HASH_TABLE_SIZE);

    if (str_len == MAX_STR_LEN - dot_name
	&& max_len > str_len
	&& strng[str_len] != '\0') {
	bind_err(SAY_NORMAL, RC_ERROR,
		 NLSMSG(STRING_TOO_LONG,
			"%1$s: 0711-950 ERROR: A name is too long.\n"
			"\tNames longer than 65535 characters are truncated.\n"
			"\tThe name begins: %2$-45.45s"),
		 Main_command_name, strng);
    }

    for (sh = strings_hash[h]; sh; sh = sh->next) {
	if (sh->s.len == str_len && memcmp(strng, sh->s.name, str_len) == 0) {
	    s_plain = &sh->s;
	    if (dot_name)
		if (s_plain->alternate != NULL)
		    return s_plain->alternate; /* duplicate */
		else
		    goto handle_dot_name;
	    return s_plain; /* duplicate */
	}
    }

    /* New string */
    sh = new_init_HASH_STR();
#ifdef DEBUG
    sh->hash = h;
#endif
    /* Add HASH_STR to chain of HASH_STRs with this hash value */
    sh->next = strings_hash[h];
    strings_hash[h] = sh;

    s_plain = &sh->s;
    s_plain->len = str_len;
    if (use_plain_name)
	s_plain->name = save_string(strng, str_len + 1 /* for '\0' */);
    else
	s_plain->name = 1 + save_dotted_string(strng,
					       str_len + 1 /* for '\0' */);

    /* Write terminating null in case string was longer than
       MAX_STR_LEN chars.  (Usually, the null will already be there.)  */
    s_plain->name[str_len] = '\0';
    /* Keep track of maximum number of strings that could possibly be
       written to symbol table string area in output file */
    if (str_len > SYMNMLEN)
	Bind_state.num_long_strings++;

    if (dot_name) {
      handle_dot_name:
	/* The plain name STR exists; we use its name for the dotted name. */
	s_dot = s_plain->alternate = new_init_STR();
	s_dot->alternate = s_plain;
	s_dot->len = str_len + 1;
	s_dot->name = s_plain->name - 1;
	if (str_len+1 > SYMNMLEN)
	    Bind_state.num_long_strings++;
	return s_dot;
    }
    else
	return s_plain;
}
#ifdef STATS
#define MAX_CHAIN_EXAMINED 40
/************************************************************************
 * Name: str_stats
 *									*
 * Purpose:
 *									*
 * Function:
 ************************************************************************/
void
str_stats(void)
{
    int		chain_lengths[MAX_CHAIN_EXAMINED];
    int		h_syms = 0, hk;
    int		h_used = 0;
    int		i, j;
    int		max_chain_length = 0;
    float	h_actdist;		/* actual distribution value */
    float	h_ratio;		/* ratio actual/random */
    float	hkf, h_fsyms;
    float	h_randist;		/* random distribution value */
    HASH_STR	*sh;

    for (i = 0; i < MAX_CHAIN_EXAMINED; i++)
	chain_lengths[i] = 0;

    /* ***************************** *
     * Hashing function diagnostic   *
     * information                   *
     * *******************************/
    h_actdist = 0.0;
    for (i=0, h_syms=0, h_used=0; i < STR_HASH_TABLE_SIZE; ++i) {
	if (strings_hash[i])
	    h_used++;
	/* calculate distribution of hash entries */
	hk = 0;
	for (sh = strings_hash[i]; sh; sh = sh->next) {
	    ++hk; ++h_syms;
	}
	if (hk >= MAX_CHAIN_EXAMINED)
	    chain_lengths[MAX_CHAIN_EXAMINED-1]++;
	else
	    chain_lengths[hk]++;
	max_chain_length = max(max_chain_length, hk);
	hkf = (float)hk;
	h_actdist = h_actdist + ( hkf * ((hkf+1.0)/2.0));
    }

    /* Static count information */
    say(SAY_NL_ONLY);
    if (h_syms == 0)
	say(SAY_NORMAL,
	    NLSMSG(STRING_NO_STATS, "%s: String statistics:  No strings."),
	    Command_name);
    else {
	say(SAY_NORMAL, NLSMSG(STRING_STATS_HDR, "%s: String statistics:"),
	    Command_name);
	h_fsyms = (float)h_syms;
	h_randist=(h_fsyms/(float)(2 * STR_HASH_TABLE_SIZE))
	    * (h_fsyms+(float)(2 * STR_HASH_TABLE_SIZE - 1));
	h_ratio = h_actdist / h_randist;
	say(SAY_NO_NL, NLSMSG(HASH_INFO,
"\tBuckets used=%1$d of %2$d Maximum-bucket-size=%3$d Strings=%4$d\n"
"\tActual distribution=%5$.1f Random distribution=%6$.1f Ratio=%7$.3f\n"
	"\tBucket sizes: "),
	    h_used, STR_HASH_TABLE_SIZE, max_chain_length, h_syms,
	    h_actdist, h_randist, h_ratio);
	j = min(MAX_CHAIN_EXAMINED-1, max_chain_length);
	hk = 0;
	h_syms = 0;
	for (i = 1; i < j;  i++)
	    say(SAY_NO_NLS | SAY_NO_NL, "%d,", chain_lengths[i]);
	say(SAY_NO_NLS, "%d", chain_lengths[i]);
    }
} /* str_stats */
/************************************************************************
 * Name: TYPECHK_stats
 *									*
 * Purpose:
 *									*
 * Function:
 ************************************************************************/
void
TYPECHK_stats(void)
{
    int		h_used = 0;
    int		max_chain_length = 0;
    int		chain_lengths[MAX_CHAIN_EXAMINED];
    int		h_syms = 0, hk;
    int		i, j;
    TYPECHK	*s;
    float	h_ratio,        /* ratio actual/random */
		h_randist,      /* random distribution value */
		h_actdist,      /* actual distribution value */
		hkf,
		h_fsyms;

    for (i = 0; i < MAX_CHAIN_EXAMINED; i++)
	chain_lengths[i] = 0;

    /* ***************************** *
     * Hashing function diagnostic   *
     * information                   *
     * *******************************/
    h_actdist = 0.0;
    for (i=0, h_syms=0, h_used=0; i < TYPECHK_HASH_TABLE_SIZE; ++i) {
	if (typechks_hash[i])
	    h_used++;
	/* calculate distribution of hash entries */
	hk = 0;
	for (s=typechks_hash[i]; s; s = s->t_next) {
	    ++hk; ++h_syms;
	}
	if (hk >= MAX_CHAIN_EXAMINED)
	    chain_lengths[MAX_CHAIN_EXAMINED-1]++;
	else
	    chain_lengths[hk]++;
	max_chain_length = max(max_chain_length, hk);
	hkf = (float)hk;
	h_actdist = h_actdist + ( hkf * ((hkf+1.0)/2.0));
    }
    say(SAY_NL_ONLY);
    if (h_syms == 0)
	say(SAY_NORMAL,
	    NLSMSG(TYPECHK_NO_STATS,
	   "%s: Type-checking string statistics: No type-checking strings."),
	    Command_name);
    else {
	say(SAY_NORMAL,
	    NLSMSG(TYPECHK_STATS_HDR, "%s: Type-checking string statistics:"),
	    Command_name);
	h_fsyms = (float)h_syms;
	h_randist=(h_fsyms/(float)(2 * TYPECHK_HASH_TABLE_SIZE))
	    * (h_fsyms+(float)(2 * TYPECHK_HASH_TABLE_SIZE - 1));
	h_ratio = h_actdist / h_randist;

	/* Static count information */
	say(SAY_NO_NL, NLSMSG(HASH_INFO,
"\tBuckets used=%1$d of %2$d Maximum-bucket-size=%3$d Strings=%4$d\n"
"\tActual distribution=%5$.1f Random distribution=%6$.1f Ratio=%7$.3f\n"
	"\tBucket sizes: "),
	    h_used, TYPECHK_HASH_TABLE_SIZE, max_chain_length, h_syms,
	    h_actdist, h_randist, h_ratio);
	j = min(MAX_CHAIN_EXAMINED-1, max_chain_length);
	hk = 0;
	h_syms = 0;
	for (i = 1; i < j;  i++)
	    say(SAY_NO_NLS | SAY_NO_NL, "%d,", chain_lengths[i]);
	say(SAY_NO_NLS, "%d", chain_lengths[i]);
    }
} /* TYPECHK_stats */
#endif
/************************************************************************
 * Name: TYPECHK_hash
 *									*
 * Purpose:
 *									*
 * Function:
 ************************************************************************/
static int
TYPECHK_hash(caddr_t t,
	     size_t l,
	     int table_size)
{
    uint	h = 0;
    uint	*t1 = (uint *)t;

    while (l >= sizeof(uint)) {
	h += *t1++;
	l -= sizeof(int);
    }
    if (l > 0) {
	/* Pick up remaining bytes in typechk */
	t1 = (uint *)((int)t1 - sizeof(uint) + l);
	h += *t1;
    }
    return h % table_size;
}
/************************************************************************
 * Name: new_TYPECHK
 *									*
 * Purpose:
 *									*
 * Function:
 ************************************************************************/
static TYPECHK *
new_TYPECHK(void)
{
    char		*id = "new_TYPECHK";
    static int		typechks_left = 0;
    static TYPECHK	*available_typechks;

    if (typechks_left == 0) {
	available_typechks = get_memory(sizeof(TYPECHK),
					TYPECHK_BUMP,
					TYPECHKS_ID,
					id);
	typechks_left = TYPECHK_BUMP;
    }
    Bind_state.num_typechks++;
    typechks_left--;
    STAT_use(TYPECHKS_ID, 1);
    available_typechks->t_value = -1;
    return available_typechks++;
}
/************************************************************************
 * Name: put_TYPECHK
 *									*
 * Purpose:
 *									*
 * Function:
 ************************************************************************/
TYPECHK *
put_TYPECHK(caddr_t typechk,		/* Pointer to typechk string to save */
	    off_t max_len)
{
    int		h;
    size_t	typechk_len;
    TYPECHK	*t;
    TYPECHK	temp_t;

    typechk -= 2;			/* Subtract 2 to get at length */
    typechk_len = 256 * typechk[0] + typechk[1]; /* Add 2 for length field*/

    if (typechk_len > max_len) {
	bind_err(SAY_NORMAL, RC_SEVERE,
		 NLSMSG(TYPECHK_ERROR1,
 "%1$s: 0711-955 SEVERE ERROR: Length of type-checking string is invalid.\n"
 "\tThe string appears to extend beyond the end of the section."),
		 Main_command_name);
	return NULL;
    }

    /* Add 2 to length to make length part of the hash */
    h = TYPECHK_hash(typechk, typechk_len + 2, TYPECHK_HASH_TABLE_SIZE);
    if (typechk_len != TYPCHKSZ) {
	/* Nonstandard TYPECHK */
	for (t = typechks_hash[h]; t; t = t->t_next) {
	    if (typechk_len == t->t_len)
		if (memcmp(typechk,
			   typechk_len <= sizeof(TYPECHK)
			   ? t->t_c_typechk
			   : t->t_cp_typechk,
			   typechk_len) == 0)
		    return t;
	}
	/* New hash */
	t = new_TYPECHK();
	t->t_len = typechk_len;
	if (typechk_len <= sizeof(TYPECHK))
	    memcpy(t->t_c_typechk, typechk, typechk_len);
	else
	    t->t_cp_typechk = save_string(typechk, typechk_len);
    }
    else {
	/* Standard TYPECHK.  Value saved in TYPECHK structure */
	memcpy(&temp_t.t_typechk, typechk+2, TYPCHKSZ);

	/* Use single representation for uhash */
	if (memcmp(temp_t.t_typechk.t_ghash, null_hash, 4) == 0)
	    memcpy(temp_t.t_typechk.t_ghash, universal_hash, 4);
	if (memcmp(temp_t.t_typechk.t_lhash, null_hash, 4) == 0)
	    memcpy(temp_t.t_typechk.t_lhash, universal_hash, 4);

	for (t = typechks_hash[h]; t; t = t->t_next) {
	    if (typechk_len == t->t_len
		&& memcmp(&t->t_typechk, &temp_t.t_typechk, TYPCHKSZ) == 0)
		return t; /* duplicate */
	}
	/* New typechk */
	t = new_TYPECHK();
	t->t_len = TYPCHKSZ;
	t->t_typechk = temp_t.t_typechk;

    }
    t->t_next = typechks_hash[h];
    typechks_hash[h] = t;

    return t;
} /* put_TYPECHK */
