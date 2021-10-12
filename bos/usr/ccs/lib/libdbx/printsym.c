static char sccsid[] = "@(#)72	1.53.3.23  src/bos/usr/ccs/lib/libdbx/printsym.c, libdbx, bos411, 9433A411a 8/8/94 18:12:31";
/*
 *   COMPONENT_NAME: CMDDBX
 *
 *   FUNCTIONS: calc_offsets
 *		classname
 *		nilname
 *		prangetype
 *		printEnum
 *		printRangeVal
 *		printString
 *		printarray
 *		printcall
 *		printchar
 *		printdecl
 *		printentry
 *		printexit
 *		printfield
 *		printlonglong
 *		printname
 *		printouter
 *		printparams
 *		printparamv
 *		printquad
 *		printrange
 *		printreal
 *		printrecord
 *		printrtn
 *		printsubarray
 *		printv
 *		printval
 *		printwhereis
 *		printwhich
 *		prtreal
 *		psym
 *		reset_subarray_vars
 *		should_print
 *		symboltype
 *
 *   ORIGINS: 26,27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Copyright (c) 1982 Regents of the University of California
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
*/


#ifdef KDBXRT
#include "rtnls.h"		/* MUST BE FIRST */
#endif
/*              include file for message texts          */
#include "dbx_msg.h" 
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

/*
 * Printing of symbolic information.
 */

#include "defs.h"
#include "envdefs.h"
#include "symbols.h"
#include "languages.h"
#include "printsym.h"
#include "tree.h"
#include "eval.h"
#include "mappings.h"
#include "process.h"
#include "runtime.h"
#include "machine.h"
#include "names.h"
#include "keywords.h"
#include "main.h"
#include "object.h"
#include "cplusplus.h"
#include <ctype.h>
#include "cma_thread.h"

#include <sys/types.h>
#include <limits.h>

boolean hexints = false;
boolean octints = false;
boolean expandunions = false;
/*
 *  Variables for subarrays
 */
struct subdim {
	long ub;
	long lb;
	struct subdim *next, *back;
};
Symbol Subar_save_sym;
Address Subar_offset;
extern struct subdim *subdim_head;
extern struct subdim *subdim_tail;
extern boolean subarray;
extern boolean subarray_seen;
extern boolean no_more_subarray;
extern boolean ptr_to_subarray;
extern Address array_addr;
extern Symbol  array_sym;
extern SymbolType symbol_type;
extern int 	eventId;

private prangetype();
private printfield();

/*
 * Maximum number of arguments to a function.
 * This is used as a check for the possibility that the stack has been
 * overwritten and therefore a saved argument pointer might indicate
 * to an absurdly large number of arguments.
 */

#define MAXARGSPASSED 20

/*
 * Return a pointer to the string for the name of the class that
 * the given symbol belongs to.
 */

public String clname[] = {
    "bad use", "constant", "type", "variable", "array", "array",
    "dynarray", "subarray", "packed array", "fileptr", "record", "field",
    "ref field", "stringptr", "string", "complex", "real", "wide character",
    "procedure", "function", "csect/function", "funcvar",
    "ref", "pointer", "file", "set", "range", "label", "withptr",
    "scalar", "string", "program", "improper", "variant",
    "procparam", "funcparam", "module", "tag", "common", "extref", "typeref",
    "character*", "fortran string", "picture", "redefined pic",
    "index", "'usage is' index", "group", "redefined group", "conditional",
    "packed record", "packed set", "packed subrange", "procparam", "funcparam",
    "union", "variant tag", "variant label", "space", "gstring", "TOC variable",
    "fortran pointer", "fortran pointee", "member", "class", "base class", 
    "nested class", "reference", "pointer to member", "ellipses", 
    "friend function", "friend class", "c++ symbol list",
    "generic function/subroutine"
};

public String classname (s)
Symbol s;
{
    extern Language cLang;
    extern Language cppLang;

    if (s->class == VAR && s->param) {
	return "parameter";
    } else if (s->class == TAG && s->type &&
	       (s->language == cppLang || s->language == cLang)) {
	return c_classname(s->type);
    } else {
	return clname[ord(s->class)];
    }
}

/*
 * Note the entry of the given block, unless it's the main program.
 */

public printentry (s)
Symbol s;
{
    if (s != program) {
	(*rpt_output)(stdout,  catgets(scmc_catd, MS_printsym, MSG_261,
					      "\nentering %s "), classname(s));
	printname( rpt_output, stdout, s, true);
	(*rpt_output)(stdout, "\n");
    }
}

/*
 * Note the exit of the given block
 */

public printexit (s)
Symbol s;
{
    if (s != program) {
	(*rpt_output)(stdout,  catgets(scmc_catd, MS_printsym, MSG_264,
						 "leaving %s "), classname(s));
	printname( rpt_output, stdout, s, true);
	(*rpt_output)(stdout, "\n\n");
    }
}

/*
 * Note the call of s from t.
 */

