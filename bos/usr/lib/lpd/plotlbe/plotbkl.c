static char sccsid[] = "@(#)29	1.3  src/bos/usr/lib/lpd/plotlbe/plotbkl.c, cmdplot, bos411, 9428A410j 2/22/93 19:04:03";
/*
 * COMPONENT_NAME: (CMDPLOT) ported RT plotter code
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*********************************************************************/
/*                                                                   */
/* Module Name : PLOTBKL                                             */
/*                                                                   */
/* Copyright 1985, IBM Corporation                                   */
/*                                                                   */
/* Date: 09/03/85                                                    */
/*                                                                   */
/* Description:                                                      */
/*        PLOTBKL  is the BACKEND PROGRAM which provides support for */
/*        plotters attached to a SAILBOAT in a SOLO environment.     */
/*        Module 'opens' the tty port passed as a parameter by       */
/*        the qdaemon, verifies any parameters entered with PRINT    */
/*        command. Plotter id, memory size and options is gathered   */
/*        and saved in the options structure. The current status of  */
/*        plotter is then checked. A message is issued to user if    */
/*        plotter is not ready and module waits for corrective       */
/*        action. The ENQ/ACK handshake is created from information  */
/*        gathered and sent to plotter. ORGPLOT is called to set     */
/*        the origin of the plotter to top left(plotter files must   */
/*        use positive offsets only). The plotter file is 'opened'   */
/*        and plotter commands read into a buffer. The ENQ           */
/*        character is placed at end of buffer and buffer sent to    */
/*        plotter. Module then waits for ACK before sending another  */
/*        buffer of data.                                            */
/*                                                                   */
/* SUBROUTINES:                                                      */
/*    WR_PLOT - Writes a string of data to the tty port.             */
/*    RD_PLOT - Reads data from RS232 port into a buffer             */
/*    GET_OPT - Obtains plotter information(options and id)          */
/*    PORT_DEF - Saves current tty settings and sets them to         */
/*               to support plotters                                 */
/*    DRIVE_PLOT - Reads plotter commands from a file and writes     */
/*               them to the tty port, waits for ack                 */
/*    FIND_NR -  Search for NR plotter command                       */
/*    END_PLOT - Ends plotting by moving media to next sheet if      */
/*               roll feed installed otherwise places plotter        */
/*               in VIEW state or waits for operator to remove       */
/*                media(small plotters only).                        */
/*    CK_STATUS - Resets RS232 errors and waits(10 seconds) for      */
/*                plotter to respond                                 */
/*    CK_PLOT - Checks the current state of the plotter and          */
/*              issues a message to user if plotter is not ready     */
/*    TIME_OUT - Issues a message to operator if plotter fails       */
/*               to respond whitin 10 seconds                        */
/*    NO_RESPONSE - Issues a message to operator after 60 seconds    */
/*                  have elapsed and plotter has not sent an         */
/*                  acknowledgement while plotting a file            */
/*    DIE - Catches cancel signal and sends a message indicating     */
/*          job is being canceled                                    */
/*                                                                   */
/* Include File:                                                     */
/*   PLOT_ORG.C: Sets the orgin of the plotter                       */
/*   PLOT_MSG.C: formulates and issues message to user or for qdaemon*/
/*   PLOT_DEF.H: defines for plotter support                         */
/*                                                                   */
/* Modifications:                                                    */
/*                                                                   */
/*    03/20/91 CM  - init nm_index to 2 because AIX V3 qdaemon       */
/*                   no longer passes -statusfile parameter          */
/*                 - remove '-fr=x' case statement to allow for      */
/*                   up to 9 long-axis frames, also add check        */
/*                   for -fr=0                                       */
/*                 - no longer terminate for unrecognized parms,     */
/*                   issue message and continue                      */
/*                 - change all fprintf(ter_fp) and log_message      */
/*                   calls to telluser(), delete first parm from     */
/*                   all plot_msg calls (this was ter_fp)            */
/*                 - for 6182 plotter re-send esc.@ to allow for     */
/*                   correct handshaking                             */
/*                 - in end_plot : add support for 6182              */
/*                   autofeed (PG; command)                          */
/*                 - in drive_plot : don't issue continue msg        */
/*                   if 6182 in autofeed and multi-page plot         */
/*                 - in ck_status : fix esc.E return code checking   */
/*                                                                   */
/*********************************************************************/
 
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <termio.h>
#include <signal.h>
#include "plot_def.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <IN/standard.h>
#define Xoff 0x36
#define Xon 0x35
#define RUNNING 2
#define WAITING 3
int fd_tty = 0;
int fd = 0;
int file_index = 3;
int logm  = 0;
int cancel = 0;
char *list[20];
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
struct termio olddef;
struct termio portdef;
char *buf;
struct {
	unsigned nr_fnd : 1;
	unsigned nonr_fnd : 1;
	unsigned eof : 1;
} flags;
main(argc,arglist)
int argc;
char *arglist[];
{
 
	int no_response();
	int die();
 
	int ct,i,j,nm_indx,fr,frcnt,bufcnt;
	short noinit;
 
	char bufsz[4];
	char dev[20];
	char init[4];
 
	ct = 0;
	noinit = 0;
	flags.nr_fnd = FALSE;
	flags.nonr_fnd = FALSE;
 
        /* 03/91 set nm_indx to 2 because AIX V3 qdaemon doesn't pass   */
        /* -statusfile parm                                             */
	nm_indx = 2;
	fr = 1;
	options.model[0] = ' ';

	for(i=0; i<argc; i++)
            {
		list[i] = arglist[i];
          /*      printf("option %d : %s\n",i,list[i]);    */
            }

	signal(SIGTERM,die);
	log_init();
 
	/* open path thru tty port  to plotter          */
	sprintf(dev,"/dev");
	strcpy(&dev[4],list[1]++);
	dev[4] = '/';
	if ((fd_tty = open(dev,O_RDWR)) == -1)
	{
		plot_msg(OPEN_PORT,list[1]);
		error_exit(EXITBAD);
	}
	port_def();
 
	/* check condition of RS232 port and that we    */
	/* can communicate with plotter - power on etc. */
	if ((ck_status()) == -1)
		error_exit(EXITBAD);
 
	/* verify and save parameters passed    */
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
			else
                                /* 03/91 allow up to 9 frames               */

				if ((strncmp(list[nm_indx],"-fr=",4)) == 0) 
                                {
                                      fr = atoi(list[nm_indx]+4);
				      nm_indx += 1;
                                }
				else
				{
					plot_msg(BAD_OPTION,
					         list[1],list[nm_indx]);
				/* 03/91 don't end if unrecognized parm   */
				/*	error_exit(EXITBAD);                   */
				}
		}
		else
			i += 1;
	}
     
        /* 03/91 do not allow -fr=0 to be passed, default to one frame */
        if (fr < 1) fr = 1;
	frcnt = fr;
        /*   printf("Number of frames = %d\n",frcnt);     */
 
	/* check if plotter is ready, if not wait then check   */
	while ((ck_plot()) != 0) {
		log_status(WAITING);
		sleep(5);
		logm = 1;
		log_status(RUNNING);
	}
	logm = 0;
 
	/*   get installed memory in plotter            */
	sprintf(dev,"%c.L",0x1b);
	if ((wr_plot(dev)) == -1)
		error_exit(EXITERROR);
	for (j=0; j<20; j++)
		dev[j] = 0x00;
	if ((rd_plot(dev,20)) == -1)
		error_exit(EXITERROR);
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
 
	/* get storage to read plotter commands into           */
	if ((buf = (char *) malloc(bufcnt+10)) == NULL)
	{
		telluser("Unable to obtain storage\n");
		error_exit(EXITBAD);
	}
 
	/*      build ENQ/ACK handshake and send to plotter */
	sprintf(buf,"%c.J%c.K%c.@;2:%c.M010;;;10:%c.I%s;5;10:%s",0x1b,0x1b,
	    0x1b,0x1b,0x1b,bufsz,init);

	if ((wr_plot(buf)) == -1)
		error_exit(EXITBAD);
 
	/*  get result of OE command and ignore         */
        sprintf(buf,"OE;");
        wr_plot(buf);
	signal(SIGALRM,no_response);
	alarm(10);
	rd_plot(dev,20);
	signal(SIGALRM,SIG_IGN);
 
	/* get and save plotter options and model        */
	if ((get_opt(dev)) == -1)
		error_exit(EXITBAD);

        /* 03/91 if 6182 plotter the re-issue esc.@ for handshaking        */
        if ((strncmp(options.model,"6182",4) == 0) ||
            (strncmp(options.model,"7550",4) == 0))
              {
                 sprintf(buf,"%c.@;2:",0x1b);
                 wr_plot(buf);
              }

	if (((options.roll_feed == '0') || (options.roll_feed == '2')) &&
	    (fr > 1))
	{
		plot_msg(BAD_FR);
		error_exit(EXITBAD);
	}
 
	for (i=nm_indx; i<argc; i++)
	{
		for(j=0; j<fr; j++)
		{
			/* open file containing plotter commands        */
			if ((fd = open(list[i],O_RDONLY)) == -1)
			{
				plot_msg(OPEN_FILE,list[i]);
				error_exit(EXITBAD);
			}
			file_index = i;
			flags.eof = FALSE;
			switch (j)
			{
			case 0 :
				/* set plotter origin                  */
				if ((ct = plot_org(fd_tty,noinit,fr)) == -1)
				{
					plot_msg(ORG_ERR,list[1]);
					error_exit(EXITBAD);
				}
				if ((ct == 1) && (bufcnt > 255))
					options.plot_size = 'l';
				else
					options.plot_size = 's';
				break;
			default :
				break;
			}
			drive_plot(bufcnt,fr,frcnt,dev,noinit);
			if (flags.nonr_fnd == FALSE)
				end_plot(frcnt,dev);
			close(fd);
		}
	}
	free(buf);
	ioctl(fd_tty,TCSETAF,&olddef);
	close(fd_tty);
}
 
