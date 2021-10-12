/* static char sccsid[] = "@(#)45	1.19  src/bos/usr/lib/methods/cfg_graphics/cfg_graphics.c, dispcfg, bos41J, 9513A_all 3/28/95 11:21:39"; */

/*
 *   COMPONENT_NAME: DISPCFG
 *
 *   FUNCTIONS: main
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp.  1985, 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/*****************************************************************************
 ********************** MODULE HEADER TOP ************************************
 *****************************************************************************

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

  
 PROGRAMMING INTERFACE:
  
   cfgXXX -l <logical_name> [- < 1 | 2 > ] [ -D ]
  
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

#ifdef CFGDEBUG
#        define STATIC
#else
#       define STATIC static
#endif

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
#include <errno.h>




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
extern 	int		getopt( );
extern 	void 		exit( );
extern 	int 		strcmp( );
extern 	int 		atoi( );
extern 	long 		strtol( );
extern  int		strncmp( );


/*--- from libcfg.a ---*/
extern 	int      	busresolve ();



/*--- from cfg_graphics_tools.o	----*/
extern int 		Get_Custom_Object( );
extern int 		Get_Predef_Object( );
extern	int		cfg_svcs_error( );
extern	int		cfg_svcs_bus_get_l( );
extern	int		cfg_svcs_bus_get_s( );
extern	int		cfg_svcs_bus_get_c( );
extern	int		cfg_svcs_bus_put_l( );
extern	int		cfg_svcs_bus_put_s( );
extern	int		cfg_svcs_bus_put_c( );
extern	int		cfg_svcs_bus_cpy( );
extern	int		FRS_Find_Video_Device( );
extern	int		FRS_Make_Temp_File( );
extern	int		FRS_Add_File_to_ODM( );
extern	int		FRS_Add_File_to_Methods( );
extern	int		FRS_Update_ODM_From_File( );
extern	int		FRS_Copy_File( );

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

/*---- from cfgXXXX.o, the actual device specific code ----
extern	int		generate_minor( );
extern	int		build_dds( );
extern	int		query_vpd( );
extern	int		download_microcode( );
----------------------------------------------------------*/


/*---- from cfg_ccmdd.o, the ccm dd version of dev specific code ---*/
extern	int		ccm_generate_minor( );
extern	int		ccm_build_dds( );
extern	int		ccm_query_vpd( );
extern	int		ccm_download_ucode( );


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

extern char *			optarg;       /* for getopt function */
extern int     			optind;	      /* for getopt function */


		/*=========================================================
		|
		| Define the anchor pointer used for cfg_graphics macros
		| to access various pieces of global data required by
		| the config methods and their device specific pieces
		|
		|==========================================================*/

cfg_graphics_data_t		__GLOBAL_DATA;

cfg_graphics_data_t *		__GLOBAL_DATA_PTR = &__GLOBAL_DATA;

		/*==========================================================
		|
		|  One of these anchors is filled in by the device config
		|  method, and the other is filled in by the CCM config
		|  method.
		|
		|===========================================================*/

/*---------------------------------------------------------------------------------------- 
 |
 |  lft architecture requires display graphics drivers to create special files for
 |  the device.  All of this will be true in 4.1.  However for 325, GTX150/100 (bbldd) is the
 |  first driver to do that.  Therefore a function pointer, "make_special_file", has been
 |  added to cfg_graphics_funcs_t.
 |
 |  In ccm/cdd mode, there is no special file to make.  Thus, set the function pointer to NULL
 |  Before calling "make_special_file" we'll check for NULL pointer first!
 | 
 ----------------------------------------------------------------------------------------*/

cfg_graphics_funcs_t 		_CCM_FUNCS	= { ccm_generate_minor,
						    ccm_build_dds,
						    ccm_download_ucode,
						    ccm_query_vpd,
                                                    NULL                  /* 601_CHANGE */
						   };

int            			Dflag;	       /* flag for defining children */


/*===========================================================================
|
|	Because we have divided "main( )" into pieces, with local subroutines
|	that are static and that act on data known to main, we need to declare
|	all of the data that "main" would use inside its {...} scope as
|	static data to this module but place it outside the scope of main( ).
|       This lets the other static routines have access to the data.
|
|===========================================================================*/



STATIC 	struct cfg_dd  	cfg_dd;	      	/* sysconfig command structure */
STATIC	struct cfg_load cfg_load;	/* sysconfig command structure 	*/

STATIC 	char           	*logical_name;  /* logical name to configure */
STATIC 	char           	sstring[MAXNAMLEN];  /* search criteria pointer */
STATIC 	char		odm_vpd[ VPDSIZE ];	/* used for ODM interface */
STATIC 	char           	conflist[1024];/* busresolve() configured devices */
STATIC 	char           	not_resolved[1024];	/* busresolve() not resolved
						 	* devices */

