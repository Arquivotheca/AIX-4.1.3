static char sccsid[] = "@(#)36  1.5  src/bos/diag/tu/msla/busopen.c, tu_msla, bos41J, 9515A_all 3/29/95 16:39:09";
/*
 * COMPONENT_NAME: ( busopen ) 
 *
 * FUNCTIONS: bus_mem_open, die 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mdio.h>
#include <sys/sysconfig.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/device.h>

#include "mslafdef.h"
#include "mslaerrdef.h"
#include "msla_diag.h"
#include "msladgappl.h"


#define DEV_NAME "gsw"
/*
*****************************************************************************
*                                                                           *
*      MODULE NAME :  bus_mem_open                                          *
*                                                                           *
*         PURPOSE :  Open the system bus memory for load & store operation  *
*                     Takes care of mc i.e RTRIOS                           *
*                     Returns memory base and iobase for the system.        *
*                     To satisfy ourselves the return code of open bus call *
*                      is tested. Then the io-base and mem-base is set and  *
*                      tested for valid operations.                         *
*                     The updated io-address and mem-address for RTRIOS     *
*                      is returned.                                         *
*                      A SUCCESS return ensures start of actual testing     *
*                                                                           *
*       ENVIRONMENT:  POS registers may not be initialized prior to call    *
*                                                                           *
*            INPUT :  devname,gm_base,gio_base                              *
*                                                                           *
*           EXAMPLE:                                                        *
*                     fd = bus_mem_open(devname,&gm_base,&gio_base,slot);   *
*                                                                           *
*           OUTPUT :  Returns the proper code.				    *
*                                                                           *
* FUNCTIONS CALLED :  openx, ioctl, perror				    *
*                                                                           *
*           STATUS :  Release 1 Version 0                                   *
*                                                                           *
*                                                                           *
*****************************************************************************
*/
 
void   die();
static int file_des;			      /*  Global for DIE routine    */
 
int bus_mem_open(devname,gm_base,gio_base)
char    *devname;
ulong   *gm_base;                       /* msla memory-base address     */
ulong   *gio_base;                      /* msla io-base address         */
{
 
   int      rc;
   int      fd;
   struct   msla_map  mslamap;
   struct   opnparms  opnx;
   void get_vpd();

/* 
 * Get VPD using the sysconfig call.This has to be done before opening
 *  the device for the sysconfig() call to work.
 */

 /* devname should be /dev/hiax if /dev/gsw0 .  If gsw, we should
    make the sysconfig call now.  hia however requires call to be
    made after the driver is opened */

  if (devname[5] == 'g')
      get_vpd(devname);

   /* open the system bus */
   opnx.diag_mode = TRUE;

   /* Signals for interrupt handler */
    signal(SIGTERM,die);
    signal(SIGIOT,die);
    signal(SIGQUIT,die);
    signal(SIGINT,die);

   
    file_des = openx(devname, O_RDWR, 0,&opnx);
    if (file_des == -1)
    {
    #ifdef PRTF
	printf("open FAILURE. errno = %d\n",errno);
    #endif
	return(-1);
    }
    fd = file_des; 
 
    
   /* Get MSLA Mem & IO base address */
    rc = ioctl (fd, MSLA_GET_ADDR, &mslamap);
    if(rc < 0)
    {
    #ifdef PRTF
	printf("MSLA Gets address Failed\n");
    #endif
	return(rc);
    }

    *gm_base = mslamap.msla_mem_start;
    *gio_base = mslamap.msla_io_start;

    rc = ioctl (fd, MSLA_MOD_POS, MP_ENA_CARD);
    if(rc < 0)
    {
    #ifdef PRTF
  	printf("MSLA card enable failed. RC: %d\n",rc);
    #endif
	return(rc);
    }

  if (devname[5] == 'h')   /* if /dev/hia- */
    get_vpd(devname);
 
    return(file_des);
}

