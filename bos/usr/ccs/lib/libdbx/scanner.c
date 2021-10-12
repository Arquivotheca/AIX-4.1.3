static char sccsid[] = "@(#)78	1.18.4.1  src/bos/usr/ccs/lib/libdbx/scanner.c, libdbx, bos41J, 9511A_all 2/7/95 16:33:56";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: beginshellmode, charcon, checkinput,
 *            endshellmode, enterlexclass, eofinput,
 *	      expand, getactuals, getident, getnum, getstring, gobble,
 *	      insertinput, isalnum, isdigit, ishexdigit, isoctdigit, isstdin,
 *	      movetochar, octal, popinput, print_token, scanner_init, setinput,
 *	      shellline, yyerror, yylex, greedy_ident, resetinput, is_type
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
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
 * Debugger scanner.
 */

#include <setjmp.h>
#include "defs.h"
#include "envdefs.h"
#include "scanner.h"
#include "main.h"
#include "keywords.h"
#include "tree.h"
#include "symbols.h"
#include "names.h"
#include "commands.h"
#include "eval.h"
#include "object.h"

#include <sys/types.h>
#include <limits.h>

public String initfile = ".dbxinit";

typedef enum { WHITE, ALPHA, NUM, MISC, OPER } Charclass;
extern enum folding { OFF, ON, PAREN, POSSIBLE } case_inhibit;

private Charclass class[256 + 1];
private Charclass *lexclass = class + 1;

#define isdigit(c) (lexclass[c] == NUM)
#define isalnum(c) (lexclass[c] == ALPHA or lexclass[c] == NUM)
#define isoper(c) (lexclass[c] == OPER)
#define ishexdigit(c) ( \
    isdigit(c) or (c >= 'a' and c <= 'f') or (c >= 'A' and c <= 'F') \
)
#define isoctdigit(c) ((c) >= '0' and (c) <= '7')

public boolean chkalias;
public char scanner_linebuf[MAXLINESIZE];
public char repeat_linebuf[MAXLINESIZE];
public boolean operator_name;

public boolean aliasingon = true;
public File in;
private char *curchar; 
private char *prevchar;
public  char *process_char;

#define MAXINCLDEPTH 10

private struct {
    File savefile;
    Filename savefn;
    int savelineno;
} inclinfo[MAXINCLDEPTH];

private unsigned int curinclindex = 0;

private Token getident();
private Token getnum();
private Token getstring();
public Boolean eofinput();
private char charcon();
int cpp_size_oper(char **);
LongLong strtoll (char *, char **, int);
uLongLong strtoull (char *, char **, int);
static Boolean is_type();

extern int	*envptr;
extern boolean hascobolsrc;

private enterlexclass(class, s)
Charclass class;
String s;
{
    register char *p;

    for (p = s; *p != '\0'; p++) {
	lexclass[*p] = class;
    }
}

public scanner_init()
{
    register Integer i;

    for (i = 0; i < 257; i++) {
	class[i] = MISC;
    }
    enterlexclass(WHITE, " \t");
    enterlexclass(ALPHA, "abcdefghijklmnopqrstuvwxyz");
    enterlexclass(ALPHA, "ABCDEFGHIJKLMNOPQRSTUVWXYZ_$@");
    enterlexclass(NUM, "0123456789");
    enterlexclass(OPER, "!%^&*-+={}|~[]<>/,");
    in = stdin;
    errfilename = nil;
    errlineno = 0;
    curchar = scanner_linebuf;
    repeat_linebuf[0] = scanner_linebuf[0] = '\0';
    chkalias = aliasingon;
}

public char *nextchar()
{
	return curchar;
}

/*
 * Read a single token.
 *
 * The input is line buffered.  Tokens cannot cross line boundaries.
 *
 * There are two "modes" of operation:  one as in a compiler,
 * and one for reading shell-like syntax.  In the first mode
 * there is the additional choice of doing alias processing.
 */

private Boolean shellmode;

