static char sccsid[] = "@(#)27  1.5  src/bos/usr/bin/copydump/copydump.c, cmddump, bos411, 9428A410j 4/22/94 11:47:33";
/*
 * COMPONENT_NAME: CMDDUMP
 *
 * FUNCTIONS: copydump
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/dump.h>
#include <sys/ioctl.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <sys/vmount.h>
#include <BosMenus_msg.h>
#include "copydump.h"

extern int errno;
extern int odmerrno;

static catd;
#define MSGSTR(n,s) catgets(catd,MS_COPYDUMP,n,s)


char buf[BUFSIZ];
/*
 * NAME: main copydump
 *
 * FUNCTION:
 *	Copy a dump from a logical volume to an external writable 
 *	and removable backup media.
 *
 * INPUT:
 *		dump device name
 *		dump size
 * 		location code
 *		output device
 *		[message flag] (optional, if not exists assume FALSE)
 * 
 * DEPENDENCIES:
 *		importvg
 *		varyonvg
 *		mkdev
 *		dd
 *		pax 
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is called only from rc.boot and bosinstall
 *	Message flag is used to indicate that utility is called
 *	from bosinst. In maintenance environment, it's not guaranteed that
 *	other catalogs are available but the bosinst catalog. Hence, this
 *	utility will put its messages in the bosinst catalog and use
 *	these messages if it's invoked from the maintenance menu.
 *
 */

main(argc,argv)
int argc; 
char *argv[];
{
	char *devicename;
	int dumpsize;
	char *location_code;
	char *outputdevice;
	int bosinst_flag;
	int first_time = TRUE;	  
        struct CuDv *pcudv;
        struct CuDv *pcudv2;
	struct listinfo listinfo;
	int rc;
	int count;
	int fd;
	int word;
 
	
	setlocale(LC_ALL,"");
	catd = catopen(MF_BOSMENUS, NL_CAT_LOCALE);
	/*********************************************************
	  check the number of input arguments.  
	*********************************************************/
	if (argc < 5)
	{
/*
		fprintf(stderr,MSGSTR(COPYDUMP_USAGE,
		"Usage: copydump dumpdevicename dumpsize locationcode outputdevice [bosinst_flag]\n"));
*/
		return(E_USAGE);
	}
	else
	{
	/*********************************************************
	  Get the arguments.
	*********************************************************/
		devicename = argv[1];
		dumpsize = atoi(argv[2]);
		location_code = argv[3];
		outputdevice = argv[4];	
		if ( atoi(argv[5]) )
		  {
		  bosinst_flag = atoi(argv[5]);

		/*********************************************
		  We need to check if this is the first time 
		  that copydump is called from copydumpmenu
		  in the bosinstall environment.  If so, then
		  import the volume group and vary it on.  Otherwise,
		  it's already been done.
	        **********************************************/

		  if ( atoi(argv[6]) )
			{
		  	first_time = FALSE; 
			}
		  else
			{
			first_time = TRUE;
			}
		  }
		else bosinst_flag = FALSE;
	}
	
	/*********************************************************
	   Initialization for accessing the ODM data base 
	*********************************************************/

        if (0 > odm_initialize()) 
	{
			return(E_ODMINIT);
	}

	/*******************************************************
	  Only do the following if it's the first time we are 
	  called from the bosinstall environment.
	*********************************************************/

	if (bosinst_flag && first_time)
	{
	  /*********************************************************
	     Get the name of the physical volume from the location
	     code.
	  *********************************************************/

	  sprintf(buf,"location = '%s'", location_code);
	  pcudv = odm_get_list(CuDv_CLASS,buf,&listinfo,1,1);
	  if ( !pcudv )
		{
		   return(E_ODMGET);
		}

  	  /*********************************************************
	     Import the rootvg
	  *********************************************************/
	  /* Note: do we want to redirect any output of the
	   *	command to /dev/null
	  */
	  sprintf(buf,"importvg -y rootvg %s >/dev/null 2>&1",pcudv->name);
	  if ( rc = system(buf))
	  {
		cleanup_and_exit(E_IMPORTVG);
	  }
	
	  /*********************************************************
	     Varyon the volume group for accessibility.
	  *********************************************************/
	  /* Note: do we want to redirect any output of the
	   *	command to /dev/null
	  */

	  sprintf(buf,"varyonvg -n rootvg >/dev/null 2>&1");
	  if ( rc = system(buf))
	  {
		cleanup_and_exit(E_VARYONVG);
	  }
	}

	/*********************************************************
	     Check for valid dump in the dump device.
	*********************************************************/

	fd = open(devicename,O_RDONLY);
	if ( fd < 0 )
	  {
		cleanup_and_exit(E_OPEN);
	  }
	if ( rc = lseek(fd,0,0) < 0 )
	  {
		close(fd);
		cleanup_and_exit(E_LSEEK);
	  }
	if ( rc = read(fd,(char *)&word,sizeof(word)) < 0 )
	  {
		close(fd);
		cleanup_and_exit(E_READ);
	  }
	if (word != DMP_MAGIC)
	  {
		if (rc = lseek(fd,512,0) < 0)
		{
		    close(fd);
		    cleanup_and_exit(E_LSEEK);
		}
		if ( rc = read(fd,(char *)&word,sizeof(word)) < 0)
		{
		    close(fd);
		    cleanup_and_exit(E_READ);
		}
		if ( word != DMP_MAGIC)
		{	
		    cleanup_and_exit(E_INVALID_DUMP);
		}
	  }
	/*********************************************************
	  Config the output device if not available
	*********************************************************/
	sprintf(buf,"name = '%s'",&(outputdevice[5]));
	pcudv2 = odm_get_list(CuDv_CLASS,buf,&listinfo,1,1);
	if (!pcudv2)
	{
		cleanup_and_exit(E_GET_DEVNAME);
	}
	if ( pcudv2->status == DEFINED)  /* need to config */
	{
	/* mkdev -R is not supported until feature 99833 is implemented*/
		sprintf(buf,"mkdev -l %s -R >/dev/null 2>&1",pcudv2->name);
		rc = system(buf);
		if (rc != 0)
			cleanup_and_exit(E_MKDEV);
	}
	/*********************************************************
	  Set up the command to copy the dump
	*********************************************************/
	count = ( (dumpsize  % (36 *512)) ? ((dumpsize / (36 * 512)) + 1) :
					  (dumpsize / (36 * 512)));
	sprintf(buf,"/usr/bin/dd if=%s  bs=36b count=%d | /usr/bin/pax -wf %s -o datastream=%s,datastr_size=%d",
			devicename,count,outputdevice,"dump_file",dumpsize);

	if (rc = system(buf))
	{
		cleanup_and_exit(E_COPY);
	}
	else cleanup_and_exit(0);
}
static
cleanup_and_exit(errorcode)
int errorcode;
{
	odm_terminate();
	exit(errorcode);
}
