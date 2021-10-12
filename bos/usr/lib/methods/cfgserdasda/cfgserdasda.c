static char sccsid[] = "@(#)18        1.14  src/bos/usr/lib/methods/cfgserdasda/cfgserdasda.c, cfgmethods, bos411, 9439B411a 9/27/94 20:27:31";
/*
 *   COMPONENT_NAME: CFGMETHODS
 *
 *   FUNCTIONS: build_dds
 *		define_children
 *		download_microcode
 *		generate_minor
 *		make_special_files
 *		query_vpd
 *		sd_child_process
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* header files needed for compilation */

#include <stdio.h>
#include <fcntl.h>
#include <cf.h>
#include <sys/sysmacros.h>
#include <sys/sysconfig.h>
#include <sys/sd.h>
#include <sys/device.h>
#include <sys/cfgodm.h>
#include <sys/errno.h>
#include <sys/cfgdb.h>
#include <sys/stat.h>
#include "cfgdebug.h"
#include "cfgserdasd.h"
#include <usersec.h>

		
int build_dds( char *, char **, long * );	

	
/*
 *  Path which config uses to to kick off
 *  daemon
 */

char daemon_child[] = "/etc/methods/sdd";   


/*
 * NAME   : generate_minor
 *
 * FUNCTION: This function generates minor device number for the 
 *	     Serial DASD adapter
 * 
 * EXECUTION ENVIRONMENT:
 *	This function operates as a device dependent subroutine 
 *	called by the generic configure method for all devices.
 *	It makes use of generic genminor() function.
 *
 * DATA STRUCTURES:
 *
 * INPUTS : device logical_name,major_no,address to store minor number.
 *
 * RETURNS: 0, or E_MINORNO
 *
 */

long generate_minor(char	*lname,	    /* Logical name of device       */
		    long	major_no,   /* Major number of device       */
		    long	*minor_dest)/* Address to store minor number*/
					    /* generated                    */
{
	long	*minorno;                   /* normal minor number          */
	long    daemon_minorno;             /* daemon's special entry point */



	DEBUG_0("cfgserdasda: generate_minor\n")

	minorno = genminor(lname,major_no,-1,1,1,1);

	/*
	 * The daemon minor number is the same minorno except
	 * that bit 13 is set to 1
	 */


	if(minorno == (long *)NULL)
		return E_MINORNO;
	DEBUG_2("genminor(..%ld..) returned %ld\n", major_no, *minorno )
	*minor_dest = *minorno;
	daemon_minorno = (*minorno | SD_DAEMON_MASK);
	minorno = genminor(lname,major_no,daemon_minorno,1,1,1);
	if(minorno == (long *)NULL)
		return E_MINORNO;
	DEBUG_2("daemon genminor(..%ld..) returned %ld\n", major_no, *minorno )
	return 0;
}

/*
 *
 * NAME   : make_special_files
 * 
 * FUNCTION: creates serial DASD adapter special file.
 * 
 * EXECUTION ENVIRONMENT:
 *	This function operates as a device dependent subroutine
 *	called by the generic configure method for all devices.
 *
 * DATA STRUCTURES:
 *
 * INPUTS : logical_name,devno
 *
 * RETURNS: Returns 0 if success else -1
 *
 */

int make_special_files(char  *lname,  /* Logical name of controller         */
		       dev_t	devno)/* Major & minor number for controller*/
{

	int ret_code = 0;           /* return code 			      */
        long minorno,majorno;       /* minor and major numbers,resp 	      */
	dev_t daemon_devno;         /* devno that daemon uses to open adapter */
	char *filename = DAEMON_NAME;/* used to make daemon's special file    */
	

	/* 
	 * The permissions are  "crw-rw-rw-"  
	 */

	DEBUG_0("cfgserdasda: make_special_files\n")

	ret_code = mk_sp_file(devno,lname,
		       (S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH));
	if (ret_code)
		return(ret_code);
	/*
	 * Now make daemon's special file
	 */
	majorno = major(devno);
	minorno = minor(devno);
	minorno = minorno | SD_DAEMON_MASK;
	DEBUG_2("cfgserdasda: daemon's majorno = %d, minorno= %d\n",majorno,minorno)
        daemon_devno = makedev(majorno,minorno);
	strcat(filename,lname);
	DEBUG_1("cfgserdasda: daemon's filename is: %s\n",filename)
	ret_code =  mk_sp_file(daemon_devno,filename,
		       (S_IFCHR|S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH));
	return (ret_code);
}

