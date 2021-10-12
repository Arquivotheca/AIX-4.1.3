static char sccsid[] = "@(#)90	1.20.2.8  src/bos/diag/da/scsi/dscsia.c, dascsi, bos411, 9428A410j 5/10/94 11:32:54";
/*
 *   COMPONENT_NAME: DASCSI
 *
 *   FUNCTIONS: check_vpd
 *		chk_asl_status
 *		chk_ela
 *		chk_terminate_key
 *		clean_up
 *		combine
 *		create_frub
 *		diag_tst
 *		display_title
 *		gen_crc
 *		gen_menugoal
 *		get_dev_type
 *		main
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1992
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#define CHK_ELA 
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <nl_types.h>
#include <limits.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/buf.h>
#include <sys/errids.h>
#include <sys/devinfo.h>
#include <sys/device.h>
#include <sys/scsi.h>
#include <sys/cfgodm.h>
#include <sys/sysconfig.h>
#include <sys/stat.h>
#include <string.h>

#include <diag/tm_input.h>                                 /* faking the ODM  */
#include <diag/tmdefs.h>                   /* diagnostic modes and variables  */
#include <diag/da.h>                       /* FRU Bucket structure definition */
#include <diag/diag.h>                       /* ela function structs and vars */
#include <diag/diag_exit.h>                                /* DA return codes */
#include <diag/diago.h>
#include <diag/dcda_msg.h>                    /* catalog of standard messages */
#include <diag/bit_def.h>
#include <diag/modid.h>
#include "dscsia_msg.h"
#include "dscsia.h"
#include <locale.h>

/******************************************************************************/
/*                                                                            */
/*    da_fru structure holds related information to the FRU bucket            */
/*    variables in this structure are explained in DA CAS (SJDIAG)            */
/*    under the function name ADDFRU.                                         */
/*                                                                            */
/******************************************************************************/

#define	MAX_FRU_ENTRY	21
#define	INTEGRATED_SCSI	0x868
struct fru_bucket frub[]=
{                                          /* adapter diagnostic test failure */
        { "", FRUB1, 0x869 , 0x110  , DSCSIA_MSG_1 ,  {
                { 80, "", "", 0, DA_NAME, NONEXEMPT },
                { 20, "", "", 0, PARENT_NAME, EXEMPT },
                                  },
        },
                                                      /* adapter fuse failure */
        { "", FRUB1, 0x869 , 0x130 , DSCSIA_MSG_2 ,  {
                { 100, "Fuse/PTC", "", FUSE_PTC_OPEN, 0, EXEMPT },
                                  },
        },
                                                         /* wrap test failure */
        { "", FRUB1, 0x869 , 0x140 , DSCSIA_MSG_3 ,  {
                { 100, "", "", 0, DA_NAME, NONEXEMPT },
                                  },
        },
                                            /* wrt/read/cmpr BCR regs failure */
        { "", FRUB1, 0x869 , 0x150 , DSCSIA_MSG_4 ,  {
                { 100, "", "", 0, DA_NAME, NONEXEMPT },
                                  },
        },
                                            /* wrt/read/cmpr POS regs failure */
        { "", FRUB1, 0x869 , 0x160 , DSCSIA_MSG_5 ,  {
                { 100, "", "", 0, DA_NAME, NONEXEMPT },
                                  },
        },
                                           /* internal-external reset failure */
        { "", FRUB1, 0x869 , 0x170 , DSCSIA_MSG_6 ,  {
                { 100, "", "", 0, DA_NAME, NONEXEMPT },
                                  },
        },
                                                   /* adapter command timeout */
        { "", FRUB1, 0x869 , 0x180 , DSCSIA_MSG_7 ,  {
                { 55, "Fuse/PTC", "", FUSE_PTC_OPEN, 0, EXEMPT },
                { 45, "", "", 0, DA_NAME, NONEXEMPT    },
                                  },
        },
                                                          /* DD/Config error  */
        { "", FRUB1, 0x869 , 0x190 , DSCSIA_MSG_8 ,  {
                { 60, "", "", 0, DA_NAME, NONEXEMPT },
                { 40, "", "", 0, PARENT_NAME, EXEMPT },
                                   },
        },
                                                        /* error log analysis */
        { "", FRUB1, 0x869 , 0x191 , DSCSIA_MSG_9 ,  {
                { 100, "", "", 0, DA_NAME, NONEXEMPT },
                                   },
        },
                                                        /* error log analysis */
        { "", FRUB1, 0x869 , 0x192 , DSCSIA_MSG_9 ,  {
                { 80, "", "", 0, DA_NAME, NONEXEMPT },
                { 20, "", "", 0, PARENT_NAME, EXEMPT },
                                   },
        },
                                                        /* error log analysis */
        { "", FRUB1, 0x869 , 0x193 , DSCSIA_MSG_9 ,  {
                { 100, "Fuse/PTC", "", FUSE_PTC_OPEN, 0, EXEMPT },
                                   },
        },
                                                    /* ROM CRC error          */
        { "", FRUB1, 0x869 , 0x200 , DSCSIA_MSG_10 ,  {
                { 100, "", "", 0, DA_NAME, NONEXEMPT },
                                  },
        },
                                                      /* Adapter RAM error    */
        { "", FRUB1, 0x869 , 0x201 , DSCSIA_MSG_11 ,  {
                { 100, "", "", 0, DA_NAME, NONEXEMPT },
                                  },
        },
                                                   /* SCORPION chip RAM error */
        { "", FRUB1, 0x869 , 0x202 , DSCSIA_MSG_12 ,  {
                { 100, "", "", 0, DA_NAME, NONEXEMPT },
                                  },
        },
                                                 /* SCORPION chip logic error */
        { "", FRUB1, 0x869 , 0x203 , DSCSIA_MSG_12 ,  {
                { 100, "", "", 0, DA_NAME, NONEXEMPT },
                                  },
        },
                                                 /* SCSI cntrl chip POR error */
        { "", FRUB1, 0x869 , 0x204 , DSCSIA_MSG_12 ,  {
                { 100, "", "", 0, DA_NAME, NONEXEMPT },
                                  },
        },
                                                 /* SCSI cntrl chip Reg error */
        { "", FRUB1, 0x869 , 0x205 , DSCSIA_MSG_12 ,  {
                { 100, "", "", 0, DA_NAME, NONEXEMPT },
                                  },
        },
                                                /* Diag complete--prev error */
        { "", FRUB1, 0x869 , 0x206 , DSCSIA_MSG_13 ,  {
                { 100, "", "", 0, DA_NAME, NONEXEMPT },
                                  },
        },
                                                /* Data Loss problem        */
        { "", FRUB1, 0x869 , 0x194 , DSCSIA_MSG_9 ,  {
                { 100, "", "", 0, DA_NAME, NONEXEMPT },
                                  },
        },
        { "", FRUB1, 0 , 0x131 , DSCSIA_MSG_15 ,  {
                { 100, "Recoverable Fuse", "", DSCSIA_MSG_15, DA_NAME, NONEXEMPT },
                                  },
        },
        { "", FRUB1, 0 , 0x211 , DSCSIA_MSG_16 ,  {
                { 50, "Fuse/PTC", "", FUSE_PTC_OPEN, 0, EXEMPT },
                { 45, "", "", 0, DA_NAME, NONEXEMPT    },
                { 5, "Software", "", SOFTWARE_ERROR, 0, NONEXEMPT    },
                                  },
        },
        { "", FRUB1, 0 , 0x211 , DSCSIA_MSG_16 ,  {
                { 95, "", "", 0, DA_NAME, NONEXEMPT    },
                { 5, "Software", "", SOFTWARE_ERROR, 0, EXEMPT    },
                                  },
        },
};

struct tm_input tm_input;                /* DA test parameters and other info */
extern getdainput();
extern long getdamode();
extern disp_menu();
extern error_log_get();
static  int	  init_config_state = -1;
static	char  adapter_timed_out = FALSE;
int     problem_found = FALSE;
int	pscsi = 0;
int	ptc_flag=0;
char    *devname;                            /* device name used in openx cmd */
char    dname[] = "/dev/";
nl_catd catd;                                  /* pointer to the catalog file */
extern nl_catd diag_catopen(char *, int);
#if	DEBUG
FILE	*fdebug;
#endif

/*
 * NAME: main()
 *
 * FUNCTION: Main routine for the SCSI Adapter Diagnostic Application.
 *           It is invoked by the diagnostic Controller.
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

	int 	 check_vpd();
	void	 chk_terminate_key();
	void	 chk_asl_status();
	void	 chk_ela();
	void	 clean_up();
	int 	 create_frub();
	void	 display_title();
	void	 diag_tst();
	register int	i;
	int	 fd;
        int    	 debug_errno;                       /* used only for debugger */
        int    	 rc;                                  /* generic return code */


	setlocale( LC_ALL, "" );

	#if	DEBUG
	fdebug=fopen("/tmp/dscsia.debug","a+");
	#endif
        DA_SETRC_TESTS(DA_TEST_FULL);
        damode = (long)NULL;
        fdes = -99;
	failing_function_code = 0x869;
        /*
          open and initialize dgodm
        */
        if ( init_dgodm() == bad_da_error ) {
                DA_SETRC_ERROR( DA_ERROR_OTHER );
                clean_up();
        }
        /*
          get the input parameters and check return
        */
        if( getdainput( &tm_input ) != FALSE ) {
                DA_SETRC_ERROR( DA_ERROR_OTHER );
                clean_up();
        } else {
		for(i=0;i<=MAX_FRU_ENTRY;i++)
			strcpy(frub[i].dname, tm_input.dname);
                devname = ( char * ) malloc( NAMESIZE + ( strlen( dname )+1 ));
                if ( devname != NULL )
                        strcpy( devname, dname );
                strcat( devname, tm_input.dname );
        }
        /*
          get the bit encoded version of the stuff in tm_input 
          then test that we got it.            
        */
        damode = getdamode( &tm_input );
        if( damode == (long)NULL ) {
                DA_SETRC_ERROR( DA_ERROR_OTHER );
                clean_up();
        } 
        /*
          start ASL if console is present and check return
        */
        if( damode & DA_CONSOLE_TRUE ) {
                chk_asl_status( diag_asl_init( "NO_TYPE_AHEAD" ) );
                catd = diag_catopen(MF_DSCSIA, 0 ); 
        } 
	/* Retrieve led value */

	failing_function_code = get_dev_type();

        /*
          get set to do tests user wants to execute
        */
        if( damode & DA_DMODE_ELA ) {
#ifdef CHK_ELA
                chk_ela();
#endif
        } else if( damode & DA_DMODE_PD || damode & DA_DMODE_REPAIR
                                || damode & DA_DMODE_FREELANCE ) {
                if( !( damode & DA_LOOPMODE_EXITLM ) ) {
			display_title();
			init_config_state = configure_device(tm_input.dname);
			if( init_config_state == -1 ){
				if( !( damode & DA_DMODE_REPAIR )){
#ifdef CHK_ELA
					chk_ela();
#endif
					if( problem_found )
						clean_up();
				}
 			/*
				ELA did not detect problem or damode prevents
				ELA to be run.
			*/
				/* Configuring the scsi failed, report fuse */
				/* and card failure.			    */
				if (ptc_flag)
					create_frub(CONFIG_FAILED_PTC);
				else
					create_frub(CONFIG_FAILED);
				clean_up();
			}
                        fdes = openx( devname, O_RDWR, NULL,
                                                   SC_DIAGNOSTIC );
                        debug_errno = errno;
                        /*
                          If device opened OK, run all the tests 
                        */
                        if (fdes < 0 ) {
                                if (( damode & DA_EXENV_CONC )
                                             && ( debug_errno == EACCES )) {
				        if( !( damode & DA_DMODE_REPAIR ))
			                	chk_ela();
					if( (problem_found == FALSE)
						 && (debug_errno == EACCES) )
						DA_SETRC_ERROR( DA_ERROR_OPEN );
					clean_up();
                                } else {
                                        DA_SETRC_ERROR( DA_ERROR_OPEN );
                                        clean_up();
				}
                        } else {
                                chk_terminate_key();
                                diag_tst( fdes );
                                chk_terminate_key();
                        } /* endif fdes < 0 */
                } /* endif */
                /*
                  if pd mode, and notlm or enterlm, do ela last
                */
                if( damode & DA_DMODE_PD  && ( damode & DA_LOOPMODE_NOTLM ||
                    damode & DA_LOOPMODE_EXITLM ) &&
                    ( ( DA_CHECKRC_STATUS() == DA_STATUS_GOOD ) ||
                    ( DA_CHECKRC_ERROR() == DA_ERROR_OPEN ) ) ) {
#ifdef CHK_ELA
                        chk_ela();
#endif
                }
        } 
        clean_up();                    /* exit call to controller...no return */
}  /* main end */
/*   */
/*
 * NAME: display_title()
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
 * RETURNS: none.
 */

