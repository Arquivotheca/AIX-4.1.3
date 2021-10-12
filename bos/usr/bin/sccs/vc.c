static char sccsid[] = "@(#)97 1.11 src/bos/usr/bin/sccs/vc.c, cmdsccs, bos41B, 9504A 12/9/94 10:00:14";
/*
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: asgfunc, chksize, dclfunc, doand, door, ecopy, endfunc,
 *            errfunc, exp, findch, findstr, getid, iffunc, lookup,
 *            main, msgfunc, numck, numcomp, putin, repfunc, replace
 *
 * ORIGINS: 3, 10, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include 	<locale.h>
#include	"stdio.h"
#include	"sys/types.h"
#include	"macros.h"
#include	"fatal.h"
#include	"defines.h"
#include 	"vc_msg.h"

#define MSGSTR(Num, Str) catgets(catd, MS_VC, Num, Str)
#define FPRINTF fprintf
#define SPRINTF sprintf

/*
 * The symbol table size is set to a limit of forty keywords per input
 * file.  Should this limit be changed it should also be changed in the
 * Help file.
 */

# define SYMSIZE 40
# define PARMSIZE 10
# define NSLOTS 32

# define USD  1
# define DCL 2
# define ASG 4

# define EQ '='
# define NEQ '!'
# define GT '>'
# define LT '<'
# define DELIM " \t"
# define TRUE 1
# define FALSE 0

static char ErrMsg[512];

static char	Ctlchar = ':';

struct	symtab	{
	int	usage;
	char	name[PARMSIZE];
	char	*value;
	int	lenval;
};
static struct	symtab	Sym[SYMSIZE];


static int	Skiptabs;
static int	Repall;

/*
 * Delflag is used to indicate when text is to be skipped.  It is decre-
 * mented whenever an if condition is false, or when an if occurs
 * within a false if/end statement.  It is decremented whenever an end is
 * encountered and the Delflag is greater than zero.  Whenever Delflag
 * is greater than zero text is skipped.
 */

static int	Delflag;

/*
 * Ifcount keeps track of the number of ifs and ends.  Each time
 * an if is encountered Ifcount is incremented and each time an end is
 * encountered it is decremented.
 */

static int	Ifcount;
static int	Lineno;

static char	*Repflag;
static char	*Linend;
static int	Silent;
static char	*getid(), *replace();
static char	*findch(), *ecopy(), *findstr();
char	*fmalloc();

nl_catd catd;

/*
 * The main program reads a line of text and sends it to be processed
 * if it is a version control statement. If it is a line of text and
 * the Delflag is equal to zero, it is written to the standard output.
 */

