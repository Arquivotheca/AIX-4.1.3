$ @(#)22	1.2.1.1  src/bos/usr/ccs/bin/make/make.msg, cmdmake, bos411, 9432A411a  8/5/94  14:07:16
$ COMPONENT_NAME: CMDMAKE      System Build Facilities (make)
$
$ FUNCTIONS: none
$
$ ORIGINS: 27, 85
$
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Right - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$ 
$ OSF/1 1.2
$

$quote "
$set MS_MAKE
$

TOUCH1 "Touch complete for file %s.\n"
$ Touch is a unix command
$ %s will be a file name

TOUCHERR2 "make: 1254-014 Cannot touch file %s.\n"
$ Do not translate "make"
$ touch is a unix command
$ %s will be a file name

NORULE1 "make: 1254-002 Cannot find a rule to create target %s from \
dependencies.\n(continuing)\n"
$ Do not translate "make"
$ %s will be the name of a target

NORULE2 "make: 1254-002 Cannot find a rule to create target %s from \
dependencies.\nStop.\n"
$ Do not translate "make"
$ %s will be the name of a target

MODIFIED "modified %s..."
$ %s will be modification time of a target

NOEXIST1 "non-existent..."

MODBEFORE "modified before source..."

NOEXIST2 "non-existent and no sources..."

OPER1 ":: operator and no sources..."

UNKNOWN "make: 1254-049 Target \"%s\" not known.\n"
$ Do not translate "make"
$ %s will be a target name

MAINNAME "(MAIN NAME)  "

MAINTARG "# *** MAIN TARGET ***\n"

UNMADEYES "# %d unmade children\n"
$ %d will be number of children not made

UNMADENO "# No unmade children\n"

UNMADE1 "unmade"

UNMADE2 "# unmade\n"

MADE1 "made"

UPTODATE1 "up-to-date"

ERRMADE "error when made"

ABORTED1 "aborted"

IPARENTS "# implicit parents: "

PARENTS "# parents: "

GRAPHMSG1 "#*** Input graph:\n"

GRAPHMSG2 "#\n#   Files that are only sources:\n"

GRAPHMSG5 "#*** Internal (default) Variables:\n"

GRAPHMSG3 "#*** Global Variables:\n"

GRAPHMSG4 "#*** Command-line Variables:\n"

INTRULESERR "make: 1254-045 Cannot open the internal rules file %s."
$ Do not translate "make"
$ %s will be a file pathname

CANTOPEN1 "make: 1254-027 Cannot open %s."
$ Do not translate "make"
$ %s will be a file pathname

CANTFORK "make: 1254-050 Cannot fork a new process for %s."
$ Do not translate "make"
$ fork is a unix command
$ %s will be a command to be executed

BADWAIT "make: 1254-011 The wait system call failed with status %d."
$ Do not translate "make"
$ wait is a unix system call
$ %d will be the return status for wait

USAGE "usage: make [-einqrst] [-k|-S] [-d[A|adg[1|2]mstv]] [-D variable] [-f makefile ] [variable=value ...] [target ...]\n"
$ Do not translate "make"

RECURSM "make: 1254-033 A macro cannot define itself: %s."
$ Do not translate "make"
$ %s will be a macro name

NOEQUAL "make: 1254-051 Equal sign not found in macro substitution.\n"
$ Do not translate "make"

VARMSG "Result is \"%s\"\n"
$ %s will be a variable assignment string

VARERR "make: 1254-052 Variable not specified correctly: %s\n"
$ Do not translate "make"
$ %s will be a variable name

REMOVED "*** %s removed\n"
$ %s will be a file name

MTSTRING "make: 1254-053 Command \"%s\" expands to empty string.\n"
$ Do not translate "make"
$ %s will be a command string

NOTFOUND ": not found\n"

ERRCODE "make: 1254-004 The error code from the last command is %d.\n"
$ %d will be a return status code

SIGCODE "make: 1254-059 The signal code from the last command is %d.\n"
$ %d will be signal number

CONTIN " (continuing)\n"

ERRCODE1I "make: 1254-005 Ignored error code %d from last command.\n"
$ Do not translate "make"
$ %d will be a return status code

EXAMINE "Examining %s..."
$ %s will be a target or dependency name

UPTODATE2 "up-to-date.\n"

OUTOFDATE1 "out-of-date.\n"

UPTIME "update time: %s\n"
$ %s will be modification time of a file

STOPMSG "\n\nStop.\n"

PREDCIRL "make: 1254-008 Target \"%s\" is dependent on itself.\n"
$ Do not translate "make"
$ %s will be target name

UP2DATE "Target \"%s\" is up to date.\n"
$ %s will be a target name

NOTMADERR "Target \"%s\" did not make because of errors.\n"
$ %s will be a target name

DIRMSG1 "Searching for %s..."
$ %s will be a filename

DIRMSG2 "could not open path, file not found\n"

DIRMSG3 "here..."

DIRMSG4 "component mismatch -- continuing..."

DIRMSG5 "returning %s\n"
$ %s will be a filename

DIRMSG6 "must be here but is not -- returning NULL\n"

DIRMSG7 "failed.\n"

DIRMSG8 "failed. Trying subdirectories..."

DIRMSG9 "checking %s..."
$ %s will be a filename

DIRMSG10 "got it.\n"

DIRMSG11 "Caching %s for %s\n"
$ First %s will be modification time for a file
$ Second %s will be a filename

DIRMSG12 "failed. "

DIRMSG13 "Checked . already, returning NULL\n"

DIRMSG14 "Looking for \"%s\"..."
$ %s will be a filename

DIRMSG15 "got it (in mtime cache)\n"

DIRMSG16 "failed. Returning NULL\n"

DIRMSG17 "Using cached time for %s for %s\n"
$ First %s will be modification time for a file
$ Second %s will be a filename

DIRMSG18 "Caching %s..."
$ %s will be a filename

DIRMSG19 "done\n"

DIRMSG20 "#*** Directory Cache:\n"

DIRMSG21 "# Stats: %d hits %d misses %d near misses %d losers (%d%%)\n"
$ all of the %d's will be numbers

DIRMSG22 "# %-20s referenced\thits\n"

DIRMSG23 "directory"

DIRMSG24 "in '.'\n"

PARSERR1 "\"%s\", line %d: "
$ %s will be a filename
$ %d will be a line number in the file

PARSEWARN "warning: "

BADRULE1 "make: 1254-037 The dependency lines for target %s cannot contain\nboth single and double colons."
$ Do not translate "make"
$ %s will be a target name

ARCHERR1 "make: 1254-054 Archive not specified correctly: %s"
$ Do not translate "make"
$ %s will be a library archive file name

PARSERR2 "make: 1254-055 Dependency line needs colon or double colon operator."
$ Do not translate "make"

POSIXERR ".POSIX directive is not first non-comment line."

EXTARGET "Extra target ignored"

MIXTARGET "Special targets cannot be mixed with regular targets.\nRegular targets ignored."

SHELLERR "make: 1254-056 Shell command not associated with a dependency: %.20s."
$ Do not translate "make"
$ %s will be a shell command

SHELLTAB "make: 1254-057 Shell command needs a leading tab."
$ Do not translate "make"

FATALMSG "make: 1254-058 Fatal errors encountered -- cannot continue.\n"

NOMAKEFILE "make 1254-025 There must be an existing description file or specify a target."

SUFFMSG1 "inserting %s(%d)..."
$ %s will be a suffix name
$ %d will be a suffix number

SUFFMSG2 "at end of list\n"

SUFFMSG3 "before %s(%d)\n"
$ %s will be a suffix name
$ %d will be a suffix number

SUFFMSG4 "already there\n"

SUFFMSG5 "deleting transformation from %s to %s\n"
$ %s's will be transformation names

SUFFMSG6 "transformation %s complete\n"
$ %s will be a transformation name

SUFFMSG7 "\ttrying %s..."
$ %s will be a filename

SUFFMSG8 "got it\n"

SUFFMSG9 "not there\n"

SUFFMSG10 "\tusing existing source %s\n"
$ %s will be a filename

SUFFMSG11 "Expanding \"%s\"..."
$ %s will be a suffix name

SUFFMSG12 "\tapplying %s -> %s to \"%s\"\n"
$ %s's will be suffix names

SUFFMSG13 "using null suffix\n"

SUFFMSG14 "\tNo transformation from %s -> %s\n"
$ %s's will be suffix names

SUFFMSG15 "\tNo known suffix on %s. Using .NULL suffix.\n"
$ %s will be a suffix name

SUFFMSG16 "\tNo valid suffix on %s.\n"
$ %s will be a suffix name

SUFFMSG17 "#\tTo: "

SUFFMSG18 "#\tFrom: "

SUFFMSG19 "#*** Suffixes:\n"

SUFFMSG20 "#*** Transformations:\n"

SUFFMSG21 "defining transformation from `%s' to `%s'\n"
$ %s's will be suffix names

NOPARENS "No closing parenthesis in archive specification.\n"

LASTMOD "# last modified %s: %s\n"
$ First %s will be modification time of file
$ Second %s will be a character string

NOMAYBE "# non-existent (maybe): %s\n"
$ %s will be a character string

ARGCORRUPT "1254-060 Argument list is corrupted."

HASHERR "Cannot delete hash table entry\n"

INTERNAL "Internal (default)"

GLOBAL "Global"

COMMAND "Command"
