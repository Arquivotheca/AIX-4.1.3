static char sccsid[] = "@(#)79	1.20.1.13  src/bos/usr/ccs/lib/libdbx/cplusplus.c, libdbx, bos412, 9445B412 11/4/94 21:00:42";
/*
 *   COMPONENT_NAME: CMDDBX
 *
 *   FUNCTIONS: cpp_addToVirtualList
 *		cpp_addreflist
 *		cpp_clrreflist
 *		cpp_emptyVirtualList
 *		cpp_equivalent
 *		cpp_fndreflist
 *		cpp_init
 *		cpp_initreflist
 *		cpp_isVirtual
 *		cpp_passaddr
 *		cpp_printClass
 *		cpp_printPtrToMem
 *		cpp_printdecl
 *		cpp_printfuncname
 *		cpp_printqfuncname
 *		cpp_printtype
 *		cpp_printval
 *		cpp_tempname
 *		cpp_touchClass
 *		cpp_typematch
 *		printBaseClass
 *		printClassType
 *		printDecl
 *		printNesting
 *
 *   ORIGINS: 26,27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Copyright (c) 1982 Regents of the University of California
 */

#include "dbx_msg.h" 
nl_catd scmc_catd;

/*
 * C++-dependent symbol routines.
 */

#ifdef _NO_PROTO
#include <varargs.h>
#else
#include <stdarg.h>
#endif /* _NO_PROTO */
#include "defs.h"
#include "symbols.h"
#include "printsym.h"
#include "languages.h"
#include "cplusplus.h"
#include "tree.h"
#include "eval.h"
#include "operators.h"
#include "mappings.h"
#include "process.h"
#include "runtime.h"
#include "machine.h"

static void cpp_addreflist(Symbol);
static int cpp_fndreflist(Symbol);
void setupMemberFunc();

extern Boolean unique_fns;
struct subdim {
    long ub, lb;
    struct subdim *next, *back;
};
extern struct subdim *subdim_tail;
extern Symbol array_sym;
extern Boolean subarray;

static int baseNest = 0;
static int firstIndent; 
Boolean dumpvarsFirstLine = false;
Boolean virtualBaseClass = false;

/*
 * Initialize C++ language information.
 */

Language cppLang;
Boolean cppModuleSeen = false;
Name this;

public void cpp_init()
{
   private Boolean cpp_passaddr();
   private void cpp_printdecl();
   public Boolean cpp_typematch();
   public void cpp_modinit();

   cppLang = language_define("c++", ".C");
   this = identname("this", false);

   language_setop(cppLang, L_PRINTDECL, cpp_printdecl);
   language_setop(cppLang, L_PRINTVAL, cpp_printval);
   language_setop(cppLang, L_TYPEMATCH, cpp_typematch);
   language_setop(cppLang, L_PASSADDR, cpp_passaddr); 
   language_setop(cppLang, L_MODINIT, c_modinit);
   language_setop(cppLang, L_BUILDAREF, buildaref);
   language_setop(cppLang, L_EVALAREF, c_evalaref);
   language_setop(cppLang, L_HASMODULES, c_hasmodules);
   language_setop(cppLang, L_FOLDNAMES, c_foldnames);
}

public Boolean cpp_tempname(n)
/* return true if n is a xlC compiler-generated temporary name */
Name n;
{
    char *r = ident(n);
    return (r[0] == '_' && r[1] == '_') ? true : false;
}

public Boolean cpp_equivalent(s, t)
/* return true if s and t, which have the same name and class, represent  */
/* the same type, despite being different symbols. This can happen should */
/* they come in from different modules.                                   */
Symbol s, t;
{
    if (s == t)
	return true;
    else if (s->name == nil || s->name != t->name ||
             s->language != cLang && s->language != cppLang ||
             t->language != cLang && t->language != cppLang)
    {
	return false;
    }

    if ((s->block->class == MODULE && t->block->class == MODULE ||
	 s->block == t->block) &&
        (s->class == TAG || s->class == TYPE))
        return true; 
    else
	return false;
}

/*
 * Test if two C++ types are compatible.
 */
public Boolean cpp_typematch(type1, type2)
Symbol type1, type2;
{
    if (type1 == type2)
        return true;
    else 
    {
        Symbol t1 = rtype(type1);
        Symbol t2 = rtype(type2);
	if (t1->class == CPPREF)
	    t1 = rtype(type1 = t1->type);
	if (t2->class == CPPREF)
	    t2 = rtype(type2 = t2->type);
        if (t1 == t2)
	    return true;
        if (t1->class == PTRTOMEM && t2->class == PTRTOMEM) 
            return (t1->type == t2->type 
	            && t1->symvalue.ptrtomem.memType == 
	 	       t2->symvalue.ptrtomem.memType
	            && t1->symvalue.ptrtomem.ptrType == 
	 	       t2->symvalue.ptrtomem.ptrType) ? true : false;
        else if (t1->class == SCAL && t2->class == SCAL)
	    return t1 == t2;
        return c_typematch(type1, type2);
    }
}

