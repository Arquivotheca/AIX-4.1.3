static char sccsid[] = " @(#)29  1.26.1.18  src/bos/usr/bin/errlg/errdemon/main.c, cmderrlg, bos41J, bai15 3/29/95 03:02:49";

/*
 * COMPONENT_NAME: CMDERRLG   system error logging and reporting facility
 *
 * FUNCTIONS: main
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errids.h>
#include <sys/mdio.h>
#include <locale.h>
#include <signal.h>
#include <errno.h>
#include <sys/erec.h>
#include <errlg.h>
#include <sys/utsname.h>
#include <sys/iplcb.h> 

extern optind;
extern char *optarg;

#ifdef DUMPLOG
/********************************************************************/
/*   Global variables to be used in debugging logfile pointer       */
/*   management problems.  To use this, compile with -D option      */
/*   ie. "build OPT_LEVEL=-DDUMPLOG"                                */
/*   Debug information for this errdemon will then go to            */
/*   /tmp/errlog.dump.                                              */

int    Global_Sequence;
int    Global_Wrap = FALSE;
int    Global_Wrap_Count = 0;
int    Global_New_Staleoff;
int    Global_New_Inoffb;
int    Global_New_Outoff;
int    Global_Entry_Length;
int    Global_Firstflag = TRUE;

/********************************************************************/
#endif /* DUMPLOG */

char	Logfile[PATH_MAX];		/* errlg file pathname */ 
int		Buffer;				/* errlg memory buffer size */
int 	Threshold;  		/* maximum size of errlg */
int 	odm_buffer;    		/* errlg memory buffer size in ODM */

int	tmpltflg;		/* true when tmplt repos. is available. */
int Logfd;
struct log_hdr   log_hdr;
struct log_entry log_entry;

struct	utsname   uts;

int   Errorfd;		/* /dev/error */
char *Errorfile=ERRORFILE_DFLT;	/* "/dev/error" */
int   Childpid;		/* pid of actual errdemon */
int   Errctlfd;		/* /dev/errorctl */
int rv;

extern	void	init_be_internal();	/* initialize buff entry internal */
extern	int		get_threshold();	/* get threshold value from odm */
extern	int		ch_threshold();		/* change threshold value in odm */
extern	int	loginit();

#ifdef DUMPLOG
static dump_log();
#endif /* DUMPLOG */


/*
 * NAME:     main
 * FUNCTION: determine the name of the program.
 *           Call cmdinit_errdemon() to process its command line.
 *           Then fork and call child() to do the real work.
 */

main(argc,argv)
char *argv[];
{

	setlocale(LC_ALL,"");		/* This sets the environment  */
	setprogname();			/* basename of argv[0] */
	catinit(MCS_CATALOG);		/* message catalog */

	get_buffer();		/* get buffer size from the device driver */

	cmdinit_errdemon(argc,argv);		/* process command line */
	
	rv = loginit(1);
	if( rv < 0 )  	/* open errlog */
		cat_fatal(CAT_DEM_DATABASE,"\
Failure to open the logfile '%s' for writing.\n\
Possible causes are:\n\
1. The file exists but the invoking process does not have write\n\
   permission. \n\
2. The file exists but the directory '%s' does not have write\n\
   permission.\n\
3. The file exists but is not a valid error logfile.  Remove the file\n\
   and restart the errdemon.\n\
4. The file does exist and the directory '%s' does not have enough\n\
   space available. The minimum logfile size is %d bytes.\n",
Logfile,dirname(Logfile),dirname(Logfile),LH_MINSIZE);

	Childpid = fork();					/* child is the daemon */
	if(Childpid == 0) {					/* if child process */
		child();						/* call child(). no return */
	}
	exit(0);
}

static
usage()
{

	cat_eprint(CAT_DEM_USAGE,"\
Usage:\n\
errdemon -i filename -s value -B value | -l\n\
-i filename  Uses the error log file specified by the filename parameter.\n\
             If this flag is not specified, the default system error log file,\n\
             /var/adm/ras/errlog, is used.\n\
-s value     Set the error log file maximum size.\n\
             Valid range is 4096 bytes to your ulimit in bytes.\n\
-l           Show the error log attributes.\n\
-B value     Set the error log memory buffer size.\n\
	     This size must be a minimum of %d bytes.\n", BUFFER_MINIMUM);
	exit(1);
}

/*
 * NAME:     cmdinit_errdemon
 * FUNCTION: Process command line args for errdemon
 *           See errdemon_usage() for command line syntax.
 * INPUTS:   argc,argv
 * RETURNS:  NONE  (Will exit on command line errors)
 */

