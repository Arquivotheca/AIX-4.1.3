static char sccsid[] = "@(#)33	1.1  src/bos/usr/lib/lpd/plotgbe/plot_int.c, cmdpsla, bos411, 9428A410j 3/27/90 17:44:14";
/************************************************************************/
/*                                                                      */
/*  COMPONENT_NAME: PLOT_INT.C.                                         */
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
/*           Initialize a buffer with graphics commands to cause        */ 
/*           the RS232 port to be initialized                           */
/*           For PSLA.                                                  */
/*;bb 032390 Changed name from 'port_int.c' to 'plot_int.c'.            */
/************************************************************************/                                                                
#include <stdio.h>                                                                                                                  
plot_int(buf,speed)
char *buf;                                                                                                                          
int speed;                                                                                                                          
{                                                                                                                                   
        sprintf(buf,"%c%c%c%c%c%c",0x2a,0x89,0x00,0x00,0x00,0x0e);                                                                  
        sprintf(buf+6,"%c%c%c%c%c%c",0x2a,0x89,0x00,0x00,0x00,0x16);                                                                
        sprintf(buf+12,"%c%c",0x2a,0x81);                                                                                           
        sprintf(buf+14,"%c%c%c%c%c%c%c%c",0x43,0x00,0x00,0x02,0x00,0x00,                                                            
                 0x00,0x1e);                                                                                                        
        sprintf(buf+22,"%c%c%c%c%c%c%c%c",0x83,0x00,0x00,0x02,0x00,0x00,                                                            
                 0x00,0x20);                                                                                                        
        sprintf(buf+30,"%c%c%c%c%c",0x70,0x10,0x34,0x18,0x00);                                                                      
                                                                                                                                    
        switch(speed){                                                                                                              
        case 150:                                                                                                                   
                buf[30] = 0x40;                                                                                                     
                buf[32] = 0x04;                                                                                                     
                break;                                                                                                              
        case 300:                                                                                                                   
                buf[30] = 0x48;                                                                                                     
                buf[32] = 0x0c;                                                                                                     
                break;                                                                                                              
        case 600:                                                                                                                   
                buf[30] = 0x50;                                                                                                     
                buf[32] = 0x14;                                                                                                     
                break;                                                                                                              
        case 1200:                                                                                                                  
                buf[30] = 0x58;                                                                                                     
                buf[32] = 0x1c;                                                                                                     
                break;                                                                                                              
        case 2400:                                                                                                                  
                buf[30] = 0x60;                                                                                                     
                buf[32] = 0x24;                                                                                                     
                break;                                                                                                              
        case 4800:                                                                                                                  
                buf[30] = 0x68;                                                                                                     
                buf[32] = 0x2c;                                                                                                     
                break;                                                                                                              
        case 9600:                                                                                                                  
                break;                                                                                                              
        default:                                                                                                                    
                return(-1);                                                                                                         
        }                                                                                                                           
}                                                                                                                                   
