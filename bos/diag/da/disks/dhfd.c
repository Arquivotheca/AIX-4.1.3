static char sccsid[] = "@(#)46	1.20.1.1  src/bos/diag/da/disks/dhfd.c, dadisks, bos411, 9428A410j 3/11/94 15:18:26";
/*
 *   COMPONENT_NAME: DADISKS
 *
 *   FUNCTIONS: MALLOC
 *		calc_se_rate
 *		calc_sk_rate
 *		calc_sr_rate
 *		chk_asl_status
 *		chk_ela
 *		chk_errno
 *		chk_terminate_key
 *		clean_up
 *		create_frub
 *		data_read
 *		diag_tst
 *		display_title
 *		main
 *		tu_test
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#define R2_ELA 

#include <stdio.h>
#include <locale.h>
#include <fcntl.h>
#include <nl_types.h>
#include <limits.h>
#include <memory.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/buf.h>
#include <sys/devinfo.h>
#include <sys/cfgodm.h>
#include <sys/badisk.h>
#include <sys/ioctl.h>
#include <sys/mdio.h>
#include <sys/iplcb.h>
#include <sys/errids.h>
#include "../../tu/bad/badatu.h"
#include "dhfd.h"                                   /* test cases and globals */
#include "dhfd_msg.h"                              /* message catalog numbers */

#include <diag/tm_input.h>                                 /* faking the ODM  */
#include <diag/tmdefs.h>                   /* diagnostic modes and variables  */
#include <diag/da.h>                       /* FRU Bucket structure definition */
#include <diag/diag.h>
#include <diag/diag_exit.h>                                /* DA return codes */
#include <diag/diago.h>
#include <diag/diagodm.h>
#include <diag/dcda_msg.h>                    /* catalog of standard messages */
#include <diag/bit_def.h>

                               /* Allocate storage to a variable or structure */
#define MALLOC(x)       (( x *) malloc(sizeof(x)))

/******************************************************************************/
/*  initialze the frubucket structure that will be used to report back        */
/*  a fru if a problem is found.                                              */
/******************************************************************************/

#define	MAX_FRU_ENTRY	2
#define	ELA_SAMPLE_SIZE	125
struct fru_bucket frub[] =
{
        { "",FRUB1,  0x949 ,  0x0 ,  0 ,  {
                        { 100, "", "", 0, DA_NAME, EXEMPT },
                },
        },

        { "",FRUB1,  0x949 ,  0x0 ,  0 ,  {
                        {  90, "", "", 0, DA_NAME, EXEMPT },
                        {  10, "", "", 0, PARENT_NAME, EXEMPT },
                },
	},
        { "",FRUB1,  0x949 ,  0x0 ,  0 ,  {
                        {  90, "", "", 0, DA_NAME, EXEMPT },
			{   5, "bus extender", "", DHFD_RISER_CARD, NOT_IN_DB, EXEMPT },
                        {   5, "", "", 0, PARENT_NAME, EXEMPT },
                },
        },
};

struct tm_input tm_input;
extern disp_menu();
long   damode=0;                      /* Selected Diagnostic Application mode */
int    conf_lvl;         /* indicates confidence level.  1 = 100%, 0 = <100%  */
char    *devname = NULL;                       /* name of device to be tested */
char    dname[] = "/dev/r";         /* dname from tm_input concatenated later */
nl_catd catd;                                  /* pointer to the catalog file */
extern nl_catd diag_catopen(char *, int);
static char  problem_found = FALSE;  /* flag to be set if a FRUB is reported  */
static char  i_lvmed_it=FALSE;
static int   init_config_state = -1;
int fd;                                    /* file designator for nvram ioctl */
int	certify_in_progress = 0;
struct badisk_ioctl_parms badisk_ioctl_params;
struct badisk_ioctl_parms arg_cert;
struct badisk_cstat_blk arg_stat;
IPL_DIRECTORY iplcb_dir;
IPL_INFO      iplcb_info;
SJL_DISK_POST_RESULTS  disk_result;

/*  */
/*
 * NAME: main()
 *
 * FUNCTION: Main routine for the Direct Bus Attach Fixed-Disk Diagnostic
 *           Application. It is invoked by the diagnostic Controller.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: The return values are set up in each function called.
 *          DA_EXIT() is called to return the codes to the
 *          controller.
 */

main(argc,argv,envp)

int argc;
char **argv;
char **envp;

