static char sccsid[] = "@(#)85	1.2  src/bos/usr/lib/lpd/plotgbe/plot_rd.c, cmdpsla, bos411, 9428A410j 6/15/90 17:40:12";
/************************************************************************/
/*                                                                      */
/*  COMPONENT_NAME: PLOT_RD                                             */
/*                                                                      */
/*  ORIGIN: IBM                                                         */
/*                                                                      */
/*. (C) Copyright International Business Machines Corp. 1989, 1990      */
/*. All Rights Reserved                                                 */
/*. Licensed Materials - Property of IBM                                */
/*.                                                                     */
/*. US Government Users Restricted Rights - Use, Duplication or         */
/*. Disclosure Restricted By GSA ADP Schedule Contract With IBM CORP.   */
/*                                                                      */
/* PURPOSE:                                                             */
/*           Build graphic orders which to read the RS232 port.         */ 
/*           Write these orders to the 5085 and wait for a GEOP         */
/*           interrupt. After interrupt has been received, move         */ 
/*           RS232 received data from 5080 buffer into buffer           */ 
/*           provided for PSLA.                                         */ 
/*           For PSLA.                                                  */
/************************************************************************/                                                              
#include <errno.h>                                                                                                                  
#include <stdio.h>                                                                                                                  
#include <fcntl.h>                                                                                                                  
#include <sys/gswio.h>                                                                                                              
#define Xoff 0x36                                                                                                                   
#define Xon 0x35                                                                                                                    
char plot_rd(fd_5080,plot_buf,count)                                                                                                
int fd_5080,count;                                                                                                                  
char plot_buf[];                                                                                                                    
{                                                                                                                                   
    int rd_cnt,wr_cnt;                                                                                                              
    char buf[20];                                                                                                                   
    char q_el[40];                                                                                                                  
                                                                                                                                    
    union {                                                                                                                         
         short i_cnt;                                                                                                               
         char c_cnt[2];                                                                                                             
    } rdcnt;                                                                                                                        
    struct rwparms rdwx;                                                                                                            
                                                                                                                                    
    buf[0] = 0x2a;     /*    GBGIOP                  */                                                                             
    buf[1] = 0x89;     /*    read data from plotter  */                                                                             
    buf[2] = 0x00;                                                                                                                  
    buf[3] = 0x00;                                                                                                                  
    buf[4] = 0x00;                                                                                                                  
    buf[5] = 0x08;                                                                                                                  
    buf[6] = 0x2a;     /*    GBGEOP                  */                                                                             
    buf[7] = 0x81;     /*    end order processing    */                                                                             
    buf[8] = 0x02;     /*    READ iocb               */                                                                             
    buf[9] = 0x00;                                                                                                                  
    buf[10] = 0x00;    /*    read length             */                                                                             
    buf[11] = 0x00;                                                                                                                 
    buf[12] = 0x00;    /*    5080 buffer address     */                                                                             
    buf[13] = 0x00;    /*    to place plotter data   */                                                                             
    buf[14] = 0x00;                                                                                                                 
    buf[15] = 0x10;                                                                                                                 
                                                                                                                                    
    rdcnt.i_cnt = (short) count;                                                                                                    
    buf[10] = rdcnt.c_cnt[0];    /* number of bytes to read */                                                                      
    buf[11] = rdcnt.c_cnt[1];                                                                                                       
                                                                                                                                    
    rdwx.stop = TRUE;                                                                                                               
    rdwx.start = FALSE;                                                                                                             
    rdwx.async_io = FALSE;                                                                                                          
    rdwx.adr.dlb_adr = 0;                                                                                                           
    wr_cnt = 0;                                                                                                                     
                                                                                                                                    
    if ((wr_cnt = writex(fd_5080,buf,16,&rdwx)) == -1)                                                                              
            return(-1);                                                                                                             
                                                                                                                                    
    if ((ioctl(fd_5080,G_SBF_START,0)) == -1)                                                                                       
            return(-1);                                                                                                             
                                                                                                                                    
    do {                                                                                                                            
       wr_cnt = 0;                                                                                                                  
       if (((ioctl(fd_5080,K_WAIT,q_el)) == -1) && (errno == 4))                                                                    
            wr_cnt = 1;                                                                                                             
      } while (wr_cnt == 1);                                                                                                        
                                                                                                                                    
       rdwx.adr.dlb_adr = 0x10;                                                                                                     
       if ((rd_cnt = readx(fd_5080,plot_buf,count,&rdwx)) == -1)                                                                    
             return(-1);                                                                                                            
                                                                                                                                    
    return(rd_cnt);                                                                                                                 
}                                                                                                                                   
                                                                                                                                    