static
cmdinit_errdemon(argc,argv)
char *argv[];
{
	int c;
	struct stat statbuf;
	int	rv, bflag = 0, iflag=0, sflag=0;

	while((c = getopt(argc,argv,"ls:i:B:")) != EOF) {
		switch(c) {
		case 'i':
			iflag = 1;
			strcpy(Logfile,optarg);
			get_threshold();	/* get errlg file size from ODM */
			setlogpath();		/* set errlg file in ODM */
			break;
		case 's':
			sflag = 1;
			getlogpath();		/* get errlg file name from ODM */
			if((rv=ch_threshold(optarg)) == 0)
				usage();			/* exits */
			break;
		case 'l':
			getlogpath();		/* get errlg file name from ODM */
			get_threshold();	/* get errlg file size from ODM */
			get_odm_buffer();	/* get errlg buffer size from ODM */
		 	show_log_attr();	/* show these attributes */
			exit(0);
		case 'B':
			rv = set_buffer(optarg);	/* set errlg buffer size in ODM */
			cat_print(CAT_DEM_BUF_RND,"\
The error log memory buffer size you supplied will be rounded up\n\
to a multiple of 4096 bytes.\n");
			bflag++;
			break;
		default:
			usage();
		}
	}

	/* Upon reboot, the error log memory buffer size gets reset   */
	/* to the minimum value by the device driver.  We check ODM   */
	/* to see if the user has previously specified a different    */
	/* buffer size using the -B flag prior to reboot.  If ODM is  */
	/* different than the default, then we reallocate the buffer. */
	/* This should only be called once after reboot.  All other   */
	/* cases of buffer changes will be handled by the -B flag.    */

	if (!bflag)
	{
		get_odm_buffer();
		if (odm_buffer > Buffer)
			set_initial_buffer(odm_buffer);
	}
	if (!iflag)
		getlogpath();		/* get errlg file name from ODM */
	
	if (!sflag)
		get_threshold();	/* get errlg file size from ODM */
	

	/*
	 * open /dev/error
	 */
	if((Errctlfd = open("/dev/errorctl",0)) < 0)
		perror("/dev/errorctl");
	if(Errorfile == NULL)
		Errorfile = ERRORFILE_DFLT;
	if(stat(Errorfile,&statbuf)) {
		perror(Errorfile);
		exit(1);
	}
	if((Errorfd = open(Errorfile,O_RDONLY)) < 0) {
		if(rv == 2)			/* user tried to change log size to what it was. */
			exit(0);

		cat_eprint(CAT_ERR_SESS,"\
The error log device driver, /dev/error, is already open.\n\
The error demon may already be active.\n");
		exit(1);
	}
}

/*
 * NAME:		pack_nv_detail
 *
 * FUNCTION:	Pack the detail data for an nvram failure.
 *				The detail data array will be packed to look like:
 *					operation	error
 *					nvread		EFAULT
 *
 *				This function assumes errno has not changed since the
 *				nvram i/o failure.
 *
 * INPUTS:		array to hold detail data, string containing nvram operation
 *
 * RETURNS:		how many bytes added to detail data.
 */
static int
pack_nv_detail(detail,opstr)
char detail[];		/* detail data array */
char* opstr;		/* nvram operation string */
{
	int saverr;
	int len;
	int idx=0;			/* index into detail data */
	
	saverr = errno;
	len = strlen(opstr);
	memcpy(&detail[idx],opstr,len);
	idx += len;

	switch (saverr) {
	case EIO:
		len = strlen(STR_EIO);
		memcpy(&detail[idx],STR_EIO,len);
		idx += len;
		break;
	case EFAULT:
		len = strlen(STR_EFAULT);
		memcpy(&detail[idx],STR_EFAULT,len);
		idx += len;
		break;
	case ENOMEM:
		len = strlen(STR_ENOMEM);
		memcpy(&detail[idx],STR_ENOMEM,len);
		idx += len;
		break;
	case EINVAL:
		len = strlen(STR_EINVAL);
		memcpy(&detail[idx],STR_EINVAL,len);
		idx += len;
		break;
	default:
		len = strlen(STR_UNKNOWN);
		memcpy(&detail[idx],STR_UNKNOWN,len);
		idx += len;
		break;
	}
	errno = saverr;
	return(idx);
}

/*
 * NAME:     bld_nv_err
 * FUNCTION: Build an errlog entry for nvram i/o failure.
 * INPUTS:   Pointer to nvram errlogging structure.
 *			 Pointer to nvram operation string.
 *			 Index into detail data for next place to add detail data.
 * RETURNS:  Number of chars packed into detail data.
 */

static int
bld_nv_err(struct nv_err* nvp, char* opstr, int idx)
{

	int chars;

	chars = pack_nv_detail(&nvp->detail_data[idx],opstr);
	nvp->erec.erec_len += chars;
	nvp->erec.erec_rec_len += chars;

	return(chars);
}

/*
 * If the checkstop count is > 0, extract the checkstop data,
 * write to /usr/lib/ras/checkstop, and log an error.
 */

#define CHKSIZE_UP   9216
#define CHKSIZE_MP   65536
#define OFF_CHKCOUNT 0x0308
#define OFF_CHKPTR   0x030C
#define MAX_CHK_STOP 2				/* maximum of two checkstop files */

