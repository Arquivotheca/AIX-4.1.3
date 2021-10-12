# @(#)54	1.2  src/bos/usr/bin/errlg/samples/README.S, cmderrlg, bos411, 9434A411a 8/19/94 16:39:57

#
# COMPONENT_NAME: (CMDERRLG) Error Logging Samples
#
# FUNCTIONS: README FILE FOR THE CORE DUMP LOCATOR
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1990,1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

When an application core dumps, a core file is placed in the
current directory if the user can write there.  Unfortunately,
we are not able to determine the path name of the dump at dump
time.  The "corepath" package uses error notification to
detect that a core dump has happened.  It then finds the
core file, if possible, and then logs an operator message
with the dump file's pathname.  This is particularly useful
for system administrators who want to look at the dumps, but
weren't the ones operating the system when the dump occurred.

NOTE:  Currently we only search journaled file systems (jfs).
There are a couple of line to comment out in corepath if you
want to search any type of file system.  Be aware that this may
cause unwanted network traffic and may take hours to find the dump,
however,

The associated files for corepath are:
corepath - the script that finds the dump.
getvfsname - A program used by corepath to get the file system
	     containing the dump.

How to use corepath
1>  You must be root to start it.

2>  Put the error notification object into ODM.  Put the error
    notification object at the end of this file into a separate
    file, then run "odmadd <file>" to add the object to the
    errnotify object class.  Note that the file should begin
    with the line "errnotify:".


To stop error notification from searching for core dumps, issue the command
"odmdelete -o errnotify -q 'en_name = corepath'".


Error notification ODM Object
errnotify:
    	en_name = corepath
    	en_persistenceflg = 1
	en_label = CORE_DUMP
    	en_method = "/usr/samples/findcore/corepath $1 root"
