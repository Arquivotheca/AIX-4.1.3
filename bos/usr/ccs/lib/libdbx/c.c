static char sccsid[] = "@(#)30	1.32.4.18  src/bos/usr/ccs/lib/libdbx/c.c, libdbx, bos411, 9433A411a 8/15/94 14:13:46";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: ansic_listparams, c_buildaref, classname, c_evalaref,
 *	      c_foldnames, c_hasmodules, c_init, c_listparams, c_modinit,
 *	      c_passaddr, c_printdecl, c_printdef, c_printstruct, 
 *	      c_printval, c_typematch, C_printdecl,
 *	      C_printtype, printunion
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1982 Regents of the University of California
 *
 */

/*
 * C-dependent symbol routines.
 */

#ifdef KDBXRT
#include "rtnls.h"		/* MUST BE FIRST */
#endif
/*              include file for message texts          */
#include "dbx_msg.h" 
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
#define GetMsg(X, Y) catgets(scmc_catd, MS_c, (X), (Y))

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
#include "cplusplus.h"
#include "object.h"

static void C_printdecl(Symbol, int);
static void c_printstruct (Symbol);

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
extern boolean unique_fns;
extern Symbol Subar_save_sym;
extern Language fLang;

/*  array is zeroed out because it is static  */
private short usetype[TP_NTYPES];

/*
 * Initialize C language information.
 */

Language cLang;

void c_init()
{
    int i;
    Language lang;
    
    cLang = language_define("c", ".c");
    lang = language_define("c", ".y"); /* yacc */
    assert(lang == cLang);
    lang = language_define("c", ".i"); /* preprocessor */
    assert(lang == cLang);
    lang = language_define("c", ".m"); /* Objective C */
    assert(lang == cLang);

    language_setop(cLang, L_PRINTDECL, c_printdecl);
    language_setop(cLang, L_PRINTVAL, c_printval);
    language_setop(cLang, L_TYPEMATCH, c_typematch);
    language_setop(cLang, L_BUILDAREF, buildaref);
    language_setop(cLang, L_EVALAREF, c_evalaref);
    language_setop(cLang, L_MODINIT, c_modinit);
    language_setop(cLang, L_HASMODULES, c_hasmodules);
    language_setop(cLang, L_PASSADDR, c_passaddr);
    language_setop(cLang, L_FOLDNAMES, c_foldnames);

    usetype[-TP_WCHAR-1] = 1;
}

/*
 * Test if two types are compatible.
 */

#define isinttype(t) \
     isintegral((t)->type->name) || ischar((t)->type->name) \
  || streq(ident((t)->type->name),"$boolean")

Boolean c_typematch(Symbol type1, Symbol type2)
{
    Boolean b, isintegral(), ischar();
    Symbol t1, t2;

    if (type1 == type2)
      return true;

    b = false;

    t1 = rtype (type1);
    t2 = rtype (type2);

    if (isinttype(t1))
      b = isinttype(t2);
    else if (t1->class == REAL)
      b = t2->class == REAL;
    else if (t1->type == t2->type && (
            (t1->class == t2->class) ||
            (t1->class == SCAL && t2->class == CONST) ||
            (t1->class == CONST && t2->class == SCAL)))

      b = true;
    else if (t1->class == PTR && cpp_typematch(t1->type, t_char) &&
	    t2->class == ARRAY && cpp_typematch(t2->type, t_char))
      b = true;
    else if (t1->class == PTR &&
                   (t2->class == FUNC || t2->class == CSECTFUNC) &&
                   (cpp_typematch(t1->type->type, t2->type)))
      b = true;
    else if ((t1->class == PTR) && (t2->class == PTR) &&
		   cpp_typematch(t1->type,t2->type))
      b = true;
    /* Addresses are compatible with any pointers */
    else if ((t1->class == PTR) && (streq(ident(type2->name),"$address")))
      b = true;

    return b;
}

/*
 * Print out the declaration of a C variable.
 */

void c_printdecl(Symbol s)
{
    C_printdecl(s, 0);
}

