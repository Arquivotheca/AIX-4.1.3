static char sccsid[] = "@(#)74  1.15.1.10  src/bos/usr/lib/sa/sadc.c, cmdstat, bos41J, 9521B_all 5/24/95 16:11:31";
/*
 * COMPONENT_NAME: (CMDSTAT) status
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 27 83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* 
 * LEVEL 1, 5 Years Bull Confidential Information
 */
/*
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 */

/*
 *  sadc.c - writes system activity binary data from /dev/kmem to
 *           a file or stdout.
 *  Usage: sadc [t n] [file]
 *         if t and n are not specified, it writes
 *         a dummy record to data file. This usage is
 *         particularly used at system booting.
 *         If t and n are specified, it writes system data n times to
 *         file every t seconds.
 *         In both cases, if file is not specified, it writes
 *         data to stdout.
 */

/**********************************************************************/
/* Include File                                                       */
/**********************************************************************/

#include  <sys/types.h>
#include  <sys/param.h>
#include  <sys/var.h>
#include  <sys/stat.h>
#include  <sys/inode.h>
#include  <sys/proc.h>
#include  <sys/thread.h>
#include  <sys/sysinfo.h>
#include  <fcntl.h>
#include  <stdio.h>
#define	  _KERNEL	1
#include  <sys/file.h>
#include  "sa.h"
#include  <sys/low.h>
#include  <errno.h>
#include  <procinfo.h>
#include  <locale.h>
#include  <stdlib.h>
#include  <sys/vminfo.h>
#include  <sys/vmker.h>
#include  <jfs/fsvar.h>
#include  <sys/id.h>
#include  <sys/systemcfg.h>
#include  "sadc_msg.h"

/**********************************************************************/
/* Constant Definition / Macro Function                               */
/**********************************************************************/
#define  PROCSIZE  sizeof (struct procsinfo)
#define  THREADSIZE sizeof(struct thrdsinfo)
#define  FSVSIZE   sizeof(struct fsvar)
#define  INOSIZE   sizeof(struct inode)
#define  GENSIZE   sizeof(struct genalloc)
#define  FSV       0                   /* entry for namelist nl*/
#define  BUFSIZE   4096*25

#define  MSGSTR(Num,Str)  catgets(catd,MS_SADC,Num,Str)

/**********************************************************************/
/* Function Prototype Declaration                                     */
/**********************************************************************/

long              time();
static void       perrexit();
static int        lseekHidext();

/**********************************************************************/
/* Global / External Variables                                        */
/**********************************************************************/

nl_catd  catd;

struct sa      d;
struct var     tbl;
struct stat    ubuf, syb;

static int  tblmap[SINFO];
static int  recsz;

char  *loc;
char  *diskloc, *tapeloc;
char  *excl;
int   v_excl = 0;

unsigned  tblloc;
int  i,j,k,f;
int  runqueue=0;    /* Number of processes in the runqueue */
int  diskcnt, tapecnt;
int  debug=0;       /* Print out debugging information */

struct sa_head sa_head;


/**********************************************************************/
/* NAME:  main                                                        */
/* FUNCTION:                                                          */
/* RETURN VALUE:                                                      */
/**********************************************************************/

