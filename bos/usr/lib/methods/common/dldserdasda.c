static char sccsid[] = "@(#)21	1.13  src/bos/usr/lib/methods/common/dldserdasda.c, cfgmethods, bos411, 9428A410j 9/17/91 13:46:47";
/*
 * COMPONENT_NAME: (CFGMETH) configuration method for serial dasd controller
 *
 * FUNCTIONS:	get_adap_lvl, download_adap download_microcode_adap
 * 
 * ORIGINS:	27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1991
 * Unpublished Work
 * All Rights Reserved
 *
 * RESTRICTED RIGHTS LEGEND
 * US Government Users Restricted Rights -  Use, Duplication or
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
 * NAME:	get_adap_lvl
 *
 * FUNCTION:	Reads the level of the adapter hardware, and the level of the
 *		microcode on the adapter
 *
 * ENVIRONMENT:	This function is used by the cfgserdasda method.
 *
 * RETURNS:	0 on success, -1 on failure
 */

int get_adap_lvl(int adap, 		  /* Adaptor file descriptor        */
		 int *adaplvl,		  /* place to store adapter level   */
		 int *ucodelvl)		  /* place to store microcode level */
{
	struct	sd_ioctl_parms parms;	/* command structure for inquiry */
	char	inquiry_results[16];
	char	tmpstr[3];

	DEBUG_0("cfgserdasda: get_adap_lvl\n")

	parms.data_length = 16;
	parms.buffer = inquiry_results;
	parms.time_out = 2;

	if( ioctl( adap, SD_ADAP_INQUIRY, &parms ) == -1 ) {
		DEBUG_1("get_adap_lvl: ioctl( ..SD_ADAP_INQUIRY.. ) failed: errno =%d\n",errno)
		return -1;
	}
	tmpstr[2] = '\0';
	strncpy( tmpstr, inquiry_results, 2 );	   /* Microcode package id */

	/*
	 * Microcode package id will be used for VV since 
	 * one file contains both adapter and controller
	 */
	*ucodelvl = atoi( tmpstr );
	tmpstr[6] = '\0';
	strncpy( tmpstr, &inquiry_results[4], 2 ); /* Microcode level id */
	*adaplvl = atoi( tmpstr );

	return 0;
}

/*
 * NAME:	download_adap
 * 
 * FUNCTION:	Downloads the microcode in ucodefile to the device accessed by
 *		devname.
 *
 * ENVIRONMENT:	This function is internal to the cfgserdasda method and daemon
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
 *
 * RETURNS:	Returns 0 if success else -1
 */

