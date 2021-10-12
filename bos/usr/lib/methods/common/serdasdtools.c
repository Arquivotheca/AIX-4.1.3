static char sccsid[] = "@(#)24	1.8.1.2  src/bos/usr/lib/methods/common/serdasdtools.c, cfgmethods, bos411, 9428A410j 12/6/93 08:06:10";
/*
 * COMPONENT_NAME: CFGMETHODS
 *
 * FUNCTIONS : 	run_scsi_cmd, read_sd_pvid, ucodename, scsi_inq,
 * FUNCTIONS :	add_desc_fixed, get_unique_type ,
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



/* header files needed for compilation */

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
#include <sys/dir.h>
#include "cfgdebug.h"
#include "cfgserdasd.h"

#define NULLPVID "00000000000000000000000000000000"
#ifndef INQSIZE
#define INQSIZE 255
#endif
/*
 * NAME:	run_scsi_cmd
 *
 * FUNCTION:	Performs a scsi pass-through command using the command
 *		structure passed in, with a retry if the command fails
 *
 * ENVIRONMENT:	This function is used by the serial DASD configuration routines
 *
 * RETURNS:	Returns 0 if success else -1
 */

int run_scsi_cmd(int fd,struct sd_iocmd *cmdaddr)
{
	struct sd_iocmd cmd_cpy;
	struct sd_iocmd sense_cmd;
	uchar sense_data[255];

	int retry_count, rc;

	cmd_cpy = *cmdaddr;

	for( retry_count=0; retry_count < 3; retry_count++ )
	{

		if( retry_count )
		{
			/* re-write the command */
			DEBUG_0("RE-TRYING COMMAND\n" )
			*cmdaddr = cmd_cpy;
		}

		DEBUG_2("run_scsi_cmd(): tarlun = %02x, scsi command = %02x\n",
			(int)cmdaddr->resvd5, (int)cmdaddr->scsi_cdb[0] )
		rc = ioctl( fd, SD_SCSICMD, cmdaddr );

		if( rc)
		{
#ifdef CFGDEBUG
			switch( errno ) {
			case EFAULT:
				DEBUG_0("Ioctl error: EFAULT\n")
				break;
			case ETIMEDOUT:
				DEBUG_0("Ioctl error: ETIMEDOUT\n")
				break;
			case EIO:
				DEBUG_0("Ioctl error: EIO\n")
				break;
			default:
				DEBUG_1("Ioclt error: %d\n", errno )
			}
#endif
			if (errno != ETIMEDOUT)
				return -1;
		}
		if (errno == ETIMEDOUT)
			continue;
		if( cmdaddr->status_validity == 0 )
		{
			DEBUG_0("status_validity = 0\n" )
			return 0;
		}

#ifdef CFGDEBUG
		DEBUG_1("status_validity = %d\n",(int)cmdaddr->status_validity)

		if( cmdaddr->status_validity & 0x01 )
		{
			DEBUG_1( "scsi_bus_status = 0x%02X\n",
				(int)cmdaddr->scsi_bus_status )
		}
		if( cmdaddr->status_validity & 0x02 )
		{
			DEBUG_1( "adapter_status  = 0x%02X\n",
				(int)cmdaddr->adapter_status )
		}
		if( cmdaddr->status_validity & 0x04 )
		{
			DEBUG_1(
			"Alert Register Adapter Status (resvd2) = 0x%02X\n",
				(int)cmdaddr->resvd2 )
			DEBUG_1(
			"Alert Register Controller Status (resvd1) = 0x%02X\n",
				(int)cmdaddr->resvd1 )
		}
#endif
		/*
		 * If it wasnt a SCSI status which failed us or
		 * a special controller status then give up now
		 */
		if( ! (( cmdaddr->status_validity == 1 ) ||
		   (( cmdaddr->status_validity & 0x04 ) && 
		    ( cmdaddr->resvd2 == SD_CTRL_STATUS ))) )
			return -1;

		if( cmdaddr->scsi_bus_status == SC_CHECK_CONDITION )
		{
			DEBUG_0("Check condition\n")

			/* Now do a request sense */
			
			memset( &sense_cmd, '\0', sizeof( sense_cmd ) );

			sense_cmd.data_length = 255;
			sense_cmd.buffer = sense_data;
			sense_cmd.timeout_value = 2;
			sense_cmd.command_length = 6;
			sense_cmd.resvd5 = cmd_cpy.resvd5;
			sense_cmd.flags = B_READ;

			sense_cmd.scsi_cdb[0] = 0x03;	/* request sense */
			sense_cmd.scsi_cdb[4] = 255;	/* data length */

			if( ioctl( fd, SD_SCSICMD, &sense_cmd ) == -1 )
			{
				DEBUG_0("REQUEST SENSE FAILED\n")
				return -1;
			}
			DEBUG_0("request_sense data:\n")
			hexdump( sense_data, (long)255 );

			continue;
		}
	}

	DEBUG_0("RETRIES FAILED\n")

	return -1;
}
/*
 * NAME:	read_sd_pvid
 *
 * FUNCTION:	Reads the first block of the disk, and extracts the pvid.
 *
 * ENVIRONMENT:	This function is used by serial DASD configuration routines
 *
 * NOTE: (On Twin tailing change with extended read )
 *
 *   Byte 1 of the CDB for read(10) now looks like this.
 *
 * bit  7    6    5    4    3    2    1    0
 *   -----------------------------------------
 *   |  L U N        | 0  |  0 | 0  | RWR| 0  |
 *   -----------------------------------------
 *
 * If this bit (RWR) is 0, the code will reject a read command to a drive 
 * reserved by the other host adapter..
 *
 * If the bit is set to 1 then the new microcode will allow the read regardless
 * of whether the disk is reserved or not. The old microcode would reject the
 * command as invalid.
 *
 * This change will only be in microcode so ensure that the code has 
 * been downloaded before you issue the modified read command.
 *
 * RETURNS:	Returns 0 if success else -1
 *		The pvid is stored via the pvid pointer passed in.
 */

