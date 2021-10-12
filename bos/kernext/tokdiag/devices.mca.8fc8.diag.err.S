* @(#)90        1.2  src/bos/kernext/tokdiag/devices.mca.8fc8.diag.err.S, diagddtok, bos411, 9428A410j 10/28/93 11:16:15
*
*   COMPONENT_NAME: DIAGDDTOK
*
*   FUNCTIONS: error description stanza file
*
*   ORIGINS: 27
*
*
*   (C) COPYRIGHT International Business Machines Corp. 1993
*   All Rights Reserved
*   Licensed Materials - Property of IBM
*   US Government Users Restricted Rights - Use, duplication or
*   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*


*
* errids.desc
* error logging description file
*
* errupdate reads this file at system installation time
*  and fills in the ODM templates.
*
* The message corresponding to each codepoint is in codepoint.desc
*

*
*  Token-Ring LAN Alert 5
*
+ TOK_ERR5:
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "permanent adapter hardware error"
    Err_Desc = 3211
    Prob_Causes = 3702, 3701, 3320
    Fail_Causes = 3712, 3701, 2600
    Fail_Actions= 2010, 3101, 1803
    Detail_Data = 168, 11, HEX
    Detail_Data = 20, 8001, HEX


*
*  Token-Ring LAN Alert 6
*       The adapter detected a wire-fault condition on the ring.
*
+ TOK_WIRE_FAULT:
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "wire-fault condition on the ring"
    Err_Desc = 3212
    Prob_Causes = 3702
    Fail_Causes = 3711, 3434, 3320, 3400
    Fail_Actions= 0302, 2010, 3101, 0105
    Detail_Data = 168, 11, HEX
    Detail_Data = 20, 8001, HEX



*
*  Token-Ring LAN Alert 7
*       The adapter was a member of the beacon fault domain and removed
*       itself to perform a self test, which was unsuccessful.
*
+ TOK_AUTO_RMV:
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "adapter auto-remove error"
    Err_Desc = 3213
    Prob_Causes = 3702
    Fail_Causes = 3711, 3434, 3320
    Fail_Actions= 2010, 3101, 0105
    Detail_Data = 168, 11, HEX
    Detail_Data = 20, 8001, HEX



*
*  Token-Ring LAN Alert 8
*       The adapter received a Remove Adapter command from a LAN
*       manager and left the LAN.
*
+ TOK_RMV_ADAP2:
    Class = S
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "Remove Adapter from LAN Manager"
    Err_Desc = 3214
    Prob_Causes = 7013
    User_Causes = 7101
    User_Actions= 2010, 3101, 0105
    Detail_Data = 168, 11, HEX
    Detail_Data = 20, 8001, HEX



*
*  Token-Ring LAN Alert 9
*       The ring has been beaconing for a time longer than the
*       hard-error detection timer.
*
+ TOK_BEACON2:
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "Ring beaconing"
    Err_Desc = 3215
    Prob_Causes = 3703
    Fail_Causes = 3703
    Fail_Actions= 2010, 3101, 0105
    Detail_Data = 168, 11, HEX
    Detail_Data = 20, 8001, HEX


*
*  Token-Ring LAN Alert 10
*       The ring was in a beaconing condition for a time shorter than
*       the hard-error detection timer.
*
+ TOK_ERR10:
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "permanent adapter hardware error"
    Err_Desc = 3330
    Prob_Causes = 3320
    Fail_Causes = 3321, 3713, 3435
    Fail_Actions= 2010, 3101, 0105
    Detail_Data = 168, 11, HEX
    Detail_Data = 20, 8001, HEX


*
*  Token-Ring LAN Alert 12
*       The ring error monitor (REM) has detected
*       excessive soft errors for the ring.
*
+ TOK_ESERR:
    Class = S
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "excessive Soft Errors for the ring"
    Err_Desc = 4001
    Prob_Causes = 3703
    Fail_Causes = 3703, 1022
    Fail_Actions= 2010, 3101
    Detail_Data = 168, 11, HEX


*
*  Token-Ring adapter check
*
+ TOK_ADAP_CHK:
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "Token-Ring adapter check"
    Err_Desc = 3302
    Prob_Causes = 3330, 3331
    Fail_Causes = 3324
    Fail_Actions = 0000, 1803
    Detail_Data = 168, 11, HEX

*
*  Token-Ring microcode download failed
*
+ TOK_DOWNLOAD:
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "microcode download failed"
    Err_Desc = 6000
    Prob_Causes = 3331, 3330
    Fail_Causes = 3324
    Fail_Actions = 0000, 1200, 1500, 1803
    Detail_Data = 168, 11, HEX

*
*  Token-Ring caught unknown system error
*
+ TOK_ERR15:
    Class = H
    Err_Type = UNKN
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "unknown system error caught by Token-Ring"
    Err_Desc = 1010
    Prob_Causes = 3330, 3331
    Fail_Causes = 3324
    Fail_Actions = 0000, 1200
    Detail_Data = 168, 11, HEX

*
*  Token-Ring has entered Network Recovery Mode
*
+ TOK_RCVRY_ENTER:
    Class = H
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "Token-Ring has entered Network Recovery Mode"
    Err_Desc = 1010
    Prob_Causes = 3323, 3320, 3330, 3331
    Fail_Causes = 3220, 3324
    Fail_Actions = 0302, 1206, 0000
    Detail_Data = 168, 11, HEX

*
*  Token-Ring exited Network Recovery Mode successfully
*
+ TOK_RCVRY_EXIT:
    Class = H
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "Successful exit of Network Recovery Mode"
    Err_Desc = A000
    Prob_Causes = FE00
    Fail_Causes = 3320
    Fail_Actions = 0100
    Detail_Data = 168, 11, HEX

*
*  Token-Ring terminated Network Recovery Mode
*
+ TOK_RCVRY_TERM:
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "Token-Ring has terminated Network Recovery Mode"
    Err_Desc = 1010
    Prob_Causes = 3323, 3320, 3330, 3331
    Fail_Causes = 3220, 3324
    Fail_Actions = 0302, 1206, 0000
    Detail_Data = 168, 11, HEX

*
*  Token-Ring detected a Micro Channel error
*
+ TOK_MC_ERR:
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "Token-Ring detected a Micro Channel error"
    Err_Desc = 1010
    Prob_Causes = 0005, 3220
    Fail_Causes = 0005, 3320
    Fail_Actions = 0000, 0400
    Detail_Data = 168, 11, HEX

*
*  Token-Ring detected a PIO error
*
+ TOK_PIO_ERR:
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "Token-Ring detected a PIO error"
    Err_Desc = 1010
    Prob_Causes = 0005, 3220
    Fail_Causes = 3320, 0005
    Fail_Actions = 0000, 0400
    Detail_Data = 168, 11, HEX

*
*  Token-Ring transmit error
*
+ TOK_TX_ERR:
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "Token-Ring transmit error"
    Err_Desc = 1010
    Prob_Causes = 0005, 3220
    Fail_Causes = 0005, 3320
    Fail_Actions = 0000, 0400
