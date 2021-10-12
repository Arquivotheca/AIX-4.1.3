$ @(#)42	1.6  src/bos/usr/bin/pg/pg.msg, cmdscan, bos411, 9428A410j 4/1/93 13:44:50
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
$ Used message numbers 110 - 140
$quote "         define quote character.
$len
$set MS_PG        Define initial set#

$      The term "regular expression" is a Unix technical term.
$      Do not add a new line unless it is included in the source.
$      Do not translate %s, %d, %1$s, etc.
$
$ The help messages, H01 - H24 cannot be expanded with new lines.
$ The text MUST all fit on one line for each of these messages.

1                    "pg: 0652-110 The pattern is not found."
USGE                 "Usage: pg [-Number] [-p String] [-cefns] [+LineNumber] [+/Pattern/] [File...]\n"
PATLONG              "pg: 0652-111 The pattern is too long."
PRMPTLNG             "pg: 0652-112 The prompt is too long.\n"
SKIPBW               "...skipping backward\n"
SKIPFW               "...skipping forward\n"
OPSAVERR             "pg: 0652-113 Cannot find or open the save file."
SAVFIL               "Saving file %s"
SAVED                "Saved"
$ This comes back to the screen to show successful completion of the file save.
EXECFAIL             "pg: 0652-114 Cannot run the command."
11                   "pg: 0652-115 The range endpoint is too large."
NOPREV               "pg: 0652-116 There is not a previous file."
NONEXT               "pg: 0652-117 There is not a next file."
DIRECT               "pg: 0652-118 %s is a directory.\n"
SPECFIL              "pg: 0652-119 Special files are handled only as standard input.\n"
16                   "pg: 0652-120 The number is not valid."
NOTEMP               "pg: 0652-121 Cannot create the temporary file.\n"
TEMPERRW             "pg: 0652-122 Cannot write to the temporary file.\n"
TEMPERRR             "pg: 0652-123 Cannot read the temporary file.\n"
PIPSAV               "The piped input is only partially saved."
MEMOUT               "pg: 0652-124 There is not enough memory available now. Stopped at line %d.\n"
NOREOPN              "pg: 0652-125 Cannot open the standard output.\n"
NXTFIL               "The next file is %s"
25                   "pg: 0652-126 The \\digit is out of range."
$ Do not translate \digit
TERMTYP              "pg: 0652-127 Cannot find the terminal type."
36                   "pg: 0652-128 Delimeter is not valid or not found."
41                   "pg: 0652-129 There is not a remembered search string."
42                   "pg: 0652-130 There is a missing \\( or \\)."
$ Do not translate \( or \)
43                   "pg: 0652-131 There are too many ( characters."
44                   "pg: 0652-132 There are more than two numbers in \\{ \\}."
$ Do not translate \{ or \}
45                   "pg: 0652-133 } is expected after \\"
$ Do not translate { or \
46                   "pg: 0652-134 The first number exceeds the second in \\{ \\}."
$ Do not translate \{ or \}
48                   "pg: 0652-135 The range endpoint is not valid."
49                   "pg: 0652-136 There is a missing [ or ] character."
50                   "pg: 0652-137 There is a regular expression overflow."
H01                  "-------------------------------------------------------\n"
$    This message is to form a frame for the menu. Do not translate.
H02                  "   h               help\n"
$    Start the items in the same columns.
$    Do not translate h.
H03                  "   q or Q          quit\n"
$    Start the items in the same columns.
$     Do not translate the first q or Q.
H04                  "   <blank> or \\n   next page\n"
$    Start the items in the same columns.
$     Do not translate \n
H05                  "   l               next line\n"
$    Start the items in the same columns.
$     Do not translate the first l
H06                  "   d or ^D         display half a page more\n"
$    Start the items in the same columns.
$     Do not translate the first d and ^D
H07                  "   . or ^L         display current page again\n"
$    Start the items in the same columns.
$     Do not translate . and ^L
H08                  "   f               skip the next page forward\n"
$    Start the items in the same columns.
$     Do not translate the first f
H09                  "   n               next file\n"
$    Start the items in the same columns.
$     Do not translate the first n
H11                  "   p               previous file\n"
$    Start the items in the same columns.
$     Do not translate the first p
H12                  "   $               last page\n"
$    Start the items in the same columns.
$     Do not translate $
H13                  "   w or z          set window size and display next page\n"
$    Start the items in the same columns.
$     Do not translate w, z on the left.
H14                  "   s savefile      save current file in savefile\n"
$    Start the items in the same columns.
$     Do not translate the first character s
H15                  "   /pattern/       search forward for pattern\n"
$    Start the items in the same columns.
$      Do not translate character /
$      "pattern" refers to the pattern in the Usage statement. Keep consistent.
H16                  "   ?pattern? or\n"
$    Start the items in the same columns.
$     Do not translate the character ?
$      "pattern" refers to the pattern in the Usage statement. Keep consistent.
H17                  "   ^pattern^       search backward for pattern\n"
$    Start the items in the same columns.
$     Do not translate the character ^
$      "pattern" refers to the pattern in the Usage statement. Keep consistent.
H18                  "   !command        run command\n"
$    Start the items in the same columns.
$     Do not translate the character !
H19                  "\n"
$ This line contains only a new line ... no text.
H20                  "Most commands can be preceded by a number, as in:\n"
H21                  "+1\\n (next page); -1\\n (previous page); 1\\n (page 1).\n"
$ You may translate only the words "next page", "previous page", "page 1"
$ This line may not exceed 80 characters, you cannot create a new line.
H22                  "\n"
$ This line contains only a newline...no text.
H23                  "See InfoExplorer for more details.\n"
H24                  "-------------------------------------------------------\n"
BADREG               "pg: 0652-138 The regular expression is not valid."
SAYEOF               "(EOF)"
$ Do not translate.
CANTFORK             "pg: 0652-139 Cannot create a process."
