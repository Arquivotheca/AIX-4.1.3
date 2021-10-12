$ @(#)29        1.3.1.1  src/bos/usr/bin/help/help.msg, cmdman, bos411, 9428A410j 9/7/93 10:40:57
$
$ COMPONENT_NAME: (CMDMAN) commands that allow users to read online
$                          documentation
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
$ messages for help
$ note: translation of this message file is limited within one screen.
$len
$set MS_HELP        Define initial set#
$ Leave spaces in message.

HELP                 "\n\
Use the InfoExplorer facility for aid. If available, you can refer to the\n\
\"Using and Managing AIX\" Information Topic for general assistance.\n\
\n\
Some basic Commands are:\n\
\n\
    man -k keyword\t- lists commands relevant to a keyword \n\
    man command\t\t- prints out the manual pages for a command \n\
    cat\t\t\t- concatenates files (and just prints them out) \n\
    vi\t\t\t- text editor \n\
    ls\t\t\t- lists contents of directory \n\
    mail\t\t- sends and receives mail \n\
    msgs\t\t- system messages and junk mail \n\
    passwd\t\t- changes login password \n\
    sccshelp\t\t- views information on the Source Code Control System \n\
    smit\t\t- system management interface tool \n\
    tset\t\t- sets terminal modes \n\
    who\t\t\t- who is on the system \n\
    write\t\t- writes to another user \n\
\n\
To find programs about mail, use the command: \t\tman -k mail \n\
and print out the man command documentation via:\tman mail \n\
You can log out of the system by typing: \t\texit\n\
\n"
