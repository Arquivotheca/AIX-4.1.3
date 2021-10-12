$ @(#)17	1.15  src/bos/usr/bin/cpio/cpio.msg, cmdarch, bos411, 9428A410j 4/28/93 13:39:39
$
$ COMPONENT_NAME: (CMDARCH) archive files
$
$ FUNCTIONS:
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
$quote "
$set MS_CPIO
$ Do not translate cpio

ATPFTS "cpio: 0511-001 Cannot pass a file to itself.\n"

BCIRR "cpio: 0511-003 Cannot use the -B or -C flags with the -p flag.\n"

BSR "The block size was reduced to %u bytes.\n"

BSTL "cpio: 0511-002 The block size is too large.\n"

CBSINPUT "Cannot read from the device.\n"
$ This is preceded by ERRNO on the same line.

CBSOUTPUT "Cannot write to the device.\n"
$ This is preceded by ERRNO on the same line.

CCD "cpio: 0511-005 Cannot create directory for %1$s; errno is %2$d.\n"
$ Do not translate errno.
$ The %1$s will be a file name and the other will be an integer

CCDNW "cpio: 0511-006 Cannot move to directory.\n"
$ Don't spend a lot of time on this one -- it is never seen

CCF "cpio: 0511-007 Cannot create %1$s; errno is %2$d.\n"
$ Do not translate errno.
$ The %1$s will be a file name and the other will be an integer

CCHD "cpio: 0511-008 Cannot move to a specified directory.\n"
$ Don't spend a lot of time on this one -- it is never seen

CCLN "cpio: 0511-009 Cannot create a symbolic link.\n"

CCM "cpio: 0511-010 Cannot change permissions of %1$s; errno is %2$d.\n"
$ Do not translate errno.
$ The %1$s will be a file name and the other will be an integer

CCO "cpio: 0511-011 Cannot change ownership of %1$s; errno is %2$d.\n"
$ Do not translate errno.
$ The %1$s will be a file name and the other will be an integer

CFN "cpio: 0511-012 current <%s> newer or same age\n"
$ The %s will be a file name

CLSTS "cpio: 0511-013 Cannot link %1$s and %2$s.\n"
$ The variables will be file names

CNMN "cpio: 0511-014 Cannot create %1$s; errno is %2$d.\n"
$ Do not translate errno.
$ The %1$s will be a file name and the other will be an integer

COBS "cpio: 0511-015 The -B flag value overrides the -C flag value.\n"

COCS "cpio: 0511-016 The -C flag value overrides the -B flag value.\n"

COPN "cpio: 0511-017 Cannot open %s.\n"

CRNDT "cpio: 0511-018 Cannot use the -r flag while running in the background.\n"

CRS "cpio: 0511-019 Cannot read from %s.\n"

CRSL "cpio: 0511-020 Cannot read from a symbolic link.\n"

CUL "cpio: 0511-021 Cannot unlink %1$s; errno is %2$d.\n"
$ Do not translate errno.
$ The %1$s will be a file name and the other will be an integer

CWF "cpio: 0511-022 Cannot write to %s.\n"
$ The %s will be a file name

CWIF "cpio: 0511-023 Cannot write to the %s directory.\n"

ERRNO "cpio: 0511-004 The errno is %d; "
$ This is followed by either CBSINPUT or CBSOUTPUT
$ Leave space at the end, a phrase will follow.

FLTF "%1$s is linked to %2$s.\n"
$ Both will be file names

GOON "Enter the device or file name when ready to continue.\n"

IGNSPECIAL "cpio: 0511-024 Special file %s was ignored.\n"

IHDRQS "cpio: 0511-025 Cannot get information about %s.\n"
$ The %s will be a file name

IHDRQ "cpio: 0511-026 Cannot open %s.\n"
$ The %s will be a file name

LOFDM "cpio: 0511-027 The file name length does not match the expected value.\n"

MISD "cpio: 0511-028 Cannot move to a necessary directory.\n"

NOTUSED "cpio: 0511-461 Original file has a later modification date. Cannot write.\n"

NEXTDISK "\007cpio: End of diskette, insert the next diskette.\n"
$ Leave the \007cpio:

NEXTTAPE "\007cpio: End of tape, insert the next tape.\n"
$ Leave the \007cpio:

NMFL "cpio: 0511-462 There is not enough memory available to continue linking files.\n"

NOTUSED2 "%ld blocks were copied.\n"

NOMLC "cpio: 0511-029 There is not enough memory available now.\n"

NOSPECIAL "cpio: 0511-030 Special file %s cannot be archived.\n"

OMIL "cpio: 0511-031 Use one of the -o, -i, or -p flags.\n"

OOF "cpio: 0511-032 Cannot copy from the media.\n"
$ Do not translate cpio

PREX "cpio: 0511-033 Cannot use the -r flag with the -p flag.\n"

RPRPT "Rename <%s>\n"
$ Do not translate.

SCLN "cpio: 0511-034 Cannot create a symbolic link.\n"

SKIP "Skipped\n"
$ Do not translate

USAGE1 "Usage: cpio -o[acv][B|CNumber]\n"
USAGE2 "       cpio -i[bcdmrstuvfS6] [B|CNumber] [Pattern ...]\n"
USAGE3 "       cpio -p[adlmuv] Directory\n"
$
USE_D "cpio: 0511-036 Use the -d flag to copy %s.\n"

WINPUT "cpio: 0511-037 Cannot read from the specified input.\n"

WOUTPUT "cpio: 0511-038 Cannot write to the specified output.\n"

UID "cpio: 0511-900 %s not archived; file's uid cannot be greater\n\
\t than 65535.\n"
$ %s is the file name.

GID "cpio: 0511-901 %s not archived; file's gid cannot be greater\n\
\t than 65535.\n"
$ %s is the file name.

FILES "cpio: 0511-902 Too many files for cpio to archive.\n"

PHASE "\n\n cpio: 0511-903 Out of phase!\n\
\t cpio attempting to continue...\n"

PHEND "\n\n cpio: 0511-904 skipping %d bytes to get back in phase!\n\
\t One or more files lost and the previous file is possibly corrupt!\n\n"
$ %d is the byte count.

NEW "cpio: 0511-461 Original file <%s> has a later modification\n\
\tdate. Cannot write.\n"
$ The %s will be a file name
NOBLKCOP "%ld blocks \n"
$ %d is the number of blocks.
