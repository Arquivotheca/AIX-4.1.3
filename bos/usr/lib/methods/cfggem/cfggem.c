static char sccsid[] = "@(#)77	1.1  src/bos/usr/lib/methods/cfggem/cfggem.c, gemini, bos411, 9428A410j 9/3/93 13:05:54";
/* COMPONENT_NAME: (SYSXHFT)   Gemini configuration method
 *
 * FUNCTIONS:
 *
 *    build_dds, generate_minor, make_special_files, download_microcode,
 *    query_vpd, define_children
 *
 * ORIGIN: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
 
 
/*
 * Include files needed for this module follow
 */
#include <sys/types.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <fcntl.h>
#include <stdio.h>
 
/* odmi interface include files */

#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <cf.h>
#include "cfgdebug.h"
 

/* Local include for hft dds structure */

#define TEXT_TBL_LEN      0x00000400
#define LOOKUP_TBL_LEN    0x00000400
#define LOC_TBL_RCD_LEN   0x00000150
#define S_LENGTH           256
#define BUS_OFFSET    0x00000020
#define SIXTY_HZ 0
#define SEVENTY_SEVEN_HZ 0x20
#define ISO_OFFSET 0x1C
#define ISO_OFFSET2 0x1 /* Due to messup in burning vpd,  we need
			  go check correlator field as well as minimum DD */
#include "gem_cb.h"
#include "gem_diag.h"
struct gem_dds          *dds;      /* pointer to dds structure to build */
 
extern int    *genminor();
extern char   *malloc();
 
 
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
 */
