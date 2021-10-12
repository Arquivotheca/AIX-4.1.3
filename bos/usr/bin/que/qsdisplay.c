static char sccsid[] = "@(#)38	1.17.1.2  src/bos/usr/bin/que/qsdisplay.c, cmdque, bos411, 9428A410j 5/27/93 08:34:06";
/*
 * COMPONENT_NAME: (CMDQUE) spooling commands
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <IN/standard.h>
#include <sys/fullstat.h>
#include <sys/types.h>
#include <IN/backend.h>
#include "common.h"
#include "qstatus.h"
#include <ctype.h>
#include <stdlib.h>
#include "qstat_msg.h"
nl_catd	catd;
#define MSGSTR(num,str)	catgets(catd,MS_QSTAT,num,str)

#define MAX_STR 44
char *dispstr();

/*==== PRINT SHORT VERSION OF HEADER */
display_sheader()
{
/*	printf("Queue   Dev   Status    Job Files               User         PP  %%  Blks Cp Rnk\n"); */
	printf("%-7.7s ",MSGSTR(MSGQUE,QMSGQUE));
	printf("%-5.5s ",MSGSTR(MSGDEV,QMSGDEV));
	printf("%-9.9s ",MSGSTR(MSGSTAT,QMSGSTAT));
	printf("%3.3s ",MSGSTR(MSGJOB,QMSGJOB));
	printf("%-18.18s ",MSGSTR(MSGFILE,QMSGFILE));
	printf("%-10.10s ",MSGSTR(MSGUSER,QMSGUSER));
	printf("%4.4s ",MSGSTR(MSGPAGE,QMSGPAGE));
	printf("%%  ");
	printf("%5.5s ",MSGSTR(MSGBLKS,QMSGBLKS));
	printf("%3.3s ",MSGSTR(MSGCOPS,QMSGCOPS));
	printf("%3.3s\n",MSGSTR(MSGRANK,QMSGRANK));
	printf("------- ----- --------- --- ------------------ ---------- ---- -- ----- --- ---\n");
	return(0);
}

/*==== PRINT LONG VERSION OF HEADER */
display_vheader()
{
	printf("%1s","");
	printf("%-7.7s ",MSGSTR(MSGQUE,QMSGQUE));
	printf("%-5.5s ",MSGSTR(MSGDEV,QMSGDEV));
	printf("%-9.9s ",MSGSTR(MSGSTAT,QMSGSTAT));
	printf("%3.3s ",MSGSTR(MSGJOB,QMSGJOB));
	printf("%4.4s","");
	printf("%-14.14s ",MSGSTR(MSGNAME,QMSGNAME));
	printf("%-14.14s ",MSGSTR(MSGFROM,QMSGFROM));
	printf("%-14.14s\n",MSGSTR(MSGTO,QMSGTO));

	printf("%25s","");
	printf("%-9.9s ",MSGSTR(MSGSUBM,QMSGSUBM));
	printf("%7s","");
	printf("%3.3s ",MSGSTR(MSGRANK,QMSGRANK));
	printf("%3.3s ",MSGSTR(MSGPRIO,QMSGPRIO));
	printf("%5.5s","");
	printf("%5.5s ",MSGSTR(MSGBLKS,QMSGBLKS));
	printf("%3.3s ",MSGSTR(MSGCOPS,QMSGCOPS));
	printf("%7.7s","");
	printf("%4.4s ",MSGSTR(MSGPAGE,QMSGPAGE));
	printf("%%\n");
/*	printf(" Queue   Dev   Status    Job     Name           From           To\n"); */
/*	printf("                         Submitted         Rnk Pri       Blks Cp          PP %%\n"); */
	printf(" ------- ----- --------- ---------        --- ---      ----- ---        ---- --\n");

	return(0);
}