#define VCPY(from,to_array) \
	strncpy(to_array,from,sizeof(to_array));

static int Nvfd = -1;
static char *Chkfile;

/*
 * NAME:     checkstop
 * FUNCTION: read out the checkstop info from nvram and copy to
 *           /usr/lib/ras/checkstop
 * INPUTS:   none  (read from /dev/nvram)
 * RETURNS:  none
 */

static
checkstop()
{
	char checkstop_count;
	int  checkstop_ptr;
	char checkstop_data[CHKSIZE_MP];
	int  zero = 0;
	int  fd;
	int  rv;
	int  data_len;

	struct {
		struct err_rec0 err_rec;
		char chk_count;
		char chk_filename[28];
	} e;

	struct nv_err nve;	/* will hold nvram error log entry */
	int i=1;

	/* set up for possible nvram i/o error log entry */
	init_be_internal(&nve, ERRID_LOG_NVRAM);

	nve.detail_data[0] = '\n';

	if(nvopen() < 0) {
		(void)bld_nv_err(&nve,NVOPEN,i);
		log(&nve);
	}
	else if(nvread(OFF_CHKCOUNT,&checkstop_count,1,MIONVGET) < 0) {	/* read check. count */
		i += bld_nv_err(&nve,CHECKSTOP_COUNT,i);
		if(nvclose() < 0)
			(void)bld_nv_err(&nve,NVCLOSE,i);
		
		log(&nve);
	}
	else if(checkstop_count == 0) { /* nothing to do */
		if(nvclose() < 0) {
			(void)bld_nv_err(&nve,NVCLOSE,i);
			log(&nve);
		}
	}
	else if(nvread(OFF_CHKPTR,&checkstop_ptr,4,MIONVGET) < 0) {		/* read check. location */
		i += bld_nv_err(&nve,CHECKSTOP_PTR,i);
		if(nvclose() < 0)
			(void)bld_nv_err(&nve,NVCLOSE,i);
		
		log(&nve);
	}
	else if ((data_len=build_checkstop_data(checkstop_ptr,checkstop_data)) < 0) {
		i += bld_nv_err(&nve,CHECKSTOP_DATA,i);
		if(nvclose() < 0)
			(void)bld_nv_err(&nve,NVCLOSE,i);
		
		log(&nve);
	}
	else if(nvwrite(OFF_CHKCOUNT,&zero,1) < 0) { /* zero out count */
		i += bld_nv_err(&nve,NVWRITE,i);
		if(nvclose() < 0)
			(void)bld_nv_err(&nve,NVCLOSE,i);
		
		log(&nve);
	}
	else if(nvclose() < 0) {
		(void)bld_nv_err(&nve,NVCLOSE,i);
		log(&nve);
	}
	else {
		setchkfile();
		if((fd = open(Chkfile,O_WRONLY|O_TRUNC|O_CREAT,0644)) < 0) {
			perror(Chkfile);
		}
		else if((rv=write(fd,checkstop_data,data_len)) != data_len) {		/* write data */
			if(rv >= 0)
				errno=ENOSPC;
			perror(Chkfile);
			close(fd);
		}
		else {
			e.err_rec.error_id = ERRID_CHECKSTOP;					/* log error */
			VCPY("sysplanar0",e.err_rec.resource_name);
			VCPY(Chkfile,e.chk_filename);
			e.chk_count = checkstop_count;
			errlog(&e,sizeof(e));
			close(fd);
		}
	}
}

#define ADDRESS_MASK 0x00FFFFFF
#define PARTITION_MASK  0x01000000
#define LENGTH_MASK  0xFF000000

/* This macro calculates the length of the checkstop data in a
   partition.  The input is a 4-byte integer, read from the
   start of the partition.  To get the length, we need only
   the value of the 1st byte, multiplied by 1024. */

#define DATA_LEN(len_and_addr) \
	((len_and_addr & LENGTH_MASK) >> 24) * 1024


/*
 * NAME:     build_checkstop_data
 * FUNCTION: Read in the checkstop data from the checkstop logout
 *           area.  
 * INPUTS:   Pointer to first byte of checkstop logout area 
 * RETURNS:  Checkstop data length  = Success
 *                              -1  = Failure
 */