/*
 *
 * NAME: build_dds
 * 
 * FUNCTION: Creates a dds structure passed to the driver via the config
 *		entry point
 * 
 * EXECUTION ENVIRONMENT:
 *	This function operates as a device dependent subroutine
 *	called by the generic configure method for all devices.
 *
 * DATA STRUCTURES:
 *
 * INPUTS: lname, dds_addr_pointer (address to store pointer to dds),
 *		dds_len (address to store length of dds )
 *
 * RETURNS: Returns 0 if success else Error number
 *
 */

int build_dds(char  *lname,	  /* Logical name for controller */
	      char **dds_addr_ptr,/*Address to store address of dds generated*/
	      long *dds_len)      /*Address to store length of dds generated */
{
	int			rc;		/* Return code */
	struct Class		*cusatt,
	*preatt;
	struct CuDv		cusobj,		/* CuDv object for adapter */
				parobj;	        /* CuDv objcet for parent */
						/* (i.e. bus) */
	struct PdAt             preobj;         /* PdAt for dma_bus_mem    */
	struct sd_adap_dds	*dds;		/* address of dds structure */
	char			sstr[SD_MAX_STRING_LEN];  /* Search string */



	DEBUG_0("cfgserdasda: build_dds\n")

	if( NULL== ( dds = ( struct sd_adap_dds *)malloc(
		sizeof(struct sd_adap_dds))))
		return E_MALLOC;

	ODMOPEN( cusatt, CuAt_CLASS )
	ODMOPEN( preatt, PdAt_CLASS )
	sprintf( sstr, "name = '%s'", lname );
	sprintf( sstr, "name = '%s'", lname );          /* compiler bug ? */
	GETOBJ( CuDv_CLASS, sstr, &cusobj, ODM_FIRST, E_NOCuDv )

	sprintf( sstr, "name = '%s'", cusobj.parent );

	GETOBJ( CuDv_CLASS, sstr, &parobj, ODM_FIRST, E_NOCuDvPARENT )

	dds->dev_type = 0; /* I.E. Adapter */

	GETATT( &dds->bus_id, 'l', parobj, "bus_id", NULL )

#ifndef SERDASDA_BUSID
#define SERDASDA_BUSID 0x000C0020
#endif

	/*
	 * Get adapter attributes
	 */
	dds->bus_id |= SERDASDA_BUSID;

	GETATT( &dds->bus_type, 'h', parobj, "bus_type", NULL )

	dds->slot = atoi( cusobj.connwhere ) - 1;

	GETATT( &dds->base_addr, 'i', cusobj, "bus_io_addr", NULL )

	strncpy( dds->resource_name, lname, 16 );

	GETATT( &dds->dma_lvl, 'i', cusobj, "dma_lvl", NULL )

	GETATT( &dds->intr_lvl, 'i', cusobj, "bus_intr_lvl", NULL )
	
	GETATT( &dds->intr_priority, 'i', cusobj, "intr_priority", NULL )

	GETATT( &dds->tcw_start_addr, 'l', cusobj, "dma_bus_mem", NULL )

	sprintf( sstr, "uniquetype = '%s' AND  attribute = '%s'", cusobj.PdDvLn_Lvalue ,"dma_bus_mem");

	GETOBJ( PdAt_CLASS, sstr, &preobj, ODM_FIRST, E_NOATTR)
	dds->tcw_length = (int) strtol(preobj.width,(char **)NULL, 0);
	ODMCLOSE( CuAt_CLASS )
	ODMCLOSE( PdAt_CLASS )

	*dds_addr_ptr = (char *)dds;
	*dds_len = sizeof( struct sd_adap_dds );

	return 0;
}

/*
 * NAME	: query_vpd
 *
 * FUNCTION :
 *	this routine gets vpd data from the adapter using
 *	a sysconfig system call and stores it in the database.
 *
 * EXECUTION ENVIRONMENT:
 *	This function is called by generic config method.
 *
 * INPUT  : Adapter's CuDv object, driver kmid, devno, and dest addr for VPD
 *
 * RETURNS: Returns 0 if success else -1
 *
 * RECOVERY OPERATION:
 *
 */

