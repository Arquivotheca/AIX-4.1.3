*
* @(#)20        1.1  src/bos/kernext/mca_async/devices.mca.ffe1.rte.err.S, sysxs128, bos41J, 9512A_all 4/28/94 18:43:05
*
*   COMPONENT_NAME: CMDTTY
*
*   FUNCTIONS: error description stanza file
*
*   ORIGINS: 27
*
*
*   (C) COPYRIGHT International Business Machines Corp. 1989
*   All Rights Reserved
*   Licensed Materials - Property of IBM
*   US Government Users Restricted Rights - Use, duplication or
*   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
*


*
* error logging template file
*
* errupdate reads this file at system installation time
*  and fills in the template data base.
*
* The message corresponding to each codepoint is in codepoint.desc
*

+ CXMA_CFG_PORT:
	Comment = "Bad Adapter I/O Port Address"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E988
	Prob_Causes = FFFF
	Inst_Causes = 4010
	Inst_Actions = 1507
	Fail_Causes = EA20
	Fail_Actions = E870
	Detail_Data = 4, 8070, HEX
 
+ CXMA_CFG_MEM:
	Comment = "Bad Adapter Memory Address"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E989
	Prob_Causes = FFFF
	Inst_Causes = 4010
	Inst_Actions = 1507
	Fail_Causes = EA21
	Fail_Actions = E871
	Detail_Data = 4, 8071, HEX
 
+ CXMA_CFG_RST:
	Comment = "Adapter Reset Failed"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E98A
	Prob_Causes = FFFF
	Fail_Causes = E938
	Fail_Actions = E801, 3000
	Detail_Data = 4, 8070, HEX
 
+ CXMA_CFG_MTST:
	Comment = "Adapter Memory Test Failed"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E98B
	Prob_Causes = FFFF
	Fail_Causes = E938
	Fail_Actions = E801, 3000
	Detail_Data = 4, 8071, HEX
 
+ CXMA_CFG_BIOS:
	Comment = "Adapter BIOS Initialization Failed"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E98C
	Prob_Causes = FFFF
	Fail_Causes = EA22, E938
	Fail_Actions = E801, 3000
	Detail_Data = 2, 8073, HEX
 
+ CXMA_CFG_FEPOS:
	Comment = "Adapter FEPOS Execution Failed"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E98D
	Prob_Causes = FFFF
	Fail_Causes = EA22, E938
	Fail_Actions = E801, 3000
	Detail_Data = 2, 8074, HEX
 
+ CXMA_CFG_TALLOC:
	Comment = "talloc failed"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E98F
	Prob_Causes = FFFF
	Fail_Causes = E931
	Fail_Actions = 3000
	Detail_Data = 4, 8015, DECIMAL
 
+ CXMA_IO_ATT:
	Comment = "I/O Segment Attach Failed"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E981
	Prob_Causes = FFFF
	Fail_Causes = EA22, E938
	Fail_Actions = 3000
	Detail_Data = 4, 8072, DEC
 
+ CXMA_MEM_ATT:
	Comment = "Memory Segment Attach Failed"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E982
	Prob_Causes = FFFF
	Fail_Causes = EA22, E938
	Fail_Actions = 3000
	Detail_Data = 4, 8072, DEC
 
+ CXMA_ADP_FAIL:
	Comment = "Async Adapter Failed"
	Class = H
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E980
	Prob_Causes = FFFF
	Fail_Causes = E938
	Fail_Actions = E801, 3000
	Detail_Data = 2, 8075, DEC
 
+ CXMA_ERR_ASSRT:
	Comment = "Driver Assert Message"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E986
	Prob_Causes = FFFF
	Fail_Causes = EA22
	Fail_Actions = 3000
	Detail_Data = 4, 8072, DEC
 
+ CXMA_LINE_ERR:
	Comment = "Synchronous Line Errors"
	Class = H
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E97A
	Prob_Causes = FFFF
	Fail_Causes = EA23
	Fail_Actions = 3000
	Detail_Data = 2, 807A, DEC
 
+ CXMA_CONC_DOWN:
	Comment = "Concentrator Turned-OFF or Removed from System"
	Class = H
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = E97B
	Prob_Causes = FFFF
	Fail_Causes = EA24
	Fail_Actions = 3000
	Detail_Data = 2, 807B, DEC
 
+ CXMA_BIOS_ERR1:
	Comment = "Error Allocating Memory"
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = EC45
	Prob_Causes = FFFF
	Fail_Causes = EA39
	Fail_Actions = 3000
	Detail_Data = 4, 8015, DEC

+ CXMA_BIOS_ERR2:
	Comment = "Error opening BIOS microcode file."
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = EC46
	Prob_Causes = FFFF
	Fail_Causes = EA3A
	Fail_Actions = EB8F, 3000
	Detail_Data = 4, 8015, DEC

+ CXMA_BIOS_ERR3:
	Comment = "Error stating BIOS microcode file."
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = EC47
	Prob_Causes = FFFF
	Fail_Causes = EA3B
	Fail_Actions = EB8F, 3000
	Detail_Data = 4, 8015, DEC

+ CXMA_BIOS_ERR4:
	Comment = "Error reading BIOS microcode file."
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = EC48
	Prob_Causes = FFFF
	Fail_Causes = EA3C
	Fail_Actions = EB90, 3000
	Detail_Data = 4, 8015, DEC

+ CXMA_BIOS_ERR5:
	Comment = "Error reading BIOS microcode file."
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = EC48
	Prob_Causes = FFFF
	Fail_Causes = EA3D
	Fail_Actions = 3000
	Detail_Data = 4, 8015, DEC

+ CXMA_BIOS_ERR6:
	Comment = "Error closing BIOS microcode file."
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = EC49
	Prob_Causes = FFFF
	Fail_Causes = EA3E
	Fail_Actions = 3000
	Detail_Data = 4, 8015, DEC

+ CXMA_FEPOS_ERR1:
	Comment = "Error opening FEPOS microcode file."
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = EC4A
	Prob_Causes = FFFF
	Fail_Causes = EA3A
	Fail_Actions = EB91, 3000
	Detail_Data = 4, 8015, DEC

+ CXMA_FEPOS_ERR2:
	Comment = "Error stating FEPOS microcode file."
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = EC4B
	Prob_Causes = FFFF
	Fail_Causes = EA3B
	Fail_Actions = EB91, 3000
	Detail_Data = 4, 8015, DEC

+ CXMA_FEPOS_ERR3:
	Comment = "Error reading FEPOS microcode file."
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = EC4C
	Prob_Causes = FFFF
	Fail_Causes = EA3C
	Fail_Actions = EB92, 3000
	Detail_Data = 4, 8015, DEC

+ CXMA_FEPOS_ERR4:
	Comment = "Error reading FEPOS microcode file."
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = EC4C
	Prob_Causes = FFFF
	Fail_Causes = EA3D
	Fail_Actions = 3000
	Detail_Data = 4, 8015, DEC

+ CXMA_FEPOS_ERR5:
	Comment = "Error closing FEPOS microcode file."
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = EC4D
	Prob_Causes = FFFF
	Fail_Causes = EA3E
	Fail_Actions = 3000
	Detail_Data = 4, 8015, DEC

+ CXMA_FEPOS_ERR6:
	Comment = "Error moving adapter FEPOS to correct location."
	Class = S
	Log = TRUE
	Report = TRUE
	Alert = FALSE
	Err_Type = PERM
	Err_Desc = EC4E
	Prob_Causes = FFFF
	Fail_Causes = EA3F
	Fail_Actions = 3000
	Detail_Data = 2, 8074, HEX
