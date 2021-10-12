static char sccsid[] = "@(#)65	1.1.2.15  src/bos/usr/ccs/lib/libdbx/overload.c, libdbx, bos41J, 9510A_all 2/20/95 12:41:24";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: resolveOverload, insertInArgArray, deleteFromArgArray
 *	      strwhich, getResolvedChoices, resolveFns
 *            resolveFilename
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
#define isfunc(s) ((s->class == FUNC) || (s->class == CSECTFUNC))
 
#include <setjmp.h>
#include "defs.h"
#include "symbols.h"
#include "resolve.h"
#include "cplusplus.h"
#include "names.h"
#include "overload.h"
#include "eval.h"
#include "mappings.h"
#include "object.h"
 
static char	*symstr;

/*
 * C++ overloaded functions.
 */
 
private Boolean args_match(proc, thisGiven, arglist, compGenParmsSeen)
Symbol proc;
Boolean thisGiven;
Node arglist;
unsigned *compGenParmsSeen;
{
    Symbol formal;
    Node actual;
 
    if (proc->class == MEMBER /* pure virtual call */)
	return thisGiven;

    if (!thisGiven && formal->name == this)
	return false;

    /* can't match args if func has no symbolic info */
    if (proc->symvalue.funcv.src == false)
        return true;

    if (varIsSet("$unsafecall"))
	return true;

    formal = proc->chain;
    actual = arglist;
 
    if (formal->name == this)
        formal = formal->chain;
 
    for (; actual != nil && formal != nil; 
	   actual = actual->value.arg[1], formal = formal->chain) 
    {
	if (actual->op == O_COMMA)
	{
	    Node a = actual->value.arg[0];
 
	    /* try to prevent the user from supplying type and tag names */
	    /* as parameter names.					 */
	    if (a->op == O_RVAL && a->value.arg[0]->op == O_SYM &&
		(a->value.arg[0]->value.sym->class == TYPE ||
		 a->value.arg[0]->value.sym->class == TAG))
	    {
		error(catgets(scmc_catd, MS_overload, MSG_603,
		      "Unexpected type name as function argument."));
	    }
	    else if (formal->type->class == ELLIPSES)
		return true;
	    else if (cpp_tempname(formal->name))
	    {
		*compGenParmsSeen += 1;
		return false;
	    }
	    else if (!compatible(formal->type, a->nodetype))
	        return false;
        }
        else
            return false;
    }
  
    if (actual != nil)
        return false;
    else if (formal != nil)
    {
	if (formal->type->class == ELLIPSES)
	    return true;
	do
	{
	    if (cpp_tempname(formal->name) && formal->type->class != ELLIPSES)
		*compGenParmsSeen += 1;
	    formal = formal->chain;
	}
	while (formal != nil);
	return false;
    }
  
    return true;
}
 
private char buffer[255];
 
#define MAXOVERLOAD 512
private Boolean use[MAXOVERLOAD];
private Boolean useBuffer[MAXOVERLOAD];
 
private void resetUseList(use)
Boolean *use;
{
    (void)memset(use, 0, sizeof(Boolean) * MAXOVERLOAD);
}
 
private void mergeUseLists(use1, use2, nMatches)
Boolean *use1, *use2;
unsigned int nMatches;
{
    int i;
    for (i = 0; i < nMatches; i++)
	use1[i] = use1[i] || use2[i];
}
 
private Boolean noChoices(nMatches)
unsigned int nMatches;
{
    int i;
    for (i = 0; i < nMatches; i++)
	if (use[i])
	    return false;
    return true;
}
 
private cppSymList makeChoices(list, nMatches)
cppSymList list;
unsigned int nMatches;
{
    cppSymList d, e = list;
    cppSymList lastE = nil;
    unsigned int i = 0;
 
    while (i < nMatches)
    {
	if (!use[i])
	{
	    if (lastE == nil)
		list = e->next;
	    else
		lastE->next = e->next;
	    d = e;
	    e = e->next;
	    free(d);
	}
	else
	{
	    lastE = e;
	    e = e->next;
	}
	i += 1;
    }
    return list;
}
    
enum ResponseKind { rnone, rforward, rbackward, rend, rchoice, rall };
 
