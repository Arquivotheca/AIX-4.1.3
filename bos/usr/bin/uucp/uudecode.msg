$ @(#)94	1.5  src/bos/usr/bin/uucp/uudecode.msg, cmduucp, bos411, 9428A410j 6/16/90 00:02:28
$quote "         define quote character.

$ @(#)uudecode.msg\t1.4  com/cmd/uucp,3.1,9008d 2/2/90 17:05:04  
$   
$ COMPONENT_NAME: UUCP uudecode.msg  
$   
$ FUNCTIONS: paragraph   
$ ORIGINS: 10  26  27   
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989  
$ All Rights Reserved  
$ Licensed Materials - Property of IBM  
$ US Government Users Restricted Rights - Use, duplication or  
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.  
$ messages for uudecode  
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
$ The range of message numbers assigned to this file is 239-248
$ Do not translate uudecode:
$ len 
$set MS_UUDECODE        Define initial set#

USAGE                "Usage:  uudecode [infile]\n"
NO_BEGIN             "uudecode: 0835-239 Cannot find the keyword \"begin\" in file.\n"
$ Do not translate \"begin\". 
BAD_USER             "uudecode: 0835-240 ~user is not known.\n"
$ Do not translate ~user.
NO_USER              "uudecode: 0835-241 There is no user %s.\n"
NO_END               "uudecode: 0835-242 Cannot find the keyword \"end\" in file.\n"
$ Do not translate \"end\". 
TOOSHORT             "uudecode: 0835-243 The file is too short.\n"