static void C_printdecl(Symbol s, int indent)
{
    Boolean semicolon, newline;
    String fn;

    semicolon = true;
    newline = true;
    if (indent > 0) {
	(*rpt_output)(stdout, "%*c", indent, ' ');
    }
    if (s->class == TYPE) {
	(*rpt_output)(stdout, "typedef ");
    }
    switch (s->class) {
	case CONST:
	    if (s->type->class == SCAL) {
		(*rpt_output)(stdout, "enumeration constant with value ");
		eval(s->symvalue.constval);
                /* handles printing enum of type unsigned int... */
                if (size(s->type->type) < size(t_int))
                   cpp_printval(t_int, 0);
                else
                   cpp_printval(s->type->type, 0);
	    } else {
		(*rpt_output)(stdout, "const %s = ", symname(s));
		printval(s, 0);
	    }
	    break;

	case TYPE:
	case VAR:
	case TOCVAR:
	    if (s->class != TYPE and s->storage == INREG) {
		(*rpt_output)(stdout, "register ");
	    }
	    c_printdef(s, indent);
	    break;

	case FIELD:
	    c_printdef(s, indent);
	    if (isbitfield(s)) {
		(*rpt_output)(stdout, " : %d", s->symvalue.field.length);
	    }
	    break;

	case TAG:
	    if (s->type == nil) {
		findtype(s);
		if (s->type == nil) {
		    error(GetMsg(MSG_6, "unexpected missing type information"));
		}
	    }
	    cpp_printtype(s, s->type, indent);
	    break;

	case REAL:
	case RANGE:
	case ARRAY:
	case RECORD:
	case UNION:
	case PTR:
	case FFUNC:
	    semicolon = false;
	    cpp_printtype(s, s, indent);
	    break;

	case SCAL:
	    (*rpt_output)(stdout, 
		    "(enumeration constant, value %d)", s->symvalue.iconval);
	    break;

	case PROC:
	    semicolon = false;
	    (*rpt_output)(stdout, "%s", symname(s));
	    c_listparams(s);
	    newline = false;
	    break;

	case FUNC:
	case CSECTFUNC:
	    semicolon = false;
	    if (not istypename(s->type, "void")) {
		cpp_printtype(s, s->type, indent);
		(*rpt_output)(stdout, " ");
	    } else (*rpt_output)(stdout, "void ");
	    (*rpt_output)(stdout, "%s", symname(s));
	    c_listparams(s);
	    newline = false;
	    break;

	case MODULE:
	    semicolon = false;
	    fn = symname(s);
	    (*rpt_output)(stdout, GetMsg(MSG_11,
			   "source file \"%s.c\""), (unique_fns) ? ++fn : fn);
	    break;

	case PROG:
	    semicolon = false;
	    (*rpt_output)(stdout, GetMsg(MSG_12,
				"executable file \"%s\""), symname(s));
	    break;

	case MEMBER: 
	case CLASS: 
	case BASECLASS: 
	case NESTEDCLASS: 
	case CPPREF:
	case PTRTOMEM: 
	case ELLIPSES: 
	case FRIENDFUNC: 
	case FRIENDCLASS:
	    assert(false);

	default:
	    (*rpt_output)(stdout, "[%s]", classname(s));
	    break;
    }
    if (semicolon) {
	(*rpt_output)( stdout, ";");
    }
    if (newline) {
	(*rpt_output)( stdout, "\n");
    }
}