main(argc, argv)
int argc;
char *argv[];
{
	int  ct, i, ch;
	int  fp;
	int  num_inode, size_inode;
	long  min;
	char  *fname, *eptr;
	char  *name = "/unix";
	unsigned  ti;
	uid_t  e_uid;    /* Effective user ID */

	struct stat       buf;
	struct vminfo     vminfo_data;
	struct vmkerdata  vmker_data;
	struct cpuinfo	  wcpuinfo;

	int    ind ;
	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_SADC,NL_CAT_LOCALE);

	excl = "";
	/* tell getopt() to be quiet for now. */
	opterr = 0;
	while ((ch=getopt(argc, argv, "x:")) != EOF) {
		switch(ch) {
		case 'x':		/* Exclude list */
			excl = optarg;
			break;
		}
		/* Will need a usage statement if this ever gets documented */
	}
	argc -= optind;
	argv += optind;

	for(eptr = excl; *eptr; eptr++) {
		switch(*eptr) {
			case 'v': v_excl++; break;
			/* we should add other cases later. */
		}
	}

	ct = ti = 0;
	if (argc >= 2) {
	    ct = atoi(argv[1]); /* in ct we take the value of the argument n */
	    ti = atoi(argv[0]); /* in ti we take the value of the argument t */
	    argc -= 2;
	    argv += 2;
	}
	min = time((long *) 0);

	/*  
	 * Get the kernel information from the knlist kernel routine.
	 */
	knlist(setup,NLIST_ELEMENTS,(sizeof (struct nlist)));

	/*
	 * must use nlist call because 
	 * these are not exported by the kernel
	 */
	nlist(name, nl); 


	if (debug) {
		for (i=0;i<NLIST_ELEMENTS;i++) {
			printf ("knlist value[%d]: %s	%x\n",
				i, setup[i].n_name, setup[i].n_value);
			printf("nlist value[%d]: %s   %x\n", 
				i, nl[i].n_name, nl[i].n_value);
		}
	}


	/*
	 * open /dev/kmem
	 */
	if ((f = open("/dev/kmem", 0)) == -1) {
		perrexit(MSGSTR(KMEM,"Can't open kmem"));
	}

	/*
	 * Get the numbers of tape drives and
	 * disk drives for AIWS 
	 */
	if (setup[TAPECNT].n_value !=0) {
		lseekHidext(f, &(setup[TAPECNT]));
		if (read(f, (char *)&tapecnt, sizeof(tapecnt)) == -1) {
			perror("sadc: ");
			exit(1);
		}
		tapeloc = malloc((size_t)(sizeof(long) * tapecnt));
		if (tapeloc == NULL) {
			perrexit(MSGSTR(NO_TAPE,"tape location not found"));
		}
	}

	if (setup[DISKCNT].n_value !=0) {
		lseekHidext(f, &(setup[DISKCNT]));
		if (read(f, (char *)&diskcnt, sizeof diskcnt) == -1) {
			perrexit(MSGSTR(DISK_READ,"diskcnt read failed"));
		}
		diskloc = malloc((size_t)(sizeof(long) * diskcnt));
		if (diskloc == NULL) {
			perrexit(MSGSTR(NODISK,"disk location not found"));
		}
	}

	/*
	 * construct the tblmap and compute the record size
	 */
	for (i=0;i<SINFO;i++) {
		if (setup[i].n_value != 0) {
			if (i == DISKINFO)       tblmap[i] = diskcnt;
			else if (i == TAPEINFO)  tblmap[i] = tapecnt;
			recsz += tblmap[i];
		}
		else {
			tblmap[i] = 0;
		}
	}
	recsz = sizeof(struct sa) - sizeof d.devio + recsz * sizeof d.devio[0];

	sa_head.magic = SAR_MAGIC;
	sa_head.nbcpu = _system_configuration.ncpus;
	if (debug) {
		printf("sa_head.nbcpu = %d \n", sa_head.nbcpu);
	}

	if (argc == 0) {

		/*
		 * If no data file is specified, direct data to stdout.
		 */
		fp = 1;

		/* write header record */

		write(fp, tblmap, sizeof tblmap);
		write(fp, &sa_head, sizeof sa_head);
	}
	else {
		fname = argv[0];

		/* Get real user and change to effective user ID */
		e_uid = geteuid();
		seteuid(getuid());

		/*  check if the data file is there  */
		/*  check if data file is too old    */
		/*  24x60x60 = 86400 sec (24 hours)  */

		if ((stat(fname, &buf) == -1) ||
		    ((min - buf.st_mtime) > 86400) ||
                    (buf.st_size<=4))
			goto credfl;
		if ((fp = open(fname, O_RDWR)) == -1) {
credfl:

		/*
		 *  If the data file does not exist,
		 *  create one and write the header record.	
		 */
			if ((fp = creat(fname, (mode_t)00644)) == -1) {
				perror("sadc: can't create file");
				exit(1);
			}
			close(fp);
			fp = open(fname, O_RDWR);
			lseek(fp, 0L, SEEK_SET);
			write(fp, tblmap, sizeof tblmap);
			write(fp, &sa_head, sizeof sa_head);
		}
		else {
			/*
			 *  If the data file exists,  position the
			 *  write pointer to the last good record.
			 */
			lseek(fp, -(long)((buf.st_size - 
			   (sizeof(tblmap) + sizeof(sa_head))) % recsz),SEEK_END);
		}
		seteuid(e_uid);    /* Return to effective user */
	}

	/*
	 * If n=0, write the additional dummy record.
	 */

	if (ct == 0) {
		for (ind = 0; ind <= N_PROCESSOR; ind++) {
			for (i=0;i<4;i++) {
				d.si[ind].cpu[i] = -300;
			}
		}
		d.ts = min;
		write(fp,&d,recsz);
	}

	/*
	 * Get memory for tables
	 */

	if (lseekHidext(f,&(setup[V])) == -1)
		perrexit(MSGSTR(LSEEK_VAR,"lseek to var location failed"));
	if (read(f,&tbl,sizeof tbl) == -1) {
		perror("sadc: var read failed ");
		exit(1);
	}
	if (debug) {
		printf ("tbl:\n	v_proc: %x ve_proc %x v_file %x\n",
		tbl.v_proc, tbl.ve_proc, tbl.v_file);
		printf ("tbl:\n	v_file %x v_text:%x \n", tbl.v_file, 0);
	}
	/*
	 * Check for mismatch between namelist and memory image.
	 * Not perfect, but handles obvious cases.
	 *
	 * We no longer use the /unix namelist, but this check is still
	 * useful to see if the internal structures make sense.
	 */
	if (tbl.v_proc <= 0 
	|| (char *)setup[PRO].n_value >= tbl.ve_proc
	|| setup[PRO].n_value + (tbl.v_proc*sizeof(struct proc)) < (int)tbl.ve_proc) {
		fprintf(stderr, MSGSTR(BADKER,
			"warning: kernel data structures inconsistent.\n"));
		/* 
		 * Let it go for now.  We won't exit.
		 */
	}

	if (tbl.v_thread <= 0 
	|| (char *)setup[THREAD].n_value >= tbl.ve_thread
	|| setup[THREAD].n_value + (tbl.v_thread*sizeof(struct thread)) < (int)tbl.ve_thread) {
		fprintf(stderr, MSGSTR(BADKER,
			"warning: kernel data structures inconsistent.\n"));
	}


	if (tblloc < (sizeof(struct file)) * tbl.v_file) {
		tblloc = (sizeof (struct file)) * tbl.v_file;
	}
	/*
	if (tblloc < sizeof (struct text)*tbl.v_text)
		tblloc = sizeof (struct text)*tbl.v_text;
	*/

	if (debug) {
		printf ("mallocing gobbs of space: %d\n",tblloc);
	}

	loc = malloc((size_t)tblloc);
	if (loc == NULL) {
		perror("sadc: malloc failed");
		exit(1);
	}

	for (;;) {
		/*
		 * Read sysinfo.h data from /dev/kmem to structure d	
		 */
		lseekHidext(f,&(setup[SINFO]));
		if (read(f, &d.si[GLOBAL], sizeof d.si[GLOBAL]) == -1)
			perrexit(MSGSTR(BADREADSI,"read of sysinfo failed"));
		lseekHidext(f,&(setup[CPUINFO]));
		for (ind = 0; ind < sa_head.nbcpu; ind++) {

		   if (read(f, &wcpuinfo, sizeof(struct cpuinfo) ) == -1)
			perrexit(MSGSTR(BADREADCPU,"read of cpuinfo failed"));
		   d.si[ind].cpu[CPU_IDLE]  = wcpuinfo.cpu[CPU_IDLE];
                   d.si[ind].cpu[CPU_USER] = wcpuinfo.cpu[CPU_USER];
                   d.si[ind].cpu[CPU_KERNEL] = wcpuinfo.cpu[CPU_KERNEL];
                   d.si[ind].cpu[CPU_WAIT] = wcpuinfo.cpu[CPU_WAIT];
		   d.si[ind].pswitch = wcpuinfo.pswitch;
		   d.si[ind].syscall = wcpuinfo.syscall;
		   d.si[ind].sysread = wcpuinfo.sysread;
		   d.si[ind].syswrite = wcpuinfo.syswrite;
		   d.si[ind].sysfork = wcpuinfo.sysfork;
		   d.si[ind].sysexec = wcpuinfo.sysexec;
		   d.si[ind].readch = wcpuinfo.readch;
		   d.si[ind].writech = wcpuinfo.writech;
		   d.si[ind].iget = wcpuinfo.iget;
		   d.si[ind].namei = wcpuinfo.namei;
		   d.si[ind].dirblk = wcpuinfo.dirblk;
		   d.si[ind].msg = wcpuinfo.msg;
		   d.si[ind].sema = wcpuinfo.sema;


		   if (debug) {
			printf("read in si information\n"); 
			printf("cpu = %d \n",ind);
			printf("values: %x %x %x %x %x\n",
				d.si[ind].cpu[CPU_IDLE],
				d.si[ind].cpu[CPU_USER],
				d.si[ind].cpu[CPU_KERNEL],
				d.si[ind].cpu[CPU_WAIT],
				d.si[ind].bread);
			printf(" %x %x %x %x %x \n",
				d.si[ind].bwrite,
				d.si[ind].lread,
				d.si[ind].lwrite,
				d.si[ind].phread,
				d.si[ind].phwrite);
			printf(" %x %x %x %x %x\n",
				d.si[ind].pswitch,
				d.si[ind].syscall,
				d.si[ind].sysread,
				d.si[ind].syswrite,
				d.si[ind].sysfork);
			printf(" %x %x %x %x %x\n",
				d.si[ind].sysexec,
				d.si[ind].runque,
				d.si[ind].runocc,
				d.si[ind].swpque,
				d.si[ind].swpocc);
			printf(" %x %x %x %x %x\n",
				d.si[ind].iget,
				d.si[ind].namei,
				d.si[ind].dirblk,
				d.si[ind].readch,
				d.si[ind].writech);
			printf(" %x %x %x %x %x\n",
				d.si[ind].rcvint,
				d.si[ind].xmtint,
				d.si[ind].mdmint,
				d.si[ind].rawch,
				d.si[ind].canch);
			printf(" %x %x %x %x %x\n",
				d.si[ind].mdmint,
				d.si[ind].outch,
				d.si[ind].msg,
				d.si[ind].sema,
				d.si[ind].ksched);
			printf(" %x %x %x %x %x\n",
				d.si[ind].koverf,
				d.si[ind].kexit,
				d.si[ind].rbread,
				d.si[ind].rcread,
				d.si[ind].rbwrt);
			printf(" %x  \n",
				d.si[ind].rcwrt);
		}
		}

		/* 
		 *  Here we get the paging information from the kernal
		 *  memory management structures.
		 */
		if (setup[VMKER].n_value != 0) {
			lseekHidext(f,&(setup[VMKER]));
			if (read(f, &vmker_data, sizeof vmker_data) == -1)
				perrexit(MSGSTR(BADREADKER,"read of vmkerdata failed"));
			d.pi.freeslots = (unsigned int) vmker_data.psfreeblks;
			if (debug) {
				printf ("number of free slots: %x\n",
				d.pi.freeslots);
				printf ("number of pageing space blks: %x\n",
				vmker_data.numpsblks);
				printf ("number of pages on the free list : %x\n",
				vmker_data.numfrb);
			}
		}

		if (setup[VMINFO].n_value != 0) {
			lseekHidext(f,&(setup[VMINFO]));
			if (read(f, &vminfo_data, sizeof vminfo_data) == -1)
				perrexit(MSGSTR(VMDERDATA,"read of vmkerdata failed"));
			d.pi.cycles = vminfo_data.cycles;
			d.pi.faults = vminfo_data.pgexct;
			d.pi.otherdiskios = 
				  (vminfo_data.pageins + vminfo_data.pageouts)
				- (vminfo_data.pgspgins + vminfo_data.pgspgouts);
		}
	/*
	 * Here we would also gather up disk and tape drive information,
	 * loading it into the dynamically allocated areas pointed to by
	 * diskloc and tapeloc.
	 */

		if (!v_excl) {
			/*
			 * Compute the size of the system table	
			 */

			/* Call to inodetbl to find inode count and size */
			inodetbl(&num_inode, &size_inode);
			d.szinode = num_inode;
			d.szfile = filetbl(loc);
			d.szproc = proctbl();
			d.szthread = threadtbl();

			/* Record maximum sizes of system tables */
			d.mszinode = size_inode;
			d.mszfile = tbl.v_file;
			d.mszproc = tbl.v_proc;
			d.mszthread = tbl.v_thread;
		}

		/*  get time stamp  */
		d.ts = time ((long *) 0);
		i=0;
		for (j=0;j<SINFO;j++) {
			if (setup[j].n_value != 0) {
				/* Here the disk/tape info is loaded into devio. */
			}
		}

	/*
	 * Write the d data to the data file.
	 */
		write(fp,&d,recsz);
		if (--ct > 0) {
			sleep(ti);
		}
		else {
			close(fp);
			exit(0);
		}
	}
}

