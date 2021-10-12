$ @(#)31	1.10.1.3  src/bos/usr/bin/iostat/iostat.msg, cmdstat, bos411, 9428A410j 4/15/94 13:12:30
$
$ COMPONENT_NAME: (CMDSTAT) iostat
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ messages for command iostat
$
$ This command is used to monitor system activity. It will be used by
$ highly technical system people
$quote "
$set MS_IOSTAT
$ Do not translate iostat


USAGE            "Usage: iostat [-t] [-d] [Drives] [Interval [Count]] \n\
\t -t specifies tty/cpu report only \n\
\t -d specifies drive report only \n\
\t -t and -d cannot both be specified \n"

IOSTE01          "iostat: 0551-150 There is a sysconfig system error.\n"
$ Do not translate sysconfig

IOSTE02          "iostat: 0551-151 There is a /dev/kmem system error.\n"
$ Do not translate /dev/kmem

IOSTE03          "iostat: 0551-152 There is a knlist system error.\n"
$ Do not translate knlist

IOSTE04          "iostat: 0551-153 Permission is denied.\n"

IOSTE05          "iostat: 0551-154 Disk %s is not found.\n "

HISINV           "\t\t\" Disk history since boot not available. \"\n\n"

$ 
$ WARNING: Be careful translating the following messages because they
$          are column aligned in the output from iostat as follows:
$ 
$ tty:      tin         tout   avg-cpu:  % user    % sys     % idle    % iowait
$           8.5        197.0              10.7      9.8       69.7       9.8 
$ 
$ Disks:        % tm_act     Kbps      tps    Kb_read   Kb_wrtn
$ hdisk0          12.4      42.8       6.9     443768    121996
$ hdisk1           4.1      10.7       2.4      68115     73008
$ 
TTYINOUT         "  tin         tout   "

CPUTIME          "%% user    %% sys     %% idle    %% iowait\n"

TTY              "tty:    "

CPU              "avg-cpu:  "
$ "avg-cpu" stands for "Average CPU Time".

DISKS            "Disks:        "

NOT USED           "%% tm_act     Kbps      tps    msps  "

DINFO2           " Kb_read   Kb_wrtn\n"

DINFO1           "%% tm_act     Kbps      tps   "