public void cpp_touchClass(c)
/* Give a class Symbol the once-over, removing all member objects that are */
/* are undefined (only member functions and static data members need be    */
/* defined). Also ensure that pure virtual functions have a FUNC symbol    */
/* set up for them.     						   */
Symbol c;
{
    extern boolean lazy;
    Symbol temp_sym;

    assert(c->class == CLASS);
    if (!c->symvalue.class.touched)
    {
	Symbol m, prev;

	for (prev = nil, m = c->chain; m != nil; m = m->next_sym)
	{
	    if (m->symvalue.member.type == DATAM && 
		m->symvalue.member.isStatic &&
		m->symvalue.member.attrs.staticData.varSym == nil &&
		m->symvalue.member.attrs.staticData.dName->name != m->name)
	    {
                if (varIsSet("$showunlinked"))
		{
		    warning(catgets(scmc_catd, MS_cplusplus, MSG_629,
		            "static data member %1$s not defined; removing"),
			m->symvalue.member.attrs.staticData.dName->qualName);
		}
		if (prev == nil)
		    c->chain = m->next_sym;
		else
		    prev->next_sym = m->next_sym;
		if (!lazy)
		    EraseDemangledName(
				  m->symvalue.member.attrs.staticData.dName);
	    }
	    else if (m->symvalue.member.type == FUNCM &&
		     m->symvalue.member.attrs.func.dName == nil)
	    {
		DemangledName d = Demangle(m->name);
		if (m->symvalue.member.attrs.func.isVirtual != CPPPUREVIRTUAL)
	        {
  		    /* last chance read. Try to find if the function symbol */
		    /* came in from a module not compiled with debug.       */
		    Symbol s;

		    find(s, m->name) where s->class == FUNC endfind(s);

		    if (s != nil)
		    {
			delete(s);

			m->symvalue.member.attrs.func.funcSym = s;
		        s->isCppFunction = true;
			s->isMemberFunc = true;
			s->symvalue.funcv.u.memFuncSym = m;
			if (m->symvalue.member.attrs.func.isInline)
			    s->block = m->block->block;
			s->name = identname(d->qualName, true);
			s->language = cppLang;

			m->name = d->name;
			m->symvalue.member.attrs.func.dName = d;
			m->symvalue.member.attrs.func.isSkeleton = true;

		 	insertsym(s);

			prev = m;
		    }
		    else 
		    {
                        Symbol funcdef;
                        Name n;
                        n = identname(d->qualName, true);
 
                        find (funcdef, n) where
                          (funcdef->isCppFunction) && 
                          (!funcdef->isMemberFunc) &&
                          (funcdef->symvalue.funcv.u.dName->mName == d->mName)
                        endfind (funcdef)

                        if (funcdef != NULL)
                        {
                          setupMemberFunc(funcdef, d);
                        }
                        else
                        {
                          if (varIsSet("$showunlinked"))
                          {
			    warning(catgets(scmc_catd, MS_cplusplus, MSG_630,
				"member function %1$s not defined; removing"),
				    d->fullName);
                          }
                          if (prev == nil)
			    c->chain = m->next_sym;
                          else
			    prev->next_sym = m->next_sym;
                          EraseDemangledName(d);
                        }
		    }
	        }
		else
		{
		    m->symvalue.member.attrs.func.dName = d;
		    m->name = d->name;
	 	}
	    }
	    else if (m->class == NESTEDCLASS
              && (temp_sym = rtype(m->type))->class == CLASS)
	    {
              /*  The following check is to avoid infinite loop  */ 
              if (c != temp_sym)
              {
                cpp_touchClass(temp_sym);
                prev = m;
              }
	    }
	    else
		prev = m;
	}
	c->symvalue.class.touched = true;
    }
}

/*
 * Qualified virtual function name lists. When the user qualifies a virtual
 * function name (i.e. h.E::f()), the virtual function call mechanism must
 * be overridden. Since qualification is only known about at name resolution
 * time and the virtual function call mechanism is invoked eval time, this
 * list is the medium of communication between the two phases. It is emptied
 * after each command has been evaluated.
 */

typedef struct QualVirtual {
    Node vFunc;
    Symbol classType;
    AccessList path;
    struct QualVirtual *next;
} *QualVirtual;

private QualVirtual qvListHead = nil;

public void cpp_addToVirtualList(vfunc, classType, path)
Node vfunc;
Symbol classType;
AccessList path;
{
    QualVirtual q = new(QualVirtual);
    q->vFunc = vfunc;
    q->classType = classType;
    q->path = path;
    q->next = qvListHead;
    qvListHead = q;
}

public Boolean cpp_isVirtual(vfunc, classType, path)
Node vfunc;
Symbol *classType;
AccessList *path;
{
    QualVirtual q = qvListHead;
    while (q != nil)
    {
	if (q->vFunc == vfunc)
        {
            *path = q->path;
            *classType = q->classType;
	    return true;
        }
	q = q->next;
    }
    return false;
}

public void cpp_emptyVirtualList()
{
    QualVirtual q = qvListHead, p;
    while (q != nil)
    {
	p = q->next;
	free(q);
	q = p;
    }
    qvListHead = nil;
}

/*
 * Print the declaration of a C++ entity.
 */
public void cpp_printdecl(s)
Symbol s;
{
    private void printDecl(/* Symbol, int */);

    printDecl(s, 0);
}

private printNesting(nesting)
Symbol nesting;
{
    assert(nesting->class == TAG);
    if (nesting->block->class == TAG)
	printNesting(nesting->block);
    (*rpt_output)(stdout, "%s::", ident(forward(nesting)->name));
}

