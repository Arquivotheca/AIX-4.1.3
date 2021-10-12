static char sccsid[] = "@(#)88	1.6.1.10  src/bos/diag/da/fpa/dfpp.c, dafpa, bos41J, 9512A_all 3/10/95 07:06:59";
/*
 * COMPONENT_NAME: DAFPA
 *
 * FUNCTIONS: check_return_code(), check_rc()
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 *
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <nl_types.h>
#include <limits.h>
#include <asl.h>
#include <locale.h>
#include <string.h>

#include "diag/da.h"		/* The following header files contain */
#include "diag/diag_exit.h"	/* information that are specific to this */
#include "diag/tmdefs.h"	/* application. */
#include "diag/tm_input.h"
#include "diag/diago.h"
#include "diag/dcda_msg.h"
#include "fpp_msg.h"

#include <sys/processor.h>
#include <odmi.h>
#include "diag/modid.h"
#include "diag/class_def.h"

/* Define for machine type values */

/* It's necessary to report these two defines below in fpa_s.h */
/* for fpa modules write in assembler language */
/* It's not possible to have common define between assembler and c code */
/* because their syntax is different */

#define POWER_MP_A_MODEL_CODE	0x01
#define OTHER_MODEL_CODE	0

#define       CPU_E1M 1
#define       CPU_E1D 2
#define       CPU_C1D 3
#define       CPU_C4D 4

struct fru_bucket frub[] = {
  
    { "", FRUB1, 0, 0, 0,        {
                                 { 0, "INVALID ERROR", "", 0, 0, 0 },
                                 },
    }, 
    { "", FRUB1, 0x815, 0x100, R_MSG_01, {
                                 { 100, "", "", 0, DA_NAME, EXEMPT },
                                 },
    },
};
  
extern          getdainput();		/* Information from diagnostic ctrl */
struct 		tm_input tm_input;	/* Controller's info. in tm_input */
extern  nl_catd diag_catopen(char *, int);
nl_catd         fdes;			/* File descriptor for catalog file */

/* global variable readable in asm modules */
unsigned int	pegas;		/* Machine type */

