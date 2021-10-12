static char sccsid[] = "@(#)70  1.19  src/bos/usr/lib/methods/graphics/cfg_graphics_frs_tools.c, dispcfg, bos411, 9428A410j 6/16/94 17:48:48";
/*
 *   COMPONENT_NAME: DISPCFG
 *
 *   FUNCTIONS: FRS_Copy_File
 *		FRS_Enable_BUID_2x_Device1620
 *		FRS_Enable_BUID_40_Device1715
 *		FRS_Find_Video_Device2292
 *		FRS_Make_Temp_File2599
 *		FRS_Search_for_RSCAN_6001_BLK1801
 *		FRS_Search_for_RSCAN_6002_BLK2032
 *		FRS_Test_Addr_for_RSCAN_PS21447
 *		FRS_Update_ODM_From_File2824
 *		FRS_Validate_RSCAN_VIDEO_HEAD1306
 *		FRS_checksum
 *		FRS_crc16
 *		nvram_close
 *		nvram_iocc_read_byte
 *		nvram_iocc_write_byte
 *		nvram_open
 *		nvram_read_byte
 *		nvram_read_bytes
 *		nvram_read_word
 *		nvram_read_words
 *		nvram_write_byte
 *		nvram_write_word
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992, 1993, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

  
/*****************************************************************************
 ********************** MODULE HEADER TOP ************************************
 *****************************************************************************
 ---------------------------------------------------------------------------
  
 PURPOSE:	
		This routine is part of a problem-state process that
		is executed by the system during configuration and by
		the system when certain configuration commands are 
		invoked.  The functions it contains are the standard
		functions used by the AIX devices configuration architecture.
  
		Typically, device specific configuration methods are
		compiled into a ".o" which are linked with a standard
		"main.o".  This is because the configuration process is
		relatively standard.  However, for display devices, there
		are sufficient reasons to have a separate "main" that
		has been design just for graphics.
  
		This software therefore is called from routines in 
		the companion file "cfg_graphics.c".
  
		This routine also supports the common character mode
		VDD device driver in addition to the regular full function
		VDD.  See design 13588 for additional details.

  MODULE ORGANIZATION:

	The executable config method for any graphics device is composed
	of several modules that are linked together.  The following
	is a list of the required and the optional modules:

	cfg_graphics.o		This module provides the "main" routine.
				It contains the control flow and the
				device independent front end.  It manages
				most of the interface with the ODM.

				The module contains static internal routines
				that it alone uses.

				The module also contains some global data
				available to all of the other modules.

	cfg_graphics_tools.o	A set of device independent tools that support
				the cfg_graphics programs but are useful to
				other configuration methods and to device
				dependent mehtods as well.

	cfg_graphics_frs_tools.o A set of device independent routines which
				help manage the Feature ROM Scan capabilities
				found on newer display adapters.

	chkslot.o		This is a system module that does checking
				of the MicroChannel slot.

	cfgtools.o		This is a system module that provides a
				basic set of configuration tools for use
				by the other pieces of the configuration
				method executable.

	cfg_ccmdd.o		This module contains the common character
				mode substitutes that are used instead
				of the per-device entry points.  The
				"main" routine will either route its
				device cfg method calls to the device 
				method or to cfg_ccmdd.o, depending on 
				the presence of a working device vdd and
				a working device ucode file.
 
 NOTES ON MACROS:

	1. This module makes heavy use of macros.
	2. In general, macos begin with CAPITALIZED words.
	3. Macros with common prefix values are generally all related
	   and may all come from the same .h file.
	4. The following pairing of macros and include files
	   is useful to understand:

	   INCLUDE FILE		MACRO FILE		PREFIX
	   ============		==============		===========
	    ccm.h		ccm_macros.h		CCM_
	    cdd.h		cdd_macros.h		CDD_
	    frs.h	 	frs_macs.h		RSCAN_
	    frs_display.h	frs_display_macs.h	RSCAN_
	    cfg_graphics.h	cfg_graphics_macros.h	CFGGRAPH_
	   
	5. Inside a given macro file, there are usually references
	   to more than one typedef.  Each typedef has its own
	   unique string.  This string is added to the macro file
	   prefix.  For example, consider "frs.h" with prefix "FRS_"
	   used in its macros.  The following table would apply:

	   File: frs.h		Prefix:	FRS_

	   TYPEDEF			MACRO_PREFIX
	   =========			==============
	   rom_scan_ps2_header_t	RSCAN_PS2_
	   rom_scan_R2_6002_block_t	RSCAN_6002_BLK_
	
	6. The portion of the macro which references specific members 
	   of a structure is usually not capitalized, e.g.

	 	RSCAN_PS2_dynamic_rom_flag( &head )

	7. If a string is FULL CAPS and has the naming convention above,
	   it is most likely a constant and was most likely defined
	   in the include file rather than the macro file.  The only
	   exceptions to this are the macros which are used as 
	   typedefs or structure sizeof values.

  
 *****************************************************************************
 ********************** MODULE HEADER BOTTOM *********************************
 *****************************************************************************/


/*==========================================================================
|
|  System include files required by all cfg method main programs
|
|==========================================================================*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/lockl.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/sysconfig.h>
#include <sys/sysmacros.h>
#include <sys/device.h>
#include <fcntl.h>
#include <sys/mode.h>

#include <sys/stat.h>			/* used by DD_Not_Found */



/*==========================================================================
|
|  Local debug routines are defined here       
|
|==========================================================================*/

#include "cfgdebug.h"

#undef	WGA_PROTOTYPE_MODEL

/*==========================================================================
|
|  Include files which describe the common character mode interface and
|  the video ROM scan interface
|
|==========================================================================*/

#ifdef CFGDEBUG
#        define STATIC
#else
#       define STATIC static
#endif

#define FAIL -1
#define PASS 0

#define MEM_BLOCK_40K ( 40 * 1024 )
#define MIN( a , b )       ( ( (a) < (b) ) ? (a) : (b) )

#define Boolean unsigned int		/* need by aixfont.h		*/
#define Bool    unsigned int		/* need by aixfont.h		*/

#include <sys/aixfont.h>
#include <sys/dir.h>		    /* needed by ccm_dds.h	  */
#include <sys/mdio.h>		   /* needed by cdd_macros.h       */
#include <sys/file.h>		   /* needed by ccm.h	      */
#include <sys/intr.h>		   /* needed by vt.h	       */

#include <sys/ioacc.h>		   /* needed by ccm_dds.h	  */
#include <sys/adspace.h>	   /* needed by ccm_dds.h	  */

#include <sys/display.h>
#include "vt.h"

#include  "cdd.h"
#include  "cdd_macros.h"

#include  "cdd_intr.h"
#include  "cdd_intr_macros.h"

#include  "ccm.h"
#include  "ccm_macros.h"

#include  "frs.h"
#include  "frs_macs.h"

#include  "frs_display.h"
#include  "frs_display_macs.h"

#include <graphics/60x_regs.h>              /* 601_CHANGE */
#include "frs_60x.h"

/*==========================================================================
|
| include a file which defines the cfg macros used by our cfg methods
|
|==========================================================================*/

#include "cfg_graphics.h"
#include "cfg_graphics_frs.h"
#include "cfg_graphics_macros.h"


/*==========================================================================
|
|  Define function references				 
|
|==========================================================================*/

/*--- from libc.a ---*/
extern	 int		getopt( );
extern	 void		 exit( );
extern	 int		 strncmp( );
extern	 int		 atoi( );
extern	 long		 strtol( );


/*--- from libcfg.a ---*/
extern	 int	      busresolve ();



/*--- from chkslot.o ----*/
extern int      chkslot ();




/*---- from cfgtools.o ----*/
extern	int		Get_Parent_Bus( );
extern	int		getatt( );
extern	int		convert_att( );
extern	int		convert_seq( );
extern	struct attr *	att_changed( );
extern	int		mk_sp_file( );
extern	char *		read_descriptor( );
extern	void		add_descriptor( );
extern	void		put_vpd( );



/*--- from AIX BOS runtime environment ---*/
extern int		 odm_initialize ( );
extern int		 odm_lock( );
extern void		 setleds( );


/*==========================================================================
|
|  Define some data global to these functions, so that it can be readily
|  shared and accessed by the functions and by the "main" and other functions
|  in "cfg_graphics.c"  and the functions in cfgyyy.c	 
|
|==========================================================================*/


		/*=========================================================
		|
		| Define the anchor pointer used for cfg_graphics macros
		| to access various pieces of global data required by
		| the config methods and their device specific pieces
		|
		|==========================================================*/


extern cfg_graphics_data_t *	__GLOBAL_DATA_PTR;


CLASS_SYMBOL odm_mount_class();
extern int stanza_line_number;


/*****************************************************************************
 ********************** GENERAL COMMENTS ON FEATURE ROM SCAN *****************
 *****************************************************************************
  
  The purpose of the ROM Scan function is to allow future
  Microchannel adapters to participate as display consoles without
  requiring changes to IPL ROS or to the Common Character Mode
  display device driver architecture.  It also minimizes the media
  requirements of boot and diagnostic media, since no device 
  microcode is stored on the media, and since only one device
  driver is stored.

  Microchannel cards implement ROM Scan using a dynamic linked
  list of ROM blocks.  Each ROM block has a field which
  identifies the ROM class.  The ROM Scan process involves a
  search of this linked list for ROMs with the class "switch
  adapter ROM".  If a ROM of this class is found, and it is
  identified as a RISC/6000 ROM, then IPL ROS will attempt to
  load and execute this code in order to complete the IPL
  process.
 
  ROM Scan involves two main functions:
 
  1. Search for feature ROM Scan cards on the Microchannel.  The
     results of this search are stored in CDD structures.
 
  2. Add the various XCOFF binaries to the program or to the
     kernel using the load or the kmod_load calls, respectively.
 
 
  DESIGN CONSIDERATIONS/HISTORY
  _____________________________

  (NOTE:  This section is based on the Microchannel adapter
  strategy.  See other design documents for the strategy for
  other buses. )
 
  ROM Scan cards will contain ROMs with Video ROM Feature Code
  that will enable the adapter to participate in the CCM
  process.  PC and PS/2 systems have used a ROM Scan process
  which allows adapter cards to automatically link their
  initialization functions into the IPL process.
 
  Listed below are some of the design considerations for 
  Video ROM Scan:
 
 
  1. Each RISC/6000 ROM will have a field which identifies it as
      either Device Boot or Video Expansion code (Video ROM Scan
      will be supported in Release 3.2.2).
 
  2. All cards implementing ROM Scan will contain a PS/2 ROM
     header.  PS/2 ROMs will be validated using a checksum.  The
     algorithm is a modulo 0x100 summation of all the bytes of
     the PS/2 ROM.  The sum will be zero for a valid ROM.
 
  3. The Feature ROMs are accessed as Bus memory.
 
  4. PS/2 ROM header records will be located on 2K boundaries in
     bus memory address space ranges of hex C0000 through C3FFF 
     and C8000 through C9FFF.
 
  5.  RISC/6000 ROMs must be in the bus memory address range 
      of 1M to 2M.
 
  6. No CCM DD or config method shall use any DMA or Interrupt 
     services in accessing the adapter card's Feature ROM.
 
  7. RISC/6000 ROMs will validated by a 16 bit CRC.  The CRC
     polynomial is 1 + X(5) + X(12) + X(16) with the CRC being
     initialized to hex 'FFFF'.  The CRC is calculated by
     starting at the fifth byte ( offset 4) in the RISC/6000
     ROM.
 
  8. During any ROM Scan I/O attempt with a ROM Scan card, all 
     other cards on the same bus will be disabled.
 

   PSEUDOCODE / LOGICAL STATES
 
  The three status flags adapter_present, adapter_ bad and
  detected_error are set according to the following conditions:
 
  o   Adapter_present:
 
      This flag is set to '1' if the conditions below are met:
 
      - A valid PS/2 ROM is found (i.e. Hex 55,AA for the first
	two bytes)
      - A dynamic link flag of 0x7D is found.
      - The PS/2 ROM contains a block with the "RISC6000" flag.
 
  o   Detected_error:
 
      This flag is set to 0, 1, 2 or 3 depending on the 
      following conditions:

      0 - No software error was detected by Romscan.
      1 - The RISC/6000 ROM failed the CRC (adapter_present will be '1').
      2 - The address of the  RISC/6000 ROM was not in the valid range
	  (adapter_present will be '1').
      3 - The PS/2 ROM failed the Check sum (adapter_present will be zero).
 
  o   Adapter_bad:
 
      This flag is meaningful only when the adapter_present flag 
      is '1'.  It is set to 0,1 or 2 depending on the following conditions:

      0 - No software error was detected.
      1 - The RISC/6000 ROM failed the CRC. (adapter_present will be '1')
      2 - The address of the  RISC/6000 ROM was out of the valid range
	  (adapter_present will be '1').
 
 
  ADAPTER ROMS
 
  All cards implementing ROM Scan will contain the standard PS/2
  ROM header.  CCM will first look at this header to
  determine if the card contains a dynamic ROM list in which a
  RISC/6000 ROM might be found.  The PS/2 ROM header has the
  format shown in Figure 1.
 
 
     0   *-----------------------------------------------------*
	 |						     |
	 |		    Hex '55AA'		       |
	 |						     |
     2   |-----------------------------------------------------|
	 |						     |
	 |       Length of PS/2 ROM in 512 byte blocks	 |
	 |						     |
     3   |-----------------------------------------------------|
	 |			 .			   |
	 |			 .			   |
	 |			 .			   |
   125   |-----------------------------------------------------|
	 |						     |
	 |      Flag indicating Dynamic ROM list exists	|
	 |    This Byte will equal hex '7D' if list exists     |
	 |						     |
   126   |-----------------------------------------------------|
	 |			 .			   |
	 |			 .			   |
	 |			 .			   |
   256   |-----------------------------------------------------|
	 |						     |
	 |	     Length of Dynamic ROM list	      |
	 |						     |
   258   |-----------------------------------------------------|
	 |						     |
	 |       Offset to beginning of Dynamic ROM list       |
	 |		    (two bytes)		      |
	 |						     |
	 *-----------------------------------------------------*
	 0						    31
 
  Figure 1. Format PS/2 ROM Header for ROM Scan adapter cards
 
 
 
  The Dynamic ROM list will be contained in the PS/2 ROM.  All
  blocks in the linked list will have a pointer and a block
  identifier.  A RISC/6000 block will have the format shown in
  Figure 2.
 
 
 
     0   *-----------------------------------------------------*
	 |						     |
	 |	  Pointer to next block in linked list       |
	 |	      (zero if last block in list)	   |
	 |						     |
     2   |-----------------------------------------------------|
	 |						     |
	 |       Block Identifier: 1 = "Switch Adapter ROM"    |
	 |						     |
	 |-----------------------------------------------------|
	 |						     |
	 | ****  The following fields pertain only to a  ****  |
	 | ****	      RISC/6000 Block	     ****  |
	 |						     |
     4   |-----------------------------------------------------|
	 |						     |
	 |	   RISC/6000 ROM Identifier Flag	     |
	 |	    ASCII characters "RISC6000"	      |
	 |						     |
    12   |-----------------------------------------------------|
	 |						     |
	 |	 Address of RISC/6000 ROM when enabled       |
	 |						     |
    16   |-----------------------------------------------------|
	 |						     |
	 |	 Value of POS Registers 2-5 to enable	|
	 |	       RISC/6000 ROM (4 bytes)	       |
	 |						     |
	 *-----------------------------------------------------*
	 0						    31
 
  Figure 2. Structure of blocks in Dynamic ROM list
 
  ROM Scan will traverse this linked list looking for a block
  with an ID of '1'.  This identifies the ROM associated with
  this block as a "Feature ROM" and can be "switched in" as a
  replacement for the PS/2 ROM.  For a RISC/6000 ROM, the eight
  bytes after the block ID will contain the ASCII characters
  "RISC6000".  The block will also contain the start address of
  the RISC/6000 ROM and the value of the POS Registers two
  through five which should be used to enable the ROM.  These
  values must be written to the POS registers for RISC/6000 ROM
  to be accessed.
 
  The RISC/6000 ROM will have a 256 byte header with the format
  shown in Figure 3.
 
 
 
 
     0   *-----------------------------------------------------*
	 |						     |
	 |      Length of RISC/6000 ROM in 512 byte blocks     |
	 |						     |
     2   |-----------------------------------------------------|
	 |						     |
	 |	     16 bit CRC for RISC/6000 ROM	    |
	 |						     |
     4   |-----------------------------------------------------|
	 |						     |
	 |		  RISC/6000 ROM Type		 |
	 |						     |
     8   |-----------------------------------------------------|
	 |						     |
	 |	    Offset to boot expansion code	    |
	 |						     |
    12   |-----------------------------------------------------|
	 |						     |
	 |		Length of data area		  |
	 |	  required by boot expansion code	    |
	 |						     |
    16   |-----------------------------------------------------|
	 |						     |
	 |		240 bytes Reserved		   |
	 |						     |
	 *-----------------------------------------------------*
	 0						    31
 
  Figure 3. Format of RISC/6000 ROM header for ROM Scan adapter cards
 
 FUNCTION/FLOW
  _____________
 
 
  For each slot the following steps must be performed:
 
  1.  Initialize the three  status  flags  adapter_present,  
      adapter_bad and detected_error to FALSE:
 
  2.  Check if an adapter card is present
      Note:
	  For slots greater than 7 (slots start at 0) the BUID 
	  in the segment register must be changed
	  in order to access them.
 
  3.  If an adapter is present then enable the card.
 
  4.  Scan the I/O address range C0000 -> DFFFF in 2K increments
      for  a  PS/2 header (55 AA).  (Only one header per slot)
 
  5.  If  a  PS/2  header is found then validate the PS/2 ROM by 
      using a modulo hex 100 checksum.
 
      -   If  the  Checksum fails then set the detected_error flag   

  6.   If the PS/2 ROM is valid then check if a Dynamic ROM list exists.
 
  7.   If a Dynamic ROM list exists then do a linear search of 
       the linked list for a RISC/6000 ROM.
 
  8.   If a RISC/6000 ROM does exist then set the adapter_present 
       flag to TRUE.
 
  9.   Enable RISC/6000 ROM by writing the given values to the 
       POS registers  2 through 5.
 
  10.  Check  to see if the RISC/6000 ROM address is in the 
       valid range of 64K to 1Meg.
 
      -   If the address is invalid then set the adapter_bad 
	  and detected_error flags 
 
  11.  If the address is valid then perform a 16 bit CRC on 
       the RISC/6000 ROM.
 
      -   If the ROM is invalid then set  the  adapter_bad  
	  and  detected_error flags 
 
  12.  If the status flags have been set to the following state:
 
      -   adapter_present= TRUE
      -   adapter_bad= FALSE
      -   detected_error = FALSE
      -   RISC6000_ROM_type = VIDEO_ROM
 
      then set the scan_code_present flag to VIDEO_ROM.  If 
      the flags are not in the valid state then set the 
      scan_code_present flag equal to the detected_error flag.
 
  13.  If the ROM is valid then attempt to allocate memory 
       for the various load modules and then attempt to read them in.
 
 
  14.  Disable card by resetting bit 0 in POS register 2.
 
  
 *****************************************************************************
 ********************** END OF GENERAL COMMENTS ******************************
 *****************************************************************************/


