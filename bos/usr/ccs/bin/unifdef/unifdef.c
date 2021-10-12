static char sccsid[] = "@(#)99	1.10  src/bos/usr/ccs/bin/unifdef/unifdef.c, cmdprog, bos411, 9428A410j 4/14/94 12:48:16";
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: main, change_elif, checkline, doif, error, flushline, getlin,
	      load_err, pfile, preproc_op, prname, putlin,
	      skip_defined, skipcomment, skipquote
 *
 * ORIGINS: 26 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * unifdef - remove ifdef'ed lines
 */

#define _ILS_MACROS    /* 139729 - use macros for better performance */
#include <sys/localedef.h>
#include <stdio.h>
#include <ctype.h>
#include <locale.h>
#include <nl_types.h>

#ifdef MSG
#include "unifdef_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_UNIFDEF,n,s) 
#else
#define MSGSTR(n,s) s
#endif

#define TRIGRAPH(p, c) ( (strlen(p)>3) && (p[0]=='?') && (p[1]=='?') && (p[2]==c))
#define BSS
FILE *input;
#ifndef YES
#define YES 1
#define NO  0
#endif

char *progname BSS;
char *filename BSS;
char text BSS;          /* -t option in effect: this is a text file */
char lnblank BSS;       /* -l option in effect: blank deleted lines */
char complement BSS;    /* -c option in effect: complement the operation */
#define MAXSYMS 100
char true[MAXSYMS] BSS;
char ignore[MAXSYMS] BSS;
char *sym[MAXSYMS] BSS;
char insym[MAXSYMS] BSS;
char nsyms BSS;
char incomment BSS;
#define QUOTE1 0
#define QUOTE2 1
char inquote[2] BSS;
int exitstat BSS;
char *skipcomment ();
char *skipquote ();

main (argc, argv)
int argc;
char **argv;
{
    char **curarg;
    register char *cp;
    register char *cp1;
    char ignorethis;

	setlocale(LC_ALL, "");
#ifdef MSG
	catd = catopen(MF_UNIFDEF, NL_CAT_LOCALE);
#endif

    load_err();

    progname = argv[0][0] ? argv[0] : "unifdef";

    for (curarg = &argv[1]; --argc > 0; curarg++) {
	if (*(cp1 = cp = *curarg) != '-')
	    break;
	if (*++cp1 == 'i') {
	    ignorethis = YES;
	    cp1++;
	}
	else
	    ignorethis = NO;
	if (   (   *cp1 == 'D'
		|| *cp1 == 'U'
		|| ( (cp1[-1] == 'i') && ( *cp1 == 'd' || *cp1 == 'u'))
	       )
	    && cp1[1] != '\0'
	   ) {
	    if (nsyms >= MAXSYMS) {
		prname ();
		fprintf (stderr, MSGSTR(TOOMANY, "too many symbols.\n")); /*MSG*/
		exit (2);
	    }
	    ignore[nsyms] = ignorethis;
	    true[nsyms] = (*cp1 == 'D' || *cp1 == 'd') ? YES : NO;
	    sym[nsyms++] = &cp1[1];
	}
	else if (ignorethis)
	    goto unrec;
	else if (strcmp (&cp[1], "t") == 0)
	    text = YES;
	else if (strcmp (&cp[1], "l") == 0)
	    lnblank = YES;
	else if (strcmp (&cp[1], "c") == 0)
	    complement = YES;
	else {
 unrec:
	    prname ();
	    fprintf (stderr, MSGSTR(ILLOPTS, "unrecognized option: %s\n"), cp); /*MSG*/
	    goto usage;
	}
    }
    if (nsyms == 0) {
 usage:
	fprintf (stderr, MSGSTR(USAGE1, "Usage: %s [-l] [-t] [-c] [[-Dsym] [-Usym] [-idsym] [-iusym]]... [file]\n"), progname); /*MSG*/
	fprintf(stderr,MSGSTR(USAGE2, "    At least one arg from [-D -U -id -iu] is required\n")); /*MSG*/
	exit (2);
    }

    if (argc > 1) {
	prname ();
	fprintf (stderr, MSGSTR(ONEFILE, "can only do one file.\n")); /*MSG*/
    }
    else if (argc == 1) {
	filename = *curarg;
	if ((input = fopen (filename, "r")) != NULL) {
	    pfile();
	    fclose (input);
	}
	else {
	    prname ();
	    perror(*curarg);
	}
    }
    else {
	filename = "[stdin]";
	input = stdin;
	pfile();
    }

    fflush (stdout);
    exit (exitstat);
}

