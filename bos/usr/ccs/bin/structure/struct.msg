$ @(#)96	1.3  src/bos/usr/ccs/bin/structure/struct.msg, cmdprog, bos411, 9428A410j 6/15/90 22:56:33
$
$ COMPONENT_NAME: (CMDPROG) Programming Utilities
$
$ FUNCTIONS: 
$
$ ORIGINS: 27
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ messages for struct
$

$len
$set MS_STRUCT

$quote "
$ Message numbers have been assigned a range of 400-499 
$ Do not translate struct:, RATFOR, %s, %d, %1$s, %1$d, etc. 
$ Beautifying is a program that struct runs that adds indentation on output 
$ lines. 

ALLOCSPC        "struct: 1286-400 There is not enough memory available now.\n"
TOTSPAC         "total space alloc'ed = %ld\n"
$ Do not translate.  This is a debug message only.
MOREBYTE        "%d more bytes requested\n"
$ Do not translate.  This is a debug message only.
COLOUTSPC       "1286-401 There is not enough memory available now."
SPCEQ           "space = %ld\n"
$ Do not translate.  This is a debug message only.
ROUTBEG         "Errors for the routine beginning on line %d:\n"
$ routine is a programming term referring to a unit inside the program.
ERRORS          "struct: %1$s %2$s %3$s\n"
$ Do not translate. 
STRUCTERR       "struct: "
$ Do not translate. 
MULTIN          "struct: 1286-402 There are multiple input files. Using the first file %s \n"
INBEAUTIFY      " On input %2$s, on output line %$1d.\n"
$ Leave a space before the message  
ONIN            " On input %2$c, on output line %1$d.\n"
$ Leave a space before the message  
MALLOCSPC       "1286-403 There is not enough memory available now."
ERRINBEAUT      "\n\
struct: 1286-404 error in beautifying, output line %1$d: %2$s %3$s %4$s \n"
NESTDEEP        "\n\
\tThe nesting is too deep, stack overflow."
STACKEXHAUST    "\n\
\tThe stack is empty."
INPBACKUP       "1286-405 RATFOR beautifying; input backed up too far during lex:\n"
$ Do not translate RATFOR, lex: 
WRONGCHAR       "1286-406 Internal error while RATFOR beautifying; wrong character passed:\n"
$ Do not translate RATFOR 
XXBUFTOOSML     "1286-407 RATFOR beautifying; buffer too small for token beginning:\n"
$ Do not translate RATFOR 
NOINFILE        "1286-408 There is no input file."
CANTOPENINFILE  "struct: 1286-409 Cannot open input file %s\n"
INVSW           "struct: 1286-410 %s is not a recognized flag.\n"
PRGRAPH         "prgraph():\n"
$ Do not translate.  This is a debug message only.
STGTOOLNG       "struct: 1286-411 Internal error on string copy of %s\n"
LABWOCOD        "1286-412 Label without code is ignored."
LINESHORT       "1286-413 A line is shorter than five characters."
LINE2LNG        "1286-414 A line is longer than 121 characters."
INDSKSEEK       "1286-415 The fseek system call failed."
$  Do not translate fseek. 
DECRAT          "1286-416 Internal buffer overflow."
INSYNT		"struct: 1286-417 The label beginning %2$ld on line %1$d is too long.\n\
%3$s\n"
TREATINGLINE    "Treating line as straight line code.\n"
SYMBOLNOTOCC    "struct: 1286-418 The symbol %1$c should not occur in position %2$d of: \n\
%3$s\n"
PARSING         "parsing\n"
$ Do not translate.  This is a debug message only.
PARSINGLINE     "parsing line %d\n"
$ Do not translate.  This is a debug message only.
PARSINVST       "struct: 1286-419 The parser state is not valid at line %d.\n"
STRGTCODE       "Treating line as straight line code.\n"
BTWNLINES       "struct: 1286-420 Syntax error between lines %1$d and %2$d.\n"
SYMBOLNOTMATCH  "symbol '%c' does not match entries for state %d\n"
$ Do not translate. This is a debug message.  This is a debug message only.
LABUSED         "1286-421 The label is used but does not appear as a statement label:"
$ This message will be followed by the label.
HASHOVERFLW     "1286-422 The hash table overflowed. Cannot have more than 347 labels."
REDEFLAB        "struct: 1286-423 Cannot redefine the value of label %1$ld\n\
\tbetween lines %2$d and %3$d\n"
$ This message is always preceded by the preceding message, CINSYSN
LINEBUFCT       "line %1$d; bufcount: %2$d\n"
$ Do not translate.  This is a debug message only.
ILLEGDORG       "struct: 1286-424 The range of do statement between lines %1$d and %2$d is not valid.\n"
$ Do not translate do. 
DOLOOPDEEP      "1286-425 Cannot nest more than 20 do loops."
$ Do not translate do. 
LINECOL         "line %d:  "
$ Do not translate.  This is a debug message only.
NODEUNREACH     "node %d unreachable\n"
$ Do not translate.  This is a debug message only.
UNREACH         "1286-426 Statements that cannot be reached are ignored."
BEIGNORED       " "
GETHEAD         "	gethead:\n"
$    Do not translate.  This is a debug message only.
GETINARC        "	getinarc:\n"
$     Do not translate.  This is a debug message only.
GETDOM          "	getdom:\n"
$     Do not translate.  This is a debug message only.
GETTREE         "	gettree:\n"
$     Do not translate.  This is a debug message only.
GTRCH           "	getreach:\n"
$     Do not translate.  This is a debug message only.
GTFLW           "	getflow:\n"
$     Do not translate.  This is a debug message only.
GTTHN           "	getthen:\n"
$     Do not translate.  This is a debug message only.
GTLP            "	getloop:\n"
$     Do not translate.  This is a debug message only.
GTBRNCH         "	getbranch:\n"
$     Do not translate.  This is a debug message only.
NDBRACE         "ndbrace:\n"
$     Do not translate.  This is a debug message only.
OUTRAT          "outrat:\n"
$     Do not translate.  This is a debug message only.
BUILD           "build:\n"
$     Do not translate.  This is a debug message only.
STRUCTURE       "structure:\n"
$     Do not translate.  This is a debug message only.
OUTPUT          "output:\n"
$     Do not translate.  This is a debug message only.
STRG2LONG       "struct: 1286-427 Internal error copying string %1$s at address %2$d.\n"
TOKENASSERT     "1286-428 Internal error in subroutine accum."
$ Do not translate accum. 
BUFINDASSERT    "1286-429 Internal error in subroutine accum."
$ Do not translate accum. 
GOTO            "1286-430 The goto label is not valid."
$ Do not translate goto. 