/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME:	 	
			nvram_open()
			nvram_close
			nvram_read_byte()
			nvram_read_bytes()
			nvram_read_word()
			nvram_write_byte()
			nvram_write_word()

  
 TITLE:	 	Functions which driver the machine device driver and perform
		I/O to determine the existence of Feature ROM Scan constructs
  
 ------------------------------------------------------------------------	
  
 FUNCTION:
		/dev/nvram is the most general means of access to the machine
		device driver.  Other forms include /dev/bus0.  We will use
		the general means whenever possible

 NOTE:		Defect 64362 discusses a bug in /usr/ccs/libc.a.min in whihc
		the ioctlx function is not exported from the library.  The
		workaround for this defect in our code is to use another
		exported symbol, __ioctl.  Not pretty, but it works.
 
 EXECUTION ENVIRONMENT: User process environment only
  
 INPUTS:
		ctl		- a structure of type FRS_access_t * 
				  this structure holds the data and pointers that
				  are used by all FRS_* subroutines
  
 RETURNS:
 		an integer return code stating whether an error occurred or not. 
	
		the member ctl.mdd is updated after calls to the machine device
		driver
 
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/

#define	IOCTLX	__ioctl	

	
/*----------------------------------------------
|
|	FUNCTION DECLARATION
|
|-----------------------------------------------*/

STATIC int
nvram_open(
		FRS_access_t *		ctl
	   )
{
	ctl->fd	= open( "/dev/nvram", O_RDWR, 0 );

	if ( ctl->fd < 0 )
	{
		DEBUG_1( "Open of /dev/nvram failed with rc=%x \n", ctl->fd )
		
		return (ctl->fd);
	}

	return (0);
}

/*----------------------------------------------
|
|	FUNCTION DECLARATION
|
|-----------------------------------------------*/

STATIC int
nvram_close(
		FRS_access_t *		ctl
	   )
{
	close( ctl->fd );

	return (0);
}

/*----------------------------------------------
|
|	FUNCTION DECLARATION
|
|-----------------------------------------------*/

STATIC int
nvram_read_byte(
		FRS_access_t *		ctl,
		ulong			addr,
		uchar *			pval
	       )
{
	int			rc;
	uchar			local_value;


	ctl->mdd.md_addr		= addr;
	ctl->mdd.md_data		= (char * ) &local_value;
	ctl->mdd.md_size		= 1;
	ctl->mdd.md_incr		= MV_BYTE;

	rc = IOCTLX( 	ctl->fd,
			MIOBUSGET,
			& (ctl->mdd),
			ctl->bus_segreg	);

	if ( rc != 0 )
	{
		return (E_DEVACCESS);
	}

	*pval			= local_value;

	return (0);
}
	
/*----------------------------------------------
|
|	FUNCTION DECLARATION
|
|-----------------------------------------------*/

STATIC int
nvram_iocc_read_byte(
		FRS_access_t *		ctl,
		ulong			addr,
		uchar *			pval
	       )
{
	int			rc;
	uchar			local_value;


	ctl->mdd.md_addr		= addr;
	ctl->mdd.md_data		= (char * ) &local_value;
	ctl->mdd.md_size		= 1;
	ctl->mdd.md_incr		= MV_BYTE;

	rc = IOCTLX( 	ctl->fd,
			MIOBUSGET,
			& (ctl->mdd),
			ctl->iocc_segreg	);

	if ( rc != 0 )
	{
		return (E_DEVACCESS);
	}

	*pval			= local_value;

	return (0);
}
	
/*----------------------------------------------
|
|	FUNCTION DECLARATION
|
|-----------------------------------------------*/

STATIC int
nvram_read_word(
		FRS_access_t *		ctl,
		ulong			addr,
		ulong *			pval
	       )
{
	int			rc;
	ulong			local_value;


	ctl->mdd.md_addr		= addr;
	ctl->mdd.md_data		= (char * ) pval;
	ctl->mdd.md_size		= 1;
	ctl->mdd.md_incr		= MV_WORD;

	rc = IOCTLX( 	ctl->fd,
			MIOBUSGET,
			& (ctl->mdd),
			ctl->bus_segreg	);

	if ( rc != 0 )
	{
		return (E_DEVACCESS);
	}

	*pval			= local_value;

	return (0);
}
	
/*----------------------------------------------
|
|	FUNCTION DECLARATION
|
| This function is not a general purpose read words function! 
|
| It is mainly used to read the Feature ROM.  On 601 device, 
| it uses ioctl MIOVPGGET provided by the machine driver
|
| Currently MIOVPDGET only works on power pc and it only
| reads the ROM in bytes.   
|
|-----------------------------------------------*/

STATIC int
nvram_read_words(
		FRS_access_t *		ctl,
		ulong			addr,
		uchar *			pval,
		ulong			len		/* Length in bytes */
	       )
{
	int			rc;
	int			i;
	ulong			address;
	void *			local_values;


	if ( ADDR_IS_60x_VPD_FRS_SPACE(ctl->bus_segreg, addr) )
	{
		rc = rd_60x_vpd_feat_rom (ctl->bus_slot, addr, pval, len);

		if (rc)
		{
			DEBUG_3("601 read_words:failed - slot=%x, seg=%x, addr=%x\n",
                                               ctl->bus_slot,ctl->bus_segreg,addr);
			return (E_DEVACCESS);
		}

		return (0);

	}
	else if ( ADDR_IS_60x_CFG_SPACE(ctl->bus_segreg, addr) )
	{
		DEBUG_3("illegal 601 read_words - slot=%x, seg=%x, addr=%x\n", 
                                          ctl->bus_slot,ctl->bus_segreg, addr);
		return (E_DEVACCESS);
	}

	local_values = (void *)malloc( len );

	if ( local_values == NULL )
	{
		return( E_MALLOC );
	}

	ctl->mdd.md_data		= (char * ) local_values;
	ctl->mdd.md_size		= 1;
	ctl->mdd.md_incr		= MV_WORD;
	address = addr;
	for (i = 0 ; i < ( len/sizeof(int) ) ; i++)
	{
		ctl->mdd.md_addr	= address;
	
		rc = IOCTLX( 	ctl->fd,
				MIOBUSGET,
				& (ctl->mdd),
				ctl->bus_segreg	);
	
		if ( rc != 0 )
		{
			return (E_DEVACCESS);
		}
		ctl->mdd.md_data +=4;
		address +=4;
	}
	memcpy( pval, local_values, len );
	free (local_values);
	return (0);
}

/*----------------------------------------------
|
|	FUNCTION DECLARATION
|
|-----------------------------------------------*/

STATIC int
nvram_write_byte(
		FRS_access_t *		ctl,
		ulong			addr,
		uchar 			val
	       )
{
	int			rc;
	uchar			local_value = val;


	ctl->mdd.md_addr		= addr;
	ctl->mdd.md_data		= (char * ) &local_value;
	ctl->mdd.md_size		= 1;
	ctl->mdd.md_incr		= MV_BYTE;

	rc = IOCTLX( 	ctl->fd,
			MIOBUSPUT,
			& (ctl->mdd),
			ctl->bus_segreg	);

	if ( rc != 0 )
	{
		return (E_DEVACCESS);
	}

	return (0);
}
	
/*----------------------------------------------
|
|	FUNCTION DECLARATION
|
|-----------------------------------------------*/

STATIC int
nvram_iocc_write_byte(
		FRS_access_t *		ctl,
		ulong			addr,
		uchar 			val
	       )
{
	int			rc;
	uchar			local_value = val;


	ctl->mdd.md_addr		= addr;
	ctl->mdd.md_data		= (char * ) &local_value;
	ctl->mdd.md_size		= 1;
	ctl->mdd.md_incr		= MV_BYTE;

	rc = IOCTLX( 	ctl->fd,
			MIOBUSPUT,
			& (ctl->mdd),
			ctl->iocc_segreg	);

	if ( rc != 0 )
	{
		return (E_DEVACCESS);
	}

	return (0);
}
	

/*----------------------------------------------
|
|      Write 1 to POS2 - bit0 - to enable
|      Micro-channel adapter at the given slot
|
|-----------------------------------------------*/
int 
enable_device(p_ctl)

   FRS_access_t *          p_ctl;

{
	int iocc_addr, rc;
	unsigned char val;

	/* 
	   calculate POS2 in iocc space for the given slot 
	   By iocc rule, only use logical slot which is
	   equal to physical slot minus one.
	*/ 

	iocc_addr = POSREG(2,p_ctl->bus_slot) + CDD_IOCC_BASE;   

	rc =  nvram_iocc_read_byte(p_ctl, iocc_addr, &val) ;

	if ( rc == 0)
	{
		#define CARD_ENABLE	0x1

		rc =  nvram_iocc_write_byte(p_ctl, iocc_addr, (val | CARD_ENABLE) ) ;
	}

	return (rc);
 		
}

/*----------------------------------------------
|
|      Write 0 to POS2 - bit0 - to disable
|      Micro-channel adapter at the given slot
|
|-----------------------------------------------*/

int 
disable_device(p_ctl)

   FRS_access_t *          p_ctl;

{
        int iocc_addr, rc;
	unsigned char val;

        /*
           calculate POS2 in iocc space for the given slot
	   By iocc rule, only use logical slot which is
	   equal to physical slot minus one.
        */

        iocc_addr = POSREG(2,p_ctl->bus_slot) + CDD_IOCC_BASE;

	rc =  nvram_iocc_read_byte(p_ctl, iocc_addr, &val) ;

	if ( rc == 0 )
	{
        	#define CARD_DISABLE     (~ (0x1) )

		val = (unsigned char) (val & CARD_DISABLE);
        	rc =  nvram_iocc_write_byte(p_ctl, iocc_addr, val) ;
	}

        return (rc);

}

/*----------------------------------------------
|
|	FUNCTION DECLARATION
|
|-----------------------------------------------*/

STATIC int
nvram_write_word(
		FRS_access_t *		ctl,
		ulong			addr,
		ulong			val
	       )
{
	int			rc;
	ulong			local_value = val;


	ctl->mdd.md_addr		= addr;
	ctl->mdd.md_data		= (char * ) &local_value;
	ctl->mdd.md_size		= 1;
	ctl->mdd.md_incr		= MV_WORD;

	rc = IOCTLX( 	ctl->fd,
			MIOBUSPUT,
			& (ctl->mdd),
			ctl->bus_segreg	);

	if ( rc != 0 )
	{
		return (E_DEVACCESS);
	}

	return (0);
}
	
/*----------------------------------------------
|
|	FUNCTION DECLARATION
|
| This function is not a general purpose read bytes function! 
|
| It is mainly used to read the Feature ROM.  On 601 device, 
| it uses ioctl MIOVPGGET provided by the machine driver
|
| Currently MIOVPDGET only works on power pc and it only
| reads the ROM in bytes.   
|
|-----------------------------------------------*/

