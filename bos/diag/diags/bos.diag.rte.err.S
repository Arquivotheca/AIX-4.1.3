# @(#)79	1.1  src/bos/diag/diags/bos.diag.rte.err.S, diagsup, bos411, 9428A410j 3/31/94 16:01:35
#
#   COMPONENT_NAME: CMDDIAG
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
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
 
+ DIAG_REPORT:
    Comment = "Diagnostics report"
    Class = H
    Err_Type = PERM
    Report = TRUE
    Log = TRUE
    Alert = FALSE
    Err_Desc = EC55
    Prob_Causes = FFFF
    Fail_Causes = FFFF
    Fail_Actions = EA02
    Detail_Data = 230, EC33, ALPHA
