/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: VarAdd
 *		VarApplyModifiers1540
 *		VarCmp
 *		VarFind
 *		VarFindEntry
 *		VarHashTable
 *		VarHead
 *		VarIndirect
 *		VarMatch
 *		VarModify
 *		VarNoMatch
 *		VarParse
 *		VarParseCond
 *		VarPrintVar
 *		VarRoot
 *		VarRunShellCmd
 *		VarSet
 *		VarSpecial
 *		VarSubstitute
 *		VarSuffix
 *		VarTail
 *		VarUnset
 *		VarValueCond
 *		Var_Append
 *		Var_Delete
 *		Var_Dump
 *		Var_Exists
 *		Var_HasMeta
 *		Var_Init
 *		Var_Parse
 *		Var_Set
 *		Var_Skip
 *		Var_StrValue
 *		Var_Subst
 *		Var_Value
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * @OSF_FREE_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1992, 1991, 1990  
 * Open Software Foundation, Inc. 
 *  
 * Permission is hereby granted to use, copy, modify and freely distribute 
 * the software in this file and its documentation for any purpose without 
 * fee, provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation.  Further, provided that the name of Open 
 * Software Foundation, Inc. ("OSF") not be used in advertising or 
 * publicity pertaining to distribution of the software without prior 
 * written permission from OSF.  OSF makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is" 
 * without express or implied warranty. 
 */
/*
 * HISTORY
 * $Log: var.c,v $
 * Revision 1.2.2.5  1992/12/03  19:07:31  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:36:44  damon]
 *
 * Revision 1.2.2.4  1992/11/09  21:50:34  damon
 * 	CR 296. Cleaned up to remove warnings
 * 	[1992/11/09  21:49:26  damon]
 * 
 * Revision 1.2.2.3  1992/09/24  19:27:49  gm
 * 	CR286: Major improvements to make internals.
 * 	[1992/09/24  17:56:04  gm]
 * 
 * Revision 1.2.2.2  1992/06/24  16:31:57  damon
 * 	CR 181. Changed vfork to fork
 * 	[1992/06/24  16:19:44  damon]
 * 
 * Revision 1.2  1991/12/05  20:45:27  devrcs
 * 	Changes for parallel make.
 * 	[91/04/21  17:19:56  gm]
 * 
 * 	Added definition of FD_SETSIZE
 * 	[91/04/03  12:43:53  damon]
 * 
 * 	Changes for Reno make
 * 	[91/03/22  16:10:49  mckeen]
 * 
 * $EndLog$
 */
/*
 * Copyright (c) 1988, 1989, 1990 The Regents of the University of California.
 * Copyright (c) 1988, 1989 by Adam de Boor
 * Copyright (c) 1989 by Berkeley Softworks
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Adam de Boor.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
static char sccsid[] = "@(#)55  1.4  src/bldenv/make/var.c, bldprocess, bos412, GOLDA411a 1/19/94 16:31:59";
#endif /* not lint */

#ifndef lint
static char rcsid[] = "@(#)var.c	5.7 (Berkeley) 6/1/90";
#endif /* not lint */

/*-
 * var.c --
 *	Variable-handling functions
 *
 * Interface:
 *	Var_Set	  	    Set the value of a variable in the given
 *	    	  	    context. The variable is created if it doesn't
 *	    	  	    yet exist. The value and variable name need not
 *	    	  	    be preserved.
 *
 *	Var_Append	    Append more characters to an existing variable
 *	    	  	    in the given context. The variable needn't
 *	    	  	    exist already -- it will be created if it doesn't.
 *	    	  	    A space is placed between the old value and the
 *	    	  	    new one.
 *
 *	Var_Exists	    See if a variable exists.
 *
 *	Var_Value 	    Return the value of a variable in a context or
 *	    	  	    NULL if the variable is undefined.
 *
 *	Var_Subst 	    Substitute for all variables in a string using
 *	    	  	    the given context as the top-most one. If the
 *	    	  	    third argument is non-zero, Parse_Error is
 *	    	  	    called if any variables are undefined.
 *
 *	Var_Parse 	    Parse a variable expansion from a string and
 *	    	  	    return the result and the number of characters
 *	    	  	    consumed.
 *
 *	Var_Delete	    Delete a variable in a context.
 *
 *	Var_Init  	    Initialize this module.
 *
 * Debugging:
 *	Var_Dump  	    Print out all variables defined in the given
 *	    	  	    context.
 *
 * XXX: There's a lot of duplication in these functions.
 */

#include    <sys/types.h>
#include    <sys/time.h>
#include    <sys/wait.h>
#include    <stdio.h>
#include    <ctype.h>
#include    <errno.h>
#include    "make.h"
#include    "hash.h"
#include    "buf.h"

#define STATIC	/* static		debugging support */

#ifndef FD_SETSIZE
#define FD_SETSIZE      256
#endif

/*
 * This is a harmless return value for Var_Parse that can be used by Var_Subst
 * to determine if there was an error in parsing -- easier than returning
 * a flag, as things outside this module don't give a hoot.
 */
char 	var_Error[] = "";

/*
 * Similar to var_Error, but returned when the 'err' flag for Var_Parse is
 * set false. Why not just use a constant? Well, gcc likes to condense
 * identical string instances...
 */
char	varNoError[] = "";

/*
 * Internally, variables are contained in four different contexts.
 *	1) the environment. They may not be changed. If an environment
 *	    variable is appended-to, the result is placed in the global
 *	    context.
 *	2) the global context. Variables set in the Makefile are located in
 *	    the global context. It is the penultimate context searched when
 *	    substituting.
 *	3) the command-line context. All variables set on the command line
 *	   are placed in this context. They are UNALTERABLE once placed here.
 *	4) the local context. Each target has associated with it a context
 *	   list. On this list are located the structures describing such
 *	   local variables as $(@) and $(*)
 * The four contexts are searched in the reverse order from which they are
 * listed.
 */
GNode          *VAR_GLOBAL;   /* variables from the makefile */
GNode          *VAR_CMD;      /* variables defined on the command-line */
GNode          *VAR_ENV;      /* variables defined in the environment */

Hash_Table     globalHashTable;
Hash_Table     cmdHashTable;
Hash_Table     envHashTable;

#define FIND_CMD	0x1   /* look in VAR_CMD when searching */
#define FIND_GLOBAL	0x2   /* look in VAR_GLOBAL as well */
#define FIND_ENV  	0x4   /* look in the environment also */

typedef struct Var {
    string_t      name;		/* the variable's name */
    string_t	  val;	    	/* its value */
    int	    	  flags;    	/* miscellaneous status flags */
#define VAR_IN_USE	1   	    /* Variable's value currently being used.
				     * Used to avoid recursion */
#define VAR_FROM_ENV	2   	    /* Variable comes from the environment */
#define VAR_JUNK  	4   	    /* Variable is a junk variable that
				     * should be destroyed when done with
				     * it. Used by Var_Parse for undefined,
				     * modified variables */
#define VAR_KEEP	8	    /* Variable is VAR_JUNK, but we found
				     * a use for it in some modifier and
				     * the value is therefore valid */
    string_t	  where;	/* makefile/context this is defined */
}  Var;

STATIC int VarParse(Buffer, const char *, GNode *, Boolean, int *);

/*-
 *-----------------------------------------------------------------------
 * VarRunShellCmd  --
 *	Run the command and return its value.
 *
 * Results:
 *	none
 *
 * Side Effects:
 *	none
 *-----------------------------------------------------------------------
 */