/* types of input lines: */
#define PLAIN       0   /* ordinary line */
#define TRUE        1   /* a true  #ifdef of a symbol known to us */
#define MYFALSE     2   /* a false #ifdef of a symbol known to us */
#define OTHER       3   /* an #ifdef of a symbol not known to us */
#define ELIF_Y      7   /* #elif yes */
#define ELIF_N      8   /* #elif no */
#define ELIF_O      9   /* #elif other */
#define END_ELIF   10   /* flags used by elif processing */
#define NO_VALUE   11   /* flags used by elif processing */
#define ELSE        4   /* #else */
#define ENDIF       5   /* #endif */
#define LEOF        6   /* end of file */
#define MSGLENG   128   /* Message length */
#define NO_ERR      0
#define END_ERR     1
#define ELSE_ERR    2
#define ENDIF_ERR   3
#define IEOF_ERR    4
#define CEOF_ERR    5
#define Q1EOF_ERR   6
#define Q2EOF_ERR   7

char reject BSS;    /* 0 or 1: pass thru; 1 or 2: ignore comments */
int linenum BSS;    /* current line number */
int stqcline BSS;   /* start of current coment or quote */
int endif_again BSS; /* flags used by elif processing */
int flush_elif BSS; /* flags used by elif processing */
int elif_changed BSS; /* flags used by elif processing */
char err[8][256];
char *errs[8];

load_err()
{
	register i;

	for(i = 0; i < 8; i++) 
		errs[i] = &err[i][0];

	NLstrncpy(errs[0], "", MSGLENG);
	NLstrncpy(errs[1], "", MSGLENG);
	NLstrncpy(errs[2], MSGSTR(BADELSE, "Inappropriate else/elif"), MSGLENG); /*MSG*/
	NLstrncpy(errs[3], MSGSTR(BADENDIF, "Inappropriate endif"), MSGLENG); /*MSG*/
	NLstrncpy(errs[4], MSGSTR(BADEOF, "Premature EOF in ifdef"), MSGLENG); /*MSG*/
	NLstrncpy(errs[5], MSGSTR(BADCOMM, "Premature EOF in comment"), MSGLENG); /*MSG*/
	NLstrncpy(errs[6], MSGSTR(BADQUOTE, "Premature EOF in quoted character"),MSGLENG); /*MSG*/
	NLstrncpy(errs[7], MSGSTR(BADQSTR, "Premature EOF in quoted string"), MSGLENG); /*MSG*/
}

			
pfile ()
{
    reject = 0;
    doif (-1, NO, reject, 0, 0);
    return;
}