/********************************************************************/
/* find_nr                                                          */
/*                                                                  */
/* Search for the NR plotter command                                */
/*                                                                  */
/********************************************************************/
char * find_nr()
{
	char *n_ptr;
 
	if (((n_ptr = (char *)strchr(buf,'n')) != NULL) ||
	    ((n_ptr = (char *)strchr(buf,'N')) != NULL))
	{
		if (((strncmp(n_ptr,"nr;",3)) == 0) ||
		    ((strncmp(n_ptr,"NR;",3)) == 0))
			return(n_ptr);
	}
	return(NULL);
}
 
/********************************************************************/
/* drive_plot                                                       */
/*                                                                  */
/* Gets files status to obtain file size in bytes. Reads from the   */
/* file into the buffer provided. Searches the buffer for the       */
/* letter 'N' indicating end of plot but not necessarily end of file*/
/* Adjusts the file pointer to point beyond the NR;. Places ENQ     */
/* character at end of buffer and writes the buffer to the tty port.*/
/* Waits for acknowledgement. If not acknowledgement for 60 seconds */
/* issue a message to operator to check device. Update progress and */
/* call end_plot routine if end of file or plot detected.           */
/********************************************************************/
int drive_plot(bufcnt,fr,frcnt,dev,noinit)
char dev[];
int bufcnt,fr,frcnt;
short noinit;
{
 
	int no_response();
 
	double  bytes_done;
	int  percent, tot_bytes, rd_cnt, dif, ct;
	struct stat filestat;
	char *fnd, ack;
 
	ack = 0x00;
	if ((fstat(fd,&filestat)) == -1)
	{
		plot_msg(STAT_ERR,list[1]);
		error_exit(EXITBAD);
	}
	tot_bytes = filestat.st_size;
 
	while((rd_cnt = read(fd,buf,bufcnt)) > 0)
	{
		dif = 0;
		buf[rd_cnt] = 0x00;
		if ((fnd = (char *)find_nr()) != NULL)
		{
			*fnd = 0x05;
			dif = rd_cnt - ((fnd - buf) +3);
			rd_cnt = fnd - buf;
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
			buf[rd_cnt] = 0x05;
		}
		if ((ct = write(fd_tty,buf,rd_cnt+1)) > 0) ;
		else
		{
			plot_msg(WR_PORT,list[1]);
			error_exit(EXITBAD);
		}
		/* set alarm and wait for acknowledgement   */
		ack = 0x00;
		signal(SIGALRM,no_response);
		alarm(180);
		ct = 0;
		do {
			if (((ct = read(fd_tty,&ack,1)) == -1) &&
			    (errno == 4))
				ct = 0;
		} while (ct  == 0);
		if (ct > 0)
		{
			if (ack == 0x0a)
				signal(SIGALRM,SIG_IGN);
			else
			{
				plot_msg(BAD_ACK,list[1],ack);
				error_exit(EXITBAD);
			}
		}
		else
		{
			plot_msg(RD_FILE,list[1]);
			error_exit(EXITBAD);
		}
		/* update percent of file done                */
		percent = ((bytes_done += rd_cnt) / tot_bytes) * 100;
		log_progress(1,percent);
 
		/* end plotting if end of figure detected    */
		if (flags.nr_fnd == TRUE)
		{
			end_plot(frcnt,dev);
			flags.nonr_fnd = TRUE;
			if ((options.plot_size == 's') &&
			    (flags.eof == FALSE))

            /* 03/91 for 6182 in autofeed mode, don't issue message    */
            if (!(((strncmp(options.model,"6182",4) == 0)     ||
                 (strncmp(options.model,"7550",4) == 0))    &&
                 ((options.roll_feed == '1')    ||
                  (options.roll_feed == '3'))))
			{
				plot_msg(CONTINUE,list[1]);
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
				if ((plot_org(fd_tty,noinit,fr)) == -1)
				{
					plot_msg(ORG_ERR,list[1]);
					error_exit(EXITBAD);
				}
			}
			flags.nr_fnd = FALSE;
		}
	}
}
 
