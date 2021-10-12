$ @(#)98	1.4  src/bos/usr/lpp/blkmux/helpmux.msg, sysxcat, bos411, 9428A410j 12/6/93 16:21:28
$
$ COMPONENT_NAME: (370pinst) Temporary message file for Block Mux Channel.
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregrated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1992
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or 
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote "
$set 1
 

$ These messages are for SMIT help text. They may be replaced by hooks into
$ INFOEXPLORER whenever this becomes available.


1 "Receive Data Transfer OFFSET \n\n\
The device driver reserves as many bytes (as specified by this \n\
value) at the head of any data packet it receives.  This may \n\
facilitate the addition of packet header information by the \n\
application reading data.\n"

2 "Channel Speed \n\n\
This attribute is used to specify the channel speed supported by \n\
the host.  Select a value that coincides with the current host \n\
channel speed.  Valid choices are: 0 (DCI), 2 (2.7 MB Data \n\
Streaming), 4 (4.5 MB Data Streaming).\n"

3 "Number of Transmit Buffers \n\n\
This attribute specifies the number of adapter buffers allocated \n\
for transmits to the host.  The default value is optimized for \n\
general use where the number of transmits is roughly equal to the \n\
number of transmits.\n"

4 "Number of Receive Buffers \n\n\
This attribute specifies the number of adapter buffers allocated \n\
for receivces from the host.  The default value is optimized for \n\
general use where the number of receives is roughly equal to the \n\
number of transmits.\n"

5 "Transmit Buffer Size \n\n\
This attribute specifies the size of the adapter buffers reserved \n\
for transmits.  The default value is optimized for the operating \n\
system's internal storage allocation and must not be modified \n\
without careful consideration of how the application, device driver, \n\
and operating system all work.  For TCP/IP support, this buffer must \n\
be equal to or smaller than the read buffer size specified at the \n\
S/370 of S/390 host.\n"

6 "Receive Buffer Size \n\n\
This attribute specifies the size of the adapter buffers reserved \n\
for receives.  The default value is optimized for the operating \n\
system's internal storage allocation and must not be modified without \n\
careful consideration of how the application, device driver, and \n\
operating system all work.  For TCP/IP support, this buffer must be \n\
equal to or smaller than the write buffer size specified at the S/370 \n\
of S/390 host.\n"

7 "Starting Subchannel Address \n\n\
This attribute determines the first subchannel in a range of \n\
subchannels that may be started and used by application programs.\n"

8 "Number of Subchannel Addresses \n\n\
This attribute determines the number of valid subchannels in a \n\
range that may be started and used by application programs.\n"

9 "CLAW Mode Subchannel Set \n\n\
This attribute is a list of subchannels that will be operated in \n\
the CLAW mode protocol.  Each value represents the lower and even \n\
value of a pair of consecutive subchannels.  This subchannel set \n\
must be within the range of the previously specified subchannel \n\
addresses.  The CLAW subchannel set does not have to be consecutive. \n\
For multiple CLAW mode subchannel sets to support multiple \n\
applications, they must all be specified here.  For example, if \n\
TCP/IP is supported, the subchannel address parameter of the Minimum \n\
Configuration and Startup panel must be specified here.\n"

10 "Online/Offline Switch \n\n\
Setting this attribute to offline will take the adapter offline \n\
from the host's point of view and temporarily restrict users from \n\
starting any subchannels.  Setting this attribute back to online \n\
does allow access again, but does not bring the adapter online until \n\
a subchannel is started again.\n"

11 "Online/Offline Indicator \n\n\
This indicates the true Online/Offline status of the adapter.  The \n\
Online/Offline switch may be set to Offline, but there may be a delay \n\
before the indicator show Offline, because several channel conditions \n\
must be satisfied before the adapter can go Offline.\n"

12 "CLAW Mode Host NAME \n\n\
This defines the unique Host name of the s/370 or s/390.  The name \n\
is used for system validation.\n"

13 "CLAW Mode Adapter NAME \n\n\
This defines the unique Adapter name of the system unit and is used \n\
for system validation.\n"

14 "CLAW Mode \n\n\
Common Link Access to Workstation (CLAW) is a low-level protocol \n\
between the address space on the S/370 or S/390 system and the device \n\
support on the channel attachment card in the system unit. \n\n\
CLAW provides a reliable connection protocol between two consenting \n\
applications, one on a S/370 or S/390 system and one running in the \n\
workstation.  Multiple applications may be running in both the S/370 \n\
or S/390 and the workstation.\n"

$ new set of messages for new smit panels parts
$set 2

1 "\tHost NAME"
2 "\tAdapter NAME"
3 "\tsubchannel set"
4 "CLAW Mode"