main(argc, argv)
int argc;
char *argv[];
{
	register  char *lineptr, *p;
	register int i;
	char line[512];
	extern int Fflags;

        (void)setlocale(LC_ALL,"");
	catd = catopen(MF_SCCS, NL_CAT_LOCALE);

	Fflags = FTLCLN | FTLMSG | FTLEXIT;
	setsig();
	for(i = 1; i< argc; i++) {
		p = argv[i];
		if (p[0] == '-')
			switch (p[1]) {
			case 's':
				Silent = 1;
				break;
			case 't':
				Skiptabs = 1;
				break;
			case 'a':
				Repall = 1;
				break;
			case 'c':
				Ctlchar = p[2];
				break;
			}
		else {
			p[size(p) - 1] = '\n';
			asgfunc(p);
		}
	}
	while (fgets(line,sizeof(line),stdin) != NULL) {
		lineptr = line;
		Lineno++;

		if (Repflag != 0) {
			ffree(Repflag);
			Repflag = 0;
		}

		if (Skiptabs) {
			for (p = lineptr; *p; p++)
				if (*p == '\t')
					break;
			if (*p++ == '\t')
				lineptr = p;
		}

		if (lineptr[0] != Ctlchar) {
			if (lineptr[0] == '\\' && lineptr[1] == Ctlchar)
				for (p = &lineptr[1]; *lineptr++ = *p++; )
					;
			if(Delflag == 0) {
				if (Repall)
					repfunc(line);
				else
					fputs(line,stdout);
			}
			continue;
		}

		lineptr++;

		if (imatch("if ", lineptr))
			iffunc(&lineptr[3]);
		else if (imatch("end", lineptr))
			endfunc();
		else if (Delflag == 0) {
			if (imatch("asg ", lineptr))
				asgfunc(&lineptr[4]);
			else if (imatch("dcl ", lineptr))
				dclfunc(&lineptr[4]);
			else if (imatch("err", lineptr))
				errfunc(&lineptr[3]);
			else if (imatch("msg", lineptr))
				msgfunc(&lineptr[3]);
			else if (lineptr[0] == Ctlchar)
				repfunc(&lineptr[1]);
			else if (imatch("on", lineptr))
				Repall = 1;
			else if (imatch("off", lineptr))
				Repall = 0;
			else if (imatch("ctl ", lineptr))
				Ctlchar = lineptr[4];
			else {
				sprintf(ErrMsg,MSGSTR(UNKCMD, "The command specified on line %d is not recognized.\n\
\tSpecify a valid command. (vc1)\n"), Lineno);  /* MSG */
				fatal(ErrMsg);
			}
		}
	}
	for(i = 0; Sym[i].usage != 0 && i<SYMSIZE; i++) {
		if ((Sym[i].usage&USD) == 0 && !Silent)
			fprintf(stderr,MSGSTR(NVRUSED, " %s was not used. (vc2) \n"),Sym[i].name);  /* MSG */
		if ((Sym[i].usage&DCL) == 0 && !Silent)
			fprintf(stderr,MSGSTR(NVRDCLD, " %s was not declared. (vc3)\n"), Sym[i].name);  /* MSG */
		if ((Sym[i].usage&ASG) == 0 && !Silent)
			fprintf(stderr,MSGSTR(NVRASSGND, " %s was not assigned a value. (vc20)\n"), Sym[i].name);  /* MSG */
	}
	if (Ifcount > 0)
		fatal(MSGSTR(NOEND, "\nThere is an if statement with no matching end statement. (vc4)\n"));  /* MSG */
	exit(0);
}


/*
 * Asgfunc accepts a pointer to a line picks up a keyword name, an
 * equal sign and a value and calls putin to place it in the symbol table.
 */

static asgfunc(aptr)
register char *aptr;
{
	register char *end, *aname;
	char *avalue;

	aptr = replace(aptr);
	NONBLANK(aptr);
	aname = aptr;
	end = Linend;
	aptr = findstr(aptr,"= \t");
	if (*aptr == ' ' || *aptr == '\t') {
		*aptr++ = '\0';
		aptr = findch(aptr,'=');
	}
	if (aptr == end) {
		sprintf(ErrMsg,MSGSTR(SYNTAX17, "The control statement syntax on line %d is not correct.\n\
\tMake sure there is an = in the :asg statement.\n\
\tMake sure there is a right side to the :asg statement.  (vc17)\n"),Lineno);  /* MSG */
		fatal(ErrMsg);
	}
	*aptr++ = '\0';
	avalue = getid(aptr);
	chksize(aname);
	putin(aname, avalue);
}


/*
 * Dclfunc accepts a pointer to a line and picks up keywords
 * separated by commas.  It calls putin to put each keyword in the
 * symbol table.  It returns when it sees a newline.
 */

static dclfunc(dptr)
register char *dptr;
{
	register char *end, *dname;
	int i;

	dptr = replace(dptr);
	end = Linend;
	NONBLANK(dptr);
	while (dptr < end) {
		dname = dptr;
		dptr = findch(dptr,',');
		*dptr++ = '\0';
		chksize(dname);
		if (Sym[i = lookup(dname)].usage&DCL) {
			SPRINTF(ErrMsg,MSGSTR(DCLDTWC, " %1$s is declared twice on line %2$d.\n\
\tDo not declare a keyword more than once on the command line. (vc5)\n"), dname, Lineno);  /* MSG */
			fatal(ErrMsg);
		}
		else
			Sym[i].usage |= DCL;
		NONBLANK(dptr);
	}
}