doif (thissym, inif, prevreject, depth, elif_depth)
register int thissym;   /* index of the symbol who was last ifdef'ed */
int inif;               /* YES or NO we are inside an ifdef */
int prevreject;         /* previous value of reject */
int depth;              /* depth of ifdef's */
int elif_depth;         /* depth of elif's */
{
    register int lineval;
    register int thisreject;
    int doret;          /* tmp return valud]e of doif */
    int cursym;         /* index of the symbol returned by checkline */
    int stline;         /* line number when called this time */
    char tmp_reject;

    stline = linenum;
    for (;;) {
	if (endif_again)
	{
	    lineval = ENDIF;
	    endif_again = NO;
	}
	else
	    lineval = checkline (&cursym, thissym);

	switch (lineval) {
	case PLAIN:
	    flushline (YES);
	    break;

	case TRUE:
	case MYFALSE:
	    flush_elif = NO;
	    elif_changed = NO;
	    thisreject = reject;
	    if (lineval == TRUE)
		insym[cursym] = 1;
	    else {
		if (reject < 2)
		    reject = ignore[cursym] ? 1 : 2;
		insym[cursym] = -1;
	    }
	    if (ignore[cursym])
		flushline (YES);
	    else {
		exitstat = 1;
		flushline (NO);
	    }
	    if ((doret = doif (cursym, YES, thisreject, depth + 1, depth + 1)) != NO_ERR)
		return error (doret, stline, depth);
    	    break;

	case ELIF_Y:
	case ELIF_N:
	    if (inif != 1)
		return error (ELSE_ERR, linenum, depth);
	    if (thissym < 0) {
		flushline(YES);
		break;
	    }
	    if (insym[thissym] == 1) {
		reject = ignore[thissym] ? 1 : 2;
		if (!ignore[thissym])
		    flushline(NO);
		break;
	    }
	    thisreject = prevreject;
	    if (lineval == ELIF_Y) {
		insym[cursym] = 1;
	        reject = prevreject;
	    }
	    else {
		if (reject < 2)
		    reject = ignore[cursym] ? 1 : 2;
		insym[cursym] = -1;
	    }
	    if (ignore[cursym])
		flushline (YES);
	    else {
		exitstat = 1;
		flushline (NO);
	    }
	    doret = doif (cursym, YES, thisreject, depth, depth + 1);
	    if (doret != NO_ERR && doret != END_ELIF)
		return error (doret, stline, depth);
	    if (depth != elif_depth && doret != END_ELIF)
		return NO_ERR;
	    if (doret == END_ELIF)
		endif_again = YES;
	    break;

	case OTHER:
	    flushline (YES);
	    if ((doret = doif (-1, YES, reject, depth + 1, depth + 1)) != NO_ERR)
		return error (doret, stline, depth);
	    break;

	case ELIF_O:
	    if (inif != 1)
		return error (ELSE_ERR, linenum, depth);
	    if (thissym >= 0)
	    {
		if (insym[thissym] == 1)
		    reject = ignore[thissym] ? 1 : 2;
		else
		{
		    flush_elif = YES;
		    reject = prevreject;
		}
		if (!ignore[thissym])
		{
		    flushline (YES);
		    doret = doif (-1, YES, reject, depth+1, depth + 2);
	    	    if (doret != NO_ERR && doret != END_ELIF)
			return error (doret, stline, depth);
		    if (depth != elif_depth && doret != END_ELIF)
			return NO_ERR;
		    if (doret == END_ELIF)
			endif_again = YES;
		    break;
		}
	    }
	    flushline(YES);
	    break;

	case ELSE:
	    if (inif != 1)
		return error (ELSE_ERR, linenum, depth);
	    inif = 2;
	    if (thissym >= 0) {

		if (insym[thissym] == 1)
		{
		    insym[thissym] = -1;
		    reject = ignore[thissym] ? 1 : 2;
		}
		else
		{
		    insym[thissym] = 1;
		    reject = prevreject;
		}
		if (!ignore[thissym]) {
		    flushline (NO);
		    break;
		}
	    }
	    flushline (YES);
	    break;

	case ENDIF:
	    if (inif == 0)
		return error (ENDIF_ERR, linenum, depth);
	    if (depth != elif_depth)
		return END_ELIF;
	    if (thissym >= 0) {
		insym[thissym] = 0;
		reject = prevreject;
		if (!ignore[thissym]) {
		    if (flush_elif)
			flushline (YES);
		    else
			flushline (NO);
		    return NO_ERR;
		}
	    }
	    flushline (YES);
	    return NO_ERR;

	case LEOF: {
	    int err;
	    err =   incomment
		  ? CEOF_ERR
		  : inquote[QUOTE1]
		  ? Q1EOF_ERR
		  : inquote[QUOTE2]
		  ? Q2EOF_ERR
		  : NO_ERR;
	    if (inif) {
		if (err != NO_ERR)
		    error (err, stqcline, depth);
		return error (IEOF_ERR, stline, depth);
	    }
	    else if (err != NO_ERR)
		return error (err, stqcline, depth);
	    else
		return NO_ERR;
	    }
	}
    }
}

