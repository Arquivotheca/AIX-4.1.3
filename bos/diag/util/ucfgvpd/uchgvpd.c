static char sccsid[] = "@(#)05	1.15  src/bos/diag/util/ucfgvpd/uchgvpd.c, dsauchgvpd, bos411, 9428A410j 5/26/93 13:58:05";
/*
 * COMPONENT_NAME: (DUTIL) DIAGNOSTIC UTILITY
 *
 * FUNCTIONS: 	dsp_alt_vpd
 *		build_device
 *		build_vpd
 *		display_vpd
 *		update_vpd
 *		mergetext
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <signal.h>
#include <nl_types.h>
#include <memory.h>
#include <sys/cfgdb.h>
#include <ctype.h>
#include <cf.h>
#include "diag/class_def.h"
#include "diag/diag.h"
#include "diag/diago.h"
#include "diag/diagvpd.h"
#include "ucfgvpd_msg.h"

#define NOT_ALTERABLE  '*'
#define ODM_FAILURE -1
#define VPD_LINE_LENGTH 60 

/* Globally Defined Variables */

/* function to retrieve binary data from VPD buffer */
extern int		get_byte();

struct ukey {
	int	keymsg;		/* message number in lscfg catalog file */
	char  	*keyword;	/* vpd keyword				*/
	int	datasize;	/* size of allowable entered data       */
	int	(*func)();	/* function to call to convert data     */
	int	alterflag;	/* Not alterable if HW_VPD      	*/
	char	*data;
	char	*vpdvalue;
};

/* System Unit VPD */
struct ukey sys_unit_vpd[] = {
	{ TM_MSG, "TM",17 },		/* Machine type/model           */
	{ SN_MSG, "SN",8 },		/* Serial number                */
	{ MF_MSG, "MF",9 },		/* Manufacturer                 */
	{ RN_MSG, "RN",2 },		/* Rack name                    */
	{ US_MSG, "US",100 },		/* User data                    */
};
/* Drawer VPD */
struct ukey drawer_vpd[] = {
	{ DU_MSG, "DU",25 },		/* Drawer Unit                  */
	{ SN_MSG, "SN",8 },		/* Serial number                */
	{ DL_MSG, "DL",25 },		/* EIA location                 */
	{ MF_MSG, "MF",9 },		/* Manufacturer                 */
	{ US_MSG, "US",100 },		/* User data                    */
};
/* System Planar VPD */
struct ukey sys_planar_vpd[] = {
	{ PN_MSG, "PN",12 }, 		/* Part number                  */
	{ EC_MSG, "EC",12 },		/* EC level                     */
	{ PI_MSG, "PI",12 },		/* Processor ID                 */
	{ RL_MSG, "RL",25}, 		/* IPL ROS Level                */
	{ RL_MSG, "RL",25}, 		/* OCS ROS Level                */
	{ RL_MSG, "RL",25}, 		/* Seeds ROS Level            	*/
	{ PC_MSG, "PC",25 },		/* Processor Component          */
	{ Z0_MSG, "Z0",25 }, 		/* Adapter Specific             */
	{ Z0_MSG, "Z1",25 }, 		/* Adapter Specific             */
	{ Z0_MSG, "Z2",25 }, 		/* Adapter Specific             */
	{ Z0_MSG, "Z3",25 }, 		/* Adapter Specific             */
	{ Z0_MSG, "Z4",25 }, 		/* Adapter Specific             */
	{ Z0_MSG, "Z5",25 }, 		/* Adapter Specific             */
	{ Z0_MSG, "Z6",25 }, 		/* Adapter Specific             */
	{ Z0_MSG, "Z7",25 }, 		/* Adapter Specific             */
	{ Z0_MSG, "Z8",25 }, 		/* Adapter Specific             */
	{ Z0_MSG, "Z9",25 }, 		/* Adapter Specific             */
	{ US_MSG, "US",100 },		/* User data - ascii            */
};
/* I/O Planar VPD */
struct ukey io_planar_vpd[] = {
	{ EC_MSG, "EC",12 },		/* EC level                     */
	{ US_MSG, "US",100 },		/* User data - ascii            */
};
/* Memory VPD */
struct ukey memory_vpd[] = {
	{ EC_MSG, "EC",12 },		/* EC level                     */
	{ Z0_MSG, "Z0",25 }, 		/* Adapter Specific             */
	{ Z0_MSG, "Z1",25 }, 		/* Adapter Specific             */
	{ Z0_MSG, "Z2",25 }, 		/* Adapter Specific             */
	{ US_MSG, "US",100 },		/* User data                    */
};
/* Serial Link Adapter and Microchannel Cards VPD */
struct ukey sla_mca_vpd[] = {
	{ PN_MSG, "PN",12 }, 		/* Part number                  */
	{ EC_MSG, "EC",12 },		/* EC level                     */
	{ FN_MSG, "FN",8 },		/* FRU number                   */
	{ SN_MSG, "SN",8 },		/* Serial number                */
	{ MF_MSG, "MF",9 },		/* Manufacturer                 */
	{ DD_MSG, "DD",25 },		/* Device Driver Level          */
	{ DG_MSG, "DG",25 },		/* Diagnostics Level            */
	{ RL_MSG, "RL",25 },		/* ROS Level ID                 */
	{ LL_MSG, "LL",25 },		/* Loadable Microcode Level     */
	{ NA_MSG, "NA",25,get_byte},	/* Network Address      	*/
	{ RA_MSG, "RA",4,get_byte},	/* ROS code pointer		*/
	{ RW_MSG, "RW",4,get_byte},	/* R/W adapter reg ptr       	*/
	{ LA_MSG, "LA",4,get_byte },	/* Loadable ROS code ptr     	*/
	{ US_MSG, "US",100 },		/* User data                    */
};
/* SCSI Devices VPD */
struct ukey scsi_vpd[] = {
	{ TM_MSG, "TM",17 },		/* Product Type                 */
	                           	/* Model Number                 */
	{ SN_MSG, "SN",8 },		/* Serial number                */
	{ MF_MSG, "MF",9 },		/* Manufacturer                 */
	{ PN_MSG, "PN",12 }, 		/* Part number                  */
	{ EC_MSG, "EC",12 },		/* EC level                     */
	{ US_MSG, "US",100 },		/* User data                    */
};
/* All other devices not identified as having VPD */
struct ukey default_vpd[] = {
	{ PN_MSG, "PN",12 }, 		/* Part number                  */
	{ EC_MSG, "EC",12 },		/* EC level                     */
	{ SN_MSG, "SN",8 },		/* Serial number                */
	{ MF_MSG, "MF",9 },		/* Manufacturer                 */
	{ US_MSG, "US",100 },		/* User data                    */
};

