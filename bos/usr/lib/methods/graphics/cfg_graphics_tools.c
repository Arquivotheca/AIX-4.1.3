static char sccsid[] = "@(#)47  1.13  src/bos/usr/lib/methods/graphics/cfg_graphics_tools.c, dispcfg, bos411, 9428A410j 5/9/94 12:56:02";
/*
 *   COMPONENT_NAME: DISPCFG
 *
 *   FUNCTIONS: DD_Not_Found
 *		Get_Custom_Object
 *		Get_Predef_Object
 *		Method_Not_Found
 *		Ucode_Not_Found
 *		cfg_svcs_bus_cpy1975
 *		cfg_svcs_bus_get_c1810
 *		cfg_svcs_bus_get_l1486
 *		cfg_svcs_bus_get_s1648
 *		cfg_svcs_bus_put_c1322
 *		cfg_svcs_bus_put_l1002
 *		cfg_svcs_bus_put_s1162
 *		cfg_svcs_error
 *		extract_ccm_vpd
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
#define PS2_ROM_HEAD 0x55aa

#define ROS_OK 0
#define ROS_CRC_ERROR 1
#define ROS_ADDR_ERROR 2
#define ROS_CHECKSUM_ERROR 3
#define POS2_ENABLE       0x01
#define _512Byte 512

#define MIN( a , b )	( ( (a) < (b) ) ? (a) : (b) )

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






/*==========================================================================
|
|  Include files which describe the common character mode interface and
|  the video ROM scan interface
|
|==========================================================================*/

#define Boolean unsigned int		/* need by aixfont.h		*/
#define Bool    unsigned int		/* need by aixfont.h		*/

#include <sys/aixfont.h>
#include <sys/dir.h>		    /* needed by ccm_dds.h	  */
#include <sys/mdio.h>		   /* needed by cdd_macros.h       */
#include <sys/file.h>		   /* needed by ccm.h	      */
#include <sys/intr.h>		   /* needed by vt.h	       */

#include <sys/ioacc.h>		   /* needed by ccm_dds.h	  */
#include <sys/adspace.h>	   /* needed by ccm_dds.h	  */

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
#include "cfg_graphics_macros.h"


/*==========================================================================
|
|  Define function references				 
|
|==========================================================================*/

/*--- from libc.a ---*/
extern	 int		getopt( );
extern	 void		 exit( );
extern	 int		 strcmp( );
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








/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME:	 Get_Custom_Object( )
  
 TITLE:		 Fetches a customized object from the ODM
  
 ------------------------------------------------------------------------	
  
 
 FUNCTION:
  
     
  
 EXECUTION ENVIRONMENT:
	Designed as a tool for use by the graphics configuration
	methods and their supporting tools functions.
  
  
 INPUTS:
			
  
  
 RETURNS:
	 E_OK		if object was retrieved
	E_ODMGET	if failed 
 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/


int
Get_Custom_Object (	struct Class*		p_Class_CuDv, 
			char *			p_Device_ObjectName, 
			struct CuDv *		p_CuDv_Object		)
{

/*----------------------------------------------
|
|	DATA DECLARATIONS
|
|-----------------------------------------------*/

	extern int		 odmerrno;

	   int			    ret_code;
  
	char			   search_string[256];
   


/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/




	   sprintf (search_string, "name = '%s'", p_Device_ObjectName);
  
	   ret_code = (int) odm_get_first (	p_Class_CuDv, 
						  search_string, 
						p_CuDv_Object	);
	
	if (    (ret_code == FAIL) 

	     || (ret_code == NULL)  )
	{
	   return ( E_ODMGET );
	} 

	return ( E_OK );
}





/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME:	 Get_Predef_Object
  
 TITLE:		 Gets an ODM Predefined Object from database
  
 ------------------------------------------------------------------------	
  
 
 FUNCTION:
  
     
  
 EXECUTION ENVIRONMENT:
  
  
 INPUTS:
			
  
  
 RETURNS:
  
 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/



int
Get_Predef_Object (	struct Class *	p_Class_PdDv, 
			char *		p_Device_Object_uniquetype, 
			struct PdDv *	p_PdDv_Object		)
{

/*----------------------------------------------
|
|	DATA DECLARATIONS
|
|-----------------------------------------------*/

	
	extern int		 odmerrno;

	   int			    ret_code;
  
	char			   search_string[256];


/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/


	   sprintf (	search_string, 
			"uniquetype = '%s'", 
			p_Device_Object_uniquetype  );
  
	   ret_code = (int) odm_get_first(	p_Class_PdDv, 
					search_string, 
					 p_PdDv_Object	);
	
	if (    (ret_code == FAIL) 

	     || (ret_code == NULL)  )
	{
	   return ( E_ODMGET );
	} 

	return ( E_OK );
}





/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME:     extract_vpd
  
 TITLE:   A routine which extracts VPD from an adapter and returns
		    it to the caller in the ccm_vpd_t structure.  It also
		    calls a device dependent routine to interpret the VPD
		    and set certain flags in the ccm_vpd_t structure.
  
 ------------------------------------------------------------------------       
 
 FUNCTION:
  
	  Uses the machine device driver interface to read the vpd
	  and returns data to caller. This function is passed the name 
	  of the bus to open and doesn't assume it is the machine bus0. 
		 Checks for a pos interpetation load module.  If one is not 
	  found, then no interpretation is made of the POS data.  Other
	  Otherwise, the routine is called.
  
  
 EXECUTION ENVIRONMENT:
  
		      This routine executes under the device configuration process and
		    is called from any configuration routine.
  
 INPUTS:
		      bus_name		String name of parent bus
  
		    p_ccm_vpd		Pointer to a common character mode VPD structure
	
 FUNCTIONS CALLED:
		    get_name_of_qvpd
  
 RETURNS:
  
	       On exit, returns a 0 if no errors, or an error value otherwise.
		    Write the VPD data into the ccm_vpd structure.
 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/



