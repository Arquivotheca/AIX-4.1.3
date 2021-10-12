static char sccsid[] = "@(#)30	1.12.1.6  src/bos/usr/bin/iostat/iostat.c, cmdstat, bos41B, 9504A 12/21/94 13:40:10";
/*
 * COMPONENT_NAME: (CMDSTAT) I/O Statistics command
 *
 * FUNCTIONS: iostat
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#define _ILS_MACROS
#include <ctype.h>
#include <stdio.h>
#include <nlist.h>
#include <signal.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/priv.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/iostat.h>
#include <sys/devinfo.h>
#include <sys/syspest.h>
#include <sys/m_param.h>
#include <sys/sysinfo.h>
#include <sys/sysconfig.h>
#include <sys/var.h>
#include <sys/signal.h>
#include <locale.h>

#include "iostat_msg.h"
static nl_catd catd;
#define MSGSTR(Num, Str)  catgets(catd, MS_IOSTAT, Num, Str)

static struct nlist nltbl[] = {		/* nlist table */
	{"sysinfo"},
#define SYSINFO		0
	{"iostat"},
#define IOSTAT  	1
	{"lbolt"},
#define LBOLT		2	
	{ 0 },
};

#define NUM_TBL		4

static int 	kmem;				/* file description for /dev/kmem */
static struct 	iostat  	iostat;
static struct 	sysinfo		sysinfo;
static time_t			lbolt;
static int			iost_run;
static char	**dr_name;
static int	*dr_select;
static ulong	*dlt_dk_time, *last_dk_time;
static ulong	*dk_xrate;
static ulong	*dk_bsize;
static ulong	*dlt_dk_xfers, *last_dk_xfers;
static ulong	*dlt_dk_rblks, *last_dk_rblks;
static ulong	*dlt_dk_wblks, *last_dk_wblks;
static ulong	*dlt_dk_seek, *last_dk_seek;
static long	dlt_tty_in, last_tty_in;
static long	dlt_tty_out, last_tty_out;
static long	dlt_cpu[CPU_NTIMES], last_cpu[CPU_NTIMES];
static time_t  last_time, cptimes;
static int 	ndrives = 0;
static struct  dkstat *dkstat;
static ulong   *offset;
static int	iost_flag;
static int	iost_dkcnt;
static int	noptions, cur_iostrun;
static int	ncpus;
static struct  var   var;
static struct  sigaction mysig, oldsig;

/*
 * NAME: setiostrun(new_run, old_runp)
 *
 * FUNCTION: 	retrieve the old kernel setting for the iostrun flag
 *		into old_iostrun and set the kernels iostrun flag to the
 *		new value in new_iostrun.
 *
 */
static void
setiostrun (new_run, old_runp)
int new_run;
int *old_runp;
{
	int	rc=0,retry;
	for (retry=1; retry < 3; retry++)
	{	
		if (sysconfig(SYS_GETPARMS, (void *)&var,
				 (int)sizeof(struct var)))
		{
			fatal(MSGSTR(IOSTE01,
			     "iostat: sysconfig system error\n"));
		}
		*old_runp = var.v_iostrun;
		if (var.v_iostrun != new_run)
		{
			var.v_iostrun = new_run;
			if(!(rc = sysconfig(SYS_SETPARMS, (void *)&var,
			 (int)sizeof(struct var))))	break;
		}
		else	break;
	}	/* end of for */
	if ((rc != 0) || (retry >= 3))
	{
		fatal(MSGSTR(IOSTE01, "iostat: sysconfig system error\n"));
	}
	return;
}	

/*
 * NAME:  iostat
 *
 * FUNCTION: report I/O statistics 
 *
 * DESCRIPTION:  This program iteratively reports the number 
 *		 of characters read and written to terminals, 
 *		 and, for each disk, the percentage of time 
 *		 the disk was active, the number of  
 *		 transfers per second, and the number
 *		 of blocks read from and written to disk. 
 *		 It also gives the percentage of time the system 
 *		 has spent in user mode, in system mode, and idling.
 *
 * DATA STRUCTURES:  iost_table
 *
 */

/*
 * NAME: sig_handler(signo)
 *
 * FUNCTION: 	catch the termination signals and put the iostrun flag
 *		back to its old value before exiting
 *
 */
static void sig_handler(int signo)
{
	switch (signo)
	{
		case SIGINT:
		case SIGQUIT:
		case SIGHUP:
		case SIGTERM:
			setiostrun(iost_flag,&cur_iostrun);		
			exit(0);
	}
}
/*
 * NAME: main (argc, argv)
 *
 * FUNCTION: 	main iostat routine
 *
 */

