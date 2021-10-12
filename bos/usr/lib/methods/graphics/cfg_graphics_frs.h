/* @(#)68	1.8  src/bos/usr/lib/methods/graphics/cfg_graphics_frs.h, dispcfg, bos411, 9428A410j 7/5/94 11:28:16 */

/*
 *   COMPONENT_NAME: SYSXDISPCCM
 *
 *   FUNCTIONS: 
 *		
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/******************************************************************************
 ---------------------------------------------------------------------------

 PURPOSE: Defines the standard data structures used by the cfg_graphics
	  config methods and by cfg_graphics_frs_tools

 
 INPUTS:  n/a

 OUTPUTS: n/a

 DATA:    A set of typedefs:

 *************************************************************************/


#ifndef	_H_CFG_GRAPHICS_FRS
#define _H_CFG_GRAPHICS_FRS

/*======================================================================
|
| Define the constants used by various programs associated with config
|
|=======================================================================*/

enum
{
	/*------------------------------------------
	| Symbols related to Feature ROM Scan Config
	|
	|------------------------------------------*/

	CFG_GRAPH_BUID_MASK			= (int)	0x0ff00000,

	CFG_GRAPH_BUID_2x			= (int)	0x20,
	CFG_GRAPH_BUID_2x_MASK                  = (int) 0x02000000,

	CFG_GRAPH_BUID_2x_PS2_HEADER_START_ADDR1	= (int) 0x000c0000,
	CFG_GRAPH_BUID_2x_PS2_HEADER_STOP_ADDR1		= (int) 0x000c3fff,

	CFG_GRAPH_BUID_2x_PS2_HEADER_START_ADDR2	= (int) 0x000c8000,
	CFG_GRAPH_BUID_2x_PS2_HEADER_STOP_ADDR2		= (int) 0x000c9fff,


	CFG_GRAPH_BUID_2x_ADDR_INCR		= (int) 0x00000800,	/* 2K bytes */
	CFG_GRAPH_BUID_2x_ROM_LO_ADDR		= (int) 0x00100000,
	CFG_GRAPH_BUID_2x_ROM_HI_ADDR		= (int) 0x00200000,

	CFG_GRAPH_BUID_40			= (int) 0x40,
	CFG_GRAPH_BUID_40_MASK			= (int) 0x04000000,
	CFG_GRAPH_BUID_40_START_ADDR		= (int) 0x0f000000,
	CFG_GRAPH_BUID_40_SEG			= (int) 0x840c0020,
	CFG_GRAPH_BUID_40_ROM_LO_ADDR		= (int) 0x0f000000,
	CFG_GRAPH_BUID_40_ROM_HI_ADDR		= (int) 0x0f3fffff,	/* span of 256 KB */


	CFG_GRAPH_60x_BUID_MASK                 = (int) 0x1FF00000,  
	CFG_GRAPH_BUID_7F                       = (int) 0x7F,           /* GXT150 on a 250 */
	CFG_GRAPH_BUID_7F_MASK                  = (int) 0x07F00000,
        /*
         *  See R2/sysx/inc/frs_60x.h  and 60x_regs.h
         */

	CFG_GRAPH_ODM_FILE			= (int)	0x101,
	CFG_GRAPH_METHOD_FILE			= (int) 0x102,
	CFG_GRAPH_CDD_FILE			= (int) 0x103,
	CFG_GRAPH_CDD_PIN_FILE			= (int) 0x104,
	CFG_GRAPH_UCODE_FILE			= (int) 0x105,
	CFG_GRAPH_PERMISSIONS			= (int) ( 0100000 | 0000700 ), 

	CFG_GRAPH_FRS_BLK_SIZE			= (int) 512,

	/*-------------------------------------------
	| dummy name, always last, no "," after it
	|-------------------------------------------*/

	__CFG_GRAPH_last_1			= (int) 0
};

/*======================================================================
|
| FRS_access_t
|
| structure used by cfg_graphics subroutines when dealing with the
| tools in cfg_graphics_frs_tools.h
|
| (tools that manage Feature ROM Scan for displays)
|
|======================================================================*/

typedef struct
{
	/*	Segment registers are required to identify bus ids and to 
	 *  	make the machine device driver interface work correctly.
	 *	The bus slot is also needed (esp. for Micro Channel) to
	 *	generate the correct IOCC address.
	 */

	ulong		bus_segreg;
	ulong		iocc_segreg;

	int		bus_slot;

	/*	The pointers to function are initialized with the addresses
	 *	of functions which invoke the machine device driver
	 */

	int		(* open)();
	int		(* close)();

	int		(* read_byte)();
	int		(* read_word)();
	int		(* write_byte)();
	int		(* write_word)();

	int		(* read_bytes)();
	int		(* read_words)();

	int		(* iocc_read_byte)();
	int		(* iocc_write_byte)();

	/*	These pointers hold the memory copies of valid data structures
	 * 	that have been loaded from the adapter
	 */

	void *		mem_rscan_ps2;
	void *		bus_rscan_ps2;

	void *		mem_rscan_blk;
	void *		bus_rscan_blk;

	void *		mem_rscan_r2_head;
	void *		bus_rscan_r2_head;

	/*	These three values are the classic state values for accessing
	 *	Feature ROM Scan adapters
	 */	

	int		frs_adapter_present;
	int		frs_detected_error;
	int		frs_adapter_bad;

	/*	These values are used for state keeping when interacting with
	 *	the machine device driver
	 */

	int		fd;
	MACH_DD_IO	mdd;

} FRS_access_t;




#endif