int 
extract_ccm_vpd (	char *		bus_name, 
			cdd_vpd_t *	 vpd, 
			int		 unit		)
{

/*-------------------------------------------
|
|	DATA DECLARATIONS
|
|-------------------------------------------*/



	MACH_DD_IO     *p_mach_dd_io;

	register int    i;
	int	     bus_fd;
	int	     rc;
	char	    path_name[MAXNAMLEN];

	int		str_test_len;

	char	    bus_pathname[MAXNAMLEN] = "/dev/";



/*------------------------------------------------
|
|	START OF SOURCE CODE
|
|------------------------------------------------*/

	if( CDD_VPD_num_valid_units( vpd ) <= unit )
	{
		DEBUG_1("cfg_graphics_tools: extract_vpd: illegal unit %d",
			unit )
		return( E_INVATTR );
	}

	CDD_VPD_buf_length( vpd, unit )	 = 0;

	CDD_VPD_min_ucode_lvl( vpd, unit )	= 0;

	/*-------------------------------------------------------
	| allocate storage for machine device driver I/O structure 
	|--------------------------------------------------------*/

	p_mach_dd_io = malloc (sizeof (MACH_DD_IO));

	if (p_mach_dd_io == NULL)
	{
		DEBUG_0 ("cfg_graphics_tools: extract_vpd: Memory allocation error\n")
		return ( E_MALLOC );
	}

	/*-----------------------------------------------------------------
	|  The bus name passed into here will be of the form bus0, bus1, etc.
	|  It will be necessary to prepend it with "/dev/" prior to calling
	|  the open routine.	
	|------------------------------------------------------------------*/


	str_test_len	= MIN(	MAXNAMLEN - strlen( bus_pathname ) - 1,
				strlen( bus_name )	);

	(void) strncat ( bus_pathname, 
			bus_name, 
			str_test_len	 );

	bus_fd = open(bus_pathname, O_RDONLY, 0);

	if (bus_fd == FAIL)
	{
		DEBUG_0 ("cfg_graphics_tools: extract_vpd: bus open failed\n")
		if (p_mach_dd_io != NULL)
		{
			free (p_mach_dd_io);
		}
		return ( E_OPEN );
	}
	/*-------------------------------------------------------------
	|  Use of the machine device driver requires passing the
	|  MACH_DD_IO structure as a parm via ioctl( ) calls
	|
	|  Here, we initialize the structure
	|
	|-------------------------------------------------------------*/

	p_mach_dd_io -> md_incr	 = MV_BYTE;
	p_mach_dd_io -> md_size	 = 1;
	p_mach_dd_io -> md_addr	 = POSREG(4, CDD_VPD_dev_slot( vpd, unit ));
	p_mach_dd_io -> md_data	 = CDD_VPD_buffer( vpd, unit );

	/*--------------------------------------------------------------
	| Read a byte at a time, calling the device driver for each
	 | byte that is desired					
	|--------------------------------------------------------------*/

	for (i = 0; i < CDD_VPD_MAX_LEN; i++)
	{
		rc = ioctl (bus_fd, CFGGRAPH_read_opcode, (caddr_t) p_mach_dd_io);

		(p_mach_dd_io -> md_data)++;

		if (rc != E_OK )
		{
			DEBUG_0 ("cfg_graphics_tools: ioctl failed\n")
			CDD_VPD_buf_length( vpd, unit ) = i;
			close (bus_fd);
			if (p_mach_dd_io != NULL)
			{
				free (p_mach_dd_io);
			}
			return ( E_BUSRESOURCE );
		}
	}


	/*---------------------------------------------------------------
	|
	| clean up and exit.  All of the data was received
	|
	|---------------------------------------------------------------*/

	CDD_VPD_buf_length( vpd, unit )		= CDD_VPD_MAX_LEN;

	close (bus_fd);
	if (p_mach_dd_io != NULL)
	{
		free (p_mach_dd_io);
	}
	return ( E_OK );

}






/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME:	 
  
 TITLE:	 
  
 ------------------------------------------------------------------------	
  
 
 FUNCTION:
	  The routine assumes a naming convention in which the name is
	either a file in /usr/lib/drivers or the name is a qualified path
	relative to $PWD or the name is a full path starting with "/".

	It builds the name and then tries to open the file for read.

	If it cannot open the file, it assumes the file is not there.
     
  
 EXECUTION ENVIRONMENT:
  
  
 INPUTS:
			
  
  
 RETURNS:
  
 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/


int
DD_Not_Found(	 char *		dd_name	)
{

/*----------------------------------------------
|
|	DATA DECLARATIONS
|
|-----------------------------------------------*/


	char		pathname[ MAXNAMLEN ];
	int		rc;
	struct stat	buffer;

/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/


	/* construct full pathname for dd_name */

	strcpy(pathname, dd_name);

	if (    (strncmp("./", pathname, 2))  
	     && (strncmp("../", pathname, 3)) 
	     && (pathname[0] != '/')		)
	{
		sprintf( pathname, "/usr/lib/drivers/%s", dd_name);
	}

	/*-----------------------------------------------
	| Use the stat( ) command to test whether the
	| file exists or not.
	|-----------------------------------------------*/

	rc = stat(	 pathname,
			&buffer		);

	if ( rc == E_OK )
	{
		/* we found the file, so invert the condition */
		return( FALSE );
	}
	else
	{
		/* we did not find the file, so the test for
		   not finding  passess				*/
		return( TRUE );
	}

}

/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME:	  Methods_Not_Found
  
 TITLE:	 
  
 ------------------------------------------------------------------------	
  
 
 FUNCTION:
	  The routine assumes a naming convention in which the name is
	either a file in /usr/lib/methods or the name is a qualified path
	relative to $PWD or the name is a full path starting with "/".

	It builds the name and then tries to open the file for read.

	If it cannot open the file, it assumes the file is not there.
     
  
 EXECUTION ENVIRONMENT:
  
  
 INPUTS:
			
  
  
 RETURNS:
  
 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/


