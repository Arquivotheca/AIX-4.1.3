static char sccsid[] = "@(#)65	1.34.2.8  src/bos/usr/ccs/lib/libdbx/cobol.c, libdbx, bos411, 9428A410j 5/2/94 11:19:31";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: builtinmatch, cobol_buildaref, classname,
 *	      cobol_evalaref, cobol_foldnames, cobol_hasmodules, cobol_init,
 *	      cobol_listparams, cobol_modinit, cobol_passaddr, cobol_printdecl,
 *	      cobol_printstruct, cobol_printsubarray, cobol_printval,
 *	      pr_cobol_store_type, cobol_typematch, create_group, enumMatch,
 *	      isConstString, nilMatch, pictureMatch, pop_field,
 *	      pop_group, printRecordDecl, printindex, printpic, prval,
 *	      push_field, push_group, rangematch, stringArrayMatch, 
 *            printdecl, printtype, organization, islang_cobol,
 * 	      access, cobol_typename 
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982 Regents of the University of California
 *
 */
#ifdef KDBXRT
#include "rtnls.h"		/* MUST BE FIRST */
#endif
/*              include file for message texts          */
#include "dbx_msg.h" 
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

/*
 * Cobol-dependent symbol routines.
 */

#include <errno.h>
#include "defs.h"
#include "symbols.h"
#include "printsym.h"
#include "languages.h"
#include "tree.h"
#include "eval.h"
#include "operators.h"
#include "mappings.h"
#include "process.h"
#include "runtime.h"
#include "machine.h"
#include <ctype.h>

extern boolean unique_fns;
extern Language cobLang;

/* macros needed ? */
      struct group {
            struct group *previous;
            int group_id;
            int record; 
            int field; 
      };

      static struct group *current_group = nil;
      static struct group *temp_group = nil;
      static int  record_level = 1;
      static int  field_level = 2;

static void cobol_listparams(Symbol);
static push_group (); 
static pop_group (); 
static push_field (); 
static pop_field (); 
static void cobol_printsubarray (long, struct subdim *, Symbol);
static void cobol_printstruct (Symbol);
static void printindex(Symbol);
static void numericEdit(Node, Symbol, Symbol, int, int, Address, char *);
static void printCondValue(Symbol);
static cob_printdecl (Symbol, int);
static cob_printtype (Symbol, Symbol, int);
static char *organization(Symbol), *access(Symbol);
static struct group *create_group (int);

/*
 * Print out a record declaration.
 */

static printRecordDecl (Symbol t, int parent_level, int n)
{
    register Symbol f;
    int   group_level;

    for (f = t->chain; f != nil; f = f->chain) {
       (*rpt_output)(stdout, "\n");
       if (n > 0) {
	   (*rpt_output)(stdout, "%*c", n, ' ');
       }
       if (f->class == COND) {
	 cob_printdecl(f, n);
       }
       else {
	 group_level = f->symvalue.field.group_level + parent_level;
	 if (group_level < 10)
           (*rpt_output)(stdout, "0");
	 (*rpt_output)(stdout, "%d  ",group_level);
	 (*rpt_output)(stdout, "%s", symname(f));
	 cob_printtype (f/*->type*/, f->type, n);
       }
    }
}

public String pr_cobol_store_type (pic_rec )
     Symbol pic_rec;		/* record for picture type to be printed */
{
  String  storetype;
  register String  name;
  char    *tmpname;
  int     pic_size;
  
  pic_size = pic_rec->symvalue.usage.picsize;
  if (pic_rec->symvalue.usage.storetype != 't')
    (*rpt_output)(stdout, "\tPIC ");
  if (pic_size && (pic_rec->symvalue.usage.edit_descript != nil)) {  
    (*rpt_output)(stdout, "%s", pic_rec->symvalue.usage.edit_descript);
  }
  else {
    switch (pic_rec->symvalue.usage.storetype) {
    case 'a':			/* plain old character array */
      if (pic_size == 1)
	(*rpt_output)(stdout, "A");
      else
	(*rpt_output)(stdout, "A(%d)", pic_size);
      break;
    case 'c':			/* plain old alphanumeric array */ 
      if (pic_size == 1)
	(*rpt_output)(stdout, "X");
      else
	(*rpt_output)(stdout, "X(%d)", pic_size);
      break;
    case 'b':			/* edited: get the edit description and */
    case 'd':			/* then do edit then display */
    case 'p':
      break;
    case 'e':			/* numeric sign trailing included */
      if (pic_size == 1)
	(*rpt_output)(stdout, "S9 SIGN IS TRAILING");
      else
	(*rpt_output)(stdout, "S9(%d) SIGN IS TRAILING", pic_size);
      break;
    case 'f':			/* numeric sign trailing separate */
      if (pic_size == 1)
	(*rpt_output)(stdout, "S9 SIGN IS TRAILING SEPARATE");
      else
	(*rpt_output)(stdout, "S9(%d) SIGN IS TRAILING SEPARATE", pic_size);
      break;
    case 'g':			/* numeric sign leading included */
      if (pic_size == 1)
	(*rpt_output)(stdout, "S9 SIGN IS LEADING");
      else
	(*rpt_output)(stdout, "S9(%d) SIGN IS LEADING", pic_size);
      break;
    case 'h':			/* numeric sign leading separate */
      if (pic_size == 1)
	(*rpt_output)(stdout, "S9 SIGN IS LEADING SEPARATE");
      else
	(*rpt_output)(stdout, "S9(%d) SIGN IS LEADING SEPARATE", pic_size);
      break;
    case 'i':			/* numeric signed default comp */
      if (pic_size == 1)
	(*rpt_output)(stdout, "S9 COMP");
      else
	(*rpt_output)(stdout, "S9(%d) COMP", pic_size);
      break;
    case 'j':			/* numeric unsigned default comp */
      if (pic_size == 1)
	(*rpt_output)(stdout, "9 COMP");
      else
	(*rpt_output)(stdout, "9(%d) COMP", pic_size);
      break;
    case 'k':			/* numeric packed decimal signed , (comp-3) */
      if (pic_size == 1)
	(*rpt_output)(stdout, "S9 COMP-3");
      else
	(*rpt_output)(stdout, "S9(%d) COMP-3", pic_size);
      break;
    case 'l':			/* numeric packed decimal unsigned */
      if (pic_size == 1)
	(*rpt_output)(stdout, "9 COMP-3");
      else
	(*rpt_output)(stdout, "9(%d) COMP-3", pic_size);
      break;
    case 'm':			/* numeric unsigned comp-x */
      if (pic_size == 1)
	(*rpt_output)(stdout, "9 COMP-X");
      else
	(*rpt_output)(stdout, "9(%d) COMP-X", pic_size);
      break;
    case 'n':			/* numeric unsigned comp-5 */
      if (pic_size == 1)
	(*rpt_output)(stdout, "9 COMP-5");
      else
	(*rpt_output)(stdout, "9(%d) COMP-5", pic_size);
      break;
    case 'o':			/* numeric signed comp-5 */
      if (pic_size == 1)
	(*rpt_output)(stdout, "S9 COMP-5");
      else
	(*rpt_output)(stdout, "S9(%d) COMP-5", pic_size);
      break;
    case 'q':			/* numeric unsigned int */
      if (pic_size == 1)
	(*rpt_output)(stdout, "9");
      else
	(*rpt_output)(stdout, "9(%d)", pic_size);
      break;
    case 's':			/* index */
      if (pic_rec->symvalue.usage.bytesize < 4)
	(*rpt_output)(stdout, "9(4) COMP USAGE IS INDEX");
      else
	(*rpt_output)(stdout, "9(9) COMP USAGE IS INDEX");
      break;
    case 't':			/* pointer */
      (*rpt_output)(stdout, "\tUSAGE IS POINTER");
      break;
    default:
      (*rpt_output)(stdout, "default usage type");
      break;
    }
  }
}

public Language cobLang;

/*
 *  Variables for subarrays
 */