#define NSYSKEYS (sizeof(sys_unit_vpd)     / sizeof( struct ukey))
#define NCPUKEYS (sizeof(drawer_vpd)       / sizeof( struct ukey))
#define NSPLKEYS (sizeof(sys_planar_vpd)   / sizeof( struct ukey))
#define NIPLKEYS (sizeof(io_planar_vpd)    / sizeof( struct ukey))
#define NMEMKEYS (sizeof(memory_vpd)       / sizeof( struct ukey))
#define NMCAKEYS (sizeof(sla_mca_vpd)      / sizeof( struct ukey))
#define NSCSKEYS (sizeof(scsi_vpd)         / sizeof( struct ukey))
#define NDEFKEYS (sizeof(default_vpd)      / sizeof( struct ukey))

typedef struct vpd {
	char	*desc;    		/* resource class, subclass, type*/
	int	num_keys;		/* number of keys in struct */
	struct  ukey  *vpd;		/* vpd structure ptr	    */
} vpddata_t;

vpddata_t vpddata[] = {
	{ CLASS_SYSUNIT, NSYSKEYS, sys_unit_vpd },/* System Unit VPD 	*/ 
	{ CLASS_DRAWER, NCPUKEYS, drawer_vpd },	/* Drawer VPD		*/
	{ CLASS_SYSPLANAR, NSPLKEYS, sys_planar_vpd },/* System Planar VPD*/
	{ CLASS_IOPLANAR, NIPLKEYS, io_planar_vpd },  /* I/O Planar VPD	*/
	{ CLASS_MEMORY,  NMEMKEYS, memory_vpd },/* Memory VPD		*/
	{ CLASS_ADAPTER,NMCAKEYS, sla_mca_vpd },/* SLA , MCA VPD	*/
	{ SUBCLASS_SCSI, NSCSKEYS, scsi_vpd },	/* SCSI VPD		*/
	{ CLASS_DEFAULT, NDEFKEYS, default_vpd }, /* Default VPD 	*/ 
};
#define NUMVPDTYPES (sizeof(vpddata) / sizeof( struct vpd))

