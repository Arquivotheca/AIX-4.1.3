$ @(#)87        1.5.1.2  src/bos/usr/ccs/bin/lint/pass2/lint.msg, cmdprog, bos411, 9428A410j 3/28/94 14:02:10
$
$ COMPONENT_NAME: (CMDPROG) Programming Utilities
$
$ FUNCTIONS: 
$
$ ORIGINS: 27
$
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "
$len 
$set MS_LINT        Define initial set#

$  Message numbers have been assigned a range of 300-399.
$  Do not translate lint:, %s, %d, %1$s, %1$d, etc ...

1                    "lint: 1286-300 The %s flag option is missing.\n"
2                    "lint: 1286-301 %s flag is not recognized. It is ignored.\n"
3                    "lint: 1286-302 File %s must have a .c or .ln extension. It is ignored.\n"
$  Do not translate .c, .ln

M_MSG_258            "lint: 1286-303 There is not enough memory available now."
M_MSG_259            "1286-304 The internal hash table overflowed."
M_MSG_260            "1286-305 File %s data error"
M_MSG_261            "1286-306 Action for symbol in file %s is not known"
$  Action is an operation that the lint program is trying to perform on symbol
$  within the file.
M_MSG_262            "1286-307 Record directive in file %s is not known"
M_MSG_263            "\"%s\", line %d: warning: "
M_MSG_264            "\"%s\", line %d (\"%s\"): warning: "
M_MSG_265            "1286-308 End of file reached before expected in %s."
M_MSG_266            "1286-309 Cannot open file %s\n"
M_MSG_304            "function %s multiply defined"
$  Message 304-319 are written as standard output. They are not error messages.
$  Talk with your technical representatives before you translate these. Some
$  customers may have programs that depend on these messages comming out exactly
$  as they are.

M_MSG_305            "symbol %s multiply defined"
M_MSG_306            "symbol %s type inconsistent"
$  Type is a C-Language term which defines classes of objects.
M_MSG_307            "struct/union/enum %s inconsistently redefined"
$  Do not translate struct, union, enum
M_MSG_308            "function %s type inconsistent"
$  Type is a C-Language term which defines classes of objects.
M_MSG_309            "function %s argument type inconsistent"
$  Argument = parameter.
$  Type is a C-Language term which defines classes of objects.
M_MSG_310            "function argument %s count mismatch"
$  The definition of the function has a different number of parameters
$  than the call of the function.
M_MSG_311            "function %s used but not defined"
M_MSG_312            "symbol %s used but not defined"
M_MSG_313            "function %s defined but never used"
M_MSG_314            "symbol %s defined but never used"
M_MSG_315            "function %s declared but never used or defined"
M_MSG_316            "symbol %s declared but never used or defined"
M_MSG_317            "function %s return value used, but none returned"
$  The function call is requesting a return value, but the function itself does
$  not return a value.
M_MSG_318            "function %s return value is always ignored"
M_MSG_319            "function %s return value is sometimes ignored"
M_MSG_320            "class %s inconsistently redefined"
$  Do not translate class
104                    "lint: 1286-330 -C specified, but no C++ lint libraries present\n"
$  Do not translate lint:, -C, C++
105                    "lint: 1286-331 .C files specified, but xlC C++ compiler not present\n"
$  Do not translate lint:, -C, C++, xlC
106                    "lint: 1286-332 File %s must have a .c, .C or .ln extension. It is ignored.\n"
$  Do not translate .c, .C, .ln
107                  "lint: 1286-333 The %s option is not recognized.\n"
108                  "lint: 1286-334 There are no files to process.\n"
109                  "lint: 1286-335 Cannot open file %s.\n"
110                  "lint: 1286-336 Incorrect usage of option: %s.\n"
