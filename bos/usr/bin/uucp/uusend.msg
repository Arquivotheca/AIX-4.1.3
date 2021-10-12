$ @(#)97	1.5  src/bos/usr/bin/uucp/uusend.msg, cmduucp, bos411, 9428A410j 6/16/90 00:03:41
$quote "         define quote character.

$    
$ COMPONENT_NAME: UUCP uusend.msg   
$    
$ FUNCTIONS: paragraph    
$ ORIGINS: 10  26  27    
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989   
$ All Rights Reserved   
$ Licensed Materials - Property of IBM   
$ US Government Users Restricted Rights - Use, duplication or   
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.   
$ @(#)uusend.msg\t1.4  com/cmd/uucp,3.1,9008d 2/2/90 17:07:21   
$ ORIGIN:  IBM   
$ IBM CONFIDENTIAL   
$ Copyright International Business Machines Corp. 1988   
$ Unpublished Work   
$ All Rights Reserved   
$ Licensed Material - Property of IBM   
$ RESTRICTED RIGHTS LEGEND   
$ Use, Dupilcation of Disclosure by the Government is subject to   
$ restrictions as set forth in paragraph (b)(3)(B) of the Rights in   
$ technical Data and Computer Software clause in DAR 7-104.9(a).   
$ messages for uusend 
$ The range of message numbers assigned to this file is 249-258.
$ Do not translate uusend:
$ len 
$set MS_UUSEND        Define initial set#

NO_APPEND            "uusend: 0835-249 Cannot append to %s.\n"
BADFLAG              "uusend: 0835-250 The flag %s is not known.\n"
USAGE                "Usage: uusend [-m ooo] [-r] -/file system!system!...!remotefile\n"
BADINPUT             "uusend: 0835-251 The input is not valid.\n"
REMOTE               "remote: nextsys='%s', destname='%s', cmd='%s'\n"
$ Do not trnaslate this message.  
BEFORE               "before ~: '%s'\n"
$ Do not translate this message.  
BADUSER              "uusend: 0835-252 The user name is not valid.\n"
NOUSER               "uusend: 0835-253 Ther user %s is not valid.\n"
BADFILE              "uusend: 0835-254 The file %s is not valid.\n"
RPT                  "local, file='%s'"
$ Do not translate this message.  
COUNT                "count %ld bytes\n"
$ Do not translate this message.  