struct subdim {
	long ub;
	long lb;
	struct subdim *next, *back;
};
extern struct subdim *subdim_tail;
extern Symbol array_sym;
extern boolean subarray;
extern Address array_addr;
Symbol array_top;

static String varname = "";


/*
 * Initialize Cobol language information.
 */

void cobol_init()
{
    cobLang = language_define("cobol", ".cbl");
    cobLang = language_define("cobol", ".CBL");
    cobLang = language_define("cobol", ".cob");
    cobLang = language_define("cobol", ".int");
    language_setop(cobLang, L_PRINTDECL, cobol_printdecl);
    language_setop(cobLang, L_PRINTVAL, cobol_printval);
    language_setop(cobLang, L_TYPEMATCH, cobol_typematch);
    language_setop(cobLang, L_BUILDAREF, cobol_buildaref);
    language_setop(cobLang, L_EVALAREF, cobol_evalaref);
    language_setop(cobLang, L_MODINIT, cobol_modinit);
    language_setop(cobLang, L_HASMODULES, cobol_hasmodules);
    language_setop(cobLang, L_PASSADDR, cobol_passaddr);
    language_setop(cobLang, L_FOLDNAMES, cobol_foldnames);
}

/*
 * Test if two types are compatible.  All pic types are arrays of the storage
   class of the pic.
 */

static boolean builtinmatch (Symbol t1, Symbol t2)
{
    boolean b;

    b = (boolean) (
	(
	    t2 == t_int->type and
	    t1->class == RANGE and istypename(t1->type, "integer")
	) or (
	    t2 == t_char->type and
	    t1->class == RANGE and istypename(t1->type, "char")
	) or (
	    t2 == t_boolean->type and
	    t1->class == RANGE and istypename(t1->type, "boolean")
	)
    ); 
    if (t1==nil or t1->type==nil or t2==nil or t2->type==nil)
        b = false;
    return b;
}

static boolean rangematch (Symbol t1, Symbol t2)
{
    boolean b;
    register Symbol rt1, rt2;

    if (streq(t1->type->name->identifier,"short") &&
        streq(t2->type->name->identifier,"$integer"))
	b = true;
    else if (t1->class == RANGE and t2->class == RANGE) {
	rt1 = rtype(t1->type);
	rt2 = rtype(t2->type);
	b = (boolean) (rt1->type == rt2->type);
    } else {
	b = false;
    }
    return b;
}

static boolean nilMatch (Symbol t1, Symbol t2)
{
    boolean b;

    b = (boolean) (
	(t1 == t_nil and t2->class == PTR) or
	(t1->class == PTR and t2 == t_nil)
    );
    return b;
}

static boolean enumMatch (Symbol t1, Symbol t2)
{
    boolean b;

    b = (boolean) (
	(t1->class == SCAL and t2->class == CONST and t2->type == t1) or
	(t1->class == CONST and t2->class == SCAL and t1->type == t2)
    );
    return b;
}

static boolean isConstString (Symbol t)
{
    boolean b;

    b = (boolean) (
	t->language == primlang and t->class == ARRAY and t->type == t_char
    );
    return b;
}

static boolean stringArrayMatch (Symbol t1, Symbol t2)
{
    boolean b;

    b = (boolean) (
	(
	    isConstString(t1) and
	    t2->class == ARRAY and compatible(t2->type, t_char->type)
	) or (
	    isConstString(t2) and
	    t1->class == ARRAY and compatible(t1->type, t_char->type)
	)
    );
    return b;
}

#define isalnumpic(t) (((t)->class == PIC || (t)->class == RPIC) &&\
		       ((t)->symvalue.usage.storetype >= 'a' &&\
			(t)->symvalue.usage.storetype <= 'd'))

/*
 * Test to see if a COBOL pic clause is of unsigned type.
 */
boolean isunsignedpic(Symbol s)
{
  Symbol t = rtype(s);

  if ((t != nil) &&
      (t->class == PIC || t->class == RPIC) &&
      (t->symvalue.usage.storetype == 'j' ||
       (t->symvalue.usage.storetype >= 'l' &&
	t->symvalue.usage.storetype <= 'n') ||
       t->symvalue.usage.storetype == 'q' ||
       t->symvalue.usage.storetype == 's'))
    return true;
  else
    return false;
}

static boolean pictureMatch (Symbol t1, Symbol t2)
{
    boolean b;

    b = (boolean) (
	(
	    isConstString(t1) and
	    isalnumpic(t2)
	) or (
	    isConstString(t2) and
	    isalnumpic(t1)
	) or (
	    isalnumpic(t1) and
	    isalnumpic(t2)
	) or (
	      (compatible(t2, t_int->type)) and
	      ((t1->class == PTR) or
	       ((t1->class == PIC or t1->class == RPIC) and
		(t1->symvalue.usage.storetype >= 'e' &&
		 t1->symvalue.usage.storetype <= 't')))
	) or ( 
	      (compatible(t2, t_real->type) || compatible(t2, t_int->type)) and
	      ((t1->class == PIC or t1->class == RPIC) and
	       ((t1->symvalue.usage.storetype == 'p') ||
		((t1->symvalue.usage.storetype >= 'e' &&
		  t1->symvalue.usage.storetype <= 't') &&
		 (t1->symvalue.usage.decimal_align > 0))))
	      )
	);
    return b;
}

Boolean cobol_typematch(Symbol type1, Symbol type2)
{
    Boolean b;
    Symbol t1, t2, tmp;

    t1 = rtype(type1);
    t2 = rtype(type2);
    if (t1 == t2) {
	b = true;
    } else {
	if (t1 == t_char->type or t1 == t_int->type or 
            t1 == t_real->type or t1 == t_boolean->type
        ) {
	    tmp = t1;
	    t1 = t2;
	    t2 = tmp;
	}
	b = (Boolean) (
	    pictureMatch(t1, t2) or
	    builtinmatch(t1, t2) or rangematch(t1, t2) or
  	    nilMatch(t1, t2) or enumMatch(t1, t2) or    
	    stringArrayMatch(t1, t2)
        );		
    }
    return b;
}

static String cobol_typename (Symbol s)
{
    String word;

    if (s->class == TYPE) {
        word = s->name->identifier;
        if (!strcmp(word,"char") || !strcmp(word,"character") ||
	    !strcmp(word,"unsigned char") || !strcmp(word,"signed char"))  
           	return(" alphanumeric edited.");
        if (!strcmp(word,"int") || !strcmp(word,"short") || 
	    !strcmp(word,"integer"))
           	return(" comp-2.");
        if (!strcmp(word,"unsigned int"))
           return ("unsigned default comp.");
        return (" NOT A COBOL DATA TYPE.");   
    }
    else if (s->class == TAG)
    {
       return(" Group name.");
    }

}

/*
 * Print out the declaration of a Cobol variable.
 */

void cobol_printdecl(Symbol s)
{
   cob_printdecl (s, 0);
   (*rpt_output)(stdout, "\n");
}

