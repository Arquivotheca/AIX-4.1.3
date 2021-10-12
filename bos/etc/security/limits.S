# @(#)86	1.12.1.3  src/bos/etc/security/limits.S, cfgsauth, bos411, 9438B411a 9/21/94 13:44:08
#  COMPONENT_NAME: CFGSAUTH
#
#  ORIGINS: 27
#
#  (C) COPYRIGHT International Business Machines Corp. 1989, 1994
#  All Rights Reserved
#  Licensed Materials - Property of IBM
#
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

*
* Sizes are in multiples of 512 byte blocks, CPU time is in seconds
*
* fsize - maximum file size in blocks
* core  - maximum core file size in blocks
* cpu   - per process CPU time limit in seconds
* data  - maximum data segment size in blocks
* stack - maximum stack segment size in blocks
* rss   - maximum real memory usage in blocks
*
* NOTE:  a value of zero or -1 implies "unlimited"
*

default:
	fsize = 2097151
	core = 2048
	cpu = -1
	data = 262144
	rss = 65536
	stack =	65536

root:

daemon:

bin:

sys:

adm:

uucp:

guest:

nobody:

lpd:

