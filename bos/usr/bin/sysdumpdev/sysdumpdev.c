static char sccsid[] = "@(#)93  1.43  src/bos/usr/bin/sysdumpdev/sysdumpdev.c, cmddump, bos41J, 9515B_all 4/13/95 13:43:06";

/*
 * COMPONENT_NAME: CMDDUMP    system dump control and formatting
 *
 * FUNCTIONS: sysdumpdev
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/err_rec.h>
#include <sys/dump.h> 
#include <sys/ioctl.h>
#include <sys/cfgodm.h>
#include <sys/errids.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <locale.h>
#include <sys/access.h>
#include <sys/vmount.h>
#include <sys/vnode.h>
#include <cf.h>
#include <sys/cfgdb.h>
#include <ras.h>
#include <cmddump_msg.h>
#include <errlg/SWservAt.h>	
#include <nl_types.h>
#include <sys/systemcfg.h>
#define MCS_CATALOG "cmddump.cat"

/* Network dump */
#include <sys/socket.h>
#include <sys/file.h>

#include <net/af.h>
#include <netdb.h>
#include <net/if.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/in_netarp.h>
#include <sys/select.h>
#include <rpc/rpc.h> 
#include <rpc/clnt.h> 
#include <sys/vfs.h>
#include <sys/vmount.h>
#include <sys/vnode.h>
#include <sys/syspest.h>
#include <grp.h>

#define NFS_FHSIZE 32
#define MOUNTPROG 100005
#define MOUNTPROC_MNT 1
#define MOUNTVERS 1
#define ERPC	(10000)

#define FAILED_COPY_MESSAGE "15"
#define ALLOWED_COPY_MESSAGE "18"

static bool_t  xdr_path();
static bool_t  xdr_fhstatus();
static bool_t  xdr_fhandle();
 /*
 * Fake errno for RPC failures that don't map to real errno's
 */

struct fhstatus {
	       int fhs_status;
               fhandle_t fhs_fh;
       };

struct hostent         *hp;
struct fhstatus		fhs;
/* Network dump */

extern char *Progname;

#include <sys/devinfo.h>

#define VCPY(from,to_array) \
	strncpy(to_array,from,sizeof(to_array)); \
	to_array[sizeof(to_array)-1] = '\0';

extern int errno;
extern optind;  /* set by getopt system call */
extern char *optarg;  /* set by getopt system call */

static int Dmpfd = -1;
static char *Pdevname = NULL;  /* pointer to the string that contains the current prim dump device */
static char *Sdevname = NULL;  /* pointer to the string that contains the current sec dump device */
static char *Sdevname2 = NULL;  /* pointer to the string that contains the current sec dump device */
static char *tPdevname = NULL; /* pointer to the string that contains the default prim dump device */
static char *tSdevname = NULL; /* pointer to the string that contains the default sec dump device */
static char *tSdevname2 = NULL; /* pointer to the string that contains the default sec dump device */
static char *nPdevname = NULL; /* temporary work pointer */
static char *nSdevname = NULL; /* temporary work pointer */
static char *Pdevarg = NULL;   /* pointer to the string of the input primary dump device */
static char *remote_fn = NULL;   /* pointer to the string of the remote filename*/
static char *Sdevarg = NULL;   /* pointer to the string of the input secondary dump device */
static char *Copyfilename = NULL;   /* pointer to the string of the file where the dump is copied */
static char *Forcecopyflag = NULL;   /* pointer to the string of the forcecopy flag */

static char *Sysdump = "/dev/sysdump";
static char *SysNull = "/dev/sysdumpnull";
static char *SysFile = "/dev/sysdumpfile";
static char *DevNull = "/dev/null";

static int permflg = 0;         /* -P make the dump device specified
 			           by the -p or -s flags permanent  */
static int odmflg = 0;          /* this flag is used to see if permanent 
                                   setting should be used                  */
static int infoflg = 0;		/* DMP_IOCINFO */
static int lineflg = 0;         /* show info line containing size and device name */
static int showflg = 0;		/* show current dump devices */
static int quietflg = 0;	/* -q don't print values after init by odm */
static int sizeflg = 0;         /* -e estimate the size of dump in bytes   */



static struct CuAt *cuattr;
static char *Corearg = NULL;     /* used with -D / -d flags to specify where
				    the dump is to be copied at the boot time */
static int Coreval = 0;   	 /* value for forcecopydump system attribute */

/* This structure is used to access the attribute ODM data base */
struct od_data {
    char *attr_name;
    struct SWservAt *swservp;
};

struct od_data od_list[] = {
#define TPRIMARY 0
    { "tprimary", NULL },
#define TSECONDARY 1
    { "tsecondary", NULL },
#define PRIMARY 2
    { "primary", NULL },
#define SECONDARY 3
    { "secondary", NULL }, 
#define AUTOCOPY 4
    { "autocopydump", NULL }, 
#define FORCECOPY 5
    { "forcecopydump", NULL } 
#define OD_LIST 6
};

#define INFO_DUMP 1
#define INFO_TYPE 2
#define INFO_LINE 3
#define CRITSIZE  256  

static void dmpinfo();
static void odminit();
static void odmread();
static void odmwrite();
static long rdev();
static void usage_dev();
static int getfh();
static int check_remote_hostname();
static int reclaim_remote_file_space();
static void error_if_multiprocessor();

/*
 * NAME:     main
 * FUNCTION: 
 * 		process the command line options
 *		invoke the specific function to do 
 * 		    the specified tasks.
 *		the supported tasks are:
 *			-L list the information about the previous dump
 *			-e estimate the size of the system dump
 *			-r free the space used for remote dump on the server
 *			-z list the information about the most recent dump
 *			-d Directory  specifies the directory where the
 * 			   dump is copied to at boot time. If the copy
 *            		   fails the system continues to boot.
 *			-D Directory  specifies the directory where the
 * 			   dump is copied to at boot time. If the copy
 *            		   fails then a  menu is displayed to allow user
 *			   to copy the dump. 
 *			-l list the dump device information
 *			-p configure the primary dump device
 *			-s configure the secondary dump devices
 *			-P make permanent the dump device specified by
 *		           the -p or -s flags. Can only be used with -p or -s
 *			   flags. 
 * INPUTS:   argc, argv
 * DEPENDENCIES:
 *	This function depends on the following commands:
 *		ping
 *		lslv
 * 		
 * RETURNS:  0 - successful
 *          -1 - unsuccessful
 *
 */
