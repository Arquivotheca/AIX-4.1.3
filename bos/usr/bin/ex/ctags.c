static char sccsid[] = "@(#)56	1.25  src/bos/usr/bin/ex/ctags.c, cmdedit, bos41B, 9504A 12/19/94 11:46:21";
/*
 * COMPONENT_NAME: (CMDEDIT) ctags.c
 *
 * FUNCTIONS: C_funcs, L_funcs, L_getit, main, PF_funcs, Y_entries, add_node,
 * find_funcs, first_char, free_tree, getit, getline, init, isgood, pfnote,
 * put_entries, put_funcs, rindex, savestr, start_func, striccmp, tail,
 * takeprec, toss_comment, toss_yyse
 *
 * ORIGINS: 3, 10, 13, 26, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 */

#define _ILS_MACROS
#include <stdio.h>

/* The following declarations have been included to keep lint happy.
   They are needed because in stdio.h, getc and putc not only have
   ANSI style prototypes, but they are also #defined as macro,
   where the macro expansion involves _filbuf and _flsbuf.
   The declarations of _filbuf and _flsbuf were found in llib-lc. 
   According to a comment in llib-lc,
   these declarations are actually UNDOCUMENTED and not for 
   general use!
*/

#include <ctype.h>
#include <locale.h>

#include "ex_msg.h"
nl_catd	catd;
#define MSGSTR(id,ds)      catgets(catd, MS_EX, id, ds)

/*
 * ctags: create a tags file
 */

#define	reg	register
#define	logical	int
#define BUFFERSIZ (2*8192)	/* to be separate from BUFSIZ in stdio.h */

#define	iswhite(arg)	(_wht[arg])	/* T if char is white		*/
#define	begtoken(arg)	(_btk[arg])	/* T if char can start token	*/
#define	intoken(arg)	(_itk[arg])	/* T if char can be in token	*/
#define	endtoken(arg)	(_etk[arg])	/* T if char ends tokens	*/
#define	isgood(arg)	(_gd[arg])	/* T if char can be after ')'	*/

#define	max(I1,I2)	(I1 > I2 ? I1 : I2)

struct	nd_st {			/* sorting structure			*/
	char	*entry;			/* entry name	        	*/
	char	*file;			/* file name			*/
	logical	f;			/* use pattern or line no	*/
	int	lno;			/* for -x option		*/
	char	*pat;			/* search pattern		*/
	logical	been_warned;		/* set if noticed dup		*/
	struct	nd_st	*left,*right;	/* left and right sons		*/
};

typedef	struct	nd_st	NODE;

static logical	number,				/* T if on line starting with #	*/
	makefile= TRUE,			/* T if to creat "tags" file	*/
	gotone,				/* found a func already on line	*/
					/* boolean "func" (see init)	*/
	_wht[0177],_etk[0177],_itk[0177],_btk[0177],_gd[0177];

	/* typedefs are recognized using a simple finite automata,
	 * tydef is its state variable.
	 */
typedef enum {none, begin, middle, end } TYST;

static int	inbrace = 0;
static TYST tydef = none;
static char	searchar = '/';			/* use /.../ searches 		*/

int	lineno;				/* line number of current line	*/
static char	line[4*BUFFERSIZ],		/* current input line	*/
	*curfile,		/* current input file name		*/
	*outfile= "tags",	/* output file				*/
	*white	= " \f\t\n",	/* white chars				*/
	*endtk	= " \t\n\"'#()[]{}=-+%*/&|^~!<>;,.:?",
				/* token ending chars			*/
	*begtk	= "ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz",
				/* token starting chars			*/
	*intk	= "ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz0123456789$",				/* valid in-token chars			*/
	*notgd	= ",;";		/* non-valid after-function chars	*/

static int	file_num;		/* current file number			*/
static int	aflag;			/* -a: append to tags */
static int	mflag;			/* -m: create no tags for macros */
static int	Tflag;			/* -T: create no tags for typedefs */
static int	uflag;			/* -u: update tags */
static int	vflag;			/* -v: create vgrind style index output */
static int	wflag;			/* -w: suppress warnings */
static int	xflag;			/* -x: create cxref style output */

static char	lbuf[BUFFERSIZ];