/********************************************************************/
/* wr_plot                                                          */
/*                                                                  */
/* write a string terminated by a null character to a tty port      */
/*                                                                  */
/********************************************************************/
int wr_plot(str)
char *str;
{
	if (write(fd_tty,str,strlen(str)) == -1)
	{
		plot_msg(WR_PORT,list[1]);
		return(-1);
	}
	return(0);
}
 
/********************************************************************/
/* port_def                                                         */
/*                                                                  */
/* saves the current settings of the port and sets up port          */
/* parameters to support a plotter                                  */
/*                                                                  */
/********************************************************************/
int port_def()
{
	int speed;
 
	if (ioctl(fd_tty,TCGETA,&olddef) == -1)
	{
		telluser("Unable to get port parameters for %s - errno = %d\n",
		    list[1],errno);
		error_exit(EXITBAD);
	}
	speed = olddef.c_cflag & CBAUD;
	portdef.c_iflag = 0;
	portdef.c_oflag = 0;
	portdef.c_cflag = PARENB + CS7 + CLOCAL + CREAD + speed;
	portdef.c_lflag = ICANON;
	portdef.c_cc[4] = 010;
	portdef.c_cc[5] = 015;
	if (ioctl(fd_tty,TCSETAF,&portdef) == -1)
	{
		telluser("Unable to set port parameters for %s - errno = %d\n",
		    list[1],errno);
		error_exit(EXITBAD);
	}
}
/********************************************************************/
/* get_opt                                                          */
/*                                                                  */
/* get installed plotter options and plotter identification. Save   */
/* information in 'options' structure.                              */
/*                                                                  */
/********************************************************************/
int get_opt(dev)
char dev[];
{
	int i;
 
	sprintf(dev,"OO;");
	if ((wr_plot(dev)) == -1)
		error_exit(EXITBAD);
	if ((rd_plot(dev,20)) == -1)
		error_exit(EXITBAD);
	options.roll_feed = dev[0];
	options.pen_select = dev[2];
	options.arc_circle = dev[8];
	options.poly_fill = dev[10];
	options.u_ram = dev[14];
	for (i=0; i<20; i++)
		dev[i] = 0x00;
	sprintf(dev,"OI;");
	if ((wr_plot(dev)) == -1)
		return(-1);
	if ((rd_plot(dev,20)) == -1)
		return(-1);
	strncpy(options.model,dev,4);
	return(0);
}
 