STATIC int
nvram_read_bytes(
		FRS_access_t *		ctl,
		ulong			addr,
		uchar *			pval,
		ulong			len
	       )
{
	int			rc;
	uchar *			local_values;

	if ( ADDR_IS_60x_VPD_FRS_SPACE(ctl->bus_segreg, addr) )
	{
		rc = rd_60x_vpd_feat_rom (ctl->bus_slot, addr, pval, len);

		if (rc)
		{
			DEBUG_3("601 read_bytes:failed - slot=%x, seg=%x, addr=%x\n",
                                                     ctl->bus_slot, ctl->bus_segreg,addr);

			return (E_DEVACCESS);
		}

		return (0);

	}
	else if ( ADDR_IS_60x_CFG_SPACE(ctl->bus_segreg, addr) )
	{
		DEBUG_3("illegal 601 read_bytes - slot=%x, seg=%x, addr=%x\n", 
                                              ctl->bus_slot, ctl->bus_segreg, addr);

		return (E_DEVACCESS);
	}

	local_values = (uchar *)malloc( len );

	if ( local_values == NULL )
	{
		return( E_MALLOC );
	}

	ctl->mdd.md_addr		= addr;
	ctl->mdd.md_data		= (char * ) local_values;
	ctl->mdd.md_size		= len;
	ctl->mdd.md_incr		= MV_BYTE;

	rc = IOCTLX( 	ctl->fd,
			MIOBUSGET,
			& (ctl->mdd),
			ctl->bus_segreg	);

	if ( rc != 0 )
	{
		return (E_DEVACCESS);
	}

	memcpy( pval, local_values, len );
	free (local_values);

	return (0);
}



/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME:	 	FRS_checksum( )
  
 TITLE:	 	Function to detect whether a Feature ROM Scan 
		RSCAN_PS2 structure in memory has a valid checksum
  
 ------------------------------------------------------------------------	
 
 FUNCTION: 	
		Initializes local pointers.
		Computes span of full size of header including linked list.
		Allocates local storage for full span.
		Reads in the full header and romscan blocks.
		Computes the checksum. 
		Returns the result.
  
 EXECUTION ENVIRONMENT: User process environment only
  
 INPUTS:
		bus_head	- holds the bus address of the RSCAN_PS2 structure

		mem_head	- holds the mem address of the copy of the RSCAN_PS2 structure

		ctl		- holds an FRS control structure pointer

 RETURNS:
 		an integer return code stating whether an error occurred or not. 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/
	
/*----------------------------------------------
|
|	FUNCTION DECLARATION
|
|-----------------------------------------------*/

STATIC int
FRS_checksum(
		FRS_access_t *	ctl,
		RSCAN_PS2 *	bus_head,
		RSCAN_PS2 *	mem_head,
		int 		(* fp) () 
	    )
{
/*-----------------------------------------------
|
|	DATA DECLARATIONS    
|
|-----------------------------------------------*/

int			rc;
uchar			checksum;
char *			buffer;
size_t			head_size;
int			i, j;

/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/


	/*------------------------------------
	| set up a buffer the size of the ROM
	|------------------------------------*/

head_size	= CFG_GRAPH_FRS_BLK_SIZE * RSCAN_PS2_Blength( mem_head );

buffer		= (char *) malloc( head_size );

if ( buffer == NULL ) 	{ return (E_MALLOC); }

	/*-------------------------------------
	| read in the full ROM
	|-------------------------------------*/

rc	= (* fp )( ctl, (uchar *) bus_head, 
				buffer, head_size );

if ( rc != 0 ) 	{ return (E_DEVACCESS); 	}

	/*--------------------------------------
	| do the checksum
	|--------------------------------------*/

checksum = 0;

for( i = 0; 	i < head_size; 		i++) 
{
	checksum += *( buffer + i );
}

	/*-------------------------------------
	| clean up and return results
	|-------------------------------------*/

free( buffer );

if( checksum != 0 )       /** if checksum nonzero then error */
{
	return(E_VPD);
}

return(0);      /** checksum is zero, ROM OK */

}


/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME:	 	FRS_crc16( )
  
 TITLE:	 	Function to detect whether a Feature ROM Scan 
		RSCAN_VIDEO_HEAD structure in memory has a valid CRC residue
  
 ------------------------------------------------------------------------	
  
 FUNCTION:  	
		RISC/6000 ROMs will validated by a 16 bit CRC.  
		The CRC polynomial is 1 + X(5) + X(12) + X(16) 
		with the CRC being initialized to hex 'FFFF'.  
		The CRC is calculated by starting at the fifth 
		byte ( offset 4) in the RISC/6000 ROM.
  
 EXECUTION ENVIRONMENT: User process environment only
  
 INPUTS:
		ctl		- holds an FRS control structure pointer

		buffer		- holds the mem address of the copy of the total
				  VIDEO rom contents
	
 RETURNS:
 		an integer return code stating whether an error occurred or not. 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/
	
/*----------------------------------------------
|
|	FUNCTION DECLARATION
|
|-----------------------------------------------*/

STATIC int
FRS_crc16(
		FRS_access_t *		ctl,
		RSCAN_VIDEO_HEAD *	mem_head
	    )
{
/*-----------------------------------------------
|
|	DATA DECLARATIONS    
|
|-----------------------------------------------*/

int			rc;
int			i;
ushort			rom_Blength;
size_t			rom_size;
ushort			rom_residue;

char *			p_buffer;
char			datav;

#define CRC_MASK 0xff07
#define COMBINE(x, y) (((x) << 8) | (y))

struct bytes 
{
	char msb;
	char lsb;
};

union accum 
{
	ushort whole;
	struct bytes byte;

} 			avalue, dvalue;

/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/

rom_Blength		= RSCAN_VIDEO_HEAD_rom_Blength( mem_head );
rom_size		= CFG_GRAPH_FRS_BLK_SIZE * rom_Blength;

dvalue.whole 		= 0xffff;
avalue.whole 		= 0;

p_buffer		= (char *) mem_head;
	
	/*----------------------------------------------------
	| process the CRC 
	|
	| start at the 4/th byte
	|
	| result is stored in dvalue.whole upon exit
	|-----------------------------------------------------*/

for ( i=4; 	i < rom_size; 	i++	)
{
	datav 		= *(p_buffer+i);

	avalue.byte.lsb = (datav ^ dvalue.byte.lsb);
	dvalue.byte.lsb = avalue.byte.lsb;
	avalue.whole 	= ((avalue.whole * 16) ^ dvalue.byte.lsb);
	dvalue.byte.lsb = avalue.byte.lsb;
	avalue.whole 	<<= 8;

	avalue.whole 	>>= 1;
	avalue.byte.lsb ^= dvalue.byte.lsb;

	avalue.whole 	>>= 4;

	avalue.whole 	= COMBINE(avalue.byte.lsb, avalue.byte.msb);
	avalue.whole 	= ((avalue.whole & CRC_MASK) ^ dvalue.byte.lsb);
	avalue.whole 	= COMBINE(avalue.byte.lsb, avalue.byte.msb);
	avalue.byte.lsb ^= dvalue.byte.msb;
	dvalue.whole 	= avalue.whole;
}

	/*------------------------------------------------------------
	| test the residue against what is stored in the ROM
	|------------------------------------------------------------*/

rom_residue		= RSCAN_VIDEO_HEAD_residue( mem_head );

if ( rom_residue != dvalue.whole )
{
	return ( E_VPD );
}

return (0);
}



/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME:	 	FRS_Validate_RSCAN_VIDEO_Head( )
  
 TITLE:	 	Function to detect whether a Feature ROM Scan 
		RSCAN_VIDEO_HEAD structure sits at a particular address.
  
 ------------------------------------------------------------------------	
 
 FUNCTION: 	
		Initialize local pointers.
		Read in the RSCAN_VIDEO_HEAD structure.
		From it, get the block length and compute total size.
		Allocate a buffer for the total size.
		Read in the full VIDEO rom.
		Perform the CRC test on the memory copy.
		If OK, then update pointers. 
  
 EXECUTION ENVIRONMENT: User process environment only
  
 INPUTS:
		ctl		- holds a structure of type FRS_access_t for use
				  in accessing the data via the correct bus
				  (ctl->bus_segreg is used to access the segment)
				  
				  The ctl->bus_rscan_r2_head member holds the address
				  of the header to be tested.

		valid		- a boolean stating whether the address is
				  a valid RSCAN_PS2 block
  
 RETURNS:
		mallocs space for the RSCAN_VIDEO_HEAD contents in the ctl structure
		The space holds the full ROM contents.

 		returns an integer return code stating whether an error occurred or not. 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/
	
/*----------------------------------------------
|
|	FUNCTION DECLARATION
|
|-----------------------------------------------*/

STATIC int
FRS_Validate_RSCAN_VIDEO_HEAD(
	
		FRS_access_t *		ctl,		/* standard ctl structure */
		ulong *			valid,		/* boolean return var */
		ulong 		 	buid_type	
	 		     )
{
/*-----------------------------------------------
|
|	DATA DECLARATIONS    
|
|-----------------------------------------------*/

int			rc;
char *			buffer;
RSCAN_VIDEO_HEAD *	bus_head;
RSCAN_VIDEO_HEAD	mem_head;
ushort			rom_Blength;
size_t			rom_size;
int			(* read_func )() ;
/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/

*valid			= FALSE;

ctl->mem_rscan_r2_head	= NULL;

bus_head		= (RSCAN_VIDEO_HEAD *) ctl->bus_rscan_r2_head;

DEBUG_1("Validate_RSCAN_VIDEO_HEAD: R2 header offset =%x\n",bus_head);

switch (buid_type)
{
	case CFG_GRAPH_BUID_2x:

 		read_func =  ctl->read_bytes;

		break;

	case CFG_GRAPH_BUID_7F:
	case CFG_GRAPH_BUID_40:

 		read_func =  ctl->read_words;

		break;

	default:
		;
}

	/*------------------------------------------------------------
	| read the RSCAN_VIDEO_HEAD into the memory copy
	|-------------------------------------------------------------*/

rc = ( * read_func )( ctl,
			   (uchar *) bus_head,
			   (uchar *) &mem_head,
			   ( RSCAN_VIDEO_HEAD_SIZE ) 	);

DEBUG_1("Validate_RSCAN_VIDEO_HEAD: read R2 header rc =%d\n",rc);

if ( rc != 0 )	{ return (E_DEVACCESS);	}

	/*------------------------------------------------------------
	| query the head, to get the total size of the structure
	|-------------------------------------------------------------*/

rom_Blength		= RSCAN_VIDEO_HEAD_rom_Blength( &mem_head );

rom_size		= CFG_GRAPH_FRS_BLK_SIZE * rom_Blength;

DEBUG_2("Validate_RSCAN_VIDEO_HEAD: len (512) =%d, len =%d\n",rom_Blength, rom_size);

	/*----------------------------------------------------------
	| set up a buffer to read in the total VIDEO rom structures
	| from bus memory
	|-----------------------------------------------------------*/

buffer			= (char *) malloc( rom_size );

if ( buffer == NULL )	{ return (E_MALLOC);	}

	/*------------------------------------------------------------
	| read the total VIDEO rom into the memory buffer
	|-------------------------------------------------------------*/

rc = ( * read_func )( ctl,
			   (uchar *) bus_head,
			   (uchar *) buffer,
			   ( rom_size )	);

DEBUG_1("Validate_RSCAN_VIDEO_HEAD: read total Video ROM rc =%d\n",rc);

if ( rc != 0 )	{ return (E_DEVACCESS);	}

	/*-------------------------------------------------------------
	| perform the CRC test on the VIDEO rom contents
	|--------------------------------------------------------------*/

rc = FRS_crc16( ctl,
	  	(RSCAN_VIDEO_HEAD *) buffer	);

DEBUG_1("Validate_RSCAN_VIDEO_HEAD:  rc =%d from FRS_crc16\n",rc);

if ( rc != 0 )	{ return (rc);	}

	/*--------------------------------------------------------------
	| the ROM is valid
	|--------------------------------------------------------------*/

ctl->mem_rscan_r2_head	= (void *) buffer;

*valid			= TRUE;

return (0);

}


/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME:	 	FRS_Test_Addr_For_RSCAN_PS2( )
  
 TITLE:	 	Function to detect whether a Feature ROM Scan 
		RSCAN_PS2 structure sits at a particular address.
  
 ------------------------------------------------------------------------	
 
 FUNCTION: 	
		Initialize local variables.
		Read in a RSCAN_PS2 header from the bus test address.
		If error on first read, assume that the address is invalid.
		Validate the RSCAN_PS2 header id and flag fields.
		Validate the RSCAN_PS2 checksum.
		Validate the RSCAN_PS2 contents.
		Malloc storage for the contents and save them in the ctl structure.
		Return.	
  
 EXECUTION ENVIRONMENT: User process environment only
  
 INPUTS:
		ctl		- holds a structure of type FRS_access_t for use
				  in accessing the data via the correct bus
				  (ctl->bus_segreg is used to access the segment)

		test_addr	- holds the candidate address

		valid		- holds a boolean stating whether the address is
				  a valid RSCAN_PS2 block
  
 RETURNS:
 		an integer return code stating whether an error occurred or not. 
 
		mallocs block to hold the contents and places it in ctl->p_rscan_head
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/
	
/*----------------------------------------------
|
|	FUNCTION DECLARATION
|
|-----------------------------------------------*/