STATIC int
VarRunShellCmd(
    Buffer	resultBuf,	/* buffer for results */
    Buffer      cmdBuf)		/* buffer with command to execute */
{
    char	result[BUFSIZ];	/* Result of command */
    const char	*args[5];   	/* Args for invoking the shell */
    const char	**av;
    int 	fds[2];	    	/* Pipe streams */
    int 	cpid;	    	/* Child PID */
    int 	pid;	    	/* PID from waitpid() */
    int		status;
    int		cc;
    Buffer	buf;
    char	*str;
    char	*cp;
    int		stringlen;
    char	*cmd = (char *)Buf_GetBase(cmdBuf);

    buf = Buf_Init(0);

    if (Var_HasMeta(cmd)) {
	/*
	 * Set up arguments for shell
	 */
	args[0] = "/bin/sh";
	args[1] = "sh";
	args[2] = "-c";
	args[3] = cmd;
	args[4] = (char *)NULL;
	av = args;
    } else {
	/*
	 * No meta-characters, so no need to exec a shell. Break the command
	 * into words to form an argument vector we can execute.
	 */
	int argmax;
	char *cmdbuf;
	register int argc, ch;
	register char inquote, *p, *start, *t;
	Boolean done;

	argmax = 32;
	av = (const char **)emalloc(argmax * sizeof(char *));

	/* allocate room for a copy of the string */
	cmdbuf = strdup(cmd);

	/*
	 * copy the string; at the same time, parse backslashes,
	 * quotes and build the argument list.
	 */
	argc = 1;
	inquote = 0;
	done = FALSE;
	for (p = cmd, start = t = cmdbuf; !done; ++p) {
	    switch(ch = *p) {
	    case '"':
	    case '\'':
		if (inquote)
		    if (inquote == ch)
			inquote = 0;
		    else
			break;
		else
		    inquote = ch;
		continue;
	    case ' ':
	    case '\t':
		if (inquote)
		    break;
		if (!start)
		    continue;
		/* FALLTHROUGH */
	    case '\n':
	    case '\0':
		/*
		 * end of a token -- make sure there's enough av
		 * space and save off a pointer.
		 */
		*t++ = '\0';
		if (argc == argmax) {
		    argmax <<= 1;		/* ramp up fast */
		    av = (const char **)realloc(av,
						argmax * sizeof(char *));
		    if (av == 0)
			enomem();
		}
		av[argc++] = start;
		start = (char *)NULL;
		if (ch == '\n' || ch == '\0')
		    done = TRUE;
		continue;
	    case '\\':
		switch (ch = *++p) {
		case '\0':
		case '\n':
		    /* hmmm; fix it up as best we can */
		    ch = '\\';
		    --p;
		    break;
		}
		break;
	    }
	    if (!start)
		start = t;
	    *t++ = ch;
	}
	av[0] = av[1];
	av[argc] = (char *)NULL;
    }

    /*
     * Open a pipe for fetching its output
     */
    pipe(fds);

    /*
     * Fork
     */
    cpid = fork();
    if (cpid == 0) {
	/*
	 * Close input side of pipe
	 */
	close(fds[0]);

	/*
	 * Duplicate the output stream to the shell's output, then
	 * shut the extra thing down. Note we don't fetch the error
	 * stream...why not? Why?
	 */
	dup2(fds[1], 1);
	close(fds[1]);

	execvp(av[0], (char * const *)&av[1]);
	_exit(1);
    }
    if (cpid < 0) {
	/*
	 * Couldn't fork -- tell the user and make the variable null
	 */
	Error("Couldn't exec \"%s\"", cmd);
	Buf_Destroy(buf);
	return(1);
    }

    /*
     * No need for the writing half
     */
    close(fds[1]);

    for (;;) {
	/*
	 * Read all the characters the child wrote.
	 */
	if ((cc = read(fds[0], result, sizeof(result))) < 0) {
	    if (errno == EINTR) {
		perror("read");
		continue;
	    }
	    /*
	     * Couldn't read the child's output -- tell the user and
	     * set the variable to null
	     */
	    Error("Couldn't read shell's output");
	    break;
	}
	if (cc == 0)
	    break;
	Buf_AddBytes(buf, cc, (Byte *)result);
    }

    /*
     * Wait for the process to exit.
     */
    if ((pid = waitpid(cpid, &status, 0)) != cpid) {
	Error("unexpected error from waitpid: %s", strerror(errno));
    }

    if (status) {
	/*
	 * Child returned an error -- tell the user but still use
	 * the result.
	 */
	Error("\"%s\" returned non-zero", cmd);
    }

    /*
     * Close the input side of the pipe.
     */
    close(fds[0]);

    /*
     * Null-terminate the result, convert newlines to spaces and
     * install it in the variable.
     */
    str = (char *) Buf_GetBase (buf);
    stringlen = Buf_Size(buf);
    cp = str + stringlen - 1;

    while (*cp == '\n') {
	/*
	 * trailing newlines are just stripped
	 */
	*cp-- = '\0';
    }
    for (cp++; str < cp; str++) {
	if (*str == '\n') {
	    Buf_AddByte(resultBuf, (Byte) ' ');
	    continue;
	}
	if (*str == '\0') {
	    Error("\"%s\" output contains nulls", cmd);
	    continue;
	}
	Buf_AddByte(resultBuf, (Byte) *str);
    }
    Buf_Destroy(buf);

    if (DEBUG(VAR)) {
	printf("Var: ${var:!cmd!} returned %s\n",
	       (char *) Buf_GetBase(resultBuf));
    }
    return(0);
}

/*-
 *-----------------------------------------------------------------------
 * VarCmp  --
 *	See if the given variable matches the named one. Called from
 *	Lst_Find when searching for a variable of a given name.
 *
 * Results:
 *	0 if they match. non-zero otherwise.
 *
 * Side Effects:
 *	none
 *-----------------------------------------------------------------------
 */
STATIC int
VarCmp (
    ClientData	v,		/* VAR structure to compare */
    ClientData	name)		/* name to look for */
{
    return ((string_t)name != ((Var *)v)->name);
}

STATIC Hash_Table *
VarHashTable(GNode *ctxt)
{
    if (ctxt == VAR_CMD)
	return(&cmdHashTable);
    if (ctxt == VAR_GLOBAL)
	return(&globalHashTable);
    if (ctxt == VAR_ENV)
	return(&envHashTable);
    return((Hash_Table *)NULL);
}

STATIC Var *
VarFindEntry (
    GNode          	*ctxt,	/* context in which to find it */
    string_t           	name)	/* name to find */
{
    Hash_Table		*ht;
    Hash_Entry		*he;	/* hash entry */
    LstNode		var;

    if ((ht = VarHashTable(ctxt)) == NULL) {
	var = Lst_Find(ctxt->context, (ClientData)name, VarCmp);
	if (var == NILLNODE)
	    return ((Var *)NULL);
	return ((Var *)Lst_Datum (var));
    }
    he = Hash_FindEntry(ht, name);
    if (he == NULL)
	return ((Var *)NULL);
    return ((Var *)Hash_GetValue(he));
}

STATIC void
VarSet(
    GNode          	*ctxt,	/* context in which to find it */
    Var			*var)
{
    Hash_Table		*ht;
    Hash_Entry *he;
    Boolean isNew;

    if ((ht = VarHashTable(ctxt)) == NULL) {
	(void) Lst_AtFront (ctxt->context, (ClientData)var);
	return;
    }
    he = Hash_CreateEntry(ht, var->name, &isNew);
    if (!isNew)
	printf("%s:%s: entry exists\n", ctxt->name->data, var->name->data);
    Hash_SetValue(he, var);
}

static void
VarUnset(
    string_t		name,
    GNode		*ctxt)
{
    register Var  *v;
    Hash_Table *ht;

    if ((ht = VarHashTable(ctxt)) == NULL) {
	LstNode ln;

	ln = Lst_Find(ctxt->context, (ClientData)name, VarCmp);
	if (ln == NILLNODE)
	    return;
	v = (Var *)Lst_Datum(ln);
	Lst_Remove(ctxt->context, ln);
    } else {
	Hash_Entry *he;

	he = Hash_FindEntry(ht, name);
	if (he == NULL)
	    return;
	v = (Var *) Hash_GetValue(he);
	Hash_DeleteEntry(ht, he);
    }
    string_deref(v->val);
    string_deref(v->name);
    free((char *)v);
}

STATIC string_t
VarSpecial(string_t name)
{
    const char *n = name->data;
    int c;

    if (*n == '.' && strchr("AIMOPT", n[1]))
	c = n[1];
    else if (*n && n[1] == 0 && strchr("@?><*!%", *n))
	c = n[0];
    else
	return((string_t) NULL);
    switch (c) {
    case '>':
    case '!':
    case 'A':
	if (name == sALLSRC || name == s_ALLSRC)
	    return(sALLSRC);
	if (name == sARCHIVE || name == s_ARCHIVE)
	    return(sARCHIVE);
	break;
    case '<':
    case 'I':
	if (name == sIMPSRC || name == s_IMPSRC)
	    return(sIMPSRC);
	break;
    case '%':
    case 'M':
	if (name == sMEMBER || name == s_MEMBER)
	    return(sMEMBER);
	break;
    case '?':
    case 'O':
	if (name == sOODATE || name == s_OODATE)
	    return(sOODATE);
	break;
    case '*':
    case 'P':
	if (name == sPREFIX || name == s_PREFIX)
	    return(sPREFIX);
	break;
    case '@':
    case 'T':
	if (name == sTARGET || name == s_TARGET)
	    return(sTARGET);
	break;
    }
    return((string_t) NULL);
}

/*-
 *-----------------------------------------------------------------------
 * VarFind --
 *	Find the given variable in the given context and any other contexts
 *	indicated.
 *
 * Results:
 *	A pointer to the structure describing the desired variable or
 *	NIL if the variable does not exist.
 *
 * Side Effects:
 *	None
 *-----------------------------------------------------------------------
 */
STATIC Var *
VarFind (
    string_t           	name,	/* name to find */
    GNode          	*ctxt,	/* context in which to find it */
    int             	flags)	/* FIND_GLOBAL set means to look in the
				 * VAR_GLOBAL context as well.
				 * FIND_CMD set means to look in the VAR_CMD
				 * context also.
				 * FIND_ENV set means to look in the
				 * environment */
{
    Var		  	*v;
    string_t		n;

    /*
     * First look for the variable in the given context. If it's not there,
     * look for it in VAR_CMD, VAR_GLOBAL and the environment, in that order,
     * depending on the FIND_* flags in 'flags'
     */
    if (ctxt != VAR_CMD && ctxt != VAR_GLOBAL && ctxt != VAR_ENV) {
	n = VarSpecial(name);
	if (n == NULL) {
	    if (ctxt->contextExtras)
		v = VarFindEntry (ctxt, name);
	    else
		v = (Var *)NULL;
	} else {
	    name = n;
	    v = VarFindEntry (ctxt, name);
	}
    } else
	v = VarFindEntry (ctxt, name);
    if (v == (Var *)NULL && (flags & FIND_CMD) && ctxt != VAR_CMD)
	v = VarFindEntry (VAR_CMD, name);
    if (!checkEnvFirst && v == (Var *)NULL && (flags & FIND_GLOBAL) &&
	ctxt != VAR_GLOBAL)
	v = VarFindEntry (VAR_GLOBAL, name);
    if (v == (Var *)NULL && (flags & FIND_ENV)) {
	v = VarFindEntry (VAR_ENV, name);
	if (v != (Var *)NULL)
	    v->flags |= VAR_FROM_ENV;
	else if (checkEnvFirst && (flags & FIND_GLOBAL) &&
		   ctxt != VAR_GLOBAL)
	    v = VarFindEntry (VAR_GLOBAL, name);
	else
	    v = (Var *)NULL;
    }
    return (v);
}

/*-
 *-----------------------------------------------------------------------
 * VarAdd  --
 *	Add a new variable of name name and value val to the given context
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	The new variable is placed at the front of the given context
 *	The name and val arguments are duplicated so they may
 *	safely be freed.
 *-----------------------------------------------------------------------
 */
