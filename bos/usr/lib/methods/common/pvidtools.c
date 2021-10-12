static char sccsid[] = "@(#)48	1.8  src/bos/usr/lib/methods/common/pvidtools.c, cfgmethods, bos411, 9428A410j 3/23/93 09:31:28";

/*
 * COMPONENT_NAME: (CFGMETHODS) pvidtools.c
 *
 * FUNCTIONS: bootdev()
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

#include <fcntl.h>
#include <sys/types.h>
#include <sys/bootrecord.h>
#include <sys/utsname.h>
#include <sys/scsi.h>
#include <cf.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <sys/hd_psn.h>
#include <stdio.h>
#include <errno.h>
#include "cfgdebug.h"

#define makehex(x) "0123456789abcdef"[x&15]
#define NULLPVID "00000000000000000000000000000000"

/*
  convert pvid to ascii string (33 bytes total)
 */

char *pvidtoa(ip)
struct unique_id *ip;
{
	int i;
	char *ptr = (char*)ip;
	static char buf[33];
	
	for(i=0;i<32;i++) {
		if (i&1)
			buf[i] = makehex(ptr[i/2]);
		else
			buf[i] = makehex(ptr[i/2]>>4);
	}
	buf[32] = '\0';
	return(buf);
}

int
pvid_to_disk(lname, pvidstr,openx_ext)
char    *lname;			/* Device's logical name	*/
char    *pvidstr;		/* ASCII PVID string created	*/
int     openx_ext;		/* extended parmeter for openx	*/
{
struct	unique_id uid;
int	rc;

	DEBUG_0("Calling mkuuid from pvid_to_disk\n")
	mkuuid(&uid);
	strcpy(pvidstr,pvidtoa(&uid));
	DEBUG_0("Calling write_pvid from pvid_to_disk\n")
	rc = write_pvid(lname, uid,openx_ext);
	DEBUG_1("write_pvid routine rc: %d\n",rc)
	return(rc);
}

int
putpvidattr(lname,pvidstr)
char	*lname;			/* Device's logical name	*/
char	*pvidstr;		/* ASCII PVID string extracted	*/
{
struct	CuAt *pvidattr;
int	cnt;

	/* get pvid attribute stuff to serve as a template */
	if ((pvidattr=getattr(lname,"pvid",FALSE,&cnt))==NULL) {
		DEBUG_1("putpvidattr: didn't get pvid attr for %s\n",
			lname)
		return(-1);
	}

	/* put pvid in this structure */
	if (*pvidstr) {
		/* If not a NULL character then use the input string */
		strcpy(pvidattr->value,pvidstr);
	} else {
		/* Make sure we are writing default value */
		strcpy(pvidattr->value,"none");
	}

	/* put new object into database */
	if (putattr(pvidattr)==-1) {
		DEBUG_1("putpvidattr: failed to put pvid attr for %s\n",
			lname)
		return(-1);
	}

	return(0);
}

int
clear_pvid(lname,openx_ext)
char	*lname;			/* Device's logical name	*/
int     openx_ext;		/* extended parmeter for openx	*/
{
struct unique_id unique_id;		/* pvid structure */

	bzero((caddr_t)&unique_id, sizeof (struct unique_id));
	return(write_pvid(lname, unique_id,openx_ext));
}

/***********************************************************************
 *                                                                     *
 * NAME:  mkuuid                                                       *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine returns a unique id.                                 *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   INPUT:                                                            *
 *     unique_id                                                       *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     *unique_id                                                      *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value = 0 indicates successful return.                   *
 *                                                                     *
 * EXTERNAL PROCEDURES CALLED:                                         *
 *                                                                     *
 ***********************************************************************/
int mkuuid (unique_id)
struct unique_id *unique_id;		/* where unique id is to be stored */
{
struct	timestruc_t cur_time;
struct	utsname uname_buf;
long	machine_id;
int	retcode;

/***********************************************************************
 *   For this release, only two words of the unique id structure will  *
 *   be used.  Words 3 and 4 will contain zeroes for this release.     *
 *   Word 1 is generated from the current time in seconds.  Word 2     *
 *   is generated from the machine id.                                 *
 ***********************************************************************/


	bzero((caddr_t) unique_id, sizeof (struct unique_id));

	retcode = gettimer (TIMEOFDAY, &cur_time);

	if (retcode == -1)		
		return (retcode);

	retcode = uname (&uname_buf);

	if (retcode == -1)
		return (retcode);

	/* initialize machine id to 0 */
	machine_id = 0;

	/* convert character machine id to an integer */
	retcode = sscanf (uname_buf.machine, "%8X", &machine_id);

	/* store the machine id into the first word of the unique id */
	unique_id -> word1 = machine_id;

	/* store the current time in milliseconds (?) into the second word of
	  the unique id */
	unique_id -> word2 = cur_time.tv_sec*1000+cur_time.tv_nsec/1000000;
	return (0);

}



/***********************************************************************
 *                                                                     *
 * NAME:  write_pvid                                                   *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine writes the pvid to the bootrecord on PSN 1	       *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   If the openx_ext flag is non zero, then a special openx using     *
 *   openx_ext as the extended parameter will be used instead of       *
 *   of a normal open.						       *
 *								       *
 *   INPUT:                                                            *
 *     unique_id, lname, openx_ext                                     *
 *                                                                     *
 *     								       *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value = 0 indicates successful return.                   *
 *                                                                     *
 * EXTERNAL PROCEDURES CALLED:                                         *
 *                                                                     *
 ***********************************************************************/
