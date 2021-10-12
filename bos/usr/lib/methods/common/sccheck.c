static char sccsid[] = "@(#)21  1.2  src/bos/usr/lib/methods/common/sccheck.c, cfgmethods, bos411, 9432A411a 8/5/94 11:40:54";
/*
 * COMPONENT_NAME: (CFGMETHODS) SCSI Common check routine
 *
 * FUNCTIONS: chktype, chkinq get_scsi_utype, 
 *            match_model_name, issue_scsi_inquiry
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993,1994
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
#include        "cfgdebug.h"
#include        "cfghscsi.h"
#include        <cf.h>

#define INQ_SIZE      255

/* forward function definitions */

int	chktype(uchar*, char*, int,char*,uchar, uchar);
int	chkinq(uchar*, struct PdAt*, int);
char    *get_scsi_utype(uchar *,struct PdAt*, int,struct mna*, 
			int,int,uchar, uchar);
char    *match_model_name(char*,struct mna*,int);
int     issue_scsi_inquiry(int, uchar*, uchar, uchar, int,int*) ;

/*
 * NAME: chktype
 *
 * FUNCTION: Validates the unique type passed is correct by comparing
 *           inquiry data with attributes in the database.
 *
 * EXECUTION ENVIRONMENT:
 *
 *    This is an internal subroutine called by query_vpd() and disk_present()
 *    in the device configuration methods.
 *
 * RETURNS:     0 if success
 *		E_WRONGDEVICE if inquiry data type is incorrect
 *		E_ODMGET if couldn't access ODM 
 *		 	
 *	 	
 */