void
display_title()
{
	char	*sub[3];
	long	menu;

	menu = failing_function_code * 0x1000;
	if(damode & DA_CONSOLE_TRUE) {
		sub[0] = diag_cat_gets(catd, DSCSIA_SET1, SCSI_ADAPTER);
		sub[1]=tm_input.dname;
		sub[2]=tm_input.dnameloc;
		/* Show start test menu */

	        if((damode & DA_LOOPMODE_NOTLM) && (damode & DA_ADVANCED_FALSE))
		{
			menu = menu + 1;
	                chk_asl_status( diag_display_menu(CUSTOMER_TESTING_MENU,
				menu, sub, 0, 0));
		}
		/* Show start test menu - entering loop mode */
	        if((damode & DA_ADVANCED_TRUE) && (damode & DA_LOOPMODE_NOTLM))
		{
			menu = menu + 2;
			chk_asl_status( diag_display_menu(ADVANCED_TESTING_MENU,
				menu, sub, 0, 0));
		}
		/* show status screen for loop mode */
        	if((damode & DA_ADVANCED_TRUE) && ((damode &
			DA_LOOPMODE_ENTERLM) || (damode & DA_LOOPMODE_INLM)))
		{ 
			menu = menu + 3;
	                chk_asl_status( diag_display_menu(LOOPMODE_TESTING_MENU,
				menu, sub, tm_input.lcount, tm_input.lerrors) );
		}
	}
} /* end display_title */
/*  */
/*
 * NAME: diag_tst()
 *
 * FUNCTION: Test the SCSI adapter as outlined in the BlueBonnet   -
 *           Preliminary Functional Specification. The function proceeds
 *           step by step calling it's respective ioctl cmd. It examines
 *           the return code from the cmd and makes a decision on which
 *           step to do next. If an error occurres it sets the return
 *           code for the controller. If a problem is found during any
 *           of the steps, it calls the addfru function to set up the
 *           FRU bucket to be passed back to the controller.
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

#define NUM_STEPS 8                       /* number of diagnostic steps to do */
#define EXIT_STEP		0
#define VPD_TEST_STEP	        1
#define CARD_DIAGNOSTICS_STEP	2
#define WRAP_TEST_STEP		3
#define W_R_C_BCR_REGS_STEP	4
#define POS_REGS_TEST_STEP	5
#define SCSI_BUS_RESET_STEP	6
#define CHECK_XTRA_STATUS_STEP	7