main(argc, argv)
int argc;
char *argv[];
{
	register int	i, j;
	int		iter, dk_find, drvdsp, ttydsp;
	int		rc, ch_cnt, disp_cnt;
	char		**cp;

	(void) setlocale (LC_ALL,"");

	catd = catopen(MF_IOSTAT, NL_CAT_LOCALE);

	iter = 0;
	ch_cnt = 0;
	disp_cnt = 0;	 /* report counter */
	drvdsp = TRUE;  /* display disk report default */
	ttydsp = TRUE;  /* display tty, cpu report defualt */
	noptions = argc-1;
	privilege (PRIV_LAPSE);
	argc--, argv++;
	while (argc>0 && argv[0][0]=='-') 
	{
		char *cp = *argv++;
		argc--;
	 	while (*++cp) 
		{
	 		if (*cp == 'd')
				ttydsp = FALSE; /* display drive stats only */
			else if (*cp == 't')
				drvdsp = FALSE; /* display tty/cpu stats only */
			else	
			{
				printf(MSGSTR(USAGE, "usage: iostat [-t] [-d] [drives] [interval [count]] \n\t -t specifies tty/cpu report only \n\t -d specifies drive report only \n\t -t and -d cannot both be specified \n"));
				exit(1);
			}
		
		}
	}
	if (ttydsp == FALSE && drvdsp == FALSE)
	{
		printf(MSGSTR(USAGE, "usage: iostat [-t] [-d] [drives] [interval [count]] \n\t -t specifies tty/cpu report only \n\t [-d] specifies drive report only \n\t [-t] and [-d] cannot both be specified \n"));
		exit(1);
	}
	privilege (PRIV_ACQUIRE); /* Get maximum allowed privilege */
	if (sysconfig(SYS_GETPARMS, (void *)&var, (int)sizeof(struct var))) {
		fatal(MSGSTR(IOSTE01, "iostat: sysconfig system error\n"));
	}
	ncpus = var.v_ncpus;
	if ((kmem = open("/dev/kmem", O_RDONLY)) < 0)
	{
		if (errno == EACCES)
			fatal(MSGSTR(IOSTE04, "iostat: Permission Denied.\n"));
		else
			fatal(MSGSTR(IOSTE02,
				 "iostat: /dev/kmem system error\n"));
	}

/* Set up signal handler to catch terminate signals */
	mysig.sa_handler = (void (*)(int))sig_handler;
	SIGINITSET(mysig.sa_mask);
	mysig.sa_flags = 0;
	sigaction(SIGHUP,(struct sigaction *)NULL, &oldsig);
	if(oldsig.sa_handler != SIG_IGN)
		rc = sigaction(SIGHUP,&mysig,(struct sigaction *)NULL);
	rc = sigaction(SIGTERM,&mysig,(struct sigaction *)NULL);
	rc = sigaction(SIGQUIT,&mysig,(struct sigaction *)NULL);
	rc = sigaction(SIGINT,&mysig,(struct sigaction *)NULL);

/* Get kernel symbol addresses */
	if ( knlist(nltbl, NUM_TBL, sizeof(struct nlist))== -1)
	{
	if (errno == EPERM)
		fatal(MSGSTR(IOSTE04, "iostat: Permission Denied.\n"));
	else
		fatal(MSGSTR(IOSTE03, "iostat: knlist system error\n"));
	}

/* check and set iostrun flag to gather statistics */
	setiostrun(TRUE,&iost_flag); 
	cur_iostrun = iost_flag;	/* starting iostrun flag */

	dr_name = (char **)calloc(noptions, sizeof(char *));
	i = 0;
	while (argc > 0 && !isdigit(argv[0][0])) 
	{
		dr_name[i] = argv[0];
		argc--, argv++;
		i++;
	}
	noptions = i;
	if (argc > 1)
		iter = atoi(argv[1]); 	/* get the display times */

	getiostat();		/* get iostat  information */
loop1:
	iost_dkcnt = iostat.dk_cnt;
	allocate_memory();	/* allocate memory space for store
				 * disks informations */
	getdkstat();		/* get disks informations */

	/*
	 * Choose drives to be displayed.  
	 */

	ndrives = 0; 
	for (j = 0; j < noptions; j++) 
	{ 
		dk_find = 0;
		for (i = 0; i < iost_dkcnt; i++) 
		{
			if (strcmp(dkstat[i].diskname, dr_name[j]))
				continue;
			dr_select[i] = 1;
			ndrives++;
			dk_find = 1;
			break;
		}
		if (!dk_find)
			printf(MSGSTR(IOSTE05,
		 "iostat: Disk \" %s \" is not found.\n "), dr_name[j]);
	}

	if (ndrives == 0)     /* display all existed disk drives by default */
		for (i = 0; i < iost_dkcnt; i++)
		{
			dr_select[i] = 1;
		}

loop:
	getsysinfo();		/* get cpu time and tty information */
	gettotaltime();		/* get time clock */
	if (!ch_cnt && ttydsp)
	{
		if (disp_cnt == 0 || drvdsp) 
		{
			print_hdr();	/* print the tty/cpu header */
			disp_cnt++;
		}
		statttycpu();		/* print tty and cpu information */
		if (disp_cnt++ == 24)
			disp_cnt = 0;
	}

	for (i = 0; i < iost_dkcnt; i++) 
	{
		if (!dr_select[i])
			continue;
#define X(name)	dlt_dk_/**/name[i] = dkstat[i].dk_/**/name      \
				   - last_dk_/**/name[i];	\
		last_dk_/**/name[i] = dkstat[i].dk_/**/name;

		X(time); X(xfers); 
		X(rblks); X(wblks); X(seek);

		dk_xrate[i] = dkstat[i].dk_xrate;
		dk_bsize[i] = dkstat[i].dk_bsize;
	}
	
	if (!ch_cnt && drvdsp)
	{
	    if (iostat.dkstatp )
	    {
		if (cur_iostrun == TRUE)
		{
			if (disp_cnt ==0 || ttydsp) 
			{
				printhdr();	/* print the disks head */
				disp_cnt++;
			}
		 	for (i = 0; i < iost_dkcnt; i++)
			{
			 	if (dr_select[i])
				{
					stats(i);	/* print disks info */
					if (disp_cnt++ == 24 && !ttydsp) 
					{	/* print header again */
						disp_cnt = 1;
						printhdr();
					}
				}
			}
		}
		else
		{
			printf(MSGSTR(HISINV, 
		"\t\t\" Disk history since boot not available. \"\n\n"));
			cur_iostrun = TRUE;
		}
	    }
	}
contin:
	if (--iter && argc > 0) 
	{
		fflush( stdout );
		sleep(atoi(argv[0]));
		getiostat();		/* get iostat information */
		if (iost_dkcnt != iostat.dk_cnt)
		{
			free_memory();
			ch_cnt = 1;
			iter++;
			goto loop1;
		}
		else
		{
			getdkstat();		/* get disks informations */
			ch_cnt = 0;
			goto loop;
		}
	}
	setiostrun(iost_flag,&cur_iostrun);
				 /* set kernel flag back to original state */
	privilege (PRIV_DROP);
}

