# @(#)05  1.3 src/bos/kernext/fddi/devices.mca.8ef4.rte.err.S, sysxfddi, bos41J, 9520A_a  5/16/95  11:38:16
#
#   COMPONENT_NAME: sysxfddi
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
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
 
+ CFDDI_RMV_ADAP:
    Comment = "Remove adapter received from LAN manager"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = PERM 
    Err_Desc = EC35
    Prob_Causes = 7013 
    Fail_Causes = EA37 
    Fail_Actions = 3103 
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 12, 8193, HEX
    Detail_Data = 56, EC30, HEX
    Detail_Data = 24, EC31, HEX
    Detail_Data = 16, EC32, HEX


+ CFDDI_ADAP_CHECK:
    Comment = "FDDI adapter check"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = PERM 
    Err_Desc = EC36
    Prob_Causes = 3330, 3331 
    Fail_Causes = 3351 
    Fail_Actions = 0000, 0302, E801 
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 12, 8193, HEX
    Detail_Data = 56, EC30, HEX
    Detail_Data = 24, EC31, HEX
    Detail_Data = 16, EC32, HEX

+ CFDDI_DWNLD:
    Comment = "microcode download failed"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP 
    Err_Desc = EC37
    Prob_Causes = 3330, 3331 
    Fail_Causes = 3351  
    Fail_Actions = 0000, 1200, E801 
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 12, 8193, HEX
    Detail_Data = 56, EC30, HEX
    Detail_Data = 24, EC31, HEX
    Detail_Data = 16, EC32, HEX

+ CFDDI_RCVRY_ENTER:
    Comment = "FDDI has entered network recovery mode"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = PEND 
    Err_Desc = EC38
    Prob_Causes = 3330, 3331 
    Fail_Causes = 2000, 3351
    Fail_Actions = 0700, 3302, 0302, E801
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 12, 8193, HEX
    Detail_Data = 56, EC30, HEX
    Detail_Data = 24, EC31, HEX
    Detail_Data = 16, EC32, HEX

+ CFDDI_RCVRY_EXIT:
    Comment = "Successful exit of network recovery mode"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP 
    Err_Desc = EC39
    Prob_Causes = FE00
    Fail_Causes = 3351  
    Fail_Actions = 0100
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 12, 8193, HEX
    Detail_Data = 56, EC30, HEX
    Detail_Data = 24, EC31, HEX
    Detail_Data = 16, EC32, HEX

+ CFDDI_RCVRY_TERM:
    Comment = "FDDI has terminated network recovery mode"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = PERM 
    Err_Desc = EC3A
    Prob_Causes = 3330, 3331, 1030
    Fail_Causes = 3351
    Fail_Actions = 0000
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 12, 8193, HEX
    Detail_Data = 56, EC30, HEX
    Detail_Data = 24, EC31, HEX
    Detail_Data = 16, EC32, HEX

+ CFDDI_MC_ERR:
    Comment = "A Microchannel error was detected"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = EC3B
    Prob_Causes = 0005, 3351
    Fail_Causes = 0005, 3351
    Fail_Actions = 0000, F011, E801
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 12, 8193, HEX
    Detail_Data = 56, EC30, HEX
    Detail_Data = 24, EC31, HEX
    Detail_Data = 16, EC32, HEX

+ CFDDI_TX_ERR:
    Comment = "A FDDI transmit error was detected"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = EC3C
    Prob_Causes = 3400, 0005, 3351
    Fail_Causes = 3400, 0005, 3351
    Fail_Actions = 0000, F011, 0302, E801
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 12, 8193, HEX
    Detail_Data = 56, EC30, HEX
    Detail_Data = 24, EC31, HEX
    Detail_Data = 16, EC32, HEX

+ CFDDI_PIO:
    Comment = "A FDDI detected a PIO error"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = EC3D
    Prob_Causes = 0005, 3351
    Fail_Causes = 0005, 3351
    Fail_Actions = 0000, F011, E801
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 12, 8193, HEX
    Detail_Data = 56, EC30, HEX
    Detail_Data = 24, EC31, HEX
    Detail_Data = 16, EC32, HEX

+ CFDDI_DOWN:
    Comment = "device down due to unrecoverable error"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = EC3E
    Prob_Causes = 3351, 3331, 0010, 0005
    Fail_Causes = 3351, 3331, 1030, 0005
    Fail_Actions = 0000, E801, 3301, 3000
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 12, 8193, HEX
    Detail_Data = 56, EC30, HEX
    Detail_Data = 24, EC31, HEX
    Detail_Data = 16, EC32, HEX

+ CFDDI_SELF_TEST:
    Comment = "self tests remotely initiated"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = EC3F
    Prob_Causes = 0010
    Fail_Causes = 1030
    Fail_Actions = 3103, 0000, E801
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 12, 8193, HEX
    Detail_Data = 56, EC30, HEX
    Detail_Data = 24, EC31, HEX
    Detail_Data = 16, EC32, HEX

+ CFDDI_SELFT_ERR:
    Comment = "Adapter self tests have failed"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = PERM
    Err_Desc = EC40
    Prob_Causes = 3351, E881, 3331
    Fail_Causes = 3351, 3331
    Fail_Actions = E801, 3300, 3000
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 12, 8193, HEX
    Detail_Data = 56, EC30, HEX
    Detail_Data = 24, EC31, HEX
    Detail_Data = 16, EC32, HEX

+ CFDDI_PATH_ERR:
    Comment = "Adapter path tests have failed"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = PERM
    Err_Desc = EC41
    Prob_Causes = 3351, 3331
    Fail_Causes = 3351
    Fail_Actions = 0000, E801, 3301, 3000
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 12, 8193, HEX
    Detail_Data = 56, EC30, HEX
    Detail_Data = 24, EC31, HEX
    Detail_Data = 16, EC32, HEX

+ CFDDI_PORT:
    Comment = "A port is stuck"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = EC42
    Prob_Causes = 3400, 3351
    Fail_Causes = 3400, 3351
    Fail_Actions = EB60, F011, E801
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 12, 8193, HEX
    Detail_Data = 56, EC30, HEX
    Detail_Data = 24, EC31, HEX
    Detail_Data = 16, EC32, HEX

+ CFDDI_BYPASS:
    Comment = "Optical bypass switch is stuck"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = PERM
    Err_Desc = EC43
    Prob_Causes = E880, 3400, 3351
    Fail_Causes = EA38, 3400, 3351
    Fail_Actions = 0000, F011, 0302, E801
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 12, 8193, HEX
    Detail_Data = 56, EC30, HEX
    Detail_Data = 24, EC31, HEX
    Detail_Data = 16, EC32, HEX

+ CFDDI_CMD_FAIL:
    Comment = "Command to the Adapter has Failed"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = EC44
    Prob_Causes = 3351, 3331
    Fail_Causes = 3351
    Fail_Actions = 0000, F011, E801
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 12, 8193, HEX
    Detail_Data = 56, EC30, HEX
    Detail_Data = 24, EC31, HEX
    Detail_Data = 16, EC32, HEX

+ CFDDI_RCV_ERR:
    Comment = "A FDDI receive error was detected"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = EC45
    Prob_Causes = 3400, 0005, 3351
    Fail_Causes = 3400, 0005, 3351
    Fail_Actions = 0000, F011, 0302, E801
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 12, 8193, HEX
    Detail_Data = 56, EC30, HEX
    Detail_Data = 24, EC31, HEX
    Detail_Data = 16, EC32, HEX