void
diag_tst( fdes )
int fdes;
{
        struct sc_card_diag sccrd;
        int  rc;
        int  stepcount[NUM_STEPS+1]; 
        char   report_frub = FALSE; 
        register int  step;
	char	odm_search_crit[80];
        struct CuDv             *cudv;        /* ODM Customized device struct */
        struct listinfo         obj_info;
        uchar  adapt_rc;
        uchar  x_status;

	#if	DEBUG
	int	i;
	#endif

        /*
          clear the accumulative step counters
        */
        for( step = 0; step <= NUM_STEPS; ++step )
                stepcount[ step ] = 0;
        rc = 0;
	if(failing_function_code == INTEGRATED_SCSI)
		step = CARD_DIAGNOSTICS_STEP;
	else
	        step = VPD_TEST_STEP;                                  
        sccrd.timeout_value = ADAP_TIMEOUT;
	sccrd.rsv1 = 0;
	sccrd.rsv2 = 0;
	sccrd.rsv3 = 0;
	sccrd.rsv4 = 0;

        while( step )
        {                                      
                switch( step ) {

		case VPD_TEST_STEP :
			#if	DEBUG
			fprintf(fdebug,"%d: VPD_TEST_STEP\n",__LINE__);
			fflush(fdebug);
			#endif
			++stepcount[VPD_TEST_STEP];
			rc = check_vpd(fdes);
			switch( rc ) {
			case 0:
			   step = CARD_DIAGNOSTICS_STEP;
			   break;
			case -2:
			   frub[VPDTEST_FAIL].rmsg = DSCSIA_MSG_14;
			   step = create_frub( VPDTEST_FAIL );
			   break;		   
                        default :
                           step = EXIT_STEP;
                           DA_SETRC_ERROR(DA_ERROR_OTHER);
                           break;
                        }
			break;
                case CARD_DIAGNOSTICS_STEP :
			#if	DEBUG
			fprintf(fdebug,"%d: CARD_DIAGNOSTICS_STEP\n",__LINE__);
			fflush(fdebug);
			#endif
                        ++stepcount[CARD_DIAGNOSTICS_STEP];
                        sccrd.option = SC_CARD_DIAGNOSTICS;
                        rc = ioctl( fdes, SCIODIAG, &sccrd );
                        errno_rc = errno;
                        switch( rc ) {
                        case   0:
                            step = POS_REGS_TEST_STEP;
                            break;
                        case  -1:
			    if(!pscsi){
                        	    adapt_rc = sccrd.diag_rc.un.card1.mb31_status[4];
	                            x_status = sccrd.diag_rc.un.card1.mb31_status[5];
			    } else {
				    adapt_rc = 0x0;
				    x_status = sccrd.diag_rc.ahs;
			    }
                            switch( errno_rc ) {
                            case ENXIO    :                 /* errno = 06 */
                                step = POS_REGS_TEST_STEP;
                                break;
                            case ETIMEDOUT:                 /* errno = 78 */
				if((adapt_rc == 0x04) && !pscsi)
					step = create_frub( FUSE_FAILURE );
				else
                                	step = create_frub( CRD_TIMEOUT );
                                break;
                            case EFAULT:                    /* errno = 14 */
                                switch( adapt_rc ) {
                                case 0x04:
                                    step = create_frub( FUSE_FAILURE );
                                    break;
                                case 0x05 :
                                case 0x0c :
                                case 0x10 :
                                case 0x1f :
                                case 0x00 :
                                    step = CHECK_XTRA_STATUS_STEP;
                                    break;
                                default :
                                    step = EXIT_STEP;
                                    DA_SETRC_ERROR(DA_ERROR_OTHER);
                                    break;
                                }
                                break;
                            case EIO:                       /* errno = 05 */

				if(adapt_rc==0x4 && x_status ==0)
				{
					step = create_frub(FUSE_FAILURE);
					break;
				}
                                if(stepcount[CARD_DIAGNOSTICS_STEP]==1)
                                    step = CHECK_XTRA_STATUS_STEP;
                                else 
                                    step = EXIT_STEP;
                                break;
                            default :
                                step = EXIT_STEP;
                                DA_SETRC_ERROR(DA_ERROR_OTHER);
                                break;
                            }
                            break;
                        default :
                                step = EXIT_STEP;
                                DA_SETRC_ERROR(DA_ERROR_OTHER);
                                break;
                        }
                        break;
                case WRAP_TEST_STEP:
			#if	DEBUG
			fprintf(fdebug,"%d: WRAP_TEST_STEP\n",__LINE__);
			fflush(fdebug);
			#endif
                  	++stepcount[WRAP_TEST_STEP];
                       	sccrd.option = SC_CARD_SCSI_WRAP;
                       	rc = ioctl( fdes, SCIODIAG, &sccrd );
                       	errno_rc = errno;
                       	switch( rc ) {
                       	case   0:
                               	step = EXIT_STEP;
                               	break;
		 	case  -1:
                                step = create_frub (CRD_WRAP_TST);
       	                        break;
			default:
				step = EXIT_STEP;
                               	DA_SETRC_ERROR(DA_ERROR_OTHER);
				break;
			}
			break;
                case W_R_C_BCR_REGS_STEP:
			#if	DEBUG
			fprintf(fdebug,"%d: W_R_C_BCR_REGS_STEP\n",__LINE__);
			fflush(fdebug);
			#endif
                        ++stepcount[W_R_C_BCR_REGS_STEP];
                        sccrd.option = SC_CARD_REGS_TEST;
                        rc = ioctl( fdes, SCIODIAG, &sccrd );
                        errno_rc = errno;
                        switch( rc ) {
                        case 0:
                                if ( damode & DA_ADVANCED_TRUE ) 
                                        step = SCSI_BUS_RESET_STEP;
                                else 
                                        step = EXIT_STEP;
                                break;
                        case  -1:
                                switch( errno_rc ) {
                                case       ENXIO:               /* errno = 06 */
                                        step = CARD_DIAGNOSTICS_STEP;
                                        break;
                                case ETIMEDOUT :                /* errno = 78 */
                                        step = create_frub( CRD_TIMEOUT );
                                        break;
                                case  EFAULT :                  /* errno = 14 */
                                case  EIO    :                  /* errno = 05 */
                                        step = create_frub( BCR_REGS_TST );
                                        break;
                                default :
                                        step = EXIT_STEP;
                                        DA_SETRC_ERROR(DA_ERROR_OTHER);
                                        break;
                                }  
                                break;
                        default :
                                step = EXIT_STEP;
                                DA_SETRC_ERROR(DA_ERROR_OTHER);
                                break;
                        } 
                        break;
                case POS_REGS_TEST_STEP:
			#if	DEBUG
			fprintf(fdebug,"%d: POS_REGS_TEST_STEP\n",__LINE__);
			fflush(fdebug);
			#endif
                        ++stepcount[POS_REGS_TEST_STEP];
                        sccrd.option = SC_CARD_POS_TEST;
                        rc = ioctl( fdes, SCIODIAG, &sccrd );
                        errno_rc = errno;
                        switch( rc ) {
                        case   0:
                                step = W_R_C_BCR_REGS_STEP;
                                break;
                        case  -1:
                                switch( errno_rc ) {
                                case      ENXIO:                /* errno = 06 */
                                        step = W_R_C_BCR_REGS_STEP;
                                        break;
                                case ETIMEDOUT:                 /* errno = 78 */
                                        step = create_frub( CRD_TIMEOUT );
                                        break;
                                case EFAULT:                    /* errno = 14 */
                                case EIO   :                    /* errno = 05 */
                                        step = create_frub(POS_REGS_TEST_STEP);
                                        break;
                                default :
                                        step = EXIT_STEP;
                                        DA_SETRC_ERROR(DA_ERROR_OTHER);
                                        break;
                                } 
                                break;
                        default :
                                step = EXIT_STEP;
                                DA_SETRC_ERROR(DA_ERROR_OTHER);
                                break;
                        } 
                        break;
                case SCSI_BUS_RESET_STEP:
			#if	DEBUG
			fprintf(fdebug,"%d: SCSI_BU_RESET_STEP\n",__LINE__);
			fflush(fdebug);
			#endif
                        ++stepcount[SCSI_BUS_RESET_STEP];
                        sccrd.option = SC_SCSI_BUS_RESET;
                        rc = ioctl( fdes, SCIODIAG, &sccrd );
                        errno_rc = errno;
                        switch( rc ) {
                        case   0:
				if(!pscsi)
	                                step = EXIT_STEP;
				else{
				/* Wrap test use id 7 for initiator and */
				/* id 0 for the target. Therefore, it   */
				/* should be skip to prevent devices    */
				/* with id 0 from responding.	        */
				        sprintf( odm_search_crit,
					   "parent = %s AND connwhere like '0,*'",
					   tm_input.dname );
				        cudv = get_CuDv_list( CuDv_CLASS,
					   odm_search_crit, &obj_info, 1, 2 );
					/* If a scsi device exists with id 0 */
					/* skip the wrap test		     */
        				if ( (cudv == ( struct CuDv * ) -1 ) ||
				             (cudv == ( struct CuDv * ) NULL ) )

					    step = WRAP_TEST_STEP;
					else
					    step = EXIT_STEP;
				}
                                break;
                        case  -1:
			    	if(!pscsi)
	                              adapt_rc =
                                         sccrd.diag_rc.un.card1.mb31_status[4];
				else
				      adapt_rc = 0x00;
                                switch( errno_rc ) {
                                case ETIMEDOUT:                 /* errno = 78 */
                                        step = create_frub( CRD_TIMEOUT );
                                        break;
                                case EIO:                       /* errno = 05 */
                                       	step = create_frub( BUS_RESET );
                                        break;
                                default:
                                        step = EXIT_STEP;
                                        DA_SETRC_ERROR(DA_ERROR_OTHER);
                                        break;
                                } 
                                break;
                        default :
                                step = EXIT_STEP;
                                DA_SETRC_ERROR(DA_ERROR_OTHER);
                                break;
                        }
                        break;
                case CHECK_XTRA_STATUS_STEP: 
			#if	DEBUG
			fprintf(fdebug,"%d: CHECK_XTRA_STATUS_STEP\n",__LINE__);
			fflush(fdebug);
			#endif
                        ++stepcount[CHECK_XTRA_STATUS_STEP];
                        /*
                          check for valid extra status then build frubucket
		        */
                        if ( x_status != 0x00 ) {
                                switch ( x_status ) {
                                case 0x80:
                                        step = create_frub( ROM_CRC_ERR );
                                        break;
                                case 0x81:
                                        step = create_frub( ADAP_RAM_ERR );
                                        break;
                                case 0x82:
                                        step = create_frub( SCRP_RAM_ERR );
                                        break;
                                case 0x83:
                                        step = create_frub( SCRP_LOGIC_ERR );
                                        break;
                                case 0x88:
                                        step = create_frub( SCSI_POR_ERR );
                                        break;
				case PIO_WR_OTHR_ERR:
				case PIO_RD_OTHR_ERR:
                                case 0x89:
                                        step = create_frub( SCSI_REG_ERR );
                                        break;
                                case 0x8f:
                                        step = create_frub( PREV_ERR );
                                        break;
                                default:
                                        step = EXIT_STEP;
                                        DA_SETRC_ERROR(DA_ERROR_OTHER);
                                        break;
                                }
                        }
                        break;
                default :
                        step = EXIT_STEP;
                        DA_SETRC_ERROR(DA_ERROR_OTHER);
                        break;
                } 
	#if	DEBUG
		fprintf(fdebug,"step = %d, x_status = %d, ioctl ret code %d\n",
			step,x_status,rc);
		fprintf(fdebug,"mb31 = %d %d, errno = %d\n",adapt_rc,
							x_status,errno_rc);
		fprintf(fdebug,"pscsi = %d\n",pscsi);
			fprintf(fdebug,"stepcount[0..8] = ");
		for (i = 0;i<=NUM_STEPS;i++)
			fprintf(fdebug,"%d ",stepcount[i]);
		fprintf(fdebug,"\n");
		fflush(fdebug);
	#endif
		
	if (cudv)
		odm_free_list(cudv, &obj_info);
        }   /* endwhile */
}   /* end function diag_tst */
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
 * RETURNS: 0 which is the last step to execute in diag_tst.
 *
 */

