$ @(#)31 1.2.1.1 src/bos/usr/bin/smit/odm_msg/sm_rds.msg, cmdcntl, bos411, 9431A411a 7/28/94 11:18:23

$ COMPONENT_NAME: CFGMETHODS
$ 
$ FUNCTIONS:  Message catalog for SMIT Removable Disk Management
$ 
$ ORIGINS: 83 
$ 
$ (C) COPYRIGHT International Business Machines Corp.1993
$ All Rights Reserved
$
$ LEVEL 1, 5 Years Bull Confidential Information
$
$ 

$ Warning -- do not remove the "$quote ~" line below.
$            It is needed to keep gencat from failing when this
$            set is merged with other sets that use other quote characters.


$ The following messages are for dialogues
$ associated with Removable Disk Management

$ messages : set 46

$set 46
$quote ~

5 List All Mounted File Systems on a Disk
10 DISK Name
15 Unmount File Systems on a Disk
20 Remove a Disk with Data
25 Select a DISK
30 VOLUME GROUP Name
35 CABINET Number
40 DISK Number
$ message 40  is no longer in use
45 KEEP definition in database?
50 yes,no
55 POWER OFF the Disk?
$ message 55 is no longer in use
60 Remove a Disk without Data
65 FORCE deallocation of all partitions  on\n  this physical volume?
70 no,yes
75 Remove a Disk
80 Power off a Disk
$ message 80 is no longer in use
85 Power on a Disk
$ message 85 is no longer in use

$ messages in shell scripts
100 file systems mounted on %s are :\n
105 file system %s unmounted.\n
110 Cannot umount file system %s.\n
115 The volume group %s is varied off.\n
120 The volume group %s is exported.\n'
125 The disk %s in the cabinet %s is powered off.\n
$ message 125 is no longer in use,  new message is 300
130 Cannot power off the disk %s in the cabinet %s.\n
$ message 130 is no longer in use
135 The volume group %s is reduced.\n
140 The disk %s in the cabinet %s is powered on.\n
$ message 140 is no longer in use
145 Cannot power on the disk %s in the cabinet %s.\n
$ message 145 is no longer in use

$ menu messages
200 Removable Disk Management
205 List All Mounted File Systems on a Disk
210 Unmount File Systems on a Disk
215 Remove a Disk from the Operating System
220 Remove a Disk
225 Power off a Disk
$ message 225 is no longer in use
230 Power on a Disk
$ message 230 is no longer in use
235 Remove a Disk with Data
240 Remove a Disk without Data

$ new messages
300 Before physically remove the disk plugged on slot %s on bus %s in cabinet %s,\n verify that the disk light is off.\n
$ message 300 replace message 125
310 BUS ID
320 CONNECTION Address
