static char sccsid[] = "@(#)70	1.40.1.6  src/bos/usr/ccs/lib/libdbx/pascal.c, libdbx, bos411, 9428A410j 5/2/94 11:23:36";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: doStringPtr, sizematch, gettag, pascal_buildaref,
 *	      pascal_evalaref, pascal_foldnames, pascal_hasmodules,
 *	      pascal_init, pascal_modinit, pascal_passaddr, pascal_printdecl,
 *	      pascal_printval, pascal_typematch, printPstring, printVarDecl,
 *	      printVrecord, pas_printfield, setmatch, indent,
 *	      listparams, printEnumDecl, printRangeDecl, isparam, rangematch,
 *	      ArrayTypeName, isChosen, printVVrecord, printVfield, pop_pas_ptr,
 *	      funcMatch, translate_type_name 
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
 * Pascal-dependent symbol routines.
 */

#include "defs.h"
#include "symbols.h"
#include "languages.h"
#include "tree.h"
#include "eval.h"
#include "mappings.h"
#include "process.h"
#include "runtime.h"
#include "machine.h"
#include "printsym.h"
#include <ctype.h>

#define isparam(p) ((p)->param)

static boolean initialized;
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
extern boolean expandunions;

static void pas_printtype(Symbol, Symbol, int);
static void printVarDecl(Symbol, int);
static void printRecordDecl(Symbol, int);
static void printRangeDecl(Symbol);
static void printEnumDecl(Symbol, int);
static void listparams(Symbol);
static void pas_prval(Symbol, int);
static void pas_printfield(Symbol);
static void doStringPtr();
static void printSetOfEnum(Symbol);
static void printSetOfRange(Symbol, Symbol);

/*  array is zeroed out because it is static  */
static short usetype[TP_NTYPES];
static void translate_type_name();

/*
 * Initialize Pascal information.
 */

Language pascalLang;

void pascal_init()
{
    pascalLang = language_define("pascal", ".pas");
    pascalLang = language_define("pascal", ".p");
    language_setop(pascalLang, L_PRINTDECL, pascal_printdecl);
    language_setop(pascalLang, L_PRINTVAL, pascal_printval);
    language_setop(pascalLang, L_TYPEMATCH, pascal_typematch);
    language_setop(pascalLang, L_BUILDAREF, buildaref);
    language_setop(pascalLang, L_EVALAREF, pascal_evalaref);
    language_setop(pascalLang, L_MODINIT, pascal_modinit);
    language_setop(pascalLang, L_HASMODULES, pascal_hasmodules);
    language_setop(pascalLang, L_PASSADDR, pascal_passaddr);
    language_setop(pascalLang, L_FOLDNAMES, pascal_foldnames);

    initialized = false;

    usetype[-TP_PASINT-1] = 1;
    usetype[-TP_BOOL-1] = 1;
    usetype[-TP_SHRTREAL-1] = 1;
    usetype[-TP_REAL-1] = 1;
    usetype[-TP_STRNGPTR-1] = 1;
    usetype[-TP_FCHAR-1] = 1;
    usetype[-TP_WCHAR-1] = 1;
}

/*
 * Typematch tests if two types are compatible.  The issue
 * is a bit complicated, so several subfunctions are used for
 * various kinds of compatibility.
 */

static boolean builtinmatch (Symbol t1, Symbol t2)
{
    boolean b;

    b = (boolean) (
        (
	    (t2 == t_int->type or istypename(t2->type, "$integer")) and
	     t1->class == RANGE and ( istypename(t1->type, "int")   
			           or istypename(t1->type, "integer"))
	) or (
	    (t2 == t_char->type or istypename(t2->type, "$char")) and
	     t1->class == RANGE and istypename(t1->type, "char")
	) or (
	    t2 == t_real->type and
	    t1->class == REAL and ( istypename(t1->type, "float")  
                                 or istypename(t1->type, "real"))
	) or (
	    t2 == t_boolean->type and
	    t1->class == RANGE and istypename(t1->type, "boolean")
	)
    );
    return b;
}

