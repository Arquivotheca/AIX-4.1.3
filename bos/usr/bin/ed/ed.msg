$ @(#)97	1.7  src/bos/usr/bin/ed/ed.msg, cmdedit, bos411, 9428A410j 3/3/93 13:43:35
$ 
$ COMPONENT_NAME: (CMDEDIT) ed.msg
$
$ FUNCTIONS: none
$
$ ORIGINS: 3, 10, 18, 27
$
$ This module contains IBM CONFIDENTIAL code. -- (IBM
$ Confidential Restricted when combined with the aggregated
$ modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1984, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ (c) Copyright 1990 OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$ OSF/1 1.0
$
$
$ 
$quote "
$set MS_ED
$len 
$     Do not translate %s, %d, etc. These will be replaced by file, numbers, etc   
$     The term "regular expression" is a Unix technical term.   
1 "ed: 0602-300 Cannot create a pipe."
2 "ed: 0602-301 Use the w subcommand to write and save the file."
3 "ed: 0602-302 The mark name must be a lower case alphabetic character."
4 "ed: 0602-303 Cannot find or open the input file."
5 "ed: 0602-304 A tab specification is not formatted correctly."
6 "ed: 0602-305 There is no subcommand to undo."
$      "undo" is a specific command within the ed editor   
7 "red: 0602-306 Cannot run the specified subcommand with a restricted shell."
8 "ed: 0602-307 Cannot create the output file."
9 "ed: 0602-308 Cannot write to the output file due to lack of filesystem space."
10 "ed: 0602-309 Cannot find or open the output file."
11 "ed: 0602-310 Cannot link to the output file."
12 "ed: 0602-311 Specify a range end point that is less than 256."
13 "ed: 0602-312 The specified subcommand does not exist."
14 "ed: 0602-313 The search string is not found."
15 "-"
16 "ed: 0602-314 The specified line does not exist."
17 "ed: 0602-315 The character or characters between \\{ and \\} must be numeric."
18 "ed: 0602-316 The first address cannot exceed the second in an address pair."
19 "ed: 0602-317 The specified subcommand does not require an address."
20 "ed: 0602-318 The global subcommand requires a pattern."
$ There is a "global" subcommand in the ed editor (it is not an adjective here) 
21 "ed: 0602-319 The subcommand contains added characters that are not valid."
22 "ed: 0602-320 The specified subcommand requires a file name."
23 "ed: 0602-321 Use a space between the subcommand and the file name."
24 "ed: 0602-322 Cannot create a process at this time."
25 "ed: 0602-323 File names cannot be longer than 64 characters."
26 "ed: 0602-324 Specify a \\digit between 1 and 9 that is\n\
\tnot greater than the number of subpatterns."
27 "ed: 0602-325 An interrupt signal was received. Returning to command mode."
28 "ed: 0602-326 The editor ignores lines longer than 512 characters."
29 "ed: 0602-327 The input file contains characters which are not valid."
30 "ed: 0602-328 Cannot write to the specified file."
31 "ed: 0602-329 There is not enough memory available for the append."
32 "ed: 0602-330 The temporary file buffer cannot contain more than 255 blocks.\n\
\tCannot edit the entire file."
33 "ed: 0602-331 Cannot read to or write from the temporary file buffer."
34 "ed: 0602-332 Cannot use multiple global subcommands."
$      There is a "global" subcommand in the ed editor.   
35 "ed: 0602-333 The global subcommand list cannot exceed 256 characters."
36 "ed: 0602-334 There is no matching pattern."
37 "ed: 0602-335 A delimiter is not correct or is missing."
38 "-"
39 "ed: 0602-336 The replacement string cannot be longer than 512 characters."
40 "ed: 0602-337 The move destination must be numeric and cannot\n\
\tbe one of the lines that is moved."
41 "-"
42 "ed: 0602-338 There is no remembered search string."
43 "ed: 0602-339 There is a missing \\( or \\)."
44 "ed: 0602-340 Specify \\( no more than 9 times."
45 "ed: 0602-341 Specify no more than 2 numbers between \\{ and \\}."
46 "ed: 0602-342 An opening \\{ must have a closing \\}."
47 "ed: 0602-343 The first number cannot exceed the second number\n\
\tbetween \\{ and \\}."
48 "ed: 0602-344 The substitute is not complete."
$      Message 48 does not exist so do not translate.   
49 "ed: 0602-345 Newline was not expected."
$      Message 49 does not exist so do not translate.   
50 "ed: 0602-346 There is a missing [ or ]."
51 "ed: 0602-347 The regular expression is too large."
52 "ed: 0602-348 There is a regular expression error.\n"
53 "ed: 0602-349 The G and V subcommands require a subcommand."
54 "ed: 0602-350 The a, i, or c subcommands cannot be used\n\
\twith the G and V subcommands."
55 "ed: 0602-351 An end-of-line character was expected."
56 "ed: 0602-352 There is no remembered replacement string."
57 "ed: 0602-353 There is no remembered subcommand."
58 "ed: 0602-354 File names cannot begin with a ! character."
59 "ed: 0602-355 A simultaneous edit is being performed on the file."
60 "ed: 0602-356 The specified command cannot be used when the -y flag is set."
61 "ed: 0602-357 The X subcommand replaced the x subcommand."
62 "ed: 0602-358 Warning: An attempt to write the file may destroy it.\n\
\tThe file contains characters which are not valid."
63 "ed: 0602-359 Warning: Quitting may cause the loss of data\n\
\tin the temporary buffer."
M_EDPARG "ed: 0602-360 The -p flag requires a parameter.\n"
M_EDFILE "ed: 0602-361 A file name cannot exceed 64 characters in length."
M_EDNULLS " null characters were replaced by the \\0 character."
$      Please leave the space at the beginning -- this follows a number.   
M_EDINCREAD "ed: 0602-362 \007The attempted read was interrupted\n\
\tand may not be complete.\007"
$ The \007 needs to be left literal.  (It "beeps" the terminal)   
M_EDLINE "ed: 0602-363 Input file lines cannot exceed 512 characters.\n\
\tLine number "
$      Leave the space at the end.   
M_EDTOOLONG "ed: 0602-364 Cannot write a line greater than 512 characters."
M_EDTAB "ed: 0602-365 Cannot write a line greater than 512 characters.\n"
M_EDKEY "Enter the file encryption key: "
M_EDGENKEY "ed: 0602-366 crypt: Cannot generate the encryption key."
$ NOTE:   
$ NOTE: These last two are NOT part of the present "ed" command   
$ NOTE: but may come back if the programmers can figure out a way   
$ NOTE: to do encryption (Do not translate "crypt:")   
$ NOTE:   
ED_USAGE "Usage: ed [-p String] [-s|-] [File]\n"
