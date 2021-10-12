static char sccsid[] = "@(#)90	1.11.1.13  src/bos/usr/bin/localedef/err.c, cmdnls, bos412, 9446B 11/15/94 11:09:14";
/*
 * COMPONENT_NAME: (CMDNLS) Locale Database Commands
 *
 * FUNCTIONS: error, diag_error, safe_malloc, yyerror
 *
 * ORIGINS: 27, 85
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 * 
 * OSF/1 1.2
 */

#include <swvpd.h>
#include <stdio.h>
#include <stdarg.h>
#include <nl_types.h>
#include "err.h"

/* This is the default message text array for localedef error messages */
static char *err_fmt[]={
"",
"The symbol '%s' is not the correct type.\n",
"Could not open '%s' for read.\n",
"Internal error. [file %s - line %d].\n",
"Syntax Error: expected %d arguments and received %d arguments.\n",
"Illegal limit in range specification.\n",
"Memory allocation failure: [line %d -- module %s].\n",
"Could not open temporary file '%s' for write.\n",
"The '%s' character is longer than <mb_cur_max>.\n",
"The '%s' character is undefined.\n",
"The start of the range must be numerically less than the\n\
end of the range.\n",
"The symbol range containing %s and %s is incorrectly formatted.\n",
"Illegal character reference or escape sequence in '%s'.\n",
"You have specified different names for the same character '%d'.\n",
"The character symbol '%s', has already been specified.\n",
"There are characters in the codeset which are unspecified\n\
in the collation order.\n",
"Illegal decimal constant '%s'.\n",
"Illegal octal constant '%s'.\n",
"Illegal hexadecimal constant '%s'.\n",
"Missing closing quote in string '%s'.\n",
"Illegal character, '%c', in input file.  Character will be ignored.\n",
"Character for escape_char statement missing.  Statement ignored.\n",
"Character for comment_char statement missing.  Statement ignored.\n",
"'%c' is not a POSIX Portable Character. Statement ignored.\n",
"The character symbol '%s' is missing the closing '>'. The '>'\
 will be added.\n",
"Unrecognized keyword, '%s', statement ignored.\n",
"The character symbol '%s' is undefined in the range '%s...%s'.\n\
The symbol will be ignored.\n",
"The encoding specified for the '%s' character is unsupported.\n",
"The character, '%s', has already been assigned a weight.\n\
Specification ignored.\n",
"The character %s in range %s...%s already has a collation weight.\
 Range ignored.\n",
"No toupper section defined for this locale sourcefile.\n",
"The use of the \"...\" keyword assumes that the codeset is contiguous\n\
between the two range endpoints specified.\n",
"The collation substitution,  %s%s, contains a symbol which is not\n\
a character.\n",
"usage: localedef [-c] [-f charmap] [-i locsrc]\\\n\t\
 [-L ld opts] [-m methfile] locname\n",
"localedef [ERROR]: FILE: %s, LINE: %d, CHAR: %d\n",
"localedef [WARNING]: FILE: %s, LINE: %d, CHAR: %d\n",
"localedef [ERROR]: FILE: %s, LINE: %d, CHAR: %d\n\
Syntax Error.\n",
"Specific collation weight assignment is not valid when no sort\n\
keywords have been specified.\n",
"The compile was unsuccessful.  Required header files\n\
sys/localedef.h and sys/lc_core.h may be the wrong\n\
version or may be missing.\n",
"The <mb_cur_min> keyword must be defined as 1, you have\
 defined it \nas %d.\n",
"The <code_set_name> must contain only characters from the\
 portable \ncharacter set, %s is not valid.\n",
"The collation directives forward and backward are mutually\n\
exclusive.\n",
"Received too many arguments, expected %d.\n",
"The %s category has already been defined.\n",
"The %s category is empty.\n",
"Unrecognized category %s is not processed by localedef.\n",
"The POSIX defined categories must appear before any unrecognized\
 categories.\n",
"The file code for the digit %s is not one greater than the \n\
file code for %s.\n",
"The process code for the digit %s is not one greater than the \n\
process code for %s.\n",
"The symbol %s has already been defined. Ignorning definition as a \n\
collating-symbol.\n",
"Locale does not conform to POSIX specifications for the\
 LC_CTYPE upper \nkeyword.\n",
"Locale does not conform to POSIX specifications for the\
 LC_CTYPE lower \nkeyword.\n",
"Locale does not conform to POSIX specifications for the\
 LC_CTYPE alpha \nkeyword.\n",
"Locale does not conform to POSIX specifications for the\
 LC_CTYPE space \nkeyword.\n",
"Locale does not conform to POSIX specifications for the\
 LC_CTYPE cntrl \nkeyword.\n",
"Locale does not conform to POSIX specifications for the\
 LC_CTYPE punct \nkeyword.\n",
"Locale does not conform to POSIX specifications for the\
 LC_CTYPE graph \nkeyword.\n",
"Locale does not conform to POSIX specifications for the\
 LC_CTYPE print \nkeyword.\n",
"Locale does not specify the minimum required for the LC_CTYPE\
 upper keyword.\n",
"Locale does not specify the minimum required for the LC_CTYPE\
 lower keyword.\n",
"Locale does not specify the minimum required for the LC_CTYPE\
 space keyword.\n",
"Locale does not specify only '0', '1', '2', '3', '4', '5',\
 '6', '7', '8',\nand '9' for LC_CTYPE digit keyword.\n",
"Either locale does not specify '0', '1', '2', '3', '4', '5',\
 '6', '7', '8',\n'9', 'a' through 'f', and 'A' through 'F' for LC_CTYPE\
 xdigit keyword\nor locale does not specify a sufficient number of\
 consecutive symbols.\n",
"The number of operands to LC_COLLATE order_start exceeds COLL_WEIGHTS_MAX.\n",
"The methods for __mbtopc, __mbstopcs, __pctomb, __pcstombs, mbtowc, \n\
mbstowcs, wctomb, wcstombs, mblen, wcwidth, and wcswidth must be specified\n\
in the <methodfile>.\n",
"Locale can not mix private method table methods and global method \n table\
 methods. \n",
"Temporary method table failed to load.\n",
"Unable to exec /usr/bin/sh to process intermediate files.\n",
"Unable to run program %s.\n",
"Locale generation is not supported on this system.\n",
"Missing end of symbol in string '%s'.\n",
"The symbolic name '%s' is not defined in the character map.\n\
This character set may not be POSIX compliant.\n",
"Unable to load locale \"%s\" for copy directive.\n",
"Locale name longer than PATH_MAX (%d).\n",
"\"%s\" was not declared in a charclass statement.\n",
"Collating symbols such as %s can not have explicit weights.\n\
Specification ignored.\n",
"Ellipsis on the right hand side may only be used\n\
with ellipsis or UNDEFINED symbols on the left hand side.\n",
"Ellipsis may not be used as one of the characters in\n\
a one-to-many mapping.\n",
"Collating symbols may not be used as one of the\n\
characters in a one-to-many mapping.\n",
"Stack overflow error.\n",
"Required symbolic name %s not defined in character map file.\n",
"The symbol %s is too long.\n\
It will be truncated to %d bytes.\n",
"The '%s' character is undefined.\n\
This character along with any range statements it may be in\n\
will be ignored.\n",
"More weights were defined for character %s than\n\
were specified with the order_start keyword.\n",


};

