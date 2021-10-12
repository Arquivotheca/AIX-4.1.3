#if lint == 0 && CENTERLINE == 0
#pragma comment (copyright, "@(#)90	1.18  src/bos/usr/ccs/bin/ld/bind/dump.c, cmdld, bos411, 9428A410j 5/12/94 10:52:10")
#endif

/*
 *   COMPONENT_NAME: CMDLD
 *
 *   FUNCTIONS: get_reltype_name
 *		get_sclass
 *		get_smclass
 *		get_smtype
 *		language_name
 *		minidump_symbol
 *		show_er
 *		show_inpndx
 *		show_rld
 *		show_sym
 *
 *   STATIC FUNCTIONS:
 *		print_tg
 *		show_number
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
#include <string.h>

#include "global.h"
#include "bind.h"
#include "strs.h"
#include "error.h"

#include "dump.h"
#include "objects.h"
#include "save.h"

int	dump_controls;			/* See dump.h for bit definitions. */

/************************************************************************
 * Name: get_sclass
 *									*
 * Purpose: Return symbolic name of sclass field.
 *
 ************************************************************************/
char *
get_sclass(unsigned char class)
{
    static char buf[10];

    switch(class) {
      case C_FILE:	return "C_FILE";
      case C_EXT:	return "C_EXT";
      case C_HIDEXT:	return "C_HIDEXT";
    }
    sprintf(buf, "C_%d", class);
    return buf;
}
/************************************************************************
 * Name: get_smclass
 *									*
 * Purpose: Return symbolic representation storage-mapping class.
 *
 ************************************************************************/
char *
get_smclass(unsigned char class)
{
    static char buf[10];

    switch(class) {
      case XMC_PR:	return "PR";
      case XMC_RO:	return "RO";
      case XMC_DB:	return "DB";
      case XMC_GL:	return "GL";
      case XMC_XO:	return "XO";
      case XMC_SV:	return "SV";
      case XMC_TI:	return "TI";
      case XMC_TB:	return "TB";
      case XMC_RW:	return "RW";
      case XMC_TC0:	return "T0";
      case XMC_TC:	return "TC";
      case XMC_TD:	return "TD";
      case XMC_DS:	return "DS";
      case XMC_UA:	return "UA";
      case XMC_BS:	return "BS";
      case XMC_UC:	return "UC";
      default:
	sprintf(buf, "XMC_%d", class);
	return buf;
    }
}
/************************************************************************
 * Name: get_smtype
 *									*
 * Purpose: Return symbolic representation symbol type.
 *
 ************************************************************************/
char *
get_smtype(unsigned char t)
{
    static char buf[10];

    switch(t) {
      case XTY_ER:	return "ER";
      case XTY_LD:	return "LD";
      case XTY_CM:	return "CM";
      case XTY_SD:	return "SD";
      case XTY_AR:	if (!(dump_controls & DUMP_GET_SMTYPE)) return "--";
			else return "AR";
      case XTY_IS:	if (!(dump_controls & DUMP_GET_SMTYPE)) return "--";
			else return "IS";
      case XTY_IF:	if (!(dump_controls & DUMP_GET_SMTYPE)) return "--";
			else return "IF";
      default:
	sprintf(buf, "XTY_%d", t);
	return buf;
    }
}
/************************************************************************
 * Name: get_reltype_name
 *									*
 * Purpose: Return symbolic representation relocation type.
 *
 ************************************************************************/
char *
get_reltype_name(unsigned char t)
{
    static char buf[10];

    switch(t) {
      case R_POS:	return "R_POS";
      case R_NEG:	return "R_NEG";
      case R_REL:	return "R_REL";
      case R_TOC:	return "R_TOC";
      case R_TRL:	return "R_TRL";
      case R_TRLA:	return "R_TRLA";
      case R_GL:	return "R_GL";
      case R_TCL:	return "R_TCL";
      case R_RL:	return "R_RL";
      case R_RLA:	return "R_RLA";
      case R_REF:	return "R_REF";
      case R_BA:	return "R_BA";
      case R_RBA:	return "R_RBA";
      case R_BR:	return "R_BR";
      case R_RBR:	return "R_RBR";
      default:
	sprintf(buf, "R_%d", t);
	return buf;
    }
}
/************************************************************************
 * Name: show_number
 *									*
 * Purpose: Format a numbered object, using the address itself or the number
 *	of the symbol.
 *
 ************************************************************************/
static void
show_number(void *thing,
	    char *buffer,
	    char prefix,
	    long number)
{
    if (Switches.dbg_opt3)
	sprintf(buffer, "0x%x", thing);
    else
	sprintf(buffer, "%c%d", prefix, number);
}
/************************************************************************
 * Name: show_er
 *									*
 * Purpose: Format a symbol address, using the address itself or the number
 *	of the symbol.
 *
 ************************************************************************/