void c_printdef (Symbol s, int indent)
{
    register Symbol t;
    int rangetop;

    if (s->type->class == ARRAY) 
    {
	t = s;
	while (t->type->class == ARRAY)
            t = t->type;
	cpp_printtype(t, t->type, indent);
	(*rpt_output)(stdout, " ");
	(*rpt_output)(stdout, "%s", symname(s));
	while (s->type->class == ARRAY) 
	{
	    t = rtype(s->type->chain);
	    s = s->type;
	    assert(t->class == RANGE);
	    if ((rangetop = t->symvalue.rangev.upper) == -1)
		(*rpt_output)(stdout, "[]");
	    else
		(*rpt_output)(stdout, "[%d]", rangetop+1);
	}
    } 
    else if (s->type->class == PTR && s->type->type != nil && 
             s->type->type->class == FFUNC)
    {
        char *buf = (char *)malloc(strlen(symname(s)) + 7);

	t = s->type->type;
        sprintf(buf, "(*%s)()", symname(s));
        while (t->type->class == PTR && t->type->type &&
               t->type->type->class == FFUNC)
	{
            char *tbuf = (char *)malloc(strlen(buf) + 6);
            sprintf(tbuf, "(*%s)()", buf);
            free(buf);
            buf = tbuf;
            t = t->type->type;
        }

	cpp_printtype(s, t->type, indent);
        (*rpt_output)(stdout, " %s", buf);
	free(buf);
    } 
    else 
    {
	cpp_printtype(s, s->type, indent);
	t = s->type;
	while (t->name != nil && t->name->identifier[0] == '?') {
	    t = t->type;
	}
	while (nilname(t) && (t->class == TYPE))
	    t = t->type; 
	
        /* don't print a c++ temp parameter or an ellipses name */
        if (!(s->language == cppLang && cpp_tempname(s->name)))
        {
           /* if we are printing out a pointer to a c++ member then do not */
	   /* put out the variable name because it has already been output */ 
	   if (t->class != PTRTOMEM) {
	      if (t->class != PTR && t->class != CPPREF)
                 (*rpt_output)(stdout, " ");
              (*rpt_output)(stdout, "%s", symname(s));
           }
        }
    }
}