private enum ResponseKind getResponse(multi, lastChoice, atBegin, atEnd,
				      windowing, nMatches)
Boolean multi, atEnd, atBegin, windowing;
unsigned int lastChoice, nMatches;
{
    extern File in;
    unsigned short lastNumber, rangeBegin;
    unsigned number, i;
    enum ResponseKind responseKind;
    unsigned short nChoices; 
    char listLengthBuffer[10];
 
    if (windowing)
	(void)sprintf(listLengthBuffer, "{%d} ", nMatches);
    else
	listLengthBuffer[0] = '\0';
 
    while (true)
    {
 	responseKind = rnone;
        resetUseList(useBuffer);
 	lastNumber = 0;
	rangeBegin = 0;
	nChoices = 0;
 
	(*rpt_output)(stdout, "%sSelect one %sof [1 - %d]%s%s%s%s%s: ", 
		      listLengthBuffer, 
		      multi ? "or more " : "", lastChoice,
		      (!atEnd || !atBegin) ? " or " : "",
		      !atEnd ? "{[f]}orward" : "",
		      (!atEnd && !atBegin) ? ", " : "",
		      !atBegin ? "[b]ackward" : "",
		      windowing ? " or [e]nd" : "");
	fflush(stdout);
 
        fgets(buffer, 255, in);
	i = 0;
 	while (i < strlen(buffer))
	{
 	    char c = buffer[i];
	    if (isdigit(c))
	    {
		if (responseKind != rchoice && responseKind != rnone &&
		    responseKind != rall)
		    break;
 
		number = c - '0';
		while (isdigit(buffer[++i]))
		    number = number * 10 + (buffer[i] - '0');
		if (number == 0 || number > lastChoice)
		    break;
		if (rangeBegin > 0)
		{
		    int j;
 
		    if (number < rangeBegin)
		    {
			int temp = rangeBegin - 1;
			rangeBegin = number - 1;
			number = temp;
		    }
		    for (j = rangeBegin + 1; j <= number; j++)
			if (!useBuffer[j - 1])
			{
			    useBuffer[j - 1] = true;
			    nChoices += 1;
			}
		    rangeBegin = 0;
		}
		else
		{
		    if (!useBuffer[number - 1])
		    {
		        useBuffer[number - 1] = true;
		        nChoices += 1;
		    }
		}
		lastNumber = number;
 
		if (responseKind != rall)
		    responseKind = rchoice;
	    }
	    else if (c == '\n')
	    {
		if (!atEnd && responseKind == rnone)
		    responseKind = rforward;
 
		if (((responseKind == rchoice || responseKind == rall) &&
		     (rangeBegin != 0) || (!multi && nChoices > 1)) ||
  		    responseKind == rnone)
		    break;
 
		return responseKind;
	    }
	    else if (c == 'F' || c == 'f')
	    {
		if (atEnd || responseKind != rnone)
		    break;
 
		responseKind = rforward;
		i += 1;
	    }
	    else if (c == 'B' || c == 'b')
	    {
		if (atBegin || responseKind != rnone)
		    break;
 
		responseKind = rbackward;
		i += 1;
	    }
	    else if (c == 'E' || c == 'e')
	    {
		if (!windowing || responseKind != rnone)
		    break;
 
		responseKind = rend;
		i += 1;
	    }
	    else if (c == '-')
	    {
		if (lastNumber == 0)
		    break;
 
		rangeBegin = lastNumber;
		i += 1;
	    }
	    else if (c == ' ' || c == ',')
		i += 1;
	    else if (c == '*')
	    {
		if ((responseKind != rnone && responseKind != rchoice &&
		     responseKind != rall) || !multi)
		    break;
 
		responseKind = rall;
		i += 1;
	    }
	    else 
		break;
	}
    }
}
 