int
create_frub( adap_err )
int    adap_err;
{
	char	srn[10];
	int	mgoal=0;
	if(! adapter_timed_out)
		if( adap_err == CRD_TIMEOUT ){
			adapter_timed_out = TRUE;
			if(!(damode & DA_DMODE_REPAIR))
			     chk_ela();
			if(problem_found)
				return( 0 );
		}

	frub[adap_err].sn = failing_function_code;
        /*
          put fuse location in frub and use proper FFC based on card type.
        */
	#if	DEBUG
	fprintf(fdebug,"%d: adap_err = %d\n", __LINE__,adap_err);
	fflush(fdebug);
	#endif
        if ( adap_err == FUSE_FAILURE || adap_err == ERROR_LOG3 ||
		adap_err == ERROR_LOG2|| (adap_err==21 &&
		/* special case for scsi 2 adapters	*/
		(failing_function_code==0x889 || failing_function_code==0x866)))
	{
		#if DEBUG
		fprintf(fdebug,"report menu goal\n");
		fflush(fdebug);
		#endif
		if(ptc_flag)
		{
			adap_err=19;
			frub[adap_err].sn = failing_function_code;
			sprintf(srn,"%x-%x",frub[adap_err].sn,
				frub[adap_err].rcode);
			#if	DEBUG
			fprintf(fdebug,"%d: adap_err reassigned to %d\n",
				__LINE__,adap_err);
			fprintf(fdebug,"%d: PTC Menu Goal with srn: %s\n",
				__LINE__,srn);
			fflush(fdebug);
			#endif
			mgoal=TRUE;
			if(failing_function_code==0x889)
				gen_menugoal(SCSI2_DE_MENU_GOAL,srn);
			else
				gen_menugoal(SCSI2_SE_MENU_GOAL,srn);

		}
		else
		{
			gen_menugoal(SCSI1_FUSE_MENU_GOAL,NULL);
			mgoal=TRUE;
		}
	}


	/* change the fuse to ptc only for scsi2 adapters	*/
	if((failing_function_code == 0x889 || failing_function_code == 0x866)
	    && (frub[adap_err].rcode == 0x191 || frub[adap_err].rcode==0x211))
	{
		frub[adap_err].frus[0].conf=80;
		strcpy(frub[adap_err].frus[0].fname,"SCSI Bus");
		frub[adap_err].frus[0].fmsg=SCSI_BUS_ERROR;
		frub[adap_err].frus[0].fru_flag=NOT_IN_DB;
		frub[adap_err].frus[0].fru_exempt=EXEMPT;

		frub[adap_err].frus[1].conf=20;
		strcpy(frub[adap_err].frus[1].fname,"");
		frub[adap_err].frus[1].fmsg=0;
		frub[adap_err].frus[1].fru_flag=DA_NAME;
		frub[adap_err].frus[1].fru_exempt=0;
		if(!mgoal)
		{
			sprintf(srn,"%x-%x",frub[adap_err].sn,
				frub[adap_err].rcode);
			if(failing_function_code==0x889)
				gen_menugoal(SCSI2_DE_MENU_GOAL,srn);
			else
				gen_menugoal(SCSI2_SE_MENU_GOAL,srn);

		}
	}

	if( !mgoal && (frub[adap_err].rcode == 0x191 ||
		frub[adap_err].rcode==0x211 || frub[adap_err].rcode==0x130 ||
		frub[adap_err].rcode==0x193 || frub[adap_err].rcode==0x180) )
	{
		mgoal=TRUE;
		if(ptc_flag)
		{
			sprintf(srn,"%x-%x",frub[adap_err].sn,
				frub[adap_err].rcode);

			if(failing_function_code == 0x889 )
				/* Report SCSI-2 DE ptc menugoal	*/
				gen_menugoal(SCSI2_DE_MENU_GOAL,srn);
			else if (failing_function_code == 0x866 )
				/* Report SCSI-2 SE ptc menugoal	*/
				gen_menugoal(SCSI2_SE_MENU_GOAL,srn);
			else
				/* Report SCSI-1 ptc menugoal	*/
				gen_menugoal(SCSI1_SE_MENU_GOAL,srn);

		}
		else
			gen_menugoal(SCSI1_FUSE_MENU_GOAL,NULL);
		
	}
        /*
          insert, then add frub for DC
        */

        if( insert_frub( &tm_input, &frub[adap_err] ) != 0 ) {
                DA_SETRC_ERROR( DA_ERROR_OTHER );
                clean_up();
        }
	if(ptc_flag)
	{
		/* reassign the failure source number */
		frub[adap_err].sn = failing_function_code;
	}

	#if	DEBUG
	fprintf(fdebug,"%d: frub element are :\n",__LINE__);
	fprintf(fdebug,"	dname=%s, ftype=%d, sn=%x, rcode=%x rmsg=%d\n",
		frub[adap_err].dname,frub[adap_err].ftype,frub[adap_err].sn,
		frub[adap_err].rcode,frub[adap_err].rmsg);
	fflush(fdebug);
	#endif

        if( addfrub( &frub[adap_err] ) != 0 ) {
                DA_SETRC_ERROR( DA_ERROR_OTHER );
                clean_up();
        }
        DA_SETRC_STATUS( DA_STATUS_BAD );
        problem_found = TRUE;
        return( 0 ); /* 0 indicates the next step to take upon return */
} /* endfunction create_frub */
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