int
Method_Not_Found(	 char *		method_name	)
{

/*----------------------------------------------
|
|	DATA DECLARATIONS
|
|-----------------------------------------------*/


	char		pathname[ MAXNAMLEN ];
	int		rc;
	struct stat	buffer;

/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/


	/* construct full pathname for method_name */

	strcpy(pathname, method_name);

	if (    (strncmp("./", pathname, 2))  
	     && (strncmp("../", pathname, 3)) 
	     && (pathname[0] != '/')		)
	{
		sprintf( pathname, "/usr/lib/methods/%s", method_name);
	}

	/*-----------------------------------------------
	| Use the stat( ) command to test whether the
	| file exists or not.
	|-----------------------------------------------*/

	rc = stat(	 pathname,
			&buffer		);

	if ( rc == E_OK )
	{
		/* we found the file, so invert the condition */
		return( FALSE );
	}
	else
	{
		/* we did not find the file, so the test for
		   not finding  passess				*/
		return( TRUE );
	}

}



/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME:	 
  
 TITLE:	 
  
 ------------------------------------------------------------------------	
  
 
 FUNCTION:
	  The routine assumes a naming convention in which the name is
	either a file in /usr/lib/drivers or the name is a qualified path
	relative to $PWD or the name is a full path starting with "/".

	It builds the name and then tries to open the file for read.

	If it cannot open the file, it assumes the file is not there.
     
  
 EXECUTION ENVIRONMENT:
  
  
 INPUTS:
			
  
  
 RETURNS:
  
 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/


int
Ucode_Not_Found(	 char *		ucode_name	)
{

/*----------------------------------------------
|
|	DATA DECLARATIONS
|
|-----------------------------------------------*/


	char		pathname[ MAXNAMLEN ];
	int		rc;
	struct stat	buffer;

/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/


	/* construct full pathname for dd_name */

	strcpy(pathname, ucode_name);
	if(    (strncmp("./", pathname, 2))  
	    && (strncmp("../", pathname, 3)) 
	    && (pathname[0] != '/')		)
	{
		sprintf( pathname, "/usr/lib/microcode/%s", ucode_name);
	}

	/*-----------------------------------------------
	| Use the stat( ) command to test whether the
	| file exists or not.
	|-----------------------------------------------*/

	rc = stat(	 pathname,
			&buffer		);

	if ( rc == E_OK )
	{
		/* we found the file, so invert the condition */
		return( FALSE );
	}
	else
	{
		/* we did not find the file, so the test for
		   not finding  passess				*/
		return( TRUE );
	}

}




/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME:	 cfg_svcs_error
  
 TITLE:		 Error Trapping Function used with cdd interface.
			In User mode, calls to i_enable, i_disable are
			not allowed.
  
 ------------------------------------------------------------------------	
  
 
 FUNCTION:		Writes a short (value) to the bus at location specified
			by segment register (seg), and pointer (ptr) .
  
     
  
 EXECUTION ENVIRONMENT:
  
  
 INPUTS:
			
  
  
 RETURNS:
  
 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/


int
cfg_svcs_error(		int		dummy	)
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


	DEBUG_0( "cfg_graphics_tools: === TRAPPED A CALL TO cfg_svcs_error\n")

	return( E_SYSTEM );

}





/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME:	 cfg_svcs_bus_put_l
  
 TITLE:		 
		
  
 ------------------------------------------------------------------------	
  
 
 FUNCTION:		Writes a long (value) to the bus at location specified
			by segment register (seg), and pointer (ptr) .
  
     
  
 EXECUTION ENVIRONMENT:
  
  
 INPUTS:
			
  
  
 RETURNS:
  
 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/


int
cfg_svcs_bus_put_l(	ulong		seg,
			ulong		ptr,
			ulong		value		)
{

/*----------------------------------------------
|
|	DATA DECLARATIONS
|
|-----------------------------------------------*/

	struct	mdio		mdio;

	int			ioctl_opcode;

	ulong			local_value;

	int			bus_num;

	char			bus_pathname[ MAXNAMLEN ]
				= "/dev/bus";
	
	char			 bus_string[ 4 ];

	int			rc;
				
/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/

	DEBUG_3("cfg_graphics_tools: busputl: \n\tseg=%#x, ptr=%#x, value=%#x\n",
		seg, ptr, value )

	/*-------------------------------------------
	| set up the basic parts of the mdio structure
	|-------------------------------------------*/

	local_value		= value;

	mdio.md_size		= 1;
	mdio.md_incr		= MV_WORD;
	mdio.md_data		= (char * ) &local_value;

	mdio.md_addr		= ptr;

	/*------------------------------------------------
	| query the segment register value to determine 
	| whether the address and segment contents are valid
	|
	| use the query to select the proper ioctl opcode
	|-----------------------------------------------*/

	if ( CDD_ADDR_IS_IOCC( seg, ptr ) )
	{
		ioctl_opcode	= MIOCCPUT;
		mdio.md_addr	-= CDD_IOCC_BASE;
	}
	else if (   ( CDD_ADDR_IS_BUS_MEM( seg, ptr ))
		 || ( CDD_ADDR_IS_BUS_IO(  seg, ptr ))	)
	{
		ioctl_opcode	= CFGGRAPH_write_opcode;
	}
	else
	{
		DEBUG_2("cfg_graphics_tools: busputl: bad addr type %x %x \n",
			seg, ptr )
		return( E_BUSRESOURCE );
	}


	/*------------------------------------------------
	| check if the bus has already been opened
	|------------------------------------------------*/

	bus_num			= CDD_ADDR_BUS_NUM( seg );
	
	if ( CFGGRAPH_bus_fd[ bus_num ] == 0 )
	{
		/*------------------------------------------
		| we have not opened this bus yet
		|-------------------------------------------*/

		sprintf( bus_string, "%1.1u", bus_num );

		strcat( bus_pathname, bus_string );

		rc = open( bus_pathname, O_RDWR, 0 );

		if ( rc == FAIL )
		{
			DEBUG_1("cfg_graphics_tools: failed to open %s\n",
				bus_string )
			return( E_DEVACCESS );
		}

		CFGGRAPH_bus_fd[ bus_num ] = rc;
	}

	/*-------------------------------------------------
	| now write out the word to the bus address
	|--------------------------------------------------*/
	
	/* If the I/O access is for Micro Channel bus memory > 256 meg	*/
	/* (i.e. the upper nibble of the bus memory addr is not 0),	*/
	/* then we must use the ioctlx() call to the machine device	*/
	/* driver which will take the "seg" parameter and directly	*/
	/* use it instead of the usual stripping of the upper nibble.	*/

	if (( CDD_ADDR_IS_BUS_MEM( seg, ptr )) && (seg & 0x0000000F))
	{
		rc = ioctlx(	CFGGRAPH_bus_fd[ bus_num ],
			ioctl_opcode,
			&mdio,
			seg			);
	}
	else
	{
		rc = ioctl(	CFGGRAPH_bus_fd[ bus_num ],
			ioctl_opcode,
			&mdio			);
	}

	if ( rc != E_OK )
	{
		/*---------------------------------------
		| failed to write to the bus
		|---------------------------------------*/

		DEBUG_1("cfg_graphics_tools: busputl: ioctl failed rc=%d",
			rc )
		return( E_DEVACCESS );
	}

	return( E_OK );

}