/*==== DISPLAY THE VERBOSE STATUS OF A SPECIFIC JOB ON A QUEUE */
display_vline(dv_dev,dv_statblk,dv_ent,dv_rank)
struct d	*dv_dev;
struct stfile	*dv_statblk;
struct e	*dv_ent;
unsigned	dv_rank;
{
	/*----Print the Queue, Device, and Status */
	printf(" ");
        display_qname(dv_dev);
        display_devname(dv_dev);
        display_stinfo(dv_dev,dv_statblk,dv_ent->e_hold);

	/*----If there is a job attached to device... */
	if (dv_ent) 
	{
		display_jobnum(dv_ent);
		printf("%4s","");
		display_reqname(dv_ent);
		display_from(dv_ent);
		display_to(dv_ent);
		printf("\n");

		/*----Next line */
		printf("%24s","");
                display_time(dv_ent);
		display_rank(dv_rank);
                display_priority(dv_ent);
		printf("%5s","");
		display_size(dv_ent);
		display_copies(dv_ent);
		printf("%7s","");
		display_pages(dv_statblk,dv_dev);
		display_percent(dv_statblk,dv_dev);
		printf("\n");

		/*----Print the files associated with this job */
		display_vjobfiles(dv_ent);
	}
	else
		printf("\n");
	return(0);
}

/*==== DISPLAY THE SHORTENED STATUS OF A SPECIFIC JOB ON A QUEUE */
display_sline(ds_dev,ds_statblk,ds_ent,ds_rank)
struct d	*ds_dev;
struct stfile	*ds_statblk;
struct e	*ds_ent;
unsigned	ds_rank;
{
	/*----Print the Queue, Device, and Status */
       	display_qname(ds_dev);
       	display_devname(ds_dev);
        display_stinfo(ds_dev,ds_statblk,ds_ent->e_hold);

	/*----If there is a job attached to device... */
	if (ds_ent) 
	{
		display_jobnum(ds_ent);
/*		display_sjobfiles(ds_ent); */
		display_sreqname(ds_ent);
                display_user(ds_ent);
		display_pages(ds_statblk,ds_dev);
		display_percent(ds_statblk,ds_dev);
		display_size(ds_ent);
		display_copies(ds_ent);
		display_rank(ds_rank);
	}
	/*----Insure new line */
	printf("\n");
	return(0);
}

/*==== DISPLAY QUEUE NAME */
display_qname(dq_dev)
struct d	*dq_dev;
{
	if(dq_dev)
		printf("%-7.7s",dq_dev->d_q->q_name);
	else
		printf("%7s","");
	return(0);
}

/*==== DISPLAY DEVICE NAME */
display_devname(dd_dev)
struct d	*dd_dev;
{
	if(dd_dev)
		printf(" %-5.5s",dd_dev->d_name);
	else
		printf(" %5s","");
	return(0);
}

/*==== DISPLAY STATUS INFORMATION FIELD FOR JOB OR DEVICE */
display_stinfo(ds_dev,ds_statblk,isheld)
struct d 	*ds_dev;
struct stfile 	*ds_statblk;
int		isheld;
{
	if (ds_dev)
	{
		if (!ds_statblk) 
			printf(" %-9.9s",MSGSTR(MSGSTUNK,QMSGSTUNK));
		else
			switch (ds_statblk->s_status) {
			case READY:
				printf(" %-9.9s",MSGSTR(MSGSTRDY,QMSGSTRDY));
				break;
			case RUNNING:
				printf(" %-9.9s",MSGSTR(MSGSTRUN,QMSGSTRUN));
				break;
			case WAITING:
				printf(" %-9.9s",MSGSTR(MSGSTDVW,QMSGSTDVW));
				break;
			case OFF:
				printf(" %-9.9s",MSGSTR(MSGSTDWN,QMSGSTDWN));
				break;
			case OPRWAIT:
				printf(" %-9.9s",MSGSTR(MSGSTOPW,QMSGSTOPW));
				break;
			case INIT:
				printf(" %-9.9s",MSGSTR(MSGSTINI,QMSGSTINI));
				break;
			case SENDING:
				printf(" %-9.9s",MSGSTR(MSGSTSEN,QMSGSTSEN));
				break;
			case GETHOST:
				printf(" %-9.9s",MSGSTR(MSGSTGET,QMSGSTGET));
				break;
			case CONNECT:
				printf(" %-9.9s",MSGSTR(MSGSTCON,QMSGSTCON));
				break;
			case BUSY:
				printf(" %-9.9s",MSGSTR(MSGSTBUS,QMSGSTBUS));
				break;
			default:
				printf(" %-9.9s",MSGSTR(MSGSTUNK,QMSGSTUNK));
			}
	}
	else if (isheld)
		printf(" %-9.9s",MSGSTR(MSGSHELD,QMSGSHELD));
	else
		printf(" %-9.9s",MSGSTR(MSGSTQUE,QMSGSTQUE));
	return(0);
}