void
chk_asl_status( asl_rc )
long asl_rc;
{
        switch ( asl_rc ) {
        case DIAG_ASL_OK:
	case DIAG_ASL_COMMIT:
	case DIAG_ASL_ENTER:
	case DIAG_ASL_YES:
	case DIAG_ASL_NO:
                break;
        case DIAG_ASL_CANCEL:
                DA_SETRC_USER(DA_USER_QUIT);
                clean_up();
                break;
        case DIAG_ASL_EXIT:
                DA_SETRC_USER(DA_USER_EXIT);
                clean_up();
                break;
        default:
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                clean_up();
                break;
        }
} /* endchk_asl_status */
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

void
chk_terminate_key()
{
	if(damode & DA_CONSOLE_TRUE)
        	chk_asl_status(diag_asl_read( ASL_DIAG_KEYS_ENTER_SC, NULL,NULL ));
} /* endfunction chk_terminate_key */

#ifdef CHK_ELA
/*  */
/*
 * NAME: chk_ela()
 *
 * FUNCTION: Scans the error log for entries put in the log by the device
 *           driver. These log entries may indicate either a hardware or
 *           software problem with the adapter.
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

void
chk_ela()
{
        struct errdata err_data;                   /* error log data template */
        char    srch_crit[80];                     /* search criteria string  */
        int     op_flg;                     /* tells error_log_get what to do */
        int     rc = -99;                   /* return code from error_log_get */
        int     num_errs = 0;           /* number of temp errors found in log */
        int     max_errs = 10;              /* max number temp errors allowed */
	char	hardware_error_found = FALSE;
	int	adapter_rc=0;

        /*
          set up the search criteria string
        */
        sprintf( srch_crit, srchstr, tm_input.date, tm_input.dname );
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
                        case ERRID_SCSI_ERR1:   /* Perm Adapter HW error */
			case ERRID_SCSI_ERR2:   /* Temp Adapter HW error */
				if(!pscsi)
				    adapter_rc=(int)(err_data.detail_data[88]);
                                /*
				  test if adapter fuse if blown
			        */
                                if( adapter_rc == 0x04 ){
			/*
			   Reset flag so that any other Permanent 
			   error SCSI_ERR1 or SCSI_ERR2 cannot be logged.
			*/
					hardware_error_found = FALSE;
                                        rc = create_frub( ERROR_LOG3 );
					DA_SETRC_STATUS (DA_STATUS_BAD);
					op_flg = TERMI;
				} else {
				        if(err_data.err_id == ERRID_SCSI_ERR1)
		  				hardware_error_found = TRUE;
					else
						++num_errs;
					op_flg = SUBSEQ;
				}
                                break;
                        case ERRID_SCSI_ERR3:/* Perm unknown adapter SW error */
                        case ERRID_SCSI_ERR5:/* Perm unknown driver error */
				if(! hardware_error_found){
	                                rc = create_frub( ERROR_LOG2 );
       		                        DA_SETRC_STATUS (DA_STATUS_BAD);
                                	op_flg = TERMI;
				}
                                break;
