/* @(#)09	1.4  src/bos/kernext/disp/ccm/inc/frs.h, dispccm, bos411, 9428A410j 7/5/94 11:34:14 */
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
|       intel_ushort_t           - Define byte reversed intel 16-bit int.
|       rom_scan_ps2_header_t    - ROM Scan PS/2 Header
|       rom_scan_R2_block_t      - Generic Dynamic ROM block
|       rom_scan_R2_6000_block_t - RS/6000 ROM Block(ID="RISC6000")
|       rom_scan_R2_6001_block_t - RS/6000 ROM Block(ID="RISC6001")
|       rom_scan_R2_6002_block_t - RS/6000 ROM Block(ID="RISC6002")
|       rom_scan_R2_head_t       - Generic RS/6000 ROM Header
|
|	A device ROM is organized as a ps2_header followed by a linked
|	list of blocks followed by other things.  For the R2 case, for
|	either boot or display adapters, the ROM will be organized as
|	a ps2_header, followed by (typically one) R2_600x block, followed
|	by (typically one) R2_head.  The type of both the R2 head and
|	the R2 block depend on the contents of certain standard members
|	common to the generic block and head.
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
|	Scan modules
|
|========================================================================*/

enum
{
#define RSCAN_R2_STRING_ID_1            "RISC6000"
#define RSCAN_R2_STRING_ID_2            "RISC6001"
#define RSCAN_R2_STRING_ID_3            "RISC6002"
#define RSCAN_R2_STRING_ID_4            "RISC6003"

        RSCAN_R2_VIDEO_MAX_POS          = (int) 40,
        RSCAN_R2_MAX_601_CFG      	= (int) 16,

        RSCAN_DYNAMIC_ROM_FLAG_VAL      = (int) 0x7d,

        RSCAN_DYNAMIC_ROM_BLOCK_ID_VAL  = (int) 0x01,

        PS2_ROM_RESERVED1_LEN           = (int) 121,
        PS2_ROM_RESERVED2_LEN           = (int) 130,

        __RSCAN_LAST                    = (int) 0
};

/*=======================================================================
 |
 | The interface to ROM on the adapter may have certain fields
 | that are in the intel format.  These require a byte reversal to
 | be used in the S/6000 code.  This byte reversal is done
 | using the macros in cdd_macros.h.  We will use the following typedef
 | (intel_ushort_t) to identify these fields.
 |
 |======================================================================*/

typedef union
{
        ushort  intel_value;
        struct
        {
                uchar   lower_value;
                uchar   upper_value;
        } RISC_value;
}intel_ushort_t;


/*====================================================================
|       Micro Channel Header
|       Required.  Defined by Boca.
|
|       Supplied by all types of adapters that support
|       all kinds of ROM SCAN
|
|       This structure coordinated
|       in System ROS Dept. D97
|
|	For the Micro Channel, there are assumptions that this structure
|	begins at certain address alignments within a certain I/O address
|	range.  For other bus types, these assumptions are either not
|	present or are present in a different form.  The design of the
|	structure should not enforece assumptions about its placement.
|====================================================================*/

typedef struct
{
        uchar           id_hex55;
                        /*  this char with value 0x55 is one of the clues
                        | that the Video ROM Scan is installed
                        |----------------------------------------------*/

        uchar           id_hexAA;
                        /*  this char with value 0xAA is another clue
                        | that the Video ROM Scan is installed
                        |-----------------------------------------------*/

        uchar           rom_length_in_512B_blocks;
                        /*  measured in units of 512 bytes each.
                        |   from the first byte to the last byte
                        |   (starts at id_hex55 )
                        |-------------------------------------------------*/

        uchar           intel_return;
                        /*
                        | This byte should be Intel Intersegment Return
                        | instruction if there is no PS/2 ROM. Hex CB
                        |-------------------------------------------------*/

        uchar           reserved_1[ PS2_ROM_RESERVED1_LEN ];

        uchar           dynamic_rom_flag;
                        /*  found at offset 125B from start
                        |   this char with value 0x7D is one of the clues
                        |   that the Video ROM Scan is installed
                        |--------------------------------------------------*/

        uchar           reserved_2[ PS2_ROM_RESERVED2_LEN ];

        intel_ushort_t  length_of_dynamic_rom;
                        /*  found at offset 256B from start
                        |   this holds the length in bytes of the Dynamic
                        |   ROM Scan linked list from the start of the
                        |   linked list until its end
                        |--------------------------------------------------*/

        intel_ushort_t  head_of_dynamic_rom;
                        /*  found at offset 258B from start
                        |   holds the offset from the start of the Video
                        |   ROM to the start of the dynamic ROM
                        |---------------------------------------------------*/

} rom_scan_ps2_header_t;


