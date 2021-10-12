* @(#)01        1.1  src/bos/kernext/entdiag/devices.mca.8ef5.diag.err.S, diagddent, bos411, 9428A410j 10/28/93 15:58:31
*
*   COMPONENT_NAME: DIAGDDENT
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

+ ENT_ERR1:
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "permanent ethernet adapter hardware error"
    Err_Desc = 1010
    Prob_Causes = 3330, 3331
    Fail_Causes = 3322
    Fail_Actions = 0000
    Detail_Data = 64, 11, HEX

+ ENT_ERR2:
    Class = H
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "temporary ethernet adapter hardware error"
    Err_Desc = 3000
    Prob_Causes = 3325, 3426, 3436, 3461
    Fail_Causes = 3322, 3323, 3426, 3436
    Fail_Actions = 0000, 0301
    Detail_Data = 64, 11, HEX

+ ENT_ERR3:
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "permanent ethernet adapter firmware error"
    Err_Desc = 1010
    Prob_Causes = 3331
    Fail_Causes = 3322
    Fail_Actions = 0000
    Detail_Data = 64, 11, HEX

+ ENT_ERR4:
    Class = H
    Err_Type = TEMP
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "temporary ethernet adapter firmware error"
    Err_Desc = 1010
    Prob_Causes = 3331
    Fail_Causes = 3322
    Fail_Actions = 0000
    Detail_Data = 64, 11, HEX

+ ENT_ERR5:
    Class = S
    Err_Type = UNKN
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Comment = "unknown system error caught by ethernet "
    Err_Desc = FE01
    Prob_Causes = 1020
    Fail_Causes = 1000
    Fail_Actions = 0000
    Detail_Data = 64, 11, HEX

+ ENT_ERR6:
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = TRUE
    Comment = "permanent ethernet adapter hardware error"
    Err_Desc = 3221
    Prob_Causes = 3426
    Fail_Causes = 3436, 3426, 3721
    Fail_Actions = 2010, 3102
    Detail_Data = 34, 11, HEX
    Detail_Data = 18, 11, HEX