STATIC int
FRS_Test_Addr_for_RSCAN_PS2(

			FRS_access_t *	ctl,
			ulong		test_addr,
			ulong *		valid,
			ulong		buid_type
			   )
{
/*-----------------------------------------------
|
|	DATA DECLARATIONS    
|
|-----------------------------------------------*/

int			rc;
RSCAN_PS2		head;
ulong			bus_addr;
int			( * read_func ) ();
/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/

bus_addr		= test_addr;

*valid			= FALSE;

ctl->mem_rscan_ps2	= NULL;
ctl->bus_rscan_ps2	= NULL;

	/*--------------------------------------
	| read the header from bus memory into our
	| local copy (head)
	|--------------------------------------*/

switch (buid_type)
{
	case CFG_GRAPH_BUID_2x:

		read_func=  ctl->read_bytes ;

		break;

	case CFG_GRAPH_BUID_7F: 
	case CFG_GRAPH_BUID_40:

		read_func=  ctl->read_words ;

		break;

}

rc	= (* read_func )( ctl, (uchar *) bus_addr, 
				(uchar *) &head, ( RSCAN_PS2_SIZE )	);
if ( rc != 0 )
{
	/*----------------------------------
	| we got an error on the first access 
	| to the adapter.  It was already re-tried
	| by the machine device driver.
	|
	| We assume that there is no Feature ROM,
	| and that the error was an expected DSI.
	|
	| Given this assumption, we conclude that
	| the address is not valid, and we return
	| with a good RC but a valid=FALSE
	|
	|-------------------------------------*/

	return (0);
}

	/*-----------------------------------
	| use a while construct to permit a long
	| series of "if this" and "if that" and 
	| break out if one of the required tests fails
	| If we break from loop, rc will hold correct value
	| and *valid will be FALSE
	|------------------------------------*/

rc	= 0;			/* preset the rc */

while ( *valid == FALSE )
{
	/*-------------------------------------------
	| test the individual locations in the header
	|-------------------------------------------*/

	if ( 	( RSCAN_PS2_id_hex55( &head ) != 0x55 )

	     ||	( RSCAN_PS2_id_hexAA( &head ) != 0xAA )

	     || ( RSCAN_PS2_flag( &head ) != RSCAN_DYNAMIC_ROM_FLAG_VAL )
	   )
	{	break;	}

	/*------------------------------------------
	| the structure has the correct flags, so
	| check its contents
	|
	| test the checksum of the header
	|------------------------------------------*/

	rc = FRS_checksum( ctl, bus_addr, &head , read_func);

	if ( rc != 0 ) 	
	{  
		rc = E_VPD;
		break; 	
	}

	/*-----------------------------------------
	| test for non-NULL pointers
	|-----------------------------------------*/

	if (     ( RSCAN_PS2_dynamic_rom_length( &head ) == 0 )

	     ||  ( RSCAN_PS2_dynamic_rom_head( &head ) == 0 )
	   )
	{  
		rc = E_VPD;
		break; 	
	}

	/*-------------------------------------------
	| the header is valid
	| allocate permanent storage for the copy
	| and copy it from the local storage
	| into the ctl structure.  Update the ctl pointers.
	| mark the result as valid.  return.
	|-------------------------------------------*/
	
	ctl->mem_rscan_ps2 = (void *) malloc( RSCAN_PS2_SIZE );
	
	if ( ctl->mem_rscan_ps2 == NULL ) { return (E_MALLOC);	}

	memcpy( (char *) ctl->mem_rscan_ps2 , (char *) &head , RSCAN_PS2_SIZE );

	DEBUG_1("FRS_Test_Addr_for_RSCAN_PS2: found PS2 at %x\n",bus_addr);

	ctl->bus_rscan_ps2 = bus_addr;

	*valid 	= TRUE;
}

return (rc);

}


/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME:	 	FRS_Enable_BUID_2x_Device( )
  
 TITLE:	 	Function to enable a BUID 2X Device to read its 
		RSCAN_R2_HEAD structure
  
 ------------------------------------------------------------------------	
 
 FUNCTION: 	
  
 EXECUTION ENVIRONMENT: User process environment only
  
 INPUTS:
		ctl		- holds a structure of type FRS_access_t for use
				  in accessing the data via the correct bus
				  (ctl->bus_segreg is used to access the segment)

		bus_romblk	- bus address of the previously found RSCAN_6001_BLK 

		mem_romblk	- mem address of copy of RSCAN_6001_BLK 

 RETURNS:
 		an integer return code stating whether an error occurred or not. 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/
	
/*----------------------------------------------
|
|	FUNCTION DECLARATION
|
|-----------------------------------------------*/

STATIC int
FRS_Enable_BUID_2x_Device(
			FRS_access_t *		ctl,		/* standard ctl structure */
			RSCAN_6001_BLK *	bus_romblk,	/* bus address */
			RSCAN_6001_BLK *	mem_romblk	/* already in memory */
	 		 )
{
/*-----------------------------------------------
|
|	DATA DECLARATIONS    
|
|-----------------------------------------------*/

int			rc;
int			i;
int			num_vals;
ulong			iocc_addr;
ulong			pos_val;

/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/

	/*---------------------------------------
	| loop through the set of POS values, 
	| writing them one at a time to the adapter
	|---------------------------------------*/

num_vals	=  RSCAN_6001_BLK_num_pos_vals( mem_romblk );

if ( num_vals == 0 )	{ return (0);	}	/* nothing to do */
	
for ( i=0;   i < num_vals;    i++ )
{
	pos_val		= RSCAN_6001_BLK_pos_val( mem_romblk, i );

	iocc_addr	= POSREG( RSCAN_6001_BLK_pos_reg( mem_romblk, i ),
				  ctl->bus_slot ) |  CDD_IOCC_BASE ;

	rc = ( * ctl->iocc_write_byte )( ctl,
					iocc_addr,
					pos_val		);

	if ( rc != 0 )	{ return (E_DEVACCESS);	}
}

	/*--------------------------------------
	| all done
	|--------------------------------------*/

return (0);

}


/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME:	 	FRS_Enable_BUID_40_Device( )
  
 TITLE:	 	Function to enable a BUID 40 Device to read its 
		RSCAN_R2_HEAD structure
  
 ------------------------------------------------------------------------	
 
 FUNCTION: 	
		Returns 0.  There is no setup for BUID 40
  
 EXECUTION ENVIRONMENT: User process environment only
  
 INPUTS:
		ctl		- holds a structure of type FRS_access_t for use
				  in accessing the data via the correct bus
				  (ctl->bus_segreg is used to access the segment)
	
		bus_romblk	- holds address in bus space of RSCAN_6002_BLK

		mem_romblk	- holds address of mem copy of RSCAN_6002_BLK

 RETURNS:
 		an integer return code stating whether an error occurred or not. 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/
	
/*----------------------------------------------
|
|	FUNCTION DECLARATION
|
|-----------------------------------------------*/

STATIC int
FRS_Enable_BUID_40_Device(
			FRS_access_t *		ctl,		/* standard ctl structure */
			RSCAN_6002_BLK *	bus_romblk,	/* address on the bus */
			RSCAN_6001_BLK *	mem_romblk	/* already in memory */
	 		 )
{
/*-----------------------------------------------
|
|	DATA DECLARATIONS    
|
|-----------------------------------------------*/

int			rc;
int			num_vals;

/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/

	/*---------------------------------------
	| loop through the set of cfg_register values, 
	| writing them one at a time to the adapter
	|---------------------------------------*/

num_vals	=  RSCAN_6001_BLK_num_pos_vals( mem_romblk );

if   ( num_vals == 0 )	{ return (0);	}	/* nothing to do */
else			{ return (0);	}	/* BUID 40 needs no setup */

}

	
/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME:	 	FRS_Search_for_RSCAN_6001_BLK( )
  
 TITLE:	 	Function to detect whether a Feature ROM Scan 
		RSCAN_6001_BLK structure sits at a particular address.
  
 ------------------------------------------------------------------------	
 
 FUNCTION: 	
		Initialize pointers with addresses of RSCAN_PS2 head.
		Validate the rom block header.
		Loop through linked list of rom blocks:
			Compute the correct bus address of the block.
			Read the block into memory.
			Validate the block class and id string.
			If valid,
				Validate the block VIDEO rom address.
				Enable the adapter to read VIDEO rom.
				Save the memory copy in the ctl structure.
				Exit
			Else continue
		Exit with proper state.
  
 EXECUTION ENVIRONMENT: User process environment only
  
 INPUTS:
		ctl		- holds a structure of type FRS_access_t for use
				  in accessing the data via the correct bus
				  (ctl->bus_segreg is used to access the segment)

		valid		- a boolean stating whether the address is
				  a valid RSCAN_PS2 block
  
 RETURNS:
 		returns an integer return code stating whether an error occurred or not. 
 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/

	
/*----------------------------------------------
|
|	FUNCTION DECLARATION
|
|-----------------------------------------------*/

STATIC int
FRS_Search_for_RSCAN_6001_BLK(
	
		FRS_access_t *		ctl,		/* standard ctl structure */
		ulong *			valid		/* boolean return var */
	 		     )
{
	
/*-----------------------------------------------
|
|	DATA DECLARATIONS    
|
|-----------------------------------------------*/

int			rc;
RSCAN_PS2 *		head;
RSCAN_PS2 *		bus_head;
int			romblk_offset;
int			romblk_next_offset;
int			romblk_max_offset;
RSCAN_6001_BLK *	bus_romblk;
RSCAN_6001_BLK  	mem_romblk;
ushort			blk_class;
ulong			this_block_not_ours;

/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/

*valid			= FALSE;

head			= (RSCAN_PS2 *) ctl->mem_rscan_ps2;
bus_head		= (RSCAN_PS2 *) ctl->bus_rscan_ps2;

ctl->mem_rscan_blk	= NULL;
ctl->bus_rscan_blk	= NULL;

	/*---------------------------------------
	| Get the first address of the linked list of
	| ROMSCAN_ROMBLK structures
	| Make sure it is valid.
	|---------------------------------------*/
	
romblk_offset		= (int) RSCAN_PS2_dynamic_rom_head( head );
romblk_next_offset	= 0;

if ( romblk_offset == 0 )  { return (E_VPD); }

romblk_max_offset	= romblk_offset + (int) RSCAN_PS2_dynamic_rom_length( head );

	/*---------------------------------------------------------
	| walk the list of ROMBLKS
	| 
	| the exit condition is based upon the FRS block links
	|----------------------------------------------------------*/

for  (	/* no initial conditions */ ;

	/* exit conditions */
        (    (romblk_offset < romblk_max_offset )
          && (romblk_offset != 0 )  );

	/* iteration terms */
	romblk_offset = romblk_next_offset
     )
{
	this_block_not_ours = FALSE;	/* preset */

		/*---------------------------------------
		| compute the bus address of the block
		| 	- offsets are always relative to start
		|	  of the RSCAN_PS2 head
		|----------------------------------------*/

	bus_romblk	= (RSCAN_6001_BLK *) 
			  ( ((char *) bus_head ) + romblk_offset );

		/*---------------------------------------
		| read in the next block from bus memory
		|----------------------------------------*/

	/* rc = ( * ctl->read_words )( ctl, */
	rc = ( * ctl->read_bytes )( ctl,
				   (uchar *) bus_romblk,
				   (uchar *) &mem_romblk,
				   ( RSCAN_6001_BLK_SIZE ) );

	if ( rc != 0 )	{ return (E_DEVACCESS);	}

		/*--------------------------------------
		| test the copy in memory for validity
		|
		| start with the class value and ID string
		|---------------------------------------*/

	if (    ( RSCAN_6001_BLK_class( &mem_romblk ) != RSCAN_DYNAMIC_ROM_BLOCK_ID_VAL )
	     || ( strncmp( RSCAN_6001_BLK_id( &mem_romblk ),
			   RSCAN_R2_STRING_ID_2,
			   8 ))
	   )
	{	this_block_not_ours = TRUE;	}	/* not one of ours */

		/*----------------------------------------------
		| this block is valid if it has met the above tests.
		| check to see.  If not, then set up pointer to the
		| next block and continue on
		|-----------------------------------------------*/

	if ( this_block_not_ours )
	{
		romblk_next_offset = RSCAN_6001_BLK_next( &mem_romblk );
		continue;
	}

		/*--------------------------------------
		| class and string match
		|
		| check whether a valid pointer to S/6000 ROM
		|--------------------------------------*/

	if (    ( RSCAN_6001_BLK_rom_addr( &mem_romblk ) < CFG_GRAPH_BUID_2x_ROM_LO_ADDR )
	     || ( RSCAN_6001_BLK_rom_addr( &mem_romblk ) > CFG_GRAPH_BUID_2x_ROM_HI_ADDR )
	   )
	{ 	return (E_VPD);	}	/* invalid pointer to ROM */

		/*-------------------------------------
		| this block is valid and has good pointers
		|
		| prepare to read the RSCAN_R2_HEAD structure
		| to do this, we need to process this blk for
		| instructions on how to 
		| enable the hardware for reading the 
		| S/6000 ROM by processing the cfg reg list
		|---------------------------------------*/

	rc = FRS_Enable_BUID_2x_Device(	ctl,
					bus_romblk,
					&mem_romblk	);

	if ( rc != 0 )	{ return (rc);	}	/* failed to enable */

		/*-----------------------------------
		| if we reach here, we have found a valid
		| block.  Malloc storage for this block.
		| Copy the mem_romblk into the malloc area.
		| Update the ctl pointer.  
		| Make sure to set the bus address of the R2_HEAD 
		| into the ctl structure, for use later by others
		|------------------------------------*/

	ctl->mem_rscan_blk	= (void *) malloc( RSCAN_6001_BLK_SIZE );

	if ( ctl->mem_rscan_blk == NULL ) { return (E_MALLOC);	}

	memcpy( (char *) ctl->mem_rscan_blk, (char *) &mem_romblk, RSCAN_6001_BLK_SIZE );

	ctl->bus_rscan_blk	= (void *) bus_romblk;

	ctl->bus_rscan_r2_head	= (void *) RSCAN_6001_BLK_rom_addr( &mem_romblk );

	*valid		= TRUE;
	
	return (0);

} /* end of for loop */

	/*-----------------------------------------------
	| fall through to here means we did not find
	| a valid block
	|-----------------------------------------------*/

*valid	= FALSE;

return (0);

}


/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME:	 	FRS_Search_for_RSCAN_6002_BLK( )
  
 TITLE:	 	Function to detect whether a Feature ROM Scan 
		RSCAN_6002_BLK structure sits at a particular address.
  
 ------------------------------------------------------------------------	
 
 FUNCTION: 	
		Initialize pointers with addresses of RSCAN_PS2 head.
		Validate the rom block header.
		Loop through linked list of rom blocks:
			Compute the correct bus address of the block.
			Read the block into memory.
			Validate the block class and id string.
			If valid,
				Validate the block VIDEO rom address.
				Enable the adapter to read VIDEO rom.
				Save the memory copy in the ctl structure.
				Exit
			Else continue
		Exit with proper state.
		
  
 EXECUTION ENVIRONMENT: User process environment only
  
 INPUTS:
		ctl		- holds a structure of type FRS_access_t for use
				  in accessing the data via the correct bus
				  (ctl->bus_segreg is used to access the segment)

		valid		- a boolean stating whether the address is
				  a valid RSCAN_PS2 block
  
 RETURNS:
 		returns an integer return code stating whether an error occurred or not. 
 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/

	
/*----------------------------------------------
|
|	FUNCTION DECLARATION
|
|-----------------------------------------------*/

