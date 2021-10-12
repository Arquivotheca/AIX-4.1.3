static char sccsid[] = "@(#)22	1.16  src/bos/usr/lib/methods/common/dldserdasdc.c, cfgmethods, bos411, 9428A410j 11/8/93 09:28:32";
/*
 * COMPONENT_NAME: CFGMETHODS
 *
 * FUNCTIONS : 	get_ctrl_lvl, download_ctrl download_microcode_ctrl
 * 
 * ORIGINS : 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1990,1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>
#include <cf.h>
#include <sys/bootrecord.h>
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/sd.h>
#include <sys/device.h>
#include <sys/cfgodm.h>
#include <sys/errno.h>
#include <sys/cfgdb.h>
#include <sys/stat.h>
#include "cfgdebug.h"
#include "cfgserdasd.h"
/*
 * NAME:	get_ctrl_lvl
 *
 * FUNCTION:	Reads the level of the controller hardware, and the level of
 *		the microcode on the controller
 *
 * ENVIRONMENT:	This function is used by the cfgserdasdc method.
 *
 * RETURNS:	0 on success, -1 on failure
 */

int get_ctrl_lvl(int adapfd,		 /* File descriptor for adapter      */
		 int sid,		 /* SCSI id for controller           */
		 int *ctrllvl,		 /* Address to store hardware level  */
		 int *ucodelvl,		 /* Address to store microcode level */
		 int *model_num)         /* model number of controller       */
{
	uchar	inq_data[256];          /* Space for inquiry data	*/
	int	rc;			/* Return code			*/
	char	romcodelvl[3];		/* Level of rom code present	*/
	char	ramcodelvl[3];		/* Level of ram code present	*/
	struct	sd_ioctl_parms parms;	/* command structure for inquiry */
	char	inquiry_results[16];
	char	tmpstr[4];

	DEBUG_0("cfgserdasdc: get_ctrl_lvl\n");
	DEBUG_1("dldserdasdc: sid = %d\n",sid);
	/* get inquiry data */
	if ((rc = scsi_inq(adapfd,inq_data,(sid<<5)|0x10))!=0) {
		DEBUG_1("get_ctrl_lvl: failed inquiry at sid %d lun 0x10\n", sid );
		return -1;
	}


	strncpy( romcodelvl, &inq_data[126], 2 );
	romcodelvl[2] = '\0';
	*ucodelvl = atoi( romcodelvl );

	strncpy( romcodelvl, &inq_data[32], 2 );
	romcodelvl[2] = '\0';
	strncpy( ramcodelvl, &inq_data[34], 2 );
	ramcodelvl[2] = '\0';
	
	strncpy( tmpstr, &inq_data[20], 3 );
	tmpstr[3] = '\0';
	*model_num = atoi(tmpstr);
	DEBUG_1("cfgserdasdc: model_num = %d\n",*model_num)
	DEBUG_1("cfgserdasdc: ucodelevel = %d\n",*ucodelvl)
	DEBUG_1("cfgserdasdc: controller level = %d\n",ctrllvl)

	parms.data_length = 16;
	parms.buffer = inquiry_results;
	parms.time_out = 2;

	if( ioctl( adapfd, SD_ADAP_INQUIRY, &parms ) == -1 ) {
		DEBUG_0("get_ctrl_lvl: ioctl( ..SD_ADAP_INQUIRY.. ) failed\n")
		return -1;
	}
	tmpstr[2] = '\0';
	strncpy( tmpstr, &inquiry_results[4], 2 ); /* Microcode level id */
	*ctrllvl = atoi( tmpstr );
	

	return 0;
}
/*
 * NAME:	download_ctrl
 * 
 * FUNCTION:	Downloads the microcode in ucodefile to the device accessed by
 *		adapfd.
 *
 * ENVIRONMENT:	This function is used by the cfgserdasdc method and the daemon.
 *	
 *	HARRIER 2 MICROCODE PACKAGE DESIGN 
 *	---------------------------------------------
 *	
 *	The purpose of the redesign is to allow a single package to contain
 *	more than one controller + dasd load.  Millbrook do not at present
 *	see any requirement for having more than one controller/dasd load,
 *	(i.e. controllers on a single adapter which need different loads),
 *	but it would be worth while allowing for such a situation.
 *	
 *	The following shows the new format of the package, in which only the
 *	system header has changed (the changes are shown by vertical change
 *	bars at the left).  The header is now of variable rather than fixed
 *	length, the length being indicated by the 'Offset address of adapter
 *	header' field.  Fields following the 'Offset address of Ctlr/Dasd
 *	header' field are optional; if they are not present then the format
 *	is identical to that used today.
 *	
 *	    -------------------------------------------------
 *	   | Proprietary notice                  (198 bytes) |               )
 *	   |- - - - - - - - - - - - - - - - - - - - - - - - -|  Package      )
 *	   | Package ID number                     (2 bytes) |  header       )
 *	   |- - - - - - - - - - - - - - - - - - - - - - - - -| (for use      )
 *	   | Offset address of Adapter hdr         (4 bytes) |  by system)   )
 *	   |- - - - - - - - - - - - - - - - - - - - - - - - -|               )
 *	   | Offset address of Ctlr/dasd hdr       (4 bytes) |               )
 *	   |- - - - - - - - - - - - - - - - - - - - - - - - -|               )
 *	|  | Identifn. of 1st C/D load  (optional) (4 bytes) | )             )
 *	|  |- - - - - - - - - - - - - - - - - - - - - - - - -| )             )
 *	|  | Offs addr of 2nd C/D hdr   (optional) (4 bytes) | )             )
 *	|  |- - - - - - - - - - - - - - - - - - - - - - - - -| ) extn. to    )
 *	|  | Identifn. of 2nd C/D load  (optional) (4 bytes) | ) existing    )
 *	|  |- - - - - - - - - - - - - - - - - - - - - - - - -| ) package     )
 *	|  | Offs addr of 3rd C/D hdr   (optional) (4 bytes) | ) header      )
 *	|  |- - - - - - - - - - - - - - - - - - - - - - - - -| ) (up to      )
 *	|  | Identifn. of 3rd C/D load  (optional) (4 bytes) | )  28 bytes)  )
 *	|  |- - - - - - - - - - - - - - - - - - - - - - - - -| )             )
 *	|  | Offs addr of 4th C/D hdr   (optional) (4 bytes) | )             )
 *	|  |- - - - - - - - - - - - - - - - - - - - - - - - -| )             )
 *	|  | Identifn. of 4th C/D load  (optional) (4 bytes) | )             )
 *	   |-------------------------------------------------|
 *	   | Adapter header                       (16 bytes) |
 *	   |- - - - - - - - - - - - - - - - - - - - - - - - -|
 *	   |                                                 |
 *	   | Adapter microcode load        (max 22032 bytes) |
 *	   |                                                 |
 *	   |-------------------------------------------------|
 *	   | Controller/dasd header                (6 bytes) |
 *	   |-------------------------------------------------|
 *	   | Controller header                    (12 bytes) |
 *	   |-------------------------------------------------|
 *	   |                                                 |
 *	   | Controller microcode load                       |
 *	   |                                                 |
 *	   |-------------------------------------------------|
 *	   | DASD header                          (12 bytes) |
 *	   |-------------------------------------------------|
 *	   |                                                 |
 *	   | DASD microcode load                             |
 *	   |                                                 |
 *	    -------------------------------------------------
 *	
 *	
 *	Notes on microcode packaging:
 *	----
 *	1.  The 'Identification of Controller/Dasd load' field relates to
 *	    the 'Model Number' given in the Controller Inquiry data.  If a
 *	    hardware change is made to the controller such that a different
 *	    microcode download is required, the changed controller will be
 *	    given a new model number and this will identify the load required.
 *	
 *	2.  It is not necessary to provide a model number identification
 *	    for the last, or only, controller download.  If only one load
 *	    is provided it should be downloaded whatever the model number
 *	    of the controller.  If several loads are provided then the last
 *	    one is used if the model number test fails on all preceding
 *	    loads.  (This allows the current header definition to remain
 *	    valid.)
 *	
 *	3.  The proposed redesign affects only the device driver code and
 *	    not the subsystem microcode.  It is the responsibility of the
 *	    Device Driver to issue 'Inquiry' to the controller, to inspect
 *	    the Model Number in the response, and to select the appropriate
 *	    load for downloading.
 *	    No change to Harrier-2 microcode will be required to handle
 *	    the extension.
 *	    However, depending on how the Device Driver works, it is possible
 *	    that no change to the Device Driver code is required at present,
 *	    in that only one controller load is currently included, and
 *	    the positions of the Offset Addresses of the adapter download
 *	    and of the sole controller download have not been changed.
 *	
 *	4.  The redesign is to handle several controller loads per adapter.
 *	    The question of several dasd loads per controller is different,
 *	    in that this can be handled either as above or by the Harrier-2
 *	    controller microcode.  It would be possible for the 'Write Buffer'
 *	    command to accept one controller download plus, say, two dasd
 *	    downloads, and to download to each attached dasd according to its
 *	    level and the download version it required.  We did not do this
 *	    for IPC1/IPC2 because IPC1 is only an interim version, support
 *	    for which is soon to be discontinued.  But it could if necessary
 *	    be done this way for future versions of the Redwing dasd, which
 *	    would be transparent to the Device Driver except for increasing
 *	    the length of the controller/dasd download data.
 *
 * RETURNS:	Returns 0 if success else -1
 *
 */