static cob_printdecl (Symbol s, int indent)
{    
    Symbol  t;
    int   group_level;
    String fn;
    
    t = s;

    switch (s->class) {
	case CONST:
            group_level = 88;
            (*rpt_output)(stdout, " %d ", group_level);
	    if (s->type->class == SCAL) {
		(*rpt_output)(stdout, "enumeration constant with value ");
		eval(s->symvalue.constval);
		cobol_printval(s);
	    } else {
		(*rpt_output)(stdout, "const %s = ", symname(s));
		printval(s, 0);
	    }
	    break;
	case TYPE:
	    if (s->type->class == FILET) {
	      (*rpt_output)(stdout, catgets(scmc_catd, MS_cobol, MSG_540,
		  "%cD %s Record Size %d.\nOrganization is %s, Access is %s."),
			    ((s->type->symvalue.usage.edit_descript[1] == 'o')
			    ? 'S' : 'F'),
			    symname(s), s->type->symvalue.usage.bytesize,
			    organization(s->type), access(s->type));
	    }
	    else {
	      (*rpt_output)(stdout, "category %s = ", symname(s));
	      cob_printtype(s, s->type, 0);
	    }
	    break;
	case VAR:
	case TOCVAR:
	    if (s->class != TYPE and s->storage == INREG) {
		(*rpt_output)(stdout, "register ");
	    }
            group_level = 1;
            (*rpt_output)(stdout, "0%d  ", group_level);
            (*rpt_output)(stdout, "%s", symname(s));
            cob_printtype (s, s->type, indent);
            pop_group ();
	    break;

	case REFFIELD:
            group_level = s->symvalue.field.group_level;
            if (group_level < 10) 
                (*rpt_output)(stdout, "0");
            (*rpt_output)(stdout, "%d  ", group_level);
            (*rpt_output)(stdout, "%s", symname(s));
            cob_printtype (s, s->type, indent);
            (*rpt_output)(stdout, "\n");
	    break;
	case TAG:
	    cob_printtype(s, s->type, indent);
	    break;
	case RECORD:
	case GROUP:		
	case RGROUP:		
            findtype (t);
            cob_printtype (t, t->type /*was t*/, indent);
            group_level = s->symvalue.field.group_level;
            (*rpt_output)(stdout, " %d ",group_level);
            break;
	case REAL:
	case RANGE:
	case ARRAY:
	case UNION:
	case PTR:
	case FFUNC:
	    cob_printtype(s, s->type /*was t*/, indent);
	    break;
	case SCAL:
            group_level = 78;
            (*rpt_output)(stdout, " %d ", &group_level);
	    (*rpt_output)(stdout, 
		     "(enumeration constant, value %d)", s->symvalue.iconval);
            group_level = 1;
	    break;
	case PROC:
	case FUNC:
	case CSECTFUNC:
	    (*rpt_output)(stdout, "%s", symname(s));
	    cobol_listparams(s);
	    break;
	case MODULE:
	    fn = symname(s);
	    (*rpt_output)(stdout,  catgets(scmc_catd, MS_cobol, MSG_505,
			 "source file \"%s.cbl\""), (unique_fns) ? ++fn : fn);
	    break;
	case PROG:
	    (*rpt_output)(stdout, catgets(scmc_catd, MS_cobol, MSG_506,
					"executable file \"%s\""), symname(s));
	    break;
	case COND:
            group_level = 88;
            (*rpt_output)(stdout, "%d ", group_level);
            (*rpt_output)(stdout, "%s ", symname(s));
	    (*rpt_output)(stdout, "VALUE");
	    s = s->type;
	    while (s) {
	      printCondValue(s);
	      s = s->chain;
	    }
            (*rpt_output)(stdout, ".");
	    break;
	default:
	    (*rpt_output)(stdout, "[%s]", cobol_classname(s));
	    break;
    }
}

/*
 * Recursive whiz-bang procedure to print the type portion
 * of a declaration.
 *
 * The symbol associated with the type is passed to allow
 * searching for type names without getting "type blah = blah".
 */

static cob_printtype (Symbol s, Symbol t, int indent)
{
    register Symbol i;
    long r0, r1;
    register String p;
    String   word;

    checkref(s);
    checkref(t); 
    switch (t->class) {
	case VAR:
	case CONST:
	case PROC:
	case TOCVAR:
	    panic(catgets(scmc_catd, MS_cobol, MSG_516,
				   "printtype: class %s"), cobol_classname(t));
	    break;
	case ARRAY:
	    (*rpt_output)(stdout, " OCCURS ");
	    i = t->chain;
	    if (i != nil) {
		i = rtype(i);
		(*rpt_output)(stdout, "%d", i->symvalue.rangev.upper);
                (*rpt_output)(stdout, " TIMES");
	    } else {
		(*rpt_output)(stdout, catgets(scmc_catd, MS_cobol, MSG_510,
						    "no range for an OCCURS"));
	    }
            if (!nilname(t)) {
                word = cobol_typename (t->type);
                (*rpt_output)(stdout, "%s", word);
	      }
            else
                cob_printtype (t, t->type, indent); 
	    break;

	case RGROUP:
	    (*rpt_output)(stdout, " REDEFINES %s",
		t->symvalue.usage.redefines);
	case RECORD:
	case GROUP:
            (*rpt_output)(stdout, ".");
	    printRecordDecl(t, /*t->symvalue.field.group_level*/ 0, indent+4);
	    break;

	case UNION:
	    (*rpt_output)(stdout, "%s \n", cobol_classname(t));
	    if (s->class == TAG && !nilname(s)) {
		p = symname(s);
                if (*p != '\0') (*rpt_output)(stdout, "%s ", p);
	    }
            if (t->chain) {
                push_field ();
                for (i = t->chain; i != nil; i = i->chain) {
		    assert(i->class == REFFIELD);
		    cob_printdecl(i, indent+4);
	        }
                pop_field ();
	      }
            if (indent > 0) {  
		(*rpt_output)(stdout, "%*c", indent, ' ');
	      }
	    break;

	case RANGE:
	case REAL:
	case FUNC:
	case FFUNC:
	    break;

	case PTR:
	    cob_printtype(t, t->type, indent);
	    /*
	    if (t->type->class != PTR) {
		(*rpt_output)(stdout, " ");
	    }
	    (*rpt_output)(stdout, "*");
	    */
	    break;

	case TYPE:
	    if (nilname(t)) {
		cob_printtype(t, t->type, indent);
	    } else {
                word = cobol_typename (t);
                (*rpt_output)(stdout, " %s", word);
	    }
	    break;

	case TYPEREF:
	    (*rpt_output)(stdout, catgets(scmc_catd, MS_cobol, MSG_511,
           						 "TYPEREF in cobol"));
	    (*rpt_output)(stdout, "@%s", symname(t));
	    break;

	case SCAL:
	    (*rpt_output)(stdout, "condition ");
	    if (s->class == TAG && !nilname(s)) {
		(*rpt_output)(stdout, "%s ", symname(s));
	    }
	    i = t->chain;
	    if (i != nil) {
		for (;;) {
		    (*rpt_output)(stdout, "%s", symname(i));
		    i = i->chain;
		if (i == nil) break;
		    (*rpt_output)(stdout, ", ");
		}
	    }
	    break;

	case TAG:
            cob_printtype(t, t->type, indent);
            break;

	case RPIC:
	    (*rpt_output)(stdout, " REDEFINES %s",
		t->symvalue.usage.redefines);
	case PIC:
	    pr_cobol_store_type(t);
	    (*rpt_output)(stdout, ".");
	    while (t->chain) {	/* COBOL conditionals */
	      t = t->chain;
	      (*rpt_output)(stdout, "\n%*c", indent+4, ' ');
	      cob_printdecl(t, indent+4);
	    }
            break;  	    
	case INDX:
            (*rpt_output)(stdout, "%s ", cobol_classname(t));
            break;
	case INDXU:
          /* conversion will be done in the printtype routine */
            (*rpt_output)(stdout, "%s ", cobol_classname(t));
            break; 
	default:
	    (*rpt_output)(stdout, catgets(scmc_catd, MS_cobol, MSG_513,
						      "(class %d)"), t->class);
	    break;
    }
}

/*
 * List the parameters of a procedure or function.
 * No attempt is made to combine like types.
 */

static void cobol_listparams(Symbol s)
{
    register Symbol t;

    (*rpt_output)( stdout, "(");
    for (t = s->chain; t != nil; t = t->chain) {
	(*rpt_output)(stdout, "%s", symname(t));
	if (t->chain != nil) {
	    (*rpt_output)(stdout, ", ");
	}
    }
    (*rpt_output)( stdout, ")");
    if (s->chain != nil) {
	(*rpt_output)(stdout, "\n");
	for (t = s->chain; t != nil; t = t->chain) {
	    if (t->class != VAR) {
		panic(catgets(scmc_catd, MS_cobol, MSG_517,
			       "unexpected class %d for parameter"), t->class);
	    }
	    cob_printdecl(t, 0);
	}
    } else {
	(*rpt_output)( stdout, "\n");
    }
}