static boolean rangematch (Symbol t1, Symbol t2)
{
    boolean b;
    register Symbol rt1, rt2;

    if ((streq(t1->type->name->identifier,"short")||
          streq(t1->type->name->identifier,"integer")) &&
          streq(t2->type->name->identifier,"$integer"))
	b = true;
    else if (t1->class == RANGE or t1->class == PACKRANGE)
	{
	  rt1 = rtype(t1->type);
          if (t2->class == RANGE or t2->class == PACKRANGE) 
          {
	    rt2 = rtype(t2->type);
	    b =  (boolean) (builtinmatch(rt1, rt2) or rt1->type==rt2->type);
	  }
          else b = (boolean) (builtinmatch(rt1, t2) or 
                              rt1->type == t2->type);
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

static boolean funcMatch (Symbol t1, Symbol t2)
{
    boolean b;

    b = (boolean) (
          (t1->class == FUNC or t1->class == FUNCPARAM)
          and (t2->class == FUNC or t2->class == FUNCPARAM)
          and pascal_typematch(t1->type, t2->type)
          or
          (t1->class == PROC or t1->class == PROCPARAM)
          and  (t2->class == PROC or t2->class == PROCPARAM)
    );
    return b;
}


static boolean isConstString (Symbol t)
{
    boolean b;

    b = (boolean) 
        (
	  t->language == primlang 
          and ((t->class == ARRAY or t->class== PACKARRAY)
						         or t->class == STRING)
          and t->type == t_char
        );
    return b;
}

static boolean stringArrayMatch (Symbol t1, Symbol t2)
{
    boolean b;

    b = (boolean) (
	(
	    isConstString(t1)
	    and ((t2->class == ARRAY or t2->class == PACKARRAY) 
            or (t2->class == STRING)) and compatible(t2->type, t_char->type)
	) or (
	    isConstString(t2)
	    and ((t1->class == ARRAY or t1->class == PACKARRAY)
            or (t1->class == STRING)) and compatible(t1->type, t_char->type)
	)
    );
    return b;
}


static boolean setmatch (Symbol t1, Symbol t2)
{
   boolean b;

   if ( streq(ident(t2->name), "$setcon")   /* if it is Constant Set, */
      and (t2->type->symvalue.rangev.upper < t2->type->symvalue.rangev.lower)) 
      return true;			    /* and it is Empty, return TRUE */
   else {
   b = (boolean) 	      /* Set matches Packed Set matches SetConstant */
      ( 
	( t1->class == SET or t1->class == PACKSET)
	and (streq(ident(t2->name), "$setcon") or t1->class == t2->class )
        and 
	( rtype(t1->type) == rtype(t2->type)
        or builtinmatch(rtype(t1->type), rtype(t2->type)))
      );  
   if ((b&&t1->type->class==RANGE) 
      and (t2->type->symvalue.rangev.upper >= t2->type->symvalue.rangev.lower))
	/* check if it's an empty set */
   {
     /* check if elements are in range */
     if ((t1->type->symvalue.rangev.lower > t2->type->symvalue.rangev.lower)
        or (t1->type->symvalue.rangev.upper < t2->type->symvalue.rangev.upper))
     {
       b = false;
       error( catgets(scmc_catd, MS_pascal, MSG_415,
						   "set member out of range"));
     }
   } 
   return b;
   }
}



/************************************************************************* 
**	Test if STRING, array and packed array of CHAR match sizes      **
*************************************************************************/
static boolean sizematch(Symbol t1, Symbol t2)
{
  int expsize, varsize;
  Symbol s;

  expsize = size(t2);
  if (istypename(t2->type, "$char"))	/* adjust for null end of 'C' string */
    expsize--;
  varsize = size(t1);
  s = rtype(t1);      
  if (s->class == STRING) { 
    if (s->symvalue.field.length)	/* account for unpadded size */
      varsize = s->symvalue.field.length;
    varsize = varsize-2;       		/* adjust for len bytes */
  }
  if (varsize>=expsize)
    return true;
  else error( catgets(scmc_catd, MS_check, MSG_0, "incompatible sizes"));
}


boolean pascal_typematch (Symbol type1, Symbol type2)
{
    boolean b;
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
	b = (Boolean) 
            (
	      setmatch(t1, t2)
	      or rangematch(t1, t2)
	      or nilMatch(t1, t2) 
	      or enumMatch(t1, t2)
	      or stringArrayMatch(t1, t2) and sizematch(type1, type2)
              or funcMatch(t1, t2)
	      or builtinmatch(t1, t2)
	    );
    }
    return b;
}


/*
 * Indent n spaces.
 */
static void indent (int n)
{
    if (n > 0) {
	PRINTF "%*c", n, ' ');
    }
}


