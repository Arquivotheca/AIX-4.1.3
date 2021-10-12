static char sccsid[] = "@(#)69	1.7  src/bos/usr/lib/methods/cfg_sgabus/cfg_sgabus.c, dispcfg, bos411, 9428A410j 6/29/94 10:00:24";
/*
 *   COMPONENT_NAME: SYSXDISPCCM
 *
 *   FUNCTIONS: build_dds
 *		def_child_obj
 *		define_children
 *		download_microcode
 *		generate_minor
 *		make_special_files
 *		mdd_get
 *		query_vpd
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

                                                                   
/***********************************************************************
 ******************* MODULE HEADER TOP *********************************
 ***********************************************************************
 ----------------------------------------------------------------------

 FUNCTION:

 	This file contains the device dependent code pieces 
	for the configuration method of the sgabus.

	Many of the entry points just return 0.

	Only define_children( ) does any work.

	The sgabus is not set up like other buses.  For instance,
	it only has one slot.  Also, its ODM setup is (historically)
	different from other buses such as microchannel.

	The system planar predefined connections are done by
	bus unit controller ID.  There are PdCn for the uniquetype
	"planar/sys/sysplanar1" for buid 20,21, and 40.  20 and 21
	are the microchannel buids.  The parent connecttion to sysplanar1
	is at 20 or 21; the child is an ioplanar device which in turn
	has more children.

	For sgabus, the parent connection to sysplanar1 is at
	40, but the child is the actual bus, not an ioplanar.
	The bus in turn has a single slot.  The slot is defined
	to be connection 0.  There is more than one kind of adapter
	that can be inserted into the slot.

	This config method resolves which adapter to use, and sets up
	the ODM connections accordingly.

 **********************************************************************
 ***************** MODULE HEADER BOTTOM *******************************
 **********************************************************************/


/*=====================================================================
|
|	STANDARD INCLUDE FILES
|
|======================================================================*/

#include <sys/types.h>
#include <cf.h>
#include <stdio.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include "cfgdebug.h"


/* Required for IPL-control block: */
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/devinfo.h>
#include <sys/mdio.h>
#include <sys/iplcb.h>

	/*----------------------------------------
	| Files which define the cdd interface and
	| Feature ROM Scan code
	|----------------------------------------*/

#include "cdd.h"
#include "cdd_macros.h"
#include "cdd_intr.h"
#include "cdd_intr_macros.h"

#include "cfg_graphics_frs.h"

#include "frs.h"
#include "frs_macs.h"
#include "frs_display.h"
#include "frs_display_macs.h"

/*=====================================================================
|
|	LOCAL SYMBOL DEFINITIONS
|
|======================================================================*/
#define SGA_UTYPE              "adapter/sys/sga"

char _PROGRAM[] = "[cfg_sgabus] ";

#ifdef CFGDEBUG
#define	STATIC
#else
#define	STATIC static
#endif

/*=====================================================================
|
|	GLOBAL DATA
|
|======================================================================*/

struct Class *		predev;
struct Class *		cusdev;

/*=====================================================================
|
|	EXTERNAL FUNCTIONS
|
|======================================================================*/

/*------ from ?? ------*/

extern	struct	CuAt	*getattr();




/************************************************************************
 ************************ FUNCTION HEADER TOP ***************************
 ************************************************************************

 NAME:		def_child_obj

 FUNCTION:	
		Reads the PdDv and CuDv objects from the database.  
		Sets the LEDs if not run-time.
		Defines the CuDv object if not already there.
		Fills out the CuDv structure.
		Returns error codes if the object is not found.

 PARAMETERS:	utype		(I) 	child device logical name
		pname		(I) 	logical name of the parent
		connection	(I) 	connection point on parent device
		cudv		(O) 	address of CuDv filled on on return
		phase		(I) 	ipl phase value

 RETURN:	integer		standard ODM error return values
 
 =======================================================================*/

/*-------------------------------------
|
|	FUNCTION DECLARATION
|
|-------------------------------------*/

STATIC int
def_child_obj	(
		char *		utype,		/* Device predefined uniquetype */
		char *		pname,		/* Parent device logical name */
		char *		connection,	/* Connection location on parent device */
		struct CuDv *	cudv,		/* Pointer to CuDv object */
		int		phase		/* Phase indicator */
		)
{
/*--------------------------------------
|
|	DATA DECLARATIONS
|
|--------------------------------------*/

struct PdDv 	pddv;		/* predefined device object storage */
int		rc;		/* ODM return code */
char		sstring[256];	/* the ODM search string */
char *		outp;

/*--------------------------------------
|
|	START OF SOURCE CODE
|
|--------------------------------------*/

DEBUG_1("%s: Getting PdDv and CuDv for:\n", _PROGRAM )
DEBUG_1("     unique type = %s\n",utype)
DEBUG_1("     parent name = %s\n",pname)
DEBUG_1("     connection  = %s\n",connection)

/* Get PdDv first.  This is done so that if the PdDv
   is not there, we can return to the calling routine
   and print out the package name.
*/

sprintf(sstring, "uniquetype = '%s'", utype);
	
rc = (int) odm_get_first(predev, sstring, &pddv);
	
if ( rc == 0 ) 
{
	/* No PdDv object for this device */
	DEBUG_2("%s: failed to find PdDv object for %s\n",_PROGRAM,utype)
	return(E_NOPdDv);
}
if ( rc == -1 ) 
{
	/* ODM failure */
	DEBUG_2("%s: ODM failure getting PdDv for %s\n",_PROGRAM,utype)
	return(E_ODMGET);
}

	/*----------------------------------------------------------
	| Search for the child customized device object 
	| Do nothing if it already exists
	|----------------------------------------------------------*/

sprintf(sstring, "PdDvLn=%s AND parent='%s' AND connwhere='%s'",
	utype , pname,connection	);

rc = (int)odm_get_first(cusdev,sstring,cudv);

if ( rc == -1 ) 
{
	/* ODM failure */
	DEBUG_1("%s: ODM failure getting CuDv object\n",_PROGRAM)
	return(E_ODMGET);
}

if ( rc == 0 ) 
{
	/*------------------------------------------------
	|  Does not exist.  Make one.
	|  We already have the predefined from above.
	|------------------------------------------------*/
	
	
	/*----------------------------------------------------------
	|  Run the define methods
	|----------------------------------------------------------*/

	sprintf( sstring, "-c %s -s %s -t %s -p %s -w %s",
		 pddv.class, pddv.subclass, pddv.type,
		 pname, connection);

	DEBUG_3( "%s: Invoking %s method with params:\n%s\n",
		_PROGRAM, pddv.Define, sstring )

	rc = odm_run_method(pddv.Define,sstring,&outp,NULL);

	if ( rc != 0 ) 
	{
		DEBUG_3( "%s: method returned string: '%s rc=%d'\n",
			_PROGRAM, outp, rc );
		return(E_ODMRUNMETHOD);
	}

	/*----------------------------------------------------------
	|  OK, device is customized.  Read out the CuDv contents
	|----------------------------------------------------------*/

	sprintf(sstring,"PdDvLn=%s AND parent='%s' AND connwhere='%s'",
		utype,pname,connection);

	rc = (int) odm_get_first(cusdev,sstring,cudv);

	if ( rc == -1 ) 
	{
		/* ODM failure */
		DEBUG_1("%s: ODM failure getting CuDv object\n", _PROGRAM )
		return(E_ODMGET);
	}
	if ( rc == 0 ) 
	{
		DEBUG_2("%s: No CuDv object for %s\n", _PROGRAM , sstring)
		return(E_NOCuDv);
	}
}
return(0);
}




/************************************************************************
 ************************ FUNCTION HEADER TOP ***************************
 ************************************************************************

 NAME:		define_children

 TITLE:		Determines the existence of all children on the
		sgabus. 

 FUNCTION:	
		The function begins by opening the ODM and making sure
		that the database access is all valid.

		There are two kinds of children presently defined for
		the sgabus.  The first kind were defined in AIX321.
		The sga adapter is the only adapter in the first kind.
		This adapter is not detected in the classical sense of
		using the machine device driver to search I/O space on
		the bus.  Instead, the adapter is found by the IPL ROS
		and the information is passed to the operating system
		via the IPL Control Block.

		If the sga is found, the routine exits, since there is
		only one child on the bus.  Before the routine exits,
		the name of the sga is passed to STDOUT.

		If the sga is not found, then a search is made for children
		of the second type.  This type is defined by the existence
		of Feature ROM Scan code on buid 40, matching the 
		RISC6002 model of FRS.  Here 6002 is used to differentiate
		from 6000 (boot) and 6001 (microchannel).

		If a FRS device is found, attention must be paid to the 
		fact that the ODM may not already hold the contents of
		the device.add file.  FRS provides for the .add file to be
		stored on the ROM.  The file is loaded, and the ODM is
		updated as required.

		After the ODM is updated, the define method is run as for
		the sga.  The name of the adapter is then output to 
		STDOUT as with the sga.
 
 =======================================================================*/

/*-------------------------------------
|
|	FUNCTION DECLARATION
|
|-------------------------------------*/

int
define_children	(
		char *		logical_name,
		int		phase
		)
{
/*--------------------------------------
|
|	DATA DECLARATIONS
|
|--------------------------------------*/

int			ndx;
int			rc;
int			allpkg = 0;

int     		slot,busid,ioaddr,adapter_type,d_length;
char    		str[128];
char *			parent;
char			p_name[32], p_utype[32], d_name[128];
char			sstring[256];
char			pstr[128];
char *			outp;
char *			errp;
mid_t			kmid;
struct cfg_dd 		cfg;
int			howmany;

ulong			found_predef;


IPL_DIRECTORY		iplcb_dir;	/* IPL control block directory 	*/
IPL_INFO		iplcb_info;	/* IPLCB info section 		*/
SGA_DATA		iplcb_sga_data;	/* SGA Post results section 	*/

RSCAN_VIDEO_HEAD *	mem_head;	/* top of FRS ROM		*/

char			file_name[ 256 ];
char			s_child_utype[ 256 ];
char			s_child_method[ 256 ];

char *			child_utype  = s_child_utype;
char *			child_method = s_child_method;

struct CuDv		child_cusobj;
struct PdDv 		child_pddv;		

/*--------------------------------------
|
|	START OF SOURCE CODE
|
|--------------------------------------*/


DEBUG_1("%s: Enter define_children()\n",_PROGRAM);
DEBUG_1("\tlogical_name=%s\n",logical_name);
DEBUG_1("\tphase = %d\n",phase);

/* Get the packaging environment variable */
if (!strcmp(getenv("DEV_PKGNAME"),"ALL"))
    allpkg = 1;

	/*---------------------------------------------
	| Open the ODM classes
	|
	|	cusdev		CuDv
	|	predev		PdDv
	|
	|---------------------------------------------*/

cusdev	= odm_open_class(CuDv_CLASS);

if ( (int) cusdev == -1) 
{
	DEBUG_1("%s: open class CuDv failed", _PROGRAM);
	return(E_ODMOPEN);
}

predev	= odm_open_class(PdDv_CLASS);

if ( (int) predev == -1) 
{
	DEBUG_1("%s: open class PdDv failed", _PROGRAM);
	return(E_ODMOPEN);
}

	/*--------------------------------------------------
	| First we process the type of adapter such as sga.
	|
	| Determine whether the sga type of adapter is 
	| present.  Do this by using the IPLCB
	|
	|---------------------------------------------------*/

	/* Read in the IPL Control Block directory */

rc = mdd_get(  &iplcb_dir, 
		128, 
		sizeof(iplcb_dir), 
		MIOIPLCB	);
if ( rc != 0 ) 
{
	return (E_DEVACCESS);
}


	/* Read in the IPL info section of the Control Block */

rc = mdd_get( 	&iplcb_info, 
		iplcb_dir.ipl_info_offset,
		sizeof(iplcb_info),
		MIOIPLCB	);
if ( rc != 0 ) 
{
	return (E_DEVACCESS);
}

	/* Get SGA post results section from IPL CB */
rc = mdd_get(	&iplcb_sga_data,
		iplcb_dir.sga_post_results_offset,
		sizeof(iplcb_sga_data),
		MIOIPLCB	);
if ( rc != 0 ) 
{
	return (E_DEVACCESS);
}

	/*------------------------------------------------
	| We have everything from the IPLCB.  Now test
	| for sga present
	|------------------------------------------------*/

if (     ( iplcb_sga_data . adapter_present == TRUE )
     &&  ( iplcb_sga_data . detected_error == FALSE )
     &&  ( iplcb_sga_data . adapter_bad    == FALSE )
   )
{
		/*----------------------------------------------
		| found an sga
		| process this case and return
		|
		| define the child device if not already defined
		| if the child is not already available
		| 	update the child location
		|	update te child chgstatus
		|	write the child CuDv back to ODM
		| write the child name to STDOUT
		|----------------------------------------------*/

	DEBUG_1("%s: sga is present \n",_PROGRAM );

	rc = def_child_obj( SGA_UTYPE,		/* uniquetype of child	*/
			   logical_name,	/* parent name		*/
			   "40",			/* connection on sgabus */	
			   &child_cusobj,	/* CuDv of child	*/
			   phase	);

	if ((rc == E_NOPdDv) || allpkg)
		fprintf( stdout , ":devices.sys.sga ");

	if ( rc != 0 )
	{
		DEBUG_2("%s: could not define sga rc=%d", _PROGRAM, rc )
		return (rc);
	}

	if ( child_cusobj.status != AVAILABLE )
	{
		strcpy( child_cusobj.location, "00-0J" );

		if ( child_cusobj.chgstatus == MISSING )
		{
			child_cusobj.chgstatus = SAME;
		}

		rc = odm_change_obj( cusdev, &child_cusobj );

		if ( rc != 0 )
		{
			DEBUG_2("%s: failed to update child CuDv. rc=%d\n",
				_PROGRAM, rc )
			return (E_ODMUPDATE);
		}
	}

	fprintf( stdout , "%s " , child_cusobj.name );

	return (0);
}
	
	/*-----------------------------------------------------------------
	| Fall through to here means we did not detect an sga device
	|
	| Now we look for the second type of device, which is the
	| Feature ROM Scan device.
	|
	|------------------------------------------------------------------*/
		  
rc = FRS_Find_Video_Device(	CFG_GRAPH_BUID_40,
				&mem_head,
				0, 0, 0		);
switch (rc)
{
case E_OK:
		/*----------------------------------------
		| Found a device -- continue in this routine
		|----------------------------------------*/
	DEBUG_1("%s: found FRS device ",_PROGRAM )
	break;


case E_NODETECT:
		/*-----------------------------------------
		| Did not find a device -- exit OK
		|------------------------------------------*/
	DEBUG_1("%s: no FRS device found",_PROGRAM )
	return ( 0 );
	break;

default:
	DEBUG_2("%s: FRS_Find_Video_Device failed with rc=%d\n",_PROGRAM, rc)
	return (rc);
	break;
}

	/*---------------------------------------------------------------
	| Fall through to here means we found an FRS device
	| mem_head points to the contents of the FRS ROM for us
	|
	| Get the ODM adapter.add file from the ROM contents
	| Update the ODM with that file's contents
	| Save the unique type and cfg method name of the child
	|---------------------------------------------------------------*/

rc = FRS_Make_Temp_File(	mem_head,
				CFG_GRAPH_ODM_FILE,
				0444,	/* perms */
				&file_name	);
if ( rc != 0 )
{
	DEBUG_2("%s: FRS_Make_Temp_File for ODM failed with rc= \n",
		_PROGRAM, rc )

	return (rc);
}


	/* determine uniquetype (PdDv.uniquetype) from temp file  */

        get_uniquetype(file_name,child_utype);

	sprintf(sstring, "uniquetype = '%s'", child_utype);
	
	rc = (int) odm_get_first(predev, sstring, &child_pddv);
	
	/* if no PdDv or if allpkg is set then print out pkg name. */
	if ( rc == 0  || allpkg )
	{
		/* No PdDv object for this device */
		DEBUG_2("%s: failed to find PdDv object for %s\n",_PROGRAM,child_utype)

                if (strcmp(child_utype,"adapter/sys/wga") != 0)
		   DEBUG_2("%s: echo wrong package name for %s\n",_PROGRAM,child_utype);  /* this ; is important */

	        fprintf( stdout , ":devices.sys.wga ");

	}
	if ( rc == -1 ) 
	{
		/* ODM failure */
		DEBUG_2("%s: ODM failure getting PdDv for %s\n",_PROGRAM,child_utype)
		return(E_ODMGET);
	}

	

rc = FRS_Update_ODM_From_File(	file_name,
				&child_utype,
				&child_method	);

(void) unlink( file_name );

if ( rc != 0 )
{
	DEBUG_2("%s: FRS_Update_ODM_From_File failed with rc= \n",
		_PROGRAM, rc )

	return (rc);
}

	/*-------------------------------------------------------------
	| Fall through to here means we are ready to define the 
	| child, just as we did for the SGA case
	|
	|-------------------------------------------------------------*/

	/*----------------------------------------------
	| found a FRS adapter
	| process this case and return
	|
	| define the child device if not already defined
	| if the child is not already available
	| 	update the child location
	|	update te child chgstatus
	|	write the child CuDv back to ODM
	| write the child name to STDOUT
	|----------------------------------------------*/

DEBUG_2("%s: FRS adapter %s is present \n",_PROGRAM, child_utype )

rc = def_child_obj( child_utype,	/* uniquetype of child	*/
		   logical_name,	/* parent name		*/
		   "40",		/* connection on sgabus */	
		   &child_cusobj,	/* CuDv of child	*/
		   phase	);

if ( rc != 0 )
{
	DEBUG_2("%s: could not define FRS adapter rc=%d", _PROGRAM, rc )
	return (rc);
}

if ( child_cusobj.status != AVAILABLE )
{
	strcpy( child_cusobj.location, "00-0J" );

	if ( child_cusobj.chgstatus == MISSING )
	{
		child_cusobj.chgstatus = SAME;
	}

	rc = odm_change_obj( cusdev, &child_cusobj );

	if ( rc != 0 )
	{
		DEBUG_2("%s: failed to update child CuDv. rc=%d\n",
			_PROGRAM, rc )
		return (E_ODMUPDATE);
	}
}

fprintf( stdout , "%s " , child_cusobj.name );

(void) odm_close_class(predev);
(void) odm_close_class(cusdev);

return(E_OK);
}



/************************************************************************
 ************************ FUNCTION HEADER TOP ***************************
 ************************************************************************

 NAME:		generate_minor

 FUNCTION:	returns 0
 
 =======================================================================*/

/*-------------------------------------
|
|	FUNCTION DECLARATION
|
|-------------------------------------*/

generate_minor(logical_name,majnum)
	char	*logical_name;
	int	majnum;
{
/*--------------------------------------
|
|	DATA DECLARATIONS
|
|--------------------------------------*/


/*--------------------------------------
|
|	START OF SOURCE CODE
|
|--------------------------------------*/

	return(E_OK);
}

/************************************************************************
 ************************ FUNCTION HEADER TOP ***************************
 ************************************************************************

 NAME:		build_dds

 FUNCTION:	returns 0
 
 =======================================================================*/

/*-------------------------------------
|
|	FUNCTION DECLARATION
|
|-------------------------------------*/

build_dds(logical_name,ddsptr,ddslen)
	char *logical_name;
	char **ddsptr;
	int *ddslen;
{
/*--------------------------------------
|
|	DATA DECLARATIONS
|
|--------------------------------------*/


/*--------------------------------------
|
|	START OF SOURCE CODE
|
|--------------------------------------*/

	return(E_OK);
}

/************************************************************************
 ************************ FUNCTION HEADER TOP ***************************
 ************************************************************************

 NAME:		make_special_files

 FUNCTION:	returns 0
 
 =======================================================================*/

