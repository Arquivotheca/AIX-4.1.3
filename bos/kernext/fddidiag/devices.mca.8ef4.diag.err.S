* @(#)39        1.1  src/bos/kernext/fddidiag/devices.mca.8ef4.diag.err.S, diagddfddi, bos411, 9428A410j 11/4/93 14:41:47
*
*   COMPONENT_NAME: DIAGDDFDDI
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


+ FDDI_RMV_ADAP:
    Comment = "Remove adapter received from LAN manager"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = PERM
    Err_Desc = 3214
    Prob_Causes = 7013
    Fail_Causes = 3214
    Fail_Actions = 3103
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 20, 8194, HEX
    Detail_Data = 118, 8195, HEX
    Detail_Data = 16, 11, HEX

+ FDDI_ADAP_CHECK:
    Comment = "FDDI adapter check"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = PERM
    Err_Desc = 1010
    Prob_Causes = 3330, 3331
    Fail_Causes = 3302
    Fail_Actions = 0000, 0302, E801
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 20, 8194, HEX
    Detail_Data = 118, 8195, HEX
    Detail_Data = 16, 11, HEX

+ FDDI_NOMBUFS:
    Comment = "mbuf request denied"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = FE01
    Prob_Causes = 1020
    Fail_Causes = 1000
    Fail_Actions = FFFF
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 20, 8194, HEX
    Detail_Data = 118, 8195, HEX
    Detail_Data = 16, 11, HEX

+ FDDI_DWNLD:
    Comment = "microcode download failed"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = 6000
    Prob_Causes = 3330, 3331
    Fail_Causes = 3302
    Fail_Actions = 0000, 0302, 1200, E801
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 20, 8194, HEX
    Detail_Data = 118, 8195, HEX
    Detail_Data = 16, 11, HEX

+ FDDI_RCVRY_ENTER:
    Comment = "FDDI has entered network recovery mode"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = PEND
    Err_Desc = E971
    Prob_Causes = 3330, 3331
    Fail_Causes = FFFF
    Fail_Actions = FFFF
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 20, 8194, HEX
    Detail_Data = 118, 8195, HEX
    Detail_Data = 16, 11, HEX

+ FDDI_RCVRY_EXIT:
    Comment = "Successful exit of network recovery mode"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = A000
    Prob_Causes = FE00
    Fail_Causes = FFFF
    Fail_Actions = FFFF
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 20, 8194, HEX
    Detail_Data = 118, 8195, HEX
    Detail_Data = 16, 11, HEX

+ FDDI_RCVRY_TERM:
    Comment = "FDDI has terminated network recovery mode"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = PERM
    Err_Desc = 1010
    Prob_Causes = 3330, 3331, 1030
    Fail_Causes = 3302
    Fail_Actions = 0000, 0302, 1206, E801
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 20, 8194, HEX
    Detail_Data = 118, 8195, HEX
    Detail_Data = 16, 11, HEX

+ FDDI_MC_ERR:
    Comment = "A Microchannel error was detected"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = 1010
    Prob_Causes = 0005, 3302
    Fail_Causes = 0005, 3302
    Fail_Actions = 0000, F011, E801
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 20, 8194, HEX
    Detail_Data = 118, 8195, HEX
    Detail_Data = 16, 11, HEX

+ FDDI_TX_ERR:
    Comment = "A FDDI transmit error was detected"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = 1010
    Prob_Causes = 3400, 0005, 3302
    Fail_Causes = 3400, 0005, 3302
    Fail_Actions = 0000, F011, 0302, E801
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 20, 8194, HEX
    Detail_Data = 118, 8195, HEX
    Detail_Data = 16, 11, HEX

+ FDDI_PIO:
    Comment = "A FDDI detected a PIO error"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = E874
    Prob_Causes = 0005, 3302
    Fail_Causes = 0005, 3302
    Fail_Actions = 0000, F011, E801
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 20, 8194, HEX
    Detail_Data = 118, 8195, HEX
    Detail_Data = 16, 11, HEX

+ FDDI_DOWN:
    Comment = "device down due to unrecoverable error"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = 1010
    Prob_Causes = 3302, 3331, 0010
    Fail_Causes = 3302, 0005, 1031
    Fail_Actions = 0000, E801, 3301, 3000
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 20, 8194, HEX
    Detail_Data = 118, 8195, HEX
    Detail_Data = 16, 11, HEX

+ FDDI_LLC_DISABLE:
    Comment = "LLC services remotely disabled"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = 3200
    Prob_Causes = 0010
    Fail_Causes = 1031
    Fail_Actions = 3103
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 20, 8194, HEX
    Detail_Data = 118, 8195, HEX
    Detail_Data = 16, 11, HEX

+ FDDI_LLC_ENABLE:
    Comment = "LLC services remotely disabled"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = A000
    Prob_Causes = 1030, FE00
    Fail_Causes = FFFF
    Fail_Actions = FFFF
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 20, 8194, HEX
    Detail_Data = 118, 8195, HEX
    Detail_Data = 16, 11, HEX

+ FDDI_SELF_TEST:
    Comment = "self tests remotely initiated"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = 3200
    Prob_Causes = 0010
    Fail_Causes = 1031
    Fail_Actions = 0000, 3103, E801
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 20, 8194, HEX
    Detail_Data = 118, 8195, HEX
    Detail_Data = 16, 11, HEX

+ FDDI_SELFT_ERR:
    Comment = "Adapter self tests have failed"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = PERM
    Err_Desc = 1010
    Prob_Causes = 3330, E881, 3331
    Fail_Causes = 3302
    Fail_Actions = 0000, E801, 3301, 3000
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 20, 8194, HEX
    Detail_Data = 118, 8195, HEX
    Detail_Data = 16, 11, HEX

+ FDDI_PATH_ERR:
    Comment = "Adapter path tests have failed"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = PERM
    Err_Desc = 1010
    Prob_Causes = 3330, 3331
    Fail_Causes = 3302
    Fail_Actions = 0000, E801, 3301, 3000
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 20, 8194, HEX
    Detail_Data = 118, 8195, HEX
    Detail_Data = 16, 11, HEX

+ FDDI_USYS:
    Comment = "Unknown system error detected by FDDI"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = UNKN
    Err_Desc = FE00
    Prob_Causes = FE00
    Fail_Causes = FFFF
    Fail_Actions = FFFF
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 20, 8194, HEX
    Detail_Data = 118, 8195, HEX
    Detail_Data = 16, 11, HEX

+ FDDI_RCV:
    Comment = "FDDI receive error"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = UNKN
    Err_Desc = 1010
    Prob_Causes = 3330, 3331
    Fail_Causes = 3302
    Fail_Actions = 0000, 3301, E801, 3000
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 20, 8194, HEX
    Detail_Data = 118, 8195, HEX
    Detail_Data = 16, 11, HEX

+ FDDI_TRACE:
    Comment = "FDDI Trace max exceeded"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = PERM
    Err_Desc = 1010
    Prob_Causes = 3330, 3331
    Fail_Causes = 3302
    Fail_Actions = 0000, 0302, E801, 3000
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 20, 8194, HEX
    Detail_Data = 118, 8195, HEX
    Detail_Data = 16, 11, HEX

+ FDDI_XCARD:
    Comment = "FDDI Trace max exceeded"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = PERM
    Err_Desc = 1010
    Prob_Causes = E881, 3451, 3330, 3331
    Fail_Causes = 3302
    Fail_Actions = 0000, E801, 3301, 3000
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 20, 8194, HEX
    Detail_Data = 118, 8195, HEX
    Detail_Data = 16, 11, HEX

+ FDDI_PORT:
    Comment = "A port is stuck"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = 1010
    Prob_Causes = 3400, 3302
    Fail_Causes = 3400, 3302
    Fail_Actions = 0000, F011, 0302, E801
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 20, 8194, HEX
    Detail_Data = 118, 8195, HEX
    Detail_Data = 16, 11, HEX

+ FDDI_BYPASS:
    Comment = "Optical bypass switch is stuck"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = PERM
    Err_Desc = 1010
    Prob_Causes = E880, 3400, 3302
    Fail_Causes = EA06, 3400, 3302
    Fail_Actions = 0000, F011, 0302, E801
    Detail_Data = 32, D0, ALPHA
    Detail_Data = 8, 8190, HEX
    Detail_Data = 6, 8191, HEX
    Detail_Data = 2, 8192, HEX
    Detail_Data = 8, 8193, HEX
    Detail_Data = 20, 8194, HEX
    Detail_Data = 118, 8195, HEX
    Detail_Data = 16, 11, HEX