/*=====================================================================
|
| GENERIC ROM SCAN BLOCK
|
| This typedef is used while scanning the PS/2 linked list of blocks.
| until the "block_class" and "R2_id_field" can be scanned.
| Even PS/2 binaries will match this structure
|
| After the "block_class" and "R2_id_field" are scanned, the pointer
| to this structure will be recast to a pointer to a particular
| rom_scan_R2_6000_block type.
|
| At present, the following list holds the actual block types:
|	-  rom_scan_R2_6000_block_t	(Micro Channel boot devices)
|	-  rom_scan_R2_6001_block_t	(Micro Channel display devices
|					and some Micro Channel boot devices)
|	-  rom_scan_R2_6002_block_t	(PowerPC devices)
|	-  rom_scan_R2_6003_block_t	(60x PowerPC devices)
|
|=====================================================================*/

typedef struct
{
        intel_ushort_t  offset_to_next_block;
                        /*  offset in bytes from start of PS/2 ROM to
                        |   the start of the next block.  Note that it is
                        |   always positive.
                        |--------------------------------------------------*/

        intel_ushort_t  block_class;
                        /* holds the value 1 if the ROM is a "feature ROM"
                        |  and can be "switched in" as a replacement for
                        |  PS2 feature ROM.  This is a required field for
                        |  S/6000 Video ROM Scan
                        |--------------------------------------------------*/

        char            R2_id_field[ 8 ];
                        /*  A valid R2 ROM will have the characters
                        |   'RISC6000' or 'RISC6001'
                        |   All Video ROM Scan adapters will use the '6001'
                        |   tag.
                        |   Note that the field is NOT terminated with \0 !!
                        |--------------------------------------------------*/

} rom_scan_R2_block_t;


/*=====================================================================
|
|       R2_ ROM SCAN BLOCK WITH "RISC6000" FLAG
|
| The generic rom scan block is recast as this type, if the
| block_class is "1" and the R2_block_id is 'RISC6000'.
|
| CAUTION:
|	The following members of this structure must be first and
|	cannot be changed:
|	-  offset_to_next_block
|	-  block_class
|	-  R2_id_field
|
|=====================================================================*/

typedef struct
{
        intel_ushort_t  offset_to_next_block;
                        /*  offset in bytes from start of this block to
                        |   the start of the next block.  Note that it is
                        |   always positive.
                        |--------------------------------------------------*/

        intel_ushort_t  block_class;
                        /* holds the value 1 if the ROM is a "feature ROM"
                        |  and can be "switched in" as a replacement for
                        |  PS2 feature ROM.  This is a required field for
                        |  S/6000 Video ROM Scan
                        |--------------------------------------------------*/

        char            R2_id_field[ 8 ];
                        /*  holds the characters 'RISC6000' if this block
                        |   of the ROM is valid for Video ROM Scan
                        |   Note that the field is NOT terminated with \0 !!
                        |--------------------------------------------------*/

        ulong           R2_rom_busmem_addr;
                        /*  actually should be considered a pointer
                        |   the most significant 4 bits are always 0
                        |   the bus mem max address is 28 MB - 1
                        |---------------------------------------------------*/

        uchar           pos2_value;
        uchar           pos3_value;
        uchar           pos4_value;
        uchar           pos5_value;
                        /*  the semantics are that these values are written
                        |   in order to the adapter to set it up for
                        |   initial access.  The intent is that one writes
                        |   these values, and then one can read the S/6000
                        |   ROM contents at the address of busmem given
                        |   above
                        |---------------------------------------------------*/

} rom_scan_R2_6000_block_t;


/*=====================================================================
|
|       R2 ROM SCAN BLOCK WITH "RISC6001" FLAG
|
| The generic rom scan block is recast as this type, if the
| block_class is "1" and the R2_block_id is 'RISC6001'.
|
| This typedef is used by the display adapter Feature  ROM scan code. 
| IPL ROS will also support device boot adapters with this block type.
|
| CAUTION:
|	The following members of this structure must be first and
|	cannot be changed:
|	-  offset_to_next_block
|	-  block_class
|	-  R2_id_field
|
|=====================================================================*/

typedef struct
{
        intel_ushort_t  offset_to_next_block;
                        /*  offset in bytes from start of this block to
                        |   the start of the next block.  Note that it is
                        |   always positive.
                        |--------------------------------------------------*/

        intel_ushort_t  block_class;
                        /* holds the value 1 if the ROM is a "feature ROM"
                        |  and can be "switched in" as a replacement for
                        |  PS2 feature ROM.  This is a required field for
                        |  S/6000 Video ROM Scan
                        |--------------------------------------------------*/

        char            R2_id_field[ 8 ];
                        /*  holds the characters 'RISC6001' if this block
                        |   of the ROM is valid for Video ROM Scan
                        |   Note that the field is NOT terminated with \0 !!
                        |--------------------------------------------------*/

        ulong           R2_rom_busmem_addr;
                        /*  actually should be considered a pointer
                        |   the most significant 4 bits are always 0
                        |   the bus mem max address is 28 MB - 1
                        |--------------------------------------------------*/

        uchar           num_pos_vals;

        struct
        {
                uchar   reg;
                uchar   val;
        }
                        pos[ RSCAN_R2_VIDEO_MAX_POS ];
                        /*  the semantics are that these values are written
                        |   in order to the adapter to set it up for
                        |   initial access.  The intent is that one writes
                        |   these values, and then one can read the S/6000
                        |   ROM contents at the address of busmem given
                        |   above.  This design is more flexible than the
                        |   RISC6000 alternative.
                        |---------------------------------------------------*/

        ulong           sw_version;
                        /* should match the SW version of the CDD and the
                        | CCM VDD
                        |----------------------------------------------------*/

} rom_scan_R2_6001_block_t;