void C_printtype (Symbol s, Symbol t, int indent)
{
    register Symbol i;
    long r0, r1;
    register String p;
    int prevEnumVal;

    checkref(s);
    checkref(t);
    switch (t->class) {
	case VAR:
	case CONST:
	case PROC:
	case TOCVAR:
	    panic(GetMsg(MSG_21, "printtype: class %s"), classname(t));
	    break;

	case ARRAY:
	    cpp_printtype(t, t->type, indent);
	    i = t->chain;
	    if (i != nil) {
		i = rtype(i);
		(*rpt_output)(stdout, "[%d]", i->symvalue.rangev.upper + 1);
	    } else {
		(*rpt_output)(stdout, "[]");
	    }
	    break;

	case RECORD:
	case UNION:
	    (*rpt_output)(stdout, "%s ", c_classname(t));
	    if (s->class == TAG && !nilname(s)) {
		p = symname(s);
		if (p[0] == '$' and p[1] == '$') {
		    (*rpt_output)(stdout, "%s ", &p[2]);
		} else {
		    (*rpt_output)(stdout, "%s ", p);
		}
	    }
	    (*rpt_output)(stdout, "{\n");
	    for (i = t->chain; i != nil; i = i->chain) {
		assert(i->class == FIELD);
		C_printdecl(i, indent+4);
	    }
	    if (indent > 0) {
		(*rpt_output)(stdout, "%*c", indent, ' ');
	    }
	    (*rpt_output)(stdout, "}");
	    break;

	case RANGE:
	    r0 = t->symvalue.rangev.lower;
	    r1 = t->symvalue.rangev.upper;
	    if (ischartype(t)) {
		if (r0 < 0x20 or r0 > 0x7e) {
		    (*rpt_output)(stdout, "%ld..", r0);
		} else {
		    (*rpt_output)(stdout, "'%c'..", (char) r0);
		}
		if (r1 < 0x20 or r1 > 0x7e) {
		    (*rpt_output)(stdout, "\\%lo", r1);
		} else {
		    (*rpt_output)(stdout, "'%c'", (char) r1);
		}
	    } else if (r0 > 0 and r1 == 0) {
		(*rpt_output)(stdout, GetMsg(MSG_34, "%ld byte real"), r0);
            } else if (t->symvalue.rangev.size == sizeofLongLong) {
                if (t->symvalue.rangev.is_unsigned) {
                  (*rpt_output)(stdout, "0..18446744073709551615");
                } else {
                  (*rpt_output)
                  (stdout, "-9223372036854775808..9223372036854775807");
                }
	    } else if (r0 >= 0) {
		(*rpt_output)(stdout, "%lu..%lu", r0, r1);
	    } else {
		(*rpt_output)(stdout, "%ld..%ld", r0, r1);
	    }
	    break;

	case REAL:
	    switch(t->symvalue.size) {
		case sizeof(float):
			 (*rpt_output)(stdout, "float ");
			 break;
		case sizeof(double):
			 (*rpt_output)(stdout, "double ");
			 break;
		case 2 * sizeof(double):
			 (*rpt_output)(stdout, "long double ");
			 break;
		default: (*rpt_output)(stdout, GetMsg(MSG_40, "%d byte real "),
					       t->symvalue.size);
			 break;
	    }
	    break;

	case PTR:
	    cpp_printtype(t, t->type, indent);
	    if (t->type->class != PTR) {
		(*rpt_output)(stdout, " ");
	    }
	    (*rpt_output)(stdout, "*");
	    break;

	case FUNC:
	case CSECTFUNC:
	case FFUNC:
	    cpp_printtype(t, t->type, indent);
	    (*rpt_output)(stdout, "()");
	    break;

	case TYPE:
	    if (nilname(t)) {
		cpp_printtype(t, t->type, indent);
	    } else {
		printname( rpt_output, stdout, t, false);
	    }
	    break;

	case TYPEREF:
	    (*rpt_output)(stdout, "@%s", symname(t));
	    break;

	case SCAL:
	    (*rpt_output)(stdout, "enum ");
	    if (s->class == TAG && !nilname(s) && !cpp_tempname(s->name))
	    {
		(*rpt_output)(stdout, "%s ", symname(s));
	    }
	    (*rpt_output)(stdout, "{ ");
	    i = t->chain;
            prevEnumVal = -1;
            if (i != nil) {
                for (;;) 
		{
                    (*rpt_output)(stdout, "%s", symname(i));
                    if (i->symvalue.constval->value.lcon != prevEnumVal + 1)
                        /* handle unsigned int values... */
                        if (i->type->class == SCAL &&
                            istypename(i->type->type,"unsigned int"))
                                (*rpt_output)(stdout, " = %u",
                                      i->symvalue.constval->value.lcon);
                        else
                            (*rpt_output)(stdout, " = %d", 
                                      i->symvalue.constval->value.lcon);   
                    prevEnumVal = i->symvalue.constval->value.lcon;
                    i = i->chain;
                    if (i == nil) break;
                    (*rpt_output)(stdout, ", ");
                }
            }

	    (*rpt_output)(stdout, " }");
	    break;

	case TAG:
	    if (t->type == nil) 
	    {
		(*rpt_output)(stdout, GetMsg(MSG_51, "unresolved tag %s"), 
				      symname(t));
	    } 
	    else if (!nilname(t) &&
                    !(t->language==cppLang && cpp_tempname(t->name)))
	    {
	        Symbol qualType, tag;
		String qual;

		(*rpt_output)(stdout, "%s ", c_classname(rtype(t->type)));

		qual = "";
		tag = rtype(t->type);
		if (tag && tag->block)
		   tag = tag->block->block;
		else
		   tag = nil;
		qualType = (tag) ? rtype(tag->type) : nil;
		while (qualType && qualType->class == CLASS)
		{
		    String className = ident(tag->name);
		    String oldqual = qual;

		    qual = malloc(strlen(className) + strlen(qual) + 3);
		    (void)strcpy(qual, className);
		    (void)strcat(qual, "::");
		    (void)strcat(qual, oldqual);
		    if (strlen(oldqual) > 0)
		        free(oldqual);

		    tag = tag->block;
		    qualType = rtype(tag->type);
		}
		(*rpt_output)(stdout, "%s%s", qual, symname(t));
		if (strlen(qual) > 0)
		    free(qual);
	    } 
	    else 
	    {
		cpp_printtype(t, t->type, indent);
	    }
	    break;

	case MEMBER: 
	case CLASS: 
	case BASECLASS: 
	case NESTEDCLASS: 
	case CPPREF:
	case PTRTOMEM: 
	case ELLIPSES: 
	case FRIENDFUNC: 
	case FRIENDCLASS:
	    assert(false);

	default:
	    (*rpt_output)(stdout, GetMsg(MSG_53, "(class %d)"), t->class);
	    break;
    }
}

