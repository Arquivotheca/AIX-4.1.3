static char sccsid[] = "@(#)75  1.12.2.5  src/bos/usr/lib/methods/cfgmid/cfgmid.c, peddd, bos411, 9428A410j 4/11/94 17:28:48";

/*
 *   COMPONENT_NAME: PEDDD
 *
 *   FUNCTIONS: INT_TO_ARRAY
 *		build_dds
 *		define_children
 *		download_microcode
 *		generate_minor
 *		get_ucode_name
 *		make_special_files
 *		max
 *		query_vpd
 *		search_ucode_dir
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




/*
 * Include files needed for this module follow
 */
#include <sys/types.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

/*
 * odmi interface include files
 */
#include <sys/cfgodm.h>
#include <cf.h>
#include "cfgdebug.h"

/*
 * Local include for hft dds structure
 */
#include "ddsmid.h"


struct ddsmid	*dds;		/* pointer to dds structure to build */
char            vpd[800];       /* vpd data storage */
char            usuffix = ' ';  /* microcode suffix:
					l = LEGA
					2 = LEGA2
					3 = LEGA3
					4 = Ped4
					6 = Ped4_6
					n = Ped w/o Pipe card   */
int             slot_number;    /* adapter slot number */
int             min_ulevel = 0; /* minimum microcode level to use */

int 		search_ucode_dir();

extern int	*genminor();
extern char	*malloc();

#       define  MID_VPD_PPR     0x01    /* host i/f & processor card    */
#       define  MID_VPD_PGR     0x02    /* screen buf & graphics card   */
#       define  MID_VPD_POP     0x04    /* 24 bit option card           */
#       define  MID_VPD_PPC     0x08    /* process pipe card            */
#       define  MAX_VPD_LEN	800	/* maximum length of VPD string */ 

/*
 * Macro for finding max of two values
 */
#define max(x,y)	( (x > y) ? (x) : (y) )

/*
 * Macro for assigning an integer into an array.
 */
#define INT_TO_ARRAY(a,i)  *((char *) (a))      = (i) >> 24; \
			   *((char *) (a) + 1)  = (i) >> 16; \
			   *((char *) (a) + 2)  = (i) >>  8; \
			   *((char *) (a) + 3)  = (i);

/*
 * NAME: build_dds
 *
 * FUNCTION:
 *   build_dds will allocate memory for the dds structure, reporting any
 *   errors, then open the Customized Attribute Class to get the attribute
 *   objects needed for filling the dds structure.
 *
 * EXECUTION ENVIRONMENT:
 *   This routine executes under the device configuration process and
 *   is called from the device independent configuration routine.
 *
 * RETURNS:
 *   The return vales are
 *     0 - for success
 *     positive return code for failure
 * 
 */
