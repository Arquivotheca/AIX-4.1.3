$ @(#)93        1.7  src/bos/usr/bin/leave/leave.msg, cmdmisc, bos411, 9428A410j 3/5/93 15:40:25
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
$ messages for leave  
$quote "         define quote character.
$len 
$set MS_LEAVE        Define initial set#
$ Message numbers have been assigned a range of 144-150
$ Do not translate leave:, %d, %s, %1$d, %1$s, etc ...
WHEN                 "At what time do you need to leave?\n"
$ this message is when leave prompts for how long till the user needs to leave.
$ it is not displayed if a time is given on the command line.  
USAGE                "Usage: leave [[+]hhmm]\n"
$ usage message
FIVE                 "\7\7\7You have to leave in 5 minutes!\n"
$ this message is given five minutes before the specified time  
ONE                  "\7\7\7Just one more minute!\n"
$ this message is given one minute before the specified time  
TIME                 "\7\7\7It is time to leave!\n"
$ this message is given at the time to leave   
LATE                 "\7\7\7You are going to be late!\n"
$ this message is given once every minute after the time to leave  
BYE                  "This is the last notice! Bye.\n"
$ this message is given after the last notice that it is past time to leave
SET                  "The alarm is set for %s.\n"
