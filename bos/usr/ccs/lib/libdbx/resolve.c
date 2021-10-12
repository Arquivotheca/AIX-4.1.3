static char sccsid[] = "@(#)67	1.1.2.10  src/bos/usr/ccs/lib/libdbx/resolve.c, libdbx, bos411, 9438C411a 9/24/94 10:23:45";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: traverse, resolveName, resolveQual, findMember, isMember, 
 *	      findQual
 *
 * ORIGINS: 27
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
 */
#ifdef KDBXRT
#include "rtnls.h"		/* MUST BE FIRST */
#endif
#include "dbx_msg.h" 	 	/* include file for message texts */
nl_catd  scmc_catd;   		/* catalog descriptor for scmc conversion */
#define GetMsg(X, Y) catgets(scmc_catd, MS_symbols, (X), (Y))

#include <setjmp.h>
#define _ALL_SOURCE
#include <sys/ioctl.h>
#include "defs.h"
#include "symbols.h"
#include "resolve.h"
#include "cplusplus.h"
#include "names.h"
#include "overload.h"

/*
 * Name resolution
 */

private char *findClassName(mName)
char *mName;
{
    int i = 0, nQuals = 0, nameLength;

    if (mName[0] == 'Q')
    {
	nQuals = 0;
	while (isdigit(mName[++i]))
	    nQuals = nQuals * 10 + mName[i] - '0';
	assert(mName[i++] == '_');

	while (nQuals > 1)
	{
	    nameLength = 0;
	    while (isdigit(mName[i]))
		nameLength = nameLength * 10 + mName[i++] - '0';
	    i += nameLength;

	    nQuals -= 1;
	}
    }
    else if (mName[0] == '_')
    {
	assert(strncmp(mName, "__L_", 4) == 0);
	i = 4;
    }
    nameLength = 0;
    while (isdigit(mName[i]))
    {
	nameLength = nameLength * 10 + mName[i] - '0';
	i += 1;
    }
    assert(strlen(&mName[i]) == nameLength);
    return &mName[i];
}

public Node buildAccess(path, classType, partialAccess)
/* Given a class type, a path of base classes from the type to a member's */
/* class and a Node (which may be nil) describing the access node tree    */
/* already built to get to an object of "classType", build the remainder  */
/* of the access node tree and return it.				  */
AccessList path;
Symbol classType;
Node partialAccess;
{
    assert(classType->class == CLASS);
    if (path != nil)
    {
        /* look down the path for the last virtual base. */
        AccessList vbase = nil;

        AccessList p = path;
        while (p != nil)
        {
            if (p->baseClass->symvalue.baseclass.isVirtual)
                vbase = p;
            p = p->next;
        }

        if (vbase != nil) /* virtual base found */
        {
            /* find the virtual base pointer within the class */
            Symbol m = classType->chain;
            String className;

            className = ident(typename(vbase->baseClass));
            while (m != nil)
            {
                if (strncmp(ident(m->name), "__vbp", 5) == 0)
                {
                    String name = ident(m->name);
                    if (streq(findClassName(&name[5]), className))
                        break;
                }
                m = m->next_sym;
            }
            assert(m != nil);

            partialAccess = buildAccess(vbase->next, classType,
                               	        build(O_RVAL,
                                              build(O_DOT,
                                                    partialAccess,
                                                    build(O_SYM, m))));
        }
        else
        {
            while (path != nil)
            {
                partialAccess = build(O_DOT, partialAccess,
                                      build(O_SYM, path->baseClass));
                path = path->next;
            }
        }
    }
    return partialAccess;
}

private boolean isBase(c1, c2)
Symbol c1, c2;
{
    Symbol b;

    assert(c1->class == CLASS);
    assert(c2->class == CLASS);

    /* is c2 a direct base class of c1? */
    b = c1->type;
    while (b != nil && rtype(b) != c2)
	b = b->chain;

    if (b == nil)
    {
	/* is c2 an indirect base class of c2? */
	b = c1->type;
	while (b != nil && !isBase(rtype(b), c2))
	    b = b->chain;
    }

    return b != nil;
}

private boolean isVBase(c1, c2)
Symbol c1, c2;
{
    Symbol b;

    assert(c1->class == CLASS);
    assert(c2->class == CLASS);

    /* is c2 a direct virtual base class of c1? */
    b = c1->type;
    while (b != nil && (rtype(b) != c2 || !b->symvalue.baseclass.isVirtual))
	b = b->chain;

    if (b == nil)
    {
	/* is c2 an indirect virtual base class of c2? */
	b = c1->type;
	while (b != nil && !isVBase(rtype(b), c2))
	    b = b->chain;
    }

    return b != nil;
}