build_dds(lname, dds_out, size)
char *lname;                    /* logical name of device */
char **dds_out;                 /* pointer to dds structure for return */
int  *size;                     /* pointer to dds structure size */
{
	char    crit[80];               /* used for search criteria string */
	int             rc;                             /* return code */
	ulong   value;                  /* temp variable to get value into */
	char    temp[256];              /* temporary string variable */
	int     i;                      /* loop control variable */
	struct  CuAt    *cusatt;        /* customized attribute ptr */


	DEBUG_0("entering build_dds\n")
	    /*
	 * Obtain size of device specific dds structure
	 */
	*size = sizeof(struct ddsmid);

	/*
	 * allocate the space required for the dds structure
	 */
	if( (dds = (struct ddsmid *) malloc(*size)) == NULL )
	{
		DEBUG_0("build_dds : malloc failed for dds structure\n")
		    return(E_MALLOC);
	}

	dds->reserved1 = 0;   /* does nothing at this time */

	/*
	 * Get the connection location for this device from the customized
	 * device data.
	 */
	if((rc = get_conn(lname, &value)) != E_OK )
		return(rc);
	dds->slot_number = slot_number = value;

	/*
	 */
	if( get_attrval(lname, "bus_mem_start", (char *)NULL, &value, &rc) == NULL )
		return(rc);
	dds->io_bus_mem_start = value;

	if( get_attrval(lname, "dma1_start", (char *)NULL, &value, &rc) == NULL )
		return(rc);
	dds->dma_range1_start = value;

	if( get_attrval(lname, "dma_channel", (char *)NULL, &value, &rc) == NULL )
		return(rc);
	dds->dma_channel = value;

	if( get_attrval(lname, "int_level", (char *)NULL, &value, &rc) == NULL )
		return(rc);
	dds->int_level = value;

	if( get_attrval(lname, "int_priority", (char *)NULL, &value, &rc) == NULL )
		return(rc);
	dds->int_priority = value;

	if( get_attrval(lname, "scrn_width_mm", (char *)NULL, &value, &rc) == NULL )
		return(rc);
	dds->screen_width_mm = value;

	if( get_attrval(lname, "scrn_height_mm", (char *)NULL, &value, &rc) == NULL)
		return(rc);
	dds->screen_height_mm = value;

	/*
	 * Load the default color palette
	 */
	for( i=0; i<16; i++)
	{
		sprintf(temp, "ksr_color%d", i+1);
		if( get_attrval(lname, temp, (char *)NULL, &value, &rc) == NULL )
			return(rc);
		dds->ksr_color_table[i] = value;
	}

	/*
	 * Obtain the path to the microcode and try opening it.
	 */
	if((rc = get_ucode_name(lname, temp)) != E_OK )
		return(rc);
#if 0
	if( (dds->microcode_path = open(temp, O_RDONLY)) == -1 )
	{
		DEBUG_1("Can't open microcode file : %s\n",temp);
		return(E_NOUCODE);
	}
#endif
	
	strncpy(dds->ucode_name,temp,32);


	/*
	 * Build the display id by obtaining the base ID from ODM then
	 * tacking on the instance number by seeing how many times this
	 * displays device driver has been configured.
	 * NOTE: This get_attrval depends on get_ucode_name() to init the
	 * usuffix variable.
	 */
	if((cusatt = get_attrval(lname, "display_id", (char *)NULL, &value, &rc))
	    == NULL )
		return(rc);

	dds->display_id = value;

	if ( usuffix == '2' ) dds->display_id = 0x042A0000;
	else if ( usuffix == '3' ) dds->display_id = 0x042B0000;
	else dds->display_id = 0x04290000;

	if((rc = get_instance(lname, &value)) != E_OK )
		return(rc);
	dds->display_id |= value;
	sprintf(cusatt->value, "0x%08x", dds->display_id);
	if (putattr(cusatt) < 0)
		return(E_ODMUPDATE);

	/* 
	 * Get refresh rate and store it into the dds
	 */

	if((cusatt = get_attrval(lname,"refresh_rate",(char *)NULL,&value,&rc))
	    == NULL )
		return(rc);

	if ( usuffix == 'l' )
	{
		value = 60;
		sprintf(cusatt->value, "%d", value);
	}

	dds->refresh_rate = value;
	if (putattr(cusatt) < 0)
		return(E_ODMUPDATE);

	/* If adapter == Gt4, then create a custom attr. for changeable
	 * refresh rate 
	*/

	if((cusatt = get_attrval(lname,"chg_ref_rate",temp,value,&rc))
			== NULL )
		return(rc);
	if ( usuffix != 'l' )
	{
		strcpy(cusatt->value, "y");
		if (putattr(cusatt) < 0)
			return(E_ODMUPDATE);
	}
	else
	{
		strcpy(cusatt->value, "n");
		if (putattr(cusatt) < 0)
			return(E_ODMUPDATE);
	}


	if( get_attrval(lname, "belongs_to", temp, &value, &rc) == NULL )
		return(rc);
	else
	{
		/*
		 * Build a customized dependency object for this display 
		 * and the hft it belongs to.
		 */
		if( (rc = build_depend(temp, lname)) < 0 )
		{
			DEBUG_1("Could not add dependency for hft -> %s\n",lname)
			    return(rc);
		}
	}

	/*
	  Copy the logical device name to dds for error logging by device.
	*/
	strcpy(dds->component, lname);

	*dds_out = (char *)dds;

	DEBUG_0("leaving build_dds\n")
	    return(E_OK);
}




/*
 * NAME: generate_minor
 *
 * FUNCTION: Device dependent routine for generating the device minor number
 *
 * EXECUTION ENVIRONMENT:
 *   This routine executes under the device configuration process and
 *   is called from the device independent configuration routine.
 *
 * RETURNS:
 *   0 success
 *   positive return code on failure
 */
int
generate_minor(lname, majno, minorno)
char  *lname;     /* logical device name */
long  majno;      /* device major number */
long  *minorno;   /* device minor number */
{
	long	*minorptr;

	DEBUG_0("entering generate_minor\n")
	    minorptr = genminor(lname, majno, -1, 1, 1, 1);

	if( minorptr == (long *)NULL )
		return(E_MINORNO);

	*minorno = *minorptr;
	DEBUG_0("leaving generate_minor\n")
	    return(E_OK);
}




/*
 * NAME: make_special_files
 *
 * FUNCTION: Device dependent routine creating the devices special files
 *   in /dev
 *
 * EXECUTION ENVIRONMENT:
 *   This routine executes under the device configuration process and
 *   is called from the device independent configuration routine.
 *
 * RETURNS:
 *   0 - success
 */
int
make_special_files(lname, devno)
char  *lname;     /* logical device name */
dev_t devno;      /* major/minor number */
{
	return(E_OK);
}




/*
 * NAME: download_microcode
 * 
 * FUNCTION: Device dependent routine for downloading microcode
 *
 * EXECUTION ENVIRONMENT:
 *   This routine executes under the device configuration process and
 *   is called from the device independent configuration routine.
 *
 * NOTES: This routine will always return success for this device instance
 *
 * RETURNS:
 *   0 - success
 */
int
download_microcode(lname)
char  *lname;     /* logical device name */
{
	return(E_OK);
}



/*
 * NAME: query_vpd
 * 
 * FUNCTION: Device dependent routine for obtaining VPD data for a device
 *                                           
 * EXECUTION ENVIRONMENT:
 *   This routine executes under the device configuration process and
 *   is called from the device independent configuration routine.
 *
 * NOTES:
 *
 * RETURNS: 
 *   0 - success
 *  -1 - failure
 */