static int
build_checkstop_data(chkstop_ptr,chkdata_buf)
{
/* This routine will read the checkstop data from the checkstop
   logout area into chkdata_buf.  The way to do this on UP 
   machines is to just read 9K of data from the address pointed 
   to by checkstop_ptr, as is illustrated by the following:

   NVRAM address 0x030C:
    Partition byte 
    0 = Not Partitioned
     - 1 byte -        - 3 bytes -
    -------------------------------------------
   | 0000 0000 |       checkstop_ptr           |
    -------------------------------------------
                            |
    -------------------------    
   |
   V              9 * 1024 bytes ...
    ------------------------------------------->
   |   checkstop data ...
    ------------------------------------------->


   The design for BUMP machines is different, however, in 
   that the checkstop logout area is now PARTITIONED in 
   the following way:

   NVRAM address 0x030C:
    Partition byte 
    1 = Partitioned
     - 1 byte -        - 3 bytes -
    -------------------------------------------
   | 0000 0001 |       checkstop_ptr           |
    -------------------------------------------
                            |
    -------------------------    
   |
   | (4 information bytes: how many bytes of
   |  checkstop data in this partition and address
   |  of next partition if this is not the last.)
   V - 1 byte -       - 3 bytes -                X * 1024 bytes ...
    ------------------------------------------  ------------------->
   | 0000 000X|       nextaddr                |  checkstop data ...
    ------------------------------------------  ------------------->
                            |
    -------------------------    
   |
   V - 1 byte -       - 3 bytes -                X * 1024 bytes ...
    ------------------------------------------- ------------------->
   | 0000 000X | 0000 0000 0000 0000 0000 0000 | checkstop data ...
    ------------------------------------------- ------------------->

   Note: A partitioned checkstop logout area is not the same as multiple
         checkstop logout areas.  The current BUMP firmware design
         does not provide for multiple checkstop logout areas, so
         this cannot be built into the code yet.  A partitioned
         checkstop logout area is considered to contain data from
         one checkstop in several partitions. This code will 
         concatenate the partitions into one contiguous buffer
         of checkstop data before writing it to a file.

   The maximum size of the checkstop logout area in BUMP machines 
   is 64K.
*/

	int nextaddr;	
	int info_ptr; /* Points to 4 information bytes at start of partition */
	int idx=0;    /* Used as index into checkstop data buffer */
	int rv = CHKSIZE_UP;
	int len;
	
	if((chkstop_ptr & PARTITION_MASK) == PARTITION_MASK) { /* MP machine? */	
		nextaddr = chkstop_ptr & ADDRESS_MASK;
		for (idx=0; nextaddr > 0; idx=idx+len) {	
			if (nvread(nextaddr, &info_ptr, 4, MIONVGET) < 0) {
				rv = -1;
				break;
			}
			else if(nvread(nextaddr+4,chkdata_buf+idx,(len=DATA_LEN(info_ptr)),MIONVGET)) {
				rv = -1;
				break;
			}
			else
				nextaddr = info_ptr & ADDRESS_MASK; 
				rv = idx+len;
		}
	}		
    /* If not an MP machine, read 9K of data from address pointed
       to by chkstop_ptr */
	else if(nvread(chkstop_ptr,chkdata_buf,CHKSIZE_UP,MIONVGET) < 0)
		rv = -1;
	
	return(rv);
	
}

/* NAME:     setchkfile
 * FUNCTION: Set up the file name to hold the checkstop  data.
 *	Checkstop files are named checkstop.A and checkstop.B
 *	Set Chkfile to the name of the earliest, so that the names "wraparound".
 * INPUTS:   none 
 * RETURNS:  none
 */

static
setchkfile()
{
	int i;
	struct stat statbuf;
	char filename[128];
	int o_mtime;
	int o_suffix;
	int suffix;

	o_mtime  = 0x7fffffff;			/* big positive number */
	o_suffix = 'A';
	uname(&uts);
	for(i = 0; i < MAX_CHK_STOP; i++) {
		suffix = i + 'A';
		sprintf(filename,"/usr/lib/ras/checkstop.%s.%c",uts.machine,suffix);
		if(stat(filename,&statbuf)) {		/* not exist. use this filename */
			o_suffix = suffix;
			break;
		}
		if(o_mtime > statbuf.st_mtime) {	/* find oldest mtime */
			o_mtime  = statbuf.st_mtime;
			o_suffix = suffix;
		}
	}
	sprintf(filename,"/usr/lib/ras/checkstop.%s.%c",uts.machine,o_suffix);
	Chkfile = stracpy(filename);
}

/*
 * NAME:     nvopen
 * FUNCTION: Open /dev/nvram.
 * INPUTS:   none 
 * RETURNS:  1 - success
 *           -1 - failure
 */

static int
nvopen()
{
	static msgflg;

	if(Nvfd >= 0)
		return(1);
	if((Nvfd = open("/dev/nvram",O_RDWR)) < 0) {	/* open /dev/nvram */
		if(!msgflg) {
			msgflg++;
			perror("/dev/nvram");
		}
		return(-1);
	}
	return(1);
}

/*
 * NAME:     nvclose
 * FUNCTION: Close /dev/nvram.
 * INPUTS:   none 
 * RETURNS:   1 - success
 *			 -1 - failure
 */

static int
nvclose()
{
	int	rc=1;

	if(Nvfd >= 0 && close(Nvfd) < 0)
		rc = -1;
	Nvfd = -1;
	return(rc);
}

/*
 * NAME:     nvread
 * FUNCTION: read from /dev/nvram.
 * INPUTS:   buf, count, offset, cmd 
 * RETURNS:  -1 - failure
 *            0 - success
 */