public Token yylex()
{
    register int c;
    register char *p, *q;
    register Token t;
    integer n;

    p = curchar;
    if (*p == '\0') {
	shellmode = false;
	chkalias = aliasingon;
	case_inhibit = ON;
	curchar = scanner_linebuf;
	longjmp( envptr, 1 );
    }
    if (!operator_name)
    while (lexclass[*p] == WHITE) {
	p++;
    }
    curchar = p;
    prevchar = curchar;
    process_char = curchar;
    c = *p;
    if (lexclass[c] == ALPHA || (isoper(c) && operator_name)) {
	if (case_inhibit == POSSIBLE)
	    case_inhibit = OFF;
	t = getident(chkalias);
	operator_name = false;
    } else if (lexclass[c] == NUM) {
	if (case_inhibit == POSSIBLE)
	    case_inhibit = OFF;
	if (shellmode) {
	    if (c == '2') {
		if ((*(++curchar)) == '>') {
		   t = ERRDIRECT;
		   curchar++;
		} else {
		   curchar--;
	    	   t = getident(chkalias);
		}
	    } else t = getident(chkalias);
	} else {
	    t = getnum();
	}
    } else {
	++curchar;
	if (((case_inhibit == POSSIBLE) && (c != '(')) || (case_inhibit == ON))
	     case_inhibit = OFF;
	switch (c) {
	    case '':
	        /* This is used by dpi debuggers to pass the address	*/
	        /* of a symbol explicitly by its address through the	*/
	        /* parser.						*/
	        {
		    char	buf[9];
		    char	*bufp = buf;

		    while (isxdigit(*curchar) && bufp-buf < 9)
			*bufp++ = *curchar++;
		    *bufp = '\0';
		    sscanf(buf, "%x", &yylval.y_sym);
		    t = EXPLICITSYM;
		    break;
		}
	    case '\n':
		t = '\n';
		if (errlineno != 0) {
		    errlineno++;
		}
		break;

	    case '"':
	    case '\'':
		t = getstring(c);
		break;

	    case '.':
		if (shellmode) {
		    --curchar;
		    t = getident(chkalias);
		} else if (*curchar == '.') {
		    ++curchar;
		    t = DOTDOT;
		} else if (*curchar == '*') {
		    ++curchar;
		    t = DOTSTAR;
		} else if (isdigit(*curchar)) {
		    --curchar;
		    t = getnum();
		} else {
		    t = '.';
		}
		break;

	    case '-':
		if (shellmode) {
		    --curchar;
		    t = getident(chkalias);
		} else if (*curchar == '>') {
		    ++curchar;
		    if (*curchar == '*') {
			++curchar;
			t = ARROWSTAR;
		    } else
			t = ARROW;
		} else {
		    t = '-';
		}
		break;

	    case ':':
	      if (shellmode) {
		--curchar;
		t = getident(chkalias);
	      } else if (*curchar == ':') {
		++curchar;
		t = SCOPE;
	      }
	      else
		t = ':';
	      break;

	    case '=':
	      if (shellmode) {
		--curchar;
		t = getident(chkalias);
	      } else if (*curchar == '=') {
		++curchar;
		t = EQUAL_EQUAL;
	      } else
		t = '=';
	      break;

	    case '!':
	      if (*curchar == '=') {
		++curchar;
		t = NOT_EQUAL;
	      } else
		t = '!';
	      break;

	    case '<':
	      if (*curchar == '<') {
		++curchar;
		t = LEFT_SHIFT;
	      } else if (*curchar == '=') {
		++curchar;
		t = LESS_EQUAL;
	      } else if (*curchar == '>') {
		++curchar;
		t = LESS_GREATER;
	      } else
		t = '<';
	      break;

	    case '>':
	      if (*curchar == '>') {
		++curchar;
		t = RIGHT_SHIFT;
	      } else if (*curchar == '=') {
		++curchar;
		t = GREATER_EQUAL;
	      } else
		t = '>';
	      break;

	    case '#':
	        if (shellmode) {
		  --curchar;
		  t = getident(chkalias);
	        } else if (not isterm(in)) {
		    *p = '\0';
		    curchar = p;
		    t = '\n';
		    ++errlineno;
		} else {
		    t = '#';
		}
		break;

	    case '\\':
		if (*(p+1) == '\n') {
		    n = MAXLINESIZE - (p - &scanner_linebuf[0]);
		    if (n > 1) {
			if (fgets(p, n, in) == nil) {
			    t = 0;
			} else {
			    curchar = p;
			    t = yylex();
			}
		    } else {
			t = '\\';
		    }
		} else {
                    if (shellmode)
		       t = getident(chkalias);
                    else
                       t = '\\';
		}
		break;

	    case '&':
		if ((shellmode) && (*curchar == '1')) {
		    ++curchar;
		    t = ERRTOOUT;
		} else if (*curchar == '&') {
		    ++curchar;
		    t = AND;
		} else {
		    t = '&';
		}
		break;

	    case '|':
		if (*curchar == '|') {
		    ++curchar;
		    t = OR;
		} else {
		    t = '|';
		}
		break;

	    case ']':
	      if (shellmode) {
	  	--curchar;
	  	t = getident(chkalias);
	      } else {
		q = curchar;
	        while (lexclass[*q] == WHITE) {
		    q++;
	        }
		if (*q == '[') {
		    curchar = ++q;
		    t = BRACKETS;
		} else
		    t = ']';
	      }
	      break;

	    case EOF:
		t = 0;
		break;

	    case '[':
	    case '*':
		if (shellmode) {
		  --curchar;
		  t = getident(chkalias);
		} else {
		  t = c;
		}
		break;
		
	    default:
		if (shellmode and index("!*<>()[];", c) == nil) {
		    --curchar;
		    t = getident(chkalias);
		} else {
		    t = c;
		    if (case_inhibit == POSSIBLE) {
			 if (t == '(')
			 	case_inhibit = PAREN;
		    } else if (case_inhibit == PAREN) {
			 if (t == ')')
			 	case_inhibit = OFF;
		    }
		}
		break;
	}
    }
    chkalias = false;
#   ifdef LEXDEBUG
	if (lexdebug) {
	    (*rpt_error)(stderr, "yylex returns ");
	    print_token( rpt_error, stderr, t);
	    (*rpt_error)(stderr, "\n");
	}
#   endif
    return t;
}

