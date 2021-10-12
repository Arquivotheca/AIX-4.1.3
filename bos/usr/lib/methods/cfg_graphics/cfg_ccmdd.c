static char sccsid[] = "@(#)43	1.14  src/bos/usr/lib/methods/cfg_graphics/cfg_ccmdd.c, dispcfg, bos411, 9439A411b 9/26/94 20:12:26";
/*
 *   COMPONENT_NAME: DISPCFG
 *
 *   FUNCTIONS: ccm_build_dds
 *		ccm_download_ucode
 *		ccm_generate_minor
 *		ccm_query_vpd
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
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
  
		The interface between this problem-state process and the
		AIX device drivers it requires are generally found
		in the AIX system call "sysconfig( )".  There are some
		front end tools in "cfgraphics.c" and in "graphicstools.c"
		which assist in the management of the sysconfig( ) call.
  
		This routine provides the standard functions to build a
		define-device-structure and to query the VPD.  It also
		has device specific code in it, to decide which type of
		GTy adapter has been installed on the bus.  It updates
		the customized attributes accordingly.
  
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

				The module uses a "call by name" interface
				to access the other required modules.

	cfg_graphics_tools.o	A set of device independent tools that support
				the cfg_graphics programs but are useful to
				other configuration methods and to device
				dependent mehtods as well.

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

	cfgXXXX.o		For device XXXX, there must be one or more
				device specific objects.  The recommended
				practise is to have the "call by name"
				entry points in cfgXXXX.o and to have the
				device specific subroutines in a separate
				file.

	In the system makefile environment, the following environment
	variable is useful:

	GRAPHBASE =	cfg_graphics.o cfg_graphics_tools.o	\
			chkslot.o cfgtools.o cfg_ccmdd.o

  
  
 *****************************************************************************
 ********************** MODULE HEADER BOTTOM *********************************
 *****************************************************************************/




/*==========================================================================
|
|  Define some useful constants
|
|==========================================================================*/


#define PASS 0
#define FAIL -1


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


/*
 *  cfg_graphics.c and iplcb.h both use GLOBAL_DATA and GLOBAL_DATA_PTR names.  So
 *  ours have to be renamed to __GLOBAL_DATA and __GLOBAL_DATA_PTR (see 96820) 
 */ 

#include <sys/iplcb.h>		        /* BUC info from ipl control block */

#include <sys/systemcfg.h>	        /* get processor type ( __power_xx macros) */

/*==========================================================================
|
|  Local debug routines are defined here       
|
|==========================================================================*/

#include "cfgdebug.h"






/*==========================================================================
|
|  Include files which describe the common character mode interface and
|  the video ROM scan interface
|
|==========================================================================*/

#define Boolean unsigned int		/* need by aixfont.h		*/
#define Bool    unsigned int		/* need by aixfont.h		*/

#include <sys/aixfont.h>
#include <sys/dir.h>                    /* needed by ccm_dds.h          */
#include <sys/mdio.h>                   /* needed by cdd_macros.h       */
#include <sys/file.h>                   /* needed by ccm.h              */
#include <sys/intr.h>                   /* needed by vt.h               */

#include <sys/display.h>
#include "vt.h"

#include  "cdd.h"
#include  "cdd_macros.h"
#include  "cdd_intr.h"
#include  "cdd_intr_macros.h"
#include  "ccm.h"
#include  "ccm_macros.h"


/*==========================================================================
|
| include a file which defines the cfg macros used by our cfg methods
|
|==========================================================================*/

#include "cfg_graphics.h"
#include "cfg_graphics_macros.h"


/*==========================================================================
|
|  Define function references				 
|
|==========================================================================*/

/*--- from libc.a ---*/
extern 	int		getopt( );
extern 	void 		exit( );
extern 	int 		strcmp( );
extern 	int 		atoi( );
extern 	long 		strtol( );


/*--- from libcfg.a ---*/
extern 	int      	busresolve ();



/*--- from cfg_graphics_tools.o	----*/
extern int 		Get_Custom_Object( );
extern int 		Get_Predef_Object( );




/*---- from ddstools.o ----*/
extern	int		get_conn( );
extern	int		get_attrval( );
extern	int		get_instance( );



/*--- from chkslot.o ----*/
extern int      	chkslot ();



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
extern int 		odm_initialize ( );
extern int 		odm_lock( );
extern void 		setleds( );


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