private boolean identical(b1, p1, b2, p2)
Symbol b1, b2;
AccessList p1, p2;
{
    AccessList l1 = nil, l2 = nil;
    /* 
       return "true" if the last base ("l1") of "[b1, p1]" is the same actual 
       class instance as the last base class ("l2") in "[b2, p2]". This will 
       occur when the paths [b1, p1] and [b2, p2] past the last virtual class
       on the paths (inclusive) are the same.

       We assume that the paths are rooted at the same class.
    */

    assert(b1 != nil);
    assert(b2 != nil);

    while (p1 != nil)
    {
	if (p1->baseClass->symvalue.baseclass.isVirtual) 
	    l1 = p1;
	p1 = p1->next;
    }
    if (l1 != nil)
	p1 = l1;

    while (p2 != nil)
    {
	if (p2->baseClass->symvalue.baseclass.isVirtual) 
	    l2 = p2;
	p2 = p2->next;
    }
    if (l2 != nil)
	p2 = l2;

    while (p1 != nil && p2 != nil)
    {
	if (rtype(p1->baseClass->type) == rtype(p2->baseClass->type))
	{
	    p1 = p1->next;
	    p2 = p2->next;
	}
	else 
	    return false;
    }

    return p1 == nil && p2 == nil && 
	   (l1 != nil && l2 != nil || /* virtual base found along the path */
	    rtype(b1->type) == rtype(b2->type));
}

private boolean dominates(b1, p1, b2, p2)
Symbol b1, b2;
AccessList p1, p2;
{
    Symbol class1;

    /* return "true" if the last base of "[b1, p1]" contains as a base class */
    /* any base in "[b2, p2]" - i.e., that it dominates p2.		     */
    assert(b1 != nil);
    assert(b2 != nil);

    /* get the last base of "[b1, p1]" */
    if (p1 != nil)
    {
	while (p1->next != nil) 
	    p1 = p1->next;
	b1 = p1->baseClass;
    }
    class1 = rtype(b1->type);

    while (p2 != nil) 
    {
	if (isVBase(class1, rtype(p2->baseClass->type)))
	    return true;
	p2 = p2->next;
    }
    return isVBase(class1, rtype(b2->type));
}

private Symbol findBase(class, baseName, accessPath)
Symbol class;
Name baseName;
AccessList *accessPath;
{
    /* first search to see if it is a direct base of "class" */
    Symbol bestBase = class->type;
    AccessList bestPath = nil;

    *accessPath = nil;

    while (bestBase != nil && bestBase->name != baseName)
	bestBase = bestBase->chain;

    if (bestBase == nil)
    {
	/* next search to see if it is an indirect base of "class" */

	Symbol i = class->type;
	int found = false;

	while (i != nil)
	{
	    AccessList p;

	    if (findBase(rtype(i), baseName, &p) != nil)
	    {
		if (found && !identical(i, p, bestBase, bestPath))
		{
		    error(catgets(scmc_catd, MS_resolve, MSG_609, 
			  "Ambiguous base class \"%1$s\"; please qualify."),
			  ident(baseName));
		    /*NOTREACHED*/
		}
		else if (!found)
		{
		    found = true;
		    bestBase = i;
		    bestPath = p;
		}
	    }
	    i = i->chain;
	}
    }

    if (bestBase != nil)
    {
	*accessPath = new(AccessList);
	(*accessPath)->baseClass = bestBase;
	(*accessPath)->next = bestPath;
    }
    return bestBase;
}

public void freeAccessList(AccessList p)
{
    AccessList q;
    while (p != nil)
    {
	q = p->next;
	free(p);
	p = q;
    }
}

public Symbol isMember(classType, memberName, criteria)
/* Return the Symbol of the member this lookup resolves to, but not the */
/* access information.							*/
Symbol classType;
Name memberName;
unsigned long criteria;
{
    Symbol b, m;

    assert(classType->class == CLASS);
    cpp_touchClass(classType);

    /* first search to see if it is a member of "instance", meeting the */
    /* criteria 							*/
    m = nil;
    if (criteria & WOTHER)
    {
        m = classType->chain;
	while (m != nil)
	{
	    if ((m->class == MEMBER || m->class == CONST /* enum. const */) && 
		m->name == memberName)
		return m;
	    m = m->next_sym;
	}
    }
    if (m == nil && (criteria & WSEARCH) != WOTHER)
    {
        m = classType->chain;
	while (m != nil)
	{
	    if (m->class == NESTEDCLASS && meets(m->type, criteria) && 
		memberName == typename(m))
		return m;
	    m = m->next_sym; 
	}
    }

    assert(m == nil);

    /* try searching the base classes of the class */
    b = classType->type;
    while (b != nil)
    {
	if (m = isMember(rtype(b), memberName, criteria))
	    return m;
	b = b->chain;
    }
    return nil;
}

public Symbol findVirtualMember(class, d, accessPath)
Symbol class;
DemangledName d;
AccessList *accessPath;
{
    Symbol m = nil;
    Symbol b = class->type;
    AccessList path = nil;

    assert(class->class == CLASS);
    cpp_touchClass(class);

    /* search the base classes of the class */
    while (b != nil)
    {
	if ((m = findVirtualMember(rtype(b), d, &path)) != nil)
	{
	    *accessPath = new(AccessList);
	    (*accessPath)->baseClass = b;
	    (*accessPath)->next = path;

	    return m;
	}
	b = b->chain;
    }

    m = class->chain;
    while (m != nil)
    {
	if (m->class == MEMBER && 
	    m->symvalue.member.type == FUNCM &&
	    m->symvalue.member.attrs.func.dName->name == d->name && 
	    streq(m->symvalue.member.attrs.func.dName->params, d->params) && 
	    m->symvalue.member.attrs.func.isVirtual != CPPREAL)
	{
	    return m;
	}
	m = m->next_sym;
    }
    return nil;
}