STATIC Var *
VarAdd (
    string_t       name,	/* name of variable to add */
    string_t       val,		/* value to set it to */
    GNode          *ctxt)	/* context in which to set it */
{
    register Var   *v;

    v = (Var *) emalloc (sizeof (Var));

    v->name = string_ref (name);
    v->val = string_ref (val);
    v->flags = 0;
    v->where = NULL;

    VarSet(ctxt, v);
    if (DEBUG(VAR)) {
	printf("%s:%s = %s\n", ctxt->name->data, name->data, val->data);
    }
    return(v);
}

/*-
 *-----------------------------------------------------------------------
 * Var_Delete --
 *	Remove a variable from a context.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	The Var structure is removed and freed.
 *
 *-----------------------------------------------------------------------
 */
void
Var_Delete(
    string_t   	  name,
    GNode	  *ctxt)
{

    if (DEBUG(VAR)) {
	printf("%s:delete %s\n", ctxt->name->data, name->data);
    }
    VarUnset(name, ctxt);
}

/*-
 *-----------------------------------------------------------------------
 * Var_Set --
 *	Set the variable name to the value val in the given context.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	If the variable doesn't yet exist, a new record is created for it.
 *	Else the old value is freed and the new one stuck in its place
 *
 * Notes:
 *	The variable is searched for only in its context before being
 *	created in that context. I.e. if the context is VAR_GLOBAL,
 *	only VAR_GLOBAL->context is searched. Likewise if it is VAR_CMD, only
 *	VAR_CMD->context is searched. This is done to avoid the literally
 *	thousands of unnecessary strcmp's that used to be done to
 *	set, say, $(@) or $(<).
 *-----------------------------------------------------------------------
 */
void
Var_Set (
    string_t	name,	/* name of variable to set */
    string_t	val,	/* value to give to the variable */
    GNode	*ctxt)	/* context in which to set it */
{
    register Var   *v;

    /*
     * We only look for a variable in the given context since anything set
     * here will override anything in a lower context, so there's not much
     * point in searching them all just to save a bit of memory...
     */
    v = VarFind (name, ctxt, 0);
    if (v == (Var *) NULL) {
	v = VarAdd (name, val, ctxt);
    } else {
	if (val != v->val) {
	    string_deref(v->val);
	    v->val = string_ref(val);
	}

	if (DEBUG(VAR)) {
	    printf("%s:%s = %s\n", ctxt->name->data, name->data, val->data);
	}
    }
    /*
     * Any variables given on the command line are automatically exported
     * to the environment (as per POSIX standard)
     */
    if (ctxt == VAR_CMD) {
	setenv(name->data, val->data, 1);
    }
    /*
     * record where this variable got set for makefile debugging
     */
    if (ctxt == VAR_CMD)
	v->where = string_ref(v->name);
    else if (v->flags & VAR_FROM_ENV)
	v->where = string_ref(VAR_ENV->name);
    else {
	extern string_t fname;     /* name of current makefile; see parse.c */

	if (fname == NULL)
	    /*
	     * misc compiled-in or MAKECONF variable
	     */
	    v->where = NULL;
	else
	    v->where = string_ref(fname);
    }
}

/*-
 *-----------------------------------------------------------------------
 * Var_Append --
 *	The variable of the given name has the given value appended to it in
 *	the given context.
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	If the variable doesn't exist, it is created. Else the strings
 *	are concatenated (with a space in between).
 *
 * Notes:
 *	Only if the variable is being sought in the global context is the
 *	environment searched.
 *	XXX: Knows its calling circumstances in that if called with ctxt
 *	an actual target, it will only search that context since only
 *	a local variable could be being appended to. This is actually
 *	a big win and must be tolerated.
 *-----------------------------------------------------------------------
 */
void
Var_Append (
    string_t       name,	/* Name of variable to modify */
    string_t	   val,		/* String to append to it */
    GNode          *ctxt)	/* Context in which this should occur */
{
    register Var   *v;
    string_t	   oval;

    v = VarFind (name, ctxt, (ctxt == VAR_GLOBAL) ? FIND_ENV : 0);

    if (v == (Var *) NULL) {
	(void) VarAdd (name, val, ctxt);
    } else {
	v->val = string_concat(oval = v->val, val, STR_ADDSPACE);
	string_deref(oval);
	if (DEBUG(VAR)) {
	    printf("%s:%s = %s\n", ctxt->name->data, name->data,
		   v->val->data);
	}

	if (v->flags & VAR_FROM_ENV) {
	    /*
	     * If the original variable came from the environment, we
	     * have to install it in the global context (we could place
	     * it in the environment, but then we should provide a way to
	     * export other variables...)
	     */
	    v->flags &= ~VAR_FROM_ENV;
	    VarSet(ctxt, v);
	}
    }
}

/*-
 *-----------------------------------------------------------------------
 * Var_Exists --
 *	See if the given variable exists.
 *
 * Results:
 *	TRUE if it does, FALSE if it doesn't
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
Boolean
Var_Exists(
    string_t	  name,    	/* Variable to find */
    GNode	  *ctxt)    	/* Context in which to start search */
{
    Var	    	  *v;

    v = VarFind(name, ctxt, FIND_CMD|FIND_GLOBAL|FIND_ENV);

    return ((Boolean)(v != (Var *)NULL));
}

/*-
 *-----------------------------------------------------------------------
 * Var_Value --
 *	Return the value of the named variable in the given context
 *
 * Results:
 *	The value if the variable exists, NULL if it doesn't
 *
 * Side Effects:
 *	None
 *-----------------------------------------------------------------------
 */
const char *
Var_Value (
    string_t       name,	/* name to find */
    GNode          *ctxt)	/* context in which to search for it */
{
    Var            *v;

    v = VarFind (name, ctxt, FIND_ENV | FIND_GLOBAL | FIND_CMD);
    if (v != (Var *) NULL) {
	return (v->val->data);
    } else {
	return ((char *) NULL);
    }
}

/*-
 *-----------------------------------------------------------------------
 * Var_StrValue --
 *	Return the value of the named variable in the given context
 *
 * Results:
 *	The value if the variable exists, NULL if it doesn't
 *
 * Side Effects:
 *	None
 *-----------------------------------------------------------------------
 */
string_t
Var_StrValue (
    string_t       name,	/* name to find */
    GNode          *ctxt)	/* context in which to search for it */
{
    Var            *v;

    v = VarFind (name, ctxt, FIND_ENV | FIND_GLOBAL | FIND_CMD);
    if (v != (Var *) NULL) {
	return (v->val);
    } else {
	return ((string_t) NULL);
    }
}

typedef struct {
    GNode   	*ctxt;    	/* The context for the variable */
    Boolean	err;    	/* TRUE if undefined variables are an error */
    Var	    	*valVar;	/* Variable to set to the current word */
    const char 	*newVal;	/* String to evaluate */
    int	    	len;		/* Length of newVal string */
} VarIndir;

/*-
 *-----------------------------------------------------------------------
 * VarIndirect --
 *	Perform a substitution on the given word, placing the
 *	result in the passed buffer.
 *
 * Results:
 *	TRUE if a space is needed before more characters are added.
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
STATIC Boolean
VarIndirect (
    const char		*word,		/* Word to modify */
    Boolean 	  	addSpace,	/* True if space should be added before
					 * other characters */
    Buffer  	  	buf,		/* Buffer for result */
    ClientData		indirDataCD)	/* Data for the indirection */
{
    register VarIndir	*indirData = (VarIndir *)indirDataCD;
    register const char	*cp;	/* General pointer */
    register Var	*valVar; /* Our indirect variable */
    int			len;
    Boolean needSpace = addSpace;

    if (buf != (Buffer) NULL) {
	valVar = indirData->valVar;
	if (indirData->len != -1)
	    string_deref(valVar->val);
	valVar->val = string_create(word);
    }
    for (cp = indirData->newVal; *cp != '@' && *cp != '\0'; cp++) {
	if (*cp != '$' || *cp == '\\') {
	    if (*cp == '\\')
		cp++;
	    if (buf == (Buffer)NULL)
		continue;
	    if (needSpace) {
		Buf_AddByte (buf, (Byte)' ');
		needSpace = FALSE;
	    }
	    addSpace = TRUE;
	    Buf_AddByte(buf, (Byte)*cp);
	    continue;
	}

	/*
	 * If unescaped dollar sign, assume it's a variable
	 * substitution and recurse.
	 */
	if (buf != (Buffer) NULL) {
	    if (needSpace) {
		Buf_AddByte (buf, (Byte)' ');
		needSpace = FALSE;
	    }
	    addSpace = TRUE;
	}
	VarParse(buf, cp, indirData->ctxt, indirData->err,
		    &len);
	cp += len - 1;
    }
    /*
     * Special case for the first time we are called.  We could not
     * determine the length of the replacement string until we were
     * called since we did not have a value for the indirection
     * variable.  Now we can use the first word for that value and
     * remember how many bytes of the newVal string were consumed
     * during the expansion.  We will then use that knowledge during
     * subsequent calls.
     */
    if (indirData->len == -1)
	indirData->len = cp - indirData->newVal;
    return(addSpace);
}

/*-
 *-----------------------------------------------------------------------
 * VarHead --
 *	Remove the tail of the given word and place the result in the given
 *	buffer.
 *
 * Results:
 *	TRUE if characters were added to the buffer (a space needs to be
 *	added to the buffer before the next word).
 *
 * Side Effects:
 *	The trimmed word is added to the buffer.
 *
 *-----------------------------------------------------------------------
 */