main(argc,argv)
char *argv[];
{
    int c;
    struct dumpinfo dumpinfo;
    struct devinfo devinfo;
    dev_t nulldev = rdev(SysNull,TRUE);
    dev_t pridev, secdev, pridevn, secdevn;
    int usage_cnt = 0;
    int    len,rv;
    char    *next;
    char    *pathname;
    char    cmdline[80];
    char    location_code[NFS_FHSIZE];
    int cnt;

    setlocale(LC_ALL,"");				/* set Locale */
    catinit(MCS_CATALOG);			/* init message catalog */
    setprogname();				/* set the Progname */
    while((c = getopt(argc,argv,"eqLzlPp:r:s:D:d:")) != EOF) {
	switch(c) {
	case 'q':
	    quietflg++;
	    break;
        case 'e':
            sizeflg++;
            break;
	case 'l':
	    showflg++;
	    break;
	case 'L':
	    infoflg++;
	    break;
	case 'z':
	    lineflg++;
	    break;
	case 'P':
	    permflg++;
	    break;
	case 'p':
	    Pdevarg = stracpy(optarg); 
	    break;
	case 'r':
	    remote_fn = stracpy(optarg);
	    break;
	case 's':
	    Sdevarg = stracpy(optarg);
	    break;
        case 'D':
            Corearg = stracpy(optarg); 
	    Coreval = 1;  /* used to differencate between d and D */
            break;
        case 'd':
            Corearg = stracpy(optarg);
	    Coreval = 0;  /* used to differenciate between d and D */ 
            break;

	default:
	    usage_cnt++;
	    }
	}
    if ((optind < argc) || usage_cnt)
	    usage_dev();

    if ((Dmpfd = open(Sysdump,0)) < 0)
	cat_fatal(CAT_SYSDUMP,
	    "Cannot open dump device %s.\n\t%s",Sysdump,errstr());

    if (infoflg) { /* sysdumpdev -L */
	dmpinfo(Dmpfd, DMP_IOCSTAT, INFO_DUMP);
	exit(0);
    }

    if (sizeflg) { /* sysdumpdev -e */
       rv = calculate_size(Dmpfd);
       exit(rv);
    }

    if (remote_fn) { /* sysdumpdev -r */
       rv = reclaim_remote_file_space(); 
       exit(rv);
    }

    if (lineflg) { /* sysdumpdev -z */
	dmpinfo(Dmpfd,DMP_IOCSTAT2, INFO_LINE);
	exit(0);
    }

    odminit(); /* Get ready to access ODM data base */

    if (Corearg != NULL) /* sysdumpdev -D|-d */
    {
	rv = set_core_file(Corearg,Coreval);
	exit(rv);
    }

    /* set dumpdevices their defaults */
    odmread(); /* Pdevname, Sdevname, tPdevname, tSdevname are set here */

    if (showflg) {	/* sysdumpdev -l . Print out current assignments */
	dmpinfo(Dmpfd, IOCINFO, INFO_TYPE);
	exit(0);
    }

    if ( permflg && !(Pdevarg || Sdevarg)) { /* -P must be used with
					        -p or -s flag */
	    usage_dev();
           
    }
       


    dmp_cklog(Dmpfd);			/* check if we should log a dump */

    /* If no flag is specified, get the default settings */
    if ( (!showflg & !infoflg & !Pdevarg & !Sdevarg) )
       odmflg++;

    if (!quietflg)  /* -q is not specified then show the primary
			and secondary dump devices after the 
			completion of the device configuration */
	showflg++;
    /* store the names of the current and permanent device for 
    duplication checking  */
    Sdevname2 = Sdevname;
    tSdevname2 = tSdevname;
    if (Sdevarg)
    {
       Sdevname2 = stracpy(Sdevarg);
       if (permflg)
         tSdevname2 = stracpy(Sdevarg);
    }
    /* Take care of the special case: /dev/null  */
    if ( (strcmp(Pdevarg,DevNull) == 0) || (strcmp(Sdevarg,DevNull) == 0) )
	{
               cat_eprint(CAT_INVALID_DEV,
                "%s is not a valid dump device.\n",DevNull);
               exit(1);
	}
		

    if (Pdevarg) {
        /* Check to see if new primary device conflicts with
        **  - the old secondary device. (if not being explicitly changed)
        **  - the new secondary device. (if one is being defined)
        **  - for default setting, also check the default secondary device.
        */
        pridev = rdev(Pdevarg,FALSE);

        if ( permflg ) /* for default setting */
        {
          if ( Sdevarg ) /* new sec. device is being defined */ 
          {
            if ( (strcmp(Pdevarg,SysNull) != 0) && (strcmp(Pdevarg,Sdevarg) == 0 ) ) {
               cat_eprint(CAT_DUP_DEV,
                "Primary and secondary dump devices cannot be the same.\n");
               exit(1);
            }
          }
          else
          {
            if ((strcmp(Pdevarg,SysNull) != 0) && ((strcmp(Pdevarg,Sdevname2) == 0 ) ||  /* current secondary device */
               (strcmp(Pdevarg,tSdevname2) == 0 )))   /* default secondary device */
           {
             cat_eprint(CAT_DUP_DEV,
                "Primary and secondary dump devices cannot be the same.\n");
             exit(1);
           }
          }
          /* if permanent change, everything looks good
             then setting the default setting to the new setting. */
          tPdevname = stracpy(Pdevarg);   
        }
        else
        {
           if ((strcmp(Pdevarg,SysNull) != 0 ) && (strcmp(Pdevarg,Sdevname2) == 0 )) { /* current secondary device */
             cat_eprint(CAT_DUP_DEV,
                "Primary and secondary dump devices cannot be the same.\n");
             exit(1);
           } 

        }
        Pdevname = stracpy(Pdevarg);
    }
    else if ( odmflg )
      Pdevname = stracpy(tPdevname);

    if (Sdevarg) {
        /*  Check to see if the new secondary devices is the same as the
        ** primary device that we are about to set.
        */
        if ( (strcmp(Sdevarg,SysNull) != 0 ) && (strcmp(Sdevarg, Pdevname) == 0 )) { 
            cat_eprint(CAT_DUP_DEV,
 		"Primary and secondary dump devices cannot be the same.\n");
            exit(1);
        }
        if ( permflg )
          if ((strcmp(Sdevarg,SysNull) != 0) && (strcmp(Sdevarg, tPdevname) == 0 ))
          {
             cat_eprint(CAT_DUP_DEV,
                "Primary and secondary dump devices cannot be the same.\n");
             exit(1);
          }
          else tSdevname = stracpy(Sdevarg);
        Sdevname = stracpy(Sdevarg);
    }
    else if ( odmflg )
        Sdevname = stracpy(tSdevname);

    memset(&dumpinfo,0,sizeof(dumpinfo));

 /* Now we are about to configure the primary dump device */

    if (Pdevname) {
        pridev = rdev(Pdevname,FALSE);
	VCPY(Pdevname,dumpinfo.dm_devicename);
        /* Check for network dump device by inspecting the input string */
        /* of the form of hostname:pathname                 */
	nPdevname = stracpy(Pdevname);
        if ( (next = strchr(nPdevname, ':' )) != NULL )
        { 

	  /* Remote dump is not supported on multiprocessor machines. */
	  error_if_multiprocessor();	

          /* separate hostname(or host address) and pathname */
          *next++ = '\0';
          pathname = stracpy(next);
          
          /* ping the remote name to refresh the arp table */
	  sprintf(cmdline,"ping -c 1 -s 1 %s 2>/dev/null 1>/dev/null",nPdevname);
          if ( system(cmdline))
          {
             cat_eprint(CAT_HOSTUNREACH,
                "%s does not respond or is not reachable.\n",nPdevname);
             exit(1);
          }
             
          /* get the IP address of the server (remote host) */ 
	  hp = gethostbyname(nPdevname);
	  if (hp == NULL) {
              cat_eprint(CAT_HOSTUNREACH,
                "%s does not respond or is not reachable.\n",nPdevname);
	      exit(1);
	  }
          
          bcopy(hp->h_addr,(caddr_t)&dumpinfo.dm_hostIP,hp->h_length);
 
          /* Check to make sure the remote host name is not */ 
          /* the same as the local hostname                 */ 
          
          if (rv = check_remote_hostname(nPdevname))
          {
              exit(rv);
          }
 
          /* get the filehandle of the dumpfile on server   */ 
          
          if (rv = getfh(nPdevname,pathname))
          {
              exit(rv);
          }
          else /* save the filehandle for the device driver call */
            bcopy(fhs.fhs_fh.x,&dumpinfo.dm_filehandle,NFS_FHSIZE);
        } 
	/****************************************************************/
	/* Get the location code if the dump device is a logical volume.*/
	/* The location code is stored in dumpinfo.dm_filehandle field. */
	/****************************************************************/
	if ( (strcmp(Pdevname,SysNull) != 0) &&
	     (strcmp(Pdevname,SysFile) != 0) &&
	     (strncmp(Pdevname,"/dev/",5) == 0) )
	{
	  /* Use the "lslv" command to determine if it's a logical volume */
	  next = Pdevname + 5;
	  sprintf(cmdline,"lslv %s 2>/dev/null 1>/dev/null",next);
	  if ( (system(cmdline)) == 0 )
	  {
	    if (rv = get_location_code(&location_code,Pdevname))
		exit(rv);
	    else 
		VCPY(location_code,dumpinfo.dm_filehandle);
	  }
	}
	      
	/* issue ioctl to device driver to set the primary dump device */
	dumpinfo.dm_mmdev = pridev;
	if (ioctl(Dmpfd,DMPSET_PRIM,&dumpinfo) < 0)
        {
         if (quietflg)
          {
            Pdevname = SysNull;
	    VCPY(SysNull,dumpinfo.dm_devicename);
    	    dumpinfo.dm_mmdev = nulldev;
	    if (ioctl(Dmpfd,DMPSET_PRIM,&dumpinfo) < 0)
            {
	        cat_eprint(CAT_DMPSET,
		    "Cannot set primary dump device %s.\n\t%s\n",
		     SysNull, errstr());
                exit(1);
            }
          }
         else 
          {
	    cat_eprint(CAT_DMPSET,
		"Cannot set primary dump device %s.\n\t%s\n",
		Pdevname,errstr());
            exit(1);
          }
        }
    } else {
	VCPY(SysNull,dumpinfo.dm_devicename);
	dumpinfo.dm_mmdev = nulldev;
	if (ioctl(Dmpfd,DMPSET_PRIM,&dumpinfo) < 0)
        {
	    cat_eprint(CAT_DMPSET,
		"Cannot set primary dump device %s.\n\t%s\n",
		 SysNull, errstr());
            exit(1);
        }
    }
/* Now we are about to configure the secondary dump device */
    if (Sdevname) {
	VCPY(Sdevname,dumpinfo.dm_devicename);
        secdev = rdev(Sdevname,FALSE);
	dumpinfo.dm_mmdev = secdev;
        /* Check for network dump device by inspecting the input string */
        /* of the form of hostname:pathname                 */
	nSdevname = stracpy(Sdevname);
        if ( (next = strchr(nSdevname, ':' )) != NULL )
        { 

	  /* Remote dump is not supported on multiprocessor machines. */	
	  error_if_multiprocessor();

          /* separate hostname(or host address) and pathname */
          *next++ = '\0';
          pathname = stracpy(next);
          
          /* ping the remote name to refresh the arp table */
	  /* dependency on the ping command */
	  sprintf(cmdline,"ping -c 1 -s 1 %s 2>/dev/null 1>/dev/null",nSdevname);
          if ( system(cmdline))
          {
             cat_eprint(CAT_HOSTUNREACH,
                "%s does not respond or is not reachable.\n",nSdevname);
             exit(1);
          }
             
          /* get the IP address of the server (remote host) */ 
	  hp = gethostbyname(nSdevname);
	  if (hp == NULL) {
              cat_eprint(CAT_HOSTUNREACH,
                "%s does not respond or is not reachable.\n",nSdevname);
	      exit(1);
	  }
          
          bcopy(hp->h_addr,(caddr_t)&dumpinfo.dm_hostIP,hp->h_length);
 
          /* Check to make sure the remote host name is not */ 
          /* the same as the local hostname                 */ 
          
          if (rv = check_remote_hostname(nSdevname))
          {
              exit(rv);
          }
 
          /* get the filehandle of the dumpfile on server   */ 
          if (rv = getfh(nSdevname,pathname))
          {
              exit(rv);
          }
          else /* save the filehandle for the device driver call */ 
            bcopy(fhs.fhs_fh.x,&dumpinfo.dm_filehandle,NFS_FHSIZE);
        }
	/****************************************************************/
	/* Get the location code if the dump device is a logical volume.*/
	/* The location code is stored in dumpinfo.dm_filehandle field. */
	/****************************************************************/
	if ( (strcmp(Sdevname,SysNull) != 0) &&
	     (strcmp(Sdevname,SysFile) != 0) &&
	     (strncmp(Sdevname,"/dev/",5) == 0) )
	{
	  /* Use the "lslv" command to determine if it's a logical volume */
	  next = Sdevname + 5;
	  sprintf(cmdline,"lslv %s 2>/dev/null 1>/dev/null",next);
	  if ( (system(cmdline)) == 0 )
	  {
	    if (rv = get_location_code(&location_code,Sdevname))
		exit(rv);
	    else 
		VCPY(location_code,dumpinfo.dm_filehandle);
	  }
	}
	/* issue ioctl to device driver to set the secondary dump device*/ 
	if(ioctl(Dmpfd,DMPSET_SEC,&dumpinfo) < 0)
        {
         if (quietflg)
          {
            Sdevname = SysNull;
	    VCPY(SysNull,dumpinfo.dm_devicename);
    	    dumpinfo.dm_mmdev = nulldev;
	    if (ioctl(Dmpfd,DMPSET_SEC,&dumpinfo) < 0)
            {
	        cat_eprint(CAT_DMPSET,
		    "Cannot set secondary dump device %s.\n\t%s\n",
		     SysNull, errstr());
                exit(1);
            }
          }
         else
          {
	    cat_eprint(CAT_DMPSET,
		"Cannot set secondary dump device %s.\n\t%s\n",
		Sdevname,errstr());
            exit(1);
          }
        }
    } else {
	VCPY(SysNull,dumpinfo.dm_devicename);
	dumpinfo.dm_mmdev = nulldev;
	if (ioctl(Dmpfd,DMPSET_SEC,&dumpinfo) < 0)
        {
	    cat_eprint(CAT_DMPSET,
		"Cannot set secondary dump device %s.\n\t%s\n",
		SysNull, errstr());
            exit(1);
        }
    }

    if (permflg) 
	odmwrite(0);
    else odmwrite(2);
    if(showflg)
	dmpinfo(Dmpfd, IOCINFO, INFO_TYPE);
    exit(0);
}