public Symbol findMember(class, memberName, accessPath, criteria)
Symbol class;
Name memberName;
AccessList *accessPath;
unsigned long criteria;
{
    Symbol b, bestBase;
    Symbol m, bestM;
    AccessList path, bestPath;

    assert(class->class == CLASS);
    cpp_touchClass(class);

    *accessPath = nil;

    /* first search to see if it is a member of "instance", meeting the */
    /* criteria 							*/
    m = nil;
    if (criteria & WOTHER)
    {
        m = class->chain;
	while (m != nil)
	{
	    if ((m->class == MEMBER || m->class == CONST /* enum. const */) && 
		m->name == memberName)
	    {
		return m;
	    }
	    m = m->next_sym;
	}
    }

    if ((criteria & WSEARCH) != WOTHER)
    {
        m = class->chain;
	while (m != nil)
	{
	    if (m->class == NESTEDCLASS && meets(m->type, criteria) && 
		memberName == typename(m))
		return forward(m->type);
	    m = m->next_sym; 
	}
    }

    assert(m == nil);

    /* try searching the base classes of the class */
    b = class->type;
    bestM = nil;
    while (b != nil)
    {
	if ((m = findMember(rtype(b), memberName, &path, criteria)) != nil)
	    if (bestM == nil || dominates(b, path, bestBase, bestPath))
	    {
		bestM = m;
		bestBase = b;
		bestPath = path;
	    }
	    else 
	    {
		if (!dominates(bestBase, bestPath, b, path) &&
		    !identical(bestBase, bestPath, b, path))
		{
		    error(catgets(scmc_catd, MS_resolve, MSG_610, 
		          "Please qualify ambiguous reference to \"%1$s\"."),
			  ident(memberName));
		}
		freeAccessList(path);
	    }
	b = b->chain;
    }

    if (bestM != nil)
    {
	*accessPath = new(AccessList);
	(*accessPath)->baseClass = bestBase;
	(*accessPath)->next = bestPath;
    }

    return bestM;
}

public Node findQual(qualifier, block, query)
/* Given a qualifier and a block in which to look it up, do that, returning */
/* the Symbol of the corresponding class object, should it exist; otherwise */
/* print an error message and return nil. "block" is possibly nil; if it is */
/* not, it is really a block.						    */
Node qualifier;
Node block;
Boolean query;
{
    Symbol s;

    /* recursively evaluate the qualifier prefix, and then look up the */
    /* last qualifier as a nested scope within the result.		   */

    if (qualifier->op == O_NAME)
    {
	/* 
	 * If block != nil, we have a name of the form
	 *     "<block>.<qualifier>::X1::...::Xn::m ..."
	 * (where n >= 0) and m is a member. If <block> is a class, the user
	 * the user is overriding the default member resolution by specify-
	 * ing the base class "qualifier" of the class given by "<block>" 
	 * through which to resolve the name. Otherwise, <block> is a 
	 * block.
	 *
	 * If block == nil, get the default class/struct/union. "which" 
	 * prints an error message should the name not be defined.
	 */

	Node symNode = nil;
	if (block != nil)
	{
	    /* the user is qualifying a class name with a block. Find the */
	    /* name within that block.			      		  */
	    find(s, qualifier->value.name) where
		s->block == block->nodetype and
		s->class == TAG and 
		(s->isClassTemplate or rtype(s)->class == CLASS)
	    endfind(s);
	    if (s == nil) 
	    {
		if (!query)
		{
		    beginerrmsg();
		    prtree(rpt_error, stderr, block);
		    (*rpt_error)(stderr, 
		         catgets(scmc_catd, MS_resolve, MSG_611, 
		         " does not contain a definition of class \"%1$s\"."),
		         ident(qualifier->value.name));
		    enderrmsg();
		}
		else 
		    return nil;
	    }
	}
	else 
	{
	    symNode = which(qualifier->value.name, WCLASS | WSTRUCT | WUNION);
	    if (symNode == nil)
		return nil;

	    assert(symNode->op == O_SYM);
	    s = symNode->value.sym;
	}
	assert(s != nil);
	if (s->isClassTemplate)
	    symNode = resolveTemplate(s, 0);
	else if (symNode == nil)
	    symNode = build(O_SYM, s);
	return symNode;
    }
    else
    {
	Node outer = qualifier->value.arg[0];
	Node inner = qualifier->value.arg[1];
	AccessList path = nil;
	Node qualNode;

	assert(inner->op == O_NAME);

	s = nil;
	qualNode = findQual(outer, block, query);
	if (qualNode != nil)
	{
	    s = findMember(rtype(qualNode->nodetype), inner->value.name, 
			   &path, WCLASS | WSTRUCT | WUNION);
	}

	freeAccessList(path);
	if (s == nil)
	{
	    if (!query)
	    {
		error(catgets(scmc_catd, MS_resolve, MSG_612, 
		      "\"%1$s\" is not a nested class of \"%2$s\"."),
		      ident(inner->value.name), 
		      outer->op == O_NAME ? 
			  ident(outer->value.name) :
			  ident(outer->value.arg[1]->value.name));
		/*NOTREACHED*/
	    }
	    else
		return nil;
	}
	else
	    return build(O_SYM, s);
    }
}

