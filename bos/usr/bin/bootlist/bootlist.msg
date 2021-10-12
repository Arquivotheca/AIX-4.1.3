$ @(#)20	1.4.3.1  src/bos/usr/bin/bootlist/bootlist.msg, cmdcfg, bos41J, 9522A_b 5/31/95 11:34:29
$ 
$  COMPONENT_NAME: (CMDCFG) Generic config support cmds
$ 
$  FUNCTIONS: bootlist.msg
$ 
$  ORIGINS: 27
$ 
$  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$  combined with the aggregated modules for this product)
$                   SOURCE MATERIALS
$  (C) COPYRIGHT International Business Machines Corp. 1989
$  All Rights Reserved
$ 
$  US Government Users Restricted Rights - Use, duplication or
$  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "
$set 1
1	"Usage: bootlist -m mode [-r] [-i]|[[-f file] device1 device2...deviceN]\n"
2	"\n------------------- NORMAL IPLIST at address 0x%x ---------------\n"
3	"\n------------------- SERVICE IPLIST at address 0x%x --------------\n"
4	"\n------------------- PREVBOOT LIST at address 0x%x ---------------\n"
5	"%d bytes were written to address 0x%x in %s\n"

$set 2
1	"0514-201 bootlist: Usage error. Conflicting flags (%s)\n"
2	"0514-202 bootlist: Invalid device name (%s)\n"
3	"0514-203 bootlist: Invalid mode (%s)\n"
4	"0514-204 bootlist: There is not enough room in the list for %s\n"
5	"0514-205 bootlist: IOCTL system call returned errno %d\n"
6	"0514-206 bootlist: Unable to open file '%s'\n"
7	"0514-207 bootlist: Unable to create file '%s'\n"
8	"0514-208 bootlist: Usage error (%s)\n"
9	"0514-209 bootlist: Was not able to access the %s object class\n"
10	"0514-210 bootlist: Device %s is not in the AVAILABLE state\n"
11	"0514-211 bootlist: No parent found for device %s\n"
12	"0514-212 bootlist: The connwhere field of CuDv has changed (it is now unrecognizable)\n"
13	"0514-213 bootlist: Unknown disk type %s\n"
14 "0514-214 bootlist: Illegal combination of attributes for %s\n",
15 "0514-215 bootlist: invalid attribute (%s)\n",
16 "0514-216 bootlist: attribute specified more than once (%s)\n",
17 "0514-200 bootlist: Usage error. Flag or parameter is missing (%s)\n"
18 "0514-217 bootlist: hardware address must be 12 hex digits; seperators are not allowed\n"
19 "0514-218 bootlist: the device %s cannot be used to boot from because\n\tthe system ROS does not support it\n"
20 "0514-219 bootlist: This model does not support boot device lists\n"
21 "0514-220 bootlist: Invalid mode (%s) for this model\n"
22 "0514-221 bootlist: Unable to make boot device list for %s\n"
23 "0514-222 bootlist: Unable to write boot device list to NVRAM\n"