/* Externally Defined Variables */
extern nl_catd	fdes;

/* Called Functions */
char	*malloc();
vpddata_t 		*build_device();
extern  char	*diag_cat_gets();

/*  */
/*
 * NAME: disp_alt_vpd
 *                                                                    
 * FUNCTION: Display VPD data for named resource. 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This program executes as an application of the kernal. It 
 *	executes under a process, and can page fault.
 *                                                                   
 * NOTES:
 *
 * RETURNS: 
 *	DIAG_ASL_CANCEL
 *	DIAG_ASL_EXIT
 *	DIAG_ASL_COMMIT
 */

dsp_alt_vpd ( dname )
char 	*dname;		/* resource name	*/
{
	int		rc, i;
	char 		resc_desc[512];	/*resource name,location,description*/
	vpddata_t	*device_vpd;

	/* get entry in CuDv for named device */
	if ( (device_vpd = build_device(dname, resc_desc))==(vpddata_t *) -1 )
		return(-1);

	/* display menu and accept any input data */
	rc = display_vpd ( dname, resc_desc, device_vpd );

	/* clear any and all previous data */
	for ( i=0; i < device_vpd->num_keys; i++ ) {
		if ( strlen(device_vpd->vpd[i].vpdvalue) )  {
			free ( device_vpd->vpd[i].vpdvalue );
			device_vpd->vpd[i].vpdvalue = NULL;
		}
		if ( strlen(device_vpd->vpd[i].data) )  {
			free ( device_vpd->vpd[i].data );
			device_vpd->vpd[i].data = NULL;
		}
	}

	return(rc);

}

/*  */
/*
 * NAME: build_device
 *                                                                    
 * FUNCTION: Searches CuDv object class for resouce and determines
 *		vpddata structure for device
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS: 
 *	vpddata_t *    : pointer to vpddata structure for resource
 *	vpddata_t * -1 : error
 */

vpddata_t *
build_device( dname, dev_desc )
char	*dname;
char	*dev_desc;
{
	int                   	rc, i;
	char			crit[132];
	char			*ddesc;
	struct	CuDv		*cudv;
	struct	CuVPD		*cuvpd_hw;
	struct	CuVPD		*cuvpd_user;
	struct  listinfo 	c_info;

#define DEV_TEXT_FMT " %-16.16s  %-16.16s  "

	/* initialize ODM */
	if ((rc=odm_initialize()) == ODM_FAILURE)
		return((vpddata_t *)-1);

	/* get entry for resource in CuDv */
	sprintf(crit, "name = %s", dname);
	cudv = get_CuDv_list(CuDv_CLASS, crit, &c_info, 1, 2);
	if ( cudv == (struct CuDv *) -1 )
		return((vpddata_t *)-1);

	sprintf( dev_desc, DEV_TEXT_FMT, dname, cudv->location);
	/* if device has one description */
        if ( cudv->PdDvLn->msgno != 0 ) {
		mergetext( strlen(dev_desc),
                           &dev_desc[strlen(dev_desc)],
                           NLgetamsg( cudv->PdDvLn->catalog,
                                      cudv->PdDvLn->setno,
                                      cudv->PdDvLn->msgno,
                                      " n/a "));
	}
	else {
		get_text( cudv, &ddesc );
		mergetext( strlen(dev_desc), 
		           &dev_desc[strlen(dev_desc)], ddesc);
	}
 
	/* determine vpddata structure for resource */
	for ( i=0; i < NUMVPDTYPES; i++ ) 
		if ( (!strcmp( cudv->PdDvLn->class,    vpddata[i].desc)) ||
		     (!strcmp( cudv->PdDvLn->subclass, vpddata[i].desc)) ) 
			break;

	/* if device not found - use default */
	i = ( i == NUMVPDTYPES ) ? NUMVPDTYPES-1 : i ;
	
	odm_free_list(cudv, &c_info);
		
	/* get hardware vpd entry */
	sprintf(crit, "name = %s and vpd_type = %d", dname, HW_VPD);
	cuvpd_hw = get_CuVPD_list(CuVPD_CLASS, crit, &c_info, 1, 1);
	if ( c_info.num > 0 )
		build_vpd( cuvpd_hw->vpd,   &vpddata[i], HW_VPD );

	odm_free_list(cuvpd_hw, &c_info);

	/* get user vpd entry */
	sprintf(crit, "name = %s and vpd_type = %d", dname, USER_VPD);
	cuvpd_user = get_CuVPD_list(CuVPD_CLASS, crit, &c_info, 1, 1);
	if ( c_info.num > 0 )
		build_vpd( cuvpd_user->vpd, &vpddata[i], USER_VPD );
	odm_free_list(cuvpd_user, &c_info);
	rc = odm_terminate();
	return(&vpddata[i]);
}

