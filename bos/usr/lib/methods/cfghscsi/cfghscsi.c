static char sccsid[] = "@(#)09	1.35.3.6  src/bos/usr/lib/methods/cfghscsi/cfghscsi.c, cfgmethods, bos412, 9446B 10/28/94 10:26:04";
/*
 *   COMPONENT_NAME: CFGMETHODS
 *
 *   FUNCTIONS: build_dds
 *		download
 *		download_microcode
 *		generate_minor
 *		get_adap_lvl
 *		hscsi_download_microcode
 *		make_special_files
 *		query_vpd
 *		set_tme_attr
 *		
 *
 *   ORIGINS: 27, 83
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1,  5 Years Bull Confidential Information
 */

/* header files needed for compilation */
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/device.h>
#include <cf.h>
#include <fcntl.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <sys/sysconfig.h>
#include <sys/scsi.h>
#include <sys/mdio.h>
#include "cfghscsi.h"
#include "cfgdebug.h"



/*	extern declarations	*/
extern	int	errno;
extern	int	Dflag;
extern	long	*genminor();
extern  char    device_state;      /* for determining the previous state of */
                                   /* the adapter, this will indicate if a  */
                                   /* download of microcode is necessary    */

/* static declarations          */
char tmp_vpd[VPDSIZE] = {'\0'};

int mode = 1;

/*
 * NAME   : generate_minor
 *
 * FUNCTION: This function generates minor device number for the 
 *	     SCSI adapter device.
 * 
 * EXECUTION ENVIRONMENT:
 *	This function operates as a device dependent subroutine 
 *	called by the generic configure method for all devices.
 *	It makes use of generic genminor() function.
 *
 * DATA STRUCTURES:
 *
 * INPUTS : device logical_name,major_no.
 *
 * RETURNS: an integer pointer to the minor number.
 *
 * RECOVERY OPERATION:
 * 
 */

long generate_minor(lname,major_no,minordest)
char	*lname;
long	major_no;
long	*minordest;
{
long	*minorno;

	minorno = genminor(lname,major_no,-1,1,1,1);
	if(minorno == (long *)NULL)
		return E_MINORNO;
	*minordest = *minorno;
	return 0;
}

/*
 *
 * NAME   : make_special_files
 * 
 * FUNCTION: creates scsi device special file.
 *	     scsi has one charcter type device special file.
 * 
 * EXECUTION ENVIRONMENT:
 *	This function operates as a device dependent subroutine
 *	called by the generic configure method for all devices.
 *
 * DATA STRUCTURES:
 *
 * INPUTS : logical_name,devno
 *
 * RETURNS: Returns 0 if success else -1
 *
 * RECOVERY OPERATION:
 * 
 */

int make_special_files(lname,devno)
char	*lname;
dev_t	devno;
{
	return(mk_sp_file(devno,lname,CRF));
}


/*
 * NAME	: query_vpd
 *
 * FUNCTION :
 *	this routine gets vpd data from scsi adapter using
 *	sysconfig system call and stores it in the databse.
 *
 * EXECUTION ENVIRONMENT:
 *	This function is called by generic config method.
 *
 * DATA STRUCTURES:
 *	bb_vpd	: structure to hold vpd.
 *
 * INPUT  : pointer to CuDv, driver kmid, devno
 *
 * RETURNS: Returns 0 if success else -1
 *
 * RECOVERY OPERATION:
 *
 */

