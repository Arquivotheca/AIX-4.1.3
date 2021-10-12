$ @(#)10	1.7  src/bos/usr/bin/dircmp/dircmp.msg, cmdfiles, bos411, 9428A410j 8/3/93 16:37:25
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
$ (C) COPYRIGHT International Business Machines Corp. 1989,1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "
$
$ $ Message numbers 155 - 174 reserved for this file.
$
$set MS_DIRCMP
$
14 "Usage: dircmp [-ds] directory1 directory2\n"
$
15 "dircmp: 0653-155 %s is not a directory.\n"
$
16 "%1$s only and %2$s only"
$ Do not put newline at the end of the above message.
$ This is the header for the lists of files in each directory that are not
$ also in the other directory.
$
17 "Comparison of %1$s and %2$s"
$ Do not put newline at the end of the above message.
$ This is the header for the comparison of files that are in both directories.
$
18 "diff of %1$s in %2$s and %3$s"
$ Do not put newline at the end of the above message.
$ This is the header for the comparison by diff of the file named %1$s in
$ directory %2$s and in directory %3$s when dircmp is invoked with option -d.
$
19 "same\t\t%s\n"
$ This line says that file %s is the same in both directories when dircmp
$ is invoked without the -s option.
$
20 "different\t%s\n"
$ This line says that file %s is different in the two directories.
$
21 "directory\t%s\n"
$ This line says that file %s is a directory.
$
22 "special\t%s\n"
$ This line says that file %s is a special file.
