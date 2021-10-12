static char sccsid[] = "@(#)23	1.13  src/bos/usr/lib/methods/sdd/serdasd_daemon.c, cfgmethods, bos411, 9428A410j 4/1/93 11:48:03";
/*
 * COMPONENT_NAME: (CFGMETH) Configuration Daemon for Serial DASD Adapter
 *
 * FUNCTIONS : main() 
 * 
 * ORIGINS : 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1991
 * Unpublished Work
 * All Rights Reserved
 *
 * RESTRICTED RIGHTS LEGEND
 * US Government Users Restricted Rights -  Use, Duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/* header files needed for compilation */
#include <stdio.h>
#include <cf.h>
#include <sys/bootrecord.h>
#include <sys/errno.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include "cfgserdasd.h"
#include "cfgdebug.h"
#include <usersec.h>
#include <sys/sd.h>
#include <signal.h>
#include <sys/wait.h>

#define DASD_TYPE 0
#define CTRL_TYPE 0x10
#define DEVICE_TYPE(a) ( a & CTRL_TYPE )
#ifndef INQSIZE
#define INQSIZE 255
#endif
#ifndef NULLPVID
#define NULLPVID "00000000000000000000000000000000"
#endif

struct CuDv adapcudv;
struct PdDv adappddv;
struct PdDv ctrlpddv;

int     read_dev  = FALSE;              /* True if initial reading of   *
					/* adapter from ODM has been done*/
char	dev[SD_DEV_FILE_LENGTH];        /* Adapter special file name    */
int     loop = TRUE;                	/* while loop controller        */
char   *adap_lname;                  	/* name of adapter              */
int     adap = -1;                      /* file descriptor for adapter  */
int     phase = 0;                      /* phase daemon was created     */
 


/* 
 *
 * NAME: main
 *                  
 * FUNCTION:  Daemon for serial dasd subsystem device driver
 *                                                 
 *    This routine will wait via a wait call, until an asynch event
 *    occurs for this given adapter.  The device driver's interrupt
 *    handler will queue up an sd_event structure and signal the
 *    daemon.  The daemon will then perform an ioctl to get the event.
 *    The daemon will then download microcode to either the
 *    controller or adapter.  When the adapter is being UNCONFIGURED
 *    the ioctl will send signal 3.      The daemon
 *    will then kill itself by breaking out of the while loop
 *    and closing the adapter.
 *
 * MOTIVATION:
 *
 *    Since the Serial adapters and controllers will lose their
 *    microcode on resets.  It is necessary for the device driver 
 *    to have a means to download microcode as part of its error 
 *    recovery.  However the device driver's error recovery is done
 *    on the interrupt level so it can not do the download.  Instead it
 *    will signal the daemon to do the download (if necessary) on its 
 *    behalf.
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault.  This daemon is execed by cfgserdasda the adapter
 *      config method for serial DASD subsystem
 *
 *
 *
 * (DATA STRUCTURES:)  struct sd_event      - event for config daemon
 *                     
 * 
 *
 * CALLED BY:
 *      cfgserdasda   - actually execed by it
 *
 * INTERNAL PROCEDURES CALLED:
 * 
 *     
 * 
 * EXTERNAL PROCEDURES CALLED:
 *      
 *      getpid                      open
 *      openx                       ioctl
 *      wait                        sigaction
 *
 *
 * (RECOVERY OPERATION:) If error occurs the daemon may kill itself
 *     depending on the severity of the error, otherwise it will
 *     try to continue running
 *
 * RETURNS:      
 *       
 *        0         -   Successful completion.
 */  
