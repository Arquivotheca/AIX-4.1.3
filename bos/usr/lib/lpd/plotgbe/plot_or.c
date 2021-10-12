static char sccsid[] = "@(#)84	1.3  src/bos/usr/lib/lpd/plotgbe/plot_or.c, cmdpsla, bos411, 9428A410j 9/19/90 09:18:29";
/************************************************************************/
/*                                                                      */
/*  COMPONENT_NAME: PLOT_OR.                                            */
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
/*           Determines the plotter size and sets the origin of         */ 
/*           the plotter to top left. The hardclip limits are used      */
/*           unless 'noin' was requested by user.                       */
/*           For PSLA.                                                  */
/*                                                                      */
/* MODIFICATIONS:                                                       */
/* 09/18/90 bb Allow more than 3 frames. Adjust the 'xmin' value.       */
/************************************************************************/                                                            
                                                                                                                                    
#include <errno.h>                                                                                                                  
#include <stdio.h>                                                                                                                  
#include <sys/gswio.h>                                                                                                              
#include <fcntl.h>                                                                                                                  
extern FILE *ter_fp;                                                                                                                
int plot_or(fd_5080,noinit,frcnt,wk_buf)                                                                                            
int fd_5080, noinit, frcnt;                                                                                                         
char wk_buf[];                                                                                                                      
{                                                                                                                                   
    int cnt,xmin,ymin,xmax,ymax,i,psize;                                                                                            
    char plot_buf[45];                                                                                                              
    char xstr[8];                                                                                                                   
    char ystr[8];                                                                                                                   
    char xmin_str[8];                                                                                                               
    char ymin_str[8];                                                                                                               
    char xmax_str[8];                                                                                                               
    char ymax_str[8];                                                                                                               
    char *cptr;                                                                                                                     
                                                                                                                                    
    psize = 0;                                                                                                                      
    cptr = plot_buf;                                                                                                                
    /* output p1 and p2 limits                           */                                                                         
    ioctl(fd_5080,G_STOP);                                                                                                          
    sprintf(&wk_buf[16],"OP;");                                                                                                     
    if ((cnt = plot_wr(fd_5080,wk_buf,3)) == -1)                                                                                    
         return(-1);                                                                                                                
    if ((cnt = plot_rd(fd_5080,plot_buf,45)) == -1)                                                                                 
         return(-1);                                                                                                                
                                                                                                                                    
    /* if large plotter and IN command executed get plotter*/                                                                       
    /*  hard clip limits otherwise use soft clip limits    */                                                                       
    if (*plot_buf == '-')                                                                                                           
    {                                                                                                                               
         psize = 1;                                                                                                                 
         cptr += 1;                                                                                                                 
         if (noinit == 0)                                                                                                           
         {                                                                                                                          
              sprintf(&wk_buf[16],"OH;");                                                                                           
              if ((cnt = plot_wr(fd_5080,wk_buf,3)) == -1)                                                                          
                   return(-1);                                                                                                      
              if ((cnt = plot_rd(fd_5080,plot_buf,45)) == -1)                                                                       
                   return(-1);                                                                                                      
         }                                                                                                                          
         cnt = (char *)(strchr(plot_buf,0x0a)) - plot_buf;                                                                          
         /* convert string limits to integers      */                                                                               
         plot_buf[cnt] = ',';                                                                                                       
         for (i=0; i<4; i++)                                                                                                        
         {                                                                                                                          
              switch (i)                                                                                                            
              {                                                                                                                     
              case 0 :                                                                                                              
                   xmin = atoi(cptr);                                                                                               
                   break;                                                                                                           
              case 1 :                                                                                                              
                   ymin = atoi(cptr);                                                                                               
                   break;                                                                                                           
              case 2 :                                                                                                              
                   xmax = atoi(cptr);                                                                                               
                   break;                                                                                                           
              case 3 :                                                                                                              
                   ymax = atoi(cptr);                                                                                               
                   break;                                                                                                           
              default :                                                                                                             
                   break;                                                                                                           
              }                                                                                                                     
              cptr = (char *) ((strchr(cptr,',')) +1);                                                                              
              if (*cptr == '-')                                                                                                     
                   cptr += 1;                                                                                                       
         }                                                                                                                          
         plot_buf[cnt] = ';';                                                                                                       
         plot_buf[cnt+1] = (char)NULL;                                                                                                    
                                                                                                                                    
         /* build an IP command                    */                                                                               
         wk_buf[16] = 0x49;        /*  I   */                                                                                       
         wk_buf[17] = 0x50;        /*  P   */                                                                                       
                                                                                                                                    
	 /* if long axis plot adjust xmin value    */
         /* and write IP command to the plotter    */                                                                               
         if (frcnt > 1)                                                                                                             
         {                                                                                                                          
	      /*-------------------------------------------------------*/
	      /* set xmin to twice frame count less 1.                 */
	      /*-------------------------------------------------------*/
	      itoa((xmin*(2*frcnt - 1)),xmin_str);
              itoa(ymin,ymin_str);                                                                                                  
              itoa(xmax,xmax_str);                                                                                                  
              itoa(ymax,ymax_str);                                                                                                  
              sprintf(wk_buf+18,"-%s,-%s,%s,%s;",xmin_str,                                                                          
                  ymin_str,xmax_str,ymax_str);                                                                                      
         }                                                                                                                          
         else                                                                                                                       
              sprintf(wk_buf+18,"%s",plot_buf);                                                                                     
         if ((plot_wr(fd_5080,wk_buf,strlen(wk_buf+16))) == -1)                                                                     
              return(-1);                                                                                                           
                                                                                                                                    
         /* convert xmax and ymax values from integers */                                                                           
         /* to string, place in SC command and write   */                                                                           
         itoa((((xmax+xmin)*frcnt)),xstr);                                                                                          
         itoa((ymax+ymin),ystr);                                                                                                    
         sprintf(wk_buf+16,"SC0,%s,0,%s;",xstr,ystr);                                                                               
         if ((plot_wr(fd_5080,wk_buf,strlen(wk_buf+16))) == -1)                                                                     
              return(-1);                                                                                                           
    }                                                                                                                               
    return(psize);                                                                                                                  
}                                                                                                                                   
itoa(num,str)                                                                                                                       
int num;                                                                                                                            
char *str;                                                                                                                          
{                                                                                                                                   
    int i,c,j;                                                                                                                      
                                                                                                                                    
    i = 0;                                                                                                                          
    do {                                                                                                                            
         str[i++] = num % 10 + '0';                                                                                                 
    }    while ((num /=10) > 0);                                                                                                    
    str[i] = '\0';                                                                                                                  
    for (i=0, j=strlen(str)-1; i<j; i++, j--)                                                                                       
    {                                                                                                                               
         c = str[i];                                                                                                                
         str[i] = str[j];                                                                                                           
         str[j] = c;                                                                                                                
    }                                                                                                                               
}                                                                                                                                   