static FILE	*inf,			/* ioptr for current input file		*/
	*outf;			/* ioptr for tags file			*/

static long	lineftell;		/* ftell after getc( inf ) == '\n' 	*/

static NODE	*head;			/* the head of the sorted binary tree	*/
/*
 * Function prototypes.
 */
/* LINTSTDLIB */
void	init(void);
void	find_funcs(char *);
void	C_funcs(void);
int	start_func(char **, char *, char *, int *);
void	Y_entries(void);
char	*toss_comment(char *);
void	getline(void);
void	free_tree(NODE *);
void	add_node(NODE *, NODE *);
void	put_entries(reg NODE *);
int	PF_funcs(FILE *);
int	tail(char *);
void	takeprec(void);
void	getit(void);
void	pfnote(char *, int, logical);
static char	*rindex(register char *, register char);
void	L_funcs(FILE *);
void	L_getit(int);
int	striccmp(register char *, register char *);
int	first_char(void);
void	toss_yysec(void);
char	*savestr(char *);
char    *index(const char *, int);


int
main(int argc, char **argv)
{
	char cmd[100];
	int c,i, exitval;
	extern char *optarg;
	extern int optind, optopt;

	(void)setlocale(LC_ALL,"");		
	catd = catopen(MF_EX, NL_CAT_LOCALE);

	while ((c = getopt(argc, argv, ":BFaf:mtTuvwx")) != -1) {
		switch(c) {
			case 'B':
				searchar='?';
				break;
			case 'F':
				searchar='/';
				break;
			case 'a':
				aflag = 1;
				break;
			case 'f':
				outfile = optarg;
				break;
			case 'm':
				mflag = 1;
				break;
			case 't':
				/* Always on per standards */
				break;
			case 'T':
				Tflag = 1;
				break;
			case 'u':
				uflag = 1;
				break;
			case 'v':
				vflag = 1;
				xflag = 1;
				break;
			case 'w':
				wflag = 1;
				break;
			case 'x':
				xflag = 1;
				break;
			case ':':
				fprintf(stderr,MSGSTR(M_604,"Option -%c requires an operand.\n"), optopt);
				goto usage;
			case '?':
				fprintf(stderr,MSGSTR(M_605,"Invalid option -%c.\n"), optopt);
				goto usage;
		}
	}

	argc -= optind;
	argv += optind;

	if (!argc) {
usage:
		(void)fprintf(stderr, MSGSTR(M_600, "Usage: ctags [-BFamtuwvx] [-f tagsfile] file ...\n"));
		catclose(catd);
		exit(1);
	}

	init();			/* set up boolean "functions"		*/
	/*
	 * loop through files finding functions
	 */
	for (exitval = file_num = 0; file_num < argc; ++file_num) {
		if ((inf=fopen(argv[file_num],"r")) == NULL) {
			perror(argv[file_num]);
			exitval++;
		} 
		else {
			find_funcs(argv[file_num]);
			(void)close(inf);
		}
	}

	if (xflag) {
		put_entries(head);
	} 
	else {
		if (uflag) {
			for (i=0; i<argc; i++) {
				(void)sprintf(cmd,
					"mv %s OTAGS;fgrep -v '\t%s\t' OTAGS >%s;rm OTAGS",
					outfile, argv[i], outfile);
				(void)system(cmd);
			}
			aflag = 1;
		}
		if ((outf = fopen(outfile, aflag ? "a" : "w")) == NULL) {
			perror(outfile);
			catclose(catd);
			exit(1);
		}
		put_entries(head);
		if (fclose(outf) == EOF) {
			perror(outfile);
			exitval++;
		}
		if (uflag || aflag) {
			(void)sprintf(cmd, "sort -u -o %s %s", outfile, outfile);
			(void)system(cmd);
		}
	}
	catclose(catd);
	exit(exitval);
}

/*
 * This routine sets up the boolean psuedo-functions which work
 * by seting boolean flags dependent upon the corresponding character
 * Every char which is NOT in that string is not a white char.  Therefore,
 * all of the array "_wht" is set to FALSE, and then the elements
 * subscripted by the chars in "white" are set to TRUE.  Thus "_wht"
 * of a char is TRUE if it is the string "white", else FALSE.
 */