static print_hdr()
{
	printf("\n");
	printf(MSGSTR(TTY, "tty:    "));
	printf(MSGSTR(TTYINOUT, "  tin         tout   "));
	printf(MSGSTR(CPU, "avg-cpu:  "));
	printf(MSGSTR(CPUTIME, "%% user    %% sys     %% idle    %% iowait\n"));
}

static statttycpu()
{
	float etime,cips,cops;
	etime = cptimes/(float)hz;
	cips = dlt_tty_in / etime;
	cops = dlt_tty_out / etime;
	printf("        ");
	printf("%5.1f        ", cips);
	printf("%5.1f      ", cops);
	printf("       ");
	printf("%5.1f    ", dlt_cpu[CPU_USER]*100.0/(cptimes*ncpus));
	printf("%5.1f      ", dlt_cpu[CPU_KERNEL]*100.0/(cptimes*ncpus));
	printf("%5.1f     ", dlt_cpu[CPU_IDLE]*100.0/(cptimes*ncpus));
	printf("%5.1f     ", dlt_cpu[CPU_WAIT]*100.0/(cptimes*ncpus));
	printf("\n");
}

static printhdr()
{
	printf("\n");
	printf(MSGSTR(DISKS, "Disks:        "));
	printf(MSGSTR(DINFO1, "%% tm_act     Kbps      tps   "));
	printf(MSGSTR(DINFO2, " Kb_read   Kb_wrtn\n"));
}

