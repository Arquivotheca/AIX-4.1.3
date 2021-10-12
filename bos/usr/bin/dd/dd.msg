$ @(#)16	1.12  src/bos/usr/bin/dd/dd.msg, cmdarch, bos411, 9428A410j 1/6/94 10:43:17
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

$ This catalog uses numbers in the range: 041-065 and 500-524.
$quote "
$set MS_DD
$ Do not translate dd

TRUNREC "%llu truncated record.\n"
$ %u is the number 1.

TRUNRECS "%llu truncated records.\n"
$ %u is a number greater than or equal to 2.

RECSIN "%1$llu+%2$llu records in.\n"
RECSOUT "%1$llu+%2$llu records out.\n"

$ The messages TRUNREC, TRUNRECS, RECSIN, and RECSOUT are informational
$ messages, not error messages.  DO NOT preceed them with "dd:" or message
$ numbers.

BYSIG "dd: 0511-041 The process was killed by signal number %d.\n"

ESCTREAD "dd: 0511-042 Cannot read sector %llu\n"

ESCTWRITE "dd: 0511-043 Cannot write to sector %llu\n"

EWRITFC "dd: 0511-044 The child process cannot write to an internal pipe.\n"
$ Do not translate child process, pipe

EREADFP "dd: 0511-045 The parent process cannot read an internal pipe.\n"
$ Do not translate parent process, pipe

EREADFC "dd: 0511-046 The child process cannot read an internal pipe.\n"
$ Do not translate child process, pipe

EWRITFP "dd: 0511-047 The parent process cannot write to an internal pipe.\n"
$ Do not translate parent process, pipe

ERRPIPE "dd: 0511-048 An error occurred on an internal pipe.\n"
$ Do not translate pipe, /tmp

EFORK "dd: 0511-049 Cannot create another process.\n"

ESHMEM "dd: 0511-050 Cannot get shared memory.\n"

EREAD "dd: 0511-051 The read failed.\n"
$ Do not translate read

ESHMAT "dd: 0511-052 A shared memory error occurred.\n"

EWRITE "dd: 0511-053 The write failed.\n"
$ Do not translate write

EBLKREAD "dd: 0511-054 Cannot read block %llu.\n"
$ Do not translate read

EBLKWRITE "dd: 0511-055 Cannot write to block %llu.\n"
$ Do not translate write

EARG "dd: 0511-056 The command parameter %s is not correct.\n"

ERECSIZ "dd: 0511-057 The specified record size for conv=block is not correct.\n"
$ Do not translate conv=block

EBLKSIZ "dd: 0511-058 Block sizes cannot be zero.\n"

ENOBSIZE "dd: 0511-059 The parameter conv=block requires block-sized records.\n"
$ Do not translate conv=block

EDAC "dd: 0511-060 The parameter conv=block requires a direct access device.\n"
$ Do not translate conv=block

ENOMRY "dd: 0511-063 There is not enough memory available now.\n"

EPRMSKP "dd: 0511-064 Must have read permission to skip into the file %s.\n"
$ Do not translate skip

EWRFEW "dd: 0511-065 Requested a write of %1$1d bytes, but wrote only %2$2d.\n"

BADCONV "dd: 0511-500 The conv=%s is does not apply to multibyte code sets.\n"
$ The %s is the conversion method that was specified by the user.

BADMBCH "dd: 0511-501 An invalid multibyte character was encountered.\n"

ESTOMP  "dd: 0511-502 The if= and of= options cannot specify the same file.\n"

USAGE1 "Usage: dd [if=InputFile] [of=OutputFile] [cbs=Number] [fskip=Number]\n"
USAGE2 "          [skip=Number] [seek=Number] [count=Number] [bs=Number]\n"
USAGE3 "          [ibs=Number] [obs=Number] [files=Number] [conv=Parameter[, ...]]\n"
$ Do not translate any of the keywords that come before the "=" characters.
$ (such as "if=" and "conv=").