#define endsym(c) (!isalpha (c) && !isdigit (c) && c != '_')

#define MAXLINE 256
char tline[MAXLINE] BSS;

checkline (cursym, thissym)
int *cursym;
int thissym;
{
    char *cp;
    register char *symp;
    register char chr;
    char *scp;
    char *tmp_cp;
    char *kw_cp;
    int retval;
    int symind;
#   define KWSIZE 7	/* max(ifndef, ifdef, if, else, endif, elif) + 1 */
    char keyword[KWSIZE+1];
    int done;		/* flag for evaluating the expression of #elif */
    int exp_val;	/* value of the #elif expression */

    linenum++;
    if (getlin (tline, sizeof tline, input, NO) == EOF)
        return LEOF;

    retval = PLAIN;

    if ( !preproc_op(cp = tline, &cp)
	|| incomment
	|| inquote[QUOTE1]
	|| inquote[QUOTE2]
       )
	goto eol;

	/* Found a '#' or equivalent */
	/* skip any added white space */

    kw_cp = cp = skipcomment (cp);
    symp = keyword;
    while (!endsym (*cp)) {
	*symp = *cp++;
	if (++symp >= &keyword[KWSIZE])	/* token too long to be keyword */
	    goto eol;
    }
    *symp = '\0';

    if (strcmp (keyword, "ifdef") == 0) {
	retval = YES;
	goto ifdef;
    }
    else if (strcmp (keyword, "ifndef") == 0) {
	retval = NO;
 ifdef:
	scp = cp = skipcomment (++cp);
	if (incomment) {
	    retval = PLAIN;
	    goto eol;
	}
	for (symind = 0; ; ) {
	    if (insym[symind] == 0) {
		for ( symp = sym[symind], cp = scp
		    ; *symp && *cp == *symp
		    ; cp++, symp++
		    )
		    {}
		chr = *cp;
		if (*symp == '\0' && endsym (chr)) {
		    *cursym = symind;
		    retval = (retval ^ true[symind]) ? MYFALSE : TRUE;
		    break;
		}
	    }
	    if (++symind >= nsyms) {
		retval = OTHER;
		break;
	    }
	}
    }
    else if (strcmp (keyword, "elif") == 0)
    {
	scp = cp = skipcomment (++cp);
	if (incomment) {
	    retval = PLAIN;
	    goto eol;
	}

	done = NO;
	exp_val = 0;
	while (!done)
	{
	    retval = skip_defined(&cp);
	    tmp_cp = cp;
	    if (retval == NO_VALUE)
	    {
		retval = ELIF_O;
		if (!elif_changed && thissym >= 0)
		{
		    change_elif(kw_cp);
		    elif_changed = YES;
		}
		break;
	    }
	    for (symind = 0; ; ) {
		if (insym[symind] == 0) {
		    for ( symp = sym[symind], cp = tmp_cp
			; *symp && *cp == *symp
			; cp++, symp++
			)
			{}
		    chr = *cp;
		    for ( ; *cp == ' ' || *cp == '\t' || *cp == ')'; cp++ );
		    if (*cp == '\0' || *cp == '\n')
			done = YES;
		    if (*symp == '\0' && endsym (chr)) {
			*cursym = symind;
			retval = (retval ^ true[symind]) ? ELIF_N : ELIF_Y;
			break;
		    }
		}
		if (++symind >= nsyms) {
		    retval = ELIF_O;
		    done = YES;
		    if (!elif_changed && thissym >= 0)
		    {
			change_elif(kw_cp);
			elif_changed = YES;
		    }
		    break;
		}
	    } /* for */
		
	    if (retval == ELIF_N || retval == ELIF_Y)
	    {
		if (exp_val == 0)
		    exp_val = retval;
		else 	/* tempatorily donot handle expression containing  */
			/*   more than 2 defined terms linked by &&, or !! */
		{
		    retval = ELIF_O;
		    if (!elif_changed && thissym >= 0)
		    {
			change_elif(kw_cp);
			elif_changed = YES;
		    }
		    break;
		}
	    }
	} /* while */
    }
    else if (strcmp (keyword, "if") == 0)
	retval = OTHER;
    else if (strcmp (keyword, "else") == 0)
	retval = ELSE;
    else if (strcmp (keyword, "endif") == 0)
	retval = ENDIF;

 eol:
    if (!text && !reject)
	for (; *cp; ) {
	    if (incomment)
		cp = skipcomment (cp);
	    else if (inquote[QUOTE1])
		cp = skipquote (cp, QUOTE1);
	    else if (inquote[QUOTE2])
		cp = skipquote (cp, QUOTE2);
	    else if (*cp == '/' && cp[1] == '*')
		cp = skipcomment (cp);
	    else if (*cp == '\'')
		cp = skipquote (cp, QUOTE1);
	    else if (*cp == '"')
		cp = skipquote (cp, QUOTE2);
	    else
		cp++;
	}
    return retval;
}
/*
 *  skip_defined scans keywords "defined" and related tokens "!", "(",
 *  ")", "||", and "&&", to make sure current #elif line has correct
 *  syntax. It returns:
 *
 *	YES when a term associated with "defined" is found,
 *	NO  when a term associated with "!defined" is found,
 *	NO_VALUE when current expression is unevaluatable.
 */