int query_vpd(cusobj,kmid,devno,vpd)
struct	CuDv *cusobj;
mid_t	kmid;
dev_t	devno;
char	*vpd;
{
#define SCSI_KEY "SCSI"			/* get all vpd in this case */
   
    int		   rc;
    int		   len=VPDSIZE, i, fd;
    char           sstring[30];
    char           sio_parent[] = "adapter/sio" ;
    MACH_DD_IO     read_record, write_record6, write_record7;
    uchar          tmp_read, tmp_write6, tmp_write7, slot, ioerr;
              

/* begin query_vpd */

    DEBUG_2("query_vpd: devno = 0x%x, kmid = 0x%x\n",devno,kmid)

  /* 
   *-----------------------------------------------------------------
   * Check to see if device is a child of SIO.  If it is, get the
   * VPD information from the parent's VPD.
   *----------------------------------------------------------------- 
   */ 
   DEBUG_1("query_VPD: uniquetype = %s\n", cusobj->PdDvLn_Lvalue)
   DEBUG_1("query_VPD: tmp_vpd = %s\n", tmp_vpd)

   /* if this is the first invocation then tmp_vpd[0]== '\0' so vpd must */
   /* be obtained.  If it does begin with the '\0' character then we have*/
   /* already obtained vpd so no need to query it again                  */

   if (tmp_vpd[0] == '\0') {
       if (!strncmp(cusobj->PdDvLn_Lvalue, sio_parent, strlen(sio_parent)))
       {
           DEBUG_0("query_vpd: getting integrated SCSI VPD") ;
	   if ((rc = get_vpd(tmp_vpd, SCSI_KEY)) != 0)
	   {
	       DEBUG_1("query_vpd: Unable to get subdvc VPD, rc = %d\n",
	                rc)
	       return(rc) ;
	   }
       }
       else /* Not an integrated SCSI, follow "normal" path */
       {
           DEBUG_0("query_vpd: getting MCA attached SCSI VPD\n") ;
           
           /* use machine dd to access pos registers to read vpd */

           /* get the slot this adapter is in */
           slot = atoi(cusobj->connwhere) - 1;
           DEBUG_1("cfghscsi query_vpd : slot = 0x%x\n", slot)

           /* initialize ioctl structures used to read pos registers */
           bzero(&read_record, sizeof(MACH_DD_IO)); 
           bzero(&write_record6, sizeof(MACH_DD_IO)); 
           bzero(&write_record7, sizeof(MACH_DD_IO)); 
           read_record.md_size = 1; /* transfer 1 byte */
           read_record.md_incr = MV_BYTE;
           read_record.md_data = &tmp_read;
           read_record.md_addr = POSREG(3, slot);

           write_record6.md_size = 1; /* transfer 1 byte */
           write_record6.md_incr = MV_BYTE;
           write_record6.md_data = &tmp_write6;
           write_record6.md_addr = POSREG(6, slot);

           write_record7.md_size = 1; /* transfer 1 byte */
           write_record7.md_incr = MV_BYTE;
           write_record7.md_data = &tmp_write7;
           write_record7.md_addr = POSREG(7, slot);

           sprintf(sstring, "/dev/%s", cusobj->parent);
           DEBUG_1("sstring = %s\n", sstring) 

           if ((fd = open(sstring, O_RDWR, 0)) == -1) {
              DEBUG_2("open of %s failed, errno = %d \n", sstring, errno)
              return(E_DEVACCESS);
           }

           ioerr = FALSE; /* init to no i/o errors */
        
           tmp_write7 = 0x00;
           for (i = 0; i < 255; i++) {
               if (ioctl(fd, MIOCCPUT, &write_record7) == -1) {
                   ioerr = TRUE;
                   break;
                }

                tmp_write6 = 0x01 + i;
                if (ioctl(fd, MIOCCPUT, &write_record6) == -1) {
                    ioerr = TRUE;
                    break;
                }

                if (ioctl(fd, MIOCCGET, &read_record) == -1) {
                    ioerr = TRUE;
                    break;
                }

                tmp_vpd[i] = tmp_read; 

            } /* end for */

            /* leave POS6 in known state */
            tmp_write6 = 0x00;
            (void) ioctl(fd, MIOCCPUT, &write_record6);
            tmp_write7 = 0x01;
            (void) ioctl(fd, MIOCCPUT, &write_record7);
            (void) ioctl(fd, MIOCCGET, &read_record);
            (void) ioctl(fd, MIOCCPUT, &read_record);
            tmp_write6 = 0x01;
            (void) ioctl(fd, MIOCCPUT, &write_record6);
            (void) ioctl(fd, MIOCCGET, &read_record);
            (void) ioctl(fd, MIOCCPUT, &read_record);
            tmp_write6 = 0x02;
            (void) ioctl(fd, MIOCCPUT, &write_record6);
            (void) ioctl(fd, MIOCCGET, &read_record);
            (void) ioctl(fd, MIOCCPUT, &read_record); 
            tmp_write7 = 0x00;
            (void) ioctl(fd, MIOCCPUT, &write_record7);
            tmp_write6 = 0x01;
            (void) ioctl(fd, MIOCCPUT, &write_record6);

            DEBUG_1("cfghscsi query_vpd : done reading vpd ioerr = %d\n", ioerr)
   
            close(fd); /* close machine dd */

            if (ioerr) return(E_SYSCONFIG);

        }
    }
    put_vpd(vpd, tmp_vpd, len);

    return(0);	
} /* end query_vpd */


/*
 * NAME : build_dds
 * 
 * FUNCTION :
 *	This function builds the dds from ODM database.
 *	DDS consists of attrbutes of the scsi adapter.
 *	It returns the pointer to dds to the generic config method
 *	which  inturn passes  it on  to  the device driver.
 *
 * EXECUTION ENVIRONMENT:
 *	This function operates as a device dependent subroutine 
 *	called by the generic configure method for all devices.
 *	This in turn calls getatt function to the attrbute value
 *	from the database.
 *
 * DATA STRUCTURES:
 *	CuDv : Customised devices objet structure
 *	cusatt, cusdev : pointers to CuAt and CuDv object classes
 *	dds : pointer to dds structure
 *	
 * INPUT  : logical_name, ptr to ddsptr, ptr to dds_length
 *
 * RETURNS: 0 if success else -1
 *
 * RECOVERY OPERATION:
 *
 */

