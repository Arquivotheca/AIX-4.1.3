* @(#)91  1.1  src/bos/kernext/sol/devices.sys.slc.rte.err.S, sysxsol, bos411, 9428A410j 10/26/93 10:24:59
*
*   COMPONENT_NAME: SYSXSOL
*
*   FUNCTIONS: error description stanza file
*
*   ORIGINS: 27
*
*
*   (C) COPYRIGHT International Business Machines Corp. 1989, 1993
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

+ SLA_EXCEPT_ERR:
   Comment = "Internal error SLA exception caught"
   Class = H
   Log = TRUE
   Report = TRUE
   Alert = FALSE
   Err_Type = PERM
   Err_Desc = E950
   Prob_Causes = E873
   Fail_Causes = EA00
   Fail_Actions = 0000
   Detail_Data = 8, 11, HEX

+ SLA_PARITY_ERR:
   Comment = "Internal error SLA buffer/tag parity error"
   Class = H
   Log = TRUE
   Report = TRUE
   Alert = FALSE
   Err_Type = TEMP
   Err_Desc = E951
   Prob_Causes = E874
   Fail_Causes = EA01
   Fail_Actions = 0000,E865
   Detail_Data = 8, 11, HEX

+ SLA_FRAME_ERR:
   Comment = "SLA LINK CHECK possible lost frame"
   Class = H
   Log = TRUE
   Report = TRUE
   Alert = FALSE
   Err_Type = TEMP
   Err_Desc = E952
   Prob_Causes = E875,E876
   Fail_Causes = EA02,EA03
   Fail_Actions = 0000
   Detail_Data = 8, 11, HEX

+ SLA_CRC_ERR:
   Comment = "SLA LINK CHECK CRC error detected"
   Class = H
   Log = TRUE
   Report = TRUE
   Alert = FALSE
   Err_Type = TEMP
   Err_Desc = E953
   Prob_Causes = E875,E876
   Fail_Causes = EA02,EA03
   Fail_Actions = 0000
   Detail_Data = 8, 11, HEX

+ SLA_SIG_ERR:
   Comment = "SLA LINK CHECK signal failure"
   Class = H
   Log = TRUE
   Report = TRUE
   Alert = FALSE
   Err_Type = PERM
   Err_Desc = E954
   Prob_Causes = E875,E876
   Fail_Causes = EA02,EA03
   Fail_Actions = 0000
   Detail_Data = 8, 11, HEX

+ SLA_DRIVER_ERR:
   Comment = "SLA LINK CHECK fault in laser driver"
   Class = H
   Log = TRUE
   Report = TRUE
   Alert = FALSE
   Err_Type = PERM
   Err_Desc = E955
   Prob_Causes = E875
   Fail_Causes = EA02
   Fail_Actions = 0000,E866
   Detail_Data = 8, 11, HEX

+ SLA_PROG_ERR:
   Comment = "SLA Program fault in driver"
   Class = S
   Log = TRUE
   Report = TRUE
   Alert = FALSE
   Err_Type = TEMP
   Err_Desc = E956
   Prob_Causes = E877
   Fail_Causes = 1000
   Fail_Actions = 3000
   Detail_Data = 8, 11, HEX