static
nvread(offset,buffer,count,cmd)
{
	MACH_DD_IO mddrec;

	if(Nvfd < 0)
		return(-1);
	mddrec.md_addr = offset;
	mddrec.md_data = (char *) buffer;
	mddrec.md_size = count;
	mddrec.md_incr = MV_BYTE;
	if(ioctl(Nvfd,cmd,&mddrec) < 0)
		return(-1);
	return(0);
}

/*
 * NAME:     nvwrite
 * FUNCTION: write to /dev/nvram.
 * INPUTS:   buf, count, offset 
 * RETURNS:  -1 - failure
 *            0 - success
 */

static
nvwrite(offset,buffer,count)
{
	MACH_DD_IO mddrec;

	if(Nvfd < 0)
		return(-1);
	mddrec.md_addr = offset;
	mddrec.md_data = (char *) buffer;
	mddrec.md_size = count;
	mddrec.md_incr = MV_BYTE;
	if(ioctl(Nvfd,MIONVPUT,&mddrec) < 0)
		return(-1);
	return(0);
}


#define MC_SIZE		20		/* Original M.C. error record size */
#define MC_SIZE_604	12		/* New 604 M.C. error record size */

#define MC_MASK		0x80		/* Original M.C. indicator mask */
#define MC_604		0x84		/* New 604 M.C. magic number */
#define OFF_MACHINECHK	0x0368

#ifndef ERRID_MACHINECHECK
#define ERRID_MACHINECHECK 0x00000001
#endif
#if	!defined(ERRID_MACHINE_CHECK_604)
#define	ERRID_MACHINE_CHECK_604	0x5471966a	/* Machine check (604) */
#endif

/*
 * NAME:     machinecheck
 * FUNCTION: read out the machine check info from nvram and create
 *           an error log entry for it.
 * INPUTS:   none  (read from /dev/nvram)
 * RETURNS:  none
 */

static
machinecheck()
{
	char buf[MC_SIZE];
	struct {
		struct err_rec0 err_rec;
		char mc_data[MC_SIZE];
	} e;

	struct nv_err nve;	/* will hold nvram error log entry */
	int i=1;

	/* set up for possible nvram i/o error log entry */
	init_be_internal(&nve, ERRID_LOG_NVRAM);

	nve.detail_data[0] = '\n';

	if(nvopen() < 0) {
		(void)bld_nv_err(&nve,NVOPEN,i);
		log(&nve);
	}
	else if(nvread(OFF_MACHINECHK,buf,MC_SIZE,MIONVGET) != 0) {
		i += bld_nv_err(&nve,MACHINECHECK_DATA,i);
		if(nvclose() < 0) 
			(void)bld_nv_err(&nve,NVCLOSE,i);
		
		log(&nve);
	}
	else if(buf[0] == MC_604) {			/* new 604 M.C. format */
		e.err_rec.error_id = ERRID_MACHINE_CHECK_604;	/* fill in error_id */
		VCPY("sysplanar0", e.err_rec.resource_name);	/* resource_name */
		memcpy(e.mc_data, &buf[sizeof(int)],		/* and detail_data */
						MC_SIZE_604);	/* (first int skipped) */
		errlog(&e, sizeof(e.err_rec) + MC_SIZE_604);	/* log the error */
		buf[0] = 0;					/* clear M.C. ... */
		if(nvwrite(OFF_MACHINECHK,&buf[0],1) < 0) {	/* clear M.C. ... */
			i += bld_nv_err(&nve,NVWRITE,i);
			if(nvclose() < 0)
				(void)bld_nv_err(&nve,NVCLOSE,i);
			log(&nve);
		}
	}
	else if(buf[0] & MC_MASK) {
		e.err_rec.error_id = ERRID_MACHINECHECK;		/* fill in error_id */
		VCPY("sysplanar0",e.err_rec.resource_name);		/* resource_name */
		memcpy(e.mc_data,buf,sizeof(e.mc_data));		/* and detail_data */
		errlog(&e,sizeof(e));							/* log the error */
		buf[0] &= ~MC_MASK;								/* clear mc bit */
		if(nvwrite(OFF_MACHINECHK,&buf[0],1) < 0) {		/* clear mc bit */
			i += bld_nv_err(&nve,NVWRITE,i);
			if(nvclose() < 0)
				(void)bld_nv_err(&nve,NVCLOSE,i);
			
			log(&nve);
		}
	}

	if(nvclose() < 0) {		/* will not fail if nvram not open... */
		(void)bld_nv_err(&nve,NVCLOSE,i);
		log(&nve);
	}
}