int
query_vpd(cusobj, kmid, devno, ppr_vpd)
struct  CuDv    *cusobj;        /* customized device object */
mid_t           kmid;           /* driver module id */
dev_t           devno;          /* major/minor number */
char            *ppr_vpd;       /* vpd passed back to generic config piece */
{
	struct	cfg_dd	cfgdata;		/* sysconfig call structure */
	int                     rc;             /* return code */
	register        int     i;              /* loop variable */
	ulong           uversion;               /* version */
	ulong           uconfig;                /* card configuration */
	/* 0x01  PPR */
	/* 0x02  PGR */
	/* 0x04  POP */
	char            *ppr=NULL,              /* pointer to PPR VPD */
	*pgr=NULL,              /* pointer to PGR VPD */
	*pop=NULL;              /* pointer to POP VPD */
	char		*define_child(),	/* define a child (make CuDv object) */
	*temp,                  /* temporary char pointer */
	*vpd_desc;              /* the vpd descriptor */

	DEBUG_0("entering query_vpd\n")

	    /*
	 * Set up the structure for the VPD sysconfig call.
	 * At the point that this routine is being called, the dds for
	 * the mid device driver may not yet have been passed down to the
	 * driver. The device driver has therefore not received the slot
	 * number which it requires to read the VPD. This slot number
	 * will be passed in the first word of the following vpd[] buffer.
	 * Make the call
	 * and then extract the version and configuration variables from
	 * the VPD field.  The VPD data is then passed over once to strip
	 * the unwanted null characters from it.
	 */
	cfgdata.kmid = kmid;
	cfgdata.devno = devno;
	cfgdata.cmd = CFG_QVPD;
	INT_TO_ARRAY(&vpd[0], slot_number);
	cfgdata.ddsptr = (char *)vpd;
	cfgdata.ddslen = MAX_VPD_LEN;

	if( (rc = sysconfig( SYS_CFGDD, &cfgdata, sizeof(struct cfg_dd) ))
	    == -1 )
	{
		DEBUG_0("query_vpd: sysconfig failed\n");
		return(E_VPD);
	}
	else
	{
		dds->hwconfig = uconfig =
		    ((((vpd[776]<<24)+vpd[777]<<16)+vpd[778]<<8)+vpd[779]);
		for(i=0; i<MAX_VPD_LEN; i++)
			if( vpd[i] == '\0' ) vpd[i] = ' ';
	}

	if( uconfig & MID_VPD_PPR )
	{
		i = vpd + (MAX_VPD_LEN - 3);

		for( temp = vpd; temp <= i && strncmp(temp,"VPD",3); temp++ );

		if (temp >= i)
		{
	   		/* Something is wrong: we didn't find the VPD string */
			return(E_VPD);
		}

		/* here we have found "VPD".  We need to skip over 7 bytes, 3
           	   over "VPD", 2 over the length field, and 2 over the CRC
           	   field to get to the very first '*' (i.e, the very first
           	   VPD descriptor).  For some adapter, the CRC might have a 
                   byte whose value happens to be the same as that of '*'.  
                   All the work above is to make sure when we do the search 
                   after the CRC field to eliminate the confusion
        	*/

		temp += 7;
		ppr = temp;

		while( *temp == '*' )
			temp += (((int)*(temp+3)) << 1);
		*temp = '\0';

		strcpy(ppr_vpd, ppr);

		/*
		 * Extract the minimum microcode level from the VPD
		 * and update the level if this card is higher.
		 */
		if( *(vpd_desc = get_vpd_descriptor(ppr, "LL")) )
			min_ulevel = max(min_ulevel, atoi(vpd_desc));

		/*
		 * Extract the card displayable name from the VPD.
		 */
		if( *(vpd_desc = get_vpd_descriptor(ppr, "DS")) )
			if (!strncmp (vpd_desc, "GRAPHIC LEGA", 12))
			{
				usuffix = 'l';
				DEBUG_0("leaving query_vpd\n")
				return(E_OK);
			}
			else if (!strncmp (vpd_desc, " GRAPHIC 2D LEGA", 16))
			{
				usuffix = '2';
				DEBUG_0("leaving query_vpd\n")
				return(E_OK);
			}
			else if (!strncmp (vpd_desc, " GRAPHIC 3D LEGA", 16))
			{
				usuffix = '3';
				DEBUG_0("leaving query_vpd\n")
				return(E_OK);
			}
			else if (!strncmp (vpd_desc, " GRAPHIC PP2", 12))
			{
				usuffix = '4';
			}
			else if (!strncmp (vpd_desc, " GRAPHIC PP6", 12))
			{
				usuffix = '6';
			}
			else if( uconfig & MID_VPD_PPC )
			{
				if ((rc = define_child( "ppc", cusobj->name,
					"")) != E_OK) return(rc);
			}
			else usuffix = 'n';

		if( uconfig & MID_VPD_PGR )
		{
			temp = pgr = (ppr + 256);

			for ( i = 1; (*temp != '*') && (i < 256) ;
			    temp++, i++ );
			while( *temp == '*' )
				temp += (((int)*(temp+3)) << 1);
			*temp = '\0';

			/*
			 * Extract the minimum microcode level from the VPD
			 * and update the level if this card is higher.
			 */
			if( *(vpd_desc = get_vpd_descriptor(pgr, "LL")) )
				min_ulevel = max(min_ulevel, atoi(vpd_desc));

			if( *(vpd_desc = get_vpd_descriptor(pgr, "DS")) )
				if (!strncmp (vpd_desc, " GRAPHIC PGR 8", 14))
				{
					if ((rc = define_child("pge",
						cusobj->name, pgr)) != E_OK)
						return(rc);
					return(E_OK);
				} else
				if (!strncmp (vpd_desc, " GRAPHIC PGR24", 14))
				{
					if ((rc = define_child("pgt",
						cusobj->name, pgr)) != E_OK)
						return(rc);
					return(E_OK);
				}
			if ((rc = define_child("pgr", cusobj->name, pgr))
				!= E_OK) return(rc);

			if( uconfig & MID_VPD_POP )
			{
				temp = pop = (pgr + 256);

				for ( i = 1; (*temp != '*') && (i < 256) ;
				    temp++, i++ );

				while( *temp == '*' )
					temp += (((int)*(temp+3)) << 1);
				*temp = '\0';

				/*
				 * Extract the minimum microcode level from
				 * the VPD and update the level if this card
				 * is higher.
				 */
				if( *(vpd_desc = get_vpd_descriptor(pop,
				    "LL")) )
					min_ulevel = max(min_ulevel,
					    atoi(vpd_desc));
				if ((rc = define_child("pop", cusobj->name,
				    pop)) != E_OK) return(rc);
			}

		}
		else
		{
		/*-------------------------------------------------------*
		 * Fall through to here when there is a PPR but no PGR	 *
		 * This is only valid for Gt3                            *
		 *-------------------------------------------------------*/

			if ( usuffix != 'l' )
			{
				DEBUG_0("Bad configuration setup in VPD\n")
				    return(E_VPD);
			}
		}
	}
	else
	{
		DEBUG_0("Bad configuration setup in VPD\n");
		return(E_VPD);
	}

	DEBUG_0("leaving query_vpd\n")
	    return(E_OK);
}




