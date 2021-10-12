$  @(#)79  1.5  src/bos/usr/bin/mkusrprof/mkusrprof.msg, rcs, bos411, 9428A410j 6/30/93 18:09:09
$ 
$    COMPONENT_NAME: RCS
$ 
$    FUNCTIONS: none
$ 
$    ORIGINS: 27
$ 
$    IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$    combined with the aggregated modules for this product)
$                     SOURCE MATERIALS
$ 
$    (C) COPYRIGHT International Business Machines Corp. 1993
$    All Rights Reserved
$    US Government Users Restricted Rights - Use, duplication or
$    disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$   

$ scmc msg file created Sun May  2 13:54:49 1993
$ user:  elaird

$quote "         define quote character.

$len 
$set MS_mkusrprof        Define initial set#

M_MSG_1              "Usage: mkusrprof -m l\n\
\t\t\-n Name\n\
\t\t\-t Connection_Type\n\
\t\t\-d Connection_Description\n\
\t\t\-e telnet { -s Server_Name } | -e hcon { -h HCON_Sessions } |\n\
\t\t\-r RecordType|\n\
Usage: mkusrprof -m s\n\
\t\t\-n Name\n\
\t\t\-i { motif | ascii }\n\
\t\t\-t Connection_Type\n\
\t\t\-d Connection_Description\n\
\t\t\-u UserID\n\
\t\t\-s Server_Name\n\
\t\t\-c [ CustomerID ]\n\
\t\t\-p [ Password ]\n\
\t\t\-r RecordType\n"
$ 
$  Don't translate the word mkusrprof (the command) in usage 
$ 
M_MSG_2              "User name was not found in the password file.\n"
M_MSG_3              "You are not permitted to create this profile;\n\
check ownership.\n"
M_MSG_5              "This user type profile has already been defined.\n"
M_MSG_6              "Unable to obtain a lock on the odm database.\n"
M_MSG_7              "Retrieval of error message failed.\n"
M_MSG_8              "Error adding %s;\nCharacter : is invalid.\n"