void pascal_printdecl (Symbol s)
{
    Boolean semicolon;
    char *buf;

    semicolon = true;
    if (s->class == TYPEREF) {
	resolveRef(s);
    }
    switch (s->class) {
	case CONST:
            if (isparam(s)) {
                (*rpt_output)(stdout, "(const parameter) %s : ", symname(s));
                pas_printtype(s, s->type, 0);
            } else if (s->type->class == SCAL) {
		semicolon = false;
		(*rpt_output)(stdout, "enum constant, ord ");
		PRINTF "%ld", s->symvalue.constval->value.lcon);
	    } else {
		(*rpt_output)(stdout, "const %s = ", symname(s));
		eval(s->symvalue.constval);
		pascal_printval(s);
	    }
	    break;

	case TYPE:
	    (*rpt_output)(stdout, "type %s = ", symname(s));
	    pas_printtype(s, s->type, 0);
	    break;

	case TYPEREF:
	    (*rpt_output)(stdout, "type %s", symname(s));
	    break;

	case VAR:
	case TOCVAR:
	    if (isparam(s)) {
		(*rpt_output)(stdout, "(parameter) %s : ", symname(s));
	    } else {
		   (*rpt_output)(stdout, "var %s : ", symname(s));
	    }
	    pas_printtype(s, s->type, 0);
	    break;

	case REF:
	    (*rpt_output)(stdout, "(var parameter) %s : ", symname(s));
	    pas_printtype(s, s->type, 0);
	    break;

	case RANGE:
	case PACKRANGE:
	case SPACE:
	case ARRAY:
	case PACKARRAY:
	case PACKRECORD:
	case RECORD:
	case UNION:
	case VARNT:
	case PTR:
	case FILET:
	case STRING:
	case GSTRING:
	case STRINGPTR:
	    pas_printtype(s, s, 0);
	    semicolon = false;
	    break;

	case FVAR:
	    (*rpt_output)(stdout, "(function variable) %s : ", symname(s));
	    pas_printtype(s, s->type, 0);
	    break;

	case FIELD:
	    (*rpt_output)(stdout, "(field) %s : ", symname(s));
	    pas_printtype(s, s->type, 0);
	    break;

	case PROC:
	case CSECTFUNC:
	    (*rpt_output)(stdout, "procedure %s", symname(s));
	    listparams(s);
	    break;

	case PROG:
	    (*rpt_output)(stdout, "program %s", symname(s));
	    listparams(s);
	    break;

	case FUNC:
	    (*rpt_output)(stdout, "function %s", symname(s));
	    listparams(s);
	    (*rpt_output)(stdout, " : ");
	    pas_printtype(s, s->type, 0);
	    break;

	case MODULE:
	    (*rpt_output)(stdout, "module %s", symname(s));
	    break;

	  /*
	   * the parameter list of the following should be printed
	   * eventually
	   */
	case PROCPARAM: 
	case FPROC:
	    (*rpt_output)(stdout, "procedure %s()", symname(s));
	    break;
	
	case FUNCPARAM:
	case FFUNC:
	    (*rpt_output)(stdout, "function %s()", symname(s));
	    break;

	case TAG:
	    pas_printtype(s, s->type, 0);
	    break;

	case LABEL:
	    /* take off '$$' in front of number labels */
	    buf = (char *) malloc(strlen(symname(s))+1);
	    strcpy(buf, symname(s));
	    if (*buf == '$' and isdigit(*(buf+2)) )
	      buf = buf+2;
	    (*rpt_output)(stdout, "label %s", buf);
	    break;

        case VLABEL:
            (*rpt_output)(stdout, "%s : variant record label", symname(s));
            break;

        case VTAG:
            (*rpt_output)(stdout, "%s : variant record tag of type ",
								   symname(s));
            pas_printtype(s, s->type, 0);
            break;

	default:
	    (*rpt_output)(stdout, "%s : (class %s)", symname(s), classname(s));
	    break;
    }
    if (semicolon) {
	(*rpt_output)( stdout, ";");
    }
    (*rpt_output)( stdout, "\n");
}

/*
 * NAME: translate_type_name
 *
 * FUNCTION: Perform appropriate translation of a type name and
 *           print it out.
 *
 * PARAMETERS:
 *      t         - input Symbol
 *      type_name - input type name
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: nothing 
 */


static void translate_type_name(Symbol t, String type_name)
{
  Boolean is_builtin = (t->type->type == t) ? true : false;

  if (!is_builtin)
    (*rpt_output)(stdout, "%s", type_name);
  else if (streq(type_name, "int"))
    (*rpt_output)(stdout, "integer");
  else if (streq(type_name, "double"))
    (*rpt_output)(stdout, "real");
  else if (streq(type_name, "wchar"))
    (*rpt_output)(stdout, "gchar");
  else
    (*rpt_output)(stdout, "%s", type_name);
}

static void ArrayTypeName(Symbol t, int n)
{
    String word;

    word = symname(t);
    if (streq(word, "(noname)") or streq(word, "") )
    {
       if ((t->class == STRING) and (t->symvalue.field.length))
         (*rpt_output)(stdout, "string(%d)", t->symvalue.field.length-2);
       else
       pas_printtype(t, t, n);
    }
    else {
       if (t->class == PTR)
         (*rpt_output)(stdout, "@");
       translate_type_name(t, word);
    }
}



/*
 * Recursive whiz-bang procedure to print the type portion
 * of a declaration.
 *
 * The symbol associated with the type is passed to allow
 * searching for type names without getting "type blah = blah".
 */

