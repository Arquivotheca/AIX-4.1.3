$ @(#)93        1.3.2.3  src/bos/usr/lib/methods/common/devices_tok.msg, cfgmethods, bos41J, 9516A_all 4/14/95 11:37:24
$
$ COMPONENT_NAME: (CFGMETHODS) device configuration methods
$
$ FUNCTIONS: Token-ring Devices configuration message catalog
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
$  Token-Ring
$
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
20 "Token-Ring High-Performance Adapter (8fa2)"
21 "Bus memory address"
22 "Number of ALTERNATE TOKEN RING addresses"
23 "Priority Transmit"
24 "CFG RING speed"
$ 
$ help messages for the smit objects 
$
25 "Indicates the ring speed of the Token-Ring network to which the \n\
adapter is attached.  This attribute must be set to match the speed \n\
at which the network is currently running.  An incorrect value can \n\
cause the network to become inoperable.  Valid values are 4 for 4 Mbps \n\
ring, 16 for a 16Mbps ring, and autosense for don't care.\n\n\
When autosense option is selected, the adapter will listen to the \n\
network ring speed and set accordingly to that speed.  If the adapter \n\
is the first adapter insert onto the network then it will be default \n\
to 16 Mbps.\n\n\
Note: If TCP/IP is running, use the smit mktcpip fast path to change \n\
      the ring speed of the token-ring adapter."
$
30 "IBM 16/4 PowerPC Token-Ring Adapter (isa)"
31 "Address of bus memory used for BIOS"
32 "Width of shared bus memory"
$
$ help messages for ISA Token-ring smit objects
$
33 "The IBM 16/4 PowerPC Token-Ring Adapter allows you the flexibility \n\
of setting the interrupt level according to the other adapter installed \n\
in your computer. You should select an interrupt level for the adapter \n\
that does not conflict with another adapter in your system and make sure \n\
that the switches (7,8) on the adapter is set correctly."
$
34 "The IBM 16/4 PowerPC Token-Ring Adapter allows you the flexibility \n\
of setting the 8 Kbytes ROM address on the adapter. You should set the \n\
ROM address so that they do not conflict with any other adapter addresses \n\
already set in your system and make sure that the switches (1-6) on the \n\
adapter is set correctly."
$
35 "There are two possible Bus I/O addresses for the IBM 16/4 PowerPC \n\
Token-Ring Adapter: 0xa20 (primary) and 0xa24 (alternate). This value \n\
is corresponding to the setting of the Primary-Alternate switch (9) on \n\
the adapter. If you are installing two 16/4 adapters in your system, set \n\
the first one to primary and the second to alternate."
$
36 "The IBM 16/4 PowerPC Token-Ring Adapter allows you the flexibility \n\
of setting the starting address of the Shared RAM. The Shared RAM size \n\
is selectible by setting switches on the adapter. You should select a \n\
Bus memory address according to the desired Shared RAM size so that \n\
the address space assigned to this adapter does not conflict with the \n\
address space assigned to any other feature on your system. The Bus \n\
memory address must always be at least on a 16KBytes boundary. Please \n\
refer to the 'Width of shared bus memory' help text for the possible \n\
Shared RAM sizes and the address boundary requirements for each RAM \n\
size setting."
$
37 "The IBM 16/4 PowerPC Token-RIng Adapter allows you the flexibility \n\
of selecting the Shared RAM size. You may select a RAM size of 8, 16, \n\
32 or 64 KBytes by setting the switches (10,11) on the adapter. \n\
Please note the address boundary requirements for the desired RAM \n\
size -  The starting address of the Shared RAM must always be at least \n\
oon a 16KBytes boundary. For a 32K selection, the starting address must \n\
be on a 32KBytes boundary. For a 64K selection, the starting address \n\
must be on a 64KBytes boundary. \n\
The address space assigned to this adapter must not conflict with the \n\
address space assigned to any other feature on your system. Please \n\
refer to the 'Bus memory address' attribute to determine the possible \n\
Shared RAM address spaces for your adapter."
38 "IBM PCMCIA 16/4 Token-Ring Card"
39 "IBM PCI Tokenring Adapter (14101800)"
40 "HARDWARE RECEIVE queue size"
41 "PRIORITY TRANSMIT queue size"
$