int main(int argc,                           /* Number of arguments       */
	 char **argv, 			     /* arguments passed to daemon*/
	 char **envp)                        /* environment settings      */
{
	int      rc = 0;                     /* return code of daemon     */
#ifdef CFGDEBUG
	int      pid;                        /* daemon's pid              */
#endif
	int      status = 0;
	char    *filename = DAEMON_NAME;     /* used to build daemon's    */
					     /* special filename          */

	struct sigaction event_handle,       /* signal for event from driver*/
	                 die_daemon;         /* signal to die               */



	sd_ignore_signal();
	DEBUG_0("SERDASD_DAEMON: Daemon just starting \n")

	/* 
	 * Fill in sigaction for an asynch event signal
	 */

	event_handle.sa_handler = (void (*)(int))sd_handle_event;
	event_handle.sa_mask.losigs = 0;
	event_handle.sa_mask.hisigs = 0;
	event_handle.sa_flags = 0;

	/* 
	 * Fill in sigaction for killing daemon
	 */

	die_daemon.sa_handler = (void (*)(int))sd_kill_daemon;
	die_daemon.sa_mask.losigs = 0;
	die_daemon.sa_mask.hisigs = 0;
	die_daemon.sa_flags = 0;	

	/* 
	 * Get adapter's name from argument list passed down from
	 * cfgserdasda (adapter config method)
	 */

	adap_lname = argv[1];

	/*
	 * Get config phase daemon was created in
	 */

	phase = atoi(argv[2]);

#ifdef CFGDEBUG
	pid = getpid();
#endif
	DEBUG_1("SERDASD_DAEMON: pid = %d\n",pid)
	DEBUG_1("SERDASD_DAEMON: adap_lname = %s|\n",adap_lname)
	DEBUG_1("SERDASD_DAEMON: phase = %d\n",phase)	
	/* 
	 * make special filename (in dev) for this adapter thru daemon 
	 * entry point
	 */


	strcat(filename,adap_lname);
        sprintf(dev,"/dev/%s",filename);
	sprintf(dev,"/dev/%s",filename);  /* compiler bug ? */

	/*
	 * Register daemon's pid with device driver by opening
	 * adapter in daemon mode
	 */


	sigaction(SIGTRAP,&event_handle,NULL);
	sigaction(SIGQUIT,&die_daemon,NULL);

	/*
	 * Daemon should always be able
	 * to open adapter
	 */
	if (( adap = openx(dev,O_RDWR,0,SD_DAEMON)) < 0 ) {
		DEBUG_1("SERDASD_DAEMON:Failed to open %s for daemon open\n",dev)
		DEBUG_1("SERDASD_DAEMON:Errno = %d\n",errno)
		return;
	}

	DEBUG_0("SERDASD_DAEMON: Daemon open successful\n") 


        /* 
	 * Perform initial download to adapter only if this is phase 2
	 * do not do this for runtime config to avoid possible lock 
	 * contention with subsequent controller config method invocations
	 * which may be attempted while we are doing a download.
	 * The initial download for runtime will be handled by the
	 * adapter config method.
	 * For diagnostic the phase 2 invocation will not cause the
	 * the daemon to download because no download will be required
	 * because it would have been done by adapter config.  This
	 * avoids the problem of device not ready yet, when invoking 
	 * controller config methods.
	 */

	 
	 if (phase == PHASE2) {
		 sd_phase_two(adap_lname,adap);

	 }
	/*
	 * Loop until signaled by the interrupt handler
	 */
	while (loop) {
		sleep(1000000000);
	}


	close(adap);

	DEBUG_0("SERDASD_DAEMON: Daemon dying now\n")
	return (0);
}
/* 
 *
 * NAME: sd_handle_event
 *                  
 * FUNCTION: 
 *      This routine will get an event via ioctl to the device.  It will
 *      then take the necessary action (ie download microcode).  When no
 *      more events are queued up by the device driver the ioctl will
 *      return the errno ECHILD.
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault. 
 *
 *
 *
 * (DATA STRUCTURES:)  struct sd_event      - event for config daemon
 *                     
 * 
 *
 * CALLED BY:
 *      sigaction
 *
 * INTERNAL PROCEDURES CALLED:
 * 
 *     
 *
 * EXTERNAL PROCEDURES CALLED:
 *      
 *      getpid                      open
 *      openx                       ioctl
 *
 * Note:
 *
 *     For microcode download download_microcode_adap and 
 *     download_microcode_ctrl they will get the
 *     database initialized and locked for the daemon and unlockit
 *
 * (RECOVERY OPERATION:) If error occurs the daemon may kill itself
 *     depending on the severity of the error, otherwise it will
 *     try to continue running
 *
 * RETURNS:      
 *       
 *        0         -   Successful completion.
 */  