/*
 * Put the given string before the current character
 * in the current line, thus inserting it into the input stream.
 */

public insertinput (s)
String s;
{
    register char *p, *q;
    int need, avail, shift;

    q = s;
    need = strlen(q);
    avail = curchar - &scanner_linebuf[0];
    if (need <= avail) {
	curchar = &scanner_linebuf[avail - need];
	p = curchar;
	while (*q != '\0') {
	    *p++ = *q++;
	}
    } else {
	p = curchar;
	while (*p != '\0') {
	    ++p;
	}
	shift = need - avail;
	if (p + shift >= &scanner_linebuf[MAXLINESIZE]) {
	    error( catgets(scmc_catd, MS_scanner, MSG_274,
						 "alias expansion too large"));
	}
	for (;;) {
	    *(p + shift) = *p;
	    if (p == curchar) {
		break;
	    }
	    --p;
	}
	p = &scanner_linebuf[0];
	while (*q != '\0') {
	    *p++ = *q++;
	}
	curchar = &scanner_linebuf[0];
    }
}

/*
 * Get the actuals for a macro call.
 */

private String movetochar (str, c)
String str;
char c;
{
    register char *p;

    p = str;
    while (*p != c) {
	if (*p == '\0') {
	    error( catgets(scmc_catd, MS_scanner, MSG_310,
						 "missing ')' in macro call"));
	} else if (*p == ')') {
	    error( catgets(scmc_catd, MS_scanner, MSG_313,
				       "not enough parameters in macro call"));
	} else if (*p == ',') {
	    error( catgets(scmc_catd, MS_scanner, MSG_317,
					 "too many parameters in macro call"));
	}
	++p;
    }
    return p;
}

private String *getactuals (n)
integer n;
{
    String *a;
    register char *p;
    int i;

    if( n <= 0 ) {
	return NULL;
    }
    a = newarr(String, n);
    p = curchar;
    while (*p != '(') {
	if (lexclass[*p] != WHITE) {
	    error( catgets(scmc_catd, MS_scanner, MSG_318,
						 "missing actuals for macro"));
	}
	++p;
    }
    ++p;
    for (i = 0; i < n - 1; i++) {
	a[i] = p;
	p = movetochar(p, ',');
	*p = '\0';
	++p;
	if (symcase == lower)
	   lowercase(a[i]);
	else if (symcase == upper)
	   uppercase(a[i]);
    }
    a[n-1] = p;
    p = movetochar(p, ')');
    *p = '\0';
    if (symcase == lower)
	lowercase(a[n-1]);
    else if (symcase == upper)
	uppercase(a[i]);
    curchar = p + 1;
    return a;
}

/*
 * Do command macro expansion, assuming curchar points to the beginning
 * of the actuals, and we are not in shell mode.
 */

private expand (pl, str)
List pl;
String str;
{
    char buf[4096], namebuf[100];
    register char *p, *q, *r;
    String *actual;
    Name n;
    integer i;
    boolean match;

    if (pl == nil) {
	insertinput(str);
    } else {
	actual = getactuals(list_size(pl));
	p = buf;
	q = str;
	while (*q != '\0') {
	    if (p >= &buf[4096]) {
		error( catgets(scmc_catd, MS_scanner, MSG_274,
						 "alias expansion too large"));
	    }
	    if (lexclass[*q] == ALPHA) {
		r = namebuf;
		do {
		    *r++ = *q++;
		} while (isalnum(*q));
		*r = '\0';
		i = 0;
		match = false;
		foreach(Name, n, pl)
		    if (streq(ident(n), namebuf)) {
			match = true;
			break;
		    }
		    ++i;
		endfor
		if (match) {
		    r = actual[i];
		} else {
		    r = namebuf;
		}
		while (*r != '\0') {
		    *p++ = *r++;
		}
	    } else {
		*p++ = *q++;
	    }
	}
	*p = '\0';
	insertinput(buf);
    }
}