/*
*****************************************************************************
*                                                                           *
*      MODULE NAME :  die                                                   *
*                                                                           *
*         PURPOSE :  Routine to be called at the end of the program  or     *
*                                                                           *
*                     in the event of an interrupt. This cleans everything. *
*                                                                           *
*            INPUT :  Uses the global variable of the file descriptor       *
*		      open above.					    * 
*                                                                           *
*           OUTPUT :  None.						    *
*                                                                           *
*                                                                           *
*****************************************************************************
*/

void die()
{

    signal(SIGTERM,SIG_IGN);
    signal(SIGIOT,SIG_IGN);
    signal(SIGQUIT,SIG_IGN);
    signal(SIGINT,SIG_IGN);
    closing();
    exit(0);
}

int
closing()
{
    int ucfd, rc;

    extern int dma_in_progress; /* TMA */

    #ifdef PRTF
      printf("\n\nCleaning up ....... ");
    #endif 
    rc = ioctl(file_des,MSLA_RET_ADDR);
    if(rc < 0)
    {
	printf("IOCTL RET ADDR FAILED\n");
    }
    
    rc = ioctl(file_des,MSLA_LOAD_UCODE);
    if(rc < 0)
    {
	printf("LOAD_UCODE ioctl failed\n");
	return(-1);
    }

/* TMA */
    if (dma_in_progress) {
 	/*  Stop the DMA mode */
    	rc = ioctl (file_des,MSLA_STOP_DMA);
    	if ( rc < 0 )
    	{
        	printf("MSLA_STOP_DMA ioctl failed\n");
        	return(-1);
    	}
    }
/* TMA */

    rc = close(file_des);
    if( rc == -1)
    {
        #ifdef PRTF
      	    printf("Close File Descriptor Failed\n");
        #endif 
	return(-1);
    }
	

    #ifdef PRTF
      printf("Done\n");
    #endif 
    return(0);

}
/*.................................................................*/

#define PBUFLEN 200
unsigned char  p_buf[PBUFLEN];

void
get_vpd(devname)
char devname[];
{
    int i, j, rc, get_len;
    char  name_str[250];
    struct Class    *predev;
    struct CuVPD    vpdobj;
    struct PdDv     preobj;
    struct cfg_dd   cfgdata;
    unsigned long   majno, major_minor;
    char shortdev[6];
 
/* Open PdDv and get major no. for gsw */

#ifndef MSLADA
    if ( odm_initialize() == -1 ) 
    {
	return;
    }
#endif
    /* change name  like /dev/xxx0 to xxx (gsw or hia)  */
     shortdev[0] = devname[5];
     shortdev[1] = devname[6];
     shortdev[2] = devname[7];
     shortdev[3] = NULL;

    predev = odm_open_class( PdDv_CLASS );
    if ( predev == NULL ) {
	return;
    }
    sprintf(name_str,"type = '%s'", shortdev);
    rc = (int)odm_get_obj(predev,name_str,&preobj,TRUE);
    if ( rc == -1 ) {
	return;
    }
    if ( rc == 0 ) {
	return;
    }
    rc = odm_close_class(predev);
    if ( rc == -1 ) {
	return;
    }
    majno = genmajor(preobj.DvDr);
    if ( majno == -1 ) {
	return;
    }
	
    /****************************************/
    /*    Call VDD to give VPD buffer       */
    /****************************************/
    for (i=0; i<16; i++)
    {
        for (j=0; j<16; j++)
	    p_buf[i*16 + j] = 0xab;
     }

    major_minor = (majno << 16); /* minor no. is taken as 0 */
    cfgdata.kmid =0;  
    cfgdata.devno = major_minor;
    cfgdata.cmd   = CFG_QVPD;
    cfgdata.ddsptr = p_buf;     /* actually vpd pointer */
    cfgdata.ddslen = PBUFLEN;    /* actually length of vpd buffer */

    i = sysconfig(SYS_CFGDD, &cfgdata, sizeof(struct cfg_dd));
    if ( i == -1 ) {
	return;
    }

#ifdef PRTF
    printf("rc from sysconfig = %x\n",i);
#endif
#ifndef MSLADA
    odm_terminate();
#endif
}