{
#ifdef R2_ELA
        void    chk_ela();
#endif
        void    diag_tst();
        int     create_frub();
        void    chk_terminate_key();
        int     chk_asl_status();
	char	criteria[80];
        int     tu_test();
        int     chk_errno();
        void    clean_up();
	void	display_title();
        int     rc_errno;
        int     lvm_rc = -99;                          /* LVM/SYS return code */
        register int     rc = -99;    /* generic ret code used lots of places */
	int	howmany;
	register int	 i;
        MACH_DD_IO  l,m,n;
        int     kaz_result;
	int	num;
	char	short_timestamp[6];
	struct	CuAt	*cuat;

	setlocale(LC_ALL,"");

          /* do some inits and set return codes to controller to known states */
        damode = (long)NULL;
        fdes = -99;
                                                 /* open and initialize dgodm */
        if ( init_dgodm() < 0  ) {
                DA_SETRC_ERROR( DA_ERROR_OTHER );
                clean_up();
        }

        if( getdainput( &tm_input ) != FALSE ) {
	       DA_SETRC_ERROR( DA_ERROR_OTHER );
	       clean_up();
        } else {
	       for(i=0;i<=MAX_FRU_ENTRY;i++)
			strcpy(frub[i].dname, tm_input.dname);
	       devname = ( char * ) malloc( NAMESIZE +
						 ( strlen( dname )+1 ));
	       if ( devname != NULL )
		       strcpy( devname, dname );
	       strcat( devname, tm_input.dname );
        } 
	   /* get the bit encoded version of the stuff in tm_input. This
	      tells the DA what it is going to do, types tests, etc.  */
       damode = getdamode( &tm_input );
       if( damode == (long)NULL ) {
	       DA_SETRC_ERROR( DA_ERROR_OTHER );
	       clean_up();           /* can't get good stuff, so quit */
       } 
		  /* start ASL if console is present and check return */
       if( damode & DA_CONSOLE_TRUE ) {
	       chk_asl_status( diag_asl_init( NULL ) );
			/* open message catalog file and check return */
	       catd = diag_catopen(MF_DHFD, 0);
       } 
	   /* Get the led value to be used for displaying screen number */
       cuat = (struct CuAt *)getattr(tm_input.dname,"size",FALSE,&howmany);
       if(cuat == (struct CuAt *)(-1)){
		/* no match  */
	       DA_SETRC_ERROR( DA_ERROR_OTHER );
	       clean_up();
       }
       if(strcmp(cuat->value, "160") == 0)
       		led_value = 0x0958;
       else
	  if(strcmp(cuat->value, "120") == 0)
		led_value = 0x0957;
	  else
		led_value = 0x0949;

       if( damode & DA_DMODE_MS1 ) { /* Missing flag set */
               DA_SETRC_TESTS (DA_TEST_FULL);
	       rc=create_frub( BADATU_0_FAIL );
	       clean_up();
       }
       if( damode & DA_DMODE_ELA ) {
	       chk_ela();
	       clean_up();
	}


        if( (fd = open( "/dev/nvram",0 ) ) < 0 ) {
                DA_SETRC_ERROR( DA_ERROR_OTHER );
                clean_up();
        }

    	l.md_incr = MV_BYTE;
    	l.md_addr = 128;
    	l.md_data = (char *) &iplcb_dir;
    	l.md_size = sizeof(iplcb_dir);
    	if (ioctl(fd, MIOIPLCB, &l)) {
       		 DA_SETRC_ERROR(DA_ERROR_OTHER);
                 clean_up();
    	}

    	m.md_incr = MV_BYTE;
   	m.md_addr = iplcb_dir.ipl_info_offset;
   	m.md_data = (char *) &iplcb_info;
   	m.md_size = sizeof(iplcb_info);
    	if (ioctl(fd, MIOIPLCB, &m)) {
      		  DA_SETRC_ERROR(DA_ERROR_OTHER);
        	  clean_up();
        }

    	n.md_incr = MV_BYTE;
   	n.md_addr = iplcb_dir.sjl_disk_post_results_offset;
   	n.md_data = (char *) &disk_result;
   	n.md_size = sizeof(disk_result);
    	if (ioctl(fd, MIOIPLCB, &n)) {
      		  DA_SETRC_ERROR(DA_ERROR_OTHER);
        	  clean_up();
        }
           /* SJL DISK POST RESULTS - KAZUSA FILES, FOR LLANO AND LITTLE ONLY */
	for (i=0; i<6; ++i)
		short_timestamp[i] = iplcb_info.ipl_ros_timestamp[i];

	num = atoi(short_timestamp);
	if (num >= 900220) {
            kaz_result = disk_result.sjl_post_result;
            kaz_result = kaz_result & 0x00000f00;
            if (kaz_result != 0) {
                    switch (kaz_result) {
                    case 0x00000100 :/* Drive in slot 07 is bad */ 
				  if(!strcmp(tm_input.dnameloc,"00-07")){
				          DA_SETRC_TESTS (DA_TEST_FULL);
					  rc = create_frub( IPLCB_ERR1 );
	    	        		  clean_up();
				  }
                                  break;
                    case 0x00000200 :/* Drive in slot 08 is bad */ 
				  if(!strcmp(tm_input.dnameloc,"00-08")){
				          DA_SETRC_TESTS (DA_TEST_FULL);
					  rc = create_frub( IPLCB_ERR2 );
	    	        		  clean_up();
				  }
                                  break;
                    case 0x00000300 : /* Both drives are bad */
			          DA_SETRC_TESTS (DA_TEST_FULL);
				  rc = create_frub( IPLCB_ERR3 );
    	        		  clean_up();
                                  break;
                    default         :
                                          break;
                    } 
	    }
	}

		   /* get set to decide types of tests to be executed */
       if( damode & DA_DMODE_PD || damode & DA_DMODE_REPAIR
				      || damode & DA_DMODE_FREELANCE ) {
	       if( !( damode & DA_LOOPMODE_EXITLM ) ) {
		       display_title();
		       init_config_state = configure_device(tm_input.dname);
		       if( init_config_state == -1 ){
				if( !( damode & DA_DMODE_REPAIR )){
#ifdef R2_ELA
					chk_ela();
#endif
    					if( problem_found )
						clean_up();
				}
		       /*
				No problem found by ELA or
				ELA not run.
		       */
		        	DA_SETRC_ERROR( DA_ERROR_OTHER );
				clean_up();
		       }
		       fdes = openx( devname, O_RDWR,NULL,1 );
		       rc_errno = errno; 

/* Check if openx failed. If it  did and mode = concurrent then issue the lvm
* command lchangepv to try and free the drive. The cmd 'system' is used to
* issue the cmd. If the lvm cmd fails the error is returned to the controller
* else diag_tst is called. If the mode is not concurrent then an open error
* is given back to DC.
*/
		       if (fdes < 0 ) {
/*
*  Hardware error if errno == EIO
*/
			       if ( rc_errno == EIO ) {
				       DA_SETRC_TESTS (DA_TEST_FULL);
				       rc=create_frub( DDCONFIG_ERR );
				       clean_up();
			       }
			       if ( rc_errno == EACCES ) {
				       if (( damode & DA_CONSOLE_TRUE ) &&
					      (! tm_input.system)) {
					       i_lvmed_it = TRUE;
					       rc = freedisk
						       (tm_input.dname);
				       } else {
#ifdef R2_ELA
						chk_ela();
#endif
    						if( problem_found )
							clean_up();
						else
					       		rc = -2;
				       }
				       switch ( rc ){
				       case -5 :
					       DA_SETRC_USER
							 (DA_USER_QUIT);
					       clean_up();
					       break;
				       case -4 :
					       DA_SETRC_USER
							 (DA_USER_EXIT);
					       clean_up();
					       break;
				       case -3 :
				       case -2 :
				       case -1 :
					       if(damode & DA_DMODE_PD)
							chk_ela();
					       if(problem_found == FALSE)
						       DA_SETRC_ERROR
							(DA_ERROR_OPEN);
					       clean_up();
					       break;
				       default :
					       fdes =openx(devname,
							    O_RDWR,0,1);
					       if (fdes < 0){
					       	       if(damode & DA_DMODE_PD)
								chk_ela();
						       if(problem_found == FALSE)
						      		DA_SETRC_ERROR
							           (DA_ERROR_OPEN);
						       clean_up();
					       }
					       DA_SETRC_TESTS
							 (DA_TEST_FULL);
					       display_title();
					       chk_terminate_key();
					       diag_tst( fdes );
					       chk_terminate_key();
					       break;
				       }
			       } else if ( ( rc_errno == EINVAL ) ||
					   ( rc_errno == ENOENT ) ||
					   ( rc_errno == ENXIO ) ||
					   ( rc_errno == EPERM ) ) {
					DA_SETRC_ERROR(DA_ERROR_OTHER);
					clean_up();
			       } else {
					DA_SETRC_ERROR(DA_ERROR_OPEN);
					clean_up();
			       }
		       } else {
			       DA_SETRC_TESTS(DA_TEST_FULL);
			       chk_terminate_key();
			       diag_tst( fdes );
			       chk_terminate_key();
		       } /* endif fdes < 0 */
	       }
#ifdef R2_ELA
		     /* if pd mode, and notlm or enterlm, do ela last */
	       if( damode & DA_DMODE_PD  &&
		   ( damode & DA_LOOPMODE_NOTLM ||
		     damode & DA_LOOPMODE_EXITLM ) &&
		   ( ( DA_CHECKRC_STATUS() == DA_STATUS_GOOD ) ||
		   ( DA_CHECKRC_ERROR() == DA_ERROR_OPEN ) ) ) {
		       chk_ela();
	       }
#endif
       } /* endif PD or REPAIR or FREELANCE */
        clean_up();                    /* exit call to controller...no return */
}  /* main end */
/*  */
/*
 * NAME: display_title
 *
 * FUNCTION: Display the menu title based on diagnostic mode chosen by user.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: none
 */