public cppSymList getChoices(list, nMatches, multi)
cppSymList list;
unsigned int nMatches;
Boolean multi;
{
    extern File in;
    static windowVar = nil;
    Node windowNode;
 
    cppSymList e = list;
    cppSymList *startArray;
 
    unsigned int nWindows;
    int winHeight = nMatches;
    unsigned int window = 0;
    unsigned int lastChoice = 0;
 
    assert(nMatches > 1);

    if (windowVar == nil)
        windowVar = identname("$menuwindow", true);
    windowNode = findvar(windowVar);

    if (windowNode != nil)
    {
	eval(windowNode);
	winHeight = pop(integer);
    }
    if (windowNode == nil || winHeight < 2)  
	winHeight = 10;
 
    nWindows = (nMatches + winHeight - 1) / winHeight;
    startArray = (cppSymList *)malloc(sizeof(cppSymList) * nWindows);
 
startChoiceProcess:
    resetUseList(use);
 
    while (window < nWindows)
    {
	unsigned int menuStart = winHeight * window + 1;
	unsigned int menuEnd = winHeight * (window + 1);
	enum ResponseKind result;
	int i;

	if (menuEnd > nMatches)
	    menuEnd = nMatches;
        if (lastChoice < menuEnd)
	    lastChoice = menuEnd;
 
        startArray[window] = e;
	for (i = menuStart; i <= menuEnd; i++)
	{
	    static char nameBuf[1024];
	    static char idBuf[10];
 
	    (*rpt_output)(stdout, "%d. ", i);
            if (e->file_list)
              (*rpt_output)(stdout, "%s", e->filename);
            else
	      printwhich(rpt_output, stdout, e->sym, true);
	    (*rpt_output)(stdout, "\n");
	    e = e->next;
	}
 
	result = getResponse(multi, lastChoice, window == 0, 
			     window == nWindows - 1, nWindows > 1, nMatches);
 
	mergeUseLists(use, useBuffer, nMatches);
 
	switch (result)
	{
	    case rchoice:
		if (multi)
		{
		    window += 1;
		    break;
		}
		/* else end */
 
	    case rend:
		e = startArray[window];
		goto terminate;
 
	    case rforward:
		window += 1;
		break;
 
	    case rbackward:
		window -= 1;
		e = startArray[window];
		break;
 
	    case rall:
		free(startArray);
		return list;
	}
    }
 
terminate:
    if (noChoices(nMatches))
	goto startChoiceProcess;
 
    list = makeChoices(list, nMatches);
 
    free(startArray);
    return list;
}


/*
 * NAME: strwhich
 *
 * FUNCTION: Collects output from printwhich() into the
 *	     static global 'symstr';
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * PARAMETERS:
 *	f -		Unused since we are collecting output into a string
 *	format -	Printf style format string
 *	s		String argument to print
 *
 * RECOVERY OPERATION: None
 *
 * DATA STRUCTURES: This function is called many times from within
 *		    printwhich().  Output is appended to the static
 *		    global 'symstr'
 *
 * TESTING: 
 *	Will be tested during testing of the resolve dialog in softdb.
 *
 * RETURNS:
 *	Nothing
 */
private int strwhich(f, format, s)
File	f;
char	*format;
char	*s;
{
    int	len, slen;

    len = strlen(format) + strlen(s);

    if (symstr == NULL)
    {
	symstr = malloc(len * sizeof(char));
	sprintf(symstr, format, s);
    }
    else
    {
	slen = strlen(symstr);
	symstr = realloc(symstr, (slen+len) * sizeof(char));
	sprintf(symstr+slen, format, s);
    }
}


/*
 * NAME: getResolvedChoices
 *
 * FUNCTION: Invokes the correct function to allow a user to select
 *	     from a list of overloaded names.
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * PARAMETERS:
 *	list -		list of overloaded names the user can choose from
 *	nMatches	Number of overloaded names
 *	multi -		True means multiple selections else single selection
 *
 * RECOVERY OPERATION: None
 *
 * DATA STRUCTURES:
 *
 * TESTING: 
 *	Issue command involving overloaded function.
 *
 * RETURNS:
 *	list of overloaded names selected
 */
