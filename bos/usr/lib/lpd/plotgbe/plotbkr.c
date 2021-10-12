static char sccsid[] = "@(#)87  1.6  src/bos/usr/lib/lpd/plotgbe/plotbkr.c, cmdpsla, bos411, 9428A410j 11/18/93 14:07:33";
/*
 *   COMPONENT_NAME: CMDPSLA
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/************************************************************************/
/*                                                                      */
/*  COMPONENT_NAME: PLOTBKR for PSLA Adapter.                           */
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
/*            This is the BACKEND program which drives plotters         */ 
/*            attached to 5085. The module initializes the RS232        */
/*            ports, establishes an ENQ/ACK protocol with plotter       */  
/*            determines size of plotter and then opens a file          */
/*            containing plotter commands. The plotter commands         */
/*            are preceded by graphic orders which are sent with        */
/*            the plotter commands to the RS232 port and read the       */
/*            plotter response.                                         */ 
/*                                                                      */ 
/* ROUTINES:                                                            */
/*      WR_PLOT: Writes RS232 initialization to 5085                    */
/*      CK_STATUS: Check condition of RS232 port in 5085                */ 
/*      CK_PLOT: Check the condition of plotter                         */ 
/*      DRIVE_PLOT : Read plotter commands from a file and write        */
/*                   them to the 5080 to be sent to plotter. Wait       */  
/*                   for ack from plotter.                              */ 
/*      FIND_NR:  Search of NR plotter command                          */
/*      END_PLOT: End of plot commands procesing                        */
/*      TIME_OUT: Field alarm signal if plotter fails to respond        */ 
/*      NO_RESPONSE: Filed alarm signal if plotter fails to respond     */ 
/*                   while plotting a file                              */
/*      DIE: Field signal when user issues cancel                       */
/*      ERROR_EXIT: Common exit point in backend                        */ 
/*                                                                      */ 
/* Include:                                                             */
/*     PLOT_RD: Appends read graphic orders to buffer provided and      */
/*              writes buffer to 5085. Returns plotter information      */ 
/*     PLOT_WR: Appends write graphic orders to buffer provided and     */
/*              write buffer to 5085.                                   */
/*     PLOT_INT: Initializes a buffer with graphic orders which will    */
/*               set up the RS232 ports in the 5085.                    */  
/*     PLOT_HND: Initializes a buffer with an ENQ/ACK handshake         */ 
/*     PLOT_OR: Set the plotter origin                                  */
/*     PLOT_MSG: Issues messages to user or qdaemon                     */
/*                                                                      */
/* MODIFICATIONS:						        */
/* 02/13/90 MJ PLOT_GETU function was removed, since terminal           */
/*             requesting the polt is identified by the new library     */
/*             function 'telluser()'; so there is no need to get a      */
/*             terminal pointer. 's_mailonly' field in the 'stfile'     */
/*             external structure indicates whether the message is      */
/*             sent to mail box only or may be displayed on the         */
/* 	       terminal also.					        */
/*             The first parameter to 'plot_msg()' function, terminal   */
/*             pointer, is removed.				        */
/*             'nm_indx' variable incrementation in the case where      */
/*             the second argument to backend is not '_statusfile'      */
/*             is removed, with that it did not work.		        */
/*             'rate' variable eliminated, not used.		        */ 
/* 03/01/90 MJ Changed the return codes to 'defines' in the             */
/*	       IN/standard.h file and made them based on the return     */
/*	       condition.					        */
/*	       Added Check of the file descriptor#3 which is supposed   */
/*	       to be opened by qdaemon and accessible by backend.       */
/*	       Passed a dummy string to 'log_init()' call.	        */
/*	       Set the 's_mailonly' field to zero for all the cases.    */
/* 03/02/90 MJ Added check for plotter status in the loop to process    */
/*             each plotter file.				        */
/* 03/22/90 MJ Added the wait for the plot to be viewed in case of the  */
/*             large plotters at the end of the plot.			*/
/* 03/23/90 bb Changed 'port_int' to 'plot_int'.                        */
/* 03/24/90 bb Added NLS support.                                       */
/* 05/19/90 bb Remove all references to 'stfile'.                       */
/* 09/10/90 bb Allow more than 3 frames.                                */
/* 04/30/91 bb Change open for plotter commands file to be O_RDONLY,    */
/*             not O_RDWR; change error msg from list[1] to list[i]     */
/*             (around line 340)                                        */
/************************************************************************/
#include <errno.h>                                                                                                                  
#include <stdio.h>                                                                                                                  
#include <fcntl.h>                                                                                                                  
#include <signal.h>                                                                                                                 
#include <sys/gswio.h>                                                                                                              
#include "plot_def.h"                                                                                                               
#include <sys/types.h>                                                                                                              
#include <nl_types.h>
#include <sys/stat.h>                                                                                                               
#include <IN/standard.h>
#include <locale.h>
#include "plotgbe_msg.h"                /* generated from plotgbe.msg   */

