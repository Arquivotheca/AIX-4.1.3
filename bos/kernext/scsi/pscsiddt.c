static char sccsid[] = "@(#)22  1.8  src/bos/kernext/scsi/pscsiddt.c, sysxscsi, bos411, 9430C411a 7/27/94 19:46:48";
/*
 * COMPONENT_NAME: (SYSXSCSI) IBM SCSI Adapter Driver Top Half
 *
 * FUNCTIONS:
 *    psc_config, psc_open, psc_close, psc_fail_open, psc_inquiry
 *    psc_start_unit, psc_test_unit_rdy, psc_readblk, psc_adp_str_init,
 *    psc_ioctl, psc_build_command, psc_script_init, psc_diagnostic,
 *    psc_run_diagnostics, psc_loop_test_diagnotics, psc_register_test,
 *    psc_pos_register_test, psc_diag_reset_scsi_bus psc_start_dev,
 *    psc_stop_dev, psc_issue_abort, psc_issue_BDR
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*                                                                      */
/* COMPONENT:   SYSXSCSI                                                */
/*                                                                      */
/* NAME:        pscsiddt.c                                              */
/*                                                                      */
/* FUNCTION:    IBM SCSI Adapter Driver Top Half Source File            */
/*                                                                      */
/*      This adapter driver is the interface between a SCSI device      */
/*      driver and the actual SCSI adapter.  It executes commands       */
/*      from multiple drivers which contain generic SCSI device         */
/*      commands, and manages the execution of those commands.          */
/*      Several ioctls are defined to provide for system management     */
/*      and adapter diagnostic functions.                               */
/*                                                                      */
/* STYLE:                                                               */
/*                                                                      */
/*      To format this file for proper style, use the indent command    */
/*      with the following options:                                     */
/*                                                                      */
/*      -bap -ncdb -nce -cli0.5 -di8 -nfc1 -i4 -l78 -nsc -nbbb -lp      */
/*      -c4 -nei -nip                                                   */
/*                                                                      */
/*      Following formatting with the indent command, comment lines     */
/*      longer than 80 columns will need to be manually reformatted.    */
/*      To search for lines longer than 80 columns, use:                */
/*                                                                      */
/*      cat <file> | untab | fgrep -v sccsid | awk "length >79"         */
/*                                                                      */
/*      The indent command may need to be run multiple times.  Make     */
/*      sure that the final source can be indented again and produce    */
/*      the identical file.                                             */
/*                                                                      */
/************************************************************************/

/* INCLUDED SYSTEM FILES */
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/syspest.h>
#include <sys/dma.h>
#include <sys/sysdma.h>
#include <sys/ioacc.h>
#include <sys/intr.h>
#include <sys/malloc.h>
#include <sys/buf.h>
#include <sys/uio.h>
#include <sys/user.h>
#include <sys/file.h>
#include <sys/pin.h>
#include <sys/sleep.h>
#include <sys/ioctl.h>
#include <sys/systm.h>
#include <sys/errno.h>
#include <sys/except.h>
#include <sys/param.h>
#include <sys/lockl.h>
#include <sys/priv.h>
#include <sys/watchdog.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dump.h>
#include <sys/xmem.h>
#include <sys/time.h>
#include <sys/errids.h>
#include <sys/ddtrace.h>
#include <sys/trchkid.h>
#include <sys/trcmacros.h>
#include <sys/adspace.h>
#include <sys/scsi.h>
#include <sys/pscsidd.h>
/* END OF INCLUDED SYSTEM FILES  */

/************************************************************************/
/* Global pinned device driver static data areas                        */
/************************************************************************/
/* global static structure to hold the driver's EPOW handler struct     */
extern struct    intr    epow_struct;

/* global driver component dump table pointer                           */
extern struct  psc_cdt_table    *psc_cdt;

/* global pointer for adapter structure                                 */
extern struct    adapter_def    adp_str;

extern int    adp_str_inited;

extern ULONG   A_abort_msg_error_Used[];
extern ULONG   A_abort_select_failed_Used[];
extern ULONG   A_abort_io_complete_Used[];
extern ULONG   A_uninitialized_reselect_Used[];
extern ULONG   R_dummy_int_Used[];

extern ULONG   scsi_0_lun_Used[];
extern ULONG   scsi_1_lun_Used[];
extern ULONG   scsi_2_lun_Used[];
extern ULONG   scsi_3_lun_Used[];
extern ULONG   scsi_4_lun_Used[];
extern ULONG   scsi_5_lun_Used[];
extern ULONG   scsi_6_lun_Used[];
extern ULONG   scsi_7_lun_Used[];

extern ULONG   INSTRUCTIONS;
/************************************************************************/
/* Global pageable device driver static data areas                      */
/************************************************************************/
/* global adapter device driver lock word                               */
lock_t    psc_lock = LOCK_AVAIL;

ULONG	A_phase_error_Used[] = {
	0x000000bb,	0x000000c5,
	0x000000d1,	0x000000db,
	0x000000df,	0x000000eb,
	0x000000fb,	0x00000181,
	0x0000018b,	0x000001ab,
	0x000001bd,	0x000001c9,
	0x000001d3,	0x000001dd,
	0x000001e9,	0x000001f5,
	0x00000201,	0x00000211
};

ULONG	A_io_done_after_data_Used[] = {
	0x000000f3
};

ULONG	A_io_done_Used[] = {
	0x000000e7
};

ULONG	A_unknown_msg_Used[] = {
	0x00000121
};

ULONG	A_ext_msg_Used[] = {
	0x00000127
};

ULONG	A_check_next_io_Used[] = {
	0x0000016b
};

ULONG	A_check_next_io_data_Used[] = {
	0x00000171
};

ULONG	A_cmd_select_atn_failed_Used[] = {
	0x00000173
};

ULONG	A_err_not_ext_msg_Used[] = {
	0x0000019b
};

ULONG	A_sync_neg_done_Used[] = {
	0x000001a3
};

ULONG	A_unexpected_status_Used[] = {
	0x000001b3,	0x00000219
};

ULONG	A_sync_msg_reject_Used[] = {
	0x0000014f,	0x00000193
};

ULONG	A_neg_select_failed_Used[] = {
	0x000001bf
};

ULONG	R_target_id_Used[] = {
	0x000000b6,	0x00000182,
	0x000001c0,	0x000001ca
};

ULONG	cmd_msg_in_addr_Used[] = {
	0x000000e1,	0x000000ed,
	0x00000101,	0x00000111
};

ULONG	cmd_addr_Used[] = {
	0x000000c7
};

ULONG	status_addr_Used[] = {
	0x000000dd,	0x000000e9,
	0x000001a9,	0x0000020f
};

ULONG	identify_msg_addr_Used[] = {
	0x000000bd
};

ULONG	sync_msg_out_addr_Used[] = {
	0x0000018d
};

ULONG	sync_msg_out_addr2_Used[] = {
	0x00000159
};

ULONG	extended_msg_addr_Used[] = {
	0x00000125,	0x0000012b,
	0x00000197,	0x0000019f,
	0x000001ad
};

ULONG	lun_msg_addr_Used[] = {
	0x00000017,	0x0000002b,
	0x0000003f,	0x00000053,
	0x00000067,	0x0000007b,
	0x0000008f,	0x000000a3
};

ULONG	reject_msg_addr_Used[] = {
	0x0000013f
};

ULONG	abort_bdr_msg_out_addr_Used[] = {
	0x000001df,	0x000001eb,
	0x000001f7
};

ULONG	abort_bdr_msg_in_addr_Used[] = {
	0x00000203,	0x00000213
};

ULONG	LABELPATCHES[] = {
	0x00000001,	0x00000005,
	0x00000007,	0x00000009,
	0x0000000b,	0x0000000d,
	0x0000000f,	0x00000011,
	0x00000013,	0x000000b7,
	0x000000b9,	0x000000bf,
	0x000000c1,	0x000000c3,
	0x000000c9,	0x000000cb,
	0x000000cd,	0x000000cf,
	0x000000d3,	0x000000d5,
	0x000000d7,	0x000000d9,
	0x000000f7,	0x000000f9,
	0x000000ff,	0x00000103,
	0x00000105,	0x00000107,
	0x00000109,	0x0000010b,
	0x0000010f,	0x00000113,
	0x00000115,	0x00000117,
	0x00000119,	0x0000011b,
	0x0000011f,	0x0000012d,
	0x00000131,	0x00000133,
	0x00000135,	0x00000137,
	0x00000139,	0x00000143,
	0x00000145,	0x00000147,
	0x00000149,	0x0000014b,
	0x00000153,	0x0000015d,
	0x0000015f,	0x00000161,
	0x00000163,	0x00000165,
	0x00000179,	0x0000017b,
	0x0000017d,	0x0000017f,
	0x00000183,	0x00000185,
	0x00000189,	0x0000018f,
	0x00000195,	0x00000199,
	0x000001a7,	0x000001b5,
	0x000001b7,	0x000001b9,
	0x000001bb,	0x000001c1,
	0x000001c3,	0x000001c7,
	0x000001cb,	0x000001cd,
	0x000001d1,	0x000001d9,
	0x000001e3,	0x000001e5,
	0x000001e7,	0x000001ef,
	0x000001f1,	0x000001f3,
	0x000001fb,	0x000001fd,
	0x000001ff,	0x0000021b,
	0x0000021d,	0x0000021f,
	0x00000221,	0x00000223,
	0x00000225,	0x00000227,
	0x00000229,	0x0000022b,
	0x0000022d,	0x0000022f,
	0x00000231
};

ULONG	PATCHES		= 0x00000061;

ULONG	PSC_SCRIPT[] = {
	0x00000050,	0x08000000,
	0x00000080,	0x07000000,
	0x81000C80,	0x58000000,
	0x82000C80,	0xA8000000,
	0x84000C80,	0xF8000000,
	0x88000C80,	0x48010000,
	0x90000C80,	0x98010000,
	0xA0000C80,	0xE8010000,
	0xC0000C80,	0x38020000,
	0x80000C80,	0x88020000,
	0x00000898,	0x1D000000,
	0x0100000F,	0x00000000,
	0x80000C80,	0x00000000,
	0x81000C80,	0x00000000,
	0x82000C80,	0x00000000,
	0x83000C80,	0x00000000,
	0x84000C80,	0x00000000,
	0x85000C80,	0x00000000,
	0x86000C80,	0x00000000,
	0x87000C80,	0x00000000,
	0x00000898,	0x1E000000,
	0x0100000F,	0x00000000,
	0x80000C80,	0x00000000,
	0x81000C80,	0x00000000,
	0x82000C80,	0x00000000,
	0x83000C80,	0x00000000,
	0x84000C80,	0x00000000,
	0x85000C80,	0x00000000,
	0x86000C80,	0x00000000,
	0x87000C80,	0x00000000,
	0x00000898,	0x1E000000,
	0x0100000F,	0x00000000,
	0x80000C80,	0x00000000,
	0x81000C80,	0x00000000,
	0x82000C80,	0x00000000,
	0x83000C80,	0x00000000,
	0x84000C80,	0x00000000,
	0x85000C80,	0x00000000,
	0x86000C80,	0x00000000,
	0x87000C80,	0x00000000,
	0x00000898,	0x1E000000,
	0x0100000F,	0x00000000,
	0x80000C80,	0x00000000,
	0x81000C80,	0x00000000,
	0x82000C80,	0x00000000,
	0x83000C80,	0x00000000,
	0x84000C80,	0x00000000,
	0x85000C80,	0x00000000,
	0x86000C80,	0x00000000,
	0x87000C80,	0x00000000,
	0x00000898,	0x1E000000,
	0x0100000F,	0x00000000,
	0x80000C80,	0x00000000,
	0x81000C80,	0x00000000,
	0x82000C80,	0x00000000,
	0x83000C80,	0x00000000,
	0x84000C80,	0x00000000,
	0x85000C80,	0x00000000,
	0x86000C80,	0x00000000,
	0x87000C80,	0x00000000,
	0x00000898,	0x1E000000,
	0x0100000F,	0x00000000,
	0x80000C80,	0x00000000,
	0x81000C80,	0x00000000,
	0x82000C80,	0x00000000,
	0x83000C80,	0x00000000,
	0x84000C80,	0x00000000,
	0x85000C80,	0x00000000,
	0x86000C80,	0x00000000,
	0x87000C80,	0x00000000,
	0x00000898,	0x1E000000,
	0x0100000F,	0x00000000,
	0x80000C80,	0x00000000,
	0x81000C80,	0x00000000,
	0x82000C80,	0x00000000,
	0x83000C80,	0x00000000,
	0x84000C80,	0x00000000,
	0x85000C80,	0x00000000,
	0x86000C80,	0x00000000,
	0x87000C80,	0x00000000,
	0x00000898,	0x1E000000,
	0x0100000F,	0x00000000,
	0x80000C80,	0x00000000,
	0x81000C80,	0x00000000,
	0x82000C80,	0x00000000,
	0x83000C80,	0x00000000,
	0x84000C80,	0x00000000,
	0x85000C80,	0x00000000,
	0x86000C80,	0x00000000,
	0x87000C80,	0x00000000,
	0x00000898,	0x1E000000,
	0x00000241,	0xC8050000,
	0x00000B83,	0x70030000,
	0x0000029E,	0x01000000,
	0x01000006,	0x00000000,
	0x00000B86,	0xF0020000,
	0x00000A83,	0x70030000,
	0x00000A87,	0x00040000,
	0x0000029A,	0x01000000,
	0x00000002,	0x00000000,
	0x00000B87,	0x00040000,
	0x00000A83,	0x70030000,
	0x00000A81,	0xF0030000,
	0x00000A80,	0xD0030000,
	0x00000898,	0x01000000,
	0x00000B87,	0x40040000,
	0x00000A83,	0xA0030000,
	0x00000A81,	0xF0030000,
	0x00000A80,	0xD0030000,
	0x00000898,	0x01000000,
	0x01000003,	0x00000000,
	0x0000039F,	0x01000000,
	0x01000007,	0x00000000,
	0x40000060,	0x00000000,
	0x00000048,	0x00000000,
	0x00000898,	0x06000000,
	0x01000003,	0x00000000,
	0x0000039F,	0x01000000,
	0x01000007,	0x00000000,
	0x40000060,	0x00000000,
	0x00000048,	0x00000000,
	0x00000898,	0x05000000,
	0x00000000,	0x00000000,
	0x00000B83,	0xA0030000,
	0x00000A87,	0x40040000,
	0x00000898,	0x01000000,
	0x00000001,	0x00000000,
	0x00000880,	0xD8030000,
	0x01000007,	0x00000000,
	0x01000C80,	0x88040000,
	0x04000C80,	0x98050000,
	0x07000C80,	0x30050000,
	0x03000C80,	0x30040000,
	0x02000480,	0x80040000,
	0x40000060,	0x00000000,
	0x00000880,	0x20030000,
	0x01000007,	0x00000000,
	0x01000C80,	0x88040000,
	0x04000C80,	0xB0050000,
	0x07000C80,	0x30050000,
	0x03000C80,	0x70040000,
	0x02000480,	0x80040000,
	0x40000060,	0x00000000,
	0x00000880,	0x48030000,
	0x00000898,	0x09000000,
	0x40000060,	0x00000000,
	0x0200000F,	0x00000000,
	0x00000898,	0x0A000000,
	0x40000060,	0x00000000,
	0x0000000F,	0x00000000,
	0x00000080,	0xB0040000,
	0x40000060,	0x00000000,
	0x00000B87,	0x00040000,
	0x00000A83,	0x70030000,
	0x00000A81,	0xF0030000,
	0x00000A80,	0xD0030000,
	0x00000880,	0x10030000,
	0x08000058,	0x00000000,
	0x40000060,	0x00000000,
	0x0100000E,	0x00000000,
	0x08000060,	0x00000000,
	0x00000B87,	0x00040000,
	0x00000A83,	0x70030000,
	0x00000A81,	0xF0030000,
	0x00000A80,	0xD0030000,
	0x00000880,	0x10030000,
	0x08000060,	0x00000000,
	0x00000898,	0x16000000,
	0x40000060,	0x00000000,
	0x00000880,	0xF8020000,
	0x08000058,	0x00000000,
	0x40000060,	0x00000000,
	0x0500000E,	0x00000000,
	0x08000060,	0x00000000,
	0x00000B87,	0x00040000,
	0x00000A83,	0x70030000,
	0x00000A81,	0xF0030000,
	0x00000A80,	0xD0030000,
	0x00000880,	0x10030000,
	0x40000060,	0x00000000,
	0x00000048,	0x00000000,
	0x00000898,	0x0B000000,
	0x40000060,	0x00000000,
	0x00000048,	0x00000000,
	0x00000898,	0x0C000000,
	0x00000898,	0x12000000,
	0x00000080,	0x00000000,
	0x40000060,	0x00000000,
	0x00000B81,	0xF0030000,
	0x00000A80,	0xD0030000,
	0x00000A83,	0x70030000,
	0x00000A87,	0x00040000,
	0x00000898,	0x01000000,
	0x00000241,	0xF8060000,
	0x00000B86,	0x30060000,
	0x08000060,	0x00000000,
	0x00000A83,	0xA0060000,
	0x00000898,	0x01000000,
	0x06000006,	0x00000000,
	0x00000B87,	0x58060000,
	0x08000060,	0x00000000,
	0x00000898,	0x16000000,
	0x00000880,	0xF8020000,
	0x01000007,	0x00000000,
	0x07000C80,	0x90060000,
	0x01000498,	0x13000000,
	0x40000060,	0x00000000,
	0x0400000F,	0x00000000,
	0x40000060,	0x00000000,
	0x00000898,	0x14000000,
	0x40000060,	0x00000000,
	0x00000880,	0xD0060000,
	0x01000003,	0x00000000,
	0x0000039F,	0x01000000,
	0x01000007,	0x00000000,
	0x40000060,	0x00000000,
	0x00000048,	0x00000000,
	0x00000898,	0x15000000,
	0x07000C80,	0x30050000,
	0x00000B86,	0x30060000,
	0x00000A87,	0x58060000,
	0x00000A83,	0xA0060000,
	0x00000898,	0x01000000,
	0x00000898,	0x18000000,
	0x00000241,	0x30080000,
	0x00000B86,	0xD8070000,
	0x08000060,	0x00000000,
	0x00000A83,	0x38080000,
	0x00000898,	0x01000000,
	0x00000241,	0x30080000,
	0x00000B86,	0xA8070000,
	0x08000060,	0x00000000,
	0x00000A83,	0x38080000,
	0x00000898,	0x01000000,
	0x08000058,	0x00000000,
	0x40000060,	0x00000000,
	0x00000B86,	0x78070000,
	0x08000060,	0x00000000,
	0x00000898,	0x01000000,
	0x0100000E,	0x00000000,
	0x08000060,	0x00000000,
	0x00000B87,	0x08080000,
	0x00000A86,	0x78070000,
	0x00000A83,	0x38080000,
	0x00000898,	0x01000000,
	0x02000006,	0x00000000,
	0x08000060,	0x00000000,
	0x00000B87,	0x08080000,
	0x00000A86,	0xA8070000,
	0x00000A83,	0x38080000,
	0x00000898,	0x01000000,
	0x01000006,	0x00000000,
	0x08000060,	0x00000000,
	0x00000B87,	0x08080000,
	0x00000A86,	0xD8070000,
	0x00000A83,	0x38080000,
	0x00000898,	0x01000000,
	0x01000007,	0x00000000,
	0x40000060,	0x00000000,
	0x04000498,	0x17000000,
	0x00000048,	0x00000000,
	0x00000898,	0x1A000000,
	0x00000898,	0x19000000,
	0x01000003,	0x00000000,
	0x0000039F,	0x01000000,
	0x01000007,	0x00000000,
	0x40000060,	0x00000000,
	0x00000048,	0x00000000,
	0x00000898,	0x15000000,
	0x00000080,	0x68080000,
	0x00000080,	0x70080000,
	0x00000080,	0x70080000,
	0x00000080,	0x80080000,
	0x00000080,	0x88080000,
	0x00000080,	0x90080000,
	0x00000080,	0x98080000,
	0x00000080,	0xA0080000,
	0x00000080,	0xA8080000,
	0x00000080,	0xB0080000,
	0x00000080,	0xB8080000,
	0x00000080,	0xC0080000
};

/************************************************************************/
/*                                                                      */
/* NAME:        psc_config                                              */
/*                                                                      */
/* FUNCTION:    Adapter Driver Configuration Routine                    */
/*                                                                      */
/*      For the INIT option, this routine allocates and initializes     */
/*      data structures required for processing user requests to the    */
/*      adapter.  If the TERM option is specified, this routine will    */
/*      delete a previously defined device and free the structures      */
/*      associated with it.  If the QVPD option is specified, this      */
/*      routine will return the adapter vital product data.             */
/*                                                                      */
/*      During device define time, this routine allocates and           */
/*      initializes data structures for processing of requests from     */
/*      the device driver heads to the scsi chip device driver.  The    */
/*      code is set up to handle only a single device instance.         */
/*                                                                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can only be called by a process.                   */
/*      It can page fault only if called under a process                */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure                     */
/*      adap_ddi - adapter dependent information structure              */
/*      uio     - user i/o area struct                                  */
/*      devsw   - kernel entry point struct                             */
/*                                                                      */
/* INPUTS:                                                              */
/*      devno   - device major/minor number                             */
/*      op      - operation code (INIT, TERM, or QVPD)                  */
/*      uiop    - pointer to uio structure for data for the             */
/*                specified operation code                              */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      EFAULT  - return from uiomove                                   */
/*      EBUSY   - on terminate, means device still opened               */
/*      ENOMEM  - memory space unavailable for required allocation      */
/*      EINVAL  - invalid config parameter passed                       */
/*      EIO     - bad operation, or permanent I/O error                 */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      lockl           unlockl                                         */
/*      devswadd        devswdel                                        */
/*      bcopy           bzero                                           */
/*      xmalloc         xmfree                                          */
/*      uiomove                                                         */
/*                                                                      */
/************************************************************************/
int
psc_config(
	   dev_t devno,
	   int op,
	   struct uio * uiop)
{
    struct devsw psc_dsw;
    struct adap_ddi local_ddi;
    int     ret_code, rc, i, data;
    uchar   vpd[VPD_SIZE];
    extern int nodev();

    DEBUG_0 ("Entering psc_config routine.\n")

    ret_code = 0;       /* default return code */
    rc = lockl(&(psc_lock), LOCK_SHORT);	/* serialize this */
    if (rc == LOCK_SUCC)
    {	/* lock succeded */
	switch (op)
	{	/* begin op switch */
	case CFG_INIT:
	    if (adp_str_inited)
	    {	/* pointer already initialized */
		ret_code = EIO;
		break;
	    }
	    else
	    {	/* new uninitialized pointer */
		/* set up entry points to the driver    */
		/* for the device switch table          */
		psc_dsw.d_open = (int (*) ()) psc_open;
		psc_dsw.d_close = (int (*) ()) psc_close;
		psc_dsw.d_read = nodev;
		psc_dsw.d_write = nodev;
		psc_dsw.d_ioctl = (int (*) ()) psc_ioctl;
		psc_dsw.d_strategy = (int (*) ()) psc_strategy;
		psc_dsw.d_ttys = 0;
		psc_dsw.d_select = nodev;
		psc_dsw.d_config = (int (*) ()) psc_config;
		psc_dsw.d_print = nodev;
		psc_dsw.d_dump = (int (*) ()) psc_dump;
		psc_dsw.d_mpx = nodev;
		psc_dsw.d_revoke = nodev;
		psc_dsw.d_dsdptr = 0;
		psc_dsw.d_selptr = 0;
		psc_dsw.d_opts = 0;

		rc = devswadd(devno, &psc_dsw);
		if (rc != 0)
		{	/* failed to add to dev switch table */
		    ret_code = EIO;
		    break;
		}

	    }	/* new uninitialized pointer */

	    /* move adapter configuration data (from uio  */
	    /* space) into local area (kernel space).     */
	    rc = uiomove((caddr_t) (&local_ddi),
			 (int) sizeof(struct adap_ddi),
			 UIO_WRITE, (struct uio *) uiop);
	    if (rc != 0)
	    {	/* copy failed */
		(void) devswdel(devno);	/* clean up */
		ret_code = EIO;
		break;
	    }

	    /* */
	    /* do any data validation here.  tcw area must  */
	    /* start on a tcw boundary, and must be twice   */
	    /* MAXREQUEST in length interrupt priority must */
	    /* be CLASS2, bus type must be Micro Channel,   */
	    /* and base address must be on a 4K boundary    */
	    /* */
	    if ((local_ddi.tcw_length < (2 * MAXREQUEST)) ||
		(local_ddi.tcw_start_addr & (PAGESIZE - 1)) ||
		(local_ddi.int_prior != INTCLASS2) ||
		(local_ddi.bus_type != BUS_MICRO_CHANNEL) ||
		!((local_ddi.base_addr == 0x0080)))
	    {	/* problem with ddi data */
		(void) devswdel(devno);	/* clean up */
		ret_code = EINVAL;
		break;
	    }

	    /* copy local ddi to global, static adapter    */
	    /* structure                                   */
	    bcopy(&local_ddi, &adp_str.ddi,
		  sizeof(struct adap_ddi));
	    /* do any required processing on the            */
	    /* configuration data                           */
	    adp_str.ddi.card_scsi_id &= 0x07;

	    /* if the tcw area is greater than 1MB plus the */
	    /* number of 4K tcws used, then scale up the    */
	    if (adp_str.ddi.tcw_length >
		(0x100000 + (TCW_TABLE_SIZE * PAGESIZE)))
		adp_str.max_request = (adp_str.ddi.tcw_length -
		     (0x100000 + (TCW_TABLE_SIZE * PAGESIZE))) +
		     MAXREQUEST;
	    else
		adp_str.max_request = MAXREQUEST;

	    /* This table reflects the min xfer period value */
	    /* returned by the device during negotiate	 */
	    adp_str.xfer_max[0] = 0x34;
	    adp_str.xfer_max[1] = 0x3E;
	    adp_str.xfer_max[2] = 0x48;
	    adp_str.xfer_max[3] = 0x53;
	    adp_str.xfer_max[4] = 0x5D;
	    adp_str.xfer_max[5] = 0x68;
	    adp_str.xfer_max[6] = 0x72;
	    adp_str.xfer_max[7] = 0x7D;

#ifdef PSC_TRACE
	    adp_str.current_trace_line = 1;
	    adp_str.trace_ptr = (struct psc_trace_struct *)
	    xmalloc(sizeof(struct psc_trace_struct), 4, pinned_heap);
#endif
	    TRACE_1 ("**PSCSTART*** ", 0)
	    TRACE_1 ("in config adp ", 0)
	    rc = psc_config_adapter();
	    TRACE_1 ("out config adp", 0)
	    if (rc != 0)
	    {	/* unsuccessful adapter config */
		(void) devswdel(devno); /* clean up */
		ret_code = EIO;
		break;
	    }

	    adp_str.devno = devno;
	    adp_str.opened = FALSE;
	    adp_str_inited = TRUE;
	    break;	/* CFG_INIT case */

	    /* handle request to terminate an adapter here      */
	case CFG_TERM:
	    if (!adp_str_inited)
	    {	/* device already deleted */
		ret_code = 0;
		break;
	    }

	    if (adp_str.opened)
	    {	/* error, someone else has it opened */
		ret_code = EBUSY;
		break;
	    }

	    /* Disable the chip */
	    (void) psc_write_POS(POS2, 0x00);

	    (void) devswdel(devno);	/* clean up */
	    adp_str_inited = FALSE;
#ifdef PSC_TRACE
	    (void) xmfree(adp_str.trace_ptr, pinned_heap);
#endif

	    break;	/* CFG_TERM case */

	    /* handle query for adapter VPD here                */
	case CFG_QVPD:
	    if (!adp_str_inited)
	    {	/* device structure not initialized */
		ret_code = ENXIO;
		break;
	    }

	    for (i = 0; i < VPD_SIZE; i++)
	    {	/* get VPD data one character at a time */
		if (psc_write_POS((uint) POS6, (int) (i + 1)) != 0)
		{
		    ret_code = EIO;
		    break;
		}
		data = psc_read_POS(POS3);
		if (data == -1)
		{
		    ret_code = EIO;
		    break;
		}
		vpd[i] = (uchar) data;	/* load VPD data */
	    }	/* get VPD data one character at a time */

	    /* leave POS6 in known state */
	    (void) psc_write_POS((uint) POS6, (int) 0x01);
	    /* move the VPD to the caller's uio structure */
	    rc = uiomove((caddr_t) vpd, (int) VPD_SIZE,
			 UIO_READ, (struct uio *) uiop);

	    if (rc != 0)
	    {	/* unsuccessful copy */
		ret_code = EIO;
	    }

	    break;	/* CFG_QVPD case */

	    /* handle invalid config parameter here */
	default:
	    ret_code = EINVAL;

	}
    }
    else
    {	/* lock failed */
	ret_code = EIO;	/* error--kernel service call failed */
	return (ret_code);
    }

    DEBUG_0 ("Leaving psc_config routine.\n")
    unlockl(&(psc_lock));
    return (ret_code);
}

/************************************************************************/
/*                                                                      */
/* NAME:        psc_open                                                */
/*                                                                      */
/* FUNCTION:    Adapter Driver Open Routine                             */
/*                                                                      */
/*      This routine opens the scsi chip and makes it ready.  It        */
/*      allocates adapter specific structures and initializes           */
/*      appropriate fields in them.  The adapter is marked as           */
/*      opened.                                                         */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called by a process.                        */
/*      It can page fault only if called under a process                */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      devno   - device major/minor number                             */
/*      devflag - unused                                                */
/*      chan    - unused                                                */
/*      ext     - extended data; this is 0 for normal use, or           */
/*                a value of SC_DIAGNOSTIC selects diagnostic mode      */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      EIO     - kernel service failed or invalid operation            */
/*      EPERM   - authority error                                       */
/*      EACCES  - illegal operation due to current mode (diag vs norm)  */
/*      ENOMEM  - not enough memory available                           */
/*          ENODEV      - device is not defined                         */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      lockl           unlockl                                         */
/*      pincode         unpincode                                       */
/*      privcheck       e_sleep                                         */
/*      i_init          i_clear                                         */
/*      xmalloc         xmfree                                          */
/*      dmp_add         dmp_del                                         */
/*      d_init          d_clear                                         */
/*      d_mask          d_unmask                                        */
/*      d_master        d_complete                                      */
/*      i_disable       i_enable                                        */
/*      w_init          w_start                                         */
/*                                                                      */
/************************************************************************/
int
psc_open(
	 dev_t devno,
	 ulong devflag,
	 int chan,
	 int ext)
{
    int     rc, ret_code;
    int     i;
    int     remaining_tcws;
    int     undo_level;
    uint    dma_addr;

    DEBUG_0 ("Entering psc_open routine.\n")

    ret_code = 0;
    undo_level = 0;

    DDHKWD5(HKWD_DD_SCSIDD, DD_ENTRY_OPEN, ret_code, devno, devflag,
	    chan, ext, 0);

    rc = lockl(&(psc_lock), LOCK_SHORT);	/* serialize this */
    if (rc == LOCK_SUCC)
    {	/* no problem with lock word */

	if (!adp_str_inited)
	{
	    ret_code = EIO;
	    psc_fail_open(undo_level, ret_code, devno);
	    return (ret_code);
	}
	if ((ext & SC_DIAGNOSTIC) && adp_str.opened)
	{	/* cannot open in DIAG if already opened */
	    ret_code = EACCES;
	    psc_fail_open(undo_level, ret_code, devno);
	    return (ret_code);
	}
	if (adp_str.opened && (adp_str.open_mode == DIAG_MODE))
	{	/* cannot open if already opened in DIAG MODE */
	    ret_code = EACCES;
	    psc_fail_open(undo_level, ret_code, devno);
	    return (ret_code);
	}
	if ((ext & SC_DIAGNOSTIC) && (privcheck(RAS_CONFIG) != 0))
	{   /* trying to open in DIAG MODE without RAS_CONFIG authority */
	    ret_code = EPERM;
	    psc_fail_open(undo_level, ret_code, devno);
	    return (ret_code);
	}
	if ((privcheck(DEV_CONFIG) != 0) && (!(devflag & DKERNEL)))
	{	/* must be normal open */
	    ret_code = EPERM;
	    psc_fail_open(undo_level, ret_code, devno);
	    return (ret_code);
	}
	if (!adp_str.opened)
	{	/* not opened yet */
	    adp_str.errlog_enable = FALSE;

	    rc = pincode(psc_intr);
	    if (rc != 0)
	    {	/* pin failed */
		ret_code = EIO;
		psc_fail_open(undo_level, ret_code, devno);
		return (ret_code);
	    }
	    undo_level++;

	    if (!(ext & SC_DIAGNOSTIC)) /* NORMAL_MODE */
	    {
		/* init SIOP watchdog timer struct */
		adp_str.adap_watchdog.dog.next = NULL;
		adp_str.adap_watchdog.dog.prev = NULL;
		adp_str.adap_watchdog.dog.func = psc_command_watchdog;
		adp_str.adap_watchdog.dog.count = 0;
		adp_str.adap_watchdog.dog.restart = SIOPWAIT;
		adp_str.adap_watchdog.timer_id = PSC_SIOP_TMR;

		/* init reset watchdog timer struct */
		adp_str.reset_watchdog.dog.next = NULL;
		adp_str.reset_watchdog.dog.prev = NULL;
		adp_str.reset_watchdog.dog.func = psc_command_watchdog;
		adp_str.reset_watchdog.dog.count = 0;
		adp_str.reset_watchdog.dog.restart = RESETWAIT;
		adp_str.reset_watchdog.timer_id = PSC_RESET_TMR;

		/* init restart watchdog timer struct */
		adp_str.restart_watchdog.dog.next = NULL;
		adp_str.restart_watchdog.dog.prev = NULL;
		adp_str.restart_watchdog.dog.func = psc_command_watchdog;
		adp_str.restart_watchdog.dog.count = 0;
		adp_str.restart_watchdog.dog.restart = 0;
		adp_str.restart_watchdog.timer_id = PSC_RESTART_TMR;

		/* initialize the system timers */
		w_init(&adp_str.adap_watchdog.dog);
		w_init(&adp_str.reset_watchdog.dog);
		w_init(&adp_str.restart_watchdog.dog);
		undo_level++;

		/* initialize the adapter structure */
		psc_adp_str_init();
		rc = i_init(&(adp_str.intr_struct));
		if (rc != 0)
		{	/* i_init of scsi chip interrupt handler */
		    ret_code = EIO;
		    psc_fail_open(undo_level, ret_code, devno);
		    return (ret_code);
		}
		undo_level++;

		INIT_EPOW(&epow_struct, (int (*) ()) psc_epow,
			  adp_str.ddi.bus_id);
		rc = i_init(&epow_struct);
		if (rc != 0)
		{	/* i_init of epow structure failed */
		    ret_code = EIO;
		    psc_fail_open(undo_level, ret_code, devno);
		    return (ret_code);
		}
		undo_level++;

		rc = psc_write_reg((uint) DCNTL,
				   (uchar) DCNTL_SIZE, 0x01);
		if (rc != 0)
		{	/* error in chip reset */
		    ret_code = EIO;
		    psc_fail_open(undo_level, ret_code, devno);
		    return (ret_code);
		}	/* error in chip reset */
		delay(1 * HZ / 1000);
		rc |= psc_write_reg((uint) DCNTL,
				    (char) DCNTL_SIZE, 0x00);
		if (rc != 0)
		{	/* error in chip reset */
		    ret_code = EIO;
		    psc_fail_open(undo_level, ret_code, devno);
		    return (ret_code);
		}	/* error in chip reset */

		rc = psc_chip_register_init();
		if (rc != 0)
		{	/* error in chip setup */
		    ret_code = EIO;
		    psc_fail_open(undo_level, ret_code, devno);
		    return (ret_code);
		}

		/* allocate and set up the component dump table entry */
		psc_cdt = (struct psc_cdt_table *) xmalloc(
					(uint) sizeof(struct psc_cdt_table),
						     (uint) 2, pinned_heap);
		if (psc_cdt == NULL)
		{	/* error in dump table memory allocation */
		    ret_code = ENOMEM;
		    psc_fail_open(undo_level, ret_code, devno);
		    return (ret_code);
		}
		undo_level++;

		/* initialize the storage for the dump table */
		bzero((char *) psc_cdt, sizeof(struct psc_cdt_table));
		rc = dmp_add(psc_cdt_func);
		if (rc != 0)
		{
		    ret_code = ENOMEM;
		    psc_fail_open(undo_level, ret_code,
				  devno);
		    return (ret_code);
		}
		undo_level++;

		/* set up STA tables and malloc the work area   */
		/* set up index for STA in the TCW static table */
		adp_str.STA_tcw = 0;

		/* malloc the 4K work area for the STAs */
		adp_str.STA[0].sta_ptr = (char *) xmalloc((uint)
				     PAGESIZE, (uint) PGSHIFT, kernel_heap);
		if (adp_str.STA[0].sta_ptr == NULL)
		{	/* error in malloc for STA */
		    ret_code = ENOMEM;
		    psc_fail_open(undo_level, ret_code, devno);
		    return (ret_code);
		}

		undo_level++;
		if (ltpin(adp_str.STA[0].sta_ptr, PAGESIZE))
		{
		    ret_code = EIO;
		    TRACE_1 ("bad ltpin", ret_code)
		    psc_fail_open(undo_level, ret_code, devno);
		    return (ret_code);
		}

		adp_str.TCW_alloc_table[adp_str.STA_tcw] = TCW_RESERVED;
		DEBUG_1 ("STA Table@ = %x\n", adp_str.STA[0].sta_ptr)

		for     (i = 0; i < NUM_STA; i++)
		{	/* initialize the STA management table */
		    adp_str.STA[i].sta_ptr = adp_str.STA[0].sta_ptr +
			(i * ST_SIZE);
		    adp_str.STA[i].in_use = STA_UNUSED;
		}
		adp_str.next_STA_req = 0;
		undo_level++;

		/* set up the 4K Transfer indices into the static */
		/* TCW status table                               */
		adp_str.begin_4K_TCW = adp_str.STA_tcw + 2;
		adp_str.ending_4K_TCW = TCW_TABLE_SIZE - 1;
		adp_str.next_4K_req = adp_str.begin_4K_TCW;

		/* set up SCRIPTS tables and malloc the work area */
		/* malloc the initial 4K work area for the SCRIPTS */
		adp_str.SCRIPTS[0].script_ptr = (ulong *) xmalloc((uint)
				     PAGESIZE, (uint) PGSHIFT, kernel_heap);
		if (adp_str.SCRIPTS[0].script_ptr == NULL)
		{	/* error in malloc for scripts */
		    ret_code = ENOMEM;
		    psc_fail_open(undo_level, ret_code, devno);
		    return (ret_code);
		}

		undo_level++;
		if (ltpin(adp_str.SCRIPTS[0].script_ptr, PAGESIZE))
		{
		    ret_code = EIO;
		    TRACE_1 ("bad ltpin", ret_code)
		    psc_fail_open(undo_level, ret_code, devno);
		    return (ret_code);
		}

		DEBUG_1 ("SCRIPTS Table@ = %x\n",
			         adp_str.SCRIPTS[0].script_ptr)
		/* set up the area for scripts and copy. */
		/* set up index for SCRIPTS in the TCW static table */
		        dma_addr = DMA_ADDR(adp_str.ddi.tcw_start_addr,
					            adp_str.STA_tcw + 1);
		for (i = 0; i < INSTRUCTIONS * 2; i++)
		{
		    *(adp_str.SCRIPTS[0].script_ptr + i) =
			PSC_SCRIPT[i];
		}
		adp_str.SCRIPTS[0].dma_ptr = (ulong *) dma_addr;
		DEBUG_1 ("psc_open: SCRIPTS@=%x\n", adp_str.SCRIPTS)
		        adp_str.SCRIPTS[0].TCW_index = adp_str.STA_tcw + 1;
		adp_str.SCRIPTS[0].in_use = SCR_UNUSED;
		adp_str.TCW_alloc_table[adp_str.STA_tcw + 1] = TCW_RESERVED;
		adp_str.num_scripts_created = 1;
		undo_level++;

		/* Now we have to calculate the "left over space"   */
		/* for the Large Transfer Area.  We take the space  */
		/* taken up by the STA, 4K Transfer Area, and the   */
		/* SCRIPTS and find what is left from the original  */
		/* area given to us in our dds.                     */

		remaining_tcws = (adp_str.ddi.tcw_length -
				  TCW_TABLE_SIZE * PAGESIZE) / LARGESIZE;
		adp_str.large_tcw_start_addr = adp_str.ddi.tcw_start_addr +
		    (TCW_TABLE_SIZE * PAGESIZE);
		adp_str.large_TCW_table = (char *) xmalloc((uint)
				     remaining_tcws, (uint) 4, pinned_heap);
		if (adp_str.large_TCW_table == NULL)
		{	/* table malloc failed */
		    ret_code = ENOMEM;
		    psc_fail_open(undo_level, ret_code, devno);
		    return (ret_code);
		}
		for (i = 0; i < remaining_tcws; i++)
		    adp_str.large_TCW_table[i] = TCW_UNUSED;
		/* initialize the table to unused */
		adp_str.large_req_begin = 0;
		adp_str.next_large_req = 0;
		adp_str.large_req_end = remaining_tcws - 1;
		adp_str.errlog_enable = TRUE;
		adp_str.epow_state = 0;
		undo_level++;

		DEBUG_2 ("dma_lvl=%x, busid=%x\n",
			         adp_str.ddi.dma_lvl, adp_str.ddi.bus_id)
		        adp_str.channel_id = d_init(adp_str.ddi.dma_lvl,
				              DMA_INIT, adp_str.ddi.bus_id);
		if (adp_str.channel_id == DMA_FAIL)
		{	/* failed to init the dma channel */
		    ret_code = EIO;
		    psc_fail_open(undo_level, ret_code, devno);
		    return (ret_code);
		}

		d_unmask(adp_str.channel_id);
		/* lock in the dma areas for STA and SCRIPTS */
		dma_addr = DMA_ADDR(adp_str.ddi.tcw_start_addr,
				    adp_str.STA_tcw);
		DEBUG_2 ("tcw_start_address = %x, dma_addr = %x\n",
			         adp_str.ddi.tcw_start_addr, dma_addr)
		DEBUG_1 ("buffer = %x\n", adp_str.STA[0].sta_ptr)

		        d_master(adp_str.channel_id, DMA_NOHIDE, 
				      (char *) adp_str.STA[0].sta_ptr, 
				      (size_t) PAGESIZE,
			              &adp_str.xmem_STA, (char *) dma_addr);

		/* execute d_master on the SCRIPTS area */
		dma_addr = (uint) adp_str.SCRIPTS[0].dma_ptr;
		DEBUG_2 ("tcw_start_address = %x, dma_addr = %x\n",
			         adp_str.ddi.tcw_start_addr, dma_addr)
		DEBUG_1 ("buffer = %x\n", adp_str.SCRIPTS[0].script_ptr)

		        d_master(adp_str.channel_id, DMA_NOHIDE,
		           (char *) adp_str.SCRIPTS[0].script_ptr, (size_t) PAGESIZE,
			             &adp_str.xmem_SCR, (caddr_t) dma_addr);

		DEBUG_1 ("TCW @ = %x\n", adp_str.TCW_alloc_table)
	    }	/* opened in normal mode */

            if (ext & SC_DIAGNOSTIC)
                adp_str.open_mode = DIAG_MODE;
            else
                adp_str.open_mode = NORMAL_MODE;

	}	/* not opened yet */
    }	/* no problem with lock word */
    else
    {
	ret_code = EIO;
	DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_OPEN, ret_code, devno);
	return (ret_code);
    }
    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_OPEN, ret_code, devno);
    adp_str.opened = TRUE;
    unlockl(&(psc_lock));
    DEBUG_0 ("Leaving psc_open routine.\n")
    return (ret_code);
}

/************************************************************************/
/*                                                                      */
/* NAME:        psc_close                                               */
/*                                                                      */
/* FUNCTION:    Adapter Driver Close Routine                            */
/*                                                                      */
/*      This routine closes the scsi chip instance and releases any     */
/*      resources (as well as unpins the code) that were setup at open  */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called by a process.                        */
/*      It can page fault only if called under a process                */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      adapter_def - adapter unique data structure (one per adapter)   */
/*                                                                      */
/* INPUTS:                                                              */
/*      devno   - device major/minor number                             */
/*      chan    - 0; unused                                             */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  The following are the return values:      */
/*      0       - successful                                            */
/*      EIO     - kernel service failed or invalid operation            */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      lockl           unlockl                                         */
/*      i_disable       i_enable                                        */
/*      e_sleep         w_clear                                         */
/*      d_mask          d_clear                                         */
/*      unpincode       xmfree                                          */
/*      i_clear                                                         */
/*                                                                      */
/************************************************************************/
int
psc_close(
	  dev_t devno,
	  int chan)
{
    int     ret_code, rc;
    int     i;

    ret_code = 0;	/* default to good completion */
    DEBUG_0 ("Entering psc_close routine.\n")
            DDHKWD1(HKWD_DD_SCSIDD, DD_ENTRY_CLOSE, ret_code, devno);

    rc = lockl(&(psc_lock), LOCK_SHORT);	/* serialize this */
    if (rc != LOCK_SUCC)
    {	/* lock kernel call failed */
	ret_code = EIO;
	DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_CLOSE, ret_code, devno);
	return (ret_code);
    }

    if (!adp_str.opened)
    {	/* adapter never opened */
	ret_code = EIO;
	DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_CLOSE, ret_code, devno);
	unlockl(&(psc_lock));
	return (ret_code);
    }

    /* Loop through the device hash table and stop devices */
    /* that are still started.                            */
    for (i = 0; i < MAX_DEVICES; i++)
    {
	if (adp_str.device_queue_hash[i] != NULL)
	{	/* If there's a device still started */
	    psc_stop_dev(i);	/* stops and frees resources */
	}
    }
    if (adp_str.open_mode == NORMAL_MODE)
    {	/* normal open */
	d_mask(adp_str.channel_id);
	d_clear(adp_str.channel_id);	/* free DMA channel */
	(void) ltunpin(adp_str.STA[0].sta_ptr, PAGESIZE);
	(void) xmfree((void *) adp_str.STA[0].sta_ptr,
		      kernel_heap);
	(void) ltunpin(adp_str.SCRIPTS[0].script_ptr, PAGESIZE);
	(void) xmfree((void *) adp_str.SCRIPTS[0].script_ptr,
		      kernel_heap);
	(void) xmfree((void *) adp_str.large_TCW_table,
		      pinned_heap);
	/* Disable dma and SCSI interrupts from the chip */
	(void) psc_write_reg((uint) SIEN, (uchar) SIEN_SIZE, 0x00);
	(void) psc_write_reg((uint) DIEN, (uchar) DIEN_SIZE, 0x00);

	i_clear(&(adp_str.intr_struct));
	/* clear and free the cdt_table */
	(void) dmp_del(psc_cdt_func);
	(void) xmfree((void *) psc_cdt, pinned_heap);
	i_clear(&epow_struct);
	/* clear all system timers */
	w_clear(&adp_str.adap_watchdog.dog);
	w_clear(&adp_str.reset_watchdog.dog);
	w_clear(&adp_str.restart_watchdog.dog);
    }

    (void) unpincode(psc_intr);
    adp_str.opened = FALSE;

    DEBUG_0 ("Leaving psc_close routine.\n")
    unlockl(&(psc_lock));
    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_CLOSE, ret_code, devno);
    return (ret_code);

}  /* end psc_close */

/**************************************************************************/
/*                                                                        */
/* NAME:  psc_fail_open                                                   */
/*                                                                        */
/* FUNCTION:  Cleans up during failed "open" processing.                  */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can only be called by a process.                     */
/*                                                                        */
/* NOTES:  This routine is called to perform processing when a failure    */
/*         of the open occurs. This entails freeing storage, unpinning    */
/*         the code, etc.                                                 */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - scsi chip unique data structure                     */
/*                                                                        */
/* INPUTS:                                                                */
/*      undo_level  -  value of amount of cleanup required.               */
/*      ret_code    -  return value used for trace.                       */
/*      devno       -  device major/minor number.                         */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      None                                                              */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*      unpincode       xmfree                                            */
/*      dmp_del         i_clear                                           */
/*      w_clear         unlockl                                           */
/*                                                                        */
/**************************************************************************/
void
psc_fail_open(
	      int undo_level,
	      int ret_code,
	      dev_t devno)
{

    DEBUG_0 ("Entering psc_fail_open routine.\n")

    switch  (undo_level)
    {	/* begin switch */
    case 11:
	/* free dynamic table for Large Transfer Area */
	(void) xmfree((void *) adp_str.large_TCW_table,
		      pinned_heap);
    case 10:
	/* unpin memory taken by SCRIPTS */
	(void) ltunpin(adp_str.SCRIPTS[0].script_ptr, PAGESIZE);
    case 9:
	/* free memory taken by SCRIPTS */
	(void) xmfree((void *) adp_str.SCRIPTS[0].script_ptr,
		      kernel_heap);
    case 8:
	/* unpin memory taken by STA */
	(void) ltunpin(adp_str.STA[0].sta_ptr, PAGESIZE);
    case 7:
	/* free memory taken by STA */
	(void) xmfree((void *) adp_str.STA[0].sta_ptr,
		      kernel_heap);
    case 6:
	/* clear the cdt_table */
	(void) dmp_del(psc_cdt_func);
    case 5:
	/* free the cdt_table */
	(void) xmfree((void *) psc_cdt, pinned_heap);
    case 4:
	/* clear the EPOW structure */
	i_clear(&epow_struct);
    case 3:
	/* clear out the scsi chip interrupt handler */
	i_clear(&(adp_str.intr_struct));
    case 2:
	/* clear all system timers */
	w_clear(&adp_str.adap_watchdog.dog);
	w_clear(&adp_str.reset_watchdog.dog);
	w_clear(&adp_str.restart_watchdog.dog);
    case 1:
	/* unpin the device driver */
	(void) unpincode(psc_intr);
    default:
	break;

    }	/* end switch */

    DEBUG_0 ("Leaving psc_fail_open routine.\n")
    DEBUG_0 ("Leaving psc_open through psc_fail_open (with errors)\n")
    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_OPEN, ret_code, devno);
    unlockl(&(psc_lock));
    return;
}

/**************************************************************************/
/*                                                                        */
/* NAME:  psc_inquiry                                                     */
/*                                                                        */
/* FUNCTION:  Issues a SCSI inquiry command to a device.                  */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine is called to issue an inquiry to a device.        */
/*         The calling process is responsible for NOT calling this rou-   */
/*         tine if the SCIOSTART failed.  Such a failure would indicate   */
/*         that another process has this device open and interference     */
/*         could cause improper error reporting.                          */
/*                                                                        */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - scsi chip unique data structure                     */
/*                                                                        */
/* INPUTS:                                                                */
/*      devno   - passed device major/minor number                        */
/*      arg     - passed argument value                                   */
/*      devflag - device flag                                             */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      0 - for good completion, ERRNO value otherwise.                   */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*      EINVAL - Device not opened.                                       */
/*      EACCES - Adapter not opened in normal mode.                       */
/*      ENOMEM - Could not allocate an scbuf for this command.            */
/*      ENODEV - Device could not be selected.                            */
/*	ENOCONNECT - No connection (SCSI bus fault).			  */
/*      ETIMEDOUT - The inquiry command timed out.                        */
/*      EIO    - Error returned from psc_strategy.                        */
/*      EIO    - No data returned from inquiry command.                   */
/*      EFAULT - Bad copyin or copyout.                                   */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:  iowait, xmfree, copyin, copyout, bcopy    */
/*                                                                        */
/**************************************************************************/
int
psc_inquiry(
	    dev_t devno,
	    int arg,
	    ulong devflag)
{
    int     ret_code, dev_index, inquiry_length;
    struct sc_buf *bp;
    struct sc_inquiry sc_inq;

    DEBUG_0 ("Entering psc_inquiry routine.\n")
    ret_code = 0;
    /* Copy in the arg structure. If the buffer resides */
    /* user space, use copyin, else use bcopy.          */
    if (!(devflag & DKERNEL))
    {
	ret_code = copyin((char *) arg, &sc_inq,
			  sizeof(struct sc_inquiry));
	if (ret_code != 0)
	{
	    DEBUG_0 ("Leaving psc_inquiry routine.\n")
	    return (EFAULT);
	}
    }
    else
    {	/* buffer is in kernel space */
	bcopy((char *) arg, &sc_inq, sizeof(struct sc_inquiry));
    }
    if (adp_str.open_mode == DIAG_MODE)
    {	/* adapter opened in diagnostic mode */
	DEBUG_0 ("Leaving psc_inquiry routine.\n")
	return (EACCES);
    }
    /* Obtain device structure from hash on the scsi id */
    /* and lun id.                                      */
    dev_index = INDEX(sc_inq.scsi_id, sc_inq.lun_id);

    if (adp_str.device_queue_hash[dev_index] == NULL)
    {	/* device queue structure already allocated */
	DEBUG_0 ("Leaving psc_inquiry routine.\n")
	return (EINVAL);
    }
    bp = psc_build_command();
    if (bp == NULL)
    {
	DEBUG_0 ("Leaving psc_inquiry routine.\n")
	return (ENOMEM);        /* couldn't allocate command */
    }

    bp->scsi_command.scsi_id = sc_inq.scsi_id;
    bp->scsi_command.scsi_length = 6;
    bp->scsi_command.scsi_cmd.scsi_op_code = SCSI_INQUIRY;
    if (sc_inq.get_extended) 
    {
        bp->scsi_command.scsi_cmd.lun = (sc_inq.lun_id << 5) | 0x01;
        bp->scsi_command.scsi_cmd.scsi_bytes[0] = sc_inq.code_page_num;
    }
    else
    {
        bp->scsi_command.scsi_cmd.lun = sc_inq.lun_id << 5;
        bp->scsi_command.scsi_cmd.scsi_bytes[0] = 0;
    }
    bp->scsi_command.scsi_cmd.scsi_bytes[1] = 0;
    /* Always set for the maximum amt of inquiry data */
    bp->scsi_command.scsi_cmd.scsi_bytes[2] = 255;
    bp->scsi_command.scsi_cmd.scsi_bytes[3] = 0;

    /* Set the no disconnect flag and the synch/asynch flag  */
    /* for proper data tranfer to occur.                     */
    bp->scsi_command.flags = (sc_inq.flags & SC_ASYNC) | SC_NODISC;

    bp->bufstruct.b_bcount = 255;
    bp->bufstruct.b_flags |= B_READ;
    bp->bufstruct.b_dev = devno;
    bp->timeout_value = 15;

    /* Set resume flag in case caller is retrying this operation. */
    /* This assumes the inquiry is only running single-threaded  */
    /* to this device.                                           */
    bp->flags = SC_RESUME;

    /* Call our strategy routine to issue the inquiry command. */
    if (psc_strategy(bp))
    {	/* an error was returned */
	(void) xmfree(bp, pinned_heap);	/* release buffer */
	DEBUG_0 ("Leaving psc_inquiry routine.\n")
	return (EIO);
    }

    iowait((struct buf *) bp);  /* Wait for commmand completion */

    /* The return value from the operation is examined to deter- */
    /* what type of error occurred.  Since the calling applica-  */
    /* requires an ERRNO, this value is interpreted here.        */
    if (bp->bufstruct.b_flags & B_ERROR)
    {	/* an error occurred */
	if (bp->status_validity & SC_ADAPTER_ERROR)
	{	/* if adapter error */
	    switch (bp->general_card_status)
	    {
	    case SC_CMD_TIMEOUT:
		psc_logerr(ERRID_SCSI_ERR10, COMMAND_TIMEOUT,
			   55, 0, NULL, FALSE);
		ret_code = ETIMEDOUT;
		break;
	    case SC_NO_DEVICE_RESPONSE:
		ret_code = ENODEV;
		break;
	    case SC_SCSI_BUS_FAULT:
		ret_code = ENOCONNECT;
		break;
	    default:
		ret_code = EIO;
		break;
	    }
	}
	else
	{
	    ret_code = EIO;
	}
    }

    /* if no other errors, and yet no data came back, then fail */
    if ((ret_code == 0) &&
	(bp->bufstruct.b_resid == bp->bufstruct.b_bcount))
    {
	ret_code = EIO;
    }
    /* give the caller the lesser of what he asked for, or */
    /* the actual transfer length                          */
    if (ret_code == 0)
    {
	inquiry_length = bp->bufstruct.b_bcount - bp->bufstruct.b_resid;
	if (inquiry_length > sc_inq.inquiry_len)
	    inquiry_length = sc_inq.inquiry_len;
	/* Copy out the inquiry data. If the buffer resides */
	/* user space, use copyin, else use bcopy.          */
	if (!(devflag & DKERNEL))
	{
	    ret_code = copyout(bp->bufstruct.b_un.b_addr,
			       sc_inq.inquiry_ptr, inquiry_length);
	    if (ret_code != 0)
	    {
		ret_code = EFAULT;
	    }
	}
	else
	{ /* buffer is in kernel space */
	    bcopy(bp->bufstruct.b_un.b_addr, sc_inq.inquiry_ptr,
		  inquiry_length);
	}
    }
    (void) xmfree(bp, pinned_heap);	/* release buffer */
    DEBUG_1 ("Leaving psc_inquiry routine. %d\n", ret_code);
    return (ret_code);
}

/**************************************************************************/
/*                                                                        */
/* NAME:  psc_start_unit                                                  */
/*                                                                        */
/* FUNCTION:  Issues a SCSI start unit command to a device.               */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine is called to to issue a start unit command to a   */
/*         device.                                                        */
/*         The calling process is responsible for NOT calling this rou-   */
/*         tine if the SCIOSTART failed.  Such a failure would indicate   */
/*         that another process has this device open and interference     */
/*         could cause improper error reporting.                          */
/*                                                                        */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - scsi chip unique data structure                     */
/*                                                                        */
/* INPUTS:                                                                */
/*      devno   - passed device major/minor number                        */
/*      arg     - passed argument value                                   */
/*      devflag - device flag                                             */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      0 - for good completion, ERRNO value otherwise.                   */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*    EINVAL - Device not opened.                                         */
/*    EACCES - Adapter not opened in normal mode.                         */
/*    ENOMEM - Could not allocate an scbuf for this command.              */
/*    ENODEV - Device could not be selected.                              */
/*    ETIMEDOUT - The start unit command timed out.                       */
/*    ENOCONNECT - No connection (SCSI bus fault).                        */
/*    EIO    - Error returned from psc_strategy.                          */
/*    EFAULT - Bad copyin or copyout.                                     */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:  iowait, xmfree, copyin, bcopy             */
/*                                                                        */
/**************************************************************************/
int
psc_start_unit(
	       dev_t devno,
	       int arg,
	       ulong devflag)
{
    int     ret_code, dev_index;
    struct sc_buf *bp;
    struct sc_startunit sc_stun;

    DEBUG_0 ("Entering psc_start_unit.\n")
            ret_code = 0;
    /* Copy in the arg structure. If the buffer resides */
    /* user space, use copyin, else use bcopy.          */
    if (!(devflag & DKERNEL))
    {
	ret_code = copyin((char *) arg, &sc_stun,
			  sizeof(struct sc_startunit));
	if (ret_code != 0)
	{
	    DEBUG_0 ("EFAULT: Leaving psc_start_unit routine.\n")
	    return (EFAULT);
	}
    }
    else
    {	/* buffer is in kernel space */
	bcopy((char *) arg, &sc_stun, sizeof(struct sc_startunit));
    }
    if (adp_str.open_mode == DIAG_MODE)
    {	/* adapter opened in diagnostic mode */
	DEBUG_0 ("Leaving psc_start_unit routine. EACCES\n")
	return (EACCES);
    }
    /* Obtain device structure from hash on the scsi id */
    /* and lun id.                                      */
    dev_index = INDEX(sc_stun.scsi_id, sc_stun.lun_id);

    if (adp_str.device_queue_hash[dev_index] == NULL)
    {	/* device queue structure not allocated */
	DEBUG_0 ("Leaving psc_start_unit .EINVAL\n")
	return (EINVAL);
    }
    bp = psc_build_command();
    if (bp == NULL)
    {
	DEBUG_0 ("Leaving psc_start_unit ENOMEM.\n")
	return (ENOMEM);        /* couldn't allocate command */
    }

    bp->scsi_command.scsi_id = sc_stun.scsi_id;
    bp->scsi_command.scsi_length = 6;
    bp->scsi_command.scsi_cmd.scsi_op_code = SCSI_START_STOP_UNIT;
    /* The immediate bit for this command is set depending */
    /* on the value of the immediate flag.                 */
    bp->scsi_command.scsi_cmd.lun = (sc_stun.lun_id << 5) |
	(sc_stun.immed_flag ? 0x01 : 0);
    bp->scsi_command.scsi_cmd.scsi_bytes[0] = 0;
    bp->scsi_command.scsi_cmd.scsi_bytes[1] = 0;
    /* Set the command start flag */
    bp->scsi_command.scsi_cmd.scsi_bytes[2] =
	(sc_stun.start_flag ? 0x01 : 0);
    bp->scsi_command.scsi_cmd.scsi_bytes[3] = 0;

    /* Set the no disconnect flag and the synch/asynch flag  */
    /* for proper data tranfer to occur.                     */
    bp->scsi_command.flags = (sc_stun.flags & SC_ASYNC) | SC_NODISC;

    bp->bufstruct.b_bcount = 0;
    bp->bufstruct.b_dev = devno;
    bp->timeout_value = sc_stun.timeout_value;	/* set timeout value */

    /* Set resume flag in case caller is retrying this operation. */
    /* This assumes the inquiry is only running single-threaded  */
    /* to this device.                                           */
    bp->flags = SC_RESUME;

    /* Call our strategy routine to issue the start unit cmd. */
    if (psc_strategy(bp))
    {	/* an error was returned */
	(void) xmfree(bp, pinned_heap);	/* release buffer */
	DEBUG_0 ("Leaving psc_start_unit .\n")
	return (EIO);
    }

    iowait((struct buf *) bp);  /* Wait for commmand completion */

    /* The return value from the operation is examined to deter- */
    /* what type of error occurred.  Since the calling applica-  */
    /* requires an ERRNO, this value is interpreted here.        */
    if (bp->bufstruct.b_flags & B_ERROR)
    {	/* an error occurred */
	if (bp->status_validity & SC_ADAPTER_ERROR)
	{	/* if adapter error */
	    switch (bp->general_card_status)
	    {
	    case SC_CMD_TIMEOUT:
		psc_logerr(ERRID_SCSI_ERR10, COMMAND_TIMEOUT,
			   60, 0, NULL, FALSE);
		ret_code = ETIMEDOUT;
		break;
	    case SC_NO_DEVICE_RESPONSE:
		ret_code = ENODEV;
		break;
	    case SC_SCSI_BUS_FAULT:
		ret_code = ENOCONNECT;
		break;
	    default:
		ret_code = EIO;
		break;
	    }
	}
	else
	{
	    ret_code = EIO;
	}
    }

    (void) xmfree(bp, pinned_heap);	/* release buffer */
    DEBUG_0 ("Leaving psc_start_unit .\n")
    return (ret_code);
}

/**************************************************************************/
/*                                                                        */
/* NAME:  psc_test_unit_rdy                                               */
/*                                                                        */
/* FUNCTION:  Issues a SCSI test unit ready command to a device.          */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine is called to to issue a test unit ready command   */
/*         to a device.                                                   */
/*         The calling process is responsible for NOT calling this rou-   */
/*         tine if the SCIOSTART failed.  Such a failure would indicate   */
/*         that another process has this device open and interference     */
/*         could cause improper error reporting.                          */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - scsi chip unique data structure                     */
/*                                                                        */
/* INPUTS:                                                                */
/*      devno   - passed device major/minor number                        */
/*      arg     - passed argument value                                   */
/*      devflag - device flag                                             */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      0 - for good completion, ERRNO value otherwise.                   */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*    EINVAL - Device not opened.                                         */
/*    EACCES - Adapter not opened in normal mode.                         */
/*    ENOMEM - Could not allocate an scbuf for this command.              */
/*    ENODEV - Device could not be selected.                              */
/*    ETIMEDOUT - The start unit command timed out.                       */
/*    ENOCONNECT - No connect (SCSI bus fault).                           */
/*    EIO    - Error returned from psc_strategy.                          */
/*    EFAULT - Bad copyin or copyout.                                     */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:  iowait, xmfree, copyin, copyout, bcopy    */
/*                                                                        */
/**************************************************************************/
int
psc_test_unit_rdy(
		  dev_t devno,
		  int arg,
		  ulong devflag)
{
    int     ret_code, ret_code2, dev_index;
    struct sc_buf *bp;
    struct sc_ready sc_rdy;

    DEBUG_0 ("Entering psc_test_unit_rdy routine.\n")

    ret_code = 0;
    /* Copy in the arg structure. If the buffer resides */
    /* user space, use copyin, else use bcopy.          */
    if (!(devflag & DKERNEL))
    {
	ret_code = copyin((char *) arg, &sc_rdy,
			  sizeof(struct sc_ready));
	if (ret_code != 0)
	{
	    DEBUG_0 ("Leaving psc_test_unit_rdy routine. EFAULT\n")
	    return (EFAULT);
	}
    }
    else
    { /* buffer is in kernel space */
	bcopy((char *) arg, &sc_rdy, sizeof(struct sc_ready));
    }
    if (adp_str.open_mode == DIAG_MODE)
    {	/* adapter opened in diagnostic mode */
	DEBUG_0 ("Leaving psc_test_unit_rdy routine. EACCES\n")
	return (EACCES);
    }
    /* Obtain device structure from hash on the scsi id */
    /* and lun id.                                      */
    dev_index = INDEX(sc_rdy.scsi_id, sc_rdy.lun_id);
    if (adp_str.device_queue_hash[dev_index] == NULL)
    {	/* device queue structure not allocated */
	DEBUG_0 ("Leaving psc_test_unit_rdy routine. EINVAL\n")
	return (EINVAL);
    }
    bp = psc_build_command();
    if (bp == NULL)
    {
	DEBUG_0 ("Leaving psc_test_unit_rdy routine. ENOMEM\n")
	return (ENOMEM);        /* couldn't allocate command */
    }

    bp->scsi_command.scsi_id = sc_rdy.scsi_id;
    bp->scsi_command.scsi_length = 6;
    bp->scsi_command.scsi_cmd.scsi_op_code = SCSI_TEST_UNIT_READY;
    bp->scsi_command.scsi_cmd.lun = (sc_rdy.lun_id << 5);
    bp->scsi_command.scsi_cmd.scsi_bytes[0] = 0;
    bp->scsi_command.scsi_cmd.scsi_bytes[1] = 0;
    bp->scsi_command.scsi_cmd.scsi_bytes[2] = 0;
    bp->scsi_command.scsi_cmd.scsi_bytes[3] = 0;

    /* There is no need to set the no disconnect flag for the */
    /* test unit ready command so just set the synch/asynch  */
    /* flag for the sc_rdy structure.                        */
    bp->scsi_command.flags = sc_rdy.flags & SC_ASYNC;
    bp->bufstruct.b_bcount = 0;
    bp->bufstruct.b_dev = devno;

    /* Initialize default status to zero.                    */
    sc_rdy.status_validity = 0;
    sc_rdy.scsi_status = 0;

    /* Set resume flag in case caller is retrying this operation. */
    /* This assumes the inquiry is only running single-threaded  */
    /* to this device.                                           */
    bp->flags = SC_RESUME;

    /* Call our strategy routine to issue the start unit cmd. */
    if (psc_strategy(bp))
    {	/* an error was returned */
	(void) xmfree(bp, pinned_heap);	/* release buffer */
	DEBUG_0 ("Leaving psc_test_unit_rdy strategy error EIO.\n")
	return (EIO);
    }

    iowait((struct buf *) bp);  /* Wait for commmand completion */

    /* The return value from the operation is examined to deter- */
    /* what type of error occurred.  Since the calling applica-  */
    /* requires an ERRNO, this value is interpreted here.        */
    if (bp->bufstruct.b_flags & B_ERROR)
    {	/* an error occurred */
	if (bp->status_validity & SC_ADAPTER_ERROR)
	{	/* if adapter error */
	    switch (bp->general_card_status)
	    {
	    case SC_CMD_TIMEOUT:
		psc_logerr(ERRID_SCSI_ERR10, COMMAND_TIMEOUT,
			   65, 0, NULL, FALSE);
		ret_code = ETIMEDOUT;
		break;
	    case SC_NO_DEVICE_RESPONSE:
		ret_code = ENODEV;
		break;
	    case SC_SCSI_BUS_FAULT:
		ret_code = ENOCONNECT;
		break;
	    default:
		ret_code = EIO;
		break;
	    }
	}
	else
	    if (bp->status_validity & SC_SCSI_ERROR)
	    {	/* if a scsi status error */
		sc_rdy.status_validity = SC_SCSI_ERROR;
		sc_rdy.scsi_status = bp->scsi_status;
		ret_code = EIO;
	    }
	    else
	    {	/* if general error (fall through case) */
		ret_code = EIO;
	    }
    }

    /* Copy out the device status to the st_ready structure      */
    /* passed in by the calling application.                     */
    if (!(devflag & DKERNEL))
    {
	ret_code2 = copyout(&sc_rdy, (char *) arg,
			    sizeof(struct sc_ready));
	if (ret_code2 != 0)
	{
	    ret_code = EFAULT;
	}
    }
    else
    {	/* buffer is in kernel space */
	bcopy(&sc_rdy, (char *) arg, sizeof(struct sc_ready));
    }
    (void) xmfree(bp, pinned_heap);	/* release buffer */
    DEBUG_0 ("Leaving psc_test_unit_rdy routine.\n")
    return (ret_code);
}

/**************************************************************************/
/*                                                                        */
/* NAME:  psc_readblk                                                     */
/*                                                                        */
/* FUNCTION:  Issues a SCSI read command to a device.                     */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine is called to read command to a device.            */
/*         The calling process is responsible for NOT calling this rou-   */
/*         tine if the SCIOSTART failed.  Such a failure would indicate   */
/*         that another process has this device open and interference     */
/*         could cause improper error reporting.                          */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - scsi chip unique data structure                     */
/*                                                                        */
/* INPUTS:                                                                */
/*      devno   - passed device major/minor number                        */
/*      arg     - passed argument value                                   */
/*      devflag - device flag                                             */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      0 - for good completion, ERRNO value otherwise.                   */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*    EINVAL - Device not opened, or transfer size is > 1 page            */
/*    EACCES - Adapter not opened in normal mode.                         */
/*    ENOMEM - Could not allocate an scbuf for this command.              */
/*    ENODEV - Device could not be selected.                              */
/*    ETIMEDOUT - The inquiry command timed out.                          */
/*    ENOCONNECT - SCSI bus fault.                                        */
/*    EIO    - Error returned from psc_strategy.                          */
/*    EIO    - No data returned from read command.                        */
/*    EFAULT - Bad copyin or copyout.                                     */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:  iowait, xmfree, copyin, copyout, bcopy    */
/*                                                                        */
/**************************************************************************/
int
psc_readblk(
	    dev_t devno,
	    int arg,
	    ulong devflag)
{
    int     ret_code, dev_index;
    struct sc_buf *bp;
    struct sc_readblk sc_rd;

    DEBUG_0 ("Entering psc_readblk routine.\n")
    ret_code = 0;
    /* Copy in the arg structure. If the buffer resides */
    /* user space, use copyin, else use bcopy.          */
    if (!(devflag & DKERNEL))
    {
	ret_code = copyin((char *) arg, &sc_rd,
			  sizeof(struct sc_readblk));
	if (ret_code != 0)
	{
	    DEBUG_0 ("Leaving psc_readblk routine.\n")
	    return (EFAULT);
	}
    }
    else
    {	/* buffer is in kernel space */
	bcopy((char *) arg, &sc_rd, sizeof(struct sc_readblk));
    }
    if (adp_str.open_mode == DIAG_MODE)
    {	/* adapter opened in diagnostic mode */
	DEBUG_0 ("Leaving psc_readblk routine.\n")
        return (EACCES);
    }
    /* Obtain device structure from hash on the scsi id */
    /* and lun id.                                      */
    dev_index = INDEX(sc_rd.scsi_id, sc_rd.lun_id);

    if (adp_str.device_queue_hash[dev_index] == NULL)
    {	/* device queue structure already allocated */
	DEBUG_0 ("Leaving psc_readblk routine.\n")
	return (EINVAL);
    }
    if ((int) sc_rd.blklen > PAGESIZE)
    {	/* The tranfer length is too long to be allowed. */
	DEBUG_0 ("Leaving psc_readblk routine.\n")
	return (EINVAL);
    }
    bp = psc_build_command();
    if (bp == NULL)
    {
	DEBUG_0 ("Leaving psc_readblk routine.\n")
	return (ENOMEM);        /* couldn't allocate command */
    }

    /* Xmalloc a page to be used for the data transfer.     */
    bp->bufstruct.b_un.b_addr = xmalloc((uint) PAGESIZE,
					(uint) PGSHIFT,
					pinned_heap);

    if (bp->bufstruct.b_un.b_addr == NULL)
    {	/* Unable to get required storage */
	(void) xmfree(bp, pinned_heap);	/* release sc_buf */
	DEBUG_0 ("Leaving psc_readblk routine.\n")
	return (ENOMEM);
    }

    bp->scsi_command.scsi_id = sc_rd.scsi_id;
    bp->scsi_command.scsi_length = 6;
    bp->scsi_command.scsi_cmd.scsi_op_code = SCSI_READ;
    /* Set up the byte count for this command */
    bp->scsi_command.scsi_cmd.lun = (sc_rd.lun_id << 5) |
	((sc_rd.blkno >> 16) & 0x1f);
    bp->scsi_command.scsi_cmd.scsi_bytes[0] = (sc_rd.blkno >> 8) & 0xff;
    bp->scsi_command.scsi_cmd.scsi_bytes[1] = sc_rd.blkno & 0xff;
    bp->scsi_command.scsi_cmd.scsi_bytes[2] = 1;	/* single block */
    bp->scsi_command.scsi_cmd.scsi_bytes[3] = 0;

    /* Set the no disconnect flag and the synch/asynch flag  */
    /* for proper data tranfer to occur.                     */
    bp->scsi_command.flags = (sc_rd.flags & SC_ASYNC) | SC_NODISC;
    /* put no disconnect flag back in after fix */

    bp->bufstruct.b_bcount = (unsigned) sc_rd.blklen;
    bp->bufstruct.b_flags |= B_READ;
    bp->bufstruct.b_dev = devno;
    bp->timeout_value = sc_rd.timeout_value;

    /* Set resume flag in case caller is retrying this operation. */
    /* This assumes the readblk is only running single-threaded  */
    /* to this device.                                           */
    bp->flags = SC_RESUME;

    /* Call our strategy routine to issue the readblk command. */
    if (psc_strategy(bp))
    {	/* an error was returned */
	(void) xmfree(bp->bufstruct.b_un.b_addr, pinned_heap);
	(void) xmfree(bp, pinned_heap);	/* release sc_buf */
	DEBUG_0 ("Leaving psc_readblk routine.\n")
	return (EIO);
    }

    iowait((struct buf *) bp);	/* Wait for commmand completion */

    /* The return value from the operation is examined to deter- */
    /* what type of error occurred.  Since the calling applica-  */
    /* requires an ERRNO, this value is interpreted here.        */
    if (bp->bufstruct.b_flags & B_ERROR)
    {	/* an error occurred */
	if (bp->status_validity & SC_ADAPTER_ERROR)
	{	/* if adapter error */
	    switch (bp->general_card_status)
	    {
	    case SC_CMD_TIMEOUT:
		psc_logerr(ERRID_SCSI_ERR10, COMMAND_TIMEOUT,
			   70, 0, NULL, FALSE);
		ret_code = ETIMEDOUT;
		break;
	    case SC_NO_DEVICE_RESPONSE:
		ret_code = ENODEV;
		break;
	    case SC_SCSI_BUS_FAULT:
		ret_code = ENOCONNECT;
		break;
	    default:
		ret_code = EIO;
		break;
	    }
	}
	else
	{
	    ret_code = EIO;
	}
    }

    /* if no other errors, and yet no data came back, then fail */
    if ((ret_code == 0) &&
	(bp->bufstruct.b_resid == bp->bufstruct.b_bcount))
    {
	ret_code = EIO;
    }
    /* No errors, so give the calling routine the data     */
    if (ret_code == 0)
    {
	/* Copy out the readblk data. If the buffer resides */
	/* user space, use copyin, else use bcopy.          */
	if (!(devflag & DKERNEL))
	{
	    ret_code = copyout(bp->bufstruct.b_un.b_addr,
			       sc_rd.data_ptr, sc_rd.blklen);
	    if (ret_code != 0)
	    {
		ret_code = EFAULT;
	    }
	}
	else
	{	/* buffer is in kernel space */
	    bcopy(bp->bufstruct.b_un.b_addr, sc_rd.data_ptr,
		  sc_rd.blklen);
	}
    }

    (void) xmfree(bp->bufstruct.b_un.b_addr, pinned_heap);
    (void) xmfree(bp, pinned_heap);	/* release sc_buf */
    DEBUG_0 ("Leaving psc_readblk routine.\n")
    return (ret_code);
}

/**************************************************************************/
/*                                                                        */
/* NAME:  psc_adp_str_init                                                */
/*                                                                        */
/* FUNCTION:  Initializes adapter stucture variables.                     */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can only be called by a process.                     */
/*                                                                        */
/* NOTES:  This routine is called to initialize the adapter structure     */
/*         variables, arrays, etc.                                        */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - scsi chip unique data structure                     */
/*                                                                        */
/* INPUTS:                                                                */
/*      None                                                              */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      None                                                              */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/**************************************************************************/
void
psc_adp_str_init()
{

    int     i;

    DEBUG_0 ("Entering psc_adp_str_init routine \n")

    /* init the adapter interrupt handler struct */
    adp_str.intr_struct.next = (struct intr *) NULL;
    adp_str.intr_struct.handler = psc_intr;
    adp_str.intr_struct.bus_type = adp_str.ddi.bus_type;
    adp_str.intr_struct.flags = 0;
    adp_str.intr_struct.level = adp_str.ddi.int_lvl;
    adp_str.intr_struct.priority = adp_str.ddi.int_prior;
    adp_str.intr_struct.bid = adp_str.ddi.bus_id;

    /* initialize other basic global variables to the adp_str */
    adp_str.channel_id = 0;
    adp_str.xmem_STA.aspace_id = XMEM_GLOBAL;
    adp_str.xmem_SCR.aspace_id = XMEM_GLOBAL;
    adp_str.iowait_inited = FALSE;
    adp_str.dump_inited = FALSE;
    adp_str.dump_started = FALSE;
    adp_str.dump_pri = 0;
    adp_str.num_4K_tcws_in_use = 0;

    /* initialize the various pointers in the structure */
    adp_str.DEVICE_ACTIVE_head = NULL;
    adp_str.DEVICE_ACTIVE_tail = NULL;
    adp_str.DEVICE_WAITING_head = NULL;
    adp_str.DEVICE_WAITING_tail = NULL;
    adp_str.DEVICE_WAITING_FOR_RESOURCES_head = NULL;
    adp_str.DEVICE_WAITING_FOR_RESOURCES_tail = NULL;
    adp_str.ABORT_BDR_head = NULL;
    adp_str.ABORT_BDR_tail = NULL;
    adp_str.large_TCW_table = NULL;

    for (i = 0; i < MAX_DEVICES; i++)
	adp_str.device_queue_hash[i] = NULL;
    for (i = 0; i < MAX_SCRIPTS; i++)
    {
	adp_str.SCRIPTS[i].script_ptr = NULL;
	adp_str.SCRIPTS[i].dma_ptr = 0;
	adp_str.SCRIPTS[i].TCW_index = 0;
	adp_str.SCRIPTS[i].in_use = SCR_UNUSED;
    }
    for (i = 0; i < TCW_TABLE_SIZE; i++)
	adp_str.TCW_alloc_table[i] = TCW_UNUSED;

    DEBUG_0 ("Leaving psc_adp_str_init routine \n")
    return;
}

/**************************************************************************/
/*                                                                        */
/* NAME:  psc_ioctl                                                       */
/*                                                                        */
/* FUNCTION:  Scsi Chip Device Driver Ioctl Routine                       */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can only be called by a process.                     */
/*                                                                        */
/* NOTES:  This routine will accept commands to perform specific function */
/*         and diagnostic operations on the scsi chip.  Supported commands*/
/*         are:                                                           */
/*                                                                        */
/*         IOCINFO     -  Returns information about the tape device.      */
/*         SCIOSTART   -  Open a SCSI ID/LUN                              */
/*         SCIOSTOP    -  Close a SCSI ID/LUN.                            */
/*         SCIOINQU    -  Issues a SCSI Inquiry command to a device.      */
/*         SCIOSTUNIT  -  Issues a SCSI start unit command to a device.   */
/*         SCIOTUR     -  Issues a SCSI test unit ready command to device.*/
/*         SCIOREAD    -  Issues a SCSI 6-byte read command to device.    */
/*         SCIOHALT    -  Issues a abort to SCSI device.                  */
/*         SCIORESET   -  Issues a Bus Device Reset to a SCSI device.     */
/*         SCIODIAG    -  Run chip diagnostics.                           */
/*         SCIOTRAM    -  No operation.  Returns ENXIO.                   */
/*         SCIODNLD    -  No operation.  Returns ENXIO.                   */
/*         SCIOSTARTTGT - No operation.  Returns ENXIO.                   */
/*         SCIOSTOPTGT  - No operation.  Returns ENXIO.                   */
/*         SCIOEVENT    - No operation.  Returns ENXIO.                   */
/*         SCIOGTHW     - No support for gathered writes.  Returns EINVAL.*/
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*                                                                        */
/* INPUTS:                                                                */
/*    devno  - major/minor number                                         */
/*    cmd    - code used to determine which operation to perform.         */
/*    arg    - address of a structure which contains values used in the   */
/*             'arg' operation.                                           */
/*    devflag - not used.                                                 */
/*    chan   - not used (for multiplexed devices).                        */
/*    ext    - not used.                                                  */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  0 for good completion, ERRNO value otherwise*/
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:  lockl, unlockl, bcopy, copyout            */
/*                                                                        */
/**************************************************************************/
int
psc_ioctl(
	  dev_t devno,	/* major and minor device numbers */
	  int cmd,	/* operation to perform */
	  int arg,	/* pointer to the user structure */
	  ulong devflag,	/* not used */
	  int chan,	/* not used */
	  int ext)	/* not used */

{
    int     rc, ret_code;
    struct devinfo scinfo;
    DEBUG_0 ("Entering psc_ioctl routine.\n")
    DEBUG_6("devno=%x,cmd=%x,arg=%x,devflag=%x,chan=%x,ext=%x\n",
	     devno, cmd, arg, devflag, chan, ext)

    ret_code = 0;       /* default to no errors found  */
    DDHKWD5(HKWD_DD_SCSIDD, DD_ENTRY_IOCTL, ret_code, devno,
	    cmd, devflag, chan, ext);
    /* lock the global lock to serialize with open/close/config */
    rc = lockl(&(psc_lock), LOCK_SHORT);	/* serialize this */
    if (rc != LOCK_SUCC)
    {
	DEBUG_0 ("Leaving psc_ioctl routine.\n")
	DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_IOCTL, ret_code, devno);
	return (EIO);	/* error--kernel service call failed */
    }
    if ((!adp_str_inited) || (!adp_str.opened))
    {	/* scsi chip has not been inited, defined, or opened */
	DEBUG_0 ("Leaving psc_ioctl routine.\n")
	unlockl(&(psc_lock));
	DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_IOCTL, ret_code, devno);
	return (EIO);
    }

    switch (cmd)
    {	/* cmd switch */
    case IOCINFO:	/* get device information */
	scinfo.devtype = DD_BUS;
	scinfo.flags = 0;
	scinfo.devsubtype = DS_SCSI;
	scinfo.un.scsi.card_scsi_id = (char) adp_str.ddi.card_scsi_id;
	scinfo.un.scsi.max_transfer = adp_str.max_request;
	if (!(devflag & DKERNEL))
	{  /* for a user process */
	    rc = copyout(&scinfo, (char *) arg,
			 sizeof(struct devinfo));
	    if (rc != 0)
		ret_code = EFAULT;
	}	/* for a user process */
	else
	{  /* for a kernel process */
	    bcopy(&scinfo, (char *) arg, sizeof(struct devinfo));
	}  /* for a kernel process */
	break;
    case SCIOSTART:	/* start a device */
	ret_code = psc_start_dev(INDEX(arg >> 8, arg));
	break;
    case SCIOSTOP:	/* stop a device */
	ret_code = psc_stop_dev(INDEX(arg >> 8, arg));
	break;
    case SCIOHALT:	/* issue a SCSI abort cmd */
	ret_code = psc_issue_abort(INDEX(arg >> 8, arg));
	break;
    case SCIORESET:	/* issue a SCSI abort cmd */
	ret_code = psc_issue_BDR(INDEX(arg >> 8, arg));
	break;
    case SCIOINQU:	/* issue a SCSI device inquiry cmd */
	ret_code = psc_inquiry(devno, arg, devflag);
	break;
    case SCIOSTUNIT:	/* issue a SCSI device start unit */
	ret_code = psc_start_unit(devno, arg, devflag);
	break;
    case SCIOTUR:	/* issue  SCSI device test unit ready */
	ret_code = psc_test_unit_rdy(devno, arg, devflag);
	break;
    case SCIOREAD:	/* issue a SCSI read cmd (6-byte) */
	ret_code = psc_readblk(devno, arg, devflag);
	break;
    case SCIODIAG:	/* run adapter diagnostics command */
	ret_code = psc_diagnostic(arg, devflag);
	break;
    case SCIOTRAM:	/* no-op, no chip ram to test */
    case SCIODNLD:	/* no-op, no microcode to download */
    case SCIOSTARTTGT:  /* no-op, target mode not supported */
    case SCIOSTOPTGT:   /* no-op, target mode not supported */
    case SCIOEVENT:     /* no-op, async event notification not supported */
	ret_code = ENXIO;
	break;
    default:	/* catch unknown ioctls and SCIOGTHW here */
	ret_code = EINVAL;
	break;
    }	/* cmd switch */

    unlockl(&(psc_lock));
    DEBUG_0 ("Leaving psc_ioctl routine.\n")
    DDHKWD1(HKWD_DD_SCSIDD, DD_EXIT_IOCTL, ret_code, devno);
    return (ret_code);
}

/**************************************************************************/
/*                                                                        */
/* NAME:        psc_build_command                                         */
/*                                                                        */
/* FUNCTION:    Builds an internal command for ioctl routines.            */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine is called to initialize fields within the sc_buf  */
/*         structure that is allocated via this routine.  This routine is */
/*         is called by ioctl routines that issue a command via the       */
/*         psc_strategy routine.                                          */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      sc_buf  - input/output request struct used between the adapter    */
/*                driver and the calling SCSI device driver               */
/*                                                                        */
/* INPUTS:                                                                */
/*      none                                                              */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      returns a pointer to the sc_buf, or NULL, if it could not         */
/*      be allocated.                                                     */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*      none                                                              */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*      xmalloc         bzero                                             */
/*                                                                        */
/**************************************************************************/
struct sc_buf *
psc_build_command()
{
    struct sc_buf *bp;

    DEBUG_0 ("Entering psc_build_command routine.\n")

    /* Allocate a sc_buf area for this command  */
    bp = (struct sc_buf *) xmalloc((uint) PAGESIZE,
				   (uint) PGSHIFT, pinned_heap);
    if (bp == NULL)
    {	/* xmalloc failed--return NULL pointer */
	DEBUG_0 ("Leaving psc_build_command routine.\n")
	return (NULL);
    }

    /* Clear the sc_buf structure to insure all */
    /* fields are initialized to zero.          */
    bzero(bp, sizeof(struct sc_buf));

    /* Initialize other fields of the sc_buf.   */
    bp->bufstruct.b_forw = NULL;
    bp->bufstruct.b_back = NULL;
    bp->bufstruct.av_forw = NULL;
    bp->bufstruct.av_back = NULL;
    bp->bufstruct.b_iodone = (void (*) ()) psc_iodone;
    bp->bufstruct.b_vp = NULL;
    bp->bufstruct.b_event = EVENT_NULL;
    bp->bufstruct.b_xmemd.aspace_id = XMEM_GLOBAL;
    bp->bufstruct.b_un.b_addr = (char *) bp + sizeof(struct sc_buf);

    /* Additional sc_buf initialization */
    bp->bp = NULL;	/* set for non-spanned command */
    bp->timeout_value = LONGWAIT;	/* set default timeout value */

    DEBUG_0 ("Leaving psc_build_command routine.\n")
    return (bp);
}

/************************************************************************/
/*                                                                      */
/* NAME:        psc_script_init                                         */
/*                                                                      */
/* FUNCTION:    Adapter Script Initialization Routine                   */
/*      This function will patch all interrupts with the half-word      */
/*      that makes up the hash index into the device structure hash     */
/*      table.  Next, it will patch all the target ids referenced       */
/*      within the script.  Then, it will patch the jump command of the */
/*      global IO_WAIT script to jump to this script when the target    */
/*      device does a reselection of the chip.  Finally, we prepare     */
/*      the negotiation, abort, and bdr scripts with the appropriate    */
/*      messages in case they are ever needed by this device.           */
/*                                                                      */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine can be called by a process.                        */
/*      It can page fault only if called under a process                */
/*      and the stack is not pinned.                                    */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*      None.                                                           */
/*                                                                      */
/* INPUTS:                                                              */
/*      *iowait_vir_addr - the virtual address pointing to the          */
/*              IO_WAIT script that all the scripts will be dependent   */
/*              on as a router.                                         */
/*      *script_vir_addr - the virtual address of the script just       */
/*              created and copied into memory.  The script that needs  */
/*              to be initialized.                                      */
/*      dev_info_hash - the hash value needed by the interrupt handler  */
/*              to determine which device-script caused the programmed  */
/*              interrupt.                                              */
/*                                                                      */
/* RETURN VALUE DESCRIPTION:                                            */
/*      None.                                                           */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED:                                          */
/*      None.                                                           */
/*                                                                      */
/************************************************************************/
void
psc_script_init(
		uint * iowait_vir_addr,
		uint * script_vir_addr,
		int dev_info_hash,
		uint iowait_dma_addr,
		uint script_dma_addr)