/*
 * Parser error handling.
 */

public yyerror(s)
String s;
{
    register char *p;
    register integer start;
    static nl_catd	yacc_catd;	/* Message Catalog descriptor for
					 * messages from yacc */
    static char	*syntax_msg;		/* Will be used to identify yacc message
					 * to compare with to check for syntax
					 * errors */

    if( yacc_catd == nil ) {
	/*
	 * Open the catalog of yacc user messages
	 */
	yacc_catd = catopen("yacc_user.cat", NL_CAT_LOCALE);

	/*
	 * Extract the syntax error message for comparisons
	 * The location of this message should not change.
	 */
	syntax_msg = catgets( yacc_catd, 1, 3, "syntax error" );
    }

    if (streq(s, syntax_msg)) {
	beginerrmsg();
	p = prevchar;
	start = p - &scanner_linebuf[0];
	if (p > &scanner_linebuf[0]) {
	    while (lexclass[*p] == WHITE and p > &scanner_linebuf[0]) {
		--p;
	    }
	}
	(*rpt_error)(stderr, "%s", scanner_linebuf);
	if (start != 0) {
	    (*rpt_error)(stderr, "%*c", start, ' ');
	}
	if (p == &scanner_linebuf[0]) {
	    (*rpt_error)(stderr,  catgets(scmc_catd, MS_scanner, MSG_329,
						    "^ unrecognized command"));
	} else {
	    (*rpt_error)(stderr,  catgets(scmc_catd, MS_scanner, MSG_334,
							    "^ syntax error"));
	}
	enderrmsg();
    } else {
	error(s);
    }
}

/*
 * Eat the current line.
 */

public gobble ()
{
    curchar = scanner_linebuf;
    scanner_linebuf[0] = '\0';
}

/*
 * After having read a new line, need to reset curchar.
 */

public resetinput ()
{
    curchar = scanner_linebuf;
}

/*
 * NAME: getident
 *
 * FUNCTION: Scan an identifier.
 *
 * PARAMETERS:
 *      chkalias      - flag - if true, check first to see if
 *                      it's an alias.  Otherwise, check to see
 *                      if it's a keyword.
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: a Token
 */