/* 
  Global indicating if an error has been encountered in the source or not.
  This flag is used in conjunction with the -c option to decide if a locale
  should be created or not.
*/
int err_flag = 0;
static nl_catd catd = 0;


/*
*  FUNCTION: usage
*
*  DESCRIPTION:
*  Prints a usage statement to stderr and exits with the specified status
*/
void usage(int status)
{
    if (catd == NULL)
	catd = catopen("localedef.cat", NL_CAT_LOCALE);

    fprintf(stderr, 
	    catgets(catd, LOCALEDEF, ERR_USAGE, err_fmt[ERR_USAGE]));

    exit(status);
}


/*
*  FUNCTION: error
*
*  DESCRIPTION:
*  Generic error routine.  This function takes a variable number of arguments
*  and passes them on to vprintf with the error message text as a format.
*
*  Errors from this routine are fatal.
*/
void error(int err, ...)
{
    extern int yylineno, yycharno;
    extern char *yyfilenm;
    va_list ap;
    va_start(ap, err);
    
    if (catd==NULL)
	catd = catopen("localedef.cat", NL_CAT_LOCALE);

    fprintf(stderr, 
	    catgets(catd, LOCALEDEF, ERR_ERROR, err_fmt[ERR_ERROR]),
	    yyfilenm, yylineno, yycharno);

    vfprintf(stderr, catgets(catd, LOCALEDEF, err, err_fmt[err]), ap);

    if (err == ERR_NOSUPPORT)
         exit(3);		/* locale generation not supported */
    
     if ((err == ERR_INTERNAL) || (err == ERR_NO_SYMBOLIC_NAME))
         exit(2);		/* Locale might be too large, etc... */

    exit(4);
}


/*
*  FUNCTION: diag_error
*
*  DESCRIPTION:
*  Generic error routine.  This function takes a variable number of arguments
*  and passes them on to vprintf with the error message text as a format.
*
*  Errors from this routine are considered non-fatal, and if the -c flag
*  is set will still allow generation of a locale.
*/
void diag_error(int err, ...)
{
    extern int yylineno, yycharno;
    extern char *yyfilenm;
    va_list ap;
    va_start(ap, err);
    
    err_flag++;
    
    if (catd==NULL)
	catd = catopen("localedef.cat", NL_CAT_LOCALE);

    fprintf(stderr, 
	    catgets(catd, LOCALEDEF, ERR_WARNING, err_fmt[ERR_WARNING]),
	    yyfilenm, yylineno, yycharno);

    vfprintf(stderr, catgets(catd, LOCALEDEF, err, err_fmt[err]), ap);
}


/*
*  FUNCTION: yyerror
*
*  DESCRIPTION:
*  Replacement for the yyerror() routine.  This is called by the yacc 
*  generated parser when a syntax error is encountered.
*
*  Syntax errors are considered fatal.
*/
void yyerror(char *s)
{
    extern int yylineno, yycharno;
    extern char *yyfilenm;

    if (catd==NULL)
	catd = catopen("localedef.cat", NL_CAT_LOCALE);

    fprintf(stderr,
	    catgets(catd, LOCALEDEF, ERR_SYNTAX, err_fmt[ERR_SYNTAX]),
	    yyfilenm, yylineno, yycharno);

    exit(4);
}


/*
*  FUNCTION: safe_malloc
*
*  DESCRIPTION:
*  Backend for the MALLOC macro which verifies that memory is available.
*
*  Out-of-memory results in a fatal error.
*/
void * safe_malloc(unsigned int bytes, char *file, int lineno)
{
  void * p;

  p = calloc(bytes, 1);
  if (p == NULL)
    error(ERR_MEM_ALLOC_FAIL, file, lineno);

  return p;
}