/*
 * List the parameters of a procedure or function.
 * No attempt is made to combine like types.
 */

void c_listparams(Symbol s)
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
		panic(GetMsg(MSG_57, "unexpected class %d for parameter"), 
		      t->class);
	    }
	    C_printdecl(t, 0);
	}
    } else {
	(*rpt_output)( stdout, "\n");
    }
}

/*
 * List the parameters of a procedure or function in ANSI C prototype format.
 * Also used for c++ args - skips the cfront generated first arg 'this'.
 */

void ansic_listparams(Symbol s)
{
  register Symbol t;
  
  (*rpt_output)( stdout, "(");
  if (s->chain != nil) {
    for (t = s->chain; t != nil; t = t->chain) {
      if (t->class != VAR)
	  panic(GetMsg(MSG_57, "unexpected class %d for parameter"), t->class);

      /* skip "this" and compiler generated parameters (excl. "...") */
      if (s->language == cppLang && (t->name == this || 
	  cpp_tempname(t->name) && t->type->class != ELLIPSES))
          continue;

      c_printdef(t, 0);
      if (t->chain != nil)
	  (*rpt_output)(stdout, ", ");
    }
  }
  (*rpt_output)( stdout, ")");
}

/*
 * Print out the value on the top of the expression stack
 * in the format for the type of the given symbol.
 */

extern Boolean expandunions;

void c_printval(Symbol s, int indent)
{
    Symbol t;
    Address a;
    integer i, len;
    Symclass class = s->class;

    while ((class == CONST) || (class == TYPE) || (class == VAR) ||
           (class == REF) || (class == FVAR) || (class == TAG) ||
           (class == TOCVAR))
    {
      s = s->type;
      class = s->class;
    }

    if (subarray)
      Subar_save_sym = s;

    switch (class) {
	case UNION:
	  if (subarray)
	  {
	    printsubarray(0, subdim_tail, array_sym->type, UNION);
	    reset_subarray_vars();
	  }
          else if (expandunions)
	    printunion(s);
          else
          {
	    if (ord(s->class) > ord(LASTCLASS)) {
		panic("printval: bad class %d", ord(s->class));
	    }
	    sp -= size(s);
	    PRINTF "[%s]", c_classname(s));
          }
	  break;

	case FIELD:
	    if (isbitfield(s)) {
		t = rtype(s->type);
		i = extractField(s, t);
		if (t->class == SCAL) {
		    printEnum(i, t);
		} else {
		    printRangeVal(i, t);
		}
	    } else {
		c_printval(s->type, indent);
	    }
	    break;

	case ARRAY:
	    t = rtype(s->type);
	    if ((t->class == RANGE and ischartype(t)) or
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
	    if (subarray)
	    {
		printsubarray(0, subdim_tail, array_sym->type, RECORD);
	        reset_subarray_vars();
	    }
	    else
	        c_printstruct(s);
	    break;

	case RANGE:
	    if (subarray)
	    {
                if(array_sym->language == fLang)
		   fortran_printsubarray(0, subdim_tail, array_sym, RANGE);
		else
		   printsubarray(0, subdim_tail, 
		      (array_sym->class == ARRAY) ? array_sym : array_sym->type,
		      RANGE);
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
	       if (a == 0) {
		   PRINTF "(nil)");
	       } else if (t->class == RANGE and ischartype(t)) {
		   printString(a, (boolean) (s->language != primlang));
	       } else {
		   PRINTF "0x%x", a);
	       }
            }
	    break;

	case REAL:
	    if (subarray)
	    {
		if(array_sym->language == fLang)
		   fortran_printsubarray(0, subdim_tail, array_sym, REAL);
		else
		   printsubarray(0, subdim_tail, array_sym->type, REAL);
	        reset_subarray_vars();
	    }
	    else {
		printreal(s);
	    }
	    break;

	case SCAL:
	    if (subarray)
	    {
		printsubarray(0, subdim_tail, array_sym->type, SCAL);
	        reset_subarray_vars();
	    }
	    else
            {
               /* c++ and c enums both can have varying sizes */
               i = popsmall(s->type);
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

	case MEMBER: 
	case CLASS: 
	case BASECLASS: 
	case NESTEDCLASS: 
	case CPPREF:
	case PTRTOMEM: 
	case ELLIPSES: 
	case FRIENDFUNC: 
	case FRIENDCLASS:
	    assert(false);

	default:
	    if (ord(s->class) > ord(LASTCLASS)) {
		panic("printval: bad class %d", ord(s->class));
	    }
	    sp -= size(s);
	    PRINTF "[%s]", c_classname(s));
	    break;
    }
}