public printcall (s, t)
Symbol s, t;
{
    struct TraceStruct trdata;

    /*
     * Start of trace output.  Indicate by setting
     * eventnum to nil.
     */
    trdata.reporting_trace_output = 1;
    (*rpt_trace)( &trdata );

    (*rpt_output)(stdout, "calling ");
    printname( rpt_output, stdout, s, false);
    printparams(s, nil);
    (*rpt_output)(stdout, " from %s ", classname(t));
    printname( rpt_output, stdout, t, false);
#ifdef K_THREADS
    /* indicate what thread is current thread if needed... */
    if (current_thread)
        (*rpt_output)(stdout, " (%s)", symname(current_thread));
#endif /* K_THREADS */
    (*rpt_output)(stdout, "\n");

    dpi_current_location( &trdata );
    trdata.eventnum = eventId;
    trdata.token = nil;
    trdata.value = nil;
    trdata.reporting_trace_output = 0;
    (*rpt_trace)( &trdata );
}

/*
 * Note the return from s.  If s is a function, print the value
 * it is returning.  This is somewhat painful, since the function
 * has actually just returned.
 */

public printrtn (s)
Symbol s;
{
    Symbol t;
    register int len;
    Boolean isindirect;
    struct TraceStruct trdata;

    /*
     * Start of trace output.  Indicate by setting
     * eventnum to nil.
     */
    trdata.reporting_trace_output = 1;
    (*rpt_trace)( &trdata );

    (*rpt_output)(stdout, "returning ");
    if (s->class == FUNC && (!istypename(s->type,"void"))) {
	len = size(s->type);
	if (canpush(len)) {
	    t = rtype(s->type);
	    isindirect = (Boolean) (t->class == RECORD or t->class == UNION or
				    t->class == CLASS);
	    pushretval(len, isindirect, t);
	    printval(s->type, 0);
	    (*rpt_output)(stdout, " " );
	} else {
	    (*rpt_output)(stdout, "(value too large) ");
	}
    }
    (*rpt_output)(stdout, "from ");
    printname( rpt_output, stdout, s, true);
    (*rpt_output)(stdout, "\n");

    dpi_current_location( &trdata );
    trdata.eventnum = eventId;
    trdata.token = nil;
    trdata.value = nil;
    trdata.reporting_trace_output = 0;
    (*rpt_trace)( &trdata );
}

/*
 * Print the values of the parameters of the given procedure or function.
 * The frame distinguishes recursive instances of a procedure.
 *
 * If the procedure or function is internal, the argument count is
 * not valid so we ignore it.
 */

public printparams (f, frame)
Symbol f;
Frame frame;
{
    Symbol param;
    int n, m, s;

    n = nargspassed(frame);
    if (isinternal(f)) {
	n = 0;
    } else if ((n > 0) && multiword(f)) {
	--n;
    }
    (*rpt_output)(stdout, "(");
    param = f->chain;
    if (param != nil or n > 0) {
	m = n;
	if (param != nil) {
	    for (;;) {
                if (rtype(param)->class == ELLIPSES) {
                   (*rpt_output)(stdout,"... = ");
                   break;
                }
		s = psize(param) / sizeof(Word);
		if (s == 0) {
		    s = 1;
		}
		m -= s;
		if (showaggrs) {
		    printv(param, frame, true);
		} else {
		    printparamv(param, frame);
		}
		param = param->chain;
	    if (param == nil) break;
		(*rpt_output)(stdout, ", ");
	    }
	}
	if (m > 0) {
	    if (m > MAXARGSPASSED) {
		m = MAXARGSPASSED;
	    }
            if ((f->chain != nil) && (rtype(param)->class != ELLIPSES)) {
		(*rpt_output)(stdout, ", ");
	    } 
	    print_passed_args(frame,n-m+1,n);
	}
    }
    (*rpt_output)(stdout, ")");
}

/*
 * Test if a symbol should be printed.  We don't print files,
 * for example, simply because there's no good way to do it.
 * The symbol must be within the given function.
 */

public Boolean should_print (s)
register Symbol s;
{
    register Symbol t;
    char *name;

    if (!s->param &&
       (s->class == VAR || s->class == TOCVAR || s->class == FVAR)) {
        name = symname(s);

        /* Check for C++ compiler gend variables. */
        if (name != nil && s->language == cppLang && cpp_tempname(s->name))
            return false;

	if (name[0] != '.') {
	    t = rtype(s->type);
	    if (t != nil && t->class != FILET && t->class != BADUSE) {
		return true;
	    }
	}
    }
    return false;
}

/*
 * Print out a parameter value.
 *
 * Since this is intended to be printed on a single line with other information
 * aggregate values are not printed.
 */

public printparamv (p, frame)
Symbol p;
Frame frame;
{
    Symbol t;

    t = rtype(p->type);
    switch (t->class) {
	case ARRAY:
	case SUBARRAY:
	case FFUNC:
	    t = rtype(t->type);
	    if ((t->class != CHARSPLAT) && compatible(t, t_char) ) {
		printv(p, frame, false);
	    } else {
		(*rpt_output)(stdout, "%s = (...)", symname(p));
	    }
	    break;

	case RECORD:
	case CLASS:
	    (*rpt_output)(stdout, "%s = (...)", symname(p));
	    break;

	case UNION:
	case VARNT:
	    (*rpt_output)(stdout, "%s = [%s]", symname(p), clname[t->class]);
	    break;

	default:
	    printv(p, frame, false);
	    break;
    }
}