void sd_handle_event(void)
{
	struct sd_event event_head;         /* asynchronous event           */
	int     loop2 = TRUE;               /* while loop controller        */
	int     rc = 0;                     /* return code                  */
	int      temp;                      /* temporary variable           */
	int      sid;                       /* location of controller       */
	struct CuDv adapcudv;               /* adapter customized data      */
	struct PdDv adappddv;               /* adapter predefined data      */ 
	struct PdDv ctrlpddv;               /* controller predefined        */
	uchar  tarlun;                      /* tarlun of device 	    */
	struct CuDv ctrlcudv;               /* controller's customize data  */
	struct CuDv dasdcudv;               /* dasd's customized data       */
	struct PdDv dasdpddv;               /* dasd's predefined data       */
	struct CuAt dasdcuat;               /* dasd's customized attribute  */
	char sstr[100];			    /* search string for database   */
	unique_id_t	disk_pvid;          /* physical volume identfier    */
	char		pvidstr[33];        /* physical volume identfier    */
	int    retrys = 0;                  /* number of retrys of a command*/
	int    lock;                        /* data base lock               */



	if (adap <  0) {
		DEBUG_0("\n\nSERDASD_DAEMON: Failed to get an event\n")
		return;
	}
	DEBUG_0("\n\nSERDASD_DAEMON: Daemon getting an event\n")

	/* 
	 * When an asynch event occurs the interupt handler will 
	 * queue up an sd_event before it signals the daemon. When
	 * no more asynch events are left ioctl will return ECHILD
	 */

	while (loop2) {
		event_head.tarlun = 0;
		event_head.event = 0;
		rc = ioctl(adap, SD_GET_EVENT,&event_head);
		if( rc) {
			
#ifdef CFGDEBUG
			switch( errno ) {
			      case EFAULT:
				DEBUG_0("SERDASD_DAEMONerror: EFAULT\n")
				break;
			      case ETIMEDOUT:
				DEBUG_0("SERDASD_DAEMONerror: ETIMEDOUT\n")
				break;
			      case EIO:
				DEBUG_0("SERDASD_DAEMON:error: EIO\n")
				break;
			      case ECHILD:
				/* 
				 * No more events are left
				 */
				DEBUG_0("SERDASD_DAEMON: ECHILD- no more events\n")
				loop2 = FALSE;  /* Exit loop */
				break;
			      default:
				DEBUG_1("SERDASD_DAEMON Ioctl error: %d\n", errno )
				}
#endif
			retrys++;
			
		}

		if (!loop2) {
			break;      /* Exit loop */
		}
		if (retrys > 3) 
			break;
		if (rc)
			continue;
		/*
		 * Process event received from interrupt handler
		 */

		
		DEBUG_2("SERDASD_DAEMON: tarlun = %d event = %d\n",
			event_head.tarlun,event_head.event)
		tarlun = event_head.tarlun;
		

		if( odm_initialize() == -1 ) {
			DEBUG_0("SERDASD_DAEMON: Daemon dying now\n")
			break;
			
		}

		
		if ( event_head.event & SD_ADAPCFG ) {
			
			DEBUG_0("SERDASD_DAEMON:Adapter config\n")
		}
		
		if ( event_head.event & SD_ADAPDLMC ) {
			DEBUG_0("SERDASD_DAEMON:Download to adapter\n")
			rc = 0;
			rc = download_microcode_adap(adap_lname,1,adap); 

			if (rc) {
				sleep(10);
				DEBUG_0("SERDASD_DAEMON:Download to adapter failed retrying\n")
				rc = 0;
				rc = download_microcode_adap(adap_lname,1,
							     adap); 

				if (rc) {
					DEBUG_0("SERDASD_DAEMON:Download to adapter failed retry\n")
					/*
					 * Download and its retry failed
					 * so log this
					 */
					 sd_log_daemon_error(SD_ADAP_CMD,0x0,
							     0x1ff,
							     SD_DAEMON_CMD_LOG,
							     0,0,0,0,0,0);

					}
				}
		}
		
		if( event_head.event & SD_DLMC ) {
			DEBUG_0("SERDASD_DAEMON: Download to controller\n")
			if (DEVICE_TYPE(tarlun) == CTRL_TYPE) {
				sleep(5);
				sid = SD_TARGET(tarlun);
			
			        DEBUG_2("SERDASD_DAEMON:sid = %d,tarlun = %d\n",
				       sid,tarlun)
			        rc = download_microcode_ctrl(sid,adap_lname,
							    1,adap);
				if (rc) {
					DEBUG_0("SERDASD_DAEMON:Download to controller failed now retrying\n")
				        sleep(30);
					rc = 0;
				        rc = download_microcode_ctrl(sid,
								     adap_lname,
								     1,adap);
				       if (rc) {
					       DEBUG_0("SERDASD_DAEMON:Download to controller failed retry\n")
					       /*
						* Download and its retry failed
						* so log this
						*/
					       sd_log_daemon_error(SD_CTRL_CMD,
								   tarlun,0x1ff,
								   SD_DAEMON_CMD_LOG,
								   0,0,0,0,0,0);
					       }
			       }

			}
			else {
				DEBUG_0("SERDASD_DAEMON:Invalid controller!! \n")
				}
		}
		if (( event_head.event == SD_CONFIG ) 
		    && ( DEVICE_TYPE(tarlun) == CTRL_TYPE )) {
			DEBUG_0("SERDASD_DAEMON:Config controller\n")
		}
		if( ( event_head.event == SD_CONFIG ) && 
		   ( DEVICE_TYPE(tarlun) == DASD_TYPE )) {
			
			DEBUG_0("SERDASD_DAEMON:Config dasd\n")
			

		}

		/*
		 * Provide small window for other processes to access data base
		 */
		 odm_terminate();
		 sleep(1);
	} /* while loop */
	
	return;
}

