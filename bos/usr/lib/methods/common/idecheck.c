static char sccsid[] = "@(#)27  1.4  src/bos/usr/lib/methods/common/idecheck.c, cfgmethods, bos41J, 9520B_all 5/17/95 20:27:56";
/*
 * COMPONENT_NAME: (CFGMETHODS) IDE Common check routine
 *
 * FUNCTIONS: chktype, chkident, get_ide_utype, 
 *            match_model_name, issue_ide_identify_device
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include        <fcntl.h>
#include        <errno.h>
#include	<stdio.h>
#include        <sys/ioctl.h>
#include        <sys/types.h>
#include        <sys/cfgdb.h>
#include        <sys/cfgodm.h>
#include        <sys/scsi.h>
#include	<sys/ide.h>
#include        "cfgdebug.h"
#include        "cfgide.h"
#include        <cf.h>

#define INQ_SIZE      255

/* forward function definitions */

int	chktype(uchar*, char*, int, char*, uchar);
int	chkident(uchar*, struct PdAt*, int);
char    *get_ide_utype(uchar *,struct PdAt*, int,struct mna*, 
			int,int,uchar);
char    *match_model_name(char*,struct mna*,int);
int	issue_ide_identify_device(int, uchar*, uchar, int*, uchar*);

/*
 * NAME: chktype
 *
 * FUNCTION: Validates the unique type passed is correct by comparing
 *           identify data with attributes in the database.
 *
 * EXECUTION ENVIRONMENT:
 *
 *    This is an internal subroutine called by query_vpd() and disk_present()
 *    in the device configuration methods.
 *
 * RETURNS:     0 if success
 *		E_WRONGDEVICE if identify data type is incorrect
 *		E_ODMGET if couldn't access ODM 
 *		 	
 *	 	
 */

int
chktype(identdata,utype,devtype,parent_name,id)
uchar   *identdata;			/* ptr to identify data		*/	
char    *utype;				/* ptr to device's uniquetype   */
int     devtype;			/* type= disk,tape,cd      	*/
char    *parent_name;			/* logical name of parent	*/
uchar   id;                             /* IDE id of device    		*/
{
	struct mna chk_mna;
        int     i,rc;
        struct  PdAt PdAt;
        char    sstr[256];
        char    model_name[41];
	struct	listinfo	list;		/* number  model_map attribs*/
	struct 	PdAt	 	*pdatobj_ptr;   /* list of model_map attribs*/
        char    c;
        int     adap_fd;                        /* file desc for ide adptr */
	char    *utype2;
	char    adp_name[32];                 /* spec file name of ide adptr */


	sprintf(adp_name, "/dev/%s", parent_name) ;

	if ((adap_fd = open(adp_name, O_RDWR)) <  0) {
		return(E_DEVACCESS);
	}
        if (ioctl(adap_fd, IDEIOSTART, id) != 0) {
		return(E_DEVACCESS);
	}

	/* search for all model_map attributes for this device  */

	sprintf(sstr, "uniquetype='%s' AND attribute=model_map",utype);
	pdatobj_ptr = odm_get_list(PdAt_CLASS, sstr, &list, 4, 1);

	if ( (int)pdatobj_ptr == -1 ) {
		DEBUG_0("error getting PdAt object list info\n")
		return(E_ODMGET);
	}

	utype2 = get_ide_utype(identdata,pdatobj_ptr,list.num,
				(struct mna *) &chk_mna,0,
				adap_fd,id);

	if (ioctl(adap_fd, IDEIOSTOP, id) != 0) {
                DEBUG_1("Failed to IDEIOSTOP device at IDE %d \n",
                (int)id) ;
	}
	close(adap_fd);

        /* If device is of correct class, & subclass, break out */
        if (strcmp(utype2,utype)) {
                DEBUG_2("chktype: disk=%s object=%s\n",PdAt.uniquetype,utype)
                return(E_WRONGDEVICE);
        }
        return(0);
}