static void pas_printtype (Symbol s, Symbol t, int n)
{
    register Symbol tmp;
    String   word;
    int curClass = 0;

    if (t->class == TYPEREF) {
	resolveRef(t);
    }
    switch (t->class) {
	case VAR:
	case CONST:
	case FUNC:
	case PROC:
	case TOCVAR:
	    panic("printtype: class %s", classname(t));
	    break;
	case SPACE:
	    (*rpt_output)(stdout, "space[%d] of ",t->symvalue.size);
	    ArrayTypeName(t->type, n);
	    break;
 	case PACKARRAY:
            (*rpt_output)(stdout, "packed ");		
            curClass = PACKARRAY; 
	case ARRAY:
	    (*rpt_output)(stdout, "array[");
	    if ( curClass != PACKARRAY) curClass = ARRAY;
            while (t->class == curClass)
            {
               tmp = t->chain;
               assert(tmp->class == RANGE);
	       printRangeDecl(tmp);
               if (t->type->class == curClass) 
                  (*rpt_output)(stdout, ", ");
               t = t->type;
            }
	    (*rpt_output)(stdout, "] of ");
	    ArrayTypeName(t, n);
	    break;

        case VARNT:
            printVarDecl(t, n);
            break;

	case PACKRECORD:
	case RECORD:
	case UNION:
	    printRecordDecl(t, n);
	    break;

	case FIELD:
	    if (t->chain != nil) {
		pas_printtype(t->chain, t->chain, n);
	    }
	    (*rpt_output)(stdout, "\t%s : ", symname(t));
	    pas_printtype(t, t->type, n);
	    (*rpt_output)(stdout, ";\n");
	    break;

	case PACKRANGE:
	case RANGE:
	    printRangeDecl(t);
	    break;

	case PTR:
	    (*rpt_output)(stdout, "@");
	    pas_printtype(t, t->type, n);
	    break;

	case REAL:
	    switch(t->symvalue.size) {
		case sizeof(float):
			 (*rpt_output)(stdout, "shortreal ");
			 break;
		case sizeof(double):
			 (*rpt_output)(stdout, "real ");
			 break;
		case 2*sizeof(double):
			 (*rpt_output)(stdout, "long real ");
			 break;
		default: (*rpt_output)(stdout, "%d byte real ",
							     t->symvalue.size);
			 break;
	    }
	    break;

	case TAG:
	case TYPE:
	    if (t->name != nil and ident(t->name)[0] != '\0')
            {
               if (t == nil)
                  (*rpt_output)(stdout, "(noname)");
               else if (t == program)  
                  (*rpt_output)(stdout, ".");
               else if (isredirected() or isambiguous(t, WANY)) 
               {
                  word = symname(t);
                  translate_type_name(t, word);
               } 
               else
               {
                  word = symname(t);
                  translate_type_name(t, word);
               }
            }
	    else 
            {
	       pas_printtype(t, t->type, n);
	    }
	    break;

	case SCAL:
	    (*rpt_output)(stdout, "enum ");
	    if (s->class == TAG && !nilname(s)) {
		(*rpt_output)(stdout, "%s ", symname(s));
	    }
	    printEnumDecl(t, n);
	    break;

	case PACKSET:
	    (*rpt_output)(stdout, "packed ");
	case SET:
	    (*rpt_output)(stdout, "set of ");
	    pas_printtype(t, t->type, n);
	    break;

	case FILET:
	    (*rpt_output)(stdout, "file of ");
	    pas_printtype(t, t->type, n);
	    break;

	case TYPEREF:
	    break;
	
	case FPROC:
	    (*rpt_output)(stdout, "procedure");
	    break;
	    
	case FFUNC:
	    (*rpt_output)(stdout, "function");
	    break;

	case PROCPARAM:
	    listparams(t);
	    break;

	case FUNCPARAM:
	    listparams(t);
	    (*rpt_output)(stdout, ": ");
	    pas_printtype(t, t->type, 0);
	    break;
	
	case GSTRING:
	    (*rpt_output)(stdout, "gstring(%d)",(t->symvalue.size-2)/2);
	    break;				    /* minus the two length bytes */
						    /* gchar is two bytes long */
	case STRING:
	    (*rpt_output)(stdout, "string(%d)",t->symvalue.size-2);
						     /* minus the two length */
	    break;				     /* bytes in PascalString*/
	default:
	    (*rpt_output)(stdout, "(class %d)", t->class);
	    break;
    }
}




static Symbol printVfield (Symbol t, int n)
{
   register Symbol f;

   for (f = t->chain; f != nil; f = f->chain)
   {
  /* first print the fixed fields and their types */
     if (f->class == FIELD)
     {
       indent(n+7);
       (*rpt_output)(stdout, "%s: ", symname(f));
       pas_printtype(f->type, f->type, n+7);
       (*rpt_output)(stdout, ";\n");
     }
  /* Then if the next one is a variant tag, print it with the "case .." */
  /* The tag field can be in three forms:                               */
  /* First, with no name field, a refer back to the fix portion,        */
  /* Second, with both name and type fields,                            */
  /* Third, with just the type field.                                   */
      if (f->class == VTAG)
      {
        if (n == 0) indent(n+6);
        if (streq(symname(f), ""))
          (*rpt_output)(stdout, " case ");
        else
          (*rpt_output)(stdout, " case %s: ", symname(f));
        if (f->type != nil)
          pas_printtype(f->type, f->type, n+4);
        (*rpt_output)(stdout, " of\n");
      }
  /* The labels for the variant fields could be of range or constant type */
      if (f->class == VLABEL)
      {
        indent(n+10);
        while (f->chain->class == VLABEL)
        {
          if (f->type->class == RANGE) {
          printRangeDecl(f->type);
          (*rpt_output)(stdout, ",");
          }
          else {
          eval(f->type->symvalue.constval);
          pascal_printval(f->type);
          (*rpt_output)(stdout, ",");
          }
          f = f->chain;
        }
          if (f->type->class == RANGE)
            printRangeDecl(f->type);
          else {
            eval(f->type->symvalue.constval);
            pascal_printval(f->type);
          }
          (*rpt_output)(stdout, ":(");
          if (f->chain->class == TAG)   /* remove empty variant label tag */
            f = f->chain;
          /* Print the variant fields */
          while (f->chain->class == FIELD)
          {
            f = f->chain;
            (*rpt_output)(stdout, " %s: ", symname(f));
            pas_printtype(f->type, f->type, n+10);
            (*rpt_output)(stdout, ";");
          }
          if (f->chain->class == VTAG)
          {
            (*rpt_output)(stdout, "\n");
            indent(n+15);
            f = printVfield(f, n+8);
            if (f->chain->class == TAG)   /* remove variant level marker */
              f = f->chain;
          }
          (*rpt_output)(stdout, ");");
          if (f->chain->class == TAG)
            return f;
          (*rpt_output)(stdout, "\n");
        }
    }
    return f;
}

