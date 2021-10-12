$ @(#)72        1.6.1.3  src/bos/usr/ccs/bin/cflow/cflow.msg, cmdprog, bos411, 9428A410j 3/31/94 12:37:07
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

$set MS_DAG        Define initial set#
$
$ The message numbers have been assigned a range of 100 - 149
$ Do not translate cflow:, %s, %c.  
1                    "cflow: 1286-100 File %s must have a .o, .s, .i, .c, .l, or .y extension.\n"
$ Do not translate .o, .s, .i, .c, .l, .y  
2                    "Usage: cflow [-dNumber] [-i_] [-ip] [-ix] [-r] [-MA]\n\
\t[-NdNumber] [-NlNumber] [-NnNumber] [-NtNumber] File ...\n"
DBOPT                "cflow: 1286-101 %c is not a recognized flag or is not used correctly.\n"
$ %c is a flag.  
TMCHS                "cflow: 1286-102 There is not enough memory available to add more characters.\n"
TMNDS                "cflow: 1286-103 There is not enough memory available to add more nodes.\n"
TMLNKS               "cflow: 1286-104 There is not enough memory available to add more links.\n"
103                  "cflow: 1286-105 %s not present -- file %s skipped.\n"
104                  "cflow: 1286-106 File %s must have a .o, .s, .i, .c, .C, .l, or .y extension.\n"
$ Do not translate .o, .s, .i, .c, .C, .l, .y  
105                  "cflow: 1286-107 Cannot open file %s.\n"
106                  "Usage: cflow [-d Number] [-i _] [-i p] [-i x] [-r] [-MA]\n\
\t[[-D Name[=Definition]] [-U Name] [-I Directory] [-qOption]] ...\n\
\t[-NdNumber] [-NlNumber] [-NnNumber] [-NtNumber] File ...\n"
107                  "cflow: 1286-108 There are no files to process.\n"
108                  "cflow: 1286-109 Incorrect usage of option: %s.\n"
109                  "cflow: 1286-110 Option: %s is not recognized.\n"
