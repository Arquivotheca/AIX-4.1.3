$ @(#)87 1.7  src/bos/usr/bin/smit/odm_msg/cmdfs.msg, cmdfs, bos411, 9428A410j 9/30/93 20:11:49
$
$ COMPONENT_NAME: (CMDFS) commands that deal with the file system
$
$ FUNCTIONS: cmdfs.msg the cmdfs.add (ie smit) message file
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$

$set 19
$quote "

5 "yes,no"
15 "Verify a File System"
20 "NAME of file system\n\
      -OR-"
25 "TYPE of file system\n"
30 "FAST check?"
35 "SCRATCH file\n\
  (must not be on the file system being checked)"
40 "List All Mounted File Systems"
45 "Mount a File System"
50 "FILE SYSTEM name"
55 "DIRECTORY over which to mount"
60 "FORCE the mount?"
65 "Request an INHERITED mount?"
70 "REMOTE NODE containing the file system\n\
  to mount"
75 "Mount as a REMOVABLE file system?"
80 "Mount as a READ-ONLY system?"
85 "Unmount a File System"
90 "NAME of file system to unmount"
95 "REMOTE NODE containing the file system(s)\n\
  to unmount"
100 "TYPE of file system"
105 "Unmount ALL mounted file systems?\n\
  (except /, /tmp, /usr)\n\
        -OR-"
110 "Unmount all REMOTELY mounted file systems?\n"
150 "Mount a Group of File Systems"
155 "GROUP name"
160 "Unmount a Group of File Systems"
165 "GROUP name"
170 "List All File Systems"
180 "Add a Journaled File System"
185 "Volume Group Name"
195 "Volume group name"
200 "SIZE of file system (in 512-byte blocks)"
205 "MOUNT POINT"
210 "Mount AUTOMATICALLY at system restart?"
215 "PERMISSIONS"
220 "Change / Show Characteristics of a Journaled File System"
225 "File System Name"
235 "File system name"
240 "NEW mount point"
250 "Mount GROUP"
265 "Volume group name"
270 "Remove a Journaled File System"
280 "Add a CDROM File System"
285 "DEVICE name"
300 "Change / Show Characteristics of a CDROM File System"
310 "Add a Journaled File System on a Previously Defined Logical Volume"
320 "LOGICAL VOLUME name"
330 "Remove a CDROM File System"
350 "read only,read/write"
360 "Mount OPTIONS"
370 "Disallow DEVICE access via this mount?"
380 "Disallow execution of SUID and sgid programs\n\
  in this file system?"
390 "Remove Mount Point"
400 "Start Disk Accounting?"
410 "Fragment Size (bytes)"
420 "Number of bytes per inode"
430 "Compression algorithm"
440 "Defragment a Journaled File System"
441 "Perform, Query, or Report ?"
442 "perform,query,report"
$ In messages 441 and 442 the context of perform, query or report is:
$	- perform the defragmentaion operation
$	- Query the current fragmentation state
$	- report on the current fragmentation state and benifits of performing
$	the defragmentaion