/*
 * Errfunc calls fatal which stops the process.
 */

static errfunc(eptr)
char *eptr;
{
	if (!Silent)
		fprintf(stderr,MSGSTR(RPLCERR, "ERROR:  %s\n"),replace(eptr));  /* MSG */
	sprintf(ErrMsg,MSGSTR(ERRSTMT, "The :err statement is on line %d. (vc15)\n"), Lineno);  /* MSG */
	fatal(ErrMsg);
}


/*
 * Endfunc indicates an end has been found by decrementing the if count
 * flag.  If because of a previous if statement, text was being skipped,
 * Delflag is also decremented.
 */

static endfunc()
{
	if (--Ifcount < 0) {
		sprintf(ErrMsg,MSGSTR(ENDWOIF, "There is an end statement without a matching if statement\n\
\ton line %d (vc10)\n"), Lineno);  /* MSG */
		fatal(ErrMsg);
	}
	if (Delflag > 0)
		Delflag--;
	return;
}


/*
 * Msgfunc accepts a pointer to a line and prints that line on the 
 * diagnostic output.
 */

static msgfunc(mptr)
char *mptr;
{
	if (!Silent)
		FPRINTF(stderr,MSGSTR(MESSAGE, "Message %1$d: %2$s\n"), Lineno, replace(mptr));  /* MSG */
}


static repfunc(s)
char *s;
{
	fprintf(stdout,"%s\n",replace(s));
}


/*
 * Iffunc and the three functions following it, door, doand, and exp
 * are responsible for parsing and interperting the condition in the
 * if statement.  The BNF used is as follows:
 *	<iffunc> ::=   [ "not" ] <door> EOL
 *	<door> ::=     <doand> | <doand> "|" <door>
 *	<doand>::=     <exp> | <exp> "&" <doand>
 *	<exp>::=       "(" <door> ")" | <value> <operator> <value>
 *	<operator>::=  "=" | "!=" | "<" | ">"
 * And has precedence over or.  If the condition is false the Delflag
 * is bumped to indicate that lines are to be skipped.
 * An external variable, sptr is used for processing the line in
 * iffunc, door, doand, exp, getid.
 * Iffunc accepts a pointer to a line and sets sptr to that line.  The
 * rest of iffunc, door, and doand follow the BNF exactly.
 */

static char *sptr;

static iffunc(iptr)
char *iptr;
{
	register int value, not;

	Ifcount++;
	if (Delflag > 0)
		Delflag++;

	else {
		sptr = replace(iptr);
		NONBLANK(sptr);
		if (imatch("not ", sptr)) {
			not = FALSE;
			sptr += 4;
		}
		else not = TRUE;

		value = door();
		if( *sptr != 0) {
			sprintf(ErrMsg,MSGSTR(SYNTAX18, "The if statement syntax on line %d is not correct. (vc18)"),Lineno);  /* MSG */
			fatal(ErrMsg);
		}

		if (value != not)
			Delflag++;
	}

	return;
}


static door()
{
	int value;
	value = doand();
	NONBLANK(sptr);
	while (*sptr=='|') {
		sptr++;
		value |= doand();
		NONBLANK(sptr);
	}
	return(value);
}


static doand()
{
	int value;
	value = exp();
	NONBLANK(sptr);
	while (*sptr=='&') {
		sptr++;
		value &= exp();
		NONBLANK(sptr);
	}
	return(value);
}


/*
 * After exp checks for parentheses, it picks up a value by calling getid,
 * picks up an operator and calls getid to pick up the second value.
 * Then based on the operator it calls either numcomp or equal to see
 * if the exp is true or false and returns the correct value.
 */