#define Xoff    0x36
#define Xon     0x35
#define WAITING 3                                                                                                                   
#define RUNNING 2

struct {
    char model[4];
    int memsize;
    char plot_size;
    char roll_feed;
    char pen_select;
    char arc_circle;
    char poly_fill;
    char u_ram;
} options;

struct {
    unsigned nr_fnd   : 1;
    unsigned nonr_fnd : 1;
    unsigned eof      : 1;
} flags;

int ignoreparm = 0;                     /* parm ignored on catopen call */
int fd_dev     = 0;
int fd         = 0;
int file_index = 0;                                                                                                                 
int logm       = 0;
int cancel     = 0;
char *dummyp   = NULL;                  /* dummy ptr for extra parms    */
char *buf      = NULL;
char *list[20];                                                                                                                     
nl_catd catd;                           /* for NLS message catalog      */
extern char *malloc();

main(argc,arglist)                                                                                                                  
int argc;                                                                                                                           
char *arglist[];                                                                                                                    
{                                                                                                                                   
    void no_response();                                                                                                              
    void die();                                                                                                                      
                                                                                                                                    
    short noinit;                                                                                                                   
    int i, j, nm_indx, fr, frcnt, rd_cnt;                                                                                           
    int speed, bufcnt, ct;                                                                                                    
    int statf_fd = 3;
                                                                                                                                    
    char bufsz[4];                                                                                                                  
    char dev[20];                                                                                                                   
    char init[4];                                                                                                                   
    char wk_buf[54];                                                                                                                
    char msgbuf[BUFSIZ];                /* hold NLS msgs for telluser   */
    struct k_enable en_geop;                                                                                                        
    struct stat filestat; 

    nm_indx = 3;                                                                                                                    
    fr = 1;                                                                                                                         
    flags.nr_fnd   = FALSE;
    flags.nonr_fnd = FALSE;                                                                                                         
    options.plot_size = ' ';                                                                                                        
    noinit = 0;                                                                                                                     
    file_index = argc-1;                                                                                                            
    speed = 9600;                                                                                                                   
                                                                                                                                    
    setlocale(LC_ALL, "");              /* set locale for NLS           */
					/* open message catalog         */
    if ( (catd = catopen(MF_PLOTGBE,NL_CAT_LOCALE)) == CATD_ERR) {
	telluser("Cannot open message catalog for plotgbe. \n");
	exit(EXITBAD);
    }
    for(i=0; i<argc; i++)
         list[i] = arglist[i];                                                                                                      
    signal(SIGTERM,die);                                                                                                            
    if ((fstat(statf_fd,&filestat)) == -1)                                                                                                
    {                                                                                                                               
	 plot_msg(STAT_ERR,list[1],dummyp);
         error_exit(EXITBAD);                                                                                                            
    }                                                                                                                               
    log_init("NAME");                   /* "NAME" is a dummy parm       */
    /* open path to plotter via 5080 RS232 port     */                                                                              
    sprintf(dev,"/dev");                                                                                                            
    strcpy(&dev[4],list[1]++);                                                                                                      
    dev[4] = '/';                                                                                                                   
    if ((fd_dev = open(dev,O_RDWR)) == -1)                                                                                          
    {                                                                                                                               
	 plot_msg(OPEN_PORT,list[1],dummyp);
         error_exit(EXITBAD);                                                                                                            
    }                                                                                                                               

    /* enable GEOP queue                           */
    en_geop.type = GGEOP;                                                                                                           
    en_geop.qualifier = -1;                                                                                                         
    en_geop.buf_adr = 0;                                                                                                            
    en_geop.data_len = 0;                                                                                                           
    en_geop.w_adr = (char *)-1;                                                                                                     
    if ((ioctl(fd_dev,K_ENABLE,&en_geop)) == -1)                                                                                    
    {                                                                                                                               
	 plot_msg(ENA_QERR,list[1],dummyp);
         error_exit(EXITFATAL);                                                                                                            
    }                                                                                                                               
    if ((strcmp(list[2],"-statusfile")) == 0) 
        speed = 9600;  
    else 
        speed = atoi(list[2]);                                                                                                     
                                                                                                                                    
    /*------------------------------------------------------------*/
    /* verify options passed to backend                           */
    /* Check frames count parm. No longer restrict # frames to 3. */
    /*------------------------------------------------------------*/
    sprintf(init,"IN;");                                                                                                            
    i = 0;                                                                                                                          
    while (i == 0)                                                                                                                  
    {                                                                                                                               
         if (*list[nm_indx] == '-')                                                                                                 
         {                                                                                                                          
              if ((strcmp(list[nm_indx],"-noin")) == 0)                                                                             
              {                                                                                                                     
                   nm_indx += 1;                                                                                                    
                   noinit = 1;                                                                                                      
                   sprintf(init,"   ");                                                                                             
              }                                                                                                                     
	      else if ((strncmp(list[nm_indx],"-fr=",4)) == 0)
	      {
		   /*-----------------------------------------------------------*/
		   /* If "-fr=" is used, set frame count equal to the number    */
		   /*     that follows it. (removed restriction of <= 3)        */
		   /*-----------------------------------------------------------*/
		   fr = atoi(list[nm_indx]+4);          /* convert # to int     */
		   nm_indx += 1;                        /* prepare for next parm*/
	      }
	      else
	      {
		   plot_msg(BAD_OPTION,
                            list[1],list[nm_indx]);                                                                                 
		   error_exit(EXITBAD);
	      }
         }                                                                                                                          
         else                                                                                                                       
              i += 1;                                                                                                               
    }                                                                                                                               
    frcnt = fr;                                                                                                                     
                                                                                                                                    
    /* set up buffer with port initialization and   */                                                                              
    /* write to 5080                                */                                                                              
    if ((plot_int(wk_buf,speed)) == -1)
    {                                                                                                                               
	 plot_msg(BAD_SPEED,list[2],dummyp);
         error_exit(EXITBAD);                                                                                                            
    }                                                                                                                               
    if ((wr_plot(wk_buf,34)) == -1)                                                                                                 
    {                                                                                                                               
	 plot_msg(WR_PORT,list[1],dummyp);
         error_exit(EXITFATAL);                                                                                                            
    }                                                                                                                               
    /* check status of RS232 port                   */                                                                              
    if ((ck_status(wk_buf)) == -1)                                                                                                  
         error_exit(EXITFATAL);                                                                                                            
                                                                                                                                    
    /* check the condition of the plotter           */                                                                              
    while ((ck_plot()) != 0) {                                                                                                      
         log_status(WAITING);                                                                                                       
         sleep(5);                                                                                                                  
         logm = 1;                                                                                                                  
         log_status(RUNNING);                                                                                                       
    }                                                                                                                               
    logm = 0;                                                                                                                       
                                                                                                                                    
    /* get installed memory in plotter              */                                                                              
    sprintf(&dev[16],"%c.L",0x1b);                                                                                                  
    if ((plot_wr(fd_dev,dev,3)) == -1)                                                                                              
         error_exit(EXITFATAL);                                                                                                            
    for (i=0; i<20; i++)                                                                                                            
         dev[i] = 0x00;                                                                                                             
    if ((plot_rd(fd_dev,dev,20)) == -1)                                                                                             
         error_exit(EXITFATAL);                                                                                                            
    options.memsize = atoi(dev);                                                                                                    
    switch (options.memsize)                                                                                                        
    {                                                                                                                               
    case 60:                                                                                                                        
         sprintf(bufsz,"50");                                                                                                       
         bufcnt = 40;                                                                                                               
         break;                                                                                                                     
    case 255:                                                                                                                       
         sprintf(bufsz,"250");                                                                                                      
         bufcnt = 240;                                                                                                              
         break;                                                                                                                     
    default:                                                                                                                        
         sprintf(bufsz,"490");                                                                                                      
         bufcnt = 480;                                                                                                              
         break;                                                                                                                     
    }                                                                                                                               
    /* get storage for a read buffer plus header    */                                                                              
    /* to hold graphic orders to drive RS232 port   */                                                                              
    if ((buf = (char *) malloc(bufcnt+26)) == NULL)                                                                                 
    {                                                                                                                               
	 telluser(catgets(catd,PSLA_MISC_MSG, PSLA_NO_STORAGE, PSLA_DFLT_MSG));
         error_exit(EXITBAD);                                                                                                           
    }                                                                                                                               
                                                                                                                                    
    /*      build handshake ENQ/ACK                 */                                                                              
    sprintf(buf+16,"%c.R%c.@;2:%c.M010;;;10:%c.I%s;5;10:%sOE;",                                                                     
        0x1b,0x1b,0x1b,0x1b,bufsz,init);                                                                                            
                                                                                                                                    
    /*  write handshake protocol to plotter         */                                                                              
    signal(SIGALRM,no_response);                                                                                                    
    alarm(50);                                                                                                                      
    if ((plot_wr(fd_dev,buf,strlen(&buf[16]))) == -1)                                                                               
    {                                                                                                                               
	 plot_msg(WR_PORT,list[1],dummyp);
         error_exit(EXITFATAL);                                                                                                            
    }                                                                                                                               
                                                                                                                                    
    plot_rd(fd_dev,dev,20);                                                                                                         
    signal(SIGALRM,SIG_IGN);                                                                                                        
    /*  clear RS232 buffers                         */                                                                              
    /* get and save plotter options                 */                                                                              
    sprintf(wk_buf+16,"OO;");                                                                                                       
    if ((plot_wr(fd_dev,wk_buf,3)) == -1)                                                                                           
         error_exit(EXITFATAL);                                                                                                            
    if ((plot_rd(fd_dev,wk_buf,20)) == -1)                                                                                          
         error_exit(EXITFATAL);                                                                                                            
    options.roll_feed = wk_buf[0];                                                                                                  
    options.pen_select = wk_buf[2];                                                                                                 
    options.arc_circle = wk_buf[9];                                                                                                 
    options.poly_fill = wk_buf[10];                                                                                                 
    options.u_ram = wk_buf[14];                                                                                                     
    if (((options.roll_feed == '0') || (options.roll_feed == '2')) &&                                                               
        (fr > 1))                                                                                                                   
    {                                                                                                                               
	 plot_msg(BAD_FR,dummyp,dummyp);
         error_exit(EXITBAD);                                                                                                            
    }                                                                                                                               
                                                                                                                                    
    for(i=nm_indx; i<argc; i++)                                                                                                     
    {                                                                                                                               
         logm = 0;
         while ((ck_plot()) != 0) { 
            log_status(WAITING);                                                                                                       
            sleep(5);                                                                                                                  
            logm = 1;                                                                                                                  
            log_status(RUNNING);                                                                                                       
         }                                                                                                                               
         logm = 0;                                                                                                                       
         for(j=0; j<fr; j++)                                                                                                        
         {                                                                                                                          
              /* open file containing plotter commands        */                                                                    
	      if ((fd = open(list[i],O_RDONLY)) == -1)
              {                                                                                                                     
		   plot_msg(OPEN_FILE,list[i],dummyp);
                   error_exit(EXITBAD);                                                                                                  
              }                                                                                                                     
              file_index = i;                                                                                                       
              flags.eof = FALSE;                                                                                                    
              switch (j)                                                                                                            
              {                                                                                                                     
              case 0 :                                                                                                              
                   /* Set plotter origin to top left        */                                                                      
                   if ((ct = plot_or(fd_dev,noinit,fr,wk_buf)) == -1)                                                               
                   {                                                                                                                
			plot_msg(ORG_ERR,list[1],dummyp);
                        error_exit(EXITFATAL);                                                                                             
                   }                                                                                                                
                   /* determine if we have a large or small plotter */                                                              
                   if (( ct == 1) && (bufcnt > 255))                                                                                
                        options.plot_size = 'l';                                                                                    
                   else                                                                                                             
                        options.plot_size = 's';                                                                                    
                   break;                                                                                                           
              default :                                                                                                             
                   break;                                                                                                           
              }                                                                                                                     
              drive_plot(bufcnt,fr,frcnt,dev,noinit,wk_buf);                                                                        
              if (flags.nonr_fnd == FALSE)                                                                                          
                   end_plot(frcnt,wk_buf);                                                                                          
              close(fd);                                                                                                            
         }                                                                                                                          
    }                                                                                                                               
    free(buf);                                                                                                                      
    close(fd_dev);                                                                                                                  
    return(EXITOK);
}                                                                                                                                   
                                                                                                                                    
