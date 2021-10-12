$ @(#)31	1.12.2.1  src/bos/usr/bin/smit/odm_msg/piosmit.msg, cmdpios, bos41J, 9512A_all 3/16/95 15:44:15
$
$   COMPONENT_NAME: CMDPIOS
$
$   FUNCTIONS: name\
$
$   ORIGINS: 27
$
$   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$   combined with the aggregated modules for this product)
$                    SOURCE MATERIALS
$
$   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
$   All Rights Reserved
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$set 1
$quote "

1 "Change / Show Remote Spooling Characteristics"
2 "PRINTER"
3 "1 Printer Setup\n"
$ Keep number and space at beginning of line
4 "2 Default Print Job Attributes\n"
$ Keep number and space at beginning of line
5 "3 Accounting File\n"
$ Keep number and space at beginning of line
6 "4 Queuing Discipline\n"
$ Keep number and space at beginning of line
7 "ACCOUNTING FILE name\n  (full path name of an existing file)\n\nNote: The accounting file specified applies to\n      all printers on this print queue."
8 "Queuing DISCIPLINE\n\nNote: The queuing discipline specified applies\n      to all printers on this print queue."
9 "Printer name\n"
10 "Xstation Printers"
11 "Xstation name"
12 "Printer interface"
13 "Port number\n"
14 "Xstation Type"
15 "none,odd,even,stuck odd,stuck even"
18 "Printer Type"
19 "BOOTP/TFTP Server"
20 "1 Make this system a BOOTP/TFTP server\n"
$ Keep number and space at beginning of line
21 "2 Do NOT make this system a BOOTP/TFTP server\n"
$ Keep number and space at beginning of line
22 "   HOSTNAME of the JetDirect Card"
23 "   HARDWARE ADDRESS of the JetDirect Card"
24 "   GATEWAY ADDRESS"
25 "   SUBNET MASK"
26 "Change / Show Printer Attribute Database"
27 "Change / Show Printer Attribute Database (Virtual Printers)"
28 "Change / Show Pre-processing Filters"
29 "Attachment Type"
30 "Add an Additional Printer to an Existing Print Queue"
31 "User Defined Backend"
32 "Printer Interface"
33 "#   Add new printer or select\n"
$ Keep hash sign (#) and spaces at beginning of line
34 "#   existing printer below\n"
$ Keep hash sign (#) and spaces at beginning of line
35 " +      Add NEW Printer\n"
$ Keep plus sign (+) and spacing.
36 "Parent Adapter"
37 "Xstation NAME"
38 "Port Name"
39 "p  Parallel\n"
$ Keep letter p and space at beginning of line and un-translated
40 "s  Serial Port\n"
$ Keep letter s and space at beginning of line and un-translated
41 "s0 Serial Port 0\n"
$ Keep s0 and space at beginning of line and un-translated
42 "s1 Serial Port 1\n"
$ Keep s1 and space at beginning of line and un-translated
43 "s2 Serial Port 2\n"
$ Keep s2 and space at beginning of line and un-translated
44 "s3 Serial Port 3\n"
$ Keep s3 and space at beginning of line and un-translated
45 "Unsupported Xstation type\n"
46 "TTY Name"
47 "File Name"
48 "Name of existing FILE in /dev directory"
49 "Names of NEW print queues to add"
50 "EXISTING print queues for this printer"
51 "Printer connection characteristics"
52 "   PORT number"
53 "   BAUD rate"
54 "   PARITY"
55 "   BITS per character"
56 "   Number of STOP BITS"
57 "   XON-XOFF handshaking?"
58 "   Use DTR communication protocol?"
59 "   TRANSMIT buffer count"
60 "   Type of PARALLEL INTERFACE"
61 "   Printer TIME OUT period (seconds)"
62 "   STATE to be configured at boot time"
63 "\nPrinter connection characteristics"
64 "Name of existing PRINT QUEUE"
65 "Printer LANGUAGE"
66 "Incorrect queue name or incorrect SMIT id in attachment file.\n"
67 "Print File Type"
68 "Print queue"
69 "Number of COPIES"
70 "ERASE the file after spooling?"
71 "SUPPRESS burst page?"
72 "CLASS to print for job classification"
73 "Number of spaces to INDENT"
74 "Page WIDTH"
75 "JOB NAME"
76 "Pre-processing FILTER"
77 "none,pr,ditroff,print control characters,troff,tex (DVI format),plot,raster image,cifplot,FORTRAN"
78 "TITLE STRING for pr filter"
79 "SEND MAIL upon completion of spooling?"
80 "TYPE of file to print"
81 "Place job in 'HELD' state when queued?"
82 "Print queue to remove"
83 "KEEP the local printer device?"
84 "Local printer device"
85 "Usage: piomisc_ext <function-name> [arg1] [arg2] ..."
86 "Change / Show Default Print Job Attributes"
87 "Change / Show Printer Setup"
88 "Name of FILE to print\n"
89 "Type of Remote Printing"
90 "Unrecognized attachment type.\n"
91 "Support for the selected printer attachment type is not available.\nThe following package must be installed:\n"
92 "Support for the selected printer type is not available.\nAt least one of the following package(s) must be installed:\n"
$ Hole for 1 message 
93 "   TYPE of print spooler on remote server"
94 "   Name of QUEUE on remote server"
95 "   HOSTNAME of remote server"
96 "AIX Version 4"
97 "Standard processing\n"
98 "Local filtering before sending to print server\n"
99 "Standard with NFS access to server print queue attributes\n"
100 "\nRemote server characteristics"
101 "   DESCRIPTION of printer on remote server\n"
102 "   Send PASS-THROUGH FLAG to queue\n      on remote server?\n"
103 "Remote Printer Type"
104 "mknfsmnt failed.\nUnable to create queue."
105 "Could not access NFS mounted directory for remote queue.\n"
106 "Usage: piomkjetd <function-name> [arg1] [arg2] ...\n"
107 "WARNING: There is already an entry for %1$s in %2$s!\n"
108 "%s does not exist or does not have write permission!\n"
109 "Add a Standard Remote Print Queue"
110 "Add a Remote Print Queue with NFS Access"
111 "Add a Remote Print Queue with Local Filtering"
112 "Name of new PRINT QUEUE to add"
113 "Host name %s does not exist.  A valid host name is required!\n"
114 "   FLOW CONTROL to be used"
115 "Printer Support Not Installed"
116 "Software to support the printer you selected\nis not installed. Press Enter to install\nprinter support software packages."
117 "There are no TTYs defined in the system!\nAfter you add a TTY for the ASCII terminal to which the printer is connected,\nthen you will be able to add a print queue for the printer.\n"
118 "Printer Attachment Type Support Not Installed"
119 "Software to support the printer attachment type\nyou selected is not installed. Press Enter to\ninstall software support packages for printer\nattachment types (and printers)."
120 "   NETWORK INTERFACE"
121 "Ethernet,Token Ring"
