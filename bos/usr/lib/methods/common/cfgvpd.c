static char sccsid[] = "@(#)61  1.2  src/bos/usr/lib/methods/common/cfgvpd.c, cfgmethods, bos411, 9428A410j 5/19/94 18:51:48";
/*
 *   COMPONENT_NAME: CFGMETHODS
 *
 *   FUNCTIONS: get_vpd
 *		get_sub_vpd
 *		b_index
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <errno.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/iplcb.h>
#include <sys/sysconfig.h>
#include <sys/rosinfo.h>
#include <sys/systemcfg.h>
#include <sys/mdio.h>
#include <sys/device.h>
#include <cf.h>
#include "cfgdebug.h"
#include <string.h>

/*
 *---------------------------------------------------------
 * Forward function definitions
 *---------------------------------------------------------
 */
int get_vpd(char*, char*); 
int get_sub_vpd(char*, char*, char*);
int b_index(char*, char*, int*);

/*^L
 * NAME: get_vpd
 *
 * FUNCTION: This routine obtains the vpd from the iplcb 
 *	     or from the pos regs.
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is called from cfgsio, cfghscsi, cfgpscsi and others.
 *
 * RETURNS:
 *   0        - no ERROR occured while obtaining sub vpd data
 *   E_VPD    - ERROR occured while obtaining sub vpd data
 */

int get_vpd(ret_vpd, key)
char *ret_vpd;				/* return vpd buffer		*/
char *key; 				/* search key in vpd buffer	*/ 
{

        IPL_DIRECTORY   iplcb_dir;      /* IPL control block directory  */
        MACH_DD_IO      mdd,            /* machdd ioctl access struct   */
			read_record,
			write_record;
        int     	rc;
        int     	fd;
	int		vpd_index;
	char		vpd_data[256];
	uchar		tmpbyte;
	int		len;

	/* if power pc, obtain vpd data from iplcb */
	if (__power_pc() ) {

	        if ((fd = open("/dev/nvram",0)) < 0)  {
        	        DEBUG_0("Unable to open /dev/nvram")
                	return(E_VPD);
        	}

 	       /*
        	* Get the IPL CB dir - so we can access other parts of IPLCB
        	*/
        	mdd.md_addr = 128;
        	mdd.md_data = (uchar *)&iplcb_dir;
        	mdd.md_size = sizeof(iplcb_dir);
        	mdd.md_incr = MV_BYTE;

        	DEBUG_0("Calling mdd ioctl for iplcb_dir\n")
        	if (ioctl(fd, MIOIPLCB, &mdd)) {
            		DEBUG_0("Error reading IPL-Ctrl block directory")
           	 	return(E_VPD);
        	}

       		/*
        	* Get the system vpd section
        	*/
		/* + 1 byte to get past reserved byte */
        	mdd.md_addr = iplcb_dir.system_vpd_offset+1;
        	mdd.md_data = (uchar *)vpd_data;
        	mdd.md_size = 256;
        	mdd.md_incr = MV_BYTE;

        	DEBUG_0("Calling mdd ioctl for sysvpd\n")
        	if (ioctl(fd, MIOIPLCB, &mdd)) {
            		DEBUG_0("Error reading IPL-Ctrl block sysvpd section")
            		return(E_VPD);
       	 	}
        	close(fd);

	}
	else {  /* obtain vpd through pos regs */ 

	        if( (fd = open( "/dev/bus0", O_RDWR )) < 0 )
        	        return E_OPEN;

        	/* write 0x01 - 0xFF to pos reg 6, slot F */
        	/* read related vpd byte back from pos reg 3 slot F */

        	write_record.md_size = 1;       /* write 1 byte */
        	write_record.md_incr = MV_BYTE;
        	write_record.md_data = &tmpbyte;
       	 	write_record.md_addr = POSREG(6,0x0f);

       		read_record.md_size = 1;        /* Transfer 1 byte */
        	read_record.md_incr = MV_BYTE;
        	read_record.md_data = &tmpbyte;
        	read_record.md_addr = POSREG(3,0x0f);

        	for (vpd_index = 0x01; vpd_index <= 0xFF; vpd_index++) {
                	tmpbyte = (uchar)vpd_index;

                	/* write to pos reg 6 */
                	if( ioctl( fd, MIOCCPUT, &write_record ) < 0 ) {
                        	close(fd);
                        	return (E_VPD);
                	}

                	/* read from pos reg 3 */
                	if( ioctl( fd, MIOCCGET, &read_record ) < 0 ) {
                        	close(fd);
                        	return (E_VPD);
                	}

                	vpd_data[vpd_index-1] = tmpbyte;
        	}

        	close(fd);
        }

 
	/* for type=sio models, get all vpd data */
	if ( !strcmp(key,"ALL") ) {
		put_vpd (ret_vpd, vpd_data, VPDSIZE-1 );
		return (0); 
	}

    	len = (vpd_data[3]*256 + vpd_data[4]) *2;
	vpd_data[len+7] = NULL;    /* set char after last to NULL */

	/* look for key in vpd data */
	rc = get_sub_vpd (key, vpd_data, ret_vpd );
	if (rc) {
		/* sub vpd for key was not found */
		return (E_VPD);
	}
	return (0) ;

} /* end get_iplcb_vpd */