public Node resolveQual(qualifier, scope, query)
/* Given a qualifier and a block in which to look it up, do that, returning */
/* the Symbol of the corresponding class object, should it exist; otherwise */
/* print an error message and return nil.				    */
Node qualifier;
Node scope;
Boolean query;
{
    Symbol classType;
    Name className;
    Boolean ptrType = false;

    assert(scope != nil);
    classType = rtype(scope->nodetype);
    className = typename(scope->nodetype);
    if (classType->class == CPPREF)
    {
	className = typename(classType);
	classType = rtype(classType->type);
    }
    if (classType->class == PTR)
    {
        ptrType = true;
	className = typename(classType);
	classType = rtype(classType->type);
    }

    if (isblock(classType))
    {
	return findQual(qualifier, scope, false);
    }
    else if (classType->class != CLASS)
    {
	if (!query)
	{
	    error(catgets(scmc_catd, MS_resolve, MSG_613, 
	          "\"%1$s\" is not a class."), ident(scope->nodetype->name));
	    /*NOTREACHED*/
	}
	return nil;
    }
    else
    {
        Node lastQual = findQual(qualifier, nil, false);
	Symbol qualTag = lastQual->value.sym;
        AccessList path;

	assert(lastQual->op == O_SYM);
	if (findBase(classType, qualTag->name, &path) != nil)
	{
	    return buildAccess(path, classType, lvalRecord(scope));
	}
	else if (className == qualTag->name)
	{
	    return scope;
	}
	else if (!query)
	{
	    beginerrmsg();
	    prtree(rpt_error, stderr, scope);
	    (*rpt_error)(stderr, 
		         catgets(scmc_catd, MS_resolve, MSG_624, 
			 "'s type is not \"%1$s\" or derived from \"%2$s\"."),
		         ident(lastQual->value.sym->name),
		         ident(lastQual->value.sym->name));
	    enderrmsg();
	    /*NOTREACHED*/
	}
	return nil;
    }
} 

public Node resolveName(qualifier, member, scope, criteria, query)
/* O_SCOPE */ Node qualifier;
/* O_NAME  */ Node member;
Node scope;
unsigned long criteria;
Boolean query;
{
    Node result;
    Symbol r;

    if (qualifier == nil and scope == nil)
    {
	result = which(member->value.name, criteria | WNIL);

	if (cppModuleSeen && !(criteria & WPARAMCOUNT) &&
	    (result == nil ||
	     (result->nodetype->class == FUNC) ||
	     (result->nodetype->class == MEMBER &&
	      result->nodetype->symvalue.member.type == FUNCM) ||
	     (result->nodetype->class == TAG && (criteria & WMULTI))))

	    /*
	     *  'member->value.name' could be an overloaded function
	     */
  	    if ((result = resolveFns(member->value.name, criteria)) != nil)
		return result;

	if (result == nil)
	{
	    error(catgets(scmc_catd, MS_resolve, MSG_614, 
	          "\"%1$s\" is not defined"), ident(member->value.name));
	    /*NOTREACHED*/
	}
    }
    else if (qualifier == nil /* and scope != nil */)
    {
	result = dot(scope, member, criteria);
    }
    else if (scope == nil /* and qualifier != nil */)
    {
	criteria |= WQUAL;
	result = qualWhich(member->value.name, qualifier, criteria);
    }
    else /* qualifier != nil and scope != nil */
    {
	criteria |= WQUAL;
	result = dot(resolveQual(qualifier, scope, false), member, criteria);
    }

    /* Resolve possibly overloaded C++ template and function names. */
    r = result->nodetype;
    if (r->language == cppLang && r->class == TAG && r->isClassTemplate)
    {
	result = resolveTemplate(r, criteria);
    }
    /* if we got a file instead of a func, check if there's a   */
    /* function with the same name (only when no scope or       */
    /* qualifier is given).                                     */
    else if (r->class == MODULE && qualifier == nil && scope == nil)
    {
        Symbol f;
        find(f, r->name) where isroutine(f) endfind(f);
        if (f != nil)
           result = resolveOverload(build(O_SYM, f), criteria);
    }
    /* Handle C++ overloaded func, and regular dbx functions in */
    /* different modules (libraries) with same name (ambiguous) */
    else if ((r->language == cppLang || isambiguous(r, WFUNC)) 
	     && (isroutine(r) || 
	     r->class == MEMBER && r->symvalue.member.type == FUNCM))
    {
	result = resolveOverload(result, criteria);
    }

    return result;
}