/*-------------------------------------
|
|	FUNCTION DECLARATION
|
|-------------------------------------*/

make_special_files(logical_name,devno)
	char	*logical_name;
	dev_t	devno;
{
/*--------------------------------------
|
|	DATA DECLARATIONS
|
|--------------------------------------*/


/*--------------------------------------
|
|	START OF SOURCE CODE
|
|--------------------------------------*/

	return(E_OK);
}

/************************************************************************
 ************************ FUNCTION HEADER TOP ***************************
 ************************************************************************

 NAME:		download_microcode

 FUNCTION:	returns 0
 
 =======================================================================*/

/*-------------------------------------
|
|	FUNCTION DECLARATION
|
|-------------------------------------*/

download_microcode(logical_name)
	char	*logical_name;
{
/*--------------------------------------
|
|	DATA DECLARATIONS
|
|--------------------------------------*/


/*--------------------------------------
|
|	START OF SOURCE CODE
|
|--------------------------------------*/

	return(E_OK);
}

/************************************************************************
 ************************ FUNCTION HEADER TOP ***************************
 ************************************************************************

 NAME:		query_vpd

 FUNCTION:	returns 0
 
 =======================================================================*/

/*-------------------------------------
|
|	FUNCTION DECLARATION
|
|-------------------------------------*/

query_vpd(newobj,kmid,devno,vpd)
	char	*newobj;
	mid_t	kmid;
	dev_t	devno;
	char	*vpd;
{
/*--------------------------------------
|
|	DATA DECLARATIONS
|
|--------------------------------------*/


/*--------------------------------------
|
|	START OF SOURCE CODE
|
|--------------------------------------*/

	return(E_OK);
}