private printBaseClass(s)
Symbol s;
{
    if (s != nil) 
    {  
	Symbol t;

        assert(s->class == BASECLASS);

        if (s->symvalue.baseclass.isVirtual)
	    (*rpt_output)(stdout, "virtual ");

	t = rtype(s->type);
	assert(t->class == CLASS);
        switch (s->symvalue.baseclass.access) 
        {
	    case PRIVATE: 
		if (t->symvalue.class.key == 's')
                    (*rpt_output)(stdout, "private ");
                break;
	    case PROTECTED: 
                (*rpt_output)(stdout, "protected "); 
		break;
	    case PUBLIC:
		if (t->symvalue.class.key == 'c')
                    (*rpt_output)(stdout, "public "); 
		break;
	    default:
	        assert(false);
        }
        assert(s->type != nil);

	if (t->block->block->class == TAG)
	    printNesting(t->block->block);
        (*rpt_output)(stdout, "%s", ident(s->name));
    }
}

private printClassType(s, t, indent)
Symbol s;
Symbol t;
int indent;
{
    String className;
    int section;
    Symbol u;
       
    s = forward(s);
    assert(s->class == TAG);

    assert(t->class == CLASS);
    cpp_touchClass(t);

    switch (t->symvalue.class.key) 
    {
	case 'c':
	    (*rpt_output)(stdout, "class");
	    section = PRIVATE;
	    break;
	case 'u':
	    (*rpt_output)(stdout, "union");
	    section = PUBLIC;
	    break;
	case 's':
	    (*rpt_output)(stdout, "struct");
	    section = PUBLIC;
	    break;
	default:
	    assert(false);
	    break;
    }

    if (!cpp_tempname(s->name))
        (*rpt_output)(stdout, " %s", symname(s));

    if (t->type != nil) 
    {			 
	/* print base classes */

	(*rpt_output)(stdout, ": ");
	for (u = t->type; u != nil; u = u->chain)
	{ 
	    printBaseClass(u);
	    if (u->chain != nil)
		(*rpt_output)(stdout, ", ");	
	}
    }

    (*rpt_output)(stdout, " {\n");
    for (u = t->chain; u != nil; u = u->next_sym) /* print the members */
    {
	/* There are times when we suppress printing of a member:
	 *    1. The "member" is an enumeration constant promoted from a
	 *       nested enumerator.
	 *    2. The member is a nested type that is nameless and is not
	 *       an anonymous union (and hence some member(s) - those 
	 *       following it along the next_sym chain - have this type
	 *       as their type)
	 *    3. Promoted members of anonymous unions.
	 */
	if ( /* 1. */ u->class == CONST ||
	     /* 2. */ u->isAnonMember ||
	    (/* 3. */ u->class == NESTEDCLASS && cpp_tempname(u->name) &&
	              rtype(u->type)->class == CLASS &&
	              (rtype(u->type)->symvalue.class.key != 'u' ||
	               (u->next_sym != nil && !cpp_tempname(u->next_sym->name)
	                && rtype(u->next_sym->type) == rtype(u->type)))))
	    continue;

	if (!u->symvalue.member.isCompGen)
	{
            if (u->symvalue.member.access == PRIVATE && section != PRIVATE)
	    {
                (*rpt_output)(stdout, "%*cprivate:\n", indent + 4, ' ');
                section = PRIVATE;
            }
            if (u->symvalue.member.access == PROTECTED && section != PROTECTED) 
            {
                (*rpt_output)(stdout, "%*cprotected:\n", indent + 4, ' ');
                section = PROTECTED;
            }
            if (u->symvalue.member.access == PUBLIC && section != PUBLIC) 
            {
                (*rpt_output)(stdout, "%*cpublic:\n", indent + 4, ' ');
                section = PUBLIC;
            }
	    printDecl(u, indent + 8);
        }
    }
    if (indent != 0)
       (*rpt_output)(stdout, "%*c", indent, ' ');
    (*rpt_output)(stdout, "}");
}
   
public void cpp_printfuncname(s, n)
/* s is a (function) MEMBER */
Symbol s;
int n;
{
    /* "s" is a member function symbol. Write its attributes and */
    /* unqualified name. If it has no associated function Symbol */
    /* (the loader optimized it away), print only its parameter  */
    /* type list. Otherwise, print its parameter list.		 */

    if (s->symvalue.member.attrs.func.isInline)
        (*rpt_output)(stdout, "inline ");
    if (s->symvalue.member.isStatic)
        (*rpt_output)(stdout, "static ");
    else if (s->symvalue.member.attrs.func.isVirtual)
        (*rpt_output)(stdout, "virtual ");

    assert(s->symvalue.member.attrs.func.dName != nil);

    if (s->symvalue.member.attrs.func.kind == CPPFUNC)
    {
        /* We want to see if the function is a conversion operator. If so */
        /* we do not print out the returned type as it should be obvious. */
        char *name = (char *) symname(s);
        if (!(!strncmp(name, "operator ", 9) &&
              strcmp(name + 9, "new") && strcmp(name + 9, "delete"))) 
	{
	    Symbol t = s->type->type;
            cpp_printtype(s, t, n);
	    if (t->class != PTR && t->class != CPPREF && t->class != PTRTOMEM)
                (*rpt_output)(stdout, " ");
        }
    }

    if (s->symvalue.member.attrs.func.funcSym != nil &&
        !s->symvalue.member.attrs.func.isSkeleton)
    {
        (*rpt_output)(stdout, "%s", 
	  	      ident(s->symvalue.member.attrs.func.dName->name));
        /* We do not want to print parameters (they should all be compiler */
        /* generated) for destructors.                                     */
        if (s->symvalue.member.attrs.func.kind == CPPDTOR) 
           (*rpt_output)(stdout,"()");
        else
           ansic_listparams(s->symvalue.member.attrs.func.funcSym);
	if (s->symvalue.member.attrs.func.isConst)
	    (*rpt_output)(stdout, " const");
	if (s->symvalue.member.attrs.func.isVolatile)
	    (*rpt_output)(stdout, " volatile");
    }
    else
    {
        (*rpt_output)(stdout, "%s%s",
                      ident(s->symvalue.member.attrs.func.dName->name),
                      s->symvalue.member.attrs.func.dName->params);
    }
    /* else the parameter types - the best we can do - were printed as */
    /* part of above.					               */

    if (s->symvalue.member.attrs.func.isVirtual == CPPPUREVIRTUAL)
	(*rpt_output)(stdout, " = 0");
}
   