private Token getident (chkalias)
boolean chkalias;
{
    char buf[1024];
    char *p, *q;
    register Token t;
    List pl;
    String str;
    Boolean found_quote = false;
    Boolean backquote = false;

    p = curchar;
    q = buf;
    if (shellmode) {
	do {
            if ( *p == '\\')   /* skip over backslash and load next */
               *p++;           /* character no matter what it is.   */
	    if ( *p == '"' || *p == '\'' || *p == '`') {
	       /* handle string appended to ident, e.g. SYSTEM="a or b" */
	       Token x;
	       curchar = p + 1;		/* skip over first quote */
	       x = getstring(*p);
	       if (*p == '`') {		/* use meta_expand to execute */
		  *q++ = '`';		/* backquoted commands        */
	 	  backquote = true;
	       }
	       if (x == CHAR)
		  *q++ = yylval.y_char;
	       else if (x = STRING) {
		  strcpy(q, yylval.y_string);
		  q += strlen(yylval.y_string);
	       }
	       if (*p == '`') *q++ = '`';	/* put backquote back */
	       p = curchar;
	       found_quote = true;
	    } else 
	       *q++ = *p++;
	} while (index(" >\t\n;", *p) == nil);
/*	} while (index(" \t\n!&<>()[]*'\"", *p) == nil); */
    } else {
        greedy_ident(&q, &p);
    }
    curchar = process_char = p;
    *q = '\0';
    /* This section attempts to deal with multi-word tokens such as
       unsigned long and signed char */
    if ((!(shellmode || chkalias)) && (lexclass[*curchar] == WHITE))
    {
      /*  handle struct, union, class and enum keywords  */

      /*  for c - strip off the keyword and place "$$" in front
            of the name.  For any other language, just discard
            the keyword  */
      if (streq(buf,"struct") || streq(buf,"union")
            || streq(buf,"class") || streq(buf,"enum"))  
      {
        do
        {
          curchar++;
        } while (lexclass[*curchar] == WHITE);

        p = curchar;

        if (lexclass[*p] == ALPHA) 
        {
          q = buf;

          if (curlang == cLang) 
          {
            *q++ = '$'; *q++ = '$';
          }

	  do {
	    *q++ = *p++;
	  } while (isalnum(*p));
	}
      }
      /*  handle unsigned and signed prefixes  */
      if (streq(buf,"unsigned") || streq(buf,"signed"))
      {
        do
        {
          curchar++;
        } while (lexclass[*curchar] == WHITE);

        p = curchar;

        if (lexclass[*p] == ALPHA)
        {
          if (streq(buf,"signed") && strncmp(p,"char", 4))
            /*  strip off "signed"  except "signed char" 
                since AIX treat "char" as unsigned */
            q = buf;
          else
            *q++ = ' ';

          do {
            *q++ = *p++;
          } while (isalnum(*p));
        }
        *q = '\0';
        curchar = p;
      }
      /*  handle long long, unsigned long long and long double */
      /*  NOTE : unsigned long double is not valid - hence next line  */
      if (streq(buf,"long") || streq(buf, "unsigned long"))
      {
        while (lexclass[*curchar] == WHITE)
        {
          curchar++;
        } 

        p = curchar;

        /*  handle long long and unsigned long long  */
        if (!strncmp (p, "long", 4))
        {
          *q++ = ' ';
          do {
            *q++ = *p++;
          } while (isalnum(*p));
        }
        /*  handle long double  */
        else if (streq(buf,"long"))
        {
          if (!strncmp (p, "double", 6))
          {
            *q++ = ' ';
            do {
              *q++ = *p++;
            } while (isalnum(*p));
          }
        }
      }
 
      *q = '\0';
      curchar = p;
    }
    yylval.y_name = identname(buf, false);
    if (found_quote && !backquote) {
        yylval.y_string = strdup(buf);
        t = STRING;
    } else if (chkalias) {
	if (shellmode) {
	    t = NAME;
	} else if (findalias(yylval.y_name, &pl, &str)) {
	    expand(pl, str);
	    while (lexclass[*curchar] == WHITE) {
		++curchar;
	    }
            if (lexclass[*curchar] == NUM)
                t = getnum();
	    else if (!isalpha(*curchar))
		t = yylex();
	    else if (pl == nil) {
		t = getident(false);
	    } else {
		t = getident(true);
	    }
	} else {
	    t = findkeyword(yylval.y_name, NAME);
	    if ((case_inhibit == OFF) && (t == NAME)) {
		if (symcase == lower) {
		    lowercase(buf);
		    yylval.y_name = identname(buf, false);
		}
		else if (symcase == upper) {
		    uppercase(buf);
		    yylval.y_name = identname(buf, false);
		}
	    }
	}
    } else if (shellmode) {
	t = NAME;
    } else {
	t = findkeyword(yylval.y_name, NAME);
	if ((t == NAME) && (case_inhibit == OFF)) {
	    if (symcase == lower) {
	 	lowercase(buf);
	 	yylval.y_name = identname(buf, false);
	    }
	    else if (symcase == upper) {
	 	uppercase(buf);
	 	yylval.y_name = identname(buf, false);
	    }
	}
    }
    if (t == NAME)
    {
      /*  NOTE : valid casts have forms :

                   type (exp)
                   (type) exp
                   (type *) exp
                   (type &) exp
                   exp \ type

          It is difficult to parse the first four because
          of ambiguities in the grammer so we decide here if
          we have a typecast.  We are looking at the 'next' 
          character to see if we actually have a cast, as
          opposed to a type being used in some other type
          of command (whatis double).  One exception is
          print sizeof (type), which is being handled in
          commands.y.  It is also difficult to decide if
          some C++ names are types without looking at the
          context.  We also deal with these problems in
          commands.y (look for cpp_name).  */

      /*  skip over white space without changing curchar  */
      while (lexclass[*p] == WHITE)
      {
        p++;
      } 
      /*  if the next character is a '(', ')' or a '*'  */
      if ((*p == ')') || (*p == '(') || (*p == '*') || (*p == '&'))
      {
        /*  check to see if the name is actually a type  */
        if (is_type(yylval.y_name))
          t = TYPECAST;
      }
    }
    if ((case_inhibit == ON) && (t != ALIAS))
	case_inhibit = POSSIBLE;
    else if (case_inhibit == POSSIBLE)
	case_inhibit = OFF;

    return t;
}

/*
 * NAME: is_type
 *
 * FUNCTION: Determines if a name is a type
 *
 * PARAMETERS:
 *      name    - Name to check
 *
 * RETURNS: True if name is a type; False otherwise
 */

