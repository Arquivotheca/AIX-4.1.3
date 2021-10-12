static char sccsid[] = "@(#)47  1.4  src/bos/diag/tu/msla/msla19test.c, tu_msla, bos411, 9428A410j 4/15/91 17:11:48";
/*
 * COMPONENT_NAME: ( dsky ) SKYWAY display adapter diagnostic application
 *
 * FUNCTIONS:  dgsftp19, dgsftp18.
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

#include <sys/types.h>
#include "mslaerrdef.h"
#include "mslafdef.h"
#include "mslatu.h"
#include "mslatime.h"
#include "mslablof.h"
#include "msla_diag.h"

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :  msla19test.c                                            */
/*                                                                           */
/*         POURPOSE :  Use the HTX interface to test MSLA  for the following */
/*                      i) mtpwrap.blo ucode is tested which tests SDLC wrap */
/*                     ii) ftp18.blo   ucode is tested which tests modem wrap*/
/*                         in the MSLA Interface Card.                       */
/*                                                                           */
/*            INPUT :  Pointer to the mslatu structure.			     */
/*                                                                           */
/*           OUTPUT :  Returns SUCCESS upon success, else returns the appro- */
/*                     priate error code.                                    */
/*                                                                           */
/* FUNCTIONS CALLED :  mslsclrm, bload, mkerr, disb_int_msla, start__msla    */
/* 		       DELAY_MS, dgsftp18.             			     */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/* COMPILER OPTIONS :  -I../common -g                                        */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

void start_msla();

int
dgsftp19(gtu)
struct mslatu *gtu;
{

    int  rc;

    /* Uses ucode: ftp14.bin */
    rc = modemwrap(gtu);
    if(rc != SUCCESS)
    {
	return(rc);
    }

    /* Uses ucode: ftp17.bin */
    rc = dgsftp17(gtu); 
    if  ( rc != SUCCESS )
    {
        mkerr(FTP19TEST,FTP17,rc,&rc);
    }

    return(rc);
  
}

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :  modemwrap                                             */
/*                                                                           */
/*         POURPOSE :  Performs the Wrap test                                */
/*                                                                           */
/*            INPUT :  Pointer to the mslatu structure.			     */
/*                                                                           */
/*           OUTPUT :  Returns SUCCESS upon success, else returns the appro- */
/*                     priate error code.                                    */
/*                                                                           */
/* FUNCTIONS CALLED :  bload, disb_int_msla, start_msla.                     */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*****************************************************************************/

int
modemwrap(gtu)
struct mslatu *gtu;
{

    extern int mslaclrm();
    extern int bload();
    extern void start_msla();

    int   fd, rc, loop;
    int   new_cnt, old_cnt;
    char  filename[60];
    unsigned long bussmem, ioaddr ;
    unsigned long *flag0;
    unsigned long *flag1;
    unsigned long *flag2;
    unsigned long *flag3;
    unsigned long *flag4;
    unsigned long *flag5;

    unsigned short new_loc_42, old_loc_42, *bus_addr_word;
    unsigned short  *flag2_errcnt, flag2_val ;
    unsigned short  *flg2_opcode, flg2_opval ;
/*    unsigned short  *flag1, *flag5; */
    struct msla_intr_count intr_count;

    fd      = gtu->fd  ;
    ioaddr  = gtu->msla_iobase ;
    bussmem = gtu->msla_membase;


#ifdef PRTF
    printf("Modem wrap......");
#endif

#ifndef CALCOMP
   /* Clear Ram Memory of Adapter */
    mslaclrm (fd,bussmem,MAX_16BIT,ioaddr) ;
#endif

   /* Load micro-code into the Ram adapter */
    strcpy (filename , MODEMWRAP);
    rc = bload (filename,bussmem,fd,7,ioaddr);
    if ( rc != SUCCESS)
    {
        /* File not loaded correctly */
        mkerr(FTP19TEST,FTP14,rc,&rc);
        return(rc);
    }


   /* Start the diagnostic mode */
    rc = ioctl(fd, MSLA_START_DIAG);
    if( rc < 0)
    {
        mkerr(FTP19TEST,FTP14,INT_DRIVER_IOCTL,&rc);
	return(rc);
    }

   /* Enable interrupts from the MSLA to the LEPB */
    enb_int_msla(fd,ioaddr); 

    /*..... Set some usefull flags ......*/
    flag0 = (unsigned long *) (bussmem | 0x040);
    flag1 = (unsigned long *) (bussmem | 0x044);
    flag2 = (unsigned long *) (bussmem | 0x048);
    flag3 = (unsigned long *) (bussmem | 0x04c);
    flag4 = (unsigned long *) (bussmem | 0x050);

    /* This location should be cleared */
    new_cnt = *flag0;
    if (new_cnt != 0)
    {
        mkerr(FTP19TEST,FTP14,FTP14_MEM_LOADING_ERR,&rc);
	return(rc);
    }
  
    rc = FAIL;
    start_msla(fd,ioaddr);

    #ifndef CALCOMP
       sleep(1);
    #else
       mdelay(300);
    #endif