/*
 * NAME:     child
 * FUNCTION: errdemon loop
 * 1. Close the three tty files and call setpgrp(0) to detach from tty signals.
 * 2. Call siginit to catch signals.
 * 3. Loop read /dev/error. Each return from a read of /dev/error will return
 *    one error log entry. The return value will be the number of bytes in the
 *    (variable length) erec structure. EOF means that errstop has been run.
 *    For each loggable erec entry:
 *    a. Fill in a T_errlog structure, which corresponds to the errlog object.
 *    b. Get the VPD and location from CuDv and fill in T_errlog
 *    c. Copy the binary detail_data into the T_errlog.el_detail_data string
 *       by converting 00 to 3030 (binary to ascii) using the memxcpy routine.
 *    d. Write the entry using udb_loginsert.
 *    e. Perform error notification using udb_notify.
 * INPUTS:  NONE
 * RETURNS: NONE
 */

static
child()
{
#define SBUF_SIZE	128
	char sbuf[SBUF_SIZE];
	int	nc;
	int rv;
	char *cp;
	struct errstat errstat;		/* errdd status */
	struct buf_entry e;			/* structure to hold error record. */
	udb_init();

	cp = vset(RASDIR);
	if(chdir(cp))
		perror(cp);
	fclose(stdin);
	fclose(stdout);
	fclose(stderr);
	signal(SIGINT,SIG_IGN);
	signal(SIGQUIT,SIG_IGN);
	setpgrp();				/* detach from the terminal */
	if(ioctl(Errctlfd,ERRIOC_STAT,&errstat))
		perror("ERRIOC_STAT");
	else if(errstat.es_opencount == 1)
		notifydelete_pers();

	siginit();

	log_onoff(ERRID_ERRLOG_ON);

	checkstop();		/* Possibly log checkstop error info */
	machinecheck();		/* Possibly log machine check error info */
	bump();             /* Possibly bringup multiprocessor (BUMP) */
                        /* error table data to read.*/

	for(;;) {
		rv = read(Errorfd,&e,EREC_MAX);
		switch(rv) {
		case 0:					/* EOF caused by errstop */
			childexit(0);
		case -1:				/* ERROR */
			nc = sprintf(sbuf,"errdemon: read from error device %s",Errorfile);
			perror(sbuf);
			childexit(1);
		}

		log(&e);	/* log the entry */
#ifdef DUMPLOG
		dump_log(&e,rv);
#endif /* DUMPLOG */
	}
}

#ifdef DUMPLOG

#define DUMPF "/tmp/errlog.dump"
FILE *dumpfp = (FILE *)NULL;

static
dump_log(char *p,int len)
{
	int i;

	
	if (!dumpfp) {
	if ((dumpfp = fopen(DUMPF,"w")) == (FILE *)NULL)
		exit(1);
	}


	if (Global_Firstflag) {
		fprintf(dumpfp,"\nThis file contains, for each entry that is logged through the buffer,");
		fprintf(dumpfp,"\nthe contents of the buffer followed by one line of log pointer management");
		fprintf(dumpfp,"\nvalues.  To interpret the buffer contents, refer to the buf_entry structure");
		fprintf(dumpfp,"\nin errlg/errlg.h.  The five pointer management values are listed as follows:");
		fprintf(dumpfp,"\nsequence number, entry length, new inoff, new outoff, and new staleoff. \n");
		Global_Firstflag = FALSE;	
	}	

	fprintf(dumpfp,"\n-- %d bytes at %x\n",len,p);
	for (i=0; i<len; i++, p++) {
		if ((i % 16) == 0) {
			if (i>0) fprintf(dumpfp,"\n");
			fprintf(dumpfp,"%08x ",i);
		}
		if ((i % 4) == 0) fprintf(dumpfp," ");
		fprintf(dumpfp,"%02x",*p);
	}
	fprintf(dumpfp,"\n");

	fprintf(dumpfp,"\n--  %d, %d, %d, %d, %d",Global_Sequence,Global_Entry_Length,Global_New_Inoffb,Global_New_Outoff,Global_New_Staleoff);
	fprintf(dumpfp,"\n");

	if (Global_Wrap) { 
		Global_Wrap_Count+=1;
		fprintf(dumpfp,"\n-- Log file wrapped - Wrap_Count equals %d\n",Global_Wrap_Count);
	}

	fflush(dumpfp);

}

#endif /* DUMPLOG */

#define	OFF_IPLDIR              0x0080
#define BUMPTBL_HDR_SIZE        0x0020
#define ERRMSGS_MAX_SIZE        0x3fe0
#define EXPECTED_SYS_INFO_SIZE  0x0068
#define EXPECTED_IPL_DIR_SIZE   0x0200
#define NEXTADDR_MASK           0x00ffffff
#define BUMP_FILENAME_MAX       0x0039

int               tbl_num;


/*
 * NAME:     bump
 * FUNCTION: read out the bringup multiprocessor (bump) error
 *           log table(s) from nvram and copy to 
 *           /usr/lib/ras/bumperrlg.<machine_id>.<tbl_num>
 * INPUTS:   none  (read from /dev/nvram)
 * RETURNS:  none
 */