/*
 * Print out the value on the top of the expression stack
 * in the format for the type of the given symbol.
 */

extern Boolean expandunions;

void cobol_printval (Symbol s)
{
    int n = size(s);
    Symbol t;
    Address a;
    integer i, len;

    switch (s->class) {
	case CONST:
	case TYPE:
	case VAR:
	case TOCVAR:
	case REF:
	case FVAR:
	case TAG:
	    cobol_printval(s->type);
	    break;

	case UNION:
	  if (subarray)
	  {
	    printsubarray(0, subdim_tail, array_sym->type, UNION);
	    reset_subarray_vars();
	  }
          else if (expandunions)
	    printindex(s);
          else
          {
	    if (ord(s->class) > ord(LASTCLASS)) {
		panic(catgets(scmc_catd, MS_cobol, MSG_518,
				     "printval: bad class %d"), ord(s->class));
	    }
	    sp -= size(s);
	    PRINTF "[%s]", cobol_classname(s));
          }
	  break;

	case REFFIELD:
	    if (isbitfield(s)) {
		i = extractField(s, NULL);
		t = rtype(s->type);
		if (t->class == SCAL) {
		    printEnum(i, t);
		} else {
		    printRangeVal(i, t);
		}
	    } else {
		cobol_printval(s->type);
	    }
	    break;

	case ARRAY:
	    t = rtype(s->type);
	    if ((t->class == RANGE and istypename(t->type, "char")) or
		t == t_char->type) 
	    {
		len = size(s);
		sp -= len;
		if (s->language == primlang) {
		    PRINTF "%.*s", len, sp);
		} else {
		    PRINTF "\"%.*s\"", len, sp);
		}
	    } 
	    else
	    {
		printarray(s);
	    }
	    break;

	case RECORD:
        case GROUP:
        case RGROUP:
            if (subarray) {
              cobol_printsubarray(0, subdim_tail, array_sym);
              reset_subarray_vars();
            }
            else
              cobol_printstruct(s);
	    break;

	case RANGE:
	    if (subarray)
	    {
		if(array_sym->language == cobLang)
		   cobol_printsubarray(0, subdim_tail, array_sym);
		else
		   printsubarray(0, subdim_tail, array_sym->type, RANGE);
	        reset_subarray_vars();
	    }
	    else
		printrange(s);
	    break;

	case PTR:
	    if (subarray)
	    {
		printsubarray(0, subdim_tail, array_sym->type, PTR);
	        reset_subarray_vars();
	    }
	    else
	    {
	       t = rtype(s->type);
	       a = pop(Address);
               if ((s->type->class == PIC || s->type->class == RPIC)
		   && s->type->symvalue.usage.storetype == 't') { /* Pointer */
		 if (a == 0) {
		   PRINTF "(nil)");
	         } else if (t->class == RANGE and
			    istypename(t->type, "char")) {
		   printString(a, (boolean) (s->language != primlang));
	         } else {
		   PRINTF "0x%x", a);
	         }
               }
               else {		/* Linkage item */
		 rpush(a, size(t));
                 cobol_printval(t);
               }
            }
	    break;

	case PIC:
	case RPIC:
            if (subarray) {
              cobol_printsubarray(0, subdim_tail, array_sym);
              reset_subarray_vars();
            }
            else
              printpic(s);
	    break;

	case SCAL:
	    if (subarray)
	    {
		printsubarray(0, subdim_tail, array_sym->type, SCAL);
	        reset_subarray_vars();
	    }
	    else
	    {
	       i = pop(Integer);
	       printEnum(i, s);
	    }
	    break;
	/*
	 * Unresolved structure pointers?
	 */
	case BADUSE:
	    a = pop(Address);
	    PRINTF "@0x%x", a);
	    break;

	case FFUNC:
	    a = pop(Address);
	    t = whatblock(a);
	    if (a == prolloc(t)) {
		PRINTF "%s()", symname(t));
	    } else { 
	        PRINTF "0x%x", a);
	    }
	    break;
        
        case COND:
            break;

        case INDX:
        case INDXU:
	default:
	    if (ord(s->class) > ord(LASTCLASS)) {
		panic(catgets(scmc_catd, MS_cobol, MSG_518,
				     "printval: bad class %d"), ord(s->class));
	    }
	    sp -= size(s);
	    PRINTF "[%s]", cobol_classname(s));
	    break;
    }
}
/* 
*  Print a cobol varying bounds array
*/

static void cobol_printsubarray(long y, struct subdim *ptr, Symbol curbound)
{
   static long index_list[10];
   long i, len;
   Address addr;

   if (ptr)
   {
      for (i = ptr->lb; i <= ptr->ub; i++)
      {
	  index_list[y] = i;
	  cobol_printsubarray(y+1, ptr->back, curbound);
      }
   }
   else
   {
      addr = array_addr;
      PRINTF "(");
      for (i = 0; i < y; i++)
      {
	 if (i+1 >= y)
            PRINTF "%d",index_list[i]);
	 else
            PRINTF "%d,",index_list[i]);
	 cobol_evalaref( curbound, addr, index_list[i]);
	 addr = pop(long);
	 if ((i == 0) && (curbound->class != ARRAY))
	    curbound = curbound->type->type;
	 else
	    curbound = curbound->type;
      }
      rpush(addr, size(curbound));
      PRINTF ") = ");
      subarray = false;
      cobol_printval(curbound);
      subarray = true;
      PRINTF "\n");
   }
}

/*
 * Print out a Cobol structure (in other words, a group).
 */

static void cobol_printstruct (Symbol s)
{
    Symbol f;
    Stack *savesp;
    integer n, off, len;

    sp -= size(s);
    savesp = sp;
    PRINTF "(");
    f = s->chain;
    for (;;) {
        if (f->class == COND) {
	    f = f->chain;
            if (f == nil) break;
        }
        else {
	    off = f->symvalue.field.offset;
	    len = f->symvalue.field.length;
	    /* n = (off + len + BITSPERBYTE - 1) / BITSPERBYTE; */
	    n = size(f) + (off / 8); /* handle cobol 'occurs' better */
	    sp += n;
	    PRINTF "%s = ", symname(f));
	    cobol_printval(f);
	    sp = savesp;
	    f = f->chain;
            if (f == nil) break;
	    PRINTF ", ");
        }
    }
    PRINTF ")");
}

/*
 * Return the Cobol name for the particular class of a symbol.
 */

String cobol_classname(Symbol s)
{
    String str;

    switch (s->class) {
	case RECORD:
        case GROUP:
	    str = " GROUP.";
	    break;
	case UNION:
	case RGROUP:
	    str = " REDEFINES ";
	    break;

	case COND:
	case SCAL:
	    str = "conditional variable";
	    break;
        case INDX:
            str = " INDEXED BY";
            break;
        case INDXU:
            str = " USAGE IS INDEX";
            break;
	case PIC:
	case RPIC:
	    str = " ELEMENTARY ITEM";
            break;
	default:
	    str = classname(s);
            break;
    }
    return str;
}

struct ArrayList {
  Symbol s;
  struct ArrayList *prev, *next;
};

/*
 * Build a subscript index.
 */