/*=====================================================================
|
|       R2 ROM SCAN BLOCK WITH "RISC6002" FLAG
|
| The generic rom scan block is recast as this type, if the
| block_class is "1" and the R2_block_id is 'RISC6002'.
|
| This form of the block is designed for a configuration interface
| in which device registers are co-located in a contiguous address
| range and in which the contents of the configuration registers
| are always 32 bits.  
|
| The PowerPC 601 Bus has such characteristics.
|
| This typedef is used by the display adapter Feature  ROM scan code. 
| IPL ROS will also support device boot adapters with this block type.
|
| CAUTION:
|	The following members of this structure must be first and
|	cannot be changed:
|	-  offset_to_next_block
|	-  block_class
|	-  R2_id_field
|
|=====================================================================*/

typedef struct
{
        intel_ushort_t  offset_to_next_block;
                        /*  offset in bytes from start of this block to
                        |   the start of the next block.  Note that it is
                        |   always positive.
                        |--------------------------------------------------*/

        intel_ushort_t  block_class;
                        /* holds the value 1 if the ROM is a "feature ROM"
                        |  and can be "switched in" as a replacement for
                        |  PS2 feature ROM.  This is a required field for
                        |  S/6000 Video ROM Scan
                        |--------------------------------------------------*/

        char            R2_id_field[ 8 ];
                        /*  holds the characters 'RISC6002' if this block
                        |   of the ROM is valid for Video ROM Scan
                        |   Note that the field is NOT terminated with \0 !!
                        |--------------------------------------------------*/

        ulong           R2_rom_busmem_addr;
                        /*  actually should be considered a pointer
                        |   Holds the address of the beginning of the
			|   R2_header associated with this block.  The address
                        |   is a 32 bit real address,
                        |--------------------------------------------------*/

	uchar		num_cfg_regs;

	uchar		cfg_reg_index[ RSCAN_R2_MAX_601_CFG ];

	ulong		cfg_reg_value[ RSCAN_R2_MAX_601_CFG ];

                        /*  the semantics are that these values are written
                        |   in order to the adapter to set it up for
                        |   initial access.  The intent is that one writes
                        |   these values, and then one can read the S/6000
                        |   ROM contents at the address of busmem given
                        |   above.  
			|
			|   The assumption is that all registers are located
			|   contiguously, and that the starting point of the
			|   registers is known and the offset between each
			|   register is known.  The index is multiplied by
			|   the offset constant and added to the base address
			|   to produce the address.  The 32 bit value is
			|   then written to the register to initialize the
			|   adapter.
                        |---------------------------------------------------*/

        ulong           sw_version;
                        /* should match the SW version of the CDD and the
                        | CCM VDD
                        |----------------------------------------------------*/

} rom_scan_R2_6002_block_t;

/*=====================================================================
|
|       R2 ROM SCAN BLOCK WITH "RISC6003" FLAG
|
| The generic rom scan block is recast as this type, if the
| block_class is "1" and the R2_block_id is 'RISC6003'.
|
| This form of the block is designed for a configuration interface
| that matches the PowerPC 60X Bus and PowerPC System Architecture
| specifications.
|
| This typedef is used by the display adapter Feature  ROM scan code.
|
| CAUTION:
|       The following members of this structure must be first and
|       cannot be changed:
|       -  offset_to_next_block
|       -  block_class
|       -  R2_id_field
|
|=====================================================================*/

typedef struct
{
        intel_ushort_t  offset_to_next_block;
                        /*  offset in bytes from start of this block to
                        |   the start of the next block.  Note that it is
                        |   always positive.
                        |--------------------------------------------------*/

        intel_ushort_t  block_class;
                        /* holds the value 1 if the ROM is a "feature ROM"
                        |  and can be "switched in" as a replacement for
                        |  PS2 feature ROM.  This is a required field for
                        |  S/6000 Video ROM Scan
                        |--------------------------------------------------*/

        char            R2_id_field[ 8 ];
                        /*  holds the characters 'RISC6002' if this block
                        |   of the ROM is valid for Video ROM Scan
                        |   Note that the field is NOT terminated with \0 !!
                        |--------------------------------------------------*/

        ulong           R2_rom_busmem_addr;
                        /*  actually should be considered a pointer
                        |   Holds the address of the beginning of the
                        |   R2_header associated with this block.  The address
                        |   is a 32 bit real address,
                        |--------------------------------------------------*/

        ulong           sw_version;
                        /* should match the SW version of the CDD and the
                        | CCM VDD
                        |----------------------------------------------------*/

} rom_scan_R2_6003_block_t;



/*=====================================================================
|
| R2 Generic Header...
|
|=====================================================================*/

typedef struct {
	ushort rom_length_in_512B_blocks;
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

}  rom_scan_R2_head_t ;

