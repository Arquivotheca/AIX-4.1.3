static char sccsid[] = "@(#)50    1.10.4.1  src/bos/usr/ccs/lib/libdbx/keywords.c, libdbx, bos41J, 9517B_all 4/27/95 17:17:08";
/*
 *   COMPONENT_NAME: CMDDBX
 *
 *   FUNCTIONS: alias
 *		defalias
 *		defvar
 *		enterkeywords
 *		findalias
 *		findkeyword
 *		findvar
 *		findword
 *		getinstructions
 *		getskip
 *		hash
 *		keywdstring
 *		keyword
 *		keywords_delete
 *		keywords_free
 *		keywords_insert
 *		keywords_lookup
 *		print_alias
 *		print_vars
 *		printmparams
 *		unalias
 *		undefvar
 *		varIsSet
 *              defaliasth
 *		
 *
 *   ORIGINS: 26,27, 83
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1994
 *   All Rights Reserved
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
 * Keywords, variables, and aliases (oh my!).
 */

#include "defs.h"
#include "keywords.h"
#include "examine.h"
#include "scanner.h"
#include "names.h"
#include "symbols.h"
#include "tree.h"
#include "lists.h"
#include "main.h"
#include "eval.h"
#include "commands.h"
#include "ops.h"
#include "envdefs.h"

boolean catchbp = false;
#ifdef KDBX
boolean	loadstop = false;
boolean	screen_on = false;
#endif
extern boolean octints, hexints, expandunions;
extern boolean textaddrs, dataaddrs, fileaddrs;
extern unsigned int textorg, dataorg;
boolean keepdescriptors = false;

private String reserved[] ={
    "alias", "all", "and", "assign", "at", "attribute", "bitand", "call", 
#ifdef KDBX
    "case", "catch", "clear", "cleari", "condition", "cont", "cpu", "debug",
#else
    "case", "catch", "clear", "cleari", "condition", "cont", NULL , "debug",
#endif
    "delete", "detach", "div", "down", 
    "dump", "edit", "exp", "file", "func", "goto", "gotoi", "help", "if", 
#ifdef KDBX
    "ignore", "in", "kload", "list", "listi", "lldb",
#else
    "ignore", "in", NULL   , "list", "listi", NULL  ,
#endif
    "map", "mod", "move", "multproc", "mutex", "next", 
    "nexti", "nil", "not", "object", "or", "print", "prompt", "psym", "quit",
    "registers", "rerun", "return", "run", "screen", "set", "sh", "sizeof",
#ifdef KDBX
    "skip","source","status","step","stepi","stop","stopi","switch","symtype",
#else
    "skip","source","status","step","stepi","stop","stopi", NULL   ,"symtype",
#endif
    "thread", "trace", "tracei", "unalias", "unset", "up", "use", "watch", 
#ifdef KDBX
    "whatis","when","where","whereis","which","@wins","@wini","xcall","xor",
#else
    "whatis","when","where","whereis","which","@wins","@wini", NULL  ,"xor",
#endif
    "class", "struct", "union", "typedef", "enum", "type", "@", "$",
    /* end of keywords - the rest is for lex debug only */
    "INT", "UINT", "LONGLONG", "ULONGLONG", "CHAR", "REAL", "QUAD", "NAME",
    "STRING", "2>", ">",
    "&1", "=", "==", "!=", "<>", "<=", ">=", ">>", "<<", "!", "..", "[]", 
    "->*", ".*", "->", "::", 
};

/*
 * The keyword table is a traditional hash table with collisions
 * resolved by chaining.
 */

#define HASHTABLESIZE 1007


typedef enum { ISKEYWORD, ISALIAS, ISVAR } KeywordType;

typedef struct Keyword {
    Name name;
    unsigned /* KeywordType */ class : 16;
    union {
	/* ISKEYWORD: */
	    Token toknum;

	/* ISALIAS: */
	    struct {
		List paramlist;
		String expansion;
	    } alias;

	/* ISVAR: */
	    Node var;
    } value;
    struct Keyword *chain;
} *Keyword;

typedef unsigned int Hashvalue;