/*
 * NAME: define_children
 *                                                                    
 * FUNCTION: Routine for detecting and managing children of a logical device
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *   This routine executes under the device configuration process and
 *   is called from the device independent configuration routine.
 *                                                                   
 * NOTES: This routine will always return success for this device instance
 *
 * RETURNS: 
 *   0 - success
 */
int
define_children(lname, phase)
char  *lname;     /* logical device name */
int   phase;      /* phase of ipl : 0=RUN, 1=PHASE_1, 2=PHASE_2 */
{
	return(E_OK);
}

/*
 * NAME: get_ucode_name
 *
 * FUNCTION: This routine finds the latest version of a given level
 *           of microcode for this device. The file pathname is passed
 *           back in the 'file' parameter.
 *
 * EXECUTION ENVIRONMENT:
 *   This routine is local to this module.
 *
 * INPUT:  lname is device logical name.
 *
 * NOTES:
 *
 * RETURNS:
 *   0 - success
 *   positive return code for failure
 */

int
get_ucode_name(lname, file)
char *lname;
char *file;
{
	struct  cfg_dd  cfgdata;/* sysconfig call structure */
	char    crit[256];      /* used to build search criteria */
	struct  CuDv    cudv;   /* customized device */
	struct  CuDvDr  cudvdr; /* customized device driver, for major/minor # */
	struct  PdDv    pddv;   /* predefined device */
	struct  PdAt    pdat;   /* predefined attribute */
	struct  CuAt    cuat;   /* customized attribute */
	dev_t   devno;
	mid_t   kmid;           /* driver module id */
	char    filename[256];
	int     rc;
char dd_name[50];
	DEBUG_0("entering get_ucode_name\n")

	    /*
	 * Make the devno from the major and minor numbers in the customized
	 * device driver object.
	 */
	sprintf(crit, "value3 = '%s' AND resource = devno", lname);
	rc = (int)odm_get_first(CuDvDr_CLASS, crit, &cudvdr);
	if( rc == 0 ) {
		DEBUG_1("get_ucode_name: can't find cudvdr object crit=%s\n",
		    crit);
		return(E_NOCuOBJ);
	}
	else if( rc == -1 ) {
		DEBUG_1("get_ucode_name: error getting cudvdr object crit=%s\n",
		    crit);
		return(E_ODMGET);
	}

	devno = makedev((ulong)strtol(cudvdr.value1,(char **)NULL, 0),
	    (ulong)strtol(cudvdr.value2,(char **)NULL, 0));

	sprintf(crit, "name = '%s'", lname);
	rc = odm_get_first(CuDv_CLASS, crit, &cudv);
	if (rc == 0 || rc == -1) {
		DEBUG_1("failed to get CuDv object for %s\n", lname);
		return(E_NOCuDv);
	}

	sprintf(crit, "uniquetype = '%s'", cudv.PdDvLn_Lvalue);
	rc = odm_get_first(PdDv_CLASS, crit, &pddv);
	if (rc == 0 || rc == -1) {
		DEBUG_1("failed to get PdDv object for %s\n", lname);
		return(E_NOPdDv);
	}

	/*
	 * Delete microcode filename attribute from CuAt.
	 */
	sprintf(crit, "name = '%s' AND attribute = 'microcode'", lname);
	odm_rm_obj(CuAt_CLASS, crit);

	DEBUG_1("get_adapter_level: pddv.dvrd =%s\n",pddv.DvDr);
	strcat( (strcpy( dd_name, "/usr/lib/drivers/")), pddv.DvDr );
	DEBUG_1("get_adapter_level: new dd name =%s\n",dd_name);
	/* if ((kmid = loadext(pddv.DvDr, FALSE, TRUE)) == NULL) { */
	if ((kmid = loadext(dd_name, FALSE, TRUE)) == NULL) {
		DEBUG_0("get_adapter_level: error getting kmid\n")
		    return(E_LOADEXT);
	}
	DEBUG_1("get_adapter_level: kmid  =%x\n",kmid);

	/*
	 * Get the adapter level.
	 */
	rc = query_vpd(&cudv, kmid, devno, vpd);
	if (rc != 0) {
		DEBUG_0("get_ucode_name: query_vpd returning error\n");
		return(rc);
	}

	/*
	 * Search /etc/microcode directory for appropriate microcode
	 * file.
	 */
	if ( search_ucode_dir( lname, filename ) != E_OK  ) {
		DEBUG_1("microcode file not found:lname=%s %s\n",lname);
		DEBUG_1("microcode file not found:filename= %s\n",filename);
		return(E_NOUCODE);
	}

	strcpy(file, filename);

	DEBUG_0("leaving get_ucode_name\n")
	    return(0);

}