public void cpp_printqfuncname(s, n)
/* s is a CSECTFUNC or FUNC */
Symbol s;
int n;
{
    char *name;

    /* "s" is a function symbol. If it is a member function, write its */
    /* attributes, qualified name and parameters. Otherwise, write its */
    /* name and parameters (it has no attributes).		       */

    Symbol t = (s->isMemberFunc ? s->symvalue.funcv.u.memFuncSym : nil);

    name = ident(s->name);
    if (t != nil && t->symvalue.member.attrs.func.isSkeleton)
    {
	cpp_printfuncname(t, n);
	return;
    }
    else if (t != nil)
    {
        char *unqualName = ident(t->name);

        if (t->symvalue.member.attrs.func.isInline)
            (*rpt_output)(stdout, "inline ");
        if (t->symvalue.member.isStatic)
            (*rpt_output)(stdout, "static ");
        else if (t->symvalue.member.attrs.func.isVirtual)
            (*rpt_output)(stdout, "virtual ");
        
        /* We want to see if the function is a conversion operator.  If */
        /* so we do not print out the returned type as it should be     */
        /* obvious.                                                     */
        if (!(strncmp(unqualName, "operator ", 9) == 0 &&
              strcmp(unqualName + 9, "new") != 0 &&
              strcmp(unqualName + 9, "delete") != 0)) 
	{
	    Symbol t = s->type;
            cpp_printtype(s, t, n);
	    if (t->class != PTR && t->class != CPPREF && t->class != PTRTOMEM)
                (*rpt_output)(stdout, " ");
        }
    }
    else /* not a class member */
    {
	Symbol t = s->type;
	if (s->class != PROC) 		/* if not an unnamed block */
            cpp_printtype(s, t, n);
	if (t->class != PTR && t->class != CPPREF && t->class != PTRTOMEM)
            (*rpt_output)(stdout, " ");
    }

    (*rpt_output)(stdout, "%s", ident(s->name));

    ansic_listparams(s);

    if (t != nil)
    {
        if (t->symvalue.member.attrs.func.isConst)
            (*rpt_output)(stdout, " const");
        if (t->symvalue.member.attrs.func.isVolatile)
            (*rpt_output)(stdout, " volatile");
        if (t->symvalue.member.attrs.func.isVirtual == CPPPUREVIRTUAL)
	    (*rpt_output)(stdout, " = 0");
    }
}

public void cpp_printtype (s, t, n)
Symbol s;
Symbol t;
int n;
{
    switch(t->class) {
        case CLASS:
	    if (t->isConst)
		(*rpt_output)(stdout, "const ");
	    if (t->isVolatile)
		(*rpt_output)(stdout, "volatile ");

	    printClassType(s, t, n);
	    break;

	case CPPREF:
	{
	    Symbol s = t->type;
	    cpp_printtype(t, s, n);
	    if (s->class != PTR && s->class != PTRTOMEM)
	        (*rpt_output)(stdout, " ");
	    (*rpt_output)(stdout, "&");
	    break;
	}

        case PTRTOMEM:
	{
	    Symbol u = t->type;
            if (u->class == FFUNC) 
	    {
                /* PTRTOMEM -> member function */
		Symbol v = u->type;

                cpp_printtype(t, v, n);
		if (v->class != PTR && v->class != PTRTOMEM && 
		    v->class != CPPREF)
		    (*rpt_output)(stdout, " ");
                (*rpt_output)(stdout, "(");
                (*rpt_output)(stdout,
                      ident(forward(t->symvalue.ptrtomem.memType)->name));
                (*rpt_output)(stdout, "::*");

		if (t->isConst || t->isVolatile)
		    (*rpt_output)(stdout, t->isConst ? " const ":" volatile ");

                if (s->name != nil) 
                    (*rpt_output)(stdout, ident(s->name));
                (*rpt_output)(stdout, ")()");
            }
            else 
	    {
                /* PTRTOMEM -> data member */
                cpp_printtype(t, u, n);
		(*rpt_output)(stdout, " ");
                (*rpt_output)(stdout, 
                      ident(forward(t->symvalue.ptrtomem.memType)->name));
                (*rpt_output)(stdout, "::*");

		if (t->isConst || t->isVolatile)
		    (*rpt_output)(stdout, t->isConst ? " const ":" volatile ");

                if (s->name != nil) 
                    (*rpt_output)(stdout, ident(s->name));
            }
            break;
	}

	case PTR:
	{
	    Symbol s = t->type;
	    cpp_printtype(t, s, n);
	    if (s->class != PTR && s->class != CPPREF && s->class != PTRTOMEM)
	        (*rpt_output)(stdout, " ");
	    (*rpt_output)(stdout, "*");

	    if (t->isConst || t->isVolatile)
		(*rpt_output)(stdout, t->isConst ? " const ":" volatile ");
	    break;
	}

	case ELLIPSES: 
	    (*rpt_output)(stdout, "...");
	    break;

	case MEMBER: 
	case BASECLASS: 
	case NESTEDCLASS: 
	case FRIENDFUNC: 
	case FRIENDCLASS:
	    assert(false);

	default:
	    if (t->isConst)
		(*rpt_output)(stdout, "const ");
	    if (t->isVolatile)
		(*rpt_output)(stdout, "volatile ");

            C_printtype(s, t, n);
	    break;
    }
}

