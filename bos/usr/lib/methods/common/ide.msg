$ @(#)77        1.5  src/bos/usr/lib/methods/common/ide.msg, cfgmethods, bos41J, 9516A_all 4/17/95 14:02:38
$
$ COMPONENT_NAME: (CFGMETHODS) device configuration methods
$
$ FUNCTIONS: IDE Devices' Configuration Message Catalog
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
$  ATA/IDE Adapter
$set 1
1 "ATA/IDE Controller Device"
2 "Bus I/O Address"
3 "Bus Interrupt Level"
4 "Interrupt Priority"
5 "DMA Level"
$
$ IDE Tape Drives
$set 2
1 "2.0 GB 1/4-Inch Tape Drive"
2 "Other ATAPI Tape Drive"
3 "Use DEVICE BUFFERS during writes"
4 "BLOCK size"
5 "RETENSION on tape change or reset"
6 "BLOCK SIZE for variable length support"
7 "DENSITY setting #1"
8 "DENSITY setting #2"
$
$ IDE Disk Drives
$set 3
1 "Physical Volume Identifier"
2 "730 MB IDE Disk Drive"
3 "360 MB IDE Disk Drive"
4 "540 MB IDE Disk Drive"
5 "720 MB IDE Disk Drive"
6 "1.0 GB IDE Disk Drive"
7 "Other IDE Disk Drive"
8 "READ/WRITE Time-out Value"
10 "START Time-out Value"
11 "IDE Disk Model Name"
$
$ IDE CDROM
$set 4
1 "IDE CD-ROM Drive"
2 "Other IDE CD-ROM Drive"
3 "PREVENT ejection of media when open"
4 "START UNIT time out value"
5 "READ time out value"
6 "Play AUDIO supported"
7 "MODE 2 FORM 1 supported"
8 "MODE 2 FORM 2 supported"
9 "CD-DA supported"
$
$ IDE SMIT Help Messages
$set 5
1 "For all IDE devices, including disks, CD-ROMs, and tapes, the\n\
location code format is AA-BB-CC-DD.  The AA-BB fields identify the\n\
location of the ATA/IDE adapter controlling the IDE device.\n\
\n\
The CC field specifies the adapter to which the device is attached.\n\
A value of 00 indicates the primary adapter driver, while a value\n\
of 01 indicates the secondary adapter driver.\n\
\n\
The DD field is the IDE device identification number used by the\n\
adapter driver to uniquely address the device.  No more than two\n\
devices may be controlled by a given adapter driver.  Valid values\n\
for this field are 00 and 01."
$
2 "Specifies the IDE device identification number used by the\n\
adapter driver to uniquely address the device.  No more than two\n\
devices may be controlled by a given adapter driver.  Valid\n\
values for this field are 0 and 1."
$
3 "Indicates the maximum number of seconds the system will allow\n\
the device to finish processing for a read ATAPI command. If a read\n\
ATAPI command takes longer than this time, the system initiates\n\
error recovery actions. Do not change this attribute unless you are\n\
familiar with ATAPI commands and the given IDE device. If the value\n\
for this attribute is too small, unnecessary error recovery actions\n\
occur and system performance will be degraded.  If the value of this\n\
attribute is too large, defective devices may cause lengthy delays."
$
4 "Indicates the maximum number of seconds the system will allow\n\
the device to finish processing for a start unit ATAPI command. If\n\
a start unit ATAPI command takes longer than this time, the system\n\
initiates error recovery actions. Do not change this attribute\n\
unless you are familiar with ATAPI commands and the given IDE\n\
device. If the value for this attribute is too small, unnecessary\n\
error recovery actions occur and system performance will be degraded.\n\
If the value of this attribute is too large, defective devices may\n\
cause lengthy delays."
$
5 "Indicates the device supports the ATAPI Play Audio commands for\n\
CD-ROM. The no value indicates the device does not support these\n\
ATAPI commands. Do not change this attribute without understanding\n\
ATAPI commands and the given device."
$
6 "Indicates the device supports the CD-DA data format when using\n\
the READ CD ATAPI command. The device can therefore support reads\n\
of a CD-DA disc. The no value indicates the device does not support\n\
reading of a CD-DA disc. Do not change this attribute without\n\
understanding ATAPI commands and the given device."
$
7 "Indicates the maximum number of seconds the system will allow\n\
the device to finish processing for an ATA read or write command. If\n\
the ATA command takes longer than this time, the system initiates\n\
error recovery actions.  Do not change this attribute unless you are\n\
familiar with ATA commands and the given IDE device.  If the value\n\
for this attribute is too small, unnecessary error recovery actions\n\
occur and system performance will be degraded.  If the value of this\n\
attribute is too large, defective devices may cause lengthy delays."
$
8 "Indicates the maximum number of seconds the system will allow\n\
the device to finish processing the first command after activation.\n\
If the first command issued after activation takes longer than this\n\
time, the system initiates error recovery actions.  Do not change this\n\
attribute unless you are familiar with ATA commands and the given IDE\n\
device.  If the value for this attribute is too small, unnecessary\n\
error recovery actions occur and system performance will be degraded.\n\
If the value of this attribute is too large, defective devices may\n\
cause lengthy delays."