/*
 * NAME : search_ucode_dir
 *
 * FUNCTION :
 *		(1) Searches /etc/microcode for a file matching the correct
 *		    pattern.  If found, the file name is written into the
 *		    string identified by the input parameter "file".
 *
 *		(2) Updates the customized attributes for the two cases where
 *                  the device is a Gt4 or Gt4X.  The predefined attributes
 *                  assume the device is a Gt3.
 *
 *		(3) Returns an integer which represents the error value 
 *		    or E_OK if no error.
 *
 * INPUT :
 *          lname       The logical name of the adapter    
 *	    file	pointer to a string for use on output
 *	    
 *	    usuffix	global variable identifying whether the adapter is
 *                      a Gt3, Gt4, or Gt4X
 *	    min_ulevel	holds the LL (load level) value of the VPD required
 *			for the microcode.  This value is encoded into the
 *			microcode file name
 *
 * OUTPUT:
 *          file        pointer to a string holding the file name
 *
 * RETURNS: an integer holding E_OK if success, otherwise an error code
 *
 * NOTE:
 *      The format for microcode file names is:
 *              cardidS.LL.VV
 *      cardid is the hex id of mid device
 *      S  is the microcode suffix (l - Gt3, n - Gt4, nothing - Gt4X )
 *      LL is revision level of the adapter from the LL field of
 *         the VPD
 *      VV is version number of the microcode
 */

#define	FILENAMESIZE	32
int   
search_ucode_dir(lname, file)

char    *lname;			/* input parm: logical adapter */
char    *file;			/* output parm: microcode file */