/*
 * Print out a C structure.
 */

static void c_printstruct (Symbol s)
{
    Symbol f;
    Stack *savesp;
    integer n, off, len;

    sp -= size(s);
    savesp = sp;
    PRINTF "(");
    f = s->chain;
    if (f) {
    	for (;;) {
		off = f->symvalue.field.offset;
		len = f->symvalue.field.length;
		n = (off + len + BITSPERBYTE - 1) / BITSPERBYTE;
		sp += n;
		PRINTF "%s = ", symname(f));
		cpp_printval(f, 0);
		sp = savesp;
		f = f->chain;
    		if (f == nil) break;
		PRINTF ", ");
    	}
    }
    PRINTF ")");
}

/*
 * Return the C name for the particular class of a symbol.
 */

String c_classname(Symbol s)
{
    String str;

    switch (s->class) {
	case CLASS: /* implies C++ code */
	    switch (s->symvalue.class.key) {
		case 's': return "struct";
		case 'u': return "union";
		case 'c': return "class";
	    }
	case RECORD: return "struct";
	case UNION: return "union";
	case SCAL: return "enum";
	default: return classname(s);
    }
}

/*
 * Evaluate a subscript index.
 */

void c_evalaref(Symbol s, Address base, long i)
{
    Symbol t;
    long lb, ub;

    t = rtype(s);
    if (s->class != PTR)
    {
       s = t->chain;
       if (s != NULL)
       {
           lb = s->symvalue.rangev.lower;
           ub = s->symvalue.rangev.upper;
           if ((i < lb or i > ub) && (!varIsSet("$unsafebounds"))) {
               error(GetMsg(MSG_70, "subscript out of range"));
           }
       }
       else
           lb = 0;

       push(long, base + (i - lb) * size(t->type));
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

void c_modinit ()
{
    addlangdefines(usetype);
}

boolean c_hasmodules ()
{
    return false;
}

boolean c_passaddr (Symbol param, Symbol exprtype)
{
    boolean b;
    Symbol t;

    t = rtype(exprtype);
    b = (boolean) (t->class == ARRAY);
    return b;
}

cases c_foldnames ()
{
    return mixed;
}

void printunion(Symbol s)
{
   Symbol t;
   Stack *savesp;
   Symbol f;
   integer n, off, len;
   long i;

   PRINTF "union: (");
   f = s->chain;
   sp -= size(s);
   savesp = sp;
   for (;;)
   {
	off = f->symvalue.field.offset;
	len = f->symvalue.field.length;
	n = (off + len + BITSPERBYTE - 1) / BITSPERBYTE;
	sp += n;
	PRINTF "%s = ", symname(f));
	cpp_printval(f, 0);
	f = f->chain;
	sp = savesp;
        if (f == nil) break;
        PRINTF ", ");
   }
   PRINTF ")");
}