/* Comment on ADAP_ERR4 and ADAP_ERR6 - These errors can be
 * considered temporary, non-fatal errors. There has been no criteria set as
 * to how many of these can occur before it can be considered an error which
 * could cause a FRU to be called out. Until these criteria are set or further
 * clarification shows that these errors can be considered fatal these errors
 * will be ignored. A code segment is included that can be used if a criteria
 * is set in the future. However the code is commented out.
 */
                        case ERRID_SCSI_ERR4:/* Temp unknown adapter SW error */
                        case ERRID_SCSI_ERR6:/* Temp unknown driver error */
                                op_flg = SUBSEQ;
                                break;
                        case ERRID_SCSI_ERR7: /* Perm unknown sys error */
                                op_flg = SUBSEQ;
                                break;
			case ERRID_SCSI_ERR9: /* Potential Data loss problem */
	                        rc = create_frub( DATA_LOSS );
       		                DA_SETRC_STATUS (DA_STATUS_BAD);
                               	op_flg = TERMI;
				break;
                        default:
                                op_flg = SUBSEQ;
                                break;
                        }
                } else {
                        op_flg = TERMI;
                }
        } while ( op_flg != TERMI ); 
	/*
	   Make sure frub is called out only if we get in here
	   when no error was detected by running diagnostics
	   test. 
	*/
	if(((hardware_error_found) && (! adapter_timed_out)) ||
		(num_errs >=  max_errs)) {
		rc = create_frub( ERROR_LOG1 );
		DA_SETRC_STATUS (DA_STATUS_BAD);
	}

        /*
          close out error_log_get
        */
        if( error_log_get( op_flg, srch_crit, &err_data ) < 0 )
                clean_up();
} /* endfunction chk_ela */
#endif
/*  */
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

void
clean_up( )
{
	int rc;

        term_dgodm();                                    /* terminate the ODM */

        if ( fdes > 0 )                              /* Close device, if open */
                close(fdes);

	if ( init_config_state >= 0 ){
	        rc = initial_state(init_config_state, tm_input.dname);
		if (rc == -1)
			DA_SETRC_ERROR( DA_ERROR_OTHER );
	}
        if (catd != CATD_ERR)                          /* Close catalog files */
                catclose( catd );

        if( damode & DA_CONSOLE_TRUE )
                diag_asl_quit( NULL );             /* Terminate the ASl stuff */
	#if	DEBUG
	fflush(fdebug);
	fclose(fdebug);
	#endif

        DA_EXIT();                       /* Exit to the Diagnostic controller */
} /* endfunction clean_up */
/*  */
/*
 * NAME: check_vpd
 *
 * FUNCTION: Check for valid VPD.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 *   0 - valid VPD.
 *  -1 - unable to get VPD.
 *  -2 - invalid VPD.
 */

int
check_vpd(fdes)
int	fdes;
{
	static  char buf[256];
	static	struct cfg_dd cfg_dd = {
		0,
		0,
		CFG_QVPD,
		buf,
		sizeof(buf),
	};
	struct  stat	scsi_buf;
	char	*vpd_data;
	unsigned short length;
	unsigned short expected_crc;
	unsigned short crc;
	unsigned short gen_crc();

	if(fstat(fdes, &scsi_buf) != -1)
		cfg_dd.devno = scsi_buf.st_rdev;
	else
		return( -1);
	if(sysconfig(SYS_CFGDD, &cfg_dd, sizeof(cfg_dd)) == CONF_FAIL) 
		return(-1);
	expected_crc = (buf[6-1]<<8) + buf[7-1];
	length = ((buf[4-1]<<8) + buf[5-1]) * 2;
	vpd_data = &buf[8-1]; 
	crc = gen_crc(vpd_data, (int)length);
	if(crc != expected_crc)
		return(-2);
	else
		return(0);
}
/*  */
/*
 * NAME: combine
 *
 * FUNCTION: Combine to bytes into 16 bits
 *
 * EXECUTION ENVIRONMENT:
 *  User level task.
 *
 * (NOTES:)
 *
 *   Purpose - This function can be used to swap the two bytes of a two byte
 *             value, or to concatenate two separate bytes into a two byte
 *             value.  The bytes are passed in the order they are to be
 *             combined in.
 *
 *   Inputs - byte1:  most significant byte
 *            byte2:  least significant byte
 *
 * RETURNS:
 *   16 bit value made up of the passed bytes.
 */

unsigned short combine (byte1, byte2)
   unsigned char byte1, 	/* msb */
		 byte2;		/* lsb */
   {                                               /* start of combine() */
	static unsigned short ret_value;

	ret_value = (byte1 << 8) | byte2;
	return(ret_value);
   }                                               /* end of combine() */
/*  */
/*
 * NAME: gen_crc
 *
 * FUNCTION: Function that does actual computation of CRC.
 *
 * EXECUTION ENVIRONMENT:
 *  User level task.
 *
 * (NOTES:)
 * Function calculates CRC.  Based on modified algorithms by
 * Jim Darrah and listing from PC/RT Hardware Technical Reference Manual,
 * Volume I.
 *
 * RETURNS:
 *  16 bit CRC
 */

#define CRC_MASK	0XFF07