/*
 * Print the declaration of a C++ variable
 */
private void printDecl(s, indent)
Symbol s;
int indent;
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

    switch (s->class) 
    {
        case CONST:
            if (s->type->class == SCAL) {
                (*rpt_output)(stdout, "enumeration constant with value %d",
				      s->symvalue.constval->value.lcon);
            } else {
                (*rpt_output)(stdout, "const %s = ", symname(s));
                printval(s, indent);
            }
            break;

        case TYPE:
        case VAR:
        case TOCVAR:
            if (s->class != TYPE and s->storage == INREG) {
                (*rpt_output)(stdout, "register ");
    	    }
            if (s->class == VAR and s->isStaticMember) {
                (*rpt_output)(stdout, "static ");
    	    }
            c_printdef(s, indent);
            break;
    
        case MEMBER:
            if (s->symvalue.member.type == DATAM) /* data member */
            {
	        if (s->symvalue.member.isStatic)
	            (*rpt_output)(stdout, "static ");
	        c_printdef(s, indent);
	        if (isbitfield(s)) {
	            (*rpt_output)(stdout, " : %d", s->symvalue.field.length);
	        }
            }
            else
	        cpp_printfuncname(s, indent);
            break;

        case FRIENDFUNC:
            (*rpt_output)(stdout, "friend ");
            cpp_printtype(s->type, s->type->type, 0);
	    if (rtype(s->type->type)->class != PTR && 
		rtype(s->type->type)->class != CPPREF && 
		rtype(s->type->type)->class != PTRTOMEM)
	        (*rpt_output)(stdout, " ");
	    (*rpt_output)(stdout, "%s", 
		s->symvalue.member.attrs.func.dName->fullName);
            break;

        case FRIENDCLASS:
            (*rpt_output)(stdout, "friend class %s", ident(typename(s)));
            break;

        case TAG:
            if (s->type == nil) {
                findtype(s);
	        assert(s != nil);
	    }
    	    cpp_printtype(s, s->type, indent);
            break;

        case RANGE:
        case ARRAY:
        case RECORD:
        case CLASS: 
        case UNION:
        case PTR:
        case FFUNC:
            semicolon = false;
            cpp_printtype(s, s, indent);
            break;

        case SCAL:
            (*rpt_output)(stdout, "(enumeration constant, value %d)",
				  s->symvalue.iconval);
            break;

        case PROC:
        case FUNC:
        case CSECTFUNC:
            cpp_printqfuncname(s);
            semicolon = true;
            newline = true;
            break;

        case MODULE:
            semicolon = false;
            fn = symname(s);
            (*rpt_output)(stdout, catgets(scmc_catd, MS_cplusplus, MSG_93,
			  "source file \"%s.C\""), (unique_fns) ? ++fn : fn);
            break;

        case PROG:
            semicolon = false;
            (*rpt_output)(stdout, catgets(scmc_catd, MS_cplusplus, MSG_94,
				  "executable file \"%s\""), symname(s));
            break;

	case CPPREF:
	    cpp_printtype(s, s, indent);
	    break;

	case BASECLASS: 
	case PTRTOMEM: 
	    cpp_printtype(s, s->type, indent);
	    break;

	case NESTEDCLASS: 
            if (s->type->class == TYPE) 
	    {
               (*rpt_output)(stdout, "typedef ");
               c_printdef(s->type, 0);
            }
            else 
               cpp_printtype(s->type, rtype(s->type), indent);
            break;

	case ELLIPSES: 
        default:
            (*rpt_output)(stdout, "[%s]", classname(s));
            break;
    }
  
    if (semicolon)
        (*rpt_output)(stdout, ";");
    if (newline)
        (*rpt_output)(stdout, "\n");
}

/* 
 * Should the variable be passed as an address?
 */

private Boolean cpp_passaddr(param, exprtype) 
Symbol param, exprtype;
{
    Boolean b;

    /* if the paramter is actually a reference. */
    if (param->type->class == CPPREF) {
        /* its address should be passed  */
        b = true;
    }
    else {
        /* otherwise follow c convention */
        b = c_passaddr(param, exprtype);
    }
    return b;
}

/* 
 * Print value of c++ specific types
 */

extern Boolean notderefed;            /* var has not been dereferenced */
Boolean specificptrtomember = false;  /* are we printing just a pointer to a */
                                      /* member or a pointer to a specific   */
                                      /* instance of a member.               */