{
	char    curfile[FILENAMESIZE];          /* cardid and LL value      */
	char    tmpfile[FILENAMESIZE];          /* cardid and LL value      */
	int     rc;                     	/* return code              */
	ulong   ulong_val;
	struct	CuAt *	cusatt;
	char    string_val[256];



	DEBUG_0("entering search_ucode_dir\n")



	    rc = E_OK;		/* preset the rc */


	/*------------------------------------------------------|
	 |	make sure we have valid inputs			|
	 |------------------------------------------------------*/
	if ( (file == NULL ) || ( lname == NULL ) )
	{
		rc = -1;
		return( rc );
	}


	/*-----------------------------------------------------------|
	 | Create basic mask to start searching the directory with.  |
	 | 	curfile holds the mask				     |
	 |-----------------------------------------------------------*/

	if ( usuffix == 'l' )
	{
		/*----------------------------------------------|
		 | If using the full functional device driver	|
		 | and this is a Gt3.                           |
		 |----------------------------------------------*/

		/* make a tmp copy of curfile */
		strncpy(tmpfile,curfile,FILENAMESIZE - 2);
		/* add the runtime Gt3 ucode name to it */
		sprintf(tmpfile, "8ee3l.%02d", min_ulevel);
		/* If the runtime Gt3 ucode is found */
		if (findmcode(tmpfile,file,VERSIONING,0))
		{
			/* Add its name to curfile */
			sprintf(curfile, "8ee3l.%02d", min_ulevel);

		} else
		{
			/* otherwise try the boot Gt3 ucode file */
			sprintf(curfile, "8ee3lb.%02d", min_ulevel);
			if (findmcode(curfile,file,VERSIONING,0))
			{
				DEBUG_1("search_ucode_dir: microcode file=%s\n"
				    , file);
			} else
			{

			/*----------------------------------------------------|
			 | if we do not find the file then return an error    |
			 |----------------------------------------------------*/

				DEBUG_0(
				    "search_ucode_dir:No microcode file found\n");
				rc 	= E_NOUCODE;
				return( rc );

			}
		}

	}

	else if ( usuffix == '2' )
	{
		/*----------------------------------------------|
		 | If using the full functional device driver   |
		 | and this is a Gt3i (2D).                     |
		 |----------------------------------------------*/

		/* make a tmp copy of curfile */
		strncpy(tmpfile,curfile,FILENAMESIZE - 2);
		/* add the runtime Gt3i ucode name to it */
		sprintf(tmpfile, "8ee3l2.%02d", min_ulevel);
		/* If the runtime Gt3i ucode is found */
		if (findmcode(tmpfile,file,VERSIONING,0))
		{
			/* Add its name to curfile */
			sprintf(curfile, "8ee3l2.%02d", min_ulevel);

		} else
		{
			/* otherwise try the boot Gt3i ucode file */
			sprintf(curfile, "8ee3l2b.%02d", min_ulevel);
			if (findmcode(curfile,file,VERSIONING,0))
			{
				DEBUG_1("search_ucode_dir: microcode file=%s\n"
				    , file);
			} else
			{

			/*----------------------------------------------------|
			 | if we do not find the file then return an error    |
			 |----------------------------------------------------*/

				DEBUG_0(
				    "search_ucode_dir:No microcode file found\n");
				rc      = E_NOUCODE;
				return( rc );

			}
		}

	}

	else if ( usuffix == '3' )
	{
		/*----------------------------------------------|
		 | If using the full functional device driver   |
		 | and this is a Gt4e (3D).                     |
		 |----------------------------------------------*/

		/* make a tmp copy of curfile */
		strncpy(tmpfile,curfile,FILENAMESIZE - 2);
		/* add the runtime Gt4e ucode name to it */
		sprintf(tmpfile, "8ee3l3.%02d", min_ulevel);
		/* If the runtime Gt4e ucode is found */
		if (findmcode(tmpfile,file,VERSIONING,0))
		{
			/* Add its name to curfile */
			sprintf(curfile, "8ee3l3.%02d", min_ulevel);

		} else
		{
			/* otherwise try the boot Gt4e ucode file */
			sprintf(curfile, "8ee3l3b.%02d", min_ulevel);
			if (findmcode(curfile,file,VERSIONING,0))
			{
				DEBUG_1(
				"search_ucode_dir: microcode file=%s\n",
				file);
			} else
			{
				/* otherwise try the boot Gt3i ucode file */
				sprintf(curfile, "8ee3l2b.%02d", min_ulevel);
				if (findmcode(curfile,file,VERSIONING,0))
				{
					DEBUG_1(
					"search_ucode_dir: microcode file=%s\n",
					file);
				} else
				{

				/*----------------------------------------------------|
				| if we do not find the file then return an error    |
				|----------------------------------------------------*/

				DEBUG_0(
				"search_ucode_dir:No microcode file found\n");
				rc      = E_NOUCODE;
				return( rc );

				}
			}
		}

	}

	else if ( usuffix == 'n' ||
		   usuffix == '4' )
	{
		/*----------------------------------------------|
		 | If not Gt3 then if it is a Gt4               |
		 |----------------------------------------------*/

		/* make a tmp copy of curfile */
		strncpy(tmpfile,curfile,FILENAMESIZE - 2);
		/* add the runtime Gt4 ucode name to it */
		sprintf(tmpfile, "8ee3n.%02d", min_ulevel);
		/* If the runtime Gt4 ucode is found */
		if (findmcode(tmpfile,file,VERSIONING,0))
		{
			/* Add its name to curfile */
			sprintf(curfile, "8ee3n.%02d", min_ulevel);

		} else
		{
			/* otherwise try the boot Gt4 ucode file */
			sprintf(curfile, "8ee3nb.%02d", min_ulevel);
			if (findmcode(curfile,file,VERSIONING,0))
			{
				DEBUG_1("search_ucode_dir: microcode file=%s\n"
				    , file);
			} else
			{

			/*----------------------------------------------------|
			 | if we do not find the file then return an error    |
			 |----------------------------------------------------*/

				DEBUG_0(
				    "search_ucode_dir:No microcode file found\n");
				rc 	= E_NOUCODE;
				return( rc );

			}
		}

	}
	else 
	{
		/*----------------------------------------------|
		 | If neither of those then assume Gt4X         |
		 |----------------------------------------------*/

		/* make a tmp copy of curfile */
		strncpy(tmpfile,curfile,FILENAMESIZE - 2);
		/* add the runtime Gt4X ucode name to it */
		sprintf(tmpfile, "8ee3.%02d", min_ulevel);
		/* If the runtime Gt4X ucode is found */
		if (findmcode(tmpfile,file,VERSIONING,0))
		{
			/* Add its name to curfile */
			sprintf(curfile, "8ee3.%02d", min_ulevel);

		} else
		{
			/* otherwise try the boot Gt4X ucode file */
			sprintf(curfile, "8ee3xb.%02d", min_ulevel);
			if (findmcode(curfile,file,VERSIONING,0))
			{
				DEBUG_1("search_ucode_dir: microcode file=%s\n"
				    , file);
			} else
			{

			/*----------------------------------------------------|
			 | if we do not find the file then return an error    |
			 |----------------------------------------------------*/

				DEBUG_0(
				    "search_ucode_dir:No microcode file found\n");
				rc 	= E_NOUCODE;
				return( rc );

			}
		}
	}


	DEBUG_1("search_ucode_dir: curfile=%s\n",curfile)


	 /*--------------------------------------------------------------|
	 | The predefined attributes assume the adapter is a Gt3.  If   |
	 | we get here, we have OK microcode.  So, check the attributes	|
	 | and update the customized attributes.                        |
	 |--------------------------------------------------------------*/


	if ( usuffix == 'l' )

	/*------------------------------------------------------|
	 | if it is a Gt3                                       |
	 |------------------------------------------------------*/

	{

		/*------------------------------------------------------|
		 | check if the customized attributes used the 		|
		 | _subtype convention to define which type of adapter	|
		 | we have (Gt3, Gt4, or Gt4X )                         |
		 |------------------------------------------------------*/

		if ( (( cusatt = get_attrval( lname, "_subtype", string_val,
		    &ulong_val, &rc ) ) != NULL )
		    &&
		    ( rc == E_OK ) )
		{

			/*----------------------------------------------|
			 | we got a pointer, therefore we  have a 	|
			 | _subtype attribute.  Update the value.    	|
			 | CuAt updates always take strings as values.	|
			 |----------------------------------------------*/

			strcpy( cusatt->value, "30" );
			if ( putattr( cusatt ) < 0 )
			{
				rc = E_ODMUPDATE;
				return( rc ) ;
			}
		}

		/*------------------------------------------------------|
		 |      Next check the "subtype"                        |
		 |                                                      |
		 |------------------------------------------------------*/

		if ( (( cusatt = get_attrval( lname, "subtype", string_val,
		    &ulong_val, &rc ) ) != NULL )
		    &&
		    ( rc == E_OK) )
		{

			strcpy( cusatt->value, "Entry" );
			if (putattr(cusatt) < 0)
			{
				rc = E_ODMUPDATE;
				return( rc );
			}
		}

		/*------------------------------------------------------|
		 |      Next check the "microcode"                      |
		 |                                                      |
		 |------------------------------------------------------*/

		if ( (( cusatt = get_attrval( lname, "microcode", string_val,
		    &ulong_val, &rc ) ) != NULL )
		    &&
		    ( rc == E_OK ) )
		{

			strcpy( cusatt->value, file );
			if (putattr(cusatt) < 0)
			{
				rc = E_ODMUPDATE;
				return( rc );
			}
		}

		/*------------------------------------------------------|
		 |      Next check the "dsp_name"                       |
		 |                                                      |
		 |------------------------------------------------------*/

		if ( (( cusatt = get_attrval( lname, "dsp_name", string_val,
		    &ulong_val, &rc ) ) != NULL )
		    &&
		    ( rc == E_OK ) )
		{
			strcpy( cusatt->value, "POWER_Gt3" );

			if ( putattr( cusatt ) < 0 )
			{
				rc = E_ODMUPDATE;
				return( rc );
			}
		}

	}

	/*------------------------------------------------------|
	 | if it is a Gt3i                                      |
	 |------------------------------------------------------*/

	else if ( usuffix == '2' )

	{


		/*------------------------------------------------------|
		 | walk through a set of attributes and update the      |
		 | values of the customized object's attributes         |
		 |------------------------------------------------------*/


		/*------------------------------------------------------|
		 |      First check the "_subtype"                      |
		 |                                                      |
		 |      Gt3i uses "54" as the value                     |
		 |------------------------------------------------------*/

		if ( (( cusatt = get_attrval( lname, "_subtype", string_val,
		    &ulong_val, &rc ) ) != NULL)
		    &&
		    ( rc == E_OK ) )
		{

			/*----------------------------------------------|
			 | we got a pointer, therefore we  have a       |
			 | _subtype attribute.  Update the value.       |
			 |----------------------------------------------*/

			strcpy( cusatt->value, "54" );
			if (putattr(cusatt) < 0)
			{
				rc = E_ODMUPDATE;
				return( rc );
			}
		}

		/*------------------------------------------------------|
		 |      Next check the "subtype"                        |
		 |                                                      |
		 |------------------------------------------------------*/

		if ( (( cusatt = get_attrval( lname, "subtype", string_val,
		    &ulong_val, &rc ) ) != NULL )
		    &&
		    ( rc == E_OK) )
		{

			strcpy( cusatt->value, "Entry2" );
			if (putattr(cusatt) < 0)
			{
				rc = E_ODMUPDATE;
				return( rc );
			}
		}

		/*------------------------------------------------------|
		 |      Next check the "microcode"                      |
		 |                                                      |
		 |------------------------------------------------------*/

		if ( (( cusatt = get_attrval( lname, "microcode", string_val,
		    &ulong_val, &rc ) ) != NULL )
		    &&
		    ( rc == E_OK ) )
		{

			strcpy( cusatt->value, file );
			if (putattr(cusatt) < 0)
			{
				rc = E_ODMUPDATE;
				return( rc );
			}
		}

		/*------------------------------------------------------|
		 |      Next check the "dsp_name"                       |
		 |                                                      |
		 |------------------------------------------------------*/

		if ( (( cusatt = get_attrval( lname, "dsp_name", string_val,
		    &ulong_val, &rc ) ) != NULL )
		    &&
		    ( rc == E_OK ) )
		{

			strcpy( cusatt->value, "POWER_Gt3i" );

			if ( putattr( cusatt ) < 0 )
			{
				rc = E_ODMUPDATE;
				return( rc );
			}
		}
	}
	/*------------------------------------------------------|
	 | if it is a Gt4e                                      |
	 |------------------------------------------------------*/

	else if ( usuffix == '3' )

	{


		/*------------------------------------------------------|
		 | walk through a set of attributes and update the      |
		 | values of the customized object's attributes         |
		 |------------------------------------------------------*/


		/*------------------------------------------------------|
		 |      First check the "_subtype"                      |
		 |                                                      |
		 |      Gt4e uses "55" as the value                     |
		 |------------------------------------------------------*/

		if ( (( cusatt = get_attrval( lname, "_subtype", string_val,
		    &ulong_val, &rc ) ) != NULL)
		    &&
		    ( rc == E_OK ) )
		{

			/*----------------------------------------------|
			 | we got a pointer, therefore we  have a       |
			 | _subtype attribute.  Update the value.       |
			 |----------------------------------------------*/

			strcpy( cusatt->value, "55" );
			if (putattr(cusatt) < 0)
			{
				rc = E_ODMUPDATE;
				return( rc );
			}
		}

		/*------------------------------------------------------|
		 |      Next check the "subtype"                        |
		 |                                                      |
		 |------------------------------------------------------*/

		if ( (( cusatt = get_attrval( lname, "subtype", string_val,
		    &ulong_val, &rc ) ) != NULL )
		    &&
		    ( rc == E_OK) )
		{

			strcpy( cusatt->value, "Entry3" );
			if (putattr(cusatt) < 0)
			{
				rc = E_ODMUPDATE;
				return( rc );
			}
		}

		/*------------------------------------------------------|
		 |      Next check the "microcode"                      |
		 |                                                      |
		 |------------------------------------------------------*/

		if ( (( cusatt = get_attrval( lname, "microcode", string_val,
		    &ulong_val, &rc ) ) != NULL )
		    &&
		    ( rc == E_OK ) )
		{

			strcpy( cusatt->value, file );
			if (putattr(cusatt) < 0)
			{
				rc = E_ODMUPDATE;
				return( rc );
			}
		}

		/*------------------------------------------------------|
		 |      Next check the "dsp_name"                       |
		 |                                                      |
		 |------------------------------------------------------*/

		if ( (( cusatt = get_attrval( lname, "dsp_name", string_val,
		    &ulong_val, &rc ) ) != NULL )
		    &&
		    ( rc == E_OK ) )
		{

			strcpy( cusatt->value, "POWER_Gt4e" );

			if ( putattr( cusatt ) < 0 )
			{
				rc = E_ODMUPDATE;
				return( rc );
			}
		}

	}
	/*------------------------------------------------------|
	 | if it is a Gt4 or Gt4X                               |
	 |------------------------------------------------------*/

	else {


		/*------------------------------------------------------|
		 | walk through a set of attributes and update the 	|
		 | values of the customized object's attributes         |
		 |------------------------------------------------------*/


		/*------------------------------------------------------|
		 |	First check the "_subtype"                    	|
		 |                                                      |
		 |      Gt4 and Gt4X both use "32" as the value,        |
		 |      the single card Gt4 uses "58",                  |
		 |      the single card Gt4x uses "59".                 |
		 |------------------------------------------------------*/

		if ( (( cusatt = get_attrval( lname, "_subtype", string_val,
		    &ulong_val, &rc ) ) != NULL)
		    && 
		    ( rc == E_OK ) )
		{

			/*----------------------------------------------|
			 | we got a pointer, therefore we  have a 	|
			 | _subtype attribute.  Update the value.    	|
			 |----------------------------------------------*/

			if ( usuffix == '4' )
				strcpy( cusatt->value, "58" );
			else if ( usuffix == '6' )
				strcpy( cusatt->value, "59" );
			else strcpy( cusatt->value, "32" );

			if (putattr(cusatt) < 0)
			{
				rc = E_ODMUPDATE;
				return( rc );
			}
		}

		/*------------------------------------------------------|
		 |	Next check the "subtype"                      	|
		 |                                                      |
		 |------------------------------------------------------*/

		if ( (( cusatt = get_attrval( lname, "subtype", string_val,
		    &ulong_val, &rc ) ) != NULL )
		    &&
		    ( rc == E_OK) )
		{

			if ( usuffix == '4' )
				strcpy( cusatt->value, "Other2" );
			else if ( usuffix == '6' )
				strcpy( cusatt->value, "Other6" );
			else strcpy( cusatt->value, "Other" );
			if (putattr(cusatt) < 0)
			{
				rc = E_ODMUPDATE;
				return( rc );
			}
		}

		/*------------------------------------------------------|
		 |	Next check the "microcode"                     	|
		 |                                                      |
		 |------------------------------------------------------*/

		if ( (( cusatt = get_attrval( lname, "microcode", string_val,
		    &ulong_val, &rc ) ) != NULL )
		    &&
		    ( rc == E_OK ) )
		{

			strcpy( cusatt->value, file );
			if (putattr(cusatt) < 0)
			{
				rc = E_ODMUPDATE;
				return( rc );
			}
		}

		/*------------------------------------------------------|
		 |	Next check the "dsp_name"                     	|
		 |                                                      |
		 |------------------------------------------------------*/

		if ( (( cusatt = get_attrval( lname, "dsp_name", string_val,
		    &ulong_val, &rc ) ) != NULL )
		    &&
		    ( rc == E_OK ) )
		{

			/*----------------------------------------------|
			 | For the display name, need to differentiate  |
			 | between Gt4 and Gt4X                         |
			 |----------------------------------------------*/

			if ( usuffix == 'n' )
			{
				/*--------------------------------------|
				 | If a Gt4                             |
				 |--------------------------------------*/

				strcpy( cusatt->value, "POWER_Gt4" );
			}
			else if ( usuffix == '4' )
			{
				/*--------------------------------------|
				 | If a Gt4i                            |
				 |--------------------------------------*/

				strcpy( cusatt->value, "POWER_Gt4i" );
			}
			else if ( usuffix == '6' )
			{
				/*--------------------------------------|
				 | If a Gt4xi                           |
				 |--------------------------------------*/

				strcpy( cusatt->value, "POWER_Gt4xi" );
			}
			else 
			{
				/*--------------------------------------|
				 | If a Gt4X                            |
				 |--------------------------------------*/

				strcpy( cusatt->value, "POWER_Gt4x" );
			}

			if ( putattr( cusatt ) < 0 )
			{
				rc = E_ODMUPDATE;
				return( rc );
			}
		}

	}

	return( rc );

}


