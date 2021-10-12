static char sccsid[] = "@(#)56	1.2  src/bos/diag/tu/msla/msla20test.c, tu_msla, bos411, 9428A410j 6/15/90 17:22:50";
/*
 * COMPONENT_NAME: ( msla20test )
 *
 * FUNCTIONS:  dgsftp20
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

#include <sys/types.h>
#include "mslaerrdef.h"
#include "mslafdef.h"
#include "mslatu.h"
#include "mslatime.h"
#include "mslablof.h"
#include "msla_diag.h"

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :  dgsftp20                                              */
/*                                                                           */
/*         POURPOSE :  Use the HTX interface to test MSLA  for the following */
/*                     dgs_ftp20 is ftp50.blo running in MSLA to test the    */
/*                     Interface Card on-board 68k processor functionality.  */
/*                                                                           */
/*            INPUT :  Pointer to the mslatu structure, file name of the XXX */
/*                                                                           */
/*           OUTPUT :  Returns SUCCESS upon success, else the proper code.   */
/*                                                                           */
/* FUNCTIONS CALLED :  mslaclrm, mkerr, dsib_int_msla, start_msla, DELAY_MS. */
/*                                                                           */
/* COMPILER OPTIONS =  -I../common -g                                        */
/*                                                                           */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*****************************************************************************/


int
dgsftp20(gtu)
struct mslatu *gtu;
{
   /*
    ** Local variable definitions
    */
    int rc, k;
    int fd;
    unsigned long bussmem, ioaddr ;
    unsigned short new_loc_42, old_loc_42, *bus_addr_word;
    unsigned short  *flag1_word ;
    char filename[60];

   /*
    ** Start of code
    */
    rc = SUCCESS ;

    fd      = gtu->fd  ;
    ioaddr  = gtu->msla_iobase ;
    bussmem = gtu->msla_membase;


   /* Clear the RAM memory of the MSLA */
    mslaclrm (fd,bussmem,MAX_16BIT,ioaddr) ;

   /*  Load the micro-code */
    strcpy (filename, MSLA20_BLO1);
    rc = bload (filename,bussmem,fd,7,ioaddr);
    if ( rc != SUCCESS)
    {
        /* File not found */
        mkerr (FTP20TEST,1,rc,&rc);
        return(rc);
    }

   /* Disable interrupts to the LEPB from the MSLA */
    disb_int_msla (fd,ioaddr) ;

   /* FTP_FLAG0.... is the flag  indicating successful looping of the u-code */
    bus_addr_word = (unsigned short *) ( bussmem | FTP_FLAG0_LO_WORD ) ;

    new_loc_42 = *bus_addr_word;
    if ( new_loc_42 != 0 )
    {
	return(MEM_LOADED_INCORRECTLY);
    }

    start_msla (fd,ioaddr);
    sleep (1);

   /* FTP20 : program started */
    rc = FAIL;
    for ( k = 20 ; k > 0 ; k--)
    {
       /* Check the u-code run at-least 3 times successfully */
        sleep (1);
        old_loc_42 = new_loc_42; 
        new_loc_42 = *bus_addr_word;
        if  ( old_loc_42 == new_loc_42 )
        {
            rc = FTP_UCODE_NOT_STARTED;
        }
        else
        {
            rc = SUCCESS ;
            if  (  new_loc_42 > 2 )
	    {
               /* the u-code is running fine */
                k = 0 ;
 	    }
            else
	    {
                continue;
 	    }
        }
    }


    if ( rc != SUCCESS )
    {
        flag1_word = ( unsigned short *)(bussmem | FTP_FLAG1_LO_WORD );
        /* 16 bit word */
         switch (*flag1_word)
         {
             case 0x00F0 :             /* program start */

                   rc = FTP_START_NO_END ;
                   break ;

             case 0x00F1 :             /* 68K instruction set test start */

                   rc = FTP_68K_INSTR_TEST ;
                   break ; 

             case 0xFFF1 :             /* 68K instruction set test FAIL*/

                   rc = FTP_68K_INSTR_TEST_FAIL ;
                   break ;

             case 0x00F2 :             /* 68K instruction set test O.K */

                   rc = FTP_68K_INSTR_TEST_OK ;
                   break ;

             case 0x00F3 :             /* Memory test by 68K starts */     

                   rc = FTP_MEMTEST_START_NO_END ;
                   break ;

             case 0x00F4 :             /* Parity test started */

                   rc = FTP_PARITYTEST_START_NO_END ;
                   break ;

             case 0xFFFC :             /* Parity check detection error */

                   rc = FTP_PARITY_ERROR ;
                   break ;

             case 0xFFFD :             /* Invalid Parity interrupt detect error */

                   rc = FTP_PARITY_INT_INVALID ;
                   break ;

             case 0xFFFE :            /* DATA strobe/lines/buffers shorts or open */

                   rc = FTP_DATALINES_HW_FAULT ;
                   break ;

             case 0x00F5 :             /* RAM test starts */

                   rc = FTP_RAMTEST_START_NO_END ;
                   break ;

             case 0xFFF5 :             /* RAM test error */                    

                   rc = FTP_RAMTEST_FAIL ;
                   break ;

             case 0x00F6 :             /* TRAP tests start */

                   rc = FTP_TRAPTEST_START_NO_END ;
                   break ;

             case 0xFFF6 :             /* TRAP tests error */               

                   rc = FTP_TRAPTEST_IN_ERROR ;
                   break ;

             case 0x00F7 :             /* TIMER interrupts and speed test starts */

                   rc = FTP_TIMERTEST_START_NO_END ;
                   break ;

             case 0x00FF :             /* Repeat the test */

                   rc = FTP_START_NO_LOOPING ;
                   break ;

             case 0xFFF7 :             /* Timer not working at all */

                   rc = FTP_TIMER_NOT_WORKING ;
                   break ;

             case 0xFFF8 :             /* Timer not working properly */

                   rc = FTP_TIMER_HW_FAULT ;
                   break ;

             case 0xFFF9 :             /* Timer running slower than FSLA specs */

                   rc = FTP_TIMER_SLOW ;
                   break ;

             case 0xFFFA :             /* Timer running faster than FSLA specs */

                   rc = FTP_TIMER_FAST ;
                   break ;

             case 0xFFFB :           /* (NON_MTOS)Unexpected Interrupt system hang*/

                   rc = FTP_NONMTOS_UNEXP_INT ;
                   break ;

             case 0xFFFF :             /* (MTOS)Unexpected Interrupt system hang*/

                   rc = FTP_MTOS_UNEXP_INT ;
                   break ;

             default :

                   break ;

       }
       mkerr(FTP20TEST,1,rc,&rc);
    }

    return(rc);

}
