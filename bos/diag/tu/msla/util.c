static char sccsid[] = "@(#)33	1.2.1.1  src/bos/diag/tu/msla/util.c, tu_msla, bos411, 9428A410j 5/2/94 10:42:38";
/*
 * COMPONENT_NAME: ( util ) 
 *
 *  FUNCTIONS :  delay, load_routin, gm_slot, det_slot, int_rt, enitmsla,
 *                intrtenb, readintr, byteswap.			     
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
#include "mslafdef.h"
#include "mslatime.h"
/* #include "hxemsla.h" */
#include "mslaerrdef.h"
#include "mslatu.h"
#include "msla_diag.h"
#include "msladgappl.h"

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :  delay                                                 */
/*                                                                           */
/*         POURPOSE :  Provides a delay. Each count will give (approx) 20 ns.*/
/*                                                                           */
/*            INPUT :  number.                          		     */
/*                                                                           */
/*           OUTPUT :  None.						     */ 
/*                                                                           */
/* FUNCTIONS CALLED : DELAY_MS						     */
/*                                                                           */
/*  DATE of Creation     : 10/07/1988                                        */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*****************************************************************************/

void
delay(number)
int number;
{
      DELAY_MS(number);
}

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :  load_routin.                                          */
/*                                                                           */
/*         POURPOSE :                                                        */
/*                 This load routine is specifically for MSLA code loading   */
/*                 The BUSSMEM is initialised to MSLA address hence  the     */
/*                 third argument is actually an offset, not absolute .      */
/*                 Loads a word array in ths adapter memory                  */
/*                                                                           */
/*            INPUT : bussmem,code_array,code_len,load_addr,fd,ioaddr        */
/*                                                                           */
/*           OUTPUT : None.						     */ 
/*                                                                           */
/* FUNCTIONS CALLED : 							     */
/*                                                                           */
/*  DATE of Creation: 10/07/1988                                	     */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*****************************************************************************/

void
load_routin(bussmem,code_array,code_len,load_addr,fd,ioaddr)
unsigned long bussmem;
unsigned short code_array[];                    /* matrix of unsigned integer */
int code_len;                                   /* dimension of the matrix    */
unsigned int load_addr;                         /* offset from MSLA memory    */
int fd;
unsigned long ioaddr;
{
    /*
    ** Local variable definitions
    */
    int loop;
    unsigned short data_hold;
    unsigned long address;
    unsigned short *bus_address ;
    /*
    ** Start of code
    */
    pre_load_setup(fd,ioaddr);

    address = bussmem | load_addr;
    bus_address = ( unsigned short *) address;
    for ( loop = 0; loop < code_len; loop++)
    {
        data_hold = code_array[loop];
        *bus_address = data_hold;
        bus_address++ ;
    }
    post_load_setup(fd);
}

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :  mslaclrm                                              */
/*                                                                           */
/*         POURPOSE :                                                        */
/*                                                                           */
/*            INPUT :  fd,slot,bussmem,sizeofmemory,ioaddr                   */
/*                                                                           */
/*           OUTPUT :  Does not returns any useful info. 		     */
/*                                                                           */
/* FUNCTIONS CALLED : delay, disable_parity_chk, reset_msla, halt_msla,      */
/*		      rst_int_msla.					     */
/*                                                                           */
/*  DATE of Creation: 10/10/1988                                             */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*****************************************************************************/

int 
mslaclrm(fd,bussmem,sizeofmemory,ioaddr)
int fd;
unsigned long bussmem;
unsigned sizeofmemory;
unsigned long ioaddr;
{
    /*
    ** Local function declarations
    */
    void delay();
    extern void halt_msla() ;   
    extern void rst_int_msla() ;

    /*
    ** Local variable definitions
    */
    int rc;
    unsigned short *bus_address ;
    register unsigned short int empty_it = NULL_DATA;
    register int memory_byte;

    /*
    ** Start of code
    */
    reset_msla(fd,ioaddr) ;                  /* Reset MSLA address */
    delay(100);

    halt_msla(fd,ioaddr) ;                   /* Halt  MSLA address */
    delay(100);

    rst_int_msla(fd,ioaddr) ;                /* Reset Interrupt MSLA */
    delay(100);

     /* disable_parity_chk(fd,slot); */
    rc = ioctl (fd, MSLA_MOD_POS, MP_DIS_PARITY);
    if (rc < 0)
    {
    	perror("Disable Parity check Failed\n");
    	return(rc);
    }

    /* The MSLA memory is 64K bytes */
    bus_address = ( unsigned short *)(bussmem);
    for ( memory_byte = sizeofmemory ; memory_byte >= 2; memory_byte -=2)
    {
        *bus_address = empty_it;
        bus_address++ ;
                
    }
    rc = SUCCESS;
    return(rc);
}