build_dds(lname, dds_out, size)
char *lname;                     /* logical name of device */
char **dds_out;                  /* pointer to dds structure for return */
int  *size;                      /* pointer to dds structure size */
{
	char    crit[80];        /* used for search criteria string */
	int     rc;              /* return status */
	ulong   value;           /* temp variable to get value into */
	char    temp[256];       /* temporary string variable */
	char    latest_fn[32];   /* temporary string variable */
	int     i;               /* loop control variable */
	struct  CuAt   *cusatt;  /* customized attribute ptr and object */
	struct  CuDv    cusobj ;    /*     customized object            */
	struct  Class   *csatt;        /* customized attribute handle  */
	struct  Class   *preatt;        /* predefined attribute handle  */
	char    sstring[S_LENGTH];  /*     search criteria string       */
	char    *tmpstr;            /*     temp pointer to string       */
 
     /*
     * Obtain size of device specific dds structure
     */
	*size = sizeof(struct gem_dds);
 
     /*
     * allocate the space required for the dds structure
     */
	if( (dds = (struct gem_dds *) malloc(*size)) == NULL )
	{
		DEBUG_0("build_dds : malloc failed for dds structure\n");
		return(E_MALLOC);
	}
 
 
    /*
     * Get the connection location for this device from the customized
     * device data.
     */
	if((rc = get_conn(lname, &value)) != E_OK )
		return(rc);
	dds->slot_number = value;
 
    /*   Read attributes from ODM  */

	if( get_attrval(lname, "bus_mem_start", (char *)NULL, &value, &rc) == NULL )
		return(rc);
	dds->mem_map_start = value>>22;  /* value to set POS register */
	dds->io_bus_mem_start = 0;       /* scratch area for now      */
 
 
	if( get_attrval(lname, "int_level", (char *)NULL, &value, &rc) == NULL )
		return(rc);
	dds->intr_level = value;
 
	if( get_attrval(lname, "int_priority", (char *)NULL, &value, &rc) == NULL )
		return(rc);
	dds->intr_priority = value;
 
	if( get_attrval(lname, "scrn_width_mm", (char *)NULL, &value, &rc) == NULL )
		return(rc);
	dds->screen_width_mm = value;
 
	if( get_attrval(lname, "scrn_height_mm", (char *)NULL, &value, &rc) == NULL )
		return(rc);
	dds->screen_height_mm = value;
 
	/**************************************************************************/
	/* Added for GEMINI II.                                                   */
	/**************************************************************************/
	if( get_attrval(lname, "dma_lvl", (char *)NULL, &value, &rc) == NULL )
		return(rc);
	dds->dma_channel = value;
 
	if( get_attrval(lname, "frame_buf_swp", (char *)NULL, &value, &rc) == NULL )
		return(rc);
	dds->features = value << 30;
 
    /*
     * Build the display id by obtaining the base ID from ODM then
     * tacking on the instance number by seeing how many times this
     * displays device driver has been configured.
     */
	if((cusatt = get_attrval(lname, "display_id", (char *)NULL, &value, &rc))
	    == NULL )
		return(rc);
	dds->display_id = value;
	if((rc =  get_instance(lname, &value)) != E_OK )
		return(rc);
	dds->display_id |= value;
	sprintf(cusatt->value, "0x%08x", dds->display_id);
	if (putattr(cusatt) < 0)
	{
		DEBUG_0("build_dds: putattr failed\n");
		return(E_ODMUPDATE);
	}
 
    /* Load the default color pallete
     */
	dds->color_count = 16;
	for( i=0; i<16; i++)
	{
		sprintf(temp, "ksr_color%d", i+1);
		if( get_attrval(lname, temp, (char *)NULL, &value, &rc) == NULL )
			return(rc);
		dds-> color_table[i] = value;
	}
 
    /* Obtain the path to the binary files and try opening them.
     */
 
    /*   GCP Microcode   */
	if(( cusatt=get_attrval(lname, "gcp_microcode", temp, &value, &rc))
								   == NULL )
		return(rc);
	if((rc =  ucodefn(temp, latest_fn)) != 0)
	      {
		DEBUG_1("rc is %x\n", rc);
		return(rc);
	      }
	/* Place name in CuAt */
	strcpy(cusatt->value, latest_fn);
	if (putattr(cusatt) > 0)
	     return(E_ODMUPDATE);

	/* Now place in dds   */
	if( (dds->u1.gcpucfd = open(latest_fn, O_RDONLY)) == -1 )
	{
		DEBUG_1("Can't open microcode file : %s",temp);
		return(E_NOUCODE);
	}
	dds->gcpuclen = lseek(dds->u1.gcpucfd, 0, SEEK_END);
	lseek(dds->u1.gcpucfd, 0, SEEK_SET);
 
     /*   GCP Table    */
	if(( cusatt=get_attrval(lname, "gcp_table", temp, &value, &rc))
								   == NULL )
		return(rc);
	if((rc =  ucodefn(temp, latest_fn)) != 0)
		return(rc);
	/* Place name in CuAt */
	strcpy(cusatt->value, latest_fn);
	if (putattr(cusatt) > 0)
	     return(E_ODMUPDATE);

	/* Now place in dds   */
	if( (dds->u2.gcptblfd = open(latest_fn, O_RDONLY)) == -1)
	 {
		DEBUG_1("Can't open microcode file : %s",temp);
		return(E_NOUCODE);
	}
	dds->gcptbllen = lseek(dds->u2.gcptblfd, 0, SEEK_END);
	lseek(dds->u2.gcptblfd, 0, SEEK_SET);
 
     /*   C25 MICROCODE  for drawing and shading processors  */
	if(( cusatt=get_attrval(lname, "c25_microcode", temp, &value, &rc))
								   == NULL )
		return(rc);
	if((rc =  ucodefn(temp, latest_fn)) != 0)
		return(rc);
	/* Place name in CuAt */
	strcpy(cusatt->value, latest_fn);
	if (putattr(cusatt) > 0)
	     return(E_ODMUPDATE);

	/* Now place in dds   */
	if( (dds->u3.c25ucfd = open(latest_fn, O_RDONLY)) == -1 )
	{
		DEBUG_1("Can't open microcode file : %s",temp);
		return(E_NOUCODE);
	}
	dds->c25uclen = lseek(dds->u3.c25ucfd, 0, SEEK_END);
	lseek(dds->u3.c25ucfd, 0, SEEK_SET);
 
     /*   Shading processor table          */
	if(( cusatt=get_attrval(lname, "shp_table", temp, &value, &rc))
								   == NULL )
		return(rc);
	if((rc =  ucodefn(temp, latest_fn)) != 0)
		return(rc);
	/* Place name in CuAt */
	strcpy(cusatt->value, latest_fn);
	if (putattr(cusatt) > 0)
	     return(E_ODMUPDATE);

	/* Now place in dds   */
	if( (dds->u4.shptblfd = open(latest_fn, O_RDONLY)) == -1 )
	{
		DEBUG_1("Can't open microcode file : %s",temp);
		return(E_NOUCODE);
	}
	dds->shptbllen = lseek(dds->u4.shptblfd, 0, SEEK_END);
	lseek(dds->u4.shptblfd, 0, SEEK_SET);
 
	/******************************************************************/
	/*   GCP GVP microcode table                                      */
	/******************************************************************/
	if(( cusatt=get_attrval(lname, "gvp_table", temp, &value, &rc))
								   == NULL )
		return(rc);
	if((rc =  ucodefn(temp, latest_fn)) != 0)
		return(rc);
	/* Place name in CuAt */
	strcpy(cusatt->value, latest_fn);
	if (putattr(cusatt) > 0)
	     return(E_ODMUPDATE);

	/* Now place in dds   */
	if( (dds->u5.gvp5afd = open(latest_fn, O_RDONLY)) == -1 )
	{
		DEBUG_1("Can't open microcode file : %s",temp);
		return(E_NOUCODE);
	}
	dds->gvp5alen = lseek(dds->u5.gvp5afd, 0, SEEK_END);
	lseek(dds->u5.gvp5afd, 0, SEEK_SET);
 
	/******************************************************************/
	/* VPD microcode for the C25 cards                                */
	/******************************************************************/
	if(( cusatt=get_attrval(lname, "c25_vpd", temp, &value, &rc))
								   == NULL )
		return(rc);
	if((rc =  ucodefn(temp, latest_fn)) != 0)
		return(rc);
	/* Place name in CuAt */
	strcpy(cusatt->value, latest_fn);
	if (putattr(cusatt) > 0)
	     return(E_ODMUPDATE);

	/* Now place in dds   */
	if( (dds->u6.c25vpdfd = open(latest_fn, O_RDONLY)) == -1 )
	{
		DEBUG_1("Can't open microcode file : %s",temp);
		return(E_NOUCODE);
	}
	dds->c25vpdlen = lseek(dds->u6.c25vpdfd, 0, SEEK_END);
	lseek(dds->u6.c25vpdfd, 0, SEEK_SET);
 
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
			DEBUG_1("Could'nt add dependency for hft -> %s",lname);
			return(rc);
		}
	}
 
 /***************************************/
 /*  Get microchannel bus_id from CuDv  */
 /***************************************/
 
	/* get customized object */
	sprintf(sstring,"name = '%s'",lname);
	DEBUG_1("cfggem:sstring = %s",sstring);
	rc = (int)odm_get_obj(CuDv_CLASS,sstring,&cusobj,TRUE);
	DEBUG_1("cfggem:rc from odm_get_obj = %x",rc);
	if(rc == 0)
	{
		DEBUG_0("cfggem : CuDv object not found\n")
		return E_NOCuDv;
	}
	else if(rc == -1)
	{
		/* odm error occured */
		DEBUG_0("cfggem : get_obj failed \n")
		return E_ODMGET;
	}
 
      /* get information from parent adapter */
	tmpstr = cusobj.parent ;
	if(tmpstr[0] == '\0' )
	{
		DEBUG_0("cfggem : parent not defined\n");
		return E_PARENT;
	}
 
	/*get the customized object of parent to get the unique type */
	sprintf(sstring,"name = '%s'",tmpstr);
	rc = (int)odm_get_obj(CuDv_CLASS,sstring,&cusobj,TRUE);
	if(rc == 0){
		DEBUG_0("cfggem: error in getting parent object\n")
		return E_NOCuDvPARENT;
	}
	else if(rc == -1){
		/* odm error occured */
		DEBUG_0("cfggem: get_obj failed\n")
		return E_ODMGET;
	}

		DEBUG_0("cfggem: ... about to get att \n");
	DEBUG_1("cfggem:name = %s\n",cusobj.name);
	DEBUG_1("cfggem:PdDvLn = %s\n",cusobj.PdDvLn_Lvalue);

	preatt=odm_open_class(PdAt_CLASS);
	csatt=odm_open_class(CuAt_CLASS);

 
	/* get the value for the attribute bus_id */
	rc = getatt(&dds->uchannel_id,'l',csatt,preatt,cusobj.name,
		cusobj.PdDvLn_Lvalue,"bus_id", (struct att *)NULL);
	if(rc > 0)
	{
		DEBUG_0("cfggem : error in getting bus_id\n")
		return rc;
	}
 
	dds->uchannel_id |= BUS_OFFSET;
 
	rc = odm_close_class(preatt);
	rc = odm_close_class(csatt);
 
    /****************************************************************/
    /*  Set refresh rate in dds                                     */
    /****************************************************************/
	if( get_attrval(lname, "refresh_rate", temp, &value, &rc) == NULL)
		 return(rc);
	else
	{
	  if (value==77)
		    dds->features = SEVENTY_SEVEN_HZ;
	  else
		    dds->features = SIXTY_HZ;
	}
     /* Note: chg_ref_rate is read and saved in query_vpd() */
 
 
    /**************************************************************
       Copy logical name to dds for error logging by device driver.
    */
	strcpy(dds->comp_name, lname);
 
	*dds_out = (char *)dds;
 
	return(E_OK);
}
 
 
/*
 * NAME: generate_minor
 *
 * FUNCTION: Device dependent routine for generating the device minor number
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is called from the device independent configuration
 *  method.
 *
 * RETURNS:
 *   minor number success
 *   positive return code on failure
 */
