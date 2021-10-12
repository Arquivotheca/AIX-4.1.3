/* @(#)10	1.4  src/bos/kernext/disp/ccm/inc/frs_display.h, dispccm, bos411, 9428A410j 7/5/94 11:34:30 */
/*
 *
 * COMPONENT_NAME: (SYSXDISPCCM) Common Character Mode Device Driver
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1992, 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/*=========================================================================
|       The typedefs in this file are:
|
|	rom_scan_video_head_t	 - The display adapter feature ROM header
|
|	A device ROM is organized as a ps2_header followed by a linked
|	list of blocks followed by other things.  For the R2 case, for
|	either boot or display adapters, the ROM will be organized as
|	a ps2_header, followed by (typically one) R2_600x block, followed
|	by (typically one) R2_head.  The type of both the R2 head and
|	the R2 block depend on the contents of certain standard members
|	common to the generic block and head.
|
|	This module defines the R2 head for display adapters.
|
|=========================================================================*/

/*=======================================================================
|
| CAUTION:
|	The typedefs and constants in this file are used by hardware
|	and by software development groups.  ROMs are built based on 
|	these structures that are included in planars and in adapters.
|	Changes to these structures should be made with the understanding
|	of the backward and forward compatibility issues associated with
|	all released hardware and future hardware offerings.
|
|========================================================================*/


/*=======================================================================
|
|	The following constants are defined for use by all Feature ROM
|	Scan display adapters on all platforms
|
|========================================================================*/

enum
{
        VRS_R2_ROM_VIDEO_BIT            = (int) ( 1UL << 1 ),

        VRS_MAX_NUM_KMODS               = (int) 3,

        VRS_MAX_NUM_EXTENSION_KMODS     = (int) 3,

	VRS_COPYRIGHT_SIZE		= (int) 256,

               /*----------------------------------------
                | The following are the indices into the
                | array of xcoff modules stored in the
                | R2 Video ROM
                |----------------------------------------*/

        VRS_KMOD_ENTRY_POINT            = (int) 0,
        VRS_KMOD_INTERRUPT              = (int) 1,
	VRS_KMOD_CFG_METHOD		= (int) 2,

		/*--------------------------------------
		| There are no extension KMODs defined at
		| this time
		|----------------------------------------*/

        __VRS_LAST                      = (int) 0
};



/*=====================================================================
|
|       R2 VIDEO ROM HEADER (Type = 2)
|
| NOTE: The start address of the R2 VIDEO ROM is assumed to be the
|	first byte of this structure. The last byte of the R2 VIDEO ROM
|	is assumed to be the last byte of the last data structure that
|	follows this structure.  All of the kmods and other files must
|	be included in the span of the R2 VIDEO ROM.
|
|====================================================================*/

typedef struct
{
        ushort          rom_length_in_512B_blocks;
                        /*  length from the start of the S/6000 ROM space
                        |   to the end, in units of 512 bytes.
                        |---------------------------------------------------*/

        ushort          rom_CRC_residue;
                        /*  16 bit CRC residue using seed of 0xFFFF
                        |   and polynomial 1+ X^5 + X^12 + X^16
                        |   runs from 4/th B of ROM to last byte of last block
                        |---------------------------------------------------*/

        ulong           rom_type;
                        /*  (1UL << 0 ) = boot device
                        |   (1UL << 1 ) = video device
                        |   both bits are permitted at same time
                        |--------------------------------------------------*/

        struct
        {
                ulong           offset_to_kmod;
                ulong           length_of_kmod;
        }
                kmod_functions[VRS_MAX_NUM_KMODS];
                        /*
                        |  the first  is the (req'd) entry point code
                        |  the second is the (optional) interrupt handler
			|  the third  is the (req'd) config method code
                        |  no others are defined
			|  a length of 0 and offset of 0 mean no kmod.
                        |  offsets are added to the base address of the R2 rom.
                        |  lengths are in units of "size_t"
                        |---------------------------------------------------*/

        ulong           offset_to_ucode;
        ulong           length_of_ucode;

                        /*  each offset is added to the base address of
                        |   the S/6000 ROM, and points to the first byte
                        |   of the corresponding XCOFF binary image that
                        |   holds a S/6000 executable.  The length holds
                        |   the length in bytes from the first to the last
                        |   of the binaries.  Microcode and the ODM file
			|   are NOT XCOFF.  Microcode format is device 
			|   specific.  ODM is a flat ASCII file.
                        |--------------------------------------------------*/

	ulong		offset_to_ODM_file;
	ulong		length_of_ODM_file;

			/* offset and length are as described above.
			|  The ODM file is not used by the IPL ROM.
			|  The device specific config method will have a 
			|  routine which will unpack and process the ODM file.
			|---------------------------------------------------*/

        struct
        {
                ulong           offset_to_kmod;
                ulong           length_of_kmod;
        }
                	kmod_extensions[VRS_MAX_NUM_EXTENSION_KMODS];
                        /*
                        |  three slots are permitted -- none will be used
			|  at the present time
			|  a length of 0 and offset of 0 mean no kmod.
                        |  offsets are added to the base address of the R2 rom.
                        |  lengths are in units of "size_t"
                        |---------------------------------------------------*/

	char		copyright_notice[VRS_COPYRIGHT_SIZE];
			/*
			|  holds a null terminated string that gives the IBM
			|  Copyright Statement.  THis is used to differentiate
			|  IBM ROMS from non-IBM ROMS.  It also has legal uses.
			|  Only IBM adapters will have an IBM Copyright
			|---------------------------------------------------------*/ 

} rom_scan_video_head_t ;