int query_vpd(struct CuDv *cusobj,    /* Pointer to customized object rec   */
	      mid_t	kmid,	      /* Kernel module I.D. for Dev Driver  */
	      dev_t	devno,	      /* Concatenated Major & Minor No.s    */
	      char	*vpd)         /* Area to place VPD	       	    */
{
	int	rc;
	struct	cfg_dd	qvpd;
	int	i;
	char	tmp_vpd[VPDSIZE];


	DEBUG_2("query_vpd: devno = 0x%x, kmid = 0x%x\n",devno,kmid)
	qvpd.kmid = kmid;
	qvpd.devno = devno;
	qvpd.cmd = CFG_QVPD;
	qvpd.ddsptr =tmp_vpd;
	qvpd.ddslen = VPDSIZE;

	rc = sysconfig(SYS_CFGDD,&qvpd,sizeof(struct cfg_dd));

	/*
	 * Sysconfig returns a nonpositive integer
	 */
	if(rc < 0){
#ifdef CFGDEBUG
	    switch(errno){
		case EINVAL:
		    DEBUG_1("cfgserdasda_qvpd: invalid kmid = %d\n",kmid)
			    break;
		case EACCES:
		    DEBUG_0("cfgserdasda_qvpd: not privileged\n")
			    break;
		case EFAULT:
		    DEBUG_0("cfgserdasda_qvpd: i/o error\n")
			    break;
		case ENODEV:
		    DEBUG_1("cfgserdasda_qvpd: invalid devno = 0x%x\n",devno)
			    break;
		default:
		    DEBUG_1("cfgserdasda_qvpd: error = 0x%x\n",errno)
	    }
#endif
	    return E_SYSCONFIG;
	}

	put_vpd( vpd, tmp_vpd, VPDSIZE );

	return(0);	
}

/*
 * NAME : download_microcode
 * 
 * FUNCTION :
 *	This function determines the proper level of microcode to
 *	download to the device, downloads it, and updates the CuAt
 *	object class to show the name of the file that was used.
 *
 * EXECUTION ENVIRONMENT:
 *	This function operates as a device dependent subroutine
 *	called by the generic configure method for all devices.
 *
 * INPUT  : logical_name
 *
 * RETURNS: Returns 0 if success else errno
 *
 * RECOVERY OPERATION:
 *
 * NOTE :
 *	During download, the adapter is opened in diagnostics mode
 *		(..thus requiring exclusive access)
 *	Microcode file size can not be greater than 64K.
 */

int download_microcode(char *lname)
{
	int rc = 0;
	char sstr[SD_MAX_STRING_LEN];
	int adap = -1;



	rc = download_microcode_adap(lname,0,adap);

	return (rc);
}
/*
 * NAME   : define_children
 *
 * FUNCTION : this routine detects and defines controllers attached to the
 *	 serial dasd adapter.
 *
 * EXECUTION ENVIRONMENT:
 *	This function operates as a device dependent subroutine
 *	called by the generic configure method for all devices.
 *
 * INPUTS : logical_name,ipl_phase
 *
 * RETURNS: Returns 0 if success else -1
 *
 */
#ifndef INQSIZE
#define INQSIZE 255
#endif