/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME:	 cfg_svcs_bus_put_s
  
 TITLE:		 
		
  
 ------------------------------------------------------------------------	
  
 
 FUNCTION:		Returns a bad RC if invoked.
  
     
  
 EXECUTION ENVIRONMENT:
  
  
 INPUTS:
			
  
  
 RETURNS:
  
 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/


int
cfg_svcs_bus_put_s(	ulong		seg,
			ulong		ptr,
			ushort		value		)
{

/*----------------------------------------------
|
|	DATA DECLARATIONS
|
|-----------------------------------------------*/

	struct	mdio		mdio;

	int			ioctl_opcode;

	ushort			local_value;

	int			bus_num;

	char			bus_pathname[ MAXNAMLEN ]
				= "/dev/bus";
	
	char			 bus_string[ 4 ];

	int			rc;
				
/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/

	DEBUG_3("cfg_graphics_tools: busputs: \n\tseg=%#x, ptr=%#x, value=%#x\n",
		seg, ptr, value )

	/*-------------------------------------------
	| set up the basic parts of the mdio structure
	|-------------------------------------------*/

	local_value		= value;

	mdio.md_size		= 2;
	mdio.md_incr		= MV_BYTE;
	mdio.md_data		= (char * ) &local_value;

	mdio.md_addr		= ptr;

	/*------------------------------------------------
	| query the segment register value to determine 
	| whether the address and segment contents are valid
	|
	| use the query to select the proper ioctl opcode
	|-----------------------------------------------*/

	if ( CDD_ADDR_IS_IOCC( seg, ptr ) )
	{
		ioctl_opcode	= MIOCCPUT;
		mdio.md_addr	-= CDD_IOCC_BASE;
	}
	else if (   ( CDD_ADDR_IS_BUS_MEM( seg, ptr ))
		 || ( CDD_ADDR_IS_BUS_IO(  seg, ptr ))	)
	{
		ioctl_opcode	= CFGGRAPH_write_opcode;
	}
	else
	{
		DEBUG_2("cfg_graphics_tools: busputs: bad addr type %x %x \n",
			seg, ptr )
		return( E_BUSRESOURCE );
	}


	/*------------------------------------------------
	| check if the bus has already been opened
	|------------------------------------------------*/

	bus_num			= CDD_ADDR_BUS_NUM( seg );
	
	if ( CFGGRAPH_bus_fd[ bus_num ] == 0 )
	{
		/*------------------------------------------
		| we have not opened this bus yet
		|-------------------------------------------*/

		sprintf( bus_string, "%1.1u", bus_num );

		strcat( bus_pathname, bus_string );

		rc = open( bus_pathname, O_RDWR, 0 );

		if ( rc == FAIL )
		{
			DEBUG_1("cfg_graphics_tools: busputs: failed to open %s\n",
				bus_string )
			return( E_DEVACCESS );
		}

		CFGGRAPH_bus_fd[ bus_num ] = rc;
	}

	/*-------------------------------------------------
	| now write out the word to the bus address
	|--------------------------------------------------*/
	
	/* If the I/O access is for Micro Channel bus memory > 256 meg	*/
	/* (i.e. the upper nibble of the bus memory addr is not 0),	*/
	/* then we must use the ioctlx() call to the machine device	*/
	/* driver which will take the "seg" parameter and directly	*/
	/* use it instead of the usual stripping of the upper nibble.	*/

	if (( CDD_ADDR_IS_BUS_MEM( seg, ptr )) && (seg & 0x0000000F))
	{
		rc = ioctlx(	CFGGRAPH_bus_fd[ bus_num ],
			ioctl_opcode,
			&mdio,
			seg			);
	}
	else
	{
		rc = ioctl(	CFGGRAPH_bus_fd[ bus_num ],
			ioctl_opcode,
			&mdio			);
	}

	if ( rc != E_OK )
	{
		/*---------------------------------------
		| failed to write to the bus
		|---------------------------------------*/

		DEBUG_1("cfg_graphics_tools: busputs: ioctl failed rc=%d",
			rc )
		return( E_DEVACCESS );
	}

	return( E_OK );

}





/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME:	 cfg_svcs_bus_put_c
  
 TITLE:		 
		
  
 ------------------------------------------------------------------------	
  
 
 FUNCTION:		Returns a bad RC if invoked.
  
     
  
 EXECUTION ENVIRONMENT:
  
  
 INPUTS:
			
  
  
 RETURNS:
  
 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/


