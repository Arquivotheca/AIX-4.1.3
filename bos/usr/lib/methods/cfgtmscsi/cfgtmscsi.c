static char sccsid[] = "@(#)77	1.4.1.3  src/bos/usr/lib/methods/cfgtmscsi/cfgtmscsi.c, cfgmethods, bos411, 9428A410j 3/30/94 13:02:21";
/*
 * COMPONENT_NAME: (CFGMETHODS) TMSCSI Config Method
 *
 * FUNCTIONS: generate_minor, make_special_files, download_microcode,
 * FUNCTIONS: query_vpd, add_desc_fixed, define_children, build_dds
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include	<stdio.h>
#include	<sys/errno.h>
#include	<sys/types.h>
#include	<sys/device.h>
#include	<sys/sysconfig.h>
#include	<sys/cfgdb.h>
#include	<sys/stat.h>
#include	<malloc.h>
#include	<fcntl.h>
#include	<sys/scsi.h>
#include	<sys/sysmacros.h>
#include	<sys/watchdog.h>
#include	<sys/tmscsidd.h>
#include 	<string.h>
#include	<sys/cfgodm.h>
#include	"cfgdebug.h"
#include	"cfghscsi.h"
#include	<cf.h>

/**********************************************************************
 * NAME: generate_minor
 *
 * FUNCTION: Generates a group of minor numbers for the tmscsi
 *
 * EXECUTION ENVIRONMENT:
 *
 * Dynamically loaded & run by generic config
 *
 * RETURNS:
 * The lowest of eight consecutive minor numbers, or -1 for an error
 * 
 ***********************************************************************/

long generate_minor(lname,major_no,minor_dest)
char *lname;
long major_no;	
long *minor_dest;
{
long	*genret;
long	*genminor();

	/* Request a list of two minor numbers. These numbers must be	*/
	/* contigous, starting on a multiple-of-two boundary		*/

	DEBUG_0("generate_minor()\n")

	if((genret = genminor(lname,major_no,-1,2,1,2)) == NULL)
	    return(E_MINORNO);
	*minor_dest = *genret;
	return(0);
}

/**********************************************************************
 * NAME: make_special_files
 *
 * FUNCTION: Generates two special files for the tmscsi
 *
 * EXECUTION ENVIRONMENT:
 *
 * Dynamically loaded & run by generic config
 *
 * NOTES:
 * 1. The files generated are /dev/<lname>.im(even minor no.),
 *			  and /dev/<lname>.tm(odd minor no.)
 * 2. The minor number passed in is simply the first in a sequence of two.
 *
 * RETURNS:
 * 0. if succeeded
 * Error_code, if failed
 * 
 **********************************************************************/

#define MSP_MAXLEN	14	/* Maximum length of a filename	*/

int make_special_files(lname,devno)
char *lname;
dev_t	devno;
{
int	suffix;		/* counter from 0 to 2		*/
int	rc;		/* Return code			*/
char basename[MSP_MAXLEN+1];	/* basename of special file */

	DEBUG_0("make_special_files()\n")

	/* Check that there is enough room in basename for r<lname>.?m */
	if(strlen(lname) > (MSP_MAXLEN-4))
		return(E_MKSPECIAL); 

	/* Create the character special files */
	/* File modes are rw-rw-rw- */

	sprintf(basename,"%s.im",lname);	/* initiator mode instance */
	if(rc = mk_sp_file(devno,basename,
		(S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP |
		 S_IWGRP | S_IROTH | S_IWOTH))) return(rc);
	sprintf(basename,"%s.tm",lname);	/* target mode instance */
	if(rc = mk_sp_file(devno+1,basename,
		(S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP |
		 S_IWGRP | S_IROTH | S_IWOTH))) return(rc);
	return(0);
}

/*********************************************************************
 * NAME: download_microcode
 *
 * FUNCTION: Downloads micro-code to device
 *
 * EXECUTION ENVIRONMENT:
 *
 * Dynamically loaded & run by generic config
 *
 * RETURNS:
 * 0. if succeeded
 * Error_code, if failed
 * 
 ************************************************************************/