int read_sd_pvid(int		ctrlfd,	  /* File descriptor for controller */
		 int		tarlun,	  /* Tar-lun for dasd 		    */
		 char		read_w_reserve, /* whether read with reserve*/
						/* is allowed		    */
		 char		*pvidstr) /* Address to store pvid 	    */
{
	struct sd_iocmd iocmd;		/* Structure for SCSI command */
	int	rc;			/* return code */
	char	firstblock[512];	/* buffer for first block from disk */
	IPL_REC_PTR ipl_rec_ptr;	/* pointer to boot record */
	char *ptr,*ptr2;

 
	DEBUG_1("cfgserdasd: read_sd_pvid( ctrlfd, tarlun=0x%02x, &pvid )\n",
		tarlun )

	/* build command structure */
	memset( &iocmd, 0, sizeof(iocmd) );
	iocmd.data_length = 512;
	iocmd.buffer = firstblock;
	iocmd.timeout_value = 2;
	iocmd.command_length = 10;
	iocmd.resvd5 = tarlun;
	iocmd.scsi_cdb[0] = (uchar)0x28;/* i.e. a read */
	iocmd.scsi_cdb[1] = (uchar) read_w_reserve & 0x2;	/* block 0 */
	iocmd.scsi_cdb[2] = (uchar)0;	/* " */
	iocmd.scsi_cdb[3] = (uchar)0;	/* " */
	iocmd.scsi_cdb[4] = (uchar)0;	/* " */
	iocmd.scsi_cdb[5] = (uchar)0;	/* " */
	iocmd.scsi_cdb[6] = (uchar)0;	/* " */
	iocmd.scsi_cdb[7] = (uchar)0;	/* one block */
	iocmd.scsi_cdb[8] = (uchar)1;	/* one block */
	iocmd.scsi_cdb[9] = (uchar)0;	
	iocmd.flags = B_READ;

	rc = run_scsi_cmd( ctrlfd, &iocmd);

	if (rc)
		return(rc);

	ipl_rec_ptr = (IPL_REC_PTR)(&firstblock[0]);

	DEBUG_0("firstblock:\n")

		
	hexdump( ipl_rec_ptr, 512L );

	if( rc == 0 ) {
		/*
		 * Determine if this is a valid pvid
		 * by checking if the IPLRECID is on the
		 * disk in its specified location.  If so then
		 * copy pvid to pvidstr and convert it to ascii.
		 * If not valid pvid then set pvidstr to NULL;
		 */


		if (ipl_rec_ptr->IPL_record_id == (unsigned int)IPLRECID) {
			DEBUG_0("getpvidstr:record_id == IPLRECID\n")
			strcpy(pvidstr,pvidtoa(&(ipl_rec_ptr->pv_id)));
			if (!strcmp(pvidstr,NULLPVID)) {
				DEBUG_0("read_sd_pvid:NullPVID !!!\n")
				pvidstr[0] = '\0';
			}
		} else
			pvidstr[0] = '\0';
	}

	DEBUG_1("offset of pv_id in block is 0x%x\n",
		(int)( & ( ipl_rec_ptr->pv_id ) ) - (int)ipl_rec_ptr )

	DEBUG_0("pv_id is:\n")

	hexdump(pvidstr, (long)sizeof(ipl_rec_ptr->pv_id) );

	return(rc);
}
/*
 * NAME:	ucodename
 * 
 * FUNCTION:	Determines the name of the file to be downloaded
 *
 * ENVIRONMENT:	This routine is used by the serial DASD configuration routines
 *
 * RETURNS:	0 if no newer file is found, otherwise 1
 *
 * NOTE:	The format for microcode file names is: deviceid.LL.VV
 *		deviceid is the hex id obtained from POS registers 0 and 1
 *		with a 'c' on the end for controllers.
 *		LL is revision level of the device from the LL field of
 *		the VPD. VV is version number of the microcode.
 */