static exp()
{
	register char op, save;
	register int value;
	char *id1, *id2, next;

	NONBLANK(sptr);
	if(*sptr == '(') {
		sptr++;
		value = door();
		NONBLANK(sptr);
		if (*sptr == ')') {
			sptr++;
			return(value);
		}
		else {
			sprintf(ErrMsg,MSGSTR(PARENERR, "There is a parenthesis error on line %d.\n\
\tSpecify a parenthesis only where correct.\n\
\tSpecify a right parenthesis for each left parenthesis. (vc11)\n"), Lineno);  /* MSG */
		}
	}

	id1 = getid(sptr);
	if (op = *sptr)
		*sptr++ = '\0';
	if (op == NEQ && (next = *sptr++) == '\0')
		--sptr;
	id2 = getid(sptr);
	save = *sptr;
	*sptr = '\0';

	if(op ==LT || op == GT) {
		value = numcomp(id1, id2);
		if ((op == GT && value == 1) || (op == LT && value == -1))
			value = TRUE;
		else value = FALSE;
	}

	else if (op==EQ || (op==NEQ && next==EQ)) {
		value = equal(id1, id2);
		if ( op == NEQ)
			value = !value;
	}

	else {
		sprintf(ErrMsg,MSGSTR(INVOP, "An operator on line %d is not valid.\n\
\tThe valid operators are !=, =, <, and >.\n"), Lineno);  /* MSG */
		fatal(ErrMsg);
	}
	*sptr = save;
	return(value);
}


/*
 * Getid picks up a value off a line and returns a pointer to the value.
 */

char *
getid(gptr)
register char *gptr;
{
	register char *id;

	NONBLANK(gptr);
	id = gptr;
	gptr = findstr(gptr,DELIM);
	if (*gptr)
		*gptr++ = '\0';
	NONBLANK(gptr);
	sptr = gptr;
	return(id);
}


/*
 * Numcomp accepts two pointers to strings of digits and calls numck
 * to see if the strings contain only digits.  It returns -1 if
 * the first is less than the second, 1 if the first is greater than the
 * second and 0 if the two are equal.
 */

static numcomp(id1, id2)
register char *id1, *id2;
{
	int k1, k2;

	numck(id1);
	numck(id2);
	while (*id1 == '0')
		id1++;
	while (*id2 == '0')
		id2++;
	if ((k1 = size(id1)) > (k2 = size(id2)))
		return(1);
	else if (k1 < k2)
		return(-1);
	else while(*id1 != '\0') {
		if(*id1 > *id2)
			return(1);
		else if(*id1 < *id2)
			return(-1);
		id1++;
		id2++;
	}
	return(0);
}


/*
 * Numck accepts a pointer to a string and checks to see if they are
 * all digits.  If they're not it calls fatal, otherwise it returns.
 */

static numck(nptr)
register char *nptr;
{
	for (; *nptr != '\0'; nptr++)
		if (!NUMERIC(*nptr)) {
			sprintf(ErrMsg,MSGSTR(NONNUMB, "There is a value on line %d that is not numeric. (vc14)\n"), Lineno);  /* MSG */
			fatal(ErrMsg);
		}
	return;
}


/*
 * Replace accepts a pointer to a line and scans the line for a keyword
 * enclosed in control characters.  If it doesn't find one it returns
 * a pointer to the begining of the line.  Otherwise, it calls
 * lookup to find the keyword.
 * It rewrites the line substituting the value for the
 * keyword enclosed in control characters.  It then continues scanning
 * the line until no control characters are found and returns a pointer to
 * the begining of the new line.
 */

# define INCR(int) if (++int==NSLOTS) { \
		sprintf(ErrMsg,MSGSTR(NOSPC, "Line %d requires too many replacements.\n\
\tThe maximum number of replacements per line is 32. (vc16)\n"),Lineno); \
		fatal(ErrMsg); }

char *
replace(ptr)
char *ptr;
{
	char *slots[NSLOTS];
	int i,j,newlen;
	register char *s, *t, *p;

	for (s=ptr; *s++!='\n';);
	*(--s) = '\0';
	Linend = s;
	i = -1;
	for (p=ptr; *(s=findch(p,Ctlchar)); p=t) {
		*s++ = '\0';
		INCR(i);
		slots[i] = p;
		if (*(t=findch(s,Ctlchar))==0) {
			SPRINTF(ErrMsg,MSGSTR(UNMTCHD, "\nThere is a %1$c on line %2$d that is not matched.\n\
\tSpecify an ending control character. (vc7)\n"), Ctlchar,Lineno);  /* MSG */
			fatal(ErrMsg);
		}
		*t++ = '\0';
		INCR(i);
		slots[i] = Sym[j = lookup(s)].value;
		Sym[j].usage |= USD;
	}
	INCR(i);
	slots[i] = p;
	if (i==0) return(ptr);
	newlen = 0;
	for (j=0; j<=i; j++)
		newlen += (size(slots[j])-1);
	t = Repflag = fmalloc(++newlen);
	for (j=0; j<=i; j++)
		t = ecopy(slots[j],t);
	Linend = t;
	return(Repflag);
}