static
void dmpinfo(dfd, iocval, info_type)
int dfd, iocval, info_type;
{
    struct dumpinfo dumpinfo;
    struct devinfo devinfo;
    struct stat copyfilestat;	
    int invalid_devicename = 0;	
    int valid_copyfilename;	
    int fd;	
    char *Primary, *Secondary;
    char *autocopy, *forcecopy;
    char *s;
    char t[100];
    char buffer[MAXPATH];	
    char default_buffer[MAXPATH]; 	/* used to hold default message for catgets() */
    char *allowed = FALSE;
    char *token, *device, *file;
    char *str;	
    char *failed_msg, *allowed_msg;
    nl_catd savecr_catalog;

#define COPYFILENAME "/var/adm/ras/copyfilename"

    bzero(buffer,MAXPATH);
    bzero(default_buffer,MAXPATH);	
	
    switch(info_type) {
    case INFO_DUMP:
	if (0 > ioctl(dfd, iocval, &dumpinfo))
	  cat_fatal(CAT_IOCSTAT,
		"Cannot read dump information from %s.\n\t%s", Sysdump, errstr());

	/*  Check if a number of the fields from the NVRAM are 0.  If they
	** are 0, report that there is no recorded dump.
	*/
	if ( (dumpinfo.dm_mmdev == 0) && (dumpinfo.dm_timestamp == 0) &&
		(dumpinfo.dm_size == 0) ) {
		cat_eprint(CAT_DMP_NOREC, "No previous dumps recorded.\n");
		exit(2);    /* used by SMIT for copying dump to diskette */ 
	}
        /* Check that the devicename has a "/" as the first character, or   */
        /* it has a colon in it to indicate that the dump device is remote. */
        if ( (strcmp(dumpinfo.dm_devicename[0],'/') != 0) &&
             (strchr(dumpinfo.dm_devicename,':') == NULL) )
                invalid_devicename++;

        /* This check is put in to make sure that we don't display garbage if   */
        /* nvram has become corrupted.  We check that the size is not negative, */
        /* the status is valid, and device name is good.  If nvram is corrupt,  */
        /* we call ioctl with DMP_IOCINFO to write zeroes to our nvram block.   */
        if ( (dumpinfo.dm_size < 0) || (dumpinfo.dm_status > 0) ||
             (dumpinfo.dm_status < -3) || (invalid_devicename) )
                {
                if (ioctl(dfd,DMP_IOCINFO,&dumpinfo) < 0)
                        perror("sysdumpdev: ioctl DMP_IOCINFO");
                cat_eprint(CAT_DMP_NOREC, "No previous dumps recorded.\n");
                exit(2);
		}
        strftime(t,100,"%c",localtime(&dumpinfo.dm_timestamp));
	cat_print(CAT_DMP_DEVINFO2,
	       "\n\nDevice name: %s\nMajor device number: %d\nMinor device number: %d\nSize:       %d bytes\nDate/Time:  %s\nDump status:     %d\n",
	    dumpinfo.dm_devicename,
	    major(dumpinfo.dm_mmdev),minor(dumpinfo.dm_mmdev),
	    dumpinfo.dm_size,
	    t,
	    dumpinfo.dm_status);

        /* Put out information about where dump was copied.
           This is kept in /var/adm/ras/copyfilename. */
	if (stat(COPYFILENAME,&copyfilestat) == 0)
		if (copyfilestat.st_mtime > dumpinfo.dm_timestamp)
			valid_copyfilename = TRUE;
		else
			valid_copyfilename = FALSE;

        if ((access(COPYFILENAME, E_ACC) == 0) && valid_copyfilename)
                {
                if  ( (fd = open(COPYFILENAME,O_RDONLY))  > 0 &&
                     (read(fd,&buffer,MAXPATH) > 0) )
                        {
			str = stracpy(buffer); 
                        if (strncmp(str,"/",1) == 0)
                                cat_print(CAT_COPY_FILENAME,
                                        "Dump copy filename: %s\n",buffer);
                        else
				{
				/* The copycore command logs the message numbers of the messages  */
				/* that sysdumpdev should display.  Parse the copyfilename, and   */
				/* print the correct messages.  */ 
				if ((token=strtok(str," ") != 0))
					{
					do
						{
						if (strcmp(token,FAILED_COPY_MESSAGE) == 0)
							;
						else if (strncmp(token,"DEVICE:",7) == 0)
							{
							device = strchr(token,':');
							device = device + 1;
							}
						else if (strncmp(token,"FILE:",5) == 0)
							{
							file = strchr(token,':');
							file = file + 1;
							}
						else if (strcmp(token,ALLOWED_COPY_MESSAGE) == 0)
							{
							allowed = TRUE;
							}
						}
					while((token=strtok(NULL," ")) != NULL);
					}
	
				/* Open the savecr.cat catalog to get the messages. */
				savecr_catalog = catopen("savecr.cat", NL_CAT_LOCALE);

				sprintf(default_buffer,"Failed to copy the dump from %s to %s.\n",device,file);	

				/* Call catgets() to get message number 15 which is the "failed copy" message. */
				failed_msg = stracpy(catgets(savecr_catalog,1,15,default_buffer));

				/* If the default message and the message returned from catgets() */
				/* are different, then print the message returned by catgets()   */
				/* with arguments.  Otherwise, print the default message.  */ 
				if (strcmp(failed_msg,default_buffer) != 0)
					fprintf(stderr,failed_msg,device,file);
				else
					fprintf(stderr,failed_msg);

				/* The "allowed copy" message should be printed. */
				if (allowed)
					{
					bzero(default_buffer,MAXPATH);
					sprintf(default_buffer,"Allowed the customer to copy the dump to external media.\n");	
					allowed_msg = stracpy(catgets(savecr_catalog,1,18,default_buffer));

					/* Print the message returned from catgets().  There are no arguments */
					/* in this message, so we don't need to check the return from catgets(). */
					fprintf(stderr,allowed_msg);
					}

				/* Close the savecr.cat message catalog. */
				catclose(savecr_catalog);
				}
                        }
                }

        break;

    case INFO_LINE:
        /* If the NEEDCOPY flag is off, no data will be passed back
           from the device driver but the return code is still zero.
           In this case, dumpinfo should be filled with zero to indicate 
           there is no new dump recorded.
        */
        bzero(&dumpinfo, sizeof(dumpinfo));

	/* Cannot read dump info */
	if (0 > ioctl(dfd, iocval, &dumpinfo))
                exit(3);

	/*  Check if a number of the fields from the NVRAM are 0.  If they
	** are 0, return with the bad return coded */
	if ( (dumpinfo.dm_mmdev == 0) && (dumpinfo.dm_timestamp == 0) &&
		(dumpinfo.dm_size == 0) ) {
		exit(2);    /* used by SMIT for copying dump to diskette */ 
	}

	/* print size and device name */
	printf("%d %s\n", dumpinfo.dm_size, dumpinfo.dm_devicename);

        break;

    case INFO_TYPE:
	if (0 > ioctl(dfd, iocval, &devinfo))
	  cat_fatal(CAT_IOCINFO,"Cannot read dump device information from %s.\n\t%s", Sysdump, errstr());
	/*
	 * fill in "primary" and "secondary" NLS strings
	 */

        if ( devinfo.un.dump.primary == -1 )
          Pdevname = stracpy("-");
        if ( devinfo.un.dump.secondary == -1 )
          Sdevname = stracpy("-");

          
	cat_string(CAT_PRIMARY,&s,"primary");
	Primary = stracpy(s);
	cat_string(CAT_SECONDARY,&s,"secondary");
	Secondary = stracpy(s);
	cat_string(CAT_AUTOCOPY,&s,"copy directory");
	autocopy = stracpy(s);
	cat_string(CAT_FORCECOPY,&s,"forced copy flag");
	forcecopy = stracpy(s);

	printf("%-20s %s\n", Primary,   Pdevname);
	printf("%-20s %s\n", Secondary, Sdevname);
	printf("%-20s %s\n", autocopy, Copyfilename);
	printf("%-20s %s\n", forcecopy,Forcecopyflag );
        break;
    
    default:
        break;
    }   /* end case */
}

