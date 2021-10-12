$ @(#)08        1.6  src/bos/usr/ccs/bin/bs/bs.msg, cmdlang, bos411, 9428A410j 3/11/94 10:16:44
$
$ COMPONENT_NAME: (CMDLANG) Language Utilities
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
$set MS_BS
$ Message numbers have been assigned a range of 100-199  
$ Do not translate bs:, %d, %s, %1$d, %1$s, etc ... 
NREADYN              "\n\
Ready\n"
$ bs is ready for input.  
FLTERR               "bs: 1285-100 Floating point exception interrupt"
$ This message is followed by EXECERR  
BADGRAF              "bs: 1285-101 Cannot direct graphic output to device."
$ This message is followed by EXECERR  
PIPECL               "bs: 1285-102 The pipe is closed."
$ pipe is a Unix technical term.  
NOTOPEN              "bs: 1285-103 Cannot open %s\n"
NOCSPAC              "There is no more code space."
INVEXPR              "Expression is not valid."
NOFORK               "The fork system call failed."
ELNOIF               "The else statement requires an if statement."
$ Do not translate else, if.  
FINOIF               "The fi statement requires an if statement."
$ Do not translate fi, if.  
BASERR               "The ibase or obase value must be 8, 10, or 16"
$ Do not translate ibase, obase.  
FORERR               "The for statement is not correct."
$ Do not translate for.  
NNFOR                "The next statement requires a for statement."
$ Do not translate for, next.  
NOFOR                "There is a missing for statement."
$ Do not translate for.  
FUNERR               "The function definition is not valid."
NSTINC               "Nested include statements are not allowed."
$ Do not translate include.  
NOPF                 "Cannot open file."
MISSFN               "Specify a filename."
EXPR                 "Expression is missing or is not valid."
RESNAME              "Reserved name is not allowed in expression."
NAMREQ               "Specify a name."
UBALQ                "There is a missing quote character."
ARGCNT               "The number of parameters passed is not correct."
SUBLN                "Cannot specify subscript on local name."
$ local name is referring to a variable name that is not a table.  
SRCLN                "Source line %d: "
$ Leave a space at the end of this message.  
DUPLBL               "There is a duplicate label."
TOOMLBS              "Specify no more than 100 labels."
TBLOVF               "bs: 1285-104 There is no more symbol table space."
ERRDEL               "The delimiter used is not correct."
TOOBIG               "bs: 1285-105 Table size cannot be greater than 1049."
$ Table is referring to a built-in fuction. Do not translate Table.  
TBLOVFL              "bs: 1285-106 Cannot exceed declared table size."
$ Table is referring to a built-in fuction. Do not translate Table.  
QERR                 "bs: 1285-107 There is no end for an if, for, or fun statement.\n\t"
$ There must be a \t at the end of this message.  
$ Do not translate if, for, fun  
NAMCON               " reserved word used as name."
$ name is variable name.  
$ Leave a space at the beginning of this message.  
SNGLDIM              "bs: 1285-108 A table must be one dimensional."
$ Do not translate table.  
SUBS                 "bs: 1285-109 Subscript found; name is not a table."
$ name is variable name.  
$ Do not translate table.  
SUBRNG               "bs: 1285-110 Subscript is out of range."
RANGE                "bs: 1285-111 Subscript is out of range on list."
$ List is a bs command operation. 
STOV                 "bs: 1285-112 There is a stack overflow."
RECUR                "bs: 1285-113 Function recursion is not allowed."
$ Recursion is programming technical term.  
UNDEF                "Function is not defined "
$ This message is followed by a line number and a function name.  
$ Leave a space at the end of this message.  
RETURNC              "Return:  "
$ Leave spaces at the end of this message.  
TOOMRG               "bs: 1285-114 Too many parameters passed."
RETURND              "bs: 1285-115 There is a return statement ouside of a function."
ALLNAM               "bs: 1285-116 Name is already defined."
NOTTAB               "bs: 1285-117 Parameter cannot be a table."
STATELT              "Statement: %d\n"
$ %d is a line number.  
FUNCTST              "Function stack:\n"
$ stack is a programming technical term.  
$ Following this message will be a list of functions from the stack.  
COMPIL               "bs: 1285-118 Internal compiler error."
EXECERR              "%s Run time error."
$ Do not move the %s.  
SOURCLN              " Line %d"
$ Leave a space at the beginning of this message.  
NOACSF               "bs: 1285-119 Cannot open file."
LSTARG               "bs: 1285-120 Final parameter to open() must be r, w, or a\n\t"
$ There must be a \t at the end of this message.  
$ Do not translate open(), r, w, a  
CONV                 "bs: 1285-121 Conversion is not valid."
STRSPC               "bs: 1285-122 There is not enough memory available now."
ILLGRAF              "bs: 1285-123 The plot() call is not correct."
$ Do not translate plot()  
ARGGRAF              "bs: 1285-124 The number of parameters passed to plot() is not correct.\n\t"
$ Do not translate plot().   
$ There must be a \t at the end of this message.  
NOGRAF               "bs: 1285-125 Specify graphic output."
DSTGRAF              "bs: 1285-126 Specify less than 11 characters for graphic destination.\n\t"
$ There must be a \t at the end of this message.  
NODOT                "bs: 1285-127 plot(6) is not implemented."
$ Do not translate plot(6)  
NOPLOT               "bs: 1285-128 Cannot open graphic output."
S_ERROR              "bs: 1285-129 Regular expression error."
$ Regular expression is a Unix technical term.  
NOLABEL              "bs: 1285-130 The label is not defined."
ARG_ERROR            "bs: 1285-131 The arg() call is not correct."
$ Do not translate arg()  