private Keyword hashtab[HASHTABLESIZE];

#undef private
#define private

#define hash(n) ((((unsigned) n) >> 2) mod HASHTABLESIZE)

#ifdef K_THREADS
/*
 * NAME: defaliasth
 *
 * FUNCTION: defines 2 aliases : bfth and blth with 2 parameters
 *     blth(f,x)      stop at f if ($running_thread == x)
 *     bfth(f,x)      stopi at &f if ($running_thread == x)
 *
 * PARAMETERS: NONE
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: NONE
 */
private void defaliasth ()
{
    List aliaslist;
    aliaslist = list_alloc();
    list_append(list_item(identname("f",false)),nil,aliaslist);
    list_append(list_item(identname("x",false)),nil,aliaslist);
    alias(identname("bfth",true), aliaslist,
          "stopi at &f if ($running_thread == x)");
    alias(identname("blth",true), aliaslist,
          "stop at f if ($running_thread == x)");

}

#endif /* K_THREADS */

/*
 * Enter all the reserved words into the keyword table.
 *
 * If the vaddrs flag is set (through the -k command line option) then
 * set the special "$mapaddrs" variable.  This assumes that the
 * command line arguments are scanned before this routine is called.
 */

public enterkeywords()
{
    register integer i;
    List args_list;
    char *p;
    char	*home;
    char	*pg_cmd;

    for (i = ALIAS; i <= DOLLAR; i++) {
	keyword(reserved[ord(i) - ord(ALIAS)], i);
    }
    defalias("c", "cont");
    defalias("d", "delete");
    defalias("h", "help");
    defalias("e", "edit");
    defalias("l", "list");
    defalias("m", "map");
    defalias("n", "next");
    defalias("p", "print");
    defalias("q", "quit");
    defalias("x", "registers");
    defalias("r", "run");
    defalias("s", "step");
    defalias("st", "stop");
    defalias("j", "status");
    defalias("t", "where");

    /* alias for pager */
    args_list = list_alloc();
    home = getenv("HOME");
    if( home == NULL ) {
	/*
	 * If $HOME is not set, use /tmp by default
	 */
	home = "/tmp";
    }
    list_append(list_item(identname("x", true)), nil, args_list);
    /*
     * Allocate enough space for command string, size is 46 which is the
     * hard-coded part of the string plus 3 instances of $HOME
     *
     * This memory is not freed here, since alias points to this string
     */
    pg_cmd = malloc( (3 * strlen(home) + 46) * sizeof(char) );
    sprintf(pg_cmd, "x >! %s/.dbxout; sh pg %s/.dbxout; rm -f %s/.dbxout",
	    home, home, home);
    alias(identname("pg", true), args_list, pg_cmd);

#if defined (CMA_THREAD) || defined (K_THREADS)
    defalias("attr", "attribute");
    defalias("cv", "condition");
    defalias("mu", "mutex");
    defalias("th", "thread");
#endif /* CMA_THREAD || K_THREADS */
#ifdef K_THREADS
    defaliasth();
#endif /* K_THREADS */
    if (vaddrs) {
	defvar(identname("$mapaddrs", true), nil);
    }
    defvar(identname("$listwindow", true), build(O_LCON, (long) 10));
    defvar(identname("$menuwindow", true), build(O_LCON, (long) 10));
    defvar(identname("$vardim", true), build(O_LCON, (long) 10));
    defvar(identname("$noflregs", true), nil);
    defvar(identname("$ignoreload", true), nil);
    defvar(identname("$showbases", true), nil);
    defvar(identname("$stepignore", true),
	   build(O_SCON, strdup("function"), 9));
    defvar(identname("$instructionset", true),
	   build(O_SCON, strdup("default"), 8));
    defvar(identname("$mnemonics", true), build(O_SCON, strdup("default"), 8));
    p = getenv("DISPLAY");
    defvar(identname("$xdisplay", true),
	   build(O_SCON, strdup(p), strlen(p)+1 ));
}

/*
 * Deallocate the keyword table.
 */