static 
void odminit()
{
    if (0 > odm_initialize()) {
	cat_fatal(CAT_ODMINIT, "Cannot initialize ODM.\n\t%s", errstr());
	exit(-1);
    }

    odm_set_path("/etc/objrepos");
}

static 
void odmread()
{
    struct SWservAt *swservp;
    int i, cnt, rv;

    rv = 0;
    for (i = 0; i < OD_LIST; i++) {
	swservp = (struct SWservAt *)ras_getattr(od_list[i].attr_name, 0, &cnt);
	Debug("odmread:[SWservAt] attr='%s' value='%s'\n", swservp->attribute,
	    swservp->value);
	if (!cnt) continue;
	if (PRIMARY == i)
	    {
	    Pdevname = stracpy(swservp->value);
	    }
	else if (SECONDARY == i)
	    {	
	    Sdevname = stracpy(swservp->value);
	    }
	else if (TPRIMARY == i)
	    tPdevname = stracpy(swservp->value);
	else if (TSECONDARY == i)
	    tSdevname = stracpy(swservp->value);
	else if (AUTOCOPY == i)
	    Copyfilename = stracpy(swservp->value);
	else if (FORCECOPY == i)
	    Forcecopyflag = stracpy(swservp->value);
	od_list[i].swservp = swservp;
    }
    if ( (Pdevname == 0) && !quietflg)
    {
	cat_eprint(CAT_ODM_PRIM,
	            "Cannot read primary dump device from ODM object class SWservAt.\n");
	rv = -1;
    }
    if ((Sdevname == 0) && !quietflg)
    {
	cat_eprint(CAT_ODM_SEC,
	            "Cannot read secondary dump device from ODM object class SWservAt.\n");
	rv = -1;
    }
    if ((Copyfilename == 0) && !quietflg)
    {
	cat_eprint(CAT_GETATTR,
	            "Unable to get %s attribute from ODM object class SWservAt.\n","autocopydump");
	rv = -1;
    }

    if ((Forcecopyflag == 0) && !quietflg)
    {
	cat_eprint(CAT_GETATTR,
	            "Unable to get %s attribute from ODM object class SWservAt.\n","forcecopydump");
	rv = -1;
    }
    if ( rv != 0 )
	exit(rv);
}

