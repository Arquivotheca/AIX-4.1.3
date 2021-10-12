$ @(#)43        1.6.1.4  src/bos/usr/ccs/bin/cxref/cxref.msg, cmdprog, bos411, 9428A410j 3/21/94 16:49:50
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

$len
$quote "         define quote character.

$set MS_CXREF        Define initial set#
$ The message numbers have been assigned a range of 150 - 199
$ Do not translate cxref:, %s, %d, %c. 

M_MSG_1        "cxref: 1286-150 Block nesting is not correct."
$ Block is a C language program technical term. 
M_MSG_2        "cxref: 1286-151 There cannot be more than 50 macro files.\n"
$ Do not translate this message, it currently is not used. 
M_MSG_3        "cxref: 1286-152 There cannot be more than 50 preprocessor parameters.\n"
M_MSG_4        "cxref: 1286-153 m4 failed with status %s.\n"
$ Do not translate this message, it currently is not used. 
M_MSG_5        "cxref: 1286-154 Assembler failed with status %d.\n"
$ Do not translate this message, it currently is not used. 
M_MSG_6        "cxref: 1286-155 The command /lib/cpp failed on %s.\n"
$ Do not translate /lib/cpp 
$ %s is the file name. 
M_MSG_7        "cxref: 1286-156 Cannot process %s.\n"
$ %s is the file name. 
M_MSG_8        "cxref"
$ Do not translate. 
M_MSG_9        "cxref: 1286-157 Internal software error.\n"
M_MSG_10       "cxref: 1286-158 Internal software error.\n"
M_MSG_11       "cxref: 1286-159 The sort command failed with status %d.\n"
$ Do not translate sort. 
M_MSG_12       "cxref: 1286-160 Cannot open %s.\n"
M_MSG_13       "Usage: cxref [-cst] [-o File] [-w[Number]]\n\
\t[[-DName[=Definition]] [-UName] [-IDirectory]] ...\n\
\t[-NdNumber] [-NlNumber] [-NnNumber] [-NtNumber] File ...\n"
M_MSG_14       "cxref: 1286-161 %s is not a recognized flag. It is ignored.\n"
M_MSG_15       "cxref: 1286-162 Cannot recover from error in %s."
$ %s is a command name. 
M_MSG_16       "SYMBOL\t\tFILE\t\t\tFUNCTION   LINE\n"
$ These are headings for the C language cross reference listing.  
$ They have technical meaning to the C programmer. It would be  
$ best to leave them as they are. If you do translate, the columns should 
$ be preserved. 
M_MSG_17       "cxref: 1286-163 The command %s failed on %s.\n"
M_MSG_18       "cxref: 1286-164 The %s command is not available to process C++ files.\n"
$ Do not translate C++
M_MSG_19       "Usage: cxref [-cst] [-o File] [-w [Number]]\n\
\t[[-D Name[=Definition]] [-U Name] [-I Directory] [-qOption]] ...\n\
\t[-NdNumber] [-NlNumber] [-NnNumber] [-NtNumber] File ...\n"
M_MSG_20       "cxref: 1286-165 File %s must have a .c or .C extension.\n"
$ Do not translate .c or .C
M_MSG_21       "cxref: 1286-166 There cannot be more than %d preprocessor options.\n"
M_MSG_22       "cxref: 1286-167 %s is not a recognized flag.\n"
M_MSG_23       "cxref: 1286-168 There is not enough memory available.\n"
M_MSG_24       "SYMBOL        FILE                 FUNCTION   LINE\n"
$ These are headings for the C language cross reference listing.  
$ They have technical meaning to the C programmer. It would be  
$ best to leave them as they are. If you do translate, the columns should 
$ be preserved. 
M_MSG_25       "cxref: 1286-169 Warning, invalid argument for -w option, 80 assumed.\n"
$ Do not translate -w