/*
 * Print out a Variant record declaration.
 */

static void printVarDecl (Symbol t, int n)
{
   register Symbol f;

    if (t->chain == nil)
      (*rpt_output)(stdout, "record end");
    else
    {
      (*rpt_output)(stdout, "record\n");
      f = t;
      printVfield( f, n);
      (*rpt_output)(stdout, "\n");
      indent(n+7);
      (*rpt_output)(stdout, "end");
    }
}



/*
 * Print out a record declaration.
 */

static void printRecordDecl (Symbol t, int n)
{
    register Symbol f;

    if (t->chain == nil) {
	(*rpt_output)(stdout, "record end");
    } else {
	if (t->class == PACKRECORD) 
		(*rpt_output)(stdout, "packed ");
	(*rpt_output)(stdout, "record\n");
	for (f = t->chain; f != nil; f = f->chain) {
	    indent(n+7);
	    (*rpt_output)(stdout, "%s : ", symname(f));
	    pas_printtype(f->type, f->type, n+4);
	    (*rpt_output)(stdout, ";\n");
	}
	indent(n+4);
	(*rpt_output)(stdout, "end");
    }
}

/*
 * Print out the declaration of a range type.
 */

static void printRangeDecl (Symbol t)
{
    long r0, r1;
    Symbol s;

    r0 = t->symvalue.rangev.lower;
    r1 = t->symvalue.rangev.upper;

    if (t->class == PACKRANGE) (*rpt_output)(stdout, "packed ");
    for (s = t->type; s->class == TYPE; s = s->type);
    if ((s->class == RANGE)||(s->class == PACKRANGE)) {
      if ( istypename(t->type, "$char") or istypename(t->type, "char"))
      {
	if (r0 < 0x20 or r0 > 0x7e) {
	    (*rpt_output)(stdout, "'%ld'xc..", r0);
	} else {
	    (*rpt_output)(stdout, "'%c'..", (char) r0);
	}
	if (r1 < 0x20 or r1 > 0x7e) {
            (*rpt_output)(stdout, "'%ld'xc", r1);
	} else {
	    (*rpt_output)(stdout, "'%c'", (char) r1);
	}
      }
      else (*rpt_output)(stdout, "%ld..%ld", r0, r1);
    } else 
    if (s->class == SCAL) {
	printEnum(r0,s);
	(*rpt_output)(stdout, "..");
	printEnum(r1,s);
    } else if (r0 > 0 and r1 == 0) {
	(*rpt_output)(stdout, "%ld byte real", r0);
    } else if (r0 >= 0) {
	(*rpt_output)(stdout, "%lu..%lu", r0, r1);
    } else {
	(*rpt_output)(stdout, "%ld..%ld", r0, r1);
    }
}

/*
 * Print out an enumeration declaration.
 */

static void printEnumDecl (Symbol e, int n)
{
    Symbol t;

    (*rpt_output)(stdout, "(");
    t = e->chain;
    if (t != nil) {
	(*rpt_output)(stdout, "%s", symname(t));
	t = t->chain;
	while (t != nil) {
	    (*rpt_output)(stdout, ", %s", symname(t));
	    t = t->chain;
	}
    }
    (*rpt_output)(stdout, ")");
}

/*
 * List the parameters of a procedure or function.
 * No attempt is made to combine like types.
 */

static void listparams(Symbol s)
{
    Symbol t;

    if (s->chain != nil) {
	(*rpt_output)( stdout, "(");
	for (t = s->chain; t != nil; t = t->chain) {
	    switch (t->class) {
		case REF:
		 if (t->type->class==PROCPARAM)
		   (*rpt_output)(stdout, "procedure %s", symname(t));
                 else if (t->type->class==FUNCPARAM)
			(*rpt_output)(stdout, "function %s", symname(t));
		 else (*rpt_output)(stdout, "var %s : ", symname(t));
		 break;

		case VAR:
		case TOCVAR:
		 (*rpt_output)(stdout, "%s : ", symname(t));
		 break;
		
		case CONST:
		 (*rpt_output)(stdout, "const %s : ", symname(t));
		 break;

		default:
		    panic( catgets(scmc_catd, MS_pascal, MSG_338,
			       "unexpected class %d for parameter"), t->class);
	    }
	    pas_printtype(t, t->type, 0);
	    if (t->chain != nil) {
		(*rpt_output)(stdout, "; ");
	    }
	}
	(*rpt_output)( stdout, ")");
    }
}


