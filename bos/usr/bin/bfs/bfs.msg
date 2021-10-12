$ @(#)32	1.8  src/bos/usr/bin/bfs/bfs.msg, cmdscan, bos411, 9428A410j 4/15/93 17:06:29
$
$ COMPONENT_NAME: (CMDSCAN) commands that scan files
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "
$set MS_BFS
$ Do not translate bfs.
$
ADDRERR         "bfs: 0652-055 Address 1 must be smaller than address 2.\n"
ALRDYDIV        "bfs: 0652-056 The output file is already specified.\n"
ARGCNT          "bfs: 0652-057 The number of parameters is not valid.\n"
BADCMD          "bfs: 0652-058 The specified command is not valid.\n"
BADCMDL         "bfs: 0652-059 The command list is not valid.\n"
BADDIGIT        "bfs: 0652-060 Only the digits 0 - 9 are valid\n."
$ A valid digit is a single number, 0,1,2,3,4,5,6,7,8, or 9.
BADMARK         "bfs: 0652-061 Only the marks a - z are valid.\n"
BADNUM          "bfs: 0652-062 The specified number is not valid.\n"
CANTCREAT       "bfs: 0652-063 Cannot create the output file.\n"
CANTOPEN        "bfs: 0652-064 Cannot open the file.\n"
CUSAGE          "bfs: 0652-065 The address is out of range.\n"
DIGRQD          "bfs: 0652-066 Specify a digit from 0 - 9.\n"
JUMPTTY         "bfs: 0652-067 Input is from the display device.\n"
LBLNFND         "bfs: 0652-068 The label is not found.\n"
NOCHANGE        "bfs: 0652-069 A change is not indicated.\n"
NODIGIT         "bfs: 0652-070 Specify a digit from 0 - 9.\n"
NODIV           "bfs: 0652-071 Specify a file for directed output.\n"
$       No output file specified.
NOFILEM         "bfs: 0652-072 Cannot find the file.\n"
NOFLNM          "bfs: 0652-073 Specify a file name.\n"
NOTFOUND        "bfs: 0652-074 The pattern is not found.\n"
RANGE           "bfs: 0652-075 The value is out of range.\n"
SYNTAX          "bfs: 0652-076 Command usage error.\n"
TOOLINES        "bfs: 0652-077 There are too many lines.\n"
UNDEFMARK       "bfs: 0652-078 The mark is not defined.\n"
TRYAGN          "bfs: 0652-079 Cannot open pipe.\n"
$ Do not translate pipe.
CANOTOPEN       "bfs: 0652-080 Cannot open %s.\n"
USAGE		"Usage: bfs [-] File"
$ bfs is command name. Do not translate