int build_dds(lname,dds_data_ptr,dds_len)
char	*lname;
char	**dds_data_ptr;
long	*dds_len;
{
long    model_code;
int	rc, adaplvl, num_attrs;
struct	CuDv	CuDv;
struct	CuAt	cuat, *cuattr;
struct	PdAt	pdat;
char	*p,sstr[512],bb[4],tm_enabled[4], *part_no, dg, rl;
char	pname[NAMESIZE],ut[UNIQUESIZE],pt[UNIQUESIZE];
static  struct  adap_ddi *dds;
struct	Class *cusatt,*preatt,*cusdev;
char    scsi_chip_type[] = "adapter/sio/pscsi" ;
uchar   pscsi_type = FALSE;
int     bus_num;

    mode = 0; /* variable used to manage the RDS facility */

        /* set device state to defined so that define children will know */
        /* to download microcode for this adapter */
        device_state = DEFINED;

	dds = (struct adap_ddi *)malloc(sizeof(struct adap_ddi));
	if(dds == (struct adap_ddi *)NULL){
	    DEBUG_0("bld_bbdds: malloc failed\n")
	    return E_MALLOC;
	}
	if((int)(cusdev = odm_open_class(CuDv_CLASS)) == -1){
	    DEBUG_0("bld_dds: can not open CuDv\n")
	    return E_ODMOPEN;
	}
	/*
        Get parent(bus) attributes viz, bus_type, bus_num, slot, etc.
	*/
	sprintf(sstr,"name = '%s'",lname);
	rc = (int)odm_get_first(cusdev,sstr,&CuDv);
	if(rc == 0){
	    DEBUG_1("bld_dds: No entry in CuDv for %s\n",sstr)
	    return E_NOCuDv;
	}
	if(rc == -1){
	    DEBUG_1("bld_dds: err in getobj from CuDv for %s\n",sstr)
	    return E_ODMGET;
	}
 	dds->slot = atoi(CuDv.connwhere) - 1;
	DEBUG_2("build_dds: conwhere=%s slot=%d\n",CuDv.connwhere,dds->slot)

	strcpy(ut,CuDv.PdDvLn_Lvalue);
	strcpy(pt,CuDv.parent);
	DEBUG_2("bld_dds: ut = %s pt = %s\n",ut,pt)
       /*
        * Get the parent bus so that we can set some attributes based
        *     based on the bus we are attached to (either directly or
        *     indirectly).
        */
        if ((rc = Get_Parent_Bus(cusdev, CuDv.parent, &CuDv)) != 0)
        {
	    DEBUG_1("cfghscsi: Unable to get parent bus object; rc = %d\n",
		     rc) 
	    if (rc == E_PARENT)
	    {
		rc = E_NOCuDvPARENT ;
 	    } 
	    return (rc) ;
	}
        strcpy(pname,CuDv.name);
	strcpy(pt,CuDv.PdDvLn_Lvalue);
	DEBUG_2("bld_dds: ut = %s pt = %s\n",ut,pt)
	odm_close_class(cusdev);

	DEBUG_2("bld_dds: parent = %s for %s\n",pname,lname)
	DEBUG_2("bld_dds: ut = %s pt = %s\n",ut,pt)
	if((int)(cusatt = odm_open_class(CuAt_CLASS)) == -1){
	    DEBUG_0("bld_dds: can not open CuAt\n")
	    return E_ODMOPEN;
	}
	if((int)(preatt = odm_open_class(PdAt_CLASS)) == -1){
	    DEBUG_0("bld_dds: can not open PdAt\n")
	    return E_ODMOPEN;
	}
	if(rc =(getatt(&dds->bus_id,'l',cusatt,preatt,pname,pt,
                 "bus_id",NULL))>0) return rc;
	dds->bus_id |= BB_BUS_ID;
	if((rc=getatt(&dds->bus_type,'h',cusatt,preatt,pname,pt,
		"bus_type",NULL))>0) return rc;
	if((rc=getatt(&dds->card_scsi_id,'c',cusatt,preatt,lname,ut,
		"id",NULL))>0) return rc;
	/*
		write card_scsi_id into nvram
	*/
        bus_num = CuDv.location[3] - '0';
        if(put_scsi_id(dds->slot+1,dds->card_scsi_id,bus_num) != 0){
	    DEBUG_1("build_dds: put_scsi_id err for card in slot%d\n",
		dds->slot)
	}
	DEBUG_1("bld_dds: uniquetype = %s\n", ut)
	if (!strncmp(ut, scsi_chip_type, strlen(scsi_chip_type))) {
            pscsi_type = TRUE;
	    if((rc=getatt(&dds->base_addr,'i',cusatt,preatt,lname,ut,
		    "bus_io_addr", NULL))>0)return rc;
	} else {
	    if((rc=getatt(&dds->base_addr,'i',cusatt,preatt,lname,ut,
		    "bus_mem_addr", NULL))>0)return rc;
	}
	if((rc=getatt(bb,'s',cusatt,preatt,lname,ut, "bb",NULL))>0)
		return rc;

	dds->battery_backed = (strcmp(bb, "yes") == 0) ? 1 : 0;

	if((rc=getatt(tm_enabled,'s',cusatt,preatt,lname,ut, "tm",NULL))>0)
		return rc;

	dds->tm_enabled = (strcmp(tm_enabled, "yes") == 0) ? 1 : 0;

	if((rc=getatt(&dds->dma_lvl,'i',cusatt,preatt,lname,ut,
		"dma_lvl",NULL))>0) return rc;
	if((rc=getatt(&dds->int_lvl,'i',cusatt,preatt,lname,ut,
		"bus_intr_lvl",NULL))>0) return rc;
	if((rc=getatt(&dds->int_prior,'i',cusatt,preatt,lname,ut,
		"intr_priority", NULL))>0)return rc;
	if((rc=getatt(&dds->tcw_start_addr,'l',cusatt,preatt,lname,ut,
		"dma_bus_mem",NULL))>0)return rc;

	strcpy(dds->resource_name,lname);
	dds->cmd_delay = 7;

	/*  new attibute tmp (percentage of dma_bus_mem used for */
	/*  hscsi target mode support).				 */

	if((rc=getatt(&dds->tm_tcw_percent,'i',cusatt,preatt,lname,ut,
		"tmp",NULL))>0)return rc;

       /* the dds needs to include information about the model in which */
        /* we are operating.  If it is a model 220 then streaming should */
        /* be disabled for the scsi adapter.                             */

        if((rc=getatt(&model_code,'l',cusatt,preatt,"sys0","",
                "modelcode",NULL))>0) model_code = 0x200;
        DEBUG_1("build_dds: model code returned = 0x%x\n", model_code)

        if (model_code & 0x200)
            dds->bb_stream = FALSE;
        else dds->bb_stream = TRUE;

        DEBUG_1("build_dds: bb_stream = %d\n", dds->bb_stream)
   
	DEBUG_1("bld_dds: uniquetype = %s\n", ut)
        /* default to let cmd_queue to be false indicating adapter does not  */
        /* support command tag queuing.  If it is not pscsi type and the     */
        /* adapter level is >= 0xa0 then it does support command tag queuing */
        dds->cmd_queue = FALSE;
	if (!(pscsi_type)) {
	    /* start by deleting max_data_rate attribute from CuAt */
	    sprintf(sstr,"name = '%s' AND attribute = 'max_data_rate'",lname);
            odm_rm_obj(CuAt_CLASS,sstr);
	    if ((adaplvl = get_adap_lvl(lname,&part_no,&dg, &rl)) >= 0xa0) { 
                dds->cmd_queue = TRUE;
                /* if the adapter supports cmd queueing then it must be a   */
                /* SCSI-2 adapter so modify the max data rate to reflect    */
                /* the 10000 kbs that the SCSI-2 adapter supports           */ 
	        if ((cuattr = getattr(lname, "max_data_rate", 0, &num_attrs))
                    != (struct CuAt *) NULL) {
                    strcpy(cuattr->value, "10000");
                    /* call putattr to update the data base */
                    (void)putattr(cuattr); 
               }
               else DEBUG_0("build_dds : max_data_rate attr not found \n");
            }
            /* FOR NON PSCSI TYPE CONTROLLERS :                              */
            /* some scsi controllers use a fuse to regulate power on the scsi*/
            /* bus while other use a PTC.  if the dg field of the vpd for a  */
            /* controller has a value of zero then this controller has a fuse*/
            /* else it has a PTC                                             */
	    DEBUG_1("bld_dds: dg = %x\n", dg)
	    DEBUG_1("bld_dds: rl = %x\n", rl)
            dds->has_fuse = !(dg);
	    DEBUG_1("bld_dds: dds->has_fuse = %x\n", dds->has_fuse)
        }
      
	/* get dma_bus_mem attribute's width value */
	/* first, see if a CuAt dbmw attibute exists */
	sprintf(sstr,"name = '%s' AND attribute = dbmw",lname);
	rc = (int)odm_get_first(cusatt,sstr,&cuat);
	if (rc==-1) {
		return(E_ODMGET);
	} else if (rc != 0) {
		DEBUG_0("Using dbmw CuAt attribute for dma width\n")
		dds->tcw_length = (int)strtoul(cuat.value,(char **)NULL,0);
	} else {
		/* see if a PdAt dbmw attribute exists */
		sprintf(sstr,"uniquetype = '%s' AND attribute = dbmw",ut);
		rc = (int)odm_get_first(preatt,sstr,&pdat);
		if (rc==-1) {
			return(E_ODMGET);
		} else if (rc != 0) {
			DEBUG_0("Using dbmw PdAt attribute for dma width\n")
			dds->tcw_length=(int)strtoul(pdat.deflt,(char **)NULL,0);
		} else {
			/* use width value from dma_bus_mem PdAt attribute */
			sprintf(sstr,
			    "uniquetype = '%s' AND attribute = dma_bus_mem",ut);
			rc = (int)odm_get_first(preatt,sstr,&pdat);
			if (rc==-1) {
				return(E_ODMGET);
			} else if (rc == 0) {
				return(E_NOATTR);
			}
			DEBUG_0("Using width from dma_bus_mem PdAt attr\n")
			dds->tcw_length=(int)strtoul(pdat.width,(char **)NULL,0);
		}
	}

	*dds_data_ptr = (char *)dds;
	*dds_len = sizeof(struct adap_ddi);

	DEBUG_1("hscsi DDS length: %d\n",*dds_len)
#ifdef	CFGDEBUG
	hexdump(*dds_data_ptr,(long)*dds_len);
#endif	CFGDEBUG
	odm_close_class(cusatt);
	odm_close_class(preatt);
	return(0);
}