int
generate_minor(lname, majno, minorno)
char  *lname;     /* logical device name */
long   majno;      /* device major number */
long	*minorno;  /* device minor number */
{
	long *minorptr;
 
	minorptr = genminor(lname, majno, -1, 1, 1, 1);
 
	if( minorptr == (long *)NULL )
		return(E_MINORNO);
 
	*minorno = *minorptr;
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
 * NOTES: This routine will always return success for this device instance
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
 *   positive return code for failure
 */
 
int
query_vpd(cusobj, kmid, devno, gem_vpd)
struct CuDv   *cusobj;      /* customized device object */
mid_t         kmid;         /* driver module id */
dev_t         devno;        /* major/minor number */
char	      *gem_vpd;	    /* vpd passed back to generic config piece */
{
	void		expand_vpd();
	char		loc[10];
	int		rc;		/* return code */
	register int	i;		/* loop variable */
	char            *gem,           /* pointer to MCBI VPD */
			*mag,           /* pointer to CVME VPD */
			*gcp,           /* pointer to GCP VPD */
			*drp,           /* pointer to DrP VPD */
			*shp,           /* pointer to ShP VPD */
			*fbb;           /* pointer to FBB VPD */
	char     *fbb_iso;              /* pointer to iso flag in VPD */
	int can_change;                 /* flag to indicate iso support */
	char		*temp;		/* temporary char pointer */
	struct  cfg_dd	cfgdata;	/* sysconfig call structure */
	struct	vpd_data vpd_data;
	  struct CuAt *cusatt;
	  char tmp[256];
	  ulong value;
       char *vpd;
       uchar vp1, vp2,vp3;
       int vp;

	gem=NULL;
	mag=NULL;
	gcp=NULL;
	drp=NULL;
	shp=NULL;
	fbb=NULL;
	fbb_iso=NULL;
	cfgdata.kmid = kmid;
	cfgdata.devno = devno;
	cfgdata.cmd = CFG_QVPD;
 
/* Get VPD for all cards thru' the  device driver */
	cfgdata.ddsptr = (char *) &vpd_data;
	cfgdata.ddslen = sizeof( struct vpd_data);
 
	if( (rc = sysconfig( SYS_CFGDD, &cfgdata, sizeof(struct cfg_dd) ))
	    == -1 )
	{
		DEBUG_0("query_vpd: sysconfig failed\n");
		return(E_VPD);
	}
	/*
	 * The VPD data is passed over once to strip
	 * the unwanted null characters from it.
	 */
	for(i=0; i<256; i++) {
		if( vpd_data.uch_vpd[i] == '\0' )
			vpd_data.uch_vpd[i] = ' ';
	}
	for ( i=0; i < 256; i++) {
		if( vpd_data.mgc_vpd[i] == '\0' )
			vpd_data.mgc_vpd[i] = ' ';
	}
	for ( i=0; i < 128; i++) {
		if( vpd_data.drp_vpd[i] == '\0' && i != 1)
			vpd_data.drp_vpd[i] = ' ';
	}
	for ( i=0; i < 128; i++) {
		if( vpd_data.shp_vpd[i] == '\0' && i != 1)
			vpd_data.shp_vpd[i] = ' ';
	}
	for ( i=0; i < 128; i++) {
		if( vpd_data.gcp_vpd[i] == '\0' && i != 1)
			vpd_data.gcp_vpd[i] = ' ';
	}
	/* Read device driver level from vpd now before
	   0x00's are massaged to 0x20's                 */
	fbb_iso = vpd_data.fbb_vpd + ISO_OFFSET;
	if (*fbb_iso)
	       can_change = TRUE;
	else
	    {
	      /* since some iso boards have incorrect dd level
		 let's also check correlator field           */

	      fbb_iso = vpd_data.fbb_vpd + ISO_OFFSET2;
	      if (*fbb_iso > 1)
		   can_change = TRUE;
	       else
		   can_change = FALSE;
	     }

	for ( i=0; i < 128; i++) {
		if( vpd_data.fbb_vpd[i] == '\0' && i != 1)
			vpd_data.fbb_vpd[i] = ' ';
	}
	for ( i=0; i < 128; i++) {
		if( vpd_data.imp_vpd[i] == '\0' )
			vpd_data.imp_vpd[i] = ' ';
	}
 
/*--------------------
	Form the VPD buffer for the MCBI card , to be passed back to
        the generic configuration code in cfgdevice.c 
   ---------------------------*/
	temp = gem = &vpd_data.uch_vpd[8];
	while( *temp == '*' )
		temp += (((int)*(temp+3)) << 1);
	*temp = '\0';
	strcpy(gem_vpd, gem);

/* If this is a gto, the default display description is accurate.
   If it is a gemini, the ODM must be updated with a new description*/
    vpd = (ulong *) gem_vpd;
    for ( i = 0; i < 0x200; i++)
    {
	vp1 = vpd[i];
	vp2 = vpd[i+1];
	vp3 = vpd[i+2];
	if((vp1 == '*') && (vp2 == 'Z') && (vp3 == 'M'))
	{
	    i += 5;
	    vp = (vpd[i] & 0x000000ff);
	    if(vp != 0x31)
		/* it is gemini so update odm */
		{
		   if((cusatt =
	      get_attrval(cusobj->name,"dsp_desc",tmp,value,&rc))== NULL )
		     return(rc);
		  strcpy(cusatt->value, "High Speed 3D Graphics Accelerator");
		  if (putattr(cusatt) < 0)
		     return(E_ODMUPDATE);
		}
	  }
      }
/*-------------------
	Call the def_gem_child() routine for each of the child cards,
  if the slot no. for the card is a valid one.
	-------------------*/
 
	/* Magic */
	temp = mag =  &vpd_data.mgc_vpd[8];
	while( *temp == '*' )
		temp += (((int)*(temp+3)) << 1);
	*temp = '\0';
	if ( vpd_data.mgc_slot >= 0 ) {
		sprintf(loc,"%x",vpd_data.mgc_slot);
		if ((rc = def_gem_child("hispdmag",cusobj->name,mag,loc)) != E_OK)
			return(rc);
	}
 
/*-----------------------
	For GCP, DrP, ShP and FBB the pnemonic codes in the VPD data
  are added by the expand_vpd() routine.
	--------------------------------*/
	/* GCP */
	gcp = vpd_data.gcp_vpd;
	expand_vpd(gcp);
	if ( vpd_data.gcp_slot >= 0 ) {
		sprintf(loc,"%x",vpd_data.gcp_slot);
		if ((rc = def_gem_child("hispdgcp", cusobj->name, gcp,loc)) != E_OK)
			return(rc);
	}
 
	/* DrP */
	drp = vpd_data.drp_vpd;
	expand_vpd(drp);
	if ( vpd_data.drp_slot >= 0 ) {
		sprintf(loc,"%x",vpd_data.drp_slot);
		if ((rc = def_gem_child("hispddrp", cusobj->name, drp,loc)) != E_OK)
			return(rc);
	}
 
	/* ShP */
	shp = vpd_data.shp_vpd;
	expand_vpd(shp);
	if ( vpd_data.shp_slot >= 0 ) {
		sprintf(loc,"%x",vpd_data.shp_slot);
		if ((rc = def_gem_child("hispdshp",cusobj->name,shp,loc)) != E_OK)
			return(rc);
	}
 
	/* FBB */
	/* If VPD indicates support for changeable display rate
	    enter this in the CuAt                              */
	if (can_change)  /* set chg_ref_rate to 'Y' */
	   /*cusobj->name <=> lname */
	   {
	    if((cusatt =
	      get_attrval(cusobj->name,"chg_ref_rate",tmp,value,&rc))== NULL )
		   return(rc);
	    strcpy(cusatt->value, "y");
	    if (putattr(cusatt) < 0)
		   return(E_ODMUPDATE);
	   }
	/* Process Vpd  */
	fbb = vpd_data.fbb_vpd;
	expand_vpd(fbb);
	if ( vpd_data.fbb_slot >= 0 ) {
		sprintf(loc,"%x",vpd_data.fbb_slot);
		if ((rc = def_gem_child("hispdfbb",cusobj->name,fbb,loc)) != E_OK)
			return(rc);
	}

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
 * NAME: expand_vpd
 *
 * FUNCTION:
 *	This routine adds the pnemonics for the sub-field descriptors
 *   in the raw VPD data of GCP, DrP, ShP and FBB cards.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS:
 *  	None
 *
 */
 
void
expand_vpd(vpd_pointer)
char	*vpd_pointer;
{
	char temp[128], aux[5];
	int 	i;
 
	for ( i = 0; i < 128; i++ ) {
		temp[i] = vpd_pointer[i];
	}
	vpd_pointer[0] = '*';          /* card ID */
	vpd_pointer[1] = 'C';   /* Was ID, changed to CD */
	vpd_pointer[2] = 'D';
	vpd_pointer[3] = 0x03;
	vpd_pointer[4] = temp[0];
	vpd_pointer[5] = ' ';
	vpd_pointer[6] = '*';         /* manufactured date */
	vpd_pointer[7] = 'D';        /* Was DT, changed to DC */
	vpd_pointer[8] = 'C';
	vpd_pointer[9] = 0x04;
        itoa(temp[2],aux);
	vpd_pointer[10] = aux[0];
	vpd_pointer[11] = aux[1];
 	itoa(temp[3],aux);
	vpd_pointer[12] = aux[0];
	vpd_pointer[13] = aux[1];
 
	vpd_pointer[14] = '*';       /* Part no. */
	vpd_pointer[15] = 'P';
	vpd_pointer[16] = 'N';
	vpd_pointer[17] = 0x06;
	vpd_pointer[18] = temp[4];
	vpd_pointer[19] = temp[5];
	vpd_pointer[20] = temp[6];
	vpd_pointer[21] = temp[7];
	vpd_pointer[22] = temp[8];
	vpd_pointer[23] = temp[9];
	vpd_pointer[24] = temp[10];
	vpd_pointer[25] = temp[11];
	vpd_pointer[26] = '*';      /* Manufacturing location */
	vpd_pointer[27] = 'M';      /* Was ML, changed to MF  */
	vpd_pointer[28] = 'F';
	vpd_pointer[29] = 0x03;
	vpd_pointer[30] = temp[12];
	vpd_pointer[31] = ' ';
	vpd_pointer[32] = '*';
	vpd_pointer[33] = 'E';      /* Engg. change level     */
	vpd_pointer[34] = 'C';
	vpd_pointer[35] = 0x06;
	vpd_pointer[36] = ' ';
	vpd_pointer[37] = temp[13];
	vpd_pointer[38] = temp[14];
	vpd_pointer[39] = temp[15];
	vpd_pointer[40] = temp[16];
	vpd_pointer[41] = temp[17];
	vpd_pointer[42] = temp[18];
	vpd_pointer[43] = temp[19];
	vpd_pointer[44] = '*';       /* Correlator no. */
	vpd_pointer[45] = 'Z';
	vpd_pointer[46] = '0';
	vpd_pointer[47] = 0x3;
	vpd_pointer[48] = '0';
	vpd_pointer[49] = temp[1] + '0';
	vpd_pointer[50] = '\0';
}
 
itoa(s,dest)
unsigned char s;
char  dest[];
{
    unsigned char tmp;
 
    /* Takes care of the tenth's */
    tmp = s;
    tmp = (int) s / 10;
    dest[0] = tmp + '0';
 
    /* Takes care of the units' */
    tmp = (int) s%10;
    dest[1] = tmp + '0';
    dest[2] = '\0';
}
 
/* 
 * Search string templates 
 */
#define	SSTRING1  "type = '%s'"
#define	SSTRING2  "name = '%s'"
#define	SSTRING3  "uniquetype = '%s'"
#define	SSTRING4  "uniquetype = '%s' AND connkey = '%s'"
/*
 * NAME: def_gem_child
 *
 * FUNCTION:
 * 	Adds the other cards of the Gemini subsystem to the CuDv.
 *
 * RETURNS:
 *     0 : success
 *     positive return code on failure
 */
 
/*------------------
	This routine is a modified version of the define_child() function
  in 'ddstools.c'.The main difference is in the way the 'location' parameter
  of the CuDv is obtained. Here it is passed as a parameter.
 	-------------------------*/
int
def_gem_child(type, parent, vpd,loc)
char		*type,
		*parent,
		*vpd,
		*loc;
{
char	*lname;
int	seqno;
char	sstring[256];			/* search string */
char	location[20];			/* location string */
struct	PdCn	pdcn;		        /* predefined connection class struct */
struct	PdDv	pddv, par_pddv;		/* predefined device class structure */
struct	CuDv	cudv, par_cudv;		/* customized device class structure */
int	rc = 0;
 
	/* 
	 * Get predefined device object for this device 
	 */
	sprintf(sstring,SSTRING1,type);
	rc = (int)odm_get_first(PdDv_CLASS, sstring, &pddv);
	if ( rc==0 || rc==-1 ) 
	{
		DEBUG_0("failed to get predefined device object\n");
		return(E_VPD);
	}
 
	/* 
	 * Get customized device object for this devices parent 
	 */
	sprintf(sstring,SSTRING2, parent);
	rc = (int)odm_get_first(CuDv_CLASS, sstring, &par_cudv);
	if ( rc==0 || rc==-1 ) 
	{
		DEBUG_0("failed to get customized device parent\n");
		return(E_VPD);
	}
 
	/* 
	 * Get predefined device object for this devices parent 
	 */
	sprintf(sstring,SSTRING3, par_cudv.PdDvLn_Lvalue);
	rc = (int)odm_get_first(PdDv_CLASS, sstring, &par_pddv);
	if ( rc==0 || rc==-1 ) 
	{
		DEBUG_0("failed to get predefined device parent\n");
		return(E_VPD);
	}
 
	/* 
	 * Get predefined connection object for this device 
	 */
	sprintf(sstring,SSTRING4, par_cudv.PdDvLn_Lvalue, pddv.subclass);
	rc = (int)odm_get_first(PdCn_CLASS, sstring, &pdcn);
	if ( rc==0 || rc==-1 ) 
	{
		DEBUG_0("failed to get predefined connection object\n");
		return(E_VPD);
	}
 
	lname = (char *)malloc(15);
	sscanf(par_cudv.name+strlen(par_pddv.prefix), "%d", &seqno);
	sprintf(lname, "%s%d", pddv.prefix, seqno);
 
	/* 
	* check if device has already been defined 
	*/ 
 
	sprintf(sstring, "name = '%s'", lname );
	rc = (int)(odm_get_first(CuDv_CLASS, sstring, &cudv));
	if ( rc == -1 )
	{
		DEBUG_0("error in odm_get_first\n");
		return(E_VPD);
	}
 
	/* device was previously defined, update if necessary */
	if (rc)
	{
		/* update chgstatus */ 
		if (cudv.chgstatus != DONT_CARE && cudv.chgstatus != SAME)
		{
			cudv.chgstatus = SAME; 
			strcpy(cudv.PdDvLn_Lvalue, pddv.uniquetype);
			if ((rc = odm_change_obj(CuDv_CLASS,&cudv)) < 0)
			{
				DEBUG_0("odm_change_obj failed\n");
				return(E_VPD);
			}
		}
		/* update location */ 
		sprintf(location, "%s-0%s",par_cudv.location, loc);
		rc = strcmp(cudv.location,location);
		if (rc != 0)
		{
			strcpy(cudv.location,location);
			if ((rc = odm_change_obj(CuDv_CLASS,&cudv)) < 0)
			{
				DEBUG_0("odm_change_obj failed\n");
				return(E_VPD);
			}
		}
		/* update vpd if necessary */
		if ( strcmp(vpd, "") != 0 )
			add_vpd(lname, vpd);
 
	}
 
	/* device needs to be defined */
	else 
	{
		strcpy(cudv.name,lname);
		cudv.status = DEFINED;
		/* cudv.chgstatus = pddv.chgstatus; */
		cudv.chgstatus = DONT_CARE;
		strcpy(cudv.ddins,pddv.DvDr);
		sprintf(cudv.location, "%s-0%s",par_cudv.location, loc);
		strcpy(cudv.parent, par_cudv.name);
		strcpy(cudv.connwhere, pdcn.connwhere);
		strcpy(cudv.PdDvLn_Lvalue, pddv.uniquetype);
 
		/* 
		 * Add customized object to customized devices object class 
	 	*/
		if( (rc = odm_add_obj(CuDv_CLASS, &cudv)) < 0 ) 
		{
			DEBUG_0("add_obj failed\n");
			return(E_VPD);
		}
 
		if ( strcmp(vpd, "") != 0 )
			add_vpd(lname, vpd);
	}
 
	return(E_OK);
}
 
/*
 * NAME: ucodefn
 * FUNCTION:
 *      Finds the latest version ucode file from a given base ucode
 *      file name input. The fully qualified base file name is passed
 *      in 'ucodenm'. This routine finds the most current version of
 *      the ucode in the /etc/microcode directory.
 *      The format of the micrcode file name is:
 *                   /etc/microcode/8ffdAA.LL.VV
 *        AA is a two letter qualifier.(ie One of; gt, gu, gv, gh, c2, vp.)
 *        LL is revision level of the adapter from the LL field of the VPD.
 *        VV is version number of the microcode.
 *
 * EXECUTION ENVIRONMENT:
 *      Internal to the cfggem program.
 *
 * INPUT:
 *      ucodenm - fully qualified file name.(Char string)
 *      file    - pointer to location to put resolved file name.
 *
 * RETURNS:
 *      Returns  0 if sucessful
 *      Returns -1 if failure
 */
 
ucodefn(ucodenm, file)
char  ucodenm[];
char  *file;
{
	char    curfn[32];       /* ucode file name */
	struct  dirent  *dfp;    /* Directory file structure */
	DIR     *dir;            /* Directory ptr */
	int     bfnlen;          /* Base filename length */
	int     i, n;            /* Array ref indices */
	int     j;               /* Loop counter */
	int rc;
	rc = -1;                 /* set to 0 when file found */

     /* Remove directory prefix in front of the unqualified file name.   */
     /* and get base file name without the level/revision fields.        */
	for(n = 0, i = 0; ucodenm[n] != '.'; n++)
	{
	   if(ucodenm[n] == '/')
	      i = 0;
	   else if(ucodenm[n] == '\0')
	   {
	      DEBUG_1("Invalid microcode file name: %s",ucodenm);
	      return(-1);
	   }
	   else
	      curfn[i++] = ucodenm[n];
	}
     /* Base file name should be of the form 8ffd-- where -- is a two    */
     /* leter suffix.(ie. gt, gu, gv, gh, c2 or vp)                      */
	if(curfn[0] != '8')
	{
	   DEBUG_1("Invalid microcode file name: %s",ucodenm);
	   return(-1);
	}
	curfn[i] = '\0';
     /* curfn contains base filename without the level/revision fields.  */
     /* Determine the base file name length.                             */
	bfnlen = strlen(curfn);

     /* Initialize suffix  */
	curfn[i++] = '.';
	curfn[i++] = '0';
	curfn[i++] = '0';
	curfn[i++] = '.';
	curfn[i++] = '0';
	curfn[i++] = '0';
	curfn[i] = '\0';

     /* Open the microcode directory                                     */
	if((dir = opendir("/etc/microcode")) == NULL)
	{
	   DEBUG_1("Open ucode directory failed",ucodenm);
	   return(-1);
	}
     /* Search directory for latest version ucode file.                  */
     /* Search directory files.                                          */
	while((dfp = readdir(dir)) != NULL)
	{
     /* Do string compare of base file name fields to find files with    */
     /* desired base file name.                                          */
	   if(!strncmp(curfn, dfp->d_name, bfnlen))
	   {
     /* Now check that rest of file name is valid for a ucode file name. */
     /* File name is of form '8ffd--.LL.VV'. So we check that there is   */
     /* 6 and only 6 more characters and that they follow above form.    */
	      for(n = bfnlen, i = 0; i <= 6; i++, n++)
	      {
		 switch(i)
		 {
     /* Adapter level or ucode version fields delimited by a '.'.        */
		    case 0:
		    case 3:
			if(dfp->d_name[n] != '.')
			{
			   i = 7;
			}
			break;
     /* Ucode file name should terminate here with a null character.     */
		    case 6:
			if(dfp->d_name[n] == '\0')
			{
			   int  cmprc;
     /* Compare complete file names(including level/version fields) to   */
     /* get the latest version of the file.                              */
			   cmprc = strncmp(curfn, dfp->d_name, bfnlen+6);
			   if(cmprc == 0)
			   {
			   DEBUG_1("Found matching file: %s\n",dfp->d_name);
			   rc=0;
			   }
     /* Will return negative if curfn is lexicographically less than     */
     /* dfp->d_name.(If curfn is older version)                          */
			   else if(cmprc < 0)
			   {
			   DEBUG_1("Found newer file: %s\n",dfp->d_name);
			   strcpy(curfn, dfp->d_name);   /* Update curfn */
			   rc=0;
			   }
			}
			break;
     /* Adapter level or ucode version character is default.             */
		 }
	      }
	   }
	}
	if(rc < 0)
	{
	   DEBUG_1("No file found matching %s\n", curfn);
	   return(rc);
	}
     /* Close the directory.                                             */
	closedir(dir);
     /* Set return file name.                                            */
	sprintf(file,"/etc/microcode/%s", curfn);
	DEBUG_1("resolved file name is %s\n", file);
	DEBUG_1("leaving ucodefn: rc is %x\n", rc);
	return(rc);
}
 