int download_adap(int	adap,		     /* File descriptor for adapter */
		  char	*ucodefile,	     /* Name of microcode file 	    */
		  int   ucodelvl)	     /* microcode level */
{
	struct	stat sbuf;		/* struct for file status record */
	struct	sd_ioctl_parms sddld;	/* command structure for download */
	uchar	*cbuf;			/* pointer to space for microcode */
	int	rem;			/* remainder after size/1024 */
	int	codelen;		/* ucode size rounded up to mult 1024*/
	int	ucode;			/* file descriptor for microcode */
	int	rc;                     /* return code                   */
	int     length;                 /* length of adapter microcode   */
	int     offset;                 /* offset to adapter microcode   */
	int     temp;                   /* offset to controller microcode*/
	int     pack_id;                /* package id  of microcode file */
	uchar   buff[2];                 /* to read in package id  */     


	DEBUG_0("dldserdasda: download\n")

	if( ucodefile == (char *)NULL )
	{
		DEBUG_0("dummy download of null file (called by daemon)\n")
		cbuf = (uchar *)NULL;  /* just some valid address */
		codelen = 0;
	}
	else
	{
		if (stat(ucodefile,&sbuf)==-1) {
			/* stat failed -- run with code in adapter */
			DEBUG_2("download: stat of %s failed, errno=%d\n",
				ucodefile, errno)
			return(-1);
		}



		if ((ucode = open(ucodefile,O_RDONLY)) <= 0) {
			DEBUG_1("serdasda_domnld: can not open %s file\n",
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
		        DEBUG_1("**ERROR: invalid package id = %d\n",pack_id)
			close(ucode);
			return(-1); 
		}
		  
		/* 
		 * Seek to adapter offset word
		 */
		
		if (lseek(ucode,SD_ADAP_OFFSET,0) == -1) {
			DEBUG_0("*ERROR: Bad seek")
			close(ucode);
			return(-1);     
		}
		
		/*
		 * Get adapter offset word
		 */
		
		if (read(ucode,&offset,SD_HEADER_SIZE) < 0) {
			DEBUG_0("**ERROR: failed to read input file")
			close(ucode);
			return(-1);      
		}		
		
		
		/* 
		 * Seek to  ctrl offset word
		 */
		
		if (lseek(ucode,SD_CTRL_OFFSET,0) == -1) {
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
		
		/*
		 * compute size of adapter microcode since it ends 
		 * where the controller microcode starts
		 */
		
		length = temp - offset;
		

		/* calculate microcode size (pad to 1K) */
		if ((rem=length%1024)!=0)
			codelen = length+(1024-rem);
		else
			codelen = length;

		DEBUG_1("download: adjusted length of ucode=%d\n",codelen)

		/* allocate space for microcode in memory */
		if ((cbuf = (uchar *) malloc(codelen)) == NULL) {
			DEBUG_0("serdasda_domnld: malloc failed\n")
			close(ucode);
			return(-1);
		}

		
		/* 
		 * Seek to adapter microcode load
		 */
		
		if (lseek(ucode,offset,0) == -1) {
			DEBUG_0("*ERROR: Bad seek");
			free(cbuf);
			close(ucode);
			return(-1);
		}

		/* 
		 * read microcode into memory 
		 */
		if (read(ucode,cbuf,length)<0){
			DEBUG_2(
			"serdasda_domnld: err reading %s file, errno=%d\n",
				ucodefile,errno)
			free(cbuf);
			close(ucode);
			return(-1);
		}
		close(ucode);

		DEBUG_1("download: read microcode into memory @ 0x%x\n",cbuf)
	}

	/* setup parameter block for download command */
	memset( &sddld, 0, sizeof(sddld) );
	sddld.data_length = codelen;
	sddld.buffer = cbuf;
	sddld.time_out = 40;


	rc = 0;

	/* download microcode */
	if (ioctl(adap,SD_ADAP_DOWNLOAD,&sddld)<0) 
		rc = -1;

	if (rc == -1) {
#ifdef CFGDEBUG
		switch( errno ) {
		case EFAULT:
			DEBUG_0("Download error: EFAULT\n")
			break;
		case ETIMEDOUT:
			DEBUG_0("Download error: ETIMEDOUT\n")
			break;
		case EIO:
			DEBUG_0("Download error: EIO\n")
			break;
		default:
			DEBUG_1("Download error: %d\n", errno )
		}
	}
	else {
		DEBUG_0("download: microcode downloaded successfully\n")
#endif
	}
	DEBUG_0("Freeing cbuf\n")
	free(cbuf);
	DEBUG_1("Returning from download() with rc = %d\n", rc ) 
	return(rc);
}


/*
 * NAME : download_microcode_adap
 * 
 * FUNCTION :
 *	This function determines the proper level of microcode to
 *	download to the device, downloads it, and updates the CuAt
 *	object class to show the name of the file that was used.
 *
 * EXECUTION ENVIRONMENT:
 *	This function operates as a device dependent subroutine
 *	called by the generic configure method for all devices.
 *
 * INPUT  : logical_name
 *
 * RETURNS: Returns 0 if success else errno
 *
 * RECOVERY OPERATION:
 *
 * NOTE :
 *	During download, the adapter is opened in diagnostics mode
 *		(..thus requiring exclusive access)
 *	Microcode file size can not be greater than 64K.
 */
int download_microcode_adap(char *lname,
			    int daemon_flg,
			    int adap)         /* Adapter file descriptor */
{
	char	sstr[SD_MAX_STRING_LEN];
	int	adaplvl;
	int	ucodelvl;
	char	filename[SD_UCODE_FILE_LENGTH];/* Microcode file name       */
	char	dev[SD_DEV_FILE_LENGTH];    /* Adapter special file name */





	DEBUG_0("dldserdasda: download_microcode\n")


        sprintf(dev,"/dev/%s",lname);

	if (!daemon_flg) {
		if(( adap = openx(dev, 0, 0, SC_DIAGNOSTIC )) == -1 ) {
			DEBUG_1("Failed to open %s in diagnostic mode\n", dev);
			return 0;
		}
	}

	

	if( get_adap_lvl(adap,&adaplvl,&ucodelvl) )
	{
		if (!daemon_flg)
			close( adap );
		DEBUG_0("dldserdasda: get_adap_lvl failed\n")

		/*
		 * Assume the worst: ie adapter is running
		 * with on board firmware
		 */

		sprintf(sstr,"name = '%s' AND attribute = 'ucode'",lname);
		odm_rm_obj(CuAt_CLASS,sstr);
		if (daemon_flg)
			return (-1);
		else
			return 0;
	}

	DEBUG_1("adaplvl = %d\n",adaplvl)

	/* 
	 * get name of microcode file -- if no file found with newer 
	 * microcode then just run
	 *  with the current version, already on the adapter
	 */

	if (ucodename(SD_CARDID,adaplvl,ucodelvl,filename)== 0) {
		DEBUG_0("microcode file not found\n")
		if (!daemon_flg)
			close(adap);
		if (ucodelvl > 0) {
			SETATT( lname, "ucode", filename)
		}
		else {
			sprintf(sstr,"name = '%s' AND attribute = 'ucode'",lname);
			odm_rm_obj(CuAt_CLASS,sstr);
		}
		return 0;
	}

	/* 
	 * download ucode to device 
	 */
	if (download_adap(adap,filename,ucodelvl)==0) {
		SETATT( lname, "ucode", filename)
	} else {
		DEBUG_0("Returning with error E_UCODE\n") 
		if (!daemon_flg)
			close(adap);		
		if (daemon_flg)
			return (-1);
		else
			return 0;

	}

	DEBUG_0("Download finished & successful...returning rc=0\n");
	if (!daemon_flg)
		close(adap);
	return 0;
}
