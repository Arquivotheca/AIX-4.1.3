static char sccsid[] = "@(#)31        1.6  src/bos/usr/bin/errlg/liberrlg/buffer.c, cmderrlg, bos41J, 9507C 2/2/95 18:03:39";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1992, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/param.h>
#include <sys/cfgodm.h>
#include <sys/erec.h>
#include <cmderrlg_msg.h>
#include <errlg.h>
#include <values.h>
#include <errlg/SWservAt.h>
#include <errno.h>


extern	void log_odm_error();
extern  int Errctlfd;		 /* /dev/errorctl file descriptor */
extern	char Logfile[PATH_MAX];		 /* the error log file name */ 
extern  int Threshold; 	   	 /* the error log file size */
extern  int Buffer;   		 /* the error log memory buffer size from the device driver */
extern  int odm_buffer;		 /* the error log memory buffer size in ODM */

/*
 * NAME: set_buffer
 *
 * FUNCTION: This function sets the errlog memory buffer size.  It takes
 *	     a size value, and sets it according to the following criteria.
 *	     The size given must be a minimum of 8256 bytes.  This is enough
 *	     room for 32 maximum size error records.  If the size is the same
 *	     as the current buffer size, nothing is done.  If the size is less
 *	     than the current buffer size, that size is stored in ODM and 
 *	     it will become effective upon the next system reboot.  If the size
 *	     is greater than the current buffer size, then the buffer size
 *	     is changed to the new size via an ioctl call and the new size
 *	     is also stored in ODM.	
 *
 * INPUTS:
 *	     size	: the new buffer size 
 *
 * RETURNS:
 *           0 		: success
 *	     2		: new size same as current size
 *			  new size less than current size
 *		          (This indicates not to restart errdemon.) 
 */

set_buffer(size)
char *size;
{
int new_size;	/* new buffer size */

	new_size = atoi(size);

	if (new_size < BUFFER_MINIMUM)	/* less than the minimum size */
		cat_fatal(CAT_BUF_BADSIZE,
		"The value %d is not a valid error log memory buffer size value.\n\
This value must be a minimum of %d bytes.\n", new_size, BUFFER_MINIMUM);

	/* Make the size a multiple of PAGESIZE */
	new_size = ((new_size/PAGESIZE) + ((new_size%PAGESIZE != 0)*1))*PAGESIZE;

	if (Buffer == new_size)	 	/* same as current size */
		{
		set_odm_buffer(new_size); /* make sure ODM is consistent with current buffer size */
		return (2);
		}	

	if (new_size < Buffer)		/* less than current size */
		{
		if (new_size > 0)       /* make sure size is not negative (no overflow) */
			{
			set_odm_buffer(new_size);	
			cat_print(CAT_BUF_LESS,
			"The value %d for the error log memory buffer size will not take affect\n\
until the next system reboot.\n",new_size);
			return (2);
			}
		else			/* there was overflow  */
			cat_fatal(CAT_BUF_OVERFLOW,
			"The value specified for the error log memory buffer size is too large.\n\
The maximum value allowed is %d bytes.\n", MAXINT);
		}
					
	/* open /dev/errorctl */
	if ((Errctlfd = open("/dev/errorctl",0)) < 0)
		perror("/dev/errorctl");

	if (ioctl(Errctlfd,ERRIOC_BUFSET,new_size) < 0) /* greater than current size */
		cat_fatal(CAT_BUF_NOMEM,	     /* not enough memory */	
		"Cannot set error log memory buffer size to %d, because there is\n\
not enough memory available.\n", new_size);
	else
		{
		set_odm_buffer(new_size);
		Buffer = new_size;
		errstop();	/* stop the demon */
		sleep(3);	/* wait for the child to die */
		}

	close(Errctlfd);
	return (0);
}

/*
 * NAME: set_odm_buffer
 *
 * FUNCTION: This function takes a buffer size and puts it into ODM.
 *	     It calls ras_getattr which returns a pointer to the  
 *	     attribute "errlg_buf" which holds the errlog memory buffer
 *	     size.  Then ras_putattr is called to put the new value into
 *	     ODM.  A savebase is done to insure the data is saved across
 *	     a boot.  
 *
 * INPUTS:   size	: the new ODM buffer size	
 *
 * RETURNS:  none	
 *
 */