void
init(void)
{

	reg	char	*sp;
	reg	int	i;

	for (i = 0; i < 0177; i++) {
		_wht[i] = _etk[i] = _itk[i] = _btk[i] = FALSE;
		_gd[i] = TRUE;
	}
	for (sp = white; *sp; sp++)
		_wht[*sp] = TRUE;
	for (sp = endtk; *sp; sp++)
		_etk[*sp] = TRUE;
	for (sp = intk; *sp; sp++)
		_itk[*sp] = TRUE;
	for (sp = begtk; *sp; sp++)
		_btk[*sp] = TRUE;
	for (sp = notgd; *sp; sp++)
		_gd[*sp] = FALSE;
}

/*
 * This routine opens the specified file and calls the function
 * which finds the function and type definitions.
 */

static void
find_funcs(char *file)
{
	char *cp;

	lineftell = 0;	/* zero out the offset when starting on a new file */
	curfile = savestr(file);
	lineno = 0;
	cp = rindex(file, '.');
	/* .l implies lisp or lex source code */
	if (cp && cp[1] == 'l' && cp[2] == '\0') {
		if (index(";([", first_char()) != NULL) {	/* lisp */
			L_funcs(inf);
			(void)fclose(inf);
			return;
		}
		else {						/* lex */
			/*
			 * throw away all the code before the second "%%"
			 */
			toss_yysec();
			getline();
			pfnote("yylex", lineno, TRUE);
			toss_yysec();
			C_funcs();
			(void)fclose(inf);
			return;
		}
	}
	/* .y implies a yacc file */
	if (cp && cp[1] == 'y' && cp[2] == '\0') {
		toss_yysec();
		Y_entries();
		C_funcs();
		(void)fclose(inf);
		return;
	}
	/*  if not a .c or .h file, try fortran */
	if (cp && !(cp[1] == 'c' && cp[2] == '\0') && 
		!(cp[1] == 'h' && cp[2] == '\0'))  {
		if (PF_funcs(inf) != 0) {
			(void)fclose(inf);
			return;
		}
		rewind(inf);	/* no fortran tags found, try C */
		lineno=0;
	}
	C_funcs();
	(void)fclose(inf);
}

static void
pfnote(char *name, int ln, logical f)
		/* f == TRUE when function */
{
	register char *fp;
	register NODE *np;
	char nbuf[BUFFERSIZ];

	if ((np = (NODE *) malloc(sizeof (NODE))) == NULL) {
		(void)fprintf(stderr, MSGSTR(M_601, "ctags: too many functions to sort\n"));
		put_entries(head);
		free_tree(head);
		head = np = (NODE *) malloc(sizeof (NODE));
	}
	if (xflag == 0 && !strcmp(name, "main")) {
		fp = rindex(curfile, '/');
		if (fp == 0)
			fp = curfile;
		else
			fp++;
		(void)sprintf(nbuf, "M%s", fp);
		fp = rindex(nbuf, '.');
		if (fp && fp[2] == 0)
			*fp = 0;
		name = nbuf;
	}
	np->entry = savestr(name);
	np->file = curfile;
	np->f = f;
	np->lno = ln;
	np->left = np->right = 0;
	if (xflag == 0) {
		lbuf[50] = 0;
		(void)strcat(lbuf, "$");
		lbuf[50] = 0;
	}
	np->pat = savestr(lbuf);
	if (head == NULL)
		head = np;
	else
		add_node(np, head);
}

/*
 * This routine finds functions and typedefs in C syntax 
 * and adds them to the list.
 */