int write_pvid (	char	*lname,
			struct	unique_id pvid,
			int	openx_ext)
{
char	bdevice[64];		/* special filename for device		*/
int	fd;			/* file descriptor for physical vol 	*/
int	rc;			/* function call return code 		*/
static	IPL_REC clearipl;	/* clears IPL record (static) 		*/
static	IPL_REC ipl_rec;	/* IPL (boot record) 			*/
off_t	offset;



	/*
	 *
	 * Read boot record from given disk.
	 *
	 */
	sprintf(bdevice,"/dev/r%s",lname);
	DEBUG_1("write_pvid: opening device: %s\n",bdevice)
	if (openx_ext) {

		/*
		 * If a non zero openx ext is passed in, then
		 * use the special openx instead of a normal
		 * open.
		 */

		fd = openx(bdevice, O_RDWR, 0, openx_ext); 
	}
 	else {
		fd = open(bdevice, O_RDWR);
	}

       	if (fd < 0) {

		if (errno == EBUSY) {
			return (E_BUSY);
		}
		else {
			return (E_DEVACCESS);
		}
	} 
	DEBUG_0("write_pvid: lseek\n")
	offset = lseek(fd, PSN_IPL_REC, 0);

       	if (offset < 0) {
		return(E_DEVACCESS);
	}

	DEBUG_0("write_pvid: read of ipl record\n")
       	rc = read(fd, (char *) &ipl_rec, sizeof(ipl_rec));

       	if (rc < 0) {
		return(E_DEVACCESS);
	}


	DEBUG_1("ipl rec id: %x\n", ipl_rec.IPL_record_id)
	/*
	 *
	 * Update boot record with given pvid.
	 */
	if (ipl_rec.IPL_record_id != (unsigned int) IPLRECID) {
		/*
		 * Boot record does not exist on disk yet.
		 */
		ipl_rec = clearipl;
		ipl_rec.IPL_record_id = IPLRECID;
	}
	ipl_rec.pv_id = pvid;
	DEBUG_1("write_pvid: ipl pv_id: %x\n", ipl_rec.pv_id)


	/*
 	 *
	 * Write IPL record to boot image file.
 	 *
	 */

	DEBUG_0("write_pvid: lseek\n")
	offset = lseek(fd, PSN_IPL_REC, 0);
       	if (offset < 0) {
		return(E_DEVACCESS);
	}

       	rc = write(fd, (char *) &ipl_rec, sizeof(ipl_rec));

       	if (rc < 0) {
		return(E_DEVACCESS);
	}
	close(fd);
	return(0);
}

/***********************************************************************
 *                                                                     *
 * NAME:  read_pvid                                                    *
 *                                                                     *
 * FUNCTION:                                                           *
 *   This routine reads the pvid from the bootrecord on PSN 1.	       *
 *   If the disk does not contain a valid bootrecord then	       *
 *   pvidstr will be set to NULLPVID				       *
 *                                                                     *
 * NOTES:                                                              *
 *                                                                     *
 *   If the openx_ext flag is non zero, then a special openx using     *
 *   openx_ext as the extended parameter will be used instead of       *
 *   of a normal open.						       *
 *								       *
 *   INPUT:                                                            *
 *     lname, openx_ext                                   	       *
 *                                                                     *
 *   OUTPUT:                                                           *
 *     pvidstr                                                         *
 *                                                                     *
 * RETURN VALUE DESCRIPTION:                                           *
 *   A return value = 0 indicates successful return.                   *
 *                                                                     *
 * EXTERNAL PROCEDURES CALLED:                                         *
 *                                                                     *
 ***********************************************************************/
int read_pvid (		char	*lname,
			char	*pvidstr,
			int	openx_ext)
{
char	bdevice[64];		/* special filename for device		*/
int	fd;			/* file descriptor for physical vol 	*/
int	rc;			/* function call return code 		*/
static	IPL_REC ipl_rec;	/* IPL (boot record) 			*/
off_t	offset;



	/*
	 *
	 * Read boot record from given disk.
	 *
	 */
	sprintf(bdevice,"/dev/r%s",lname);
	DEBUG_1("read_pvid: opening device: %s\n",bdevice)

	if (openx_ext) {
		/*
		 * If a non zero openx ext is passed in, then
		 * use the special openx instead of a normal
		 * open.
		 */
		fd = openx(bdevice,O_RDONLY, 0, openx_ext); 
	}
 	else {
		fd = open(bdevice, O_RDONLY);
	}

       	if (fd < 0) {
		return(E_DEVACCESS);
	}

	DEBUG_0("read_pvid: lseek\n")

	offset = lseek(fd, PSN_IPL_REC, 0);
       	if (offset < 0) {
		return(E_DEVACCESS);
	}

	DEBUG_0("read_pvid: read of ipl record\n")

       	rc = read(fd, (char *) &ipl_rec, sizeof(ipl_rec));
       	if (rc < 0) {
		return(E_DEVACCESS);
	}

	DEBUG_1("ipl rec id: %x\n", ipl_rec.IPL_record_id)

	if (ipl_rec.IPL_record_id != (unsigned int) IPLRECID) {
		/*
		 * Boot record does not exist on device.
		 */
		strcpy(pvidstr,NULLPVID);

	}
	else {
		/*
		 * Device does have a valid boot
		 * record, so let's extract the
		 * PVID.
		 */
		strcpy(pvidstr,pvidtoa(&(ipl_rec.pv_id)));

	}

	DEBUG_1("read_pvid: ipl pv_id: %s\n", *pvidstr)


	close(fd);
	return(0);
}