/*  */
/*
 * NAME: build_vpd
 *                                                                    
 * FUNCTION: Read vpd data for device and convert to displayable message
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS:  0
 */

int 
build_vpd( vpddata, dev_vpd, vpd_type )
char            *vpddata;
vpddata_t 	*dev_vpd;
int		vpd_type;
{
	int 	cnt;
	int   	i;
	VPDBUF	vbuf;
	char 	*vptr;

	parse_vpd(vpddata,&vbuf,0);
	for(cnt=0; cnt < vbuf.entries; cnt++) {
		vptr = vbuf.vdat[cnt];
		++vptr;
		for ( i=0; i < dev_vpd->num_keys; i++ )
			if ( (!strncmp(vptr, dev_vpd->vpd[i].keyword, 2)) &&
			     (!strlen(dev_vpd->vpd[i].vpdvalue)) )  {
				dev_vpd->vpd[i].vpdvalue = 
					(char*) calloc(1,VPDSIZE);
				strcpy(dev_vpd->vpd[i].vpdvalue,(vptr+3));
				dev_vpd->vpd[i].alterflag = vpd_type;  
				break;
			}
	}
	free_vbuf(&vbuf);
	return(0);
}

/*  */
/*
 * NAME: display_vpd
 *                                                                    
 * FUNCTION: Display vpd data and accept any input changes.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS: 
 *	DIAG_ASL_CANCEL
 *	DIAG_ASL_EXIT
 *	DIAG_ASL_COMMIT
 */

int 
display_vpd( dname, device_desc, dev_vpd )
char		*dname;
char 		*device_desc;
vpddata_t 	*dev_vpd;
{
	int   	i, rc = -1;
	int	entry = 0, entry_size;
	int	num_entries;
	char	buffer[512];
	char	specific[126];
	ASL_SCR_INFO	*menuinfo;
	static	ASL_SCR_TYPE	menutype = DM_TYPE_DEFAULTS;

	/* allocate space for title, selection, last line, and vpd data */
	num_entries = dev_vpd->num_keys + 2;
	menuinfo = (ASL_SCR_INFO *) calloc(num_entries, sizeof(ASL_SCR_INFO));

	/* put in the title and resource description */
	sprintf( buffer, diag_cat_gets(fdes, MSET_DSP_VPD, DSPALTER),device_desc);
	menuinfo[entry++].text = buffer;

	/* put in each vpd entry */
	for ( i=0; entry < (num_entries-1); i++, entry++ ) {
		if ( dev_vpd->vpd[i].keyword[0] == 'Z' )  {
			sprintf( specific, diag_cat_gets( fdes, SETVPD, 
				  	          dev_vpd->vpd[i].keymsg ), 
				 dev_vpd->vpd[i].keyword[0],
				 dev_vpd->vpd[i].keyword[1] );
				 
			menuinfo[entry].text = malloc(strlen(specific)+1);
			strcpy(menuinfo[entry].text, specific);
		}
		else
			menuinfo[entry].text = diag_cat_gets( fdes, SETVPD, 
					dev_vpd->vpd[i].keymsg );
		if ( (dev_vpd->vpd[i].alterflag == USER_VPD) ||
		     (!strlen(dev_vpd->vpd[i].vpdvalue)) ) {
			menuinfo[entry].entry_type = ASL_TEXT_ENTRY;
			entry_size = dev_vpd->vpd[i].datasize;
			dev_vpd->vpd[i].alterflag = USER_VPD;
		}
		else {
			menuinfo[entry].item_flag = '*'; 
			menuinfo[entry].entry_type = ASL_NO_ENTRY;
			entry_size = strlen(dev_vpd->vpd[i].vpdvalue);
		}
		menuinfo[entry].entry_size = entry_size;
		dev_vpd->vpd[i].data = (char *)calloc(1,entry_size+1);
		menuinfo[entry].data_value = dev_vpd->vpd[i].data;
		menuinfo[entry].disp_values = dev_vpd->vpd[i].vpdvalue;
	}

	/* put in the last line */
	menuinfo[entry].text = diag_cat_gets(fdes, MSET_DSP_VPD, DSPALTLAST);

	menutype.max_index = entry;

	while ( rc != DIAG_ASL_CANCEL && rc != DIAG_ASL_EXIT && 
	        		         rc != DIAG_ASL_COMMIT ) {
		rc = diag_display(0x802033, fdes, NULL, DIAG_IO, 
			  ASL_DIAG_DIALOGUE_SC, &menutype, menuinfo);
		if ( rc == DIAG_ASL_COMMIT )
			if ( update_vpd (dname, entry, menuinfo, dev_vpd) == -1)
				return(-1);
	}
	return(rc);
}