/*
 * NAME : download_microcode
 * 
 * FUNCTION :
 *      microcode for scsi1/scsi2 scsi adapters will be downloaded
 *      during phase2 of ipl after the adapter has been made availible.
 *      This will be done by hscsi_download_microcode being called by
 *      the define_children routine.  This function will still be 
 *      called by the generic device configuration method but it will
 *      only perform an open/close against the adapter to attempt to
 *      ensure the adapter is functioning properly before it is put 
 *      in the availible state. 
 *
 * EXECUTION ENVIRONMENT:
 *	This function operates as a device dependent subroutine
 *	called by the generic configure method for all devices.
 *
 * DATA STRUCTURES:
 *
 * INPUT  : logical_name
 *
 * RETURNS: Returns 0 if success else error indication
 *
 * RECOVERY OPERATION:
 *
 */

int download_microcode(lname)
char	*lname;
{
	char dev[32];
        int rc;

	/* open the adapter to ensure it is operable before continuing */
	sprintf(dev,"/dev/%s",lname);
        if ((rc = open(dev, O_RDWR, 0)) == -1) {
            DEBUG_2("download_microcode :open for device %s returned %d\n",
                     dev, rc)
            return(E_DEVACCESS);
        }

	/* open succeeded so close the adapter and contiue with download */
	close(rc);
	return(0);
}
/*
 * NAME : hscsi_download_microcode
 * 
 * FUNCTION :
 *	This function determines the proper level of microcode to
 *	download to the device, downloads it, and updates the CuAt
 *	object class to show the name of the file that was used.
 *
 * EXECUTION ENVIRONMENT:
 *	This function operates as a device dependent subroutine
 *	called by define_children for scsi adapters.
 *
 * DATA STRUCTURES:
 *	scdld : parameter block for download command.
 *	ucode_buf : pointer to microcode data.
 *
 * INPUT  : logical_name
 *
 * RETURNS: Returns 0 if success else error indication
 *
 * RECOVERY OPERATION:
 *
 * NOTE :
 *	Microcode file size can not be greater than 64K.
 */