extern	cfg_graphics_data_t *		__GLOBAL_DATA_PTR;









/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME: 	ccm_build_dds	
  
 TITLE: 		Builds a the common character mode define device
			structure for use by the "cfg_graphics.o" module
  
 ------------------------------------------------------------------------	
  
 
   FUNCTION:
 	ccm_build_dds will allocate memory for the dds structure, reporting any
     	errors, then open the Customized Attribute Class to get the attribute
     	objects needed for filling the dds structure.
  
	NOTES:
	(1)	The following members of the CCM DDS structure are not
		built in this routine.  These members are assumed to have
		been built in the cfg_graphics.o module or in other routines
		such as "ccm_download_ucode".

		MACRO NAME			MEMBER NAME
		=========			==========
		CCM_DDS_bus_id			bus_id
		CCM_DDS_bus_type		bus_type
		CCM_DDS_ucode_fd		microcode_fd
		CCM_DDS_ucode_filename		ucode_name
		CCM_DDS_ccm_dd_name		device_driver
		CCM_DDS_cdd_kmod_name		cdd_kmod
		
	(2)	The following members of the CCM DDS are used by the 
		device driver and are not touched by the configuration
		method

		MACRO NAME			MEMBER NAME
		==========			==========
		CCM_DDS_global_dd_data 		p_ccm_dd_data
		CCM_DDS_x_min			x_min
		CCM_DDS_y_min			y_min
		CCM_DDS_x_max			x_max
		CCM_DDS_y_max			y_max
		
     
  
   EXECUTION ENVIRONMENT:
     	This routine executes under the device configuration process and
     	is called from the device independent configuration routine.
  
  
  INPUTS:
			
  
  
   RETURNS:
  
 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/