#define DEBUG 0
main()
{
    int	i;		/* Return code from a test case */
    int	rc;		/* User's response */
    int	model;		/* Determine if machine is a desk top or not */

int	CuAt_value;
int	num_cudv;	/* physical number processor */
cpu_t	numlog;		/* logical number processor */
pid_t   pid;
int cr;
struct CuDv *cudv;
struct listinfo cudv_info;
char criteria[256];

   setlocale(LC_ALL,"");
/* Initialize default return status (a safety measure) */
    DA_SETRC_STATUS(DA_STATUS_GOOD);
    DA_SETRC_USER(DA_USER_NOKEY);
    DA_SETRC_ERROR(DA_ERROR_NONE);
    DA_SETRC_TESTS(DA_TEST_SHR);
    DA_SETRC_MORE(DA_MORE_NOCONT);

    if (init_dgodm() != 0) {
        term_dgodm();
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        DA_EXIT();
    }
    rc = getdainput(&tm_input);
    if (rc != 0) {
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        DA_EXIT();
    }
    fdes = diag_catopen(MF_FPP,0);

	/* Get machine model */
	model=get_cpu_model(&CuAt_value);

	/* Test for PowerPC_SMP model */
	if( IsPowerPC_SMP(model) ) {	/* Bind processor */
		pegas = POWER_MP_A_MODEL_CODE;

		/* Get logical processor number */
		/* The physical processor number is the number of the */
		/* CuDv object (proc0, proc1 ...) */
		/* It's extacted from the TMInput dname field */

		/* Check if the last byte is a number */
		if((isdigit(tm_input.dname[strlen(tm_input.dname) - 1])) == 0) {
			DA_SETRC_ERROR(DA_ERROR_OTHER);
			DA_EXIT();
		}

		/* Convert the last byte to an integer,it's the physical number */
		num_cudv = atoi(&tm_input.dname[strlen(tm_input.dname) - 1]);

		/* Convert num_cudv to logical processor number */
		numlog = -1;
		for( i = 0 ; i <= num_cudv ; i++) {
			sprintf(criteria,"name=proc%x",i);
			cudv = odm_get_list(CuDv_CLASS,criteria,&cudv_info,3,1);

			if(cudv !=(struct CuDv *) -1 && cudv != (struct CuDv *)0 ) {
				/* There is a CuDv for this physical processor*/
				if(cudv->chgstatus != MISSING &&
						cudv->status == AVAILABLE) {
					numlog++; /* processor available */
				}
				else {
					if(num_cudv == i) {
					/* processor to test is not available */
						DA_SETRC_ERROR(DA_ERROR_OTHER);
						DA_EXIT();
					}
				}
			}
		}

		/* Check for processor board type */
		rc = get_cpucard_vpd(tm_input.parent);

		/* Initialize fru_bucket structure */
		switch(rc) {
		case CPU_E1M:
			frub[1].rcode = 0x101;
			break;
		case CPU_C1D:
			frub[1].rcode = 0x102;
			break;
		case CPU_E1D:
			frub[1].sn = 0x810;
			frub[1].rcode = 0x501;
			break;
		case CPU_C4D:
			frub[1].sn = 0x810;
			frub[1].rcode = 0x502;
			break;
		default :
			DA_SETRC_ERROR(DA_ERROR_OTHER);
			DA_EXIT();
		}
			
		/* Get the PID of this process */
		pid = getpid();

		/* Bind processor */
		if((cr = bindprocessor(BINDPROCESS,pid,numlog)) < NULL ) {
			DA_SETRC_ERROR(DA_ERROR_OTHER);
			DA_EXIT();
		}
	}
	else {
		pegas = OTHER_MODEL_CODE;
	}	/* End of if( IsPowerPC_SMP ) */

/* Display "stand by" screen and do analysis */

    if (tm_input.console == CONSOLE_TRUE) {
        diag_asl_init(ASL_INIT_DEFAULT);
        switch (tm_input.advanced) {
        case ADVANCED_TRUE:
            if (tm_input.loopmode != LOOPMODE_NOTLM)
                rc = diag_msg_nw(0x815004,fdes,ALOOP,ALTITLE,tm_input.lcount,
                                 tm_input.lerrors);
            else
                rc = diag_msg_nw(0x815003, fdes, ADVANCED, ATITLE);
            break;
        case ADVANCED_FALSE:
            if (tm_input.loopmode != LOOPMODE_NOTLM)
                rc = diag_msg_nw(0x815002, fdes, LOOP, LTITLE, tm_input.lcount,
                                 tm_input.lerrors);
            else
                rc = diag_msg_nw(0x815001, fdes, REGULAR, RTITLE);
            break;
        default:
            break;
        }
        sleep(1);
	rc = diag_asl_read(ASL_DIAG_KEYS_ENTER_SC, FALSE, NULL);
        check_rc(rc);
    }
    model = 1;

/* Start executing tests */
    rc = 99;        /* initializing rc to non-zero */

    i = fpfail2();
    rc = check_return_code(model, i, rc);
    i = fpfail8();
    rc = check_return_code(model, i, rc);
    i = chkreg();
    rc = check_return_code(model, i, rc);
    i = startup();
    rc = check_return_code(model, i, rc);
    if (nota604()) {
        i = decfull();
        rc = check_return_code(model, i, rc);
    }
    i = expfull();
    rc = check_return_code(model, i, rc);
    i = lzafull();
    rc = check_return_code(model, i, rc);
    i = cmpp();
    rc = check_return_code(model, i, rc);
    i = div_fpp();	/* Don't mix up with standard div function */
    rc = check_return_code(model, i, rc);
    i = fbd();
    rc = check_return_code(model, i, rc);
    i = mlfs();
    rc = check_return_code(model, i, rc);
    i = mlfsx();
    rc = check_return_code(model, i, rc);
    i = mlfdx();
    rc = check_return_code(model, i, rc);
    i = mlfsu();
    rc = check_return_code(model, i, rc);
    i = mlfsux();
    rc = check_return_code(model, i, rc);
    i = mlfdu();
    rc = check_return_code(model, i, rc);
    i = mlfdux();
    rc = check_return_code(model, i, rc);
    i = mstfs();
    rc = check_return_code(model, i, rc);
    i = mstfsx();
    rc = check_return_code(model, i, rc);
    i = mstfdx();
    rc = check_return_code(model, i, rc);
    i = mstfsu();
    rc = check_return_code(model, i, rc);
    i = mstfsux();
    rc = check_return_code(model, i, rc);
    i = mstfdu();
    rc = check_return_code(model, i, rc);
    i = mstfdux();
    rc = check_return_code(model, i, rc);
    if (nota604()) {
        i = all();
        rc = check_return_code(model, i, rc);
    }
    i = chip();
    rc = check_return_code(model, i, rc);
    i = fa();
    rc = check_return_code(model, i, rc);
    i = fap();
    rc = check_return_code(model, i, rc);
    i = fdd();
    rc = check_return_code(model, i, rc);
    i = fdpp();
    rc = check_return_code(model, i, rc);
    i = fm();
    rc = check_return_code(model, i, rc);
    i = fm2();
    rc = check_return_code(model, i, rc);
    i = fma();
    rc = check_return_code(model, i, rc);
    i = fmap();
    rc = check_return_code(model, i, rc);
    i = fmp();
    rc = check_return_code(model, i, rc);
    i = frsp();
    rc = check_return_code(model, i, rc);
    i = frspp();
    rc = check_return_code(model, i, rc);
    if (nota604()) {
        i = mtfsf();
        rc = check_return_code(model, i, rc);
        i = sys();
        rc = check_return_code(model, i, rc);
    }
    if (rs2()) {
        i = sqrt();
        rc = check_return_code(model, i, rc);
        i = sqrtp();
        rc = check_return_code(model, i, rc);
    }

    term_dgodm();
    if (tm_input.console == CONSOLE_TRUE)
        diag_asl_quit();

    /* All tests passed, return to diagnostic controller with GOOD status */
    DA_SETRC_TESTS(DA_TEST_SHR);
    DA_EXIT();

    return (i);
}   /* End Main */

