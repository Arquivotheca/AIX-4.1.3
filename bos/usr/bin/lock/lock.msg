$ @(#)91	1.5  src/bos/usr/bin/lock/lock.msg, cmdmisc, bos411, 9428A410j 1/22/93 09:39:47
$
$ COMPONENT_NAME: (CMDMISC) miscellaneous commands
$
$ FUNCTIONS:   
$
$ ORIGINS: 27  
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when  
$ combined with the aggregated modules for this product)  
$                  SOURCE MATERIALS  
$
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$ All Rights Reserved  
$
$ US Government Users Restricted Rights - Use, duplication or  
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.  
$ messages for lock  
$quote "         define quote character.

$len 
$set MS_LOCK        Define initial set#
$ Message numbers have been assigned a range of 160-165 
$ Do not translate lock:, %d, %s, %1$d, %1$s, etc ...

LNOTTERM             "lock: 0506-160 This command runs only on a display device.\n"
$ lock was not run on a terminal.  Error recovery is ask user to re-run  
$ command on a terminal.  

LKEY                 "Type the password or key:\n"
$ Do not translate key. 
$ the "key" message asks the user for the password to lock the   
$ terminal with.  

LAGAIN               "Type the password or key again.\n"
$ Do not translate key. 
$ the "Again" message re-asks the user for the key that he/she just typed  
$ in  to make sure that they typed the right thing.  

LWHENTIMEOUT         "lock: Session for %1$s on %2$s will timeout in %3$d minutes.\n"
$ the parameters are:   tty  host           # of minutes till timeout  

LTIMENOWIS           "The current time is %s.\n"

LUSAGE               "Usage: lock [-Timeout]\n"
$ standard useage message: 'timeout' is the number of minutes to wait   
$ before timing out.  

LLOCKTIMOUT          "lock: A timeout occurred.\n"
$ this message is printed when lock's input times out.  

LTYPINULK            "lock: Type in the unlock password or key. Timeout will occur in %d minutes.\n"
$ %d is a number of minutes till timeout.  