/*
 * Print the name and value of a variable.
 */

public printv (s, frame, dump)
Symbol s;
Frame frame;
boolean dump;
{
    Address addr;
    int r, len;
    boolean inareg;
    int indent;

    /* indent is negative to start with to indicate that the first line of */
    /* what is to be printed should ignore identation and then use         */ 
    /* abs[indent] for subsequent lines.                                   */

    if (isambiguous(s, WANY) and ismodule(container(s))) {
	printname( rpt_output, stdout, s, false);
	(*rpt_output)(stdout, " = ");
        indent = -(strlen(symname(s->block))+1+strlen(symname(s))+3);
    } else {
	(*rpt_output)(stdout, "%s = ", symname(s));
        indent = -(strlen(symname(s))+3);
    }
    if (isvarparam(s)) {
	rpush(address(s, frame), sizeof(Address));
	addr = pop(Address);
    } else {
	addr = address(s, frame);
    }

    /* if the variable is a C++ ref or not a pass-by-value C++ class */
    /* dereference the pointer.					     */
    if (dump && s->language == cppLang)
    {
	Symbol t = rtype(s);
	if (s->param && t->class == CLASS && !t->symvalue.class.passedByValue
	    || t->class == CPPREF)
	{
	    push(Address,addr);
	    dereference();
	    addr = pop(Address);
	    if (t->class == CPPREF)
		len = size(t->type);
	    else
		len = size(t);
	}
	else
            len = size(s);
    }
    else
        len = size(s);

    inareg = false;
    if (s->param) {
	r = preg(s, frame);
	if (r != -1) {
	    inareg = true;
	    addr = r;
	}
    }
    if (s->storage == INREG || inareg)
    {
       pushregvalue(s, addr, frame, len);
       printval(s->type, 0);
    }
    else 
    {
       if (canpush(len)) 
       {
	   rpush(addr, len);
           if ( (s->class == CONST or s->class == REF)
                and s->param and rtype(s)->class == STRING)
             printval(s, 0);
           else
	     printval(s->type, indent);
       } 
       else
	   (*rpt_output)(stdout,  catgets(scmc_catd, MS_printsym, MSG_284,
					      "*** expression too large ***"));
    }
}

/*
 * Check if the name of a symbol is nil.
 */

boolean nilname (s)
Symbol s;
{
    Name n;
    char *name,*nameend;
    int i;
    boolean	rc;

    checkref(s);
    n = s->name;
    if (n == nil) {
	return true;
    } else {
	name = ident(n);
	if (name[0] == '\0' || name[0] == '?') {
	    return true;
	} else {
	    nameend = malloc(strlen(name)+1);
	    rc = (Boolean) ((sscanf(name,".%d%s", &i, nameend) == 2) && 
						(streq(nameend,"fake")));
	    free( nameend );
	    return rc;
	}
    }
}

/*
 * Print out the name of a symbol.
 */

public printname ( report, f, s, printmodule)
int (*report)();
File f;
Symbol s;
Boolean printmodule;		/* Set if we want to print name of module */
{
    if (s == nil) {
	(*report)(f, "(noname)");
    } else if (s == program) {
	(*report)(f, ".");
    /*
     * If this is a predefined type, we don't want to print where it was
     * defined even if there are more than one of these definitions
     */
    } else if (s->ispredefined == false &&
	       (isredirected() or isambiguous(s, WANY))) {
	printwhich(report, f, s, printmodule);
    } else {
	(*report)(f, "%s", symname(s));
    }
}

/*
 * Print the fully specified variable that is described by the given identifer.
 */

public printwhich ( report, f, s, printmodule)
int (*report)();
File f;
Symbol s;
Boolean printmodule;		/* Set if we want to print name of module */
{
    printouter( report, f, container(s));
    (*report)(f, "%s", symname(s));
    /* For handling routine from different modules (libraries) have same name */
    /* if this is not a C++ symbol, display what module it's from             */
    if (printmodule && isblock(s) && 
        (s->language != cppLang) && isambiguous(s, WFUNC)) {
        (*report)(f, " [%s]", fd_info[addrtoobj(prolloc(s))].pathname);
    }

}

/*
 * Print the fully qualified name of each symbol that has the same name
 * as the given symbol.
 */

public printwhereis ( report, f, n)
int (*report)( );
File f;
Name n;
{
    Symbol t;
    unsigned h;
    boolean foundSymbol = false;
    extern boolean cpp_whereis(/* report, File, Name */);

    t = lookup(n);
    while (t != nil) 
    {
       if (t->name == n)
       {
	  foundSymbol = true;
	  if (t->class == TAG && t->isClassTemplate)
	  {
	     TemplateClassListEntry s = t->symvalue.template.list;
	     while (s != nil)
	     {
	        printwhich( report, f, s->templateClass, true);
		(*report)(f, "\n");
		s = s->next;
	     }
	  }
	  else
	  {	
	     printwhich( report, f, t, true);
	     (*report)(f, "\n");
	  }
       }
       t = t->next_sym;
    }

    /* New for C++, we want to run thru the entire hash table looking for */ 
    /* classes which have data and function members whose names match the */
    /* search name.                                                       */
    if (cppModuleSeen && cpp_whereis(report, f, n))
        foundSymbol = true;
 
    if (!foundSymbol)
        error(catgets(scmc_catd, MS_check, MSG_16, "symbol not defined"));
}

