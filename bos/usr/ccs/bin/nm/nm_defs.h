/*
 * @(#)60       1.11.1.9  src/bos/usr/ccs/bin/nm/nm_defs.h, cmdaout, bos41B, 9504A 1/17/95 11:04:39
 */
/*
 * COMPONENT_NAME: CMDAOUT (nm command)
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27, 9, 3
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*******************************************************************************
	nm_defs.h
	Constants used by nm
*******************************************************************************/

/*
*	Format String for all headers
*/
#define FMTS_D_0	"Name               Type    Value     Size\n\n"

/*
*	Size Format Strings for decimal
*/
#define FMTS_D_SDB_0	"  %6hu"
#define FMTS_D_SDB_1	"  %6hu"

/*
*	Size Format Strings for hexadecimal
*/
#define FMTS_X_SDB_0	"  0x%.4hx"
#define	FMTS_X_SDB_1	"    %4hx"

/*
*	Size Format Strings for octal
*/
#define FMTS_O_SDB_0	"  0%.6ho"
#define FMTS_O_SDB_1	"   %6ho"


/*
*	Constants
*/
#define	MAXLEN		512
#define	TYPELEN		64
#define	TMPLEN		10

/*
*	Format array indices
*/
#define noPsize		0
#define Psize		1

/*
*	Symbol types
*/
#define	VOID_TYPE	"void"
#define	ARG_TYPE	"arg"
#define CHAR_TYPE	"char"
#define	SHORT_TYPE	"short"
#define INT_TYPE	"int"
#define LONG_TYPE	"long"
#define	FLOAT_TYPE	"float"
#define DOUBLE_TYPE	"double"
#define STRUCT_TYPE	"struct"
#define UNION_TYPE	"union"
#define	ENUM_TYPE	"enum"
#define	ENMEM_TYPE	"enmem"
#define	UCHAR_TYPE	"Uchar"
#define	USHORT_TYPE	"Ushort"
#define UINT_TYPE	"Uint"
#define ULONG_TYPE	"Ulong"


/*
*	Message Strings
*/
#define USAGE "Usage: nm [-ACfhprTv] [-B|-P] [-e|-g|-u] \
[-d|-o|-x|{-t [d|x|o]}] [--] File ...\n"
#define	FILE_NAME "Filename"
#define	NO_NAME_STR	"**_no_name_**"
#define	BAD_NAME_STR "**_incorrect_symbol_name_**"
#define	NO_SCN_STR "**_no_scn_**"
#define	UNDEF_SYM_FIL "\n\nUndefined symbols from %s:\n\n"
#define	UNDEF_SYM_MEM "\n\nUndefined symbols from %s[%.14s]:\n"
#define	DEF_SYM_FIL "\n\nSymbols from %s:\n\n"
#define	DEF_SYM_MEM "\n\nSymbols from %s[%.14s]:\n\n"
#define	OPEN_ERR "0654-200 Cannot open the specified file.\n"
#define	REOPEN_ERR "0654-201 Cannot open the specified file.\n"
#define	NOT_OBJECT_ERR "0654-202 Error while reading object file header.\n"
#define	NOT_XCOFF_ERR "0654-203 Specify an XCOFF object module.\n"
#define	NO_SYMBOLS "0654-204 There are no symbols in the file.\n"
#define	TEMP_OPEN_ERR "0654-205 Cannot open the temporary file required with \
the -vn flags.\n"
#define	BAD_SYMTAB_ERR "0654-206 Cannot process the symbol table.\n"
#define	SORT_ABEND_ERR "0654-207 The sort process was stopped prematurely.\n"
#define	UNKNOWN_OPT_ERR "0654-208 nm: %c is not a recognized flag.\n"
#define	NO_MEM "0654-209 There is not enough memory available now.\n"
#define NM_ERR_MSG	"nm: %s: %s"
#define NM_AR_ERR_MSG	"nm: %s[%s]: %s"
#define	QUESTION_MARKS	"??????"
#define UNDEF_SCN	"undef"
#define	ABS_SCN		"abs"
#define	DEBUG_SCN	"debug"
#define DOT_TEXT	".text"
#define DOT_DATA	".data"
#define DOT_BSS		".bss"

/*
*	Text formatting string
*/
#define	TAGNDX_FMT	"-%.8xs"
