$ @(#)43	1.12  src/bos/usr/bin/pack/pack.msg, cmdfiles, bos411, 9428A410j 11/23/92 16:46:47
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
$ $ Message numbers 500 - 534 reserved for this file.
$
$set MS_PACK
$               GLOBAL COMMENT FOR TRANSLATORS
$ Do not translate pack: -- this is the command name.
$ Do not translate .z; this is the filename extension for packed files.
$ Do not translate %o %s %1$s %2$s etc.
$ The program replaces %o,%1$o,etc with an octal number.
$ The program replaces %d,%1$d,etc with a decimal number.
$ the program replaces %.2f with a fixed-point number.
$ The program replaces %3$.1f%% with a percentage.
$ The program replaces %s,%1$s,etc with a string of
$ characters.
$
READERR1 "%1$s: %2$s: 0653-501 Read error;"
$ This message is preceded by pack: filename
$
WRITERR "%1$s: %2$s.z: 0653-502 Write error;"
$ This message is preceded by pack: filename
$
PCKDMSG "%1$s: %2$s: This file is already packed.\n"
$ This message is preceded by pack: filename
$
FILNAMLEN "%1$s: %2$s: 0653-504 This file name is too long.\n"
$ This message is preceded by pack: filename
$
CANTOPEN "%1$s: %2$s: 0653-505 Cannot open this file.\n"
$ This message is preceded by pack: filename
$
PACKDIR "%1$s: %2$s: 0653-506 Cannot pack a directory.\n"
$ This message is preceded by pack: filename
$
LINKCNT "%1$s: %2$s: 0653-507 Cannot pack this file. It has links.\n"
$ This message is preceded by pack: filename
$
ZEXISTS "%1$s: %2$s.z: 0653-508 A file by this name already exists.\n"
$ This message is preceded by pack: filename
$
CANTCREAT "%1$s: %2$s.z: 0653-509 Cannot create this file.\n"
$ This message is preceded by pack: filename
$
CANTUNLNK "%1$s: 0653-511 Cannot unlink %2$s.\n"
$
COMPRESS "%1$s: %2$s: This file is reduced in size %3$.1f%%.\n"
$ This message is preceded by pack: filename
$ The program replaces %.1f%% with a percentage.
$
SIZEMSG "\tThe file changed from %1$ld to %2$ld bytes.\n"
$
HUFFLVLS "\tThe Huffman tree has %d levels below the root.\n"
$ Do not translate "Huffman".  It is the name of a file building technigue.
$
DISTINCT "\tThere are %d distinct bytes in input.\n"
$ %d is replaced by the number of bytes in the unpacked file.
DICTOVRHD "\tThe dictionary overhead is %ld bytes.\n"
$ dictionary overhead is a part of the packed file which contains information
$ that will be later used by the unpack command. 
$ %d is the size of the dictionary overhead.
EFFENTROPY "\tThe effective entropy is %.2f bits per byte.\n"
$ effective entropy means the average number of bits per byte for this
$ packed file including the dictionary overhead.
ASYMENTROPY "\tThe asymptotic entropy is %.2f bits per byte.\n"
$ asymptotic entropy means the average number of bits per byte for this
$ packed file not including the dictionary overhead.
$
USAGE "Usage: pack [-f] [-] File...\n"
$ "pack" is the command name. Do not translate "pack".
NOSAVE2 "%1$s: %2$s: The pack command cannot make the file smaller,\n\
\tso it does not change the file.\n"
$