int hscsi_download_microcode(lname)
char	*lname;
{
	char	filename[32],dev[32],curfile[32], *part_no, dg, rl,len;
	char	sstr[256];
	int	adaplvl,num_attrs,mcode_flag;
	struct	CuAt *cuattr;
	int	rc,tm_enable;


	/* start by deleting target mode enable attribute from CuAt */
	sprintf(sstr,"name = '%s' AND attribute = 'tme'",lname);
	odm_rm_obj(CuAt_CLASS,sstr);

        /* get the value of the tm attribute to determine if         */
        /* target mode microcode should be downloaded                */
        if ((cuattr = getattr(lname, "tm", 0, &num_attrs))
             == (struct CuAt *)NULL) { 
            DEBUG_0("hscsi_download_microcode: ") 
            DEBUG_0("getattr for attribute tm failed\n")
            return(E_NOATTR);
        }
        DEBUG_0("hscsi_download_microcode: ") 
        DEBUG_1("tm enable attribute has a value of %s\n", cuattr->value)
  
        /* use a local boolean to reflect the value of tm enable    */
        if(strcmp(cuattr->value, "yes") == 0) 
           tm_enable = TRUE;
        else tm_enable = FALSE;
        

	/* get the adapter version level */
	if ((adaplvl = get_adap_lvl(lname,&part_no,&dg,&rl)) == -1) {
                /* if we cannot get the adapter level, then we will continue */
                /* to run the card on eprom microcode.  Target mode will not */
                /* be enabled in this case since without adaplvl, it cannot  */
                /* be determined if this level of adapter will support tm    */
		DEBUG_0("hscsi_download_microcode: error getting adapter lvl")
	        return(0);	
	}
        /* some older level 32 adapters have invalid vpd, so adjust adaplvl */
        if (adaplvl < 0x32) {
		DEBUG_0("hscsi_download_microcode: setting adaplvl = 32\n")
		adaplvl = 0x32;
        }

	DEBUG_1("hscsi_download_microcode: adaplvl = %x\n",adaplvl)
        DEBUG_1("hscsi_download_microcode: part_no = %s\n",part_no)

        /* if the adapter version level is less that 0x44 or the     */
        /* part no is 00G2362 the card cannot support target mode    */
        if(((adaplvl < 0x44) || (strncmp(part_no," 00G2362",8) == 0)) 
            && (tm_enable)) {
           tm_enable = FALSE;
           strcpy(cuattr->value, "no");
           if(putattr(cuattr) < 0) {
              DEBUG_0("hscsi_download_microcode : putattr failed\n")
              return(E_ODMUPDATE);
           }
        }   

        /* create the necessary prefix of microcode filename to pass */
        /* to the findmcode function                                 */
        /* if tm enabled then look for target mode microcode         */
        /* NOTE : scsi1 adapters require that target mode            */
        /* microcode be downloaded to the adapter before it will     */
        /* function as a target device.  For scsi2 adapters,         */
        /* adap_lvl 0xa0, the target mode function will exist on the */
        /* eprom microcode so scsi2 cards do not require a download  */
        /* of microcode to enable the target mode function           */

        /* if tm enabled and it is a scsi1 adapter      */
        /* then look for target mode microcode file     */
        if((tm_enable)  && (adaplvl < 0x60))
           sprintf(curfile,"8d77t.%x",adaplvl);
        else sprintf(curfile, "8d77.%x",adaplvl);

        /* set mcode_flag to search for most recent version of mcode   */
        mcode_flag = VERSIONING;
	/* get name of microcode file                                  */
        DEBUG_1("hscsi_download_microcode: about to look for mcode %s\n", 
			curfile)
	if (!(findmcode(curfile,filename,mcode_flag,NULL))) {
	   DEBUG_0("hscsi_download_microcode: microcode file not found\n")
           /* could not find microcode file, if it was target mode      */
           /* microcode then try to find initator mode file             */
           DEBUG_0("hscsi_download_microcode: ") 
           DEBUG_0("could not find mcode on the first try\n")
           if((tm_enable)  && (adaplvl < 0x60)) {
              /* note: the global tm_enable flag is set to false but the */
              /* odm will still contain "yes" as the value of the tm attr*/
              tm_enable = FALSE;
              sprintf(curfile, "8d77.%x",adaplvl);
	      if (!(findmcode(curfile,filename,mcode_flag, NULL))) {
	         return(0);
	      }
           }
           /* initiator microcode not found                            */
           /* if we could not find initiator microcode and we are a    */
           /* scsi2 adapter, then before returning, the tme            */
           /* attribute must be updated because scsi2 adapters support */
           /* target mode without a download occuring.                 */
           else  { 
               if((tm_enable) && (adaplvl >= 0xa0)) {
                   (void)set_tme_attr(lname);
               }
               return(0);
           }
        }

        /* if the level of microcode resident on the eprom in the adapter  */
        /* is of a higher level than what is found on the filesystem then  */
        /* do not download it.  If it is a scsi1 adapter and tm_enabled is */
        /* yes then we must always download the target mode microcode and  */
        /* not perform the test.                                           */
   
        if((tm_enable)  && (adaplvl < 0x60)) {
            DEBUG_0("hscsi_download_microcode:found bb tm file don't compare");
        }
        else { 
            len=strlen(filename);
            sprintf(curfile, "%02x",rl);
            DEBUG_1("hscsi_download_microcode:curfile  = %s\n", curfile)
            DEBUG_1("hscsi_download_microcode:filename = %s\n", filename)
            DEBUG_1("hscsi_download_microcode:filename = %s\n",&filename[len-2])
            if (strcmp(curfile,&filename[len-2]) >= 0) {
                DEBUG_0("hscsi_download_microcode: do not download\n")
                if((tm_enable) && (adaplvl >= 0xa0)) {
                    (void)set_tme_attr(lname);
                }
                return(0); 
            }
        }
            
	/* download ucode to device */
        sprintf(dev, "/dev/%s",lname);
	rc = download(dev,filename);
	if (rc > 0) {
		DEBUG_0("hscsi_download_microcode: download failed\n")
		return(E_UCODE);
	} else if (rc == -1) {
		DEBUG_0("hscsi_download_microcode: download failed ")
		DEBUG_0("continuing with card microcode\n")

                /* here the data base must be updated to indicate if */
                /* target mode has been enabled.  Lace adapters allow*/
                /* target mode function without a download so update */
                /* the tme attribute accordingly                     */
                if((tm_enable) && (adaplvl >= 0xa0)) {
                    (void)set_tme_attr(lname);
                }
		return(0);
	}

	DEBUG_1("hscsi_download_microcode: microcode downloaded for %s...",
                 lname)
        
        /* if we get this far and tm_enable is TRUE, then change the */
        /* tme attribute to yes so that define children will create  */
        /* any target mode instances  for this device                */
        if(tm_enable) { 
           (void)set_tme_attr(lname);
        }

        if ((cuattr = getattr(lname, "ucode", 0, &num_attrs))
             == (struct CuAt *)NULL) {
            DEBUG_0("hscsi_download_microcode: ") 
            DEBUG_0("getattr for attribute ucode failed\n")
            return(E_NOATTR);
        }

        /* change the value of the attribute to reflect microcode */
        /* which was downloaded                                   */
        strcpy(cuattr->value, filename);

        DEBUG_0("hscsi_download_microcode: ") 
	DEBUG_4("ucode attr: name=%s, attr=%s, value=%s, type=%s\n",
                 cuattr->name,cuattr->attribute, cuattr->value,cuattr->type)
	DEBUG_3("generic=%s, rep=%s, nls_index=%d\n",
                 cuattr->generic, cuattr->rep,cuattr->nls_index)

	/* call putattr() to update the data base */
        if(putattr(cuattr) < 0) {
           DEBUG_0("download_microcode : putattr failed\n")
           return(E_ODMUPDATE);
        }

	DEBUG_0("Download finished & successful...returning rc=0\n")

	return(0);
}