int
ccm_build_dds(		char *		lname, 
			ccm_dds_t  **	dds, 
			int  *		size		)
{

/*----------------------------------------------
|
|	DATA DECLARATIONS
|
|-----------------------------------------------*/


	int     	rc;            		/* return code */
	ulong   	value_UL;      		/* return val from get_attr */
	char    	temp[MAXNAMLEN];     	/* temporary string variable */

	char *    	dep_dev = temp;     	/* temporary string variable */
	int     	i;             		/* loop control variable */
	
	ulong		num_mem_ranges;		/* Number of memory ranges */
	ulong		num_buid;		/* Number of bus unit id's (60x bus only) */
	struct	CuAt *	p_CuAt_display_id;


	BUC_DATA       buc_info_data;           /* Bus Unit Controller info. */

/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/



	DEBUG_0("cfg_ccmdd: entering build_dds\n")

	/*-------------------------------------------------------------
	| we should have been called with a valid pointer to the
	| dds and a valid length.  Check these parameters
	|-------------------------------------------------------------*/

	if (    ( dds	== NULL )
	     || ( (*dds)  == NULL )
	     || ( *size <  sizeof( ccm_dds_t ) )	)
	{
		DEBUG_2("cfg_ccmdd: invalid dds. ptr=%x size=$d \n",
			 dds , *size )
		return( E_DDS );
	}

	/*---------------------------------------------------------------------------
	| If the device is on the microchannel then we need to get 
	| the following data.
	| Connection location for this device.
	|--------------------------------------------------------------------------*/


	if ( ( CCM_DDS_bus_type( CFGGRAPH_ccm_dds ) == CCM_BUS_TYPE_MICROCHANNEL))
	{
		DEBUG_0("cfg_ccmdd: Attributes for Microchannel bus only \n")

		/*---------------------------------------------------------------
		|
		|  Get the required customized attributes for this device from
		|  the ODM, and write them into the DDS	
		|
		|---------------------------------------------------------------*/

		/*------- number bus mem ranges --------*/

		if(  NULL == get_attrval(	lname, 
						"num_mem_ranges", 
						(char *)NULL, 
						&num_mem_ranges, 
						&rc)  		)
		{	
			DEBUG_0("cfg_ccmdd: failed to get number bus ranges \n")
			return(rc);	
		}

		/*---------------------------------------------------------------------------------
		|
		|  For compatibility with the existing config methods of the PED, 
		|  the way we will work with multiple bus memory spaces is as follows:
		|  IF only one ibus memory space is required it will be identified by the field
		|  "bus_mem_start" and the field "num_bus_ranges" will be set to "1".  IF 
		|  multiple spaces are required the field "num_mem_ranges" will be set to
		| the required number, and the address ranges will be specified as 
		|  "bus_mem_start", "bus_mem_start1", bus_mem_start2" ....
		|---------------------------------------------------------------------------------*/

		/*------- bus mem start --------*/

		if(  NULL == get_attrval(	lname, 
						"bus_mem_start", 
						(char *)NULL, 
						&value_UL, 
						&rc)  		)
		{	
			DEBUG_0("cfg_ccmdd: failed to get bus memory start \n")
			return(rc);	
		}

		CCM_DDS_bus_mem_start( *dds,0 )	= value_UL;
		CDD_busmem_base(CFGGRAPH_cdd,0) = value_UL;
		
		/*------- bus mem length --------*/

		if(  NULL == get_attrval(	lname, 
						"bus_mem_length", 
						(char *)NULL, 
						&value_UL, 
						&rc)  		)
		{
			DEBUG_0("cfg_ccmdd: failed to get bus memory length \n")
			return(rc);	
		}

		CCM_DDS_bus_mem_length( *dds,0 ) =  	value_UL;
		CDD_busmem_len(CFGGRAPH_cdd,0) = 	value_UL;
		
		for(i = 1 ; i < num_mem_ranges ; i++)
		{

			sprintf(temp, "bus_mem_start%d", i+1);

			/*------- bus mem start --------*/

			if(  NULL == get_attrval(	lname, 
							temp, 
							(char *)NULL, 
							&value_UL, 
							&rc)  		)
			{	
				DEBUG_0("cfg_ccmdd: failed to get bus mem start\n")
				return(rc);
			}

			CCM_DDS_bus_mem_start( *dds,i )	= value_UL;
			CDD_busmem_base(CFGGRAPH_cdd,i) = value_UL;
		

			/*------- bus mem length --------*/

			sprintf(temp, "bus_mem_length%d", i+1);

			if(  NULL == get_attrval(	lname, 
							temp, 
							(char *)NULL, 
							&value_UL, 
							&rc)  		)
			{	
				DEBUG_0("cfg_ccmdd: failed to get bus memory length \n")
				return(rc);	
			}
			CCM_DDS_bus_mem_length( *dds,i )	= value_UL;
			CDD_busmem_len(CFGGRAPH_cdd,i) = value_UL;

		}
		/*------- bus int level --------*/


		if(  NULL == get_attrval(	lname, 
					"int_level", 
					(char *)NULL, 
					&value_UL, 
					&rc) 		)
		{	
			DEBUG_0("cfg_ccmdd: failed to get bus int level \n")
			return(rc);	
		}
		CCM_DDS_bus_int_level( (*dds) )	= ( ushort ) value_UL;


		/*------- bus int priority --------*/


		if(  NULL == get_attrval(	lname, 
						"int_priority", 
						(char *)NULL, 
						&value_UL, 
						&rc) 		 )
		{	
			DEBUG_0("cfg_ccmdd: failed to get bus priority \n")
			return(rc);	
		}
		CCM_DDS_bus_int_priority( (*dds) )	= ( short ) value_UL;
	}

	if ( ( CCM_DDS_bus_type( CFGGRAPH_ccm_dds ) == CCM_BUS_TYPE_MICROCHANNEL) ||
	     ( CCM_DDS_bus_type( CFGGRAPH_ccm_dds ) == CCM_BUS_TYPE_60x )
	   )
	{
		rc = get_conn( lname, &value_UL );     /* get logical slot (physical slot -1) */
		
		if ( rc != E_OK )
		{
			DEBUG_0("cfg_ccmdd: failed to get connection data\n")
			return(rc);
		}

		CCM_DDS_bus_slot( (*dds) )		= value_UL;
		DEBUG_1("cfg_ccmdd: Slot number = %d\n",CCM_DDS_bus_slot( (*dds) ))
	}

	/*---------------------------------------------------------------------------
	|
	| If the device is on the 60xbus then we need to get 
	| the following data:
	|
	|      Number of bus unit id's needed for the device,
	|
	|      segment number (device base address)
	|
	|      The bus unit id's       
	|
	|      The interrupt server number
	|
	|--------------------------------------------------------------------------*/


	if ( CCM_DDS_bus_type( CFGGRAPH_ccm_dds ) == CCM_BUS_TYPE_60x )
	{

		DEBUG_0("cfg_ccmdd: Attributes for only 60x bus \n")

		/*
		 * Correct the slot number obtained above for 60x bus 
		 */

		CCM_DDS_bus_slot( (*dds) )		+= 1;   /* physical slot  = logical slot +1 */

		/*------- number bus unit id's --------*/

		CCM_DDS_60x_int_server( (*dds) )	= ( short ) 0xff;

   		/*
       		   Get BUC (Bus Unit Controller) information from IPL control block
       		   For informattion about slot, see Rainbow 3 Engineering Workbook.
    		*/

		rc = get_buc(CCM_DDS_bus_slot( (*dds) ), &buc_info_data);

		if (rc != 0)
		{
			DEBUG_0("cfg_ccmdd: failed to get ipl information \n")
			return (-1);
		}
#if 0
   		prnt(&buc_info_data);   /* if you want to look at the content */
#endif

   		/* 
    		 * See Dev Characteristic Reg Definition (FF20 0000) for Power PC 
    		 */

   		#define IO_TYPE	0x3 
   		if(buc_info_data.device_type != IO_TYPE )    
   		{
      			DEBUG_0("  Non IO Device !!! \n");
     			return (-1);
   		}

		(*dds)->architecture   = _system_configuration.architecture; 
		(*dds)->implementation = _system_configuration.implementation;


		DEBUG_1("cfg_ccmdd: number 60x buid's = %d \n", buc_info_data.num_of_buids);
	
		for(i = 0 ; i < buc_info_data.num_of_buids ; i++)
   		{
      			DEBUG_1("build_dds: buid = %x\n", buc_info_data.buid_data[i].buid_value);

      			CCM_DDS_60x_buid( *dds, i ) = buc_info_data.buid_data[i].buid_value;
   		}

   		DEBUG_1("build_dds: base address = %x\n", buc_info_data.mem_addr1);
		CCM_DDS_bus_mem_start( *dds,0 )	= buc_info_data.mem_addr1;

        	/* 
		 * For 60x bus we don't need the length - only for micro-channel adapter
         	 */


		/* 
	 	 *  It has been decides that Baby Bule should have different device ids
	 	 *  for different card configuration (vram - 1M vs 3M) and the machine it
	 	 *  is attached too.  If this is no longer true, we can't have a PdAt for
	 	 *  the bus id in .add file
	 	 */

		if(  NULL == get_attrval(	lname, 
						"bus_id", 
						(char *)NULL, 
						&value_UL, 
						&rc) 		 )
		{	
			DEBUG_0("cfg_ccmdd: failed to get bus bus id for power pc\n")
			return(rc);	
		}

		CCM_DDS_60x_segment( (*dds) ) = value_UL; 
		DEBUG_1("cfg_ccmdd: Bus bus id for power pc = 0x%x\n",CCM_DDS_60x_segment(*dds) )

	}

		/*------- screen width mm --------*/


	if(  NULL == get_attrval(	lname, 
					"scrn_width_mm", 
					(char *)NULL, 
					&value_UL, 
					&rc) 		 )
	{
		DEBUG_0("cfg_ccmdd:failed to get screen width\n")
		return(rc);
	}

	CCM_DDS_screen_width_mm( (*dds) )	= ( short ) value_UL;


		/*------- screen height mm  --------*/

	if(  NULL == get_attrval(	lname, 
					"scrn_height_mm", 
					(char *)NULL, 
					&value_UL, 
					&rc) 		)
	{
		DEBUG_0("cfg_ccmdd:failed to get screen heigth\n")
		return(rc);
	}

	CCM_DDS_screen_height_mm( (*dds) )	= ( short ) value_UL;


		/*------- KSR color table --------*/

	for( i=0; i< VT_NUM_OF_COLORS; i++)
	{
		/*---------------------------------------------
		| use a temporary string to hold the correct
		| name of the attribute
		|---------------------------------------------*/

		sprintf(temp, "ksr_color%d", i+1);

		if(  NULL == get_attrval( lname, 
					temp, 
					(char *)NULL, 
					&value_UL, 
					&rc) 		)
		{
			DEBUG_0("cfg_ccmdd:failed to get ksr color\n")
		return(rc);
		}


		CCM_DDS_ksr_color_table( (*dds) )[i]= value_UL;
	}


		/*------- config display id ----------
		|    save the ptr to this attribute
		|    for later processing 
		|-----------------------------------*/

	p_CuAt_display_id = get_attrval( lname, 
					"display_id", 
					(char *)NULL, 
					&value_UL, 
					&rc  		);	
	if ( NULL == p_CuAt_display_id )
	{
		DEBUG_0("cfg_ccmdd:failed to get display id\n")
		return(rc);
	}

	CCM_DDS_config_display_id( (*dds) )	= value_UL;


		/*----------------------------------------
		| use the display ID and get which instance
		| of display this really is.  Update the
		| customized attributes and the DDS
		|----------------------------------------*/

	rc = get_instance(lname, &value_UL);
	
	if ( rc != E_OK )
	{
		DEBUG_0("cfg_ccmdd:failed to get instance\n")
		return(rc);
	}


	CCM_DDS_config_display_id( (*dds) )	|= value_UL;

	sprintf(  	p_CuAt_display_id->value, 
			"0x%08x", 
			CCM_DDS_config_display_id( (*dds) )	);

	rc = putattr( p_CuAt_display_id );

	if ( rc != E_OK  )
	{	return(E_ODMUPDATE);	}


		/*-------- LFT "belongs to" dependency ---*/


	if(  NULL == get_attrval(	lname, 
					"belongs_to", 
					dep_dev, 
					&value_UL, 
					&rc	) 		)
	{
		DEBUG_0("cfg_ccmdd:failed to get belongs to\n")
		return(rc);
	}


	rc = build_depend( dep_dev , lname );
	
	if ( rc != E_OK )
	{
		DEBUG_1("cfg_ccmdd: Could not add dependency for lft -> %s\n",lname)
		return(rc);
	}



		/*----------------------------------------
		| Copy the logical device name to dds 
		| for error logging by device.
		|----------------------------------------*/

	strcpy( CCM_DDS_component( (*dds) ) , lname);


	DEBUG_0("cfg_ccmdd: leaving build_dds\n")
	
	return(E_OK);
}







