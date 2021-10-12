/* @(#)72	1.8  src/bos/kernel/db/POWER/vdberr.h, sysdb, bos411, 9428A410j 6/5/91 09:42:39 */
/*
 * COMPONENT_NAME: (SYSDB) Kernel Debugger
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Debugger error message text.
 */
#ifndef _h_VDBERR
#define _h_VDBERR

#define CMDERRENDP  -1				/* must be last parameter*/

#ifdef DEFINE_ERROR_TABLE

#define ERROR_PREFIX "032-0"			/* message prefix	*/

struct emstruct {
	char subtext;
	char *msg;
};

struct emstruct error_text[] = {
	{TRUE,"01  You entered a command"},
	{FALSE,"that is not valid."},
	{TRUE,"02  You entered a parameter"},
	{FALSE,"03  You did not enter all required parameters."},
	{FALSE,"04  The address you specified is not in real storage."},
	{FALSE,"05  You cannot STEP or GO into paged-out storage."},
	{FALSE,"06  You cannot set more than 32 breakpoints."},
	{FALSE,"07  No breakpoints are set."},
	{FALSE,"08  A breakpoint is undefined or not currently addressable."},
	{TRUE,"09  You cannot define more than"},
	{FALSE,"variables."},
	{TRUE,"11  The value"},
	{FALSE,"cannot be found."},
	{TRUE,"12  The page at"},
	{FALSE,"is not in real storage."},
	{FALSE,"13  Do you want to continue the search? (Y/N)"},
	{FALSE,"14  The printer is not available."},
	{FALSE,"15  No trace buffer is present."},
	{FALSE,"16  The debugger cannot display a system map."},
	{FALSE,"17  Touch cannot be issued with the system in this state."},
	{FALSE,"18  Variable size limit exceeded."},
	{TRUE,"99  Debugger internal error:"},
	{FALSE,"."}
};

/* number of error messages defined. */
#define MAXERR (sizeof(error_text)/sizeof(struct emstruct))

/*
 *  Abend 2ndary text
 */
#define ABEND_PREFIX  "032-1"
char *abend_text[] = {
	"01  A level-2 hardware interrupt has occurred.",
	"02  A machine check has occurred.",
	"03  The VRM received an SVC that was not issued by a Virtual Machine.",
	"04  An unidentified program check occurred.",
	"05  A trap instruction was encountered.",
	"06  A data protection exception has occurred.",
	"07  An unidentified data exception has occurred.",
	"08  An unidentified IAR exception has occurred.",
	"09  An IAR protection exception has occurred.",
	"10  A privileged operation exception has occurred.",
	"11  An illegal operation exception has occurred.",
	"12  A page fault occurred before the VRM was fully operational.",
	"13  A page fault occurred that could not be processed by the VRM.",
	"14  A floating point exception has occurred.",
	"15  The VRM encountered an unresolved reference.",
	"16  The VRM has encountered a stack overflow.",
	"17  All paging space is in use.",
	"18  The VRM tried to reference an address that is not valid.",
	"19  The VRM encountered a permanent I/O error in the paging space.",
	"20  The VRM does not have enough real memory to continue.",
	"21  The VRM minidisk cannot be accessed.",
	"22  The VRM cannot find the paging space minidisk.",
	"23  The VRM has received initialization data that is not valid.",
	"24  The VRM is unable to allocate a system control block.",
	"25  The VRM could not find a fixed disk device driver.",
	"26  An I/O channel error has occurred.",
	"27  An I/O error occurred while accessing cylinder 0 of a fixed disk.",
	"28  A processor I/O error has occurred.",
	"29  A floating point DMA error has occurred.",
	"30  bad block in early part pf temp, not handled (ykt proto)"
};

/* number of abend messages. */
#define MAXABEND  (sizeof(abend_text)/sizeof(abend_text[0]))

#else /*DEFINE_ERROR_TABLE*/

extern vdbperr();

#endif /*DEFINE_ERROR_TABLE*/

/* define indicies for error messages. */
#define ivc_text1   0
#define ivc_text2   1
#define ivd_text2   1
#define ivd_text1   2
#define ins_text    3
#define not_in_real 4
#define cant_step_or_go 5
#define brktablefull 6
#define no_breaks_set 7
#define no_such_break 8
#define nomore_vars1 9
#define nomore_vars2 10
#define arg_not_found1 11
#define arg_not_found2 12
#define page_not_in_real1 13
#define page_not_in_real2 14
#define debeenv 15
#define debenpt 16
#define no_trace_buffer 17
#define cant_do_map 18
#define touch_illegal 19
#define varsize_limit 20
#define deb_err1 21
#define deb_err2 22

#endif /* h_VDBERR */