/*
 * NAME: chkident
 *
 * FUNCTION: Compares the identify data with the "model_map" attributes
 *           in the database.
 *
 * EXECUTION ENVIRONMENT:
 *
 *    This is an internal subroutine called by chktype & det_utype.
 *
 * RETURNS:    if match, index into the listinfo structure
 *	       if no match -1
 */

int
chkident(identdata,obj_ptr,list_num)
uchar		*identdata;
struct	PdAt	*obj_ptr;
int		list_num; 
{
        int     i;
	char	*ptr;				/* ptr used in comparison   */
	int	offset;				/* offset into identify data */
	int	length;				/* length of identify data   */
	int	found_flag;			/* for determining if match */
	char	*deflt_end;			/* ptr to end of dflt field */


	/* Loop through the PdAt model_map attributes until a     
	   comparison string matches.  An example of the deflt field:

		SSTTchar_string1,UUVVchar_string2/char_string3

		where:
		SS is identify data offset
		TT is the identify data length
		char_string1 is the character string to compare with
	 	UU is the second field in the identify data offset
		VV is the second length of identify data 
		char_string2 OR char_string3 must be present in the ident data
									*/

	/* loop through all model_map attr. until find a match or until reach
	   end of list */
	
	for ( i=0; i < list_num; i++ ) {
		ptr = (char *)obj_ptr[i].deflt;
		deflt_end = ptr + strlen (ptr) ;

		while ( ptr < deflt_end ) {  

			found_flag = FALSE;
			sscanf(ptr,"%2x%2x",&offset, &length);

			ptr= ptr + 4;		/* point past offset/len      */

			/* ensure char_strings in deflt field are valid */

			if (length == 0 || (ptr+length) > deflt_end )
				break;
				

			/* loop through strings for this field  */
			while (ptr < deflt_end ){

				if (strncmp(identdata+ offset, ptr,length)== 0) {

					/* Matched on this field */
					found_flag=TRUE;

					/* Need to position to next field */
					while ( ptr < deflt_end){
						ptr=ptr+length+1;
						if ( *(ptr-1) != '/' )
							break;
					}
					break;
				}
				else{	
					/* go one past,skip delimiter  */
	 		 		ptr= ptr + length + 1 ;
					if ( *(ptr-1) == ',' )
						break;
				}
			}  /* end while */

			if (!found_flag)
				break;
		} /* end while */

		if (found_flag)   	/* found a match */
			return(i);		
	} /* end for */
	return (-1);	/* did not find a match */

}

/*
 * NAME: get_ide_utype
 *
 * FUNCTION: Finds a IDE devices uniquetype.
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *    This is an internal subroutine called by chktype & det_utype.
 *
 * RETURNS:    if match, index into the listinfo structure
 *	       if no match -1
 */

char *
get_ide_utype(identdata,obj_ptr,list_num,mn_attr,num_nm,adapter,id)
uchar		*identdata;
struct	PdAt	*obj_ptr;
int		list_num;
struct mna	*mn_attr; 
int		num_nm;				/* Number of struct mn_attrs*/
						/* NOTE: if num_nm, there   */
						/* still must be one there  */
						/* return the utype to the  */
						/* caller		    */