/***************************************************************/                                                                   
/* find_nr                                                     */                                                                   
/*                                                             */                                                                   
/* Search for the NR plotter command                           */                                                                   
/*                                                             */                                                                   
/***************************************************************/                                                                   
char * find_nr()                                                                                                                    
{                                                                                                                                   
    char *n_ptr;                                                                                                                    
                                                                                                                                    
    if (((n_ptr = (char *)strchr(buf+16,'n')) != NULL) ||                                                                           
        ((n_ptr = (char *)strchr(buf+16,'N')) != NULL))                                                                             
    {                                                                                                                               
         if (((strncmp(n_ptr,"nr;",3)) == 0) ||                                                                                     
             ((strncmp(n_ptr,"NR;",3)) == 0))                                                                                       
              return(n_ptr);                                                                                                        
    }                                                                                                                               
    return(NULL);                                                                                                                   
}                                                                                                                                   
/***************************************************************/                                                                   
/* drive_plot                                                  */                                                                   
/*                                                             */                                                                   
/* Gets file status to obtain file size in bytes. Reads        */                                                                   
/* from the file into the buffer provided. Searches the        */                                                                   
/* buffer for the plotter NR command indicating end of         */                                                                   
/* plotter figure. Adjusts the file pointer to point beyond    */                                                                   
/* the NR;. Places ENQ character at end of buffer and          */                                                                   
/* writes buffer to 5080. Waits for plotter acknowledgement.   */                                                                   
/* If no ack for 60 seconds, issue a message for operator      */                                                                   
/* to check device. Call end_plot routine when end of file     */                                                                   
/* or plotter figure detected.                                 */                                                                   
/*                                                             */                                                                   
/***************************************************************/                                                                   
int drive_plot(bufcnt,fr,frcnt,dev,noinit,wk_buf)                                                                                   
char dev[], wk_buf[];                                                                                                               
int bufcnt,fr,frcnt;                                                                                                                
short noinit;                                                                                                                       
{                                                                                                                                   
    void no_response();                                                                                                              
                                                                                                                                    
    double bytes_done;                                                                                                              
    int percent, tot_bytes, rd_cnt, dif, ct;                                                                                        
    struct stat filestat;                                                                                                           
    char *fnd, ack;                                                                                                                 
                                                                                                                                    
    ack = 0x00;                                                                                                                     
    if ((fstat(fd,&filestat)) == -1)                                                                                                
    {                                                                                                                               
	 plot_msg(STAT_ERR,list[1],dummyp);
         error_exit(EXITBAD);                                                                                                            
    }                                                                                                                               
    tot_bytes = filestat.st_size;                                                                                                   
    while((rd_cnt = read(fd,buf+16,bufcnt)) >0)                                                                                     
    {                                                                                                                               
         dif = 0;                                                                                                                   
         buf[rd_cnt+16] = (char)NULL;                                                                                                     
         if ((fnd = (char *)find_nr()) != NULL)                                                                                     
         {                                                                                                                          
              *fnd =0x05;                                                                                                           
              dif = rd_cnt - ((fnd - (buf+16)) +3);                                                                                 
              rd_cnt = fnd - (buf+16);                                                                                              
              filestat.st_size -= rd_cnt +3;                                                                                        
              if (filestat.st_size < 10)                                                                                            
                   flags.eof = TRUE;                                                                                                
              else                                                                                                                  
                   lseek(fd,-dif,1);                                                                                                
              flags.nr_fnd = TRUE;                                                                                                  
         }                                                                                                                          
         else                                                                                                                       
         {                                                                                                                          
              filestat.st_size -= rd_cnt;                                                                                           
              buf[rd_cnt+16] = 0x05;                                                                                                
         }                                                                                                                          
         if ((plot_wr(fd_dev,buf,rd_cnt+1)) == -1)                                                                                  
              break;                                                                                                                
         signal(SIGALRM,no_response);                                                                                               
         alarm(180);                                                                                                                
         if ((ct = plot_rd(fd_dev,&ack,1)) == -1)                                                                                   
         {                                                                                                                          
	      plot_msg(RD_FILE,list[1],dummyp);
              error_exit(EXITFATAL);                                                                                                       
         }                                                                                                                          
         if (ct > 0)                                                                                                                
         {                                                                                                                          
              if (ack == 0x0a)                                                                                                      
                   signal(SIGALRM,SIG_IGN);                                                                                         
         }                                                                                                                          
         else                                                                                                                       
         {                                                                                                                          
	      plot_msg(RD_FILE,list[1],dummyp);
              error_exit(EXITFATAL);                                                                                                       
         }                                                                                                                          
         /* update percent of file processed         */                                                                             
         percent = ((bytes_done += rd_cnt) / tot_bytes) * 100;                                                                      
         log_progress(1,percent);                                                                                                   
                                                                                                                                    
         if (flags.nr_fnd == TRUE)                                                                                                  
         {                                                                                                                          
              end_plot(frcnt,wk_buf);                                                                                               
              flags.nonr_fnd = TRUE;                                                                                                
              if ((options.plot_size == 's') &&                                                                                     
                  (flags.eof == FALSE))                                                                                             
              {                                                                                                                     
		   plot_msg(CONTINUE,list[1],dummyp);
                   logm = 1;                                                                                                        
                   while ((ck_plot()) != 0) {                                                                                       
                        log_status(WAITING);                                                                                        
                        sleep(5);                                                                                                   
                        log_status(RUNNING);                                                                                        
                   }                                                                                                                
                   logm = 0;                                                                                                        
              }                                                                                                                     
              if ((cancel == 0) &&                                                                                                  
                  (flags.eof == FALSE))                                                                                             
              {                                                                                                                     
                   if ((plot_or(fd_dev,noinit,fr,wk_buf)) == -1)                                                                    
                   {                                                                                                                
			plot_msg(ORG_ERR,list[1],dummyp);
                        error_exit(EXITFATAL);                                                                                             
                   }                                                                                                                
              }                                                                                                                     
              flags.nr_fnd = FALSE;                                                                                                 
         }                                                                                                                          
    }                                                                                                                               
}                                                                                                                                   
                                                                                                                                    