/**********************************************************************/
/* NAME:  inodetbl                                                    */
/* FUNCTION:  count the number of incore inodes and calculate maximum */
/*            inode table size.                                       */
/*            Note: the size is dynamically allocated so maximum      */
/*            calculated can change when more incore inodes allocated */
/* RETURN VALUE:  The number of active used inodes is returned and    */
/*                the maximum table size of inodes allocated.         */
/**********************************************************************/

inodetbl(num_inode, size_inode)
int *num_inode;
int *size_inode;
{
	int i;
	struct fsvar fsv;  /* file system var structure that contains
	                      v_inode pointer */
	genalloc_t genalloc; /* genalloc structure points to
	                        inode structure */
	long tmpaddr;  /* tmp address for TOC entry */

	struct inode *buf_inode_table;
	int tmp_size, read_size, buf_sz_inode;

	*num_inode = 0;
	*size_inode = 0;

	if (lseekHidext(f,&(nl[FSV])) < 0) {
		perror("sadc: lseek to fsv structure failed");
		exit(1);
	}
	if (read(f, &fsv, FSVSIZE) != FSVSIZE) {
		perror("sadc: read of fsvar structure failed");
		exit(1);
	}
	if (lseek(f, fsv.v_inode, SEEK_SET) < 0) {
		perror("sadc: lseek to v_inode failed");
		exit(1);
	}
	if (read(f, &genalloc, GENSIZE) != GENSIZE) { 
		perror("sadc: read of genalloc structure failed");
		exit(1);
	}

	/* calculate maximum inode table size */
	*size_inode = (genalloc.a_hiwater - genalloc.a_table)/INOSIZE;

	/* the following reads from the beginning of the inode table
	   a_table to its hiwater mark and counts the number of inode
	   structures defined in memory */

	buf_sz_inode = (BUFSIZE/INOSIZE);
	buf_inode_table = (struct inode*)malloc(buf_sz_inode * INOSIZE);

	if (buf_inode_table == NULL) {
		perror("sadc: malloc failed");
		exit(1);
	}

	if (lseek(f, genalloc.a_table, SEEK_SET) < 0) {
		perror("sadc: lseek to a_table failed");
		exit(1);
	}

	for(tmp_size = 0; tmp_size < *size_inode; tmp_size += read_size) {
		read_size = buf_sz_inode;
		if (tmp_size + buf_sz_inode > *size_inode)
			read_size = *size_inode - tmp_size;

		if (read(f, buf_inode_table, (read_size*INOSIZE)) !=
							(read_size*INOSIZE)) {
			perror("sadc: read of inode_t structure failed");
			exit(1);
		}
		for(i=0; i < read_size; i++) {
			if (buf_inode_table[i].i_count != 0) {
				    (*num_inode)++;
			}
		}
	}

	free(buf_inode_table);

	if (debug) {
		printf ("inodetbl out:%d\n",num_inode);
	}
	return;
}