public void cpp_printval (s, indent)
Symbol s;
int indent;
{
    Symbol t;
    long thethisptr;
    Symclass class = s->class;

    while ((class == CONST) || (class == TYPE) || (class == VAR) ||
           (class == REF) || (class == FVAR) || (class == TAG) ||
           (class == TOCVAR))
    {
      if (class == TAG)
        cpp_addreflist(s);
      s = s->type;
      class = s->class;
    }

    switch (class) {  
        case CPPREF:                       /* reference type               */
                                           /* if the type is non-scalar    */
            t = rtype(s->type);
            if (notderefed && (t->class == ARRAY || t->class == CLASS)) 
	    {
                pop(Address);
                (*rpt_output)(stdout, "&(...)"); 
            }
            else {
               if (notderefed) 
	       {
                   int tsize;
                   Address addr;

                   tsize = size(t);
                   addr = pop(Address);
                   dread(sp, addr, tsize);
                   sp += tsize;
               }

               /* print references below this point as addresses */
               notderefed = true;                                           
               /* print out what this reference points to          */
               cpp_printval(s->type, indent);
            }
            break; 

	case CLASS:
	    if (subarray)
	    {
	       printsubarray(0, subdim_tail, array_sym->type, CLASS);
	       reset_subarray_vars();
	    }
	    else
	       cpp_printClass(s, indent, nil);
	    break;

        case MEMBER:   /* (MH) */
            if (isbitfield(s)) 
	    {
                int i;
                t = rtype(s->type);
                i = extractField(s, t);
                if (t->class == SCAL)
		{
		    printf("%s::", symname(t->block));
                    printEnum(i, t);
		}
                else
                    printRangeVal(i, t);
            } 
	    else
                cpp_printval(s->type, indent);
            break;

        case FFUNC:                            
	    /* if we are printing a type of function check to see  */
            /* if we are printing a ptr to a member function.      */
            if (specificptrtomember) 
	    {
               long function;
               Symbol funcname;
               char *symfunc;

               /* get the "this" pointer & function address off the */
               /* stack and print them.                             */

               thethisptr = pop(long);
               function = pop(long);
               (*rpt_output)(stdout, "(function = 0x%x, this = 0x%x)",
                                     function, thethisptr);

               /* find the function that this is refering to and  */
               funcname = whatblock(function);

               /* print it's name if the address can be associated */
               /* with a name.                                     */
               symfunc = symname(funcname);
               if (symfunc != nil)
                  (*rpt_output)(stdout, " (%s)", symfunc);
               else
                  (*rpt_output)(stdout, " (???)");
               specificptrtomember = false;
            }
            else {
               /* if not a pointer to a member function, then process */
               /* as normal.			                      */
               c_printval(s, indent);
            }
            break;

	case PTRTOMEM: 
	    if (subarray)
	    {
               printsubarray(0, subdim_tail, array_sym->type, PTRTOMEM);
               reset_subarray_vars();
	    }
	    else
	       cpp_printPtrToMem(s);
            break;

	case BASECLASS: 
	case NESTEDCLASS: 
	case ELLIPSES: 
	case FRIENDFUNC: 
	case FRIENDCLASS:
	    sp -= size(s);
	    (*rpt_output)(stdout, "[%s]", classname(s));
	    break;
	    
	default:
            c_printval(s, indent);
    }
}

public void cpp_printPtrToMem(s)
Symbol s;
{
    /* if this is a pointer to a member function */
    if (rtype(s->type)->class == FFUNC) 
    {
       long pdisp, tdisp, fdisp, faddr;

       /* pop off all the info on the stack to be printed out */
       pdisp = pop(long);
       tdisp = pop(long);
       fdisp = pop(long);
       faddr = pop(long);

       /* print out the faddr portion of the pointer to member */
       if (faddr == 0)
	   (*rpt_output)(stdout, "(faddr = (nil), ");
       else
	   (*rpt_output)(stdout, "(faddr = 0x%x, ",faddr);

       /* if the ptr has virtual bases then printout all fields */
       if (s->symvalue.ptrtomem.hasVBases)
	  (*rpt_output)(stdout, "fdisp = %d, tdisp = %d, pdisp = %d)",
			fdisp, tdisp, pdisp);
       else 
	  /* if the ptr has multiple bases but no virtual bases then */
          /* printout all but pdisp field.			     */
	  if (s->symvalue.ptrtomem.hasMultiBases)
	     (*rpt_output)(stdout, "fdisp = %d, tdisp = %d)", fdisp, tdisp);
	  else
	     /* otherwise only print out the faddr and fdisp fields */
	     (*rpt_output)(stdout, "fdisp = %d)", fdisp);
    }
    else 
    {
       /* otherwise we have a pointer to a data member             */
       long mdisp, pdisp;
       
       pdisp = pop(long);
       mdisp = pop(long);

       /* if we have virtual bases then printout the entire pointer to */
       /* member structure              			       */
       if (s->symvalue.ptrtomem.hasVBases)
	   (*rpt_output)(stdout, "(mdisp = %d, pdisp = %d)", mdisp, pdisp);
       else 
	   /* otherwise only the mdisp field is valid so only print it out */
	   (*rpt_output)(stdout, "(mdisp = %d)",mdisp);
    }
}

/*
 * Print out a C++ class.
 */