public Symbol passPandR(outsym)
Symbol outsym;
{
    Symbol innersym = rtype(outsym);
 
    if (innersym->class == CPPREF)
        innersym = rtype(innersym->type);
    if (innersym->class == PTR)
        innersym = rtype(innersym->type);
    return innersym;
}

/*
 * This routine is called for casting.  If we are casting a class to a
 * class then we check to see if it is a base class.  If it is then we
 * will print out the base class.  Otherwise, if we are casting to a
 * class (regardless of wheather we are casting a class) we change the
 * nodetype so that the object we are casting will cast left justified
 * instead of right justified.
 */

Node findBaseClass(class, possibleSubClass)
Node class;
Node possibleSubClass;
{
    Node p;
    AccessList path;
    Symbol tempsym;
    Symbol classSym = rtype(class->nodetype);
    Symbol possibleSubClassSym = rtype(possibleSubClass->nodetype);

    if ((class->nodetype->class == LABEL) 
     || (class->nodetype->class == PROC) 
     || (class->nodetype->class == FUNC) 
     || (class->nodetype->language == CSECTFUNC))
    {
      beginerrmsg();
      (*rpt_error)(stderr,  catgets(scmc_catd, MS_symbols, MSG_385,
                                    "operation not defined on \""));
      (*rpt_error)(stderr, "%s", symname(class->nodetype));
      (*rpt_error)(stderr, "\"");
      enderrmsg();
    }

    if ((possibleSubClass->nodetype->class == LABEL) 
     || (possibleSubClass->nodetype->class == PROC) 
     || (possibleSubClass->nodetype->class == FUNC) 
     || (possibleSubClass->nodetype->language == CSECTFUNC))
    {
      beginerrmsg();
      (*rpt_error)(stderr,  catgets(scmc_catd, MS_symbols, MSG_385,
                                    "operation not defined on \""));
                  
      (*rpt_error)(stderr, "%s", symname(possibleSubClass->nodetype));
      (*rpt_error)(stderr, "\"");
      enderrmsg();
    }

    /* if we are not casting a class */
    if (classSym->class != CLASS) 
    {
	/* We are not casting a class, but we are casting to a class, so */
        /* change the nodetype to the left of the TYPERENAME             */

        if (possibleSubClassSym->class == CLASS) 
            class->nodetype = possibleSubClass->nodetype;
        p = class;
    }
    else 
    {
        /* We are casting a class. If we are not casting to a class, do */
	/* nothing. Otherwise, check first to see if we are casting to  */
	/* a base class. If so, we adjust the "this" pointer to point   */
	/* to the base. If not, merely modify the type of the node.     */
        if (possibleSubClassSym->class != CLASS) 
            p = class;
        else 
	{
            if (isBase(classSym, possibleSubClassSym)) 
	    {
                findBase(classSym, possibleSubClass->nodetype->name, &path);
                /* remove extra O_TYPERENAME for multiple castings... */
                if (class->op == O_TYPERENAME) {
                        class->value.arg[0]->nodetype = class->nodetype;
                        class = class->value.arg[0];
                }
                p = build(O_RVAL,
			  buildAccess(path, classSym, lvalRecord(class)));
                p->nodetype = possibleSubClass->nodetype;
            }
            else 
	    {
                class->nodetype = possibleSubClass->nodetype;
                p = class;
            }
        }
    }
    return p;
}

