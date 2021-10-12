$ @(#)42        1.8  src/bos/usr/bin/compress/compress.msg, cmdfiles, bos411, 9428A410j 1/27/94 16:21:09
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
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ messages for compress
$
$quote "
$
$ $ Message numbers 050 - 079 reserved for this file.
$
$set MS_COMPRESS
$               GLOBAL COMMENT FOR TRANSLATORS
$ Do not translate compress -- this is the command name.
$ Do not translate uncompress
$ Do not translate %d %s %1$s %2$s etc.
$ The program replaces %d,%1$d,etc with a decimal number.
$ The program replaces %s,%1$s,etc with a string of
$ characters.

MISSMAXBT "compress: 0653-050 Specify a number of bits with the -b flag.\n"
$
NOTCOMP "0653-052 The %s file is not compressed.\n"
$ Do not translate uncompress.
$
BITSWRONG "0653-053 File %1$s was compressed \n\
\twith %2$d bits. Only %3$d bits are allowed.\n"
$
ALREADYZ "compress: 0653-054 %s already has a .Z suffix. It is not changed.\n"
$ Do not translate .Z
$ %s is replaced with a file name.
$
NAME2LONG "compress: 0653-055 File name %s is too long.\n"
$ %s is replaced with a file name.
$
ALRDYEXST ""
$ This is a null line. Do not translate.
$
WSHOVRWRIT "Do you wish to write over the existing file %s? "
$ Do not put a new line at the end. Leave a blank at the end.
$
NOTOVRWRIT "\tThe file is not written over.\n"
$
INNOTCOMP "0653-057 The standard input is not compressed.\n"
$
STDINBITS "0653-058 The standard input was compressed\n\
\twith %2$d bits. Only %3$d bits are allowed.\n"
$
COMPRESSION "Compression: "
$  the program inserts a calculated percentage after the (:) colon.
$ Leave a space after the colon. Do not put a new line at the end.
$
NOTREG "This is not a regular file. It is not changed."
$ This message is preceded by the filename on the same line.
$ Do not put a new line at the end.
$ Message NOTREG is obsolete, replaced by NOTREG2.
HASLINKS "This file already has %d links. It is not changed."
$ This message is preceded by the filename on the same line.
$ Do not put a new line at the end.
$ Message HASLINKS is obsolete, replaced by HASLINKS2.
UNCHANGE "This file is not changed; compression does not save space."
$ This message is preceded by the filename on the same line.
$ Do not put a new line at the end.
$ Message UNCHANGE is obsolete, replaced by UNCHANGE2.
REPLACEW "This file is replaced with %s."
$ This message is preceded by the filename on the same line.
$ Do not put a new line at the end.
$ Message REPLACEW is obsolete, replaced by REPLACEW2.
CORRUPTIN "uncompress: 0653-059 The input file contains bad data; SIGSEGV signal received.\n"
$ Do not translate SIGSEGV
$
OPTS "Options: "
$ Leave a space after the colon.
CPUSAGE2 "Usage: compress [-cCdfFnqvV] [-b Bits] [File...]\n"
UNCPUSAGE2 "Usage: uncompress [-cfFnqvV] [File...]\n"
ZCATUSAGE2 "Usage: zcat [-nV] [File...]\n"
$
NOTREG2 "This is not a regular file. It is not changed.\n"
$ This message is preceded by the filename on the same line.
$
HASLINKS2 "This file already has %d links. It is not changed.\n"
$ This message is preceded by the filename on the same line.
$
UNCHANGE2 "This file is not changed; compression does not save space.\n"
$ This message is preceded by the filename on the same line.
$
REPLACEW2 "This file is replaced with %s.\n"
$ This message is preceded by the filename on the same line.
$