/* 
 *
 * NAME: sd_phase_two
 *                  
 * FUNCTION: 
 *     
 *	This routine will only be called if the daemon is created at the 
 *	beginning of phase 2.  It is necessary to download to its (the
 *	daemon's) adapter and to all of the available controllers connected
 *	to this adapter.
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault. 
 *
 *
 *
 * (DATA STRUCTURES:)  struct sd_event      - event for config daemon
 *                     
 * 
 *
 * CALLED BY:
 *      main
 *
 * INTERNAL PROCEDURES CALLED:
 * 
 *   	 download_microcode_adap		download_microcode_ctrl
 *     
 *
 * EXTERNAL PROCEDURES CALLED:
 *      
 *	odm_initialize				odm_lock
 *	odm_terminate				odn_unlock
 *     
 *
 * (RECOVERY OPERATION:) If error occurs the daemon may kill itself
 *     depending on the severity of the error, otherwise it will
 *     try to continue running
 *
 * Note:
 *
 *     For microcode download download_microcode_adap will get the
 *     database initialized and locked for the daemon and unlock it.
 *
 * RETURNS:      
 *       
 *        0         -   Successful completion.
 */  
void sd_phase_two(char *adap_lname,int adap)
{
	int rc = 0;                 /* return code from dowload routines */
	int lock;  		    /* data base lock			 */
	struct CuDv cusobj;	    /* Controller CuDv object 		 */
	struct CuDv dasdobj;	    /* DASD CuDv Object 		 */
	struct CuAt dasdatt;	    /* DASD CuAt Object 		 */
	struct PdDv PdDv;	    /* DASD PdDv Object 		 */
	char	sstr[100];	    /* Search string 			 */   
	int	sid;                /* controllers connection 		 */
	int     status;             /* used for data base search         */    


	DEBUG_0 ("SERDASD_DAEMON: phase 2 download starting\n")

	if( odm_initialize() == -1 ) {
		DEBUG_0("SERDASD_DAEMON: Daemon dying now\n")
	        return;
	}
	rc = download_microcode_adap(adap_lname,1,adap); 
	if (rc) { 

		DEBUG_0("SERDASD_DAEMON:Download to adapter failed retrying\n")
		rc = download_microcode_adap(adap_lname,1,adap); 
		if (rc) {
			DEBUG_0("SERDASD_DAEMON:Download to adapter failed retry\n")
		        /*
			 * Download and its retry failed
			 * so log this
			 */
			 sd_log_daemon_error(SD_ADAP_CMD,0x0,0x1ff,
					     SD_DAEMON_CMD_LOG,0,0,0,0,0,0);


		}
	}

	odm_terminate();
	return;
}


