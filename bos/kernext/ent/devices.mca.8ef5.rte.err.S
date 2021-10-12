# @(#)26        1.2  src/bos/kernext/ent/devices.mca.8ef5.rte.err.S, sysxent, bos411, 9439A411b 9/26/94 16:13:59
#
#   COMPONENT_NAME: SYSXENT
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
 
+ EN3COM_TMOUT:
    Class = H
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "ethernet adapter command timed out"
    Err_Desc = 1010
    Prob_Causes = 0443, 3330, 3331
    Fail_Causes = 3322
    Fail_Actions = 0000
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 11, HEX
    Detail_Data = 6, 51, HEX
    Detail_Data = 12, 11, HEX

+ EN3COM_FAIL:
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "ethernet adapter hardware failure"
    Err_Desc = 1010
    Prob_Causes = 3330, 3331
    Fail_Causes = 3322
    Fail_Actions = 0000
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 11, HEX
    Detail_Data = 6, 51, HEX
    Detail_Data = 12, 11, HEX
 
+ EN3COM_UCODE:
    Class = H
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "ethernet adapter firmware error"
    Err_Desc = 1010
    Prob_Causes = 3330, 3331
    Fail_Causes = 3322
    Fail_Actions = 0000
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 11, HEX
    Detail_Data = 6, 51, HEX
    Detail_Data = 12, 11, HEX
 
+ EN3COM_PARITY:
    Class = H
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "ethernet adapter parity error"
    Err_Desc = 1010
    Prob_Causes = 3330
    Fail_Causes = 3322
    Fail_Actions = 0000
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 11, HEX
    Detail_Data = 6, 51, HEX
    Detail_Data = 12, 11, HEX
 
+ EN3COM_DMAFAIL:
    Class = H
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "system/ethernet adapter DMA error"
    Err_Desc = 1603
    Prob_Causes = 1020, 3322
    Fail_Causes = 0005, 0403, 0443
    Fail_Actions = 0000
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 11, HEX
    Detail_Data = 6, 51, HEX
    Detail_Data = 12, 11, HEX
 
+ EN3COM_NOBUFS:
    Class = S
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "system resource problem"
    Err_Desc = FE01
    Prob_Causes = 8000
    Fail_Causes = 1000
    Fail_Actions = 0001
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 11, HEX
    Detail_Data = 6, 51, HEX
    Detail_Data = 12, 11, HEX
 
+ EN3COM_PIOFAIL:
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "ethernet adapter bus I/O failure"
    Err_Desc = 1603
    Prob_Causes = 1020, 3322
    Fail_Causes = 0005, 0403, 0443
    Fail_Actions = 0000
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 11, HEX
    Detail_Data = 6, 51, HEX
    Detail_Data = 12, 11, HEX