void
display_title()
{

        /* Show start test menu */
	if ( ( damode & DA_LOOPMODE_NOTLM ) && ( damode & DA_CONSOLE_TRUE ) &&
             		( damode & DA_ADVANCED_FALSE ) )
		   chk_asl_status( disp_menu( WE_R_TESTING ) );
        /* Show start test menu - entering loop mode */
	if ( ( damode & DA_ADVANCED_TRUE ) && ( damode & DA_CONSOLE_TRUE ) &&
          		( damode & DA_LOOPMODE_NOTLM ) )
		   chk_asl_status( disp_menu( WE_R_ADVANCED ) );
        /* show status screen for loop mode */
       	if ( ( damode & DA_ADVANCED_TRUE ) && ( damode & DA_CONSOLE_TRUE ) &&
            		( ( damode & DA_LOOPMODE_ENTERLM ) ||
             		( damode & DA_LOOPMODE_INLM ) ) )
               	   chk_asl_status( disp_menu( LOOPMODE_STATUS ) );
} /* end display_title */
/*  */
/*
 * NAME: diag_tst()
 *
 * FUNCTION: Tests the fixed-disk as outlined in the Fixed-Disk Problem
 *           Determination Guide. The function proceeds step by step calling
 *           it's respective test unit. It examines
 *           the return code from the TU and makes a decision on which
 *           step to do next. If an error occurres it sets the return
 *           code for the controller. If a problem is found during any
 *           of the steps, it calls the create_frub  procedure to set
 *           up the FRU bucket to be passed back to the controller.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: The return values are set up in each function called.
 *          DA_EXIT() is called to return the codes to the
 *          controller.
 */

#define NUM_STEPS 10                       /* number of diagnostic steps to do */
#define EXIT_STEP		0
#define DIAGNOSTIC_SELF_TEST	1
#define BUFFER_WRT_READ_COMP_00	2 
#define BUFFER_WRT_READ_COMP_FF	3
#define BUFFER_WRT_READ_COMP_AA	4
#define BUFFER_WRT_READ_COMP_55 5
#define	READ_WRITE_TEST		6
#define	SEEK_TEST		7
#define	READ_VERIFY_TEST	8
#define	IOCINFO_TEST		9
#define CERTIFY_STEP		10