Node cobol_buildaref(Node a, Node slist)
{
    register Symbol t /*, eltype */;
    register Node esub, old_rval, ptr_node;
    Node r = a, p = slist;
    int i = 0;
    struct ArrayList *al, *head, *tail;
    int ndims = 0;

    t = a->nodetype;
    al = (struct ArrayList *)malloc(sizeof(struct ArrayList));
    al->s = t;
    al->prev = nil;
    al->next = nil;
    head = tail = al;
    if (t->type->class == ARRAY)
      ndims++;
    while (t->class == REFFIELD) {
      t = t->symvalue.field.parent;
      if (t->type->class == ARRAY) {
	al->prev = (struct ArrayList *)malloc(sizeof(struct ArrayList));
	al->prev->next = al;
	al = al->prev;
	al->prev = nil;
	al->s = t;
	ndims++;
	head = al;
      }
      if (t->class == VAR)
	break;
    }
      
    if (ndims == 0)
    {
	beginerrmsg();
	(*rpt_error)(stderr, "\"");
	prtree( rpt_error, stderr, a);
	(*rpt_error)(stderr,  catgets(scmc_catd, MS_symbols, MSG_396,
							"\" is not an array"));
	enderrmsg();
    }
    else 
    {
        r = build(O_SYM, head->s);
	for (al = head; p != nil && (al != nil && al->s->type->class == ARRAY)
	     ; p = p->value.arg[1])
	{
	   esub = p->value.arg[0];
	   if (esub->op == O_DOTDOT) {
	     beginerrmsg();
	     (*rpt_error)(stderr, catgets(scmc_catd, MS_cobol, MSG_548,
		          "Subarrays in COBOL not currently supported.\n"));
	     enderrmsg();
	   }
	   r = build(O_INDEX, r, esub);
	   al = al->next;
	   if (al == nil) {
	     if (a->nodetype->type->class == ARRAY)
	       r->nodetype = a->nodetype->type->type;
	     else
	       r->nodetype = a->nodetype;
	   }
	   else
	     r->nodetype = al->s;
	}
	if (al == tail) {
	  r = build(O_INDEX, r, build(O_LCON, 1));
	  al = al->next;
	  r->nodetype = a->nodetype;
	}
	if (tail->s->type->class == ARRAY) {
	  if (p != nil or al != nil) {
	    beginerrmsg();
	    prtree( rpt_error, stderr, a);
	    (*rpt_error)(stderr, catgets(scmc_catd, MS_cobol, MSG_546,
					   " requires %d or %d subscripts.\n"),
					   ndims-1, ndims);
	    enderrmsg();
	  }
	}
	else {
	  if (p != nil or al != nil) {
	    beginerrmsg();
	    prtree( rpt_error, stderr, a);
	    (*rpt_error)(stderr, catgets(scmc_catd, MS_cobol, MSG_547,
					   " requires %d subscript(s).\n"),
					   ndims);
	    enderrmsg();
	  }
	}
      }

    al = head;
    while (al->next != nil) {	/* free up linked list */
      al = al->next;
      free(al->prev);
    }
    free(al);
    
    return r;
}

/*
 * Determine the offset from s to the parent array above it (if any).
 */
static int offsetParentArray(Symbol s)
{
  int offset = 0;

  while (s->class == REFFIELD) {
    offset += s->symvalue.field.offset;
    s = s->symvalue.field.parent;
    if (s->type->class == ARRAY)
      return offset/8;		/* offset is in bits */
  }

  return 0;
}

/*
 * Evaluate a subscript index.
 */

void cobol_evalaref(Symbol s, Address base, long i)
{
    Symbol t, u;
    long lb, ub;

    t = rtype(s);
    if (s->class != PTR)
    {
      if (s->type->class == ARRAY) {
	u = t->chain;
	lb = u->symvalue.rangev.lower;
	ub = u->symvalue.rangev.upper;
	if ((i < lb or i > ub) && (!varIsSet("$unsafebounds"))) {
	  error(catgets(scmc_catd, MS_cobol, MSG_520,
			  "subscript out of range"));
	}
	push(long, base + ((i-lb)*size(t->type)) + offsetParentArray(s));
      }
      else
	push(long, base + offsetParentArray(s));
    }
    else
    {
       dread(&base, base, sizeof(Address));
       push(long, base + i * size(t->type));
    }
}

/*
 * Initialize typetable information.
 */

void cobol_modinit ()
{
    addpredefined(TP_INT);
}

boolean cobol_hasmodules ()
{
    return false;
}

boolean cobol_passaddr (Symbol param, Symbol exprtype)
{
    boolean b;
    Symbol t;

    t = rtype(exprtype);
    b = (boolean) (t->class == ARRAY);
    return b;
}

cases cobol_foldnames ()
{
    return upper;
}

static void printindex(Symbol s)
{
   Symbol t;
   long i;

   PRINTF "union: (");
   s = s->chain;
   for (;;)
   {
      PRINTF "%s = ", symname(s));
      if (isbitfield(s))
      {
	i = extractField(s, NULL);
	t = rtype(s->type);
	if (t->class == SCAL)
	   printEnum(i, t);
	else
	   printRangeVal(i, t);
      } 
      else
         cobol_printval(s->type);
      s = s->chain;
      if (s == nil)
          break;
      /*
       * reset stack pointer so there is 
       * something to print out if there
       * are more fields in the union.
       */
      sp += size(s->type);
      PRINTF ", ");
   }
   PRINTF ")");
}

static printpic (Symbol pic_rec)
{
  Symbol f;
  integer display_width, off, len;
  Address symbol_addr;
  String  display_format;
  String  internal_format;
  int i;
  unsigned int ui;
  float fl;
  double d = 0;
  unsigned char *uc;
  char a[BUFSIZ];
  long result = 0;
  boolean neg = false;
  int decimal;

/* the address of the variable is on the internal evaluation stack already,
 * having been put there by printval (in printsym.c).
 */   

  len = size(pic_rec);
  decimal = pic_rec->symvalue.usage.decimal_align;

  switch (pic_rec->symvalue.usage.storetype) {
  case 'a':			/* plain old character array */
  case 'b':			/* alphabetic edited */
  case 'c':			/* plain old alphanumeric array */ 
  case 'd':			/* alphanumeric edited */
    sp -= len;
    if (len == 1 and varIsSet("$hexchars"))
      (*rpt_output)(stdout, "0x%lx", *sp);
    else

    (*rpt_output)(stdout, "\"%.*s\"", len, sp);
    break; 

  case 'p':			/* numeric edited */
  case 'q':			/* numeric unsigned unsigned int */
    sp -= len;
    if (decimal > 0 && decimal < len) {	/* implied decimal */
      (*rpt_output)(stdout, "%.*s", len-decimal, sp);
      (*rpt_output)(stdout, ".");
      (*rpt_output)(stdout, "%.*s", decimal, sp+len-decimal);
    }
    else
      (*rpt_output)(stdout, "%.*s", len, sp);
    break; 

  case 'e':			/* numeric sign trailing included */
    sp -= len;
    if (sp[len-1] & 0x40) {
      sp[len-1] -= 0x40;
      (*rpt_output)(stdout, "-");
    }
    if (decimal > 0 && decimal < len) {	/* implied decimal */
      (*rpt_output)(stdout, "%.*s", len-decimal, sp);
      (*rpt_output)(stdout, ".");
      (*rpt_output)(stdout, "%.*s", decimal, sp+len-decimal);
    }
    else
      (*rpt_output)(stdout, "%.*s", len, sp);
    break; 

  case 'g':			/* numeric sign leading included */
    sp -= len;
    if (*sp & 0x40) {
      *sp -= 0x40;
      (*rpt_output)(stdout, "-");
    }
    if (decimal > 0 && decimal < len) {	/* implied decimal */
      (*rpt_output)(stdout, "%.*s", len-decimal, sp);
      (*rpt_output)(stdout, ".");
      (*rpt_output)(stdout, "%.*s", decimal, sp+len-decimal);
    }
    else
      (*rpt_output)(stdout, "%.*s", len, sp);
    break; 

  case 'f':			/* numeric sign trailing separate */
    sp -= len;
    if (decimal > 0 && decimal < len) {	/* implied decimal */
      (*rpt_output)(stdout, "%.*s", len-decimal-1, sp);
      (*rpt_output)(stdout, ".");
      (*rpt_output)(stdout, "%.*s", decimal+1, sp+len-decimal-1);
    }
    else
      (*rpt_output)(stdout, "%.*s", len, sp);
    break; 

  case 'h':			/* numeric sign leading separate */
    sp -= len;
    if (decimal > 0 && decimal < len) {	/* implied decimal */
      (*rpt_output)(stdout, "%.*s", len-decimal, sp);
      (*rpt_output)(stdout, ".");
      (*rpt_output)(stdout, "%.*s", decimal, sp+len-decimal);
    }
    else
      (*rpt_output)(stdout, "%.*s", len, sp);
    break; 

  case 'i':			/* numeric signed default comp */
  case 'o':			/* numeric signed comp-5 */
  case 's':			/* index */
    sp -= len;
    uc = sp;
    neg = (uc[0] & 0x80);
    if (neg){
      for (i = 0; i < len; ++i)
	d = d * 0x100 + ~(~0xff | uc[i]);
      d = -(d+1);
    }
    else {
      for (i = 0; i < len; ++i)
	d = d * 0x100 + uc[i];
    }
    if (decimal > 0) {		/* implied decimal */
      for (i = 0; i < decimal; ++i)
	d /= 10;
      (*rpt_output)(stdout, "%.*f", decimal, d);
    }
    else
      (*rpt_output)(stdout, "%.0f", d);
    break; 

  case 'j':			/* numeric unsigned default comp */
  case 'm':			/* numeric unsigned comp-x */
  case 'n':			/* numeric unsigned comp-5 */
    sp -= len;
    uc = sp;
    for (i = 0; i < len; ++i)
      d = d * 0x100 + uc[i];
    if (decimal > 0) {		/* implied decimal */
      for (i = 0; i < decimal; ++i)
	d /= 10;
      (*rpt_output)(stdout, "%.*f", decimal, d);
    }
    else
      (*rpt_output)(stdout, "%.0f", d);
    break; 

  case 'k':			/* numeric packed decimal signed, ie. comp-3 */
  case 'l':			/* numeric packed decimal unsigned */
				/* unpack 4 bits into 8 */
    {
      unsigned char *x;
      int j, k;
      sp -= len;
      x = sp;
      for (j = 0, k = 1; j < len; ++j) {
        a[k++] = ((*x >> 4) & 0x0f) + '0';
        a[k++] = (*x++ & 0x0f) + '0';
      }
      if ((a[k-1] - '0') == 0x0c)      /* Signed '+' */
	a[0] = '+';
      else if ((a[k-1] - '0') == 0x0d) /* Signed '-' */
	a[0] = '-';
      else			       /* Unsigned */
	a[0] = ' ';
      a[k-1] = '\0';
      if (decimal > 0 && decimal < (k-1)) {	/* implied decimal */
	(*rpt_output)(stdout, "%.*s", k-decimal-1, a);
	(*rpt_output)(stdout, ".");
	(*rpt_output)(stdout, "%.*s", decimal, a+k-decimal-1);
      }
      else
	(*rpt_output)(stdout, "%s", a);
    }
    break; 

  case 't':			/* Pointer */
    sp -= len;
    (*rpt_output)(stdout, "0x%x", *((int *)sp));
    break;

  default:
    break;
  } /* end switch */

} /* end printpic */