private cppSymList getResolvedChoices(list, nMatches, multi)
cppSymList	list;
unsigned int	nMatches;
Boolean		multi;
{
    extern boolean	isXDE;
    extern char		scanner_linebuf[];
    extern void		(*resolved_choices)();
    cppSymList		l;
    unsigned int	i;
    char		**clist;

    if (isXDE && resolved_choices != NULL)
    {
	clist = (char **)malloc(nMatches * sizeof(char *));
	for (i = 0, l = list;  i < nMatches;  i++, l=l->next)
	{
	    symstr = NULL;
            if (l->file_list)
              (*strwhich)((File)NULL, "%s", l->filename);
            else
	      printwhich(strwhich, (File)NULL, l->sym, true);
	    clist[i] = symstr;
	}

	(*resolved_choices)(scanner_linebuf, clist, nMatches, multi, use);
	list = makeChoices(list, nMatches);

	for (i = 0;  i < nMatches;  i++)
	    free(clist[i]);
	free(clist);
    }
    else
    {
      /*  if reading from standard input 
            and a single selection is needed or this is C++ */
      if (isstdin() && (!multi || (curlang == cppLang))) 
    	list = getChoices(list, nMatches, multi);
      else
      {
        /*  not reading from standard input 
              or something other than C++ and multiple selections OK */
 
        /*  if the list is not empty and only one element can
              be returned  */
        /*  NOTE : if more than one element can be selected,
              the entire list will be returned  */
        if ((list != NULL) && !multi)
        {
          /*  return the first element  */
          /*  do it this way to make sure memory gets freed properly  */
          resetUseList(use);
          use[0] = true;
          makeChoices(list, nMatches);
        }
      }
    }

    if (list == nil)
	erecover();

    return list;
}


#define ARGARRAYSIZE 16
private Symbol argArray[16];

public int insertInArgArray(args)
Symbol args;
{
    /* find a slot in the argument array */
    int slot;
    for (slot = 0; slot < ARGARRAYSIZE && argArray[slot] != nil; slot++);
    if (slot == ARGARRAYSIZE)
	error(catgets(scmc_catd, MS_overload, MSG_606,
	      "Too many function calls in expression."));
    argArray[slot] = args;
    return slot;
}

public void deleteFromArgArray(slot)
int slot;
{
    argArray[slot] = nil;
}

private Boolean overloadPair(s, t)
/* This function returns true when FUNC or (function) MEMBER symbols s    */
/* and t are overload pairs. This happens when:			          */
/* 1. They are member functions with the same name from the same class.   */
/*    Two classes are the same if their TAG names are the same and their  */
/*    types are the same. (block equivalency isn't good enough).          */
/* 2. They are non-member functions at global scope with the same name.   */
/*    Functions in modules compiled with debug will have MODULE scope,    */
/*    while other functions will have PROG scope. Both must be considered */
/*    in global scope.							  */
/* 3. For regular C func with same name from different libraries fix.     */
/*    Return true if the symbols are the same.                            */
Symbol s, t;
{
    Symbol sb = s->block;
    Symbol tb = t->block;

    assert(s->name == t->name);
    return s == t || sb->class == TAG && sb->type == tb->type ||
	   ((sb->class == MODULE || sb->class == PROG) && 
	    (tb->class == MODULE || tb->class == PROG));
}


private Symbol firstGlobalCandidate(s)
/* Find the first FUNC Symbol in the Symbol chain with the same name as "s". */
/* We must hash the symbol's name and start at the beginning of the hash     */
/* chain.                                        			     */
Symbol s;
{
    Symbol block = s->block;
    Name name = s->name;
    Symbol t;

    assert(isroutine(s));
    find(t, name) where isroutine(t) and overloadPair(s, t) endfind(t);

    assert(t != nil);
    return t;
}

private Symbol nextGlobalCandidate(s)
Symbol s;
{
    Symbol f = s;

    /* find the next function along the hash chain that has the same name */
    /* as "f". 								  */
    assert(isroutine(s));
    s = s->next_sym;
    while (s != nil)
    {
        if (s->name == f->name && isroutine(s) && overloadPair(s, f))
            return s;
        s = s->next_sym;
    }
    return nil;
}