int ucodename(char	*deviceid,		/* Adapter card id         */
	      int	devlvl,			/* Hardware level          */
	      int	ucodelvl,		/* Microcode level         */
	      char	*filename)		/* File name pointer       */
{
	struct	dirent	*fp;		/* Filesystem directory entry */
	DIR	*dp;			/* Pointer to Directory structure */
	char	curfile[SD_UCODE_FILE_LENGTH];	/* Name of current file */
	int	len;			/* Length of name which must match */
	int	rc=0;
	char    tempfile[3];            /* contains ascii version of ucodelv*/


	DEBUG_0("cfgserdasd: ucodename\n")


#ifdef	SD_3_1_BASE

	/* create basic mask to start searching the directory with */
	sprintf(curfile,"%s.%02d.",deviceid,devlvl);


	/* 
	 * If this if for a 3.1 operating system
	 */

	len = strlen(curfile);

	/* add on current microcode level */
	sprintf(&curfile[len],"%02d",ucodelvl);

	DEBUG_1("ucodename: curfile=%s\n",curfile)

	/* open the microcode directory */
	if ((dp = opendir("/etc/microcode"))==NULL) {
		DEBUG_1("opendir(/etc/microcode) failed, errno=%d\n", errno)
		return(0);
	}

 	/* look through the directory and try to find the latest version
	   for this hardware level */
	while ((fp = readdir(dp)) != NULL) {

		if (!strncmp(curfile,fp->d_name,len)) {
			if ( strncmp(curfile,fp->d_name,len+2) < 0 ) {
				DEBUG_1("ucodename: found newer file %s\n",
					fp->d_name)
				strcpy(curfile,fp->d_name);
				rc = 1;
			}
#ifdef CFGDEBUG
		} else {
			DEBUG_2("ucodename: no match filename=%s,curfile=%s\n",
				fp->d_name,curfile)
#endif
		}
	}

	closedir(dp);

	sprintf(filename,"/etc/microcode/%s",curfile);

#else
	/* 
         * If this is for a 3.2 or newer operating system
	 */


	/* create basic mask to start searching the directory with */
	sprintf(curfile,"%s.%02d",deviceid,devlvl);

	sprintf(tempfile,"%02d",ucodelvl);
        if (findmcode(curfile,filename,VERSIONING,0)) {
		len = strlen(filename);
		if (len > 0) {
			/*
                         * Check if this version is newer then
                         * version on the hardware.
                         */

			if (strncmp(&filename[len-2],tempfile,2) > 0 ) {
				DEBUG_1("ucodename: found newer file %s\n",
					filename)
				rc = 1;
			}
			else {
				DEBUG_0("No microcode file found\n")
				sprintf(filename,"%s.%02d.",deviceid,devlvl);
				len = strlen(filename);

				/* add on current microcode level */
				sprintf(&filename[len],"%02d",ucodelvl);
 				return(0);
			}
		}
		else {
			DEBUG_0("No microcode file found\n")
			sprintf(filename,"%s.%02d.",deviceid,devlvl);
			len = strlen(filename);

			/* add on current microcode level */
			sprintf(&filename[len],"%02d",ucodelvl);
			return(0);
		}
	}
	else {
		DEBUG_0("No microcode file found\n")
		sprintf(filename,"%s.%02d.",deviceid,devlvl);
		len = strlen(filename);

		/* add on current microcode level */
		sprintf(&filename[len],"%02d",ucodelvl);

		return(0);
	}
		
#endif
	DEBUG_1("ucodename: microcode file=%s\n",filename)
	return(rc);
}
/*
 * NAME:	scsi_inq
 *
 * FUNCTION:	Performs a SCSI inquiry to the specified sid, and lun
 *
 * RETURNS:	0 for success, -1 for failure
 *
 */

