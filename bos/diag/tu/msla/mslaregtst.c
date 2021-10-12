static char sccsid[] = "@(#)52  1.2.1.1  src/bos/diag/tu/msla/mslaregtst.c, tu_msla, bos41J, 9517A_all 4/25/95 08:45:10";
/*
 * COMPONENT_NAME: ( mslaregtst )
 *
 * FUNCTIONS:  regtest, mslastst, intrmsla
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
#include "mslaregdat.h"
#include "msla_diag.h"

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :  regtest.                                              */
/*                                                                           */
/*          PURPOSE :  Use the HTX interface to test MSLA  for i/o           */
/*                     register read/writes on the micro-channel             */
/*                     Interface Card.                                       */
/*                                                                           */
/*            INPUT :  pointer to the mslatu structure                       */
/*                                                                           */
/*           OUTPUT :  Retuen SUCCESS upon success, else the proper code.    */
/*                                                                           */
/* FUNCTIONS CALLED :  mslaclrm, mslastst, mkerr, intrmsla, int_rt.          */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/* COMPILER OPTIONS =                                                        */
/*                                                                           */
/*****************************************************************************/

int
regtest(gtu)
struct mslatu *gtu;
{
    int rc ;
    int fd ;
    unsigned long bussmem ;
    unsigned long ioaddr ;
    unsigned short val ;

    fd      = gtu->fd  ;
    ioaddr  = gtu->msla_iobase ;
    bussmem = gtu->msla_membase;

   /* Clear the MSLA RAM */
    mslaclrm(fd,bussmem,MAX_16BIT,ioaddr);

    rc = mslastst(fd,bussmem,ioaddr);
    if (rc != SUCCESS )
    {
        mkerr(REGTEST,1,rc,&rc);
        return(rc);
    }


   /* Clear the MSLA RAM */
    mslaclrm(fd,bussmem,MAX_16BIT,ioaddr);

    rc = intrmsla(bussmem,fd,&val,ioaddr);
    if (rc != SUCCESS )
    {
        mkerr(REGTEST,1,rc,&rc);
        return(rc);
    }

    return(rc);
}

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :  mslastst.                                             */
/*                                                                           */
/*        MODULE No  :                                                       */
/*                                                                           */
/*          PURPOSE :                                                        */
/*             VERIFY THAT THE RIOS CAN START AND STOP THE 68000 ON THE MSLA */ 
/*            BY LOADING A PROGRAM TO LOOP AND COUNT AND THEN ISSUING START. */ 
/*            CHECK FOR THE COUNT INCREMENTS THEN ISSUE A HALT.              */
/*            CHECK THAT THE COUNT IS NOT INCREMENTING ANY LONGER.           */ 
/*            THE 68K IS STARTED AGAIN AND A RESET IS ISSUED. AFTER A DELAY  */ 
/*            OF 200ms THE 68K IS HALTED AND RESTARTED.  THE COUNTER SHOULD  */ 
/*            INCREMENT FROM ZERO.HENCE IF THE COUNTER VALUE IS STORED BEFORE*/ 
/*            RESET AND THEN COMPARED AFTER HALT-START SEQUENCE THEN RESET IS*/ 
/*            WORKING CORRECTLY .                                            */ 
/*                                                                           */
/*            INPUT :                                                        */
/*                                                                           */
/*           OUTPUT :                                                        */
/*                  A1. RETURN CODE                                          */ 
/*                      RC = 0  ==> ALL RESULTS GOOD                         */ 
/*                      RC = 1  ==> START FAILS                              */ 
/*                      RC = 2  ==> STOP FAILS                               */ 
/*                      RC = 4  ==> START after STOP FAILS                   */ 
/*                      RC = 5  ==> RESET FAILS                              */ 
/*                      RC = 6  ==> HALT AFTER RESET FAILS                   */ 
/*                                                                           */
/* FUNCTIONS CALLED :   None.                                                */
/*                                                                           */
/*  DATE of Creation: 10/06/1988                                             */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*           STATUS : TESTED                                                 */
/*                                                                           */
/*****************************************************************************/