/*
 * Fix up a COBOL PIC 9 and COMP-3 data types. Pop off the ASCII or 
 * packed bytes, convert them to integer, push the integer onto the stack
 * and change the nodetype of n to t_int.
 */
void fixCobolIntArg(Node n)
{
  Symbol t = rtype(n->nodetype);
  boolean neg = false;
  long l = 0;
  int len;
  unsigned char *buf;

  if (t != nil && (t->class == PIC || t->class == RPIC)) {
    int st = t->symvalue.usage.storetype;
    if ((st >= 'e' && st <= 'h') || st == 'q') { /* ASCII */
      int ndx = 0;
      len = size(t);
      sp -= len;
      buf = (char *)malloc(len+1);
      strncpy(buf, sp, len);
      buf[len] = '\0';
      switch(st) {
      case 'e':
	ndx = len - 1;		/* fall through */
      case 'g':
	if (buf[ndx] & 0x40) {
	  buf[ndx] -= 0x40;
	  neg = true;
	}
	break;
      case 'f':
	if (buf[len-1] == '-') {
	  buf[len-1] = '\0';
	  neg = true;
	}
	else if (buf[len-1] == '+')
	  buf[len-1] = '\0';
	break;
      case 'h':
      case 'q':
      default:
	break;
      }
      errno = 0;
      if (neg)
	l = -atoi(buf);
      else
	l = atoi(buf);
      if (errno == ERANGE) {
	beginerrmsg();
	(*rpt_error)(stderr, catgets(scmc_catd, MS_cobol, MSG_549,
	  "Operation results in an integer overflow or underflow condition."));
	enderrmsg();
      }
      free(buf);
      push(long, l);
      n->nodetype = t_int;
    }
    else if (st == 'k' || st == 'l') { /* packed */
      int j;
      len = size(t);
      sp -= len;
      buf = sp;
      for (j = 0; j < len-1; ++j) {
        l = l * 10 + ((*buf >> 4) & 0x0f);
        l = l * 10 + (*buf++ & 0x0f);
      }
      l = l * 10 + ((*buf >> 4) & 0x0f);
      if ((*buf & 0x0f) == 0x0d)
	l =  -l;
      push(long, l);
      n->nodetype = t_int;
    }
  }
}

/*
 * Fix a COBOL numeric data type with implied decimal place. Make sure what
 * is on the stack is an int, pop it off and divide by 10 until you have
 * the proper floating point value. Push this onto the stack and set the
 * node type to t_real.
 */
void fixCobolFloatArg(Node n)
{
  Symbol t = rtype(n->nodetype);
  int i;
  long l;
  double d;

  if (t != nil && (t->class == PIC || t->class == RPIC) &&
      t->symvalue.usage.decimal_align > 0) {
    fixCobolIntArg(n);
    l = popsmall(n->nodetype);
    for (d = l, i = 0; i < t->symvalue.usage.decimal_align; ++i)
      d /= 10;
    push(double, d);
    n->nodetype = t_real;
  }
}

/*
 * Change edit_descript from 'X(3)/X(3)BX' to 'XXX/XXXBX' to make
 * parsing easier during editing.
 */
static char *expandEditDescr(Symbol s)
{
  int i = 0;
  char *r = s->symvalue.usage.edit_descript;
  char *q = malloc(s->symvalue.usage.bytesize+1); /* Caller must free()! */

  while (*r != '\0' && i < s->symvalue.usage.bytesize) {
    if (*r == '(') {	/* Get repeat count */
      int j, n = 0;
      ++r;
      while (isdigit((int)(*r))) {
	n = 10*n + (*r - '0');
	++r;
      }
      if (*r == ')')
	++r;		/* move over ')' */
      else
	n = 0;		/* punt */
      for (j = 0; j < (n-1); ++j, ++i) /* Insert repeat chars */
	q[i] = q[i-1];
    }
    else {
      q[i++] = *r++;		/* Just copy the char */
    }
  }
  q[i] = '\0';
  return q;
}

/*
 * Pop exp off the stack (either int or real), apply the implied decimal
 * point correction, if any, and return the resulting long (truncating
 * any remaining decimal places).
 */
static long popCobolInt(Node exp, Symbol svar)
{
  int i;
  double d;
  Symbol sexp;
  int expsize, decimal = svar->symvalue.usage.decimal_align;

  if (isconst(exp->nodetype)) {	/* get exp size and symbol */
    sexp = exp->nodetype->symvalue.constval->nodetype;
    expsize = size(sexp);
  }
  else {
    sexp = exp->nodetype;
    expsize = size(sexp);
  }

  if (isreal(exp->op) || exp->op == O_FCON) { /* pop real */
    if (expsize == sizeof(double))
      d = pop(double);
    else
      d = pop(float);
  }
  else				/* pop int, convert to real */
    d = (double)popsmall(sexp);

  for(i = 0; i < decimal; ++i)	/* apply decimal adjustment */
    d *= 10;

  return itrunc(d);		/* truncate decimal places and return */
}

/*
 * Handle assigning to COBOL vars that require special handling not 
 * available in the language independant assign code.
 */ 