/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME: 	ccm_generate_minor
  
 TITLE: 		standard device config entry point which defines
			and returns a minor number used in building
			a device number
  
 ------------------------------------------------------------------------	
  
 
   FUNCTION:	Calls genminor to get the next device minor number
  


   EXECUTION ENVIRONMENT:
     This routine executes under the device configuration process and
     is called from the device independent configuration routine.
     
  
  INPUTS:
			
  
  
   RETURNS:
  
 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/


int
ccm_generate_minor(	char *		lname,
			long		majorno,
			long *		minorno		)
{

/*----------------------------------------------
|
|	DATA DECLARATIONS
|
|-----------------------------------------------*/


	long	*minorptr;



/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/

	DEBUG_0("cfg_ccmdd: entering generate_minor\n")

	minorptr = genminor(lname, majorno , -1, 1, 1, 1);

	if( minorptr == (long *)NULL )
	{	return(E_MINORNO);	}

	*minorno = *minorptr;

	DEBUG_0("cfg_ccmdd: leaving generate_minor\n")

	return(E_OK);
}









/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME: 	ccm_download_ucode
  
 TITLE: 		Ensures that, if appropriate, the CCM version of
			the adapter microcode is opened and made available
			for downloading into the hardware.  The actual 
			download is deferred until the CCM_VDD vdd_init( )
			routine is called.
  
 ------------------------------------------------------------------------	
  
 
   FUNCTION:
  
     
  
   EXECUTION ENVIRONMENT:
  
  
  INPUTS:
			
  
  
   RETURNS:
  
 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/