static 
void odmwrite(start)
{
    int i,j;

    for (i = start; i < OD_LIST; i++) {
	if ((TPRIMARY == i) && od_list[i].swservp)
	    od_list[i].swservp->value = tPdevname;
	else if ((TSECONDARY == i) && od_list[i].swservp)
	    od_list[i].swservp->value = tSdevname;
	else if ((PRIMARY == i) && od_list[i].swservp)
	    od_list[i].swservp->value = Pdevname;
	else if ((SECONDARY == i) && od_list[i].swservp)
	    od_list[i].swservp->value = Sdevname;
	j = ras_putattr(od_list[i].swservp);
    }

    savebase();			/* sync the putattr()'s */

	/* Call the sync() routine to make sure the changes  */
	/* are written to disk.  This is necessary if a dump */
	/* happens before the syncd has had a chance to run. */ 
	sync();
}

/*
 * NAME:     rdev
 * FUNCTION: return the major/minor device number of the supplied filename
 * INPUTS:   devname   filename to get major/minor of
 * RETURNS:  major/minor device number of 'devname'
 */
static 
rdev(devname,msg)
char *devname;
int msg;
{
	struct stat statbuf;

	/* Check to see if the device is remote.  If so, just */
	/* return 0 for the major / minor number, because the */
	/* network device drivers don't use /dev files and don't */
	/* have major / minor numbers.  */
        if ( (strchr(devname, ':' )) != NULL )
		return(0);

	if(stat(devname,&statbuf)) {
		if (msg) perror(devname);
		return(-1);
	}
	return(statbuf.st_rdev);
}




/*
 * NAME:     usage_dev
 * FUNCTION: Output usage message for sysdumpdev and exit.
 * INPUTS:   None
 * RETURNS:  None (exits)
 */
static 
void usage_dev()
{

	cat_eprint(CAT_USAGEDEV,"\
Usage:\n\
%s [-P] [-p Device | -s Device]\n\
%s [-l | -q | -z | -r Host:Path | -p Device | -s Device | -L | -e]\n\
%s [-d | -D] Directory\n\
\n\
Change the primary and secondary dump device designations\n\
in a running system.\n\
\n\
-d Directory  Specify the directory where the dump is copied to at\n\
              boot time. If the copy fails the system continues to boot.\n\
-D Directory  Specify the directory where the dump is copied to at\n\
              boot time. If the copy fails then a menu is displayed\n\
              to allow user to copy the dump.\n\
-e            Estimate the size of the dump (in bytes) for the\n\
              current running system.\n\
-l            List the current dump device designations.\n\
-L            Display statistical information about the previous dump.\n\
-p Device     Change the primary dump device to the specified\n\
              device temporarily.\n\
-P            Make the dump device specified by -p or -s flags permanent.\n\
              Can only be used with -p or -s flags.\n\
-q            Suppress any error messages that are written\n\
              to stdout.\n\
-r Host:Path  Free the space used by the remote dump file.\n\
-s Device     Change the secondary dump device to the specified\n\
              device temporarily.\n\
-z            Write out to stdout the string containing the size\n\
              of the dump in bytes and the name of the dump device\n\
              if a new dump is present.\n\
\n\
If no flag is specified, the permanent dump device designations are used.\n",
		Progname,Progname,Progname);
	exit(1);
}

#define MAX_DEVNAME  128
#define MAX_FNAME     52
#define COPYFILENAME  "/var/adm/ras/copyfilename"
struct dmp_status {
	struct err_rec0 err_rec;
	struct {
		char	name[MAX_DEVNAME];
		short	maj;
		short	min;
		off_t	size;
		char	time[32];
		int	type;
		int 	status;
		char    fname[MAX_FNAME];
	} detail;
};

/* dmp_cklog:
**  - does a DMP_IOCSTAT ioctl to get what nvram thinks is the most recent
**  dump.  If the DMPFL_NEEDLOG is set, then we dump the information from
**  nvram to the errlog.  The driver knows enough to set this flag off
**  after this ioctl.  It is our responsibility to errlog() it.
*/

dmp_cklog(fd)
	int fd;
{
	struct dumpinfo d;
	char filename[MAXPATH];

	if ((ioctl(fd,DMP_IOCSTAT,&d) >= 0) && (d.dm_flags & DMPFL_NEEDLOG)) {
		struct dmp_status e;
		char *t,*p;

		e.err_rec.error_id = ERRID_DUMP_STATS;
		strncpy(e.err_rec.resource_name,"SYSDUMP",ERR_NAMESIZE);
		strncpy(e.detail.name,d.dm_devicename,MAX_DEVNAME);
		e.detail.maj = major(d.dm_mmdev);
		e.detail.min = minor(d.dm_mmdev);
		e.detail.size = d.dm_size;
		t = ctime(&d.dm_timestamp);
		p = strrchr(t,'\n');			/* kill \n */
		if (p != (char *)NULL) *p = '\0';
		strncpy(e.detail.time,t,32);
		e.detail.type = d.dm_type;
		if (e.detail.type == DMPD_PRIM_HALT) e.detail.type = DMPD_PRIM;
		if (e.detail.type == DMPD_SEC_HALT) e.detail.type = DMPD_SEC;
		e.detail.status = d.dm_status;
		/*****************************************************
		if the dump is copied out to the filesystem,
		/var/adm/ras/copyfilename 
		should exist and contains the full path name of file.
		This file name should also be in the error log entry.
		*****************************************************/
		if ( access(COPYFILENAME, E_ACC) == 0  )
		{
		  if  ((fd =  open(COPYFILENAME,O_RDONLY))  > 0 )
		  {
		     if ((lseek(fd,0,SEEK_SET) >= 0 ) &&
			 (read(fd,&filename,MAXPATH) > 0 ) )
		     {
			/* put the file name in the detail record */
			strncpy(e.detail.fname,filename,MAX_FNAME);
			/* Now close COPYFILENAME   */
			close(fd);
		     }
		     else
		        close(fd);
		  }
		}

		/* log the entry */
		errlog(&e, sizeof(struct dmp_status));
	}
}