static void
C_funcs(void)
{
	register int c;
	register char *token, *tp;
	logical incomm, inquote, inchar, midtoken;
	int level;
	char *sp;
	char tok[BUFFERSIZ];

	number = gotone = midtoken = inquote = inchar = incomm = FALSE;
	lineno++;
	lineftell = ftell(inf);
	level = 0;
	sp = tp = token = line;
	for (;;) {
		*sp=c=getc(inf);
		if (feof(inf))
			break;
		if (c == '\n')
			lineno++;
		if (c == '\\') {
			c = *++sp = getc(inf);
			if (c == '\n')
				c = ' ';
		} else if (incomm) {
			if (c == '*') {
				while ((*++sp=c=getc(inf)) == '*')
					continue;
				if (c == '\n')
					lineno++;
				else
				if (c == '/')
					incomm = FALSE;
			}
		} else if (inquote) {
			/*
		 	* Too dumb to know about \" not being magic, but
		 	* they usually occur in pairs anyway.
		 	*/
			if (c == '"')
				inquote = FALSE;
			continue;
		} else if (inchar) {
			if (c == '\'')
				inchar = FALSE;
			continue;
		} else switch (c) {
			case '"':
				inquote = TRUE;
				continue;
			case '\'':
				inchar = TRUE;
				continue;
			case '/':
				if ((*++sp=c=getc(inf)) == '*')
					incomm = TRUE;
				else
					(void)ungetc(*sp, inf);
				continue;
 			case '#':
				if (sp == line)
					number = TRUE;
				continue;
			case '{':
				if (tydef == begin) {
					tydef=middle;
				}
				level++;
				inbrace++;
				continue;
			case '}':
				if (sp == line)
					level = 0;	/* reset */
				else
					level--;
				if (!level && tydef==middle) {
					tydef=end;
				}
				inbrace--;
				continue;
		}
		if (!level && !inquote && !incomm && gotone == FALSE) {
			if (midtoken) {
				if (endtoken(c)) {
					int f;
					int pfline = lineno;
					if (start_func(&sp,token,tp,&f)) {
						(void)strncpy(tok,token,tp-token+1);
						tok[tp-token+1] = 0;
						getline();
						pfnote(tok, pfline, f);
						gotone = f;	/* function */
					}
					midtoken = FALSE;
					token = sp;
				} else if (intoken(c))
					tp++;
			} else if (begtoken(c)) {
				token = tp = sp;
				midtoken = TRUE;
			}
		}
		if (c == ';'  &&  tydef==end && !inbrace)  /* clean typedefs */
			tydef=none;
		sp++;
		if (c == '\n' || sp > &line[sizeof (line) - BUFFERSIZ]) {
			tp = token = sp = line;
			lineftell = ftell(inf);
			number = gotone = midtoken = inquote = inchar = FALSE;
		}
	}
}

/*
 * This routine  checks to see if the current token is
 * at the start of a function, or corresponds to a typedef
 * It updates the input line so that the '(' will be
 * in it when it returns.
 */
static int
start_func(char **lp, char *token, char *tp, int *f)
{

	reg	char	c,*sp,*tsp;
	static	logical	found;
	logical	firsttok;		/* T if have seen first token in ()'s */
	int	bad;

	*f = 1;
	sp = *lp;
	c = *sp;
	bad = FALSE;
	/* check for the macro cases		*/
	if  ( number && mflag) {
		goto badone;
	}
	if (!number) {		/* space is not allowed in macro defs	*/
		while (iswhite(c)) {
			*++sp = c = getc(inf);
			if (c == '\n') {
				lineno++;
				if (sp > &line[sizeof (line) - BUFFERSIZ])
					goto ret;
				break;
			}
		}
	/* the following tries to make it so that a #define a b(c)	*/
	/* doesn't count as a define of b.				*/
	} else {
		logical	define;

		define = TRUE;
		for (tsp = "define"; *tsp && token < tp; tsp++)
			if (*tsp != *token++) {
				define = FALSE;
				break;
			}
		if (define)
			found = 0;
		else
			found++;
		if (found >= 2) {
			gotone = TRUE;
badone:			bad = TRUE;
			goto ret;
		}
	}

	/* check for the typedef cases      */
	if (!Tflag && !strncmp(token, "typedef", 7)) {
		tydef=begin;
		goto badone;
	}
	if (tydef==begin && (!strncmp(token, "struct", 6) ||
	    !strncmp(token, "union", 5) || !strncmp(token, "enum", 4))) {
		goto badone;
	}
	if (tydef==begin) {
		tydef=end;
		goto badone;
	}
	if (tydef==end) {
		if (c == ',' || c == ';' || c == '[' || c == '(') {
			*f = 0;
			goto ret;
		} else
			goto badone;
	}

	if (c != '(')
		goto badone;
	firsttok = FALSE;
	while ((*++sp=c=getc(inf)) != ')') {
		if (feof(inf))			/* A1473 */
			goto badone;		/* A1473 */
		if (c == '\n') {
			lineno++;
			if (sp > &line[sizeof (line) - BUFFERSIZ])
				goto ret;
		}
		/*
		 * This line used to confuse ctags when it was used in a
		 * file that ctags tried to parse.
		 *	int	(*oldhup)();
		 * This fixes it. A nonwhite char before the first
		 * token, other than a / (in case of a comment in there)
		 * makes this not a declaration.
		 */
		if (begtoken(c) || c=='/') firsttok++;
		else if (!iswhite(c) && !firsttok) goto badone;
	}
	while (iswhite(*++sp=c=getc(inf)))
		if (c == '\n') {
			lineno++;
			if (sp > &line[sizeof (line) - BUFFERSIZ])
				break;
		}
ret:
	*lp = --sp;
	if (c == '\n')
		lineno--;
	(void)ungetc(c,inf);
	return !bad && (!*f || isgood(c));
					/* mods for typedefs */
}