/*
 * NAME : download
 * 
 * FUNCTION :
 *	Downloads the microcode in ucodefile to the device accessed by
 *	devname.
 *
 * EXECUTION ENVIRONMENT:
 *	This function is internal to the cfghscsi method.
 *
 * DATA STRUCTURES:
 *	sc_download controls the download
 *
 * INPUT  :
 *	devname		ptr to name of special file of device
 *	ucodefile	ptr to name of microcode file
 *
 * RETURNS: Returns 0 if success.
 *	    Returns -1 if download failed but we are to continue with
 *		card microcode.
 *	    Returns >0 on fatal error.
 *
 */

int
download(devname,ucodefile)
char	*devname,*ucodefile;
{
struct	stat sbuf;
struct	sc_download scdld;
uchar	*cbuf;
int	rem,codelen;
int	ucode,adap;
int	rc;

	if (stat(ucodefile,&sbuf)==-1) {
		/* stat failed -- run with code in adapter */
		DEBUG_2("download: stat of %s failed, errno=%d\n",
							ucodefile,errno)
		return(-1);
	}

	/* calculate microcode size (pad to 1K) */
	if ((rem=sbuf.st_size%1024)!=0)
		codelen = sbuf.st_size+(1024-rem);
	else
		codelen = sbuf.st_size;

	DEBUG_1("download: adjusted length of ucode=%d\n",codelen)

	if ((ucode = open(ucodefile,O_RDONLY,0)) == -1) {
		DEBUG_1("bb_downld: can not open %s file\n",ucodefile)
		return(-1);
	}

	/* allocate space for microcode in memory */
	if ((cbuf = (uchar *) malloc(codelen)) == NULL) {
		DEBUG_0("bb_downld: malloc failed\n")
		close(ucode);
		return(-1);
	}

	/* read microcode into memory */
	if (read(ucode,cbuf,sbuf.st_size)<0){
		DEBUG_2("bb_downld: err reading %s file, errno=%d\n",ucodefile,errno)
		free(cbuf);
		close(ucode);
		return(-1);
	}
	close(ucode);

	DEBUG_1("download: read microcode into memory @ 0x%x\n",cbuf)

	/* open the adapter device in normal mode */
	if((adap = open(devname,O_RDWR,0)) == -1) {
		DEBUG_2("cfghscsi: can't open %s, errno=%d\n",devname,errno)
		free(cbuf);
		return(E_DEVACCESS);
	}

	/* setup parameter block for download command */
        bzero(&scdld, sizeof(struct sc_download)); 
	scdld.option = SC_DOWNLOAD;
	scdld.microcode_len = codelen;
	scdld.microcode_ptr = cbuf;

	rc = 0;

	/* download microcode */
	if (ioctl(adap,SCIODNLD,&scdld) == -1) {
		if ((errno==EIO) || (errno == EINVAL)) {
			DEBUG_0("cfghscsi: err downloading ucode, ")
			DEBUG_1("errno = %d\n", errno)
			DEBUG_0("continue with microcode on card\n")
                        rc = -1;
		} else {
			DEBUG_1("cfghscsi: err downloading ucode, errno=%d\n",
									errno)
			DEBUG_0("download of microcode failed\n")
			rc = errno;
		}
	} else {
		DEBUG_0("download: microcode downloaded successfully\n")
	}

	free(cbuf);
	close(adap);
	return(rc);
}