int     	adapter;                        /* adapter file descriptor  */
uchar   	id;                             /* IDE id of device 	    */
{
	char    *utype;                         /* pointer to unique type */
        int     i;
	int     index;                         /* index into PdAt object array */
        struct  PdAt PdAt;
        char    sstr[256];
        char    model_name[41];
	char	*model_ptr;			/* ptr to identify data     */
	ushort	dev_type;			/* device type, if any	    */
	int	rc=0;			        /* return code		    */
        int 	error;			        /* Error issue_ide_identify */
	struct  identify_device *identptr;

	/* set uniquetype pointer to NULL in case no match */
	utype = NULL;

	if ( list_num != 0 ) {		/* retrieved at least 1 attr   */
 		/* chkident compares the identify data with the attributes */
		index = chkident(identdata, obj_ptr, list_num);
		if (index != -1 ) {
			utype = (char *)&obj_ptr[index].uniquetype;
			return (utype);	/* success, match was found */
		}
	}


	/* no match on model_map attribute, try to extract the model_name   */
	model_ptr = (char *)
		    ((struct identify_device *)identdata)->model_number;

	/* copy model name data from identify data, convert certain characters
	   to blanks */
	for (i=0;  i<40; i++ )  {
		if ( model_ptr[i] > ' ' && model_ptr [i] < '\177' ){
				model_name[i] = model_ptr[i];
		}
		else {
			model_name[i] = ' ';
		}
	}
        model_name[40] = '\0'; 
  
	/* set unique type pointer to NULL in case no match */
	utype = NULL;

        DEBUG_1("get_ide_utype: model_name=%s\n",model_name)

	utype = match_model_name(model_name,mn_attr,num_nm);


	identptr = (struct identify_device *) identdata;
	if (utype == NULL) {

		DEBUG_0("No utype match - using default.\n")
		if (identptr->gen_config & ID_ATAPI) {
			dev_type = (identptr->gen_config & ID_DEVICE_TYPE_MASK)
                        >> ID_DEVICE_TYPE_SHIFT;
			/*
			 * Only ATAPI CDROMs and tape drives supported
			 */

			if (dev_type == IDECDROM)
				utype = match_model_name(DEFLT_CDROM_TYP,
							mn_attr,num_nm);
			else if (dev_type == IDE_TAPE)
				utype = match_model_name(DEFLT_TAPE_TYP,
							mn_attr,num_nm);
		}
		else {
			/*
			 * Only ATA disk drives supported
			 */
			utype = match_model_name(DEFLT_DISK_TYP,mn_attr,num_nm);
		}

	}/*end if utype */
	return(utype);

}

/*
 * NAME   : match_model_name
 *
 * FUNCTION :
 *      This function finds the default unique type for a disk, CD-ROM,
 *      or tape.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is local to this module.
 *
 * DATA STRUCTURES:
 *
 * INPUT  : default model name attribute value.
 *
 * RETURNS:
 *
 */

char *
match_model_name(default_model_name,mn_attr,num_mn)
char         *default_model_name;       /* default model name attribute val */
struct mna   *mn_attr; 
int	     num_mn;
{
	char    *utype;                 /* ptr to unique type */
	int     i;                      /* loop variable */
        char    sstr[256];
        struct  PdAt PdAt;
	int     rc;

	/* set unique type pointer to NULL in case no match */
	utype = NULL;



	if (num_mn > 0) {
		/*
		 * If the caller supplied buffer
		 * of model_names paired with uniquetypes
		 * then we'll use it.
		 */
	
		/* look for a model name attribute matching identify data */
		for(i=0; i<num_mn; i++) {
			if (!strncmp(default_model_name, mn_attr[i].value2,
				     strlen(mn_attr[i].value2))) {
				utype = (char *)&mn_attr[i].value1;
				return(utype);
			}
		}
		 
	}
	else {

		sprintf(sstr,"attribute = model_name AND deflt = '%s'",default_model_name);
		DEBUG_1("get_ide_utype: calling odm_get for *%s*\n",sstr)

		rc = (int)odm_get_first(PdAt_CLASS,sstr,&PdAt);
		if (rc > 0) {
			/* 
			 * We found a match so set utype
			 */
			
			strcpy(mn_attr[0].value1,PdAt.uniquetype);
			utype = (char *)&mn_attr[0].value1;
			return(utype);
		}

	}
	return(utype);

}