void diag_tst( dt_fdes )
int     dt_fdes;
{

        int  rc;                                       /* tu_test return code */
        int errno_rc;                                 /* errno value returned */
        int  stepcount[NUM_STEPS+1];            /* counter for reaching various
                                                 * checkpoints in the test proc-
                                                 * dure. */
        int  step;                                  /* current step executing */
        extern  int     insert_frub();              /* add data in FRU bucket */
        extern  int     addfrub();                   /* add FRU bucket for DC */

        errno_rc = 0;
                                      /* clear the accumulative step counters */
        for( step = 0; step <= NUM_STEPS; ++step )
                stepcount[ step ] = 0;
        rc = 0;
        step = DIAGNOSTIC_SELF_TEST;
        while( step )
        {                                                    /* run the tests */
                switch( step ) {
                case DIAGNOSTIC_SELF_TEST :      /* TU0 */
                        ++stepcount[DIAGNOSTIC_SELF_TEST];
                        rc = tu_test( dt_fdes, step );
                        errno_rc = errno;                  /* get errno value */
                        switch( rc ) {
                        case BAD_DIAGS_GOOD :                         /* rc=0 */
                                step = BUFFER_WRT_READ_COMP_00;
                                break;
                        case BAD_DIAGS_FAIL :                         /* rc=1 */
                                step = create_frub( BADATU_0_FAIL );
                                break;
                        case BAD_ABORT_SENT :                         /* rc=2 */
                                if ( stepcount[DIAGNOSTIC_SELF_TEST] > 1 ) {
                                        step = create_frub(BADATU_0_ABRT );
                                } else {
                                        step = DIAGNOSTIC_SELF_TEST;
                                }
                                break;
                        case BAD_AIX_ERR :                           /* rc=-1 */
                                step = chk_errno( errno_rc );
                                break;
                        default :
                                DA_SETRC_ERROR( DA_ERROR_OTHER );
                                step = EXIT_STEP;
                                break;
                        } 
                        break;
                case BUFFER_WRT_READ_COMP_00 :           /* TU1  Write 0x00 */
                        ++stepcount[BUFFER_WRT_READ_COMP_00];
                        rc = tu_test( dt_fdes, step );
                        errno_rc = errno;                  /* get errno value */
                        switch( rc ) {
                        case BAD_DIAGS_GOOD :                         /* rc=0 */
                                step = BUFFER_WRT_READ_COMP_FF;
                                break;
                        case BAD_DIAGS_FAIL :                         /* rc=1 */
                                step = create_frub( BADATU_1_FAIL );
                                break;
                        case BAD_ABORT_SENT:                          /* rc=2 */
                                if ( stepcount[BUFFER_WRT_READ_COMP_00] > 1 ) {
                                        step = create_frub(BADATU_1_ABRT );
                                } else {
                                        step = DIAGNOSTIC_SELF_TEST;
                                }

                                break;
                        case BAD_BUFF_FAIL :                         /* rc=80 */
                                step = create_frub( BUF_FAILED );
                                break;
                        case BAD_AIX_ERR :                           /* rc=-1 */
                                step = chk_errno( errno_rc );
                                break;
                        default :
                                DA_SETRC_ERROR( DA_ERROR_OTHER );
                                step = EXIT_STEP;
                                break;
                        }
                        break;
                case BUFFER_WRT_READ_COMP_FF :           /* TU1 - Write 0xff */
                        ++stepcount[BUFFER_WRT_READ_COMP_FF];
                        rc = tu_test( dt_fdes, step );
                        errno_rc = errno;                  /* get errno value */
                        switch( rc ) {
                        case BAD_DIAGS_GOOD :                         /* rc=0 */
                                step = BUFFER_WRT_READ_COMP_AA;
                                break;
                        case BAD_DIAGS_FAIL :                         /* rc=1 */
                                step = create_frub( BADATU_1_FAIL );
                                break;
                        case BAD_ABORT_SENT:                          /* rc=2 */
                                if ( stepcount[BUFFER_WRT_READ_COMP_FF] > 1 ) {
                                        step = create_frub(BADATU_1_ABRT );
                                } else {
                                        step = DIAGNOSTIC_SELF_TEST;
                                }
                                break;
                        case BAD_BUFF_FAIL :                         /* rc=80 */
                                step = create_frub( BUF_FAILED );
                                break;
                        case BAD_AIX_ERR :                         /* rc = -1 */
                                step = chk_errno( errno_rc );
                                break;
                        default :
                                DA_SETRC_ERROR( DA_ERROR_OTHER );
                                step = EXIT_STEP;
                                break;
                        }
                        break;
                case BUFFER_WRT_READ_COMP_AA :           /* TU1 -  Write 0xaa */
                        ++stepcount[BUFFER_WRT_READ_COMP_AA];
                        rc = tu_test( dt_fdes, step );
                        errno_rc = errno;                  /* get errno value */
                        switch( rc ) {
                        case BAD_DIAGS_GOOD :                         /* rc=0 */
                                step = BUFFER_WRT_READ_COMP_55;
                                break;
                        case BAD_DIAGS_FAIL :                         /* rc=1 */
                                step = create_frub( BADATU_1_FAIL );
                                break;
                        case BAD_ABORT_SENT:                          /* rc=2 */
                                if ( stepcount[BUFFER_WRT_READ_COMP_AA] > 1 ) {
                                        step = create_frub(BADATU_1_ABRT );
                                } else {
                                        step = DIAGNOSTIC_SELF_TEST;
                                }

                                break;
                        case BAD_BUFF_FAIL :                         /* rc=80 */
                                step = create_frub( BUF_FAILED );
                                break;
                        case BAD_AIX_ERR :                           /* rc=-1 */
                                step = chk_errno( errno_rc );
                                break;
                        default :
                                DA_SETRC_ERROR( DA_ERROR_OTHER );
                                step = EXIT_STEP;
                                break;
                        }
                        break;
                case BUFFER_WRT_READ_COMP_55 :         /* TU1 - Write 0x55 */
                        ++stepcount[BUFFER_WRT_READ_COMP_55];
                        rc = tu_test( dt_fdes, step );
                        errno_rc = errno;                  /* get errno value */
                        switch( rc ) {
                        case BAD_DIAGS_GOOD :                         /* rc=0 */
                                step = READ_WRITE_TEST;
                                break;
                        case BAD_DIAGS_FAIL :                         /* rc=1 */
                                step = create_frub( BADATU_1_FAIL );
                                break;
                        case BAD_ABORT_SENT:                          /* rc=2 */
                                if ( stepcount[BUFFER_WRT_READ_COMP_55] > 1 ) {
                                        step = create_frub(BADATU_1_ABRT );
                                } else {
                                        step = DIAGNOSTIC_SELF_TEST;
                                }

                                break;
                        case BAD_BUFF_FAIL :                         /* rc=80 */
                                step = create_frub( BUF_FAILED );
                                break;
                        case BAD_AIX_ERR :                           /* rc=-1 */
                                step = chk_errno( errno_rc );
                                break;
                        default :
                                DA_SETRC_ERROR( DA_ERROR_OTHER );
                                step = EXIT_STEP;
                                break;
                        }
                        break;
                case READ_WRITE_TEST :           /* TU2 - Read/Write Test */
                        ++stepcount[READ_WRITE_TEST];
                        rc = tu_test( dt_fdes, step );
                        errno_rc = errno;                  /* get errno value */
                        switch( rc ) {
                        case BAD_DIAGS_GOOD :                         /* rc=0 */
                                step = SEEK_TEST;
                                break;
                        case BAD_DIAGS_FAIL :                         /* rc=1 */
                                step = create_frub( BADATU_2_FAIL );
                                break;
                        case BAD_ABORT_SENT:                          /* rc=2 */
                                if ( stepcount[READ_WRITE_TEST] > 1 ) {
                                        step = create_frub(BADATU_2_ABRT );
                                } else {
                                        step = DIAGNOSTIC_SELF_TEST;
                                }
                                break;
                        case BAD_AIX_ERR :                           /* rc=-1 */
                                step = chk_errno( errno_rc );
                                break;
                        default :
                                DA_SETRC_ERROR( DA_ERROR_OTHER );
                                step = EXIT_STEP;
                                break;
                        }
                        break;
                case SEEK_TEST :                           /* TU3 - Seek Test */
                        ++stepcount[SEEK_TEST];
                        rc = tu_test( dt_fdes, step );
                        errno_rc = errno;                  /* get errno value */
                        switch( rc ) {
                        case   BAD_DIAGS_GOOD :                       /* rc=0 */
                                step = READ_VERIFY_TEST;
                                break;
                        case BAD_DIAGS_FAIL :                         /* rc=1 */
                                step = create_frub( BADATU_3_FAIL );
                                break;
                        case   BAD_ABORT_SENT:                        /* rc=2 */
                                if ( stepcount[SEEK_TEST] > 1 ) {
                                        step = create_frub(BADATU_3_ABRT );
                                } else {
                                        step = DIAGNOSTIC_SELF_TEST;
                                }
                                break;
                        case BAD_AIX_ERR :                           /* rc=-1 */
                                step = chk_errno( errno_rc );
                                break;
                        default :
                                DA_SETRC_ERROR( DA_ERROR_OTHER );
                                step = EXIT_STEP;
                                break;
                        }
                        break;
                case READ_VERIFY_TEST :             /* TU4 - Read Verify Test */
                        ++stepcount[READ_VERIFY_TEST];
                        rc = tu_test( dt_fdes, step );
                        errno_rc = errno;                  /* get errno value */
                        switch( rc ) {
                        case   BAD_DIAGS_GOOD :                       /* rc=0 */
                                step = IOCINFO_TEST;
                                break;
                        case   BAD_DIAGS_FAIL :                       /* rc=1 */
                                step = create_frub( BADATU_4_FAIL );
                                break;
                        case   BAD_ABORT_SENT:                        /* rc=2 */
                                if ( stepcount[READ_VERIFY_TEST] > 1 ) {
                                        step = create_frub(BADATU_4_ABRT );
                                } else {
                                        step = DIAGNOSTIC_SELF_TEST;
                                }
                                break;
                        case BAD_AIX_ERR :                           /* rc=-1 */
                                step = chk_errno( errno_rc );
                                break;
                        default :
                                DA_SETRC_ERROR( DA_ERROR_OTHER );
                                step = EXIT_STEP;
                                break;
                        }
                        break;
                case IOCINFO_TEST :                   /* TU5 - IOCINFO Test */
                        ++stepcount[IOCINFO_TEST];
                        rc = tu_test( dt_fdes, step );
                        errno_rc = errno;                  /* get errno value */
                        switch( rc ) {
                        case BAD_DIAGS_GOOD :                         /* rc=0 */
                                step = CERTIFY_STEP;
                                break;
                        case BAD_DIAGS_FAIL :                         /* rc=1 */
                                step = create_frub( BADATU_5_FAIL );
                                break;
                        case BAD_AIX_ERR :                           /* rc=-1 */
                                step = chk_errno( errno_rc );
                                break;
                        default :
                                DA_SETRC_ERROR( DA_ERROR_OTHER );
                                step = EXIT_STEP;
                                break;
                        }
                        break;
		case CERTIFY_STEP :
			if((!tm_input.system) && (damode & DA_CONSOLE_TRUE) &&
		   		( damode & DA_LOOPMODE_NOTLM ||
		     			damode & DA_LOOPMODE_EXITLM )) {
				chk_asl_status(disp_menu(CERTIFY_PROMPT));
				if(menu_return != 2) {
					++stepcount[CERTIFY_STEP];
					chk_asl_status(disp_menu(CERTIFYING_DISK));
 					badisk_ioctl_params.milli_secs = 200000;
					badisk_ioctl_params.diag_test = BADIAG_READV;
					if( ioctl(dt_fdes,BADIAGTEST,&badisk_ioctl_params)
						== 0)
						certify_in_progress = 1;
					if(ioctl(dt_fdes, BAWAITCC, 
						&badisk_ioctl_params) != 0) {
						rc = ioctl(dt_fdes, BAABORT, &arg_cert);
						chk_asl_status(disp_menu(CERTIFY_ABORT));
					}
				    	rc = ioctl(dt_fdes, BACCSTAT, &arg_stat);	
					if((arg_stat.cmd_status == 0x01) ||
						(arg_stat.cmd_status = 0x03) ||
						(arg_stat.cmd_status = 0x05)) {
						rc = ioctl(dt_fdes, BAABORT, &arg_cert);
						chk_asl_status(disp_menu(CERTIFY_COMPLETED));
						certify_in_progress = 0;
					} else
						chk_asl_status(disp_menu(CERTIFY_ABORT));
				}
			}
			step = EXIT_STEP;
			break;
                default:
                        DA_SETRC_ERROR( DA_ERROR_OTHER );
                        step = EXIT_STEP;
                        break;
                } 
                chk_terminate_key();
        } /* endwhile */

}  /* end of function diag_tst */
/*  */
/*
 * NAME: tu_test()
 *
 * FUNCTION: tu_test sets some parameters to be used in the execution
 *           of each test unit. It then calls the exectu function to
 *           actually perform the test.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: rc - which is return code from the test unit that was run.
 *
 */