STATIC Boolean
VarHead (
    const char 	  *word,    	/* Word to trim */
    Boolean 	  addSpace, 	/* True if need to add a space to the buffer
				 * before sticking in the head */
    Buffer  	  buf,	    	/* Buffer in which to store it */
    ClientData	  unused)	/* Unused client data */
{
    register char *slash;

    slash = strrchr (word, '/');
    if (slash != (char *)NULL) {
	if (addSpace) {
	    Buf_AddByte (buf, (Byte)' ');
	}
	*slash = '\0';
	Buf_AddBytes (buf, strlen (word), (Byte *)word);
	*slash = '/';
	return (TRUE);
    } else {
	/*
	 * If no directory part, give . (q.v. the POSIX standard)
	 */
	if (addSpace) {
	    Buf_AddBytes(buf, 2, (Byte *)" .");
	} else {
	    Buf_AddByte(buf, (Byte)'.');
	}
	return(TRUE);
    }
}

/*-
 *-----------------------------------------------------------------------
 * VarTail --
 *	Remove the head of the given word and place the result in the given
 *	buffer.
 *
 * Results:
 *	TRUE if characters were added to the buffer (a space needs to be
 *	added to the buffer before the next word).
 *
 * Side Effects:
 *	The trimmed word is added to the buffer.
 *
 *-----------------------------------------------------------------------
 */
STATIC Boolean
VarTail (
    const char 	  *word,    	/* Word to trim */
    Boolean 	  addSpace, 	/* TRUE if need to stick a space in the
				 * buffer before adding the tail */
    Buffer  	  buf,	    	/* Buffer in which to store it */
    ClientData	  unused)	/* Unused client data */
{
    register char *slash;

    if (addSpace) {
	Buf_AddByte (buf, (Byte)' ');
    }

    slash = strrchr (word, '/');
    if (slash != (char *)NULL) {
	*slash++ = '\0';
	Buf_AddBytes (buf, strlen(slash), (Byte *)slash);
	slash[-1] = '/';
    } else {
	Buf_AddBytes (buf, strlen(word), (Byte *)word);
    }
    return (TRUE);
}

/*-
 *-----------------------------------------------------------------------
 * VarSuffix --
 *	Place the suffix of the given word in the given buffer.
 *
 * Results:
 *	TRUE if characters were added to the buffer (a space needs to be
 *	added to the buffer before the next word).
 *
 * Side Effects:
 *	The suffix from the word is placed in the buffer.
 *
 *-----------------------------------------------------------------------
 */
STATIC Boolean
VarSuffix (
    const char 	  *word,    	/* Word to trim */
    Boolean 	  addSpace, 	/* TRUE if need to add a space before placing
				 * the suffix in the buffer */
    Buffer  	  buf,	    	/* Buffer in which to store it */
    ClientData	  unused)	/* Unused client data */
{
    register char *dot;

    dot = strrchr (word, '.');
    if (dot != (char *)NULL) {
	if (addSpace) {
	    Buf_AddByte (buf, (Byte)' ');
	}
	*dot++ = '\0';
	Buf_AddBytes (buf, strlen (dot), (Byte *)dot);
	dot[-1] = '.';
	return (TRUE);
    } else {
	return (addSpace);
    }
}

/*-
 *-----------------------------------------------------------------------
 * VarRoot --
 *	Remove the suffix of the given word and place the result in the
 *	buffer.
 *
 * Results:
 *	TRUE if characters were added to the buffer (a space needs to be
 *	added to the buffer before the next word).
 *
 * Side Effects:
 *	The trimmed word is added to the buffer.
 *
 *-----------------------------------------------------------------------
 */
STATIC Boolean
VarRoot (
    const char 	  *word,    	/* Word to trim */
    Boolean 	  addSpace, 	/* TRUE if need to add a space to the buffer
				 * before placing the root in it */
    Buffer  	  buf,	    	/* Buffer in which to store it */
    ClientData	  unused)	/* Unused client data */
{
    register char *dot;

    if (addSpace) {
	Buf_AddByte (buf, (Byte)' ');
    }

    dot = strrchr (word, '.');
    if (dot != (char *)NULL) {
	*dot = '\0';
	Buf_AddBytes (buf, strlen (word), (Byte *)word);
	*dot = '.';
    } else {
	Buf_AddBytes (buf, strlen(word), (Byte *)word);
    }
    return (TRUE);
}

/*-
 *-----------------------------------------------------------------------
 * VarMatch --
 *	Place the word in the buffer if it matches the given pattern.
 *	Callback function for VarModify to implement the :M modifier.
 *	
 * Results:
 *	TRUE if a space should be placed in the buffer before the next
 *	word.
 *
 * Side Effects:
 *	The word may be copied to the buffer.
 *
 *-----------------------------------------------------------------------
 */
STATIC Boolean
VarMatch (
    const char 	  *word,    	/* Word to examine */
    Boolean 	  addSpace, 	/* TRUE if need to add a space to the
				 * buffer before adding the word, if it
				 * matches */
    Buffer  	  buf,	    	/* Buffer in which to store it */
    ClientData	  pattern)	/* Pattern the word must match */
{
    if (Str_Match(word, (char *)pattern)) {
	if (addSpace) {
	    Buf_AddByte(buf, (Byte)' ');
	}
	addSpace = TRUE;
	Buf_AddBytes(buf, strlen(word), (Byte *)word);
    }
    return(addSpace);
}

/*-
 *-----------------------------------------------------------------------
 * VarNoMatch --
 *	Place the word in the buffer if it doesn't match the given pattern.
 *	Callback function for VarModify to implement the :N modifier.
 *	
 * Results:
 *	TRUE if a space should be placed in the buffer before the next
 *	word.
 *
 * Side Effects:
 *	The word may be copied to the buffer.
 *
 *-----------------------------------------------------------------------
 */
STATIC Boolean
VarNoMatch (
    const char    *word,    	/* Word to examine */
    Boolean 	  addSpace, 	/* TRUE if need to add a space to the
				 * buffer before adding the word, if it
				 * matches */
    Buffer  	  buf,	    	/* Buffer in which to store it */
    ClientData 	  pattern) 	/* Pattern the word must match */
{
    if (!Str_Match(word, (char *)pattern)) {
	if (addSpace) {
	    Buf_AddByte(buf, (Byte)' ');
	}
	addSpace = TRUE;
	Buf_AddBytes(buf, strlen(word), (Byte *)word);
    }
    return(addSpace);
}

typedef struct {
    char    	  *lhs;	    /* String to match */
    int	    	  leftLen;  /* Length of string */
    char    	  *rhs;	    /* Replacement string (w/ &'s removed) */
    int	    	  rightLen; /* Length of replacement */
    int	    	  flags;
#define VAR_SUB_GLOBAL	1   /* Apply substitution globally */
#define VAR_MATCH_START	2   /* Match at start of word */
#define VAR_MATCH_END	4   /* Match at end of word */
#define VAR_NO_SUB	8   /* Substitution is non-global and already done */
} VarPattern;

/*-
 *-----------------------------------------------------------------------
 * VarSubstitute --
 *	Perform a string-substitution on the given word, placing the
 *	result in the passed buffer.
 *
 * Results:
 *	TRUE if a space is needed before more characters are added.
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
STATIC Boolean
VarSubstitute (
    const char 	  	*word,	    /* Word to modify */
    Boolean 	  	addSpace,   /* True if space should be added before
				     * other characters */
    Buffer  	  	buf,	    /* Buffer for result */
    ClientData	  	patternCD)  /* Pattern for substitution */
{
    register VarPattern	*pattern = (VarPattern *)patternCD;
    register int  	wordLen;    /* Length of word */
    register const char	*cp;	    /* General pointer */

    wordLen = strlen(word);
    if ((pattern->flags & VAR_NO_SUB) == 0) {
	/*
	 * Still substituting -- break it down into simple anchored cases
	 * and if none of them fits, perform the general substitution case.
	 */
	if ((pattern->flags & VAR_MATCH_START) &&
	    (strncmp(word, pattern->lhs, pattern->leftLen) == 0)) {
		/*
		 * Anchored at start and beginning of word matches pattern
		 */
		if ((pattern->flags & VAR_MATCH_END) &&
		    (wordLen == pattern->leftLen)) {
			/*
			 * Also anchored at end and matches to the end (word
			 * is same length as pattern) add space and rhs only
			 * if rhs is non-null.
			 */
			if (pattern->rightLen != 0) {
			    if (addSpace) {
				Buf_AddByte(buf, (Byte)' ');
			    }
			    addSpace = TRUE;
			    Buf_AddBytes(buf, pattern->rightLen,
					 (Byte *)pattern->rhs);
			}
		} else if (pattern->flags & VAR_MATCH_END) {
		    /*
		     * Doesn't match to end -- copy word wholesale
		     */
		    goto nosub;
		} else {
		    /*
		     * Matches at start but need to copy in trailing characters
		     */
		    if ((pattern->rightLen + wordLen - pattern->leftLen) != 0){
			if (addSpace) {
			    Buf_AddByte(buf, (Byte)' ');
			}
			addSpace = TRUE;
		    }
		    Buf_AddBytes(buf, pattern->rightLen, (Byte *)pattern->rhs);
		    Buf_AddBytes(buf, wordLen - pattern->leftLen,
				 (Byte *)(word + pattern->leftLen));
		}
	} else if (pattern->flags & VAR_MATCH_START) {
	    /*
	     * Had to match at start of word and didn't -- copy whole word.
	     */
	    goto nosub;
	} else if (pattern->flags & VAR_MATCH_END) {
	    /*
	     * Anchored at end, Find only place match could occur (leftLen
	     * characters from the end of the word) and see if it does. Note
	     * that because the $ will be left at the end of the lhs, we have
	     * to use strncmp.
	     */
	    cp = word + (wordLen - pattern->leftLen);
	    if ((cp >= word) &&
		(strncmp(cp, pattern->lhs, pattern->leftLen) == 0)) {
		/*
		 * Match found. If we will place characters in the buffer,
		 * add a space before hand as indicated by addSpace, then
		 * stuff in the initial, unmatched part of the word followed
		 * by the right-hand-side.
		 */
		if (((cp - word) + pattern->rightLen) != 0) {
		    if (addSpace) {
			Buf_AddByte(buf, (Byte)' ');
		    }
		    addSpace = TRUE;
		}
		Buf_AddBytes(buf, cp - word, (Byte *)word);
		Buf_AddBytes(buf, pattern->rightLen, (Byte *)pattern->rhs);
	    } else {
		/*
		 * Had to match at end and didn't. Copy entire word.
		 */
		goto nosub;
	    }
	} else {
	    /*
	     * Pattern is unanchored: search for the pattern in the word using
	     * String_FindSubstring, copying unmatched portions and the
	     * right-hand-side for each match found, handling non-global
	     * subsititutions correctly, etc. When the loop is done, any
	     * remaining part of the word (word and wordLen are adjusted
	     * accordingly through the loop) is copied straight into the
	     * buffer.
	     * addSpace is set FALSE as soon as a space is added to the
	     * buffer.
	     */
	    register Boolean done;
	    int origSize;

	    done = FALSE;
	    origSize = Buf_Size(buf);
	    while (!done) {
		cp = strstr(word, pattern->lhs);
		if (cp != (char *)NULL) {
		    if (addSpace && (((cp - word) + pattern->rightLen) != 0)){
			Buf_AddByte(buf, (Byte)' ');
			addSpace = FALSE;
		    }
		    Buf_AddBytes(buf, cp-word, (Byte *)word);
		    Buf_AddBytes(buf, pattern->rightLen, (Byte *)pattern->rhs);
		    wordLen -= (cp - word) + pattern->leftLen;
		    word = cp + pattern->leftLen;
		    if (wordLen == 0) {
			done = TRUE;
		    }
		    if ((pattern->flags & VAR_SUB_GLOBAL) == 0) {
			done = TRUE;
			pattern->flags |= VAR_NO_SUB;
		    }
		} else {
		    done = TRUE;
		}
	    }
	    if (wordLen != 0) {
		if (addSpace) {
		    Buf_AddByte(buf, (Byte)' ');
		}
		Buf_AddBytes(buf, wordLen, (Byte *)word);
	    }
	    /*
	     * If added characters to the buffer, need to add a space
	     * before we add any more. If we didn't add any, just return
	     * the previous value of addSpace.
	     */
	    return ((Buf_Size(buf) != origSize) || addSpace);
	}
	/*
	 * Common code for anchored substitutions: if performed a substitution
	 * and it's not supposed to be global, mark the pattern as requiring
	 * no more substitutions. addSpace was set TRUE if characters were
	 * added to the buffer.
	 */
	if ((pattern->flags & VAR_SUB_GLOBAL) == 0) {
	    pattern->flags |= VAR_NO_SUB;
	}
	return (addSpace);
    }
 nosub:
    if (addSpace) {
	Buf_AddByte(buf, (Byte)' ');
    }
    Buf_AddBytes(buf, wordLen, (Byte *)word);
    return(TRUE);
}