/*  */
/*
 * NAME: update_vpd
 *                                                                    
 * FUNCTION: Update vpd data for device in CuDv
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS: 
 *	0 : no errors
 *     -1 : error
 */

update_vpd ( dname, menu_entries, menuinfo, dev_vpd )
char		*dname;
int		menu_entries;
ASL_SCR_INFO	*menuinfo;
vpddata_t 	*dev_vpd;
{
	int                   	rc, i, j;
	int			length;
	int			val;
	int			new_vpd_entry = DIAG_FALSE;
	char			crit[132], vpdvalue[VPDSIZE];
	char			*bufptr, *dataptr, *vpdptr;
	struct	CuVPD		*cuvpd;
	struct  listinfo 	c_info;

	/* check to see if any data was changed  */
	for ( i=0; i < menu_entries; i++ )
		if ( menuinfo[i].changed == ASL_YES )
			break;
	/* return if nothing done */
	if ( i == menu_entries )
		return(0);

	/* initialize ODM */
	rc=odm_initialize();

	/* get entry for resource in CuVPD */
	sprintf(crit, "name = %s and vpd_type = %d", dname, USER_VPD);
	cuvpd = get_CuVPD_list(CuVPD_CLASS, crit, &c_info, 1, 1);
	if ( c_info.num == 0 ) {
		new_vpd_entry = DIAG_TRUE;
		cuvpd = (struct CuVPD *)calloc(1, sizeof(struct CuVPD));
		strncpy(cuvpd->name, dname, NAMESIZE);
		cuvpd->vpd_type = USER_VPD;
	}

	/* update all of the alterable fields  */
	memset(cuvpd->vpd,0,VPDSIZE);
	vpdptr = cuvpd->vpd;
	for ( i=0; i < dev_vpd->num_keys; i++ )
		if ( dev_vpd->vpd[i].alterflag == USER_VPD ) {
			if ( !(length = strlen(dev_vpd->vpd[i].data)) )
				continue;

			/* if data should be in binary format */
			dataptr = dev_vpd->vpd[i].data;
			if ( dev_vpd->vpd[i].func != NULL ) {

				/* put in keyword and length */
				*vpdptr++ = '*';
				*vpdptr++ = dev_vpd->vpd[i].keyword[0];
				*vpdptr++ = dev_vpd->vpd[i].keyword[1];
				length = tohex(dataptr,vpdvalue);
				*vpdptr++ = (length + 4) /2;
				memcpy(vpdptr,vpdvalue,length); 	
				vpdptr += length;
			}

			/* data is in character format */
			else {
				/* if odd byte - add right justified space */
				length += 4;
				if ( length%2 ) 
					sprintf( vpdvalue, "*%s %s ", 
				 	     dev_vpd->vpd[i].keyword, dataptr);
				else
					sprintf( vpdvalue, "*%s %s", 
					     dev_vpd->vpd[i].keyword, dataptr);

				vpdvalue[3] = (( length%2 ) 	
						? ++length : length) / 2;
				strcat( vpdptr, vpdvalue );		
				vpdptr += length;
				if ( length%2 )
					vpdptr++;
			}
		}

	if (new_vpd_entry == DIAG_TRUE ) {
		rc = odm_add_obj(CuVPD_CLASS, cuvpd);
		free ( cuvpd );
	}
	else {
		rc = odm_change_obj(CuVPD_CLASS, cuvpd);
		odm_free_list ( cuvpd, &c_info );
	}

	rc = odm_terminate();
	return(0);
}
/*   */
/*
 * NAME: mergetext
 *                                                                    
 * FUNCTION: Adjust wraparound of text on screen
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS: 0
 */

