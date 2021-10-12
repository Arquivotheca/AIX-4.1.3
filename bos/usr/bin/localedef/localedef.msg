$ @(#)11	1.7.2.9  src/bos/usr/bin/localedef/localedef.msg, cmdnls, bos411, 9428A410j 5/13/94 15:32:36
$
$ COMPONENT_NAME: (CMDNLS)
$
$ ORIGINS: 27, 85
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$ OSF/1 1.2
$
$quote "
$set LOCALEDEF

ERR_WRONG_SYM_TYPE   "The symbol '%s' is not the correct type.\n"

ERR_OPEN_READ        "Could not open '%s' for read.\n"

ERR_INTERNAL         "Internal error. [file %s - line %d].\n"

ERR_N_SARGS          "Syntax Error: expected %d arguments and received\
 %d arguments.\n"

ERR_ILL_RANGE_SPEC   "Illegal limit in range specification.\n"

ERR_MEM_ALLOC_FAIL   "Memory allocation failure: [line %d -- module %s].\n"

ERR_WRT_PERM         "Could not open temporary file '%s' for write.\n"

ERR_CHAR_TOO_LONG    "The '%s' character is longer than <mb_cur_max>.\n"

ERR_SYM_UNDEF        "The '%s' character is undefined.\n"

ERR_INVAL_COLL_RANGE "The start of the range must be numerically less\
 than the\nend of the range.\n"

ERR_INVALID_SYM_RNG  "The symbol range containing %s and %s is\
 incorrectly formatted.\n"

ERR_BAD_STR_FMT      "Illegal character reference or escape sequence\
 in '%s'.\n"

ERR_PC_COLLISION     "You have specified different names for the\
 same character '%d'.\n"

ERR_DUP_CHR_SYMBOL   "The character symbol '%s', has already been\
 specified.\n" 

ERR_NO_UNDEFINED     "There are characters in the codeset which are\
 unspecified in the collation order.\n"
 
ERR_ILL_DEC_CONST    "Illegal decimal constant '%s'.\n"

ERR_ILL_OCT_CONST    "Illegal octal constant '%s'.\n"

ERR_ILL_HEX_CONST    "Illegal hexadecimal constant '%s'.\n"

ERR_MISSING_QUOTE    "Missing closing quote in string '%s'.\n"

ERR_ILL_CHAR         "Illegal character, '%c', in input file.  Character\
 will be ignored.\n"

ERR_ESC_CHAR_MISSING "Character for escape_char statement missing.\
 Statement ignored.\n"

ERR_COM_CHAR_MISSING "Character for comment_char statement missing.\
 Statement ignored.\n"

ERR_CHAR_NOT_PCS     "'%c' is not a POSIX Portable Character. Statement\
 ignored.\n"

ERR_ILL_CHAR_SYM     "The character symbol '%s' is missing the closing '>'.\
 The '>' will be added.\n"

ERR_UNKNOWN_KWD      "Unrecognized keyword, '%s', statement ignored.\n"

ERR_UNDEF_RANGE_SYM  "The character symbol '%s' is undefined in the range\
 '%s...%s'.\n The symbol will be ignored.\n"

ERR_UNSUP_ENC        "The encoding specified for the '%s' character is\
 unsupported.\n"

ERR_DUP_COLL_SPEC    "The character, '%s', has already been assigned a\
 weight.\n Specification ignored.\n"

ERR_DUP_COLL_RNG_SPEC "The character %s in range %s...%s already has a\
 collation weight. Range ignored.\n"

ERR_TOUPPER_NOT_OPT  "No toupper section defined for this locale\
 sourcefile.\n"

ERR_CODESET_DEP	     "The use of the \"...\" keyword assumes that the\
 codeset is contiguous\n between the two range endpoints specified.\n"

ERR_ILL_COLL_SUBS    "The collation substitution,  %s%s, contains a\
 symbol which is not\n a character.\n",

ERR_USAGE            "Usage: localedef [-c] [-f charmap] [-i locsrc]\
\\\n\t [-L ld opts] [-m methfile] locname\n"

ERR_ERROR            "localedef [ERROR]: FILE: %s, LINE: %d, CHAR: %d\n"

ERR_WARNING          "localedef [WARNING]: FILE: %s, LINE: %d, CHAR: %d\n"

ERR_SYNTAX           "localedef [ERROR]: FILE: %s, LINE: %d, CHAR: %d\n\
Syntax Error.\n"

ERR_TOO_MANY_ORDERS  "Specific collation weight assignment is not valid\
 when no sort\n keywords have been specified.\n"

ERR_BAD_CHDR         "The compile was unsuccessful.  Required header files\n\
sys/localedef.h and sys/lc_core.h may be the wrong\n\
version or may be missing.\n"

ERR_INV_MB_CUR_MIN   "The <mb_cur_min> keyword must be defined as 1, you have\
 defined it \nas %d.\n"

ERR_INV_CODE_SET_NAME "The <code_set_name> must contain only characters from\
 the portable \ncharacter set, %s is not valid.\n"

ERR_FORWARD_BACKWARD "The collation directives forward and backward are\
 mutually \nexclusive.\n"
$ translator: do not translate "forward" or "backward".  They are keywords.

ERR_TOO_MANY_ARGS "Received too many arguments, only expected %d.\n"

ERR_DUP_CATEGORY "The %s category has already been defined.\n"

ERR_EMPTY_CAT "The %s category is empty.\n"

ERR_UNDEF_CAT "Unrecognized category %s is not processed by localedef.\n"

ERR_USER_DEF "The POSIX defined categories must appear before any unrecognized\
 categories.\n"

ERR_DIGIT_FC_BAD "The file code for the digit %s is not one greater than\
 the \nfile code for %s.\n"

$ ie, we want:  (value of first code) + 1 = (value of second code)

ERR_DIGIT_PC_BAD "The process code for the digit %s is not one greater than\
 the \nprocess code for %s.\n"

$ ie, we want:  (value of first code) + 1 = (value of second code)

ERR_DUP_COLL_SYM "The symbol %s has already been defined. Ignoring definition\
 as a \ncollating-symbol.\n"

ERR_INVALID_UPPER "Locale does not conform to POSIX specifications for the\
 LC_CTYPE upper \nkeyword.\n"

ERR_INVALID_LOWER "Locale does not conform to POSIX specifications for the\
 LC_CTYPE lower \nkeyword.\n"

ERR_INVALID_ALPHA "Locale does not conform to POSIX specifications for the\
 LC_CTYPE alpha \nkeyword.\n"

ERR_INVALID_SPACE "Locale does not conform to POSIX specifications for the\
 LC_CTYPE space \nkeyword.\n"

ERR_INVALID_CNTRL "Locale does not conform to POSIX specifications for the\
 LC_CTYPE cntrl \nkeyword.\n"

ERR_INVALID_PUNCT "Locale does not conform to POSIX specifications for the\
 LC_CTYPE punct \nkeyword.\n"

ERR_INVALID_GRAPH "Locale does not conform to POSIX specifications for the\
 LC_CTYPE graph \nkeyword.\n"

ERR_INVALID_PRINT "Locale does not conform to POSIX specifications for the\
 LC_CTYPE print \nkeyword.\n"

ERR_MIN_UPPER "Locale does not specify the minimum required for the LC_CTYPE\
 upper keyword.\n"

ERR_MIN_LOWER "Locale does not specify the minimum required for the LC_CTYPE\
 lower keyword.\n"

ERR_MIN_SPACE "Locale does not specify the minimum required for the LC_CTYPE\
 space keyword.\n"

ERR_INV_DIGIT "Locale does not specify only '0', '1', '2', '3', '4', '5',\
 '6', '7', \n'8', and '9' for LC_CTYPE digit keyword.\n"

$ translators, do not translate "digit".

ERR_INV_XDIGIT "Either locale does not specify '0', '1', '2', '3', '4', '5',\
 '6', '7', \n'8', '9', 'a' through 'f', and 'A' through 'F' for LC_CTYPE\
 xdigit keyword.\n or locale does not specify a sufficient number of\
 consecutive symbols.\n"

$ translators, do not translate "xdigit".

ERR_COLL_WEIGHTS "The number of operands to LC_COLLATE order exceeds\
 COLL_WEIGHTS_MAX.\n"

ERR_METHODS "The methods for __mbtopc, __mbstopcs, __pctomb, __pcstombs, \
mbtowc, \nmbstowcs, wctomb, wcstombs, mblen, wcwidth, and wcswidth must be \
specified \nin the <methodfile>.\n"

ERR_PRIVATE_TABLE "Locale can not mix private method table methods and global\
 method \n table methods.\n"

ERR_LOAD_FAIL "Temporary method table failed to load.\n"

ERR_NOSHELL "Unable to exec /usr/bin/sh to process intermediate files.\n"
$ translator:  do not translate /usr/bin/sh

ERR_NOPROG "Unable to run program %s.\n"

ERR_NOSUPPORT "Locale generation is not supported on this system.\n"

ERR_BAD_SYMBOL_STRING "Missing end of symbol in string '%s'.\n"

ERR_NO_SYMBOLIC_DIGIT "The symbolic name '%s' is not defined in the character\
map.\n  This character set may not be POSIX compliant.\n"

CANT_LOAD_LOCALE	"Unable to load locale '%s' for copy directive.\n"
$ translator: do not translate copy

NAME_TOO_LONG		"Locale name longer than PATH_MAX (%d).\n"
$ translator:  do not translate PATH_MAX

CHARCLASS_NOT_DECLARED	"\"%s\" not declared in a charclass statement.\n"

ERR_BAD_WEIGHT	"Collating symbols such as %s can not have explicit weights.\n\
Specification ignored.\n"

ERR_BAD_ELLIPSIS1	"Ellipsis on the right hand side may only be used\n\
with ellipsis or UNDEFINED symbols on the left hand side.\n"
$ translator:  do not translate UNDEFINED

ERR_BAD_ELLIPSIS2	"Ellipsis may not be used as one of the characters in\n\
a one-to-many mapping.\n"

ERR_BAD_COLL_SYM	"Collating symbols may not be used as one of the\n\
characters in a one-to-many mapping.\n"

STACK_OVERFLOW		"Stack overflow error.\n"

ERR_NO_SYMBOLIC_NAME	"Required symbolic name %s not defined in character \
map file.\n"

SYMBOL_NAME_TOO_LONG	"The symbol %s is too long.\n\
It will be truncated to %d bytes.\n"

ERR_SKIP_UNDEF_SYM	"The '%s' character is undefined.\n\
This character along with any range statements it may be in\n\
will be ignored.\n"

TOO_MANY_WEIGHTS	"More weights were defined for character %s than\n\
were specified with the order_start keyword.\n"
