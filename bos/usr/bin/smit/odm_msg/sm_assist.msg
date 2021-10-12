$
$  @(#)21  1.15  src/bos/usr/bin/smit/odm_msg/sm_assist.msg, cmdassist, bos411, 9428A410j 6/24/94 14:14:10
$
$ COMPONENT_NAME:  cmdassist
$ 
$ FUNCTIONS:  Message catalog for cmdassist smit messages
$
$ ORIGINS:  27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$
$ This is the message file for menus associated with the Installation
$ Assistant.  The Installation Assistant menus are called via the
$ install_assist command.  The install_assist command is called via
$ inittab upon reboot after a BOS installation.
$

$ Do not translate the term "rootvg" in the messages below; rootvg
$ is the name of the system volume group.
$ Do not translate "root" in "Set root Password"; root is the name
$ of a user.

$ The term "MB" in the messages below is an abbreviation for
$ megabytes. 

$
$quote "
$set 52
$
1 "Installation Assistant"
2 "Set Date and Time"
3 "Set root Password"
4 "Set Installation Device"
5 "Configure Network Communications"
6 "Mount a Remote File System"
7 "Manage System Storage and Paging Space (rootvg)"
8 "Add/Show Disks for System Storage (rootvg)"
9 "Add/Show Paging Space"
10 "SPACE available in rootvg (MB)"
11 "Space ALLOCATED in rootvg (MB)"
12 "DISKS in rootvg"
13 "Disks to be ADDED"
14 "MEMORY size (MB)"
15 "RECOMMENDED paging space (MB)"
16 "CURRENT paging space (MB)\n"
17 "NEW paging space (MB)"
18 "Create Users"
19 "Create a User"
20 "Set Password"
21 "Define Printers"
22 "Import Existing Volume Groups"
23 "Install Software Applications"
24 "Add License Passwords for Applications"
25 "Set Up for Adding Concurrent-Use and Use-Once Licenses"

$ Message numbers 26, 27, 28 are no longer used.
26 "NODELOCKED licenses to add?"
27 "CONCURRENT / USE-ONCE licenses to add?"
28 "Set Up for Adding License Passwords"

29 "Install a Bundle of Software (Easy Install)"
30 "Tasks Completed - Exit to AIX Login"
31 "Back Up the System"
32 "INPUT device/directory for software"
33 "Set Installation Device"
34 "Manage Language Environment"
35 "Manage Nodelocked Licenses"
36 "Manage Concurrent-Use and Use-Once Licenses"
37 "Manage Vendors"
38 "Import a Volume Group"
39 "Mount a File System"
40 "Install Printer Software"
41 "Add a Print Queue"
42 "TCP/IP Startup"
43 "Add a Hostname to Access Other Systems"
44 "Start NFS"
45 "yes,no"
46 "Press Enter to automatically install the backup command.\nPress F3 to cancel."
47 "Press Enter to automatically install NFS.\nPress F3 to cancel."
48 "Press Enter to automatically install the printer backend.\nPress F3 to cancel."
49 "Press Enter to continue."

$ Message numbers 50, 51, 52, 53 are no longer used.
50 "Set Date and Time (Recommended)"
51 "Set root Password (Recommended)"
52 "Set Installation Device (Recommended)"
53 "Set Up for Adding License Passwords (Recommended)"

54 "Click on this item to automatically install the backup command."
55 "Click on this item to automatically install NFS."
56 "Press Enter to automatically install the printer software.\nPress F3 to cancel."
57 "Click on this item to automatically install the printer software."

58 "Add an ISA Adapter"
59 "Install Selectable Software (Custom Install)"