STATIC int
FRS_Search_for_RSCAN_6002_BLK(
	
		FRS_access_t *		ctl,		/* standard ctl structure */
		ulong *			valid		/* boolean return var */
	 		     )
{
	
/*-----------------------------------------------
|
|	DATA DECLARATIONS    
|
|-----------------------------------------------*/

int			rc;
RSCAN_PS2 *		head;
RSCAN_PS2 *		bus_head;
int			romblk_offset;
int			romblk_next_offset;
int			romblk_max_offset;
RSCAN_6002_BLK *	bus_romblk;
RSCAN_6002_BLK  	mem_romblk;
ushort			blk_class;
ulong			this_block_not_ours;

/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/

*valid			= FALSE;

head			= (RSCAN_PS2 *) ctl->mem_rscan_ps2;
bus_head		= (RSCAN_PS2 *) ctl->bus_rscan_ps2;

ctl->mem_rscan_blk	= NULL;
ctl->bus_rscan_blk	= NULL;

	/*---------------------------------------
	| Get the first address of the linked list of
	| ROMSCAN_ROMBLK structures
	| Make sure it is valid.
	| Then compute the max offset value
	|---------------------------------------*/
	
romblk_offset		= (int) RSCAN_PS2_dynamic_rom_head( head );
romblk_next_offset	= 0;

if ( romblk_offset == 0 )  { return (E_VPD); }

romblk_max_offset	= romblk_offset + (int) RSCAN_PS2_dynamic_rom_length( head );

	/*---------------------------------------------------------
	| walk the list of ROMBLKS
	| 
	| the exit condition is based upon the FRS block links
	|----------------------------------------------------------*/

for  (	/* no initial conditions */ ;

	/* exit conditions */
        (    (romblk_offset < romblk_max_offset )
          && (romblk_offset != 0 )  );

	/* iteration terms */
	romblk_offset = romblk_next_offset
     )
{
	this_block_not_ours = FALSE;	/* preset */

		/*---------------------------------------
		| compute the bus address of the block
		| 	- offsets are always relative to start
		|	  of the RSCAN_PS2 head
		|----------------------------------------*/

	bus_romblk	= (RSCAN_6002_BLK *) 
			  ( ((char *) bus_head ) + romblk_offset );

		/*---------------------------------------
		| read in the next block from bus memory
		|----------------------------------------*/

	rc = ( * ctl->read_words )( ctl,
				   (uchar *) bus_romblk,
				   (uchar *) &mem_romblk,
				   ( RSCAN_6002_BLK_SIZE ) );

	if ( rc != 0 )	{ return (E_DEVACCESS);	}

		/*--------------------------------------
		| test the copy in memory for validity
		|
		| start with the class value and ID string
		|---------------------------------------*/

	if (    ( RSCAN_6002_BLK_class( &mem_romblk ) != RSCAN_DYNAMIC_ROM_BLOCK_ID_VAL )
	     || ( strncmp( RSCAN_6002_BLK_id( &mem_romblk ),
			   RSCAN_R2_STRING_ID_3,
			   8 ))
	   )
	{	this_block_not_ours = TRUE;	}	/* not one of ours */

		/*----------------------------------------------
		| this block is valid if it has met the above tests.
		| check to see.  If not, then set up pointer to the
		| next block and continue on
		|-----------------------------------------------*/

	if ( this_block_not_ours )
	{
		romblk_next_offset = RSCAN_6002_BLK_next( &mem_romblk );
		continue;
	}

		/*--------------------------------------
		| class and string match
		|
		| check whether a valid pointer to S/6000 ROM
		|--------------------------------------*/

	if (    ( RSCAN_6002_BLK_rom_addr( &mem_romblk ) < CFG_GRAPH_BUID_40_ROM_LO_ADDR )
	     || ( RSCAN_6002_BLK_rom_addr( &mem_romblk ) > CFG_GRAPH_BUID_40_ROM_HI_ADDR )
	   )
	{ 	return (E_VPD);	}	/* invalid pointer to ROM */

		/*-------------------------------------
		| this block is valid and has good pointers
		|
		| prepare to read the RSCAN_R2_HEAD structure
		| to do this, we need to process this blk for
		| instructions on how to 
		| enable the hardware for reading the 
		| S/6000 ROM by processing the cfg reg list
		|---------------------------------------*/

	rc = FRS_Enable_BUID_40_Device(	ctl,
					bus_romblk,
					&mem_romblk	);

	if ( rc != 0 )	{ return (rc);	}	/* failed to enable */

		/*-----------------------------------
		| if we reach here, we have found a valid
		| block.  Malloc storage for this block.
		| Copy the mem_romblk into the malloc area.
		| Update the ctl pointer.  
		| Make sure to set the bus address of the R2_HEAD 
		| into the ctl structure, for use later by others
		|------------------------------------*/

	ctl->mem_rscan_blk	= (void *) malloc( RSCAN_6002_BLK_SIZE );

	if ( ctl->mem_rscan_blk == NULL ) { return (E_MALLOC);	}

	memcpy( (char *) ctl->mem_rscan_blk, (char *) &mem_romblk, RSCAN_6002_BLK_SIZE );

	ctl->bus_rscan_blk	= (void *) bus_romblk;

	ctl->bus_rscan_r2_head	= (void *) RSCAN_6002_BLK_rom_addr( &mem_romblk );

	*valid		= TRUE;
	
	return (0);

} /* end of for loop */

	/*-----------------------------------------------
	| fall through to here means we did not find
	| a valid block
	|-----------------------------------------------*/

*valid	= FALSE;

return (0);

}



/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME:	 	FRS_Search_for_RSCAN_6003_BLK( )
  
 TITLE:	 	Function to detect whether a Feature ROM Scan 
		RSCAN_6003_BLK structure sits at a particular address.
  
 ------------------------------------------------------------------------	
 
 FUNCTION: 	
		Initialize pointers with addresses of RSCAN_PS2 head.
		Validate the rom block header.
		Loop through linked list of rom blocks:
			Compute the correct bus address of the block.
			Read the block into memory.
			Validate the block class and id string.
			If valid,
				Validate the block VIDEO rom address.
				Save the memory copy in the ctl structure.
				Exit
			Else continue
		Exit with proper state.
		
  
 EXECUTION ENVIRONMENT: User process environment only
  
 INPUTS:
		ctl		- holds a structure of type FRS_access_t for use
				  in accessing the data via the correct bus
                                  (ctl->bus_segreg is used by nvram_read_words/bytes to tell if 601)
                                  (ctl->bus_slot is passed to mdd to do the read for us)

		valid		- a boolean stating whether the address is
				  a valid RSCAN_PS2 block
  
 RETURNS:
 		returns an integer return code stating whether an error occurred or not. 
 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/

	
/*----------------------------------------------
|
|	FUNCTION DECLARATION
|
|-----------------------------------------------*/

STATIC int
FRS_Search_for_RSCAN_6003_BLK(                 /* 601_CHANGE : new func */

		FRS_access_t *		ctl,	/* standard ctl structure */
		ulong *			valid	/* boolean return var */
	 		     )
{
	
/*-----------------------------------------------
|
|	DATA DECLARATIONS    
|
|-----------------------------------------------*/

   int			rc;
   RSCAN_PS2 *		head;
   RSCAN_PS2 *		bus_head;
   int			romblk_offset;
   int			romblk_next_offset;
   int			romblk_max_offset;
   RSCAN_6003_BLK *	bus_romblk;
   RSCAN_6003_BLK  	mem_romblk;

   ushort		blk_class, word_incr;

   ulong		this_block_not_ours, 
                        dev_characteristic;

   /*-----------------------------------------------
   |
   |	START OF SOURCE CODE
   |
   |-----------------------------------------------*/


        /* --------------------------------------------------------------- 
        |  1, read the device characteristic register 
        |
        |  2, from it examine the word increment value.  If it is 4 then
        |     it is a one-to-one mapping; if it is 8, then every 4 bytes
        |     of ROM are mapped to every 8 bytes of system address space. 
	|     We don't have to deal with this; the machine dd does all the
        |     reading for us;  however, we need to know this to handle
        |     set the limit of where to stop looking for the 6003 block. 
        |
        ---------------------------------------------------------------*/ 

   /* 
    * here we can read a word because this register is not in ROM 
    */
   rc = rd_60x_std_cfg_reg_w( ctl->bus_slot, BUS_60X_DEV_CHAR_REG, 
                                         & dev_characteristic); 

   if ( rc != 0 ) 	
   { 
      DEBUG_2("Search_6003_BLK: can't read dev char, slot=%x, reg=%x\n",
                                            ctl->bus_slot, BUS_60X_DEV_CHAR_REG);
      return (rc); 	
   }

   if ( ((dev_characteristic & RSCAN_60X_DEVCHAR_TYPE_MASK) == RSCAN_60X_DEVCHAR_TYPE_NOT_PRESENT) || 
             ((dev_characteristic & RSCAN_60X_DEVCHAR_TYPE_MASK) != RSCAN_60X_DEVCHAR_TYPE_IO ) 
      )
   {
           DEBUG_0("Search_6003_BLK: *** NO DEVICE PRESENT ****\n");
          return (-1);
   }

   /* 
    * compute word stride factor
    */ 
   if ( RSCAN_60X_BOOL_INCREMENT_1(dev_characteristic) )
   {
           DEBUG_0("Search_6003_BLK: word increment value is 4 bytes\n");
           word_incr = 1;   
   }
   else if ( RSCAN_60X_BOOL_INCREMENT_2(dev_characteristic) )
   {
           DEBUG_0("Search_6003_BLK: word increment value is 8 bytes\n");
           word_incr = 2;  
   }
   else
   {
            DEBUG_0("Search_6003_BLK: unknown word increment value !\n");
   }

   /*
	address of where PS2 ROM Header is in the form of 0xFFA0 0000 + some offset 
        note offset < 0x1FFFFF 
   */
   bus_head		= (RSCAN_PS2 *) ctl->bus_rscan_ps2;

   DEBUG_1("Search_6003_BLK: ctl->bus_rscan_ps2 =%x\n", bus_head);

   *valid = FALSE; 

   head	= (RSCAN_PS2 *) ctl->mem_rscan_ps2;  /* buffer with PS/2 Header */

   ctl->mem_rscan_blk	= NULL;
   ctl->bus_rscan_blk	= NULL;

	/*---------------------------------------
	| Get the first address of the linked list of
	| ROMSCAN_ROMBLK structures
	| Make sure it is valid.
	| Then compute the max offset value
        |
        | *** ASSUME rom block offseet is accounted for 
        |            differnt work incr 
        | ***
        |
	|---------------------------------------*/
	
   /* 
      Offset to to the beginning of ROM blocks in linked list from 
      the PS2 ROM Header
   */
   romblk_offset = (int) RSCAN_PS2_dynamic_rom_head( head );

   DEBUG_1("Search_6003_BLK: PS2 offset = %x\n",bus_head);

   DEBUG_0("Search_6003_BLK: linked list offset must be equal to size of ps2\n");
   DEBUG_1("Search_6003_BLK: linked list offset =%x\n", romblk_offset);
   DEBUG_1("Search_6003_BLK: size of ps2 =%x\n", sizeof(RSCAN_PS2));

   romblk_next_offset	= 0;

   if ( romblk_offset == 0 )  
   { 
      return (E_VPD); 
   }

   if (word_incr == 1)
   {
      romblk_max_offset = romblk_offset + 
                               (int)RSCAN_PS2_dynamic_rom_length(head);
   }
   else
   {
                       			/*--------------------------------- 
                       			|  every 4 bytes of data take up 
                                        |  twice as much sytem address space
                       			-------------------------------- */
      romblk_max_offset = romblk_offset   + 
                     ( ( (int)RSCAN_PS2_dynamic_rom_length(head) / 4 ) +1) * 8 ;
   }


	/*---------------------------------------------------------
	| walk the list of ROMBLKS
	| 
	| the exit condition is based upon the FRS block links
	|----------------------------------------------------------*/

   for  ( ; (romblk_offset < romblk_max_offset ) && (romblk_offset != 0 ) ;
	                                  romblk_offset = romblk_next_offset)
   {
	this_block_not_ours = FALSE;	/* preset */

		/*---------------------------------------
		| compute the bus address of the block
		| 	- offsets are always relative to start
		|	  of the RSCAN_PS2 head
		|----------------------------------------*/

	bus_romblk	= (RSCAN_6003_BLK *) 
			  ( ((char *) bus_head ) + romblk_offset );

	if ( bus_romblk > BUS_60X_END_VPD_FEATURE_ROM)
	{
                /* 
                   should never happen. will take out break point later
                */ 
                DEBUG_0("Search_6003_BLK: **** invalid rom address **** \n");

	        return (E_DEVACCESS);
	}
		/*---------------------------------------
		| read in the next block from bus memory
		|----------------------------------------*/

        DEBUG_1("Search_6003_BLK: for loop.  ioctl rd addr =%x\n", bus_romblk);

	rc = ( * ctl->read_bytes )( ctl,
				   (uchar *) bus_romblk,
				   (uchar *) &mem_romblk,
				   ( RSCAN_6003_BLK_SIZE ) );

        DEBUG_1("Search_6003_BLK: call read_bytes, rc =%x\n", rc);

        DEBUG_2("Search_6003_BLK: 6003 id str =<%s>, blk id =%d\n", 
           RSCAN_6003_BLK_id( &mem_romblk ), RSCAN_6003_BLK_class(&mem_romblk));

	if ( rc != 0 )	{ return (E_DEVACCESS);	}

		/*--------------------------------------
		| test the copy in memory for validity
		|
		| start with the class value and ID string
		|---------------------------------------*/

	if ( (RSCAN_6003_BLK_class(&mem_romblk) != 
                                          RSCAN_DYNAMIC_ROM_BLOCK_ID_VAL)

	     || ( strncmp( RSCAN_6003_BLK_id( &mem_romblk ),
			   RSCAN_R2_STRING_ID_4, 8 ) )
	   )

	{
  		DEBUG_1("Search_6003_BLK: for loop.  not found RISC6003 @a %x\n", bus_romblk);
        	this_block_not_ours = TRUE;	/* not one of ours */
        }

		/*----------------------------------------------
		| this block is valid if it has met the above tests.
		| check to see.  If not, then set up pointer to the
		| next block and continue on
		|-----------------------------------------------*/

	if ( this_block_not_ours )
	{
		romblk_next_offset = RSCAN_6003_BLK_next( &mem_romblk );
		continue;
	}

		/*--------------------------------------
		| class and string match
		|
		| check whether a valid pointer to S/6000 ROM
		|--------------------------------------*/

	if ( 
             (RSCAN_6003_BLK_rom_addr(&mem_romblk) <  BUS_60X_START_VPD_FEATURE_ROM)
          || (RSCAN_6003_BLK_rom_addr(&mem_romblk) > BUS_60X_END_VPD_FEATURE_ROM)

           )
	{
	  	/* invalid pointer to ROM */
           	return (E_VPD);
        }	

  	DEBUG_1("Search_6003_BLK: for loop.  found RISC6003 @a %x\n", bus_romblk);

		/*-----------------------------------
		| if we reach here, we have found a valid
		| block.  Malloc storage for this block.
		| Copy the mem_romblk into the malloc area.
		| Update the ctl pointer.  
		| Make sure to set the bus address of the R2_HEAD 
		| into the ctl structure, for use later by others
		|------------------------------------*/

	ctl->mem_rscan_blk	= (void *) malloc( RSCAN_6003_BLK_SIZE );

	if ( ctl->mem_rscan_blk == NULL ) { return (E_MALLOC);	}

	memcpy( (char *) ctl->mem_rscan_blk, (char *) &mem_romblk,
                                                    RSCAN_6003_BLK_SIZE);

	ctl->bus_rscan_blk	= (void *) bus_romblk;

        /* 
         * This is the absolute address of where the video rom is.
         */ 

	ctl->bus_rscan_r2_head	= (int)RSCAN_6003_BLK_rom_addr(&mem_romblk) ;

	*valid		= TRUE;
	
	return (0);

   } /* for loop */

	/*-----------------------------------------------
	| fall through to here means we did not find
	| a valid block
	|-----------------------------------------------*/

   *valid	= FALSE;

   return (0);

}