/*^L
 *
 * NAME: get_sub_vpd
 *
 * FUNCTION: Parses specified subdevice VPD and return to caller.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine will be called from get_vpd routine.
 *
 * NOTES:
 *
 *   get_sub_vpd(key, vpdptr, vpdlen, retvpd)
 *      key    - INPUT : keyword to search for in the *DS field to
 *                       identify the beginning of the subdevice VPD.
 *      vpdptr- INPUT: address of all VPD data
 *      retvpd- OUTPUT: address of return buffer - subset of VPD
 *	
 *	If the key is "SIO" & not powerpc then we must copy all of 
 *      the global fields at the beginning of the VPD (up to the 
 *      first *DS) AND the SIO specific data.
 *
 * RETURNS:
 *      0         : Successfully found the VPD.
 *      1         : unable to find subset VPD in all vpd
 */

int get_sub_vpd(key, vpdptr, retvpd)
char    *key;
char    *vpdptr;
char	*retvpd;
{
    char    *temp;
    char    *vpd_start;
    char    *begin_vpd;			/* ptr to first * in vpd    */
    char     found;
    int      len;
    int      rc;
    char     keyword1[32];		/* first possible keyword   */
    char     keyword2[32];		/* second possible keyword  */

   /*
    *-----------------------------------------------------------------
    * Parse the requested part.
    *
    * Loop through the VPD by accessing each VPD field.  Each field
    * has the following format :
    *     offset 0 : *          1 character field marker ('*')
    *     offset 1 : cc         2 character field Id keyword
    *     offset 3 : n          1 byte length of field in half words
    *                                   (2 byte pairs).  NOTE the
    *                                   length includes the field
    *                                   identifier section (*ccn)
    *     offset 4 : data       VPD field data (variable length).
    *
    * Search each *DS field for the specified Key.
    * Once found, skip to next *DS.  The area between the *DS field
    *      identifiers is the requested subdevice VPD.
    *-----------------------------------------------------------------
    */

    /* possible keywords to search for in the VPD  */

    if (!strcmp(key,"SIO") )  {
    	strcpy(keyword1,"SIO");	
    	strcpy(keyword2,"STANDARD I/O");	
    }
    else if (!strcmp(key,"ETHERNET") ) {
	strcpy(keyword1,"ETHERNET");
	strcpy(keyword2,"STANDARD ETHERNET");
    }
    else if (!strcmp(key,"SCSI") ) {
	strcpy(keyword1,"SCSI");
	strcpy(keyword2,"STANDARD SCSI");
    }
    else {
        len=strlen(key) > sizeof(keyword1) ? sizeof(keyword1) : strlen(key);

	strncpy(keyword1,key,len);
	strncpy(keyword2,key,len);
    }

    temp = vpdptr+7;				/* skip past vpd header */

    found = FALSE;
    begin_vpd = temp;			

    while ((temp[0] == '*') ) {
	len = temp[3] << 1 ;
	if (!bcmp(temp, "*DS", 3)) {
		if (!found) {
		   if ((found = (b_index(temp, keyword1, len) != NULL)) ||
		       (found = (b_index(temp, keyword2, len) != NULL) )){

		       		vpd_start = temp;
		   }
		}
		else { /* got VPD addr & now have end addr; so we're DONE */
			break;
		}
       	}
       	temp += len;
    }/* end while loop */
   /*
    *------------------------------------------------------------
    * If we found the keyword, copy the VPD to the caller's
    * buffer and set the length.  Otherwise set length to 0;
    * indicating the subdevice VPD was not found.
    *------------------------------------------------------------
    */

    if (found) {

	/* if key = SIO and not powerpc	*/
    	if (!strcmp(key,"SIO")  &&  !(__power_pc()) ) {

		len = temp - begin_vpd;
		bcopy(begin_vpd, retvpd, len);
	}
	else {	
        	len = temp - vpd_start;
        	bcopy(vpd_start, retvpd, len);
	}
        rc = 0;
    }
    else {
        rc = 1;
    }

    return(rc);

} /* end get_sub_VPD */



/*
 *======================================================================
 * NAME: b_index
 *
 * FUNCTION: Searches the byte string Src for the null terminated
 *           string Target.  Len indicates the length of Src.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is to be used only within this file.
 *
 * NOTES:
 *
 * int
 *   b_index(Src, Target, Len)
 *      Src    - INPUT: Pointer to source byte string.
 *      Target - INPUT: Pointer to null terminated string to search
 *                      for within Src.
 *      Len    - INPUT: Length in bytes of the source string Src.
 *
 * RETURNS:
 *      NULL : Could not find Target in Src.
 *      addr : Pointer to the address of the beginning of the Target
 *                string in Src.
 *======================================================================
 */

int b_index(src, target, len)
    char    *src;
    char    *target;
    int     *len;

{
    int      slen;   /* length of Target string                      */
    int      i;

    i = 0;
    slen = strlen(target);
    while ((len >= i + slen) && bcmp(&src[i], target, slen))
        i++;

    if (len < i + slen)          /* if Target was NOT found     */
        return((char *)NULL);
    else
        return(&src[i]);
} /* END b_index */