mergetext( string_length, buffer, text )
int	string_length;	/* current length of text in buffer	*/
char	*buffer;	/* buffer to append text to		*/
char 	*text;		/* text to be appended			*/
{
	int	i;
	int 	space_count;
	int 	char_positions;

	/* determine if length of text string will fit on one line */
	char_positions = LINE_LENGTH - string_length;
	if ( char_positions < strlen(text))  {

		/* dont break the line in the middle of a word */
		if(text[char_positions] != ' ' && text[char_positions+1] != ' ')
			while ( --char_positions )
			   	if( text[char_positions] == ' ')
					break;
		if ( char_positions == 0 )
			char_positions = LINE_LENGTH - string_length;

		for ( i = 0; i <= char_positions; i++, buffer++, text++ )
			*buffer = ( *text == '\n' ) ? ' ' : *text;
		*buffer++ = '\n';
		while ( *text == ' ' )   /* remove any leading blanks */
			text++;
		space_count = string_length;
		while ( space_count-- )
			*buffer++ = ' ';
		mergetext( string_length, buffer, text);
	}
	else
		sprintf(buffer, "%s", text);

	return(0);
} 

/*   */
/*
 * NAME: tohex 
 *                                                                    
 * FUNCTION: convert binary to hexadecimal with 0 fill to even bounder  
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS: length of string
 */

tohex(in,out)
char *in,*out;
{
short val;
int len;
char tmp=0;
int shift_cnt=4;
int rval=0;
char tbuf[16],*buf;

	buf=tbuf;
	*buf=0;
	++buf;
	if((len=strlen(in))%2) 
		shift_cnt=0;
	while(len > 0) {
		
		val = (isxdigit(*in) ? (isdigit(*in) ? *in-0x30 :
					(islower(*in) ? *in-0x57 : 
							*in-0x37)) : -1);
		if(val == -1) {
			if(shift_cnt == 4) {
				++in;
				--len;
				shift_cnt=0;
			}
			val=0;
		}
		tmp |= val;	
		tmp <<= shift_cnt;
		if(shift_cnt ==0) {
			*buf=tmp;
			++buf;
			shift_cnt=4;
			tmp=0;
			++rval;
		}
		else shift_cnt-=4;
		--len;
		++in;
	}
	if(rval%2)
		memcpy(out,tbuf,++rval);
	else 
		memcpy(out,&tbuf[1],rval);
	return(rval);
}

			
/* ^L */

/* NAME: get_text
 *
 * FUNCTION: This function searches the CuAt data base for attributes for
 *              a device. If not found, use standard device text.
 *              Else build text string from attributes.
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *
 */

get_text( cudv, textptr ) 
struct CuDv *cudv;
char **textptr;
{
        int             msgno=0;
        struct CuAt     *cuat = (struct CuAt *) NULL;
        struct PdAt     *pdat = (struct PdAt *) NULL;
        struct listinfo c_info;
        char            *tmp;
        char            crit[100];

        /* check for type 'T' attribute in customized attributes */
        sprintf( crit, "name = %s AND type = T", cudv->name);
        cuat = (struct CuAt *)get_CuAt_list(CuAt_CLASS, crit, &c_info, 1,1 );
        if ( cuat == (struct CuAt *) -1 )
                return(-1);

        /* if no customized attribute, then get default from PdAt */
        if ( c_info.num == 0 ) {
                sprintf( crit, "uniquetype = %s AND type = T",
                                                cudv->PdDvLn_Lvalue);
                pdat = (struct PdAt *)get_PdAt_list(PdAt_CLASS, crit,
                                                        &c_info, 1,1 );
                if ( pdat == (struct PdAt *) -1 )
                        return(-1);
                else if (c_info.num == 1 )
                        msgno = atoi(pdat->deflt);
        }
        else
                msgno = atoi(cuat->value);

        /* use attributes value for message index */
        tmp = NLgetamsg(cudv->PdDvLn->catalog, cudv->PdDvLn->setno, msgno, "n/a" );

        *textptr = (char *)malloc( strlen(tmp) + 1);
        strcpy( *textptr, tmp );
        if ( cuat != (struct CuAt *) NULL )
                free(cuat);
        return(0);
}