/*
 * Y_entries:
 *	Find the yacc tags and put them in.
 */
static void
Y_entries(void)
{
	register char		*sp, *orig_sp;
	register int		brace;
	register logical	in_rule, toklen;
	char		tok[BUFFERSIZ];

	brace = 0;
	getline();
	pfnote("yyparse", lineno, TRUE);
	while (fgets(line, sizeof line, inf) != NULL) {
		lineno++;
		for (sp = line; *sp; sp++)
			switch (*sp) {
			  case ' ':
			  case '\n':
			  case '\t':
			  case '\f':
			  case '\r':
				break;
			  case '"':
				do {
					while (*++sp != '"')
						continue;
				} while (sp[-1] == '\\');
				break;
			  case '\'':
				do {
					while (*++sp != '\'')
						continue;
				} while (sp[-1] == '\\');
				break;
			  case '/':
				if (*++sp == '*')
					sp = toss_comment(sp);
				else
					--sp;
				break;
			  case '{':
				brace++;
				break;
			  case '}':
				brace--;
				break;
			  case '%':
				if (sp[1] == '%' && sp == line) 
					return;
				break;
			  case '|':
			  case ';':
				in_rule = FALSE;
				break;
			  default:
				if (brace == 0  && !in_rule && 
					(isalpha(*sp) || *sp == '.' || *sp == '_')) {
					orig_sp = sp;
					++sp;
					while (isalnum(*sp) || *sp == '_' || *sp == '.')
						sp++;
					toklen = sp - orig_sp;
					while (isspace(*sp))
						sp++;
					if (*sp == ':' || (*sp == '\0' &&
							   first_char() == ':'))
					{
						(void)strncpy(tok, orig_sp, toklen);
						tok[toklen] = '\0';
						(void)strcpy(lbuf, line);
						lbuf[strlen(lbuf) - 1] = '\0';
						pfnote(tok, lineno, TRUE);
						in_rule = TRUE;
					}
					else
						sp--;
				}
				break;
			}
	}
}

static char *
toss_comment(char *start)
{
	register char	*sp;

	/*
	 * first, see if the end-of-comment is on the same line
	 */
	do {
		while ((sp = index(start, '*')) != NULL)
			if (sp[1] == '/')
				return ++sp;
			else
				start = ++sp;
		start = line;
		lineno++;
	} while (fgets(line, sizeof line, inf) != NULL);
	return (NULL);
}

void
getline(void)
{
	long saveftell = ftell( inf );
	register char *cp;

	(void)fseek( inf , lineftell , 0 );
	(void)fgets(lbuf, sizeof lbuf, inf);
	cp = rindex(lbuf, '\n');
	if (cp)
		*cp = 0;
	(void)fseek(inf, saveftell, 0);
}

static void
free_tree(NODE *node)
{

	while (node) {
		free_tree(node->right);
		free((void *)node);
		node = node->left;
	}
}

