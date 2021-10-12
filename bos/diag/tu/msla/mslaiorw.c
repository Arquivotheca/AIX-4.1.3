static char sccsid[] = "@(#)41	1.2  src/bos/diag/tu/msla/mslaiorw.c, tu_msla, bos411, 9428A410j 6/15/90 17:23:32";
/*
 * COMPONENT_NAME: ( mslaiorw ) 
 *
 * FUNCTIONS :  start_msla, reset_msla, halt_msla, rst_int_msla,
 *		   enb_int_msla, disb_int_msla, stat_msla, id_hi_msla,
 *		   intrpt_mlsa, id_low_msla, ioprt_write, ioport_read.
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

#include <unistd.h>
#include <sys/mdio.h>
#include "mslafdef.h"

#define DUMMY_DATA_C        0x00             /* Char data   */

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :  start_msla.                                           */
/*                                                                           */
/*         PURPOSE :                                                         */
/*                                                                           */
/*            INPUT :  fd, ioaddr                       		     */
/*                                                                           */
/*           OUTPUT :  None.						     */
/*                                                                           */
/* FUNCTIONS CALLED : ioport_write.					     */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*****************************************************************************/


void start_msla (fd,ioaddr)
int fd;
ulong ioaddr;
{                     
    char *addr;
    
     addr = (char *)(ioaddr | START_MSLA) ; 
     *addr  = DUMMY_DATA_C ;

}

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :   reset_msla                                           */
/*                                                                           */
/*         PURPOSE :                                                        */
/*                                                                           */
/*            INPUT :   fd, ioaddr                      		     */
/*                                                                           */
/*           OUTPUT :   None.						     */
/*                                                                           */
/* FUNCTIONS CALLED :                    				     */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*****************************************************************************/

void reset_msla(fd,ioaddr)
int fd;
ulong ioaddr;
{                      
    char *addr;

     addr = (char *)(ioaddr | RSET_MSLA) ; 
     *addr  = DUMMY_DATA_C ;

}

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :   halt_msla                                            */
/*                                                                           */
/*         PURPOSE :                                                        */
/*                                                                           */
/*            INPUT :   fd, ioaddr                      		     */
/*                                                                           */
/*           OUTPUT :   None.						     */
/*                                                                           */
/* FUNCTIONS CALLED :                              			     */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*****************************************************************************/

void halt_msla(fd,ioaddr)
int fd;
ulong ioaddr;
{                       
    void  ioport_write();
    char *addr;

     addr = (char *)(ioaddr | HLT_MSLA) ; 
     *addr  = DUMMY_DATA_C ;

}

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :   rst_int_msla                                         */
/*                                                                           */
/*         PURPOSE :                                                        */
/*                                                                           */
/*            INPUT :   fd, ioaddr                      		     */
/*                                                                           */
/*           OUTPUT :   None.						     */
/*                                                                           */
/* FUNCTIONS CALLED :                      			             */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*****************************************************************************/

void rst_int_msla(fd,ioaddr)
int fd;
ulong ioaddr;
{                    
    void  ioport_write();
    char *addr;

     addr = (char *)(ioaddr | RESET_INTERUPT_MSLA) ; 
     *addr  = DUMMY_DATA_C ;

}

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :   enb_int_msla                                         */
/*                                                                           */
/*         PURPOSE :                                                        */
/*                                                                           */
/*            INPUT :   fd, ioaddr                      		     */
/*                                                                           */
/*           OUTPUT :   None.						     */
/*                                                                           */
/* FUNCTIONS CALLED :  ioport_write.					     */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*****************************************************************************/

void enb_int_msla(fd,ioaddr)
int fd;
ulong ioaddr;
{                    
    void  ioport_write();
    char *addr;

     addr = (char *)(ioaddr | ENABLE_INTERUPT_MSLA) ; 
     *addr  = DUMMY_DATA_C ;

}
/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :   disb_int_msla                                         */
/*                                                                           */
/*         PURPOSE :                                                        */
/*                                                                           */
/*            INPUT :   fd, ioaddr                      		     */
/*                                                                           */
/*           OUTPUT :   None.						     */
/*                                                                           */
/* FUNCTIONS CALLED :  ioport_write.					     */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*****************************************************************************/

void disb_int_msla(fd,ioaddr)
int fd;
ulong ioaddr;
{                    
    void  ioport_write();
    char *addr;

     addr = (char *)(ioaddr | DISABLE_INTERUPT_MSLA) ; 
     *addr  = DUMMY_DATA_C ;

}

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :   intrp_msla                                           */
/*                                                                           */
/*         PURPOSE :                                                        */
/*                                                                           */
/*            INPUT :   fd, ioaddr                      		     */
/*                                                                           */
/*           OUTPUT :   None.						     */
/*                                                                           */
/* FUNCTIONS CALLED :  ioport_write.					     */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*****************************************************************************/
void intrpt_msla(fd,ioaddr)
int fd;
ulong ioaddr;
{                    
    void  ioport_write();
    char *addr;

     addr = (char *)(ioaddr | INTERUPT_MSLA) ; 
     *addr  = DUMMY_DATA_C ;

}

/*****************************************************************************/
/*                                                                           */
/*      MODULE NAME :   stat_msla                                            */
/*                                                                           */
/*         PURPOSE :                                                        */
/*                                                                           */
/*            INPUT :   fd, ioaddr                      		     */
/*                                                                           */
/*           OUTPUT :   None.						     */
/*                                                                           */
/* FUNCTIONS CALLED :  ioport_read.					     */
/*                                                                           */
/*           STATUS :  Release 1 Version 0                                   */
/*                                                                           */
/*****************************************************************************/
int
stat_msla(fd,ioaddr)
int fd;
ulong ioaddr;
{                    
    char ioport_read();
    char data ;
    ulong adr;
    char *addr;


    addr = (char *)(ioaddr | READ_SR_MSLA) ; 
    data = *addr ;

    return((int )data) ; 
}