/**********************************************************************/
/* NAME:  filetble                                                    */
/* FUNCTION:  Count number of open files.                             */
/* RETURN VALUE:  Return number of open files.                        */
/**********************************************************************/

filetbl(x)
register struct file *x;
{
	register i,n;

	lseekHidext(f,&(setup[FLE]));
	read(f,x,tbl.v_file*sizeof(struct file));
	for (i=n=0;i<tbl.v_file; i++,x++) {
		if (x->f_count != 0) {
			n++;
		}
	}
	if (debug) {
		printf ("filetbl out:%d\n",n);
	}
	return(n);
}

/**********************************************************************/
/* NAME:  proctbl                                                     */
/* FUNCTION:  Count number of process.                                */
/* RETURN VALUE:  Return number of process.                           */
/**********************************************************************/

proctbl()
{
	int nproc;
	struct procsinfo *P;
	pid_t index;
	int count;

	P = NULL;
	index = 0;
	count = 1000000;
	
	nproc = getprocs(P, PROCSIZE, NULL, 0, &index, count);

	if (debug) {
		printf (" nb of process : %d\n",nproc);
	}
	return(nproc);
}

/**********************************************************************/
/* NAME:  threadtbl                                                   */
/* FUNCTION:  Count number of threads.                                */
/* RETURN VALUE:  Return number of threads                            */
/**********************************************************************/