set_odm_buffer(size)
int size;
{
int number;
struct SWservAt *swservp;
char tmp_size[PATH_MAX];
int sverr;

	if (odm_initialize() < 0)
	{
		sverr = errno;
		log_odm_error(ERRLG_BUF,ODM_INIT,sverr);
		cat_fatal(CAT_ODM_INIT, "Cannot initialize ODM.\n");
	}
	odm_set_path(ODM_DATABASE);

	swservp = ras_getattr("errlg_buf", 0, &number);

	if (swservp == NULL)
	{
		sverr = errno;
		log_odm_error(ERRLG_BUF,ODM_READ,sverr);
		cat_fatal(CAT_BUF_GETATTR,
			"Unable to get the error log memory buffer size from the ODM object class SWservAt.\n"); 
	}

	sprintf(tmp_size,"%d",size);
	swservp->value = tmp_size;

	if (ras_putattr(swservp))
	{
		sverr = errno;
		log_odm_error(ERRLG_BUF,ODM_WRITE,sverr);
		cat_fatal(CAT_BUF_PUTATTR,
			"Unable to put the error log memroy buffer size in the ODM object class SWservAt.\n");
	}

	if (odm_terminate() < 0)
	{
		sverr = errno;
		log_odm_error(ERRLG_BUF,ODM_TERM,sverr);
		cat_fatal(CAT_ODM_TERM, "Unable to terminate ODM.\n");
	}

	free(swservp);
	savebase();	/* save data across a boot */
}

/*
 * NAME: get_buffer
 *
 * FUNCTION: This function gets the errlog memory buffer size used 
 *	     by the errlog device driver. 
 *
 * INPUTS:   none
 *
 * RETURNS:  none
 *
 */

get_buffer()
{
struct SWservAt *swservp;
int bufsize;

	if((Errctlfd = open("/dev/errorctl",0)) < 0)
		perror("/dev/errorctl");

   	if (ioctl(Errctlfd,ERRIOC_BUFSTAT,&bufsize) < 0)
		perror("ERRIOC_BUFSTAT");	

 	Buffer = bufsize;
	close(Errctlfd);
}

/*
 * NAME: get_odm_buffer
 *
 * FUNCTION: This function gets the errlog memory buffer size from
 *	     ODM. It calls ras_getattr with the attribute value of
 *	     "errlg_buf".  If the attribute is not found in the ODM
 *	     database, the default errlog memory buffer size is used.
 *
 * INPUTS:   none
 *
 * RETURNS:  none
 */

get_odm_buffer()
{
struct SWservAt *swservp;
int number;

	if (odm_initialize() < 0)
		{
		cat_eprint(CAT_ODM_INIT, "Cannot initialize ODM.\n");
		odm_buffer = BUFFER_DFLT;
		return;
		}
	odm_set_path(ODM_DATABASE);

	swservp = ras_getattr("errlg_buf", 0, &number);

	if (swservp == NULL)
		odm_buffer = BUFFER_DFLT;
	else
		{
		odm_buffer = atoi(swservp->value);
		free(swservp);
		}

	if (odm_terminate() < 0)
		cat_eprint(CAT_ODM_TERM, "Unable to terminate ODM.\n");

}

/*
 * NAME: show_log_attr
 * 
 * FUNCTION: This function shows the ODM values of the errlog
 *	     file name, file size, and memory buffer size.  These 
 *	     values are set by errdemon. 
 *
 * INPUTS:   none
 *
 * RETURNS:  none
 */

show_log_attr()
{

	cat_print(CAT_SHOW_ATTR,
	"Error Log Attributes\n---------------------------------------------\n\
Log File          \t%s\n\
Log Size          \t%d bytes\n\
Memory Buffer Size\t%d bytes\n", Logfile, Threshold, odm_buffer);

}

/*
 * NAME: set_initial_buffer
 *
 * FUNCTION: This function sets the initial errlog memory buffer size.
 *	     This buffer size is set to the default (8256 bytes) upon
 *	     boot.  When errdemon is started, the ODM buffer size value
 *	     is checked to see if it is greater than the default.  If
 *	     it is, then this routine is called to reallocate the errlog
 *	     buffer to the size specified in ODM.  This function should
 *	     only be called a maximum of once after each boot.  
 *
 * INPUTS:   size	: buffer size from ODM	
 *
 * RETURNS:  none
 */

set_initial_buffer(size)
int size;
{
	/* Make the size a multiple of EREC_MAX */
	size = ((size/EREC_MAX) + ((size%EREC_MAX != 0)*1))*EREC_MAX;

	/* open /dev/errorctl */
	if ((Errctlfd = open("/dev/errorctl",0)) < 0)
		perror("/dev/errorctl");

	/* set the current buffer size to the ODM value */ 
	if (ioctl(Errctlfd,ERRIOC_BUFSET,size) < 0) 
		cat_fatal(CAT_BUF_NOMEM,	/* not enough memory */  
		"Cannot set error log memory buffer size to %d, because there is\n\
not enough memory available.\n", size);
	else				/* everything's ok, stop the demon */			
		{
		errstop();	
		sleep(3);	/* wait for child to die */
		}

}