STATIC 	char 		mcabus_type[] 	= "mca";
STATIC 	char		sgabus_type[] = "sgabus";
STATIC  char            buc601_type[] = "sysplanar3"; /* 601 Bus Unit Controller type    */
						/* Feature #117983 forced this to be     */
						/* changed from sysplanar1 to sysplanar3 */

STATIC 	struct Class   	*p_Class_CuAt;	/* customized Attributes class ptr */
STATIC 	struct Class   	*p_Class_PdAt;	/* predefined Attributes class ptr */
STATIC 	struct Class   	*p_Class_CuDv;	/* customized devices class ptr */
STATIC 	struct Class   	*p_Class_PdDv;	/* predefined devices class ptr */
STATIC 	struct Class   	*p_Class_vpd;   /* customized vpd class ptr */

STATIC 	struct CuDv    	s_CuDv_object; /* customized device object storage */
STATIC 	struct PdDv    	s_PdDv_object; /* predefined device object storage */
STATIC 	struct CuDv    	s_CuDv_parent; /* customized device object storage */
STATIC 	struct PdDv    	s_PdDv_parent; /* predefined device object storage */
STATIC 	struct CuDv    	s_CuDv_temp;   /* customized device object storage */
STATIC 	struct CuVPD   	s_CuVPD_object;/* customized vpd object */

STATIC 	int            	major_num;     /* major number assigned to device */
STATIC 	int            	minor_num;     /* minor number assigned to device */
STATIC 	int		device_num;    /* composed w. makedev(maj,min)	*/
STATIC 	long           	*minor_list;   /* list returned by getminor */
STATIC 	int            	how_many;      /* number of minors in list */
STATIC  int		lock_id;       /* odm_lock id returned by odm_lock */


STATIC 	ushort         	devid;	       /* Device id - used at run-time */
STATIC 	int            	ipl_phase;     /* ipl phase: 0=run,1=phase1,2=phase2 */
STATIC 	int            	slot;	       /* slot of adapters */
STATIC 	int            	rc;	       /* return codes go here */
STATIC 	int            	errflg,
			c;	       /* used in parsing parameters   */


STATIC 	ulong		bus_id;


STATIC 	char           	*phase1,
	               	*phase2;       /* ipl phase flags */


STATIC	void *		p1;		/* temp pointer */

STATIC	char		path1[ MAXNAMLEN ];
STATIC 	char		path2[ MAXNAMLEN ];
STATIC	int		sysconfig_cmd;	/* used when loading dd */

STATIC 	char		tmp_cfg_meth[ MAXNAMLEN ];
STATIC 	char		tmp_ucode[ MAXNAMLEN ];
STATIC 	char		tmp_cdd[ MAXNAMLEN ];
STATIC 	char		tmp_cdd_pin[ MAXNAMLEN ];

STATIC	mid_t		temp_kmid;

STATIC	void *		p_rom_kmod;
STATIC	ulong		rom_kmod_length;

STATIC  void *		p_video_head;

STATIC 	char         	driver_name[10] = "ccmdd" ;



/*==========================================================================
|
|	include the private subroutine files here
|
|	This set of subroutines is declared "static" and is used by 
|	"main" and not by any other part of the graphics config method
|	set of object files
|
|===========================================================================*/

#include "cfg_graphics_priv.c"

/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME: 	main( )
  
 TITLE: 	Run by the various config commands and config libs.
		Causes the device to be configured, the ODM database
		to be updated, and the device driver to be loaded
  
 ------------------------------------------------------------------------	
  
 
   FUNCTION:
  
     The routine first parse argc and argv for any input flags.  In particular,
     it looks for the logical name of the device.
  
     Next, the routine reads opens, initializes, and locks the ODM.  It then
     opens the customized object class, searches for a match of a customized
     object to the input logical name, and, if it finds a match, checks that
     the predefined object also exists.
  
     Having found the predefined object, the function sets the front panel 
     LEDs to the value stored in the predefined object.
  
     The function then tests whether the logical device has already been 
     configured.  Typically, it has not.  If the device is DEFINED and therefore
     not configured, a long if branch is taken to configure the device.
  
     In this if branch, the parent of the object is checked.  The parent
     must be AVAILABLE. 
     
     Next, the addressability is resolved for the device, if the config method
     has been invoked during RUNTIME_CFG.  During the address resolution, the
     parent device is obtained to find out if we are dealing with an adapter
     or device on the system planar.  If the device is an adapter and its 
     parent device is a bus the following occurs:

     1). The bus_id is obtained. 
     2). The slot is checked, to make sure that no other device has already 
 	 been defined, for the slot taken by the adapter.  The slot is tested 
	 to make sure that the card is actually plugged in.
     3). Busresolve is called, to update the customized bus attributes.
  
     Also in this branch, the VPD is read from the adapter.  This is done,
     because many IBM graphics adapters do not comply with extended VPD 
     rules.  The VPD is required by the device drivers and by the 
     config methods, and it is even used to select or tailor the 
     microcode.
  
     Finally, in this branch the device driver defined in the predefined object
     is checked and loaded.  This is done by calling the "load_dvdr"
     routine, then building the DDS structure, then calling the sysconfig
     routine to init the device driver, then loading the microcode.
     
  
   EXECUTION ENVIRONMENT:
  
     This routine executes under the device configuration process and
     is called from the device independent configuration routine.
  
  INPUTS:
	argc, argv, envp
			
  
  
   RETURNS:
  
     On exit, returns an integer error value
 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/


