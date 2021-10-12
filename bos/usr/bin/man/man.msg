$ @(#)52        1.11.1.2  src/bos/usr/bin/man/man.msg, cmdman, bos411, 9428A410j 10/25/93 17:01:40
$
$ COMPONENT_NAME: (CMDMAN) commands that allow users to read online
$ documentation
$
$ FUNCTIONS: none
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
$quote "         define quote character.
$len
$set MS_man        Define initial set#
$ $error numbers will be 3xx

$ Do not translate man, %s, etc.

M_MSG_1		"man: 0703-302 The man page name is missing.\n"
M_MSG_3		"There is not an entry for %s.\n"
M_MSG_4		"There is not an entry for %1$s in section %2$c.\n"
M_MSG_5		"Reformatting page. Please wait ..."
M_MSG_6		"man: 0703-304 The program has ended abnormally.\n"
M_MSG_7		" completed\n"
M_MSG_8		"man: 0703-305 A parameter is missing.\n"
M_MSG_9		"%s: Cannot find matching entry.\n"
M_MSG_10	"man: 0703-306 A parameter is missing.\n"
M_MSG_11	"man: 0703-307 %s is not found.\n"
M_MSG_12	"Usage: man [[[-ct] [section]] | [-f | -k]] [-M path] name...\n"
M_MSG_14	"Usage: apropos [-M path] keyword...\n"
M_MSG_15	"Usage: whatis [-M path] command...\n"
M_MSG_18	"man: 0703-308 Nroff/troff must be installed in order\n\
to use formatted man pages.\n"
$ Do not translate catman -w
ISEMPTY		"\nman: 0703-309 database %s is empty.\n  Build the database using a <catman -w> command.\n"
$ do not translate whatis
NOTFOUND	"\nman: 0703-310 file %s not found.\n  Create the whatis database using the <catman -w> command\n"
M_MSG_19	"man: 0703-311 Error writing to temporary file.\n" 
M_MSG_20	"man: 0703-312 Cannot open the database of manual pages.\n"
M_MSG_21	"man: 0703-313 A section is not valid.\n"
M_MSG_22	"man: 0703-314 The ispaths file is in the wrong format.\n"
$ Do not translate ispaths.
M_MSG_23	"man: 0703-315 Cannot access InfoExplorer Databases.\n"
$ Do not translate InfoExplorer.
NODB		"man: 0703-316 Cannot open %s.\n"
