$  @(#)77  1.14.3.1  src/bos/usr/bin/mksysb/mksysb.msg, cmdbsys, bos41J, 9512A_all 3/17/95 12:47:52
$  COMPONENT_NAME: CMDBSYS
$
$  FUNCTIONS: mksysb.cat
$
$  ORIGINS: 27
$
$  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$  combined with the aggregated modules for this product)
$                   SOURCE MATERIALS
$
$  (C) COPYRIGHT International Business Machines Corp. 1989, 1995
$  All Rights Reserved
$  US Government Users Restricted Rights - Use, duplication or
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ End of Prolog
$
$       This file contains the corresponding messages for the errors generated
$       during the System Backup routines.  These messages will only be used
$       by these shell scripts: mksysb, mkszfile, mkvgdata, savevg, and restvg.
$
$ messages used mostly by mksysb/savevg cmds
$set 1
$quote "

1 "0512-004 %s: The %s file does not exist.  Backup canceled."
$ Do not translate %s.  It refers to a program name.

2 "0512-005 %s: Backup Completed.\n\
\tThe backup command completed with errors.\n\
\tThe messages displayed on Standard Error contained additional\n\
\tinformation."
$ Do not translate %s.  It refers to a program name.
$ Do not translate "backup" (Note Lower case b).  It refers to a program name

3 "0512-006 %s: Backup device or file name must start with a '/'."
$ Do not translate %s.  It refers to a program name.

4 "0512-007 %s: Abnormal program termination."
$ Do not translate %s.  It refers to a program name.

5 "0512-008 %s: The %s command failed.  Backup canceled."
$ Do not translate %s's they refer to a program name.

6 "0512-009 %s: Invalid or missing Volume Group Name.\n"
$ Do not translate %s's they refer to a program name.

7 "0512-010 %s: Creation of backup list failed"
$ Do not translate %s.  It refers to a program name.

8 "\nUsage:\t%s [-i] [-m] [-e] [-b blocks] device\n\n\
-i\tCreate the /image.data file.\n\
-m\tCreate the /image.data file and physical partition maps\n\
-e\tExclude the files/directories\n\
-b blocks\tNumber of 512-byte blocks to write in a single\n\
	 \toutput operation\n\
device\tName of device to receive the backup information.\n\
      \tExample: %s /dev/rmt0\n"
$ Do not translate %s's they refer to a program name.
$ Do not translate "/image.data", It refers to a Data file name.
$ Do not translate the -i -m -e -f or /dev/rmt0, they refer to
$ command parameters

9 "\nUsage:\t%s [-i] [-m] [-e] [-b blocks] [-f device] vgName\n\n\
-i\tCreate the <vgname>.data file.\n\
-m\tCreate the <vgname>.data file and physical partition maps\n\
-e\tExclude the files/directories listed in /etc/exclude.<vgname>\n\
-b blocks\tNumber of 512-byte blocks to write in a single\n\
	 \toutput operation\n\
-f device\tName of device to receive the backup information.\n\
	 \tDefault is /dev/rmt0\n\
vgName\tName of Volume Group to backup.\n"
$ Do not translate %s's they refer to a program name.
$ Do not translate ".data", It is a prefix to a Data file name.
$ Do not translate the -i -m -e -f or /dev/rmt0, they refer to
$ command parameters

10 "0512-015 %s: %s appears to be a member of a volume group.\n\tBackup canceled."
$ Do not translate %s's they refer to a program name.

17 "0512-017 %s: Cannot write to the device %s.\n\t\t Either write protected or in use."
$ Do not translate %s's they refer to a program name and tape device name.

18 "0512-016 %s: Attempt to create a bootable tape failed:\n\tbosboot -d %s -a failed with return code %s."
$ Do not translate %s's they refer to a program name.
$ Do not translate "bosboot", It refers to a program name.
$ Do not translate the -d -a , they refer to  command parameters

19 "0512-021 %s: Attempt to create a bootable tape failed:\n\tmkinsttape %s failed with return code %s."
$ Do not translate %s's they refer to a program name.
$ Do not translate "mkinsttape", It refers to a program name.

20 "0512-038 %s: Backup Completed Successfully.\n"
$ Do not translate %s's they refer to a program name.

21 "0512-039 %1$s: WARNING: %2$s does not appear to be a tape device and\n\
\twill NOT have a bootable image.\n"
$ Do not translate %1$s, it refers to a program name.
$ Do not translate %2$s, it refers to a Device or File name.

22 "0512-040 %1$s: WARNING: %2$s does not appear to be a tape device and\n\
\tdoes NOT have a bootable image.\n"
$ Do not translate %1$s, it refers to a program name.
$ Do not translate %2$s, it refers to a Device or File name.

23 "0512-041 %s: The /tmp file system could not be expanded.\n"
$ Do not translate $s, it refers to a program name.
$ Do not translate /tmp; it is the name of a file system.

24 "0512-042 %s: The backup could not continue because the /tmp file\n\
\tsystem needs an additional %s Kbytes of free space to make\n\
\tthe boot image. To make a system backup image, do one of\n\
\tthe following:\n\
\t  (1) Change your answer on the dialog screen to allow the\n\
\t      /tmp file system to be expanded, or\n\
\t  (2) Change your answer on the dialog screen to allow a\n\
\t      nonbootable backup media.\n"
$ Do not translate $s, it refers to a program name.
$ Do not translate /tmp; it is the name of a file system.
$ Kbytes means 1024 bytes.