/*-
 *-----------------------------------------------------------------------
 * VarModify --
 *	Modify each of the words of the passed string using the given
 *	function. Used to implement all modifiers.
 *
 * Results:
 *	All the words modified appropriately in valBuf.
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
STATIC void
VarModify(
    char	  *str,		    /* String whose words should be trimmed */
    Buffer	  valBuf,	    /* Buffer for the value */
    Boolean    	  (*modProc)(const char *, Boolean, Buffer, ClientData),
				    /* Function to use to modify them */
    ClientData	  datum)    	    /* Datum to pass it */
{
    char	  *cp;	    	    /* Pointer to end of current word */
    char    	  endc;	    	    /* Character that ended the word */
    Boolean 	  addSpace; 	    /* TRUE if need to add a space to the
				     * buffer before adding the trimmed
				     * word */
    
    cp = str;
    addSpace = FALSE;
    
    while (1) {
	/*
	 * Skip to next word and place cp at its end.
	 */
	while (*str == ' ' || *str == '\t')
	    str++;
	cp = str;
	while (*cp != '\0' && *cp != ' ' && *cp != '\t')
	    cp++;
	if (cp == str) {
	    /*
	     * If we didn't go anywhere, we must be done!
	     */
	    return;
	}
	/*
	 * Nuke terminating character, but save it in endc b/c if str was
	 * some variable's value, it would not be good to screw it
	 * over...
	 */
	endc = *cp;
	*cp = '\0';

	addSpace = (* modProc)(str, addSpace, valBuf, datum);

	if (endc)
	    *cp++ = endc;
	str = cp;
    }
}

/*-
 *-----------------------------------------------------------------------
 * VarApplyModifiers --
 *
 * Results:
 *
 *
 *    Original RENO modifiers are:
 *  	  :M<pattern>	words which match the given <pattern>.
 *  	  	    	<pattern> is of the standard file
 *  	  	    	wildcarding form.
 *  	  :N<pattern>	words which do not match the given <pattern>.
 *  	  :S<d><pat1><d><pat2><d>[g]
 *  	  	    	Substitute <pat2> for <pat1> in the value
 *  	  :H	    	Substitute the head of each word
 *  	  :T	    	Substitute the tail of each word
 *  	  :E	    	Substitute the extension (minus '.') of
 *  	  	    	each word
 *  	  :R	    	Substitute the root of each word
 *  	  	    	(pathname minus the suffix).
 *    	  :lhs=rhs  	Like :S, but the rhs goes to the end of
 *    	    	    	the invocation.
 * New modifiers:
 *	  :@<tmpvar>@<newval>@
 *			Assign a temporary local variable <tmpvar>
 *			to the current value of each word in turn
 *			and replace each word with the result of
 *			evaluating <newval>
 *	  :D<newval>	Use <newval> as value if variable defined
 *	  :U<newval>	Use <newval> as value if variable undefined
 *	  :L		Use the name of the variable as the value.
 *	  :P		Use the path of the node that has the same
 *			name as the variable as the value.  This
 *			basically includes an implied :L so that
 *			the common method of refering to the path
 *			of your dependent 'x' in a rule is to use
 *			the form '${x:P}'.
 *	  :!<cmd>!	If the value is currently non-empty and
 *			contains non-whitespace characters, then
 *			use the result of running cmd through the
 *			system(3) call.  The optimization of not
 *			actually running a shell if there are no
 *			shell meta characters is valid.
 *
 * Side Effects:
 *
 *-----------------------------------------------------------------------
 */
