$ @(#)38        1.6  src/bos/usr/ccs/bin/regcmp/regcmp.msg, cmdprog, bos411, 9428A410j 2/16/94 16:32:50
$
$   COMPONENT_NAME: (CMDPROG) Programming Utilities
$
$   FUNCTIONS: none
$
$   ORIGINS: 27,10
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1988,1989
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$len
$set    MS_REGCMP
$quote  "
$ The message numbers have been assigned a range of 600 - 649
$ Do not translate regcmp: 

NOTOPN               "regcmp: 1286-600 Cannot open the input file %s\n"
NOCREAT              "regcmp: 1286-601 Cannot create the .i file.\n"
$ Do not translate .i.  
UNEXEOF              "regcmp: 1286-602 The end of the file was not expected.\n"
INITQUOT             "regcmp: 1286-603 The initial quote is missing for regular expression %s\n"
$ The REMIGN message follows this message.  
REMIGN               "\tThe remainder of the line is ignored.\n"
$ The INIQUOT message precedes this message.  
FAIL                 "regcmp: 1286-604 Cannot compile regular expression associated with %s\n"
