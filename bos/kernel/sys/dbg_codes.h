/* @(#)84	1.19  src/bos/kernel/sys/dbg_codes.h, sysdb, bos411, 9428A410j 10/19/93 09:22:19 */

/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS:
 *
 *
 * ORIGINS: 27 83
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
*/

/*
 *	This file defines the valid-entry-reason codes for the low-level
 *	debugger, and their associated informative messages.
 *
 *	This file ***MUST*** be kept parallel with sys/ml/dbg_codes.m4
 */

#ifndef _H_DBG_CODES
#define _H_DBG_CODES

#define  DBG_FPEN          1
#define  DBG_FPEN_MSG      "Floating Point Enabled Exception interrupt."

#define  DBG_INVAL         2
#define  DBG_INVAL_MSG     "Invalid operation interrupt."

#define  DBG_PRIV          3
#define  DBG_PRIV_MSG      "Privileged operation interrupt."

#define  DBG_TRAP          4
#define  DBG_TRAP_MSG      "Trap instruction interrupt."

#define  DBG_UNK_PR        5
#define  DBG_UNK_PR_MSG    "Unknown program interrupt type."

#define  DBG_MCHECK        6
#define  DBG_MCHECK_MSG    "Machine check interrupt."

#define  DBG_SYSRES        7
#define  DBG_SYSRES_MSG    "System reset interrupt."

#define  DBG_ALIGN         8
#define  DBG_ALIGN_MSG     "Alignment check interrupt."

#define  DBG_VM            9
#define  DBG_VM_MSG        "Virtual memory error, code = 0x%X."

#define  DBG_KBD           10
#define  DBG_KBD_MSG       "Debugger entered via keyboard."

#define  DBG_RECURSE       11
#define  DBG_RECURSE_MSG   "Recursion; program check in debugger."

#define  DBG_PANIC         12
#define  DBG_PANIC_MSG     "Panic:  %s"

#define  DBG_KBD_NORMAL    13
#define  DBG_KBD_NORMAL_MSG "Debugger entered via keyboard with key in NORMAL position"

#define  DBG_KBD_SECURE    14
#define  DBG_KBD_SECURE_MSG "Debugger entered via keyboard with key in SECURE position"

#define  DBG_KBD_SERVICE   15
#define  DBG_KBD_SERVICE_MSG "Debugger entered via keyboard with key in SERVICE position"

#define  DBG_KBD_SERVICE1   16
#define  DBG_KBD_SERVICE1_MSG "Debugger entered via keyboard with key in SERVICE position using numpad 1"

#define  DBG_KBD_SERVICE2   17
#define  DBG_KBD_SERVICE2_MSG "Debugger entered via keyboard with key in SERVICE position using numpad 2"

#define  DBG_KBD_SERVICE4   18
#define  DBG_KBD_SERVICE4_MSG "Debugger entered via keyboard with key in SERVICE position using numpad 4"

#define	 DBG_DSI_IOCC	    19
#define	 DBG_DSI_IOCC_MSG   "Data Storage Interrupt - IOCC"

#define  DBG_DSI_SLA	    20
#define	 DBG_DSI_SLA_MSG    "Data Storage Interrupt - SLA"

#define  DBG_DSI_SCU	    21
#define	 DBG_DSI_SCU_MSG    "Data Storage Interrupt - SCU"

#define	 DBG_DSI_PROC	    22
#define  DBG_DSI_PROC_MSG   "Data Storage Interrupt - PROC"

#define	 DBG_EXT_DMA	    23
#define	 DBG_EXT_DMA_MSG    "External Interrupt - DMA Error"

#define	 DBG_EXT_SCR	    24
#define	 DBG_EXT_SCR_MSG    "External Interrupt - Scrub Error"

#define	 DBG_FPT_UN	    25
#define	 DBG_FPT_UN_MSG	    "Floating Point Unavailable Interrupt"

#define	 DBG_ISI_PROC	    26
#define  DBG_ISI_PROC_MSG   "Instruction Storage Interrupt - PROC"

#define  DBG_HTRAP          27
#define  DBG_HTRAP_MSG      "Illegal Trap Instruction Interrupt in Kernel"

#define DBG_DSI_SGA	    28
#define DBG_DSI_SGA_MSG	    "Data Storage Interrupt - SGA"

#define DBG_KBDEXT	    29
#define DBG_KBDEXT_MSG	    "Misc. Interrupt: Keyboard"

#define DBG_BUS_TIMEOUT	    30
#define DBG_BUS_TIMEOUT_MSG "Bus Timeout"

#define DBG_CHAN_CHECK	    31
#define DBG_CHAN_CHECK_MSG  "Channel check" 

#define DBG_BTARGET	    32
#define DBG_BTARGET_MSG	    "Branch Target Interrupt"

#define DBG_WATCH	    33
#define DBG_WATCH_MSG	    "Watch Point Interrupt"

#define DBG_MPC_STOP	    34
#define DBG_MPC_STOP_MSG    "Debugger entered via MPC stop"

#define  DBG_STATIC_TRAP    35
#define  DBG_STATIC_TRAP_MSG      "Trap instruction interrupt."

#endif /* _H_DBG_CODES */