int skip_defined(cp_ptr)
char **cp_ptr;
{
    register char *cp = *cp_ptr;
    int negation = NO;
    int evaluatable = YES;
    int retval = NO_VALUE;

    while (evaluatable && retval == NO_VALUE)
    {
	switch (*cp)
	{
	    case 'd':
		if (strlen(cp) > 6)
		    if (strncmp(cp, "defined", 7) == 0)
		    {
			cp += 7;
			retval = YES;
			for ( ; *cp==' ' || *cp=='\t' || *cp=='('; cp++);
			break;
		    };
		evaluatable = NO;
		break;
	    case '!':
		negation = YES ^ negation;
		cp++;
		break;
	    case '&':
		if (cp[1] != '&')
		    evaluatable = NO;
		else
		    cp += 2;
		break;
	    case '|':
		if (cp[1] != '|')
		    evaluatable = NO;
		else
		    cp += 2;
		break;
	    case '(':
	    case ')':
		cp++;
		break;
	    default:
		evaluatable = NO;
		break;
	}  /* switch */

	/* skip space following current token */
	for ( ; *cp == ' ' || *cp == '\t' ; cp++ );
    }  /* while */

    if (evaluatable && negation)
	retval = NO;
    *cp_ptr = cp;
    return retval;
}


/*
 *  change_elif is envoked to change an #elif expression into an #if
 *  expression when current #elif expression is found unevaluatable.
 */
change_elif(cp)
register char *cp;
{
    cp[0] = 'i'; cp[1] = 'f';
    cp[2] = cp[3] = ' ';
}

/*  Skip over comments and stop at the next charaacter  */
/*  position that is not whitespace. 			*/
/**/
char *
skipcomment (cp)
register char *cp;
{
    if (incomment)
	goto inside;
    for (;; cp++) {
        while (*cp == ' ' || *cp == '\t')
            cp++;
	if (text)
            return cp;
	if (   cp[0] != '/'
	    || cp[1] != '*'
	   )
            return cp;
	cp += 2;
	if (!incomment) {
	    incomment = YES;
	    stqcline = linenum;
	}
 inside:
	for (;;) {
	    for (; *cp != '*'; cp++)
		if (*cp == '\0')
		    return cp;
	    if (*++cp == '/')
		break;
	}
	incomment = NO;
    }
}

/*  Skip over a quoted string or character and stop at the next charaacter */
/*  position that is not whitespace. */			 
/**/
char *
skipquote (cp, type)
register char *cp;
register int type;
{
    register char qchar;

    qchar = type == QUOTE1 ? '\'' : '"';

    if (inquote[type])
	goto inside;
    for (;; cp++) {
	if (*cp != qchar)
	    return cp;
	cp++;
	if (!inquote[type]) {
	    inquote[type] = YES;
	    stqcline = linenum;
	}
 inside:
	for (; ; cp++) {
	    if (*cp == qchar)
		break;
	    if (   *cp == '\0'
		|| *cp == '\\'
		&& *++cp == '\0'
	       )
		return cp;
	    else if ( TRIGRAPH(cp, '/') && (cp[4] == '\n') )	/* line continuation */
	    {
		cp += 4;
		if (*cp == '\0')
		    return cp;
	    }
	}
	inquote[type] = NO;
    }
}