main (int             argc,
      char          **argv,
      char          **envp)
{

/*----------------------------------------------
|
|	DATA DECLARATIONS
|
|	all of the data for main are declared as static 
|	and placed outside of the bounds of "main"
|
|-----------------------------------------------*/


/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/

	/*----------------------------------------------------------------
        |  check if environment variable set to install all packages 
	|----------------------------------------------------------------*/
        if (   (!strcmp(getenv("DEV_PKGNAME"),"ALL") )
	    /* || (DD_Not_Found(driver_name)) */ )
	{
		DEBUG_0 ("cfg_graphics: install device.graphics \n")
        	printf(":devices.graphics ");
	}




	/*----------------------------------------------------------------
	| clear the global data and set it up
	|----------------------------------------------------------------*/

	bzero( (char *) __GLOBAL_DATA_PTR, sizeof( cfg_graphics_data_t ) );

	CFGGRAPH_use_dev_vdd = FALSE;
	CFGGRAPH_use_ccm_vdd = FALSE;
	CFGGRAPH_use_device_ucode = FALSE;
	CFGGRAPH_use_ccm_ucode = FALSE;


	/*----------------------------------------------------------------
	| Initialize the cfg_func pointer in the GLOBAL DATA STRUCTURE
	|----------------------------------------------------------------*/

	 CFGGRAPH_cfg_func 		= &__GLOBAL_DATA.s_cfg_func;


	/*---------------------------------------------------------------------------------------- 
 	|
 	|  bbldd is the first driver which creates the special file for the device.  Ped, 
 	|  wga, sga don't (probably it might in 4.1).  Since a function pointer, make_special_file
 	|  has been added to cfg_graphics_funcs structure, and not all drivers initialize this 
 	|  pointer in their xx_cfg_load.c except bbldd (bbl_cfg_load.c), we want to initialize 
 	|  all functions the function pointers to NULL.  Actually, only "make_special_file is 
 	|  sufficient but it is not clear.  Later before calling "make_special_file" we'll check 
 	|  for NULL pointer first!
 	| 
 	---------------------------------------------------------------------------------------- */

	CFG_gen_minor_dev_num(CFGGRAPH_cfg_func) = NULL;
	CFG_build_dds(CFGGRAPH_cfg_func) 	 = NULL;
	CFG_download_ucode(CFGGRAPH_cfg_func)    = NULL;
	CFG_query_vpd(CFGGRAPH_cfg_func) 	 = NULL ;
	CFG_make_special_file(CFGGRAPH_cfg_func) = NULL;


	/*----------------------------------------------------------------
	| Preset variables before parsing inputs 
	|----------------------------------------------------------------*/

	ipl_phase 	= RUNTIME_CFG;
	errflg 		= 0;
	logical_name 	= NULL;
	Dflag 		= FALSE;

	/*------------------------------------------------------------------
	 | process the argc and argv for valid flags	 
	 |-----------------------------------------------------------------*/

	Check_Input_Parms( argc, argv, envp );

	/*-----------------------------------------------------------------
	| Inputs are OK.  Now we start up the ODM.  We also get the 
	| customized and predefined objects, the parent object, the bus,
	| and the dynamic bind object defined in the PdAt cfg_method_load.  
	| This is used to dynamically load device dependent config method 
	| routines.
	|-----------------------------------------------------------------*/

	Open_ODM_and_Get_Objects( );

	/*----------------------------------------------------------------
	|  If this device is being configured during an ipl phase, then
	|  display this device's LED value on the system LEDs.	       
	|---------------------------------------------------------------*/

	if (ipl_phase != RUNTIME_CFG)
	{
		setleds (s_PdDv_object.led);
	}


	/*-----------------------------------------------------------------
	|  Check to see if the device is already configured (AVAILABLE). The
	|  other possible state is DEFINED (not configured). e actually go
	|  about the business of configuring the device	only if the device is
	|  not configured yet. Configuring the	device in this case refers to
	|  the process of checking parent status, checking
	|  for attribute consistency, building a DDS, loading the driver,
	|  etc... 
	|-------------------------------------------------------------------*/

	if (s_CuDv_object.status == DEFINED)
	{

	/*-----------------------------------------------------------------
	|  The device is not available to the system yet. Now check
	|  to make sure that the device's relations will allow it to
	|  be configured. In particular, make sure that the parent is
	|  configured (AVAILABLE), and that no other devices are
	|  configured at the same location. 
	|-------------------------------------------------------------------*/

		DEBUG_0 ("cfg_graphics: device not available yet\n")

		/*----------------------------------------------------------
		| make sure that the parent is AVAILABLE	 
		-----------------------------------------------------------*/

		if (s_CuDv_parent.status != AVAILABLE)
		{
			DEBUG_0 ("cfg_graphics: parent is not AVAILABLE")
			err_exit (E_PARENTSTATE);
		}


		/*---------------------------------------------------------
		| if this graphics device is a bus adapter make sure that
		| no other device is configured at the same location.
		| The ODM permits duplicate objects to be defined for the
		| same adapter slot
		| 
		| Different types of adapter bus types require different checking
		| and preparation.
		|
		|----------------------------------------------------------*/

			
		if (strncmp( 	s_PdDv_parent.type, 
				&mcabus_type[0], 
				sizeof(mcabus_type)) 		== 0)
		{
			DEBUG_0("cfg_graphics: Call Prepare_MCAbus_Type_Device\n")
			Prepare_MCAbus_Type_Device( );
		}	


		else if (strncmp( 	s_PdDv_parent.type, 
					&sgabus_type[0], 
					sizeof(sgabus_type))	== 0)
		{
			/*--------------------------------------------------
			| insert code for SGA bus devices here
			| and match it to the logic above
			|--------------------------------------------------*/

			DEBUG_0("cfg_graphics: Call Prepare_SGAbus_Type_Device\n")
			Prepare_SGAbus_Type_Device( );
		}

                else if (strncmp(       s_PdDv_parent.type,
                                        buc601_type,
                                        sizeof(buc601_type))    == 0)
                {
                        /*--------------------------------------------------
                        |  601 Bus device
                        |--------------------------------------------------*/

			DEBUG_0("cfg_graphics: Call Prepare_601bus_Type_Device\n")
                        Prepare_601bus_Type_Device( ); 

                }

		else
		{
			/*-------------------------------------------------
			| error condition -- parent bus type not correct
			|-------------------------------------------------*/

			DEBUG_0("cfg_graphics: parent.type mismatch\n")
			err_exit( E_PARENTSTATE );
		}

		/*---------------------------------------------------------
		 | Perform all of the additional generic processing
		 | that doesn't depend on bus or sys level adapters
		 |-------------------------------------------------------*/

		Prepare_Device( );

		/*----------------------------------------------------------
		| fall through to here means status == DEFINED
		| and the device driver selection is  already done
		| and recorded in the global data
		|
		| The next step is to load the bottom half of the configuration
		| We use a "replacement of pointers to functions" technique.
		| There are standard entry points into the bottom half.  We
		| either use the ones supplied in the module as the device
		| specific ones, or we use the ones supplied by us for the 
		| common character mode.
		|
		| THIS IS THE GANG SWITCH BETWEEN DEV AND CCM MODES!
		|
		|-----------------------------------------------------------*/

		if ( CFGGRAPH_use_dev_vdd )
		{
			/*-------------------------------------------------
			| this is the case of FULL FUNCTION vdd and ucode
			| load the device dependent config methods 
			|--------------------------------------------------*/

			/*
			 * NOTE: Don't have to check to see if config method
			 * exists since we now look for it in PrepareDevice()
	  	   	 * to fix defect #154212.
			 */

			/*------------------------------------------------------
			|	Use the load command to dynamically bind the
			|  object into this runtime environment
			|------------------------------------------------------*/
	
			CFGGRAPH_methods_entry_init = load( CFGGRAPH_cfg_methods, 
								0, 
								NULL );
							
			if ( CFGGRAPH_methods_entry_init == NULL )
			{
				DEBUG_0("cfg_graphics: load failed on device dependent congiguration methods\n")
				err_exit( E_LOADEXT );
			}
		
			/*---- intialize the device dependent configuration 
			       method function pointers             ----*/

			if (CFGGRAPH_methods_entry(CFGGRAPH_funcs) != 0)
			{
				DEBUG_0("cfg_graphics: device dependent config method function initialization error\n")
				err_exit ( E_SYSTEM );

			}
			/*---- the device routine will malloc the DDS ----*/
			CFGGRAPH_dev_dds	= NULL;

			CFGGRAPH_dds_length	= 0;

			/*---- set up the pointer to the dds ----*/
			CFGGRAPH_dds_ptr_init	= &CFGGRAPH_dev_dds;

			/*-----	set the global ddname ----*/
			(void) strcpy(	CFGGRAPH_ddname, CFGGRAPH_dev_dd_name );

			/*-----	set the global pinned ddname ----*/
			(void) strcpy(	CFGGRAPH_dd_pin, CFGGRAPH_dev_dd_pin );
		}

		else if ( CFGGRAPH_use_ccm_vdd )
		{
			/*-------------------------------------------------
			| this is the case of COMMON CHARACTER MODE
			| we will need to do more processing than for the
			| full function case
			|--------------------------------------------------*/

			/*----	extract cdd driver from device's ROM  ----*/
			Prepare_Feature_ROM_Files( );
		
			/*----	copy the function pointers structure ----*/
			*CFGGRAPH_cfg_func 	= *(&(_CCM_FUNCS));

			/*---- we have already allocated the ccm DDS ----*/
			CFGGRAPH_ccm_dds	= &(__GLOBAL_DATA_PTR -> ccm_dds_struc);

			CFGGRAPH_dds_length	= sizeof( ccm_dds_t );

			/*---- set up the pointer to the dds ----*/
			CFGGRAPH_dds_ptr_init	= (void *) &CFGGRAPH_ccm_dds;

			/*-----	set the global ddname ----*/
			(void) strcpy(	CFGGRAPH_ddname, CFGGRAPH_ccm_dd_name );

			/*-----	set the global ddname ----*/
			(void) strcpy(	CFGGRAPH_dd_pin, CFGGRAPH_ccm_dd_pin );

			/*----- preset the kmids ------*/

			CCM_DDS_cdd_ext_kmid( CFGGRAPH_ccm_dds ) = 0;

			CCM_DDS_cdd_ext_pin_kmid( CFGGRAPH_ccm_dds ) = 0;

			/*--------------------------------------------------
			|
			| additional processing for the CCM case
			| is too verbose to include here, so we call to
			| a separate function
			|
			|--------------------------------------------------*/

			Set_Up_CCM_DD_Case( );
	
			/*------------------------------------------------------------
			|
			|  Load the CDD non-pinned kernel extension
			|  Look for them in the filesystem first, then out on the adapter. 
			|  Use sysconfig() with SYS_KLOAD to load multiple instances
			|  current versions cannot be depended on for re-entrant 
			|
			|  When Set_Up_CCM_DD_Case returns, we know that the
			|  string held in "path1" is good and points to the CDD module
			|
			|--------------------------------------------------------------*/

			cfg_load.path		= path1;
			cfg_load.libpath	= (char *) NULL;
	
			rc = sysconfig(	SYS_KLOAD,
					(void *) &cfg_load,
					(int) sizeof( struct cfg_load) );

			if ( rc != 0 )
			{
				DEBUG_1 ("cfg_graphics: loadext failed on %s.\n",
					 CFGGRAPH_cdd_name	)
				err_exit (E_LOADEXT);
			}

			CCM_DDS_cdd_kmid( CFGGRAPH_ccm_dds ) = cfg_load.kmid;

			/*------------------------------------------------------------
			|
			|  Load the CDD pinned kernel extension
			|  Look for them in the filesystem first, then out on the adapter. 
			|  Use sysconfig() with SYS_KLOAD to load multiple instances
			|  current versions cannot be depended on for re-entrant 
			|
			|-------------------------------------------------------------*/

			memset (path1, '\0', sizeof(path1));
	
				/*-----------------------------------------
				| Check to see if optional cdd_pin is indicated
				| If there is a name, then either ODM told us the name
				| or FRS found a file.
				|------------------------------------------*/

			if (  0 != ( strcmp ( CFGGRAPH_cdd_pin , "" ) ) )
        		{
					/*----------------------------------------------
					| convert to full path if not already 
					|-----------------------------------------------*/
		
				if (      ( strncmp( "./",  CFGGRAPH_cdd_pin, 2 ))
				      &&  ( strncmp( "../", CFGGRAPH_cdd_pin, 3 ))
				      &&  ( CFGGRAPH_cdd_pin[ 0 ] != '/' )	 	)
				{
			 		(void) strcat( ( strcpy( path1, "/usr/lib/drivers/") ),
					    		 CFGGRAPH_cdd_pin );
				}
				else
				{
					strcpy( path1, CFGGRAPH_cdd_pin );
				}
	
					/*----------------------------------------------
					| make sure that the file is really there
					| We need to do this in case ODM lied to us
					|----------------------------------------------*/

            		    	if (  DD_Not_Found ( CFGGRAPH_cdd_pin ) ) 
				{
					memset (path1, '\0', sizeof(path1));
				}
			}
			
				/*-----------------------------------------------
				| fall through to here whether or not there was a name
				| So, we test the name again
				| If we have a good name, we know that it has been
				| validated, and we load it
				|----------------------------------------------*/

			if ( 0 != strcmp ( path1, "" ) )
			{
				cfg_load.path		= path1;
				cfg_load.libpath	= (char *) NULL;
	
				rc = sysconfig(	SYS_KLOAD,
						(void *) &cfg_load,
						(int) sizeof( struct cfg_load) );
	
				if ( rc != 0 )
				{
					DEBUG_1 ("cfg_graphics: loadext failed on %s.\n",
							 CFGGRAPH_cdd_pin	)
					err_exit (E_LOADEXT);
				}

				CCM_DDS_cdd_pin_kmid( CFGGRAPH_ccm_dds ) = cfg_load.kmid;
			}

			/*-------------------------------------------------------
			| Fall through to here means we loaded the option cdd_pin
			| if it was present.  Now process the CCM DD PIN case
			|--------------------------------------------------------*/

			if ( 0 != strcmp( "", CFGGRAPH_ccm_dd_pin  )  )
			{
				/*-------------------------------------------------------
				|  If the ccmdd has a valid interrupt handler, then
				|  Load the dd pin code as an AIX kernel extension
				|------------------------------------------------------*/

				strcat( (strcpy( path1, "/usr/lib/drivers/")),
						CFGGRAPH_ccm_dd_pin);

				cfg_load.path		= path1;
				cfg_load.libpath	= (char *) NULL;
		
				rc = sysconfig(	SYS_KLOAD,
						(void *) &cfg_load,
						(int) sizeof( struct cfg_load) );
	
				if ( rc != 0 )
				{
					DEBUG_1 ("cfg_graphics: loadext failed on %s.\n",
			 			CFGGRAPH_ccm_dd_pin	)

					err_undo_kmods ();
					err_exit (E_LOADEXT);
				}

				CCM_DDS_ccmdd_pin_kmid(CFGGRAPH_ccm_dds) = cfg_load.kmid;
			}
		}
		else
		{
			DEBUG_0("cfg_graphics: cfg_func selection error!\n")
			err_exit ( E_SYSTEM );
		}


		/*------------------------------------------------------------
		|
		|  Load the device driver as an AIX kernel extension
		|  If the device driver has a seperate pinned (interrupt) handler
		|  the dd is responsible for loading the pinned part itself.
		|
		|-------------------------------------------------------------*/
  
  		if ( CFGGRAPH_use_dev_vdd ) 	{ sysconfig_cmd	= SYS_SINGLELOAD; }
  		else 				{ sysconfig_cmd	= SYS_KLOAD; }

		( void ) strcat( (strcpy( path1, "/usr/lib/drivers/")), ( CFGGRAPH_ddname ) );

		cfg_load.path		= path1;
		cfg_load.libpath	= (char *) NULL;
	
  		rc = sysconfig(	sysconfig_cmd,
				(void *) &cfg_load,
				(int) sizeof( struct cfg_load) );

		if ( rc != 0 )
		{
			DEBUG_1 ("cfg_graphics: loadext failed on %s.\n",
				 CFGGRAPH_ddname	)
			err_undo_kmods ();
			err_exit (E_LOADEXT);
		}

		cfg_dd.kmid	= cfg_load.kmid;
		
		if ( cfg_dd.kmid == NULL )
		{
			DEBUG_1 ("cfg_graphics: loadext NULL kmid on %s.\n",
				 CFGGRAPH_ddname	)
			err_undo_kmods ();
			err_exit (E_LOADEXT);
		}

		if ( CFGGRAPH_use_ccm_vdd )
		{
			CCM_DDS_ccmdd_kmid(CFGGRAPH_ccm_dds) = cfg_dd.kmid;
		}


		/*-------------------------------------------------------
		|
		|   Generate the major device number
		|
		|-------------------------------------------------------*/

		/*	
		major_num	= genmajor( CFGGRAPH_ddname );
		*/
		major_num	= genmajor( CFGGRAPH_dev_dd_name );

		if ( major_num == -1 )
		{
			DEBUG_0("cfg_graphics: bad major_num.\n")
			err_undo_kmods ();
			err_exit( E_MAJORNO );
		}

		DEBUG_1("cfg_graphics: major_num is %d.\n", major_num )

		/*-------------------------------------------------------
		|
		|  Generate the minor device number
		|
		|-------------------------------------------------------*/

		minor_list	= getminor( major_num,
					     &how_many,
					     logical_name	);

		if (    ( minor_list == NULL )
		     || ( how_many   == 0    )  )
		{
			/*----------------------------------------------
			| No minors were found by system, need to make
			| one up in this config method
			|----------------------------------------------*/
			
			DEBUG_0("cfg_graphics: calling generate_minor.\n")

			rc = CFGGRAPH_gen_minor_dev_num( logical_name,
							 major_num,
							 &minor_num	);

			if ( rc != E_OK )
			{
				DEBUG_1("cfg_graphics: minor_num error rc=%d.\n",
					rc	)

				reldevno( logical_name, TRUE );
				err_undo_kmods ();
	
				if (    ( rc < 0 )
				     || ( rc > 255 )  )
				{ 	err_exit( E_MINORNO ); 	}
				else
				{ 	err_exit( rc ); 	}
			}
		}
		else
		{
			/*------------------------------------------------
			| use the minor number supplied by the system
			|------------------------------------------------*/

			minor_num = *minor_list;
		}

		DEBUG_1("cfg_graphics: minor_num is %d.\n", minor_num )

		
		/*----------------------------------------------------------
		|
		|  There are no special files for graphics, so we do not call
		|  a "make_special_files" entry point
		|
		|  No longer true!  bbldd creates special file now.  We 
                |  only do that when loading the full function driver but not cdd
                |
		|-----------------------------------------------------------*/

		/*------------------------------------------------------------
		|
		|  build the "define device structure"
		|
		|------------------------------------------------------------*/

		rc = CFGGRAPH_build_dds( logical_name,
					  CFGGRAPH_dds_ptr_init,
					  &CFGGRAPH_dds_length	);

		if ( rc != E_OK )
		{
			DEBUG_1("cfg_graphics: error on build dds rc=%d.\n",rc)
			err_undo_kmods ();
	
			if (    ( rc < 0 )
			     || ( rc > 255 ) )
			{	err_exit( E_DDS );	}
			else
			{	err_exit( rc );		}
		}

		cfg_dd.ddsptr	= CFGGRAPH_dds;
		cfg_dd.ddslen	= CFGGRAPH_dds_length;

		/*-----------------------------------------------------------
		|
		|  save the device number
		|
		|------------------------------------------------------------*/
						
		device_num	= makedev( major_num , minor_num );


		cfg_dd.devno	= device_num;

		if ( CFGGRAPH_use_ccm_vdd )
		{
			CCM_DDS_device_num( ((ccm_dds_t *)CFGGRAPH_dds) ) = device_num;
		}


		/*-----------------------------------------------------------
		|
		|  call sysconfig to send the define device structure 
		|  to the already loaded device driver.  This call
		|  invokes the entry point of the device driver, which is
		|  the config entry point.  This is typically the first
		|  time program control is passed into the device driver.
		|
		|  The command cause the config entry point of the device
		|  driver to take the "CFG_INIT" case.
		|
		|-----------------------------------------------------------*/
			
		cfg_dd.cmd 	= CFG_INIT;

		rc = sysconfig (  SYS_CFGDD, 
				  &cfg_dd, 
				  sizeof (struct cfg_dd)	);

		if (rc != E_OK )
		{
			if ( p1 != NULL)
			{
				free(p1);			/* Free the ddf memory	*/
			}
			DEBUG_0 ("cfg_graphics: error configuring device\n")
			err_undo_kmods ();
			err_exit (E_CFGINIT);
		}


		/*----------------------------------------------------------
		|
		|  query the vpd from the adapter IF:
		|     the PdDv claims it supports VPD and
		|      it is not a neptune (GXT150M) in CCM mode.
		|
		|----------------------------------------------------------*/

		DEBUG_0 ("cfg_graphics: Checking for vpd\n")
		DEBUG_1("cfg_graphics: logical_name = %s\n", logical_name) ;
		DEBUG_1("cfg_graphics: length = %d\n", strlen(logical_name)) ;

		if ( 	(s_PdDv_object.has_vpd == TRUE) 
		     && ! (   (CFGGRAPH_use_ccm_vdd) 
			   && (strncmp(logical_name, "nep", 
			  	       (strlen(logical_name)-1)) == 0) ) 
		   )
		{
		 	DEBUG_0("cfg_graphics: Call Proc_Dev_Customized_VPD\n");
			Process_Device_Customized_VPD( );
		}	

		/*----------------------------------------------------------
		|
		|  download the microcode into the adapter
		|
		|---------------------------------------------------------*/

		DEBUG_0 ("cfg_graphics: Calling download_microcode()\n")

		rc = CFGGRAPH_download_ucode(logical_name);

		if ( rc != E_OK )
		{
			DEBUG_1 ("cfg_graphics: download ucode, rc=%d\n", rc)
			err_undo_kmods ();

			if (    ( rc < 0 )
			     || ( rc > 255 ) )
			{	err_exit( E_UCODE );	}
			else
			{	err_exit( rc );		}
		}

		DEBUG_0 ("cfg_graphics: Returned from download_microcode()\n")

		/*----------------------------------------------------------
		|
		|  device object can now be marked as AVAILABLE 
		|  update customized device object with a change operation 
		| 
		|-----------------------------------------------------------*/

		s_CuDv_object.status = AVAILABLE;

		strcpy(s_CuDv_object.ddins, CFGGRAPH_dev_dd_name);

		rc = odm_change_obj( p_Class_CuDv, 
				     &s_CuDv_object	);
		
		if ( rc  == FAIL)
		{
			DEBUG_0 ("cfg_graphics: ODM failure updating CuDv object\n")
	
			err_undo_kmods ();
			err_exit (E_ODMUPDATE);
		}
	}			       /* end if (device is DEFINED) */


	/*--------------------------------------------------------------------
	|
	|  When we get here, the device either already was AVAILABLE or we
	|  just made it AVAILABLE without any errors 
	|
	|-------------------------------------------------------------------*/

        /*
         *   DON'T make special file when running ccm/cdd mode or driver
	 *   which does not supply one (i.e. its xx_cfg_load.c does not
         *   initialize the make_special_file field of the structure.  
         */

        if ( CFG_make_special_file(CFGGRAPH_funcs) != NULL )             /* 601_CHANGE */
        {
           DEBUG_0 ("cfg_graphics: make special file\n")
           rc = CFGGRAPH_make_special_file(logical_name,device_num);

        }
	else
	{
           DEBUG_0 ("cfg_graphics: not calling make_special file\n")
	}


	/*--------------------------------------------------------------------
	|  this is where one would call the device specific routine to 
	|  manage and configure child devices.  (This routine is
	|  unnecesary given the present	design of the VDD and config
	|  methods.  It is kept	for future compatibility issues)         
	|-------------------------------------------------------------------*/


	/*-------------------------------------------------------------------
	| Free memory and unload the modules loaded earlier
	--------------------------------------------------------------------*/
	if ( p1 != NULL)
	{
		free(p1);			/* Free the ddf memory	*/
	}

	if ( p_video_head != NULL)
	{
		free(p_video_head);		/* Free the FRS allocated memory */
	}


	 if ( CDD_procs( CFGGRAPH_cdd)->entry_point != NULL )
	 {
	 	rc = unload(CDD_procs( CFGGRAPH_cdd)->entry_point);
		if ( rc == FAIL )
		{
			DEBUG_0 ("cfg_graphics: unload of cdd  failed")
		}
	 }

	 if ( CFGGRAPH_methods_entry_init != NULL )
	 {
	 	rc = unload(CFGGRAPH_methods_entry_init);
		if ( rc == FAIL )
		{
			DEBUG_0 ("cfg_graphics: unload of methods load module failed")
		}
	}

	/*--------------------------------------------------------------------
	|  Close the customized and predefined object classes and terminate
	|  the ODM	 
	|-------------------------------------------------------------------*/

	rc = odm_close_class( p_Class_PdDv );

	if ( rc == FAIL )
	{
		DEBUG_0 ("cfg_graphics: close object class PdDv failed")
		err_undo_kmods ();
		err_exit (E_ODMCLOSE);
	}
	rc = odm_close_class( p_Class_CuDv );

	if ( rc == FAIL )
	{
		DEBUG_0 ("cfg_graphics: close object class CuDv failed")
		err_undo_kmods ();
		err_exit (E_ODMCLOSE);
	}
	rc = odm_close_class( p_Class_PdAt );

	if ( rc == FAIL )
	{
		DEBUG_0 ("cfg_graphics: close object class PdAt failed")
		err_undo_kmods ();
		err_exit (E_ODMCLOSE);
	}

	rc = odm_close_class( p_Class_CuAt );

	if ( rc == FAIL )
	{
		DEBUG_0 ("cfg_graphics: error closing CuAt object class\n")
		err_undo_kmods ();
		err_exit (E_ODMCLOSE);
	}

	odm_unlock(lock_id);

	odm_terminate( );

	unlink_tmp_files( );

	exit (0);
}

