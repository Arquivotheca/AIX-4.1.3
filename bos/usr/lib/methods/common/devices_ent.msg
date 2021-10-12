$ @(#)02        1.2.2.1  src/bos/usr/lib/methods/common/devices_ent.msg, cfgmethods, bos41B 1/25/95 11:14:37
$
$ COMPONENT_NAME: (CFGMETHODS) device configuration methods
$
$ FUNCTIONS: Ethernet Devices configuration message catalog
$
$ ORIGINS: 27, 83
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
$quote "
$
$  Ethernet
$
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
21 "Adapter has BNC/DIX hardware jumper"
22 "RECEIVE buffer pool size"
23 "Integrated Ethernet Adapter"
41 "IBM ISA Ethernet Adapter"
42 "Media Type"
$ Do not translate message 43
43 "bnc,dix,tp"
44 "Indicates which one of the external connectors on the adapter is \n\
being used. There are three types of connectors: a BNC (10Base2) \n\
connector selected with the value of bnc, a 15-pin DIX (10Base5) \n\
connector selected with the value of dix, and a twisted-pair (10BaseT)\n\
connector selected with the value of tp. The default value is bnc."
45 "IBM PCMCIA Ethernet Card"
46 "IBM PCI Ethernet Adapter (22100020)"
47 "HARDWARE TRANSMIT queue size"
48 "HARDWARE RECEIVE queue size"