int tu_test( fdes, step )
int   fdes;
int   step;
{
        int    rc;                              /* functions return code      */
        struct bad_tucb_t *tucb_ptr;          /* kazusa command structure     */

/*
 * Allocate storage and initialize variables
 */
        tucb_ptr = MALLOC( struct bad_tucb_t );
        tucb_ptr->tucb.loop = 1;
        tucb_ptr->tucb.mfg = tucb_ptr->tucb.r1 = tucb_ptr->tucb.r2 = 0;
        switch ( step)
        {
                case 1 :                        /* TU0 - Diagnostic Self-Test */
                        tucb_ptr->tucb.tu = BADATU_0;
                        break;
                case 2 :           /* TU1 - Buffer Wrt/Rd Compare. Write 0x00 */
                        tucb_ptr->tucb.tu = BADATU_1;
                        tucb_ptr->ip1 = 0x00;
                        tucb_ptr->ip2 = 2;
                        break;
                case 3 :           /* TU1 - Buffer Wrt/Rd Compare. Write 0xff */
                        tucb_ptr->tucb.tu = BADATU_1;
                        tucb_ptr->ip1 = 0xff;
                        tucb_ptr->ip2 = 2;
                        break;
                case 4 :           /* TU1 - Buffer Wrt/Rd Compare. Write 0xaa */
                        tucb_ptr->tucb.tu = BADATU_1;
                        tucb_ptr->ip1 = 0xaa;
                        tucb_ptr->ip2 = 2;
                        break;
                case 5 :           /* TU1 - Buffer Wrt/Rd Compare. Write 0x55 */
                        tucb_ptr->tucb.tu = BADATU_1;
                        tucb_ptr->ip1 = 0x55;
                        tucb_ptr->ip2 = 2;
                        break;
                case 6 :                             /* TU2 - Read/Write Test */
                        tucb_ptr->tucb.tu = BADATU_2;
                        break;
                case 7 :                                   /* TU3 - Seek Test */
                        tucb_ptr->tucb.tu = BADATU_3;
                        break;
                case 8 :                            /* TU4 - Read Verify Test */
                        tucb_ptr->tucb.tu = BADATU_4;
                        break;
                case 9 :                                /* TU5 - IOCINFO Test */
                        tucb_ptr->tucb.tu = BADATU_5;
                        break;
                default:
                  ;
        }
	chk_terminate_key();
        /* execute the test unit now */
        rc = exectu( fdes, tucb_ptr );
	chk_terminate_key();
        return( rc );

}  /*  end of function tu_test */
/*  */
/*
 * NAME: create_frub()
 *
 * FUNCTION: Will be called if the testing of the device indicates a
 *           problem. It sets up the FRU buckets based on what test
 *           unit failed.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: Step - which is the next step in diag_tst that is to be done.
 *
 */