/*
 *  Pops pascal pointer out from stack.
 *  Pascal pointer is eight bytes in length. The first four byte is a
 *  regular pointer, and the second four bytes is the size of whatever
 *  it is pointing at.
 */

Address pop_pas_ptr()
{
  long len_info;

  len_info = pop(long);         /* first pops out the size info */
  return pop(Address);          /* (not used), then return the  */
}                               /* address it's pointing at.    */



/*
 * Print out the value on the top of the expression stack
 * in the format for the type of the given symbol.
 */

void pascal_printval (Symbol s)
{
      pas_prval(s, size(s));
}

static void pas_prval (Symbol s, int n)
{
    Symbol t;
    Address a;
    integer len;
    integer i;

    if (s->class == TYPEREF) {
	resolveRef(s);
    }
    switch (s->class) {
	case TYPE:
	    if (istypename(s, "stringptr") and s->type->class == STRINGPTR)
	    {
 	      a = pop_pas_ptr();
	      if (a == 0)
              	PRINTF "nil");
	      else PRINTF "0x%x", a);
	      break;
 	    }
	case CONST:
	case REF:
	case VAR:
	case FVAR:
	case TAG:
	case TOCVAR:
	    pas_prval(s->type, n);
	    break;

	case VTAG:
	case FIELD:
	    pas_prval(s->type, n);
	    break;

	case LABEL:
 	    a = pop(Address);
	    if (a == 0)
            	PRINTF "nil");
	    else PRINTF "0x%x", a);
	    break;

	case SPACE:
            error( catgets(scmc_catd, MS_pascal, MSG_418,
	               "must specify index when printing space type"));
	    break;
		
	case PACKARRAY:
	case ARRAY:
            if (subarray) {
              printsubarray(0, subdim_tail, array_sym->type, s->class);
              reset_subarray_vars();
            } else {
	      t = rtype(s->type);
	      if (t == t_char->type or
	  	  (t->class == RANGE and istypename(t->type, "char"))
	      ) {
	  	  len = size(s);
		  sp -= len;
		  PRINTF "'%.*s'", len, sp);
		  break;
	      } else {
		  printarray(s);
	      }
	    }
	    break;

	case PACKRECORD:
	case RECORD:
	    if (subarray)
	    {
	      printsubarray(0, subdim_tail, array_sym->type, s->class);
	      reset_subarray_vars();
	    } else
	      printrecord(s);
	    break;

	case VARNT:
	    if (subarray)
	    {
	      printsubarray(0, subdim_tail, array_sym->type, VARNT);
	      reset_subarray_vars();
	    }
            else
	      printVrecord(s);
	    break;

	case PACKRANGE:
	case RANGE:
	    if (subarray)
	    {
	      printsubarray(0, subdim_tail, array_sym->type, s->class);
	      reset_subarray_vars();
	    } 
	    else printrange(s);
	    break;

	case REAL:
	    if (subarray)
	    {
	      printsubarray(0, subdim_tail, array_sym->type, REAL);
	      reset_subarray_vars();
	    }
	    else printreal(s);
	    break;

	case FILET:
	    a = pop(Address);
	    if (a == 0) {
		PRINTF "nil");
	    } else {
		PRINTF "0x%x", a);
	    }
	    break;

	case STRINGPTR:
	    doStringPtr();
	    break;

	case PTR:
	    if (subarray)
	    {
	      printsubarray(0, subdim_tail, array_sym->type, PTR);
	      reset_subarray_vars();
	    }
            else
            {
	      a = pop_pas_ptr();
	      if (a == 0) {
		  PRINTF "nil");
	      } else {
		  PRINTF "0x%x", a);
	      }
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
	      i = popsmall(s);
	      printEnum(i, s);
	    }
	    break;

	case PROCPARAM:
	case FUNCPARAM:
            a = pop(long);              /* There's one more level of    */
            rpush(a, sizeof(Address));  /* indirection for these params */
	case FPROC:
	case FFUNC:
	    a = pop(long);
	    t = whatblock(a);
	    if (t == nil) {
		PRINTF "(proc 0x%x)", a);
	    } else {
		PRINTF "%s", symname(t));
	    }
	    break;

	case PACKSET:
	case SET:
	    if (subarray)
	    {
 	      printsubarray(0, subdim_tail, array_sym->type, s->class);
	      reset_subarray_vars();
	    }
	    else printSet(s);
	    break;

	case GSTRING:
	case STRING:
	    if (subarray)
	    {
	      printsubarray(0, subdim_tail, array_sym->type, s->class);
	      reset_subarray_vars();
	    }
	    else printPstring(s, n);
	    break;

	default:
	    if (ord(s->class) > ord(LASTCLASS)) {
		panic("printval: bad class %d", ord(s->class));
	    }
	    PRINTF "[%s]", classname(s));
	    break;
    }
}