/*==== PRINT THE JOB NUMBER */
display_jobnum(dj_ent)
struct e	*dj_ent;
{
	printf(" %3d",JOBNUM(dj_ent->e_jobnum));
	return(0);
}

/*==== PRINT THE ORIGINATION USER NAME */
display_from(df_ent)
struct e	*df_ent;
{
	char *dispname;

	if((__max_disp_width == 1) && (MB_CUR_MAX == 1))
		dispname = df_ent->e_from;
	else
  		dispname = dispstr(df_ent->e_from,14);

	printf(" %-14.14s",dispname);
	return(0);
}

/*==== PRINT THE DESTINATION USER NAME */
display_to(dt_ent)
struct e	*dt_ent;
{
	char *dispname;
	
	if((__max_disp_width == 1) && (MB_CUR_MAX == 1))
		dispname = dt_ent->e_to;
	else
		dispname = dispstr(dt_ent->e_to,14);

	printf(" %-14.14s",dispname);
	return(0);
}

/*==== PRINT THE JOB REQUEST NAME */
display_reqname(dr_ent)
struct e	*dr_ent;
{
	char *dispname;


	if ((__max_disp_width == 1) && (MB_CUR_MAX == 1))
		dispname = dr_ent->e_request;
	else
		dispname = dispstr(dr_ent->e_request,14);

	printf(" %-14.14s",dispname);

	return(0);
}

/*==== PRINT THE JOB REQUEST NAME */
display_sreqname(ds_ent)
struct e	*ds_ent;
{
	char *dispname; 

	if ((__max_disp_width == 1) && (MB_CUR_MAX == 1))
		dispname = ds_ent->e_request;
	else
		dispname = dispstr(ds_ent->e_request,18);
	printf(" %-18.18s",dispname);
	return(0);
}

/*==== DISPLAY TIME SUBMITTED */
display_time(dt_ent)
struct e	*dt_ent;
{
	char	dt_timbuf[26];

	strftime(dt_timbuf,(size_t)18,"%D %T",localtime(&dt_ent->e_time));
	printf("%17.17s " ,dt_timbuf);
	return(0);
}

/*==== PRINT THE USER NAME FOR SHORT OUTPUT */
display_user(du_ent)
struct e	*du_ent;
{
	char *dispname;

	if((__max_disp_width == 1) && (MB_CUR_MAX == 1))
		dispname = du_ent->e_to;
	else
		dispname = dispstr(du_ent->e_to,10);

	printf(" %-10.10s",dispname);
	return(0);
}

/*==== DISPLAY NUMBER OF PAGES PRINTED */
display_pages(dp_statblk,dp_dev)
struct stfile	*dp_statblk;
struct d 	*dp_dev;
{
	if ((dp_dev && dp_statblk) &&
	    (dp_statblk->s_status == RUNNING))
		printf(" %4.1d",dp_statblk->s_pages % 10000);
	else
		printf(" %4s","");
	return(0);
} 

/*==== DISPLAY PERCENTAGE PRINTED */
display_percent(dp_statblk,dp_dev)
struct stfile	*dp_statblk;
struct d	*dp_dev;
{
	if ((dp_dev && dp_statblk) &&
	    (dp_statblk->s_status == RUNNING))
		printf(" %2.1d",dp_statblk->s_percent);
	else
		printf(" %2s","");
	return(0);
}