void cobolassign(Node var, Node exp)
{
  char buf[BUFSIZ];
  int varsize, expsize;
  Symbol svar, sexp;
  Address addr;
  char storetype;

  varsize = size(var->nodetype);
  if (isconst(exp->nodetype)) {
    sexp = exp->nodetype->symvalue.constval->nodetype;
    expsize = size(sexp);
  }
  else {
    sexp = exp->nodetype;
    expsize = size(sexp);
  }
  svar = rtype(var->nodetype);
  addr = lval(var);

  switch(storetype = svar->symvalue.usage.storetype) {
  case 'a':			/* Alpha */
  case 'c':			/* AlphaNum */
    sp -= expsize;
    sprintf(buf, "%-*s", varsize, sp); /* Left justified, space filled */
    dwrite(buf, addr, varsize);
    break;
  case 'b':			/* Alpha edited */
  case 'd':			/* AlphaNum edited */
    {
      int i;
      char *p = sp;
      char *q = expandEditDescr(svar);
      for (p -= expsize, i = 0; q[i] != nil; ++i) {
	if (q[i] == 'B')	/* 'B', '0' and '/' are special */
	  buf[i] = ' ';
	else if  (q[i] == '0' || q[i] == '/')
	  buf[i] = q[i];
	else {			/* Anything else must be actual data */
	  if (p < (sp-1))
	    buf[i] = *p++;
	  else
	    buf[i] = ' ';	/* Pad to the right with blanks */
	}
      }
      sp -= expsize;
      free(q);			/* malloc()'ed in expandEditDescr() */
      dwrite(buf, addr, varsize);
    }
    break;
  case 'e':			/* Numeric sign trailing */
  case 'g':			/* Numeric sign leading */
    {
      boolean neg = false;
      long i = popCobolInt(exp, svar);
      if (i < 0)
	i = -i, neg = true;
      sprintf(buf, "%.*d", varsize, i);
      if (neg)			/* Put in the sign bit */
	buf[(storetype == 'e') ? (varsize - 1) : 0] += 0x40;
      dwrite(buf, addr, varsize);
    }
    break;
  case 'f':			/* Numeric sign trailing separate */
    {
      boolean neg = false;
      long i = popCobolInt(exp, svar);
      if (i < 0)
	i = -i, neg = true;
      sprintf(buf, "%.*d%c", varsize-1, i, ((neg) ? '-' : '+'));
      dwrite(buf, addr, varsize);
    }
    break;
  case 'h':			/* Numeric sign leading separate */
    {
      long i = popCobolInt(exp, svar);
      sprintf(buf, "%+.*d", varsize-1, i);
      dwrite(buf, addr, varsize);
    }
    break;
  case 'i':			/* Integral types */
  case 'j':
  case 'm':
  case 'n':
  case 'o':
    {
      long i = popCobolInt(exp, svar);
      if (varsize <= sizeof(long))
	dwrite(((char *)&i) + sizeof(long) - varsize, addr, varsize);
      else
	dwrite((char *)&i, addr + varsize - sizeof(long), sizeof(long));
    }
    break;
  case 'k':			/* Signed packed */
    {
      int j, k;
      boolean neg = false;
      unsigned char *ubuf = buf;
      long i = popCobolInt(exp, svar);
      if (i < 0)
	i = -i, neg = true;
      sprintf(buf, "%.*d", (varsize*2) - 1, i);
      for (j = 0, k = 0; j < varsize-1; ++j, ++k) /* Pack 8 bits into 4 */
	ubuf[j] = (((ubuf[k] - '0') << 4) & 0xf0) + (ubuf[++k] - '0');
      ubuf[j] = (((ubuf[k] - '0') << 4) & 0xf0) + /* Last nibble of value */
	((neg) ? 0x0d : 0x0c);	/* Sign nibble */
      dwrite(ubuf, addr, varsize);
    }
    break;
  case 'l':			/* Unsigned packed */
    {
      int j, k;
      unsigned char *ubuf = buf;
      unsigned long ui = (unsigned long)popCobolInt(exp, svar);
      sprintf(buf, "%.*ud", (varsize*2) - 1, ui);
      for (j = 0, k = 0; j < varsize-1; ++j, ++k) /* Pack 8 bits into 4 */
	ubuf[j] = (((ubuf[k] - '0') << 4) & 0xf0) + (ubuf[++k] - '0');
      ubuf[j] = (((ubuf[k] - '0') << 4) & 0xf0) + /* Last nibble of value */
	0x0e;			/* Sign nibble */
      dwrite(ubuf, addr, varsize);
    }
    break;
  case 'p':			/* Numeric edited */
    numericEdit(exp, sexp, svar, expsize, varsize, addr, buf);
    break;
  case 'q':			/* Numeric unsigned */
  default:
    {
      unsigned long ui = (unsigned long)popCobolInt(exp, svar);
      sprintf(buf, "%.*ud", varsize, ui);
      dwrite(buf, addr, varsize);
    }
    break;
  }
}

/*
 * Perform COBOL numeric editing while assigning svar = sexp.
 * This is not a perfect implementation of COBOL numeric editing
 * rules, but it gets the most common cases right, and gives reasonable
 * values for the more obscure cases.
 */