/********************************************************************/
/* end_plot                                                         */
/*                                                                  */
/* called when a file has been processed and places plotter in      */
/* a state to accept the next file or waits for operator assist     */
/*                                                                  */
/********************************************************************/
int end_plot(frcnt,dev)
int frcnt;
char dev[];
{
 
	if (options.plot_size == 'l')
	{
		if (frcnt == 1)
		{
			if ((options.roll_feed == '1') ||
			    (options.roll_feed == '3'))
			{
				sprintf(dev,"SP0;EC;AF;OE;");
				wr_plot(dev);
				rd_plot(dev,20);
			}
			else
			{
				sprintf(dev,"SP0;NR;");
				wr_plot(dev);
				if (cancel == 0)
				{
					logm = 1;
					log_status(WAITING);
					while ((ck_plot()) != VIEW)
						sleep(2);
					logm = 0;
					log_status(RUNNING);
				}
			}
		}
		else
		{
			sprintf(dev,"FR;");
			frcnt -= 1;
			wr_plot(dev);
		}
	}
	else
                   /*   small plotters start here     */
	{
            /*  03/91 add PG support for 6182 autofeed plotter    */
            if (((strncmp(options.model,"6182",4) == 0)     ||
                 (strncmp(options.model,"7550",4) == 0))    &&
                 ((options.roll_feed == '1')    ||
                  (options.roll_feed == '3')))
                {
                  sprintf(dev,"SP0;PG;OE;");
                  wr_plot(dev);
                  /* wait for response from OE before ending  */
                  rd_plot(dev,20);
                }
            else
                {
		sprintf(dev,"SP0;PU0,7200;OE;");
		wr_plot(dev);
		rd_plot(dev,20);
		plot_msg(REMOVE_PAPER,list[1],dev);
		logm = 1;
		log_status(WAITING);
		while ((ck_plot()) == 0)
			sleep(2);
		logm = 0;
                }
	}
}
 