int create_frub( kazusa_err )
int    kazusa_err;
{
        int next_step = 0; 
	int conf_level;
	
        if( kazusa_err == BADATU_0_FAIL )
		conf_level = 2;
	else
	   if ( (kazusa_err == CMD_TIMEDOUT)
                   || (kazusa_err == DDCONFIG_ERR) || (kazusa_err == ELA_NOT_100))
		conf_level = 1;
	   else
		conf_level = 0;
        switch( kazusa_err ) {
        case BADATU_0_FAIL:                            
                frub[conf_level].rmsg = DHFD_TU0_1;   
		strcpy(frub[conf_level].frus[1].floc, tm_input.parentloc);
                break;
        case BADATU_0_ABRT:                         
                frub[conf_level].rmsg = DHFD_TU0_2;
                break;
        case BADATU_1_FAIL:
                frub[conf_level].rmsg = DHFD_TU1_3;
                break;
        case BADATU_1_ABRT:
                frub[conf_level].rmsg = DHFD_TU1_4;
                break;
        case BADATU_2_FAIL:
                frub[conf_level].rmsg = DHFD_TU2_5;
                break;
        case BADATU_2_ABRT:
                frub[conf_level].rmsg = DHFD_TU2_6;
                break;
        case BADATU_3_FAIL:
                frub[conf_level].rmsg = DHFD_TU3_7;
                break;
        case BADATU_3_ABRT:
                frub[conf_level].rmsg = DHFD_TU3_8;
                break;
        case BADATU_4_FAIL:
                frub[conf_level].rmsg = DHFD_TU4_9;
                break;
        case BADATU_4_ABRT:
                frub[conf_level].rmsg = DHFD_TU4_10;
                break;
        case BADATU_5_FAIL:
                frub[conf_level].rmsg = DHFD_TU5_11;
                break;
        case BUF_FAILED:
                frub[conf_level].rmsg = DHFD_TU1_12;
                break;
        case CMD_TIMEDOUT:
                frub[conf_level].rmsg = DHFD_MSG_13;
                break;
        case ERRNO_FAILURE:
                frub[conf_level].rmsg = DHFD_MSG_14;
                break;
        case DDCONFIG_ERR:
                frub[conf_level].rmsg = DHFD_MSG_15;
                break;
        case ELA_100:
        case ELA_NOT_100:
                frub[conf_level].rmsg = DHFD_MSG_16;
                break;
	case IPLCB_ERR1:
                frub[conf_level].rmsg = DHFD_MSG_17;
		break;
	case IPLCB_ERR2:
                frub[conf_level].rmsg = DHFD_MSG_18;
		break;
	case IPLCB_ERR3:
                frub[conf_level].rmsg = DHFD_MSG_19;
		break;
        default :
                DA_SETRC_ERROR( DA_ERROR_OTHER );
                break;
        }
        frub[conf_level].rcode = kazusa_err;        /* set reason code */
        if( insert_frub( &tm_input, &frub[conf_level] ) != 0 ) {
                DA_SETRC_ERROR( DA_ERROR_OTHER );
                clean_up();
        }
	frub[conf_level].sn = led_value;
        if( addfrub( &frub[conf_level] ) != 0 ) {
                DA_SETRC_ERROR( DA_ERROR_OTHER );
                clean_up();
        }
	if((conf_level != 0) && (kazusa_err != DDCONFIG_ERR))
                DA_SETRC_MORE( DA_MORE_CONT );
        /* set return code bad to display FRU bucket */
	problem_found = TRUE;
        DA_SETRC_STATUS( DA_STATUS_BAD );
        return( next_step );
} /* endfunction create_frub */
/*  */
/*
 * NAME: chk_errno()
 *
 * FUNCTION: Determines the AIX error returned by errno.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: next_step - which is the next step in diag_tst that is to be done.
 *
 */
chk_errno( errno_rc )
int     errno_rc;
{
        int  next_step;                                   /* next step to do  */

        switch( errno_rc ) {
        case EIO   :                                                    /* 05 */
                next_step = create_frub( ERRNO_FAILURE );
                break;
        case ENOTREADY:                                                 /* 46 */
                next_step = create_frub( CMD_TIMEDOUT );
                break;
        default :
                DA_SETRC_ERROR( DA_ERROR_OTHER );
                break;
        }
        return( next_step );
} /* endfunction chk_errno() */
/*  */
/*
 * NAME: chk_asl_status()
 *
 * FUNCTION: Checks the return code status of the asl commands. Based
 *           on the return code the function sets error codes to be
 *           returned back to the diagnostic controller. If the asl error
 *           is catastrophic the function will call the clean_up function
 *           which will terminate the module and return control to the
 *           diagnostic controller.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: None.
 *
 */