25 "0512-043 %s: WARNING: This system does not support booting\n\
\tfrom a tape device; therefore, the mksysb tape does not contain a\n\
\tbootable image.  To install the mksysb image you will need to boot\n\
\tyour system from a bootable media.\n"
$ Do not translate $s, it refers to a program name.

26 "Checking for a valid device.\n"

27 "Getting the device type.\n"

28 "Calculating space requirements.\n"

29 "Expanding /tmp.\n"

30 "Backing up the system...\n"

31 "0512-003 %s may not have been able to archive some files.\nThe messages displayed on the Standard Error contained additional\ninformation.\n"
$ Do not translate $s, it refers to a program name.

$ Additional messages used by mkszfile/mkvgdata cmds
$set 2

$ Messages 512-012, 512-013, 512-014 are no longer used and were deleted.

4 "\nUsage:  %s [-m]\n\n\
-m\tGenerate Physical Partition maps\n"
$ Do not translate %s.  It refers to a program name.

5 "\nUsage:  %s [-m] vgname\n\n\
-m\tGenerate Physical Partition maps\n\
vgname\tName of volume group to backup.\n\n"
$ Do not translate %s.  It refers to a program name.

18 "0512-018 %s: The backup could not continue because the %s Kbytes\n\
of free work space on the / filesystem that are necessary\n\
were not found.  Before you reissue this command, you\n\
should extend the / filesystem (chfs) or free up space to provide a\n\
total of %s Kbytes of free work space in the / filesystem.\n\n"
$ Do not translate %s.  It refers to a program name or a number parameter.

20 "0512-020 %s: The logical volume %s does not have an entry in\n\
/etc/filesystems which is needed to accurately define the mount point.\n\
As a result, it will not be backed up.  If you want it to be included\n\
in the backup, add a stanza for it in the /etc/filesystems file and\n\
reissue this command before running mksysb.\n\n"

22 "0512-022 %s: An Error occurred creating the Image Data File or\n\
the Physical Partition Map Files. Make sure you have write permission\n\
to the /tmp and / directories.\n\n"
$ Do not translate %s.  It refers to a program name.

23 "0512-023 %s: The backup could not continue because the %s KBytes\n\
of free work space on the /tmp filesystem that are necessary\n\
were not found.  Before you reissue this command, you\n\
should extend the /tmp filesystem (chfs) or free up space to provide a\n\
total of %s Kbytes of free work space in the /tmp filesystem.\n\n"

$ Additional messages used by restvg cmd
$set 3

1 "\nUsage:  %s [-q] [-s] [-b blocks] [-f device] diskname...\n\n\
-q\tdo not prompt before creating the Volume Group\n\
-s\tCreate the Logical Volumes to the Minimum Size as specified\n\
  \tin the vgname.data file.\n\
-b blocks\tNumber of 512-byte blocks to read in a single\n\
	 \tinput operation\n\
-f device\tName of device to restore the information from.\n\
	 \tDefault is /dev/rmt0\n\
diskname..\tlist of target disk devices to overwrite the disks\n\
	  \tspecfied in the vgname.data file.\n\n"
$ Do not translate %s's they refer to a program name.
$ Do not translate the -q -s -f or /dev/rmt0, they refer to
$ command parameters

2 "0512-025 %s: The image data file does not exist.  Restore of Volume Group canceled."
$ Do not translate %s.  It refers to a program name.

3 "0512-026 %s: The %s command cannot restore the %s Volume Group.  Restore of Volume Group canceled."
$ Do not translate %s.  It refers to a program name.

4 "\nWill create the Volume Group:\t%s\n\
Target Disks:\t"
$ Do not translate %s.  It refers to a program name.

5 "Enter 'y' to continue: "
$ Do not translate 'y'.  It refers to a parameter.

6 "0512-027 %s: Need to specify more Disks, need at least %s.  Restore of Volume Group canceled."
$ Do not translate %s.  It refers to a program name.
    
7 "0512-028 %s: One or more Disks are of the wrong size.  Restore of Volume Group canceled."
$ Do not translate %s.  It refers to a program name.

8 "0512-029 %s: There is not enough Room in the specified disks for the volume group.  Restore of Volume Group canceled."
$ Do not translate %s.  It refers to a program name.

9 "0512-030 %s: Unable to varyon the Volume Group %s.  Restore of Volume Group canceled."
$ Do not translate %s.  It refers to a program name.

10 "0512-031 %s: Unable to create the Volume Group %s.  Restore of Volume Group canceled."
$ Do not translate %s.  It refers to a program name.

11 "0512-032 %s: The map file for the Logical Volume %s does not exist or is not readable. Restore of Volume Group canceled."
$ Do not translate %s.  It refers to a program name.

12 "0512-033 %s: Failed creating the Logical Volume %s.  Restore of Volume Group canceled."
$ Do not translate %s.  It refers to a program name.

13 "0512-034 %s: Failed creating the Filesystem %s.  Restore of Volume Group canceled."
$ Do not translate %s.  It refers to a program name.

14 "0512-035 %s: Failed Mounting the Filesystem %s.  Restore of Volume Group canceled."
$ Do not translate %s.  It refers to a program name.

15 "Allocation Policy:\n\
\tShrink Filesystems:\t%s\n\
\tPreserve Physical Partitions for each Logical Volume:\t%s\n\n"
$ Do not translate %s.  It refers to a program name.

16 "0512-036 %s: Target Disk %s is invalid.  Restore of Volume Group canceled."
$ Do not translate %s.  It refers to a program name and a disk name.

17 "0512-037 %s: Target Disk %s Already belongs to a Volume Group.  Restore of Volume Group canceled."
$ Do not translate %s.  It refers to a program name and a disk name.