public printouter ( report, f, s)
int (*report)();
File f;
Symbol s;
{
    Symbol outer;

    if (s != nil) {
	outer = container(s);
	if (outer != nil and outer != program) {
	    printouter(report, f, outer);
	}
	if (s->class == TAG && rtype(s)->class == CLASS)
	    (*report)(f, "%s::", symname(s));
	else
	    (*report)(f, "%s.", symname(s));
    }
}

public printdecl (s)
Symbol s;
{
    Language lang;

    checkref(s);
    if (s->language == nil or s->language == primlang) {
	lang = findlanguage(".s");
    } else {
	lang = s->language;
    }
    (*language_op(lang, L_PRINTDECL))(s);
}

public symboltype( s )
Symbol s;
{
	switch (s->class) {
	case VAR:
	case FVAR:
	case ARRAY:
	case SUBARRAY:
	case TOCVAR:
	case FPTR:
	case FPTEE:
        case REFFIELD:
        case REF:
        case CONST:
		symbol_type = VARIABLE;
		break;

	case CSECTFUNC:
	case FUNC:
	case FFUNC:
	case PROC:
	case PROG:
	case MODULE:
		symbol_type = FUNCTION;
		break;

	case CPPSYMLIST:
		if (s->symvalue.sList->sym->class == FUNC)
			symbol_type = FUNCTION;
		else
			symbol_type = UNKNOWN;  /* Class Template */
		break;

	/* This function currently is not used, so the C++ extensions have */
	/* not been made. 						   */
	case MEMBER:
	case CLASS:  /* type */
	case BASECLASS: 
	case NESTEDCLASS:
	case CPPREF: /* type */
	case PTRTOMEM: /* type */
	case ELLIPSES: /* type ? */
	case FRIENDFUNC: 
	case FRIENDCLASS:

	default:
		symbol_type = UNKNOWN;
		break;
	}

	return;
}

/*
 * Straight dump of symbol information.
 */
static	Boolean		pointer = false;

public psym (s)
Symbol s;
{
    (*rpt_output)(stdout, "name\t%s\n", symname(s));
    (*rpt_output)(stdout, "lang\t%s\n", language_name(s->language));
    (*rpt_output)(stdout, "level\t%d\n", s->level);
    (*rpt_output)(stdout, "class\t%s\n", classname(s));
    (*rpt_output)(stdout, "type\t0x%x", s->type);
    if (s->type != nil and s->type->name != nil) {
	(*rpt_output)(stdout, " (%s)", symname(s->type));
    }
    (*rpt_output)(stdout, "\nchain\t0x%x", s->chain);
    if (s->chain != nil and s->chain->name != nil) {
	(*rpt_output)(stdout, " (%s)", symname(s->chain));
    }
    (*rpt_output)(stdout, "\nblock\t0x%x", s->block);
    if (s->block != nil and s->block->name != nil) {
	(*rpt_output)(stdout, " (");
	printname( rpt_output, stdout, s->block, true);
	(*rpt_output)(stdout, ")");
    }
    (*rpt_output)(stdout, "\n");
    switch (s->class) {
	case TYPE:
	    (*rpt_output)(stdout, "size\t%d\n", size(s));
	    break;

	case VAR:
	case TOCVAR:
	case REF:
	    switch (s->storage) {
		case INREG:
		    (*rpt_output)(stdout, "reg\t%d\n", s->symvalue.offset);
		    break;

		case STK:
		    (*rpt_output)(stdout, "offset\t%d\n", s->symvalue.offset);
		    break;

		case EXT:
		    (*rpt_output)(stdout, "address\t0x%x\n",
							   s->symvalue.offset);
		    break;
	    }
	    (*rpt_output)(stdout, "size\t%d\n", size(s));
	    break;

	case CLASS:
	case RECORD:
	case UNION:
	case VARNT:
	    (*rpt_output)(stdout, "size\t%d\n", s->symvalue.offset);
	    break;

	case FIELD:
	case REFFIELD:
	    (*rpt_output)(stdout, "offset\t%d\n", s->symvalue.field.offset);
	    (*rpt_output)(stdout, "size\t%d\n", s->symvalue.field.length);
	    break;

	case MEMBER:
	    if (s->symvalue.member.type == DATAM)
	    {
	        (*rpt_output)(stdout, "offset\t%d\n", 
			      s->symvalue.member.attrs.data.offset);
	        (*rpt_output)(stdout, "size\t%d\n", 
			      s->symvalue.member.attrs.data.length);
	    }
	    break;

	case PROG:
	case PROC:
	case FUNC:
	case CSECTFUNC:
	    (*rpt_output)(stdout, "address\t0x%x\n",
						  s->symvalue.funcv.beginaddr);
	    if (isinline(s)) {
		(*rpt_output)(stdout, "inline procedure\n");
	    }
	    if (nosource(s)) {
		(*rpt_output)(stdout, "does not have source information\n");
	    } else {
		(*rpt_output)(stdout,  "has source information\n");
	    }
	    break;

	case RANGE:
	    prangetype(s->symvalue.rangev.lowertype);
	    (*rpt_output)(stdout, "lower\t%d\n", s->symvalue.rangev.lower);
	    prangetype(s->symvalue.rangev.uppertype);
	    (*rpt_output)(stdout, "upper\t%d\n", s->symvalue.rangev.upper);
	    break;

	default:
	    /* do nothing */
	    break;
    }
}

