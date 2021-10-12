$ @(#)83	1.7  src/bos/usr/bin/install/install.msg, cmdfiles, bos411, 9428A410j 12/7/92 10:03:50
$
$ COMPONENT_NAME: (CMDFILES) commands that manipulate files
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1992
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "
$
$ $ Message numbers 230 - 269 reserved for this file
$
$set MS_INSTALL

1 "Usage: install [-c DirectoryA] [-f DirectoryB] [-i] [-m] [-M Mode] [-O Owner]\n\
\t       [-G Group] [-S] [-n DirectoryC] [-o] [-s] File [DirectoryX ...]\n"

2 "install: 0653-230 The %1$s and %2$s flags may not be used together.\n" 
$ The two flags %1$s and %2$s may not both appear in the same install command.

3 "install: 0653-231 File %1$s already exists in directory %2$s .\n"

4 "File %1$s is moved to %2$s .\n"
$ %2$s is the pathname of the destination.

5 "install: 0653-232 The command \"cp %1$s %2$s\" failed.\n"
$ This message reports that a cp command failed.

6 "File %1$s is installed as %2$s .\n"
$ File %1$s was installed as path name %2$s.

7 "The old file %1$s is moved to %2$s .\n"
$ File %1$s already existed. The old one was moved to path name %2$s.

8 "install: 0653-233 File %s was not found.\n"

9 "File %1$s is installed as %2$s by default.\n"
$ File %1$s was installed as path name %2$s.

10 "install: 0653-234 Specify permissions for -M option in absolute mode.\n"

11 "install: 0653-235 The directory %s does not exist.\n"
$ %s represents the directory name.