/* 
 *
 * NAME: sd_kill_daemon
 *                  
 * FUNCTION: Terminate main which is the config daemon for the
 *           serial DASD subsystem.
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault.  
 *
 *
 *
 * (DATA STRUCTURES:) 
 *                     
 * 
 *
 * CALLED BY:
 *     sigaction
 *
 * INTERNAL PROCEDURES CALLED:
 * 
 *    None
 *
 * EXTERNAL PROCEDURES CALLED:
 *      
 *    None
 *
 * (RECOVERY OPERATION:) If error occurs the daemon may kill itself
 *     depending on the severity of the error, otherwise it will
 *     try to continue running
 *
 * RETURNS:      
 *       
 *        0         -   Successful completion.
 */  
void sd_kill_daemon(void)
{

	loop = FALSE;
	return;
}


/* 
 *
 * NAME: sd_ignore_signal
 *                  
 * FUNCTION: Prevent all signals except SIGKILL from interrupting daemon.
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault.  
 *
 *
 *
 * (DATA STRUCTURES:) 
 *                     
 * 
 *
 * CALLED BY:
 *     main
 *
 * INTERNAL PROCEDURES CALLED:
 * 
 *    None
 *
 * EXTERNAL PROCEDURES CALLED:
 *      
 *    sigaction
 *
 * (RECOVERY OPERATION:) If error occurs the daemon may kill itself
 *     depending on the severity of the error, otherwise it will
 *     try to continue running
 *
 * RETURNS:      
 *       
 *        0         -   Successful completion.
 */
void sd_ignore_signal(void)
{
	struct sigaction event;

	event.sa_handler = SIG_IGN;
	event.sa_mask.losigs = 0;
	event.sa_mask.hisigs = 0;
	event.sa_flags = 0;

	/*
	 * Block all known signals
	 */
	sigaction(1,&event,NULL);
	sigaction(2,&event,NULL);
	sigaction(3,&event,NULL);
	sigaction(4,&event,NULL);
	sigaction(5,&event,NULL);
	sigaction(6,&event,NULL);
	sigaction(7,&event,NULL);
	sigaction(8,&event,NULL);
	sigaction(10,&event,NULL);
	sigaction(11,&event,NULL);
	sigaction(12,&event,NULL);
	sigaction(13,&event,NULL);
	sigaction(14,&event,NULL);
	sigaction(15,&event,NULL);
	sigaction(16,&event,NULL);
	sigaction(17,&event,NULL);
	sigaction(18,&event,NULL);
	sigaction(19,&event,NULL);
	sigaction(20,&event,NULL);
	sigaction(21,&event,NULL);
	sigaction(22,&event,NULL);
	sigaction(23,&event,NULL);
	sigaction(24,&event,NULL);
	sigaction(25,&event,NULL);
	sigaction(26,&event,NULL);
	sigaction(27,&event,NULL);
	sigaction(28,&event,NULL);
	sigaction(29,&event,NULL);
	sigaction(30,&event,NULL);
	sigaction(31,&event,NULL);
	sigaction(32,&event,NULL);
	sigaction(33,&event,NULL);
	sigaction(34,&event,NULL);
	sigaction(35,&event,NULL);
	sigaction(36,&event,NULL);
	sigaction(60,&event,NULL);
	sigaction(61,&event,NULL);
	sigaction(62,&event,NULL);
	sigaction(63,&event,NULL);

	return;

}

/* 
 *
 * NAME: sd_unlock_sleep_lock
 *                  
 * FUNCTION: Unlock database and sleep.  Then gets database lock again.
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault.  
 *
 *
 *
 * (DATA STRUCTURES:) 
 *                     
 * 
 *
 * CALLED BY:
 *     main				sd_phase_two
 *     sd_handle_event
 *
 * INTERNAL PROCEDURES CALLED:
 * 
 *    None
 *
 * EXTERNAL PROCEDURES CALLED:
 *      
 *    odm_initialize			odm_lock
 *    odm_unlock			odm_terminate
 *
 * (RECOVERY OPERATION:) If error occurs the daemon may kill itself
 *     depending on the severity of the error, otherwise it will
 *     try to continue running
 *
 * RETURNS:      
 *       
 *        0         -   Successful completion.
 */