{
    ulong   top_half_word, id_bit_mask;
    ulong  *target_script, *io_wait_ptr;
    ulong   word_buffer;
    uint    jump_address;
    int     i, scsi_id;
    uchar   id_mask, initiator_mask;

    target_script = (ulong *) script_vir_addr;

    /******************** BEGIN INTERRUPT PATCHES **********************/

    /* Entering psc_script_init */

    /* patch all interrupts' msb half-word with the hash value for the */
    /* table hash_dev_info[]. */
    top_half_word = word_reverse((((ulong) dev_info_hash) << 16));
    for (i = 0; i < 18; i++)
    {
	target_script[A_phase_error_Used[i]] &= 0xFFFF0000;
	target_script[A_phase_error_Used[i]] |= top_half_word;
    }
    target_script[A_io_done_Used[0]] &= 0xFFFF0000;
    target_script[A_io_done_Used[0]] |= top_half_word;

    target_script[A_io_done_after_data_Used[0]] &= 0xFFFF0000;
    target_script[A_io_done_after_data_Used[0]] |= top_half_word;

    for (i = 0; i < 2; i++)
    {
	target_script[A_unexpected_status_Used[i]] &= 0xFFFF0000;
	target_script[A_unexpected_status_Used[i]] |= top_half_word;

    }
    target_script[A_unknown_msg_Used[0]] &= 0xFFFF0000;
    target_script[A_unknown_msg_Used[0]] |= top_half_word;

    target_script[A_sync_msg_reject_Used[0]] &= 0xFFFF0000;
    target_script[A_sync_msg_reject_Used[0]] |= top_half_word;
    target_script[A_sync_msg_reject_Used[1]] &= 0xFFFF0000;
    target_script[A_sync_msg_reject_Used[1]] |= top_half_word;

    target_script[A_check_next_io_Used[0]] &= 0xFFFF0000;
    target_script[A_check_next_io_Used[0]] |= top_half_word;

    target_script[A_check_next_io_data_Used[0]] &= 0xFFFF0000;
    target_script[A_check_next_io_data_Used[0]] |= top_half_word;

    target_script[A_ext_msg_Used[0]] &= 0xFFFF0000;
    target_script[A_ext_msg_Used[0]] |= top_half_word;

    target_script[A_cmd_select_atn_failed_Used[0]] &= 0xFFFF0000;
    target_script[A_cmd_select_atn_failed_Used[0]] |= top_half_word;

    target_script[A_err_not_ext_msg_Used[0]] &= 0xFFFF0000;
    target_script[A_err_not_ext_msg_Used[0]] |= top_half_word;

    target_script[A_sync_neg_done_Used[0]] &= 0xFFFF0000;
    target_script[A_sync_neg_done_Used[0]] |= top_half_word;

    target_script[A_abort_msg_error_Used[0]] &= 0xFFFF0000;
    target_script[A_abort_msg_error_Used[0]] |= top_half_word;

    target_script[A_neg_select_failed_Used[0]] &= 0xFFFF0000;
    target_script[A_neg_select_failed_Used[0]] |= top_half_word;

    target_script[A_abort_select_failed_Used[0]] &= 0xFFFF0000;
    target_script[A_abort_select_failed_Used[0]] |= top_half_word;

    target_script[A_abort_io_complete_Used[0]] &= 0xFFFF0000;
    target_script[A_abort_io_complete_Used[0]] |= top_half_word;

    target_script[R_dummy_int_Used[0] - 1] = word_reverse(0x80000000);
    target_script[R_dummy_int_Used[0]] = 0x00000000;

    /*********************************************************************/
    /* The three interrupts, illegal_target_id, unknown_reselect_id, and */
    /* uninitialized_reselect will not be patched over with the          */
    /* top_half_word because these cases represent ones in which we do   */
    /* not have a script associated with a device.  Since we don't       */
    /* recognize the device, it will not exist in the dev_info hash table*/
    /* that the top_half_word is used for.  Thus, it is not included     */
    /* because it is not a valid assignment-patch to the interrupts.     */
    /*********************************************************************/

    /*********************************************************************/
    /* We will patch all the jump values with the virtual address where  */
    /* we want the jump to go to.  We take the relative jump value       */
    /* (in bytes) sitting at the jump address and add it to the base,    */
    /* virtual address and write that back into the jump address         */
    /* location.  We must be sure to take the WORD REVERSE value before  */
    /* and after the addition of the base address.                       */
    /*********************************************************************/
    DEBUG_1 ("script dma address is >>> %x\n", script_dma_addr);
    for (i = 0; i < PATCHES; i++)
    {
	jump_address = word_reverse(PSC_SCRIPT[LABELPATCHES[i]]);
	jump_address += script_dma_addr;
	target_script[LABELPATCHES[i]] = word_reverse(jump_address);
    }
    /*********************************************************************/
    /* Patch the interrupt for the case where we have been reselected by */
    /* a target but we don't have a valid, set jump point.  Since the    */
    /* top_half_word represents a recognized, distinct scsi id, we must  */
    /* patch this interrupt individually everytime a new script is       */
    /* generated and this function is called to intialize the new script */
    /* for its new device.                                               */
    /*********************************************************************/

    io_wait_ptr = (ulong *) iowait_vir_addr;
    /* We only want to do this once, at the first open of the device */
    if ((io_wait_ptr == target_script) && (!(adp_str.iowait_inited)))
    {
	/***************************************************************/
	/* Find out what the initiator's SCSI ID is, then make this an */
	/* illegal target id.  If a target ever selects the script     */
	/* with this id, we will hit the illegal_target_id interupt    */
	/***************************************************************/
	initiator_mask = (0x01 << adp_str.ddi.card_scsi_id);
	id_mask = 0x01;
	for (i = 4; i < 20; i += 2)
	{
	    if (initiator_mask == id_mask)
	    {
		io_wait_ptr[i] = word_reverse(0x800C00FF);
	    }
	    else
	    {
		io_wait_ptr[i] = word_reverse(0x800C0000 |
				      ((ulong) (id_mask | initiator_mask)));
	    }
	    id_mask = (id_mask << 1);
	}
	for (i = 0; i < 8; i++)
	{
	    io_wait_ptr[scsi_0_lun_Used[i]] =
		word_reverse(((A_uninitialized_reselect_Used[0]
			       - 1) * 4 + script_dma_addr));
	    io_wait_ptr[scsi_1_lun_Used[i]] =
		word_reverse(((A_uninitialized_reselect_Used[1]
			       - 1) * 4 + script_dma_addr));
	    io_wait_ptr[scsi_2_lun_Used[i]] =
		word_reverse(((A_uninitialized_reselect_Used[2]
			       - 1) * 4 + script_dma_addr));
	    io_wait_ptr[scsi_3_lun_Used[i]] =
		word_reverse(((A_uninitialized_reselect_Used[3]
			       - 1) * 4 + script_dma_addr));
	    io_wait_ptr[scsi_4_lun_Used[i]] =
		word_reverse(((A_uninitialized_reselect_Used[4]
			       - 1) * 4 + script_dma_addr));
	    io_wait_ptr[scsi_5_lun_Used[i]] =
		word_reverse(((A_uninitialized_reselect_Used[5]
			       - 1) * 4 + script_dma_addr));
	    io_wait_ptr[scsi_6_lun_Used[i]] =
		word_reverse(((A_uninitialized_reselect_Used[6]
			       - 1) * 4 + script_dma_addr));
	    io_wait_ptr[scsi_7_lun_Used[i]] =
		word_reverse(((A_uninitialized_reselect_Used[7]
			       - 1) * 4 + script_dma_addr));
	    io_wait_ptr[lun_msg_addr_Used[i]] =
		word_reverse(iowait_dma_addr + Ent_lun_msg_buf);
	}

	io_wait_ptr[(Ent_lun_msg_buf / 4)] = 0x00000000;
	io_wait_ptr[(Ent_lun_msg_buf / 4) + 1] = 0x00000000;

	adp_str.iowait_inited = TRUE; /* only do this once - ever */
    }
    /*********************** END INTERRUPT PATCHES ***********************/
    /*********************** BEGIN ID PATCHES ****************************/

    /*********************************************************************/
    /* We need to set the SCSI ID bits in the format required by SCSI    */
    /* protocol and use the values of the id_bit_mask set in the previous*/
    /* switch-case code.  These ID patches are used in all the SELECT ATN*/
    /* commands to start the SCSI protocol with a target device.         */
    /*********************************************************************/
    scsi_id = SID(dev_info_hash);
    switch (scsi_id)
    {
    case 0:
	id_bit_mask = 0x00010000;
	break;
    case 1:
	id_bit_mask = 0x00020000;
	break;
    case 2:
	id_bit_mask = 0x00040000;
	break;
    case 3:
	id_bit_mask = 0x00080000;
	break;
    case 4:
	id_bit_mask = 0x00100000;
	break;
    case 5:
	id_bit_mask = 0x00200000;
	break;
    case 6:
	id_bit_mask = 0x00400000;
	break;
    case 7:
	id_bit_mask = 0x00800000;
	break;
    default:
	break;
    }
    for (i = 0; i < 4; i++)
    {

	word_buffer = word_reverse(target_script[R_target_id_Used[i]]);
	word_buffer = ((word_buffer & 0xFF00FFFF) | id_bit_mask);
	target_script[R_target_id_Used[i]] = word_reverse(word_buffer);
    }
    /**********************************************************************/
    /* point to the location in memory where our identify_msg_buf resides */
    /* This is used for the regular command, sync, abort, or bdr cases.   */
    /**********************************************************************/
    target_script[identify_msg_addr_Used[0]] =
	word_reverse(script_dma_addr + Ent_identify_msg_buf);
    /**********************************************************************/
    /* initialize the identify_msg_buffer to the lun id of the device     */
    /* this script is associated with.  Bit 7 is set to show that it is   */
    /* an identify message.  Bit 6 is set to show we allow disconnections.*/
    /* The lun pattern of (0-8) is held in Bits 2-0.                      */
    /**********************************************************************/
    id_bit_mask = 0xC0000000;
    id_bit_mask |= ((ulong) LUN(dev_info_hash) << 24);
    target_script[(Ent_identify_msg_buf / 4)] = id_bit_mask;
    target_script[(Ent_identify_msg_buf / 4) + 1] = 0x00000000;

    /**************** END ID PATCHES **********************/
    /**************** BEGIN BUFFER PATCHES ****************/
    /* point to the location in memory where our cmd_msg_in_buf resides. */
    for (i = 0; i < 4; i++)
    {
	target_script[cmd_msg_in_addr_Used[i]] =
	    word_reverse(script_dma_addr + Ent_cmd_msg_in_buf);
    }
    /* Clear out the message buffers used in synchronous negotiation.    */
    target_script[(Ent_cmd_msg_in_buf / 4)] = 0x00000000;
    target_script[(Ent_cmd_msg_in_buf / 4) + 1] = 0x00000000;
    for (i = 0; i < 4; i++)
    {
	target_script[status_addr_Used[i]] =
	    word_reverse(script_dma_addr + Ent_status_buf);
    }
    target_script[reject_msg_addr_Used[0]] =
	word_reverse(script_dma_addr + Ent_reject_msg_buf);
    target_script[(Ent_reject_msg_buf / 4)] = 0x07000000;
    target_script[(Ent_reject_msg_buf / 4) + 1] = 0x00000000;
    /* point to the location in memory where our sync_msg_out_buf resides. */
    target_script[sync_msg_out_addr_Used[0]] =
	word_reverse(script_dma_addr + Ent_sync_msg_out_buf);
    target_script[sync_msg_out_addr2_Used[0]] =
	word_reverse(script_dma_addr + Ent_sync_msg_out_buf2);

    /* Patch the messages needed to do synchronous negotiation.           */
    id_bit_mask = 0xC0000000;
    id_bit_mask |= ((ulong) LUN(dev_info_hash) << 24);
    target_script[(Ent_sync_msg_out_buf / 4)] = 0x00010301 | id_bit_mask;
    word_buffer = 0;
    word_buffer = DEFAULT_MIN_PHASE;
    word_buffer = (word_buffer << 8) | DEFAULT_BYTE_BUF;
    word_buffer = (word_buffer << 16);
    target_script[(Ent_sync_msg_out_buf / 4) + 1] = word_buffer;

    /* Patch the messages needed to do synchronous negotiation.           */
    target_script[(Ent_sync_msg_out_buf2 / 4)] = (0x01030100 |
						  (ulong) DEFAULT_MIN_PHASE);
    target_script[(Ent_sync_msg_out_buf2 / 4) + 1] =
	((ulong) DEFAULT_BYTE_BUF << 24);

    /* point to the location in memory where our cmd_buf resides.         */
    target_script[cmd_addr_Used[0]] =
	word_reverse(script_dma_addr + Ent_cmd_buf);

    /* Clear out the buffers to be used to hold the SCSI commands         */
    target_script[(Ent_cmd_buf / 4)] = 0x00000000;
    target_script[(Ent_cmd_buf / 4) + 1] = 0x00000000;
    target_script[(Ent_cmd_buf / 4) + 2] = 0x00000000;
    target_script[(Ent_cmd_buf / 4) + 3] = 0x00000000;

    /* Clear out the status buffers                                       */
    target_script[(Ent_status_buf / 4)] = 0x00000000;
    target_script[(Ent_status_buf / 4) + 1] = 0x00000000;

    /* point to the location in memory where our extended_msg_buf resides. */
    for (i = 0; i < 5; i++)
	target_script[extended_msg_addr_Used[i]] =
	    word_reverse(script_dma_addr + Ent_extended_msg_buf);

    /* Clear out the extended msg buffers                                 */
    target_script[(Ent_extended_msg_buf / 4)] = 0x00000000;
    target_script[(Ent_extended_msg_buf / 4) + 1] = 0x00000000;

    /* point to the location in memory where our abort_msg_out_buf resides */
    for (i = 0; i < 3; i++)
	target_script[abort_bdr_msg_out_addr_Used[i]] =
	    word_reverse(script_dma_addr + Ent_abort_bdr_msg_out_buf);

    /* Clear out the abort_msg_out msg buffers and set the abort message  */
    target_script[(Ent_abort_bdr_msg_out_buf / 4)] = 0x06000000;
    target_script[(Ent_abort_bdr_msg_out_buf / 4) + 1] = 0x00000000;

    /* point to location in memory where our abort_bdr_msg_in_buf resides. */

    for (i = 0; i < 2; i++)
	target_script[abort_bdr_msg_in_addr_Used[i]] =
	    word_reverse(script_dma_addr + Ent_abort_bdr_msg_in_buf);

    /* Clear out the abort_msg_in msg buffers                             */
    target_script[(Ent_abort_bdr_msg_in_buf / 4)] = 0x00000000;
    target_script[(Ent_abort_bdr_msg_in_buf / 4) + 1] = 0x00000000;
    /**************** END BUFFER PATCHES ******************/
    return;
}

/**************************************************************************/
/*                                                                        */
/* NAME:  psc_diagnostic                                                  */
/*                                                                        */
/* FUNCTION:  Runs chip diagnostics.                                      */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine causes diagnostics to be run.                     */
/*         The calling process is responsible for NOT calling this rou-   */
/*         tine if the SCIOSTART failed.  Such a failure would indicate   */
/*         that another process has this device open and interference     */
/*         could cause improper error reporting.                          */
/*                                                                        */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    scsi chip structure - scsi chip information.                        */
/*    diagnostic structure - diagnostic information.                      */
/*                                                                        */
/* INPUTS:                                                                */
/*    cmd       - command arguments.                                      */
/*    devflag   - device flag.                                            */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  0 for good completion, ERRNO value otherwise*/
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/**************************************************************************/
int
psc_diagnostic(
	       int cmd,
	       ulong devflag)
{
    int     rc, ret_code;
    struct sc_card_diag *diag_ptr;
    struct sc_card_diag diag;
    caddr_t iocc_addr;

    DEBUG_0 ("Entering psc_diagnostic\n")
    if (!(devflag & DKERNEL))
    {	/* handle user process */
	rc = copyin((char *) cmd, &diag, sizeof(struct sc_card_diag));
	if (rc != 0)
	{
	    return (EFAULT);
	}
    }
    else
    {
	/* handle kernel process */
	bcopy((char *) cmd, &diag, sizeof(struct sc_card_diag));
    }

    rc = 0;
    diag_ptr = &diag;
    bzero(&diag_ptr->diag_rc, sizeof(struct rc));
    if (adp_str.opened != TRUE)
    {
	DEBUG_0 ("psc_diag: adapter not opened\n")
	return EINVAL;
    }
    if (adp_str.open_mode == NORMAL_MODE)
    {
	DEBUG_0 ("psc_diag: adapter not opened in diag mode\n")
	return EACCES;
    }
    switch (diag_ptr->option)
    {
    case SC_CARD_DIAGNOSTICS:
	rc = psc_run_diagnostics(diag_ptr);
	break;
    case SC_RESUME_DIAGNOSTICS:
	rc = psc_run_diagnostics(diag_ptr);
	break;
    case SC_CARD_SCSI_WRAP:
	rc = psc_loop_test_diagnostics(diag_ptr);
	break;
    case SC_CARD_REGS_TEST:
	rc = psc_register_test(diag_ptr);
	if (rc != 0)
	{
	    diag_ptr->diag_rc.diag_stat = SC_DIAG_MISCMPR;
	    diag_ptr->diag_rc.diag_validity = 0x01;
	    diag_ptr->diag_rc.ahs_validity = 0x01;
	    diag_ptr->diag_rc.ahs = PIO_RD_OTHR_ERR;
	}
	break;
    case SC_CARD_POS_TEST:
	rc = psc_pos_register_test();
	if (rc != 0)
	{
	    diag_ptr->diag_rc.diag_stat = SC_DIAG_MISCMPR;
	    diag_ptr->diag_rc.diag_validity = 0x01;
	    diag_ptr->diag_rc.ahs_validity = 0x01;
	    diag_ptr->diag_rc.ahs = PIO_RD_OTHR_ERR;
	}
	break;
    case SC_SCSI_BUS_RESET:
	rc = psc_diag_reset_scsi_bus();
	break;
    default:
	DEBUG_0 ("psc_diag: Invalid command arg\n")
	rc = EINVAL;
	break;
    }
    /* Copy the status bytes to the user space provided. */
    if (!(devflag & DKERNEL))
    {	/* handle user process */
	ret_code = copyout(diag_ptr, (char *) cmd,
			   sizeof(struct sc_card_diag));
	if (ret_code != 0)
	    rc = EFAULT;
    }
    else
    {	/* handle kernel process */
	bcopy(diag_ptr, (char *) cmd, sizeof(struct sc_card_diag));
    }
    /* Assert a chip reset */
    iocc_addr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);
    BUS_PUTCX((iocc_addr + DCNTL), 0x01);
    psc_delay(1);
    BUS_PUTCX((iocc_addr + DCNTL), 0x00);
    /* chip register init */
    BUS_PUTCX((iocc_addr + SCNTL0), SCNTL0_INIT);
    BUS_PUTCX((iocc_addr + SCNTL1), SCNTL1_INIT);
    BUS_PUTCX((iocc_addr + SIEN), SIEN_MASK);
    BUS_PUTCX((iocc_addr + SCID), (0x01 << adp_str.ddi.card_scsi_id));
    BUS_PUTCX((iocc_addr + SXFER), SYNC_VAL);
    BUS_PUTCX((iocc_addr + DMODE), DMODE_INIT);
    BUS_PUTCX((iocc_addr + DIEN), DIEN_MASK);
    BUS_PUTCX((iocc_addr + DCNTL), DCNTL_INIT);

    BUSIO_DET(iocc_addr);
    DEBUG_0 ("Leaving psc_diagnostic\n");
    return rc;
}  /* end psc_diagnostic */

/**************************************************************************/
/*                                                                        */
/* NAME:  psc_run_diagnostics                                             */
/*                                                                        */
/* FUNCTION:  Runs a test of the chip DMA and SCSI FIFOs.                 */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine causes chip diagnostics to be run.  This involves */
/*         doing write, read, and compare of the DMA ans SCSI FIFO on the */
/*         chip.  Parity is also tested during this phase.  In addition, a*/
/*         test of the chip parity logic is performed.                    */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    scsi chip structure - scsi chip information.                        */
/*    diagnostic structure - diagnostic information.                      */
/*                                                                        */
/* INPUTS:                                                                */
/*    diag_ptr - diagnostic structure pointer                             */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  0 for good completion, ERRNO value otherwise*/
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/**************************************************************************/
int
psc_run_diagnostics(
		    struct sc_card_diag * diag_ptr)

{
    int     rc, count1, count2;
    char    pio_rc;
    caddr_t iocc_addr;