chk_asl_status( asl_rc )
long asl_rc;
{
        switch ( asl_rc ) {
        case DIAG_ASL_OK:
                break;
        case DIAG_MALLOCFAILED:
        case DIAG_ASL_ERR_NO_SUCH_TERM:
        case DIAG_ASL_ERR_NO_TERM:
        case DIAG_ASL_ERR_INITSCR:
        case DIAG_ASL_ERR_SCREEN_SIZE:
        case DIAG_ASL_FAIL:
                DA_SETRC_ERROR( DA_ERROR_OTHER );
                clean_up();
                break;
        case DIAG_ASL_CANCEL:
                DA_SETRC_USER( DA_USER_QUIT );
                clean_up();
                break;
        case DIAG_ASL_EXIT:
                DA_SETRC_USER( DA_USER_EXIT );
                clean_up();
                break;
        default:
                break;
        }
} /* endfunction chk_asl_status */
/*  */
/*
 * NAME: chk_terminate_key()
 *
 * FUNCTION: Checks whether a user has asked to terminate an application
 *           by hitting the Esc or Cancel key.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: None.
 *
 */

void chk_terminate_key()
{
        sleep(1);
	if(damode & DA_CONSOLE_TRUE)
	        chk_asl_status( diag_asl_read( ASL_DIAG_KEYS_ENTER_SC, NULL, NULL ));
} /* endfunction chk_terminate_key */
/*  */
#ifdef R2_ELA
/*
 * NAME: chk_ela()
 *
 * FUNCTION: Scans the error log for entries put in the log by the device
 *           driver. These log entries may indicate either a hardware or
 *           software problem with the direct bus attach fixed-disk.
 *
 * EXECUTION ENVIRONMENT: Called when the tm_input.dmode is set to ELA.
 *                        This function will also be called if the problem
 *                        determination proc indicates no trouble found.
 *
 * NOTES: At this time criteria for temporary errors have not been set, so
 *        these errors are ignored until thresholds are set by engineering.
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES: errdata
 *
 * RETURNS: None.
 */

void chk_ela()
{
	void     calc_sr_rate(), calc_hr_rate(), calc_se_rate(),
		 calc_sk_rate();
	uint	 data_read();
        struct   errdata err_data;                 /* error log data template */
        char     srch_crit[80];                    /* search criteria string  */
        int      op_flg;                    /* tells error_log_get what to do */
        int      i;                                       /* counter variable */
        int      rc = -99;                  /* return code from error_log_get */
        int      err_count[num_errs + 1];
	char	 hard_error_found = FALSE;
	int	 rbas[3];
	int	 total_rba_count=0;
	uint	 rba;
	char	 rba_inlist=FALSE;
        int      max_errs = 10;             /* max number temp errors allowed */
	int	 segment_count = 0;
	int	 first_error = 1;
	int	 initcnt = 0;
	int      current = 0;
	int      previous = 0;

        DA_SETRC_TESTS (DA_TEST_FULL);              /* set type tests to full */
        sprintf( srch_crit, srchstr, tm_input.date, tm_input.dname );
        for( i = 0; i <= num_errs; ++i )
                err_count[ i ] = 0;
        for( i = 0; i <= num_errs; ++i )
                rbas[ i ] = -1;

        op_flg = INIT;
        do {
                rc = error_log_get( op_flg, srch_crit, &err_data );
                if ( rc < 0 ) {
                        op_flg = TERMI;
                        rc = error_log_get( op_flg, srch_crit, &err_data );
                        clean_up();
                }
                if ( rc > 0 ) {
                        switch ( err_data.err_id) {
                        case ERRID_BADISK_ERR1: 
			case ERRID_BADISK_ERR3:
			case ERRID_BADISK_ERR8:
                                op_flg = SUBSEQ;
                                break;
                        case ERRID_BADISK_ERR2: /* Hard Read Error 3 is fatal */
				rba = ((int)(err_data.detail_data[16] < 24) +
					(int)(err_data.detail_data[17] < 16) +
					(int)(err_data.detail_data[18] < 8) +
					(int)(err_data.detail_data[19]) );
				for(i=0; i<=3; i++)
					if(rba == rbas[i]){
						rba_inlist = TRUE;
						break;
					}
				if(!rba_inlist){
	                                ++err_count[2];
					rbas[total_rba_count++] = rba;
       		                        op_flg = SUBSEQ;
				}else
					rba_inlist = FALSE;
				if(err_count[2]>3){
					rc = create_frub( ELA_100 );
					hard_error_found = TRUE;
                                	DA_SETRC_STATUS (DA_STATUS_BAD);
                                	op_flg = TERMI;
				}
                                break;
                        case ERRID_BADISK_ERR4:     /* Hard equipment check */
                                ++err_count[4];
				hard_error_found = TRUE;
                                rc = create_frub( ELA_100 );
                                DA_SETRC_STATUS (DA_STATUS_BAD);
                                op_flg = TERMI;
                                break;
                        case ERRID_BADISK_ERR5:         /* Attachment error */
                                ++err_count[5];
				hard_error_found = TRUE;
                                rc = create_frub( ELA_NOT_100 );
                                DA_SETRC_STATUS (DA_STATUS_BAD);
                                op_flg = TERMI;
                                break;
                        default:
                                op_flg = TERMI;
                                break;
                        }
                } else 
                        op_flg = TERMI;
        } while ( op_flg != TERMI ); /* enddo */
                                                   /* close out error_log_get */
        if( error_log_get( op_flg, srch_crit, &err_data ) < 0 )
                clean_up();
	if(! hard_error_found){
        /* set up new search criteria string */
        sprintf( srch_crit, " -N %s", tm_input.dname );
		op_flg = INIT;
        	do {
               		 rc = error_log_get( op_flg, srch_crit, &err_data );
                	 if ( rc < 0 ) {
                        	op_flg = TERMI;
                        	rc = error_log_get(op_flg,srch_crit,&err_data );
                        	clean_up();
                	 } 
                	if ( rc > 0 ) {
                       		 switch ( err_data.err_id) {
                        	 case ERRID_BADISK_ERR1: /* Soft read error */
                                	++err_count[1];
	                                op_flg = SUBSEQ;
					break;
                        	 case ERRID_BADISK_ERR3: /* Soft equip. check */
                                	++err_count[3];
	                                op_flg = SUBSEQ;
					break;
                        	case ERRID_BADISK_ERR8:  /* Seek error */
	                                ++err_count[6];
        	                        op_flg = SUBSEQ;
					break;
				default:
        	                        op_flg = SUBSEQ;
					break;
				}
				/* Now calculate segment count */
				if( (err_data.err_id == ERRID_BADISK_ERR1) ||
				    (err_data.err_id == ERRID_BADISK_ERR3) ||
				    (err_data.err_id == ERRID_BADISK_ERR8) ){

                                	if (first_error){
                                          initcnt = data_read(err_data);
					  previous = initcnt;
					  first_error = FALSE;
                                        } else {
	  			          current = data_read(err_data); 
					  if(current > previous){
					  /* Handle segment cnt reset by IPL */
					     if(initcnt == previous)
						  segment_count = segment_count
						       + initcnt;
					     else
					          segment_count = segment_count
						     + (initcnt - previous);
					     initcnt = current;
					  }
					  previous = current;
				       }
				}
                	} else {
				if(initcnt == current)
				     segment_count = segment_count + initcnt;
				else
				     segment_count = segment_count+
						(initcnt - current);
                	        if( err_count[1] > 0 )
                        	       calc_sr_rate(segment_count,err_count[1]);
                       		 if( err_count[3] > 0 )
                                       calc_se_rate(segment_count,err_count[3]);
                       		 if( err_count[6] > 0 )
                               	       calc_sk_rate(segment_count,err_count[6]);
				  if(problem_found)
                                        DA_SETRC_STATUS (DA_STATUS_BAD);
	                          op_flg = TERMI;
                                                   /* close out error_log_get */
			}
        	} while ( op_flg != TERMI ); /* enddo */
	        if( error_log_get( op_flg, srch_crit, &err_data ) < 0 )
               			 clean_up();
	}
} /* endfunction chk_ela */
/*  */
/*
 * NAME:data_read()
 *
 * FUNCTION:Calculates the amount of data that has been read from a DASD
 *          hard-disk. The segment count and byte count are contained in
 *          the detail_data field of the err_data structure.
 *          The function uses bytes 0-3 to calculate the segment count
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES: errdata
 *
 * RETURNS:Total byte count. 
 */