/*
 * Lookup accepts a pointer to a keyword name and searches the symbol
 * table for the keyword.  It returns its index in the table if its there,
 * otherwise it puts the keyword in the table.
 */

static lookup(lname)
char *lname;
{
	register int i;
	register char *t;
	register struct symtab *s;

	t = lname;
	while ((i = *t++) &&
		((i>='A' && i<='Z') || (i>='a' && i<='z') ||
			(i!= *lname && i>='0' && i<='9')));
	if (i) {
		sprintf(ErrMsg,MSGSTR(INVKEYWRD, "A keyword name on line %d is not valid.\n\
\tA keyword name must begin with an alphabetic character and must contain\n\
\tonly alphanumeric characters. (vc9)\n"),Lineno);  /* MSG */
		fatal(ErrMsg);
	}

	for(i =0; Sym[i].usage != 0 && i<SYMSIZE; i++)
		if (equal(lname, Sym[i].name)) return(i);
	if (i >= SYMSIZE)
		fatal(MSGSTR(OUTOFSPCVC6, "\nDo not specify more than 40 keywords on the command line. (vc6)\n"));
	s = &Sym[i];
	copy(lname,s->name);
	copy("",(s->value = fmalloc(s->lenval = 1)));
	return(i);
}


/*
 * Putin accepts a pointer to a keyword name, and a pointer to a value.
 * It puts this information in the symbol table by calling lookup.
 * It returns the index of the name in the table.
 */

static putin(pname, pvalue)
char *pname;
char *pvalue;
{
	register int i;
	register struct symtab *s;

	s = &Sym[i = lookup(pname)];
	ffree(s->value);
	s->lenval = size(pvalue);
	copy(pvalue, (s->value = fmalloc(s->lenval)));
	s->usage |= ASG;
	return(i);
}


static chksize(s)
char *s;
{
	if (size(s) > PARMSIZE) {
		sprintf(ErrMsg,MSGSTR(NMTOOLNG, "A keyword name on line %d is too long.\n\
\tA keyword name cannot exceed 9 characters. (vc8)\n"),Lineno);  /* MSG */
		fatal(ErrMsg);
	}
}


char *
findch(astr,match)
char *astr, match;
{
	register char *s, *t, c;
	char *temp;

	for (s=astr; (c = *s) && c!=match; s++)

		if (c=='\\') {
			if (s[1]==0) {
				sprintf(ErrMsg,MSGSTR(SYNTAX19, "The syntax on line %d is not correct. (vc19)\n"),Lineno);  /* MSG */
				fatal(ErrMsg);
			}
			else {
				for (t = (temp=s) + 1; *s++ = *t++;);
				s = temp;
			}
		}
	return(s);
}


char *
ecopy(s1,s2)
char *s1, *s2;
{
	register char *r1, *r2;

	r1 = s1;
	r2 = s2;
	while (*r2++ = *r1++);
	return(--r2);
}


char *
findstr(astr,pat)
char *astr, *pat;
{
	register char *s, *t, c;
	char *temp;

	for (s=astr; (c = *s) && any(c,pat)==0; s++)
		if (c=='\\') {
			if (s[1]==0) {
				sprintf(ErrMsg,MSGSTR(SYNTAX19, "The syntax on line %d is not correct. (vc19)\n"),Lineno);  /* MSG */
				fatal(ErrMsg);
			}
			else {
				for (t = (temp=s) + 1; *s++ = *t++;);
				s = temp;
			}
		}
	return(s);
}