threadtbl()
{
	int nthread;
	pid_t pid;
	struct thrdsinfo *T;
	tid_t index;
	int count;

	pid = -1;
	T = NULL;
	index = 0;
	count = 1000000;
	
	nthread = getthrds(pid, T, THREADSIZE, &index, count);

	if (debug) {
		printf (" nb of threads : %d\n",nthread);
	}
	return(nthread);
}


/**********************************************************************/

/**********************************************************************/
/* NAME:  perrexit                                                    */
/* FUNCTION:  Call perror and exit program.                           */
/* RETURN VALUE:  none                                                */
/**********************************************************************/

static void  perrexit(msg)
char  *msg;
{
	char buf[255];

	sprintf (buf,"sadc: %s",msg);
	perror(buf);
	exit(2);
}

/**************************************************
 *
 * NAME:     lseekHidext
 * FUNCTION: de-reference 'off' TOC entry and lseek
 * RETURN VALUE: lseek() return value or -1
 *
 **************************************************/
static int
lseekHidext(int fdes, struct nlist *off )
{
	long    offset ;

	if ( off->n_sclass == C_HIDEXT ) {
		if ( lseek(fdes,off->n_value,SEEK_SET) == -1 ||
		    read(fdes,&offset,sizeof(offset)) != sizeof(offset) )
			return( -1 );
	} else
		offset = off->n_value;
	return( lseek(fdes, offset, SEEK_SET) );
}