int define_children(char  *lname,	     /* Logical name of adapter   */
		    int	phase)		     /* Phase that is running        */
{
	char filename[20];               /* special file name             */
	int fd;				 /* file descriptor               */
	int rc;				 /* return code                   */
	int sid;			 /* target it of controller       */
	struct sd_iocmd inq;             /* pass thru scsi command        */
	char inqdata[INQSIZE];           /* inquiry data from controller  */
	char sstr[100];                  /* search string                 */
	char *out_p;                     /* out put from odm_run_method   */
	struct CuDv cusobj;              /* customized device             */
	struct PdDv PdDv;                /* predefined device             */
	int    daemon_pid = 0;           /* pid of config daemon          */





	DEBUG_0("cfgserdasda: define_children\n")

	/*
	 * Exec of daemon 
	 */
	
        if ( (phase == RUNTIME_CFG) || (phase == PHASE2)) {
		
		if ((daemon_pid = sd_child_process(lname,phase)) <= 0) {
			DEBUG_1("execing daemon failed daemon_pid = %d\n",daemon_pid )

			if (daemon_pid == -2) {
				/*
				 * We must return here (i.e. we are the forked
				 * off child process that failed to exec),
				 * to prevent the define children 
				 * from running twice (once from cfgserdasda 
				 * and once from the forked 
				 * off child). If we didn't return here
				 * then we could potentially corrupt ODM,
				 * because these two copies of define_children
				 * can step on each other.
				 */
				exit(0);
			}
		}
#ifdef CFGDEBUG
		else {
			DEBUG_1("cfgserdasda: daemon_pid = %d\n",daemon_pid)

			}
#endif	
	}
	/*
	 * Assume all controllers and DASD were defined in
	 * phase one for the normal ipl situation
	 */

	
	sprintf( sstr, "type = serdasdc" );

	GETOBJ( PdDv_CLASS, sstr, &PdDv, ODM_FIRST, E_NOPdDv )

	DEBUG_0( "GETOBJ returned ok\n" )

	sprintf( filename, "/dev/%s", lname );

	DEBUG_0( "CALLING open() \n" )

		
	if(( fd = open(filename,O_RDWR)) < 0)
		return -1;

	DEBUG_0( "open() returned ok\n" )

	/*
	 * Search all connection locations of an adapter for 
	 * powered on controllers.
	 */

	for( sid=0; sid<8; sid++ ) {
		DEBUG_1("RUNNING INQUIRY ON sid %d\n", sid )

		/* 
		 * build inquiry command structure 
		 */
		memset( &inq, 0, sizeof(inq) );
		inq.data_length = INQSIZE;
		inq.buffer = inqdata;
		inq.timeout_value = 2;
		inq.command_length = 6;
		inq.resvd5 = ( sid << 5 ) | 0x10; /* 0x10 means controller */
		inq.scsi_cdb[0] = 0x12;	          /* i.e. an inquiry */

		inq.scsi_cdb[4] = INQSIZE;
		inq.flags = B_READ;

                /*
		 * Send inquiry to this connection via a pass thru
		 * scsi inquiry thru the adapter
		 */
		if( run_scsi_cmd(fd, &inq) ) {
			if( (int)inq.status_validity & 0x04 )
				if( (int)inq.resvd2 == SD_BAD_CTRL_ADDRESS ) {
					DEBUG_1("sid %d out of range\n", sid )
					break;
				}
			continue;
		}

		sprintf( sstr, "parent = '%s' AND connwhere = '%d'",
			lname, sid );
		rc = (int)odm_get_obj( CuDv_CLASS, sstr, &cusobj, ODM_FIRST );

		if( rc == -1 ) {
			close( fd );
			return E_ODMGET;
		}
		
		if( rc == 0 ) {
			/* 
			 * If not already defined or available then 
			 * define a new controller.
			 */
			sprintf(sstr, "-c %s -s %s -t %s -p %s -w %d",
				PdDv.class, PdDv.subclass, PdDv.type, lname,
				sid );
			DEBUG_2("def_scsi_dev: Do method :%s %s\n",
				PdDv.Define, sstr)
			if (odm_run_method(PdDv.Define, sstr, &out_p, NULL)) {
				DEBUG_1("def_scsi_dev:can't run %s\n",
					PdDv.Define)
				close( fd );
				return E_ODMRUNMETHOD;
			}
			fprintf(stdout, "%s ", out_p);
		} else {
			if (cusobj.chgstatus == MISSING ) {
				cusobj.chgstatus = SAME;
				if( odm_change_obj( CuDv_CLASS,&cusobj) == -1){
					close( fd );
					return E_ODMUPDATE;
				}
			}
			printf( "%s \n", cusobj.name );
		}
	}
	close( fd );
	return 0;
}


/* 
 *
 * NAME: sd_child_process
 *                  
 * FUNCTION: Execs off a config  method daemon.
 *   
 *        Since configuration aspects involve operations best done
 *        outside the kernel,we will exec off a daemon "method".
 *        This method (in daemon_child) will determine what type of 
 *        event the daemon had received and then will perform the requested
 *        operations (ie download microcode or odm_run a config method).
 *       
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault.
 *
 * 
 * INTERNAL PROCEDURES CALLED:
 * 
 *      None
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *   
 *      fork                          getpenv
 *      setpenv                       open
 *      close
 *      
 *
 * (RECOVERY OPERATION:)  If an error occurs, the proper errno is returned
 *      and the caller is left responsible to recover from the error.
 *
 * RETURNS:     
 *       
 *        pid       -   Successful completion.
 *       -1         -   If fork, getpenv or setpenv fails.
 */ 
