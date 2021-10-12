$ @(#)67        1.4.2.1  src/bos/usr/bin/license/license.msg, cmdlicense, bos411, 9430C411a 7/21/94 17:00:48
$
$  COMPONENT_NAME: CMDLICE - Message Catalog for License commands
$
$  FUNCTIONS:
$
$  ORIGIN: 27
$
$  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$  combined with the aggregated modules for this product)
$                   SOURCE MATERIALS
$  (C) COPYRIGHT International Business Machines Corp. 1989,1994
$  All Rights Reserved
$
$  US Government Users Restricted Rights - Use, duplication or
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$       This file contains the corresponding messages for the errors generated
$       during the chlicense/lslicense command.
$ Do not translate any of the %$s or %$d.
$ Most of the commands have a '%s:' at the beginning of the message.  This
$ refers to the command name.

$set 1          This is the set of messages for the chlicense command.
$quote "

1     "Usage: chlicense -uLicenses -fStatus\n\
\tChanges the number of fixed licenses and the floating licensing status.\n"
$ The correct syntax for the chlicense command.  Do not translate "-u". 
$ Do not translate "-f". 
$  ***  There are 2 types of user licensing: fixed and floating.  ***
$  ***  Fixed licensing is always enabled and the number can be changed. ***
$  ***  Floating licensing can be enabled or disabled (on or off). ***

2     "3015-100 %s: The number of fixed licenses must be an integer\n\
\tbetween 1 and 32767.\n"
$ The user entered an invalid number.  

3     "3015-101 %s: The floating licensing status must be either \"on\" or \"off\".\n"
$ Do not translate "on" or "off".

4     "%s: The system must be rebooted before the new number of\n\
\t   fixed licenses will take effect.\n"


$set 2         This is the set of messages for the lslicense command.

1     "Usage: lslicense [-c]\n\
\tDisplays the number of fixed licenses and the status of floating licensing.\n"
$ The correct syntax for the lslicense command.  Do not translate -c.


$set 3         This is the set of messages for the license routines

1     "3015-301 %1$s: Cannot open the file %2$s\n\
\tfor read access. Check the permissions on the file and try again.\n"
$ Could not open the file because of the file permissions. The %2$s refers to
$ a filename.

2     "3015-302 %1$s: Cannot create the file %2$s.\n\
\tCheck the permissions on the directory and try again.\n"
$ Could not create the login.cfg file.  The %2$s refers to a filename.

3     "3015-303 %1$s: Cannot open the file %2$s for\n\
\tread/write access. Check the permissions on the file and try again.\n"
$ The file could not be opened for read/write in the chlicense command.
$ Thhe %2$s refers to a filename.

4     "3015-304 %1$s: Could not retrieve information for the file\n\
\t%2$s.  Make sure the file is accessable and try again.\n"
$ The stat() call was unsuccessful.  The %2$s refers to a filename.

5     "3015-305 %1$s: Could not allocate %2$d bytes of storage.\n\
\tTry again later or contact the systems administrator.\n"
$ The malloc() call failed.  The %2$d refers to an number.

6     "3015-306 %1$s: Could not read all of the file %2$s.\n\
\tMake sure there are no locks on the file and try again.\n"
$ The read() call was unsucessful.  The %2$s refers to a filename.

7     "3015-307 %1$s: Error in the file %2$s.\n\
\tThe %3$s entry must have an = symbol.\n\
\tContact the systems administrator.\n"
$ Could not find the '=' after the 'maxlogins' value.  The %2$s refers
$ to a filename, The %3$s refers to an entry name

8     "3015-308 %1$s: Could not set the file pointer in the file\n\
\t%2$s. Contact the systems administrator with\n\
\terrno %3$d.\n"
$ The lseek() function failed.  The %2$s refers to a filename.
$ The %3$d refers to a number.  Do not translate 'errno'.

9     "3015-309 %1$s: Could not update the file %2$s.\n\
\tContact the systems administrator with errno %3$d.\n"
$ The write() function failed.  The %2$s refers to a filename.  The
$ %3$d refers to a number.  Do not translate 'errno'

10    "3015-310 %1$s: Could not find the %2$s entry in the\n\
\tfile %3$s. Use the command chlicense to set\n\
\tthe number of users.\n"
$ The 'maxlogins' entry was not in the stanza.  The %2$s refers to an
$ entry name.  The %3$s refers to a filename.

11    "3015-311 %1$s: Could not find the %2$s value in the\n\
\tfile %3$s. Run the command chlicense to correctly\n\
\tset the number of users.\n"
$ The value for maxlogins could not be found.  The %2$s refers to an
$ entry name.  The %3$s refers to a filename.

12    "3015-312 %1$s: The %2$s value which is in the\n\
\t%3$s file is not valid. Set the number of users with\n\
\tthe chlicense command.\n"
$ An invalid number is in the login.cfg file for maxlogins.
$ The %2$s refers to an entry name.  The %3$s refers to a filename

13     "3015-313 %1$s: Could not retrieve the licensing information.\n"
$ An invalid number was returned by the libs.a license count routine.

14     "3015-315 %1$s: Could not update /etc/inittab.\n"
$ Do not translate the file name "etc/inittab".

15     "3015-316 %1$s: Could not enable floating licensing.\n"
$ LICENSE_ENABLE

16     "3015-317 %1$s: Could not disable floating licensing.\n"
$ LICENSE_DISABLE

17     "Floating licensing is enabled.\n"
$ Floating licensing is turned on.

18     "Floating licensing is disabled.\n"
$ Floating licensing is turned off.

19     "Maximum number of fixed licenses is %d.\n"
$ The title for the non-colon output.