/***************************************************************/                                                                   
/* wr_plot                                                     */                                                                   
/*                                                             */                                                                   
/* write 5080 RS232 initialization to 5085                     */                                                                   
/*                                                             */                                                                   
/***************************************************************/                                                                   
int wr_plot(str,len)                                                                                                                
char *str;                                                                                                                          
int len;                                                                                                                            
{                                                                                                                                   
    char q_element[40];                                                                                                             
    struct rwparms wrtx;                                                                                                            
                                                                                                                                    
    wrtx.start = FALSE;                                                                                                             
    wrtx.stop = TRUE;                                                                                                               
    wrtx.async_io = FALSE;                                                                                                          
    wrtx.adr.dlb_adr = 0;                                                                                                           
    if ((writex(fd_dev,str,len,&wrtx)) == -1)                                                                                       
         return(-1);                                                                                                                
                                                                                                                                    
    if ((ioctl(fd_dev,G_SBF_START,0)) == -1)                                                                                        
            return(-1);                                                                                                             
                                                                                                                                    
    if ((ioctl(fd_dev,K_WAIT,q_element)) == -1)                                                                                     
         return(-1);                                                                                                                
    return(0);                                                                                                                      
}                                                                                                                                   
                                                                                                                                    
/***************************************************************/                                                                   
/* end_plot                                                    */                                                                   
/*                                                             */                                                                   
/* End of plotter commands. Do special processing based        */                                                                   
/* on plotter size and type of plot requested.                 */                                                                   
/*                                                             */                                                                   
/***************************************************************/                                                                   
int end_plot(frcnt,wk_buf)                                                                                                          
int frcnt;                                                                                                                          
char wk_buf[];                                                                                                                      
{                                                                                                                                   
                                                                                                                                    
    if (options.plot_size == 'l')                                                                                                   
    {                                                                                                                               
         if (frcnt == 1)                                                                                                            
         {                                                                                                                          
              if ((options.roll_feed == '1') ||                                                                                     
                  (options.roll_feed == '3'))                                                                                       
              {                                                                                                                     
                   sprintf(wk_buf+16,"SP0;EC;AF;OE;");                                                                              
                   plot_wr(fd_dev,wk_buf,strlen(wk_buf+16));                                                                        
                   plot_rd(fd_dev,wk_buf,20);                                                                                       
              }                                                                                                                     
              else                                                                                                                  
              {                                                                                                                     
                   sprintf(wk_buf+16,"SP0;NR;"); 
                   plot_wr(fd_dev,wk_buf,strlen(wk_buf+16));                                                                        
                   if (cancel == 0)                                                                                                 
                   {                                                                                                                
                        logm = 1;                                                                                                   
                        log_status(WAITING);                                                                                        
                        while ((ck_plot()) != VIEW)                                                                              
                             sleep(2);        
			plot_msg(VIEW,list[1],dummyp);
                        logm = 1;  
                        while ((ck_plot()) != 0)                                                                                 
                             sleep(5);                                                                                              
                        logm = 0;                                                                                                   
                   }                                                                                                                
              }                                                                                                                     
         }                                                                                                                          
         else                                                                                                                       
         {                                                                                                                          
              sprintf(wk_buf+16,"FR;");                                                                                             
              frcnt -= 1;                                                                                                           
              plot_wr(fd_dev,wk_buf,strlen(wk_buf+16));                                                                             
         }                                                                                                                          
    }                                                                                                                               
    else                                                                                                                            
    {                                                                                                                               
         sprintf(wk_buf+16,"PU0,0;SP0;OE;");                                                                                        
         plot_wr(fd_dev,wk_buf,strlen(wk_buf+16));                                                                                  
         plot_rd(fd_dev,wk_buf,20);                                                                                                 
	 plot_msg(REMOVE_PAPER,list[1],dummyp);
         logm = 1;                                                                                                                  
         log_status(WAITING);                                                                                                       
         while ((ck_plot()) == 0)                                                                                                   
              sleep(2);                                                                                                             
         logm = 0;                                                                                                                  
    }                                                                                                                               
}                                                                                                                                   
                                                                                                                                    
