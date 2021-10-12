$ @(#)62        1.68.10.8 4/27/95 src/bos/usr/lib/methods/common/devices.msg, cfgmethods, bos41J, 9520A_all 13:02:58
$
$ COMPONENT_NAME: (CFGMETHODS) device configuration methods
$
$ FUNCTIONS: Devices configuration message catalog
$
$ ORIGINS: 27, 83
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1995
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ LEVEL 1, 5 Years Bull Confidential Information
$
$quote "
$
$  System Object and related devices
$set 1
1 "System Object"
2 "System Unit"
3 "CPU Planar"
4 "Floating Point Processor"
5 "I/O Planar"
6 "Memory Card"
7 "Size in Mbytes"
8 "SCSI Device Drawer"
9 "SCSI DASD Drawer"
10 "Asynchronous Expansion Drawer"
11 "CPU Drawer"
12 "Simm type"
13 "Card type"
14 "Error log file size"
15 "Size of data cache in bytes"
16 "Size of instruction cache in bytes"
17 "State of system keylock at boot time"
18 "Maximum number of pages in block I/O BUFFER CACHE"
19 "Maximum Kbytes of real memory allowed for MBUFS"
20 "Maximum number of PROCESSES allowed per user"
21 "Automatically REBOOT system after a crash"
22 "Continuously maintain DISK I/O history"
23 "Amount of usable physical memory in Kbytes"
24 "Primary dump device"
25 "Secondary dump device"
26 "System Console Login "
27 "HIGH water mark for pending write I/Os per file"
28 "LOW water mark for pending write I/Os per file"
29 "Enable memory SCRUBBING"
30 "8 MB Memory Card"
31 "16 MB Memory Card"
32 "32 MB Memory Card"
33 "64 MB Memory Card"
34 "128 MB Memory Card"
35 "256 MB Memory Card"
36 "512 MB Memory Card"
37 "Memory SIMM"
38 "1 MB Memory SIMM"
39 "2 MB Memory SIMM"
40 "4 MB Memory SIMM"
41 "8 MB Memory SIMM"
42 "16 MB Memory SIMM"
43 "32 MB Memory SIMM"
44 "64 MB Memory SIMM"
45 "Enable full CORE dump"
46 "CPU card"
47 "Processor"
48 "Processor state"
49 "Processor type"
50 "Size of L2 cache in Kbytes"
51 "System Planar"
52 "LVM Device Driver"
53 "L2 Cache"
54 "Firmware version and revision levels"
55 "Memory"
56 "Total amount of physical memory in Mbytes"
57 "Amount of usable physical memory in Mbytes"
$
$  Family 2 bus
$set 3
1 "Microchannel Bus"
2 "PCI Bus"
3 "ISA Bus"
$
$  Serial Optical Link
$set 4
1 "Serial Optical Link Port Subsystem"
2 "Serial Optical Link Chip"
3 "Serial Optical Channel Converter"
4 "Serial Optical Link Port"
5 "PROCESSOR ID number for this machine"
6 "RECEIVE queue size"
7 "STATUS BLOCK queue size"
$
$  NIO card
$set 21
1 "Standard I/O Planar"
$
$  SCSI Adapter
$set 22
1 "SCSI I/O Controller"
2 "Bus memory address"
3 "Address of bus memory used for DMA"
4 "Bus interrupt level"
5 "Interrupt priority"
6 "DMA arbitration level"
7 "Adapter card SCSI ID"
8 "Number of COMMANDS per device which may be queued to adapter"
9 "Name of adapter code download file"
10 "BATTERY backed adapter"
11 "Bus I/O address"
12 "DMA bus memory LENGTH"
13 "PERCENTAGE of bus memory DMA area for target mode"
14 "Enable TARGET MODE interface"
15 "Target Mode interface enabled"
17 "External bus SCSI ID"
18 "Internal bus SCSI ID"
19 "Maximum synchronous data transfer rate (mHz)"
20 "Standard SCSI I/O Controller"
22 "Wide SCSI I/O Controller Adapter"
23 "SCSI I/O Controller Protocol Device"
24 "Number of target mode RECEIVE BUFFERS"
25 "Address of bus memory used for target mode"
26 "Target mode bus memory length"
27 "Wide SCSI I/O Controller"
28 "Maximum number of commands to queue to adapter driver"
29 "Protocol device SCSI ID"
30 "Wide bus enabled"
31 "External bus wide enabled"
32 "Internal bus wide enabled"
$
$  GIO adapter
$set 24
1 "Graphics Input Adapter"
2 "Bus I/O address"
3 "Bus interrupt level"
4 "Interrupt priority"
5 "Lighted Program Function Keyboard"
6 "Dials"
7 "STATE to be configured at boot time"
$
$  Async Expansion Adapter
$set 25
1 "Async Expansion Adapter"
2 "Bus I/O address"
$
$  3270 connection adapter
$set 26
1 "3270 Connection Adapter Version B"
2 "Bus memory address"
3 "Bus I/O address"
4 "Bus interrupt level"
5 "Interrupt priority"
6 "1ST printer address"
7 "2ND printer address"
8 "3RD printer address"
9 "4TH printer address"
10 "5TH printer address"
11 "6TH printer address"
12 "7TH printer address"
13 "8TH printer address"
14 "Number of SESSIONS"
15 "BUFFER size"
$
$  Ethernet
$set 27
1 "Ethernet High-Performance LAN Adapter (8ef5)"
2 "Bus memory address"
3 "Bus I/O address"
4 "Bus interrupt level"
5 "Interrupt priority"
6 "DMA arbitration level"
7 "Receive data transfer OFFSET"
8 "ALTERNATE ETHERNET address"
9 "Offset to ETHERTYPE field"
10 "TRANSMIT queue size"
11 "RECEIVE queue size"
12 "STATUS BLOCK queue size"
13 "Enable ALTERNATE ETHERNET address"
14 "Offset to 802.3 ETHERTYPE"
15 "Adapter CONNECTOR"
16 "Address of bus memory used for DMA"
$ The following message has been replaced by message 23.
20 "Standard Ethernet Adapter"
21 "Adapter has BNC/DIX hardware jumper"
22 "RECEIVE buffer pool size"
$ The following message replaces message 20.
23 "Integrated Ethernet Adapter"
$
$  Token-Ring
$set 28
1 "Token-Ring High-Performance Adapter (8fc8)"
2 "Address of bus memory used for DMA"
3 "Bus I/O address"
4 "Bus interrupt level"
5 "Interrupt priority"
6 "DMA arbitration level"
7 "TRANSMIT queue size"
8 "RECEIVE queue size"
9 "STATUS BLOCK queue size"
10 "Receive ATTENTION MAC frame"
11 "Receive data transfer OFFSET"
12 "RING speed"
13 "Enable ALTERNATE TOKEN RING address"
14 "ALTERNATE TOKEN RING address"
15 "Receive BEACON MAC frame"
$
$  MPQP adapter
$set 29
1 "4-Port Selectable Portmaster Adapter/A (MPQP)"
2 "Bus memory address"
3 "Address of bus memory used for DMA"
4 "Bus I/O address"
5 "Bus interrupt level"
6 "Interrupt priority"
7 "DMA arbitration level"
8 "Adapter code file name"
10 "Portmaster Adapter/A"
11 "6-Port Portmaster Adapter/A V.35"
12 "6-Port Portmaster Adapter/A X.21"
13 "8-Port Portmaster Adapter/A RS-232"
14 "8-Port Portmaster Adapter/A RS-422"
$
$  5085/86/88 Attachment Adapter
$set 30
1 "5085/86/88 Attachment Adapter"
2 "Bus memory address"
3 "Bus I/O address"
4 "Bus interrupt level"
5 "Interrupt priority"
6 "DMA arbitration level"
7 "5080 Coax Communication Adapter"
$
$  Direct Attached Disk
$set 31
1 "Direct Attached Disk Drive"
2 "Bus I/O address"
3 "Bus interrupt level"
4 "Interrupt priority"
5 "DMA arbitration level"
6 "BATTERY backed adapter"
7 "Physical volume identifier"
8 "Size in Mbytes"
9 "120 MB Direct Attached Disk Drive"
10 "160 MB Direct Attached Disk Drive"
$
$  serial adapters
$set 32
1 "8-Port Asynchronous Adapter EIA-232"
2 "8-Port Asynchronous Adapter EIA-422A"
3 "8-Port Asynchronous Adapter MIL-STD 188"
4 "16-Port Asynchronous Adapter EIA-232"
5 "16-Port Asynchronous Adapter EIA-422A"
6 "Bus I/O address"
7 "Bus I/O address for interrupt arbitration register"
8 "Bus interrupt level"
9 "Interrupt priority"
10 "TRANSMIT buffer count"
11 "RECEIVE trigger level"
12 "BAUD rate"
$
$  64 Port rs232 serial adapter
$set 37
1 "64-Port Asynchronous Controller"
2 "Bus memory address"
3 "Bus interrupt level"
4 "Interrupt priority"
5 "TRANSMIT buffer count"
6 "RECEIVE trigger level"
7 "BAUD rate"
8 "Priority of transparent print"
9 "Transparent Print ON String"
10 "Transparent Print OFF String"
$
$  NIO Serial Ports
$set 39
1 "Standard I/O Serial Port 1"
2 "Standard I/O Serial Port 2"
3 "Bus I/O address"
4 "Bus I/O address for interrupt arbitration register"
5 "Bus interrupt level"
6 "Interrupt priority"
7 "DMA arbitration level"
8 "Enable DMA"
9 "Standard I/O Serial Port 3"
10 "TRANSMIT buffer count"
11 "RECEIVE trigger level"
12 "BAUD rate"
$
$  NIO floppy diskette adapter
$set 40
1 "Standard I/O Diskette Adapter"
2 "Bus I/O address"
3 "Bus interrupt level"
4 "Interrupt priority"
5 "DMA arbitration level"
$
$  NIO Parallel Port
$set 41
1 "Standard I/O Parallel Port Adapter"
2 "Bus I/O address"
3 "Bus interrupt level"
4 "Interrupt priority"
$
$  System/370 Host Interface Adapter (5088)
$set 42
1 "System/370 Host Interface Adapter (5088)"
2 "Number of 3270 SESSIONS"
3 "3270 session BUFFER size"
4 "3270 LOWER BOUND session address"
5 ""
6 "LINK speed"
7 "Number of 5080 SESSIONS"
8 "5080 LOWER BOUND session address"
9 ""
10 "5080 CHANNEL ADDRESS"
$
$  X25 ADAPTER
$set 43
1 "X.25 CoProcessor/2 Adapter"
2 "Bus Interrupt Level"
3 "Interrupt Priority"
6 "Bus IO address"
7 "Bus memory address"
8 "Bus memory window size"
180 "X.25 CoProcessor/2 or Multiport/2 Adapter"
181 "X.25 CoProcessor/2 Device Driver"
182 "4-Port Multiport/2 Adapter RS-232"
183 "6-Port Multiport/2 Adapter SYNC"
184 "8-Port Multiport/2 Adapter RS-232"
185 "8-Port Multiport/2 Adapter RS-422"
186 "4-Port RS232/4-Port RS-422 Multiport/2 Adapter"
$ 186 used to state 8 port, but should have shown 4 and 4.
$  Messages 9-179 are obsolete in V4 and have been deleted.
$
$ Midrange Graphics Adapters
$set 44
1 "High Performance 3D Color Graphics Processor"
2 "Screen width in millimeters"
3 "Screen height in millimeters"
4 "Physical display ID for adapter"
5 "Name by which this display is referenced"
6 "Display description"
7 "Color 1 of default color lookup table"
8 "Color 2 of default color lookup table"
9 "Color 3 of default color lookup table"
10 "Color 4 of default color lookup table"
11 "Color 5 of default color lookup table"
12 "Color 6 of default color lookup table"
13 "Color 7 of default color lookup table"
14 "Color 8 of default color lookup table"
15 "Color 9 of default color lookup table"
16 "Color 10 of default color lookup table"
17 "Color 11 of default color lookup table"
18 "Color 12 of default color lookup table"
19 "Color 13 of default color lookup table"
20 "Color 14 of default color lookup table"
21 "Color 15 of default color lookup table"
22 "Color 16 of default color lookup table"
23 "Default path to the downloadable microcode"
24 "Which LFT this display belongs to"
25 "Color Graphics Video Card"
26 "24-bit Color Graphics frame buffer card"
27 "8-bit Color Graphics frame buffer card"
28 "24-bit Z-buffer option card"
29 "Graphics Adapter"
30 "POWER Gt3 Graphics Adapter"
31 "POWER Gt3i 60/77 Hz Graphics Adapter"
32 "POWER Gt4 Midrange Graphics Adapter"
33 "POWER Gt4 Midrange 8-bit Frame Buffer"
34 "POWER Gt4 Midrange 24-bit Frame Buffer option"
35 "POWER Gt4x Midrange Graphics Adapter Turbo option"
36 "Midrange Graphics Adapter subtype"
37 "Bus memory address"
38 "Address of bus memory used for DMA"
39 "DMA arbitration level"
40 "Bus interrupt level"
41 "Interrupt priority"
42 "POWER Gt3zi 60/77 Hz 8-bit Z-buffer Graphics Adapter"
43 "CRT video refresh rate"
44 "CRT video refresh rate enable/disable flag"
45 "Pathname of the device dependent configuration method"
46 "Pathname to the Common Character Mode microcode"
47 "Name of the full function device driver interrupt routine"
48 "Name of the Common Character Mode device driver"
49 "Name of the Common Character Mode device driver interrupt routine"
50 "Name of the CDD kernel load module"
51 "Name of the CDD kernel load module interrupt routine"
52 "Number of seperate bus memory ranges"
53 "Length of the bus memory range"
54 "POWER Gt3i Graphics Adapter"
55 "POWER Gt4e Graphics Adapter"
56 "POWER Gt4 Midrange 24-bit Frame Buffer"
57 "POWER Gt4x Midrange Graphics Adapter"
58 "POWER Gt4i Graphics Adapter"
59 "POWER Gt4xi Graphics Adapter"
60 "POWER Gt4 8-bit Feature"
61 "POWER Gt4 24-bit Feature"
62 "Midrange Graphics Adapter device message subtype"
63 "LFT customized font index"
64 "LFT font kernel process"
65 "Index of the Common Character Mode device driver load module"
$ Following 4 messages are copied to Set 45 - nobody should use them anymore
66 "Display type enable/disable flag"
67 "Display type"
68 "Display resolution and refresh rate enable/disable flag"
69 "Display resolution and refresh rate"
$
$ Color Graphics Display Adapter
$set 45
1 "Color Graphics Display Adapter"
2 "Screen width in millimeters"
3 "Screen height in millimeters"
4 "Physical display ID for adapter"
5 "Name by which this display is referenced"
6 "Display description"
7 "Color 1 of default color lookup table"
8 "Color 2 of default color lookup table"
9 "Color 3 of default color lookup table"
10 "Color 4 of default color lookup table"
11 "Color 5 of default color lookup table"
12 "Color 6 of default color lookup table"
13 "Color 7 of default color lookup table"
14 "Color 8 of default color lookup table"
15 "Color 9 of default color lookup table"
16 "Color 10 of default color lookup table"
17 "Color 11 of default color lookup table"
18 "Color 12 of default color lookup table"
19 "Color 13 of default color lookup table"
20 "Color 14 of default color lookup table"
21 "Color 15 of default color lookup table"
22 "Color 16 of default color lookup table"
23 "Which LFT this display belongs to"
24 "Fairness and Nibble mode enable mask"
25 "POWER Gt1 Frame Buffer Adapter"
26 "POWER Gt1x Graphics Adapter"
27 "Current number of SIM modules on adapter"
28 "Previous number of SIM modules on adapter"
29 "Pathname of the device dependent configuration method"
30 "Pathname to the full function  microcode"
31 "Pathname to the Common Character Mode microcode"
32 "Name of the Common Character Mode device driver"
33 "Name of the Common Character Mode device driver interrupt routine"
34 "Name of the CDD kernel load module"
35 "Name of the CDD kernel load module interrupt routine"
36 "Name of the full function device driver interrupt routine"
37 "GXT100 Graphics Adapter"
38 "GXT150 Graphics Adapter"
39 "Device Interrupt Level"
40 "Device Interrupt Priority"
41 "GXT150L Graphics Adapter"
42 "GXT150M Graphics Adapter"
43 "GXT155L Graphics Adapter"
44 "Bus memory address"
45 ""
46 "GXT150P Graphics Adapter"
47 "Performance Graphics Adapter"
48 "Standard Graphics Adapter"
49 "G10 Graphics Adapter"
$ Following 4 messages are copied from Set 44
66 "Display type enable/disable flag"
67 "Display type"
68 "Display resolution and refresh rate enable/disable flag"
69 "Display resolution and refresh rate"
70 "BBLMCA Graphics Adapter"
71 "IBM Personal Computer Power Series E10 Graphics Adapter"
72 "IBM Personal Computer Power Series S10 Graphics Adapter"
73 "IBM Personal Computer Power Series E15 Graphics Adapter"
74 "IBM Personal Computer Power Series S15 Graphics Adapter"
75 "IBM Personal Computer Power Series H10 Graphics Adapter"
76 "IBM Personal Computer Power Series 600 E10 Graphics Adapter"
77 "IBM Personal Computer Power Series 600 E15 Graphics Adapter"
78 "IBM Personal Computer Power Series 600 S15 Graphics Adapter"
79 "IBM Personal Computer Power Series GXT150P Graphics Adapter"
80 "E10 Graphics Adapter"
81 "S10 Graphics Adapter"
82 "E15 Graphics Adapter"
83 "S15 Graphics Adapter"
84 "H10 Graphics Adapter"
85 "600 E10 Graphics"
86 "600 E15 Graphics"
87 "600 S15 Graphics"
$
$ Grayscale Graphics Display Adapter
$set 46
1 "Grayscale Graphics Display Adapter"
2 "Screen width in millimeters"
3 "Screen height in millimeters"
4 "Physical display ID for adapter"
5 "Name by which this display is referenced"
6 "Display description"
7 "Color 1 of default grayscale lookup table"
8 "Color 2 of default grayscale lookup table"
9 "Color 3 of default grayscale lookup table"
10 "Color 4 of default grayscale lookup table"
11 "Color 5 of default grayscale lookup table"
12 "Color 6 of default grayscale lookup table"
13 "Color 7 of default grayscale lookup table"
14 "Color 8 of default grayscale lookup table"
15 "Color 9 of default grayscale lookup table"
16 "Color 10 of default grayscale lookup table"
17 "Color 11 of default grayscale lookup table"
18 "Color 12 of default grayscale lookup table"
19 "Color 13 of default grayscale lookup table"
20 "Color 14 of default grayscale lookup table"
21 "Color 15 of default grayscale lookup table"
22 "Color 16 of default grayscale lookup table"
23 "Which LFT this display belongs to"
24 "Fairness and Nibble mode enable mask"
$
$ High Speed 3D Graphics Accelerator
$set 47
1 "High Speed 3D Graphics Accelerator"
2 "Screen width in millimeters"
3 "Screen height in millimeters"
4 "Physical display ID for adapter"
5 "Name by which this display is referenced"
6 "Display description"
7 "Color 1 of default color lookup table"
8 "Color 2 of default color lookup table"
9 "Color 3 of default color lookup table"
10 "Color 4 of default color lookup table"
11 "Color 5 of default color lookup table"
12 "Color 6 of default color lookup table"
13 "Color 7 of default color lookup table"
14 "Color 8 of default color lookup table"
15 "Color 9 of default color lookup table"
16 "Color 10 of default color lookup table"
17 "Color 11 of default color lookup table"
18 "Color 12 of default color lookup table"
19 "Color 13 of default color lookup table"
20 "Color 14 of default color lookup table"
21 "Color 15 of default color lookup table"
22 "Color 16 of default color lookup table"
23 "microcode for the graphics control processor"
24 "graphics control processor table"
25 "microcode for c25 based adapters"
26 "shading processor table"
27 "Which LFT this display belongs to"
28 "CVME interface card"
29 "Graphics control processor card"
30 "Drawing processor card"
31 "Shading processor card"
32 "Pixel memory card"
$
$ GXT300 Graphics Adapter
$set 48
1 "Gxt300-160 Graphics Adapter"
2 "Screen width in millimeters"
3 "Screen height in millimeters"
4 "Screen width in pixels"
5 "Screen height in pixels"
6 "Physical display ID for adapter"
7 "Name by which this display is referenced"
8 "Display description"
9 "Color 1 of default color lookup table"
10 "Color 2 of default color lookup table"
11 "Color 3 of default color lookup table"
12 "Color 4 of default color lookup table"
13 "Color 5 of default color lookup table"
14 "Color 6 of default color lookup table"
15 "Color 7 of default color lookup table"
16 "Color 8 of default color lookup table"
17 "Color 9 of default color lookup table"
18 "Color 10 of default color lookup table"
19 "Color 11 of default color lookup table"
20 "Color 12 of default color lookup table"
21 "Color 13 of default color lookup table"
22 "Color 14 of default color lookup table"
23 "Color 15 of default color lookup table"
24 "Color 16 of default color lookup table"
25 "Which LFT this display belongs to"
26 "Customization Features""
27 "Configuration Method Load"
28 "CCM Device Driver "
29 "CCM Interupt Handler"
30 "Interupt Level"
31 "Interupt Priority"
32 "Bus ID"
33 "Interupt Handler"
34 "Bus Address Start"
35 "Bus Memory Start"
36 "CCD kmod entry"
37 "CCD kmod interupt"
38 "GXT300-80 Graphics Adapter"
$
$ GXT150M Graphics Adapter
$set 49
$ 1 "GXT170M Graphics Adapter"
$ Above message is replaced by new one.
1 "GXT150M Graphics Adapter"
$ Above is the new one
2 "Start Address of Bus I/O"
3 "Start Address of Bus Memory used for PRAM"
4 "Start Address of Bus Memory used for Frame Buffer"
5 "DMA Arbitration Level"
6 "Start Address of DMA Window 0"
7 "Start Address of DMA Window 1"
8 "Start Address of DMA Window 2"
9 "Start Address of DMA Window 3"
10 "Interrupt Level"
11 "Interrupt Priority"
12 "Screen Width in Millimeters"
13 "Screen Height in Millimeters"
14 "Physical Display ID"
15 "Name by which this display is referenced"
16 "Display Description"
17 "Color 1 of default color lookup table"
18 "Color 2 of default color lookup table"
19 "Color 3 of default color lookup table"
20 "Color 4 of default color lookup table"
21 "Color 5 of default color lookup table"
22 "Color 6 of default color lookup table"
23 "Color 7 of default color lookup table"
24 "Color 8 of default color lookup table"
25 "Color 9 of default color lookup table"
26 "Color 10 of default color lookup table"
27 "Color 11 of default color lookup table"
28 "Color 12 of default color lookup table"
29 "Color 13 of default color lookup table"
30 "Color 14 of default color lookup table"
31 "Color 15 of default color lookup table"
32 "Color 16 of default color lookup table"
33 "Pathname to the full function microcode"
34 "Which LFT this display belongs to"
35 "Name of the Common Character Mode device driver"
36 "Name of the Common Character Mode microcode"
37 "Name of the Common Character Mode device driver interrupt routine"
38 "Name of the CDD kernel load module"
39 "Name of the CDD kernel load module interrupt routine"
40 "Pathname of the device dependent configuration method"
$
$  PTY
$set 50
1 "Asynchronous Pseudo-Terminal"
2 "Maximum number of Pseudo-Terminals"
3 "Maximum number of BSD Pseudo-Terminals"
4 "STATE to be configured at boot time"
$
$  CDROM
$set 51
1 "CD-ROM Drive"
2 "RESERVE device on open"
3 "PREVENT ejection of media when open"
4 "Physical volume identifier"

10 "Other SCSI CD-ROM Drive"
11 "Multimedia CD-ROM Drive"
20 "Other SCSI Read/Write Optical Drive"
21 "127 MB SCSI Read/Write Optical Drive"
30 "Queue DEPTH"
31 "Queuing TYPE"
32 "Use QERR bit"
33 "Device CLEARS its Queue on error"
34 "READ/WRITE time out value"
35 "START UNIT time out value"
36 "REASSIGN time out value"
37 "FORMAT unit time out value"
38 "READ time out value"
40 "Play AUDIO supported"
41 "MODE 2 FORM 1 Density Code"
42 "MODE 2 FORM 2 Density Code"
43 "CD-DA Density Code"

$
$  SCSI Tape Drives
$set 54
1 "150 MB 1/4-Inch Tape Drive"
2 "2.3 GB 8mm Tape Drive"
3 "1/2-inch 9-Track Tape Drive"
4 "Other SCSI Tape Drive"
5 "Use DEVICE BUFFERS during writes"
6 "BLOCK size (0=variable length)"
7 "RETENSION on tape change or reset"
8 "Use EXTENDED file marks"
9 "RESERVE/RELEASE support"
10 ""
11 "BLOCK SIZE for variable length support"
12 "DENSITY setting #1"
13 "DENSITY setting #2"
14 "5.0 GB 8mm Tape Drive"
15 "Use data COMPRESSION"
16 "525 MB 1/4-Inch Tape Drive"
17 "1.2 GB 1/4-Inch Tape Drive"
18 "2.0 GB 4mm Tape Drive"
19 "7.0 GB 8mm Tape Drive"
20 "3490E Autoloading Tape Drive"
21 "4.0 GB 4mm Tape Drive"
22 "AUTOLOAD next cartridge"
23 "Set delay after a FAILED command"
24 "Set timeout for the READ or WRITE command"
$
$  SCSI Disk Drives
$set 57
1 "320 MB SCSI Disk Drive"
2 "Physical volume identifier"
3 "400 MB SCSI Disk Drive"
4 "160 MB SCSI Disk Drive"
5 "355 MB SCSI Disk Drive"
6 "670 MB SCSI Disk Drive"
7 "857 MB SCSI Disk Drive"
8 "Other SCSI Disk Drive"
9 "1.0 GB SCSI Disk Drive"
10 "1.37 GB SCSI Disk Drive"
11 "1.2 GB SCSI Disk Drive (in 2.4 GB Disk Unit)"
12 "200 MB SCSI Disk Drive"
13 "2.0 GB SCSI Disk Drive" 
14 "540 MB SCSI Disk Drive"
15 "1.0 GB Differential SCSI Disk Drive"
16 "2.0 GB Differential SCSI Disk Drive"
17 "2.0 GB 16 Bit Differential SCSI Disk Drive"
18 "2.0 GB 16 Bit SCSI Disk Drive"
19 "1.0 GB 16 Bit Differential SCSI Disk Drive"
20 "730 MB SCSI Disk Drive"
21 "1.1 GB SCSI Disk Drive"
22 "1.1 GB 16 Bit SCSI Disk Drive"
23 "1.1 GB 16 Bit Differential SCSI Disk Drive"
24 "2.2 GB SCSI Disk Drive"
25 "2.2 GB 16 Bit SCSI Disk Drive"
26 "2.2 GB 16 Bit Differential SCSI Disk Drive"
27 "4.5 GB 16 Bit SCSI Disk Drive"
28 "4.5 GB 16 Bit Differential SCSI Disk Drive"
29 "270 MB SCSI Disk Drive"
30 "Queue DEPTH"
31 "Queuing TYPE"
32 "Use QERR bit"
33 "Device CLEARS its Queue on error"
34 "READ/WRITE time out value"
35 "START unit time out value"
36 "REASSIGN time out value"
40 "340 MB SCSI Disk Drive"
41 "360 MB SCSI Disk Drive"

$
$  Floppy Diskette Drive
$set 60
1 "Diskette Drive"
2 "Diskette DRIVE TYPE"
$
$  MPQP port
$set 61
1 "Multiprotocol Adapter Port"
2 "Receive data transfer OFFSET"
3 "STATE to be configured at boot time"
5 "4-Port Multiprotocol Device Driver"
$
$  SCSI I/O Controller Initiators
$set 62
1 "SCSI I/O Controller Initiator Device"
2 "Other SCSI Initiator Device"
3 "Receive data buffer SIZE"
4 "NUMBER of receive data buffers"
$
$  TTY
$set 65
1 "Asynchronous Terminal"
2 "2nd END OF LINE character"
3 "BAUD rate"
4 "KILL character"
5 "ERASE character"
6 "END OF FILE character"
7 "DELAY SUSPEND PROCESS character"
8 "SUSPEND PROCESS character"
9 "LITERAL NEXT character"
10 "START character"
11 "STOP character"
12 "WORD ERASE character"
13 "PARITY"
14 "BITS per character"
15 "Number of STOP BITS"
16 "XON-XOFF handshaking"
17 "TERMINAL type"
18 "Enable LOGIN"
19 "STTY attributes for RUN time"
20 "QUIT character"
21 "INTERRUPT character"
22 "REPRINT LINE character"
23 "STTY attributes for LOGIN"
24 "END OF LINE character"
25 "DISCARD character"
26 "STATUS of device at BOOT time"
27 "INPUT map file"
28 "OUTPUT map file"
29 "TRANSMIT buffer count"
30 "RUN shell activity manager"
31 "TIME before advancing to next port setting"
32 "LOGGER name"
33 "CODESET map file"
34 "STREAMS modules to be pushed at OPEN time"
35 "FLOW CONTROL to be used"
36 "OPEN DISCIPLINE to be used"
37 "RECEIVE trigger level"
43 "Priority of transparent print"
44 "Transparent Print ON String"
45 "Transparent Print OFF String"
46 Transparent Print Maximum Characters per Second
47 Transparent Print Maximum Character Packet Size
48 Transparent Print Printer Buffer Size
49 Force Carrier
50 Perform Cooked Processing in Adapter
51 Use Alternate RJ-45 Pinouts
52 2200 Flow Control
53 2200 Print Control
61 Monitor 128-Port Async Adapters
62 128-Port Asynchronous Adapter
63 List All Defined 128-Port Async Adapters
64 Change / Show Characteristics of a 128-Port Async Adapter
65 Configure a Defined 128-Port Async Adapter
66 Generate an Error Report
67 Trace a 128-Port Async Adapter
68 128-Port Async Adapter
69 128-Port Async Adapter Type
71 Description
72 Status
73 Location
74 Parent Device
77 Line 1 Speed
78 Line 1 Cable Type
79     Node 1 Comm Mode
80     Node 2 Comm Mode
81     Node 3 Comm Mode
82     Node 4 Comm Mode
83 Line 2 Speed
84 Line 2 Cable Type
88 Asynchronous Adapters
89 Receive Event Delay Time
$
$  Parallel Printers
$set 71
1 "IBM 4201 Model 3 Proprinter III"
2 "IBM 4207 Model 2 Proprinter X24E"
3 "IBM 5202 Quietwriter III"
4 "IBM 4216 Personal Page Printer, Model 031"
5 "IBM 4234 Dot Band Printer Model 013"
6 "IBM 4019 LaserPrinter"
7 "Hewlett-Packard LaserJet II"
8 "Texas Instruments OmniLaser 2115 Page Printer"
9 "Dataproducts LZR 2665 Laser Printer"
10 "Dataproducts BP2000 Line Printer"
11 "Printronix P9012 Line Printer"
12 "IBM 5575 Model B02/F02"
13 ""
14 ""
15 "IBM 5327 Model 011"
16 "Other parallel printer"
17 "Printer TIME OUT period"
18 "Number of LINES per page"
19 "Number of COLUMNS per page"
20 "Number of columns to INDENT"
21 "Send all characters to printer UNMODIFIED"
22 "Send BACKSPACES"
23 "Send CARRIAGE RETURNS"
24 "Send FORM FEEDS"
25 "Send LINE FEEDS"
26 "Add CARRIAGE RETURNS to LINE FEEDS"
27 "Convert lowercase to UPPERCASE"
28 "EXPAND TABS on eight position boundaries"
29 "WRAP CHARACTERS beyond the specified width"
30 "Return on ERROR"
31 "Type of PARALLEL INTERFACE"
32 "STATE to be configured at boot time"
33 "Microseconds to delay between characters"
$ More parallel printer descriptions
$       still in set 71
$       gap in numbers is so additional attributes can be added if necessary
50 "IBM 4201 Model 2 Proprinter II"
51 "IBM 4202 Model 2 Proprinter II XL"
52 "IBM 4202 Model 3 Proprinter III XL"
53 "IBM 4208 Model 2 Proprinter XL24E"
54 "IBM 5204 Quickwriter"
55 "IBM 4216 Model 510"
56 "IBM 4208 Model 502 Proprinter XL24EK"
57 "IBM 5572 Model B02"
58 "QMS ColorScript 100, Model 20"
59 "IBM 4072 ExecJet"
60 "IBM 4212 Proprinter 24P"
61 "IBM 4226 Printer"
62 "IBM 2380 Personal Printer II"
63 "IBM 2381 Personal Printer II"
64 "IBM 2390 Personal Printer II"
65 "IBM 2391 Personal Printer II"
66 "IBM 4029 LaserPrinter"
67 ""
68 ""
69 "IBM 5577 Model B02/F02/H02/G02/FU2"
70 "IBM 5587 Model G01"
71 "Canon Laser Shot LBP-B406/S/D,A404"
72 "IBM 5587 Model H01"
73 "IBM 6252 Impactwriter"
74 "IBM 6262 Printer"
75 "Hewlett-Packard LaserJet III"
76 "IBM 5585 Model H01"
77 "IBM 5589 Model H01"
78 "Canon Laser Shot LBP-B406/S/D/E,A404"
79 "Bull Compuprint 922"
80 "Bull Compuprint 923"
81 "Bull Compuprint 924"
82 "Bull Compuprint 924 N"
83 "Bull Compuprint 4/51"
84 "Bull Compuprint 4/51"
85 "Bull Compuprint 4/54"
86 "Bull Compuprint 4/54"
87 "Bull Compuprint 970"
88 "Bull Compuprint 1070"
89 "Bull Compuprint PageMaster 411"
90 "Bull Compuprint PageMaster 422"
91 "Bull Compuprint PageMaster 721"
92 "Bull Compuprint PageMaster 1021"
93 "Bull Compuprint PageMaster 201"
94 "Bull PR-88"
95 "Bull PR-90"
96 "IBM 4070 IJ printer"
97 "IBM 4079 Color Jetprinter PS"
98 "Canon Laser Shot LBP-B406/S/D/E,A404/E,A304E"
103 "IBM 4039 LaserPrinter"
105 "Hewlett-Packard LaserJet IIISi"
106 "Hewlett-Packard LaserJet 4,4M"
107 "IBM 2380 Plus printer (Model 2)"
108 "IBM 2381 Plus printer (Model 2)"
109 "IBM 2390 Plus printer (Model 2)"
110 "IBM 2391 Plus printer (Model 2)"
111 "IBM 4037 LP printer"
112 "IBM 4076 IJ printer"
113 "IBM 5579 Model H02"
114 ""
115 ""
116 "IBM 5573 Model H02"
117 "IBM 5577 Model B02/F02/H02/G02/FU2/J02/K02"
118 "IBM 5579 Model H02/K02"
119 "IBM 5584 Model G02/H02"
120 "Canon LASER SHOT LBP-B406G"
121 "Canon LASER SHOT LBP-A404PS/Lite"
122 "Star AR2463 printer"
123 "Epson LQ1600k printer"
129 ""
130 "Hewlett-Packard LaserJet 4Si"
131 "Hewlett-Packard LaserJet 4 Plus"
132 "Hewlett-Packard LaserJet 4V"
133 "Hewlett-Packard LaserJet Color"
$ New BULL parallel printers
$
154	"Bull PR-88 VFU Handling"
155	"Bull Compuprint 914"
156	"Bull Compuprint 914 N"
157	"Bull Compuprint 956"
158	"Bull Compuprint PageMaster 200"
159	"Bull Compuprint PageMaster 413"
160	"Bull Compuprint PageMaster 815"
161	"Bull Compuprint PageMaster 825"
162	"Bull Compuprint PageMaster 1015"
163	"Bull Compuprint PageMaster 1025"
164	"Bull Compuprint PageMaster 1625"
$ New IBM Printers
$
180 "IBM 3112 Page Printer"
181 "IBM 3116 Page Printer"
182 "IBM 3930 Page Printer"
183 "IBM 4230 Printer"
184 "IBM 6408 Printer"
185 "IBM 6412 Printer"
$ New Lexmark Printers
$
201 "Lexmark Optra LaserPrinter"
202 "Lexmark ExecJet IIc"
203 "Lexmark ValueWriter 600"
204 "Lexmark 4039 plus LaserPrinter"
205 "Lexmark 4079 Color Jetprinter Plus"
$
$  Serial Printers
$set 72
1 "IBM 4201 Model 3 Proprinter III"
2 "IBM 4207 Model 2 Proprinter X24E"
3 "IBM 3812 Model 2 Page Printer"
4 "IBM 3816 Page Printer"
5 "IBM 4216 Personal Page Printer, Model 031"
6 "IBM 4224 Printer, Models 301,302,3C2,3E3"
7 "IBM 4234 Dot Band Printer, Model 013"
8 "IBM 4019 LaserPrinter"
9 "Hewlett-Packard LaserJet II"
10 "Texas Instruments OmniLaser 2115 Page Printer"
11 "Dataproducts LZR 2665 Laser Printer"
12 "Dataproducts BP2000 Line Printer"
13 "Printronix P9012 Line Printer"
14 "Other serial printer"
15 "BAUD rate"
16 "PARITY"
17 "Printer TIME OUT period"
18 "Number of LINES per page"
19 "Number of COLUMNS per page"
20 "Number of COLUMNS to indent"
21 "Send BACKSPACES"
22 "Convert lowercase to uppercase"
23 "Send CARRIAGE RETURNS"
24 "Return on ERROR"
25 "Send FORM FEEDS"
26 "Send LINE FEEDS"
27 "Send all characters to printer UNMODIFIED"
28 "Expand TABS on eight position boundaries"
29 "WRAP CHARACTERS beyond the specified width"
30 "BITS per character"
31 "Number of STOP BITS"
32 "Add CARRIAGE RETURNS to LINE FEEDS"
33 "STATUS of device at BOOT time"
34 "XON-XOFF handshaking"
35 "Use DTR communication protocol"
36 "TRANSMIT buffer count"
37 "STREAMS modules to be pushed at OPEN time"
38 "FLOW CONTROL to be used"
39 "OPEN DISCIPLINE to be used"
40 "Receive TRIGGER level"
41 "Use Alternate RJ-45 Pinouts"
$ More serial printer descriptions
$       still in set 72
$       gap in numbers is so additional attributes can be added if necessary
50 "IBM 4201 Model 2 Proprinter II"
51 "IBM 4202 Model 2 Proprinter II XL"
52 "IBM 4202 Model 3 Proprinter III XL"
53 "IBM 4208 Model 2 Proprinter XL24E"
54 "IBM 5204 Quickwriter"
55 "IBM 6180 Color Plotter"
56 "IBM 6182 Auto Feed Color Plotter"
57 "IBM 6184 Color Plotter"
58 "IBM 6186 Color Plotter"
59 "IBM 7372 Color Plotter"
60 "IBM 6185-1 Color Plotter"
61 "IBM 6185-2 Color Plotter"
62 "IBM 4072 ExecJet"
63 "IBM 4226 Printer"
64 "IBM 2380 Personal Printer II"
65 "IBM 2381 Personal Printer II"
66 "IBM 2390 Personal Printer II"
67 "IBM 2391 Personal Printer II"
68 "IBM 4029 LaserPrinter"
69 ""
70 ""
71 "OKI Microline 801PS"
72 "IBM 6252 Impactwriter"
73 "IBM 6262 Printer"
74 "Hewlett-Packard LaserJet III"
75 "Bull Compuprint 922"
76 "Bull Compuprint 923"
77 "Bull Compuprint 924"
78 "Bull Compuprint 924 N"
79 "Bull Compuprint 4/51"
80 "Bull Compuprint 4/51"
81 "Bull Compuprint 4/54"
82 "Bull Compuprint 4/54"
83 "Bull Compuprint 970"
84 "Bull Compuprint 1070"
85 "Bull Compuprint PageMaster 411"
86 "Bull Compuprint PageMaster 422"
87 "Bull Compuprint PageMaster 721"
88 "Bull Compuprint PageMaster 1021"
89 "Bull Compuprint PageMaster 201"
90 "Bull PR-88"
91 "Bull PR-90"
92 "IBM 4079 Color Jetprinter PS"
103 "IBM 4039 LaserPrinter"
105 "Hewlett-Packard LaserJet IIISi"
106 "Hewlett-Packard LaserJet 4,4M"
107 "IBM 2380 Plus printer (Model 2)"
108 "IBM 2381 Plus printer (Model 2)"
109 "IBM 2390 Plus printer (Model 2)"
110 "IBM 2391 Plus printer (Model 2)"
111 "IBM 4037 LP printer"
112 "IBM 4076 IJ printer"
118 ""
119 "OKI MICROLINE 801PS/+F,801PSII/+F,800PSIILT"
120 "Canon LASER SHOT LBP-B406G"
121 "Canon LASER SHOT LBP-A404PS/Lite"
129 ""
130 "Hewlett-Packard LaserJet 4Si"
131 "Hewlett-Packard LaserJet 4 Plus"
132 "Hewlett-Packard LaserJet 4V"
133 "Hewlett-Packard LaserJet Color"
$
$ New BULL serial printers
$
154	"Bull PR-88 VFU Handling"
155	"Bull Compuprint 914"
156	"Bull Compuprint 914 N"
157	"Bull Compuprint 956"
158	"Bull Compuprint PageMaster 200"
159	"Bull Compuprint PageMaster 413"
160	"Bull Compuprint PageMaster 815"
161	"Bull Compuprint PageMaster 825"
162	"Bull Compuprint PageMaster 1015"
163	"Bull Compuprint PageMaster 1025"
164	"Bull Compuprint PageMaster 1625"
$ New IBM Printers
$
180 "IBM 3112 Page Printer"
181 "IBM 3116 Page Printer"
182 "IBM 3930 Page Printer"
183 "IBM 4230 Printer"
184 "IBM 6408 Printer"
185 "IBM 6412 Printer"
$ New Lexmark Printers
$
201 "Lexmark Optra LaserPrinter"
202 "Lexmark ExecJet IIc"
203 "Lexmark ValueWriter 600"
204 "Lexmark 4039 plus LaserPrinter"
205 "Lexmark 4079 Color Jetprinter Plus"
$
$ 370 PCA Adapter
$set 79
1 "370 Parallel Channel Adapter"
2 "Bus memory address"
3 "Bus interrupt level"
4 "Interrupt priority"
5 "DMA arbitration level"
6 "Transmit buffer SIZE"
7 "NUMBER of transmit buffers"
8 "Receive buffer SIZE"
9 "NUMBER of receive buffers"
10 "Receive data transfer OFFSET"
11 "Channel SPEED"
12 "STARTING subchannel address"
13 "NUMBER of subchannel addresses"
14 "CLAW mode subchannel set"
15 "Online/Offline SWITCH"
16 "Online/Offline INDICATOR"
$
$ FDDI Adapter
$set 80
1 "FDDI Single Ring Card"
2 "Bus interrupt level"
3 "Address of bus memory used for DMA"
4 "Interrupt priority"
5 "RECEIVE queue size"
6 "STATUS BLOCK queue size"
7 "Receive data transfer OFFSET"
8 "Bus I/O address"
9 "Bus memory address"
10 "DMA arbitration level"
11 "Enable ALTERNATE FDDI Source address"
12 "ALTERNATE FDDI Source address"
13 "Receive SHORT ADDRESS frames"
14 "Receive BEACON frames"
15 "Receive SMT frames"
16 "Receive NSA frames"
17 "FDDI Dual Ring Upgrade Kit"
18 "PMF password"
19 "AUTHORIZATION enabled"
20 "USER data"
21 "T-MAX greatest lower bound"
22 "T-MAX lower bound"
23 "T-MAX"
24 "T-MIN"
25 "T-REQ"
26 "TVX"
27 "INSERT policy"
28 "Port A LER cutoff"
29 "Port B LER cutoff"
30 "Port A LER alarm"
31 "Port B LER alarm"
32 "SAMPLE time"
33 "WEIGHTING factor"
$
$ Asynchronous I/O
$set 100
1 "Asynchronous I/O"
2 "MINIMUM number of servers"
3 "MAXIMUM number of servers"
4 "Maximum number of REQUESTS"
5 "Server PRIORITY"
6 "STATE to be configured at system restart"
$
$ Internet Network Interface
$set 110
1  "Standard Ethernet Network Interface"
2  "Maximum IP Packet Size for This Device"
3  "Maximum IP Packet Size for REMOTE Networks"
4  "Internet Address"
5  "Current Interface Status"
6  "TRAILER Link-Level Encapsulation"
7  "Address Resolution Protocol (ARP)"
8  "Subnet Mask"
9  "Security Level"
10 "Authorized Users"
11 "Broadcast Address"
12 "Destination Address"
13 "Baud Rate"
14 "TTY Port Number"
15 "Dial String"
16 "Confine Broadcast to Local Token-Ring"
17 "Enable Hardware Loopback Mode"
18 "IEEE 802.3 Ethernet Network Interface"
19 "Standard Ethernet Network Interface"
20 "Serial Line Network Interface"
21 "Token Ring Network Interface"
22 "X.25 Network Interface"
23 "Internet Network Extension"
24 "Host Name"
25 "Gateway"
26 "Route"
27 "Serial Optical Network Interface"
28 "FDDI Network Interface"
29 "370 Parallel Channel Network Interface"
30 "Loopback Network Interface"
$
$ Keyboard/Tablet/Sound
$set 126
1 "Keyboard Adapter"
2 "Tablet Adapter"
3 "Keyboard repeat delay"
4 "Keyboard repeat rate"
5 "Resolution for the Tablet"
6 "Sample rate for the Tablet"
7 "Horizontal dead zone for the Tablet"
8 "Vertical dead zone for the Tablet"
9 "Origin of the Tablet"
10 "Tablet conversion"
11 "Alarm volume"
12 "Clicker volume"
13 "United States keyboard"
14 "International keyboard"
15 "International keyboard"
16 "5086 keyboard"
17 "6.144 x 6.144 inch tablet"
18 "11.5 x 11.5 inch tablet"
19 "Bus I/O address"
20 "Bus interrupt level"
21 "Interrupt priority"
22 "Keyboard/Tablet Adapter"
23 "Extended keyboard identifier"
24 "PS/2 keyboard"
25 "Keyboard/Mouse Adapter"
$
$ Mouse
$set 127
1 "Mouse Adapter"
2 "Resolution for mouse movement"
3 "Scaling factor"
4 "Sample rate for mouse"
5 "Horizontal threshold"
6 "Vertical threshold"
7 "3 button mouse"
8 "2 button mouse"
$
$ LFT messages
$set 128
1 "Low Function Terminal Subsystem"
3 "Logical name of default display for LFT"
4 "Default software keyboard path"
5 "Predefined font path number 1"
6 "Predefined font path number 2"
7 "Predefined font path number 3"
8 "Predefined font path number 4"
9 "Predefined font path number 5"
10 "Predefined font path number 6"
11 "Predefined font path number 7"
12 "Predefined font path number 8"
13 "Timeout value for display entering standby or dimmed state"
14 "Timeout value for display entering suspend state"
15 "Timeout value for display entering off state"
$
$ Data Link Control messages
$set 129
$  The following messages describe the types of Data Link Control Protocols
1 "X.25 QLLC Data Link Control"
2 "Token-Ring Data Link Control"
3 "SDLC Data Link Control"
4 "Standard Etherent Data Link Control"
5 "IEEE Ethernet (802.3) Data Link Control"
$ The following message describes the DLC attribute
6 "Depth of the queue"
7 "FDDI Data Link Control"
$
$  5085/86 Attachment Adapter (GSW)
$set 130
1 "5085/86 Attachment Adapter"
$
$ Multimedia Audio Capture and Playback Adapter
$set 131
1 "Bus memory address "
2 "Bus interrupt level"
3 "Interrupt priority"
4 "5.5 kHz READ buffer size in seconds"
5 "5.5 kHz WRITE buffer size in seconds"
6 "11 kHz READ buffer size in seconds"
7 "11 kHz WRITE buffer size in seconds"
8 "22 kHz READ buffer size in seconds"
9 "22 kHz WRITE buffer size in seconds"
10 "44 kHz READ buffer size in seconds"
11 "44 kHz WRITE buffer size in seconds"
12 "NUMBER of requests in request queue"
13 "8 kHz READ buffer size in seconds"
14 "8 kHz WRITE buffer size in seconds"
15 "Multimedia Audio Capture & Playback Adapter"
$
$ Multimedia Video Capture Adapter
$set 132
1 "Multimedia Video Capture Adapter"
2 "Bus I/O address"
3 "Bus interrupt level"
4 "Interrupt priority"
5 "Image plane Bus Memory Start"
6 "Overlay plane Bus Memory Start"
7 "BLUE BLACK dac"
8 "GREEN BLACK dac"
9 "RED BLACK dac"
10 "BLUE WHITE dac"
11 "GREEN WHITE dac"
12 "RED WHITE dac"
13 "HUE dac"
14 "SATURATION dac"
15 "CONTRAST dac"
16 "CURSOR ENABLE"
17 "CURSOR TYPE"
18 "CUROSR RED"
19 "CURSOR GREEN"
20 "CURSOR BLUE"
$
$ Serial Dasd Subsystem
$set 140
1  "Serial-Link Disk Adapter"
2  "Serial-Link Disk Controller"
3  "Serial Dasd microcode file"
4  "Bus interrupt level"
5  "DMA arbitration level"
6  "Address of bus memory used for DMA"
7  "Bus I/O address"
8  "Interrupt priority"
15 "Physical volume identifier"
30 "857MB Serial-Link Disk Drive"
31 "857MB C Serial-Link Disk Drive"
32 "471MB F Serial-Link Disk Drive"
33 "471MB C Serial-Link Disk Drive"
34 "1.07GB Serial-Link Disk Drive"
35 "1.07GB C Serial-Link Disk Drive"
36 "2.0GB Serial-Link Disk Drive"
50 "Serial-Link Two Port Controller"
51 "Serial-Link Four Port Controller"
52 "Serial-Link Eight Port Controller"
53 "Serial-Link Switched Controller"
$
$ Network File System swap device
$set 141
1 "NFS Swap DEVICE"
2 "HOST name of NFS server"
3 "FILE name to use for swapping"
4 "NUMBER of daemons to start for NFS swap device"
$
$ 128-Port Asynchronous Adapter
$set 142
1 128-Port Asynchronous Adapter
2 16-Port RAN EIA-232 for 128-Port Adapter
3 Bus memory address
4 Bus I/O address
5 Line Speed
6 Cable Type
7 Comm Mode
8 8-Port Enhanced Async Adapter EIA-232
9 "BAUD rate"
10 "Transparent Print ON String"
11 "Transparent Print OFF String"
12 Transparent Print Maximum Characters per Second
13 Transparent Print Maximum Character Packet Size
14 Transparent Print Printer Buffer Size
15 Force Carrier
16 Perform Cooked Processing in Adapter
17 Use Alternate RJ-45 Pinouts
18 Receive Event Delay Time
19 2200 Flow Control
20 2200 Print Control
$
$set 143 Sgabus
1 "Special graphics slot"
2 "Bus unit id"
$set 144
$ "7250 POWER GXT1000 Graphics Accelerator"
1 "Display Description"
2 "Screen width in millimeters"
3 "Screen height in millimeters"
4 "Bus memory starting address"
5 "Interrupt level"
6 "Interrupt priority"
7 "Physical display ID for adapter"
8 "Name by which this display is referenced"
9 "7250 POWER GXT1000 Graphics Accelerator Model 1"
10 "Color 1 of default color lookup table"
11 "Color 2 of default color lookup table"
12 "Color 3 of default color lookup table"
13 "Color 4 of default color lookup table"
14 "Color 5 of default color lookup table"
15 "Color 6 of default color lookup table"
16 "Color 7 of default color lookup table"
17 "Color 8 of default color lookup table"
18 "Color 9 of default color lookup table"
19 "Color 10 of default color lookup table"
20 "Color 11 of default color lookup table"
21 "Color 12 of default color lookup table"
22 "Color 13 of default color lookup table"
23 "Color 14 of default color lookup table"
24 "Color 15 of default color lookup table"
25 "Color 16 of default color lookup table"
$ message 26 has been deleted and replaced with message 54
26 ""
27 "Address of Bus Memory used for DMA channel 1"
28 "Arbitration level for dma channel 1"
29 "Address of Bus Memory used for DMA channel 2"
30 "Arbitration level for dma channel 2"
31 "GXT1000's Configuration Method"
32 "Number of Address domains"
33 "GXT1000's PIO space"
34 "CCM microcode(not applicable for GXT1000)"
35 "CCM device driver"
36 "CCM device driver interrupt handler"
37 "GXT1000's CDD entry point"
38 "GXT1000's CP microcode"
39 "GXT1000's AP microcode"
40 "GXT1000's Phigs Top Node microcode"
41 "GXT1000's Phigs bottom Node microcode"
42 "GXT1000's GL Top Node microcode"
43 "GXT1000's GL bottom Node microcode"
44 "Graphics Processor Subsystem"
45 "Raster Subsystem"
46 "Video Output Option"
47 "VRAM options on RSS"
48 "GXT1000's Features"
49 "Screen width in pixels"
50 "Screen height in pixels"
51 "Raster Subsystem with Advanced Graphics Feature"
52 "Raster Subsystem"
53 "7250 POWER GXT1000 Graphics Accelerator Model 2"
54 "Which LFT this display belongs to"
$
$ RCM messages
$set 145
1 "Rendering Context Manager Subsystem"
$
$ Ultimedia Video I/O messages
$set 146
1 "Ultimedia Video I/O Adapter"
$
$ Ultimedia Audio Adapter messages
$set 147
1 "Ultimedia Audio Adapter"
$
$ Ultimedia Base Audio for PowerPC messages
$set 148
1 "Ultimedia Integrated Audio" 
$
$  ThinkPad Power 800 Series Video Capture Adapter
$set 149
1 "G10 Graphics with Motion Video Adapter"
2 "Video Camera"
$
$  PCMCIA Bus
$set 150
1 "PCMCIA Bus"
2 "Bus memory address"
3 "Bus I/O address"
4 "Interrupt level"
5 "Interrupt priority"
$
$  PCMCIA Serial Ports
$set 151
1 "PCMCIA Fax/Modem Card"
2 "PCMCIA High Speed Fax/Modem Card"
3 "Bus I/O address"
4 "Bus I/O address for interrupt arbitration register"
5 "Bus interrupt level"
6 "Interrupt priority"
7 "DMA arbitration level"
8 "Enable DMA"
9 "FIFO Control Register"
10 "TRANSMIT buffer count"
11 "RECEIVE trigger level"
12 "BAUD rate"
$
$set 152
1 IBM 8-Port Async, EIA-232 (ISA)
2 IBM 128-Port Async, EIA-232 (ISA)
3 Bus Memory Address
4 Bus I/O Address
5 Line Speed
6 Cable Type
7 Comm Mode
8 16-Port RAN EIA-232 for 128-Port Adapter
9 "BAUD rate"
10 "Transparent Print ON String"
11 "Transparent Print OFF String"
12 Transparent Print Maximum Characters per Second
13 Transparent Print Maximum Character Packet Size
14 Transparent Print Printer Buffer Size
15 Force Carrier
16 Perform Cooked Processing in Adapter
17 Use Alternate RJ-45 Pinouts
18 Receive Event Delay Time
19 2200 Flow Control
20 2200 Print Control
$
$set 153
1 Add a IBM 8-Port Async, EIA-232 (ISA) Adapter
2 Change/Show Characteristics of a IBM 8-Port Async, EIA-232 (ISA) Adapter
3 Add a IBM 128-Port Async, EIA-232 (ISA) Adapter
4 Change/Show Characteristics of a IBM 128-Port Async, EIA-232 (ISA) Adapter
5 Line 1 Speed
6 Line 1 Cable Type
7     Node 1 Comm Mod"
8     Node 2 Comm Mod"
9     Node 3 Comm Mod"
10    Node 4 Comm Mod"
11 Line 2 Speed
12 Line 2 Cable Type
$