int download_ctrl(int	adapfd,		/* File descripter of parent adapter */
		  int	sid,		/* SCSI-id of controller 	     */
		  char	*ucodefile,
		  int   daemon_flg,	/* Microcode file 		     */
		  int   model_num,      /* controller's model number         */
		  int   ucodelvl)      /* microcode level                   */
{
	struct	stat sbuf;		/* stat structure */
	struct	sd_iocmd sddld;		/* SCSI download command structure */
	uchar	*cbuf;			/* Microcode buffer */
	int	rem;			/* Remainder after size / 1024 */
	int	codelen;		/* Length of microcode rounded up */
	int	ucode;			/* File descriptor for microcode */
	int	rc;
	int     length;
	int     temp,temp2;
	int     offset;
	int     adap_offset;		/* Field giving adapter offset */
	int     num_loads;              /* Number of controller/DASD loads   */
					/* in this ucode file                */
	int     i;                      /* general counter                   */
	int     pos,pos2;               /* position in ucode file            */
	char    tmpstr[5];              /* ascii equivalent of model number  */
	int     pack_id;                /* package id of microcode file      */
	char    buff[2];       		 /* to read in package id  */ 


	DEBUG_0("cfgserdasdc: download()\n")

	if( ucodefile == (char *)NULL )
	{
		DEBUG_0("special download of null file (called by daemon)\n")
		codelen = 0;
		cbuf = (uchar *)NULL;	/* Just some valid address */
	}
	else
	{

		if (stat(ucodefile,&sbuf)==-1) {
			/* stat failed -- run with code in controller */
			DEBUG_2(
		"download: stat(%s) failed, errno=%d, will use on board code\n",
				ucodefile, errno)
			return(-1);
		}

		if ((ucode = open(ucodefile,O_RDONLY)) <= 0) {
			DEBUG_1("serdasdc_domnld: can not open %s file\n",
				ucodefile)
			return(-1);
		}
		

		
		/* 
		 * Seek to microcode package id
		 */
		
		if (lseek(ucode,PACK_ID_OFFSET,0) == -1) {
			DEBUG_0("*ERROR: Bad seek")
			close(ucode);
			return(-1);      
		}


		/*
		 * Get package identification number.  This 
		 * is really just a double check.  The VV part of the
		 * microcode filename should be the same as this
		 * package id.   If not then don't download
		 */
		
		if (read(ucode,buff,2) < 0) {
			DEBUG_0("**ERROR: failed to read input file")
			close(ucode);
			return(-1);      
		}		

		pack_id = atoi(buff);
		DEBUG_2("pack_id = %d, ucodelvl = %d\n",pack_id,ucodelvl)
                if (pack_id <= ucodelvl) {
		        DEBUG_1("**ERROR: invalid package id= %d\n",pack_id)
			close(ucode);
			return(-1); 
		}
		  



		/* 
		 * Seek to  adapter offset word, because there may
		 * be multiple controller/DASD loads in this file
		 * and all of these offset are bounded below by the adapter 
		 * header.
		 */

		

		if (lseek(ucode,SD_ADAP_OFFSET,0) == -1) {
			DEBUG_0("*ERROR: Bad seek")
			close(ucode);
			return(-1);      
		}
		
		/*
		 * Get adapter offset word
		 */
		
		if (read(ucode,&adap_offset,SD_HEADER_SIZE) < 0) {
			DEBUG_0("**ERROR: failed to read input file")
			close(ucode);
			return(-1);      
		}		
		
		/*
		 * Determine how many Controller/Dasd
		 * downloads are in this file.
		 */

		num_loads = ((adap_offset - SD_CTRL_OFFSET) - 4)/8 + 1;
		
		pos = SD_CTRL_OFFSET;
		
		for(i = 0; i < num_loads;i++) {
			if ((i==0) && (num_loads > 1)) {
				pos += 4;
			}
			if (lseek(ucode,pos,0) == -1) {
				DEBUG_0("*ERROR: Bad seek");
				close(ucode);
				return(-1);
			}

			/*
			 * Get controller microcode offset/ or id
			 */
			
			if (read(ucode,&temp,SD_HEADER_SIZE) < 0) {
				DEBUG_0("**ERROR: failed to read input file")
					close(ucode);
				return(-1);
			}	
			
			DEBUG_1("found a package for model number %d\n",temp)
			
			if (i < (num_loads - 1)) {  /* not last one yet   */
				/*
				 * Compare model number of our controller
				 * with that of this load
				 */
				if (temp == model_num) {
					DEBUG_1("match for model num %d found\n",model_num)
					/*
					 * Get offset from file
					 */
					
					pos2 = pos - 4;
					
					if (lseek(ucode,(pos2),0) == -1) {
						DEBUG_0("*ERROR: Bad seek");
						close(ucode);
						return(-1);
					}
				
					/*
					 * Get controller microcode offset
					 */
				
					if (read(ucode,&temp,SD_HEADER_SIZE) < 0) {
						DEBUG_0("**ERROR: failed to read input file")
						close(ucode);
						return(-1);
					}	
					offset = temp + 6;  /* skip over ctlr package header*/
					/*
					 * Get next offset from file
					 */
					
					pos2 = pos + 4;
					
					if (lseek(ucode,(pos2),0) == -1) {
						DEBUG_0("*ERROR: Bad seek");
						close(ucode);
						return(-1);
					}
				
					/*
					 * Get controller microcode offset
					 */
				
					if (read(ucode,&temp2,SD_HEADER_SIZE) < 0) {
						DEBUG_0("**ERROR: failed to read input file")
						close(ucode);
						return(-1);
					}
					length = temp2 - offset;
					break;
				}
				else {
					DEBUG_0("not match model num yet\n")
					pos += 8;     /* goto to next load id */
				}
			}
			else {
				DEBUG_0("Using last or only load found\n")

				/*
				 * Compute size of controller microcode.
				 * Compute offset of controller by 
				 * ignoring the Controller/DASD header
				 *  which is not downloaded.
				 */
				
				if (num_loads == 1)  { /* Only load */
					offset = temp + 6;
					DEBUG_0("Only load offset \n")
				}
				else {
					/*
					 * Get offset from file
					 */
					if (lseek(ucode,(pos - 4),0) == -1) {
						DEBUG_0("*ERROR: Bad seek");
						close(ucode);
						return(-1);
					}
				
					/*
					 * Get controller microcode offset
					 */
				
					if (read(ucode,&temp,SD_HEADER_SIZE) < 0) {
						DEBUG_0("**ERROR: failed to read input file")
						close(ucode);
						return(-1);
					}	
					offset = temp + 6;
				}
				length = sbuf.st_size - offset;
				
				
			       
			}
			
		}

		/* 
		 * calculate microcode size (pad to 1K) 
		 */
		if ((rem=length%1024)!=0)
			codelen = length +(1024-rem);
		else
			codelen = length;

		DEBUG_1("download: adjusted length of ucode=%d\n",codelen)


		/* allocate space for microcode in memory */
		if ((cbuf = (uchar *) malloc(codelen)) == NULL) {
			DEBUG_0("serdasdc_domnld: malloc failed\n")
			close(ucode);
			return(-1);
		}
		
		/* 
		 * Seek to controller microcode load
		 */
		
		if (lseek(ucode,offset,0) == -1) {
			DEBUG_0("*ERROR: Bad seek");
			free(cbuf);
			close(ucode);
			return(-1);
		}

		/* read microcode into memory */
		if (read(ucode,cbuf,length)<0){
			DEBUG_2(
			"serdasdc_domnld: err reading %s file, errno=%d\n",
				ucodefile,errno)
			free(cbuf);
			close(ucode);
			return(-1);
		}
		close(ucode);
	}

	DEBUG_1("download: read microcode into memory @ 0x%x\n",cbuf)

	memset( &sddld, 0, sizeof(sddld) );
	sddld.buffer = cbuf;
	sddld.data_length = codelen;
	sddld.command_length = 10;
	sddld.timeout_value = 45;

	sddld.resvd5 = ( sid << 5 ) | 0x10;	/* set tarlun bit (0x10) */
	sddld.scsi_cdb[0] = 0x3B;	/* Write Buffer */
	sddld.scsi_cdb[1] = 0x04;	/* download microcode instruction */
	sddld.scsi_cdb[6] = 0xff & ( codelen >> 16 );
	sddld.scsi_cdb[7] = 0xff & ( codelen >> 8 );
	sddld.scsi_cdb[8] = 0xff & ( codelen );
	sddld.flags = B_WRITE;

	rc = run_scsi_cmd( adapfd, &sddld);

	/* sleep(5);	*//* after download, let controller recover */

	free(cbuf);
	return(rc);
}