static void
add_node(NODE *node, NODE *cur_node)
{
	register int dif;

	dif = strcmp(node->entry,cur_node->entry);
	if (dif == 0) {
		if (node->file == cur_node->file) {
			if (!wflag) {
(void)fprintf(stderr, MSGSTR(M_602, "Duplicate entry in file %s, line %d: %s.  Second entry ignored.\n"), 
node->file,node->lno,node->entry);
			}
			return;
		}
		if (!cur_node->been_warned)
			if (!wflag)
(void)fprintf(stderr, MSGSTR(M_603, "Duplicate function in files %s and %s: %s (Warning only)\n"),
node->file, cur_node->file, node->entry);
		cur_node->been_warned = TRUE;
		return;
	} 
	if (dif < 0) {
		if (cur_node->left != NULL)
			add_node(node,cur_node->left);
		else
			cur_node->left = node;
		return;
	}
	if (cur_node->right != NULL)
		add_node(node,cur_node->right);
	else
		cur_node->right = node;
}

static void
put_entries(reg NODE *node)
{
	reg char	*sp;

	if (node == NULL)
		return;
	put_entries(node->left);
	if (xflag == 0)
		if (node->f) {		/* a function */
			(void)fprintf(outf, "%s\t%s\t%c^",
				node->entry, node->file, searchar);
			for (sp = node->pat; *sp; sp++)
				if (*sp == '\\')
					(void)fprintf(outf, "\\\\");
				else if (*sp == searchar)
					(void)fprintf(outf, "\\%c", searchar);
				else
					(void)putc(*sp, outf);
			(void)fprintf(outf, "%c\n", searchar);
		}
		else {		/* a typedef; text pattern inadequate */
			(void)fprintf(outf, "%s\t%s\t%d\n",
				node->entry, node->file, node->lno);
		}
	else if (vflag)
		(void)fprintf(stdout, "%s %s %d\n",
				node->entry, node->file, (node->lno+63)/64);
	else
		(void)fprintf(stdout, "%-16s %4d %-16s %s\n",
			node->entry, node->lno, node->file, node->pat);
	put_entries(node->right);
}

static char	*dbp = lbuf;
static int	pfcnt;

static int
PF_funcs(FILE *fi)
{

	lineno = 0;
	pfcnt = 0;
	while (fgets(lbuf, sizeof(lbuf), fi)) {
		lineno++;
		dbp = lbuf;
		if ( *dbp == '%' ) dbp++ ;	/* Ratfor escape to fortran */
		while (isspace(*dbp))
			dbp++;
		if (*dbp == 0)
			continue;
		switch (*dbp |' ') {

		case 'i':
			if (tail("integer"))
				takeprec();
			break;
		case 'r':
			if (tail("real"))
				takeprec();
			break;
		case 'l':
			if (tail("logical"))
				takeprec();
			break;
		case 'c':
			if (tail("complex") || tail("character"))
				takeprec();
			break;
		case 'd':
			if (tail("double")) {
				while (isspace(*dbp))
					dbp++;
				if (*dbp == 0)
					continue;
				if (tail("precision"))
					break;
				continue;
			}
			break;
		}
		while (isspace(*dbp))
			dbp++;
		if (*dbp == 0)
			continue;
		switch (*dbp|' ') {

		case 'f':
			if (tail("function"))
				getit();
			continue;
		case 's':
			if (tail("subroutine"))
				getit();
			continue;
		case 'p':
			if (tail("program")) {
				getit();
				continue;
			}
			if (tail("procedure"))
				getit();
			continue;
		}
	}
	return (pfcnt);
}

int
tail(char *cp)
{
	register int len = 0;

	while (*cp && (*cp&~' ') == ((*(dbp+len))&~' '))
		cp++, len++;
	if (*cp == 0) {
		dbp += len;
		return (1);
	}
	return (0);
}

static void
takeprec(void)
{

	while (isspace(*dbp))
		dbp++;
	if (*dbp != '*')
		return;
	dbp++;
	while (isspace(*dbp))
		dbp++;
	if (!isdigit(*dbp)) {
		--dbp;		/* force failure */
		return;
	}
	do
		dbp++;
	while (isdigit(*dbp));
}