static
bump()
{

	struct	ipl_directory    ipl_dir; 
	struct	system_info      sys_info;

	struct nv_err	nve;  /* will hold nvram error log entry */
	int i=1;
	
	/* set up for possible nvram i/o error log entry */
	init_be_internal(&nve, ERRID_LOG_NVRAM);

	nve.detail_data[0] = '\n';

	/* The relative address into nvram of the bump error log table
       will be kept in the SP_Error_Log_Table field of the system_info
       structure in the ipl control block.  So, to get this address,
       the ipl_directory structure in the ipl control block will 
       first be retrieved.  In this will be stored the address and
       size of the system_info structure.  If the size of the 
       ipl_directory or system_info structures is less than some
       predetermined expected sizes, we are dealing with pre-bump-
       table hardware, and so we will not continue with the bump
       code.  Otherwise, if the sizes of the ipl_directory and 
       system_info structures are greater than or equal to the
       expected sizes, we will go on to get the system_info
       structure out of the ipl control block and use the 
       SP_Error_Log_Table field of the system_info structure to
       get to the (first) bump error log table. */

	if (nvopen() < 0) {
		(void)bld_nv_err(&nve,NVOPEN,i);	
		log(&nve);
	}
	else if(nvread(OFF_IPLDIR,&ipl_dir,sizeof(ipl_dir),MIOIPLCB) < 0)	{ 
		/* read ipl directory */
		i += bld_nv_err(&nve,IPL_DIR,i);
		if(nvclose() < 0)	
			(void)bld_nv_err(&nve,NVCLOSE,i);
		
		log(&nve);
	}
	else if(( ipl_dir.ipl_info_offset - OFF_IPLDIR ) < EXPECTED_IPL_DIR_SIZE )	{ 
       /* Calculate the real size of the ipl directory by subtracting */
       /* the ipl_directory offset from the offset of the ipl_info    */
       /* structure that immediately follows the ipl_directory        */
       /* structure in the ipl control block.                         */
		if(nvclose() < 0) {	
			(void)bld_nv_err(&nve,NVCLOSE,i);
			log(&nve);
		}
	}
	else if(ipl_dir.system_info_offset <= 0 ) {
		/* Check to make sure that the system_info_offset contains */
        /* a valid value.                                          */
		if(nvclose() < 0) {	
			(void)bld_nv_err(&nve,NVCLOSE,i);
			log(&nve);
		}
	} 
	else if(ipl_dir.system_info_size < EXPECTED_SYS_INFO_SIZE) {
		if(nvclose() < 0) {	
			(void)bld_nv_err(&nve,NVCLOSE,i);
			log(&nve);
		}
	} 
	else if(nvread(ipl_dir.system_info_offset,&sys_info,sizeof(sys_info),MIOIPLCB) < 0 ) {
		i += bld_nv_err(&nve,SYS_INFO,i);
		if(nvclose() < 0)	
			(void)bld_nv_err(&nve,NVCLOSE,i);
		
		log(&nve);
	}
	else if(sys_info.SP_Error_Log_Table <= 0) {
		/* Check to make sure that the SP_Error_Log_Table field */
		/* contains a valid value.                              */
		if(nvclose() < 0) {	
			(void)bld_nv_err(&nve,NVCLOSE,i);
			log(&nve);
		}
	} 
	else if( process_bumptbls(&nve,&sys_info) < 0 )
		; /* If process_bumptbls() fails, don't log a bump error. */
	else {	
		logbump();
		if(nvclose() < 0) {	
			(void)bld_nv_err(&nve,NVCLOSE,i);
			log(&nve);
		}
	}
	
} /* end bump() */


char	errmsgs[ERRMSGS_MAX_SIZE];
int		errmsgs_readoff;
int		errmsgs_size;

struct {
	int next_tbl_offset;  /* offset of next error logging table */
	int sizeof_errmsgs;   /* error messages storage area size in bytes */
	int startoff;	      /* offset to start reading error entries  */
	int endoff;	      /* offset to end reading error entries */
	int nextoff;	      /* offset to start writing errors- used by BUMP */
	int crc;	      /* used by BUMP for checking integrity of data */
	int reserved[2];      /* reserved for later use */
} bumptbl_hdr;			

int bumptbl_addr;