int download_microcode(lname)
char *lname;
{

	DEBUG_0("Downloading Microcode\n")
	return(0);
}

/*************************************************************************
 * NAME: query_vpd
 *
 * FUNCTION: Obtains Vital Product Data From TMSCSI Device, and places it
 *				in the customized database
 *
 * EXECUTION ENVIRONMENT:
 *
 * Dynamically loaded & run by generic config
 *
 * RETURNS:
 * 0. if succeeded
 * Error_code, if failed
 * 
 *************************************************************************/

#define	INQSIZE	255

int query_vpd(cusobj,kmid,devno,vpd_dest)
struct	CuDv	*cusobj;	/* Pointer to customized object rec*/
mid_t	kmid;			/* Kernel module I.D. for Dev Driver	*/
dev_t	devno;			/* Concatenated Major & Minor No.s		*/
char	*vpd_dest;		/* destination for vpd			*/
{
uchar	inq_data[INQSIZE];	/* Storage area for inquiry data	*/
struct	cfg_dd cfg;		/* Command structure for ddconfig()	*/
char	adapter_name[20];	/* Name of scsi adapter device	*/
int	adapter_dev;		/* Adapter fileno		*/
uchar	scsi_id, lun_id;	/* Extracted from connwhere	*/
struct	sc_inquiry inq_pb;	/* Command structure		*/
int	rc;			/* Return code			*/
int	readcount = 0;		/* Count of number of attempts	*/
char	tmp_str[20];


	DEBUG_0("query_vpd: entered\n")
	if(get_sid_lun(cusobj->connwhere,&scsi_id,&lun_id))
		return(E_INVCONNECT);
	sprintf(adapter_name,"/dev/%s",cusobj->parent);
	if((adapter_dev = open(adapter_name,O_RDWR)) < 0){
		DEBUG_1("query_vpd:failed to open adapter %s\n",adapter_name)
		return(E_DEVACCESS);
	}

	/* Allocate resource */

	if(ioctl(adapter_dev,SCIOSTART,IDLUN((int)scsi_id,(int)lun_id)) < 0){
		close(adapter_dev);
		DEBUG_2("query_vpd:Failed to SCIOSTART dev at id%d,lun%d\n",
			(int)scsi_id,(int)lun_id)
		return(E_DEVACCESS);
	}
	bzero(&inq_pb, sizeof(struct sc_inquiry) ) ;


	/* Initialize Inquiry Control Structure */

	do {
		inq_pb.scsi_id = scsi_id;
		inq_pb.lun_id = lun_id;
		inq_pb.inquiry_len = (uchar)INQSIZE;	/* Note 256 was 0 */
		inq_pb.inquiry_ptr = inq_data;
		DEBUG_4("ioctl() called with: sid %d lun %d inq_len %d inq_ptr %d\n",
			(int)inq_pb.scsi_id,
			(int)inq_pb.lun_id,
			(int)inq_pb.inquiry_len,
			(int)inq_pb.inquiry_ptr);

		rc = ioctl(adapter_dev,SCIOINQU,&inq_pb);
		DEBUG_1("ioctl() err. errno = %d\n",errno)
		readcount++;

	} while(rc < 0 && (errno==EIO || errno==ETIMEDOUT) && readcount<=2);

#ifdef CFGDEBUG
	if(rc == 0) {
		hexdump(inq_data,(long)inq_data[4]+5);
	} else {
		switch(errno){
		case EIO:
		case ETIMEDOUT:
			DEBUG_2("TIMEOUT / EIO on sid %d, lun %d\n",
				(int)scsi_id,(int)lun_id)
			break;
		case ENODEV:
			DEBUG_2( "NO Device on sid %d, lun %d\n",
				(int)scsi_id,(int)lun_id)
			break;
		default:
			DEBUG_3("Error %d on sid %d,lun %d\n",
				errno,(int)scsi_id,(int)lun_id)
			break;
		}
	}
#endif	CFGDEBUG

	/* Release the resource */

	if(ioctl(adapter_dev,SCIOSTOP,IDLUN((int)scsi_id,(int)lun_id)) < 0){
		DEBUG_1( "Error %d releasing device\n", errno );
		rc = E_DEVACCESS;
	}

	close(adapter_dev);

        if (((rc !=0) && (errno == ENODEV))  &&
            (strcmp(cusobj->PdDvLn_Lvalue, "tmscsi/scsi/tmscsi") == 0)) {
            DEBUG_0("query_vpd : ENODEV but tmscsi unique type\n");
            return(0);
        }

	if(rc) return(rc);

	/*	make sure that the device we are looking at is	*/
	/*	the same type as the database says it is.	*/

	rc = chktype(inq_data,cusobj->PdDvLn_Lvalue);
	if(rc){
		DEBUG_1("query_vpd: chktype failed. rc = %d\n",rc)
		return(rc);
	}

	/*	Store the VPD in the database	*/

	*vpd_dest = '\0';
	sprintf(tmp_str,"%02x%02x",(int)inq_data[0],(int)inq_data[1]);
	add_descriptor(vpd_dest,"Z0",tmp_str);
	add_desc_fixed("MF",vpd_dest,inq_data,8,8);	/* manufacturer */
	add_desc_fixed("TM",vpd_dest,inq_data,16,16);	/* part number */
	DEBUG_0("VPD stored\n")
	return(0);
}