int sd_child_process(char *adap_name,	/* Logical name of adapter      */
		     int   phase)	/* Phase that is running        */
{
	int   pid;                                   /* process id          */
	/*char **envp;   use for getpenv*/           /* environment for     */
	char  *envp[2];          		     /* environment for     */
						     /* execed process      */
	int   rc = 0;
	char *argv[4];                               /* for passing values to*/
						     /* the execed process  */
        char  value[9];                              /* string equivalent of*/
                                                     /* an int, one index per*/
						     /* nibble (hex digit)   */
        int   shift;                                 /* for bitwise shifting*/
        int   work;                                  /* work variable       */
        int   nibble;                                /* One hex digit       */
        int   index;                                 /* index into value    */
                                                     /* array               */
        /*
         * Array for converting hex digits into
         * characters 
         */

        char convert[] = { '0','1','2','3','4','5','6','7','8','9',
                                   'A','B','C','D','E','F'};
        int   count;                                 /* general  counter    */




	if ((pid = fork()) < 0) {  /* fork failed */
		DEBUG_0("Fork failed \n")
		return(-1);       
	}                 
	else if (pid > 0)  {               /* parents fork call */
		DEBUG_0("Fork succeeded \n")
		return(pid);                       
	}

	/*
	 * Only the child process will reach this point
	 * ie fork = 0.
	 * NOTE: From here down all failures that cause 
	 * a return without a successfull exec need to
	 * return a -2 so define_children can distinguish
	 * the forked child process from the parent process.
	 */

	/*
	 * Pass arguments and environment to child
	 */
	DEBUG_0("Child continuing \n")

	/*
	 * Get current (parent's) environment
	 * This has been removed since libs.a which is required for it
	 * will enlarge the config method by about 60K.  If dynamic
	 * configuration is supported or the ability to move the
	 * database is desired then this getpenv will need to be uncommented
	 * as well as it corresponding setpenv below.
	 */



	/* envp = getpenv(PENV_USR | PENV_SYS);
	if (envp == NULL) {
		DEBUG_0("Getpenv failed  \n")
		DEBUG_1("child: errno = %d\n",errno)
		DEBUG_0("child: daemon will die now\n")
		return(-2);
	}
        else {
		DEBUG_0("Child got environment  \n")
	}  */
	
	/*
	 * Do some clean up so if we are invoked
	 * via odm_run_method the daemon will not 
	 * cause a hang.  odm_run_method waits for
	 * its descendents to close their stdin, stdout,stderr
	 */
        
	close(0);
	close(1);
	close(2);

#ifdef CFGDEBUG

	/*
	 * Only need to open stdin, stdout, and stderr
	 * for debugging purposes
	 */
	if (open("/dev/console",O_RDONLY) < 0) {
		
		return(-2);
	}
		
        if (open("/dev/console",O_WRONLY) < 0) {
		
                return(-2);
	}
	if (open("/dev/console",O_WRONLY) < 0) {
		
		return(-2);
	}
#endif
	/* 
	 * so we will not die when the 
	 * config method completes we must change
	 * our process group
	 */

	(void) setpgrp();   

	argv[1] = adap_name; 
        
	argv[3] = NULL;


        /*
         * Convert hex (32bits) to individual characters to make up a string
	 * by converting one nibble (hex digit)
	 * at a time to its ascii equivalent.
	 * This is required because the element os argv passed to the
	 * child created by setpenv must all be strings
         */
        work = phase;
        shift = 0;
        value[8] = '\0';          /* Null terminate string */
        index = 7;                /* start at lowest nibble  */

	/* 
	 * loop 8 times since there are 
	 * 8 nibbles in a 32 bit word
	 */
        for (count = 0 ; count < 8;count++) {
		/*
		 * Get one nibble at a time 
		 * and convert it to ascii storing 
		 * it in value
		 */
                nibble = (work >> shift) & 0xF;
                value[index] = convert[nibble];
                shift += 4;
                index--;
        }

        argv[2] = &value[0];
	DEBUG_1("d_child_process:phase = %d\n",phase)
	DEBUG_1("sd_child_process: argv[2] =? phase = %s\n",argv[2])
	argv[0] = daemon_child;

	envp[0] = "ODMDIR=/etc/objrepos";

	envp[1] = NULL;	
	DEBUG_3("Execing off %s, argv[1] = %s,argv[2] = %s\n",argv[0],
		argv[1],argv[2]);
	DEBUG_2("envp[0] = %s, envp[1] = %s\n",envp[0],envp[1])
	
	/* 
 	 * Exec off daemon with same 
	 * environment settings that we (cfgserdasda)
	 * have
	 */
	/* rc = setpenv(NULL,(PENV_ARGV | PENV_RESET),envp,(char *)argv);
	if (rc < 0) {
		DEBUG_1("SERDASD_DAEMON: setpenv failed rc = %d\n",rc)
	}*/
	
	/*
	 * Need to hard code ODMDIR if not using getpenv and setpenv
	 */

        /*
	 * Need to use execve if not using setpenv
	 */
       if (execve(argv[0],argv,envp)) {
		DEBUG_1("Exec failed argv[0] = %s\n",argv[0])
		/*
		 * Use a different return here to indicate 
		 * that this is an child process failure.
		 */
		return(-2);
	}
	return (0);
}
