$ @(#)53	1.14.2.14  src/bos/usr/bin/bsh/bsh.msg, cmdbsh, bos411, 9428A410j 1/28/94 17:13:47
$
$ COMPONENT_NAME: (CMDBSH) Bourne shell and related commands
$
$ FUNCTIONS:
$
$ ORIGINS: 27, 71
$
$ This module contains IBM CONFIDENTIAL code. -- (IBM
$ Confidential Restricted when combined with the aggregated
$ modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
$ ALL RIGHTS RESERVED 
$
$ OSF/1 1.1
$
$ Messages for the shell.
$
$quote  "

$set MS_BSH
$ Do not translate bsh

$       informational messages

M_ATLINE " at line "
$ This is preceded by M_SYNMSG, and followed by a line number.
$ leave a space at the beginning and at the end.
M_BUILT "%s is a shell builtin.\n"
$ %s is substituted with the name of a command.
$ A builtin is a function that is within the shell.
M_COREDUMP ": A memory image file is created as \"core\"."
$ Do not translate "core"
M_ENDOFFILE "end of file"
M_FUNC "%s is a function.\n"
$ %s is substituted with the name of a function.
M_HPR "Hits\tCost\tCommand\n"
$ These are table headers. The first two cannot be longer than 8 characters.
$ Number of times the hash value of the symbol is found.
$ Cost is when the symbol is not found immediately, and goes searching.
M_IS "%1$s is probably %2$s.\n"
$ The first %s is substituted with the name of a command that is hashed to the
$ second %s which is substituted with the name of command
M_ISH "%1$s is the hashed version of %2$s.\n"
$ The first %s is substituted with the name of a command that is hashed to the
$ second %s which is substituted with the name of command
M_MAILMSG "You have mail.\n"
M_NL "newline or ;"
$ This is preceded by M_SYNMSG, and sometimes M_ATLINE.
$ This is then followed by M_UNEXPECTED. Example:
$ Syntax error at line 3: `newline or ;' not expected.
M_NOTFOUND " not found."
$ This is preceded by a filename, or whatever could not be found.
M_SYNMSG "Syntax error"
$ This precedes the M_ATLINE message and others, to indicate an error.
M_UNEXPECTED " not expected."
$ This line is preceded by either the symbol that is not expected, or
$ by M_SYNMSG, M_ATLINE, and M_NL
M_WTFAILED "This variable is read only."
$


$ $      error messages

M_ARGEXP "0402-017 Specify a parameter with this command."
M_ARGLIST "0402-018 The parameter list is too long."
M_BADCREATE "0402-011 Cannot create the specified file."
M_BADEXEC "0402-021 Cannot run the command as specified."
M_BADEXPORT "bsh: 0402-029 Cannot export functions."
M_BADHASH "Usage: hash [-r] [Name . . .]"
$ Do not translate "hash" or "-r"
M_BADFILE "0402-022 Generated or received a file descriptor number that is not valid."
M_BADNUM "0402-007 The specified number is outside the range for this command."
M_BADOP "0402-034 The specified operator is not valid with this command"
$ Do not put a period at the end.
M_BADOPEN "0402-016 Cannot find or open the file."
M_BADOPT "0402-003 A specified flag is not valid for this command."
M_BADPARAM "0402-008 Parameter is null or not set."
M_BADPERM "0402-032 Execute permission is denied."
$ Execute is referring to the execute permission of the file. It is a
$ Security term.
M_BADRETURN "bsh: 0402-028 Cannot perform a return when not in a function."
M_BADSHIFT "bsh: shift: 0402-023 The specified number is not valid for this command."
$ Do not translate shift
M_BADSUB "bsh: 0402-010 The specified substitution is not valid."
M_BADTRAP "0402-024 The specified trap syntax is not correct."
$ Do not translate trap.
M_BADUNSET "0402-030 Cannot find variable to unset."
$ Do not translate unset
M_CD_ARGS "0402-040 The cd command in bsh accepts only one parameter."
$ Do not translate cd
M_LONGPWD "bsh: pwd: 0402-033 The specified path name is too long."
M_NOFORK "bsh: 0402-012 The fork function failed. Too many processes already exist."
$ Do not translate fork
M_NOHOME "bsh: 0402-031 Cannot determine what directory is the home directory."
M_NOSPACE "bsh: 0402-004 There is not enough memory available now."
M_NOSWAP "bsh: 0402-013 There is not enough memory available now."
M_NOTID "0402-026 The specified data is not a valid identifier."
M_NOTF "0402-002 Cannot find %s.\n"
M_PERM "0402-035 Permission denied."
M_PIPERR "bsh: 0402-015 Cannot create a pipe."
$ Do not translate pipe.
M_PWDOPEN "bsh: pwd: 0402-038 Cannot open parent directory (..)"
$ Do not translate pwd, (..)
M_PWDREAD "bsh: pwd: 0402-005 Cannot read parent directory (..)"
$ Do not translate pwd, (..)
M_RBRACKET "0402-039 There is a missing ] character."
$ Do not translate ]
M_RESTRICTED "0402-014 The operation is not allowed in a restricted shell."
M_RPAREN "0402-020 There is a missing ) character."
$ Do not translate 0
M_STATDOT "bsh: pwd: 0402-036 Cannot get file status information for this directory (.)"
$ Do not translate pwd, (.)
M_STATDOTDOT "bsh: pwd: 0402-037 Cannot get file status information for parent directory (..)"
$ Do not translate pwd, (..)
M_TRAP "0402-001 Cannot trap SIGSEGV."
$ Do not translate trap SIGSEGV
M_TXTBSY "0402-019 Cannot access a required executable file. It is in use."
M_UNKNOWN "0402-025 The specified name is not a builtin function.\n"
M_UNSET "0402-009 Parameter is not set."
M_NO_ARGS "requires argument.\n"
M_UNLIMITED "unlimited"
$ There is no ulimit for this resource.
M_ULIMITEXCEED  "0402-041 Requested ulimit exceeds current hard limit."
$ Do not translate ulimit
M_ULIMITUSAGE "Usage: ulimit [-H | -S] [ -c | -d | -f | -m | -s | -t] [limit]"
$ Do not translate ulimit, -H, -S, -c, -d, -f, -m, -s, -t
M_ULIMITBAD "0402-043 Specify a numeric ulimit value."
$ Do not translate ulimit
M_ULIMITNOTSU "0402-044 Only superuser can set hard limit."
M_ULIMITRESOURCE "0402-045 Failure getting or setting a resource value."
M_ULIMITHARD "0402-046 Current soft limit exceeds requested hard limit."
M_ULIMITSOFT  "0402-049 Requested ulimit exceeds current soft limit."
M_SYSCONF "bsh: sysconf error"
$ Do not translate sysconf, a perror message comes after to complete message
M_NOMEM "bsh: no memory"
$ A perror message comes after to complete message
M_FULLFS "bsh: 0402-047 File system is full."
M_MSSGARGN "0402-048 The getopts command in bsh requires at least two parameters."
$ Do not translate getopts