unsigned short
gen_crc (buf, len)
   unsigned char *buf;
   int len;
   {
	union accum
	   {
		unsigned short whole;	/* defines entire 16 bits */
		struct bytes
		   {			/* used to define 2 bytes */
			unsigned char msb;
			unsigned char lsb;
		   } byte;
	   } avalue, dvalue;

	static unsigned short ret_crc;	/* value to be returned */
	unsigned char datav;
	int i;

	dvalue.whole = 0xffff;

/**************************************************************************
  Operate on each byte of data in the passed array, sending the data through
  the algorithm to generate a crc.  Then use the crc just generated as the
  base for the next pass of the data through the algorithm.
***************************************************************************/

	for (i = 0; len > 0; i++, len--)
	   {				/* step through the CRC area */
		datav = *(buf + i);	/* GET BYTE */
		avalue.byte.lsb = datav ^ dvalue.byte.lsb;
		dvalue.byte.lsb = avalue.byte.lsb;
		avalue.whole = (avalue.whole * 16) ^ dvalue.byte.lsb;
		dvalue.byte.lsb = avalue.byte.lsb;
		avalue.whole <<= 8;

		avalue.whole >>= 1;
		avalue.byte.lsb ^= dvalue.byte.lsb;

		avalue.whole >>=4;

		avalue.whole = combine(avalue.byte.lsb, avalue.byte.msb);
		avalue.whole = (avalue.whole & CRC_MASK) ^ dvalue.byte.lsb;
		avalue.whole = combine(avalue.byte.lsb, avalue.byte.msb);
		avalue.byte.lsb ^= dvalue.byte.msb;
		dvalue.whole = avalue.whole;
	   }				/* end step through the CRC area */
	ret_crc = dvalue.whole;
	return(ret_crc);
   }					/* end of crc_chk */
/*  */
/*
 * NAME: get_dev_type()
 *
 * FUNCTION: Looks at the led value found in the PdDv structure in the odm
 *           data base. The function searches the Customized Devices Data
 *           base for the name passed in tm_input.dname. It then uses the
 *           link found in the PdDvLn entry to go to the Predefined Devices
 *           data base to retreive the value found in the led entry.
 *           This is neccessary to set up the test parameters for each
 *           fixed-disk type that may be tested and to create the correct
 *           fru bucket in the event a test indicates a drive failure.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: This function returns the led value if found or and error
 *          code if the data was not found.
 */

int get_dev_type()

{
        char                    odm_search_crit[40];	/* odm search criteria */
	char			*dg;
	char			dg_field[3];
        int                     dev_led_rc;	/* return valu to calling func */
        struct CuDv             *cudv;		/* ODM Customized device struct */
        struct CuVPD             *cuvpd;	/* ODM Customized VPD struct */
        struct listinfo         obj_info;

        sprintf( odm_search_crit, "name = %s", tm_input.dname );
        cudv = get_CuDv_list( CuDv_CLASS, odm_search_crit, &obj_info, 1, 2 );
        if ( ( cudv == ( struct CuDv * ) -1 ) ||
             ( cudv == ( struct CuDv * ) NULL ) )
                dev_led_rc = bad_da_error;
        else
	{
                dev_led_rc = (int)cudv->PdDvLn->led;
		if(!strcmp(cudv->PdDvLn->uniquetype,"adapter/sio/pscsi") ||
		   !strcmp(cudv->PdDvLn->uniquetype,"adapter/sio/8fba") )
			pscsi=1;
		if(dev_led_rc == 0x869)
		{
			#if DEBUG
			fprintf(fdebug,"%d:odm_search_crit size= %d\n",
				__LINE__,sizeof(odm_search_crit));
			fflush(fdebug);
			#endif

			(void)memset(odm_search_crit,0,sizeof(odm_search_crit));
        		sprintf( odm_search_crit, "name = %s", tm_input.dname );
        		cuvpd = get_CuVPD_list( CuVPD_CLASS, odm_search_crit,
					&obj_info, 1, 2 );
        		if ( (cuvpd == ( struct CuVPD * ) -1 ) ||
				  (cuvpd == ( struct CuVPD * ) NULL ) )

			{

                		DA_SETRC_ERROR( DA_ERROR_OTHER );
                		clean_up();
        		}


			dg=strstr(cuvpd->vpd,"DG");
			#if DEBUG
			fprintf(fdebug,"%d: DG = %s\n",__LINE__,dg);
			fflush(fdebug);
			#endif
			sprintf(dg_field,"%c%c",dg[3],dg[4]);
			#if	DEBUG
			fprintf(fdebug,"%d: atoi(dg_field) = %d dg_field = %s\n"
				,__LINE__,atoi(dg_field),dg_field);
			fflush(fdebug);
			#endif
			switch(atoi(dg_field))
			{
				case 1:
					ptc_flag=TRUE; 		/* BB W/PTC */
					break;
				case 2:
					dev_led_rc=0x868; 	/* IB W/PTC */
					ptc_flag=TRUE;
					break;
					
				case 3:
					dev_led_rc=0x866;	/* SE */
					ptc_flag=TRUE;
					break;

				case 4:
					dev_led_rc=0x889;	/* DE */
					ptc_flag=TRUE;
					break;
				default:
					break;
			}

		
		}
	}

	if (cudv)
		odm_free_list(cudv, &obj_info);

        return( dev_led_rc  );
} /* endfunction get_dev_type */

/*
 * NAME: gen_menugoal
 *
 * FUNCTION: generate a menu goal for ptc or fuse.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	Environment must be a diagnostics environment which is described 
 *	in Diagnostics subsystem CAS.
 *
 * RETURNS:  
 */
gen_menugoal(fuse_type, srn)
int	fuse_type;
char	*srn;
{
	char	*ptcgoal;
	char	*tmp;

	if (tm_input.console==CONSOLE_TRUE)
	{
		ptcgoal = diag_cat_gets(catd, DSCSIA_SET1, fuse_type);
		tmp = calloc(strlen(ptcgoal)+strlen(tm_input.dname)+
			strlen(tm_input.dnameloc)+20,sizeof(char));
		sprintf(tmp,ptcgoal,(failing_function_code*0x1000)+8,
			tm_input.dnameloc,srn);
		menugoal(tmp);
	}
}