int
cfg_svcs_bus_put_c(	ulong		seg,
			ulong		ptr,
			uchar		value		)
{

/*----------------------------------------------
|
|	DATA DECLARATIONS
|
|-----------------------------------------------*/

	struct	mdio		mdio;

	int			ioctl_opcode;

	uchar			local_value;

	int			bus_num;

	char			bus_pathname[ MAXNAMLEN ]
				= "/dev/bus";
	
	char			 bus_string[ 4 ];

	int			rc;
				
/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/

	DEBUG_3("cfg_graphics_tools: busputc: \n\tseg=%#x, ptr=%#x, value=%#x\n",
		seg, ptr, value )

	/*-------------------------------------------
	| set up the basic parts of the mdio structure
	|-------------------------------------------*/

	local_value		= value;

	mdio.md_size		= 1;
	mdio.md_incr		= MV_BYTE;
	mdio.md_data		= (char * ) &local_value;

	mdio.md_addr		= ptr;

	/*------------------------------------------------
	| query the segment register value to determine 
	| whether the address and segment contents are valid
	|
	| use the query to select the proper ioctl opcode
	|-----------------------------------------------*/

	if ( CDD_ADDR_IS_IOCC( seg, ptr ) )
	{
		ioctl_opcode	= MIOCCPUT;
		mdio.md_addr	-= CDD_IOCC_BASE;
	}
	else if (   ( CDD_ADDR_IS_BUS_MEM( seg, ptr ))
		 || ( CDD_ADDR_IS_BUS_IO(  seg, ptr ))	)
	{
		ioctl_opcode	= CFGGRAPH_write_opcode;
	}
	else
	{
		DEBUG_2("cfg_graphics_tools: busputc: bad addr type %x %x \n",
			seg, ptr )
		return( E_BUSRESOURCE );
	}


	/*------------------------------------------------
	| check if the bus has already been opened
	|------------------------------------------------*/

	bus_num			= CDD_ADDR_BUS_NUM( seg );
	
	if ( CFGGRAPH_bus_fd[ bus_num ] == 0 )
	{
		/*------------------------------------------
		| we have not opened this bus yet
		|-------------------------------------------*/

		sprintf( bus_string, "%1.1u", bus_num );

		strcat( bus_pathname, bus_string );

		rc = open( bus_pathname, O_RDWR, 0 );

		if ( rc == FAIL )
		{
			DEBUG_1("cfg_graphics_tools: busputc: failed to open %s\n",
				bus_string )
			return( E_DEVACCESS );
		}

		CFGGRAPH_bus_fd[ bus_num ] = rc;
	}

	/*-------------------------------------------------
	| now write out the word to the bus address
	|--------------------------------------------------*/
	
	/* If the I/O access is for Micro Channel bus memory > 256 meg	*/
	/* (i.e. the upper nibble of the bus memory addr is not 0),	*/
	/* then we must use the ioctlx() call to the machine device	*/
	/* driver which will take the "seg" parameter and directly	*/
	/* use it instead of the usual stripping of the upper nibble.	*/

	if (( CDD_ADDR_IS_BUS_MEM( seg, ptr )) && (seg & 0x0000000F))
	{
		rc = ioctlx(	CFGGRAPH_bus_fd[ bus_num ],
			ioctl_opcode,
			&mdio,
			seg			);
	}
	else
	{
		rc = ioctl(	CFGGRAPH_bus_fd[ bus_num ],
			ioctl_opcode,
			&mdio			);
	}

	if ( rc != E_OK )
	{
		/*---------------------------------------
		| failed to write to the bus
		|---------------------------------------*/

		DEBUG_1("cfg_graphics_tools: busputc: ioctl failed rc=%d",
			rc )
		return( E_DEVACCESS );
	}

	return( E_OK );

}







/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME:	 cfg_svcs_bus_get_l
  
 TITLE:		 
		
  
 ------------------------------------------------------------------------	
  
 
 FUNCTION:		Reads a long from the bus at location specified
			by segment register (seg), and pointer (ptr) and
			puts it in specified location (ulong * value).
  
     
  
 EXECUTION ENVIRONMENT:
  
  
 INPUTS:
			
  
  
 RETURNS:
  
 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/


