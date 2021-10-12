# @(#)93  1.6  src/bos/kernext/ient/devices.mca.8ef2.com.err.S, sysxient, bos411, 9439A411b 9/26/94 16:34:30
#
#   COMPONENT_NAME: SYSXIENT
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
 
+ IENT_TMOUT:
    Class = H
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "Integrated Ethernet adapter command timed out"
    Err_Desc = 1010
    Prob_Causes = 3111, 0443, 3330 
    Fail_Causes = 0403 
    Fail_Actions = 0012, 0302, EA01, 0000
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 11, HEX
    Detail_Data = 6, 51, HEX
    Detail_Data = 12, 11, HEX

+ IENT_FAIL:
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "Integrated Ethernet adapter hardware failure"
    Err_Desc = 1010
    Prob_Causes = 3302, E890, E89D
    Fail_Causes = 10C8, 10C7, 12B7, 12BB
    Fail_Actions = EA01, 0404, 0000
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 11, HEX
    Detail_Data = 6, 51, HEX
    Detail_Data = 12, 11, HEX
 
+ IENT_DMAFAIL:
    Class = H
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "system/ethernet adapter DMA error"
    Err_Desc = EB75
    Prob_Causes = 3111, EB0A, 3322
    Fail_Causes = F080, 0005, 0443
    Fail_Actions = EA01, 1004, 0000
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 11, HEX
    Detail_Data = 6, 51, HEX
    Detail_Data = 12, 11, HEX
 
+ IENT_PIOFAIL:
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "Integrated Ethernet adapter bus I/O failure"
    Err_Desc = E874
    Prob_Causes = EB0A, 0403, 1012
    Fail_Causes = 0005, 1040, E937
    Fail_Actions = EA01, 1004, 0000
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 11, HEX
    Detail_Data = 6, 51, HEX
    Detail_Data = 12, 11, HEX

+ IENT_SOFTFAIL:
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "Integrated Ethernet adapter software failure"
    Err_Desc = 1002
    Prob_Causes = 3340, E911, EA00
    Fail_Causes = 0413, 3322, E98F
    Fail_Actions = EA01, 1004, 0000
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 11, HEX
    Detail_Data = 6, 51, HEX
    Detail_Data = 12, 11, HEX