#define MAX_TRIES 3

 /* NAME : getfh
    FUNCTION : Get the file handle of the file on the server 
               from the server's mountd.
    RETURN : zero - sucessful
               non zero - error
  */

static int
getfh(host, object)
char    *host;
char    *object;
{
	struct sockaddr_in	sin;
	struct hostent			*hp;
	int				s,rv,num_tries ;
	struct timeval			timeout;
	CLIENT				*client;
	enum clnt_stat			rpc_stat;

	rv = 0;
	/*
	 * Get server's address
	 */
        
	if ((hp = gethostbyname(host)) == NULL) {
		/*
		 * Failure may be due to yellow pages, try again
		 */
		if ((hp = gethostbyname(host)) == NULL) {
	            cat_eprint(CAT_HOSTUNREACH,
	            "Host %s does not respond or is not reachable.\n",
                    host);
		    rv =  EHOSTUNREACH;
		}
	}
        else {

	  /*
	   * get fhandle of remote path from server's mountd
	  */
	  bzero(&sin, sizeof(sin));
	  bcopy(hp->h_addr, (char *) &sin.sin_addr, hp->h_length);
	  sin.sin_family = AF_INET;
	  timeout.tv_usec = 0;
	  timeout.tv_sec = 20;
	  s = RPC_ANYSOCK;

	  if ((client = clntudp_create(&sin, MOUNTPROG, MOUNTVERS,
	    timeout, &s)) == NULL) {
		rv = -1 ;
       	  }
          else {
	    client->cl_auth = authunix_create_default();
	    timeout.tv_usec = 0;
	    timeout.tv_sec = 20;
            num_tries = 1;
            while ( (num_tries == 1 ) || ((num_tries <= MAX_TRIES) && (rv == ETIMEDOUT )))
            {
	      rpc_stat = clnt_call(client, MOUNTPROC_MNT, xdr_path, &object,
	                           xdr_fhstatus, &fhs, timeout);
	      if (rpc_stat != RPC_SUCCESS) {
	      	  switch (rpc_stat) {
		  case RPC_TIMEDOUT:
		  	  rv = ETIMEDOUT;
		  case RPC_AUTHERROR:
			  rv = EACCES ;
		  case RPC_PMAPFAILURE:
		  case RPC_PROGNOTREGISTERED:
		  default:
			  rv = ERPC;
		  }
	      }
              else
                if (fhs.fhs_status != 0)
                   rv = fhs.fhs_status;
                else rv = 0;
              num_tries++;
            }
	    clnt_destroy(client);
       	  }
	}

        if (rv)
	      cat_eprint(CAT_UNABLE_TO_ACCESS,
	            "Unable to access file %s on host %s.\n",object,host);
        return(rv);
}

static
xdr_fhstatus(xdrs, fhsp)
	XDR *xdrs;
	struct fhstatus *fhsp;
{
	if (!xdr_int(xdrs, &fhsp->fhs_status))
		return (FALSE);
	if (fhsp->fhs_status == 0) {
		if (!xdr_fhandle(xdrs, &fhsp->fhs_fh))
			return (FALSE);
	}
	return (TRUE);
}

static bool_t
xdr_path(xdrs, pathp)
	XDR *xdrs;
	char **pathp;
{
	if (xdr_string(xdrs, pathp, 1024)) {
		return(TRUE);
	}
	return(FALSE);
}

static
xdr_fhandle(xdrs, fhp)
	XDR *xdrs;
	fhandle_t *fhp;
{
	if (xdr_opaque(xdrs, (char *) fhp, NFS_FHSIZE)) {
		return (TRUE);
	}
	return (FALSE);
}


static
int reclaim_remote_file_space()
{

  char *host_name;
  char *dir_name;
  char *path_name;
  char *next;
  char cmdline[256];
  char *full_path;
  char *file_name;
  char *fn;
  char local_dir[80];
  int  rv, pid, fd;

  /* Pseudo:
  ** input:         <hostname>:<pathname>
  ** 
  ** Parse the input string to get hostname and pathname.
  ** Parse the pathname for dir_name and file_name.
  ** Mount the dir_name onto a known temporary dir,/usr/adm/ras/ddd_reclaim.
  ** If it is an unsuccessful mount, strip off the last part of the dir_name
  ** Mount the new dir_name until there is a sucessful mount or the 
  ** dir_name is an empty string.
  ** Return an error message if the dirname is an empty string.
  ** Otherwise, open the file with O_TRUNC to set file pointer back to 
  ** beginning of the file.
  ** Umount the file system.
  */
  rv = 0;

  /* Remote dump is not supported on multiprocessor machines. */
  error_if_multiprocessor();

  if ( ( path_name = strchr(remote_fn, ':')) == NULL )
    cat_fatal(CAT_INV_INPUT,
	"The input string should be in the form of <hostname>:<pathname>\n");
  else
	{      
    *path_name++ = '\0';
    host_name = remote_fn;

    /* ping the host name to see if we if the host name accessible */
    sprintf(cmdline,"ping -c 1 -s 1 %s 2>/dev/null 1>/dev/null",host_name);
    if ( system(cmdline) )
    	{
		cat_fatal(CAT_HOSTUNREACH,
			"%s does not respond or is not reachable.\n",host_name);
    	}

    /* Parse the pathname for dir_name and file_name */

    full_path = stracpy(path_name);
    if ((fn  = strrchr(path_name,'/')) == NULL)
	    cat_fatal(CAT_NO_PATH,
		"Enter the full pathname to the file, %s, on the remote host %s.\n", path_name, host_name);
    file_name = stracpy(fn);
    *fn = '\0';
    
    dir_name = path_name;
 
    /* Create a temporary directory to mount the file system. */
    pid = getpid();
    sprintf(local_dir,"%s%d","/usr/adm/ras/r",pid);
    if ( (rv = mkdir(local_dir,0755)) != 0)
      cat_fatal(CAT_CANT_CREATE_TDIR,
	"Unable to create the temporary directory %s.\n",local_dir);
      
    /* mount the directory */
    
    strcpy(cmdline,"mount ");
    strcat(cmdline,host_name);
    strcat(cmdline,":");
    strcat(cmdline,dir_name);
    strcat(cmdline,"  ");
    strcat(cmdline,local_dir);
         
    if ( rv = (system(cmdline)))
    { /* unable to mount */
      cat_eprint(CAT_CANT_MOUNT,
	"Unable to mount the directory %s from host %s\n",dir_name,host_name);
      rv = -1;
    }
    else
    {
      /* Remove the file */
      strcpy(cmdline,local_dir);
      strcat(cmdline,file_name);
      if ((fd = open(cmdline, O_WRONLY|O_TRUNC)) < 0)
      {
		/* Using perror here */
        cat_eprint(CAT_UNABLE_TO_OPEN,
		"Unable to open file %s on host %s.\n",full_path, host_name);
        rv = -1;
      }
      else
        close(fd);      
     
      /* Unmount the file system before exit */
      strcpy(cmdline,"umount ");
      strcat(cmdline,local_dir);
      if (rv)
         {
	 if (system(cmdline))	
		cat_eprint(CAT_CANT_UMOUNT,
			"Unable to unmount %s.\n", local_dir);
          }
      else
	  {
      	  if ( rv = (system(cmdline)))
        	{
          	cat_eprint(CAT_CANT_UMOUNT,
			  "Unable to unmount %s.\n", local_dir);
	 	rv = -1;
        	}
          }
    }    
    /* Delete the temporary file */
    rmdir(local_dir);
    return(rv);
  }
}

