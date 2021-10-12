$ @(#)41	1.22  src/bos/usr/bin/tar/tar.msg, cmdarch, bos411, 9428A410j 1/31/94 14:38:56
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
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "
$set MS_TAR
$
$ Valid error number ranges for tar: 0511-162 thru 0511-199
$
$
$               GLOBAL COMMENT FOR TRANSLATORS
$ Do not translate tar -- this is the command name.
$ Do not translate %o %s  %1$s %2$s etc.
$ These are output formats for file names, directory
$ names, permission codes, command names, and the like,
$ depending on their context within each message.
$ The program replaces %o,%1$o,etc with an octal number.
$ The program replaces %d,%1$d,etc with a decimal number.
$ the program replaces %.2f with a fixed-point number.
$ the program replaces %u with an unsigned integer.
$ The program replaces %.1f%% with a percentage.
$ The program replaces %s,%1$s,etc with a string of
$ characters.

BACKSPCI "tar: 0511-181 ioctl backspace error"
$ This message is followed by a colon and a perror() message.

BACKSPCL "tar: 0511-196 lseek backspace error"
$ This message is followed by a colon and a perror() message.

BLKS "a %1$s %2$ld blocks.\n"
$ %1$s is replaced by a file name.
$ %2$ld is replaced by a whole number.
$ do not translate "a" -- it means "archive" by AIX convention.

BLKSIZE "tar: The block size is %d bytes.\n"
$ %d is replaced by a whole number.

BLKSON "tar: There are %1$ld blocks on %2$s.\n"
$ %2$s is replaced by a file/device name.
$ %1$ld is replaced by a whole number.

CREDUCED "tar: The cluster size is too large; reduced to %d blocks.\n"
$ %d is replaced by a whole number.

DOARCH "Archive %s? "
$ Do not add a newline (\n) to this message.
$ %s is replaced by a file name.

DOEXTR "Extract %s? "
$ Do not add a newline (\n) to this message.
$ %s is replaced by a file name.

EBLKSIZE "tar: 0511-164 There is a media read or write block size error.\n"

EBLKBIG "tar: 0511-165 Not enough memory available for block size %d bytes.\n"

EBSIZE "tar: 0511-166 %s is not a valid numeric block size.\n"
$ %s is replaced by a file name.

ECHBACK "tar: 0511-167 Cannot change back: "
$ The program inserts a standard system message
$ from the "perror" subroutine after the : character.

ECHDIR "tar: 0511-168 Cannot change directories to "
$ The program inserts a standard system message
$ from the "perror" subroutine after the word to.

ECSUMC "tar: 0511-169 A directory checksum error on media; %1$d not equal to %2$d.\n"
$ do not translate "checksum" -- it is an AIX term.

EDIRREAD "tar: 0511-170 Cannot read directory %s.\n"

EEOT "tar: 0511-171 Reached end of media.\n"
$ The tar command cannot prompt for a new\n\
$ tape when running in background. Run the tar command\n\
$ in the foreground.\n"

EFFLAG "tar: 0511-172 Specify a file name with the -f flag.\n"

EFSIZE "tar: 0511-173 File %s is not of the expected file size.\n"

EFLINK "tar: 0511-174 Cannot link %1$s to %2$s: "
$ The program inserts a standard system message
$ from the "perror" subroutine after the : character.

ELINKS "tar: 0511-175 Links to %s are missing.\n"

ELONGFN "tar: 0511-176 File name %s is too long.\n"

ELONGSL "tar: 0511-177 Symbolic link name %s is too long.\n"

EMALLOC "tar: 0511-162 There is not enough memory available now.\n"

EMEM "tar: 0511-178 There is not enough memory available now.\n"

EMKNOD "0511-179 mknod failed while trying to create file %s.\n"
$ Do not translate mknod. It is an AIX system call.

ENOARCH "tar: 0511-180 %s is not a valid tar file type.\n"

EOT "\007tar: Mount next media on device %s and press return.\n"
$ Do not translate \007tar:
$ %s is replaced by a tape unit identifier.

EREAD "tar: 0511-182 Read error on "
$ The program inserts a standard system message
$ from the "perror" subroutine after the word on.

EREADSL "tar: 0511-183 Cannot read symbolic link: "
$ The program inserts a standard system message
$ from the "perror" subroutine after the word link.

ESETTIME "tar: 0511-184 Cannot set the time on %s: "
$ %s is replaced by a file name.
$ The program inserts a standard system message
$ from the "perror" subroutine after the : character.

ESTAT "tar: 0511-185 Cannot get information about file %s.\n"

ESTDOUT "tar: 0511-186 Can create only standard output archives.\n"

ESYMFAIL "tar: 0511-187 %s: Symbolic link has failed: "
$ %s is replaced by a file name.
$ The program inserts a standard system message
$ from the "perror" subroutine after the : character.

ETAR "tar: "
$ The program inserts a standard system message
$ from the "perror" subroutine after the : character.

ETARS "tar: %s\n"
$ %s is replaced by a file name.

ETCREATE "tar: 0511-188 Cannot create %s: "
$ Do not translate "create" on the first line.  It is
$ the name on an AIX system call.  Translate "create"
$ on the third line -- it means "to make".

ETCRTMP "tar: 0511-189 Cannot create temporary file %s.\n"

ETLONGFN "tar: 0511-190 File name %s is too long.\n"

ETNOBSIZE "tar: 0511-191 Specify the block size with the -b or -N flag.\n"

ETOPEN "tar: 0511-192 Cannot open %s.\n"

ETREAD "tar: 0511-193 An error occurred while reading from the media.\n"

ETWREOF "tar: 0511-194 Reached end-of-file before expected.\n"

ETWRITE "tar: 0511-195 An error occurred while writing to the storage media.\n"

EXWRITE "tar: 0511-197 %s: Cannot write data extracted with the tar command:\n"

LINKTO "a %1$s is linked to %2$s.\n"
$ %1$s and %2$s  are replaced by file names.

LINKEDTO "tar: %1$s is linked to %2$s.\n"
$ %1$s and %2$s  are replaced by file names.

LINKED " is linked to %s.\n"
$ the program inserts a file name in front of this message.

PROCEED "tar: Proceeding to device %s.\n"
$ this is an informational message that the user
$ sees after responding to message EOT

SLINKTO "a %1$s symbolic link to %2$s.\n"
$ the letter  a indicates archive.  do not translate.
$ %1$s and %2$s  are replaced by file names.

SLINKED " symbolic link to %s.\n"
$ the program inserts a file name in front of this message.

USAGE1 "Usage: tar -{c|r|t|u|x} [-BdFhilmopsvw]\n\
\t   [-Number] [-fFile]\n"

USAGE2 "\t   [-bBlocks] [-S [Feet] [Feet @Density] [Blocksb]]\n"

USAGE3 "\t   [-LInputList] [-NBlocks] [-C Directory] File ...\n"

XSTAT "x %1$s, %2$ld bytes, %3$ld media blocks.\n"
$ appears as message when using the -v flag.
$ the letter  x indicates extract.  do not translate.

XSYMLINK "x %1$s is a symbolic link to %2$s.\n"
$ appears as message when using the -v flag.
$ the letter  x indicates extract.  do not translate.

ELONGLN "tar: 0511-163 %s is linked to a path name that is too long.\n"

CHKDEV "tar: 0511-198 Check the backup medium and press return to continue.\n"

ETREADEOF "tar: Unexpected end-of-file while reading from the storage media.\n"

NODEVTTYOPEN "tar: 0511-199 Can not open /dev/tty for interactive operation.\n"
