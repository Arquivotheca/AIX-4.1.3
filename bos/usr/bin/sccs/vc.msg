$ @(#)09        1.5  src/bos/usr/bin/sccs/vc.msg, cmdsccs, bos411, 9428A410j 4/25/91 19:26:47
$
$ COMPONENT_NAME: CMDSCCS       source code control system
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1991
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "         define quote character.

$ Do not translate vc.
$ Do not translate %s, %d, %1$s, etc...
$ Do not translate the parenthesis (vc#) at the end of each message
$len 
$set 13        Define initial set#

UNKCMD               "vc: 1255-156 The command specified on line %d is not valid. (vc1)\n"
NVRUSED              "vc: 1255-157 %s was not used. (vc2)\n"
NVRDCLD              "vc: 1255-158 %s was not declared. (vc3)\n"
NOEND                "\n\
vc: 1255-159 There is an if statement with no matching end statement. (vc4)\n"
DCLDTWC              "vc: 1255-160 %1$s is declared twice on line %2$d. (vc5)\n"
$ %1$s is a keyword. 
$ 2$d is a line number. 
OUTOFSPCVC6          "\n\
vc: 1255-161 Cannot specify more than 40 keywords on the command line. (vc6)\n"
UNMTCHD              "\n\
vc: 1255-162 There is a %1$c on line %2$d that is not matched. (vc7)\n"
$ %1$c is a control character. 
$ %2$d is a line number. 
NMTOOLNG             "vc: 1255-163 A keyword name on line %d exceeds 9 characters. (vc8)\n"
INVKEYWRD            "vc: 1255-164 A keyword name on line %d is not valid. (vc9)\n"
ENDWOIF              "vc: 1255-165 There is an end statement without a matching if statement\n\
\ton line %d. (vc10)\n"
PARENERR             "vc: 1255-166 There is a parenthesis error on line %d. (vc11)\n"
INVOP                "vc: 1255-167 An operator on line %d is not valid. (vc12)\n"
MESSAGE              "Message(%1$d): %2$s\n"
$ %1$d is the line number. 
$ %2$s is the message. 
NONNUMB              "vc: 1255-169 There is a value on line %d that is not numeric.(vc14)\n"
ERRSTMT              "The :err statement is on line %d. (vc15)\n"
$ Do not translate :err
$ This message appears below message RPLCERR. 
NOSPC                "vc: 1255-171 Line %d requires more than 32 replacements. (vc16)\n"
SYNTAX17             "vc: 1255-172 The control statement syntax on line %d is not correct. (vc17)\n"
SYNTAX18             "vc: 1255-173 The if statement syntax on line %d is not correct. (vc18)\n"
SYNTAX19             "vc: 1255-174 The syntax on line %d is not correct. (vc19)\n"
$ This message is a catch-all message. 
NVRASSGND            "vc: 1255-175 %s was not assigned a value. (vc20)\n"
$ %s is a keyword name. 
RPLCERR              "ERROR: %s\n"
$ %s is the error message. 