/*
 * NAME: check_return_code
 *                                                                    
 * FUNCTION: Design to check for return code after each test case is
 *           executed. If i != 0, then a fru bucket will be called out, and
 *           this routine will exit and return back to the controller.
 *           Otherwise, this routine would check if the QUIT or EXIT key is
 *           entered; if none is entered, routine will return to caller.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *    This routine is called by the main program. This routine in turn
 *      calls another routine check_rc().
 *                                                                   
 * RETURNS: This returns the user's input to the screen, rc.
 */


int 
check_return_code(model, i, rc)
    int	model;	/* 1 means machine is non-desk top */
    int	i;	/* return code from a test case, 0=good */
    int	rc;	/* user's input, could be Esc or Cancel key */
{
    short save_sn;

    check_rc(rc);
    if (i != 0) {
	save_sn = frub[1].sn;
   	if (insert_frub(&tm_input, &frub[1]) != 0) {
	    /* diag_asl_msg("insert_frub failed\n"); */
            DA_SETRC_ERROR(DA_ERROR_OTHER);
        } else {
		frub[1].sn = save_sn;	/* Source number restitution */
		strncpy(frub[1].dname,tm_input.dname,sizeof(frub[1].dname));

		/* Put location-code in error message */
		if ( pegas == POWER_MP_A_MODEL_CODE ) {
			strncpy(frub[1].frus[0].floc,tm_input.dnameloc,
					sizeof(frub[1].frus[0].floc));
		}
		addfrub(&frub[model]);
		DA_SETRC_STATUS(DA_STATUS_BAD);
	}
        term_dgodm();
        if (tm_input.console == CONSOLE_TRUE)
            diag_asl_quit();
        DA_EXIT();
    }
    if (tm_input.console == CONSOLE_TRUE)
        rc = diag_asl_read(ASL_DIAG_KEYS_ENTER_SC, FALSE, NULL);
    return (rc);
}


/*
 * NAME: check_rc
 *                                                                    
 * FUNCTION: This routine checks if the user has entered the Esc or Cancel key.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *    This routine is called by check_return_code.
 *                                                                   
 * RETURNS: This routine returns the input parameter to the caller, rc.
 */