int
cfg_svcs_bus_get_l(	ulong		seg,
			ulong		ptr,
			ulong *		value		)
{

/*----------------------------------------------
|
|	DATA DECLARATIONS
|
|-----------------------------------------------*/

	struct	mdio		mdio;

	int			ioctl_opcode;

	ulong			local_value;

	int			bus_num;

	char			bus_pathname[ MAXNAMLEN ]
				= "/dev/bus";
	
	char			 bus_string[ 4 ];

	int			rc;

	ulong			slot;        /* 60x Bus Slot Select Configuration Reg */
				
/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/

	DEBUG_3("cfg_graphics_tools: busgetl: \n\tseg=%#x, ptr=%#x, &value=%#x\n",
		seg, ptr, value )

        if ( CDD_ADDR_IS_60x_VPD_FRS_SPACE( seg, ptr ) )
        {
                slot = CCM_DDS_bus_slot( (CFGGRAPH_ccm_dds) );

                DEBUG_1("cfg_graphics_tools: bus_get_l(vpd/frs): slot=%x\n",slot);

                rc = rd_60x_vpd_feat_rom(slot, ptr, (char *) value, sizeof(int));

                if (rc)
                {
                        /*---------------------------------------
                        | failed to read/write the bus
                        |---------------------------------------*/

                        DEBUG_0("cfg_graphics_tools: rd_60x_vpd_feat_rom failed");
                        return( E_DEVACCESS );
                }

                return(E_OK);
        }
        else if ( CDD_ADDR_IS_60x_CFG_SPACE( seg, ptr ) )
        {
                slot = CCM_DDS_bus_slot( (CFGGRAPH_ccm_dds) );

                DEBUG_1("cfg_graphics_tools: bus_get_l(cfg regs): slot=%x\n",slot);

                rc = rd_60x_std_cfg_reg_w(slot, ptr, value);

                if (rc)
                {
                        /*---------------------------------------
                        | failed to read/write the bus
                        |---------------------------------------*/

                        DEBUG_0("cfg_graphics_tools: rd_60x_std_cfg_reg_w failed");
                        return( E_DEVACCESS );
                }

                return(E_OK);
        }

	/*-------------------------------------------
	| set up the basic parts of the mdio structure
	|-------------------------------------------*/

	mdio.md_size		= 1;
	mdio.md_incr		= MV_WORD;
	mdio.md_data		= (char * ) &local_value;

	mdio.md_addr		= ptr;

	/*------------------------------------------------
	| query the segment register value to determine 
	| whether the address and segment contents are valid
	|
	| use the query to select the proper ioctl opcode
	|-----------------------------------------------*/

	if ( CDD_ADDR_IS_IOCC( seg, ptr ) )
	{
		ioctl_opcode	= MIOCCGET;
		mdio.md_addr	-= CDD_IOCC_BASE;
	}
	else if (   ( CDD_ADDR_IS_BUS_MEM( seg, ptr ))
		 || ( CDD_ADDR_IS_BUS_IO(  seg, ptr ))	)
	{
		ioctl_opcode	= CFGGRAPH_read_opcode;
	}
	else
	{
		DEBUG_2("cfg_graphics_tools: busgetl: bad addr type %x %x \n",
			seg, ptr )
		return( E_BUSRESOURCE );
	}


	/*------------------------------------------------
	| check if the bus has already been opened
	|------------------------------------------------*/

	bus_num			= CDD_ADDR_BUS_NUM( seg );
	
	if ( CFGGRAPH_bus_fd[ bus_num ] == 0 )
	{
		/*------------------------------------------
		| we have not opened this bus yet
		|-------------------------------------------*/

		sprintf( bus_string, "%1.1u", bus_num );

		strcat( bus_pathname, bus_string );

		rc = open( bus_pathname, O_RDWR, 0 );

		if ( rc == FAIL )
		{
			DEBUG_1("cfg_graphics_tools: failed to open %s\n",
				bus_string )
			return( E_DEVACCESS );
		}

		CFGGRAPH_bus_fd[ bus_num ] = rc;
	}

	/*-------------------------------------------------
	| now write out the word to the bus address
	|--------------------------------------------------*/
	
	/* If the I/O access is for Micro Channel bus memory > 256 meg	*/
	/* (i.e. the upper nibble of the bus memory addr is not 0),	*/
	/* then we must use the ioctlx() call to the machine device	*/
	/* driver which will take the "seg" parameter and directly	*/
	/* use it instead of the usual stripping of the upper nibble.	*/

	if (( CDD_ADDR_IS_BUS_MEM( seg, ptr )) && (seg & 0x0000000F))
	{
		rc = ioctlx(	CFGGRAPH_bus_fd[ bus_num ],
			ioctl_opcode,
			&mdio,
			seg			);
	}
	else
	{
		rc = ioctl(	CFGGRAPH_bus_fd[ bus_num ],
			ioctl_opcode,
			&mdio			);
	}

	if ( rc != E_OK )
	{
		/*---------------------------------------
		| failed to read from the bus
		|---------------------------------------*/

		DEBUG_1("cfg_graphics_tools: busgetl: ioctl failed rc=%d",
			rc )
		return( E_DEVACCESS );
	}

	*value		 = local_value;

	return( E_OK );

}





/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME:	 cfg_svcs_bus_get_s
  
 TITLE:		 
		
  
 ------------------------------------------------------------------------	
  
 
 FUNCTION:		Reads a short from the bus at location specified
			by segment register (seg), and pointer (ptr) and
			puts it in specified location (ushort * value).
  
     
  
 EXECUTION ENVIRONMENT:
  
  
 INPUTS:
			
  
  
 RETURNS:
  
 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/


int
cfg_svcs_bus_get_s(	ulong		seg,
			ulong		ptr,
			ushort *	value		)
{

/*----------------------------------------------
|
|	DATA DECLARATIONS
|
|-----------------------------------------------*/

	struct	mdio		mdio;

	int			ioctl_opcode;

	ushort			local_value;

	int			bus_num;

	char			bus_pathname[ MAXNAMLEN ]
				= "/dev/bus";
	
	char			 bus_string[ 4 ];

	int			rc;
				
/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/

	DEBUG_3("cfg_graphics_tools: busgets: \n\tseg=%#x, ptr=%#x, &value=%#x\n",
		seg, ptr, value )

	/*-------------------------------------------
	| set up the basic parts of the mdio structure
	|-------------------------------------------*/

	mdio.md_size		= 2;
	mdio.md_incr		= MV_BYTE;
	mdio.md_data		= (char * ) &local_value;

	mdio.md_addr		= ptr;

	/*------------------------------------------------
	| query the segment register value to determine 
	| whether the address and segment contents are valid
	|
	| use the query to select the proper ioctl opcode
	|-----------------------------------------------*/

	if ( CDD_ADDR_IS_IOCC( seg, ptr ) )
	{
		ioctl_opcode	= MIOCCGET;
		mdio.md_addr	-= CDD_IOCC_BASE;
	}
	else if (   ( CDD_ADDR_IS_BUS_MEM( seg, ptr ))
		 || ( CDD_ADDR_IS_BUS_IO(  seg, ptr ))	)
	{
		ioctl_opcode	= CFGGRAPH_read_opcode;
	}
	else
	{
		DEBUG_2("cfg_graphics_tools: busgets: bad addr type %x %x \n",
			seg, ptr )
		return( E_BUSRESOURCE );
	}


	/*------------------------------------------------
	| check if the bus has already been opened
	|------------------------------------------------*/

	bus_num			= CDD_ADDR_BUS_NUM( seg );
	
	if ( CFGGRAPH_bus_fd[ bus_num ] == 0 )
	{
		/*------------------------------------------
		| we have not opened this bus yet
		|-------------------------------------------*/

		sprintf( bus_string, "%1.1u", bus_num );

		strcat( bus_pathname, bus_string );

		rc = open( bus_pathname, O_RDWR, 0 );

		if ( rc == FAIL )
		{
			DEBUG_1("cfg_graphics_tools: busgets: failed to open %s\n",
				bus_string )
			return( E_DEVACCESS );
		}

		CFGGRAPH_bus_fd[ bus_num ] = rc;
	}

	/*-------------------------------------------------
	| now write out the word to the bus address
	|--------------------------------------------------*/
	
	/* If the I/O access is for Micro Channel bus memory > 256 meg	*/
	/* (i.e. the upper nibble of the bus memory addr is not 0),	*/
	/* then we must use the ioctlx() call to the machine device	*/
	/* driver which will take the "seg" parameter and directly	*/
	/* use it instead of the usual stripping of the upper nibble.	*/

	if (( CDD_ADDR_IS_BUS_MEM( seg, ptr )) && (seg & 0x0000000F))
	{
		rc = ioctlx(	CFGGRAPH_bus_fd[ bus_num ],
			ioctl_opcode,
			&mdio,
			seg			);
	}
	else
	{
		rc = ioctl(	CFGGRAPH_bus_fd[ bus_num ],
			ioctl_opcode,
			&mdio			);
	}

	if ( rc != E_OK )
	{
		/*---------------------------------------
		| failed to read from the bus
		|---------------------------------------*/

		DEBUG_1("cfg_graphics_tools: busgets: ioctl failed rc=%d",
			rc )
		return( E_DEVACCESS );
	}

	*value		= local_value;

	return( E_OK );

}