STATIC int
VarApplyModifiers(const char **tstrp,
		  Buffer valBuf,
		  Var *v,
		  GNode *ctxt,
		  Boolean err,
		  int endc)
{
    const char	*tstr;
    char	*str;
    const char	*cp;

    tstr = *tstrp;
    cp = tstr + 1;
    if (valBuf != (Buffer) NULL) {
	str = (char *) Buf_GetBase(valBuf);
	if (DEBUG(VAR))
	    printf("Applying :%c to \"%s\"\n", *tstr, str);
    } else {
	str = (char *) NULL;
	if (DEBUG(VAR))
	    printf("Skipping :%c\n", *tstr);
    }
    switch (*tstr) {
    case '@':
    {
	VarIndir	indirData;
	char		*cp2;
	string_t	s;

	while (*cp != '@') {
	    if (*cp == '\0') {
		Error("Unclosed variable specification %s", tstr);
		*tstrp = cp;
		return(1);
	    }
	    cp++;
	}
	if (valBuf == (Buffer) NULL) {
	    for (cp++; *cp != '@'; cp++) {
		int len;

		if (*cp != '$') {
		    if (*cp == '\\')
			cp++;
		    if (*cp == '\0') {
			Error("Unclosed variable specification %s", tstr);
			*tstrp = cp;
			return(1);
		    }
		    continue;
		}
		VarParse((Buffer) NULL, cp, ctxt, err, &len);
		cp += len - 1;
	    }
	    *tstrp = ++cp;
	    if (*cp != ':' && *cp != endc)
		return(1);
	    break;
	}
	cp2 = strndup(tstr + 1, cp - (tstr + 1));
	s = string_create(cp2);
	free(cp2);
	indirData.valVar = VarAdd(s, sNULL, ctxt);
	string_deref(s);
	ctxt->contextExtras++;
	indirData.ctxt = ctxt;
	indirData.err = err;
	indirData.newVal = ++cp;
	indirData.len = -1;

	str = strndup(str, Buf_Size(valBuf));
	Buf_Discard(valBuf, Buf_Size(valBuf));
	VarModify(str, valBuf, VarIndirect, (ClientData)&indirData);
	free(str);

	/*
	 * Need a special call if str is empty to get newval len
	 */
	if (indirData.len == -1)
	    VarIndirect("", 0, (Buffer)0, &indirData);

	VarUnset(indirData.valVar->name, ctxt);
	ctxt->contextExtras--;
	cp += indirData.len;
	if (*cp == '\0') {
	    Error("Unclosed variable specification %s", tstr);
	    *tstrp = cp;
	    return(1);
	}
	*tstrp = ++cp;
	if (*cp != ':' && *cp != endc)
	    return(1);
	break;
    }
    case 'D':
    case 'U':
    {
	int	wantit;		/* want data in buffer */

	if (valBuf == (Buffer) NULL)
	    wantit = FALSE;
	else {
	    if (*tstr == 'U')
		wantit = ((v->flags & VAR_JUNK) != 0);
	    else
		wantit = ((v->flags & VAR_JUNK) == 0);
	    if (v->flags & VAR_JUNK)
		v->flags |= VAR_KEEP;

	    if (wantit)
		Buf_Discard(valBuf, Buf_Size(valBuf));
	}

	/*
	 * Pass through tstr looking for 1) escaped delimiters,
	 * '$'s and backslashes (place the escaped character in
	 * uninterpreted) and 2) unescaped $'s that aren't before
	 * the delimiter (expand the variable substitution).
	 * The result is left in the Buffer buf.
	 */
	for (; *cp != endc && *cp != ':'; cp++) {
	    int len;

	    if (*cp != '$') {
		if (*cp == '\\')
		    cp++;
		if (*cp == '\0') {
		    *tstrp = cp;
		    Error("Unclosed variable specification %s", tstr);
		    return(1);
		}
		if (wantit)
		    Buf_AddByte(valBuf, (Byte)*cp);
		continue;
	    }
	    /*
	     * If unescaped dollar sign, assume it's a
	     * variable substitution and recurse.
	     */
	    VarParse(wantit ? valBuf : (Buffer) NULL, cp, ctxt, err, &len);
	    cp += len - 1;
	}
	*tstrp = cp;
	if (!wantit)
	    break;
	break;
    }
    case 'L':
    {
	*tstrp = cp;
	if (*cp != ':' && *cp != endc)
	    return(1);
	if (valBuf == (Buffer) NULL)
	    break;
	if (v->flags & VAR_JUNK)
	    v->flags |= VAR_KEEP;
	Buf_Discard(valBuf, Buf_Size(valBuf));
	Buf_AddBytes(valBuf, v->name->len, (Byte *) v->name->data);
	break;
    }
    case 'P':
    {
	GNode *gn;

	*tstrp = cp;
	if (*cp != ':' && *cp != endc)
	    return(1);
	if (valBuf == (Buffer) NULL)
	    break;
	if (v->flags & VAR_JUNK)
	    v->flags |= VAR_KEEP;
	Buf_Discard(valBuf, Buf_Size(valBuf));
	gn = Targ_FindNode(v->name, TARG_NOCREATE);
	if (gn == NILGNODE || gn->path == NULL)
	    Buf_AddBytes(valBuf, v->name->len, (Byte *) v->name->data);
	else
	    Buf_AddBytes(valBuf, gn->path->len, (Byte *) gn->path->data);
	break;
    }
    case '!':
    {
	Buffer cmdBuf;

	if (valBuf != (Buffer) NULL)
	    cmdBuf = Buf_Init(Buf_Size(valBuf) + 1);
	else
	    cmdBuf = (Buffer) NULL; /* LINT */

	/*
	 * Pass through tstr looking for 1) escaped delimiters,
	 * '$'s and backslashes (place the escaped character in
	 * uninterpreted) and 2) unescaped $'s that aren't before
	 * the delimiter (expand the variable substitution).
	 * The result is left in the Buffer buf.
	 */
	for (; *cp != '!'; cp++) {
	    int len;

	    if (*cp != '$') {
		if (*cp == '\\')
		    cp++;
		if (*cp == '\0') {
		    *tstrp = cp;
		    Error("Unclosed variable specification %s", tstr);
		    if (valBuf != (Buffer) NULL)
			Buf_Destroy(cmdBuf);
		    return(1);
		}
		if (valBuf != (Buffer) NULL)
		    Buf_AddByte(cmdBuf, (Byte)*cp);
		continue;
	    }
	    /*
	     * If unescaped dollar sign, assume it's a
	     * variable substitution and recurse.
	     */
	    VarParse(cmdBuf, cp, ctxt, err, &len);
	    cp += len - 1;
	}
	*tstrp = ++cp;
	if (*cp != ':' && *cp != endc)
	    return(1);
	if (valBuf == (Buffer) NULL)
	    break;
	Buf_Discard(valBuf, Buf_Size(valBuf));
	VarRunShellCmd(valBuf, cmdBuf);
	Buf_Destroy(cmdBuf);
	break;
    }
    case 'N':
    case 'M':
    {
	Buffer	patBuf;

	if (valBuf != (Buffer) NULL)
	    patBuf = Buf_Init(0);
	else
	    patBuf = (Buffer) NULL;

	for (; *cp != endc && *cp != ':'; cp++) {
	    int len;

	    if (*cp != '$') {
		if (*cp == '\\')
		    cp++;
		if (*cp == '\0') {
		    *tstrp = cp;
		    Error("Unclosed variable specification %s", tstr);
		    if (valBuf != (Buffer) NULL)
			Buf_Destroy(patBuf);
		    return(1);
		}
		if (valBuf != (Buffer) NULL)
		    Buf_AddByte(patBuf, (Byte)*cp);
		continue;
	    }

	    /*
	     * If unescaped dollar sign, assume it's a
	     * variable substitution and recurse.
	     */
	    VarParse(patBuf, cp, ctxt, err, &len);
	    cp += len - 1;
	}
	*tstrp = cp;
	if (valBuf == (Buffer) NULL)
	    break;
	str = strndup(str, Buf_Size(valBuf));
	Buf_Discard(valBuf, Buf_Size(valBuf));
	VarModify(str, valBuf, (*tstr == 'M') ? VarMatch : VarNoMatch,
		  (ClientData)Buf_GetBase(patBuf));
	free(str);
	Buf_Destroy(patBuf);
	break;
    }
    case 'S':
    {
	VarPattern	pattern;
	register char	delim;
	Buffer		patBuf;    	/* Buffer for patterns */

	pattern.flags = 0;
	delim = *cp;
	if (delim == '\0') {
	    *tstrp = cp;
	    return(1);
	}
	/*
	 * If pattern begins with '^', it is anchored to the
	 * start of the word -- skip over it and flag pattern.
	 */
	if (cp[1] == '^') {
	    pattern.flags |= VAR_MATCH_START;
	    cp++;
	}

	if (valBuf != (Buffer) NULL)
	    patBuf = Buf_Init(0);
	else
	    patBuf = (Buffer) NULL;

	/*
	 * Pass through the lhs looking for 1) escaped delimiters,
	 * '$'s and backslashes (place the escaped character in
	 * uninterpreted) and 2) unescaped $'s that aren't before
	 * the delimiter (expand the variable substitution).
	 * The result is left in the Buffer buf.
	 */
	for (cp++; *cp != delim; cp++) {
	    int	    len;

	    if (*cp != '$') {
		if (*cp == '\\')
		    cp++;
		if (*cp == '\0') {
		    *tstrp = cp;
		    if (valBuf != (Buffer) NULL)
			Buf_Destroy(patBuf);
		    return(1);
		}
		if (valBuf != (Buffer) NULL)
		    Buf_AddByte(patBuf, (Byte)*cp);
		continue;
	    }
	    if (cp[1] == delim) {
		/*
		 * Unescaped $ at end of pattern => anchor
		 * pattern at end.
		 */
		pattern.flags |= VAR_MATCH_END;
		cp++;
		break;
	    }
	    /*
	     * If unescaped dollar sign not before the
	     * delimiter, assume it's a variable
	     * substitution and recurse.
	     */
	    VarParse(patBuf, cp, ctxt, err, &len);
	    cp += len - 1;
	}

	/*
	 * Fetch pattern and destroy buffer, but preserve the data
	 * in it, since that's our lhs.
	 */
	if (valBuf != (Buffer) NULL) {
	    pattern.leftLen = Buf_Size(patBuf);
	    Buf_AddByte(patBuf, (Byte) '\0');
	}

	/*
	 * Now comes the replacement string. Three things need to
	 * be done here: 1) need to compress escaped delimiters and
	 * ampersands and 2) need to replace unescaped ampersands
	 * with the l.h.s. (since this isn't regexp, we can do
	 * it right here) and 3) expand any variable substitutions.
	 */
	for (cp++; *cp != delim; cp++) {
	    int	    len;

	    if (*cp == '&') {
		char *cp2;

		if (valBuf == (Buffer) NULL)
		    continue;
		cp2 = (char *)Buf_GetBase(patBuf);
		cp2 = strndup(cp2, pattern.leftLen);
		Buf_AddBytes(patBuf, pattern.leftLen, (Byte *)cp2);
		free(cp2);
		continue;
	    }
	    if (*cp != '$') {
		if (*cp == '\\')
		    cp++;
		if (*cp == '\0') {
		    *tstrp = cp;
		    if (valBuf != (Buffer) NULL)
			Buf_Destroy(patBuf);
		    return(1);
		}
		if (valBuf != (Buffer) NULL)
		    Buf_AddByte(patBuf, (Byte)*cp);
		continue;
	    }
	    if (cp[1] == delim) {
		if (valBuf != (Buffer) NULL)
		    Buf_AddByte(patBuf, (Byte)*cp++);
		break;
	    }
	    /*
	     * If unescaped dollar sign not before the
	     * delimiter, assume it's a variable
	     * substitution and recurse.
	     */
	    VarParse(patBuf, cp, ctxt, err, &len);
	    cp += len - 1;
	}

	if (valBuf != (Buffer) NULL) {
	    pattern.lhs = (char *)Buf_GetBase(patBuf);
	    pattern.rhs = pattern.lhs + pattern.leftLen + 1;
	    pattern.rightLen = Buf_Size(patBuf);
	    pattern.rightLen -= pattern.leftLen + 1;
	}

	/*
	 * Check for global substitution. If 'g' after the final
	 * delimiter, substitution is global and is marked that
	 * way.
	 */
	cp++;
	if (*cp == 'g') {
	    pattern.flags |= VAR_SUB_GLOBAL;
	    cp++;
	}
	*tstrp = cp;
	if (*cp != ':' && *cp != endc) {
	    if (valBuf != (Buffer) NULL)
		Buf_Destroy(patBuf);
	    return(1);
	}
	if (valBuf == (Buffer) NULL)
	    break;
	str = strndup(str, Buf_Size(valBuf));
	Buf_Discard(valBuf, Buf_Size(valBuf));
	VarModify(str, valBuf, VarSubstitute, (ClientData)&pattern);
	free(str);
	Buf_Destroy(patBuf);
	break;
    }
    case 'T':
    {
	*tstrp = cp;
	if (*cp != ':' && *cp != endc)
	    return(1);
	if (valBuf == (Buffer) NULL)
	    break;
	str = strndup(str, Buf_Size(valBuf));
	Buf_Discard(valBuf, Buf_Size(valBuf));
	VarModify(str, valBuf, VarTail, (ClientData)0);
	free(str);
	break;
    }
    case 'H':
    {
	*tstrp = cp;
	if (*cp != ':' && *cp != endc)
	    return(1);
	if (valBuf == (Buffer) NULL)
	    break;
	str = strndup(str, Buf_Size(valBuf));
	Buf_Discard(valBuf, Buf_Size(valBuf));
	VarModify(str, valBuf, VarHead, (ClientData)0);
	free(str);
	break;
    }
    case 'E':
    {
	*tstrp = cp;
	if (*cp != ':' && *cp != endc)
	    return(1);
	if (valBuf == (Buffer) NULL)
	    break;
	str = strndup(str, Buf_Size(valBuf));
	Buf_Discard(valBuf, Buf_Size(valBuf));
	VarModify(str, valBuf, VarSuffix, (ClientData)0);
	free(str);
	break;
    }
    case 'R':
    {
	*tstrp = cp;
	if (*cp != ':' && *cp != endc)
	    return(1);
	if (valBuf == (Buffer) NULL)
	    break;
	str = strndup(str, Buf_Size(valBuf));
	Buf_Discard(valBuf, Buf_Size(valBuf));
	VarModify(str, valBuf, VarRoot, (ClientData)0);
	free(str);
	break;
    }
    default:
    {
	/*
	 * This can either be a bogus modifier or a System-V
	 * substitution command.
	 */
	VarPattern	pattern;
	const char	*eq;

	pattern.flags = 0;
	eq = NULL;
	/*
	 * First we make a pass through the string trying
	 * to verify it is a SYSV-make-style translation:
	 * it must be: <string1>=<string2>)
	 */
	for (cp = tstr; *cp != endc; cp++) {
	    if (*cp == '=') {
		if (eq == NULL)
		    eq = cp;
		continue;		/* continue looking for endc */
	    } else if (*cp == '\0') {
		*tstrp = cp;
		return(1);
	    }
	}
	*tstrp = cp;
	if (eq == NULL) {
	    Error ("Unknown modifier '%c'\n", *tstr);
	    return(1);
	}
	if (valBuf == (Buffer) NULL)
	    break;

	/*
	 * Now we break this sucker into the lhs and rhs.
	 */
	pattern.lhs = strndup(tstr, eq - tstr);
	pattern.leftLen = eq - tstr;
	eq++;
	pattern.rhs = strndup(eq, cp - eq);
	pattern.rightLen = cp - eq;

	/*
	 * SYSV modifications happen through the whole
	 * string. Note the pattern is anchored at the end.
	 */
	pattern.flags |= VAR_SUB_GLOBAL|VAR_MATCH_END;
	str = strndup(str, Buf_Size(valBuf));
	Buf_Discard(valBuf, Buf_Size(valBuf));
	VarModify(str, valBuf, VarSubstitute, (ClientData)&pattern);
	free(str);
	free(pattern.lhs);
	free(pattern.rhs);
	break;
    }
    }
    if (DEBUG(VAR)) {
	if (valBuf != (Buffer) NULL)
	    printf("Result is \"%s\"\n", (char *)Buf_GetBase(valBuf));
	else
	    printf("Result is not wanted\n");
    }
    return(0);
}