/*
 * NAME: mdd_get
 *                                                                    
 * FUNCTION: Reads "num_bytes" bytes from nvram, IPL control block, or the
 *	     iocc.  Bytes are read from the address "address" and stored at
 *	     address "dest".
 *                                                                    
 * RETURNS:  error code.  0 means no error.
 */  

int
mdd_get(dest, address, num_bytes, ioctl_type)
char	*dest;
int	address;
int	num_bytes;
int	ioctl_type;
{
	int		fd;		/* file descriptor */
	MACH_DD_IO	mdd;


	if ((fd = open("/dev/nvram",0)) < 0) {
		DEBUG_0("Unable to open /dev/nvram");
		return(E_DEVACCESS);
	}

	DEBUG_2("mdd_get: address=%08x, size=%08x\n",address,num_bytes)
	DEBUG_1("mdd_get: ioctl type = %d\n",ioctl_type)

	mdd.md_addr = address;
	mdd.md_data = dest;
	mdd.md_size = num_bytes;
	mdd.md_incr = MV_BYTE;

	DEBUG_0("Calling mdd ioctl\n")
	if (ioctl(fd,ioctl_type,&mdd)) {
		DEBUG_0("Error reading IPL-Ctrl block")
		return(E_DEVACCESS);
	}

	close(fd);
	return(0);
}


/* 
 *  open the file containing the .add from rom and
 *  extract the uniquetype 
 */
get_uniquetype(fname,utype)
   char * fname;
   char * utype;
{

   FILE * fp;

   char line[256];

   int i,j,len,found;

   i = j = found = 0;
   utype[0] = '\0'; 

   fp = fopen(fname,"r");

   if (fp != NULL)
   {
      /* while not end of file, look for PdDv 
       * this is our anchor point in order
       * to find the PdDv.uniquetype field
       */

      while (get_line(fp,line,&len) != EOF)
      {

	DEBUG_2("get_uniquetype: line=<%s> len=%d\n",line,len);
         if (str_has_sub_str(line,len,"PdDv:"))
            break;
      }

      /* while not end of file, look for  
       * uniquetype = xxx 
       */
      while (get_line(fp,line,&len) != EOF)
      {
	 DEBUG_2("get_uniquetype: line=<%s> len=%d\n",line,len);
         if (str_has_sub_str(line,len,"uniquetype"))
         {
            found = 1;
            break;
         }
      }

      /* found what we have looked for 
       *  (i.e. uniquetype = xxx)
       */
      if (found)
      {
         DEBUG_1("found utype = <%s> \n",line);

         /* uniquetype = "xxx"  so we skip to 
          * the very first "
          */
         while ( (i< len) && (line[i++] != '"') )  
                ;

         DEBUG_1("index =%d\n",i);

         /* skip any space after the first " 
          */ 
         while ( (i< len) && (line[i] == ' ') )  i++; 
                ;

         DEBUG_1("index =%d\n",i);

         /* assume any non blank character after that until
          * either another blank or " is the uniquetype 
          */ 
         while ( (i < len) && (line[i] != ' ') && (line[i] != '"') )
            utype[j++]= line[i++];

         utype[j] = '\0'; 
          
         DEBUG_1("utype=<%s> \n",utype);
      } 
      else
      {
         DEBUG_0("exhausted the file - no match\n");
      }

   }

   fclose(fp);

}

int get_line(fp,bf,len)
FILE *fp;
char * bf;
int * len;
{
   int i = 0;
   int ch;

   DEBUG_0("enter get_line\n");

   ch = getc(fp);
   while ( (ch != EOF) && (ch != '\n') )
   {
      bf[i++] = ch;
      ch = getc(fp);
   } 

   bf[i] = '\0';  /* terminate string */

   *len = i;

   DEBUG_0("exit get_line\n");

   return(ch);   
}

str_has_sub_str(bf,len,sub_str)
char * bf, * sub_str;
int len;
{
   int i,j,found;

   DEBUG_0("enter sub str \n");

   j = strlen(sub_str); 

   i = found = 0;

   while (i < len)
   { 
      if (bf[i] != sub_str[0])  /* match the 1st char */
      {
          i++;
          continue;
      }

      if  (i + j  <= len)    /* is buffer long enough to compare the rest */ 
      {
         if (strncmp(bf+i, sub_str, j) == 0)  /* found sub string */
         {
           found = 1;
           break;
         }
         else
           i += j;   /* skip over next j char.'s already compared */
      }
      else
         break;      /* not long enough so to compare the rest*/

   }

   DEBUG_0("exit sub str \n");

   return(found);
}
