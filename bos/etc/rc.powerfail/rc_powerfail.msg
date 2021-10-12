$ @(#)05  1.6.2.1  src/bos/etc/rc.powerfail/rc_powerfail.msg, cmdoper, bos41J, 9512A_all 3/20/95 09:54:24
$
$ COMPONENT_NAME: (CMDOPER) commands needed for basic system needs  
$
$ FUNCTIONS: rc.powerfail   
$
$ ORIGINS: 27, 83  
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when  
$ combined with the aggregated modules for this product)  
$                  SOURCE MATERIALS  
$
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$ All Rights Reserved  
$
$ US Government Users Restricted Rights - Use, duplication or  
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.  
$
$ LEVEL 1,  5 Years Bull Confidential Information
$
$ messages for the rc.powerfail.sh shell script  
$ Note to translators: Do not add a newline character ( 
$ \ or  
$ is not there already.  
$quote "         define quote character.

$len 
$set 1        Define initial set#
$ Message numbers have been assigned a range of 080-081
$ Do not translate rc.powerfail, %d, %s, %1$d, %1$s, 
$	or any flags eg. -t, -s etc. 

1 "Usage: rc.powerfail [-s] [-h] [-t Minutes]\n"
$ Minutes designates the number of minutes until the system shuts down

2 "rc.powerfail: 0481-080 send_msg error"
$ Do not translate send_msg

3 "rc.powerfail: init has received a SIGPWR signal.\n \
\tThe power status is currently %s.\n \
\tThe system may power down immediately.\n \
\tExecute rc.powerfail -h as the root user for more information.\n"
$ Do Not translate init
$ Do Not translate SIGPWR
$ %s will be a number from 0-15
$ Do not translate rc.powerfail -h

4 "rc.powerfail: init has received a SIGPWR signal.\n \
\tThe power status is currently %s.\n \
\tThis indicates that a fan fault has occurred.\n \
\tThe system will shut down in %s minutes unless the problem is\n \
\tresolved before then.  Execute rc.powerfail -h as the root user \n \
\tfor more information.\n"
$ Do Not translate init
$ Do Not translate SIGPWR
$ The first %s will be a number from 0-15
$ The second %s indicates the number of minutes until system shutdown
$ Do not translate rc.powerfail -h

5 "rc.powerfail: init has received a SIGPWR signal.\n \
\tThe power status is currently %s.\n \
\tThis indicates that the system is now running on battery backup.\n \
\tThe system will shut down in %s minutes unless the problem is \n \
\tresolved before then.  Execute rc.powerfail -h as the root user \n \
\tfor more information.\n"
$ Do Not translate init
$ Do Not translate SIGPWR
$ The first %s will be a number from 0-15
$ The second %s indicates the number of minutes until shutdown 
$ Do not translate rc.powerfail -h

6 "rc.powerfail: init has received a SIGPWR signal.\n \
\tA call to machstat to validate the signal has failed.\n \
\tThe system will automatically shutdown NOW.\n \
\tExecute rc.powerfail -h as the root user for more information.\n"
$ Do Not translate init
$ Do Not translate SIGPWR
$ Do Not translate machstat
$ Do not translate rc.powerfail -h

7 "rc.powerfail: init has received a SIGPWR signal.\n \
\tA call to machstat to validate the signal has returned \n \
\tan undefined value of %s.  No action will be taken.\n \
\tExecute rc.powerfail -h as the root user for more information.\n"
$ Do Not translate init
$ Do Not translate SIGPWR
$ Do Not translate machstat
$ %s will be a number from 10-13
$ Do not translate rc.powerfail -h

8 "rc.powerfail: shutdown failed.\n \
\tYou should manually shutdown the system NOW with shutdown -F.\n"
$ Do not translate shutdown
$ Do not translate shutdown -F

9 "rc.powerfail: halt failed.\n \
\tYou should manually halt the system NOW with halt -q.\n"
$ Do not translate halt -q

10 "rc.powerfail: 0481-081 The time in minutes given with the -t option\n \
\twas invalid.  System shutdown will occur in 15 minutes.\n"

11 "rc.powerfail: The system will shut down in %s minutes.\n \
\tThe system will shut down because either a fan failure has occurred\n \
\tor the system is running on battery backup power.\n"
$ %s will be the number of minutes remaining until system shutdown

12 "rc.powerfail: The power status has changed to %s.\n \
\tSystem shutdown will occur NOW.\n"
$ %s will be a number from 0-15 which is the value of the power status

13 "rc.powerfail: The power status has changed to %s.\n \
\tThe system shutdown has been canceled.\n"
$ %s will be a number from 0-15 which is the value of the power status

14 "rc.powerfail: The power status has changed to %s.\n \
\tThis indicates that another fan has failed or the system is\n \
\tnow running on battery backup.  \n \
\tThe system will shut down as scheduled.\n"
$ %s will be a number from 0-15 which is the value of the power status

15 "rc.powerfail: The time until system shutdown has expired.\n \
\tThe system will be shutting down NOW.\n"

16 "rc.powerfail: The time until system shutdown has expired.\n \
\tYou should shut down the system NOW using shutdown -F.\n"

17 "rc.powerfail: \n \
This command is used to handle power losses or fan failures. \n \
There are several different states that the system can be in when \n \
the signal SIGPWR is received by init.  The action taken will be \n \
determined by the value of the power status.  The following table \n \
shows the values of the power status and action taken. \n \
Power \n \
Status\tIndication \n \
------\t---------------------------------------------------------------- \n \
  0\tSystem is running normally, there is no action taken. \n \
  1\tSystem is running on battery power, shutdown in 15 minutes. \n \
  2\tProgrammed power off, no action from this command. \n \
  3\tManual off button depressed, no action from this command. \n \
  4\tUndefined status, no action for this state. \n \
  5\tThermal high has been detected, system power off in 20 seconds. \n \
  6\tInternal power fail, immediate system power off. \n \
  7\tPower supply overload, immediate system power off. \n \
  8\tLoss of primary power without battery backup, \n \
\t\timmediate system power off. \n \
  9\tLoss of primary power fan, system power off in 20 seconds. \n \
 10\tLoss of secondary fan, system shutdown in 15 minutes. \n \
 11\tLoss of secondary fan, system shutdown in 15 minutes. \n \
 12\tLoss of secondary fan, system shutdown in 15 minutes. \n \
 13\tLoss of secondary fan, system shutdown in 15 minutes. \n \
 14\tUndefined status, no action for this state. \n \
 15\tUndefined status, no action for this state. \n \
255\tERROR with the machstat command, system shutdown starts immediately.\n"
$ Do not translate SIGPWR or machstat
	
18 "rc.powerfail: An internal error has occurred in the rc.powerfail\n \
\tcommand.  If you have received messages that the system is shutting \n \
\tdown, you should execute shutdown -F immediately.\n \
\tYou should also call your Service Representative\n \
\tand report this problem.\n"
$ Do not translate rc.powerfail in either instance.
$ Do not translate shutdown -F

19 "rc.powerfail: An internal error has occurred in the machstat command.\n \
\tThe system will be shut down NOW.\n \
\tYou should also call your Service Representative\n \
\tand report this problem.\n"
$ Do not translate machstat

$set 2        set for fan/power failures#

1 "rc.powerfail: init has received a SIGPWR signal.\n \
\tThe system is now operating with partial backup cooling.\n"
$ message no longer in use; replaced by message 8 in set 2
$ Do not translate rc.powerfail
$ Do Not translate init
$ Do Not translate SIGPWR

2 "rc.powerfail: init has received a SIGPWR signal.\n \
\tThe system is now operating without backup power.\n"
$ Do not translate rc.powerfail
$ Do Not translate init
$ Do Not translate SIGPWR

3 "rc.powerfail: init has received a SIGPWR signal.\n \
\tThe system has no backup cooling .\n \
\tThe system will shut down in %s minutes unless the problem is\n \
\tresolved before then.  Execute rc.powerfail -h as the root user \n \
\tfor more information.\n"
$ Do not translate rc.powerfail
$ Do Not translate init
$ Do Not translate SIGPWR
$ %s indicates the number of minutes until system shutdown

4 "rc.powerfail: init has received a SIGPWR signal.\n \
\tThe system has severe cooling fault.\n \
\tThe system  will shut down NOW.\n"
$ Do not translate rc.powerfail
$ Do Not translate init
$ Do Not translate SIGPWR

5 "rc.powerfail: The power status has changed.\n \
\tThis indicates a very severe fan fault.\n \
\tThe system will shut down as scheduled.\n"
$ Do not translate rc.powerfail

6 "rc.powerfail: The power status has changed.\n \
\tSystem shutdown will occur NOW.\n"
$ Do not translate rc.powerfail

7 "rc.powerfail: The power status has changed.\n \
\tThe system shutdown has been canceled.\n"
$ Do not translate rc.powerfail

8 "rc.powerfail: init has received a SIGPWR signal.\n \
\tThe system is now operating with cooling problem.\n"
$ replace message 1 in set 2
$ Do not translate rc.powerfail
$ Do Not translate init
$ Do Not translate SIGPWR