/* 
 * Test if a variant field of a variant record is active 
 * by testing the value of the variant label against the 
 * variant tag.
 */
static boolean isChosen(Symbol f, int tagvalue)
{
  if (f->type->class == RANGE)
    return ( (tagvalue >= (int) f->type->symvalue.rangev.lower) and
             (tagvalue <= (int) f->type->symvalue.rangev.upper) );
  else if (f->type->class == CONST)
         return (tagvalue == (int) f->type->symvalue.constval->value.lcon);
}
	

/*
 * Return the value of the variant tag of a varaint record.
 * If the type of tag is nil, refer back to fix portion 
 * of variant record to get the correct value.
 */
static int gettag(Symbol f, Symbol s)
{
  Stack *savesp;
  register int off, len, ss, tag;
  char *buffer;

  if (f->type == nil) 
  {
    while ((s != nil) and (!streq(symname(f), symname(s))) )
      s = s->chain;
    f = s;
  }
  savesp = sp;
  off = f->symvalue.field.offset;
  len = f->symvalue.field.length;
  sp += ((off + len + BITSPERBYTE -1) / BITSPERBYTE);
  ss = ((len + BITSPERBYTE -1) / BITSPERBYTE);

  if (ss == sizeof(integer))
    tag = pop(integer); 
  else 
  {
    buffer = (char *) malloc(ss);
    popn( ss, buffer);
    tag = (int) *buffer;
  }
  sp = savesp;
  return tag;
}

static Symbol printVVrecord (Symbol s, boolean first)
{
    register Symbol f;
    int tagvalue = 0;
    boolean notag = false;

    f = s;
    if (f != nil)
    {
      for (;;)
      {
        /* Process the tag field, get tag value. */
        if (f->class == VTAG)
        {
          if (streq(symname(f),"")) notag = true;
            else tagvalue = gettag(f, s);
        }
        /* Test if the variant portion is active.                */
        /* If no tag, print all variant fields.                  */
        while (f->class == VLABEL)
        {
          /* Loop to next variant portion if current is not active */
          if (!isChosen(f, tagvalue) and !notag and !expandunions)
          {
            f = f->chain;
            while (f->class != VLABEL)
            {
              /* VTAG here means we are entering variant portion */
              /* within a variant portion, so we have to jump to */
              /* the end of this variant part (TAG)              */
	      if (f->class == VTAG) {
		int varlevel = 1;
	        while ((varlevel > 0) and f != nil) {
	 	  f = f->chain;
		  if (f->class == VTAG)
		    varlevel++;
		  if (f->class == TAG)
	            varlevel--;
	        }
	      }

              f = f->chain;
              if (f == nil or f->class == TAG) break;
            }
          }
          else          /* else point to next field to print */
          {
            f = f->chain;
            while (f->class != FIELD and f->class != TAG)
            {
              f = f->chain;
              if (f == nil) break;
            }
          }
        }
        if ((f->class == VTAG and f->type != nil and !notag)
            or f->class == FIELD)
        {
          if (!first)
            PRINTF ", ");
          pas_printfield(f);
          first = false;
        }
        if (f != nil and f->class != VLABEL and f->class != TAG)
          f = f->chain;
        if (f == nil) break;
        /* return when reach the end of variant portion of a variant */
        if (f->class == TAG)
          return f->chain;
        /* use recursion to handle variant part within variant part */
        if (f->class == VTAG)
          f = printVVrecord(f, first);
      }
    }
    return f;
}


/*
 * Print out the value of a Variant record, field by field.
 */
void printVrecord (Symbol s)
{
    if (s->chain == nil) {
        error( catgets(scmc_catd, MS_printsym, MSG_320,
                                                      "record has no fields"));
    }
    PRINTF "(");
    sp -= size(s);
    printVVrecord(s->chain, true);
    PRINTF ")");
}

/*
 * Print out a field.
 */

static void pas_printfield (Symbol f)
{
    Stack *savesp;
    register int off, len;

    PRINTF "%s = ", symname(f));
    savesp = sp;
    off = f->symvalue.field.offset;
    len = f->symvalue.field.length;
    sp += ((off + len + BITSPERBYTE - 1) / BITSPERBYTE);
    printval(f, 0);
    sp = savesp;
}

/**************************************************
* Print string of what Pascal StringPtr points to *
**************************************************/

static void doStringPtr ()
{
  int i;
  unsigned short *size;
  char *buffer, *sizebuf;
  Address a;
  
  a = pop_pas_ptr();
  if (a == 0)
    error( catgets(scmc_catd, MS_eval, MSG_84,
			     "reference through nil pointer"));
  sizebuf = (char *) malloc(sizeof(unsigned short)); 
  dread(sizebuf, a, sizeof(unsigned short));	/* read len bytes */
  size = (unsigned short *) sizebuf;
  a += sizeof(unsigned short);
  buffer = (char *) malloc(*size);		/* read string */
  dread(buffer, a, *size); 
  PRINTF "'");
  for (i=0; i<*size; i++) {
    PRINTF "%c", buffer[i]);
  }
  PRINTF "'");  
}  



