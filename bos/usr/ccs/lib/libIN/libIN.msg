$ @(#)02	1.2  src/bos/usr/ccs/lib/libIN/libIN.msg, libIN, bos411, 9428A410j 6/7/91 10:47:04
$
$  COMPONENT_NAME: (LIBIN) INed library functions
$
$  FUNCTIONS: libIN messages
$
$  ORIGINS: 27,10
$
$  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$  combined with the aggregated modules for this product)
$                   SOURCE MATERIALS
$  (C) COPYRIGHT International Business Machines Corp. 1989
$  All Rights Reserved
$
$  US Government Users Restricted Rights - Use, duplication or
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

$quote "
$set MS_LIBIN

$ The following messages are for ARfuncs.c
M_SEEKERROR	"1231-001 AR: unable to seek in archive (?).\n"
$	Note: The file name is not known when this error occurs,
$	hence the use of (?) to denote an unknown name.
M_NAMEERROR	"1231-002 AR: bad member name in archive (?).\n"
$	Note: The file name is not known when this error occurs,
$	hence the use of (?) to denote an unknown name.

$ The following messages are for caller's of ERcmderr.c and ERvcmderr.c
M_ER_S_CREATE	"Cannot create \"%s\"."
M_ER_S_EXEC	"Cannot execute \"%s\"."
M_ER_S_LINK	"Cannot link \"%s\" to \"%s\"."
M_ER_S_MEMORY	"Out of memory."
M_ER_S_OPEN	"Cannot open \"%s\"."
M_ER_S_READ	"Cannot read \"%s\"."
M_ER_S_SEEK	"Cannot seek on \"%s\"."
M_ER_S_STAT	"Cannot get status of \"%s\"."
M_ER_S_UNLINK	"Cannot unlink \"%s\"."
M_ER_S_WRITE	"Cannot write \"%s\"."
$	%s in all the prior messages refer to files

$ The following messages are for ERsysmsg.c
M_ERSYSMSG1	"Error %d"
$	this message returned to caller as a string
$	%d is a numeric errno value
M_ERSYSMSG2	"No system error"
$	this message returned to caller as a string

$ The following message is for FSattr.c and FSdskname.c
M_CANTOPEN	"1231-003 Cannot open %s.\n\
\tCheck path name and permissions.\n"
$	%s is the name of a file (e.g. /etc/filesystems)

$ The following message is for FSdskname.c
M_FSDSK		"1231-004 No device specified for %s.\n"
$	%s is the name of a file system

$ The following messages are for FSgetlabel.c
M_FSLABEL	"unlabeled file system"
$	this message returned to caller as a string
M_FSPACK	"unlabeled pack"
$	this message returned to caller as a string

$ The following messages are for MSfcns.c
M_ALLOC		"1231-005 Memory allocation failed.\n"
M_MSGETBLOCK	"1231-006 Invalid allocation size: %d.\n"
$	%d is the numeric size of the allocation request