public keywords_free()
{
    register Integer i;
    register Keyword k, nextk;

    for (i = 0; i < HASHTABLESIZE; i++) {
	k = hashtab[i];
	while (k != nil) {
	    nextk = k->chain;
	    dispose(k);
	    k = nextk;
	}
	hashtab[i] = nil;
    }
}

/*
 * Insert a name into the keyword table and return the keyword for it.
 */

private Keyword keywords_insert (n)
Name n;
{
    Hashvalue h;
    Keyword k;

    h = hash(n);
    k = new(Keyword);
    k->name = n;
    k->chain = hashtab[h];
    hashtab[h] = k;
    return k;
}

/*
 * Find the keyword associated with the given name.
 */

private Keyword keywords_lookup (n)
Name n;
{
    Hashvalue h;
    register Keyword k;

    h = hash(n);
    k = hashtab[h];
    while (k != nil and k->name != n) {
	k = k->chain;
    }
    return k;
}

/*
 * Delete the given keyword of the given class.
 */

private boolean keywords_delete (n, class)
Name n;
KeywordType class;
{
    Hashvalue h;
    register Keyword k, prevk;
    boolean b;

    h = hash(n);
    k = hashtab[h];
    prevk = nil;
    while (k != nil and (k->name != n or k->class != class)) {
	prevk = k;
	k = k->chain;
    }
    if (k != nil) {
	b = true;
	if (prevk == nil) {
	    hashtab[h] = k->chain;
	} else {
	    prevk->chain = k->chain;
	}
	tfree( k->value.var);
	dispose(k);
    } else {
	b = false;
    }
    return b;
}

/*
 * Enter a keyword into the table.  It is assumed to not be there already.
 * The string is assumed to be statically allocated.
 */

private keyword (s, t)
String s;
Token t;
{
    Keyword k;
    Name n;

    n = identname(s, true);
    k = keywords_insert(n);
    k->class = ISKEYWORD;
    k->value.toknum = t;
}

/*
 * Define a builtin command name alias.
 */

private defalias (s1, s2)
String s1, s2;
{
    alias(identname(s1, true), nil, s2);
}

/*
 * Look for a word of a particular class.
 */

private Keyword findword (n, class)
Name n;
KeywordType class;
{
    register Keyword k;

    k = keywords_lookup(n);
    while (k != nil and (k->name != n or k->class != class)) {
	k = k->chain;
    }
    return k;
}

/*
 * Return the token associated with a given keyword string.
 * If there is none, return the given default value.
 */

public Token findkeyword (n, def)
Name n;
Token def;
{
    Keyword k;
    Token t;

    k = findword(n, ISKEYWORD);
    if (k == nil) {
	t = def;
    } else {
	t = k->value.toknum;
    }
    return t;
}

/*
 * Return the associated string if there is an alias with the given name.
 */

public boolean findalias (n, pl, str)
Name n;
List *pl;
String *str;
{
    Keyword k;
    boolean b = true;

    k = findword(n, ISALIAS);
    if (k == nil) {
	b = false;
    } else {
	*pl = k->value.alias.paramlist;
	*str = k->value.alias.expansion;
    }
    return b;
}

/*
 * Return the string associated with a token corresponding to a keyword.
 */

public String keywdstring (t)
Token t;
{
    return reserved[ord(t) - ord(ALIAS)];
}

/*
 * Process an alias command, either entering a new alias or printing out
 * an existing one.
 */

public alias (newcmd, args, str)
Name newcmd;
List args;
String str;
{
    Keyword k;

    if (str == nil) {
	print_alias(newcmd);
    } else {
	k = findword(newcmd, ISALIAS);
	if (k == nil) {
	    k = keywords_insert(newcmd);
	}
	k->class = ISALIAS;
	k->value.alias.paramlist = args;
	k->value.alias.expansion = str;
    }
}

/*
 * Print out an alias.
 */