public void cpp_printClass (s, indent, name)
Symbol s;
int indent;
Name name;
{
    Symbol f;
    Stack *savesp;
    integer n, off, len;
    Boolean comma, parenthesis, TAGfollows;
    extern Boolean expandunions;
    char *tempStack;
    Boolean tempStackActive;

    assert(s->class == CLASS);
    cpp_touchClass(s);

    sp -= size(s);
    savesp = sp;

    if (s->symvalue.class.key == 'u' && !expandunions)
    {
        (*rpt_output)(stdout, "[union]");
	return;
    }

    /* See if there's any data members in base classes... */
    f = s->type;
    while (f != nil) {
        if (f->class == BASECLASS && 
            !f->symvalue.baseclass.isVirtual &&
            varIsSet("$showbases")) {
           Symbol c = rtype(f->type);
           if (c->class == CLASS) {   /* make sure we are dealing with class */
               Symbol baseClassType = f->type;
               sp += (f->symvalue.baseclass.offset + size(c));
               baseNest++;

               /* If indent is negative, we want to indent on subsequent  */
               /* base class lines, and the eventual class line.  The     */
               /* indent value wanted is abs[indent].                     */
               if (indent < 0) {
                  indent = -indent;
                  firstIndent = indent;
                  /* if the variable name for "varname = " is too large,  */
                  /* put out a new line and yank the indent back.         */ 
                  if (indent > 40) {
                     (*rpt_output)(stdout, "\n");
                     indent = 4;
                     dumpvarsFirstLine = false;
                  }   
               }

               /* Normally, f->type has a class of TAG and a name field.    */
               /* In the event of a base class which has a circular ref     */  
               /* (say a ptr to itself), f->type will have a class of TAG   */
               /* but no name ptr.  f->type->type should have a class of    */
               /* TAG and a valid name ptr in this case.                    */
               if (f->type->name == nil)
                  baseClassType = f->type->type;

               assert(baseClassType->class == TAG && 
		      baseClassType->name != nil);
	       cpp_printClass(c, indent+4, baseClassType->name);
               (*rpt_output)(stdout, "\n");
               sp = savesp;
           }
        }
        f = f->chain;
    }

    if (indent > 0) 
       if (dumpvarsFirstLine)
          (*rpt_output)(stdout, "%*c", indent - firstIndent, ' ');
       else
          (*rpt_output)(stdout, "%*c", indent, ' ');
    dumpvarsFirstLine = false;

    if (baseNest != 0) {
       /* if we are printing out the name of a virtual base class, we want */
       /* to include the term virtual before the name.  Through all the    */
       /* recursion of this routine, we can know that the name we are      */
       /* printing is for a virtual base class by having virtualBaseClass  */
       /* set, and baseNest of only 1 for printing of the virtual class.   */
       if (baseNest == 1 &&
           virtualBaseClass) {   
          (*rpt_output)(stdout, "virtual ");
          virtualBaseClass = false;
       }
       (*rpt_output)(stdout, "%s:", ident(name));
    }

    /* assume no need for comma separator between class data elements */
    comma = false; 
    /* need a lead parenthesis before class data elements */
    parenthesis = true;
    /* assume no need to allow for a class instance in a class indentation */
    TAGfollows = false;
    for (f = s->chain; f != nil; f = f->next_sym) {
        if (f->class == NESTEDCLASS || f->class == FRIENDCLASS ||
            f->class == FRIENDFUNC || f->class == CONST)
        {
           continue;
        }

        if (comma &&
            f->symvalue.member.type == DATAM && 
            (!f->symvalue.member.isCompGen)) {
           (*rpt_output)(stdout, ", ");
           comma = false;
           if (TAGfollows) {
              /* If we just printed out a data member that's a TAG, we want */
              /* to add another line to delmit.                             */
              (*rpt_output)(stdout, "\n ");
              if (indent > 0)
                 (*rpt_output)(stdout, "%*c", indent, ' ');
           }
           TAGfollows = false;
        }

        /* Here we want to print out the base class that the data member, */
        /* which is a virtual base pointer, points to.  This is only      */
        /* done for the highest level class that we are printing (hence,  */
        /* the check of baseNest).  Note also that we do not print out    */
        /* a virtual base pointer's class if the pointer points to its    */
        /* own class.                                                     */
        tryVirtBase:
        if (f->symvalue.member.type == DATAM &&
            !baseNest &&
            varIsSet("$showbases") &&
            f->symvalue.member.attrs.data.isVbasePtr &&
            !f->symvalue.member.attrs.data.isVbaseSelfPtr) {
           int addr;
           Symbol TAGtype;
	   unsigned TAGsize;

           off = f->symvalue.member.attrs.data.offset;
           len = f->symvalue.member.attrs.data.length;
           n = (off + len + BITSPERBYTE - 1) / BITSPERBYTE - sizeof(Address);
           /* read the virtual base pointer's contents from the stack */
           addr = *(int*)(sp + n);

           TAGtype = f->type->type;
           if (TAGtype->type->class != CLASS)
              TAGtype = f->type->type->type;
           /* read the class object in and place it on the stack */
           TAGsize = size(TAGtype->type);
           tempStack = sp = malloc(TAGsize);
           tempStackActive = true;
           dread(sp, addr, TAGsize);
           sp += TAGsize;
           baseNest++;
           if (indent < 0)
              indent = -indent;
           firstIndent = indent;
           dumpvarsFirstLine = true;
           virtualBaseClass = true;
           cpp_printClass(TAGtype->type, indent+4, TAGtype->name);
           (*rpt_output)(stdout, "\n");
           if (indent > 0)
               (*rpt_output)(stdout, "%*c", indent, ' ');
           tempStackActive = false;
	   free(tempStack);
           sp = savesp;
           /* process more virtual base class pointers before parenthesis */
           f = f->next_sym;
           if (f == nil)
              break;
           goto tryVirtBase;
        }

        if (parenthesis)
           (*rpt_output)(stdout, "(");
        parenthesis = false;

        /* print value of regular data members that are not compiler gend */
        if (f->symvalue.member.type == DATAM && !f->symvalue.member.isCompGen)
	{
	   Symbol t = rtype(f->type);
	   Symbol s;
           int TAGindent, saveNest;

           off = f->symvalue.member.attrs.data.offset;
           len = f->symvalue.member.attrs.data.length;
           n = (off + len + BITSPERBYTE - 1) / BITSPERBYTE;

           /* if this is a reference then dereference the pointer and put */
           /* the contents after all the info on the stack                */
           if (t->class == CPPREF) {
              int tempsp;    
              Address addr;
              int rsize;
                                        /* find the size of the element    */
              rsize = size(t->type);
                                        /* get to the reference value      */
              tempsp = (int) sp + n - sizeof(Address);
              addr = * (Address *) tempsp;

	      if (cpp_fndreflist(t->type)) {
		(*rpt_output)(stdout, "%s = 0x%x", symname(f), addr);
		comma = true;
		continue;
	      }
                                        
              sp = tempStack = malloc(rsize);
              tempStackActive = true;
                                        /* derefernce the pointer          */
              dread(sp, addr, rsize);
                                        /* get sp past element put on stack*/
              sp += rsize;
	      s = t->type;
	      t = rtype(t->type);
           }
	   /* for static member, needs to obtain value from real symbol */
	   else if (f->symvalue.member.isStatic) {
	      int rsize;
	      s = f->symvalue.member.attrs.staticData.varSym;
	      if (s == nil)		/* skip if real symbol not known */
		 continue;
	      rsize = size(s);
              sp = tempStack = malloc(rsize);
              tempStackActive = true;
              dread(sp, address(s), rsize);
              sp += rsize;
	   }
           else
	   {
	      tempStackActive = false;
              sp += n;
	      s = f;
	   }
	
           (*rpt_output)(stdout, "%s = ", symname(f));
           /* Here we want to see if the data member is a class or       */
           /* structure that has a base class.  If so, we skip to the    */
           /* next line to make the format more attractive.              */
           if (t->class == CLASS && t->type != nil) 
	   {
              /* Indicate that we have further TAG formatting to do. */
              TAGfollows = true;

              /* Allow for the negative indent value from the dump cmd. */
              if (TAGindent < 0)
                 TAGindent = indent = -indent;
	      else
                 TAGindent = indent;

              TAGindent += 1 + strlen(symname(f)) + 3;
              if (TAGindent > 40)
                 TAGindent = indent;
              (*rpt_output)(stdout, "\n");
           }
           else
              TAGindent = 0;

           /* we will require a comma iff any more data elements follow */
           comma = true;

           /* Nothing is easy. We need to reset baseNest before we call */
           /* cpp_printval because we may end up back in printClass as  */
           /* f's type may be of class TAG.  This causes problems if    */
           /* baseNest is already non-zero.                             */
           saveNest = baseNest;
           baseNest = 0;

           cpp_printval(s, TAGindent);
           baseNest = saveNest;

	   if (tempStackActive)
	       free(tempStack);
           sp = savesp;
        }
    }
    if (parenthesis)
       (*rpt_output)(stdout, "(");
    (*rpt_output)(stdout, ")");

    if (baseNest != 0)
       baseNest--;
}