private prangetype (r)
Rangetype r;
{
    switch (r) {
	case R_CONST:
	    (*rpt_output)(stdout, "CONST");
	    break;

	case R_ARG:
	    (*rpt_output)(stdout, "ARG");
	    break;

        case R_REGARG:
            (*rpt_output)(stdout, "REGARG");
            break;

        case R_REGTMP:
            (*rpt_output)(stdout, "REGTEMP");
            break;

        case R_STATIC:
            (*rpt_output)(stdout, "STATIC");
            break;

	case R_TEMP:
	    (*rpt_output)(stdout, "TEMP");
	    break;

	case R_ADJUST:
	    (*rpt_output)(stdout, "ADJUST");
	    break;
    }
}

/*
 * Print out the value on top of the stack according to the given type.
 */

public printval (t, indent)
Symbol t;
int indent;
{
    Symbol s;

    checkref(t);
    if (t) {
        if (t->class == TYPEREF) {
	    resolveRef(t);
	}
	switch (t->class) {
	    case LABEL:
	    case PROC:
	    case FUNC:
	    case CSECTFUNC:
	        s = pop(Symbol);
	        (*rpt_output)(stdout, "%s", symname(s));
	        break;

	    default:
	        if (t->language == nil or t->language == primlang) {
		    (*language_op(findlanguage(".c"), L_PRINTVAL))(t, indent);
	        } else {
		    if (t->language == cppLang) {
			cpp_initreflist();
		    }

		    (*language_op(t->language, L_PRINTVAL))(t, indent);
		    if (t->language == cppLang) {
			cpp_clrreflist();
		    }
	        } 
	        break;
	}
    }
}

/*
 * Print out the value of a record, field by field.
 */

public printrecord (s)
Symbol s;
{
    Symbol f;

    if (s->chain == nil) {
	error( catgets(scmc_catd, MS_printsym, MSG_320,
						      "record has no fields"));
    }
    PRINTF "(");
    sp -= size(s);
    f = s->chain;
    if (f != nil) {
	for (;;) {
	    printfield(f);
	    f = f->chain;
	if (f == nil) break;
	    PRINTF ", ");
	}
    }
    PRINTF ")");
}

/*
 * Print out a field.
 */

private printfield (f)
Symbol f;
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

/*
 * Print out the contents of an array.
 * Haven't quite figured out what the best format is.
 *
 * This is rather inefficient.
 *
 * The "2*elsize" is there since "printval" drops the stack by elsize.
 */

public printarray (a)
Symbol a;
{
    Stack *savesp, *newsp;
    Symbol eltype;
    long elsize;
    String sep;

    savesp = sp;
    sp -= (size(a));
    newsp = sp;
    eltype = rtype(a->type);
    elsize = size(eltype);
    PRINTF "(");
    if (eltype->class == PACKARRAY or eltype->class == ARRAY or
        eltype->class == UNION or eltype->class == VARNT or
        eltype->class == SET or eltype->class == PACKSET or
        eltype->class == PACKRECORD or eltype->class == RECORD or 
	eltype->class == CLASS)
    {
	sep = "\n";
	PUTCHAR('\n');
    } else {
	sep = ", ";
    }
    for (sp += elsize; sp <= savesp; sp += 2*elsize) {
	if (sp - elsize != newsp) {
	    FPUTS(sep);
	}
	if (expandunions && (eltype->class == UNION))
        {
	   printunion(eltype);
	}
	else
	   printval(eltype, 0);
    }
    sp = newsp;
    if (streq(sep, "\n")) {
	PUTCHAR('\n');
    }
    PRINTF ")");
}

/* 
 * Calculate offset into arrays for printing
 */

public calc_offsets (s, x, offsets, elsize, eltype)
Symbol s;
short *x;
long  offsets[];
long *elsize;
Symbol *eltype;
{
   if (s->class != ARRAY)
   {
       *eltype = rtype(s);                 /* calculate type of array */
       *elsize = size(s);                  /* calculate element size  */
       return;
   }
   else
   {
      long bounds;
      calc_offsets (s->type, x, offsets, elsize, eltype);
      bounds = (s->chain->symvalue.rangev.upper) - 
			       (s->chain->symvalue.rangev.lower) + 1;
      if (*x == 0)
	 offsets[*x] = *elsize * bounds;
      else
	 offsets[*x] = offsets[(*x)-1] * bounds;
      (*x)++;
   }
}

/*
 * Print out a character using ^? notation for unprintables.
 */