static Boolean is_type(name)
Name name;
{
  Symbol s;
  Boolean type_seen = false;

  s = lookup(name);

  /*  look at all instances of this symbol  */
  while (s)
  {
    /*  check the name in case 2 different names hash to the same
          thing.  Example - "signed char" and "auth_none"  */
    if (streq(name->identifier,s->name->identifier))
    {
      /*  if this is a type, remember it  */
      if (s->class == TYPE)
      {
        type_seen = true;
      }
      else if ((s->class == TAG)
        && ((s->type->class == RECORD)
         || (s->type->class == SCAL)
         || (s->type->class == PACKRECORD)
         || (s->type->class == UNION)
         || (s->type->class == CLASS)
         || (s->type->class == BASECLASS)
         || (s->type->class == NESTEDCLASS)
         || (s->type->class == CPPREF)
         || (s->type->class == FRIENDCLASS)))
      {
        type_seen = true;
      }
      /*  else if this is active  */
      else if (isactive(container(s)))
      {
        return (false);
      }
    }
    s = s->next_sym;
  }

  if (type_seen)
  {
    return (true);
  }
  return (false);
}

/*
 * greedy_ident(pdest, psrc) - If there are any COBOL src files, hyphens
 *   are legal in identifiers.
 */
greedy_ident(pdest, psrc)
     char **pdest, **psrc;
{
int x;
char *beginptr = *pdest;

  if (hascobolsrc) {		/* Any COBOL src files? */
    do {
      *(*pdest)++ = *(*psrc)++;
    } while (isalnum(**psrc) || **psrc == '-');
    if (*((*psrc)-1) == '-') {	/* trailing '-' illegal */
      --(*pdest);
      --(*psrc);
    }
  }
  else {
    do {
      *(*pdest)++ = *(*psrc)++;
    } while (isalnum(**psrc) || (isoper(**psrc) && operator_name));
    **pdest = '\0';
    if (streq(beginptr, "operator") && (x = cpp_size_oper(psrc))) {
        while (x) {
           *(*pdest)++ = *(*psrc)++;
           --x;
        }
    }
  }
}

/*
 * get the size of the c++ operator that follows the operator keyword.
 */

int cpp_size_oper(psrc) 
char **psrc;
{
    int x;
    char *cptr;

    while (**psrc == ' ') 
	*psrc += 1;

    if (isalpha(**psrc)) 
    {
        cptr = *psrc;
	*psrc -= 1;
        for (x = 1; isalpha(*cptr); cptr++, x++);
    }
    else 
    {
        char cp = **psrc;
        char cp1 = *(*psrc + 1);
        char cp2 = *(*psrc + 2);

	x = 0;
        switch(cp) 
        {  
            case '+':           /* +, ++, +=, &, &&, &=, |, ||, |= */
            case '&':
            case '|':
                x = (cp1 == '=' || cp1 == cp) ? 2 : 1;
                break;
            case '-':           /* -, --, -=, ->, ->* */
                if (cp1 == '=' || cp1 == '-')
                    x = 2;
                else if (cp1 == '>')
                    x = (cp2 == '*') ? 3 : 2;
                else
                    x = 1;
                break;
            case '*':            /* *, *=, /, /=, %, %=, ^, ^=, !, !=, =, == */
            case '/':
            case '%':
            case '^':
            case '!':
            case '=':
                x = (cp1 == '=') ? 2 : 1;
                break;
            case '~':            /* ~ and , */
            case ',':
                x = 1;
                break;
            case '<':            /* <, <=, <<, <<=, >, >=, >>, >>= */
            case '>':
                if (cp1 == '=')
                    x = 2;
                else if (cp1 == cp) 
                    x = (cp2 == '=') ? 3 : 2;
                else
                    x = 1;
                break;
            case '[':           /* [] and () */
            case '(':
                x = 2;
        }  
    }
    return x;
}

/*
 * Convert a string of octal digits to an integer.
 */

private int octal(s)
String s;
{
    register Char *p;
    register Integer n;

    n = 0;
    for (p = s; *p != '\0'; p++) {
	n = 8*n + (*p - '0');
    }
    return n;
}

/*
 * NAME: getnum
 *
 * FUNCTION: Scan a number
 *
 * PARAMETERS:
 *      none
 *
 * RECOVERY OPERATION: NONE NEEDED
 *
 * DATA STRUCTURES: NONE
 *
 * RETURNS: a Token
 */