/*
 * reference stack routines and data structures.
 *
 * - the basic idea here is to stop C++ from going into
 *   an infinite loop when we hit looping references.
 */

typedef struct reflist reflist;

static struct reflist {
    Symbol	type;
    reflist	*next;
} *Reflist, *Reflisttail;

/*
 * NAME: cpp_initreflist
 *
 * FUNCTION: Initializes the reference list
 *
 * PARAMETERS: NONE
 *
 * DATA STRUCTURES:
 *	Reflist	- Initialized to NULL
 *
 * RETURNS: NONE
 */
void cpp_initreflist(void)
{
	Reflist = NULL;
}


/*
 * NAME: cpp_clrreflist
 *
 * FUNCTION: Frees memory associated with current reference list
 *
 * PARAMETERS: NONE
 *
 * RETURNS: NONE
 */
void cpp_clrreflist(void)
{
    reflist *r, *n;

    for (r = Reflist; r != NULL; r = n) {
	n = r->next;
	free((void *) r);
    }
}


/*
 * NAME: cpp_addreflist
 *
 * FUNCTION: Adds a new item to the reference linked list
 *
 * PARAMETERS:
 *	type	- Item to be added to the list
 *
 * DATA STRUCTURES:
 *	Reflist		- Top of list, may be changed if this is first item
 *	Reflisttail	- Bottom of list, will be updated to point to latest
 *			  item added.
 *
 * RETURNS: NONE
 */
static void cpp_addreflist(Symbol type)
{
    reflist *new;

    if ((new = (reflist *) malloc(sizeof(*new))) != NULL) {
	new->type = type;
	new->next = NULL;
	if (Reflist == NULL) {
	    Reflist = new;
	} else {
	    Reflisttail->next = new;
	}
	Reflisttail = new;
    }
}


/*
 * NAME: cpp_fndreflist
 *
 * FUNCTION: Searches for item in reference list
 *
 * PARAMETERS:
 *	type	- Item to search for in reference list
 *
 * DATA STRUCTURES:
 *	Reflist	- List to search on
 *
 * RETURNS:
 *	0: Not found
 *	1: Found
 */
static int cpp_fndreflist(Symbol type)
{
    reflist *r;

    for (r = Reflist; r != NULL && r->type != type; r = r->next)
	;

    return (r != NULL);
}
