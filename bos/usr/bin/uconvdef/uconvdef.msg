$ @(#)06  1.3  src/bos/usr/bin/uconvdef/uconvdef.msg, cmdiconv, bos411, 9428A410j  1/10/94  03:47:11
$
$   COMPONENT_NAME:	CMDICONV
$
$   FUNCTIONS:		uconvdef message catalogue
$
$   ORIGINS: 27
$
$   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$   combined with the aggregated modules for this product)
$                    SOURCE MATERIALS
$
$   (C) COPYRIGHT International Business Machines Corp. 1993
$   All Rights Reserved
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote	"
$set	UCONVDEF

M_USAGE			"Usage: uconvdef [-f source_file] [-v] uconv_table\n"
M_START_PROCESSING	"Start processing the source file '%s'.\n"
M_COMPLETED		"%s created.\n"
M_UNCOMPLETED		"Output file '%s' was not created.\n"
M_CANNOT_ALLOCATE	"uconvdef: 0791-201 Not enough space for allocation.\n"
M_CANNOT_OPEN_INPUT	"uconvdef: 0791-202 Cannot open input file '%s': %s.\n"
M_CANNOT_OPEN_TEMP	"uconvdef: 0791-203 Cannot open temporary file: %s.\n"
M_CANNOT_CLOSE_INPUT	"uconvdef: 0791-204 Cannot close input file '%s': %s.\n"
M_CANNOT_CLOSE_TEMP	"uconvdef: 0791-205 Cannot close temporary file: %s.\n"
M_ERROR_RENAME		"uconvdef: 0791-206 Cannot rename temporary file to output file '%s': %s.\n"
M_ERROR_READ		"uconvdef: 0791-207 Error while reading input file '%s': %s.\n"
M_ERROR_WRITE		"uconvdef: 0791-208 Error while writing temporary file: %s.\n"
M_INVAL_MB_CUR		"uconvdef: 0791-209 \'MB_CUR_MIN\' is greater than \'MB_CUR_MAX\' default.\n"
M_INVAL_SPACE		"uconvdef: 0791-210 Line %4.4d: space at the line top is not allowed.\n"
M_INVAL_FORMAT		"uconvdef: 0791-211 Line %4.4d: illegal line format.\n"
M_INVAL_TOKEN		"uconvdef: 0791-212 Line %4.4d: illegal token.\n"
M_INVAL_TOKEN_VALUE	"uconvdef: 0791-213 Line %4.4d: illegal value for token '%s'.\n"
M_UNDEF_TOKEN_VALUE	"uconvdef: 0791-214 Line %4.4d: token '%s' must have value.\n"
M_EXTRA_CS_NAME		"uconvdef: 0791-215 Line %4.4d: code set name can be defined only once.\n"
M_EXTRA_UCONV_CLASS	"uconvdef: 0791-216 Line %4.4d: \'UCONV_CLASS\' can be defined only once.\n"
M_EXTRA_CHARMAP		"uconvdef: 0791-217 Line %4.4d: CHARMAP section is already started.\n"
M_EXTRA_CHARS		"uconvdef: 0791-218 Line %4.4d: extra characters.\n"
M_CONFLICT_UCONV_CLASS	"uconvdef: 0791-219 Line %4.4d: conflict with \'UCONV_CLASS\'.\n"
M_CONFLICT_MB_CUR	"uconvdef: 0791-220 Line %4.4d: conflict with \'MB_CUR_MIN\' or \'MB_CUR_MAX\'.\n"
M_CONFLICT_LEN		"uconvdef: 0791-221 Line %4.4d: conflicting code length.\n"
M_OVER_BYTE_RANGE	"uconvdef: 0791-222 Line %4.4d: range exceeds limits of one byte.\n"
M_TOO_MANY_PLANES	"uconvdef: 0791-223 Line %4.4d: Too many planes.\n"
M_ESCAPE_COMMENT	"uconvdef: 0791-224 Line %4.4d: escape and comment characters must be distinct.\n"
M_MISSING_CS_NAME	"uconvdef: 0791-225 No code set name is defined.\n"
M_MISSING_UCONV_CLASS	"uconvdef: 0791-226 No \'UCONV_CLASS\' is defined.\n"
M_MISSING_CHARMAP	"uconvdef: 0791-227 CHARMAP section must start with \'CHARMAP\'.\n"
M_MISSING_CHARMAP_END	"uconvdef: 0791-228 CHARMAP section has no \'END CHARMAP\'.\n"