public printchar (c)
char c;
{
    if (c == 0) {
	PUTCHAR('\\');
	PUTCHAR('0');
    } else if (c == '\n') {
	PUTCHAR('\\');
	PUTCHAR('n');
    } else if (c != 0 and c < ' ') {
	PUTCHAR('^');
	PUTCHAR(c - 1 + 'A');
    } else if (c >= ' ' && c <= '~') {
	PUTCHAR(c);
    } else {
	PRINTF "\\%o",c);
    }
}

/*
 * Print out a value for a range type (integer, char, or boolean).
 */

public printRangeVal (val, t)
long val;
Symbol t;
{
    if (t->class == SCAL)
      printEnum( val, t);
    else
    if (t == t_boolean->type or istypename(t->type, "boolean")) {
	if ((boolean) val) {
	    PRINTF "true");
	} else {
	    PRINTF "false");
	}
    } else if (t == t_char->type or ischartype(t)) {
         if (varIsSet("$hexchars")) {
             PRINTF "0x%x", (unsigned char)val);
         }
         else {
             if(t->symvalue.rangev.lower < 0) {
                /* decimal output when signed char */
                PRINTF "%d", val);
             }
             else { /* octal or char output when unsigned char */
              PUTCHAR('\'');
              printchar((char) val);
              PUTCHAR('\'');
             }
	}
    } else if (istypename(t->type, "wchar")) {
        /*  print the value as a wide character  */ 
        PRINTF "%C", val);
    } else if (istypename(t->type, "NLS")) {	/* National Language Support */
	wchar_t temp_var[2];
	char temp_cstr[MB_LEN_MAX + 1];
	temp_var[0] = (wchar_t) val;
	temp_var[1] = (wchar_t) 0;
	sprintf( temp_cstr, "%S", temp_var );
	PRINTF "%s",temp_cstr);
    } else if (hexints) {
         switch(size(t)) {
            /* ensure a negative short does not get 4 bytes printed */
            case sizeof(short): PRINTF "0x%x",(unsigned short)val); break;
            default: PRINTF "0x%lx", val); break;
         }
    } else if (octints) {
         switch(size(t)) {
            /* ensure a negative short does not get 4 bytes printed */
            case sizeof(short): PRINTF "0%o",(unsigned short)val); break;
            default: PRINTF "0%lo", val); break;
         }
    } else if (t->symvalue.rangev.lower >= 0) {
	PRINTF "%lu", val);
    } else {
	PRINTF "%ld", val);
    }
}

/*
 * NAME: printlonglong
 *
 * FUNCTION: Print a long long integer.
 *
 * PARAMETERS:
 *      val     - input number
 *      t       - Symbol describing input number
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: nothing
 */

public printlonglong (val, t)
LongLong val;
Symbol t;
{
    if (hexints) {
        PRINTF "0x%llx", val);
    } else if (octints) {
        PRINTF "0%llo", val);
    } else if (t->symvalue.rangev.is_unsigned) {
        PRINTF "%llu", val);
    } else {
        PRINTF "%lld", val);
    }
}

/*
 * Print out an enumerated value by finding the corresponding
 * name in the enumeration list.
 */

public printEnum (i, t)
integer i;
Symbol t;
{
    register Symbol e;

    e = t->chain;
    while (e != nil and e->symvalue.constval->value.lcon != i) {
	e = e->chain;
    }
    if (e != nil) 
    {
	if (t->language == cppLang && rtype(t->block->block)->class == CLASS)
	{
	    assert(t->block->class == TAG);
	    PRINTF "%s::", symname(t->block->block));
	}
	PRINTF "%s", symname(e));
    } 
    else 
    {
        /* handles unsigned int enum values... */
        if (t->chain && t->chain->type->class == SCAL &&
            istypename(t->chain->type->type,"unsigned int"))
            PRINTF "%u", i);
        else
	    PRINTF "%d", i);
    }
}

/*
 * NAME: printString
 *
 * FUNCTION: Print out a null-terminated string
 *
 * PARAMETERS:
 *      addr   - starting address of the string
 *      quotes - flag to indicate if the string should be quoted
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: no return value
 *
 */


public printString (addr, quotes)
Address addr;
boolean quotes;
{
    register Address a;
    boolean quoted = false;
    int mbl;
    union {
	char ch[MB_LEN_MAX];
	int word;
    } u;

    if (varIsSet("$hexstrings"))
    {
	PRINTF "0x%x", addr);
    }
    else
    {
	a = addr;

        /*  do until the null-terminator is reached  */
	do 
        {
            /*  read a character - it may be multi-byte  */
	    dread(&u, a, sizeof(u));
 
            /*  if the memory contains all x'ff', treat as an 
                  invalid pointer  */
	    if (u.word == -1)
            {
		PRINTF "(invalid char ptr (0x%x))",addr);
		return;
	    }

            /*  if this is a quoted string and this is the first
                  time through the loop  */
	    if ((!quoted) && (quotes))
            {
		 PUTCHAR('"');
		 quoted = true;
	    }

            /*  if the character is not a null-terminator  */
            if (u.ch[0] != '\0')
            {
              int j;
         
              mbl = mblen(&u.ch[0], MB_LEN_MAX);

              /*  if mblen cannot determine the character length,
                    it will return -1 - this is probably a partial
                    character.  Print each byte of it as a single
                    character.  */
    
              mbl = (mbl > 0) ? mbl : 1;
    
              /*  if the character is multi-byte  */
              if (mbl > 1)
              {
                /*  print directly by looping through all the bytes  */
                for (j = 0; j < mbl; j++)
                  (*rpt_output)( stdout, "%c", u.ch[j]);

              }
              else
              {
                /*  call printchar to be sure non-printable
                      characters are handled correctly  */
                printchar(u.ch[0]);
              }
            }
            a += mbl;
	} while (u.ch[0] != '\0');

        /*  if this is a quoted string  */
	if (quotes) 
        {   
          PUTCHAR('"');
	}
    }
}