/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME:	 cfg_svcs_bus_get_c
  
 TITLE:		 
		
  
 ------------------------------------------------------------------------	
  
 
 FUNCTION:		Reads a char from the bus at location specified
			by segment register (seg), and pointer (ptr) and
			puts it in specified location (char * value).
  
     
  
 EXECUTION ENVIRONMENT:
  
  
 INPUTS:
			
  
  
 RETURNS:
  
 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/


int
cfg_svcs_bus_get_c(	ulong		seg,
			ulong		ptr,
			uchar *		value		)
{

/*----------------------------------------------
|
|	DATA DECLARATIONS
|
|-----------------------------------------------*/

	struct	mdio		mdio;

	int			ioctl_opcode;

	uchar			local_value;

	int			bus_num;

	char			bus_pathname[ MAXNAMLEN ]
				= "/dev/bus";
	
	char			 bus_string[ 4 ];

	int			rc;
				
/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/

	DEBUG_3("cfg_graphics_tools: busgetc: \n\tseg=%#x, ptr=%#x, &value=%#x\n",
		seg, ptr, value )

	/*-------------------------------------------
	| set up the basic parts of the mdio structure
	|-------------------------------------------*/

	mdio.md_size		= 1;
	mdio.md_incr		= MV_BYTE;
	mdio.md_data		= (char * ) &local_value;

	mdio.md_addr		= ptr;

	/*------------------------------------------------
	| query the segment register value to determine 
	| whether the address and segment contents are valid
	|
	| use the query to select the proper ioctl opcode
	|-----------------------------------------------*/

	if ( CDD_ADDR_IS_IOCC( seg, ptr ) )
	{
		ioctl_opcode	= MIOCCGET;
		mdio.md_addr	-= CDD_IOCC_BASE;
	}
	else if (   ( CDD_ADDR_IS_BUS_MEM( seg, ptr ))
		 || ( CDD_ADDR_IS_BUS_IO(  seg, ptr ))	)
	{
		ioctl_opcode	= CFGGRAPH_read_opcode;
	}
	else
	{
		DEBUG_2("cfg_graphics_tools: busgetc: bad addr type %x %x \n",
			seg, ptr )
		return( E_BUSRESOURCE );
	}


	/*------------------------------------------------
	| check if the bus has already been opened
	|------------------------------------------------*/

	bus_num			= CDD_ADDR_BUS_NUM( seg );
	
	if ( CFGGRAPH_bus_fd[ bus_num ] == 0 )
	{
		/*------------------------------------------
		| we have not opened this bus yet
		|-------------------------------------------*/

		sprintf( bus_string, "%1.1u", bus_num );

		strcat( bus_pathname, bus_string );

		rc = open( bus_pathname, O_RDWR, 0 );

		if ( rc == FAIL )
		{
			DEBUG_1("cfg_graphics_tools: busgetc: failed to open %s\n",
				bus_string )
			return( E_DEVACCESS );
		}

		CFGGRAPH_bus_fd[ bus_num ] = rc;
	}

	/*-------------------------------------------------
	| now write out the word to the bus address
	|--------------------------------------------------*/
	
	/* If the I/O access is for Micro Channel bus memory > 256 meg	*/
	/* (i.e. the upper nibble of the bus memory addr is not 0),	*/
	/* then we must use the ioctlx() call to the machine device	*/
	/* driver which will take the "seg" parameter and directly	*/
	/* use it instead of the usual stripping of the upper nibble.	*/

	if (( CDD_ADDR_IS_BUS_MEM( seg, ptr )) && (seg & 0x0000000F))
	{
		rc = ioctlx(	CFGGRAPH_bus_fd[ bus_num ],
			ioctl_opcode,
			&mdio,
			seg			);
	}
	else
	{
		rc = ioctl(CFGGRAPH_bus_fd[ bus_num ],
			ioctl_opcode,
			&mdio			);
	}


	if ( rc != E_OK )
	{
		/*---------------------------------------
		| failed to read from the bus
		|---------------------------------------*/

		DEBUG_1("cfg_graphics_tools: busgetc: ioctl failed rc=%d",
			rc )
		return( E_DEVACCESS );
	}

	*value		= local_value;

	return( E_OK );

}








/*****************************************************************************
 ********************** FUNCTION HEADER TOP***********************************
 *****************************************************************************
  
 FUNCTION NAME:	 cfg_svcs_bus_cpy
  
 TITLE:		 
		
  
 ------------------------------------------------------------------------	
  
 
 FUNCTION:		Moves a block of data a char at a time to/from  the 
						bus at location specified by segment register (seg), 
						and bus address (p_bus) from/to  the local pointer 
						for a specified length (length).
     
  

    			For 60x, this function can only be used for reading 
    			the VPD/FRS space, 0xFFA0 0000 to 0xFFBF FFFF, and 
    			the configuration registers space, 0xFF20 0000 to 0xFF20 1FFF.

    			References:  Power PC 32 Bit Arch.
                 		     Rain Bow 3 Engineering Work Book

 EXECUTION ENVIRONMENT:
  
  
 INPUTS:
			
  
  
 RETURNS:
  
 
  
 *****************************************************************************
 ********************** FUNCTION HEADER BOTTOM *******************************
 *****************************************************************************/