int
ccm_download_ucode(	char *		lname	)
{

/*----------------------------------------------
|
|	DATA DECLARATIONS
|
|-----------------------------------------------*/



/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/


	return(E_OK);
}






/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME: 	ccm_query_vpd
  
 TITLE: 		read the device VPD and return it in a standard 
			device independent fashion to the calling party
  
 ------------------------------------------------------------------------	
  
 
   FUNCTION:
 	Uses calls to cdd_query_vpd to extract the VPD from the
	adapter.  The adapter code must perform any setup that is
	required to make reading the POS work correctly.

  
   EXECUTION ENVIRONMENT:
  
  
  INPUTS:
			
  
  
   RETURNS:
  
 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/

int
ccm_query_vpd(		struct CuDv *	CuDv_obj, 
			mid_t		kmid, 
			dev_t		devno, 
			char *          p_odm_vpd,
                        cdd_vpd_t *	p_cdd_vpd          )

{

/*----------------------------------------------
|
|	DATA DECLARATIONS
|
|-----------------------------------------------*/

	int	ret_code,
		i;

	cdd_qvpd_cmd_t	s_cdd_vpd_cmd;
	cdd_qvpd_cmd_t	*p_cdd_vpd_cmd;
	cdd_qvpd_out_t	*p_cdd_vpd_out;
	cdd_qvpd_out_t	s_cdd_vpd_out;

/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/

	
	DEBUG_0("cfg_ccmdd: entering query_vpd\n")

	/*-------------------------------------------------------------
        | we should have been called with a valid pointer to the
	| ccm vpd structure and to the odm_vpd buffer as well as the 
        | ccm query_vpd routine.  Check these parameters
        |-------------------------------------------------------------*/

        if ( (p_odm_vpd == NULL) || (p_cdd_vpd == NULL) )   
        {
                DEBUG_0("cfg_ccmdd: NULL query_vpd ptr\n")
                return( E_ARGS );
        }
		
	/*-------------------------------------------------------------
	| Initialize all pointer variables to be used          
	|------------------------------------------------------------*/

	p_cdd_vpd_cmd = & s_cdd_vpd_cmd;
	p_cdd_vpd_out = & s_cdd_vpd_out;

	/*-------------------------------------------------------------
	| Set up all local variables to call the cdd query vpd 
	|------------------------------------------------------------*/

	CDD_cmd(CFGGRAPH_cdd) = CDD_CMD_QVPD;
	CDD_DataIn(CFGGRAPH_cdd) = &s_cdd_vpd_cmd;
	CDD_len_in(CFGGRAPH_cdd) = sizeof(cdd_qvpd_cmd_t);

	CDD_DataOut(CFGGRAPH_cdd) = &s_cdd_vpd_out;
	CDD_len_out(CFGGRAPH_cdd) = sizeof(cdd_qvpd_out_t);

	CDD_iocc_base(CFGGRAPH_cdd) = CDD_IOCC_BASE;
	s_cdd_vpd_out.p_cdd_vpd = p_cdd_vpd;

	/*---------------------------------------------------------------------------
	| If the device is on the microchannel then we need to find the slot number
	| and validate the IOCC segment contents.
	|--------------------------------------------------------------------------*/

	if ( CCM_DDS_bus_type( CFGGRAPH_ccm_dds ) == CCM_BUS_TYPE_MICROCHANNEL)
	{

		/*
	 	 * Get the starting slot number to pass in to the cdd_query_vpd routine.
	 	 */

		CDD_QVPD_slot(p_cdd_vpd_cmd) = CCM_DDS_bus_slot(CFGGRAPH_ccm_dds);

		/*
	 	 * Check the device attributes iocc segment for validity.
	 	 */
	
		if ( ! (CCM_IOCC_VALID( CDD_iocc_seg(CFGGRAPH_cdd))) ) 	
		{
			return(E_ARGS);
		}
	}
	else if ( CCM_DDS_bus_type( CFGGRAPH_ccm_dds ) == CCM_BUS_TYPE_60x )
	{
		/*
	 	 * Get the starting slot number to pass in to the cdd_query_vpd routine.
	 	 */
		CDD_QVPD_slot(p_cdd_vpd_cmd) = CCM_DDS_bus_slot(CFGGRAPH_ccm_dds);
	}

	/* else it must be the SGA bus; there is nothing special to do for the */
	/* SGA bus */


	/*--------------------------------------------------------------------------
	| call cdd_query_vpd
	|---------------------------------------------------------------------------*/

	ret_code = (CDD_qvpd(CFGGRAPH_cdd));

	if(ret_code != E_CDD_PASS)
	{
		return(E_VPD);
	}

        /*
         * The number of valid units is the number of modules on the adapter
         * that return vpd AND must be set in the cdd code.
	 *
	 * This dates back to Pedernales adapter (GT4x) which is a card set
         * of 2 or 3 boards.  Each board has it own vpd.  In this case, the
 	 * number of units is equal to number of boards.  For single board
	 * adapter the number of unit is 1.
         */

	for(i = 0 ; i < CDD_VPD_num_valid_units( p_cdd_vpd ) ; i++)
	{
		strcat(p_odm_vpd, CDD_VPD_buffer(p_cdd_vpd, i));
	}

	return( E_OK );

}