/************************************************************************/
/* NAME	: chktype							*/
/************************************************************************/
int chktype(inqdata,utype)
uchar	*inqdata;
char	*utype;
{
int	i,obj_flag,rc;
struct	PdAt PdAt;
char	sstr[256];
char	model_name[20];


	/* built model_name attribute value from inquiry data */
	for (i=0; i<16; i++) {
		char	c;
		c = (char) inqdata[i+16];
		if( c < ' ' || c >= '\177' )
			c = ' ';
		model_name[i] = c;
	}
	model_name[i] = '\0';

	DEBUG_1("chktype: passed in utype=%s*\n",utype)
	DEBUG_1("chktype: model_name=%s\n",model_name)

	/* make sure inquiry data is for a processor type device */
	if ((uchar)(inqdata[0] & 0x1F) != (uchar)0x03) {
		DEBUG_0("Device is of wrong class entirely\n")
		return (E_WRONGDEVICE);	/* Device class is incorrect */
	}

	sprintf(sstr,"attribute = 'model_name' AND deflt = '%s'",model_name);
	DEBUG_1("chktype: calling odm_get for *%s*\n",sstr)
	rc = (int)odm_get_obj(PdAt_CLASS,sstr,&PdAt,ODM_FIRST);
	if (rc == 0) {
		DEBUG_0("chktype: unable to find tmscsi type match\n")
                /* set model name to default for target mode, search again */
	        sprintf(sstr,"attribute = 'model_name' AND deflt = '%s'",
                        DEFLT_TM_TYP);
	        rc = (int)odm_get_obj(PdAt_CLASS,sstr,&PdAt,ODM_FIRST);
                if (rc == 0) {
                   DEBUG_0("Unable to read default target mode type\n")
                   return(E_WRONGDEVICE);
                }
	}
	if (rc == -1) {
		DEBUG_1("Error searching for device type using\n%s\n",sstr)
		return(E_ODMGET);
	}

	DEBUG_1("chktype: PdAt utype=%s*\n",PdAt.uniquetype)
	/* If device is of correct class, & subclass, break out */
	if (strcmp(PdAt.uniquetype,utype)) {
		DEBUG_2("chktype: tape=%s object=%s\n",PdAt.uniquetype,utype)
		return(E_WRONGDEVICE);
	}
	return(0);
}

/************************************************************************
 * NAME   : add_desc_fixed
 *
 * FUNCTION : Adds a fixed-length descriptor to the VPD from the
 *	Inquiry data returned from the disk.
 *	The Inquiry data is also examined to ensure that enough
 *	data is available.
 *
 * EXECUTION ENVIRONMENT:
 *	This function operates as a device dependent subroutine
 *	called by the generic configure method for all devices.
 *
 * DATA STRUCTURES:
 *
 * INPUTS : Two character name of descriptor, Destination VPD,
 *	Inquiry data from disk, Offset within inq-data, and Length to copy.
 *
 * RETURNS: NONE
 *
 *************************************************************************/