/***************************************************************/                                                                   
/* ck_status                                                   */                                                                   
/*                                                             */                                                                   
/* clear RS232 errors and abort any pending graphic orders     */                                                                   
/* wait 10 seconds for plotter to respond                      */                                                                   
/*                                                             */                                                                   
/***************************************************************/                                                                   
int ck_status(wk_buf)                                                                                                               
char wk_buf[];                                                                                                                      
{                                                                                                                                   
    void time_out();                                                                                                                 
    int code;                                                                                                                       
                                                                                                                                    
    signal(SIGALRM,time_out);                                                                                                       
    alarm(50);                                                                                                                      
    sprintf(wk_buf+16,"%c.J%c.K%c.E",0x1b,0x1b,0x1b);                                                                               
    if ((plot_wr(fd_dev,wk_buf,strlen(wk_buf+16))) == -1)                                                                           
         return(-1);                                                                                                                
    if ((plot_rd(fd_dev,wk_buf,3)) ==-1)                                                                                            
         return(-1);                                                                                                                
    signal(SIGALRM,SIG_IGN);                                                                                                        
    code = atoi(wk_buf[0]);                                                                                                         
    if (code != 0 && code == 1)                                                                                                     
    {                                                                                                                               
         wk_buf[2] = 0x00;                                                                                                          
         plot_msg(RS232_ERROR,list[1],wk_buf);                                                                                 
         return(-1);                                                                                                                
    }                                                                                                                               
    return(0);                                                                                                                      
}                                                                                                                                   
/***********************************************************/                                                                       
/* ck_plot                                                 */                                                                       
/*                                                         */                                                                       
/* Check condition of plotter and issue message to user    */                                                                       
/* if plotter is not ready                                 */                                                                       
/*                                                         */                                                                       
/***********************************************************/                                                                       
int ck_plot ()                                                                                                                      
{                                                                                                                                   
    char ck_buf[30];                                                                                                                
    int err_cd,msgnum;                                                                                                              
    typedef struct x                                                                                                                
    {                                                                                                                               
         unsigned resv: 9;                                                                                                          
         unsigned fld6: 1;                                                                                                          
         unsigned fld5: 1;                                                                                                          
         unsigned fld4: 1;                                                                                                          
         unsigned fld3: 1;                                                                                                          
         unsigned fld2: 1;                                                                                                          
         unsigned fld1: 1;                                                                                                          
         unsigned fld0: 1;                                                                                                          
    };                                                                                                                              
    struct x status_byte;                                                                                                           
    short int *stptr;                                                                                                               
                                                                                                                                    
    sprintf(ck_buf+16,"%c.O",0x1b);                                                                                                 
    if ((plot_wr(fd_dev,ck_buf,strlen(ck_buf+16))) == -1)                                                                           
         return(-1);                                                                                                                
    if (((plot_rd(fd_dev,ck_buf,4))) == -1)                                                                                         
         return(-1);                                                                                                                
    err_cd = atoi(ck_buf);                                                                                                          
    stptr = (short int *)&status_byte;                                                                                              
    *stptr = err_cd;                                                                                                                
    msgnum = 0;                                                                                                                     
    if (status_byte.fld4 == 1) 
         msgnum = VIEW;                                                                                                             
    if (status_byte.fld5 == 1)
         msgnum = NOPAPER;                                                                                                          
    if (status_byte.fld6 == 1) 
         msgnum = CLOSE;     

    if (msgnum == CLOSE && status_byte.fld5 == 1)
         msgnum = NOPAPER_CLOSE;                                                                                                    
    if (msgnum == CLOSE && status_byte.fld4 == 1)   
         msgnum = CLOSE_VIEW;                                                                                                       
    if (msgnum == 0)                                                                                                                
         logm = 1;                                                                                                                  
    if (logm == 0)                                                                                                                  
         plot_msg(msgnum,list[1],&err_cd);                                                                                   
    return(msgnum);                                                                                                                 
}                                                                                                                                   
                                                                                                                                    