private Token getnum()
{
    char buf[1024];
    register Char *p, *q;
    register Token t;
    Integer base;
    Boolean quad_num = false;
    quadf _qatof();

    p = curchar;
    q = buf;
    if (*p == '0') {
	if (*(p+1) == 'x') {
	    p += 2;
	    curchar += 2;
	    base = 16;
	} else if (*(p+1) == 't') {
	    p += 2;
	    curchar += 2;
	    base = 10;
	} else if (varIsSet("$hexin")) {
	    base = 16;
	} else {
	    base = 8;
	}
    } else if (varIsSet("$hexin")) {
	base = 16;
    } else if (varIsSet("$octin")) {
	base = 8;
    } else {
	base = 10;
    }
    if (base == 16) {
	 while (ishexdigit(*p)) {
	    *q++ = *p++;
	}
    } else if (base == 8) {
	 while (isoctdigit(*p)) {
	    *q++ = *p++;
	}
    } else {
	 while (isdigit(*p)) {
	    *q++ = *p++;
	}
    }
    if ((*p == '.') && ( *(p+1) != '.')) {   /* Avoid "..", this is a range */
	do {
	    *q++ = *p++;
	} while (isdigit(*p));
	if (*p == 'e' or *p == 'E' or *p == 'q' or *p == 'Q'
         or *p == 'l' or *p == 'L') {
	    if (*p == 'q' or *p == 'Q' or *p == 'l' or *p == 'L')
	 	quad_num = true;
	    p++;
	    if (*p == '+' or *p == '-' or isdigit(*p)) {
		if (quad_num)
		  *q++ = 'q';
		else
		  *q++ = 'e';
		do {
		    *q++ = *p++;
		} while (isdigit(*p));
	    }
	}
	*q = '\0';
	if (quad_num) {
	  yylval.y_quad = _qatof(buf);
	  t = QUAD;
	} else {
	  yylval.y_real = atof(buf);
	  t = REAL;
	}
    } else {
        LongLong temp;
        uLongLong utemp = 0;
        unsigned char type = ISLONG;
 
	*q = '\0';

        /*  if the next 3 characters are not lld, llu, llx or llo  */
        if (strncmp(p, "lld", 3) && strncmp(p, "llu", 3)
         && strncmp(p, "llx", 3) && strncmp(p, "llo", 3))
        {       
          if ((*p == 'u') || (*p == 'U'))
          {
            type |= ISUNSIGNED;
            p++;
          }
          if ((*p == 'l') || (*p == 'L'))
          {
            p++;

            if ((*p == 'l') || (*p == 'L'))
            {
              type |= ISLONGLONG;
              p++;
            }
          }
        }
        switch (type)
        {
          case ISUNSIGNEDLONGLONG:
            t = ULONGLONG;
            yylval.y_longlong =
                       strtoull(buf, (char **) NULL, base);
            break;

          case ISLONGLONG:
            t = LONGLONG;
            if (base == 10)
              yylval.y_longlong = strtoll(buf, (char **) NULL, base);
            else
              yylval.y_longlong = strtoull(buf, (char **) NULL, base);
            break;

          case ISUNSIGNEDLONG:
            t = UINT;
            yylval.y_int = strtoul(buf, (char **) NULL, base);
            break;

          case ISLONG:
            t = INT;
            if (base == 10)
              yylval.y_int = strtol(buf, (char **) NULL, base);
            else
              yylval.y_int = strtoul(buf, (char **) NULL, base);
            break;
        }
    }
    curchar = process_char = p;
    return t;
}

/*
 * Scan a string.
 */

private Token getstring (quote)
char quote;
{
    register char *p, *q;
    char buf[MAXLINESIZE];
    int i, mbl;
    boolean endofstring;
    Token t;

    p = curchar;
    q = buf;
    endofstring = false;
    while (not endofstring) {
	if (*p == '\\' and *(p+1) == '\n') {
	    if (fgets(scanner_linebuf, MAXLINESIZE, in) == nil) {
		error( catgets(scmc_catd, MS_scanner, MSG_335,
						     "non-terminated string"));
	    }
	    p = &scanner_linebuf[0] - 1;
	} else if (*p == '\n' or *p == '\0') {
	    error( catgets(scmc_catd, MS_scanner, MSG_335,
						     "non-terminated string"));
	    endofstring = true;
	} else if (*p == quote) {
	    endofstring = true;
	} else {
	    curchar = p;
            if ((mbl = mblen(p, MB_LEN_MAX)) < 2)
              *q++ = charcon(p);
            else
            {
	      for(i = mbl; i > 0; --i)
		*q++ = *p++;
	      curchar += mbl - 1;
            }
	    p = curchar;
	}
	p++;
    }
    curchar = p;
    *q = '\0';
    if (quote == '\'' and buf[1] == '\0') {
	yylval.y_char = buf[0];
	t = CHAR;
    } else {
	yylval.y_string = strdup(buf);
	t = STRING;
    }
    return t;
}