add_desc_fixed(desc_name,vpd_dest,inq_data,offset,len)
char	*desc_name;		/* Two character name of descriptor	*/
char	*vpd_dest;		/* VPD field within object		*/
char	*inq_data;		/* Inquiry data containing data		*/
int	len,offset;		/* Offset within inq_data of reqd data	*/
{
char	tmp_space[100];

	if(offset+len > 4+(int)inq_data[4])
		return;		/* The inq_data returned was too short	*/
	strncpy(tmp_space,&inq_data[offset],len);
	tmp_space[len] = '\0';
	add_descriptor(vpd_dest,desc_name,tmp_space);
}

/*********************************************************************
 * NAME: define_children
 *
 * FUNCTION: Scans for child devices
 *
 * EXECUTION ENVIRONMENT:
 *
 * Dynamically loaded & run by generic config
 *
 * The TMSCSI do not have child devices, so this routine always succeeds
 *
 * RETURNS:
 * 0. if succeeded
 * Error_code, if failed
 * 
 ********************************************************************/

int define_children(lname,phase)
char	*lname;
int	phase;
{
	DEBUG_0("define_children()\n")
	return(0);
}

/**********************************************************************
 * NAME: build_dds
 *
 * FUNCTION: Device dependant dds build for TMSCSI
 *
 * EXECUTION ENVIRONMENT:
 *
 * Dynamically loaded routine.
 *
 * RETURNS:	0 if succeed, or errno if fail
 **********************************************************************/