int sd_unlock_sleep_lock(int sleep_for,int *lock)
{
	

	if (odm_unlock(*lock) == -1) {
		DEBUG_0("SERDASD_DAEMON: ODM unlocked failed\n")
		return(-1);
	}
	odm_terminate();
	sleep(sleep_for);
	if( odm_initialize() == -1 ) {
		DEBUG_0("SERDASD_DAEMON: Daemon dying now\n")
		return (-1);
		
	}
	
	DEBUG_0 ("SERDASD_DAEMON: ODM initialized now getting lock\n")
		
       /* lock the database */
        while ((*lock =odm_lock("/etc/objrepos/config_lock",0)) == -1) {
		       DEBUG_0("SERDASD_DAEMON: odm_lock() failed\n")
		       sleep(1);
			
        }
	
	DEBUG_0 ("ODM locked\n")

	return (0);
}

/* 
 *
 * NAME: sd_log_daemon_error
 *                  
 * FUNCTION: Log an error via the device driver.
 *
 *    This routine will use an ioctl to the driver to allow the daemon
 *    log errors using the device drivers internal error log routines
 *                                                                         
 * EXECUTION ENVIRONMENT:                                                  
 *                                                                         
 *      This routine is  called by a process at the process level and it
 *      can page fault.  
 *
 *
 *
 * (DATA STRUCTURES:) 
 *                     
 * 
 *
 * CALLED BY:
 *     sd_phase_two
 *     sd_handle_event
 *
 * INTERNAL PROCEDURES CALLED:
 * 
 *    None
 *
 * EXTERNAL PROCEDURES CALLED:
 *      
 *    Ioctl
 *
 * (RECOVERY OPERATION:) If error occurs the daemon may kill itself
 *     depending on the severity of the error, otherwise it will
 *     try to continue running
 *
 * RETURNS:      
 *       
 *       Void
 */
void sd_log_daemon_error(uchar type,        /* cmd type : adap, ctrl,dasd  */
			 uchar tarlun,      /* device address              */
			 ushort uec,        /* Unit Error Code             */
			 uchar err_function,/* log function to call        */
			 uint  elog_sys_dma_rc,
			 uchar status_validity, 
			 uchar adapter_status, 
			 uchar controller_status, 
			 uchar scsi_status,
			 uchar elog_validity)
{
	struct sd_daemon_errlog d_log;      /* for error logging information */
	int			 rc =0;     /* ioctl's return code           */


	DEBUG_0("Sd_log_daemon_error: Entering function\n")
	/*
	 * Build up structure to pass to device driver
	 */
	d_log.cmd_type 			= type;
	d_log.log_func 			= err_function;
	d_log.elog_sys_dma_rc 		= elog_sys_dma_rc;
	d_log.status_validity 		= status_validity;
	d_log.scsi_status		= scsi_status;
	d_log.adapter_status		= adapter_status;
	d_log.controller_status		= controller_status;
	d_log.driver_status		= 0;
	d_log.uec			= uec;
	d_log.tarlun			= tarlun;
	d_log.elog_validity		= elog_validity;
	

	/*
	 * Have device driver log this error for us
	 */
	DEBUG_0("Sd_log_daemon_error: about to log error\n")
	rc = ioctl(adap, SD_DAEMON_ERROR,&d_log);
	if( rc) {
			
#ifdef CFGDEBUG
		switch( errno ) {
		      case EFAULT:
			DEBUG_0("SERDASD_DAEMONerror: EFAULT\n")
				break;
		      case ETIMEDOUT:
			DEBUG_0("SERDASD_DAEMONerror: ETIMEDOUT\n")
				break;
		      case EIO:
			DEBUG_0("SERDASD_DAEMON:error: EIO\n")
				break;
		      case ECHILD:
			/* 
			 * No more events are left
			 */
			DEBUG_0("SERDASD_DAEMON: ECHILD- no more events?\n")
			break;
		      default:
			DEBUG_1("SERDASD_DAEMON Ioctl error: %d\n", errno )
	       }
#endif

			
	}
	DEBUG_0("Sd_log_daemon_error: Successfully logged\n")
	return;
}
