$ @(#)08  1.5  src/bos/usr/bin/iconv/iconv.msg, cmdiconv, bos411, 9428A410j  11/8/93  00:07:23
$
$   COMPONENT_NAME: CMDICONV
$
$   FUNCTIONS: iconv message catalogue
$
$   ORIGINS: 27
$
$   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$   combined with the aggregated modules for this product)
$                    SOURCE MATERIALS
$
$   (C) COPYRIGHT International Business Machines Corp. 1991, 1993
$   All Rights Reserved
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "
$set MS_ICONV

M_ARGBOO	"iconv: 0791-001 argument not valid\n"
$
$	"fromcode" is a part of syntax. Do not translate
$
M_FROMMISS	"iconv: 0791-002 fromcode is missing\n"
$
$	"tocode" is a part of syntax. Do not translate
$
M_TOMISS	"iconv: 0791-003 tocode is missing\n"
M_CANNOTCONV	"iconv: 0791-004 cannot open converter\n"
M_CANNOTFILE	"iconv: 0791-005 cannot open input file\n"
M_INVALIDCHAR	"iconv: 0791-006 invalid chararacter found\n"
M_TRUNC		"iconv: 0791-007 truncated character found\n"
M_NOMEMORY	"iconv: 0791-008 unable to allocate enough memory\n"
M_IOERROR	"iconv: 0791-009 I/O error\n"
M_USAGE		"Usage: iconv -f FromCode -t ToCode [FileName...]\n" 
$
$	The following set is used from a shell script that is part of
$	SMIT - Manage Language Environment - Convert Flat files.
$	This set number and the associated messages are hard coded and should
$	not be changed ( any messages with mnemonics should either be added to
$	the previous set, or to a new set which follows set number 2 ).
$
$set 2
$quote "
1    "Cannot read %s.\n"
2    "%s must be a directory.\n"
3    "Cannot make directory %s.\n"
4    "Cannot write to %s.\n"
5    "The directory %1$s/%2$s was not converted.\n"
6    "The file %1$s/%2$s was not converted.\n"
