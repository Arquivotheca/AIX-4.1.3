$ @(#)41	1.9  src/bos/usr/ccs/bin/ld/ld.msg, cmdld, bos411, 9428A410j 3/7/94 22:28:25 
$ 
$  COMPONENT_NAME: (CMDLD) Message file for the ld command
$ 
$  FUNCTIONS: ld
$ 
$  ORIGINS: 27
$ 
$  (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$  All Rights Reserved
$  Licensed Materials - Property of IBM
$ 
$  US Government Users Restricted Rights - Use, duplication or
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "

$set	LD_ERR_MSG

$ GENERAL COMMENTS FOR TRANSLATION:
$
$ Do not translate "ld:" in any of the messages.  This is the command name.
$
$ Do not add a newline to the end of any message.
$
$ If possible, messages should not be wider than 80 display columns,
$	and they can be reformated if necessary.  To reformat, insert the
$	characters "\n\t" (that is, backslash, 'n', backslash, 't')
$	where line breaks are needed.
$
$ To allow space for parameters (e.g., %1$s, %d, %c), allow approximately:
$		 1 column for %c
$		10 columns for %d or %u (i.e. numbers)
$		30 columns for a file name
$		10 columns for other %s parameters
$	(The English messages may not have used these numbers consistently.)
$
$ GENERAL COMMENTS FOR PROGRAMMERS:
$
$ Messages 0706-001 to 0706-099 can be used for ld.  The rest of the 0706
$	messages were used by "bind".
$
$ Note:  Message 0706-001 must be the first one in the message catalog.

LD_MSG_ERR	"ld: 0706-001 Internal message error.\n\
\tDepending on where this product was acquired, contact your service\n\
\trepresentative or the approved supplier."
$	Additional information:
$		This error is never generated explicitly.  It is used in case
$		of an internal error that causes an out-or-range message
$		number to be used.

LD_INTERNAL_ERR	"ld: 0706-002 Internal error at line %1$d (v=%2$d).\n\
\tDepending on where this product was acquired, contact your service\n\
\trepresentative or the approved supplier."
$	Translation information:
$		Do not translate v=
$		%1$d:	Line number
$		%2$d:	Arbitrary number
$	Additional information:
$		This error should never occur, because it indicates a bug.
$		The number after v= can be used to diagnose the error.

LD_NOTFOUND1	"ld: 0706-003 Cannot find or read import file: %s"
$	Translation information:
$		%s:	File name.
$	Additional information:
$		perror() will report the reason

LD_NOTFOUND2	"ld: 0706-004 Cannot find or read export file: %s"
$	Translation information:
$		%s:	File name.
$	Additional information:
$		perror() will report the reason

LD_NOTFOUND3	"ld: 0706-005 Cannot find or open file: %s"
$	Translation information:
$		%s:	File name.
$	Additional information:
$		perror() will report the reason

LD_LIBNOTFOUND	"ld: 0706-006 Cannot find or open library file: -l %s"
$	Translation information:
$		%s:	File name (Parameter for -l flag)
$	Additional information:
$		perror() will report the reason

LD_OUTOFMEM1	"ld: 0706-007 There is not enough memory available.  Check\
 your ulimit\n\
\tor paging space or use local problem reporting procedures."
$	Translation information:
$		Do not translate: ulimit
$		"paging space" is a technical term

LD_LINE_TOOLONG	"ld: 0706-008 A line is too long in file: -f %1$s\n\
\tThe maximum line length is %2$d bytes."
$	Translation information:
$		%1$s:	File name
$		%2$d:	Maximum line length (an integer)

LD_BADVAL	"ld: 0706-009 The number %1$s is not valid for the \
-b %2$s option.\n\
\tUse a value that is a multiple of %3$d."
$		%1$s:	The number as entered by the user
$		%2$s:	Option name
$		%3$d:	A number

LD_BINDKILLED	"\nld: 0706-010 The binder was killed by a signal"
$	Translation information:
$		The initial "\n" is intentional.  Do not delete it.
$		"killed" is a technical term
$		This message is printed by the library function psignal(),
$		which adds a colon (:) and a description of the signal.
$		Then message LD_BINDKILLED2 is printed.

LD_BINDKILLED2	"\tCheck for binder messages or use local problem reporting procedures."
$	Translation information:
$		This message is always printed immediately
$		after message LD_BINDKILLED is printed.

LD_ARGREQ	"ld: 0706-011 The -%c flag needs a parameter."
$	Translation information:
$		%c	A single character

LD_BADOPT	"ld: 0706-012 The -%c flag is not recognized."
$	Translation information:
$		%c	A single character

LD_BADNOP	"ld: 0706-013 The string %s is not valid.  The parameter for\n\
\tthe -b nop option must be cror15, cror31, ori, or an unsigned,\n\
\t8-digit hexadecimal number, with or without a leading 0x or 0X."
$	Translation information:
$		Do not translate: -b nop, cror15, cror31, ori
$		%s:	Option parameter.

LD_UNKBOPT	"ld: 0706-014 The -b %s option is not recognized."
$	Translation information:
$		%s:	Option name

LD_ARGBREQ	"ld: 0706-015 The -b %1$s option needs a parameter.\n\
\tOption syntax: -b %1$s:PARM"
$	Translation information:
$		%1$s:	Option name
$		PARM is an abbreviation for PARAMETER

LD_BNOARG	"ld: 0706-016 The -b %s option does not take a parameter."
$	Translation information:
$		%s:	Option name

LD_B2ARG	"ld: 0706-017 The -b %1$s option needs two parameters.\n\
\tOption syntax: -b %1$s:PARM1,PARM2"
$	Translation information:
$		%1$s:	Option name
$		PARM1 is an abbreviation for PARAMETER1
$		PARM2 is an abbreviation for PARAMETER2

LD_BNO2		"ld: 0706-018 The -b %1$s option does not take two \
parameters.\n\
\tOption syntax: -b %1$s:PARM"
$	Translation information:
$		PARM is an abbreviation for PARAMETER
$		%1$s:	Option name

LD_BADNUM	"ld: 0706-019 The number %1$s is too large for the \
-%2$c flag.\n\
\tUse a value that is %3$u (hex %3$X) or less."
$	Translation information:
$		%1$s:	The number as entered by the user
$		%2$c:	A single character
$		%3$u, %3$x:	A number

LD_BADNUMOPT	"ld: 0706-020 The number %1$s is too large for the \
-b %2$s option.\n\
\tUse a value that is %3$u (hex %3$X) or less."
$	Translation information:
$		%1$s:	The number as entered by the user
$		%2$s:	Option name
$		%3$u, %3$x:	A number

LD_SYSCALLFAIL	"ld: 0706-021 The %s() system call failed."
$	Translation information:
$		%s:	System call name
$	Additional information:
$		perror() will report the reason

LD_NOBINDARGS	"ld: 0706-022 Cannot write the binder command string to\
 file: %s"
$	Translation information:
$		"command string" is a technical term
$		%s:	File name
$	Additional information:
$		perror() will report the reason

LD_NOBINDCMDS	"ld: 0706-023 Cannot write the binder commands to\
 file: %s"
$	Translation information:
$		%s:	File name
$	Additional information:
$		perror() will report the reason

LD_WAITFAILED	"ld: 0706-024 The wait() system call returned %d.\n\
\tCheck for binder messages or use local problem reporting procedures."
$	Translation information:
$		Do not translate: wait()
$		%d:	The return code (an integer)

LD_NOEXEC	"ld: 0706-025 Cannot call the binder program: %s"
$	Additional information:
$		perror() will report the reason
$	Translation information:
$		%s:	File name (pathname of binder command)

LD_OIGNORED1	"ld: 0706-026 The -b %s option is ignored."
$	Translation information:
$		%s:	Option name

LD_FIGNORED1	"ld: 0706-027 The -%c%s%s flag is ignored."
$	Translation information:
$		Do not reorder -%c%s%s parameters
$		%c:	A flag (a single character)
$		%s:	" " if the flag has an parameter;  "" otherwise
$		%s:	Flag parameter, or "", if the flag has no parameter.

LD_BADSYN1	"ld: 0706-028 Use a number with the -b %s option."
$	Translation information:
$		%s:	Option name

LD_BADSYN2	"ld: 0706-029 Use a number with the -%c flag."
$	Translation information:
$		%c:	A single character

LD_NO_INPUT	"ld: 0706-030 No processing done.  Specify at \
least one\n\
\tinput or import file or use at least one -b ex<n> option."
$	Translation information:
$		Do not translate "-b ex"
$		(The notation "-b ex<n>" is an abbreviation for the five
$		options -b ex1, -b ex2, -b ex3, -b ex4, -b ex5)