public printsubarray(y, ptr, curbound, Type)
long y;
struct subdim *ptr;
Symbol curbound;
Symclass Type;
{
   static long index_list[10];
   long i, len;
   Address addr;
   Symbol elem_type;

   if (ptr)
   {
      for (i = ptr->lb; i <= ptr->ub; i++)
      {
	  index_list[y] = i;
	  printsubarray(y+1, ptr->back, curbound, Type);
      }
   }
   else
   {
      char index[16];
      unsigned indexLengths = 0;
      extern Boolean dumpvarsFirstLine;
      int Subar_field_offset;

      addr = array_addr;
      Subar_field_offset = Subar_offset != 0 ? Subar_offset - addr : 0 ;
      curbound = rtype(curbound);
      
      for (i = 0; i < y; i++)
      {
         sprintf(index, "%d", index_list[i]);
	 indexLengths += strlen(index) + 2;
         PRINTF "[%s]", index);
	 evalindex( curbound, addr, index_list[i]);
	 addr = pop(long);
	 curbound = curbound->type;
	 while (curbound->class == TYPE)	/* take care of cases like */
	   curbound = curbound->type;           /* array->type->string     */
      }
      if (ptr_to_subarray)
      {
	 dread(&addr, addr, sizeof(Address));
	 rpush(addr, size(curbound->type));
      }
      else if (Subar_save_sym)
        rpush(addr+Subar_field_offset, size(Subar_save_sym)); 
      else
        rpush(addr, size(curbound)); 

      PRINTF " = ");
      curbound=Subar_save_sym != 0 ? Subar_save_sym : curbound;
      indexLengths += 3;
      elem_type = (curbound->type->class == TAG) ? curbound->type : curbound;

      switch (Type)
      {
        case PACKARRAY:
        case ARRAY:
           subarray = false;
           printarray(curbound);
           break;
        case PACKRANGE:	
	case RANGE:
           printrange(Subar_save_sym != 0 ? curbound : curbound->type);
	   break;
	case REAL:
	   printreal(curbound);
	   break;
	case STRING:
	   printPstring(curbound->type, curbound->symvalue.size);
	   break;
	case PACKSET:
	case SET:
	   printSet(curbound->type);
	   break;
	case SCAL:
           if(!Subar_save_sym)
           {
             while (elem_type->class == TAG)
               elem_type = elem_type->type;
             printEnum(popsmall(elem_type), elem_type);
           }
           else
           {
             printEnum(popsmall(curbound->type),curbound);
           }
	   break;
	case PTRTOMEM:
	   subarray = false;
	   cpp_printPtrToMem(elem_type, 0, curbound->name);
	   break;
	case CLASS:
	   subarray = false;
	   dumpvarsFirstLine = true;
	   cpp_printClass(elem_type->type, -indexLengths, curbound->name);
	   dumpvarsFirstLine = false;
	   break;
  	case PACKRECORD:
	case RECORD:
	   subarray = false;
           printrecord(Subar_save_sym != 0 ? Subar_save_sym 
                                           : elem_type->type);
	   break;
	case VARNT:
	   subarray = false;
           printVrecord(elem_type->type);
           break;
        case GROUP:
        case RGROUP:
           subarray = false;
           if (ptr_to_subarray)
              cobol_printval(elem_type->type);
           else
              cobol_printval(elem_type);
           break;
	case PTR:
	   {
	      Address a;
	      Symbol s = curbound;
	      Symbol t;

	      t = rtype(s);
	      if (t->language == pascalLang)
		a = pop_pas_ptr();
	      else
	        a = pop(Address);
	      if (a == 0)
		PRINTF "(nil)");
	      else if ((t->class == RANGE or t->class == PTR) and
						    	     ischartype(t))
		 printString(a, (boolean) (s->language != primlang));
	      else
		 PRINTF "0x%x", a);
	   }
	   break;
	case UNION:
	     if (expandunions)
	     {
	        subarray = false;
                printunion(Subar_save_sym != 0 ? curbound :elem_type->type);
             }
             else
             {
	       if (ord(curbound->class) > ord(LASTCLASS))
		   panic("printval: bad class %d", ord(curbound->class));
               sp -= size(Subar_save_sym != 0 ? curbound : elem_type->type);
               PRINTF "[%s]", c_classname(Subar_save_sym != 0 ?
                                                curbound : elem_type->type));

             }
	     break;
      }    /* end switch */
      PRINTF "\n");
   }
}


/* 
 * Printing of quad precision numbers using Fortran
 * quad convertion routine.
 */