private Symbol nextMemberCandidate(s)
Symbol s;
{
    Symbol m;

    /* find the next function along the hash chain that has the same name */
    /* as "s". Recall that s may be either a MEMBER or a FUNC.            */
    if (isfunc(s))
    {
        m = s->symvalue.funcv.u.memFuncSym;
	/* If "m" is an inline function, search for alternative inline */
	/* function definitions. 				       */
 	if (m->symvalue.member.attrs.func.isInline)
	{
	    Symbol i = s->next_sym;
	    while (i != nil)
	    {
		if (s->class == i->class && overloadPair(s, i) &&
		    i->symvalue.funcv.u.memFuncSym == 
		    s->symvalue.funcv.u.memFuncSym &&
		    !s->symvalue.funcv.u.memFuncSym->symvalue.member.isCompGen)
		{
		    return i;
		}
		i = i->next_sym;
	    }
	}
	s = m->next_sym;	   
    }
    else
	s = (m = s)->next_sym;
    while (s != nil)
    {
	if (s->name == m->name && s->class == m->class && overloadPair(s, m)
	    && !s->symvalue.member.isCompGen)
	{
	    if (s->symvalue.member.attrs.func.isVirtual != CPPPUREVIRTUAL)
	    {
	        s = s->symvalue.member.attrs.func.funcSym;
		assert(s != nil);
	    }
	    break;
	}
	s = s->next_sym;
    }
    return s;
}

public Node resolveTemplate(templateTag, criteria)
Symbol templateTag;
unsigned long criteria;
{
    TemplateClassListEntry s;
    cppSymList list = nil;
    unsigned int nMatches = 0;
    Symbol classSym;

    s = templateTag->symvalue.template.list;
    while (s != nil)
    {
	cppSymList e = new(cppSymList);
	e->sym = s->templateClass;
        e->file_list = false;
	e->next = list;
	list = e;

	nMatches += 1;
	s = s->next;
    }

    assert(nMatches > 0);

    if (criteria & WTMPLALL)
    {
	if (nMatches > 1)
	{
	    classSym = new(Symbol);
	    classSym->class = CPPSYMLIST;
	    classSym->symvalue.sList = list;
	    /*
	     * Clear the block so if touch_sym() is called it will not fail.
	     */
	    classSym->block = NULL;
	    return build(O_SYM, classSym);
	}
    }
    else if (nMatches > 1)
    {
	list = getResolvedChoices(list, nMatches, false);
	assert(list->next == nil);
    }

    classSym = list->sym;
    free(list);

    return build(O_SYM, classSym);
}