/*==== DISPLAY NUMBER OF BLOCKS IN REQUEST */
display_size(ds_ent)
struct e	*ds_ent;
{
	printf(" %5.1d",ds_ent->e_size % 100000);
	return(0);
}

/*==== DISPLAY NUMBER OF COPIES OF JOB TO PRINT */
display_copies(dc_ent)
struct e	*dc_ent;
{
	printf(" %3.1d",dc_ent->e_nc % 1000);
	return(0);
}

/*==== DISPLAY THE JOBS RANKING */
display_rank(dr_rank)
unsigned	dr_rank;
{
	printf(" %3.1d",dr_rank);
	return(0);
}

/*==== DISPLAY THE JOBS PRIORITY */
display_priority(dp_ent)
struct e	*dp_ent;
{
	printf(" %3.1d",dp_ent->e_pri1);
	return(0);
}

/*==== PRINT THE FILES ASSOCIATED WITH A JOB (VERBOSE VERSION) */
display_vjobfiles(dv_ent)
struct e	*dv_ent;
{
	struct str_list	*dv_thisfile;
	char *dispjob;

	for(dv_thisfile = dv_ent->e_fnames;
	    dv_thisfile != NULL;
	    dv_thisfile = dv_thisfile->s_next)
	{
		if ((__max_disp_width == 1) && (MB_CUR_MAX == 1))
			dispjob = dv_thisfile->s_name;
		else 
			dispjob = dispstr(dv_thisfile->s_name,31);
		printf("%31s","");
		printf("%-48.48s\n",dispjob);
	}
	return(0);
}

/*==== PRINT THE FILES ASSOCIATED WITH A JOB (SHORT VERSION) */
display_sjobfiles(dj_ent)
struct e	*dj_ent;
{
#define DJFLEN 19
	char				*filenameonly();
	register struct str_list	*dj_thisfile;
	unsigned			dj;		/* Length total */
 	char				dj_string[DJFLEN + 1];
	char				*dj_thisname;

	/*----Scan list of filenames in job */
	dj = 0;	
	strcpy(dj_string,"");
	for(dj_thisfile = dj_ent->e_fnames;
	    dj_thisfile != NULL;
	    dj_thisfile = dj_thisfile->s_next)
	{
		/*----Extract file name only, no path */
		dj_thisname = filenameonly(dj_thisfile->s_name);

		/*----If enough room to print file name...*/
	 	if ((dj_thisfile->s_next &&
		     ((strlen(dj_thisname) + 4 + dj) <= DJFLEN)) ||
                    (!dj_thisfile->s_next &&
	             ((strlen(dj_thisname) + dj) <= DJFLEN)))
		{
			/*----Add this file name to end of string */
			strcat(dj_string,dj_thisname);

			/*----Add a comma if not last file in job */
			if (dj_thisfile->s_next) 
			{
				strcat(dj_string,",");
				dj = dj + strlen(dj_thisname) + 1;
			} /* if */
		}
		else
			/*----If more files, add dots */
			if (dj_thisfile->s_next)
			{
				strcat(dj_string,"...");
			        break;
			} /* if */
	} /* for */

	/*----Print the string we just made */
	printf(" %-19.19s",dj_string);
}

char *dispstr(str,length)
char *str;
int length;
{

	int count = 0, len, wd;
	int sum = 0;
	wchar_t wc;

	do {
		if ((len = mbtowc(&wc,&str[count],MB_CUR_MAX)) == -1)
			return(str=" ");
		else {
			wd = wcwidth(wc);
			if (wd == -1)
				return(str=" ");
			sum += wd;
		 	if (sum <= length)
				count += len;
			else{
				str[count] = '\0';
				break;
			}
		}
	}
	while((wd <= length) && (len != 0)); 
	return(str);
}
	