/*-
 *-----------------------------------------------------------------------
 * VarParse --
 *	Given the start of a variable invocation, extract the variable
 *	name and find its value, then modify it according to the
 *	specification.  If buf is NULL, we just go through the
 *	motions until we know where the variable invocation ends.
 *
 * Results:
 *	The (possibly-modified) value of the variable or var_Error if the
 *	specification is invalid. The length of the specification is
 *	placed in *lengthPtr (for invalid specifications, this is just
 *	2...?).
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
STATIC int
VarParse (
    Buffer	   buf,		/* The buffer for the result */
    const char	  *str,	    	/* The string to parse */
    GNode   	  *ctxt,    	/* The context for the variable */
    Boolean 	    err,    	/* TRUE if undefined variables are an error */
    int	    	    *lengthPtr)	/* OUT: The length of the specification */
{
    const char	    *tstr;    	/* Pointer into str */
    Var	    	    *v;	    	/* Variable in invocation */
    register char   *cp;    	/* Secondary pointer into str (place marker
				 * for tstr) */
    Boolean 	    haveModifier;/* TRUE if have modifiers for the variable */
    register char   endc;    	/* Ending character when variable in parens
				 * or braces */
    string_t	    name;	/* string for name lookup */
    Buffer	    nameBuf;	/* buffer for variable name */
    Buffer	    valBuf;	/* buffer for variable value */
    int		    initialSize;
    
    if (buf != (Buffer) NULL) {
	nameBuf = Buf_Init(0);
	initialSize = Buf_Size(buf);
    } else {
	nameBuf = (Buffer) NULL;
	initialSize = 0;	/* LINT */
    }

    if (DEBUG(VAR)) {
	printf("VarParse: Parsing '%s'.\n", str);
    }

    tstr = str + 2;
    if (str[1] != '(' && str[1] != '{') {
	/*
	 * If it's not bounded by braces of some sort, life is much simpler.
	 * We just need to check for the first character and return the
	 * value if it exists.
	 */
	if (str[1] == '$') {
	    /*
	     * $$ is a special case for a single literal dollar-sign
	     */
	    *lengthPtr = tstr - str;
	    if (buf != (Buffer) NULL) {
		Buf_Destroy(nameBuf);
		Buf_AddByte(buf, (Byte)'$');
	    }
	    return (0);
	}
	haveModifier = FALSE;
	endc = '\0';		/* LINT */
	if (buf != (Buffer) NULL)
	    Buf_AddByte(nameBuf, (Byte) str[1]);
    } else {
	endc = str[1] == '(' ? ')' : '}';

	/*
	 * Skip to the end character or a colon, whichever comes first.
	 */
	for (; *tstr != endc && *tstr != ':'; tstr++) {
	    int len;

	    if (*tstr != '$') {
		if (*tstr == '\0') {
		    /*
		     * If we never did find the end character, return failure
		     * right now, setting the length to be the distance to
		     * the end of the string, since that's what make does.
		     */
		    *lengthPtr = tstr - str;
		    if (buf != (Buffer) NULL)
			Buf_Destroy(nameBuf);
		    return (1);
		}
		if (buf != (Buffer) NULL)
		    Buf_AddByte(nameBuf, (Byte)*tstr);
		continue;
	    }
	    VarParse(nameBuf, tstr, ctxt, err, &len);
	    tstr += len - 1;
	}
	haveModifier = (*tstr++ == ':');
    }

    if (!haveModifier) {
	*lengthPtr = tstr - str;
	if (buf == (Buffer) NULL) {
	    if (DEBUG(VAR)) {
		printf("VarParse: returning success without data\n");
	    }
	    return(0);
	}
    }

    if (buf != (Buffer) NULL) {
	name = string_create((char *)Buf_GetBase(nameBuf));
	Buf_Destroy(nameBuf);
	v = VarFind (name, ctxt, FIND_ENV | FIND_GLOBAL | FIND_CMD);
	if (v == (Var *)NULL) {
	    if (!haveModifier) {
		if (DEBUG(VAR)) {
		    printf("VarParse: returning failure\n");
		}
		string_deref(name);
		return(1);
	    }
	    /*
	     * Still need to get to the end of the variable specification,
	     * so kludge up a Var structure for the modifications
	     */
	    v = (Var *) emalloc(sizeof(Var));
	    v->name = string_ref(name);
	    v->val = string_ref(sNULL);
	    v->flags = VAR_JUNK;
	}
	if (v->flags & VAR_IN_USE) {
	    Fatal("Variable %s is recursive.", v->name->data);
	    /*NOTREACHED*/
	} else {
	    v->flags |= VAR_IN_USE;
	}
	/*
	 * Before doing any modification, we have to make sure the value
	 * has been fully expanded. If it looks like recursion might be
	 * necessary (there's a dollar sign somewhere in the variable's value)
	 * we just call Var_Subst to do any other substitutions that are
	 * necessary. Note that the value returned by Var_Subst will have
	 * been dynamically-allocated, so it will need freeing when we
	 * return.
	 */
	if (!haveModifier)
	    valBuf = buf;
	else
	    valBuf = Buf_Init(0);
	if (string_hasvar(v->val)) {
	    cp = Var_Subst(v->val->data, ctxt, err);
	    Buf_AddBytes(valBuf, strlen(cp), (Byte *)cp);
	    free(cp);
	} else
	    Buf_AddBytes(valBuf, v->val->len, (Byte *)v->val->data);
	v->flags &= ~VAR_IN_USE;
    } else {
	v = (Var *) NULL;	/* LINT */
	valBuf = (Buffer) NULL;
    }

    /*
     * Now we need to apply any modifiers the user wants applied.
     */
    if (haveModifier) {
	for (;;) {
	    if (VarApplyModifiers(&tstr, valBuf, v, ctxt, err, endc) != 0) {
		Error("Unclosed variable specification - %s", str);
		*lengthPtr = tstr - str;
		if (buf != (Buffer) NULL) {
		    Buf_Destroy(valBuf);
		    if (v->flags & VAR_JUNK) {
			/*
			 * Perform any free'ing needed.
			 */
			string_deref(v->val);
			string_deref(v->name);
			free((Address)v);
		    }
		}
		return(1);
	    }
	    if (*tstr++ == endc)
		break;
	}
	*lengthPtr = tstr - str;
	if (buf == (Buffer) NULL)
	    return (0);
	Buf_AddBytes(buf, Buf_Size(valBuf), Buf_GetBase(valBuf));
	Buf_Destroy(valBuf);
    }

    if (v->flags & VAR_JUNK) {
	/*
	 * Perform any free'ing needed.
	 */
	string_deref(v->val);
	string_deref(v->name);
	free((Address)v);
    }

    if (DEBUG(VAR)) {
	printf("VarParse: returning %s\n",
	       (char *)Buf_GetBase(buf) + initialSize);
    }
    return (0);
}