int scsi_inq(int	adapfd,		   /* File descripter for adapter   */
	     uchar	*inqdata,	   /* Address to store inquiry data */
	     int	tarlun)		   /* target sid & lun              */
{
	struct	sd_iocmd inq;		/* Structure for SCSI pass-thru ioctl */
	int	rc;			/* Return code */

	DEBUG_1("cfgserdasd: scsi_inq(tarlun = %02x)\n", tarlun )

	/* build inquiry command structure */
	memset( &inq, 0, sizeof(inq) );
	inq.data_length = INQSIZE;
	inq.buffer = inqdata;
	inq.timeout_value = 10;
	inq.command_length = 6;
	inq.resvd5 = tarlun;
	inq.scsi_cdb[0] = 0x12;	/* i.e. an inquiry */
	inq.scsi_cdb[4] = INQSIZE;
	inq.flags = B_READ;

	return run_scsi_cmd( adapfd, &inq)?E_DEVACCESS:0;
}
/*
 * NAME:	add_desc_fixed
 *
 * FUNCTION:	Adds a fixed-length descriptor to the VPD from the
 *		Inquiry data returned from the disk.
 *		The Inquiry data is also examined to ensure that enough
 *		data is available.
 *
 * INPUTS:	Two character name of descriptor, Destination VPD,
 *		Inquiry data from disk, Offset within inq_data, and
 *		Length to copy.
 *
 * RETURNS:	NONE
 *
 */

void add_desc_fixed(char *desc_name, /* Two character name of descriptor     */
		    char *vpd_dest,  /* VPD field within object		     */
		    char *inq_data,  /* Inquiry data containing data	     */
		    int	offset,	     /* Offset within inq_data of reqd data  */
		    int	len)	     /* Length of data			     */
{
	char	tmp_space[100];

	DEBUG_1("serdasdtools: add_desc_fixed len = %d\n",len)

	if( offset+len > 5+(int)inq_data[4] )
		return;		/* The inq_data returned was too short	*/

	strncpy( tmp_space, &inq_data[offset], len );

	tmp_space[len] = '\0';

	add_descriptor( vpd_dest, desc_name, tmp_space );

}
/*
 * NAME:	get_unique_type
 *
 * FUNCTION:	Determines the modelname from the SCSI-inquiry data passed in,
 *		then matches that with a PdAt attribute in the database.
 *		If no match is found, the DEFAULTSERDASDD model_name is used.
 *
 * INPUTS:	The inquiry data from the disk, and an address to store the
 *		unique_type.
 *
 * RETURNS:	0 = success, else an error number from cf.h
 *
 */