private print_alias (cmd)
Name cmd;
{
    register Keyword k;
    register Integer i;

    if (cmd == nil) {
	for (i = 0; i < HASHTABLESIZE; i++) {
	    for (k = hashtab[i]; k != nil; k = k->chain) {
		if (k->class == ISALIAS) {
		    if (isredirected()) {
			(*rpt_output)(stdout, "alias %s", ident(k->name));
			printmparams(stdout, k->value.alias.paramlist);
			(*rpt_output)(stdout, "\t\"%s\"\n",
						     k->value.alias.expansion);
		    } else {
			(*rpt_output)(stdout, "%s", ident(k->name));
			printmparams(stdout, k->value.alias.paramlist);
			(*rpt_output)(stdout, "\t%s\n",
						     k->value.alias.expansion);
		    }
		}
	    }
	}
    } else {
	k = findword(cmd, ISALIAS);
	if (k == nil) {
	    (*rpt_output)(stdout, "\n");
	} else {
	    printmparams(stdout, k->value.alias.paramlist);
	    (*rpt_output)(stdout, "%s\n", k->value.alias.expansion);
	}
    }
}

public printmparams (f, pl)
File f;
List pl;
{
    Name n;

    if (pl != nil) {
	(*rpt_output)(f, "(");
	foreach(Name, n, pl)
	    (*rpt_output)(f, "%s", ident(n));
	    if (not list_islast()) {
		(*rpt_output)(f, ", ");
	    }
	endfor
	(*rpt_output)(f, ")");
    }
}

/*
 * Remove an alias.
 */

public unalias (n)
Name n;
{
    if (not keywords_delete(n, ISALIAS)) {
	error( catgets(scmc_catd, MS_keywords, MSG_171,
					       "%s is not aliased"), ident(n));
    }
}

/*
 * NAME: getskip
 *
 * FUNCTION: Attempt to get a value for $stepignore from the passed in Node.
 *
 * EXECUTION ENVIRONMENT:
 *	Normal user process
 *
 * NOTES: The valid values are "function", "module" and "none"
 *
 * RECOVERY OPERATION: none
 *
 * DATA STRUCTURES: none
 *
 * PARAMETERS:
 *	n		SCON node containing string to convert
 *
 * TESTING: Test all values of $stepignore to insure they work correctly,
 *	also that bad values are ignored.
 *
 * RETURNS: skiptype if successfull, -1 if it fails.
 *
 */
private skiptype getskip(n)
Node	n;
{
    if (n->op == O_SCON)
    {
	if (!strcmp(n->value.scon, "function"))
	    return skipfunc;
	else if (!strcmp(n->value.scon, "module"))
	    return skipmodule;
	else if (!strcmp(n->value.scon, "none"))
	    return skipnone;
    }
    (*rpt_error)(stderr, catgets(scmc_catd, MS_keywords, MSG_783,
		"Valid values are \"function\", \"module\", and \"none\"\n"));
    return -1;
}


/*
 * NAME: getintructions
 *
 * FUNCTION: Attempt to get a value for $instructionset or $mnemonics from the
 *	passed in Node.
 *
 * NOTES: The valid values are "pwr", "pwrx", "ppc", "601", "603", "604", "com",
 *	"620", and "any" for $instructionset.  And "pwr", "ppc" for $mnemonics.
 *
 * RECOVERY OPERATION: none
 *
 * DATA STRUCTURES: none
 *
 * PARAMETERS:
 *	n		SCON node containing string to convert
 *	instructions	If true means checking for $instructionset, if false
 *			means checking for $menmonics.
 *
 * RETURNS: instruction set type if successful, NOSET if it fails.
 */