/*
   NAME : check_remote_hostname
   FUNCTION : Get the IP address of the remote hostname.
              Compare this remote host address with each of the
              addresses of the local host. If they are the same
              as local host, return a non-zero return code,
              otherwise return 0.
*/

static int
check_remote_hostname(host_name)
char *host_name;
{
	int s;
	char buf[BUFSIZ], *cp, *cplim;
	struct ifconf ifc;
	struct ifreq ifreq, *ifr;
        struct hostent *hp;	/* Pointer to host info */
        struct sockaddr local;
	struct sockaddr_in *local_host = (struct sockaddr_in *) &local;
	struct sockaddr_in *sin;
        int rv = 0;

	bzero( (char *)&local, sizeof(struct sockaddr) );
        hp = gethostbyname(host_name);
        bcopy(hp->h_addr,(caddr_t)&local_host->sin_addr,hp->h_length);
       

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)	
		return(1);
	
	ifc.ifc_len = sizeof (buf);
	ifc.ifc_buf = buf;
	if (ioctl(s, SIOCGIFCONF, (char *)&ifc) < 0)
		return(2);
	
	ifr = ifc.ifc_req;

#define size(p)	MAX((p).sa_len, sizeof(p))

	cplim = buf + ifc.ifc_len; /*skip over if's with big ifr_addr's */
	for (cp = buf; cp < cplim ;
		       cp += sizeof(ifr->ifr_name) + size(ifr->ifr_addr))
        {
		ifr = (struct ifreq *)cp;
                
		if ( ifr->ifr_addr.sa_family == AF_INET)
                {
                   sin = (struct sockaddr_in *)&ifr->ifr_addr;
                   if (local_host->sin_addr.s_addr == sin->sin_addr.s_addr )
                   {
                     cat_eprint(CAT_CANT_SAME,
                           "Remote host can not be the same as local host.\n");
                     rv = 3;
                     break;
                   }
		}
	}
		
        return(rv);
}


/* NAME : calculate_size
   FUNCTION : Issue an ioctl DMP_SIZE to the /dev/sysdump to get
              the size of the memory allocated for kernel segment.
              The estimation is size + 25 % of the size.
*/

static int
calculate_size(dfd)
int dfd;
{
        struct devinfo devinfo;
        int dump_size;
       
        if (0 > ioctl(dfd, DMP_SIZE, &devinfo)) {
        	perror("sysdumpdev : ioctl (DMP_SIZE)");
		exit (1);
	}
        dump_size = (devinfo.un.dump.mdt_size * 1.25 ) ;
        cat_print(CAT_DUMP_SIZE,
                        "Estimated dump size in bytes: %d\n",dump_size);
        exit(0);
}

/* NAME : get_location_code
   FUNCTION : 
		The logical name is used to access the volume group id from ODM.
		The volume group id is used to retrieve the physical volume id.
		The physical volume id is used to access name of the physical volume.
		The physical volume name is used to access the location code.
*/

static int
get_location_code(location_code,lv_name)
char *location_code;
char *lv_name;
{
	int i;
	char *bufptr;
	struct CuAt *pcuat;
	struct CuDv *pcudv;
	struct listinfo stat_info;
	char crit[CRITSIZE];
	char buf[256];
	FILE *fp;
	int boot_type = 0;

        /* get the LVID for the logical volume */
        pcuat = (struct CuAt *)getattr(&lv_name[5],"lvserial_id" , 0, &i);

        if (pcuat == NULL) {
		cat_fatal(CAT_CANT_GETATTR,
			"Unable to get attribute %s for %s in ODM.",
			"lvserial_id",&lv_name[5]);
	}
	else
	{
                bufptr = strtok(pcuat->value,".");
                sprintf(crit,"attribute = '%s' and value = '%s'","vgserial_id",bufptr);
                pcuat = (struct CuAt *)odm_get_list(CuAt_CLASS,crit,&stat_info,1,1);
                if  (pcuat == NULL)
		{
			cat_fatal(CAT_CANT_GET_LIST,
			"No attribute %s with the value of %s in ODM",
			"vgserial_id",bufptr);
		}
                else if ( strcmp(pcuat->name,"rootvg") != 0) 
                {
			/* This is for a dataless machine.  It creates a local
			   volume group that's not called rootvg, and sets the
			   dump device to a logical volume on that device.  We
			   call bootinfo -t.  If it returns "5", then we know it's
			   a dataless machine and we don't care if it's in the 
			   root volume group. */		 
			sprintf(buf,"bootinfo -t");
			if ((fp = popen(buf,"r")) == NULL)
				{
				perror("sysdumpdev: popen");
				exit (1);	
				}
			fscanf(fp,"%d",&boot_type);
			pclose(fp);
			if ((boot_type != 5) && (permflg))	
                        	cat_fatal(CAT_NOT_ROOTVG,
				  "%s is not in volume group rootvg.\n",lv_name);
                }
                sprintf(crit,"attribute = '%s' and name = '%s'","pv",pcuat->name);
                pcuat = (struct CuAt *)odm_get_list(CuAt_CLASS,crit,&stat_info,1,1);
                if ( (pcuat == NULL) )
                {
                        cat_fatal(CAT_CANT_GET_PVID,
			 "Unable to get physical volume id for %s from ODM.\n",pcuat->name);
                }
                sprintf(crit,"attribute = '%s' and value = '%s'","pvid",pcuat->value);
                pcuat = (struct CuAt *)odm_get_list(CuAt_CLASS,crit,&stat_info,1,1);
                if ( (pcuat == NULL) )
                {
                        cat_fatal(CAT_CANT_GET_PVNAME,
                        "Unable to get the physical volume name for %s from ODM.\n",pcuat->value);
                }
                sprintf(crit,"name = '%s'",pcuat->name);
                pcudv = (struct CuDv *)odm_get_list(CuDv_CLASS,crit,&stat_info,1,1);
                if ( pcudv == NULL )
                {
                        cat_fatal(CAT_CANT_GET_LOC,
                               "Unable to get the location code for %s from ODM.\n",
				pcuat->name);
                }
		else
		{
			strcpy(location_code,pcudv->location);
		}

        }
	return(0);
}

/*
        NAME :  set_core_file
        FUNCTION: Verify if the input file is a directory in the
                  local JFS.
                  Resolve the symbolic link to get the real filename.
                  Get the filesystem of the file
                  Get the mount point of the filesystem.
                  Check if the filesystem is on rootvg volume group
                  Save the filename, real filename, file system and
                  mount point, in ODM database.
*/


#define NEXT_VMT(vmtp) \
        ((vmtp) = (struct vmount *)((char *)(vmtp) + (vmtp)->vmt_length))