char *
show_er(SYMBOL *er,
	char *buffer)			/* Buffer for result, if not NULL */
{
    static char buf[30];

    if (buffer == NULL)
	buffer = buf;

    if (er)
	show_number(er, buffer, 'E', er->er_number);
    else
	buffer[0] = '\0';

    return buffer;
}
/************************************************************************
 * Name: show_sym
 *									*
 * Purpose: Format the internal representation of a symbol, prefixing
 *	with an 'S' (or an 'E' for XTY_ERs), the value returned by
 *	show_number();
 *
 ************************************************************************/
char *
show_sym(SYMBOL *s,
	 char *buffer)			/* Buffer for result, if not NULL */
{
    static char buf[30];

    if (buffer == NULL)
	buffer = buf;

    if (s)
	show_number(s, buffer, s->s_smtype == XTY_ER ? 'E' : 'S',
		    (s->s_flags & S_NUMBER_USURPED) ?
		    saved_stuff[s->s_number].number : s->s_number);
    else
	buffer[0] = '\0';

    return buffer;
}
/************************************************************************
 * Name: show_rld
 *									*
 * Purpose: Format an RLD address, using the address itself or the number
 *	of the RLD entry.
 *
 ************************************************************************/
char *
show_rld(RLD *r,
	 char *buffer)			/* Buffer for result, if not NULL */
{
    static char buf[30];

    if (buffer == NULL)
	buffer = buf;

    if (r)
	show_number(r, buffer, 'R', r->r_number);
    else
	buffer[0] = '\0';

    return buffer;
}
/************************************************************************
 * Name: show_inpndx
 *									*
 * Purpose: Print the inpndx of a symbol.  Negative values are printed
 *		symbolically.
 *
 * Returns: The printed length of the symbol's input index (not the length
 *	of the entire printed string).
 ************************************************************************/
int
show_inpndx(SYMBOL *sym,		/* Symbol whose input index to print */
	    char *format_s)		/* Format for input index */
{
    int 	inpndx;
    int		l;
    char	buf[20];
    char	*s;

    if (sym->s_flags & S_INPNDX_MOD)
	inpndx = symtab_index[sym->s_inpndx];
    else
	inpndx = sym->s_inpndx;

    switch(inpndx) {
      case INPNDX_FIXUP:
	s = msg_get(NLSMSG(DUMPLIT_FIXUP, "FIXUP"));
	break;
      case INPNDX_GENERATED:
	if (!(dump_controls & DUMP_SHOW_INPNDX) || !imported_symbol(sym)) {
	    s = msg_get(NLSMSG(DUMPLIT_GENERATED, "GENERATED"));
	    break;
	}
	/* else fall through */
      case INPNDX_IMPORT:
      case INPNDX_IMPORT_TD:
	s = msg_get(NLSMSG(DUMPLIT_IMPORT, "IMPORT"));
	break;
      case INPNDX_ARCHIVE:
	s = msg_get(NLSMSG(DUMPLIT_ARCHIVE, "ARCHIVE"));
	break;
      default:
	l = sprintf(buf, "%d", inpndx);
	say(SAY_NO_NLS | SAY_NO_NL, format_s, buf);
	return l;
    }
    say(SAY_NO_NLS | SAY_NO_NL, format_s, s);
    return strlen(s);
}
/************************************************************************
 * Name: print_tg							*
 *									*
 * PURPOSE: Prints the source file (from the C_FILE symbol table entry)
 * for a given symbol along with the input file name, including the archive
 * name if appropriate.
 *
 * Format:
 *
 ************************************************************************/
static void
print_tg(SYMBOL *sym)			/* Symbol to print about */
{
    if (sym->s_smtype == XTY_IS)
	say(SAY_NO_NLS, " {%s}",
	    get_object_file_name(sym->s_csect->c_srcfile->sf_object));
    else if (sym->s_smtype == XTY_IF)
	say(SAY_NO_NLS, " %s{%s}",
	    get_object_file_name(sym->s_csect->c_srcfile->sf_object),
	    sym->s_csect->c_srcfile->sf_name->name);
    else if (sym->s_smtype == XTY_ER)
	say(SAY_NO_NLS, " (%s)", get_object_file_name(sym->er_object));
    else if (sym->s_csect->c_srcfile) {
	if (sym->s_csect->c_srcfile->sf_inpndx != SF_GENERATED_INPNDX)
	    say(SAY_NO_NLS, " %s(%s)",
		sym->s_csect->c_srcfile->sf_name->name,
		get_object_file_name(sym->s_csect->c_srcfile->sf_object));
	else
	    say(SAY_NO_NLS, " (%s)",
		get_object_file_name(sym->s_csect->c_srcfile->sf_object));
    }
    else			/* Symbol from archive global symbol table */
	say(SAY_NO_NLS, " {%s}", get_object_file_name(sym->s_object));
}
/************************************************************************
 * Name: minidump_symbol
 *									*
 * Purpose: Print the name and information about a symbol
 *
 * Parameters:
 *
 ************************************************************************/