int 
check_rc(rc)
    int		rc;	/* user's response */
{
    if (rc == ASL_CANCEL) {
        term_dgodm();
        if (tm_input.console == CONSOLE_TRUE)
            diag_asl_quit();
        DA_SETRC_USER(DA_USER_QUIT);
        DA_EXIT();
    }
    if (rc == ASL_EXIT) {
        term_dgodm();
        if (tm_input.console == CONSOLE_TRUE)
            diag_asl_quit();
        DA_SETRC_USER(DA_USER_EXIT);
        DA_EXIT();
    }
    return (rc);
}

/*
 * NAME: rs2
 *                                                                    
 * FUNCTION: This routine checks if the machine is an RS2 machine
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *    This routine is called by main
 *                                                                   
 * RETURNS: 
 *        1 : if RS2
 *        0 : if not RS2
 */
int
rs2()
{
int     cuat_mod;
int     ipl_mod;
#if DEBUG
int     x;
char    *tbuf;
#endif

ipl_mod = get_cpu_model(&cuat_mod);
#if DEBUG
tbuf = (char *) malloc(1024);
sprintf(tbuf,"ipl_mod = %x\n",ipl_mod);
diag_asl_msg(tbuf);
sprintf(tbuf,"cuat_mod = %x\n",cuat_mod);
diag_asl_msg(tbuf);
free (tbuf);
#endif

/* 70 and 80 - RS2 machines */

if ((ipl_mod == 0x4000070) || (ipl_mod == 0x4020080))
    return(1);
else
    return(0);
}


/*
 * NAME: nota604
 *                                                                    
 * FUNCTION: This routine checks if the machine is a 604 or 620 machine
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *    This routine is called by main
 *                                                                   
 * RETURNS: 
 *        1 : if machine is not a 604 nor 620
 *        0 : machine is a 604 or 620
 */
int
nota604()
{
int     cuat_mod;
int     ipl_mod;
#if DEBUG
int     x;
char    *tbuf;
#endif

ipl_mod = get_cpu_model(&cuat_mod);
#if DEBUG
tbuf = (char *) malloc(1024);
sprintf(tbuf,"ipl_mod = %x\n",ipl_mod);
diag_asl_msg(tbuf);
sprintf(tbuf,"cuat_mod = %x\n",cuat_mod);
diag_asl_msg(tbuf);
free (tbuf);
#endif

ipl_mod &= 0x000000ff;

/* model  	id

   Rainbow 4+	91
   Rainbow 6	90
   604 Peg	A1
   620 Peg	A2
   604 Pan	A4
   620 Pan	A5
   604 Fireb	A7
   620 Fireb	A8 */

if ((ipl_mod == 0x90) || (ipl_mod == 0x91) ||
    (ipl_mod == 0xA1) || (ipl_mod == 0xA2) ||
    (ipl_mod == 0xA4) || (ipl_mod == 0xA5) ||
    (ipl_mod == 0xA7) || (ipl_mod == 0xA8))
    return(0);
else
    return(1);
}

/*
 * NAME: get_cpucard_vpd
 *                                                                    
 * FUNCTION:  Search in VPD for cpucard type
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS:	cpucard type
		0 if error
 */

int get_cpucard_vpd(char *cuvpd_name) {

char	criteria[256];
struct  listinfo	vpd_info;
struct	CuVPD	*vpd;
char	*pt;
int	cpu;

	cpu = 0;
	sprintf(criteria,"name=%s",cuvpd_name);
	vpd = odm_get_list( CuVPD_CLASS,criteria,&vpd_info,1,2);
	if( vpd != (struct CuVPD *) -1 && vpd_info.num >= 1) {
		if((pt=strstr(vpd->vpd,"*FN")) != 0) {
			pt+=4;
			if     (strncmp(pt,"E1M",3) == 0) cpu = CPU_E1M;
			else if(strncmp(pt,"E1D",3) == 0) cpu = CPU_E1D;
			else if(strncmp(pt,"C1D",3) == 0) cpu = CPU_C1D;
			else if(strncmp(pt,"C4D",3) == 0) cpu = CPU_C4D;
		}
	}
	odm_free_list(vpd,&vpd_info);
	return(cpu);
}