/*
 * NAME : get_adap_lvl
 *
 * FUNCTION :
 *	Returns the LL level of the adapter. This is in the VPD
 *	for the adapter.
 *
 * EXECUTION ENVIRONMENT:
 *	This function is internal to the cfghscsi method.
 *
 * INPUT  :
 *	lname	pointer to the logical name of the device
 *
 * RETURNS:
 *	The LL level of the adapter, or 0 if there is a failure,
 *	or if there is no LL field in the VPD.
 *
 * NOTE :
 *	query_vpd is called to get the VPD for the adapter.
 *
 */

int
get_adap_lvl(lname,part_no,dg,rl)
char	*lname;
char    **part_no;
char    *dg;
char    *rl;
{
struct	CuDv	device;
long	maj,*min;
char	sstr[256],*p;
uchar	vpd[VPDSIZE];
mid_t	kmid;
int	cnt,rc;
int	lvl;

	/* get customized object for device */
	sprintf(sstr,"name = '%s'",lname);
	rc = odm_get_first(CuDv_CLASS,sstr,&device);
	if (rc == 0 || rc == -1) {
		DEBUG_1("get_adap_lvl: failed to get CuDv object for %s\n",lname)
		return(0);
	}

	/* get major & minor numbers and kmid for the device */
	if ((maj = genmajor(device.ddins)) == -1) {
		DEBUG_2("get_adap_lvl: genmajor failed, name=%s ddins=%s\n",
			lname,device.ddins)
		return(0);
	}
	if ((min = getminor(maj,&cnt,lname)) == NULL) {
		DEBUG_1("get_adap_lvl: getminor failed, maj=%d\n",maj)
		return(0);
	}
	if ((kmid = loadext(device.ddins,FALSE,TRUE)) == NULL) {
		DEBUG_1("get_adap_lvl: loadext failed, ddins=%s\n",device.ddins)
		return(0);
	}

	/* get the vpd for the device */
	if (query_vpd(&device, kmid, makedev(maj,*min), vpd)) {
		DEBUG_4("get_adap_lvl: query_vpd failed for %s, kmid=%x devno=%x vpd=%x\n",
			lname,kmid,makedev(maj,*min),vpd)
		return(0);
	}

	DEBUG_0("get_adap_lvl: got VPD\n")

	/* return adapter level */
	p = vpd; while (*p != '*') p++;

	/* Since LL has leading zeros make sure converted as hex */
	lvl = (int)strtoul(read_descriptor(p,"LL"),NULL,16);
	*dg = (char)strtoul(read_descriptor(p,"DG"),NULL,16);
	*rl = (char)strtoul(read_descriptor(p,"RL"),NULL,16);
	DEBUG_1("get_adap_lvl: lvl=%x\n",lvl)
	DEBUG_1("get_adap_lvl: dg=%x\n",*dg)
	DEBUG_1("get_adap_lvl: rl=%x\n",*rl)
        *part_no = read_descriptor(p, "PN");
        DEBUG_1("get_adap_lvl: part_no = %s\n", *part_no)

	return(lvl);
}

/*
 * NAME : set_tme_attr
 *
 * FUNCTION :
 *      This function sets the tme attribute for a given scsi adapter to
 *      a value of yes.	
 *
 * EXECUTION ENVIRONMENT:
 *	This function is internal to the cfghscsi method.
 *
 * INPUT  :
 *	lname	pointer to the logical name of the device
 *
 * RETURNS:
        This function returns zero if successful, else ODM error indication
 *
 * NOTE :
 *
 */
int
set_tme_attr(lname)
char	*lname;
{
	int	num_attrs;
	struct	CuAt *cuattr;
               
	if ((cuattr = getattr(lname, "tme", 0, &num_attrs))
              == (struct CuAt *) NULL) {
		DEBUG_0("set_tme_attr: ") 
		DEBUG_0("getattr for attribute tme failed\n")
                return(E_NOATTR);
        }
        strcpy(cuattr->value, "yes");
        /* call putattr to update the data base */
        if(putattr(cuattr) < 0) {
            DEBUG_0("set_tme_attr: putattr failed\n")
            return(E_ODMUPDATE);
        }
}


