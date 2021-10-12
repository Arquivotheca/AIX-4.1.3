$ @(#)47	1.1.1.1  src/bos/usr/bin/scls/scls.msg, cmdpse, bos411, 9428A410j 3/24/93 10:18:58
$
$ COMPONENT_NAME: (CMDPSE) STREAMS commands
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ Messages for scls.
$ Do not translate scls.
$ Message numbers have been assigned a range of XXX - YYY
$
$quote "		define quote character.
$set MS_SCLS		Define initial set#

USAGE1	"[-l] [-c] [-m sc_module_name] [modules_to_list]"
MSG1	"couldn't open '%s', errno == %d\n"
MSG2	"couldn't push module '%s', errno == %d\n"
MSG3	"couldn't retrieve module information\n"
MSG4	"couldn't allocate memory\n"
MSG5	"%s not found\n"
$ DO NOT TRANSLATE NEXT TWO LINES
MSG6	"name    type maj put-cntsrv-cnt   open-cnt  close-cnt admin-cnt\n"
MSG7	"name     type  major  idnum  idname   minpsz  maxpsz  lowat   hiwat\n"
MSG8	"no information available\n"

MSG11	"scls: couldn't open '%s', errno == %d\n"
MSG12	"scls: couldn't push module '%s', errno == %d\n"
MSG13	"scls: couldn't retrieve module information\n"
MSG14	"scls: couldn't allocate memory\n"
MSG15	"scls: %s not found\n"
$ DO NOT TRANSLATE NEXT TWO LINES
MSG16	"name    type   major  put-cnt  srv-cnt  open-cnt  close-cnt admin-cnt\n"
MSG17	"name     type  major  idnum  idname   minpsz  maxpsz  lowat   hiwat\n"
MSG18	"scls: no information available\n"