/************************************************
* Print out a Pascal String.	                *
************************************************/

void printPstring (Symbol s, int size)
{
    int i;
    char *buffer;
    unsigned short *truesize;

    buffer = (char *) malloc(size);
    popn( size, buffer);   
    truesize = (unsigned short *) buffer;
    if (*truesize > size)	/* Protection against */
      *truesize = 0;		/* uninitialized data */
    PRINTF "'");
    for (i=0; i<*truesize; i++) {
      PRINTF "%c", buffer[i+2]);
    }
    PRINTF "'");
}

/*
 * Print out a set.
 */

void printSet (Symbol s)
{
    Symbol t;
    integer nbytes;

  if (streq(symname(s), "$emptySet"))
      PRINTF "[]");
  else {
    nbytes = size(s);
    t = rtype(s->type);
    PRINTF "[");
    sp -= nbytes;
    if (t->class == SCAL) {
	printSetOfEnum(t);
    } else if (t->class == RANGE) {
               printSetOfRange(t, s);
    } else {
	error( catgets(scmc_catd, MS_pascal, MSG_342,
	    "internal error: expected range or enumerated base type for set"));
    }
    PRINTF "]");
  }
}

/*
 * Print out a set of an enumeration.
 */

static void printSetOfEnum (Symbol t)
{
    register Symbol e;
    register integer i, j, *p;
    register firstbit = 0x80000000;
    boolean first;

    p = (int *) sp;
    i = *p;
    j = 0;
    e = t->chain;
    first = true;
    while (e != nil) {
	if ((i&firstbit) == firstbit) {
	    if (first) {
		first = false;
		PRINTF "%s", symname(e));
	    } else {
		PRINTF ", %s", symname(e));
	    }
	}
	i <<= 1;
	++j;
	if (j >= sizeof(integer)*BITSPERBYTE) {
	    j = 0;
	    ++p;
	    i = *p;
	}
	e = e->chain;
    }
}

/*
 * Print out a set of a subrange type.
 */

static void printSetOfRange (Symbol t, Symbol s)
{
    register integer i, j, *p;
    register firstbit = 0x80000000;
    long v, bitcount;
    boolean first;

    p = (int *) sp;
    i = *p;
    j = 0;
    v = 0;
    first = true;

    if (s->class == PACKSET and s->symvalue.size != 0)
      bitcount = s->symvalue.size;
    else {
      /* Unpacked set of range have a length limit of 255 */
      /* Therefore, needs at most 255 bits == 32 bytes    */
      if (t->symvalue.rangev.upper > 255)
        bitcount = 255;
      else
        bitcount = t->symvalue.rangev.upper;
    }
    while (v <= bitcount) {
	if ((i&firstbit) == firstbit) {
	    if (first) {
		first = false;
	        printRangeVal(v, t);
	    } else {
		PRINTF ", ");
		printRangeVal(v, t);
	    }
	}
	i <<= 1;
	++j;
	if (j >= sizeof(integer)*BITSPERBYTE) {
	    j = 0;
	    ++p;
	    i = *p;
	}
	++v;
    }
}

/*
 * Evaluate a subscript index.
 */

void pascal_evalaref (Symbol s, Address base, long i)
{
    Symbol t;
    long lb, ub;
    int distance;
    Boolean isString = (s->class == STRING);
    
    t = rtype(s);
    if (t->class == SPACE)
    {
      lb = 0; 
      ub = t->symvalue.size - size(t->type) + 1;
      /* Say a space[20] of integer, should allow access from index 0 to */
      /* 17, since the index 17 refers to the integer from byte 17 to 20    */
    }
    else
    {
      s = rtype(t->chain);
      findbounds(s, &lb, &ub);
    }
    if ((i < lb or i > ub) && (!varIsSet("$unsafebounds"))) {
	if (isString) {
	  error( catgets(scmc_catd, MS_pascal, MSG_343,
			     "subscript %d out of range [%d..%d]"), i, 1, ub);
	} else error( catgets(scmc_catd, MS_pascal, MSG_343,
			     "subscript %d out of range [%d..%d]"), i, lb, ub);
    }
    if (t->class == SPACE)
      push(long, base + i )
    else
    { 
      distance = size(t->type);
      push(long, base + (i - lb) * distance);
    }
}

/*
 * Initial Pascal type information is now performed in pascal_init().
 */

/*
 * Initialize typetable.
 */

void pascal_modinit ()
{
    addlangdefines(usetype);
    initialized = true;
}

boolean pascal_hasmodules ()
{
    return false;
}

boolean pascal_passaddr (Symbol param, Symbol exprtype)
{
    return false;
}

cases pascal_foldnames ()
{
    return lower;
}
