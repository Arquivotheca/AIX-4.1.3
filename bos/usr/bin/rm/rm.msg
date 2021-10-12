$ @(#)96	1.18.1.1  src/bos/usr/bin/rm/rm.msg, cmdfiles, bos411, 9428A410j 12/10/92 13:34:31
$
$ COMPONENT_NAME: (CMDFILES) commands that manipulate files 
$
$ FUNCTIONS: message file
$
$ ORIGINS: 18, 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1992
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "
$
$ $ Message numbers 600 - 624 reserved for this file.
$
$set MS_RM
$               GLOBAL COMMENT FOR TRANSLATORS
$ Do not translate %o %s  %1$s %2$s etc.
USAGE "Usage: %s [-firRe] [--] File...\n"
$
USAGE_RMDIR "Usage: rmdir [-p] DirectoryName...\n"
$
CANTREMOVE "%s: 0653-600 Cannot remove '.' or '..'.\n"
$ First %s is the command, the second %s is a file name.
$
DIRECTORY "%1$s: Select files in directory %2$s? Enter y for yes. "
$ Type y key for a yes response. Any other key on the keyboard $ automatically gives the default response which is "no".
$
CANTREAD "%1$s: 0653-602 Cannot read %2$s.\n"
$
ISDIRECT "%1$s: 0653-603 Cannot remove directory %2$s.\n"
$ %s is a directory name.
$
NOTREMOVED "%1$s: 0653-609 Cannot remove %2$s.\n"
$ %1$s is a command that attempted to remove %2$s. The attempt to remove %2$s
$ failed. A system error message about %2$s will be displayed on the next line.
$
CANTRMSELF "%1$s: 0653-610 Cannot remove current directory %2$s.\n"
$ %1$s is the command name.  %2$s is the directory which cannot be removed.
$
CANTRMFULL "%1$s: 0653-611 Directory %2$s is not empty.\n"
$ %1$s is the command name.  %2$s is the name of a directory.
$
REMOVEDDIR "%s: Removing directory %s\n"
$
FILEREMOVED "%s: Removing %s\n"
$ 
REMPROMPT "%s: Remove %s? "