public Node resolveOverload(funcNode, criteria)
/* funcNode represents either a FUNC or a MEMBER of type FUNC in the */
/* case of pure virtuals.					     */
Node funcNode;
unsigned long criteria;
{
    Symbol funcSym, s;
    unsigned nSeen, nMatches, slot, compGenParmsSeen;
    Boolean argsAvailable, thisPresent;
    Node dotNode, p;
    cppSymList list;

    p = funcNode;
    if (p->op == O_RVAL)
	p = p->value.arg[0];
    if (p->op == O_DOT)
	dotNode = p;
    else
    {
	assert(p->op == O_SYM);
	dotNode = nil;
    }

    if (argsAvailable = (criteria & WPARAMCOUNT))
    {
        slot = criteria & WPARAMSLOT;

	/* determine if, given that the function is introduced via a ".", */
	/* that the information to the left of the dot is in fact a real  */
	/* class instance, and not just a class (i.e. x.f vs. X::f) so    */
	/* we know if a "this" pointer has been supplied. If not, we will */
	/* resolve only to static member functions. Otherwise, we may     */
	/* resolve to both (remember that f may be static and called via  */
	/* "x.f".							  */
	if (dotNode->op == O_DOT && dotNode->value.arg[0]->op == O_SYM &&
	    dotNode->value.arg[0]->value.sym->class == TAG)
	{
	    thisPresent = false;
	}
	else
	    thisPresent = true;
    }

    s = funcSym = funcNode->nodetype;
    nSeen = nMatches = 0;
    compGenParmsSeen = 0;
    list = nil;

    if (s->class == MEMBER)
    {
        if (s->symvalue.member.attrs.func.isVirtual != CPPPUREVIRTUAL)
        {
	    s = s->symvalue.member.attrs.func.funcSym;
	    assert(s != nil);
        }

	while (s != nil)
	{
	    nSeen += 1;
	    if (!argsAvailable || 
		args_match(s, thisPresent, argArray[slot], &compGenParmsSeen))
	    {
		cppSymList e = new(cppSymList);
		e->sym = s;
                e->file_list = false;
		e->next = list;
		list = e;

		nMatches += 1;
	    }
	    s = nextMemberCandidate(s);
	}
    }
    else /* s is a global function */
    {
	s = firstGlobalCandidate(s);
	while (s != nil)
	{
	    nSeen += 1;
	    if (!argsAvailable ||
		args_match(s, thisPresent, argArray[slot], &compGenParmsSeen))
	    {
		cppSymList e = new(cppSymList);
		e->sym = s;
                e->file_list = false;
		e->next = list;
		list = e;

		nMatches += 1;
	    }
	    s = nextGlobalCandidate(s);
	}
    }

    if (nMatches == 0)
    {
	/* This can only happen because the arguments failed to match. */
	if (compGenParmsSeen)
	    warning(catgets(scmc_catd, MS_overload, MSG_607,
		    "Function(s) contain compiler generated parameters."));
        deleteFromArgArray(slot);
	error(catgets(scmc_catd, MS_overload, MSG_608,
	      "Given argument list fails to match any definition of %1$s."),
	      symname(funcSym->class == MEMBER && 
	              funcSym->symvalue.member.attrs.func.funcSym != nil ?
	              funcSym->symvalue.member.attrs.func.funcSym : funcSym));
    }

    if ((criteria & WMULTI) | (criteria & WALL))
    {
        if (nMatches > 1 && (criteria & WMULTI))
	    list = getResolvedChoices(list, nMatches, true);
	
        if (list->next == nil) /* implies one function was selected */
        {
	    funcSym = list->sym;
	    free(list);
	}
	else
	{
	    funcSym = new(Symbol);
	    funcSym->class = CPPSYMLIST;
	    funcSym->symvalue.sList = list;
	    /*
	     * Clear the block so if touch_sym() is called it will not fail.
	     */
	    funcSym->block = NULL;
	}

	if (dotNode != nil)
	{
	    dotNode->value.arg[1] = build(O_SYM, funcSym);
	    funcNode->nodetype = dotNode->nodetype = funcSym;
	}
	else
	    funcNode = build(O_SYM, funcSym);

	return funcNode;
    }
 
    /* We are in "SINGLE" mode. Should the command be a call-type com- */
    /* mand (which can be determined by the presence of arguments) it  */
    /* is possible, should the choice be a virtual member function,    */
    /* that we want to resolve the function to the class that introd-  */
    /* uces the virtual, rather than the normal class selected by the  */
    /* name resolution mechanism. Also, if the function call is to a   */
    /* static member function, we want to throw away the "this"        */
    /* pointer information.					       */
    /* We also must deal with explicitly qualified virtual functions.  */

    if (nMatches > 1)
    {
	list = getResolvedChoices(list, nMatches, false);
	assert(list->next == nil);
    }
    s = list->sym;
    free(list);

    if (s->class == FUNC && s->isMemberFunc || s->class == MEMBER)
    {
	Symbol m;
        if (s->class == FUNC)
	    m = s->symvalue.funcv.u.memFuncSym;
	else if (s->class == MEMBER)
	    m = s;

	if (argsAvailable && m->symvalue.member.isStatic)
	{
	    assert(m->symvalue.member.attrs.func.funcSym != nil);
	    funcNode = build(O_SYM, s);
	}
	else if (argsAvailable && !(criteria & WQUAL) &&
	         m->symvalue.member.attrs.func.isVirtual != CPPREAL)
	{
	    Symbol findVirtualMember();
	    Symbol classType, funcSym;
	    AccessList path = nil;
	    DemangledName dName;

	    assert(dotNode != nil);

	    funcSym = dotNode->value.arg[1]->value.sym;
	    if (funcSym->class == FUNC)
		classType = rtype(dotNode->value.arg[1]->value.sym->
				  symvalue.funcv.u.memFuncSym->block);
	    else /* funcSym->class == MEMBER */
		classType = rtype(dotNode->value.arg[1]->value.sym->block);

	    dName = m->symvalue.member.attrs.func.dName;
	    m = findVirtualMember(classType, dName, &path);
	    assert(m->class == MEMBER && m->symvalue.member.type == FUNCM)

	    funcNode->nodetype = dotNode->nodetype = m;
	    dotNode->value.arg[1] = build(O_SYM, m);

	    cpp_addToVirtualList(dotNode->value.arg[1], classType, path);
	}
	else if (dotNode != nil)
	{
	    funcNode->nodetype = dotNode->nodetype = s;
	    dotNode->value.arg[1] = build(O_SYM, s);
	}
	else
	    funcNode = build(O_SYM, s);
    }
    else /* normal function */
    {
	funcNode = build(O_SYM, s);
    }
    return funcNode;
}