public Node traverse(p, criteria)
Node p;
unsigned long criteria;
{
    Node left, right;
    Symbol basesym;
    AccessList path;
    Node	p0;

    if (p != nil)
    {
	switch (p->op)
	{
	    case O_SETCON: 
		/* nodetype has already been assigned in cons (by ugly hack) */
		break;

	    case O_RVAL:
	    case O_INDIR:
	    case O_INDIRA:
		p0 = traverse(p->value.arg[0], criteria);
		if( p0 != p->value.arg[0] ) {
		    free( p->value.arg[0] );
		    p->value.arg[0] = p0;
		}
		break;

	    case O_CALL:
	    case O_CALLPROC:
	    {
		Symbol s, t;
		Node q, args;
		unsigned short nArgs = 0;

		/* evaluate the parameters first */
		args = traverse(p->value.arg[1], WANY);
		if( args != p->value.arg[1] ) {
		    free( p->value.arg[1] );
		    p->value.arg[1] = args;
		}

		criteria = (WPARAMCOUNT | insertInArgArray(args) | WANY);
		/* evaluate the function name */
		q = traverse(p->value.arg[0], criteria);
		if( q != p->value.arg[0] ) {
		    free( p->value.arg[0] );
		    p->value.arg[0] = q;
		}
		deleteFromArgArray(criteria & WPARAMSLOT);

		if (q->op == O_SYM) 
		{
		    s = q->value.sym;
		    if (s->class == TYPE or s->class == TAG) 
		    {
			p->op = O_TYPERENAME;
			p->value.arg[0] = p->value.arg[1];
			p->value.arg[1] = q;
			q = p->value.arg[0];
			if (q == nil)
			{
	    	            error(catgets(scmc_catd, MS_resolve, MSG_615,
			          "Too few arguments to type rename."));
			}
			if (q->value.arg[1] != nil) 
			{
	    	            error(catgets(scmc_catd, MS_resolve, MSG_616,
			          "Too many arguments to type rename."));
			}
			p0 = findBaseClass(q->value.arg[0], p->value.arg[1]);
			free( p->value.arg[0] );
			p->value.arg[0] = p0;
		    } 
		    else if (s->class == MODULE) 
		    {
			for (t = lookup(s->name); t != nil; t = t->next_sym) 
			{
			    if (t->block == s and isroutine(t)) 
			    {
				q->value.sym = t;
				q->nodetype = t;
				break;
			    }
			}
		    }
		}
		break;
	    }

	    case O_ADDR:
		return amper(traverse(p->value.arg[0], criteria));

	    case O_DOT:
	    {
		Node context, qualifier, name;

		context = traverse(p->value.arg[0], (criteria & ~WALL) | WANY);

                /* Remove O_TYPERENAME for dot operator after casting */
                if (context->op == O_TYPERENAME) {
                   context->value.arg[0]->nodetype = context->nodetype;
                   context = context->value.arg[0];
                } else if (context->op == O_RVAL &&
                           context->value.arg[0]->op == O_TYPERENAME) {
                   context->value.arg[0]->value.arg[0]->nodetype =
                                                        context->nodetype;
                   context = context->value.arg[0]->value.arg[0];
                }

		if (p->value.arg[1]->op == O_UNRES)
		{
		    qualifier = p->value.arg[1]->value.arg[0];
		    name = p->value.arg[1]->value.arg[1];
		}
		else
		{
		    assert(p->value.arg[1]->op == O_NAME);

		    qualifier = nil;
		    name = p->value.arg[1];
		}
		return resolveName(qualifier, name, context, criteria, false);
	    }

            case O_DOTSTAR:
            {
		Symbol t;

                left = traverse(p->value.arg[0], criteria);
                if( left != p->value.arg[0] ) {
		    free( p->value.arg[0] );
		    p->value.arg[0] = left;
		}
                right = traverse(p->value.arg[1], criteria);
                if( right != p->value.arg[1] ) {
		    free( p->value.arg[1] );
		    p->value.arg[1] = right;
		}

                if (right->op == O_RVAL)
                    right = right->value.arg[0];
		if (right->op == O_CPPREF)
                    right = right->value.arg[0];

                /* if the variable to the right of the .* or ->* is not a */
                /* pointer to a member then we have an error.             */
                t = rtype(right->nodetype);
		if (t->class == CPPREF)
		    t = t->type;

                if (t->class != PTRTOMEM)
                {
                    beginerrmsg();
		    (*rpt_error)(stderr, "\"");
		    prtree(rpt_error, stderr, right);
		    (*rpt_error)(stderr, 
				 catgets(scmc_catd, MS_resolve, MSG_625, 
		                 "\" is not a pointer to a member."));
                    enderrmsg();
                }

                /* if the left node is not a class then flag an error     */
                if (passPandR(left->nodetype)->class != CLASS)
                {
                    beginerrmsg();
                    (*rpt_error)(stderr, "\"");
                    prtree(rpt_error, stderr, left);
		    (*rpt_error)(stderr, 
				 catgets(scmc_catd, MS_resolve, MSG_628, 
		                 "\" is not a class."));
                    enderrmsg();
                }

                /* find the class that the pointer to member points to  */
                basesym = t->symvalue.ptrtomem.memType;
  
                /* if the left symbol is not the same class pointed to by */
		/* the pointer to member then build the nodes to access   */
		/* the base class                                         */
                if (passPandR(left->nodetype) != rtype(basesym)) 
		{
                    /* make sure that the ptr to mem class type is a base */
		    /* of the class to left of .*.                        */
                    if (isBase(passPandR(rtype(left->nodetype)),rtype(basesym)))
		    {
                        /* find the path to the base */
                        findBase(passPandR(left->nodetype), 
                                 forward(basesym)->name, &path);
                        /* build the tree structure to access the base class */
                        p->value.arg[0] = buildAccess(path, 
                                 passPandR(left->nodetype), lvalRecord(left));
		    }
                    else 
		    {
			beginerrmsg();
			(*rpt_error)(stderr, "\"");
			prtree(rpt_error, stderr, right);
			(*rpt_error)(stderr,
				     catgets(scmc_catd, MS_resolve, MSG_626, 
				     "\" does not point to member of class \"%1$s\"."),
			             ident(forward(left->nodetype->type)->name));
                        enderrmsg();
                    }
                }
                else 
		{
                    p->value.arg[0] = lvalRecord(p->value.arg[0]);
                }
                return dotptr(p->value.arg[0], p->value.arg[1]);
            }

	    case O_UNRVAL:
		return unrval(traverse(p->value.arg[0], criteria));

            case O_INDEX_OR_CALL:
            {
                Node p1 = traverse(p->value.arg[1], criteria);

		/* in case C++ function call being performed */
		criteria = (WPARAMCOUNT | insertInArgArray(p1) | WANY);
                p0 = traverse(p->value.arg[0], criteria);
		deleteFromArgArray(criteria & WPARAMSLOT);

                p0 = isarray(p0) ? subscript(p0, p1) : build(O_CALL, p0, p1);

		return p0;
            }

	    case O_INDEX:
	    {
		Node p1 = p->value.arg[1];

		p0 = traverse(p->value.arg[0], criteria);

		if (rtype(p0->nodetype)->class == CPPREF)
		    p0 = build(O_CPPREF, p0);

		return subscript(p0, traverse(p1, criteria));
	    }

	    case O_PADD:
		p0 = traverse(p->value.arg[0], criteria);
		if( p0 != p->value.arg[0] ) {
		    free( p->value.arg[0] );
		    p->value.arg[0] = p0;
		}
		p0 = traverse(p->value.arg[1], criteria);
		if( p0 != p->value.arg[1] ) {
		    free( p->value.arg[1] );
		    p->value.arg[1] = p0;
		}
		return build_add_sub(O_ADD, p->value.arg[0], p->value.arg[1]);

	    case O_PSUB:
		p0 = traverse(p->value.arg[0], criteria);
		if( p0 != p->value.arg[0] ) {
		    free( p->value.arg[0] );
		    p->value.arg[0] = p0;
		}
		p0 = traverse(p->value.arg[1], criteria);
		if( p0 != p->value.arg[1] ) {
		    free( p->value.arg[1] );
		    p->value.arg[1] = p0;
		}
		return build_add_sub(O_SUB, p->value.arg[0], p->value.arg[1]);

	    case O_BSET:
		return buildSet(traverse(p->value.arg[0], criteria));

	    case O_ADDEVENT:
	    case O_TRACEON:
	    case O_IF:
		assert(false);
		return nil;

	    case O_ONCE:
	    {
		ListItem i = list_head(p->value.trace.actions);
		while (i != nil)
		{    
		    i->element = (ListElement)
				     traverse(list_element(Node, i), WANY);
		    i = list_next(i);
		}
		p->value.trace.cond = traverse(p->value.trace.cond, WANY);
		break;
	    }

	    case O_EXAMINE:
	        p->value.examine.beginaddr = 
		    traverse(p->value.examine.beginaddr, WANY);
		p->value.examine.endaddr = 
		    traverse(p->value.examine.endaddr, WANY);
		break;

	    case O_WHATIS:
	    {
		criteria = ((unsigned long)p->value.arg[1]) | WALL;
		p0 = traverse(p->value.arg[0], criteria);
		if( p0 != p->value.arg[0] ) {
		    free( p->value.arg[0] );
		    p->value.arg[0] = p0;
		}
		break;
	    }

	    case O_FREE:
	    {
		Node nameNode = traverse(p->value.arg[0], criteria);
		Name name = nameNode->value.name;
		p0 = p;
		if ((criteria & WSEARCH) != WANY || (p0 = findvar(name)) == nil)
		    p0 = resolveName(nil, nameNode, nil, criteria, false);

		if( p0->op == O_SYM ) {
		    touch_sym( p0->value.sym );
		}
		if (rtype(p0->nodetype)->class == CPPREF)
		    p0 = build(O_CPPREF, p0);
		return p0;
	    }

	    case O_UNRES:
	    {
		/* given a qualifier and a member name, find the member */
		/* symbol. 						*/
		Node qualifier = p->value.arg[0];
		Node member = p->value.arg[1];
		p0 = resolveName(qualifier, member, nil, criteria, false);

		if (rtype(p0->nodetype)->class == CPPREF)
		    p0 = build(O_CPPREF, p0);
		return p0;
	    }

            case O_REFRENAME:
              {
                Boolean reffound = false;
                Node s;
                p->value.arg[0] = traverse(p->value.arg[0], WANY);

                s = p->value.arg[0];
                /*  if casting a reference to another reference  */
                do 
                {
                  if (s->value.arg[0]->op == O_CPPREF)
                  {
                    s->value.arg[0] = s->value.arg[0]->value.arg[0];
                    reffound = true;
                  }
                  s = s->value.arg[0];
                }
                while ((s != NULL) && !reffound);

                s = p->value.arg[0];
                p->value.arg[1] = traverse(p->value.arg[1], WTYPE);
                p->value.arg[1]->nodetype =  
                   newSymbol(nil, 0, CPPREF, p->value.arg[1]->nodetype, nil);
                p->value.arg[1]->nodetype =  
                   newSymbol(nil, 0, VAR, p->value.arg[1]->nodetype, nil);
                return build(O_TYPERENAME, p->value.arg[0], p->value.arg[1]);
              }

	    case O_PTRRENAME:
		p0 = traverse(p->value.arg[0], WANY);
		if( p0 != p->value.arg[0] ) {
		    free( p->value.arg[0] );
		    p->value.arg[0] = p0;
		}
		p0 = traverse(p->value.arg[1], WTYPE);
		if( p0 != p->value.arg[1] ) {
		    free( p->value.arg[1] );
		    p->value.arg[1] = p0;
		}

		return ptrrename(p->value.arg[0], p->value.arg[1]);

            case O_TYPERENAME:
            {
                p0 = traverse(p->value.arg[0], WANY);
                if( p0 != p->value.arg[0] ) {
		    free( p->value.arg[0] );
		    p->value.arg[0] = p0;
		}
                p0 = traverse(p->value.arg[1], WTYPE);
                if( p0 != p->value.arg[1] ) {
		    free( p->value.arg[1] );
		    p->value.arg[1] = p0;
		}
		     
                p->value.arg[0] = findBaseClass(p->value.arg[0],
						p->value.arg[1]);
                break;
            }

	    case O_SCOPE:
	    {
		Node context, qualifier, name;
		Symbol m;

		/* O_SCOPE can only be reached directly when considering */
		/* C++ module scope.					 */
		assert(p->value.arg[0] == nil);

		/* find the current module */
		m = curfunc;
		while (m->class != MODULE)
		    m = m->block;
		assert(m->class == MODULE);
		if (m->language != cppLang)
		    error(catgets(scmc_catd, MS_resolve, MSG_617,
			  "The current module is not a C++ module."));
	 	context = traverse(build(O_SYM, m), WOTHER);

		if (p->value.arg[1]->op == O_UNRES)
		{
		    qualifier = p->value.arg[1]->value.arg[0];
		    name = p->value.arg[1]->value.arg[1];
		}
		else
		{
		    assert(p->value.arg[1]->op == O_NAME);

		    qualifier = nil;
		    name = p->value.arg[1];
		}
		p0 = resolveName(qualifier, name, context, criteria, false);

		/* remove the RVAL supplied by '.', as one is already here */
		if (p0->op == O_RVAL)
		    p0 = p0->value.arg[0];

		return p0;
	    }

            case O_NAMELIST:
                /* O_NAMELIST nodes are already complete. */
                break;

	    case O_STOP:
	    case O_STOPI:
	    case O_TRACE:
	    case O_TRACEI:
		/* all these commands have three arguments, the first of    */
		/* which is an expression denoting the stop/trace location. */

                /*  trace and tracei are set up to handle multiple
                      selections for the expression, but stop and
                      stopi are not  */
                criteria = ((p->op == O_TRACE) || (p->op == O_TRACEI))
                                  ? (WMULTI | WANY) : WANY;
                p0 = traverse(p->value.arg[0], criteria);
                if( p0 != p->value.arg[0] ) {
		    free( p->value.arg[0] );
		    p->value.arg[0] = p0;
		}

                /*  only allow multiple selections for cases where the
                      location is form "in funcname".  Do not allow
                      multiple selections for "at lineno" or 
                      "at &funcname".  */

                criteria = WANY;
                if (p->value.arg[1]->op == O_UNRVAL)
                {
                  p0 = p->value.arg[1]->value.arg[0];
                  if ((p0->op == O_RVAL) || (p0->op == O_DOT)
                   || (p0->op == O_SYM))
                  criteria = WMULTI | WANY;
                }
                p0 = traverse(p->value.arg[1], criteria);

                if( p0 != p->value.arg[1] ) {
		    free( p->value.arg[1] );
		    p->value.arg[1] = p0;
		}
                p0 = traverse(p->value.arg[2], WANY);
                if( p0 != p->value.arg[2] ) {
		    free( p->value.arg[2] );
		    p->value.arg[2] = p0;
		}
		break;

	    case O_WHICH:
                p0 = traverse(p->value.arg[0], WALL | WTMPLALL | WANY);
                if( p0 != p->value.arg[0] ) {
		    free( p->value.arg[0] );
		    p->value.arg[0] = p0;
		}
		break;

	    case O_SYMTYPE:
                p0 = traverse(p->value.arg[0], WALL | WTMPLALL | WANY);
                if( p0 != p->value.arg[0] ) {
		    free( p->value.arg[0] );
		    p->value.arg[0] = p0;
		}
		break;

	    default:
	    {
		int i;
		short	numb_args;
		criteria |= WANY;
		numb_args = nargs(p->op);
		for (i = 0; i < numb_args; i++) {
		    p0 = traverse(p->value.arg[i], criteria);
		    if( p0 != p->value.arg[i] ) {
			free( p->value.arg[i] );
			p->value.arg[i] = p0;
		    }
		}
	    }
	}

	/* assign the types of the nodes */
	switch(p->op) {
	    case O_LCON:      p->nodetype = t_int; 	 break;
	    case O_ULCON:     p->nodetype = dt_uint;   	 break;
	    case O_LLCON:     p->nodetype = t_longlong;	 break;
	    case O_ULLCON:    p->nodetype = t_ulonglong; break;
	    case O_CCON:      p->nodetype = t_char; 	 break;
	    case O_FCON:      p->nodetype = t_real; 	 break;
	    case O_QCON:      p->nodetype = t_quad; 	 break;
	    case O_KCON:      p->nodetype = t_complex; 	 break;
	    case O_QKCON:     p->nodetype = t_qcomplex;  break;
	    case O_SETCON: /* p->nodetype already set */ break;
	    default:
		check(p);
		assigntypes(p);
	}
    }

    return p;
}
