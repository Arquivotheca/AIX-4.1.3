$ @(#)04    1.7.1.1 src/bos/usr/bin/uname/uname.msg, cmdstat, bos411, 9428A410j 4/1/92 10:35:49
$
$ COMPONENT_NAME: (CMDSTAT) status
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

$set MS_UNAME

$ Do not translate "uname:"

USAGE           "Usage: uname [-%s]\n"
UNAMEFAILD      "uname: 0551-027 The uname system call failed."
$ This error message should not occur in a production machine.
$ if it does, follow normal reporting procedures for problems

PERMS           "uname: 0551-028 Must be root to use -S flag.\n"

TOOBIG          "uname: 0551-029 The node name cannot be greater than %d bytes.\n"
MALFAIL         "uname: 0551-030 There is not enough memory available now.\n"
STRFAIL         "uname: 0551-031 The copy to setuname failed.\n"
$ Do not translate setuname
SONLY           "uname: 0551-032 The -S flag cannot be used with any other flags.\n"
FAILU           "uname: 0551-033 The failed error is %d.\n"
UNAMEXFAILD     "uname: 0551-026 The unamex system call failed."
$ Do not translate unamex
PERMST           "uname: 0551-034 Must be root to use -S or -T flag.\n"
TOOBIGS          "uname: 0551-035 The system name cannot be greater than %d bytes.\n"
STONLY           "uname: 0551-036 The -S or -T flags cannot be used with any other flags.\n"