/*
 *   special getlin - treats form-feed as an end-of-line
 *                    and expands tabs if asked for
 *
 */
getlin (line, maxline, inp, expandtabs)
register char *line;
int maxline;
FILE *inp;
int expandtabs;
{
    int tmp;
    register int num;
    register int chr;
#ifdef FFSPECIAL
    static char havechar = NO;  /* have leftover char from last time */
    static char svchar BSS;
#endif

    num = 0;
#ifdef FFSPECIAL
    if (havechar) {
	havechar = NO;
	chr = svchar;
	goto ent;
    }
#endif
    while (num + 8 < maxline) {   /* leave room for tab */
        chr = getc (inp);
	if (isprint (chr)) {
#ifdef FFSPECIAL
 ent:
#endif
	    *line++ = chr;
	    num++;
	}
	else
	    switch (chr) {
	    case EOF:
		return EOF;

	    case '\t':
		if (expandtabs) {
		    num += tmp = 8 - (num & 7);
		    do
			*line++ = ' ';
		    while (--tmp);
		    break;
		} 
            default:
                *line++ = chr;
                num++;
		break;

	    case '\n':
                *line = '\n';
                num++;
                goto end;
    
#ifdef FFSPECIAL
	    case '\f':
		if (++num == 1)
		    *line = '\f';
		else {
		    *line = '\n';
		    havechar = YES;
                    svchar = chr;
                }
                goto end;
#endif
	    }
    }
 end:
    *++line = '\0';
    return num;
}

flushline (keep)
{
    if ((keep && reject < 2) ^ complement)
	putlin (tline, stdout);
    else if (lnblank)
	putlin ("\n", stdout);
    return;
}

/*
 *  putlin - for tools
 *
 */
putlin (line, fio)
register char *line;
register FILE *fio;
{
    register char chr;

    while (chr = *line++)
	putc (chr, fio);
    return;
}

prname ()
{
    fprintf (stderr, "%s: ", progname);
    return;
}


error (err, line, depth)
{
    if (err == END_ERR)
	return err;

    prname ();

#ifndef TESTING
    fprintf (stderr, MSGSTR(LINEERR, "Error in %s line %d: %s.\n"), /*MSG*/
	     filename, line, errs[err]);
#endif

#ifdef TESTING
    fprintf (stderr, MSGSTR(LINERRN, "Error in %s line %d: %s. "), /*MSG*/
	     filename, line, errs[err]);
    fprintf (stderr, MSGSTR(DEPTH, "ifdef depth: %d\n"), depth); /*MSG*/
#endif

    exitstat = 2;
    return depth > 1 ? IEOF_ERR : END_ERR;
}


/*
 *  check whether the current line contains preprocessor operator.
 *  eg.: #if, #ifdef, #ifndef, #elif, #else, #endif, ... etc.
 *  It is possible to have some white spaces (including comments)
 *  before # sign, or right after it.
 *  Any time we're certain that there can't be the legal start of a
 *  preprocessor token, we're free to exit from this routine.
*/

int	preproc_op(line_ptr, ret_ptr)
register char	*line_ptr;
char		**ret_ptr;
{
    register  char    *s = line_ptr;
    char flag = 0;	/* Assumes no # to be found */

	s = skipcomment(s);	/* Skip over white-space & comments */

	switch(*s) {
	case '\0':	break;
	case '#':	{ s++; flag++; break; }
	case '\n':	break;
	case '?':
		if TRIGRAPH(s, '=') {	/* Have trigraph for '#' */
			s +=3;
			flag++;
			break;
		}
	}				/* No other non-whitespaces to worry about */

    *ret_ptr = s;
    return(flag);
}