/*
 * NAME:	download_microcode_ctrl
 * 
 * FUNCTION:	This function determines the proper level of microcode to
 *		download to the device, downloads it, and updates the CuAt
 *		object class to show the name of the file that was used.
 *
 * ENVIRONMENT:	This function operates as a device dependent subroutine
 *		called by the generic configure method for all devices.
 *
 * INPUT:	logical_name of device
 *
 * RETURNS:	Returns 0 if success else -1
 *
 * NOTE :	During download, the adapter is opened in diagnostics mode
 *		(..thus requiring exclusive access)
 *		Microcode file size can not be greater than 64K.
 */

int download_microcode_ctrl(int sid, char *adap_name,
			    int daemon_flg,
			    int adapfd)
{
	char	sstr[SD_MAX_STRING_LEN];/* Search string              */
	int	ctrllvl;		/* Controller hardware level  */
	int	ucodelvl;		/* Controller microcode level */
	char	filename[SD_UCODE_FILE_LENGTH];	/* Microcode file name  */
	char	dev[SD_DEV_FILE_LENGTH];     /* Adapter special file name  */
	struct	CuDv	cusobj;		/* CuDv ovject for controller */
	int     rc;
	int     model_num = 0;


	sprintf(dev,"/dev/%s",adap_name);

	/* 
	 * open adapter (i.e. parent), if we are the daemon the
	 * we already have the adapter open
	 */
	if (!daemon_flg) {
		if((adapfd = open(dev,O_RDWR)) < 0) {
			DEBUG_2("download_microcode: failed to open %s,errno=%d\n",dev,errno);
			return(0);
		}
	}

	DEBUG_1("adapter open with adapfd = %d\n",adapfd)

        sprintf( sstr,"parent = '%s' AND connwhere = '%d'",adap_name,sid);
	rc = (int)odm_get_obj( CuDv_CLASS, sstr, &cusobj, ODM_FIRST );

	if( rc <= 0) {
		DEBUG_0("SERDASD_DAEMON:odm_get_obj has failed \n")
                return;
	}
	if( get_ctrl_lvl(adapfd,sid,&ctrllvl,&ucodelvl,&model_num) ) {
		if (!daemon_flg)
			close(adapfd);

		DEBUG_0("dldserdasdc: get_ctrl_lvl failed\n")

		/*
		 * Assume the worst: ie controller is running
		 * with on board firmware
		 */

		sprintf(sstr,"name = '%s' AND attribute = 'ucode'",cusobj.name);
		odm_rm_obj(CuAt_CLASS,sstr);		
		if (daemon_flg)
			return (-1);
		else
			return 0;

	}

	DEBUG_1("ctrllvl = %d\n",ctrllvl)

	/* 
	 * get name of microcode file -- if no file found, just run
	 * with the on-board firmware 
	 */

	if ( !ucodename(SD_CARDID,ctrllvl,ucodelvl,filename) ) {
		DEBUG_1("microcode file not found adapfd = %d\n",adapfd);
		if (!daemon_flg) {
			if ((rc = close(adapfd)))
				DEBUG_1("closed failed errno = %d\n",errno);
			DEBUG_1("adapter now closed rc = %d\n",rc);
		}
		if (ucodelvl > 0) {
			SETATT( cusobj.name, "ucode", filename)
		}
		else {
			sprintf(sstr,"name = '%s' AND attribute = 'ucode'",cusobj.name);
			odm_rm_obj(CuAt_CLASS,sstr);			

		}
		return(0);
	}

	/* download ucode to device */
	rc = download_ctrl(adapfd,sid,filename,daemon_flg,model_num,ucodelvl);


	if (!rc) {
		SETATT( cusobj.name, "ucode", filename)
		DEBUG_0("Download finished & successful...returning rc=0\n");
	}
	else {
		DEBUG_0("Download error\n");
		if (!daemon_flg)
			close(adapfd);
		if (daemon_flg)
			return (-1);
		else
			return 0;
	}
	if (!daemon_flg) {
		if ((rc = close(adapfd)))
			DEBUG_1("closed failed errno = %d\n",errno);
		DEBUG_1("adapter now closed rc = %d\n",rc);
	}
	return(0);
}
