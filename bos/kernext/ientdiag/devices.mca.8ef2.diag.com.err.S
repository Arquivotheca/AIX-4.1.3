* @(#)32        1.1  src/bos/kernext/ientdiag/devices.mca.8ef2.diag.com.err.S, diagddient, bos411, 9428A410j 11/8/93 10:29:29
*
*   COMPONENT_NAME: DIAGDDIENT
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

+ IENT_ERR1:
    Comment = "permanent integrated ethernet adapter hardware error"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = 1010
    Prob_Causes = 3302
    Fail_Causes = 3302, 0503
    Fail_Actions = 0000, 0301, E801
    Detail_Data = 4, 0F, HEX
    Detail_Data = 4, 13, HEX
    Detail_Data = 4, 0D, HEX

+ IENT_ERR2:
    Comment = "software cannot activate the device, resource problem"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = 8000
    Prob_Causes = 8000, E850
    Fail_Causes = 1012
    Fail_Actions = 0000, E84E, E847, E84A
    Detail_Data = 4, 0F, HEX
    Detail_Data = 4, 13, HEX

+ IENT_ERR3:
    Comment = "DSI-IOCC, error found on DMA completion"
    Class = H
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = TEMP
    Err_Desc = E833
    Prob_Causes = 1012, 6000, 200B, 1040
    Fail_Causes = 1012, 3302
    Fail_Actions = 0000
    Detail_Data = 4, 0F, HEX
    Detail_Data = 4, 13, HEX
    Detail_Data = 4, 0D, HEX

+ IENT_ERR4:
    Comment = "unknown system error caught by ethernet"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = UNKN
    Err_Desc = FE00
    Prob_Causes = FE00, 1012
    Fail_Causes = 3301, 0503, 1000
    Fail_Actions = 0000
    Detail_Data = 4, 0F, HEX
    Detail_Data = 4, 13, HEX
    Detail_Data = 4, 0D, HEX
    Detail_Data = 4, 11, HEX

+ IENT_ERR5:
    Comment = "transmit error"
    Class = S
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Type = UNKN
    Err_Desc = 1603
    Prob_Causes = 3330, 1012, 3451
    Fail_Causes = 1012, 2000, 3300
    Fail_Actions = 0000
    Detail_Data = 4, 0F, HEX
    Detail_Data = 4, 13, HEX
    Detail_Data = 4, 0D, HEX
    Detail_Data = 4, 11, HEX
