$ @(#)76	1.7  src/bos/usr/bin/id/id.msg, cmdsauth, bos411, 9428A410j 5/6/91 15:43:34
$
$ COMPONENT_NAME: (CMDSAUTH) security: authentication functions
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "

$set MS_id

M_SUID	"uid=%u"
$ SGID prints with SUID
M_SGID	" gid=%u"
$ SGID prints with SUID
M_SEUID	" euid=%u"
$ SEGID prints with SEUID
M_SEGID	" egid=%u"
$ SEUID prints with SEGID
$ lines above do not need newline character...it is in the code.
M_NOUSER	"3004-800 Could not get \"user\" information\n"
$ Translate "user" 
M_NOGROUP	"3004-810 Could not get \"group\" information\n"
$ Translate "group" 
M_BADUSER	"3004-820 User not found in /etc/passwd file\n"
$ Do not translate "/etc/passwd" 
M_USE1		"usage: id [user]\n"
M_USE2		"       id  -G [-n] [user]\n"
M_USE3		"       id  -g [-nr] [user]\n"
M_USE4		"       id  -u [-nr] [user]\n"
$ All four lines above are usage messages and are printed together
$ Do not translate any occurance of "id" 
$ Translate "user" 