public printquad(report, f, q)
int (*report)( );
File f;
quadf q;
{
     char *qstr;		/* buffer pointer of output */
     integer leftdigit; 	/* number of digits left of "." */
     integer sign;		/* sign of value */
     char *ptr;
     integer i = 0;

     /* Call convertion routine to get output string  (default len = 31) */ 
     qstr = (char *) _qecvt(q, 31, &leftdigit, &sign);

     /* Check if it's INF or NaNs */
     if (!isdigit(*qstr)) {
        (*report)(f, "%s", qstr);
	return;
     }

     /* Now it must be a valid number, so print sign if needed */
     if (sign)
        (*report)(f, "-");
     ptr = qstr;

     /* Print value in e format (0.123e-12) if... */
     if (leftdigit < -3 || leftdigit > 17) {
        (*report)(f, "%c.", *ptr);
	if (*++ptr == 0) 
	  (*report) (f, "0");
	else
	  (*report) (f, "%s", ptr);
	(*report) (f, "e%d", leftdigit-1);
     } else if (leftdigit < 0) {
        /* Else if we need to add 0's in front... (-3 < leftdigit < 0) */
	(*report) (f, "0.");
	while (leftdigit++ < 0)
	   (*report) (f, "0");
	(*report) (f, "%s", ptr);
     } else {
        /* Else just print as normal... (0 <= leftdigit <= 17) */
        while (i++ < leftdigit) {
           (*report) (f, "%c", *ptr++);
        }
        if (leftdigit == 0)
           (*report) (f, "0");
        (*report) (f, ".%s", ptr);
    }
}



public printreal(s)
Symbol s;
{
  Symbol t;
  quadf qnum;
  extern Boolean is_fortran_padded();


        t = rtype(s);
	switch (t->symvalue.size) {
	   case sizeof(float):
	     prtreal((double) pop(float), true);
	     break;
	   case sizeof(double):
	     prtreal(pop(double), false);
	     break;
	   case 2*sizeof(double):  /* FORTRAN REAL*16 - quad numbers */
	     qnum.val[1] = pop(double);
	     qnum.val[0] = pop(double);
	     printquad(rpt_output, stdout, qnum);
	     break;
	   case 4*sizeof(double):  /* Padded FORTRAN REAL*16 - AUTODBL */
	     if (is_fortran_padded(s)) {
		 sp -= 2*sizeof(double);	/* pop out the padding */
	         qnum.val[1] = pop(double);
	         qnum.val[0] = pop(double);
	         printquad(rpt_output, stdout, qnum);
	         break;
	     }
	   default:
    	     sp -= size(t);
	     error( catgets(scmc_catd, MS_printsym, MSG_325,
	      "unable to evaluate floating point variable of length %d bytes")
							   , t->symvalue.size);
	     break;
	}
}

public printrange(s)
Symbol s;
{
   Symbol t;

   t = rtype(s->type);
   if (t->type == t_boolean or istypename(t->type, "boolean")) 
   {
      printRangeVal(popsmall(s), t);
   }
   else if (t->type == t_char or ischartype(t)) 
   {
       if(t->symvalue.rangev.lower < 0)  printRangeVal(pop(signed char), t);
       else printRangeVal(pop(unsigned char), t);  /* unsigned char */
   }
   else if (istypename(t->type, "wchar"))
   {
      printRangeVal(pop(short), t);
   }
   else if (t->type == t_real or istypename(t->type, "double") or
            istypename(t->type, "float")) 
   {
      switch (t->symvalue.rangev.lower)
      {
          case sizeof(float):
	      prtreal((double) (pop(float)), true);
	      break;

          case sizeof(double):
	      prtreal(pop(double), false);
	      break;

          default:
	      panic( catgets(scmc_catd, MS_printsym, MSG_325,
				"bad real size %d"), t->symvalue.rangev.lower);
   	      break;
      }
   }
   else 
   {
      if (t->symvalue.rangev.size == sizeofLongLong)
        printlonglong(poplonglong(s), t);
      else
        printRangeVal(popsmall(s), t);
   }
}

/*
 * Print out the value of a real number in Pascal notation.
 * This is, unfortunately, different than what one gets
 * from "%g" in printf.
 */

public prtreal (r, isfloat)
double r;
boolean isfloat;
{
    char buf[256];

    if (isfloat) {
       sprintf(buf, "%.9g", r);
    } else {
       sprintf(buf, "%.17g", r);
    }
    if (buf[0] == '.') {
	PRINTF "0%s", buf);
    } else if (buf[0] == '-' and buf[1] == '.') {
	PRINTF "-0%s", &buf[1]);
    } else {
	PRINTF "%s", buf);
    }
    if (!isalpha((int)(buf[1])) && index(buf, '.') == nil) {
	PRINTF ".0");
    }
}

public reset_subarray_vars()
{
   struct subdim *q, *p;

   q = p = subdim_head;  /* Reset subarray variables */
   while (q)
   {
      q = p->next;
      free((void *) p);
      p = q;
   }
   subdim_head = nil;
   subdim_tail = nil;
   no_more_subarray = false;
   ptr_to_subarray = false;
   subarray = false;
   subarray_seen = false;
   array_addr = 0;
   array_sym = nil;
   Subar_save_sym=0;
   Subar_offset=0;
}
