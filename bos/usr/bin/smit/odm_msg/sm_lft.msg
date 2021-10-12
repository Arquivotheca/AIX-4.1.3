$ @(#)35	1.3.1.1  src/bos/usr/bin/smit/odm_msg/sm_lft.msg, lftdd, bos411, 9430C411a 7/26/94 10:17:46
$
$   COMPONENT_NAME: lftdd
$
$   FUNCTIONS: none
$
$   ORIGINS: 27
$
$   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$   combined with the aggregated modules for this product)
$                    SOURCE MATERIALS
$
$   (C) COPYRIGHT International Business Machines Corp. 1994
$   All Rights Reserved
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$set 7
$quote "

51    "Move the LFT to Another Display"
52    "Display Mnemonic"
53    "Take Effect Now, Next IPL, Both"
54    "Now,Next IPL,Both"
79    "Select the Active Font for Next System Restart"
80    "Font Identifier"
91    "Add a Font to the System"
92    "PATHNAME of Font File"
98    "Change the Keyboard Map for Next System Restart"
99    "PATHNAME of Keyboard Map"
107   "Select the Display"
108   "Select the Display Size"
109   "Display"
110   "Display screen WIDTH (in millimeters)"
111   "Display screen HEIGHT (in millimeters)"
124   "Select the Display Refresh Rate"
125   "Display Refresh RATE (in hertz)"
126   "\n\
WARNING: Make sure the selected monitor supports \n\
         the refresh rate chosen before executing \n\
         this command or the monitor will not be \n\
         readable. The selected refresh rate will \n\
         not take effect until the next system \n\
         reboot."
130   "List All Graphics Adapters Available"
131   "Select the Display Type"
132   "Select the Display Resolution and Refresh Rate"
133   "Graphics Adapter"
134   "Display Type"
135   "Display Power Management"
136   "Time before display dimmed (in minutes)"
137   "Time before display power suspended (in minutes)"
138   "\n\n\
       To disable Display Power Management, \n\
       set the dimmed time to zero.  Note \n\
       all changes will take effect next IPL."
139   "Time before display power turned off (in minutes)"