static unsigned int getinstructions(n, instructions)
Node	n;
Boolean instructions;
{
    if (n->op == O_SCON)
    {
	if (!strcmp(n->value.scon, "pwr"))
	    return PWR;
	else if ( instructions == true && !strcmp(n->value.scon, "pwrx"))
	    return PWRX;
	else if (!strcmp(n->value.scon, "ppc"))
	    return PPC;
	else if ( !strcmp(n->value.scon, "default"))
	    return DEFAULT;
	else if ( instructions == true && !strcmp(n->value.scon, "601"))
	    return SET_601;
	else if ( instructions == true && !strcmp(n->value.scon, "603"))
	    return SET_603;
	else if ( instructions == true && !strcmp(n->value.scon, "604"))
	    return SET_604;
	else if ( instructions == true && !strcmp(n->value.scon, "620"))
	    return SET_620;
	else if ( instructions == true && !strcmp(n->value.scon, "com"))
	    return COM;
	else if ( instructions == true && !strcmp(n->value.scon, "any"))
	    return ANY;
    }
    if( instructions == true ) {
	(*rpt_error)(stderr, catgets(scmc_catd, MS_keywords, MSG_784,
		     "Invalid value\n\
\tUsage: set $instructionset = {\"pwr\", \"pwrx\", \"ppc\", \"601\",\n\
\t\t\"603\", \"604\", \"com\", \"any\", \"default\"}\n"));
    } else {
	(*rpt_error)(stderr, catgets(scmc_catd, MS_keywords, MSG_785,
		     "Invalid value\n\
\tUsage: set $mnemonics = {\"pwr\", \"ppc\", \"default\"}\n"));
    }
    return NOSET;
}


/*
 * Define a variable.
 */

public defvar (n, val)
Name n;
Node val;
{
    Keyword k;
    Node	oldval;
    skiptype	st;
    unsigned	instruction_value;
    Boolean	oldval_used = false;

    if (n == nil) {
	print_vars();
    } else {
	if (lookup(n) != nil) {
	    error( catgets(scmc_catd, MS_keywords, MSG_172,
			"\"%s\" is a program symbol -- use assign"), ident(n));
	}
	k = findword(n, ISVAR);
	if (k == nil) {
	    k = keywords_insert(n);
	    k->value.var = nil;
	}
        action_mask |= CONFIGURATION;
	oldval = k->value.var;
	k->class = ISVAR;
	k->value.var = val;
	if (n == identname("$stepignore", true))
	{
	    st = getskip(val);
	    if (st < 0)		/* Don't change if the value is bad	*/
	    {
		if (oldval) {
		    oldval_used = true;
		    k->value.var = oldval;
		} else {
		    k->value.var = build(O_SCON, strdup("function"), 9);
		    stepignore = skipfunc;
		}
	    }
	    else
		stepignore = st;
	}
	else if (n == identname("$instructionset", true))
	{
	    instruction_value = getinstructions(val, true);
	    /*
	     * Don't change if the value is bad
	     */
	    if (instruction_value == NOSET) {
		if (oldval) {
		    oldval_used = true;
		    k->value.var = oldval;
		} else {
		    k->value.var = build(O_SCON, NULL, 0);
		    instruction_set = DEFAULT;
		}
	    } else {
		instruction_set = instruction_value;
	    }
	}
	else if (n == identname("$mnemonics", true))
	{
	    instruction_value = getinstructions(val, false);
	    /*
	     * Don't change if the value is bad
	     */
	    if (instruction_value == NOSET) {
		if (oldval) {
		    oldval_used = true;
		    k->value.var = oldval;
		} else {
		    k->value.var = build(O_SCON, NULL, 0);
		    mnemonic_set = DEFAULT;
		}
	    } else {
		mnemonic_set = instruction_value;
	    }
	}
	else if (n == identname("$mapaddrs", true)) {
	    vaddrs = true;
	}
	else if (n == identname("$expandunions", true)) {
	    expandunions = true;
	}
	else if (n == identname("$catchbp", true)) {
	    catchbp = true;
	}
	else if (n == identname("$keepdescriptors", true)) {
	    keepdescriptors = true;
	}
	else if (n == identname("$hexints", true)) {
	    hexints = true;
	    if (octints) {
	       undefvar(identname("$octints",true));
	       octints = false;
	    }
        }
	else if (n == identname("$octints", true)) {
	    octints = true;
	    if (hexints) {
	       undefvar(identname("$hexints",true));
	       hexints = false;
	    }
        }
	else if (n == identname("$text", true)) {
	    textaddrs = true;
	    if (fileaddrs) {
	       undefvar(identname("$file",true));
	       fileaddrs = false;
	    }
	    else if (dataaddrs) {
	       undefvar(identname("$data",true));
	       dataaddrs = false;
	    }
        }
	else if (n == identname("$data", true)) {
	    dataaddrs = true;
	    if (textaddrs) {
	       undefvar(identname("$text",true));
	       textaddrs = false;
	    }
	    else if (fileaddrs) {
	       undefvar(identname("$file",true));
	       fileaddrs = false;
	    }
        }
	else if (n == identname("$file", true)) {
	    fileaddrs = true;
	    if (textaddrs) {
	       undefvar(identname("$text",true));
	       textaddrs = false;
	    }
	    else if (dataaddrs) {
	       undefvar(identname("$data",true));
	       dataaddrs = false;
	    }
        }
	else if (n == identname("$textorg", true)) {
	    eval(val);
	    textorg = pop(integer);
        }
	else if (n == identname("$dataorg", true)) {
	    eval(val);
	    dataorg = pop(integer);
        }
#ifdef KDBX
	else if (n == identname("$loadstop", true)) {
	    loadstop = true;
	}
	else if (n == identname("$screen", true)) {
	    screen_on = true;
	}
	else if (n == identname("$mst", true)) {
	    doflush ();
	}
#endif
	if( oldval_used == false ) {
	    tfree( oldval );
	}
    }
}