/********************************************************************/
/* ck_status                                                        */
/*                                                                  */
/* check for RS232 error conditions                                 */
/* time out in 10 seconds if plotter fails to respond               */
/*                                                                  */
/********************************************************************/
int ck_status()
{
	int time_out();
	int code;
	char stat_buf[10];
 
	signal(SIGALRM,time_out);
	alarm(10);
	sprintf(stat_buf,"%c.J%c.K%c.E",0x1b,0x1b,0x1b);
	if ((wr_plot(stat_buf)) == -1)
		return(-1);
	if ((rd_plot(stat_buf,10)) == -1)
		return(-1);
	signal(SIGALRM,SIG_IGN);
        alarm(0);

        /* 03/91 following statement was incorrect, atoi expects ptr */
	/* code = atoi(stat_buf[0]); */
	code = atoi(&stat_buf[0]);

	if (code != 0)
	{
		stat_buf[2] = 0x00;
		plot_msg(RS232_ERROR,list[1],stat_buf);
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
 
	sprintf(ck_buf,"%c.O",0x1b);
	if ((wr_plot(ck_buf)) == -1)
		return(-1);
	if ((rd_plot(ck_buf,10)) == -1)
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
 
/********************************************************************/
/* rd_plot                                                          */
/*                                                                  */
/* read plotter information into buffer provided                    */
/*                                                                  */
/********************************************************************/
int rd_plot(rbuf,ln)
char *rbuf;
int ln;
{
	int cnt ,i;
 
	if ((cnt = read(fd_tty,rbuf,ln)) < 0)
	{
		plot_msg(RD_PORT,list[1]);
		return(-1);
	}
 
    /*   telluser("read count = %d, tty data =",cnt);     */
    /*   for (i=0; i<cnt; i++)                            */
    /*   telluser("x%x",rbuf[i]);                         */
    /*   telluser("\n");                                  */

	return(0);
}
 
/********************************************************************/
/* time_out                                                         */
/*                                                                  */
/* Plotter has failed to respond within 10 seconds                  */
/*                                                                  */
/********************************************************************/
int time_out(sig_type)
int sig_type;
{
	if (sig_type != SIGALRM)
		return;
	plot_msg(TIME_OUT,list[1]);
	error_exit(EXITBAD);
}
 
/********************************************************************/
/* no_response                                                      */
/*                                                                  */
/* no response from plotter while a file was being processed        */
/*                                                                  */
/********************************************************************/
int no_response()
{
	plot_msg(NO_RESPONSE,list[1],0);
	return;
}
 
/********************************************************************/
/* die                                                              */
/*                                                                  */
/* process SIGTERM signal. user wishes to cancel the job.           */
/*                                                                  */
/********************************************************************/
int die(sig_type)
int sig_type;
{
	char work_buf[20];
 
	if (sig_type != SIGTERM)
		return;
	cancel = 1;
	if (options.model[0] != ' ')
	{
		sprintf(work_buf,"%c.J%c.K;",0x1b,0x1b);
		wr_plot(work_buf);
		end_plot(1,work_buf);
	}
	plot_msg(CANCEL,list[file_index],0);
	error_exit(EXITSIGNAL);
}
 
/********************************************************************/
/* error_exit                                                       */
/*                                                                  */
/* common error exit routine                                        */
/*                                                                  */
/********************************************************************/
int error_exit(er_cd)
int er_cd;
{
	if (fd_tty != 0)
	{
		close(fd_tty);
		ioctl(fd_tty,TCSETAF,&olddef);
	}
	if (buf != NULL)
		free(buf);
	if (fd != 0)
		close(fd);
	exit(er_cd);
}