static void numericEdit(Node exp, Symbol sexp, Symbol svar,
                        int expsize, int varsize, Address addr, 
                        char *buf)
{
  double d = 0.0;
  boolean dec = false, neg = false;
  boolean value_is_zero = false;
  boolean floating = false;
  long whole = 0;
  int i, wholesize = 0, fractsize = 0;
  char *r, *t;
  char *q = expandEditDescr(svar);
  char *p = strchr(q, '.');

				/* First get the whole number part */
				/* of the rhs. */
  if (isreal(exp->op) || exp->op == O_FCON) {
    if (expsize == sizeof(double))
      d = pop(double);
    else
      d = pop(float);
    whole = itrunc(d);
    if (d == 0.0)
      value_is_zero = true;
  }
  else {
    whole = popsmall(sexp);
    if (whole == 0)
      value_is_zero = true;
  }
  if (whole < 0)
    whole = -whole, neg = true;
				/* determine number of digits for the */
				/* whole and fractional part */
  if (p) {
    wholesize = p - q;
    fractsize = strlen(q) - wholesize - 1;
    dec = true;
  }
  else
    wholesize = varsize;
  
				/* Now start editing in the rhs */
  for (i = 0; q[i] != '\0'; ++i)
    if (q[i] == 'B' && (i == 0 || q[i-1] != 'D'))
      q[i] = ' ';		/* Simple Insertion, replace 'B' w/ space */
  if (wholesize > 0) {		/* Edit in the whole number part */
    boolean dollar_done = false;
    boolean sign_done = false;
    sprintf(buf, "%d", whole);
    r = buf + strlen(buf) - 1;	/* whole number */
    p = q + wholesize - 1;	/* edit description and target */
    while (p >= q && r >= buf) { /* fill in the numbers */
      switch(*p) {
      case '$':
	if (r == buf && *r == '0') { /* replace leading zero with $ */
	  p--, r--;
	  dollar_done = true;
	}
	else
	  *p-- = *r--;
	break;
      case '*':			/* zero suppress with '*' */
	if (r == buf && *r == '0')
	  --p, --r;
	else
	  *p-- = *r--;
	break;
      case 'Z':			/* zero suppress with space */
	if (r == buf && *r == '0') {
	  *p-- = ' ';
	  --r;
	}
	else
	  *p-- = *r--;
	break;
      case '9':			/* digits go here */
	*p-- = *r--;
	break;
      case '+':			/* could be digit, sign or space */
      case '-':
	if (p == q) {		/* leftmost position is sign position */
	  if (floating && value_is_zero)
	    *p-- = ' ';
	  else {
	    sign_done = true;	/* put in sign */
	    if (neg)
	      *p-- = '-';
	    else {
	      if (*p == '+')
		*p-- = '+';
	      else
		*p-- = ' ';
	    }
	  }
	}
	else {			/* this is a floating position */
	  floating = true;
	  if (value_is_zero) {
	    *p-- = ' ';		/* space it out */
	    --r;
	  }
	  else
	    *p-- = *r--;	/* put in the value */
	}
	break;
      default:
	--p;
	break;
      }
    }
    while (p >= q) {		/* take care of left filling */
      switch(*p) {
      case '$':
	if (dollar_done)	/* only put in one $ sign */
	  *p = ' ';
	else
	  dollar_done = true;
	break;
      case '+':			/* put in sign or space */
      case '-':
	if (p != q)
	  floating = true;
	if (floating && value_is_zero)
	  *p = ' ';
	else {
	  if (sign_done)	/* space it out */
	    *p = ' ';
	  else {		/* put in sign */
	    sign_done = true;
	    if (neg)
	      *p = '-';
	    else {
	      if (*p == '+')
		*p = '+';
	      else
		*p = ' ';
	    }
	  }
	}
	break;
      case ' ':			/* simple insertion editing  */
      case ',':
      case '0':
      case '/':
	t = strchr(q, '*');
	if (t != nil && t < p)	/* special case for '*' zero suppression */
	  *p = '*';
	else
	  *p = ' ';
	break;
      case '9':			/* left fill with zeros */
	*p = '0';
	break;
      case 'Z':			/* left fill with spaces */
	*p = ' ';
	break;
      }
      --p;
    }
  }
				/* Now do the fractional part */
  if (dec && fractsize > 0) {
    sprintf(buf, "%.*f", fractsize+1, d);
    t = strchr(buf, '.');	/* skip past decimal point */
    ++t;
    p = strchr(q, '.');
    ++p;			/* edit description and target */
    r = t;			/* fraction part of number */
    while (*p != '\0' && (p-q) < varsize) {
      switch(*p) {
      case '$':
      case 'Z':
      case '*':
	if (value_is_zero) {	/* suppress with '*' or space */
	  if (*p == '*')
	    p++;
	  else
	    *p++ = ' ';
	  if (*r != '\0')
	    ++r;
	}
	else {			/* put in value or trailing zero */
	  if (*r == '\0')
	    *p++ = '0';
	  else
	    *p++ = *r++;
	}
	break;
      case '9':			/* put in number, right fill w/ zeros */
	if (*r == '\0')
	  *p++ = '0';
	else
	  *p++ = *r++;
	break;
      case 'C':			/* CR or DB sign */
      case 'R':
      case 'D':
      case 'B':
	if (!neg)
	  *p = ' ';
	++p;
	break;
      case '+':
      case '-':
	if (p == (q+varsize-1) && !floating) { /* sign position */
	  if (neg)
	    *p++ = '-';
	  else {
	    if (*p == '+')
	      *p++ = '+';
	    else
	      *p++ = ' ';
	  }
	}
	else {			/* floating insertion */
	  floating = true;
	  if (value_is_zero) {
	    *p++ = ' ';
	    if (*r != '\0')
	      ++r;
	  }
	  else {
	    if (*r == '\0')
	      *p++ = '0';
	    else
	      *p++ = *r++;
	  }
	}
	break;
      default:
	++p;
	break;
      }
    }
  }
				/* If all that is left is spaces and */
				/* decimal points, space it all out. */
  p = q;
  while (*p != '\0') {
    if (*p != ' ' && *p != '.')
      break;
    ++p;
  }
  if (*p == '\0') {
    p = q;
    while (*p != '\0') {
      if (*p == '.')
	*p = ' ';
      ++p;
    }
  }

  dwrite(q, addr, varsize);
  free(q);			/* malloc()'ed in expandEditDescr() */
}

static push_group () 
{   
    temp_group = current_group;
    if (current_group = create_group (0)) {
        current_group->group_id = 0;
        if (temp_group == nil) {
            current_group->record = 1;
            current_group->field = 2;
            current_group->previous = nil;
	  }
        else {
            current_group->record = temp_group->field; 
            current_group->field = current_group->record + 1; 
            current_group->previous = temp_group;
	  }
    }
    else {
        (*rpt_output)(stdout, catgets(scmc_catd, MS_cobol, MSG_514,
							  "out of memory \n"));
        exit(1);
    }
} /* end push_group */

static pop_group () 
{
    struct group *n, *m;
    n = m = current_group;

    while (n != nil)  {
            temp_group = current_group;
            n = current_group = temp_group->previous;
            free ((struct group *)(temp_group));
    }   
} /* end pop_group */

static push_field () 
{
    struct group *temp;
    int temp_field;

    if (!current_group ) {
        push_group ();
        current_group->record = 1;
        current_group->field = 2;        
    }
    else {
        create_group (current_group->group_id);
        if (current_group->previous != nil) { 
            current_group->record = current_group->previous->record;
            current_group->field = current_group->record + 1;
	}
        else {
            current_group->record = 1;
            current_group->field = 2;
	}
      }


} /* end push_field */

static  pop_field () 
{
     if (current_group == nil) {
               field_level = 1;
               record_level = 1; 
               return;
	     }
      temp_group = current_group;
      current_group = temp_group->previous;
      free ((struct group *)(temp_group));
      if (current_group)
           field_level = current_group->field;
      else
           field_level = 1; 
} /* end pop_field */

static struct group *create_group (int id)
{

     temp_group = current_group;
            /* create a new node if there are none */
     if (!(current_group = (struct group *) malloc (sizeof(struct group)))) { 
          (*rpt_output)(stdout, catgets(scmc_catd, MS_cobol, MSG_515,
			"no more memory while creating a cobol level record"));
          exit (1);
     }
     current_group->previous = temp_group;
     current_group->field = 0;
     current_group->group_id = id;
     return current_group;
} /* end create_group */

/*
 * organization(s) - return name of file descriptor organization for a symbol
 */
static char *organization(Symbol s)
{
  char *r;

  switch (s->symvalue.usage.edit_descript[0])
    {
    case 'i':
      r = "Indexed";
      break;
    case 'l':
      r = "Line-Sequential";
      break;
    case 'r':
      r = "Relative";
      break;
    case 's':
      r = "Sequential";
      break;
    default:
      r = "Unknown";
      break;
    }

  return r;
}
/*
 * access(s) - return name of file descriptor access for a symbol
 */
static char *access(Symbol s)
{
  char *r;

  switch (s->symvalue.usage.edit_descript[1])
    {
    case 'd':
      r = "Dynamic";
      break;
    case 'o':
      r = "Sort";
      break;
    case 'r':
      r = "Random";
      break;
    case 's':
      r = "Sequential";
      break;
    default:
      r = "Unknown";
      break;
    }

  return r;
}

static void printCondValue(Symbol s)
{
  if (s->symvalue.usage.storetype == 'n') { /* numeric */
    (*rpt_output)(stdout, " %s", s->symvalue.usage.edit_descript);
  }
  else if (s->symvalue.usage.storetype == 'f') { /* figurative */
    switch(*(s->symvalue.usage.edit_descript)) {
    case '0':
      (*rpt_output)(stdout, " ZEROS");
      break;
    case 'S':
      (*rpt_output)(stdout, " SPACES");
      break;
    case 'Q':
      (*rpt_output)(stdout, " QUOTES");
      break;
    case 'L':
      (*rpt_output)(stdout, " LOW-VALUES");
      break;
    case 'H':
      (*rpt_output)(stdout, " HIGH-VALUES");
      break;
    case 'N':
    default:
      (*rpt_output)(stdout, " ALL \"%s\"", s->symvalue.usage.edit_descript);
      break;
    }
  }
  else {			/* alphanumeric */
    (*rpt_output)(stdout, " \"%s\"", s->symvalue.usage.edit_descript);
  }
}

/*
 * NAME: islang_cobol
 *
 * FUNCTION: Returns if the specified language is cobol
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * PARAMETERS:
 *    lang    - Language to test
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: true      - If lang is cobol
 *        false       - If lang is not cobol
 */
Boolean islang_cobol(Language lang)
{
    return lang == cobLang ? true : false;
}