/***********************************************************/                                                                       
/* time_out                                                */                                                                       
/*                                                         */                                                                       
/* send time out message to operator                       */                                                                       
/*                                                         */                                                                       
/***********************************************************/                                                                       
void time_out(sig_type)                                                                                                              
int sig_type;                                                                                                                       
{                                                                                                                                   
                                                                                                                                    
    if (sig_type != SIGALRM)                                                                                                        
         return;                                                                                                                    
    plot_msg(TIME_OUT,list[1],dummyp);
    error_exit(EXITBAD);                                                                                                                 
}                                                                                                                                   
                                                                                                                                    
/***********************************************************/                                                                       
/* no_response                                             */                                                                       
/*                                                         */                                                                       
/* no response from plotter while plotting a file          */                                                                       
/*                                                         */                                                                       
/***********************************************************/                                                                       
void no_response()                                                                                                                   
{                                                                                                                                   
                                                                                                                                    
    plot_msg(NO_RESPONSE,list[1],dummyp);
    return;
}                                                                                                                                   
                                                                                                                                    
/***********************************************************/                                                                       
/* die                                                     */                                                                       
/*                                                         */                                                                       
/* Process SIGTERM signal received from cancel             */                                                                       
/*                                                         */                                                                       
/***********************************************************/                                                                       
void die(sig_type)                                                                                                                   
int sig_type;                                                                                                                       
{                                                                                                                                   
    char work_buf[30];                                                                                                              
    int len;                                                                                                                        
                                                                                                                                    
    ioctl(fd_dev,G_STOP);                                                                                                           
    cancel = 1;                                                                                                                     
    if (options.plot_size != ' ')                                                                                                   
    {                                                                                                                               
         sprintf(work_buf+16,"%c.J%c.K;",0x1b,0x1b);                                                                                
         plot_wr(fd_dev,work_buf,strlen(work_buf+16));                                                                              
         end_plot(1,work_buf);                                                                                                      
    }                                                                                                                               
    plot_msg(CANCEL,list[file_index],dummyp);
    error_exit(EXITSIGNAL);                                                                                                                 
}                                                                                                                                   
                                                                                                                                    
/***********************************************************/                                                                       
/* error_exit                                              */                                                                       
/*                                                         */                                                                       
/* common error exit                                       */                                                                       
/*                                                         */                                                                       
/***********************************************************/                                                                       
int error_exit(er_cd)                                                                                                               
int er_cd;                                                                                                                          
{                                                                                                                                   
    int rc;
    char msgbuf[BUFSIZ];
    if (fd_dev != 0)                                                                                                                
         close(fd_dev);                                                                                                             
    if (buf != NULL)                                                                                                                
         free(buf);                                                                                                                 
    if (fd != 0)                                                                                                                    
         close(fd);                                                                                                                 
    rc = catclose(catd);        /* close NLS message catalog    */
    if (rc == -1)
    {
	sprintf(msgbuf,
		catgets(catd,PSLA_MISC_MSG, PSLA_CLOSEMSG, PSLA_DFLT_MSG));
	telluser(msgbuf);
    }
    exit(er_cd);                                                                                                                    
}                                                                                                                                   
                                                                                                                                    
                                                                                                                                    