int get_unique_type(uchar *inqdata,
		    char *unique_type)
{
	int	i,rc;
	struct	PdAt PdAt;
	char	sstr[SD_MAX_STRING_LEN];
	char	model_name[17];

	/* built model_name attribute value from inquiry data */
	for (i=0; i<3; i++) {
		char	c;
		c = (char) inqdata[i+20];
		if( c < ' ' || c >= '\177' )
			c = ' ';
		model_name[i] = c;
	}
	model_name[i] = '\0';

	DEBUG_1("chktype: model_name=%s\n",model_name)

	/* make sure inquiry data is for a disk */
	if ((inqdata[0] & '\037') != '\000') {
		DEBUG_0("Device is of wrong class entirely\n")
		return E_WRONGDEVICE;	/* Device class is incorrect */
	}
	sprintf(sstr,"attribute = 'model_name' AND deflt = '%s'",model_name);
	DEBUG_1("chktype: calling odm_get for *%s*\n",sstr) 
	rc = (int)odm_get_obj(PdAt_CLASS,sstr,&PdAt,ODM_FIRST);
	if (rc == 0) {
		DEBUG_0("Unable to read disk type\n")
		return E_WRONGDEVICE;
	}

	if (rc == -1) {
		DEBUG_1("Error searching for device type using\n%s\n",sstr)
		return E_ODMGET;
	}

	/* store the unique_type, and return */
	strcpy( unique_type, PdAt.uniquetype );
	return 0;
}



/* 
 *
 * NAME: sd_daemon_get_lock
 *                  
 * FUNCTION: This routine  gets database lock for daemon.
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault.  
 *
 *
 *
 * (DATA STRUCTURES:) 
 *                     
 * 
 *
 * CALLED BY:
 *      
 * 	download_microcode_adap		download_microcode_ctrl
 *
 * INTERNAL PROCEDURES CALLED:
 * 
 *    None
 *
 * EXTERNAL PROCEDURES CALLED:
 *      
 *    odm_initialize			odm_lock
 *
 * (RECOVERY OPERATION:) If error occurs the daemon may kill itself
 *     depending on the severity of the error, otherwise it will
 *     try to continue running
 *
 * RETURNS:      
 *       
 *        0         -   Successful completion.
 */
int sd_daemon_get_lock(int *lock)
{
	


	if( odm_initialize() == -1 ) {
		DEBUG_0("SERDASD_DAEMON: Daemon error initializing database\n")
		return (-1);
		
	}
	
	DEBUG_0 ("SERDASD_DAEMON: ODM initialized now getting lock\n")
		
       /* lock the database */
        while ((*lock =odm_lock("/etc/objrepos/config_lock",0)) == -1) {
		       DEBUG_0("SERDASD_DAEMON: odm_lock() failed\n")
		       sleep(1);
			
        }
	
	DEBUG_0 ("ODM locked\n")

	return (0);
}


/* 
 *
 * NAME: sd_daemon_free_lock
 *                  
 * FUNCTION: This routine  gets frees database lock for daemon.
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault.  
 *
 *
 *
 * (DATA STRUCTURES:) 
 *                     
 * 
 *
 * CALLED BY:
 *      
 * 	download_microcode_adap		download_microcode_ctrl
 *
 * INTERNAL PROCEDURES CALLED:
 * 
 *    None
 *
 * EXTERNAL PROCEDURES CALLED:
 *      
 *    odm_terminate			odm_unlock
 *
 * (RECOVERY OPERATION:) If error occurs the daemon may kill itself
 *     depending on the severity of the error, otherwise it will
 *     try to continue running
 *
 * RETURNS:      
 *       
 *        0         -   Successful completion.
 */
void sd_daemon_free_lock(int *lock)
{
	

	if (odm_unlock(*lock) == -1) {
		DEBUG_0("SERDASD_DAEMON: ODM unlocked failed\n")
		return;
	}
	odm_terminate();


	return;
}