    DEBUG_0 ("Entering psc_run_diagnostics\n")
    TRACE_1 ("in run_diag", (int) diag_ptr);
    diag_ptr->diag_rc.diag_stat = 0;
    diag_ptr->diag_rc.diag_validity = 0;
    diag_ptr->diag_rc.ahs_validity = 0;
    diag_ptr->diag_rc.ahs = 0;
    iocc_addr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);
    /* The proceeding section of code writes the entire chip DMA */
    /* FIFO and then performs a readback check.                  */
    for (count1 = 4; count1 < 8; count1++)
    {
	/* enable the the DMA FIFO lane. */
	if ((BUS_PUTCX((iocc_addr + CTEST4), count1)) != 0)
	{
	    DEBUG_0 ("psc_run_diag: error enabling DMA FIFO lane\n");
	    TRACE_1 ("failed C4", 0);
	    TRACE_1 ("out run_diag", EFAULT);
	    BUSIO_DET(iocc_addr);
	    return EFAULT;
	}
	for (count2 = 1; count2 < 9; count2++)
	{
	    /* force even parity. */
	    if ((BUS_PUTCX((iocc_addr + CTEST7), 0x00)) != 0)
	    {
		DEBUG_0 ("psc_run_diag: error writing CTEST7\n");
		TRACE_1 ("failed C7", 0);
		TRACE_1 ("out run_diag", EFAULT);
		BUSIO_DET(iocc_addr);
		return EFAULT;
	    }
	    /* write a data pattern with even parity to the DMA FIFO. */
	    if ((BUS_PUTCX((iocc_addr + CTEST6), 0xAA)) != 0)
	    {
		DEBUG_0 ("psc_run_diag: error writing CTEST6\n");
		TRACE_1 ("failed C6", 0);
		TRACE_1 ("out run_diag", EFAULT);
		BUSIO_DET(iocc_addr);
		return EFAULT;
	    }
	}
    }
    /* Read DSTAT reg to check DMA FIFO Empty bit. */
    BUS_GETCX((iocc_addr + DSTAT), &pio_rc);
    pio_rc &= 0x80;
    if (pio_rc != 0)
    {
	DEBUG_0 ("psc_run_diag: DMA FIFO empty\n");
	TRACE_1 ("dfifo empty", 0);
	TRACE_1 ("out run_diag", EFAULT);
	(void) BUS_PUTCX((iocc_addr + CTEST4), 0x00);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    for (count1 = 4; count1 < 8; count1++)
    {
	/* enable the the DMA FIFO lane. */
	if ((BUS_PUTCX((iocc_addr + CTEST4), count1)) != 0)
	{
	    DEBUG_0 ("psc_run_diag: error enabling DMA FIFO lane\n");
	    TRACE_1 ("failed CT4", 0);
	    TRACE_1 ("out run_diag", EFAULT);
	    diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	    diag_ptr->diag_rc.diag_validity = 0x01;
	    diag_ptr->diag_rc.ahs_validity = 0x01;
	    diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	    BUSIO_DET(iocc_addr);
	    return EFAULT;
	}
	for (count2 = 1; count2 < 9; count2++)
	{
	    BUS_GETCX((iocc_addr + CTEST6), &pio_rc);
	    if (pio_rc != 0xAA)
	    {
		DEBUG_0 ("psc_run_diag: compare error CTEST6\n");
		TRACE_1 ("failed CT6", 0);
		TRACE_1 ("out run_diag", EFAULT);
		BUS_PUTCX((iocc_addr + CTEST4), 0x00);
		diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
		diag_ptr->diag_rc.diag_validity = 0x01;
		diag_ptr->diag_rc.ahs_validity = 0x01;
		diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
		BUSIO_DET(iocc_addr);
		return EFAULT;
	    }
	    BUS_GETCX((iocc_addr + CTEST2), &pio_rc);
	    pio_rc &= 0x08;  /* check dfp bit */
	    if (pio_rc != 0)
	    {
		DEBUG_0 ("psc_run_diag: Even parity error in CTEST2\n");
		TRACE_1 ("even par err", 0xc2);
		TRACE_1 ("out run_diag", EFAULT);
		diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
		diag_ptr->diag_rc.diag_validity = 0x01;
		diag_ptr->diag_rc.ahs_validity = 0x01;
		diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
		BUS_PUTCX((iocc_addr + CTEST4), 0x00);
		BUSIO_DET(iocc_addr);
		return EFAULT;
	    }
	}	/* end count2 loop */
    }	/* end count1 loop */
    BUS_GETCX((iocc_addr + DSTAT), &pio_rc);
    pio_rc &= 0x80;
    if (pio_rc != 0x80)
    {
	DEBUG_0 ("psc_run_diag: DMA FIFO not empty\n");
	BUS_PUTCX((iocc_addr + CTEST4), 0x00);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    for (count1 = 4; count1 < 8; count1++)
    {
	/* enable the the DMA FIFO lane. */
	if ((BUS_PUTCX((iocc_addr + CTEST4), count1)) != 0)
	{
	    DEBUG_0 ("psc_run_diag: error enabling DMA FIFO lane\n");
	    TRACE_1 ("failed CTS4", 0);
	    TRACE_1 ("out run_diag", EFAULT);
	    diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	    diag_ptr->diag_rc.diag_validity = 0x01;
	    diag_ptr->diag_rc.ahs_validity = 0x01;
	    diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	    BUSIO_DET(iocc_addr);
	    return EFAULT;
	}
	for (count2 = 1; count2 < 9; count2++)
	{
	    /* force odd parity. */
	    if ((BUS_PUTCX((iocc_addr + CTEST7), 0x08)) != 0)
	    {
		DEBUG_0 ("psc_run_diag: error writing CTEST7\n");
		TRACE_1 ("failed CTS7", 0);
		TRACE_1 ("out run_diag", EFAULT);
		diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
		diag_ptr->diag_rc.diag_validity = 0x01;
		diag_ptr->diag_rc.ahs_validity = 0x01;
		diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
		BUSIO_DET(iocc_addr);
		return EFAULT;
	    }
	    /* write a data pattern with odd parity to the DMA FIFO. */
	    if ((BUS_PUTCX((iocc_addr + CTEST6), 0x55)) != 0)
	    {
		DEBUG_0 ("run_diag:error CTEST6 odd parity\n");
		TRACE_1 ("failed CTS6", 0);
		TRACE_1 ("out run_diag", EFAULT);
		diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
		diag_ptr->diag_rc.diag_validity = 0x01;
		diag_ptr->diag_rc.ahs_validity = 0x01;
		diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
		BUSIO_DET(iocc_addr);
		return EFAULT;
	    }
	}	/* end count2 loop */
    }	/* end count1 loop */

    /* Read DSTAT reg to check DMA FIFO Empty bit. */
    rc = psc_read_reg(DSTAT, DSTAT_SIZE);
    BUS_GETCX((iocc_addr + DSTAT), &pio_rc);
    pio_rc &= 0x80;
    if (pio_rc == 0x80)
    {
	DEBUG_0 ("psc_run_diag: DMA FIFO empty\n");
	TRACE_1 ("dfifo empty", 0);
	TRACE_1 ("out run_diag", EFAULT);
	BUS_PUTCX((iocc_addr + CTEST4), 0x00);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    for (count1 = 4; count1 < 8; count1++)
    {
	/* enable the the DMA FIFO lane. */
	if ((BUS_PUTCX((iocc_addr + CTEST4), count1)) != 0)
	{
	    DEBUG_0 ("psc_run_diag: error enabling DMA FIFO lane\n");
	    TRACE_1 ("failed CTST4", 0);
	    TRACE_1 ("out run_diag", EFAULT);
	    diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	    diag_ptr->diag_rc.diag_validity = 0x01;
	    diag_ptr->diag_rc.ahs_validity = 0x01;
	    diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	    BUSIO_DET(iocc_addr);
	    return EFAULT;
	}
	for (count2 = 1; count2 < 9; count2++)
	{
	    BUS_GETCX((iocc_addr + CTEST6), &pio_rc);
	    if (pio_rc != 0x55)
	    {
		DEBUG_0 ("psc_run_diag: error comparing CTEST6 odd parity\n");
		TRACE_1 ("failed CTST6", 0);
		TRACE_1 ("out run_diag", EFAULT);
		diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
		diag_ptr->diag_rc.diag_validity = 0x01;
		diag_ptr->diag_rc.ahs_validity = 0x01;
		diag_ptr->diag_rc.ahs = PIO_RD_OTHR_ERR;
		BUS_PUTCX((iocc_addr + CTEST4), 0x00);
		BUSIO_DET(iocc_addr);
		return EFAULT;
	    }
	    BUS_GETCX((iocc_addr + CTEST2), &pio_rc);
	    pio_rc &= 0x08;  /* check dfp bit */
	    if (pio_rc != 0x08)
	    {
		DEBUG_0 ("psc_run_diag:Odd parity err CTEST2\n");
		TRACE_1 ("odd par err", 0);
		TRACE_1 ("out run_diag", EFAULT);
		diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
		diag_ptr->diag_rc.diag_validity = 0x01;
		diag_ptr->diag_rc.ahs_validity = 0x01;
		diag_ptr->diag_rc.ahs = PIO_RD_OTHR_ERR;
		BUS_PUTCX((iocc_addr + CTEST4), 0x00);
		BUSIO_DET(iocc_addr);
		return EFAULT;
	    }
	}	/* end count2 loop */
    }	/* end count1 loop */

    BUS_GETCX((iocc_addr + DSTAT), &pio_rc);
    pio_rc &= 0x80;
    if (pio_rc != 0x80)
    {
	DEBUG_0 ("psc_run_diag: DMA FIFO not empty\n");
	TRACE_1 ("dfifo !empty", 0);
	TRACE_1 ("out run_diag", EFAULT);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_RD_OTHR_ERR;
	BUS_PUTCX((iocc_addr + CTEST4), 0x00);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    /* The following section of code writes the entire chip SCSI  */
    /* FIFO and then performs a readback check.                   */
    if ((BUS_PUTCX((iocc_addr + SCNTL0), 0x04)) != 0)
    {
	DEBUG_0 ("psc_run_diag: error enabling parity in SCNTL0\n");
	TRACE_1 ("enable par err", 0);
	TRACE_1 ("out run_diag", EFAULT);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    /* start check for even parity logic */
    if ((BUS_PUTCX((iocc_addr + SCNTL1), 0x04)) != 0)
    {
	DEBUG_0 ("psc_run_diag: error forcing even parity in SCNTL1\n");
	TRACE_1 ("even par sctnl1", 0);
	TRACE_1 ("out run_diag", EFAULT);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    if ((BUS_PUTCX((iocc_addr + CTEST4), 0x08)) != 0)
    {
	DEBUG_0 ("psc_run_diag: error enabling SCSI FIFO in CTEST4\n");
	TRACE_1 ("failed en sfifo", 0);
	TRACE_1 ("out run_diag", EFAULT);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    for (count1 = 1; count1 < 9; count1++)
    {
	if ((BUS_PUTCX((iocc_addr + SODL), 0xAA)) != 0)
	{
	    DEBUG_0 ("psc_run_diag: error writing SODL for even parity\n");
	    TRACE_1 ("failed sodl", 0);
	    TRACE_1 ("out run_diag", EFAULT);
	    diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	    diag_ptr->diag_rc.diag_validity = 0x01;
	    diag_ptr->diag_rc.ahs_validity = 0x01;
	    diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	    BUSIO_DET(iocc_addr);
	    return EFAULT;
	}
    }
    for (count1 = 1; count1 < 9; count1++)
    {
	rc = psc_read_reg(CTEST3, CTEST3_SIZE);
	BUS_GETCX((iocc_addr + CTEST3), &pio_rc);
	if (pio_rc != 0xAA)
	{
	    DEBUG_0 ("psc_run_diag: error comparing CTEST3\n");
	    TRACE_1 ("failed c3", 0);
	    TRACE_1 ("out run_diag", EFAULT);
	    diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	    diag_ptr->diag_rc.diag_validity = 0x01;
	    diag_ptr->diag_rc.ahs_validity = 0x01;
	    diag_ptr->diag_rc.ahs = PIO_RD_OTHR_ERR;
	    BUS_PUTCX((iocc_addr + SCNTL0), 0x00);
	    BUS_PUTCX((iocc_addr + SCNTL1), 0x00);
	    BUS_PUTCX((iocc_addr + CTEST4), 0x00);
	    BUSIO_DET(iocc_addr);
	    return EFAULT;
	}
	BUS_GETCX((iocc_addr + CTEST2), &pio_rc);
	pio_rc &= 0x10;
	if (pio_rc != 0)
	{
	    DEBUG_0 ("psc_run_diag: error checking parity in CTEST2\n");
	    TRACE_1 ("failed c2", pio_rc);
	    TRACE_1 ("out run_diag", EFAULT);
	    diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	    diag_ptr->diag_rc.diag_validity = 0x01;
	    diag_ptr->diag_rc.ahs_validity = 0x01;
	    diag_ptr->diag_rc.ahs = PIO_RD_OTHR_ERR;
	    BUS_PUTCX((iocc_addr + SCNTL0), 0x00);
	    BUS_PUTCX((iocc_addr + SCNTL1), 0x00);
	    BUS_PUTCX((iocc_addr + CTEST4), 0x00);
	    BUSIO_DET(iocc_addr);
	    return EFAULT;
	}
    }	/* end count1 loop */

    /* Now check the odd parity logic */
    if ((BUS_PUTCX((iocc_addr + SCNTL1), 0x00)) != 0)
    {
	DEBUG_0 ("psc_run_diag: error forcing even parity in SCNTL1\n");
	TRACE_1 ("failed scntl1", 0);
	TRACE_1 ("out run_diag", EFAULT);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    for (count1 = 1; count1 < 9; count1++)
    {
	if ((BUS_PUTCX((iocc_addr + SODL), 0x55)) != 0)
	{
	    DEBUG_0 ("run_diag: error writing SODL odd parity\n");
	    TRACE_1 ("failed odd par", SODL);
	    TRACE_1 ("out run_diag", EFAULT);
	    diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	    diag_ptr->diag_rc.diag_validity = 0x01;
	    diag_ptr->diag_rc.ahs_validity = 0x01;
	    diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	    BUSIO_DET(iocc_addr);
	    return EFAULT;
	}
    }
    for (count1 = 1; count1 < 9; count1++)
    {
	BUS_GETCX((iocc_addr + CTEST3), &pio_rc);
	if (pio_rc != 0x55)
	{
	    DEBUG_0 ("run_diag: compare error CTEST3 odd parity\n");
	    TRACE_1 ("failed compare", 0);
	    TRACE_1 ("out run_diag", EFAULT);
	    diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	    diag_ptr->diag_rc.diag_validity = 0x01;
	    diag_ptr->diag_rc.ahs_validity = 0x01;
	    diag_ptr->diag_rc.ahs = PIO_RD_OTHR_ERR;
	    BUS_PUTCX((iocc_addr + SCNTL0), 0x00);
	    BUS_PUTCX((iocc_addr + SCNTL1), 0x00);
	    BUS_PUTCX((iocc_addr + CTEST4), 0x00);
	    BUSIO_DET(iocc_addr);
	    return EFAULT;
	}
	BUS_GETCX((iocc_addr + CTEST2), &pio_rc);
	pio_rc &= 0x10;
	if (pio_rc != 0x10)
	{
	    DEBUG_0 ("psc_run_diag: error checking parity in CTEST2\n");
	    TRACE_1 ("failed par chk", 0);
	    TRACE_1 ("out run_diag", EFAULT);
	    diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	    diag_ptr->diag_rc.diag_validity = 0x01;
	    diag_ptr->diag_rc.ahs_validity = 0x01;
	    diag_ptr->diag_rc.ahs = PIO_RD_OTHR_ERR;
	    BUS_PUTCX((iocc_addr + SCNTL0), 0x00);
	    BUS_PUTCX((iocc_addr + SCNTL1), 0x00);
	    BUS_PUTCX((iocc_addr + CTEST4), 0x00);
	    BUSIO_DET(iocc_addr);
	    return EFAULT;
	}
    }	/* end count1 loop */

    BUS_GETCX((iocc_addr + CTEST3), &pio_rc);
    BUS_GETCX((iocc_addr + SSTAT0), &pio_rc);
    pio_rc &= 0x08;
    if (pio_rc != 0x08)
    {
	DEBUG_0 ("psc_run_diag: error checking Gross error in SSTAT0\n");
	TRACE_1 ("failed gross chk", 0);
	TRACE_1 ("out run_diag", EFAULT);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_RD_OTHR_ERR;
	BUS_PUTCX((iocc_addr + SCNTL0), 0x00);
	BUS_PUTCX((iocc_addr + SCNTL1), 0x00);
	BUS_PUTCX((iocc_addr + CTEST4), 0x00);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    if ((BUS_PUTCX((iocc_addr + CTEST4), 0x10)) != 0)
    {
	DEBUG_0 ("psc_run_diag: error enabling loopback mode\n");
	TRACE_1 ("CTEST4 error", 0);
	TRACE_1 ("out run_diag", EFAULT);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    if ((BUS_PUTCX((iocc_addr + SCNTL0), 0x0c)) != 0)
    {
	DEBUG_0 ("psc_run_diag: error enabling parity in SCNTL0\n");
	TRACE_1 ("failed scntl0", 0);
	TRACE_1 ("out run_diag", EFAULT);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    if ((BUS_PUTCX((iocc_addr + SCNTL1), 0x04)) != 0)
    {
	DEBUG_0 ("psc_run_diag: error forcing even parity in SCNTL1\n")
	TRACE_1 ("failed scntl1 par", 0);
	TRACE_1 ("out run_diag", EFAULT);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    if ((BUS_PUTCX((iocc_addr + CTEST7), 0x04)) != 0)
    {
	DEBUG_0 ("psc_run_diag: error forcing even parity in CTEST7\n");
	TRACE_1 ("failed ct7 par", 0);
	TRACE_1 ("out run_diag", EFAULT);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    if ((BUS_PUTCX((iocc_addr + SODL), 0x07)) != 0)
    {
	DEBUG_0 ("psc_run_diag: error writing data to SODL\n");
	TRACE_1 ("failed sodl data   ", 0);
	TRACE_1 ("out run_diag", EFAULT);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    if ((BUS_PUTCX((iocc_addr + SCNTL1), 0x44)) != 0)
    {
	DEBUG_0 ("psc_run_diag: error asserting data in SCNTL1\n");
	TRACE_1 ("scntl1 data assert  ", 0);
	TRACE_1 ("out run_diag", EFAULT);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    rc = psc_read_reg(SSTAT0, SSTAT0_SIZE);
    BUS_GETCX((iocc_addr + SSTAT0), &pio_rc);
    pio_rc &= 0x01;
    if (pio_rc != 0)
    {
	DEBUG_0 ("psc_run_diag: error checking parity error in SSTAT0\n")
	TRACE_1 ("par chk err    ", 0);
	TRACE_1 ("out run_diag", EFAULT);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_RD_OTHR_ERR;
	BUS_PUTCX((iocc_addr + SCNTL0), 0x00);
	BUS_PUTCX((iocc_addr + SCNTL1), 0x00);
	BUS_PUTCX((iocc_addr + CTEST4), 0x00);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    DEBUG_0 ("Leaving psc_run_diagnostics\n");
    BUSIO_DET(iocc_addr);
    return (0);
}  /* end psc_run_diagnostics */

/**************************************************************************/
/*                                                                        */
/* NAME:  psc_loop_test_diagnotics                                        */
/*                                                                        */
/* FUNCTION:  Runs a loop test of the chip SCSI buss.                     */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine runs a loop test of the chip scsi bus.  Signals   */
/*         and some register testing is done here.                        */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    scsi chip structure - scsi chip information.                        */
/*    diagnostic structure - diagnostic information.                      */
/*                                                                        */
/* INPUTS:                                                                */
/*    diag_ptr - diagnostic structure pointer                             */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  0 for good completion, ERRNO value otherwise*/
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/**************************************************************************/
int
psc_loop_test_diagnostics(
			  struct sc_card_diag * diag_ptr)
{
    int     rc;
    char    pio_rc, count;
    caddr_t iocc_addr;

    DEBUG_0 ("Entering psc_loop_test_diagnostics\n")
    TRACE_1 ("in test ", 0);
    iocc_addr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);
    rc = BUS_PUTCX((iocc_addr + DCNTL), 0x81);
    if (rc != 0)
    {
	DEBUG_0 ("error writing to chip reset in DCNTL\n")
	BUS_PUTCX((iocc_addr + DCNTL), 0x00);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    rc = BUS_PUTCX((iocc_addr + DCNTL), 0x80);
    if (rc != 0)
    {
	DEBUG_0 ("error clearing chip reset in DCNTL\n")
	BUS_PUTCX((iocc_addr + DCNTL), 0x00);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    TRACE_1 ("arb mode ", 0);
    BUS_PUTCX((iocc_addr + SCNTL0), 0xc0);
    /* Arbitrate for the bus */
    TRACE_1 ("lpback mode ", 0);
    rc = BUS_PUTCX((iocc_addr + CTEST4), 0x10);
    if (rc != 0)
    {
	DEBUG_0 ("error writing to loopback mode in CTEST4\n")
	BUS_PUTCX((iocc_addr + CTEST4), 0x00);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    TRACE_1 ("arb mode ", 0);
    rc = BUS_PUTCX((iocc_addr + SCNTL0), 0xc0);
    if (rc != 0)
    {
	DEBUG_0 ("error writing to arbitration mode in SCNTL0\n")
	BUS_PUTCX((iocc_addr + SCNTL0), 0x00);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    /* Set low level mode */
    rc = BUS_PUTCX((iocc_addr + DCNTL), 0x80);
    if (rc != 0)
    {
	DEBUG_0 ("error writing to low level mode in DCNTL\n")
	BUS_PUTCX((iocc_addr + DCNTL), 0x00);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    /* Set destination ID */
    rc = BUS_PUTCX((iocc_addr + SDID), 0x01);
    if (rc != 0)
    {
	DEBUG_0 ("error writing dest. id in SDID\n")
	BUS_PUTCX((iocc_addr + SDID), 0x00);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    /* Set card IDs */
    rc = BUS_PUTCX((iocc_addr + SCID), 0x81);
    if (rc != 0)
    {
	DEBUG_0 ("error writing chip id in SCID\n")
	BUS_PUTCX((iocc_addr + SCID), 0x00);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    TRACE_1 ("start seq ", 0);
    rc = BUS_PUTCX((iocc_addr + SCNTL0), 0xe0);
    if (rc != 0)
    {
	DEBUG_0 ("error writing to start sequence bit in SCNTL0\n")
	BUS_PUTCX((iocc_addr + SCNTL0), 0x00);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    /* read ISTAT to see if we are connect to the bus to */
    /* tell if we won arbitration                        */
    count = 5;
    pio_rc = 0;
    TRACE_1 ("arb check ", 0);
    while ((count > 0) && (pio_rc != 0x08))
    {
	BUS_GETCX((iocc_addr + ISTAT), &pio_rc);
	pio_rc &= 0x08;
        TRACE_1 ("ISTAT     ", pio_rc);
	count--;
    }
    if (pio_rc != 0x08)
    {
	DEBUG_0 ("Unsuccessful arbitration phase\n")
	BUS_PUTCX((iocc_addr + SOCL), 0x00);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_RD_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    TRACE_1 ("sel phase ", 0);
    /* Go to selection phase */
    BUS_GETCX((iocc_addr + SBCL), &pio_rc);
    if (pio_rc < 0)
    {
	DEBUG_0 ("error reading SBCL\n")
	BUS_PUTCX((iocc_addr + SOCL), 0x00);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_RD_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    pio_rc |= 0x20;
    TRACE_1 ("mid sel  ", rc);
    rc = BUS_PUTCX((iocc_addr + SOCL), pio_rc);
    if (rc != 0)
    {
	DEBUG_0 ("error writing to selection phase in SOCL\n")
	BUS_PUTCX((iocc_addr + SOCL), 0x00);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    count = 5;
    pio_rc = 0;
    while ((count > 0) && (pio_rc != 0x40))
    {
	BUS_GETCX((iocc_addr + SSTAT0), &pio_rc);
	pio_rc &= 0x40;
	count--;
    }
    TRACE_1 ("end sel  ", (int) pio_rc);
    if (pio_rc != 0x40)
    {
	DEBUG_0 ("Unsuccessful selection phase\n")
	BUS_PUTCX((iocc_addr + SOCL), 0x00);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_RD_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    /* Successful selection phase */
    /* Get to data out phase by asserting REQ */
    TRACE_1 ("strt data out ", 0);
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    pio_rc &= 0xF8;
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc | 0x80));
    for (count = 0; count < 16; count++)
    {
	/* Assert REQ to request target to receive data */
	BUS_GETCX((iocc_addr + SOCL), &pio_rc);
	BUS_PUTCX((iocc_addr + SOCL), (pio_rc | 0x80));
	/* change to target mode to receive data */
	rc = BUS_PUTCX((iocc_addr + SCNTL0), 0xC1);
	if (rc != 0)
	{
	    DEBUG_0 ("error changing to target in SCNTL0\n")
	    BUS_PUTCX((iocc_addr + SCNTL0), 0x00);
	    diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	    diag_ptr->diag_rc.diag_validity = 0x01;
	    diag_ptr->diag_rc.ahs_validity = 0x01;
	    diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	    BUSIO_DET(iocc_addr);
	    return EFAULT;
	}

	/* write data onto bus */
	BUS_PUTCX((iocc_addr + SODL), count);
	BUS_GETCX((iocc_addr + SCNTL1), &pio_rc);
	BUS_PUTCX((iocc_addr + SCNTL1), (pio_rc | 0x40));

	/* assert ACK to acknowledge data received */
	BUS_GETCX((iocc_addr + SOCL), &pio_rc);
	BUS_PUTCX((iocc_addr + SOCL), (pio_rc | 0x40));

	/* compare data received */
	BUS_GETCX((iocc_addr + SBDL), &pio_rc);
	if (pio_rc != count)
	{
	    DEBUG_0 ("error comparing data out of SBDL\n")
	    TRACE_1 ("cmp error  ", count);
	    BUS_PUTCX((iocc_addr + SOCL), 0x00);
	    diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	    diag_ptr->diag_rc.diag_validity = 0x01;
	    diag_ptr->diag_rc.ahs_validity = 0x01;
	    diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	    BUSIO_DET(iocc_addr);
	    return EFAULT;
	}
	/* deassert REQ */
	BUS_GETCX((iocc_addr + SOCL), &pio_rc);
	BUS_PUTCX((iocc_addr + SOCL), (pio_rc & 0x7F));

	/* deassert ACK */
	BUS_GETCX((iocc_addr + SOCL), &pio_rc);
	BUS_PUTCX((iocc_addr + SOCL), (pio_rc & 0xBF));
	BUS_PUTCX((iocc_addr + SODL), 0x00);
    }	/* end for loop */
    /* Write the bus phases in the SOCL reg to bring the chip */
    /* to the command completion phase                        */

    /* deassert ATN */
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc & 0xF7));

    /* change to target mode to receive data */
    rc = BUS_PUTCX((iocc_addr + SCNTL0), 0xC1);
    if (rc != 0)
    {
	DEBUG_0 ("error changing to target in SCNTL0\n")
	BUS_PUTCX((iocc_addr + SCNTL0), 0x00);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }

    /* Get to msg in phase (assert I/O,MSG & C/D) */
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc | 0x07));

    /* assert REQ */
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc | 0x80));

    /* indicate command completion */
    BUS_PUTCX((iocc_addr + SODL), 0x00);

    /* assert ACK */
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc | 0x40));

    /* deassert ATN */
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc & 0xF7));

    /* deassert ACK */
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc & 0xBF));

    /* deassert MSG & C/D (take out of msg in phase) */
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc & 0xF8));
    if ((BUS_PUTCX((iocc_addr + SCNTL1), 0x08)) != 0)
    {
	DEBUG_0 ("Error writing to SCNTL1\n");
	BUS_PUTCX((iocc_addr + SCNTL1), 0x00);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    delay(1 * HZ);
    BUS_PUTCX((iocc_addr + SCNTL1), 0x00);

    /* change to initiator mode to free bus */
    TRACE_1("initiator    ",0);
    rc = BUS_PUTCX((iocc_addr + SCNTL0), 0xC0);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc & 0x0));

    /* bring the chip to the data in phase */

    rc = BUS_PUTCX((iocc_addr + DCNTL), 0x81);
    if (rc != 0)
    {
	DEBUG_0 ("error writing to chip reset in DCNTL\n")
	BUS_PUTCX((iocc_addr + DCNTL), 0x00);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    rc = BUS_PUTCX((iocc_addr + DCNTL), 0x80);
    if (rc != 0)
    {
	DEBUG_0 ("error clearing chip reset in DCNTL\n")
	BUS_PUTCX((iocc_addr + DCNTL), 0x00);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    TRACE_1 ("arb mode ", 0);
    BUS_PUTCX((iocc_addr + SCNTL0), 0x00);
    /* Arbitrate for the bus */
    TRACE_1 ("lpback mode ", 0);
    rc = BUS_PUTCX((iocc_addr + CTEST4), 0x10);
    if (rc != 0)
    {
	DEBUG_0 ("error writing to loopback mode in CTEST4\n")
	        BUS_PUTCX((iocc_addr + CTEST4), 0x00);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    TRACE_1 ("arb mode ", 0);
    rc = BUS_PUTCX((iocc_addr + SCNTL0), 0xc1);
    if (rc != 0)
    {
	DEBUG_0 ("error writing to arbitration mode in SCNTL0\n")
	BUS_PUTCX((iocc_addr + SCNTL0), 0x00);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    /* Set low level mode */
    rc = BUS_PUTCX((iocc_addr + DCNTL), 0x80);
    if (rc != 0)
    {
	DEBUG_0 ("error writing to low level mode in DCNTL\n")
	BUS_PUTCX((iocc_addr + DCNTL), 0x00);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    /* Set destination ID */
    rc = BUS_PUTCX((iocc_addr + SDID), 0x01);
    if (rc != 0)
    {
	DEBUG_0 ("error writing dest. id in SDID\n")
	BUS_PUTCX((iocc_addr + SDID), 0x00);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    /* Set card IDs */
    rc = BUS_PUTCX((iocc_addr + SCID), 0x81);
    if (rc != 0)
    {
	DEBUG_0 ("error writing chip id in SCID\n")
	BUS_PUTCX((iocc_addr + SCID), 0x00);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    TRACE_1 ("start seq ", 0);
    rc = BUS_PUTCX((iocc_addr + SCNTL0), 0xe1);
    if (rc != 0)
    {
	DEBUG_0 ("error writing to start sequence bit in SCNTL0\n")
	BUS_PUTCX((iocc_addr + SCNTL0), 0x00);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    /* read SBCL sel active and bsy inactive indicating */
    /* we won arbitration                               */
    count = 5;
    pio_rc = 0;
    TRACE_1 ("arb check ", 0);
    while ((count > 0) && (pio_rc != 0x08))
    {
	BUS_GETCX((iocc_addr + ISTAT), &pio_rc);
	pio_rc &= 0x08;
        TRACE_1 ("ISTAT     ", pio_rc);
	count--;
    }
    if (pio_rc != 0x08)
    {
	DEBUG_0 ("Unsuccessful arbitration phase\n")
	BUS_PUTCX((iocc_addr + SOCL), 0x00);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_RD_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    TRACE_1 ("sel phase ", 0);
    /* Go to selection phase */
    BUS_GETCX((iocc_addr + SBCL), &pio_rc);
    if (pio_rc < 0)
    {
	DEBUG_0 ("error reading SBCL\n")
	BUS_PUTCX((iocc_addr + SOCL), 0x00);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_RD_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    pio_rc |= 0x20;
    TRACE_1 ("mid sel  ", (int) pio_rc);
    rc = BUS_PUTCX((iocc_addr + SOCL), pio_rc);
    if (rc != 0)
    {
	DEBUG_0 ("error writing to selection phase in SOCL\n")
	BUS_PUTCX((iocc_addr + SOCL), 0x00);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    count = 5;
    pio_rc = 0;
    while ((count > 0) && (pio_rc != 0x40))
    {
	BUS_GETCX((iocc_addr + SSTAT0), &pio_rc);
	pio_rc &= 0x40;
	count--;
    }
    TRACE_1 ("end sel  ", (int) pio_rc);
    if (pio_rc != 0x40)
    {
	DEBUG_0 ("Unsuccessful selection phase\n")
	BUS_PUTCX((iocc_addr + SOCL), 0x00);
	diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	diag_ptr->diag_rc.diag_validity = 0x01;
	diag_ptr->diag_rc.ahs_validity = 0x01;
	diag_ptr->diag_rc.ahs = PIO_RD_OTHR_ERR;
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }

    /* Put into initiator mode */
    BUS_PUTCX((iocc_addr + SCNTL0), 0xc0);

    /* write data onto bus */
    BUS_PUTCX((iocc_addr + SODL), 0x01);

    BUS_GETCX((iocc_addr + SCNTL1), &pio_rc);
    BUS_PUTCX((iocc_addr + SCNTL1), (pio_rc | 0x40));

    /* Put into target mode */
    BUS_PUTCX((iocc_addr + SCNTL0), 0xc1);
    /* assert ACK */

    /* Put into initiator mode */
    BUS_PUTCX((iocc_addr + SCNTL0), 0xc0);
    TRACE_1 ("strt data in ", 0);
    for (count = 0; count < 16; count++)
    {
	/* write data onto bus */
	BUS_PUTCX((iocc_addr + SODL), count);
	BUS_GETCX((iocc_addr + SCNTL1), &pio_rc);
	BUS_PUTCX((iocc_addr + SCNTL1), (pio_rc | 0x40));

	/* Put into initiator mode */
	BUS_PUTCX((iocc_addr + SCNTL0), 0xc0);

	/* Change to data in */
	BUS_PUTCX((iocc_addr + SOCL), 0x21);

	/* assert REQ */
	BUS_GETCX((iocc_addr + SOCL), &pio_rc);
	BUS_PUTCX((iocc_addr + SOCL), (pio_rc | 0x80));

	/* deassert REQ */
	BUS_GETCX((iocc_addr + SOCL), &pio_rc);
	BUS_PUTCX((iocc_addr + SOCL), (pio_rc & 0x7F));

	/* assert ACK */
	BUS_GETCX((iocc_addr + SOCL), &pio_rc);
	BUS_PUTCX((iocc_addr + SOCL), (pio_rc | 0x40));

	/* compare data received */
	BUS_GETCX((iocc_addr + SBDL), &pio_rc);
	if (pio_rc != count)
	{
	    DEBUG_0 ("error comparing data out of SBDL\n")
	    TRACE_1 ("cmp err2     ", count);
	    BUS_PUTCX((iocc_addr + SOCL), 0x00);
	    diag_ptr->diag_rc.diag_stat = SC_DIAG_UNRECOV_PIO;
	    diag_ptr->diag_rc.diag_validity = 0x01;
	    diag_ptr->diag_rc.ahs_validity = 0x01;
	    diag_ptr->diag_rc.ahs = PIO_WR_OTHR_ERR;
	    BUSIO_DET(iocc_addr);
	    return EFAULT;
	}

	/* deassert ACK */
	BUS_GETCX((iocc_addr + SOCL), &pio_rc);
	BUS_PUTCX((iocc_addr + SOCL), (pio_rc & 0xBF));

	BUS_PUTCX((iocc_addr + SODL), 0x00);
    }	/* end for loop */
    TRACE_1 ("end data in  ", 0);

    /* Write the bus phases in the SOCL reg to bring the chip */
    /* to the command completion phase                        */

    /* assert ATN */
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc | 0x08));

    /* Get to msg out phase (assert MSG & C/D) */
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc | 0x60));

    /* assert REQ */
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc | 0x80));

    /* indicate command completion */
    BUS_PUTCX((iocc_addr + SODL), 0x00);

    /* assert ACK */
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc | 0x40));

    /* deassert REQ */
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc & 0x7F));

    /* deassert ACK */
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc & 0xBF));

    /* deassert ATN */
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc & 0xF7));

    BUS_PUTCX((iocc_addr + SODL), 0x00);
    /* deassert MSG & C/D (take out of msg out phase) */
    BUS_GETCX((iocc_addr + SOCL), &pio_rc);
    BUS_PUTCX((iocc_addr + SOCL), (pio_rc & 0xF9));

    /* Go to bus free (deassert all signals) */
    BUS_PUTCX((iocc_addr + SOCL), 0x00);
    TRACE_1 ("out test", 0);
    DEBUG_0 ("Leaving psc_loop_test_diagnostics\n")
            BUSIO_DET(iocc_addr);
    return (PSC_NO_ERR);
}  /* end psc_loop_test_diagnostics */

/**************************************************************************/
/*                                                                        */
/* NAME:  psc_register_test                                               */
/*                                                                        */
/* FUNCTION:  Runs a test of the chip register logic circuits.            */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine runs test of chip register logic circuitry.  Data */
/*         patterns are written to each applicable register and read back */
/*         for compare purposes.                                          */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    scsi chip structure - scsi chip information.                        */
/*    diagnostic structure - diagnostic information.                      */
/*                                                                        */
/* INPUTS:                                                                */
/*    diag_ptr - diagnostic structure pointer                             */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  0 for good completion, ERRNO value otherwise*/
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/**************************************************************************/
int
psc_register_test(
		  struct sc_card_diag * diag_ptr)
{
    int     rc;
    char    pio_rc;
    caddr_t iocc_addr;

    DEBUG_0 ("Entering psc_register_test\n")
    TRACE_1 ("in reg_test       ", (int) diag_ptr);
    iocc_addr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);
    BUS_PUTCX((iocc_addr + SCNTL0), 0xCA);
    BUS_GETCX((iocc_addr + SCNTL0), &pio_rc);
    if (pio_rc != 0xCA)
    {
	DEBUG_0 ("write to SCNTL0 of 0xCA failed\n");
	TRACE_1 ("SCNTL0 failed", pio_rc);
	BUS_PUTCX((iocc_addr + SCNTL0), 0x00);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    BUS_PUTCX((iocc_addr + SCNTL0), 0x55);
    BUS_GETCX((iocc_addr + SCNTL0), &pio_rc);
    if (pio_rc != 0x55)
    {
	DEBUG_0 ("write to SCNTL0 of 0x55 failed\n");
	TRACE_1 ("E_scntl0 55    ", pio_rc);
	BUS_PUTCX((iocc_addr + SCNTL0), 0x00);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    BUS_PUTCX((iocc_addr + SCNTL0), 0x00);
    BUS_PUTCX((iocc_addr + SCNTL1), 0xA0);
    BUS_GETCX((iocc_addr + SCNTL1), &pio_rc);
    if (pio_rc != 0xA0)
    {
	DEBUG_0 ("write to SCNTL1 of 0xA0 failed\n");
	TRACE_1 ("E_scntl1 A0    ", pio_rc);
	BUS_PUTCX((iocc_addr + SCNTL1), 0x00);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    BUS_PUTCX((iocc_addr + SCNTL1), 0x04);
    BUS_GETCX((iocc_addr + SCNTL1), &pio_rc);
    if (pio_rc != 0x04)
    {
	DEBUG_0 ("write to SCNTL1 of 0x04 failed\n");
	TRACE_1 ("E_scntl1 04    ", pio_rc);
	BUS_PUTCX((iocc_addr + SCNTL1), 0x00);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    BUS_PUTCX((iocc_addr + SCNTL1), 0x00);
    BUS_PUTCX((iocc_addr + SDID), 0xAA);
    BUS_GETCX((iocc_addr + SDID), &pio_rc);
    if (pio_rc != 0xAA)
    {
	DEBUG_0 ("write to SDID of 0xAA failed\n");
	TRACE_1 ("E_sdid AA    ", pio_rc);
	BUS_PUTCX((iocc_addr + SDID), 0x00);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    BUS_PUTCX((iocc_addr + SDID), 0x55);
    BUS_GETCX((iocc_addr + SDID), &pio_rc);
    if (pio_rc != 0x55)
    {
	DEBUG_0 ("write to SDID of 0x55 failed\n")
	TRACE_1 ("E_sdid 55    ", pio_rc);
	BUS_PUTCX((iocc_addr + SDID), 0x00);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    BUS_PUTCX((iocc_addr + SDID), 0x00);
    BUS_PUTCX((iocc_addr + SIEN), 0xAA);
    BUS_GETCX((iocc_addr + SIEN), &pio_rc);
    if (pio_rc != 0xAA)
    {
	DEBUG_0 ("write to SIEN of 0xAA failed\n");
	TRACE_1 ("E_sien AA    ", pio_rc);
	BUS_PUTCX((iocc_addr + SIEN), 0x00);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    BUS_PUTCX((iocc_addr + SIEN), 0x55);
    BUS_GETCX((iocc_addr + SIEN), &pio_rc);
    if (pio_rc != 0x55)
    {
	DEBUG_0 ("write to SIEN of 0x55 failed\n");
	TRACE_1 ("E_sien 55    ", pio_rc);
	BUS_PUTCX((iocc_addr + SIEN), 0x00);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    BUS_PUTCX((iocc_addr + SIEN), 0x00);
    BUS_PUTCX((iocc_addr + SCID), 0xAA);
    BUS_GETCX((iocc_addr + SCID), &pio_rc);
    if (pio_rc != 0xAA)
    {
	DEBUG_0 ("write to SCID of 0xAA failed\n")
	TRACE_1 ("E_scid AA    ", pio_rc);
	BUS_PUTCX((iocc_addr + SCID), 0x00);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    BUS_PUTCX((iocc_addr + SCID), 0x55);
    BUS_GETCX((iocc_addr + SCID), &pio_rc);
    if (pio_rc != 0x55)
    {
	DEBUG_0 ("write to SCID of 0x55 failed\n")
	TRACE_1 ("E_scid 55    ", pio_rc);
	BUS_PUTCX((iocc_addr + SCID), 0x00);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    BUS_PUTCX((iocc_addr + SCID), 0x00);
    BUS_PUTCX((iocc_addr + SXFER), 0xAA);
    BUS_GETCX((iocc_addr + SXFER), &pio_rc);
    if (pio_rc != 0xAA)
    {
	DEBUG_0 ("write to SXFER of 0xAA failed\n");
	TRACE_1 ("E_sxfer AA    ", pio_rc);
	BUS_PUTCX((iocc_addr + SXFER), 0x00);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    BUS_PUTCX((iocc_addr + SXFER), 0x55);
    BUS_GETCX((iocc_addr + SXFER), &pio_rc);
    if (pio_rc != 0x55)
    {
	DEBUG_0 ("write to SXFER of 0x55 failed\n")
	TRACE_1 ("E_sxfer 55    ", pio_rc);
	BUS_PUTCX((iocc_addr + SXFER), 0x00);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    BUS_PUTCX((iocc_addr + SXFER), 0x00);
    BUS_PUTCX((iocc_addr + DCMD), 0xAA);
    BUS_GETCX((iocc_addr + DCMD), &pio_rc);
    if (pio_rc != 0xAA)
    {
	DEBUG_0 ("write to DCMD of 0xAA failed\n")
	TRACE_1 ("E_dcmd AA    ", pio_rc);
	BUS_PUTCX((iocc_addr + DCMD), 0x00);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    BUS_PUTCX((iocc_addr + DCMD), 0x55);
    BUS_GETCX((iocc_addr + DCMD), &pio_rc);
    if (pio_rc != 0x55)
    {
	DEBUG_0 ("write to DCMD of 0x55 failed\n");
	TRACE_1 ("E_dcmd 55    ", pio_rc);
	BUS_PUTCX((iocc_addr + DCMD), 0x00);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    BUS_PUTCX((iocc_addr + DCMD), 0x00);
    BUS_PUTLX((long *)(iocc_addr + DNAD), 0xAA55AA55);
    BUS_GETLX((long *)(iocc_addr + DNAD), (long *)&rc);
    if (rc != 0xAA55AA55)
    {
	DEBUG_0 ("write to DNAD of 0xAA55AA55 failed\n");
	TRACE_1 ("E_dnad AA55AA55   ", rc);
	BUS_PUTLX((long *)(iocc_addr + DNAD), 0x00000000);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    BUS_PUTLX((long *)(iocc_addr + DNAD), 0x55AA55AA);
    BUS_GETLX((long *)(iocc_addr + DNAD), (long *)&rc);
    if (rc != 0x55AA55AA)
    {
	DEBUG_0 ("write to DNAD of 0x55AA55AA failed\n");
	TRACE_1 ("E_dnad 55AA55AA   ", rc);
	BUS_PUTLX((long *)(iocc_addr + DNAD), 0x00000000);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    BUS_PUTLX((long *)(iocc_addr + DNAD), 0x00000000);
    BUS_PUTCX((iocc_addr + DMODE), 0x01);
    BUS_PUTLX((long *)(iocc_addr + DSP), 0xAA55AA55);
    BUS_GETLX((long *)(iocc_addr + DSP), (long *)&rc);
    if (rc != 0xAA55AA55)
    {
	DEBUG_0 ("write to DSP of 0xAA55AA55 failed\n");
	TRACE_1 ("E_dsp AA55AA55   ", rc);
	BUS_PUTLX((long *)(iocc_addr + DSP), 0x00000000);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    BUS_PUTLX((long *)(iocc_addr + DSP), 0x55AA55AA);
    BUS_GETLX((long *)(iocc_addr + DSP), (long *)&rc);
    if (rc != 0x55AA55AA)
    {
	DEBUG_0 ("write to DSP of 0x55AA55AA failed\n")
	TRACE_1 ("E_dsp 55AA55AA   ", rc);
	BUS_PUTLX((long *)(iocc_addr + DSP), 0x00000000);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    BUS_PUTLX((long *)(iocc_addr + DSP), 0x00000000);
    BUS_PUTLX((long *)(iocc_addr + DSPS), 0xAA55AA55);
    BUS_GETLX((long *)(iocc_addr + DSPS), (long *)&rc);
    if (rc != 0xAA55AA55)
    {
	DEBUG_0 ("write to DSPS of 0xAA55AA55 failed\n");
	TRACE_1 ("E_dsps AA55AA55   ", rc);
	BUS_PUTLX((long *)(iocc_addr + DSPS), 0x00000000);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    BUS_PUTLX((long *)(iocc_addr + DSPS), 0x55AA55AA);
    BUS_GETLX((long *)(iocc_addr + DSPS), (long *)&rc);
    if (rc != 0x55AA55AA)
    {
	DEBUG_0 ("write to DSPS of 0x55AA55AA failed\n")
	TRACE_1 ("E_dsps 55AA55AA   ", rc);
	BUS_PUTLX((long *)(iocc_addr + DSPS), 0x00000000);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    BUS_PUTLX((long *)(iocc_addr + DSPS), 0x00000000);
    BUS_PUTCX((iocc_addr + DMODE), 0xAA);
    BUS_GETCX((iocc_addr + DMODE), &pio_rc);
    if (pio_rc != 0xAA)
    {
	DEBUG_0 ("write to DMODE of 0xAA failed\n")
	TRACE_1 ("E_dmode AA    ", pio_rc);
	BUS_PUTCX((iocc_addr + DMODE), 0x00);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    BUS_PUTCX((iocc_addr + DMODE), 0x00);
    BUS_PUTCX((iocc_addr + DIEN), 0xAA);
    BUS_GETCX((iocc_addr + DIEN), &pio_rc);
    if (pio_rc != 0xAA)
    {
	DEBUG_0 ("write to DIEN of 0xAA failed\n")
	TRACE_1 ("E_dien AA    ", pio_rc);
	BUS_PUTCX((iocc_addr + DIEN), 0x00);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    BUS_PUTCX((iocc_addr + DIEN), 0x15);
    BUS_GETCX((iocc_addr + DIEN), &pio_rc);
    if (pio_rc != 0x15)
    {
	DEBUG_0 ("write to DIEN of 0x15 failed\n")
	TRACE_1 ("E_dien 15    ", pio_rc);
	BUS_PUTCX((iocc_addr + DIEN), 0x00);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    BUS_PUTCX((iocc_addr + DIEN), 0x00);
    BUS_PUTCX((iocc_addr + DWT), 0xAA);
    BUS_GETCX((iocc_addr + DWT), &pio_rc);
    if (pio_rc != 0xAA)
    {
	DEBUG_0 ("write to DWT of 0xAA failed\n")
	TRACE_1 ("E_dwt AA    ", pio_rc);
	BUS_PUTCX((iocc_addr + DWT), 0x00);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    BUS_PUTCX((iocc_addr + DWT), 0x55);
    BUS_GETCX((iocc_addr + DWT), &pio_rc);
    if (pio_rc != 0x55)
    {
	DEBUG_0 ("write to DWT of 0x55 failed\n")
	TRACE_1 ("E_dwt 55    ", pio_rc);
	BUS_PUTCX((iocc_addr + DWT), 0x00);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    BUS_PUTCX((iocc_addr + DWT), 0x00);
    BUS_PUTCX((iocc_addr + DCNTL), 0xA0);
    BUS_GETCX((iocc_addr + DCNTL), &pio_rc);
    if (pio_rc != 0xA0)
    {
	DEBUG_0 ("write to DCNTL of 0xA0 failed\n")
	TRACE_1 ("E_sdid A0    ", pio_rc);
	BUS_PUTCX((iocc_addr + DCNTL), 0x00);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    BUS_PUTCX((iocc_addr + DCNTL), 0x50);
    BUS_GETCX((iocc_addr + DCNTL), &pio_rc);
    if (pio_rc != 0x50)
    {
	DEBUG_0 ("write to DCNTL of 0x50 failed\n")
	TRACE_1 ("E_sdid 50    ", pio_rc);
	BUS_PUTCX((iocc_addr + DCNTL), 0x00);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    BUS_PUTCX((iocc_addr + DCNTL), 0x00);
    DEBUG_0 ("Leaving psc_register_test\n")
    TRACE_1 ("out reg test   ", 0);
    BUSIO_DET(iocc_addr);
    return (0);
}  /* end psc_register_test */

/**************************************************************************/
/*                                                                        */
/* NAME:  psc_pos_register_test                                           */
/*                                                                        */
/* FUNCTION:  Runs a test of the POS register logic circuits.             */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine runs a test of the POS registers by first writing */
/*         a data pattern and then reading it for compare.                */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    scsi chip structure - scsi chip information.                        */
/*    diagnostic structure - diagnostic information.                      */
/*                                                                        */
/* INPUTS:  none                                                          */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  0 for good completion, ERRNO value otherwise*/
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/**************************************************************************/
int
psc_pos_register_test()
{
    caddr_t iocc_addr;
    char    pio_rc, pos_save;

    DEBUG_0 ("Entering psc_pos_register_test\n")
    iocc_addr = IOCC_ATT(adp_str.ddi.bus_id, 0);
    BUS_GETCX((iocc_addr + (adp_str.ddi.slot << 16) + POS0), &pio_rc);
    if (pio_rc != 0xF4)
    {
	DEBUG_0 ("Error reading POS0\n");
	TRACE_1 ("err pos0", pio_rc);
	return EFAULT;
    }
    BUS_GETCX((iocc_addr + (adp_str.ddi.slot << 16) + POS1), &pio_rc);
    if (pio_rc != 0x8F)
    {
	DEBUG_0 ("Error reading POS1\n");
	TRACE_1 ("err pos1", pio_rc);
	return EFAULT;
    }
    BUS_GETCX((iocc_addr + (adp_str.ddi.slot << 16) + POS4), &pos_save);
    BUS_PUTCX((iocc_addr + (adp_str.ddi.slot << 16) + POS4), 0x0A);
    BUS_GETCX((iocc_addr + (adp_str.ddi.slot << 16) + POS4), &pio_rc);
    pio_rc &= 0x0A;
    if (pio_rc != 0x0A)
    {
	DEBUG_0 ("Error reading POS4\n");
	TRACE_1 ("err1pos4", pio_rc);
	return EFAULT;
    }
    BUS_PUTCX((iocc_addr + (adp_str.ddi.slot << 16) + POS4), 0x15);
    BUS_GETCX((iocc_addr + (adp_str.ddi.slot << 16) + POS4), &pio_rc);
    pio_rc &= 0x15;
    if (pio_rc != 0x15)
    {
	DEBUG_0 ("Error reading POS4\n");
	TRACE_1 ("err2pos4", pio_rc);
	return EFAULT;
    }
    BUS_PUTCX((iocc_addr + (adp_str.ddi.slot << 16) + POS4), pos_save);
    BUS_PUTCX((iocc_addr + (adp_str.ddi.slot << 16) + POS6), 0xAA);
    BUS_GETCX((iocc_addr + (adp_str.ddi.slot << 16) + POS6), &pio_rc);
    if (pio_rc != 0xAA)
    {
	DEBUG_0 ("Error reading POS6\n");
	TRACE_1 ("err1pos6", pio_rc);
	return EFAULT;
    }
    BUS_PUTCX((iocc_addr + (adp_str.ddi.slot << 16) + POS6), 0x55);
    BUS_GETCX((iocc_addr + (adp_str.ddi.slot << 16) + POS6), &pio_rc);
    if (pio_rc != 0x55)
    {
	DEBUG_0 ("Error reading POS6\n");
	TRACE_1 ("err2pos6", pio_rc);
	return EFAULT;
    }
    IOCC_DET(iocc_addr);
    DEBUG_0 ("Leaving psc_pos_register_test\n");
    return (0);
}  /* end psc_pos_register_test */

/**************************************************************************/
/*                                                                        */
/* NAME:  psc_diag_reset_scsi_bus                                         */
/*                                                                        */
/* FUNCTION:  Resets the SCSI bus lines.                                  */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine will toggle the SCSI bus reset line for 30 micro- */
/*         seconds to assert a reset on the SCSI bus. This routines exe-  */
/*         cutes to reset the bus during diagnostic execution.            */
/*                                                                        */
/* DATA STRUCTURES: None                                                  */
/*                                                                        */
/* INPUTS: None                                                           */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  0 for good completion, ERRNO value otherwise*/
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*                                                                        */
/**************************************************************************/
int
psc_diag_reset_scsi_bus()
{
    caddr_t iocc_addr;

    DEBUG_0 ("Entering psc_diag_reset_scsi_bus\n")
    iocc_addr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);
    if ((BUS_PUTCX((iocc_addr + SCNTL1), 0x08)) != 0)
    {
	DEBUG_0 ("Error writing to SCNTL1\n");
	BUS_PUTCX((iocc_addr + SCNTL1), 0x00);
	BUSIO_DET(iocc_addr);
	return EFAULT;
    }
    delay(1 * HZ);
    BUS_PUTCX((iocc_addr + SCNTL1), 0x00);
    BUSIO_DET(iocc_addr);
    DEBUG_0 ("Leaving psc_diag_reset_scsi_bus\n")
    return (PSC_NO_ERR);
}  /* end psc_diag_reset_scsi_bus */

/**************************************************************************/
/*                                                                        */
/* NAME:        psc_start_dev                                             */
/*                                                                        */
/* FUNCTION:  Allocates resources and starts a device                     */
/*                                                                        */
/*      This routine initializes the device queue to prepare for          */
/*      command processing.                                               */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can only be called by a process.                     */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - scsi chip unique data structure                     */
/*                                                                        */
/* INPUTS:                                                                */
/*      dev_index - index to device information                           */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      0 - for good completion, ERRNO value otherwise.                   */
/*                                                                        */
/* ERROR DESCRIPTION                                                      */
/*      EINVAL - device not opened                                        */
/*      EIO - unable to pin code                                          */
/*      ENOMEM - unable to xmalloc memory                                 */
/*      EACCES - adapter not opened in normal mode                        */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*      pincode     unpincode                                             */
/*      xmalloc     xmfree                                                */
/*      w_start                                                           */
/*                                                                        */
/**************************************************************************/
int
psc_start_dev(
	      int dev_index)
{

    int     i, old_level;
    struct dev_info *dev_ptr;
    int     scr_index, TCW_index;
    uint    dma_addr;

    DEBUG_0 ("Entering psc_start_dev routine.\n")

    if (adp_str.open_mode == DIAG_MODE)
    {  /* adapter opened in diagnostic mode */
	return (EACCES);
    }
    if (adp_str.device_queue_hash[dev_index] != NULL)
    {	/* device queue structure already allocated */
	return (EINVAL);
    }
    if (adp_str.ddi.card_scsi_id == SID(dev_index))
    {	/* Device SCSI ID matches adapter SCSI ID.  */
	return (EINVAL);
    }

    /* device queue structure doesn't exist yet */
    adp_str.device_queue_hash[dev_index] = xmalloc(
			    (uint) sizeof(struct dev_info), 4, pinned_heap);
    if (adp_str.device_queue_hash[dev_index] == NULL)
    {	/* malloc failed */
	return (ENOMEM);
    }
    dev_ptr = adp_str.device_queue_hash[dev_index];

    i = pincode(psc_start_dev);
    if (i != 0)
    {
        adp_str.device_queue_hash[dev_index] = NULL;
        (void) xmfree(dev_ptr, pinned_heap);
        return (EIO);
    }

    bzero(dev_ptr, (int) sizeof(struct dev_info));

    /* Search through the device queue area for a free script */
    /* instance.                                              */
    if (adp_str.SCRIPTS[0].in_use == SCR_UNUSED)
    {	/* A scripts sequence has been found */
	scr_index = 0;	/* used to address scripts */
    }
    else
	/* If no free script was found, then it's necessary  */
	/* to malloc another page and create additional      */
	/* script instances.                                 */
    {
	for (scr_index = 1; scr_index < MAX_SCRIPTS; scr_index++)
	{
	    if (adp_str.SCRIPTS[scr_index].script_ptr == NULL)
		break;
	}

	/* malloc an additional 4K work area for the SCRIPTS */
	adp_str.SCRIPTS[scr_index].script_ptr =
	    (ulong *) xmalloc((uint) PAGESIZE, (uint) PGSHIFT, kernel_heap);
	if (adp_str.SCRIPTS[scr_index].script_ptr == NULL)
	{	/* Unable to malloc script area. */
	    adp_str.device_queue_hash[dev_index] = NULL;
	    (void) xmfree(dev_ptr, pinned_heap);
            (void) unpincode(psc_start_dev);
	    return (ENOMEM);
	}
        if (ltpin(adp_str.SCRIPTS[scr_index].script_ptr, PAGESIZE))
	{	/* Unable to pin script area. */
	    (void) xmfree(adp_str.SCRIPTS[scr_index].script_ptr, kernel_heap);
	    adp_str.device_queue_hash[dev_index] = NULL;
	    (void) xmfree(dev_ptr, pinned_heap);
            (void) unpincode(psc_start_dev);
	    return (EIO);
	}

	/* find a tcw from the 4K xfer area to use */
	old_level = i_disable(adp_str.ddi.int_prior);
	for (i = adp_str.begin_4K_TCW; i < NUM_4K_TCWS; i++)
	{
	    if (adp_str.TCW_alloc_table[i] == TCW_UNUSED)
	    {
		adp_str.TCW_alloc_table[i] = TCW_RESERVED;
		TCW_index = i;
		break;
	    }
	}
	for (i = 0; i < NUM_4K_TCWS; i++)
	{
	    if (adp_str.TCW_alloc_table[i] != TCW_RESERVED)
	    {
		adp_str.begin_4K_TCW = i;
		if (adp_str.begin_4K_TCW > adp_str.next_4K_req)
		    adp_str.next_4K_req = adp_str.begin_4K_TCW;
		break;
	    }
	}
	i_enable(old_level);
	/* set up the area for scripts and copy. */
	/* set up index for SCRIPTS in the TCW static table */
	dma_addr = DMA_ADDR(adp_str.ddi.tcw_start_addr, TCW_index);
	for (i = 0; i < INSTRUCTIONS * 2; i++)
	{
	    *(adp_str.SCRIPTS[scr_index].script_ptr + i) =
		PSC_SCRIPT[i];
	}
	adp_str.SCRIPTS[scr_index].dma_ptr = (ulong *) dma_addr;
	DEBUG_1 ("psc_open: SCRIPTS@=%x\n", adp_str.SCRIPTS)
	        adp_str.SCRIPTS[scr_index].TCW_index = TCW_index;
	d_master(adp_str.channel_id, DMA_NOHIDE,
		 (char *) adp_str.SCRIPTS[scr_index].script_ptr,
		 (size_t) PAGESIZE,
		 &adp_str.xmem_SCR, (char *) dma_addr);
	adp_str.num_scripts_created++;
    }
    (void) unpincode (psc_start_dev);

    dev_ptr->script_dma_addr = (uint) adp_str.SCRIPTS[scr_index].dma_ptr;
    dev_ptr->cmd_script_ptr = scr_index;
    adp_str.SCRIPTS[scr_index].in_use = SCR_IN_USE;

    /* Call the setup routine for this script entry */
    psc_script_init((uint *) (adp_str.SCRIPTS[0].script_ptr),
		(uint *) (adp_str.SCRIPTS[scr_index].script_ptr), dev_index,
		(uint) adp_str.SCRIPTS[0].dma_ptr, dev_ptr->script_dma_addr);

    /* Initialize device queue flags.             */
    dev_ptr->DEVICE_ACTIVE_fwd = NULL;
    dev_ptr->DEVICE_ACTIVE_bkwd = NULL;
    dev_ptr->DEVICE_WAITING_fwd = NULL;
    dev_ptr->DEVICE_WAITING_FOR_RESOURCES_fwd = NULL;
    dev_ptr->ABORT_BDR_fwd = NULL;
    dev_ptr->ABORT_BDR_bkwd = NULL;
    dev_ptr->head_pend = NULL;
    dev_ptr->tail_pend = NULL;
    dev_ptr->active = NULL;
    dev_ptr->scsi_id = SID(dev_index);
    dev_ptr->lun_id = LUN(dev_index);
    dev_ptr->negotiate_flag = TRUE;
    dev_ptr->async_device = FALSE;
    dev_ptr->restart_in_prog = FALSE;
    dev_ptr->ioctl_wakeup = FALSE;
    dev_ptr->ioctl_event = EVENT_NULL;
    dev_ptr->stop_event = EVENT_NULL;
    dev_ptr->agreed_xfer = DEFAULT_MIN_PHASE;
    dev_ptr->agreed_req_ack = DEFAULT_BYTE_BUF;
    dev_ptr->flags = RETRY_ERROR;
    dev_ptr->disconnect_flag = 0;

    /* init watchdog timer struct                 */
    dev_ptr->dev_watchdog.dog.next = NULL;
    dev_ptr->dev_watchdog.dog.prev = NULL;
    dev_ptr->dev_watchdog.dog.func = psc_command_watchdog;
    dev_ptr->dev_watchdog.dog.count = 0;
    dev_ptr->dev_watchdog.dog.restart = 0;
    dev_ptr->dev_watchdog.timer_id = PSC_COMMAND_TMR;
    w_init(&(dev_ptr->dev_watchdog.dog));

    /* init command state flags                    */
    dev_ptr->queue_state = ACTIVE;
    dev_ptr->opened = TRUE;
    /* init the max_disconnect to 0 */
    dev_ptr->max_disconnect = 0;

    DEBUG_1 ("Device pointer address = %x\n", dev_ptr)
    DEBUG_0 ("Leaving psc_start_dev routine.\n")
    return (0);
}

/**************************************************************************/
/*                                                                        */
/* NAME:  psc_stop_dev                                                    */
/*                                                                        */
/* FUNCTION:  Stops a device and deallocates resources.                   */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine is called to stop a device.  Any further command  */
/*         processing for the device will be halted from this point on.   */
/*                                                                        */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*      adapter_def - scsi chip unique data structure                     */
/*                                                                        */
/* INPUTS:                                                                */
/*      dev_index - index to device information                           */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*      0 - for good completion, ERRNO value otherwise.                   */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*    EINVAL - device not opened.                                         */
/*    EACCES - Adapter not opened in normal mode.                         */
/*    EIO - unable to pin code                                            */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED: i_enable, i_disable, e_sleep, w_clear,     */
/*    pincode, unpincode                                                  */
/*                                                                        */
/**************************************************************************/
int
psc_stop_dev(
	     int dev_index)
{
    int     i, old_level, TCW_index;
    struct dev_info *dev_ptr;

    DEBUG_0 ("Entering psc_stop_dev routine.\n")

    if (adp_str.open_mode == DIAG_MODE)
    {	/* adapter opened in diagnostic mode */
	return (EACCES);
    }
    if (adp_str.device_queue_hash[dev_index] == NULL)
    {	/* device queue structure already allocated */
	return (EINVAL);
    }
    /* Obtain device structure pointer and disable */
    /* interrupts for processing.                 */

    i = pincode(psc_stop_dev);
    if (i != 0)
    {
        return (EIO);
    }

    dev_ptr = adp_str.device_queue_hash[dev_index];
    old_level = i_disable(adp_str.ddi.int_prior);
    dev_ptr->queue_state = STOPPING;

    /* Search both the active and pending queue of */
    /* this device structure to determine if there */
    /* are any commands outstanding.              */
    while ((dev_ptr->head_pend != NULL) || (dev_ptr->active != NULL))
    {
	e_sleep(&dev_ptr->stop_event, EVENT_SHORT);
    }
    i_enable(old_level);	/* let interrupts in */

    /* Free the device's script entry for other   */
    /* use.                                       */
    /* script 0 is freed when the device is closed */
    if (dev_ptr->cmd_script_ptr != 0)
    {
	old_level = i_disable(adp_str.ddi.int_prior);
	TCW_index = adp_str.SCRIPTS[dev_ptr->cmd_script_ptr].TCW_index;
	adp_str.TCW_alloc_table[TCW_index] = TCW_UNUSED;
	for (i = 0; i < NUM_4K_TCWS; i++)
	{
	    if (adp_str.TCW_alloc_table[i] != TCW_RESERVED)
	    {
		adp_str.begin_4K_TCW = i;
		if (adp_str.begin_4K_TCW > adp_str.next_4K_req)
		    adp_str.next_4K_req = adp_str.begin_4K_TCW;
		break;
	    }
	}
	i_enable(old_level);	/* let interrupts in */
	adp_str.SCRIPTS[dev_ptr->cmd_script_ptr].in_use = SCR_UNUSED;
	adp_str.SCRIPTS[dev_ptr->cmd_script_ptr].dma_ptr = 0;
	adp_str.SCRIPTS[dev_ptr->cmd_script_ptr].TCW_index = 0;
	(void) ltunpin(adp_str.SCRIPTS[dev_ptr->cmd_script_ptr].script_ptr,
		      PAGESIZE);
	(void) xmfree(adp_str.SCRIPTS[dev_ptr->cmd_script_ptr].script_ptr,
		      kernel_heap);
	adp_str.SCRIPTS[dev_ptr->cmd_script_ptr].script_ptr = NULL;
	adp_str.num_scripts_created--;
    }
    else
    {
	adp_str.SCRIPTS[dev_ptr->cmd_script_ptr].in_use = SCR_UNUSED;
    }
    /* Free the device's watchdog timer entry.   */
    w_clear(&(dev_ptr->dev_watchdog.dog));
    /* Free the device information table and clear */
    /* the hash table entry.                      */
    adp_str.device_queue_hash[dev_index] = NULL;
    (void) xmfree(dev_ptr, pinned_heap);
    (void) unpincode(psc_stop_dev);
    DEBUG_0 ("Leaving psc_stop_dev routine.\n")
    return (0);
}

/**************************************************************************/
/*                                                                        */
/* NAME:    psc_issue_abort                                               */
/*                                                                        */
/* FUNCTION:  Issues a SCSI abort to a device.                            */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine causes an abort command to be issued to a SCSI    */
/*         device.  Note that this action will also halt the device queue.*/
/*         The calling process is responsible for NOT calling this rou-   */
/*         tine if the SCIOSTART failed.  Such a failure would indicate   */
/*         that another process has this device open and interference     */
/*         could cause improper error reporting.                          */
/*                                                                        */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    scsi chip structure - scsi chip information.                        */
/*                                                                        */
/* INPUTS:                                                                */
/*    dev_index - index to device information                             */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*    0 - for good completion, ERRNO value otherwise                      */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*    EINVAL    - device not opened.                                      */
/*    EACCES    - Adapter not opened in normal mode.                      */
/*    ETIMEDOUT - Command timed out.                                      */
/*    ENOCONNECT- SCSI bus fault.                                         */
/*    EIO       - Adapter error or unable to pin code.                    */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED: i_disable, i_enable, e_sleep, pincode,     */
/*    unpincode                                                           */
/*                                                                        */
/**************************************************************************/
int
psc_issue_abort(
		int dev_index)
{
    struct dev_info *dev_ptr;
    struct dev_info *next_ptr;
    struct sc_buf *bp;
    int     old_pri1;
    int     i, ret_code;
    uchar   istat_val;
    caddr_t iocc_addr;
    register int rc;

    ret_code = 0;
    DEBUG_0 ("Entering psc_issue_abort routine\n")
    if (adp_str.open_mode == DIAG_MODE)
    {
	DEBUG_0 ("Leaving psc_issue_abort routine with EACCES.\n")
	return (EACCES);
    }

    /* Obtain device structure pointer and disable */
    /* interrupts for processing.                 */
    i = pincode(psc_issue_abort);
    if (i != 0)
    {
        return (EIO);
    }
    dev_ptr = adp_str.device_queue_hash[dev_index];
    old_pri1 = i_disable(adp_str.ddi.int_prior);

    /* if Q state is set to STOPPING or HALTED return EIO */
    if ((dev_ptr = adp_str.device_queue_hash[dev_index]) == NULL)
    {
	DEBUG_0 ("Leaving psc_issue_abort routine with EINVAL.\n")
	i_enable(old_pri1);     /* re-enable */
        (void) unpincode(psc_issue_abort);
	return (EINVAL);
    }

    if ((dev_ptr->queue_state & STOPPING) ||
	(dev_ptr->queue_state & HALTED))
    {
	i_enable(old_pri1);	/* re-enable */
        (void) unpincode(psc_issue_abort);
	return (EIO);
    }

    if ((dev_ptr->cmd_activity_state == ABORT_IN_PROGRESS) ||
	(dev_ptr->cmd_activity_state == BDR_IN_PROGRESS) ||
	(dev_ptr->ABORT_BDR_bkwd != NULL) ||
	(adp_str.ABORT_BDR_head == dev_ptr))

    {
	i_enable(old_pri1);	/* re-enable */
        (void) unpincode(psc_issue_abort);
	return (EIO);
    }

    /************************************************************/
    /*           DEVICE_WAITING_FOR_RESOURCES QUEUE             */
    /************************************************************/

    if ((dev_ptr->DEVICE_WAITING_FOR_RESOURCES_fwd != NULL) ||
	(adp_str.DEVICE_WAITING_FOR_RESOURCES_tail == dev_ptr))
    {
	dev_ptr->queue_state = HALTED;
	if (adp_str.DEVICE_WAITING_FOR_RESOURCES_head == dev_ptr)
	{
	    adp_str.DEVICE_WAITING_FOR_RESOURCES_head =
		    dev_ptr->DEVICE_WAITING_FOR_RESOURCES_fwd;
	    if (adp_str.DEVICE_WAITING_FOR_RESOURCES_tail == dev_ptr)
		adp_str.DEVICE_WAITING_FOR_RESOURCES_tail = NULL;
	}
	else
	{
	    next_ptr = adp_str.DEVICE_WAITING_FOR_RESOURCES_head;
	    while (next_ptr != NULL)
	    {
		if (next_ptr->DEVICE_WAITING_FOR_RESOURCES_fwd == dev_ptr)
		{
		    next_ptr->DEVICE_WAITING_FOR_RESOURCES_fwd =
			dev_ptr->DEVICE_WAITING_FOR_RESOURCES_fwd;
		    if (adp_str.DEVICE_WAITING_FOR_RESOURCES_tail ==
			dev_ptr)
			adp_str.DEVICE_WAITING_FOR_RESOURCES_tail =
			   next_ptr;
		    break;
		}
		next_ptr = next_ptr->DEVICE_WAITING_FOR_RESOURCES_fwd;
	    }
	}
	/* setup status in sc_buf to indicate error for each  */
	/* pending command                                    */
	if (dev_ptr->head_pend != NULL)
	{
	    psc_fail_cmd(dev_ptr, 1);
	    dev_ptr->head_pend = NULL;
	}

	i_enable(old_pri1);	/* re-enable */
        (void) unpincode(psc_issue_abort);
	return (PSC_NO_ERR);	/* no error */
    }

    /************************************************************/
    /*              DEVICE_WAITING QUEUE                        */
    /************************************************************/

    if ((dev_ptr->DEVICE_WAITING_fwd != NULL) ||
	(adp_str.DEVICE_WAITING_tail == dev_ptr))
    {
	dev_ptr->queue_state = HALTED;
	if (adp_str.DEVICE_WAITING_head == dev_ptr)
	{
	    adp_str.DEVICE_WAITING_head =
		    dev_ptr->DEVICE_WAITING_fwd;
	    if (adp_str.DEVICE_WAITING_tail == dev_ptr)
		adp_str.DEVICE_WAITING_tail = NULL;
	}
	else
	{
	    next_ptr = adp_str.DEVICE_WAITING_head;
	    while (next_ptr != NULL)
	    {
		if (next_ptr->DEVICE_WAITING_fwd == dev_ptr)
		{
		    next_ptr->DEVICE_WAITING_fwd = dev_ptr->DEVICE_WAITING_fwd;
		    if (adp_str.DEVICE_WAITING_tail == dev_ptr)
			adp_str.DEVICE_WAITING_tail = next_ptr;
		    break;
		}
		next_ptr = next_ptr->DEVICE_WAITING_fwd;
	    }
	}
	/* release resources for command */
	(void) psc_free_resources(dev_ptr, FALSE);
	/* setup status in sc_buf to indicate error on the  */
	/* currently executing command                      */
	bp = dev_ptr->active;
	bp->status_validity = 0;
	bp->bufstruct.b_resid = bp->bufstruct.b_bcount;

	/* setup status in sc_buf to indicate error for each */
	/* pending command                                   */
	/* iodone active and pending cmds (ENXIO) */
	psc_fail_cmd(dev_ptr, 2);
	i_enable(old_pri1);	/* re-enable */
        (void) unpincode(psc_issue_abort);
	return (PSC_NO_ERR);
    }

    /************************************************************/
    /*              DEVICE_ACTIVE QUEUE                         */
    /************************************************************/

    if ((dev_ptr->DEVICE_ACTIVE_bkwd != NULL) ||
	(adp_str.DEVICE_ACTIVE_head == dev_ptr))
    {
	dev_ptr->flags |= SCSI_ABORT;
	dev_ptr->ioctl_wakeup = TRUE;
	psc_enq_abort_bdr(dev_ptr);

	/* if dev is not connected                               */
	/* to the scsi bus, then SUSPEND processing by chip      */
	/* (use a timer to keep an eye on things)                */

	/* see if chip is waiting for reselection */
	iocc_addr = BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);
	psc_patch_iowait_int_on();
	for (i = 0; i < PIO_RETRY_COUNT; i++)
	{
            rc = BUS_GETCX(iocc_addr + ISTAT, (char *) &istat_val);
            if ((rc == 0) && !(istat_val & CONN_INT_TEST))
              /* Successful register read, & chip not connected */
            {
                rc = BUS_PUTCX(iocc_addr + ISTAT, 0x80);
                if (rc == 0)
                {
                    /* Start a timer for the chip suspend */
                    w_start(&(adp_str.adap_watchdog.dog));
                    break;
                }
            }
            if (rc == 0)
            {   /* no pio errors to retry, so */
                break;
            }
        }
        BUSIO_DET(iocc_addr);
	dev_ptr->ioctl_wakeup = TRUE;
	while (dev_ptr->ioctl_wakeup)
	    e_sleep(&dev_ptr->ioctl_event, EVENT_SHORT);
	ret_code = dev_ptr->ioctl_errno;

    }
    DEBUG_1 ("Leaving psc_issue_abort routine. rc=0x%x\n", ret_code);
    i_enable(old_pri1);	/* re-enable */
    (void) unpincode(psc_issue_abort);
    return (ret_code);	/* no error */
}  /* psc_issue_abort  */

/**************************************************************************/
/*                                                                        */
/* NAME:    psc_issue_BDR                                                 */
/*                                                                        */
/* FUNCTION:  Issues a bus device reset to a scsi device.                 */
/*                                                                        */
/*      This performs actions necessary to send a SCSI bus                */
/*      device reset mesg to the scsi controller.                         */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*                                                                        */
/* NOTES:  This routine causes an reset command to be issued to a SCSI    */
/*         device.  Note that this action will also halt the device queue.*/
/*         The calling process is responsible for NOT calling this rou-   */
/*         tine if the SCIOSTART failed.  Such a failure would indicate   */
/*         that another process has this device open and interference     */
/*         could cause improper error reporting.                          */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    adapter_def - adapter unique data structure                         */
/*                                                                        */
/* INPUTS:                                                                */
/*    dev_index - index to device information                             */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:                                              */
/*    0 - for good completion, ERRNO value otherwise                      */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*    EINVAL    - device not opened.                                      */
/*    EACCES    - Adapter not opened in normal mode.                      */
/*    ETIMEDOUT - Command timed out.                                      */
/*    ENOCONNECT- No connection.                                          */
/*    EIO       - Adapter error or unable to pincode.                     */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*  i_disable     i_enable    e_sleep    w_start    pincode   unpincode   */
/*                                                                        */
/**************************************************************************/
int
psc_issue_BDR(
	      int dev_index)
{
    struct dev_info *dev_ptr;
    struct dev_info *next_ptr;
    int     old_pri1;
    int     i, ret_code;
    uchar   istat_val;
    caddr_t iocc_addr;
    register int rc;


    DEBUG_0 ("Entering psc_issue_BDR routine.\n")
    if (adp_str.open_mode == DIAG_MODE)
    {
	DEBUG_0 ("Leaving psc_issue_BDR routine.\n")
	return (EACCES);
    }

    /* Obtain device structure pointer and disable */
    /* interrupts for processing.                 */
    i = pincode(psc_issue_BDR);
    if (i != 0)
    {
        return (EIO);
    }
    dev_ptr = adp_str.device_queue_hash[dev_index];
    old_pri1 = i_disable(adp_str.ddi.int_prior);

    if ((dev_ptr = adp_str.device_queue_hash[dev_index]) == NULL)
    {
	i_enable(old_pri1);	/* re-enable */
        (void) unpincode(psc_issue_BDR);
	return (EINVAL);
    }

    /* if Q state is set to STOPPING or HALTED return EIO */
    if ((dev_ptr->queue_state & STOPPING) ||
	(dev_ptr->queue_state & HALTED))
    {
	i_enable(old_pri1);	/* re-enable */
        (void) unpincode(psc_issue_BDR);
	return (EIO);
    }

    /************************************************************/
    /*                  ABORT_BDR QUEUE                         */
    /************************************************************/

    if ((dev_ptr->cmd_activity_state == ABORT_IN_PROGRESS) ||
	(dev_ptr->cmd_activity_state == BDR_IN_PROGRESS) ||
	(dev_ptr->ABORT_BDR_bkwd != NULL) ||
	(adp_str.ABORT_BDR_head == dev_ptr))
    {
	i_enable(old_pri1);
        (void) unpincode(psc_issue_BDR);
	return (EIO);
    }

    /************************************************************/
    /*            DEVICE_WAITING_FOR_RESOURCES QUEUE            */
    /************************************************************/
    else if ((dev_ptr->DEVICE_WAITING_FOR_RESOURCES_fwd != NULL) ||
	    (adp_str.DEVICE_WAITING_FOR_RESOURCES_tail == dev_ptr))
    {
	if (adp_str.DEVICE_WAITING_FOR_RESOURCES_head == dev_ptr)
	{
	    adp_str.DEVICE_WAITING_FOR_RESOURCES_head =
		    dev_ptr->DEVICE_WAITING_FOR_RESOURCES_fwd;
	    if (adp_str.DEVICE_WAITING_FOR_RESOURCES_tail == dev_ptr)
		adp_str.DEVICE_WAITING_FOR_RESOURCES_tail = NULL;
	}
	else
	{
	    next_ptr = adp_str.DEVICE_WAITING_FOR_RESOURCES_head;
	    while (next_ptr != NULL)
	    {
		if (next_ptr->DEVICE_WAITING_FOR_RESOURCES_fwd == dev_ptr)
		{
		    next_ptr->DEVICE_WAITING_FOR_RESOURCES_fwd =
			dev_ptr->DEVICE_WAITING_FOR_RESOURCES_fwd;
		    if (adp_str.DEVICE_WAITING_FOR_RESOURCES_tail == dev_ptr)
			adp_str.DEVICE_WAITING_FOR_RESOURCES_tail =
			   next_ptr;
		    break;
		}
		next_ptr = next_ptr->DEVICE_WAITING_FOR_RESOURCES_fwd;
	    }
	}
    }
    /************************************************************/
    /*                   DEVICE_WAITING QUEUE                   */
    /************************************************************/
    else if ((dev_ptr->DEVICE_WAITING_fwd != NULL) ||
	    (adp_str.DEVICE_WAITING_tail == dev_ptr))
    {
	if (adp_str.DEVICE_WAITING_head == dev_ptr)
	{
	    adp_str.DEVICE_WAITING_head =
		    dev_ptr->DEVICE_WAITING_fwd;
	    if (adp_str.DEVICE_WAITING_tail == dev_ptr)
		adp_str.DEVICE_WAITING_tail = NULL;
	}
	else
	{
	    next_ptr = adp_str.DEVICE_WAITING_head;
	    while (next_ptr != NULL)
	    {
		if (next_ptr->DEVICE_WAITING_fwd == dev_ptr)
		{
		    next_ptr->DEVICE_WAITING_fwd = dev_ptr->DEVICE_WAITING_fwd;
		    if (adp_str.DEVICE_WAITING_tail == dev_ptr)
			adp_str.DEVICE_WAITING_tail = next_ptr;
		    break;
		}
		next_ptr = next_ptr->DEVICE_WAITING_fwd;
	    }
	}
	/* release resources for command */
	(void) psc_free_resources(dev_ptr, FALSE);
	if (dev_ptr->head_pend == NULL)
	{       /* queue is empty  */
	    dev_ptr->head_pend = dev_ptr->active;
	    dev_ptr->tail_pend = dev_ptr->active;
	}
	else
	{       /* pending queue not empty */
	    /* point first cmd's av_forw at the new request */
	    dev_ptr->active->bufstruct.av_forw =
					 (struct buf *) dev_ptr->head_pend;
	    dev_ptr->head_pend = dev_ptr->active;
	}
	dev_ptr->active = NULL;
    }
    dev_ptr->flags |= SCSI_BDR;
    dev_ptr->ioctl_wakeup = TRUE;
    if (adp_str.DEVICE_ACTIVE_head == NULL)
    {
	/* update the WAIT script */
	psc_reset_iowait_jump((uint *) adp_str.SCRIPTS[0].script_ptr, 
			      dev_index);
	/* start timer */
	dev_ptr->dev_watchdog.dog.restart = LONGWAIT;
	w_start(&dev_ptr->dev_watchdog.dog);
	/* issue the cmd */
	dev_ptr->cmd_activity_state = BDR_IN_PROGRESS;
	psc_enq_active(dev_ptr);
	psc_issue_bdr_script((uint *) adp_str.SCRIPTS[dev_ptr->cmd_script_ptr].
			   script_ptr, (uint *) dev_ptr->script_dma_addr, 
			   dev_index);
    }
    else
    {
	psc_enq_abort_bdr(dev_ptr);

	/* see if chip is waiting for reselection */
	iocc_addr =
	    BUSIO_ATT((adp_str.ddi.bus_id | 0x000c0060), 0);
	psc_patch_iowait_int_on();
	for (i = 0; i < PIO_RETRY_COUNT; i++)
	{
            rc = BUS_GETCX(iocc_addr + ISTAT, (char *) &istat_val);
            if ((rc == 0) && !(istat_val & CONN_INT_TEST))
              /* Successful register read, & chip not connected */
            {   /* The register read is successful */
                rc = BUS_PUTCX(iocc_addr + ISTAT, 0x80);
                if (rc == 0)
                {
                    /* Start a timer for the chip suspend */
                    w_start(&(adp_str.adap_watchdog.dog));
                    break;
                }
            }
            if (rc == 0)
            {   /* no pio to retry, so */
                break;
            }
        }
        BUSIO_DET(iocc_addr);
    }
    dev_ptr->ioctl_wakeup = TRUE;
    while (dev_ptr->ioctl_wakeup)
	e_sleep(&dev_ptr->ioctl_event, EVENT_SHORT);
    ret_code = dev_ptr->ioctl_errno;
    i_enable(old_pri1);	/* re-enable */
    (void) unpincode(psc_issue_BDR);
    DEBUG_1 ("Leaving psc_issue_BDR routine. rc=0x%x\n", ret_code);
    return (ret_code);
}  /* psc_issue_BDR */