int
mslastst(fd,bussmem,ioaddr )
int fd ;
unsigned long bussmem ;
unsigned long ioaddr ;
{

    int   rc, index, i;
    int   retry, error_no;
    unsigned short *bus_address;
    unsigned long   address;
    unsigned long   counter_word1 , counter_word2;
    volatile unsigned short  counter_test_word, msla_count_word ;
    unsigned short  counter_hitest_word ,counter_lotest_word ;


    /* Reset MSLA, Disable parity, Halt 68k and Enable Card */
    pre_load_setup(fd,ioaddr);


    /* Load the 68K procesor with the count code */
    address = bussmem | START_PARM ;
    bus_address =  (unsigned short *) address;

    for (index = 0; index < MSLA_COUNT_ROUT_LEN; index++)
    {
        *bus_address = msla_count_rout[index] ;
        bus_address++ ;
    }

    /* Start the 68K procesor with the count code */
    delay (200);
    start_msla(fd,ioaddr);

    rc = MSLASTST_START_FAIL;
    address = bussmem | MSLA_LOOK_COUNTER;
    bus_address =  (unsigned short *) address;
    for ( retry = 0, error_no = 0; error_no < MAX_ERROR; retry++)
    {
        counter_test_word = *bus_address ;
        delay(20);   

        msla_count_word = *bus_address ;
        if ( counter_test_word == msla_count_word )  /* 68K not in run*/
        {                                            /* state for TRUE*/
            if (error_no == MAX_ERROR -1 )
            {
                rc = MSLASTST_START_FAIL ;
                break ;
            }
            else
            {
                error_no += 1;
            }
        }
        else
        {
            if (( retry > 1 ) && ( rc == SUCCESS ))
            {
                rc = SUCCESS;
                error_no = MAX_ERROR;
                break ;
            }
            else
            {
                rc = SUCCESS;
            }
        }
    }/* end of test for 68K running */

    if ( rc != SUCCESS )
    {
 	return(rc);
    }


    /* 
    ***********************************************
    *  HALT the 68K procesor with the count code  *
    ***********************************************
    */ 
    halt_msla(fd,ioaddr);
        
    address = bussmem | MSLA_LOOK_COUNTER;
    bus_address =  (unsigned short *) address;
    for ( retry = 0, error_no = 0; error_no < MAX_ERROR; retry++)
    {
        counter_test_word = *bus_address ;
        delay(20); 

        msla_count_word = *bus_address ;
        if ( counter_test_word != msla_count_word )
        {                    /* 68K not in halt state for TRUE*/
            if (error_no == MAX_ERROR - 1 )
            {
                rc = MSLASTST_STOP_FAIL ;
                break ;
            }
            else
            {
                error_no += 1;
            }
        }
        else
        {
            rc = SUCCESS ;
            break ;
        }
    }

    if ( rc != SUCCESS )
    {
	return(rc);
    }


    start_msla(fd,ioaddr);
    counter_test_word = *bus_address ;
    delay(20); 
    msla_count_word = *bus_address ;
    for (i=0;i<5; i++)
	{   /* since there is a very small chance that we have
	      wrapped and value is equal even though code is running
	      try several times to be sure  */
	    if ( counter_test_word == msla_count_word )
		  sleep(1);
	    else
		 break;
	}
    if ( i > 4 )    /* we fell through... 68k not running */
    {                   /* 68K should be in run state TRUE*/
        rc = MSLASTOP_START_FAIL ;
    }
    else
    {   

        reset_msla(fd,ioaddr);
        counter_test_word = *bus_address ;
        delay(20);
        msla_count_word = *bus_address ;
        if ( counter_test_word != msla_count_word )
        {
            rc = MSLA_RESET_FAIL ;
        }
        else
        {
	    /* test deleted... was incorrect and redundant */
	    reset_msla(fd,ioaddr) ;
        }
    }

    return(rc);
}

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :   intrmlsa                                             */
/*                                                                           */
/*       MODULE No  :                                                        */
/*                                                                           */
/*          PURPOSE :                                                        */
/*              VERIFY THAT THE RIOS CAN INTERUPT THE MSLA AND THAT THE MSLA */
/*              CAN RESET THE INTERUPT.                                      */
/*              PROGRAMS ARE LOADED TO THE  MSLA TO HANDLE THE INTERUPT AND  */
/*              THE INTERUPT IS ISSUED.                                      */
/*              THE COMMUNICATIONS AREAS ARE ALSO LOADED AND QUERIED TO      */
/*              DETERMINE THE INTERUPT STATUS.                               */
/*                                                                           */
/*            INPUT :                                                        */
/*                                                                           */
/*           OUTPUT :                                                        */
/*                  A1. RETURN CODE                                          */
/*                      RC = 0  ==> ALL RESULTS GOOD                         */
/*                      RC = 1  ==> ROUTINE FAILS                            */
/*                                                                           */
/* FUNCTIONS CALLED :   delay, load_routin.                                  */
/*                                                                           */
/*  DATE of Creation: 10/07/1988                                             */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*           STATUS : TESTED                                                 */
/*                                                                           */
/*****************************************************************************/

int
intrmsla(bussmem,fd,val,ioaddr)
unsigned long bussmem;
int fd;
unsigned short *val;
unsigned long ioaddr;
{
    /*
    ** Local variable definitions
    */
    int rc ;
    char *bus_addr_char;
    char hold_103, hold_107;
    unsigned int addr;
    unsigned short data, k ;

    rc = SUCCESS;
    *val = 0 ;

    load_routin(bussmem,msla_intrupt_rout1,MSLA_INTRUPT_ROUT1_LEN,ROUT1_ADDR,fd,ioaddr);
    load_routin(bussmem,msla_intrupt_rout2,MSLA_INTRUPT_ROUT2_LEN,ROUT2_ADDR,fd,ioaddr);
    load_routin(bussmem,msla_intrupt_rout3,MSLA_INTRUPT_ROUT3_LEN,ROUT3_ADDR,fd,ioaddr);
    load_routin(bussmem,msla_intrupt_rout4,MSLA_INTRUPT_ROUT4_LEN,ROUT4_ADDR,fd,ioaddr);

   /* Start the 68K processor */
    post_load_setup(fd);

    start_msla(fd,ioaddr) ;
    delay(100);

    addr = bussmem | ROUT3_ADDR;
    bus_addr_char = (char *) addr;
    hold_103 = *bus_addr_char ;
    data = (unsigned short )hold_103;

    for (k = 1; k <= 3; k++)
    {
        addr = bussmem | ROUT5_ADDR;
        bus_addr_char = (char *) addr;
        hold_103 = *bus_addr_char ;
        delay(40);

        intrpt_msla(fd,ioaddr) ;
                            /* wait for the 68k interrupt handler     */
        delay(1000);        /* to complete and increment location 107 */

        addr = bussmem | ROUT6_ADDR;
        bus_addr_char = (char *) addr;
        hold_107 = *bus_addr_char ;

        data = (unsigned short )hold_107;
        if ( data != k )
        {
            rc = INTR_TO_MSLA_FAIL;
            *val = data;
        }
        else
        {
            rc = SUCCESS;
        }
    }

    halt_msla(fd,ioaddr) ;

    return(rc);
}
