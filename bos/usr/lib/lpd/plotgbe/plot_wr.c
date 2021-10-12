static char sccsid[] = "@(#)86	1.2  src/bos/usr/lib/lpd/plotgbe/plot_wr.c, cmdpsla, bos411, 9428A410j 6/15/90 17:40:15";
/************************************************************************/
/*                                                                      */
/*  COMPONENT_NAME: PLOT_WR                                             */
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
/*           Builds graphics commands to write data to an RS232         */  
/*           port in a 5085. Writes buffer to 5085 and waits for        */ 
/*           GEOP interrupt to indicate operation complete.             */
/*           For PSLA.                                                  */
/************************************************************************/                                                            
                                                                                                                                    
#include <errno.h>                                                                                                                  
#include <stdio.h>                                                                                                                  
#include <fcntl.h>                                                                                                                  
#include <sys/gswio.h>                                                                                                              
#define Xoff 0x36                                                                                                                   
#define Xon 0x35                                                                                                                    
#define DATA_LEN 476                                                                                                                
plot_wr(fd_5080,buf,cnt)                                                                                                            
int fd_5080,cnt;                                                                                                                    
char buf[];                                                                                                                         
{                                                                                                                                   
    int wr_cnt;                                                                                                                     
    char status[4];                                                                                                                 
    union {                                                                                                                         
         short i_cnt;                                                                                                               
         char c_cnt[2];                                                                                                             
    } rd_cnt;                                                                                                                       
    char q_el[40];                                                                                                                  
    struct rwparms rdx;                                                                                                             
                                                                                                                                    
    buf[0] = 0x2a;     /*    GBGIOP           */                                                                                    
    buf[1] = 0x89;     /*    write to plotter */                                                                                    
    buf[2] = 0x00;                                                                                                                  
    buf[3] = 0x00;                                                                                                                  
    buf[4] = 0x00;                                                                                                                  
    buf[5] = 0x08;                                                                                                                  
    buf[6] = 0x2a;     /*    GBGEOP            */                                                                                   
    buf[7] = 0x81;     /*    end processing    */                                                                                   
    buf[8] = 0x01;     /*    iocb WRITE        */                                                                                   
    buf[9] = 0x00;                                                                                                                  
    buf[10] = 0x00;    /*    data length to write to */                                                                             
    buf[11] = 0x00;    /*    RS232 port              */                                                                             
    buf[12] = 0x00;                                                                                                                 
    buf[13] = 0x00;                                                                                                                 
    buf[14] = 0x00;                                                                                                                 
    buf[15] = 0x10;                                                                                                                 
                                                                                                                                    
    rd_cnt.i_cnt = (short) cnt;                                                                                                     
    buf[10] = rd_cnt.c_cnt[0];                                                                                                      
    buf[11] = rd_cnt.c_cnt[1];                                                                                                      
                                                                                                                                    
    rdx.start = FALSE;                                                                                                              
    rdx.stop = TRUE;                                                                                                                
    rdx.async_io = FALSE;                                                                                                           
    rdx.adr.dlb_adr = 0;                                                                                                            
    if ((wr_cnt = writex(fd_5080,buf,rd_cnt.i_cnt+16,&rdx)) < 0)                                                                    
         return(-1);                                                                                                                
                                                                                                                                    
    if ((ioctl(fd_5080,G_SBF_START,0)) == -1)                                                                                       
            return(-1);                                                                                                             
                                                                                                                                    
    if ((ioctl(fd_5080,K_WAIT,q_el)) == -1)                                                                                         
         return(-1);                                                                                                                
                                                                                                                                    
    return(0);                                                                                                                      
}                                                                                                                                   