/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME:	 	FRS_Find_Video_Device( )
  
 TITLE:	 	Function to detect whether a Feature ROM Scan Device exists
		on a particular bus at a particular subaddress.
  
 ------------------------------------------------------------------------	
 
 FUNCTION: 	
		Writes NULL to the return data pointer.
		Sets up pointers to function in the ctl structure.
		Sets up state flags in the ctl structure.

		Does the pre-processing phase:

			If Microchannel, 
		             Opens the machine device driver.
                             saves input subaddresses in ctl structure.
			If buid40, 
		             Opens the machine device driver.
                             initializes ctl structure.

			If buid7F, 
                             initializes ctl structure (bus segment and slot number fields)

		Attempts to locate a RSCAN_PS2 header on the adapter.

			If Microchannel, 
                             loops through a range of possible address points, looking for 
                             first valid occurrence.

			If buid40, 
                             checks a hard-coded address.

			If buid7F, 

                             read device characteristic at 0xFF20 0000.  From this figure out
                             the word increment (also named delta or i)

                             VPD and FRS begin at 0xFFA0 0000 - so called the ROM BASE

                             read ROM characteristic at BASE + 1 * i, to get number of ROM objects -
                             (right now there are 2, VPD and Feature ROM (more in the future)
                             Each object are described by 3 words, its type, offset from BASE and its
                             length 
			    
                             Read the object array (length = number of object * 4 bytes) beginning at
                             BASE + 3 * i

                             Search this object array for block id (lower 16 bits of type field) equal 2 (FRS)
                             If found, use offset to look for PS2 ROM header.

		Attempts to locate a RSCAN_600x_BLK on the adapter.

			If Microchannel, looks for 6001.

			If buid40, looks for 6002.

			If buid7F, looks for 6003.

		Attempts to locate a RSCAN_VIDEO_HEAD header on the adapter.

			Same for all buses.  If found, copies in the full
			structure for later use by caller.

		Writes back pointer to copy of VIDEO rom and returns.
  
 EXECUTION ENVIRONMENT: User process environment only
  
 INPUTS:
		buid_type	- a flag stating the type of bus being searched
				  at present only Microchannel, BUID=40 and BUID=7F are 
				  supported

		p_video_head	- address of pointer which will hold the location
				  of the RSCAN_R2_HEAD structure.  The contents are assumed
			  	  to have been malloc-ed by this routine, and should
				  be freed by the calling routine

		subaddr_1	- if buid_type == Microchannel or 7F (601 - Rain Bow Box)
				  then holds the bus memory segment reg contents

				  else ignored

		subaddr_2	- if buid_type == Microchannel 
				  then holds the iocc segment register contents

				  else ignored

		subaddr_3	- if buid_type == Microchannel or 7F (601 - Rain Bow Box)
				  then holds the Microchannel/601 slot number

				  else ignored

 RETURNS:
 		an integer return code stating whether an error occurred or not. 

		E_NODETECT	could not locate/validate some piece of FRS structures
		E_OPEN		could not open a file or device driver
		E_MALLOC	failed to malloc somewhere
		E_BADATTR	invalid input to some routine
		E_DEVACCESS	one of the machine device driver calls failed
		E_VPD		CRC, checksum, or contents of FRS structure was not good
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/

	
/*----------------------------------------------
|
|	FUNCTION DECLARATION
|
|-----------------------------------------------*/

int
FRS_Find_Video_Device(
		int			buid_type,
		void **			p_video_head,
		ulong			subaddr_1,
		ulong			subaddr_2,
		ulong			subaddr_3
		     )
{
	
/*-----------------------------------------------
|
|	DATA DECLARATIONS    
|
|-----------------------------------------------*/

FRS_access_t		ctl;
int			rc;
int			scan_address;
void *			block;
ulong			valid;

RSCAN_ROMBLK *		mem_romblk;
RSCAN_VIDEO_HEAD *	mem_head;


ulong                   addr,                   /* 601_CHANGE */
                        dev_characteristic;   

rscan_60x_romchar_t        rom_char;          
char *                     ptr;
rscan32_60x_obj_info_t *   p_obj_info_buf;

unsigned int              i, nbytes, len,
                          delta,
                          num_objects,
                          ps2_offset;   /* offset from 0xFFA0 0000 to PS2 Header blk */

/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/

*p_video_head		= (void *) NULL;

	/*--------------------------------------
	| set up pointers to function for machine
	| device driver access
	|---------------------------------------*/

ctl.open		= nvram_open;
ctl.close		= nvram_close;
ctl.read_byte		= nvram_read_byte;
ctl.read_bytes		= nvram_read_bytes;
ctl.read_word		= nvram_read_word;
ctl.read_words		= nvram_read_words;
ctl.write_byte		= nvram_write_byte;
ctl.write_word		= nvram_write_word;
ctl.iocc_read_byte	= nvram_iocc_read_byte;
ctl.iocc_write_byte	= nvram_iocc_write_byte;

	/*---------------------------------------
	| preset the FRS detection variables
	|---------------------------------------*/

ctl.frs_adapter_present	= -1;
ctl.frs_detected_error	= -1;
ctl.frs_adapter_bad	= -1;


	/*--------------------------------------
	| Pre-Processing Phase
	|
	| Set up Segment Registers
	| Set up read/write functions
	|
	|--------------------------------------*/

switch (buid_type)
{

case CFG_GRAPH_BUID_2x:	

	/*--------------------------------------
	| open /dev/nvram (machine device driver)
	|--------------------------------------*/

	rc = ( * ctl.open )( &ctl );

	if ( rc != 0 ) 	
        { 
        	return (E_OPEN); 	
	}

	ctl.bus_segreg	= subaddr_1;
	ctl.iocc_segreg	= subaddr_2;

	/* 
	 *   physical slot is passed in. 
         *
         *   To access iocc space (POS), need to use logical slot which is 
         *   one less than that 
         */ 

	ctl.bus_slot = subaddr_3 - 1;   

	/*  
	   In order to read the FRS ROM, the
	   card has to be enabled by setting 
	   bit 0 of POS2 to 1
	*/
	rc = enable_device(&ctl);

	if ( rc != 0 ) 	
        { 
        	return (rc); 	
	}

	break;


case CFG_GRAPH_BUID_40:

	/*--------------------------------------
	| open /dev/nvram (machine device driver)
	|--------------------------------------*/

	rc = ( * ctl.open )( &ctl );

	if ( rc != 0 ) 	
        { 
        	return (E_OPEN); 	
	}

	ctl.bus_segreg	= CFG_GRAPH_BUID_40_SEG;

	break;

case CFG_GRAPH_BUID_7F:                          /* 601_CHANGE */

        /*
         *  Don't need to open /dev/nvram because the read/write
         *  functions for 60x does its own open (see 60x_tools.c)
         */

        ctl.bus_segreg  = subaddr_1;          /* used by nvram_read_words/bytes to tell 601 h/w */

        ctl.bus_slot    = subaddr_3;          /* need to pass it to machine driver */

        break;

default:

	return (E_BADATTR);

} /* end of switch */

	/*-------------------------------------------------------------------
	| Locate a Valid RSCAN_PS2 header
	|
	| The output of each case should either return
	| if no head was found, or it should set valid=TRUE
	|
	| if valid, then the ctl structure holds:
	|	ctl.bus_rscan_ps2	- bus address of structure
	|	ctl.mem_rscan_ps2	- mem address of copy of structure
	|
	|--------------------------------------------------------------------*/

valid = FALSE;

switch (buid_type)
{

case CFG_GRAPH_BUID_2x:

		/*-------------------------------------------
		| Microchannel adapters can place the ROM within
		| ranges of addresses, aligned on 2K boundaries.
		| We need to search the all possible ranges to find a
		| header
		|---------------------------------------------*/

	for (  	scan_address	=  CFG_GRAPH_BUID_2x_PS2_HEADER_START_ADDR1;
		scan_address	<  CFG_GRAPH_BUID_2x_PS2_HEADER_STOP_ADDR1;
		scan_address	+= CFG_GRAPH_BUID_2x_ADDR_INCR		)
	{
		rc = FRS_Test_Addr_for_RSCAN_PS2( &ctl,
						  scan_address,
						  &valid, buid_type);
		if ( valid  || (rc != 0) ) 	{ break; 	}
	}

	/* 
         * if we have not found the PS2 header, scan the 2nd range 
         */
	if ( !valid  ) 	
	{ 

		for (  	scan_address	=  CFG_GRAPH_BUID_2x_PS2_HEADER_START_ADDR2;
			scan_address	<  CFG_GRAPH_BUID_2x_PS2_HEADER_STOP_ADDR2;
			scan_address	+= CFG_GRAPH_BUID_2x_ADDR_INCR		)
		{
			rc = FRS_Test_Addr_for_RSCAN_PS2( &ctl,
						  scan_address,
						  &valid, buid_type);
			if ( valid  || (rc != 0) ) 	{ break; 	}
		}
	}

	if   ( (! valid)  || rc) 
	{ 

	       /*  
       		   disable the card by
		   setting bit 0 of POS2 to 0
       		*/
       	 	disable_device(&ctl);

		if (!valid)	
		{
			rc = E_NODETECT;
		}
		return (rc);		
	}

	break;	 
	
case CFG_GRAPH_BUID_40:

	scan_address = CFG_GRAPH_BUID_40_START_ADDR;

	rc = FRS_Test_Addr_for_RSCAN_PS2( &ctl,
					  scan_address,
					  &valid, buid_type);

	if ( rc != 0 ) 	{ return (rc); 		}
	if ( ! valid ) 	{ return (E_NODETECT);	}

	break;	 


case CFG_GRAPH_BUID_7F:                          /* 601_CHANGE */

        /* --------------------------------------------------------------- 
        |  1, read the device characteristic register 
        |
        |   - which tells us how to address each byte of the ROM and whether 
        |     the ROM can be read in words or bytes.  Byte access is permitted 
        |     in both cases.  To keep it simple (i.e., not handle word access 
        |     when H/W is capable of), mdd always choose byte access.
        |
        |  2, read VPD/Feature ROM characteristic register 
        |
        |   - which tells us how many objects the ROM contains.
        |
        |  3, read the offset, length and type fields (3 words) for each object
        |
        |   - search the object array for the feature rom object,  If found
        |    use the offset to find the PS2 ROM header.
        |
        -----------------------------------------------------------------*/ 

        /* here we can read a word because this register is not in ROM */
        rc = rd_60x_std_cfg_reg_w( ctl.bus_slot, BUS_60X_DEV_CHAR_REG, 
                                         & dev_characteristic); 

        DEBUG_1("cfg_graphics_frs_tools.c:Find_Video_Device: dev char =%x\n", dev_characteristic);

	if ( rc != 0 ) 	
        { 
           DEBUG_0("cfg_graphics_frs_tools.c:Find_Video_Device: can't read dev char\n");
           return (E_DEVACCESS); 	
	}

     	if ( ((dev_characteristic & RSCAN_60X_DEVCHAR_TYPE_MASK) == RSCAN_60X_DEVCHAR_TYPE_NOT_PRESENT) || 
             ((dev_characteristic & RSCAN_60X_DEVCHAR_TYPE_MASK) != RSCAN_60X_DEVCHAR_TYPE_IO ) 
           )
        {
           DEBUG_0("cfg_graphics_frs_tools.c:Find_Video_Device:  Device not present or none IO type\n");
           return (E_DEVACCESS); 	
        }

        /* compute word stride factor */ 
        if ( RSCAN_60X_BOOL_INCREMENT_1(dev_characteristic) )
        {
           DEBUG_0("cfg_graphics_frs_tools.c:Find_Video_Device: word increment value is 4 bytes\n");
           delta = 4;   
        }
        else if ( RSCAN_60X_BOOL_INCREMENT_2(dev_characteristic) )
        {
           DEBUG_0("cfg_graphics_frs_tools.c:Find_Video_Device: word increment value is 8 bytes\n");
           delta = 8;  
        }
	else
        {
            DEBUG_0("cfg_graphics_frs_tools.c:Find_Video_Device: unknown word increment value !\n");
           return (E_DEVACCESS); 	
        }


        /* 
           ROM Characteristic Register is at 0xFFA0 0000 + 1 * delta 
        */
	addr = BUS_60X_START_VPD_FEATURE_ROM +  delta;

        rc = rd_60x_vpd_feat_rom (ctl.bus_slot, addr, &rom_char, sizeof(rom_char));

	if ( rc != 0 ) 	
        { 
           DEBUG_0("cfg_graphics_frs_tools.c:Find_Video_Device: can't read ROM char\n");
           return (E_DEVACCESS); 	
        }

        /* 
           extract the number of objects contain in ROM from the ROM 
           characteristic register.
        */
        num_objects = rom_char.sub.num_items;

        DEBUG_1("cfg_graphics_frs_tools.c:Find_Video_Device: number of objects =%d\n", num_objects);

        if (num_objects == 0)       /* ROM contains only VPD data */
        {
           return (E_NODETECT);	
        }

        /* allocate space for the ROM objects array */

        nbytes =  sizeof(rscan32_60x_obj_info_t) * num_objects;
        ptr = (char *) malloc(nbytes);

        /* 
           Here read the Feature/VPD information array which begins at
           0xFFA0 0000 + 3 * delta
        */

	addr = BUS_60X_START_VPD_FEATURE_ROM +  3 * delta;

        rc = rd_60x_vpd_feat_rom (ctl.bus_slot, addr, ptr, nbytes);

        p_obj_info_buf = (rscan32_60x_obj_info_t *) ptr;

        ps2_offset = 0;        /* offset from 0xFFA0 0000  to PS2 ROM header */
        len        = 0;       

        for (i=0 ; i < num_objects ; i ++)
        {

          /* looking for Feature ROM object in the array */

          if (p_obj_info_buf->type.sub.id == RSCAN_60X_ROM_FRS) 
          {
             ps2_offset = p_obj_info_buf->offset;
             len    = p_obj_info_buf->length;      /* what is this len for ? */
          }

          p_obj_info_buf ++;    /* point to next object in array */

        }

        if (! ps2_offset)        /* if no Feature ROM */ 
	{ 
           DEBUG_0("cfg_graphics_frs_tools.c:Find_Video_Device: No Feature ROM object found !\n");
           return (E_NODETECT);	
        }

	/* 
         * ROM creater makes sure this offset is from 0xFFA0 0000 and 
	 * accounts for word increment too
         */ 
        scan_address =  ps2_offset + BUS_60X_START_VPD_FEATURE_ROM;
                                         
        DEBUG_1("cfg_graphics_frs_tools.c:Find_Video_Device: found FRS obj, ps2 offset =%x\n",ps2_offset);

        /* 
           Note we set ctl.read_words to the address of nvram_60x_rd_bytes... 
           so it should read the rom in bytes in stead of words
        */
	rc = FRS_Test_Addr_for_RSCAN_PS2( &ctl,
					  scan_address,
					  &valid, buid_type);

	DEBUG_3("cfg_graphics_frs_tools.c:Find_Video_Device: Test_Addr_for_RSCAN_PS2 returns rc = %d, valid = %d, PS2 at %x \n", rc,valid,ctl.bus_rscan_ps2);

	if ( rc != 0 ) 	{ return (rc); 		}
	if ( ! valid ) 	{ return (E_NODETECT);	}


	break;	 

default:
	/* should never get here */

	/*
	assert( 0 );
	*/
	break;

} /* end of switch */

	/*--------------------------------------------------------
	| We have a vaild RSCAN_PS2 header
	| Now locate a valid RSCAN_600X_BLK structure
	|
	| Walk the linked list of dynamic blocks
	| (Starting at the head) and find the
	| first one that matches our needs
	| If the search routine finds a valid block, it will also
	| enable the adapter to have the block read
	|
	| if valid, then the ctl structure holds:
	|	ctl.bus_rscan_blk	- bus address of structure
	|	ctl.mem_rscan_blk	- mem address of copy of structure
	|	ctl.bus_rscan_r2_head	- bus address of RSCAN_VIDEO_HEAD
	|	ctl.mem_rscan_r2_head	- invalid (NULL)
	|
	| also, if valid, the device will be enabled for reading the
	| Feature ROM Scan contents
	|
	|--------------------------------------------------------*/

valid = FALSE;

switch (buid_type)
{

case CFG_GRAPH_BUID_2x:

	rc = FRS_Search_for_RSCAN_6001_BLK(	&ctl,
						&valid	);

	if ( rc  || ( ! valid ) ) 
	{ 
               /*
                   disable the card by
                   setting bit 0 of POS2 to 0
                */
                disable_device(&ctl);

		if ( ! valid )
		{ 
			rc = E_NODETECT;	
		}
		return (rc); 		
	}

	break;

case CFG_GRAPH_BUID_40:
	
	rc = FRS_Search_for_RSCAN_6002_BLK(	&ctl,
						&valid 	);
	if ( rc != 0 ) 	{ return (rc); 		}
	if ( ! valid ) 	{ return (E_NODETECT);	}

	break;
	
case CFG_GRAPH_BUID_7F:                  /* 601_CHANGE */


        rc = FRS_Search_for_RSCAN_6003_BLK(     &ctl,
                                                &valid  );

       DEBUG_2("cfg_graphics_frs_tools.c:Find_Video_Device: FRS_Search_for_6003_BLK returns rc = %d, valid = %d \n", rc,valid);

        if ( rc != 0 )  { return (rc);          }
        if ( ! valid )  { return (E_NODETECT);  }

        break;

default:
	/* should never get here */
	
	/*
	assert( 0 );
	*/
	break;
} /* end of switch */

	/*--------------------------------------------------------
	| We have a vaild RSCAN_PS2 header and a valid RSCAN_600X_BLK structure
	|
	| Now validate the RSCAN_VIDEO_HEAD
	|
	| if valid, then the ctl structure holds:
	|	ctl.mem_rscan_r2_head	- mem address of copy of VIDEO rom
	|
	| also, if valid, the device will be enabled for reading the
	| Feature ROM Scan contents
	|
	|--------------------------------------------------------*/

valid 	= FALSE;

rc	= FRS_Validate_RSCAN_VIDEO_HEAD(  &ctl,
					  &valid, buid_type );

DEBUG_2("cfg_graphics_frs_tools.c:Find_Video_Device: FRS_Validate_RSCAN_VIDEO_HEAD returns rc = %d, valid = %d \n", rc,valid);


	/*------------------------------------------------------
	| 
	| disable the card by
	| setting bit 0 of POS2 to 0
	|------------------------------------------------------*/

if ( buid_type == CFG_GRAPH_BUID_2x)
{
	disable_device(&ctl);
}

if ( rc != 0 )  { return (rc);          }
if ( ! valid )  { return (E_NODETECT);  }



	/*------------------------------------------------------
	| 
	| Everything is valid.
	|
	| release the malloc-ed structures that we no longer need
	|------------------------------------------------------*/

if ( ctl.mem_rscan_ps2 != NULL )	{ free( ctl.mem_rscan_ps2 );	}
if ( ctl.mem_rscan_blk != NULL )	{ free( ctl.mem_rscan_blk );	}


	/*------------------------------------------------------
	| pass back the pointer to the VIDEO rom
	| 
	| DON'T free up this one.  Caller gets to use it!!
	|-------------------------------------------------------*/

*p_video_head	= (void *) ctl.mem_rscan_r2_head;

return(0);

}

	

/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME:	 	FRS_Make_Temp_File( )
  
 TITLE:	 	Function to create a temporary file that will be used later
		to transform an FRS module into something usefile by AIX
  
 ------------------------------------------------------------------------	
 
 FUNCTION: 	
  
 EXECUTION ENVIRONMENT: User process environment only
  
 INPUTS:
		p_video_head	- address of pointer which holds the location
				  of the RSCAN_R2_HEAD structure.  This pointer was
				  created by FRS_Find_Video_Device( )

		frs_module	- id of which FRS module to extract and place into 
				  the temporary file

		file_perms	- the permissions desired for the file

		file_name	- address of pointer which will hold the filename

 RETURNS:
 		an integer return code stating whether an error occurred or not. 

		E_NODETECT	could not locate/validate some piece of FRS structures
		E_OPEN		could not open a file or device driver
		E_MALLOC	failed to malloc somewhere
		E_BADATTR	invalid input to some routine
		E_DEVACCESS	one of the machine device driver calls failed
		E_VPD		CRC, checksum, or contents of FRS structure was not good
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/

	
/*----------------------------------------------
|
|	FUNCTION DECLARATION
|
|-----------------------------------------------*/

int
FRS_Make_Temp_File(
		void *			p_video_head,
		int			frs_module,
		int			file_perms,
		char **			file_name
		  )
{
/*-----------------------------------------------
|
|	DATA DECLARATIONS    
|
|-----------------------------------------------*/

int			rc;
RSCAN_VIDEO_HEAD *	mem_head;
char *			buffer;
ulong			buffer_offset;
size_t			buffer_length;
struct stat		s_stat;
int			fd;

/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/

mem_head		= (RSCAN_VIDEO_HEAD *) p_video_head;

	/*-------------------------------------
	| validate input parameters
	|-------------------------------------*/

if  ( mem_head == NULL ) {	return (E_BADATTR);	}
if  ( file_perms == 0  ) { 	return (E_BADATTR);	}

	/*--------------------------------------
	| Create the temp file name
	|--------------------------------------*/

(void) tmpnam( file_name );

	/*-------------------------------------
	| Check if the file already exists.  
	| If it does, unlink it
	|--------------------------------------*/

rc = stat( file_name, &s_stat );

if ( rc == 0 )
{
		/*--- file exists ---*/
	rc = unlink( file_name );

	if ( rc != 0 )	{	return (E_STAT);	}
}

	/*---------------------------------------
	| Now create the new file
	|---------------------------------------*/

fd = creat( file_name, file_perms );

if ( fd < 0 )		{	return (E_STAT);	}

	/*---------------------------------------
	| all exits after this point should close the
	| file and check the file_name for validity!
	|----------------------------------------*/

	/*--------------------------------------
	| use the frs_module to read in the correct
	| item from the RSCAN_VIDEO_HEAD rom
	|
	| the following modules are supported:
	|
	|	CFG_GRAPH_ODM_FILE
	|	CFG_GRAPH_METHOD_FILE
	|	CFG_GRAPH_CDD_FILE
	|	CFG_GRAPH_CDD_PIN_FILE
	|	CFG_GRAPH_UCODE_FILE
	|
	|--------------------------------------*/

switch (frs_module)
{

case CFG_GRAPH_ODM_FILE:

	buffer_offset	= RSCAN_VIDEO_HEAD_odm_offset( mem_head );
	buffer_length	= RSCAN_VIDEO_HEAD_odm_len( mem_head );
	break;

case CFG_GRAPH_UCODE_FILE:

	buffer_offset	= RSCAN_VIDEO_HEAD_ucode_offset( mem_head );
	buffer_length	= RSCAN_VIDEO_HEAD_ucode_len( mem_head );
	break;

case CFG_GRAPH_METHOD_FILE:

	buffer_offset	= RSCAN_VIDEO_HEAD_kmod_offset( mem_head, VRS_KMOD_CFG_METHOD );
	buffer_length	= RSCAN_VIDEO_HEAD_kmod_len( mem_head, VRS_KMOD_CFG_METHOD );
	break;

case CFG_GRAPH_CDD_FILE:

	buffer_offset	= RSCAN_VIDEO_HEAD_kmod_offset( mem_head, VRS_KMOD_ENTRY_POINT );
	buffer_length	= RSCAN_VIDEO_HEAD_kmod_len( mem_head, VRS_KMOD_ENTRY_POINT );
	break;

case CFG_GRAPH_CDD_PIN_FILE:

	buffer_offset	= RSCAN_VIDEO_HEAD_kmod_offset( mem_head, VRS_KMOD_INTERRUPT );
	buffer_length	= RSCAN_VIDEO_HEAD_kmod_len( mem_head, VRS_KMOD_INTERRUPT );
	break;

default:
	(void) close( fd );
	(void) unlink( *file_name );
	*file_name = NULL;
	return (E_BADATTR);
}

	/*---------------------------------------------
	| was there really a kmod in the ROM copy?
	|----------------------------------------------*/

if (     ( buffer_offset == 0 )
      || ( buffer_length == 0 )
   )
{
		/*-------------------------
		| NO.  Clean up and exit
		|--------------------------*/

	(void) close( fd );
	(void) unlink( *file_name );
	*file_name = NULL;
	return (E_NODETECT);
}

	/*-------------------------------------------------
	| move the kmod into the file
	|-------------------------------------------------*/

buffer 	= ((char *) mem_head) + buffer_offset;

rc 	= write( fd, buffer, buffer_length );

if ( rc != buffer_length )	
{
		/*-------------------------
		| Clean up and exit
		|--------------------------*/

	(void) close( fd );
	(void) unlink( *file_name );
	*file_name = NULL;
	return (E_STAT);
}

rc	= fsync( fd );

if ( rc != 0 )
{
		/*-------------------------
		| Clean up and exit
		|--------------------------*/

	(void) close( fd );
	(void) unlink( *file_name );
	*file_name = NULL;
	return (E_STAT);
}

	/*--------------------------------------------------
	| Everything is OK
	|--------------------------------------------------*/

rc = close( fd );

if ( rc != 0 )
{
		/*-------------------------
		| Clean up and exit
		|--------------------------*/

	return (E_STAT);
}
	
return(0);

}


/*
 * NAME: FRS_Update_ODM_From_File()
 *
 * FUNCTION:
 *
 *        This routine opens a file containing the odm stanzas and
 *        builds the appropriate struct for that stanza.  It then queries 
 *        the database for entries of the same type.  If the entry is
 *        not in the data base it is added.  If the entry in the database
 *        is different it is removed and the new entry is added, otherwase, 
 *        no action is taken.
 *
 * EXECUTION ENVIRONMENT:
 *
 *         The objects come from a file 
 *
 * RETURNS:                                                              .
 *
 *        Returns a 0 if successful, a number greater than 0 if not
 */


/*----------------------------------------------
|
|	FUNCTION DECLARATION
|
|-----------------------------------------------*/

int 
FRS_Update_ODM_From_File(char *		stanza_input_file,
			 char **	uniquetype,
			 char **	method_file)
{
/*-----------------------------------------------
|
|	DATA DECLARATIONS    
|
|-----------------------------------------------*/

	/*-------------------------------
	| external data
	|--------------------------------*/

extern int 	odmerrno;

	/*-------------------------------
	| data for this function
	|-------------------------------*/

FILE *		file_ptr;

int 		stanza_length;
int 		skip_spaces;
int 		number_of_values;
int 		descriptor_index;
int 		returnstatus;

unsigned int  	offset;

char 		terminating_char;
char 		sstring[255];

char *		stanza;
char *		class_name;
char *		descrip_name;
char *		descrip_value;
char *		new_entry;
char *		old_entry;
char *		descriptor_offset;

CLASS_SYMBOL 	Class;
CLASS_SYMBOL 	return_class;

char **		vchar_location;  /* ptr to vchar location in the structure */
char *		first_err;

struct PdAt * 	p_PdAt;

struct PdDv * 	p_PdDv;

struct PdCn * 	p_PdCn;

/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/

	/*---------------------------------------------------------
	| Initialiaze some data
	|--------------------------------------------------------*/

stanza_line_number = 0;
**method_file = '\0';
**uniquetype = '\0';


if (odm_initialize () == FAIL)
{
	/* initialization failed */
        DEBUG_0 ("FRS_Update_ODM_From_File: odm_initialize() failed\n");
       return (odmerrno);
}

	/*---------------------------------------------------------
	| open the input file
	|---------------------------------------------------------*/

file_ptr = fopen(stanza_input_file,"r");
	
if (file_ptr == NULL )
{
	return ODMI_OPEN_ERR;
}

	/*--------------------------------------------------------
	| loop through the file contents
	|--------------------------------------------------------*/

while ( (stanza_length = get_ascii_phrase(file_ptr,STANZA,&stanza)) > 0)
{
	DEBUG_1("FRS_Update_ODM_From_File: stanza length is %d\n",stanza_length);

	skip_spaces = FALSE;

        /*--------------------------------------------------*/
        /* Get the first value from the phrase.  Subsequent */
        /* calls will be made with the value 'NULL' for     */
        /* 'ascii_string' since the 'get_value' routine     */
        /* keeps track of where it was last.                */
        /*--------------------------------------------------*/

        class_name = get_value_from_string(stanza,":\n",skip_spaces,
					&terminating_char);

        if (class_name == (char *) NULL || terminating_char != ':')
	{
		/*--------------------------------*/
		/* Could not find the class name. */
		/*--------------------------------*/

		DEBUG_0("FRS_Update_ODM_From_File: could not find class name!\n");
            
	} /* endif */

        DEBUG_1("FRS_Update_ODM_From_File: class name %s\n",class_name);

	if(    ( (Class=odm_mount_class(class_name))==NULL ) 
	    || ( Class == (CLASS_SYMBOL) -1 )
	  )
	{
		DEBUG_1("FRS_Update_ODM_From_File: could not open class %s\n",class_name);
		fclose( file_ptr );
		return(ODMI_INVALID_CLASS);

	}
        else
	{
		skip_spaces = TRUE;

		return_class = (CLASS_SYMBOL) odm_open_class(Class);

		if ((int) return_class == FAIL)
		{
			DEBUG_1("FRS_Update_ODM_From_File: Could not open class! err %d\n",odmerrno);
			fclose( file_ptr );
			return(ODMI_OPEN_ERR);
		} /* endif */

		new_entry = (char *) malloc(Class->structsize);

		if (new_entry == (char *) NULL)
		{
			DEBUG_1("FRS_Update_ODM_From_File: new_entry malloc failed! %d\n", Class->structsize);
			fclose( file_ptr );
			return(ODMI_MALLOC_ERR);
		} /* endif */

		old_entry = (char *) malloc(Class->structsize);

		if (old_entry == (char *) NULL)
		{
			DEBUG_1("FRS_Update_ODM_From_File: old_entry malloc failed! %d\n", Class->structsize);
			(void) free( new_entry );
			fclose( file_ptr );
			return(ODMI_MALLOC_ERR);
		} /* endif */

		bzero(new_entry,Class->structsize);
		bzero(old_entry,Class->structsize);

		/*------------------------------------------------------------*/
		/* Since the variable length char (vchars) are stored in a    */
		/* separate buffer rather than in the structure itself,       */
		/* go through the class info and malloc space for the vchars */
		/*------------------------------------------------------------*/

		for (	descriptor_index = 0; 
			descriptor_index < Class->nelem;
		     	descriptor_index++	)
		{

			if  ((Class->elem)[descriptor_index].type == ODM_VCHAR)
			{

				vchar_location = (char **)
					(new_entry + (Class->elem)[descriptor_index].offset);

				*vchar_location = (char *) NULL;
			} /* endif */
		} /* endfor */

		while (TRUE)
		{
			/*-------------------------------------------*/
			/* Find the name of the next descriptor      */
			/*-------------------------------------------*/

			DEBUG_0("obj_find_string: looking for stanza descrip NAME\n");

			descrip_name = get_value_from_string(	(char *) NULL,"=\n",
								skip_spaces,
								&terminating_char);

			if (    ( descrip_name == (char *) NULL)
			     || ((*descrip_name == '\0' && terminating_char == '\0')))
			{
				/*---------------------------*/
				/* Found the end-of-stanza.  */
				/*---------------------------*/
				DEBUG_0("obj_find_string: found_stanza_end!!\n");
				break;
			}
			else if (terminating_char != '=')
			{
				/*---------------------*/
				/* free(descrip_name); */
				/*---------------------*/
				DEBUG_0("FRS_Update_ODM_From_File: could not find name!!\n");
				(void) free( new_entry );
				(void) free( old_entry );
				fclose( file_ptr );
				return(E_OPEN);
			}
			else
			{

				DEBUG_1("FRS_Update_ODM_From_File: descriptor NAME is %s\n",descrip_name);
			} /* endif */


			/*-------------------------------------------*/
			/* Determine the offset for this descriptor. */
			/*-------------------------------------------*/

			for (	descriptor_index = 0; 
				descriptor_index < Class->nelem;
				descriptor_index++   )
			{

				if (0 == strcmp( (Class->elem)[descriptor_index].elemname,
                                                 descrip_name) )
				{
					/*-------------*/
					/* Found name! */
					/*-------------*/
					DEBUG_1("FRS_Update_ODM_From_File: found table name %d\n",descriptor_index);
					break;

				}
			} /* endfor */

			if (descriptor_index >= Class->nelem)
			{
				DEBUG_1("FRS_Update_ODM_From_File: could not find name %s\n",descrip_name);
				(void) free( new_entry );
				(void) free( old_entry );
				fclose( file_ptr );
				return(E_OPEN);
			} /* endif */

			/*-----------------------------------*/
			/* Get the value for this descriptor */
			/*-----------------------------------*/



			descrip_value = get_value_from_string((char *) NULL,"\n",
						skip_spaces, &terminating_char);

			if (descrip_value == (char *) NULL)
			{
				/*-----------------------------------------------*/
				/* Could not find the value for this descriptor. */
				/*-----------------------------------------------*/

				(void) free( new_entry );
				(void) free( old_entry );
				fclose( file_ptr );
				return(E_OPEN);

			} /* endif */

			DEBUG_1("FRS_Update_ODM_From_File: Descriptor value is %s\n",descrip_value);

			/*-------------------------------------------------------*/
			/* We now have the descriptor name and its value.  Store */
			/* it in the structure.                                  */
			/*-------------------------------------------------------*/


			descriptor_offset = new_entry + Class->elem[descriptor_index].offset;

			switch ( (Class->elem)[descriptor_index].type )
			{
			case ODM_LINK:

				/* skip pointers */
				descriptor_offset = descriptor_offset + 2 * sizeof (char *);

				/*-----------------------------------*/
				/* Fall through and save the string. */
				/*-----------------------------------*/

			case ODM_CHAR:
			case ODM_LONGCHAR:
			case ODM_METHOD:

				*descriptor_offset = '\0';

				strncat( descriptor_offset,descrip_value,
					(Class->elem)[descriptor_index].size - 1);
	
				DEBUG_1("FRS_Update_ODM_From_File: storing character %s\n", descriptor_offset);
				break;

			case ODM_VCHAR:

				/*------------------------------------------------*/
				/* Since the vchars are not put directly into the */
				/* structure, we need to save the string in the   */
				/* buffer pointed to by descriptor_offset.        */
				/*------------------------------------------------*/
	
				vchar_location =  (char **) descriptor_offset;
	
				if (*vchar_location != NULL)
				{
					/*-----------------------------------------------*/
					/* When *vchar_location != NULL occurs, this     */
					/* means that the user has two lines in the      */
					/* stanza for the same value.  Free the previous */
					/* buffer and malloc a new buffer so we only     */
					/* keep the last value.                          */
					/*-----------------------------------------------*/
					free(*vchar_location);
					*vchar_location = NULL;
				} /* endif */
	
				*vchar_location = (char *) malloc(strlen(descrip_value) + 1);
	
				if (*vchar_location == NULL)
				{
					DEBUG_1("FRS_Update_ODM_From_File: vchar malloc failed! %d\n",
					strlen(descrip_value) + 1);
					(void) free( new_entry );
					(void) free( old_entry );
					fclose( file_ptr );
		
					return(ODMI_MALLOC_ERR);
				} /* endif */
	
				strcpy(*vchar_location,descrip_value);
	
				DEBUG_1("FRS_Update_ODM_From_File: storing character %s\n", descrip_value);
				break;

			case ODM_BINARY:

				DEBUG_1("FRS_Update_ODM_From_File: converting hex %s\n",descrip_value);
	
				returnstatus = convert_to_binary(descrip_value,
								descriptor_offset,
								(Class->elem)[descriptor_index].size);
				if (returnstatus < 0)
				{
					DEBUG_0("FRS_Update_ODM_From_File: could not convert hex\n");
					(void) free( new_entry );
					(void) free( old_entry );
					fclose( file_ptr );
					return(E_OPEN);
				} /* endif */
	
				break;

			case ODM_LONG:

				*(long *)descriptor_offset= strtol(descrip_value,&first_err,NULL);

				if(*first_err)
				{
					DEBUG_1("FRS_Update_ODM_From_File: bad long value %s\n", descrip_value);
					(void) free( new_entry );
					(void) free( old_entry );
					fclose( file_ptr );
					return(E_OPEN);
				} /* endif */
	
				DEBUG_1("FRS_Update_ODM_From_File: storing long %ld\n", *(long *)descriptor_offset);
				break;

			case ODM_DOUBLE:

				*(double *)descriptor_offset= strtod(descrip_value,&first_err);

				if (*first_err)
				{
					DEBUG_1("FRS_Update_ODM_From_File: bad double value %s\n", descrip_value);
					(void) free( new_entry );
					(void) free( old_entry );
					fclose( file_ptr );
					return(E_OPEN);
				} /* endif */
	
				DEBUG_1("FRS_Update_ODM_From_File: storing long %ld\n", *(long *)descriptor_offset);
				break;

			case ODM_SHORT:

				*(short *)descriptor_offset= (short) strtol(descrip_value,&first_err,NULL);
				if (*first_err)
				{
					DEBUG_1("FRS_Update_ODM_From_File: bad short value %s\n", descrip_value);
					(void) free( new_entry );
					(void) free( old_entry );
					fclose( file_ptr );
					return(E_OPEN);
				} /* endif */
				
				DEBUG_1("FRS_Update_ODM_From_File: storing short %d\n", *(short *)descriptor_offset);
				break;

			default:

				DEBUG_2("FRS_Update_ODM_From_File: bad type!! %s, type %d",
					(Class->elem)[descriptor_index].elemname,
					(Class->elem)[descriptor_index].type);
				(void) free( new_entry );
				(void) free( old_entry );
				fclose( file_ptr );
			
				return(E_OPEN);

			}


			number_of_values++;
			
			/*----------------------------------------*/
			/*           if (descrip_value)           */
			/*                   free(descrip_value); */
			/*                                        */
			/*           if (descrip_name)            */
			/*                   free(descrip_name);  */
			/*----------------------------------------*/

		} /* endwhile */

		/*---------------------------------------------------
		| Setup the search criterion
		|--------------------------------------------------*/
	
		if ( ( strncmp(Class->classname, "PdDv", 4) ) == 0 )
		{
			 p_PdDv = (struct PdDv *) new_entry;
			 offset = sizeof (p_PdDv->_id) + 
				  sizeof (p_PdDv->_reserved) +
				  sizeof (p_PdDv->_scratch);
			 sprintf(sstring, "type = '%s'", p_PdDv->type);
			 strcpy( *uniquetype, p_PdDv->uniquetype );
	
		} 
		else if ( ( strncmp(Class->classname, "PdCn", 4) ) == 0 )
		{
			 p_PdCn = (struct PdCn *) new_entry;
			 offset = sizeof (p_PdCn->_id) + 
				  sizeof (p_PdCn->_reserved) +
				  sizeof (p_PdCn->_scratch);
			 /* sprintf(sstring, "uniquetype = '%s'", p_PdCn->uniquetype); */
 			sprintf(sstring, "uniquetype = '%s' and connkey = '%s'", p_PdCn->uniquetype,p_PdCn->connkey);
		} 
		else if ( ( strncmp(Class->classname, "PdAt", 4) ) == 0 )
		{
			 p_PdAt = (struct PdAt *) new_entry;
			 offset = sizeof (p_PdAt->_id) + 
				  sizeof (p_PdAt->_reserved) +
				  sizeof (p_PdAt->_scratch);
			 sprintf(sstring, "uniquetype = '%s'and attribute = '%s'", 
				 p_PdAt->uniquetype,p_PdAt->attribute );
			if ( strncmp ( p_PdAt->attribute, "cfg_method_load", 15) == 0 )
			{
				strcpy( *method_file, p_PdAt->deflt );
			}
                        if ( !strncmp ( p_PdAt->attribute, "belongs_to", 10 ) )
                        {
                              /* if we detect "belongs_to=hft0" we need to remove */
                              /* it and replace it with "belongs_to=graphics" for */
                              /* AIX 4.1 LFT which replaces the older HFT.        */

                              if ( !strncmp ( p_PdAt->deflt, "hft0", 4 ) ) {
                                 DEBUG_0("Found hft0 in ODM database\n")
                                 strcpy( p_PdAt->deflt, "graphics" );
                                 /* delete "hft0" stinky from ODM and add "graphics" */
                                 returnstatus = odm_rm_obj ( Class, sstring ); 
                                 DEBUG_1("deleting belongs_to=hft0 returned %d\n",returnstatus)
                                 }
                        }
		} 
		else
		{
			(void) free( new_entry );
			(void) free( old_entry );
			fclose( file_ptr );
			return ODMI_INVALID_CLASS;
		}


		returnstatus = odm_get_obj ( Class, sstring, old_entry, ODM_FIRST);
	
		if ( returnstatus == NULL ) 
		{
			if ((returnstatus = odm_add_obj(Class,new_entry)) < 0)
			{
					(void) free( new_entry );
					(void) free( old_entry );
					fclose( file_ptr );
					return(E_VPD);
			}
		}
		else if ( memcmp((new_entry + offset),
				 (old_entry + offset),
				  ((Class->structsize) - offset ) ) != 0)
		{
			returnstatus = odm_rm_obj ( Class, sstring );

			if ( returnstatus != FAIL )
			{
			   odm_add_obj(Class,new_entry);
			}

		}
		free(new_entry);
		free(old_entry);
		
		/*------------------------------------------------------------*/
		/* Since the variable length char (vchars) are stored in a    */
		/* separate buffer rather than in the structure itself,       */
		/* go through the class info and free   space for the vchars */
		/*------------------------------------------------------------*/
		
		for (descriptor_index = 0; descriptor_index < Class->nelem;
		descriptor_index++)
		{
		
			if  ((Class->elem)[descriptor_index].type == ODM_VCHAR)
			{
				vchar_location = (char **)
						(new_entry + (Class->elem)[descriptor_index].offset);
				
				if (*vchar_location != NULL)
				{
					free (*vchar_location);
					*vchar_location = NULL;
				}
			} /* endif */
		} /* endfor */
	} /* if OpenClass */

	/*--------------------------------*/
	/* if (stanza != (char *) NULL)   */
	/*                  free(stanza); */
	/*--------------------------------*/


} /* endwhile */

/*------------------------------------*/
/* returnstatus = close_class(Class); */
/*------------------------------------*/

if (stanza_length < 0)
{
	DEBUG_1("FRS_Update_ODM_From_File: Stanza length is negative!! %d\n",stanza_length);
	fclose( file_ptr );
	return(E_VPD);
} /* endif */

fclose( file_ptr );
return(0);

}  /* end  FRS_Update_ODM_From_File()*/







int
FRS_Copy_File(char * s_pathname, char * d_pathname)
{


/*----------------------------------------------
|
|	DATA DECLARATIONS
|
|-----------------------------------------------*/

char *		p_buffer;

FILE * 		source_desc;
FILE * 		destination_desc;

int 		num_bytes;
int 		total_bytes;
int 		block_size;

struct stat     s_stat;

/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/

	/*-------------------------------------------------
        |
        |  First test that the supplied source pathname is valid
        |
        |----------------------------------------------------*/

        if(( s_pathname == NULL ) || (( stat( s_pathname, &s_stat )) != 0 ))
	{		
                /*-------------------------------------------
                | the source_path is invalid
                |------------------------------------------*/

                return( E_OPEN );
        }

	/*-------------------------------------------------
	| malloc enough memory to read in the file, if not
	| more than 40kB, otherwise malloc 40K
	|------------------------------------------------*/

	block_size = MIN (s_stat.st_size, MEM_BLOCK_40K) ;
	p_buffer = malloc ( block_size ); 

	if ( p_buffer == NULL )
	{
		return( E_MALLOC );
	}


	/*-------------------------------------------------
	| Open the source and destination files 
	| then read and write the file 
	| then close them up
	|------------------------------------------------*/

	if ( ( source_desc = fopen ( s_pathname, "r") ) == NULL )
	{
		free ( p_buffer );
		return E_OPEN ;
	}

	if ( ( destination_desc = fopen ( d_pathname, "w") ) == NULL )
	{
		free ( p_buffer );
		return E_OPEN ;
	}


	/*---------------------------------------------------------
	| Read the file and check that we got it all
	|--------------------------------------------------------*/

	num_bytes = 0;
	total_bytes = 0;

	do
	{
		num_bytes = fread(p_buffer, 
				   1, 
				   block_size, 
				   source_desc); 

		/*---------------------------------------------------------
		| Write the file and check that we got it all
		|--------------------------------------------------------*/

		if ( (fwrite(p_buffer, 1, num_bytes, destination_desc) ) != s_stat.st_size )
		{
			free (p_buffer);
			return E_OPEN; 
		}
		total_bytes += num_bytes;
	}
	while ( total_bytes < s_stat.st_size );
	
	/*----------------------------------------------------------
	| Free up the memory we used and close files 
	|---------------------------------------------------------*/

	free ( p_buffer );

	fclose ( source_desc) ;
	fclose ( destination_desc);
	
	return PASS;
}