/*
 ***************************************************************
 * NAME     : issue_ide_identify_device
 * FUNCTION : Performs the low level work to get the identify_device
 *            data from the device.
 * EXECUTION ENVIRONMENT:
 *    This is an internal subroutine called only from within
 *    ide device configuration methods.
 * NOTES    :
 *    - This routine expects the adapter to be opened and started
 *      by the caller.
 * RETURNS:
 *      0           if succeed
 *      E_DEVACCESS if fail
 ***************************************************************
 */
int
issue_ide_identify_device(int     adapfd  ,
                   uchar  *identdata ,
                   uchar  id      ,
		   int    *error,
		   uchar  *dev_type)
{
    struct ide_identify ident;
    int         try;
    int         rc;
    int		i;
    ushort	tshort;
    struct identify_device *device_ident;
    char	model_number_2X[8] = {"XF0ND1 E"}; 	/* Byte swapped model number FXN01DE */

/* BEGIN issue_ide_identify_device */

    /* Zero out identify device command structure */
    bzero(&ident, sizeof(ident));


    *error = 0;

    /* build identify device command structure */
    ident.identify_ptr = identdata;
    ident.identify_len = sizeof(struct identify_device);
    ident.flags        = 0;
    ident.ide_device   = id;


    for (try=0; try<3; try++)
    {
        rc = ioctl(adapfd, IDEIOIDENT, &ident);
        if (rc == 0)
        {
	    /* set the device type to either ATA or ATAPI */
	    *dev_type = ident.ata_atapi;

	    /* The data format is little endian shorts, so must change to big */
	    for ( i=0; i < sizeof(struct identify_device); i=i+2 ) {
		tshort = *(uchar *)(identdata+i) +
			((ushort)*((uchar *)(identdata+i+1))<<8);
		*((ushort *)(identdata+i)) = tshort;
	    }

	    /* The byte ordering endianess is not the same for all devices */
	    /* ATA-2 disks are little endian so above swap fixes order     */
	    /* old ATA disks (from a sample of 1 disk) are big endian, so  */
	    /*               must swap bytes back                          */
	    /* ATAPI CD-ROM (from a sample of 1 disk) are big endian, so   */
	    /*               must swap bytes back                          */
	    /* ATAPI TAPE    (from a sample of 1 tape) has flipped back &  */
	    /*               forth between firmware releases.  Last state  */
	    /*               was little endian.                            */
	    device_ident = (struct identify_device *)identdata;
	    if ((device_ident->gen_config & ID_ATAPI)) {
		if ( ((device_ident->gen_config & ID_DEVICE_TYPE_MASK) >> 
                       ID_DEVICE_TYPE_SHIFT) == IDECDROM) {
		    if (strncmp((char *)device_ident->model_number, model_number_2X, 8) == 0) {
			/* The model number is that of the 2X CDROM so swap the bytes */

		        for (i=0; i<20; i++) {
		            device_ident->model_number[i] = 
			        (device_ident->model_number[i] >> 8) |
			        ((device_ident->model_number[i] & 0x00ff) << 8);
		        }
		    }
		}
	    }
	    else if (!(device_ident->capabilities & (ID_CAP_DMA | ID_CAP_LBA)))
	    {
		for (i=0; i<20; i++) {
		    device_ident->model_number[i] = 
			(device_ident->model_number[i] >> 8) |
			((device_ident->model_number[i] & 0x00ff) << 8);
		}
	    }

            return(0);      /* Successful completion! */
        }

        /* Handle error conditions */
        if (try > 0)
        {
            if (errno == ENOCONNECT)
            {
                DEBUG_0("issue_ide_identify_device: ident returned ENOCONNECT\n") ;
            }
            else
            {
                DEBUG_1("issue_ide_identify_device: ident returned errno: %d\n",
                        errno) ;
                break;
            }
        }
    } /* END for loop */
    if (errno == ENODEV) 
	    *error = ENODEV;
    else
	    *error = -1;

    return(E_DEVACCESS);
} /* END issue_ide_identify_device */


