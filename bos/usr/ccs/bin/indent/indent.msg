$ @(#)01        1.8  src/bos/usr/ccs/bin/indent/indent.msg, cmdprog, bos411, 9428A410j 3/24/92 10:37:11
$
$ COMPONENT_NAME: (CMDPROG) Programming Utilities
$
$ FUNCTIONS: 
$
$ ORIGINS: 27
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1991
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ messages for indent
$

$len
$set MS_INDENT

$quote "
$ The message numbers have been assigned a range of 225-274
$ Do not translate indent:, %s, %c, %d. 

BADPARAM        "indent: 1286-225 %s is not a recognized flag.\n"
NEEDPARAM       "indent: 1286-226 The %s flag requires a parameter.\n"
PARSE_ERR       "indent: 1286-227 Internal error in set_option with a p_type of %d.\n"
$ Do not translate set_option, p_type 
CANTOPEN        "indent: 1286-228 Cannot open %s.\n"
CANTOWRITE      "indent: 1286-229 The input and output file names cannot be the same.\n"
CANTCREATE      "indent: 1286-230 Cannot create %s.\n"
BADPARAM2       "indent: 1286-231 %s is not a recognized flag.\n"
USAGE           "Usage: indent InputFile [OutputFile] [Flags]\n"
BUFFERFULL      "1286-232 Internal buffer overflow.\n\
\tMove the large comment from the right."
BROKENLINE      "The line is split"
EARLYEOF        "1286-233 Did not expect end of file."
EXTRAPAREN      "1286-234 There is an extra %c."
UNBALPAREN      "1286-235 There is a missing ( or )."
TOOMANYIFDEF    "Use no more than 8 nested #if statements."
$ Do not translate #if 
$ Nested is a programming technical concept. 
ORPHANELIF      "Use a #elif only within a #if/#endif block."
$ Do not translate #elif, #if/#endif 
ORPHANELSE      "Use a #else only within a #if/#endif block."
$ Do not translate #else, #if/#endif 
ORPHANENDIF     "There is a #endif without a matching #if."
$ Do not translate #endif, #if 
BADSYNTAX       "Syntactically inconsistent #ifdef alternatives."
$ Do not translate. 
CANTCREATEBAK   "indent: 1286-236 Cannot create the backup file %s\n"
NOWRITEBAK      "indent: 1286-237 Cannot write to the backup file %s\n"
NOREADINPUT     "indent: 1286-238 Cannot read the input file %s\n"
CANTREOPEN      "indent: 1286-239 Cannot reopen the backup file\n"
$ To reopen means to open again. 
WARNING         "indent: Warning on line %d: "
ERROR           "indent: Error on line %d:   "
NOENDLITERAL    "indent: 1286-240 The literal on line %d does not end.\n"
$ Literal is a programming technical term. 
NOENDLIT        "indent: 1286-241 A literal does not end.\n"
$ Literal is a programming technical term. 
SOLO_ELSE       "1286-242 There is an else without a matching if."
$ Do not translate else, if. These are C Language keywords. 
NESTINGERROR    "1286-243 There is a statement nesting error."
$ Nesting is a programming technical concept. 
UNKNOWNCODE     "1286-244 Code is not valid C Language syntax."
NOENDCOMMENT    "indent: 1286-245 Reached end of file before end of comment.\n"
INTERROR        "indent: 1286-246 Specify an integer with the %s flag.\n"
NUMERROR        "indent: 1286-247 Specify a number with the %s flag.\n"
OUTLINE         "There were %1$d output lines and %2$d comments.\n"
COMPERCODE      "(Lines with comments) / (Lines with code): %6.3f\n"
$ Do not translate %6.3f
FLIPOPER        "Meaning may have been altered -- operators were flipped\n"
$ Internationalization.
BADCHAR         "Invalid character.\n"
OUTOFMEMORY     "indent: Out of memory.\n"