uint data_read(err_data)
struct       errdata err_data; 
{
        uint         seg_cnt;

        seg_cnt = ( ( uint ) ( err_data.detail_data[0] << 24 ) +
                  ( err_data.detail_data[1] << 16 ) +
                  ( err_data.detail_data[2] << 8 ) +
                  ( err_data.detail_data[3] ) );
        return( seg_cnt );
} /* endfunction data_read */
/*  */
/*
 * NAME:calc_sr_rate()
 *
 * FUNCTION: check to see if soft read errors exceed
 *	     the maximum allowable value.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES: errdata
 *
 * RETURNS: None.
 */

void calc_sr_rate( totcnt_sr, err_cnt )
int    totcnt_sr;
int	err_cnt;
{
        int             rc;
	float	threshold, error_rate;

	threshold = 1.0 / 12.5;
	error_rate = (float)err_cnt / (float)totcnt_sr;
	if((totcnt_sr > 125) || ((err_cnt > ELA_SAMPLE_SIZE) && (totcnt_sr > 10)))
		if(error_rate > threshold)
                	rc = create_frub( ELA_100 );
} /* endfunction calc_sr_rate */
/*  */
/*
 * NAME:calc_se_rate()
 *
 * FUNCTION: Check to see if soft equipment errors exceed
 *	     allowable value.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES: errdata
 *
 * RETURNS: None.
 */

void calc_se_rate(totcnt_se, err_cnt  )
int    totcnt_se;
int     err_cnt;
{
	int	rc;
	float	threshold, error_rate;

	threshold = 1.0 / 395.0;
	error_rate = (float)err_cnt / (float)totcnt_se;
        if((totcnt_se > 125) || ((err_cnt > ELA_SAMPLE_SIZE) && (totcnt_se > 10)))
        	if (error_rate > threshold)
                	rc = create_frub( ELA_100 );
} /* endfunction calc_se_rate */
/*  */
/*
 * NAME:calc_sk_rate()
 *
 * FUNCTION:Check to see if seek errors exceed alowable value.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES: errdata
 *
 * RETURNS: None.
 */

void calc_sk_rate( totcnt_sk, err_cnt  )
int    totcnt_sk;
int     err_cnt;
{
	int	rc;
	float	threshold, error_rate;

	threshold = 1.0 / 4000.0;
	error_rate = (float)err_cnt / (float)totcnt_sk;
	if((totcnt_sk > 125) || ((err_cnt > ELA_SAMPLE_SIZE) && (totcnt_sk > 10)))
	        if (error_rate > threshold)
	                rc = create_frub( ELA_100 );
} /* endfunction calc_sk_rate */
/*  */
#endif
/*
 * NAME: clean_up()
 *
 * FUNCTION: Clean_up is called whenever a fatal error occurs or after
 *           the diagnostic tests have completed. The purpose is to
 *           close files, quit ASL and provide a single point exit back
 *           to the diagnostic controller.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: None.
 */

void    clean_up( )
{
	int rc;

        term_dgodm();                                    /* terminate the ODM */

	if(certify_in_progress)
		rc = ioctl(fdes, BAABORT, &arg_cert);
        if ( fdes > 0 ){                             /* Close device, if open */
                close(fdes);
                if (i_lvmed_it == TRUE){
                        if ( restoredisk() == -1 )
                                DA_SETRC_ERROR( DA_ERROR_OTHER );
                }
        }
	if( init_config_state >= 0 ){
		rc = initial_state( init_config_state, tm_input.dname );
		if (rc == -1) 
			DA_SETRC_ERROR( DA_ERROR_OTHER );
	}
        if (catd != CATD_ERR)                          /* Close catalog files */
                catclose( catd );

       if( damode & DA_CONSOLE_TRUE ) 
        	diag_asl_quit( NULL );         /* Terminate the ASl stuff */
	if(devname != NULL)
		free( devname );
        DA_EXIT();                       /* Exit to the Diagnostic controller */
} /* endfunction clean_up */