void
minidump_symbol(SYMBOL *sym,		/* Symbol to print about */
		int name_len,		/* Length of name field:
					   0: don't print name
					   > 0: Minimum length for name
					   < 0: Length to leave blank*/
		int flags,		/* What information to print. */
		SYMBOL *print_tg_symbol) /* Symbol whose tag information
					    to print (since ERs don't print
					    srcfile information.*/
{
    int n;

    if (name_len != 0)
	if (name_len < 0)
	    say(SAY_NO_NLS | SAY_NO_NL, "%*s", 1 + -name_len, " ");
	else
	    say(SAY_NO_NLS | SAY_NO_NL, " %*s", -name_len, sym->s_name->name);

    if (flags & MINIDUMP_HASH_PAD)
	say(SAY_NO_NLS | SAY_NO_NL, "%*s", 1 + HASH_FIELD_WIDTH, " ");
    else if (flags & MINIDUMP_HASH) {
	if (sym->er_typechk == NULL)
	    say(SAY_NO_NL | SAY_NO_NLS, " %-*s",
		HASH_FIELD_WIDTH,
		msg_get(NLSMSG(LIT_NO_HASH, "** No Hash **")));
	else {
	    say(SAY_NO_NL | SAY_NO_NLS, " %s %08X %08X",
		language_name(sym->er_typechk->t_typechk.t_lang),
		*(ulong *)&sym->er_typechk->t_typechk.t_ghash[0],
		*(ulong *)&sym->er_typechk->t_typechk.t_lhash[0]);
	}
    }

    if (flags & MINIDUMP_SYMNUM_DBOPT11) {
	if (Switches.dbg_opt11)
	    say(SAY_NO_NLS | SAY_NO_NL, " %-5s", show_sym(sym, NULL));
    }
    else if (flags & MINIDUMP_SYMNUM)
	say(SAY_NO_NLS | SAY_NO_NL, " %-5s", show_sym(sym, NULL));

    if (flags & MINIDUMP_LONG_INPNDX) {
	n = 17 - show_inpndx(sym, " [%s]");
	if (n > 0)
	    say(SAY_NO_NLS | SAY_NO_NL, "%*s", n, "");
    }
    else {
	if (flags & MINIDUMP_INPNDX) {
	    n = 5 - show_inpndx(sym, " [%s]");
	    if (n > 0)
		say(SAY_NO_NLS | SAY_NO_NL, "%*s", n, "");
	}
	if (flags & MINIDUMP_ADDRESS)
	    say(SAY_NO_NLS | SAY_NO_NL, " %08x", sym->s_addr);

	if (flags & MINIDUMP_TYPE)
	    say(SAY_NO_NLS | SAY_NO_NL, " %2s", get_smtype(sym->s_smtype));
    }

    if (flags & MINIDUMP_SMCLASS)
	say(SAY_NO_NLS | SAY_NO_NL, " %2s", get_smclass(sym->s_smclass));


    if (flags & MINIDUMP_CSECT_LEN_ALIGN) {
	say(SAY_NO_NLS | SAY_NO_NL, " %6d %2d",
	    sym->s_csect->c_len, sym->s_csect->c_align);
    }
    else if (flags & MINIDUMP_LEN_ALIGN) {
	if (sym->s_smtype == XTY_CM || sym->s_smtype == XTY_SD) {
	    say(SAY_NO_NLS | SAY_NO_NL, " %6d %5d",
		sym->s_csect->c_len, sym->s_csect->c_align);
	}
	else
	    say(SAY_NO_NLS | SAY_NO_NL, "             " /* 13 spaces */);
    }

    if (flags & MINIDUMP_SOURCE_INFO)
	print_tg(print_tg_symbol ? print_tg_symbol : sym);
    else
	say(SAY_NL_ONLY);
} /* minidump_symbol */
/***************************************************************************
 * Name:	language_name
 *
 * Purpose:
 *
 * Results:
 * *************************************************************************/
char *
language_name(unsigned short lang)
{
#define NUM_LANGUAGES 12

    static char lang_names[NUM_LANGUAGES][5]
#ifndef NO_NLS
	;
    static int lang_ids[NUM_LANGUAGES]
#endif
	= {
	    NLSMSG(LIT_LANG_C, " C  "),		NLSMSG(LIT_LANG_FORT, "Fort"),
	    NLSMSG(LIT_LANG_PASC, "Pasc"),	NLSMSG(LIT_LANG_ADA, "Ada "),
	    NLSMSG(LIT_LANG_PLI, "PL/I"),	NLSMSG(LIT_LANG_BASIC, "BASC"),
	    NLSMSG(LIT_LANG_LISP, "Lisp"),	NLSMSG(LIT_LANG_COBOL, "Cobl"),
	    NLSMSG(LIT_LANG_MOD2, "Mod2"),	NLSMSG(LIT_LANG_CPLUS, "C++ "),
	    NLSMSG(LIT_LANG_RPG, "RPG "),	NLSMSG(LIT_LANG_PL8, "PL8 ")
	    };

    static char buf[4];

    if (lang >= NUM_LANGUAGES) {
	sprintf(buf, "%4d", lang);
	return buf;
    }
    else {
#ifndef NO_NLS
	if (lang_names[lang][0] == '\0') {
	    strncpy(lang_names[lang], msg_get(lang_ids[lang]), 4);
	    lang_names[lang][4] = '\0';
	}
#endif
	return lang_names[lang];
    }
}