/*
 * NAME: resolveFns
 *
 * FUNCTION: Search the function table for functions matching an
 *	     unqualified name.
 *
 * EXECUTION ENVIRONMENT: normal user process
 *
 * PARAMETERS:
 *	name		Contains function name to search for
 *	criteria	Search criteria
 *
 * RECOVERY OPERATION: None
 *
 * DATA STRUCTURES: Uses globals nfuncs and functab.
 *
 * TESTING: 
 *	Issue command containing unqualifed member function name.
 *
 * RETURNS:
 *	Any functions which matched.
 */
public Node resolveFns(Name name, unsigned long criteria)
{
    extern int		nfuncs;
    extern AddrOfFunc	*functab;

    char		*fname, *s, *s1;
    unsigned		nmatches;
    int			i, len, fname_len;
    cppSymList		list = nil;
    cppSymList		l, p1, p2;
    Symbol		funcsym;

    fname = name->identifier;
    fname_len = strlen(fname);

    /*
     *  Look through the function table for
     *  functions which match 'fname'.
     */
    for (i = 0, nmatches = 0;  i < nfuncs;  i++)
    {
	funcsym = functab[i].func;
	s = funcsym->name->identifier;
	len = strlen(s);
	s += len-fname_len;
	s1 = s - 1;

	if ((len == fname_len ||
	     (len > fname_len && !isalnum(*s1) && *s1 != '_' && *s1 != '~'))
	    &&
	    strcmp(s, fname) == 0)
	{

	    /*
	     *  Found a match - Find the insertion point in the list
	     */
	    for (p1 = nil, p2 = list;  p2 != nil;  p1 = p2, p2 = p2->next)
	    {
		 if (p2->sym == funcsym ||
		     strcmp(p2->sym->name->identifier,
			    funcsym->name->identifier ) > 0)
		     break;
	    }
		 
	    /*
	     *  Add it to the list if it's not already there
	     */
	    if (p2 == nil || p2->sym != funcsym)
	    {
              if (!funcsym->symvalue.funcv.islinkage)
              {
		l = new(cppSymList);
		l->sym = funcsym;
                l->file_list = false;

		if (p1 == nil)
		    list = l;
		else
		    p1->next = l;
		l->next = p2;
		
		nmatches++;
              }
	    }
	}

    }

    if (nmatches == 0)
	return nil;

    if ((criteria & WMULTI) || (criteria & WALL))
    {
        if (nmatches > 1 && (criteria & WMULTI))
	    /* Allow multiple selections */
	    list = getResolvedChoices(list, nmatches, true);
    }
    else
	if (nmatches > 1)
	    /* Single selection only */
	    list = getResolvedChoices(list, nmatches, false);

    if (list->next == nil)
    {
	/* There was only one function */
        funcsym = list->sym;
        free(list);
    }
    else
    {
        funcsym = new(Symbol);
        funcsym->class = CPPSYMLIST;
        funcsym->symvalue.sList = list;
	/*
	 * Clear the block so if touch_sym() is called it will not fail.
	 */
	funcsym->block = NULL;
    }

    return build(O_SYM, funcsym);
}

