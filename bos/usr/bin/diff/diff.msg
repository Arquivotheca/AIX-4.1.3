$ @(#)06	1.15.1.1  src/bos/usr/bin/diff/diff.msg, cmdfiles, bos411, 9433A411a 8/12/94 11:05:29
$
$ COMPONENT_NAME: (CMDFILES) commands that manipulate files
$
$ FUNCTIONS:
$
$ ORIGINS: 18, 27, 71
$
$ This module contains IBM CONFIDENTIAL code. -- (IBM
$ Confidential Restricted when combined with the aggregated
$ modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$ 
$ (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$ OSF/1 1.1
$ No RCS id string found in diff.msg
$
$ messages for command diff (programs diff and diff -h)
$
$quote "
$
$ $ Message numbers 800 - 859 reserved for this file.
$
$set MS_DIFF
$ Do not translate diff, %s, %1$s, %d, -D, -s, -l, -e
$
ENOI "diff: 0653-800 Cannot use the -D flag when comparing directories.\n"
ENOSL "diff: warning: The -s or -l flag should not be used with the -e flag.\n"
$
EONLY "Only in %.*s"
$
EIDENT "Common identical files"
$ 
EBINARY "Binary files which differ"
$ 
$
ESUBDIR "Common subdirectories"
$ 
$
ETWO "diff: 0653-802 Specify two file names.\n"
EBAD "diff: 0653-804 Cannot use the -h flag with the -e, -f, -n, -c, or -D flags.\n"
EMEM "diff: 0653-805 There is not enough memory available now.\n"

SUBDIR "Common subdirectories: %s and %s\n"
$ 

FILES "Files %s and %s are identical\n"
$ the %s's are replaced by file names.
DBINARY "Binary files %s and %s differ\n"
$ 

NOPROC "diff: 0653-806 Cannot create another process.\n"

SUBARE "Common subdirectories are "
$ 

IDENT "%s and %s are identical\n"
$ 
$
SYMLINK "symbolic link files:\n\t"
$ 

ESIZE "diff: 0653-807 The specified files are too large. Try the -h flag.\n"
ECREATE "diff: 0653-808 Cannot create %s.\n"
$ %s is replaced by a file name.
EOPEN "diff: 0653-809 Cannot open %s.\n"
$ %s is replaced by a file name.
EDIFFH "0653-810 Cannot find the diffh command.\n"
$ do not translate diffh

DMAGIC "0653-812 Cannot open %s.\n"
$ %s is replaced by a file name.
DDIFFER1 "%s --> %s "
$ Do not put a newline character at the end. DDIFFER2 will follow this.
DDIFFER2 " and %s --> %s differ\n"
$ 

DSTDINS "diff: 0653-813 Cannot use the - character for more than one file name.\n"
ENODIF "There are no differences between the files.\n"
$ 
DSTDIN "0653-814 Cannot compare standard input against the directory.\n"

ESTDINS "0653-817 Cannot use the - character for more than one file name.\n"

EPROGERR "0653-820 Program error "
$ Leave the space at the end -- A number -- something like "4"  or  "2"
$ will be printed on the same line.
ECHANGE "---change record omitted\n"
$ 
ERESYNC "Cannot recover. Try using the bdiff command.\n"

$ New or revised messages for 3.2
$ Do not translate the names:  diff  or  diffh.  
EILLOPT "diff: 0653-821 illegal option -- %c\n"
EOPTARG "diff: 0653-822 option requires an argument -- %c\n"
EBADCNT2 "diff: 0653-803 Specify a number with the -%c flag.\n"
USAGE "Usage: diff [-bcitw] [[-C Lines|-D String|-e|-f|-n]|[-h]] File1 File2\n\
       diff [-bcilrstw] [[-C Lines|-e|-f|-n]|[-h]] [-S File] Directory1 Directory2\n"
EALLOTMP "diff: 0653-823 Cannot allocate space for temporary file.\n"
ENOPATHMAX "diff: 0653-824 Cannot get _PC_NAME_MAX.\n"
$ Do not translate _PC_NAME_MAX. Be sure to spell _PC_NAME_MAX exactly as it appears.
ESLINK "diff:0653-825 Cannot allocate symbolic link space.\n"
EARGS2 "diffh: 0653-816 Specify two file names.\n"
DIFFHUSE "Usage: diffh [-b] File1 File2\n"
EACCESS2 "diffh: 0653-818 Cannot access or get status of file %s\n"
EOPEN3 "diffh: 0653-819 Cannot open file %s.\n"

EONLY2 "Only in %.*s"
$ DO NOT TRANSLATE
$ This is diff output, and the EXACT wording is dictated by standards.
$ Your customers will expect it this way.
$ What will follow is a list of files that are only in a certain directory
EIDENT2 "Common identical files in %.*s and %.*s"
$ DO NOT TRANSLATE
$ This is diff output, and the EXACT wording is dictated by standards.
$ Your customers will expect it this way.
$ Common meaning files which are in both directories
EBINARY2 "Binary files which differ in %.*s and %.*s"
$ DO NOT TRANSLATE
$ This is diff output, and the EXACT wording is dictated by standards.
$ Your customers will expect it this way.
ESUBDIR2 "Common subdirectories of %.*s and %.*s"
$ DO NOT TRANSLATE
$ This is diff output, and the EXACT wording is dictated by standards.
$ Your customers will expect it this way.
$ Common meaning subdirectories which are in both directories
EBADCNT3 "diff: 0653-826 Specify a positive number with the -%c flag.\n"
NONEW "diff: 0653-827 Missing newline at the end of file %s.\n"
SPFILE "File %s is a %s while file %s is a %s.\n"
DIRT "directory"
CHAR "character special file"
BLCK "block special file"
FIFO "fifo"
SOCK "socket"
SMLK "symbolic link file"
REGU "regular file"