/*
 * NAME   : define_children
 *
 * FUNCTION : This routine determines whether or not to download
 *            microcode to the SCSI adapter.  It then calls the
 *            routine to define child devices on a SCSI bus.
 *
 * EXECUTION ENVIRONMENT:
 *      This function operates as a device dependent subroutine
 *      called by the generic configure method for all devices.
 *
 * DATA STRUCTURES:
 *
 * INPUTS : logical_name,ipl_phase
 *
 * RETURNS: Returns 0 if success
 *
 * RECOVERY OPERATION:
 *
 */

int
define_children (lname, ipl_phase)
char    *lname;
int     ipl_phase;
{
	char    adap_dev[32];           /* adapter device /dev name */
	int     adapter;                /* file descriptor for adapter */
	int     rc;                     /* for return codes */
        struct  CuDv cudev;             /* for getting CuDv attribute */
        char	sstr[256];		/* for getting CuAt attribute */
        int     do_not_care;            /* passed to get_adap_lvl to store */
                                        /* values of no interest */
        int     adap_lvl;               /* used to determine if cmd_tag    */
                                        /* queing should be enabled after  */
                                        /* a download is attempted         */
        struct sc_download scd;         /* structure used to issue version */
                                        /* query of adapter microcode, and */
                                        /* to enable cmd_tag_queuing       */
	int     num_luns;               /* number of luns device supports  */
	int     num_sids;               /* number of scsi ids device supports */
        char    pscsi_utype[] =  "adapter/sio/pscsi" ;

	DEBUG_0("define_children: entering define children routine\n")

        if (ipl_phase == PHASE1) {
	    /* start by deleting microcode filename attribute from CuAt */
	    sprintf(sstr,"name = '%s' AND attribute = 'ucode'",lname);
	    odm_rm_obj(CuAt_CLASS,sstr);
        }
        /* microcode download is initiated from define children */
        /* if it is phase 2 of ipl then initiate download, also if it is */
        /* runtime and the adapter was previously in a defined state     */
        /* then initiate microcode download                              */
        if ((ipl_phase == PHASE2) || ((ipl_phase == RUNTIME_CFG)
             && (device_state == DEFINED))) {
           /* determine scsci adapter unique type */
           /* get CuDv object for device */
           sprintf(sstr, "name = '%s'",lname);
           rc = (int)odm_get_first(CuDv_CLASS,sstr,&cudev);
           if(rc == 0) {
               DEBUG_1("define_children: No entry in CuDv for %s\n", sstr)
               return(E_NOCuDv);
           }
           if(rc == -1) {
               DEBUG_0("define_children : ")
               DEBUG_1("error in getobj from CuDv for %s\n", sstr)
               return(E_ODMGET);
           }
           DEBUG_1("define_children : uniquetype = %s\n", cudev.PdDvLn_Lvalue)

           /* if this adapter is not a pscsi then it must be a hscsi so */
           /* try to download microcode to it.                          */
           if(strncmp(cudev.PdDvLn_Lvalue, pscsi_utype,strlen(pscsi_utype))) {
               (void) hscsi_download_microcode(lname);
                /* if this is a hscsi SCSI-2 adapter and the version of  */
                /* microcode which is executing on the adapter after the */
                /* download was attempted is equal or greater to 9, then */
                /* issue the IOCTL to enable the cmd_tag_queuing function*/
                /* of the adapter in the device driver.  NOTE : the      */
                /* default for the device driver is to not command tag   */
                /* queue.  Command tag queuing should only be enabled if */
                /* this is a SCSI-2 adapter.                             */
                if ((adap_lvl = get_adap_lvl(lname,&do_not_care,
                                     &do_not_care, &do_not_care)) >= 0xa0){
                    /* this is a SCSI-2 adapter so query the currently */
                    /* running version of microcode to determine if    */
                    /* cmd_tag_queuing can be enabled for this adapter */
                    DEBUG_1("define_children : adap_lvl = %x\n", adap_lvl)
                    /* open adapter device */
                    sprintf(adap_dev,"/dev/%s",lname);
                    if((adapter = open(adap_dev,O_RDWR)) < 0){
                        DEBUG_1("define_children:can not open %s \n",adap_dev)
                        return(E_DEVACCESS);
                    }
                    /* setup parameter block for version query command */
                    bzero(&scd, sizeof(struct sc_download));

                    scd.option = SC_VERSION_NUMBER;
                    scd.microcode_len = 0;
                    scd.microcode_ptr = NULL;
                    scd.version_number = 0;
                    rc = ioctl(adapter,SCIODNLD,&scd);
                    if (rc == 0) {   /* if version query succeeded */
                        /* if the version number is 9 or greater then enable */
                        /* command tag queuing for this hscsi SCSI-2 adapter */
                        DEBUG_1("define_children : version = %d\n",
				scd.version_number)
                        if(scd.version_number >= 9) {
                            scd.option = SC_ENABLE_CMD_Q;
                            rc = ioctl(adapter,SCIODNLD,&scd);
                           DEBUG_1("define_children: SC_ENABLE_CMD_Q returned %d\n",
                                    rc)
                        }
                    }
                    close(adapter);
                }
           }
       }

	num_sids = 8;
	num_luns = 8;
	rc = def_scsi_children (lname, ipl_phase, num_sids, num_luns);
	return (rc);
}