/*
 * NAME: resolveFilename
 *
 * FUNCTION: Search the file table for filenames matching a
 *           name.  If there is more than one, prompt
 *           the user to pick one.
 *
 * PARAMETERS:
 *      name : the filename
 *      line : line number desired, if known.  zero otherwise.
 *      ftp_pointer : return area for filetab entry
 *                 NULL is passed in if the caller does not
 *                 need this information.
 *      inc_file_pointer : return area for include file entry in filetab
 *                 NULL is passed in if the caller does not
 *                 need this information.
 *      is_ambig : pointer to Boolean flag set in this routine to
 *                 tell the caller if the filename passed in is
 *                 ambiguous.  The caller uses this information fo
 *                 decide if the breakpoint message should contain
 *                 a fullpath.  If NULL, is passed in, this parameter
 *                 is ignored.
 *      return_list : area to return cppSymList containing all
 *                    of the addresses of where to set breakpoints.
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: filetab - scan the filetable to find possible
 *                            filenames.  If more than one possibility,
 *                            prompt.
 *                  use struct cppSymList to hold list of file table
 *                  pointers
 *
 * RETURNS: filename as known in filetab 
 *          NULL if name cannot be resolved in file table
 */
String resolveFilename(name, line, file_found,
                       line_addr_pointer, is_ambig, return_list)
char *name;
Lineno line;
Boolean *file_found;
Address *line_addr_pointer;
Boolean *is_ambig;
cppSymList *return_list;
{ 
  Filetab     *ftp;
  int          ldndx;
  extern int   loadcnt;
  unsigned     nmatches = 0;
  unsigned     basename_matches = 0;
  cppSymList   list = NULL;
  char        *filename;
  char        *base_filename = basefile(name);
  Boolean      nearestline = varIsSet("$inexact");
  Address      line_addr;

  /*
   *  Look through the file table for
   *  filenames which match 'name'.
   */
  for (ldndx = 0; ldndx < loadcnt; ldndx++) 
  { 
    for (ftp = &filetab[ldndx][0];
           ftp < &filetab[ldndx][nlhdr[ldndx].nfiles]; ftp++)
    { 
      /*  if the file contains line number info  */
      if (ftp->lineptr)
      {
        /*  if the filenames match - and the line number is in range */
        if (is_match (ftp->filename, name)) 
        {
          if (file_found != NULL)
            *file_found = true;

          if (line != 0)
          {
            Linechunk *chunkp;
            Linetab *linetable_ptr;
            Linetab *end_linetable_ptr;
            Address      nl = NOADDR;
            int linediff = 0x7fffffff;

            line_addr = NOADDR;
            chunkp = ftp->lineptr;
            for (;(chunkp && (line_addr == NOADDR));
                 chunkp=chunkp->next) {
              linetable_ptr = chunkp->lp;
              end_linetable_ptr = chunkp->lend;
              while (linetable_ptr <= end_linetable_ptr) {
                /* skip over any include file entries */
                if (ftp->incl_chain) {
                  if (skipIncl(ftp->incl_chain, &linetable_ptr))
                    continue;
                }
                if (linetable_ptr->line == line) {
                   line_addr = linetable_ptr->addr;
                   break;
                }
                if (nearestline) {
                  if (linetable_ptr->line - line < linediff) {
                    linediff = linetable_ptr->line - line;
                    nl = linetable_ptr->addr;
                  }
                }
                linetable_ptr++;
              }
            }
            line_addr = (line_addr == NOADDR) ? nl : line_addr;
          }
          add_to_list (&list, ftp->filename, line, line_addr, &nmatches);
        }

        /*  else if the base filename is the same as that of this entry
              in the filetable  */
        else if (is_match(ftp->filename, base_filename))
        {
          basename_matches++;
        }

        /*  if this file includes files with source  */
        if (ftp->incl_chain)
        {
          /*  add any include file matches to the list  */

          nametoincl (ftp->incl_chain, name, line, &list, file_found,
                      &nmatches, &basename_matches);
        }
      }
    }
  }
  if (nmatches == 0)
    return (NULL); 

  /*  if more than one file was found  */
  if (nmatches > 1)
  {
    /*  put up a menu to let the user decide which to use  */
    list = getResolvedChoices(list, nmatches, false);
  }

  if (line_addr_pointer != NULL)
    *line_addr_pointer = list->line_addr;

  if (is_ambig != NULL)
    if ((nmatches > 1) || (basename_matches > 0))
      *is_ambig = true;
    else
      *is_ambig = false;

  /*  set filename so we can return to correct pointer  */
  filename = list->filename;

  if (return_list != NULL)
    *return_list = list;
  else
    free_element_chain (list);
   
  return (filename);
}