int
chktype(inqdata,utype,devtype,parent_name,id,lun)
uchar   *inqdata;			/* ptr to inquiry data		*/	
char    *utype;				/* ptr to device's uniquetype   */
int     devtype;			/* type= disk,tape,cd,rw   	*/
char    *parent_name;			/* logical name of parent	*/
uchar   id,lun;                         /* SCSI id and lun of device    */
{
	struct mna chk_mna;
        int     i,rc;
        struct  PdAt PdAt;
        char    sstr[256];
        char    model_name[17];
	struct	listinfo	list;		/* number  model_map attribs*/
	struct 	PdAt	 	*pdatobj_ptr;   /* list of model_map attribs*/
        char    c;
	char	deflt_model_name[30];
	int	inq_len;			/* length of inquiry data   */
	char	*inq_ptr;			/* ptr to inquiry data      */
        int     adap_fd;                        /* file desc for scsi adptr */
	char    *utype2;
	char     adp_name[32] ;                 /* spec file name of scsi adptr */


	/* ensure the inquiry data matches the correct type of device */

	if (devtype != ( (uchar)inqdata[0] & 0x1F) ) {
		DEBUG_0("Device is of wrong class entirely\n")
		return(E_WRONGDEVICE);	
	}

	sprintf(adp_name, "/dev/%s", parent_name) ;

	if ((adap_fd = open(adp_name, O_RDWR)) <  0) {
		return(E_DEVACCESS);
	}
        if (ioctl(adap_fd, SCIOSTART, IDLUN((int)id, (int)lun)) != 0) {
		return(E_DEVACCESS);
	}



	/* search for all model_map attributes for this device  */

	sprintf(sstr, "uniquetype='%s' AND attribute=model_map",utype);
	pdatobj_ptr = odm_get_list(PdAt_CLASS, sstr, &list, 4, 1);

	if ( (int)pdatobj_ptr == -1 ) {
		DEBUG_0("error getting PdAt object list info\n")
		return(E_ODMGET);
	}

	utype2 = get_scsi_utype(inqdata,pdatobj_ptr,list.num,
				(struct mna *) &chk_mna,0,
				adap_fd,id,lun);

	if (ioctl(adap_fd, SCIOSTOP, IDLUN((int)id, (int)lun)) != 0) {
                DEBUG_2("Failed to SCIOSTOP device at scsi %d lun %d\n",
                (int)id, (int)lun) ;
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
 * NAME: chkinq
 *
 * FUNCTION: Compares the inquiry data with the "model_map" attributes
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
chkinq(inqdata,obj_ptr,list_num)
uchar		*inqdata;
struct	PdAt	*obj_ptr;
int		list_num; 
{
        int     i;
	char	*ptr;				/* ptr used in comparison   */
	int	offset;				/* offset into inquiry data */
	int	length;				/* length of inquiry data   */
	int	found_flag;			/* for determining if match */
	char	*deflt_end;			/* ptr to end of dflt field */

	/* if non-printable character is found in the inquiry data buffer,
	   change it to a blank.  \177 is after a tilde in the list of
	   hex characters.  Start at 5 to skip the header		*/

	for ( i=5; i <= sizeof(struct inqry_data) - 5 ; i++ )
		if (inqdata[i] < ' ' || inqdata[i] >= '\177' )
			inqdata[i] = ' ';


	/* Loop through the PdAt model_map attributes until a     
	   comparison string matches.  An example of the deflt field:

		SSTTchar_string1,UUVVchar_string2/char_string3

		where:
		SS is inquiry data offset
		TT is the inquiry data length
		char_string1 is the character string to compare with
	 	UU is the second field in the inquiry data offset
		VV is the second length of inquiry data 
		char_string2 OR char_string3 must be present in the inq data
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

				if (strncmp(inqdata+ offset, ptr,length)== 0) {

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
 * NAME: get_scsi_utype
 *
 * FUNCTION: Finds a SCSI devices uniquetype.
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
get_scsi_utype(inqdata,obj_ptr,list_num,mn_attr,num_nm,adapter,id,lun)
uchar		*inqdata;
struct	PdAt	*obj_ptr;
int		list_num;
struct mna	*mn_attr; 
int		num_nm;				/* Number of struct mn_attrs*/
						/* NOTE: if num_nm, there   */
						/* still must be one there  */
						/* return the utype to the  */
						/* caller		    */
int     	adapter;                        /* adapter file descriptor */
uchar   	id,lun;                         /* SCSI id and lun of device */
{
	char    *utype;                         /* pointer to unique type */
        int     i;
	int     index;                         /* index into PdAt object array */
        struct  PdAt PdAt;
        char    sstr[256];
        char    model_name[17];
	char	deflt_model_name[30];
	int	inq_len;			/* length of inquiry data   */
	char	*inq_ptr;			/* ptr to inquiry data      */
	struct  scsd_inqry_header scsd_inq;	/* struct for SCSD test     */
	int 	pg=0xc7;		        /* pg for SCSD test         */
	int	rc=0;			        /* return code		    */
        int 	error;			        /* Error issue_scsi_inquiry  */
	struct  inqry_data *inqptr;

	/* set uniquetype pointer to NULL in case no match */
	utype = NULL;

	if ( list_num != 0 ) {		/* retrieved at least 1 attr   */
 		/* chkinq compares the inquiry data with the attributes */
		index = chkinq(inqdata, obj_ptr, list_num);
		if (index != -1 ) {
			utype = (char *)&obj_ptr[index].uniquetype;
			return (utype);	/* success, match was found */
		}
	}


	/* no match on model_map attribute, try to extract the model_name   */

        /* The inquiry data consists of a 5 byte header followed by optional
         * vendor unique data. The fifth byte of the header indicates the
         * length of the optional vendor data. We use 16 bytes starting with
         * the 12th byte of the optional vendor data to compare against our
         * model name attributes in the CuAt database. */

	inq_len = inqdata[LENGTH_BYTE];   /* get length of opt. vendor data*/
	inq_ptr = (char *)inqdata + LENGTH_BYTE + 1 ; /* pt to opt. vendor
							  area              */	

	/* copy model name data from inquiry data, convert certain characters
	   to blanks */

	for (i=0;  i<16; i++ )  {
		if (inq_len <= i+11) {
			/* just use blanks if past end of inquiry data 	*/
			model_name[i] = ' ';
		}
		else {
			if ( inq_ptr[i+11] > ' ' && inq_ptr [i+11] < '\177' ){
				model_name[i] = inq_ptr[i+11];
			}
			else {
				model_name[i] = ' ';
			}
		}
	}
        model_name[16] = '\0'; 
  
	/* set unique type pointer to NULL in case no match */
	utype = NULL;

        DEBUG_1("get_scsi_utype: model_name=%s\n",model_name)

	utype = match_model_name(model_name,mn_attr,num_nm);



	inqptr = (struct inqry_data *) inqdata;
	if (utype == NULL) {

		DEBUG_0("No utype match - using default.\n")

		/* Let's check if this device  is an SCSD */
		/* (Self Configuring SCSI Device). Such a */
		/* device will have a inquiry data page   */
		/* of 0xC7 and bytes 5-8 will say "SCDD", */
		/* but will not have a match in ODM 	  */
		/* for model_name nor model_map	   	  */
			 
                rc = issue_scsi_inquiry(adapter,(uchar *)&scsd_inq,id,lun,pg,&error);

	        if ((rc == 0 ) && 
		    (strncmp(scsd_inq.scsd_id_field,"SCDD",4)==0)) {

			    /* This is an SCSD device. So determine */
			    /* which kind based on peripheral device*/
			    /* type.				    */
			    
			    if (inqptr->pdevtype == SCSI_DISK)
				    utype = match_model_name(SCSD_DISK_TYP,
							     mn_attr,num_nm);
			    else if (inqptr->pdevtype == SCSI_TAPE) {
				    /*
				     * For future SCSD support
				     *
				     * utype = match_model_name(SCSD_TAPE_TYP,
				     * mn_attr,num_nm);
				     */

				    /* 
				     * Treat as OEM until SCSD tape support
				     * is dropped
				     */
				    utype = match_model_name(DEFLT_TAPE_TYP,
							     mn_attr,num_nm);
			    }
			    else if (inqptr->pdevtype == SCSI_PROC) {
				    /*
				     * If a target mode device supports
				     * the SCSD page let's configure it
				     * as an OEM TM.
				     */
				    utype = match_model_name(DEFLT_TM_TYP,
							     mn_attr,num_nm);
			    }
			    else if (inqptr->pdevtype == SCSICDROM)
				    utype = match_model_name(SCSD_CDROM_TYP,
							     mn_attr,num_nm);
			    else if (inqptr->pdevtype == SCSI_RWOPT)
				    utype = match_model_name(SCSD_RWOPT_TYP,
							     mn_attr,num_nm);
			    
			    /* NOTE: if the SCSD device is not one */
			    /* of the types we know then will drop */
			    /* through without a match.	           */ 



                }/* end if SCSD */
		else {

			/* This is not an SCSD device	*/

			if (inqptr->pdevtype == SCSI_DISK)
				utype = match_model_name(DEFLT_DISK_TYP,mn_attr,num_nm);
			else if (inqptr->pdevtype == SCSI_TAPE)
				utype = match_model_name(DEFLT_TAPE_TYP,mn_attr,num_nm);
			else if (inqptr->pdevtype == SCSI_PROC)
				utype = match_model_name(DEFLT_TM_TYP,mn_attr,num_nm);
			else if (inqptr->pdevtype == SCSICDROM)
				utype = match_model_name(DEFLT_CDROM_TYP,mn_attr,num_nm);
			else if (inqptr->pdevtype == SCSI_RWOPT)
				utype = match_model_name(DEFLT_RWOPT_TYP,mn_attr,num_nm);
		}/* end else */

	}/*end if utype */
	return(utype);


}

/*
 * NAME   : match_model_name
 *
 * FUNCTION :
 *      This function finds the default unique type for a disk, CD-ROM,
 *      tape, read/write optical, or target mode device.
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
	
		/* look for a model name attribute matching inquiry data */
		for(i=0; i<num_mn; i++) {
			if (!strcmp(default_model_name,mn_attr[i].value2)) {
				utype = (char *)&mn_attr[i].value1;
				return(utype);
			}
		}
		 
	}
	else {

		sprintf(sstr,"attribute = model_name AND deflt = '%s'",default_model_name);
		DEBUG_1("get_scsi_utype: calling odm_get for *%s*\n",sstr)

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
 * NAME     : issue_scsi_inquiry
 * FUNCTION : Performs the low level work to get the inquiry
 *            data from the device.
 * EXECUTION ENVIRONMENT:
 *    This is an internal subroutine called only from within
 *    scsi device configuration methods.
 * NOTES    :
 *    - This routine expects the adapter to be opened and started
 *      by the caller.
 * RETURNS:
 *      0           if succeed
 *      E_DEVACCESS if fail
 ***************************************************************
 */
int
issue_scsi_inquiry(int     adapfd  ,
                   uchar  *inqdata ,
                   uchar  sid      ,
                   uchar  lun      ,
                   int    code_page,
		   int    *error)
{
    struct sc_inquiry inq;
    int         try;
    int         tried_async = 0 ;
    int         rc;

/* BEGIN issue_scsi_inquiry */

    /* Zero out inquiry command structure */
    bzero(&inq, sizeof(struct sc_inquiry));


    *error = 0;

    /* build inquiry command structure */
    inq.scsi_id = sid;
    inq.lun_id  = lun;
    inq.inquiry_len = (uchar)INQ_SIZE;
    inq.inquiry_ptr = inqdata;

   /*
    *-------------------------------------------------------------------
    *     get_extended  - indicates extended inquiry requested
    *             (TRUE= extended; FALSE=standard).
    *     code_page_num - identifies the code page requested if this is an
    *             extended unquiry.
    *-------------------------------------------------------------------
    */
    if (code_page != (int)NO_PAGE)
    {
        inq.get_extended = TRUE ;
        inq.code_page_num = (uchar)code_page ;
    }

    for (try=0; try<3; try++)
    {
        rc = ioctl(adapfd, SCIOINQU, &inq);
        if (rc == 0)
        {
            return(0);;      /* Successful completion! */
        }

        /* Handle error conditions */
        if (try > 0)
        {
            if ((errno == ENOCONNECT) && (tried_async == 0))
            {
                DEBUG_0("issue_scsi_inquiry: inquiry returned ENOCONNECT\n") ;
                DEBUG_0("....now trying ASYNC\n") ;
                inq.flags = SC_ASYNC;
                tried_async = 1;
            }
            else
            {
                DEBUG_1("issue_scsi_inquiry: inquiry retruned errno: %d\n",
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
} /* END issue_scsi_inquiry */