int
cfg_svcs_bus_cpy(	ulong		seg,
			ulong		p_local,
			ulong		p_bus,
			ulong		length,
			ulong 	direction)
{

/*----------------------------------------------
|
|	DATA DECLARATIONS
|
|-----------------------------------------------*/

	struct	mdio		mdio;

	int			ioctl_opcode;

	uchar			local_value;

	int			bus_num;

	char			bus_pathname[ MAXNAMLEN ]
				= "/dev/bus";
	
	char			 bus_string[ 4 ];

	int			rc;

	ulong			slot;        /* 60x Bus Slot Select Configuration Reg */
				
/*-----------------------------------------------
|
|	START OF SOURCE CODE
|
|-----------------------------------------------*/

	DEBUG_4("cfg_graphics_tools: buscpy: \n\tseg=%#x, src=%#x, dest=%#x, len=%#x\n",
		seg, p_local, p_bus, length )

        if ( CDD_ADDR_IS_60x_VPD_FRS_SPACE( seg, p_local ) )
        {
                slot = CCM_DDS_bus_slot( (CFGGRAPH_ccm_dds) );

                DEBUG_1("cfg_graphics_tools: buscpy(vpd/frs): slot=%x\n",slot);

                rc = rd_60x_vpd_feat_rom(slot, p_local, (char *) p_bus, length);

                if (rc)
                {
                        /*---------------------------------------
                        | failed to read/write the bus
                        |---------------------------------------*/

                        DEBUG_0("cfg_graphics_tools: rd_60x_vpd_feat_rom failed");
                        return( E_DEVACCESS );
                }

                return(E_OK);
        }

        else if ( CDD_ADDR_IS_60x_CFG_SPACE( seg, p_local ) )
        {
                DEBUG_1("cfg_graphics_tools: buscpy(cfg regs): slot=%x\n",slot);
                return( E_DEVACCESS );
	}

	/*-------------------------------------------
	| set up the basic parts of the mdio structure
	|-------------------------------------------*/

	mdio.md_size		= length;
	mdio.md_incr		= MV_BYTE;
	mdio.md_data		= (char * ) &p_local;

	mdio.md_addr		= p_bus;

	/*------------------------------------------------
	| query the segment register value to determine 
	| whether the address and segment contents are valid
	|
	| use the query to select the proper ioctl opcode
	|-----------------------------------------------*/

	if ( CDD_ADDR_IS_IOCC( seg, p_bus ) )
	{
		mdio.md_addr	-= CDD_IOCC_BASE;
		if(direction == CDD_TO_BUS)
		{
			ioctl_opcode	= MIOCCPUT;
		}
		else if(direction == CDD_FROM_BUS)
		{
			ioctl_opcode	= MIOCCGET;
		}
		else
		{
			DEBUG_1("cfg_graphics_tools: buscpy: bad direction %x\n",direction)
			return( E_BUSRESOURCE );
		}
	}
	else if (   ( CDD_ADDR_IS_BUS_MEM( seg, p_bus ))
		 || ( CDD_ADDR_IS_BUS_IO(  seg, p_bus ))	)
	{
		if(direction == CDD_TO_BUS)
		{
			ioctl_opcode	= CFGGRAPH_write_opcode;
		}
		else if(direction == CDD_FROM_BUS)
		{
			ioctl_opcode	= CFGGRAPH_read_opcode;
		}
		else
		{
			DEBUG_1("cfg_graphics_tools: buscpy: bad direction %x\n",direction)
			return( E_BUSRESOURCE );
		}
	}
	else
	{
		DEBUG_2("cfg_graphics_tools: buscpy: bad addr type %x %x \n",
			seg, p_bus )
		return( E_BUSRESOURCE );
	}


	/*------------------------------------------------
	| check if the bus has already been opened
	|------------------------------------------------*/

	bus_num			= CDD_ADDR_BUS_NUM( seg );
	
	if ( CFGGRAPH_bus_fd[ bus_num ] == 0 )
	{
		/*------------------------------------------
		| we have not opened this bus yet
		|-------------------------------------------*/

		sprintf( bus_string, "%1.1u", bus_num );

		strcat( bus_pathname, bus_string );

		rc = open( bus_pathname, O_RDWR, 0 );

		if ( rc == FAIL )
		{
			DEBUG_1("cfg_graphics_tools: buscpy: failed to open %s\n",
				bus_string )
			return( E_DEVACCESS );
		}

		CFGGRAPH_bus_fd[ bus_num ] = rc;
	}

	/*-------------------------------------------------
	| now read/write to the bus address
	|--------------------------------------------------*/
	
	/* If the I/O access is for Micro Channel bus memory > 256 meg	*/
	/* (i.e. the upper nibble of the bus memory addr is not 0),	*/
	/* then we must use the ioctlx() call to the machine device	*/
	/* driver which will take the "seg" parameter and directly	*/
	/* use it instead of the usual stripping of the upper nibble.	*/

	if (( CDD_ADDR_IS_BUS_MEM( seg, p_local )) && (seg & 0x0000000F))
	{
		rc = ioctlx(	CFGGRAPH_bus_fd[ bus_num ],
			ioctl_opcode,
			&mdio,
			seg			);
	}
	else
	{
		rc = ioctl(	CFGGRAPH_bus_fd[ bus_num ],
			ioctl_opcode,
			&mdio			);
	}

	if ( rc != E_OK )
	{
		/*---------------------------------------
		| failed to read/write the bus
		|---------------------------------------*/

		DEBUG_1("cfg_graphics_tools: buscpy: ioctl failed rc=%d",
			rc )
		return( E_DEVACCESS );
	}

	return( E_OK );

}