/*-
 *-----------------------------------------------------------------------
 * Var_Parse --
 *	Given the start of a variable invocation, extract the variable
 *	name and find its value, then modify it according to the
 *	specification.
 *
 * Results:
 *	The (possibly-modified) value of the variable or var_Error if the
 *	specification is invalid. The length of the specification is
 *	placed in *lengthPtr (for invalid specifications, this is just
 *	2...?).
 *	A Boolean in *freePtr telling whether the returned string should
 *	be freed by the caller.
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
char *
Var_Parse (
    const char 	  *str,	    	/* The string to parse */
    GNode   	  *ctxt,    	/* The context for the variable */
    Boolean 	    err,    	/* TRUE if undefined variables are an error */
    int	    	    *lengthPtr,	/* OUT: The length of the specification */
    Boolean 	    *freePtr) 	/* OUT: TRUE if caller should free result */
{
    Buffer buf = Buf_Init(0);
    char *result;

    if (VarParse(buf, str, ctxt, err, lengthPtr) != 0) {
	*freePtr = FALSE;
	Buf_Destroy(buf);
	return(err ? var_Error : varNoError);
    }
    result = (char *) Buf_GetBase(buf);
    result = strndup(result, Buf_Size(buf));
    Buf_Destroy(buf);
    return(result);
}

/*-
 *-----------------------------------------------------------------------
 * Var_Skip --
 *	Given the start of a variable invocation, extract the variable
 *	name and find its value, then modify it according to the
 *	specification.
 *
 * Results:
 *	The (possibly-modified) value of the variable or var_Error if the
 *	specification is invalid. The length of the specification is
 *	placed in *lengthPtr (for invalid specifications, this is just
 *	2...?).
 *	A Boolean in *freePtr telling whether the returned string should
 *	be freed by the caller.
 *
 * Side Effects:
 *	None.
 *
 *-----------------------------------------------------------------------
 */
char *
Var_Skip (
    char    	  *str,	    	/* The string to parse */
    GNode   	  *ctxt,    	/* The context for the variable */
    Boolean 	    err)    	/* TRUE if undefined variables are an error */
{
    int length;

    if (VarParse((Buffer) NULL, str, ctxt, err, &length) != 0)
	printf("Var_Skip: Error\n");
    return(str + length);
}

/*-
 *-----------------------------------------------------------------------
 * Var_Subst  --
 *	Substitute for all variables in the given string in the given context
 *	If undefErr is TRUE, Parse_Error will be called when an undefined
 *	variable is encountered.
 *
 * Results:
 *	The resulting string.
 *
 * Side Effects:
 *	None. The old string must be freed by the caller
 *-----------------------------------------------------------------------
 */
char *
Var_Subst (
    const char	  *str,		    /* the string in which to substitute */
    GNode         *ctxt,	    /* the context wherein to find variables */
    Boolean 	  undefErr) 	    /* TRUE if undefineds are an error */
{
    Buffer  	  buf;	    	    /* Buffer for forming things */
    int	    	  length;   	    /* Length of the variable invocation */
    static Boolean errorReported;   /* Set true if an error has already
				     * been reported to prevent a plethora
				     * of messages when recursing */
    char *result;

    buf = Buf_Init (0);
    errorReported = FALSE;

    while (*str) {
	if ((*str == '$') && (str[1] == '$')) {
	    /*
	     * A dollar sign may be escaped either with another dollar sign.
	     * In such a case, we skip over the escape character and store the
	     * dollar sign into the buffer directly.
	     */
	    str++;
	    Buf_AddByte(buf, (Byte)*str);
	    str++;
	} else if (*str != '$') {
	    /*
	     * Skip as many characters as possible -- either to the end of
	     * the string or to the next dollar sign (variable invocation).
	     */
	    const char  *cp;

	    for (cp = str++; *str != '$' && *str != '\0'; str++) {
		;
	    }
	    Buf_AddBytes(buf, str - cp, (Byte *)cp);
	} else {
	    if (VarParse(buf, str, ctxt, undefErr, &length) != 0) {
		/*
		 * If performing old-time variable substitution, skip over
		 * the variable and continue with the substitution. Otherwise,
		 * store the dollar sign and advance str so we continue with
		 * the string...
		 */
		if (oldVars) {
		    str += length;
		} else if (undefErr) {
		    /*
		     * If variable is undefined, complain and skip the
		     * variable. The complaint will stop us from doing anything
		     * when the file is parsed.
		     */
		    if (!errorReported) {
			Parse_Error (PARSE_FATAL,
				     "Undefined variable \"%.*s\"",length,str);
		    }
		    str += length;
		    errorReported = TRUE;
		} else {
		    Buf_AddByte (buf, (Byte)*str);
		    str += 1;
		}
	    } else {
		/*
		 * We've now got a variable structure to store in. But first,
		 * advance the string pointer.
		 */
		str += length;
	    }
	}
    }
	
    result = (char *)Buf_GetBase(buf);
    result = strndup(result, Buf_Size(buf));
    Buf_Destroy (buf);
    return (result);
}

/*
 * The following array is used to make a fast determination of which
 * characters are interpreted specially by the shell.  If a command
 * contains any of these characters, it is executed by the shell, not
 * directly by us.
 */

STATIC char 	    meta[256];

/*-
 *-----------------------------------------------------------------------
 * Var_Init --
 *	Initialize the module
 *
 * Results:
 *	None
 *
 * Side Effects:
 *	The VAR_CMD and VAR_GLOBAL contexts are created 
 *-----------------------------------------------------------------------
 */
void
Var_Init (void)
{
    char	 *cp, *vp;
    const char	 *p;
    char	 **pp;
    extern char  **environ;
    string_t	   name, val;

    VAR_GLOBAL = Targ_NewGN (string_create("Global"));
    VAR_CMD = Targ_NewGN (string_create("Command"));
    VAR_ENV = Targ_NewGN (string_create("Environment"));
    Hash_InitTable(&cmdHashTable, 32);
    Hash_InitTable(&globalHashTable, 256);
    Hash_InitTable(&envHashTable, 64);
    for (pp = environ; *pp; pp++) {
	cp = strdup(*pp);
	vp = strchr(cp, '=');
	if (vp) {
	    *vp++ = '\0';
	    name = string_create(cp);
	    val = string_create(vp);
	    (void) VarAdd(name, val, VAR_ENV);
	    string_deref(name);
	    string_deref(val);
	}
	(void) free(cp);
    }

    for (p = "#=|^(){};&<>*?[]:$`\\\n"; *p != '\0'; p++)
	meta[(int)*p] = 1;

    /*
     * The null character serves as a sentinel in the string.
     */
    meta[0] = 1;
}

/*-
 *-----------------------------------------------------------------------
 * Var_HasMeta --
 *	Check if string contains a meta character.
 *
 * Results:
 *	0 if there are no meta characters, 1 if there are.
 *
 * Side Effects:
 *	none
 *
 *-----------------------------------------------------------------------
 */
Boolean
Var_HasMeta (const char *cmd)		/* Command to check */
{
    register const char *cp;
    Boolean quote;

    /*
     * Search for meta characters in the command. If there are no meta
     * characters, there's no need to execute a shell to execute the
     * command.
     */
    quote = FALSE;
    for (cp = cmd; *cp; cp++) {
	if (quote) {
	    quote = FALSE;
	    continue;
	}
	if (*cp == '\\') {
	    quote = TRUE;
	    continue;
	}
	if (meta[(int)*cp])
	    return (1);
    }

    if (quote)
	return (1);
    return (0);
}

/****************** PRINT DEBUGGING INFO *****************/
STATIC int
VarPrintVar (ClientData vCD, ClientData unused)
{
    Var *v = (Var *)vCD;

    if (DEBUG(GRAPH1) || DEBUG(GRAPH2))
	printf ("\n# Variable %s assigned in %s\n",
		v->name->data,
		v->where ? v->where->data : "(Unknown)");
    printf ("%-16s = %s\n", v->name->data, v->val->data);
    return (0);
}

/*-
 *-----------------------------------------------------------------------
 * Var_Dump --
 *	print all variables in a context
 *-----------------------------------------------------------------------
 */
void
Var_Dump (GNode *ctxt)
{
    Hash_Table *ht;
    Hash_Entry *he;
    Hash_Search hs;

    if ((ht = VarHashTable(ctxt)) == NULL) {
	Lst_ForEach (ctxt->context, VarPrintVar, (ClientData)NULL);
	return;
    }
    he = Hash_EnumFirst(ht, &hs);
    for (;;) {
	if (he == NULL)
	    return;
	VarPrintVar((ClientData)Hash_GetValue(he), (ClientData)NULL);
	he = Hash_EnumNext(&hs);
    }
}

char *
VarParseCond(const char *cp, Boolean *errorStatePtr, int *lenPtr)
{
    char *cp2;
    Boolean doFree;

    if (DEBUG(COND))
	printf("VarParseCond: %s", cp);
    cp2 = Var_Parse(cp, VAR_CMD, *errorStatePtr, lenPtr, &doFree);
    if (cp2 == var_Error) {
	*errorStatePtr = TRUE;
	if (DEBUG(COND))
	    printf(": (error)\n");
    } else {
	*errorStatePtr = FALSE;
	if (DEBUG(COND))
	    printf(": value %s, len %d\n", cp2, *lenPtr);
    }
    if (!doFree)
	cp2 = strdup(cp2);
    return(cp2);
}

char *
VarValueCond(const char *cp)
{
    string_t s = string_create(cp);

    cp = Var_Value(s, VAR_CMD);
    string_deref(s);
    if (cp == NULL)
	return(NULL);
    return(strdup(cp));
}