static stats(n)
register int n;
{
	float etime, atime, tps, bps;

	dlt_dk_rblks[n] = (dlt_dk_rblks[n] * dk_bsize[n])/1024;
	dlt_dk_wblks[n] = (dlt_dk_wblks[n] * dk_bsize[n])/1024;
	etime = cptimes/(float)hz;
	atime = dlt_dk_time[n]/(float)hz;
	if (atime == 0.0)
		atime = 1.0;
	tps = dlt_dk_xfers[n]/etime;
	bps = (dlt_dk_rblks[n] + dlt_dk_wblks[n])/etime;
	printf("%-14.14s", dkstat[n].diskname);
	printf(" %5.1f     ", dlt_dk_time[n]*100./cptimes);
	printf("%5.1f     ", bps);
	printf("%5.1f   ", tps);
	printf("%8u  ", dlt_dk_rblks[n]);
	printf("%8u", dlt_dk_wblks[n]);
	printf("\n");
}

static getiostat()
{
	if (readkmem(&iostat, &nltbl[IOSTAT], sizeof(struct iostat)))
	{
		fatal(MSGSTR(IOSTE02, "iostat: /dev/kmem system error\n"));
	}
}

static getdkstat()
{
	register int i;

	offset[0] = (long)iostat.dkstatp;
	for (i = 0; i < iost_dkcnt && offset[i] != (long)NULL; i++)
	{
		lseek(kmem, (long)offset[i], 0);
		read(kmem, &dkstat[i], sizeof(struct dkstat));
		offset[i+1] = (long)dkstat[i].dknextp;
	}
}

static getsysinfo()
{
	register int i;

	if (readkmem(&sysinfo, &nltbl[SYSINFO], sizeof(struct sysinfo)))
	{

		fatal(MSGSTR(IOSTE02, "iostat: /dev/kmem system error\n"));
	}
	for (i = 0; i < CPU_NTIMES; i++)
	{
		dlt_cpu[i] = sysinfo.cpu[i] - last_cpu[i];
		last_cpu[i] = sysinfo.cpu[i];
	}
	dlt_tty_in = sysinfo.ttystat.rawinch - last_tty_in;
	last_tty_in = sysinfo.ttystat.rawinch;
	dlt_tty_out = sysinfo.ttystat.rawoutch - last_tty_out;
	last_tty_out = sysinfo.ttystat.rawoutch;
}

static gettotaltime()
{
	if (readkmem(&lbolt, &nltbl[LBOLT], sizeof(time_t)))
	{
		fatal(MSGSTR(IOSTE02, "iostat: /dev/kmem system error\n"));
	}
	cptimes = lbolt - last_time;
	last_time = lbolt;
}

/*
 * NAME: readkmem(buf, ptr, len)
 *
 * FUNCTION: 	Read len number of bytes beginning at a ptr
 *		within /dev/kmem into buf. Return a zero if the 
 *		read was successfull and a -1 otherwise.
 *
 */

static readkmem(buf, ptr, len)
char *buf;
struct nlist *ptr;
int len;
{
	if (lseek(kmem, (long)ptr->n_value, 0) == -1)
		return(-1);
	
	if (read(kmem, &buf[0], len) != len)
		return(-1);

	return(0);
}

/*
 * NAME: allocate_memory()
 *
 * FUNCTION: 	allocate memory spaces upon iost_dkcnt parameter
 *		for storing disks informations.
 *
 */

static allocate_memory()
{
	dr_select = (int *)calloc(iost_dkcnt, sizeof (int));
	dkstat = (struct dkstat *)calloc(iost_dkcnt, sizeof (struct dkstat));
	offset = (ulong *)calloc(iost_dkcnt+1, sizeof (ulong));
	dk_xrate = (ulong *)calloc(iost_dkcnt, sizeof(ulong));
	dk_bsize = (ulong *)calloc(iost_dkcnt, sizeof(ulong));
#define allocate(e)		\
	dlt_dk_/**/e = (ulong *)calloc(iost_dkcnt, sizeof(ulong));  \
	last_dk_/**/e = (ulong *)calloc(iost_dkcnt, sizeof(ulong)); 
	allocate(time);
	allocate(xfers);
	allocate(rblks);
	allocate(wblks);
	allocate(seek);
}

/*
 * NAME: free_memory()
 *
 * FUNCTION: 	free memory spaces for reallocate memory.
 *
 */

static free_memory()
{
	free(dr_select);
	free(dkstat);
	free(offset);
	free(dk_xrate);
	free(dk_bsize);
#define memfree(e)		\
	free(dlt_dk_/**/e);	\
	free(last_dk_/**/e);
	memfree(time);
	memfree(xfers);
	memfree(rblks);
	memfree(wblks);
	memfree(seek);
}
/*
 * NAME: fatal(str)
 *
 * FUNCTION: 	prints a string to STDERR, drops max privilege and exits,
 *		returning 1.
 *
 */

static fatal(str)
char *str;

{
	fprintf(stderr, str);
	privilege (PRIV_DROP);
	exit(1);
}




