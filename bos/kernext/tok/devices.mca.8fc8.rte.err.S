# @(#)48	1.8  src/bos/kernext/tok/devices.mca.8fc8.rte.err.S, sysxtok, bos41B, 412_41B_sync 12/19/94 08:50:06
#
#   COMPONENT_NAME: SYSXTOK
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
*
* errids.desc
* error logging description file
*
* errupdate reads this file at system installation time
*  and fills in the ODM templates.
*
* The message corresponding to each codepoint is in codepoint.desc
*
 
+ CTOK_ADAP_CHECK:
    Alert = TRUE
    Comment = "adapter error"
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Err_Desc = 1010
    Prob_Causes = 3330, 3331
    Fail_Causes = 3320
    Fail_Actions = 0000, 1803
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 20, 10, HEX
    Detail_Data = 18, 11, HEX
    Detail_Data = 30, 11, HEX
    Detail_Data = 12, 11, HEX

+ CTOK_ADAP_OPEN:
    Alert = TRUE
    Comment = "open failure"
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Err_Desc = 3211
    Prob_Causes = 3702
    Fail_Causes = 3320, 3434, 3711
    Fail_Actions = 0000, 2010, 3101
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 20, 10, HEX
    Detail_Data = 18, 11, HEX
    Detail_Data = 30, 11, HEX
    Detail_Data = 12, 11, HEX
 
+ CTOK_AUTO_RMV:
    Alert = TRUE
    Comment = "auto removal"
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Err_Desc = 3213
    Prob_Causes = 3320
    Fail_Causes = 3320, 3711, 3434
    Fail_Actions= 0000, 2010, 3101
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 20, 10, HEX
    Detail_Data = 18, 11, HEX
    Detail_Data = 30, 11, HEX
    Detail_Data = 12, 11, HEX
 
+ CTOK_CONFIG:
    Alert = TRUE
    Comment = "configuration or customization error"
    Class = S
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Err_Desc = 8000
    Prob_Causes = 7003
    Fail_Causes = 3703
    Fail_Actions = 0133, 3101
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 20, 10, HEX
    Detail_Data = 18, 11, HEX
    Detail_Data = 30, 11, HEX
    Detail_Data = 12, 11, HEX
 
+ CTOK_DEVICE_ERR:
    Alert = TRUE
    Comment = "device error"
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Err_Desc = 1002
    Prob_Causes = 0005, 3220
    Fail_Causes = 0005, 3320
    Fail_Actions = 0000, 0400, 1803
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 20, 10, HEX
    Detail_Data = 18, 11, HEX
    Detail_Data = 30, 11, HEX
    Detail_Data = 12, 11, HEX
 
+ CTOK_DOWNLOAD:
    Alert = TRUE
    Comment = "microcode program abnormally terminated"
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Err_Desc = 6000
    Prob_Causes = 3331, 3330
    Fail_Causes = 3320
    Fail_Actions = 0000, 1200, 1500, 1803
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 20, 10, HEX
    Detail_Data = 18, 11, HEX
    Detail_Data = 30, 11, HEX
    Detail_Data = 12, 11, HEX

+ CTOK_DUP_ADDR:
    Alert = FALSE
    Comment = "duplicate MAC address on TR"
    Class = S
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Err_Desc = 321B
    Prob_Causes = 3704
    Fail_Causes = 3703
    Fail_Actions = 0133, 3101
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 20, 10, HEX
    Detail_Data = 18, 11, HEX
    Detail_Data = 30, 11, HEX
    Detail_Data = 12, 11, HEX
 
+ CTOK_MEM_ERR:
    Alert = FALSE
    Comment = "out of memory"
    Class = S
    Err_Type = UNKN
    Report = TRUE
    Log = TRUE
    Err_Desc = 5008
    Prob_Causes = 1012
    Fail_Causes = 1000
    Fail_Actions = 0000
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 20, 10, HEX
    Detail_Data = 18, 11, HEX
    Detail_Data = 30, 11, HEX
    Detail_Data = 12, 11, HEX
 
+ CTOK_PERM_HW:
    Alert = TRUE
    Comment = "initialization failure"
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Err_Desc = 3210
    Prob_Causes = 3320, 3702, 3701
    Fail_Causes = 3712, 3701, 2600
    Fail_Actions= 0000, 3101, 1803
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 20, 10, HEX
    Detail_Data = 18, 11, HEX
    Detail_Data = 30, 11, HEX
    Detail_Data = 12, 11, HEX
 
+ CTOK_RCVRY_EXIT:
    Alert = TRUE
    Comment = "problem resolved"
    Class = H
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Err_Desc = A000
    Prob_Causes = FE00
    Fail_Causes = 3320
    Fail_Actions = 0100
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 20, 10, HEX
    Detail_Data = 18, 11, HEX
    Detail_Data = 30, 11, HEX
    Detail_Data = 12, 11, HEX
 
+ CTOK_RMV_ADAP:
    Alert = TRUE
    Comment = "remove adapter command received"
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Err_Desc = 3214
    Prob_Causes = 3705
    User_Causes = 7101
    User_Actions= 2010, 3101
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 20, 10, HEX
    Detail_Data = 18, 11, HEX
    Detail_Data = 30, 11, HEX
    Detail_Data = 12, 11, HEX

+ CTOK_WIRE_FAULT:
    Alert = TRUE
    Comment = "wire fault"
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Err_Desc = 3212
    Prob_Causes = 3702
    Fail_Causes = 3434, 3400, 3711, 3320
    Fail_Actions= 0302, 2010, 3101
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 20, 10, HEX
    Detail_Data = 18, 11, HEX
    Detail_Data = 30, 11, HEX
    Detail_Data = 12, 11, HEX
