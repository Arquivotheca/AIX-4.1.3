$ @(#)10        1.8  src/bos/usr/bin/sccs/bsd.msg, cmdsccs, bos411, 9428A410j 3/16/93 16:01:43
$
$ COMPONENT_NAME: CMDSCCS       source code control system
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1991, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "         define quote character.

$ Do not translate %s, %d, %1$s, etc..
$ Do not translate sccs:
$len 
$set 14        Define initial set#

$ This is the message catalog for the sccs command. 
NOUSER               "sccs: 1255-180 User %s does not exist.\n\
\tChange the PROJECTDIR environment variable to an existing user name.\n"
$ %s is a user name. 
$ Do not translate PROJECTDIR. 
NOSOURCE             "sccs: 1255-181 Project %s has no /source or /src subdirectory.\n"
$ Project %s is the user named with the PROJECTDIR variable. 
SCCSUSAGE            "Usage: sccs [-d Path] [-p Path] [-r] Command [CommandFlag...] [File..]\n"
$ %s is the sccs command 
$ Do not translate sccs. 
UNKNOWNOPT           "sccs: 1255-183 Flag -%s is not recognized.\n\
\tUsage: sccs [-d Path] [-p Path] [-r] Command [CommandFlag...] [File...]\n"
$ %s is a flag. 
$ Do not translate sccs. 
UNKNOWNCMD           "sccs: 1255-184 %s is not an SCCS command.\n"
$ %s is a command that is not recognized. 
NEEDR4FIX            "sccs: 1255-185 Specify the -r flag with the fix command.\n"
$ Do not translate -r. 
$ Do not translate fix. 
CANNOTEXEC           "sccs: 1255-186 Cannot run %s.\n"
$ %s is a program. 
CANNOTFORK           "sccs: 1255-187 Cannot create another process.\n"
NOMEM                "sccs: 1255-189 There is not enough memory available.\n"
$ Do not translate sccs. 
PATHERR              "sccs: 1255-190 Cannot use .. or full path names.\n"
$ Do not translate .. 
TOOMANYARGS          "sccs: 1255-191 Specify only one directory.\n"
CANTOPEN             "sccs: 1255-192 Cannot open directory %s.\n"
$ %s is a directory. 
BEDIT                "%s is being edited: "
$ %s is a file. 
$ Because this can be an informational message or an error message we cannot 
$ show recovery information. 
$ Following the colon (:) is the version number and date. 
NOEDIT               "Nothing is being edited.\n"
$ This message shows that no trunks or branches of the SCCS file tree are being 
$ edited. 
NOEDITTRNK           "No trunk of the SCCS file tree is being edited.\n"
NOEDITBY             "Nothing is being edited by %s.\n"
$ %s is a user name. 
NOEDITTRNKBY         "No trunk of the SCCS file tree is being edited by %s.\n"
$ %s is a user name. 
BADFNAME             "sccs: 1255-198 File name %s is not in correct SCCS format.\n"
$ %s is a file name. 
NOBEDIT              "sccs: 1255-199 File %s is not being edited.\n"
$ %s is a file name. 
$ Do not translate unedit. 
CANTCREATE           "sccs: 1255-200 Cannot create %s.\n"
$ %s is a file name. 
CANTREMOVE           "sccs: 1255-201 Cannot remove %s.\n"
$ %s is a file. 
CANTREOPEN           "sccs: 1255-202 Cannot reopen %s.\n"
$ to reopen is to open again
REMOVED              "Temporary file %s is removed.\n"
$ %s is a temporary file. 
$ This is an informational message used by the unedit pseudocommand only. It 
$ removes the temporary file. 
NOEDITBYYOU          "sccs: 1255-204 Only the user who is editing the file can unedit the file.\n"
$ Do not translate unedit
DODIFFPIPE           "sccs: 1255-205 The pipe system call failed.\n"
$ Do not translate pipe, dodiff. 
DODIFFFORK           "sccs: 1255-206 Cannot create another process.\n"
$ Do not translate dodiff
DODIFFMAGIC          "sccs: 1255-207 The close system call or the dup system call failed.\n"
$ Do not translate close, dup, dodiff. 
UNKNOWNUID           "sccs: 1255-208 %d is not an existing user number.\n"
FNAMSTOOBIG          "sccs: 1255-209 The following file name(s) are too large:\n\
\t%1$s %2$s.\n"
$ %1$s is a file. %2$s is a file. 