int build_dds(lname,dds_data_ptr,dds_len)
char	*lname;
char	**dds_data_ptr;
int	*dds_len;
{
struct	tm_ddi	*dds;	/* dds for TMSCSI			*/
char	search_str[128];	/* Search criterion			*/
struct	Class	*c_att_oc;	/* Customized Attribute Object Class	*/
struct	Class	*p_att_oc;	/* Predefined Attribute Object Class	*/
struct	Class	*c_dev_oc;	/* Customized Devices Object Class	*/
struct	CuDv	cusdesc;	/* Customized Device Object	*/
int	rc,how_many;
int     myrc ;
char	pname[NAMESIZE],ut[UNIQUESIZE],pt[UNIQUESIZE],tm[4];
long	major_no;
long	minor_no;

long	*getminor();
long	genmajor();

	DEBUG_0( "build_dds()\n" )

	if((dds = (struct tm_ddi *)malloc(sizeof(struct tm_ddi)))
		== NULL) {
		DEBUG_0("malloc for struct tm_ddi{} failed\n")
		return(E_MALLOC);
	}

	/* Open required Object Classes */

	if((c_att_oc = odm_open_class(CuAt_CLASS)) == NULL){
		DEBUG_0("build_dds() for TMSCSI: Error opening CuAt\n")
		return(E_ODMOPEN);
	}
	if((p_att_oc = odm_open_class(PdAt_CLASS)) == NULL){
		DEBUG_0("build_dds() for TMSCSI: Error opening PdAt\n")
		return(E_ODMOPEN);
	}
	if((rc = c_dev_oc = odm_open_class(CuDv_CLASS)) == NULL){
		DEBUG_0("build_dds() for TMSCSI: Error opening CuDv\n")
		return(E_ODMOPEN);
	}
	DEBUG_0("CuDv open\n")

	/* Read CuDv for TMSCSI */

	sprintf(search_str,"name = '%s'",lname);
	DEBUG_1("Performing odm_get_obj(CuDv,%s)\n",search_str)
	if((rc = (int)odm_get_obj(c_dev_oc,search_str,&cusdesc,TRUE)) == 0){
		DEBUG_1("build_dds():Record not found in CuDv: %s\n",search_str)
                return(E_NOCuDv);
	}
	else if(rc == -1){
		DEBUG_0("build_dds():for TMSCSI:Error Reading CuDv\n")
                return(E_ODMGET);
	}
	DEBUG_1("odm_get_obj(CuDv,%s)succeeded\n",search_str)
	strcpy(ut,cusdesc.PdDvLn_Lvalue);
	if(get_sid_lun(cusdesc.connwhere,&dds->scsi_id,&dds->lun_id)){
                return(E_DDS);
	}
	DEBUG_2("build_dds: scsi_id = %d lun_id = %d\n",dds->scsi_id,
		dds->lun_id)

	/* Read CuDv for parent (SCSI-Adapter) */

	sprintf(search_str,"name = '%s'",cusdesc.parent);
        strcpy(pname, cusdesc.parent);
	DEBUG_1("performing odm_get_obj(CuDv,%s)\n", search_str)
	if((rc = (int)odm_get_obj(c_dev_oc,search_str,&cusdesc,TRUE)) == 0){
		DEBUG_1("build_dds():Record not found in CuDv: %s\n",
			search_str)
                return(E_NOCuDv);
	}
	else if(rc == -1) {
		DEBUG_0("build_dds() for TMSCSI: Error Reading CuDv\n")
                return(E_ODMGET);
	}

	DEBUG_0("odm_get_obj() succeeded\n")

        strcpy(pt, cusdesc.PdDvLn_Lvalue);

	/* Generate Parent's devno */

	major_no = genmajor(cusdesc.ddins);
	minor_no = *getminor(major_no,&how_many,cusdesc.name);
	dds->adapter_devno = makedev(major_no,minor_no);
	DEBUG_1("dds->adapter_devno = %ld\n",dds->adapter_devno)

	/* Read the attributes from the customized attribute class */

        if((rc = getatt(tm, 's',c_att_oc, p_att_oc,pname,pt,
                 "tme", NULL))>0)
           return(rc);
        DEBUG_1("build_dds:tme=%s\n", tm) 
 
        /* if the target mode interface enabled attribute is set to no */
        /* then fail config                                            */
        if(strcmp(tm, "no") == 0) {
           return(E_DDS);
        }

        myrc = E_PARENT;
        sprintf (search_str, "name = '%s'", pname);
        DEBUG_1("build_dds: Searching for intr_priority. getting CuDv for %s\n",
		search_str);
        rc = (int)odm_get_first (c_dev_oc, search_str, &cusdesc);

        while ((rc != 0) && (myrc == E_PARENT)) 
        {
            if (rc == -1)
	    {
	        myrc = E_ODMGET ;	/* ODM error occurred; abort out.   */
	    }
            else if (strncmp (cusdesc.PdDvLn_Lvalue, "adapter/", 
			      strlen ("adapter/")) == 0)
            {
	        myrc = 0 ;		/* Got object.		   */
	    }
	    else
	    {
	        sprintf (search_str,"name = '%s'", cusdesc.parent);
	        DEBUG_1("build_dds: Searching for intr_priority. getting CuDv for %s\n",
			search_str);
		rc = (int)odm_get_first (c_dev_oc, search_str, &cusdesc);
            }
        }
        if((rc = getatt (&dds->int_prior, 'i', c_att_oc, p_att_oc, cusdesc.name,
			 cusdesc.PdDvLn_Lvalue, "intr_priority",NULL)) > 0)
	    return(rc);

        DEBUG_1("build_dds:int_prior=%d\n", dds->int_prior) 

	if((rc = getatt(&dds->num_bufs,'i',c_att_oc,p_att_oc,lname,ut,
		"num_bufs",NULL))>0)
           return(rc);
	DEBUG_1("build_dds:num_bufs = %d\n",dds->num_bufs)

	if((rc = getatt(&dds->buf_size,'i',c_att_oc,p_att_oc,lname,ut,
		"buffer_size",NULL))>0)
           return(rc);
	DEBUG_1("build_dds: buffer_size = 0x%x\n",dds->buf_size)

	strncpy(dds->resource_name,lname,sizeof(dds->resource_name));

	*dds_data_ptr = (char *)dds;		/* Store address of struct */
	*dds_len = sizeof(struct tm_ddi);/* Store size of structure */

#ifdef CFGDEBUG
	dump_dds(*dds_data_ptr,*dds_len);
#endif

	DEBUG_0("build_dds()returning 0\n")

        odm_close_class(c_att_oc);
        odm_close_class(p_att_oc);
        odm_close_class(c_dev_oc);
}