static void
getit(void)
{
	register char *cp;
	char c;
	char nambuf[BUFFERSIZ];

	for (cp = lbuf; *cp; cp++)
		;
	*--cp = 0;	/* zap newline */
	while (isspace(*dbp))
		dbp++;
	if (*dbp == 0 || !isalpha(*dbp))
		return;
	for (cp = dbp+1; *cp && intoken(*cp); cp++)
		continue;
	c = cp[0];
	cp[0] = 0;
	(void)strcpy(nambuf, dbp);
	cp[0] = c;
/* --- Use of third parameter is unclear --- */
	pfnote(nambuf, lineno, TRUE);
	pfcnt++;
}

static char *
savestr(char *cp)
{
	register int len;
	register char *dp;

	len = strlen(cp);
	dp = (char *)malloc(len+1);
	(void)strcpy(dp, cp);
	return (dp);
}

/*
 * Return the ptr in sp at which the character c last
 * appears; NULL if not found
 *
 * Identical to v7 rindex, included for portability.
 */

static char *
rindex(register char *sp, register char c)
{
	register char *r;

	r = NULL;
	do {
		if (*sp == c)
			r = sp;
	} while (*sp++);
	return(r);
}

/*
 * lisp tag functions
 * just look for (def or (DEF
 */
static void
L_funcs (FILE *fi)
{
	register int	special;

	pfcnt = 0;
	while (fgets(lbuf, sizeof(lbuf), fi)) {
		lineno++;
		dbp = lbuf;
		if (dbp[0] == '(' &&
		    (dbp[1] == 'D' || dbp[1] == 'd') &&
		    (dbp[2] == 'E' || dbp[2] == 'e') &&
		    (dbp[3] == 'F' || dbp[3] == 'f')) {
			dbp += 4;
			if (striccmp(dbp, "method") == 0 ||
			    striccmp(dbp, "wrapper") == 0 ||
			    striccmp(dbp, "whopper") == 0)
				special = TRUE;
			else
				special = FALSE;
			while (!isspace(*dbp))
				dbp++;
			while (isspace(*dbp))
				dbp++;
			L_getit(special);
		}
	}
}

static void
L_getit(int special)
{
	register char	*cp;
	register char	c;
	char		nambuf[BUFFERSIZ];

	for (cp = lbuf; *cp; cp++)
		continue;
	*--cp = 0;		/* zap newline */
	if (*dbp == 0)
		return;
	if (special) {
		if ((cp = index(dbp, ')')) == NULL)
			return;
		while (cp >= dbp && *cp != ':')
			cp--;
		if (cp < dbp)
			return;
		dbp = cp;
		while (*cp && *cp != ')' && *cp != ' ')
			cp++;
	}
	else
		for (cp = dbp + 1; *cp && *cp != '(' && *cp != ' '; cp++)
			continue;
	c = cp[0];
	cp[0] = 0;
	(void)strcpy(nambuf, dbp);
	cp[0] = c;
	pfnote(nambuf, lineno,TRUE);
	pfcnt++;
}

/*
 * striccmp:
 *	Compare two strings over the length of the second, ignoring
 *	case distinctions.  If they are the same, return 0.  If they
 *	are different, return the difference of the first two different
 *	characters.  It is assumed that the pattern (second string) is
 *	completely lower case.
 */
static int
striccmp(register char *str, register char *pat)
{
	register int	c1;

	while (*pat) {
		if (isupper(*str))
			c1 = tolower(*str);
		else
			c1 = *str;
		if (c1 != *pat)
			return c1 - *pat;
		pat++;
		str++;
	}
	return 0;
}

/*
 * first_char:
 *	Return the first non-blank character in the file.  After
 *	finding it, rewind the input file so we start at the beginning
 *	again.
 */
static int
first_char(void)
{
	register int	c;
	register long	off;

	off = ftell(inf);
	while ((c = getc(inf)) != EOF)
		if (!isspace(c) && c != '\r') {
			(void)fseek(inf, off, 0);
			return c;
		}
	(void)fseek(inf, off, 0);
	return EOF;
}

/*
 * toss_yysec:
 *	Toss away code until the next "%%" line.
 */
static void
toss_yysec(void)
{
	char		buf[BUFFERSIZ];

	for (;;) {
		lineftell = ftell(inf);
		if (fgets(buf, BUFFERSIZ, inf) == NULL)
			return;
		lineno++;
		if (strncmp(buf, "%%", 2) == 0)
			return;
	}
}
