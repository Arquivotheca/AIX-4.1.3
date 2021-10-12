$ @(#)81 1.4.4.4 src/bos/usr/bin/smit/odm_msg/cfgmeth.msg, cfgmethods, bos411, 9434A411a 8/22/94 11:35:42
$
$  COMPONENT_NAME: (CFGMETHODS) User Interface for Configuration
$
$  FUNCTIONS: Devices SMIT messages
$
$  ORIGINS: 27, 83
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
$ LEVEL 1, 5 Years Bull Confidential Information
$

$quote "
$
$set 27
$
$
$  Commonly Used Messages (1-49)
1 "Parent Adapter"
2 "Parent adapter"
3 "PORT number"
4 "Port number"
5 "CONNECTION address"
6 "Connection address"
7 "Description"
8 "Status"
9 "Location"
10 "KEEP definition in database"
11 "Apply change to DATABASE only"
12 "yes,no"
13 "true,false"
14 "on,off"
15 "Device type"
16 "Device interface"
17 "Device Interface"
18 "Device Class"
19 "Device Type"
20 "Defined Devices"
21 "available,defined"
22 "NOTE: A selection of \"none\" configures devices\n\
added after IPL without the installation of \n\
software"
$
$  Printer/Plotter messages (50-99)
50 "Printer/Plotter"
51 "Printer/Plotter Type"
52 "Printer/Plotter type"
53 "Printer/Plotter interface"
54 "\n\
Ignore the following attributes when using\n\
the Print Queue:\n"
55 "none,even,odd"
56 "standard,converged"
57 "Printer/Plotter Interface"
$
$  TTY messages (100-124)
100 "TTY"
101 "TTY Type"
102 "TTY type"
103 "TTY interface"
104 "none,even,odd,mark,space"
105 "enable,disable,share,delay"
106 "\n\
POSIX special control characters:\n"
$
$  ACPA messages (125-149)
125 "ACPA Adapter"
$
$  messages 150-174 available
$
$  messages 175-199 available
$
$  X.25 messages (200-299)  (200-249 available -- obsolete in V4)
250 "X.25 CoProcessor/2 or Multiport/2 Adapter"
251 "List All X.25 CoProcessor/2 or Multiport/2 Adapters"
252 "Manage Device Drivers for X.25 CoProcessor/2 or Multiport/2 Adapters"
258 "Trace an X.25 CoProcessor/2 or Multiport/2 Adapter"
$
$  messages 300-399 available
$
$  Disk messages (400-424)
400 "Disk"
401 "Disk Type"
402 "Disk type"
403 "Disk interface"
404 "Cannot add this type of disk."
405 "Physical volume IDENTIFIER"
406 "ASSIGN physical volume identifier"
407 "Parent Controller"
$
$  CD-ROM messages (425-434)
425 "CD ROM Drive"
426 "CD ROM Drive Type"
427 "CD ROM Drive type"
428 "CD ROM Drive interface"

$  R/W Optical drive  messages (435-449)
435 "R/W Optical Drive" 
436 "R/W Optical Drive Type"
437 "R/W Optical Drive type"
438 "R/W Optical Drive interface"
$
$  Diskette messages (450-474)
450 "Diskette Drive"
451 "3.5inch,5.25inch,3.5inch4Mb"
$
$  Tape messages (475-499)
475 "Tape Drive"
476 "Tape Drive Type"
477 "Tape Drive type"
478 "Tape Drive interface"
$
$  Ethernet adapter messages (500-524)
500 "Ethernet Adapter"
501 "bnc,dix"
$
$  Token Ring adapter messages (525-549)
525 "Token Ring Adapter"
$
$  Multiprotocol messages (550-574)
550 "Multiprotocol Port"
551 "List All Portmaster Adapter/A Adapters"
552 "Manage Device Drivers for Portmaster Adapter/A"
553 "Manage 4-Port Multiprotocol Device Driver"
554 "Add a Device Driver to a Portmaster Adapter/A"
555 "Remove a Device Driver"
556 "Configure a Defined Device Driver"
557 "Manage Ports"
558 "Trace a Portmaster Adapter/A"
559 "Portmaster Adapter/A"
$
$  3270 adapter messages (575-599)
575 "3270 Connection Adapter"
576 " \n\
For each of the following printer addresses,\n\
a value of 0x0100 indicates no printer.\n"
$
$  5085/86/88 Attachment adapter messages (600-624)
600 "5085/86/88 Attachment Adapter"
601 "5085/86/88 Mode"
602 "5085/86/88 mode"
$
$ Dials/LPFKeys messages (625-649)
625 "Dials/LPFKeys"
626 "Dials/LPFKeys Type"
627 "Dials/LPFKeys interface"
$
$ System object (sys0) messages (650-674)
650 "Change / Show Characteristics of Operating System"
$
$ Serial Optical Link messages (675-699)
675 "Serial Optical Link"
676 "Complete Initial Configuration for the Serial Optical Link"
677 "Change / Show Characteristics of the Serial Optical Link"
678 "List All Serial Optical Channel Converter Adapters"
679 "Trace an Optical Port Device"
$
$ SCSI Initiator Device messages (700-724)
700 "SCSI Initiator Device Type"
701 "SCSI Initiator Device type"
$
$ SMIT Messages for ISA Adapters (725-749)
725 "ISA Adapter"
726 "ISA Adapters"
727 "Install ISA Adapter Software"
728 "Add an ISA Adapter"
729 "Change / Show Characteristics of an ISA Adapter"
730 "Remove an ISA Adapter"
731 "List all Defined ISA Adapters"
732 "List all Supported ISA Adapters"
733 "List all ISA Adapters"
734 "Parent Device"
$
$  SMIT Messages for Token Ring ISA Adapters (750-774)
750 "Add a Token Ring ISA Adapter"
751 "Change / Show Characteristics of a Token Ring ISA Adapter"
$
$  SMIT Messages for Ethernet ISA Adapters (775-799)
775 "Add an Ethernet ISA Adapter"
776 "Change / Show Characteristics of an Ethernet ISA Adapter"
$
$  SMIT Messages for X.25 CoProcessor/1 Adapters (800-824)
800 "X.25 CoProcessor/1 Adapter"
801 "List All X.25 CoProcessor/1 Adapters"
802 "Manage Device Drivers for X.25 CoProcessor/1 Adapters"
803 "Trace an X.25 CoProcessor/1 Adapter"
804 "Add a X.25 CoProcessor/1 Adapter"
805 "Change / Show Characteristics of a X.25 CoProcessor/1 Adapter"
806 "Remove a X.25 CoProcessor/1 Adapter"
807 "X25 Adapter Type"
808 "Parent Bus"
809 "ISA Virtual Connection"
810 "ISA Interrupt Level"
811 "Interrupt Priority"
812 "Bus IO Address"
813 "Bus Memory Address"
814 "Window Size"
815 "Configure a Defined X25 Adapter"
$
$ SMIT Messages for PCMCIA Cards (825-849)
825 "PCMCIA Card"
826 "PCMCIA Cards"
827 "Install PCMCIA Card Software"
828 "Add a PCMCIA Card"
829 "Change / Show Characteristics of a PCMCIA Card"
830 "Remove a PCMCIA Card"
831 "List all Defined PCMCIA Cards"
832 "List all Supported PCMCIA Cards"
833 "List all PCMCIA Cards"
834 "Parent Device"
836 "Slot number"
837 "Configure all PCMCIA Cards Inserted after IPL"
838 "Configure a Defined PCMCIA Card"
$ The following message is used to specify a new slot number on re-configuring
$ a PCMCIA card.
839 "Same,0,1,2,3"
$
$  SMIT Messages for Token Ring PCMCIA Cards (850-874)
850 "Add a Token Ring PCMCIA Card"
851 "Change / Show Characteristics of a Token Ring PCMCIA Card"
$
$  SMIT Messages for Ethernet PCMCIA Cards (875-899)
875 "Add an Ethernet PCMCIA Card"
876 "Change / Show Characteristics of an Ethernet PCMCIA Card"
$
$  SMIT Messages for Fax/Modem PCMCIA Cards (900-924)
900 "Add a Fax/Modem PCMCIA Card"
901 "Change / Show Characteristics of a Fax/Modem PCMCIA Card"
$