static set_core_file(dirname,Coreval)
char *dirname;
int Coreval;
{
	struct SWservAt *swservp;
	
        int cnt;
        int                     size;

        struct vmount   *vmtp;
        int             i;
        struct stat     statbuf;

        int             nmounts;
        struct vmount   *vmountp;
	char buff[BUFSIZ];
	char mountpt[MAXPATH];
	char lvname[MAXPATH];
	char *bufptr;
	struct CuAt *pcuat;
	char crit[CRITSIZE];
	struct listinfo stat_info;


	/************************************/
	/* /dev/null is special case.       */
	/************************************/

	if ((strcmp(dirname,DevNull) == 0 ))
  	{
	  strcpy(lvname,dirname);
	  strcpy(mountpt,dirname);
	}
	else
	{
	  /************************************/
	  /* get the status of the input file */
	  /************************************/

          if (stat(dirname, &statbuf) < 0) {
	        perror(dirname);
                return(-1);
          }

	  /************************************/
	  /* Check if the input is a directory */
	  /************************************/

	   if (statbuf.st_type != VDIR)
	  	{
		  cat_fatal(CAT_NOT_DIR,"%s is not a directory.\n",dirname);
		}	

	  /************************************/
	  /* Check if the input is in a local JFS */
	  /************************************/

	   if (statbuf.st_vfstype != MNT_JFS)
		{
		  cat_fatal(CAT_NOT_JFS,"%s is not in a journaled filesystem.\n",dirname);
		}

	  /************************************/
	  /* Resolve the symbolic link to get the real filename */
	  /************************************/

	  chdir(dirname);
	  getcwd(dirname,BUFSIZ);  	/* after the execution of this line
 					 dirname should
				   	 contain the absolute pathname of the  
				   	 that input file */

	  /************************************/
	  /* Get the file status structure of the real file */
	  /************************************/
	  /*
	   if (stat(dirname,&statbuf) < 0)
	    {
		perror(dirname);
	    }
	  */
          size = BUFSIZ;
	  /* get the structure  mount for all the filesystems */
          /* try the operation until ok or a fatal error */
          while (1) {
                if ((vmountp = (struct vmount *)malloc((size_t)size)) == NULL)
		{
                   cat_fatal(CAT_MALLOC,"Unable to allocate memory for mntctl call.\n");
                }

                /*
                 * perform the QUERY mntctl - if it returns > 0, that is the
                 * number of vmount structures in the buffer.  If it returns
                 * -1, an error occured.  If it returned 0, then look in
                 * first word of buffer for needed size.
                 */
                if ((nmounts = mntctl(MCTL_QUERY, size, (caddr_t)vmountp)) > 0)
                {
                                /* OK, got it, now return */
                        break;

                } else if (nmounts == 0) {
                        /* the buffer wasn't big enough .... */
                        /* .... get required buffer size */
                        size = *(int *)vmountp;
                        free((void *)vmountp);

                } else {
                        /* some other kind of error occurred */
                        free((void *)vmountp);
			cat_fatal(CAT_MOUNT_ERR,"Error in mntctl call. \n");
                }
          }


	  /************************************/
          /* Find out what filesystem it's in by comparing vfs number from
             statbuf and vmount struct
          */
	  /************************************/
          for (vmtp = vmountp, i = 0; i < nmounts; NEXT_VMT(vmtp), ++i) {
                if (statbuf.st_vfs == vmtp->vmt_vfsnumber)
                        break;
          }
          sprintf(lvname,"%s",vmt2dataptr(vmtp, VMT_OBJECT));
          sprintf(mountpt,"%s",vmt2dataptr(vmtp, VMT_STUB));

	  /*************************************/
	  /*   Make sure that the directory entered
	       is in one of the following filesystems:
	       /, /var, /usr, /tmp, /home  */
	  /*************************************/
	  if (strcmp(mountpt,"/") && strcmp(mountpt, "/usr") && 
	      strcmp(mountpt,"/var") && strcmp(mountpt,"/tmp") &&
	      strcmp(mountpt,"/home"))
	        {	
		cat_fatal(CAT_BAD_MOUNTPT, "\
The specified directory %s must be in one of the \n\
following filesystems: /, /usr, /home, /tmp, /var\n", dirname);
		}
	  /************************************/
	  /* Check to make sure the logical volume
	     is in the rootvg volume group
             get the LVID for the logical volume.
	     LVID is made up from the volume group serial id.  */
	  /************************************/

          pcuat = (struct CuAt *)getattr(&lvname[5],"lvserial_id" , 0, &i);

          if (pcuat != NULL) {
                bufptr = strtok(pcuat->value,".");

                sprintf(crit,"attribute = '%s' and value = '%s'","vgserial_id",
bufptr);
	  	/************************************/
	  	/* get the name of the volume group id 
		   from the volume group serial id. 
                   Compare the name with "rootvg"  */ 
	  	/************************************/

                pcuat = (struct CuAt *)odm_get_list(CuAt_CLASS,crit,&stat_info,1
,1);
                if ( (pcuat != NULL) && (strcmp(pcuat->name,"rootvg") != 0) )
                {
		    
        		cat_fatal(CAT_NOT_ROOTVG,
			 "%s is not in volume group rootvg.\n",dirname);
                }

          }

	}  /* end of else clause */

	/************************************/
        /* get current value of autocopydump */
	/************************************/

        swservp = (struct SWservAt *)ras_getattr("autocopydump", 0, &cnt);
        if (swservp == NULL)
        {
            cat_fatal(CAT_GETATTR,
		 "Unable to get %s attribute from ODM object class SWservAt.\n","autocopydump");
        }

	/************************************/
        /* save new value of autocopydump */
	/************************************/

 	swservp->value = dirname;
        if (ras_putattr(swservp) < 0)
            {
            cat_fatal(CAT_PUTATTR,
		 "Unable to put %s attribute into ODM object class SWservAt.\n","autocopydump");
            }

	/************************************/
        /* get current value of copyfilesystem */
	/************************************/

        swservp = (struct SWservAt *)ras_getattr("copyfilesystem", 0, &cnt);
        if (swservp == NULL)
            {
            cat_fatal(CAT_GETATTR,
		 "Unable to get %s attribute from ODM object class SWservAt.\n","copyfilesystem");
            }
	
	sprintf(buff,"%s,%s",lvname,mountpt);

	/************************************/
        /* save new value of copyfilesystem */
	/************************************/

	swservp->value = buff;
        if (ras_putattr(swservp) < 0)
            {
            cat_fatal(CAT_PUTATTR,
		 "Unable to put %s attribute into ODM object class SWservAt.\n","copyfilesystem");
            }

	/************************************/
        /* set forcecopydump attribute according to value of Coreval */
	/************************************/

        swservp = (struct SWservAt *)ras_getattr("forcecopydump", 0, &cnt);
        if (swservp == NULL)
            {
            cat_fatal(CAT_GETATTR,
		 "Unable to get %s attribute from ODM object class SWservAt.\n","forcecopydump");
            }

	/************************************/
        /* save new value of forcecopydump */
	/************************************/
	if (Coreval)
	    swservp->value = stracpy("TRUE");	
	else
	    swservp->value = stracpy("FALSE");

        if (ras_putattr(swservp) < 0)
            {
            cat_fatal(CAT_PUTATTR,
		 "Unable to put %s attribute into ODM object class SWservAt.\n","forcecopydump");
            }

	savebase();	/* Call savebase to make sure these attributes persist across boots. */ 

	/* Call the sync() routine to make sure the changes  */
	/* are written to disk.  This is necessary if a dump */
	/* happens before the syncd has had a chance to run. */ 
	sync();

	return(0);
}

/*
 *  NAME:  error_if_multiprocessor()
 *
 *  FUNCTION:   This function checks to see if we are running on
 *		a machine with multiple processors.  If so, we give
 *		an error message that remote dump is not supported.
 *		This function is only called when someone tries to  
 *		initialize a remote primary or secondary dump device,
 *		or clear a remote dump file with the -r flag.	
 *
 *  INPUTS:	none
 *  RETURNS:    none
 * 		Exits if the machine we are running on has multiple 
 *		processors.
 */
void error_if_multiprocessor()
{
	if (_system_configuration.ncpus > 1)
		cat_fatal(CAT_REMOTE_MP, 
			"Remote dump is not supported in a multiprocessor environment.\n");
}