/*
 * Process a character constant.
 * Watch out for backslashes.
 */

private char charcon (s)
String s;
{
    register char *p, *q;
    char c, buf[10];

    p = s;
    if (*p == '\\') {
	++p;
	switch (*p) {
	    case '\\':
		c = '\\';
		break;

	    case 'n':
		c = '\n';
		break;

	    case 'r':
		c = '\r';
		break;

	    case 't':
		c = '\t';
		break;

	    case '\'':
	    case '"':
		c = *p;
		break;

	    default:
		if (isoctdigit(*p)) {
		    q = buf;           /* If the next character is a number  */
		    *q++ = *p++;       /* we need to take it and any others  */
		    if isoctdigit(*p)  /* up to a total of three. This is    */
		       *q++ = *p++;    /* correct definition of an arbitrary */
		    if isoctdigit(*p)  /* byte-sized bit pattern: '\ddd'...  */
		       *q++ = *p++;    /* the old code took any amount of    */
		    *q = '\0';         /* digits which is wrong.             */
		    c = (char) octal(buf);
		--p;
		}
                else c = *p;   /* any other case...ignore backslash */
		break;
	}
	curchar = p;
    } else {
	c = *p;
    }
    return c;
}

/*
 * Input file management routines.
 */

public setinput(filename)
Filename filename;
{
    File f;

    f = fopen(filename, "r");
    if (f == nil) {
	error( catgets(scmc_catd, MS_scanner, MSG_337, "cannot open %s"),
								     filename);
    } else {
	if (curinclindex >= MAXINCLDEPTH) {
	    error( catgets(scmc_catd, MS_scanner, MSG_339,
			    "unreasonable input nesting on \"%s\""), filename);
	}
	inclinfo[curinclindex].savefile = in;
	inclinfo[curinclindex].savefn = errfilename;
	inclinfo[curinclindex].savelineno = errlineno;
	curinclindex++;
	in = f;
	errfilename = filename;
	errlineno = 1;
    }
}

public Boolean eofinput()
{
    register Boolean b;

    if (curinclindex == 0) {
	if (isterm(in)) {
	    (*rpt_output)(stdout, "\n" );
	    clearerr(in);
	    b = false;
	} else {
	    b = true;
	}
    } else {
	fclose(in);
	--curinclindex;
	in = inclinfo[curinclindex].savefile;
	errfilename = inclinfo[curinclindex].savefn;
	errlineno = inclinfo[curinclindex].savelineno;
	b = false;
    }
    return b;
}

/*
 *  Return whether we have a input file.
 */

public Boolean checkinput(fd)
int fd;		/* file descriptor                                      */
{
    int j;
    register Boolean b = false;

    if (fd == in->_file)
       b = true;
    else
        for (j = 0; j < curinclindex; j++)
            if (fd == inclinfo[curinclindex].savefile->_file) {
               b = true;
               break;
	    }
    return b;
	
}

/*
 * Pop the current input.  Return whether successful.
 */

public Boolean popinput()
{
    Boolean b;

    if (curinclindex == 0) {
	b = false;
    } else {
	b = (Boolean) (not eofinput());
    }
    return b;
}

/*
 * Return whether we are currently reading from standard input.
 */

public Boolean isstdin()
{
    return (Boolean) (in == stdin);
}

/*
 * Send the current line to the shell.
 */

public shellline()
{
    register char *p;

    p = curchar;
    while (*p != '\0' and (*p == '\n' or lexclass[*p] == WHITE)) {
	++p;
    }
    (*rpt_shell)();
    shell(p);
    if (*p == '\0' and isterm(in)) {
	(*rpt_output)(stdout, "\n" );
    }
    erecover();
}

/*
 * Read the rest of the current line in "shell mode".
 */

public beginshellmode()
{
    shellmode = true;
}

/*
 * Read the rest of the current line in "non-shell mode".
 */

public endshellmode()
{
    shellmode = false;
}

/*
 * Print out a token for debugging.
 */

public print_token( report, f, t)
int (*report)( );
File f;
Token t;
{
    if (t == '\n') {
	(*report)(f, "char '\\n'");
    } else if (t == EOF) {
	(*report)(f, "EOF");
    } else if (t < 256) {
	(*report)(f, "char '%c'", t);
    } else {
	(*report)(f, "\"%s\"", keywdstring(t));
    }
}

public char *gettokenstring( t )
Token t;
{
    return(keywdstring(t));
}