    /* FTP14 : program started */
    for ( loop=0; loop < 3; loop++ )
    {
       /* Listen to  the flag#0 changing  */
       /* Check at-least 3 times the u-code running successfully */
	#ifndef CALCOMP
	   sleep(1);
	#else
           mdelay(400);
	#endif

        old_cnt = new_cnt ; 
        new_cnt = *flag0 ;
        if (old_cnt == new_cnt)
        {
            rc = FTP14_UCODE_NOT_STARTED;
        }
        else
        {
            /* the u-code is running fine */
            rc = SUCCESS ;
        }
    }

    if(rc != SUCCESS)
    {
        mkerr(FTP19TEST,FTP14,rc,&rc);
	return(rc);
    }

    /*... Query the number of interrupts ...*/
    rc = ioctl(fd, MSLA_QUERY_DIAG, &intr_count);
    if( rc < 0)
    {
        mkerr(FTP19TEST,FTP14,INT_DRIVER_IOCTL,&rc);
	return(rc);
    }

    /*... Reset the counting of interrupts ...*/
    rc = ioctl (fd,MSLA_STOP_DIAG);
    if ( rc < 0 )
    {
    	#ifdef PRTF
	   printf("IOCTL STOP DIAG FAILED\n");
    	#endif
        mkerr(FTP19TEST,FTP14,INT_DRIVER_IOCTL,&rc);
	return(rc);
    }

    /*... Did we get parity error ? ...*/
    if (intr_count.total_cnt > 0)
    {
    	#ifdef PRTF
	   printf("Parity Error Ocurred %d times\n",intr_count.parity_cnt);
	   printf("Parity Error Ocurred %d times\n",*flag5);
    	#endif
        rc = FTP_SDLC_HW_ERR ;
    }

    /*... Chech the frame error count ...*/
    if (*flag1 != 0)
    {
    	#ifdef PRTF
	   printf("SOURCE BYTE: 0x%08\n",*flag2);
	   printf("SINK BYTE  : 0x%08\n",*flag3);
    	#endif
        rc = FTP_SDLC_HW_ERR;
	mkerr(FTP19TEST,FTP14,rc,&rc);
	return(rc);
    }

#ifdef PRTF
    printf("Complete\n");
#endif
    return(rc);
}

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :  dgsftp17                                              */
/*                                                                           */
/*         POURPOSE :  Performs SDLC Wrap test                                */
/*                                                                           */
/*            INPUT :  Pointer to the mslatu structure.			     */
/*                                                                           */
/*           OUTPUT :  Returns SUCCESS upon success, else returns the appro- */
/*                     priate error code.                                    */
/*                                                                           */
/* FUNCTIONS CALLED :  bload, disb_int_msla, start_msla.                     */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*****************************************************************************/

int
dgsftp17(gtu)
struct mslatu *gtu;
{

    int  rc, loop, fd;
    int  new_cnt, old_cnt;
    unsigned long *flag0;
    unsigned long *flag1;
    unsigned long *flag2;
    unsigned long ioaddr ;
    unsigned long bussmem;
    char filename[60];


    fd      = gtu->fd;
    ioaddr  = gtu->msla_iobase;
    bussmem = gtu->msla_membase;

    (void) strcpy (filename , SDLCWRAP );

#ifdef PRTF
    printf("Ftp17 test......");
#endif

#ifndef CALCOMP
    mslaclrm(fd,bussmem,MAX_16BIT,ioaddr);
#endif

    rc = bload(filename,bussmem,fd,7,ioaddr);
    if ( rc != SUCCESS)
    {
       /* Ucode File not found */
        return(rc);
    }

   /* Disable interuupts from MSLA to LEPB */
    disb_int_msla(fd,ioaddr);

    /*...... Set some flags that are required .....*/ 
    flag0 = (unsigned long *) ( bussmem | 0x040 ) ;
    flag1 = (unsigned long *) ( bussmem | 0x044 ) ;
    flag2 = (unsigned long *) ( bussmem | 0x048 ) ;

    new_cnt = *flag0;
    if (new_cnt != 0)
    {
        rc = FTP17_MEM_LOADING_ERR;
	return(rc);
    }

    (void) start_msla(fd,ioaddr);

    #ifndef CALCOMP
      sleep(1);
    #else
      mdelay(150);
    #endif

   /* FTP17 : program started */
    for (loop = 0; loop < 3 ; loop++ )
    {
        /* Listen to  the flag#0 changing  */
        /* Check at-least 3 times the u-code running successfully */
	#ifndef CALCOMP
          (void) sleep(1);
	#else
          mdelay(270);
	#endif

        old_cnt = new_cnt; 
        new_cnt = *flag0;

        if (old_cnt == new_cnt)
        {
            rc = FTP17_UCODE_NOT_STARTED;
        }
        else
        {
            /* U-code is running fine */
            rc = SUCCESS ;
        }
    }

    if(rc != SUCCESS)
    {
	/* U-code did not started */
         return(rc);
    }



    if (*flag1 != 0)
    {
	#ifdef PRTF
	    printf("FLAG1: 0x%08x\n",flag1);
	    printf("FLAG2: 0x%08x\n",flag2);
	#endif
        rc =FTP17_MODEMWRAP_ERR ;
    }

#ifdef PRTF
    printf("Complete\n");
#endif
    return(rc);
}