/*
 * Return the value associated with a variable.
 */

public Node findvar (n)
Name n;
{
    Keyword k;
    Node val;

    k = findword(n, ISVAR);
    if (k == nil) {
	val = nil;
    } else {
	/*
	 * Get a copy of the value Node, so we don't free it by mistake
	 */
	val = copynode( k->value.var );
    }
    return val;
}

/*
 * Return whether or not a variable is set.
 */

public boolean varIsSet (s)
String s;
{
    return (boolean) (findword(identname(s, false), ISVAR) != nil);
}

/*
 * Delete a variable.
 */

public undefvar (n)
Name n;
{
    if (not keywords_delete(n, ISVAR)) {
	error( catgets(scmc_catd, MS_keywords, MSG_223,
						   "%s is not set"), ident(n));
    }
    if (n == identname("$stepignore", true)) {
	stepignore = skipfunc;
    }
    else if (n == identname("$instructionset", true)) {
	instruction_set = DEFAULT;
    }
    else if (n == identname("$mnemonics", true)) {
	mnemonic_set = DEFAULT;
    }
    else if (n == identname("$mapaddrs", true)) {
	vaddrs = false;
    } else if (n == identname("$hexints", true)) {
	hexints = false;
    } else if (n == identname("$octints", true)) {
	octints = false;
    } else if (n == identname("$catchbp", true)) {
	catchbp = false;
    } else if (n == identname("$keepdescriptors", true)) {
	keepdescriptors = false;
    } else if (n == identname("$expandunions", true)) {
	expandunions = false;
    } else if (n == identname("$text", true)) {
	textaddrs = false;
    } else if (n == identname("$data", true)) {
	dataaddrs = false;
    } else if (n == identname("$file", true)) {
	fileaddrs = false;
    } else if (n == identname("$dataorg", true)) {
	dataorg = 0;
    } else if (n == identname("$textorg", true)) {
	textorg = 0;
    }
#ifdef KDBX
    else if (n == identname("$loadstop", true)) {
	loadstop = false;
    } else if (n == identname("$screen", true)) {
	screen_on = false;
    } else if (n == identname("$mst", true)) {
	doflush ();
    }
#endif
    action_mask |= CONFIGURATION;
}

/*
 * Print out all the values of set variables.
 */

private print_vars ()
{
    register integer i;
    register Keyword k;

    for (i = 0; i < HASHTABLESIZE; i++) {
	for (k = hashtab[i]; k != nil; k = k->chain) {
	    if (k->class == ISVAR) {
		if (isredirected()) {
		    (*rpt_output)(stdout, "set ");
		}
		(*rpt_output)(stdout, "%s", ident(k->name));
		if (k->value.var != nil) {
		    (*rpt_output)(stdout, "\t");
		    prtree( rpt_output, stdout, k->value.var);
		}
		(*rpt_output)(stdout, "\n");
	    }
	}
    }
}