static
process_bumptbls(struct nv_err* nve,struct system_info *sys_info)
{
	int rc = 0;   /* 0=good return; -1=bad return                   */
                      /* bad return will cause logbump() in the calling */
                      /* routine to be skipped.                         */
	int i = 1;


	bumptbl_addr = sys_info->SP_Error_Log_Table; 

	for (tbl_num=0; bumptbl_addr > 0 ; ++tbl_num)
	{
		if ( nvread(bumptbl_addr,&bumptbl_hdr,sizeof(bumptbl_hdr),MIONVGET) < 0 )  {
			i += bld_nv_err(nve,BUMPTBL_HDR,i);
			if(nvclose() < 0)	
				(void)bld_nv_err(nve,NVCLOSE,i);
		
			log(nve);

			/* If a failure occurred before successfully processing the */
			/* first table, there will be nothing to log, so fail the   */
			/* routine.  Otherwise, pass it so that an error can be     */
			/* logged to show at least the bump tables that were        */
			/* successfully written.                                    */

			if (tbl_num == 0)
				rc = -1;      

			break;
		}
		else if (process_bumphdr() < 0) { 

		/* process_bumphdr() will calculate the errmsgs_readoff  
                (the offset into nvram to begin reading) from the start-
                off in the table header - and it will calculate the size 
                of the data stored in the table (errmsgs_size) by   
                subtracting the startoff from the endoff. */ 

		/* process_hdr() will fail if there are no errmsgs
	   	in the current table (ie. startoff = endoff) or if table   
	   	is corrupt. */  
			if(nvclose() < 0){	
				(void)bld_nv_err(nve,NVCLOSE,i);
				log(nve);
			}

			if ( tbl_num == 0 )	
				rc = -1; 

			break;
		}
		else if (nvread(errmsgs_readoff,&errmsgs,errmsgs_size,MIONVGET) < 0)  {
			i += bld_nv_err(nve,BUMPTBL_ERRMSGS,i);
			if(nvclose() < 0)	
				(void)bld_nv_err(nve,NVCLOSE,i);
		
			log(nve);

			if ( tbl_num == 0 )
				rc = -1;

			break;
		}
		else if ( writeto_bumpfile() < 0 ) {

			if ( tbl_num == 0 )
				rc = -1;

			break;
		}
		else if (nvwrite(bumptbl_addr,&bumptbl_hdr,sizeof(bumptbl_hdr)) < 0) {
                /* Write the header back out with its startoff equal to its endoff.*/
                /* This will have the effect of clearing out this bump table, so   */ 
                /* we don't reprocess the same information and log an error every  */
                /* time the errdemon is restarted.                                 */
			i += bld_nv_err(nve,NVWRITE,i);
			if(nvclose() < 0)	
				(void)bld_nv_err(nve,NVCLOSE,i);
		
			log(nve);

			if ( tbl_num == 0 )
				rc = -1;

			break;
		}  
		else { 
			/* The lower order three bytes of the first word of the  */
                        /* bump error log table contain the relative offset into */
                        /* nvram for the next bump error log table. Mask off the */
                        /* first byte to get the offset.                         */ 
			bumptbl_addr = bumptbl_hdr.next_tbl_offset & NEXTADDR_MASK;
		}
	}  /* end for loop */	

	return(rc);

} /* end process_bumptbls() */


static
process_bumphdr()
{
	int rc; /* 0=good return; -1=bad return */
	
	/* calculate the size of the error messages area to be read */
	errmsgs_size = bumptbl_hdr.endoff - bumptbl_hdr.startoff;

	/* If the errmsgs_size is greater than the maximum errmsgs size,
	   or less than zero, something's wrong with this table - it's
	   corrupted and can't be processed. If it's equal to zero, 
           there are no error messages in this table to process. */
	if ( (errmsgs_size > ERRMSGS_MAX_SIZE) || (errmsgs_size <= 0))
		rc = -1;
	
	else {
		rc = 0;

		/* calculate the offset to begin reading the error messages */
		errmsgs_readoff = bumptbl_addr + BUMPTBL_HDR_SIZE + bumptbl_hdr.startoff;

		/* Clear the table by setting the startoff     */
		/* equal to the endoff for the nvwrite         */
		/* operation of the table header that will     */
                /* follow.                                     */
		bumptbl_hdr.startoff = bumptbl_hdr.endoff;	
	}

	return(rc);

}  /* end process_bumphdr() */


static
writeto_bumpfile()
{
	char   Bumpfile[FILENAME_MAX];
	int fd;
	int rv;
	int rc; /* 0=good return; -1=bad return */
	
	uname(&uts);
	sprintf(Bumpfile,"/var/adm/ras/bumperrlg.%s.%d",uts.machine,tbl_num+1);
	if ( (fd = open(Bumpfile,O_WRONLY|O_TRUNC|O_CREAT,0644)) < 0 ) {
		perror(Bumpfile);
		rc = -1;
	}
	else if((rv=write(fd,errmsgs,errmsgs_size)) != errmsgs_size) {
		if(rv >= 0) 
			errno=ENOSPC;
		perror(Bumpfile);
		close(fd);
		rc = -1;
	}
	else  { 
		rc = 0;
		close(fd);

	return(rc);

	}	
} /* end writeto_bumpfile() */


static
logbump()
{
	struct {
		struct	err_rec0 err_rec;
		int		number_of_tables;
		char 	bump_filename[BUMP_FILENAME_MAX];
	} e;

	e.err_rec.error_id = ERRID_BUMP_ERROR_TABLES;
	VCPY("bump",e.err_rec.resource_name);
	e.number_of_tables = tbl_num;
	sprintf(e.bump_filename,"/var/adm/ras/bumperrlg.%s.#",uts.machine);
	errlog(&e,sizeof(e));

} /* end logbump() */	
