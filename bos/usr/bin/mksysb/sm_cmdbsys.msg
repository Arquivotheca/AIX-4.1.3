$  @(#)05  1.9  src/bos/usr/bin/mksysb/sm_cmdbsys.msg, cmdbsys, bos411, 9428A410j 5/25/94 12:53:59
$  COMPONENT_NAME: CMDBSYS
$
$  FUNCTIONS: sm_cmdbsys.cat
$
$  ORIGINS: 27
$
$  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$  combined with the aggregated modules for this product)
$                   SOURCE MATERIALS
$
$  (C) COPYRIGHT International Business Machines Corp. 1993
$  All Rights Reserved
$  US Government Users Restricted Rights - Use, duplication or
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ End of Prolog
$
$ This file contains the corresponding messages for the
$ SMIT Menus for the System Backup routines.
$
$  SMIT Menu Line Items
$set 1
$quote "

1 "System Backup Manager"
2 "Back Up the System"
3 "List Files in a System Image"
4 "Restore Files in a System Image"
5 "List Files in a Volume Group Backup"
6 "Restore Files in a Volume Group Backup"
7 "Save a Volume Group"
$ Message 7 is no longer used it has been replaced by Message 25
8 "Restore a Volume Group"
$ Message 8 is no longer used it has been replaced by Message 26
9 "Number of BLOCKS to write in a single output"
10 "Create MAP files?"
11 "EXCLUDE files?"
12 "Backup DEVICE or FILE"
13 "DEVICE or FILE"
14 "FILES to restore"
15 "   (Leave blank to restore entire archive)"
16 "VOLUME GROUP to back up"
17 "SHRINK the filesystems?"
18 "PHYSICAL VOLUME names"
19 "  WARNING:  Execution of the mksysb command will\n\
	    result in the loss of all material\n\
	    previously stored on the selected\n\
	    output medium. This command backs\n\
	    up only rootvg volume group.\n\
  "
$ Do not translate rootvg or mksysb they refer to a volume group name and
$  program name .

20 "Restore DEVICE or FILE"
$ Do not translate /dev/rmt0 it refers to a device name.
21 "  WARNING:  Execution of the savevg command will\n\
	    result in the loss of all material\n\
	    previously stored on the selected\n\
	    output medium.\n\
  "
$ Do not translate savevg it refers to a program name .

22 "   (Leave blank to use the PHYSICAL VOLUMES listed\n\
    in the vgname.data file in the backup image)"
$ Do not translate vgname.data it refers to a file name.

23 "Number of BLOCKS to read in a single input"

24 "   (Leave blank to use a system default)"

25 "Back Up a Volume Group"

26 "Remake a Volume Group"

27 "Make BOOTABLE backup?\n   (Applies only to tape)"
28 "EXPAND /tmp if needed?\n   (Applies only to bootable tape)"
$ Do not translate /tmp; it refers to a file system name.
