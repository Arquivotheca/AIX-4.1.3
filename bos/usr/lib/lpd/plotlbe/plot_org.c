static char sccsid[] = "@(#)33	1.1  src/bos/usr/lib/lpd/plotlbe/plot_org.c, cmdplot, bos411, 9428A410j 4/16/91 18:12:22";
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

/**********************************************************************/
/*                                                                    */
/* Name: PLOT_ORG                                                     */
/*                                                                    */
/* Date: 09/03/85                                                     */
/*                                                                    */
/* (c) Copyright 1985, IBM Corporation                                */
/*                                                                    */
/* Description:                                                       */
/*           Sets the plotter origin to top left. If handshake used   */
/*           the IN command the orgin is set at the hardclip limits.  */
/*                                                                    */
/*                                                                    */
/* Return: modules returns 0 if small plotter, 1 if large plotter or  */
/*         -1 if error detected                                       */
/*                                                                    */
/* 03/91 CM changed frcnt case stmt to formula to allow for any       */
/*          number of frames                                          */
/*                                                                    */
/**********************************************************************/
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
int plot_org(fd_tty,noinit,frcnt)
int fd_tty;
short noinit;
int frcnt;
{
	int cnt,xmin,ymin,xmax,ymax,i, psize;
	char plot_buf[45];
	char xstr[8];
	char ystr[8];
	char xmin_str[8];
	char ymin_str[8];
	char xmax_str[8];
	char ymax_str[8];
	char *cptr;
 
	psize = 0;
	cptr = plot_buf+2;
	/* output p1 and p2 limits                           */
	sprintf(plot_buf,"OP;");
	if ((cnt = write(fd_tty,plot_buf,strlen(plot_buf))) == -1)
		return(-1);
	if ((cnt = read(fd_tty,plot_buf+2,45)) == -1)
		return(-1);
 
	/* if large plotter and IN command executed get plotter*/
	/*  hard clip limits otherwise use soft clip limits    */
	if (plot_buf[2] == '-')
	{
		psize = 1;
		cptr += 1;
		if (noinit == 0)
		{
			sprintf(plot_buf,"OH;");
			if ((cnt = write(fd_tty,plot_buf,strlen(plot_buf))) == -1)
				return(-1);
			if ((cnt = read(fd_tty,plot_buf+2,45)) == -1)
				return(-1);
		}
 
		/* convert string limits to integers      */
		plot_buf[cnt+1] = ',';
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
			cptr = (char *)((strchr(cptr,',')) +1);
			if (*cptr == '-')
				cptr += 1;
		}
 
		plot_buf[cnt+1] = ';';
		plot_buf[cnt+2] = NULL;
		/* build an IP command                    */
		plot_buf[0] = 'I';
		plot_buf[1] = 'P';
 
		/* if long axis plot adjust xmin value    */
		/* and write IP command to the plotter    */
		if (frcnt > 1)
		{
                /* 03/91 allow for extended long-axis           */
                        itoa((xmin*(2*frcnt-1)),xmin_str);
			itoa(ymin,ymin_str);
			itoa(xmax,xmax_str);
			itoa(ymax,ymax_str);
			sprintf(plot_buf+2,"-%s,-%s,%s,%s;",xmin_str,
			    ymin_str,xmax_str,ymax_str);
		}
		if ((cnt = write(fd_tty,plot_buf,strlen(plot_buf))) == -1)
			return(-1);
 
		/* convert xmax and ymax values from integers */
		/* to string, place in SC aommand and write   */
		itoa((((xmax+xmin)*frcnt)),xstr);
		itoa((ymax+ymin),ystr);
		sprintf(plot_buf,"SC0,%s,0,%s;",xstr,ystr);
		if ((cnt = write(fd_tty,plot_buf,strlen(plot_buf))) == -1)
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
	}	while ((num /=10) > 0);
	str[i] = '\0';
	for (i=0, j=strlen(str)-1; i<j; i++, j--)
	{
		c = str[i];
		str[i] = str[j];
		str[j] = c;
	}
}
