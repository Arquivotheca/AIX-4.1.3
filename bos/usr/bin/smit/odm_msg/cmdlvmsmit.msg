$ @(#)12 1.3  src/bos/usr/bin/smit/odm_msg/cmdlvmsmit.msg, cmdlvm, bos411, 9428A410j 3/18/94 08:52:24

$
$ COMPONENT_NAME:  (CMDLVM) cmdlvmsmit.msg
$
$ FUNCTIONS: (none)
$
$ ORIGINS: 27
$
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$ Warning -- do not remove the "$quote ~" line below.
$            It is needed to keep gencat from failing when this
$            set is merged with other sets that use other quote characters.

$set 15
$quote ~
$ chlv
001 Logical volume NAME
002 Logical volume TYPE
003 POSITION on physical volume
004 RANGE of physical volumes
005 MAXIMUM NUMBER of PHYSICAL VOLUMES\n  to use for allocation
006 Number of copies allocated for each\n  logical partition
007 Allocate each logical partition copy\n  on a SEPARATE physical \
volume?
008 RELOCATE the logical volume during\n  reorganization?
009 MAXIMUM NUMBER of LOGICAL PARTITIONS
010 PERMISSIONS
011 Enable BAD BLOCK relocation?
012 SCHEDULING POLICY for writing logical\n  partition copies
013 Enable WRITE VERIFY?
014 Logical volume LABEL
015 Rename a Logical Volume
016 CURRENT logical volume name
017 NEW logical volume name
019 no,yes
$ chpv
020 Change Characteristics of a Physical Volume
021 Physical volume NAME
022 Allow physical partition ALLOCATION?
023 Physical volume STATE
025 active,not active
$ chvg
030 Change a Volume Group
031 VOLUME GROUP name
032 Activate volume group AUTOMATICALLY\n  at system restart?
$ cplv
040 Copy a Logical Volume
041 SOURCE logical volume name
042 How is the DESTINATION logical volume specified?
043 DESTINATION logical volume
044 Destination VOLUME GROUP name
045 new logical volume name,overwrite existing logical volume,system assigned logical volume name
$ exportvg
050 Export a Volume Group
$ 031 051 VOLUME GROUP name
$ extendlv
060 Increase the Size of a Logical Volume
061 LOGICAL VOLUME name
062 Number of ADDITIONAL logical partitions
063 PHYSICAL VOLUME names
$ 003 064 POSITION on physical volume
$ 004 065 RANGE of physical volumes
$ 005 066 MAXIMUM NUMBER of PHYSICAL VOLUMES \n  to use for allocation
$ 007 067 Allocate each logical partition copy\n  on a SEPARATE physical \
$ volume?
068 File containing ALLOCATION MAP
$ extendvg
070 Add a Physical Volume to a Volume Group
$ 031 071 VOLUME GROUP name
$ 063 072 PHYSICAL VOLUME names
$ importvg
080 Import a Volume Group
$ 031 081 VOLUME GROUP name
$ 101 082 PHYSICAL VOLUME name
083 ACTIVATE volume group after it is\n  imported?
$ lslv
090 Show Characteristics of a Logical Volume
$ 061 091 LOGICAL VOLUME name
092 List OPTION
093 status,physical volume map,logical partition map
$ lspv
100 List Contents of a Physical Volume
101 PHYSICAL VOLUME name
$ 092 102 List OPTION
103 status,logical volumes,physical partitions
109 List All Physical Volumes
$ lsvg
110 List Contents of a Volume Group
$ 031 111 VOLUME GROUP name
$ 092 112 List OPTION
113 status,logical volumes,physical volumes
119 List All Logical Volumes by Volume Group
$ lsvgall
120 List All Volume Groups
121 List only the ACTIVE volume groups?
$ migratepv
130 Move Contents of a Physical Volume
131 SOURCE physical volume name
132 DESTINATION physical volumes
133 Move only data belonging to this\n  LOGICAL VOLUME?
$ mklv
140 Add a Logical Volume
$ 061 141 LOGICAL VOLUME name
$ 031 142 VOLUME GROUP name
143 Number of LOGICAL PARTITIONS
$ 063 144 PHYSICAL VOLUME names
145 Logical volume name PREFIX
$ 002 146 Logical volume TYPE
$ 003 147 POSITION on physical volume
$ 004 148 RANGE of physical volumes
$ 005 149 MAXIMUM NUMBER of PHYSICAL VOLUMES\n  to use for allocation
150 Number of COPIES of each logical\n  partition
$ 007 151 Allocate each logical partition copy\n  on a SEPARATE physical \
$ volume?
$ 008 152 RELOCATE the logical volume during reorganization?
153 MAXIMUM NUMBER of LOGICAL PARTITIONS
$ 010 154 PERMISSIONS
$ 011 155 Enable BAD BLOCK relocation?
$ 012 156 SCHEDULING POLICY for writing logical\n  partition copies
$ 013 157 Enable WRITE VERIFY?
$ 068 158 File containing ALLOCATION MAP
$ mklvcopy
160 Add Copies to a Logical Volume
$ 061 161 LOGICAL VOLUME name
162 NEW TOTAL number of logical partition\n  copies
$ 063 163 PHYSICAL VOLUME names
$ 003 164 POSITION on physical volume
$ 004 165 RANGE of physical volumes
$ 005 166 MAXIMUM NUMBER of PHYSICAL VOLUMES \n  to use for allocation
$ 007 167 Allocate each logical partition copy\n  on a SEPARATE physical \
$ volume?
$ 068 168 File containing ALLOCATION MAP
169 SYNCHRONIZE the data in the new\n  logical partition copies?
$ mkvg
170 Add a Volume Group
$ 031 171 VOLUME GROUP name
172 Physical partition SIZE in megabytes
$ 063 173 PHYSICAL VOLUME names
$ 032 174 Activate volume group AUTOMATICALLY\n  at system restart?
175 ACTIVATE volume group after it is\n  created?
176 Volume Group MAJOR NUMBER
$ reducevg
181 Remove a Physical Volume from a Volume Group
$ 031 181 VOLUME GROUP name
$ 063 182 PHYSICAL VOLUME names
183 FORCE deallocation of all partitions on\n  this physical volume?
189 Remove a Volume Group
$ renamelv chlv2
190 Rename a Logical Volume
191 CURRENT logical volume name
192 NEW logical volume name
$ reorgvg
200 Reorganize a Volume Group
$ 031 201 VOLUME GROUP name
202 LOGICAL VOLUME names
$ 063 203 PHYSICAL VOLUME names
$ rmlv
210 Remove a Logical Volume
$ 061 211 LOGICAL VOLUME name
$ rmlvcopy
220 Remove Copies from a Logical Volume
$ 061 221 LOGICAL VOLUME name
222 NEW maximum number of logical partition\n  copies
$ varyoffvg
230 Deactivate a Volume Group
$ 031 231 VOLUME GROUP name
232 Put volume group in SYSTEM\n  MANAGEMENT mode?
$ varyonvg
240 Activate a Volume Group
$ 031 241 VOLUME GROUP name
242 RESET device configuration database?
243 RESYNCHRONIZE stale physical partitions?
244 Activate volume group in SYSTEM\n  MANAGEMENT mode?
245 no,yes,on error
246 FORCE activation of the volume group?\n \
Warning--this may cause loss of data\n \
integrity.
$ Ring display values
250 jfs,jfslog,paging,boot,copy,sysdump
251 edge,middle,center,inner-middle,inner-edge
252 minimum,maximum
253 parallel,sequential
254 read/write,readonly
255 Mirror Write Consistency?
256 Synchronize a Volume Group
257 Change a Logical Volume
