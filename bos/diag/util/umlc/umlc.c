static char sccsid[] = "@(#)45  1.21.2.9  src/bos/diag/util/umlc/umlc.c, dsaumlc, bos411, 9428A410j 3/14/94 17:18:54";
/*
 * COMPONENT_NAME: (DUTIL) DIAGNOSTIC UTILITY - umlc.c
 *
 * FUNCTIONS:   main(argc,argv)
 *              int_handler(signal)
 *              genexit(exitcode)
 *              usage_exit(exitcode)
 *              disp_and_sleep()
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
#include <signal.h>
#include <nl_types.h>
#include <sys/cfgdb.h>
#include <locale.h>
#include "umlc_msg.h"
#include "mlc_defs.h"
#include "diag/class_def.h"
#include "diag/diago.h"
#include "diag/diag.h"
#include "diag/tmdefs.h"


/* GLOBAL VARIABLES     */
extern  nl_catd fdes;
extern  char    *keywrds[];
extern  char    *lvl_kwds[];
extern  VPD_REC vprec;
extern  struct  PN_PTR  *pn_ptr;
extern  struct  FB_PTR  *fb_ptr;
extern  struct  FC_PTR  *fc_ptr;
extern  MACH_REC machine;
extern  int     fccnt;
extern  int     fbcnt;
extern  int     pncnt;
extern  int     vpcnt;
extern  int     diskette;
extern  int     install_yes;
extern  int     db_loaded;
extern  int     output;
extern  int     fdd_gone;
extern  int     fdd_not_supported;
extern  char    mfg_sno[];
extern  char    mfg_typ[];
extern  char    diagdir[];
extern  int     no_menus;
extern  char    outputfile[];
extern  int     cstop_found;
extern  int     output_file_open;



/* EXTERNAL VARIABLES   */
extern ASL_SCR_TYPE dm_menutype;

/* CALLED FUNCTIONS */
unsigned int get_cpu_model(int *);

/* EXTERNAL FUNCTIONS   */
extern char *getenv();
extern nl_catd diag_catopen(char *, int);

/*  ANSI C FUNCTION PROTOTYPING FOLLOWS */
int main(int, char *[]);
void int_handler(int);
void genexit(int);
void usage_exit(int);
void disp_and_sleep(void);

/*  */
/*
 * NAME: main
 *
 * FUNCTION: This unit controls the mainline process of Product Topology.
 *
 * RETURNS: NONE
 */

main(int argc, char *argv[])
{
int             status = -1;
char            *pchr;
int             model_code;
unsigned int    mach_model;
struct sigaction act;
static struct msglist menulist2[] = {
        { END_SET, END_TITLE },
        { END_SET, END_TEXT },
        { END_SET, END_LAST },
        { (int )NULL, (int )NULL }
};

struct CuDv     *cudv;
struct listinfo c_info;
char            criteria[128];
char            tempoutput[256];
char            *tempstr;
int             j,k,m,n;
int             bad_parms=FALSE;
int             help_flag=FALSE;
int             do_mfg_bld=FALSE;

        setlocale(LC_ALL,"");

        /* set up interrupt handler     */
        act.sa_handler = int_handler;
        sigaction(SIGINT, &act, (struct sigaction *)NULL);

        init_dgodm(); /* initialize ODM         */
        ipl_mode(&diskette);

        if(argc > 1) { /* If options passed in on the command line */
            for(j=1;((j<argc) && (bad_parms==FALSE));j++) {
                if ((argv[j][0] == '-') && (strlen(argv[j]) > 1)) {
                    for (k=1;((k<(strlen(argv[j]))) && (bad_parms==FALSE));k++) {
                        switch(argv[j][k]) {
                            case 'r':  /* manufacturing rebuild function */
                                if (((do_mfg_bld==TRUE) ||
                                  (k<(strlen(argv[j])-1))) || ((argc-j) < 3)) {
                                    do_mfg_bld=TRUE;
                                    bad_parms=TRUE;
                                }
                                else {
                                    strncpy(mfg_sno, argv[j+1], FIELD_SIZE-7);
                                    strcat(mfg_sno,"\n");
                                    strncpy(mfg_typ, argv[j+2], FIELD_SIZE-7);
                                    strcat(mfg_typ,"\n");
                                    do_mfg_bld=TRUE;
                                    j=j+2;
                                    k=strlen(argv[j]); /*Force k to get next j*/
                                } /* endelse */
                                break;
                            case 'e':  /* do not display any menus */
                                if(no_menus == TRUE)
                                    bad_parms=TRUE;
                                else
                                    no_menus=TRUE;
                                break;
                            case 'f':  /* Send output to a file */
                                if((strncmp(outputfile,"\0",1)!=0) ||
                                 ((k==(strlen(argv[j])-1)) && ((argc-j)<2)))
                                    bad_parms=TRUE;
                                else {
                                    if(k < (strlen(argv[j])-1)) {
                                        for(m=0,n=(k+1);(n<strlen(argv[j]));n++,m++)
                                                outputfile[m]=argv[j][n];
                                         outputfile[m]='\0';
                                    }
                                    else {
                                        strncpy(outputfile, argv[j+1], FIELD_SIZE-7);
                                        j++;
                                    }
                                    k=strlen(argv[j]); /*Force k to get next j*/
                                } /* endelse */
                                break;
                            case 'h':  /* User requested help */
                            case '?':
                                if(help_flag == FALSE)
                                    help_flag=TRUE;
                                else
                                    bad_parms=TRUE;
                                break;
                            default:
                                bad_parms=TRUE;
                                break;
                        } /* endswitch */
                    } /* endfor */
                } /* endif */
                else {
                    if(strlen(argv[j]) > 0)  /* No error if NULL */
                        bad_parms=TRUE;
                }
            } /* endfor */
        } /* endif */

        /* If outputfile wasn't set by the "-f" option, check UMLCOUTPUT*/
        if (strncmp(outputfile,"\0",1) == 0) {
            if((pchr=getenv("UMLCOUTPUT")) != NULL)
                strcpy(outputfile, pchr);
        } /* endif */

        /* If output file specified, it must not contain a path */
        if (strncmp(outputfile,"\0",1) != 0) {
            tempstr=strrchr(outputfile,'/');
            if(tempstr)                          /* A '/' was found */
                    bad_parms=TRUE;
        }

        if ((do_mfg_bld == TRUE) &&
          (bad_parms == FALSE) &&
          (help_flag == FALSE)) {
            if((no_menus == TRUE) || (strncmp(outputfile,"\0",1)!=0))
                bad_parms = TRUE;
            else
                mfg_build();  /* stand-alone mfg rebuild doesn't return */
        } /* endif */


        /* open the catalog file containing the menus */
        fdes = diag_catopen(MF_UMLC,0);

        /* If there were command line errors or help was requested */
        if ((bad_parms == TRUE) || (help_flag == TRUE)) {
            if ((no_menus == FALSE) && (do_mfg_bld == FALSE)) {
                if((bad_parms == TRUE) || ((help_flag == TRUE) &&
                 (strncmp(outputfile,"\0",1)!=0)))
                    printf("%s\n",diag_cat_gets(fdes, ESET, INVALID_PARM_ERR));
                sprintf(tempoutput,diag_cat_gets(fdes,ESET,USAGE_ERR),diagdir);
                printf("%s\n",tempoutput);
            }
            usage_exit(5);
        } /* endif */

        /* If outputfile isn't set, try to get UMLCOUTPUT*/
        if (strncmp(outputfile,"\0",1) == 0) {
                if((pchr=getenv("UMLCOUTPUT")) != (char *) NULL)
                        strcpy(outputfile, pchr);
        } /* endif */


        /* get path to diagnostics from environment */
        if ((pchr = getenv("DIAGDATADIR")) != (char *)NULL)
          strcpy(diagdir, pchr);

        /* See if unit supports writing files to a floppy disk drive */
        mach_model = get_cpu_model(&model_code);
        if (mach_model & RSC_MODEL)
                fdd_not_supported = TRUE;

        /* Determine if floppy disk drive is available.               */
        /* On some models, no system disk is shipped, so force all    */
        /*   processing to be done on the hardfile by fdd_gone = TRUE */
        sprintf(criteria, "name=fd0 and chgstatus != %d", MISSING);
        cudv = get_CuDv_list(CuDv_CLASS, criteria, &c_info, 1, 1);
        if (cudv == (struct CuDv *)-1) genexit(201);
        if ((c_info.num == 0) || (fdd_not_supported))
                fdd_gone = TRUE;
        else
                fdd_gone = FALSE;

        /* If called with "-e" option, create PT Update diskette on hardfile */
        if (no_menus == TRUE) {
                if(strncmp(outputfile,"\0",1)==0) /* must specify -f or UMLCOUTPUT with -e */
                        genexit(1);
                else
                        create_update();  /* does not return */
        } /* endif */

        /* initialize ASL       */
        diag_asl_init("NO_TYPE_AHEAD");

        disp_and_sleep();
        while (!process_rebuild()) {    /* Get system record info */
                disp_terminate();
        }
        update_sysunit_vpd();            /* Update sysunit0 in CuVPD */

        disp_and_sleep();
        process_repair();                /* Update configuration & write dskt */

        if (output) diag_display(COMPLETE, fdes, menulist2,
                    DIAG_IO, ASL_DIAG_NO_KEYS_ENTER_SC,
                    NULL, NULL);

        genexit(0);                      /* Exit */
}

/*  */
/*
 * NAME: int_handler
 *
 * FUNCTION: Perform general clean up, and then exit with the status code
 *
 * RETURNS: NONE
 */

void int_handler(int sig)
{
char filename[256];

        if(output_file_open == TRUE) {
                sprintf(filename, "%s/PT_SYS", diagdir);
                unlink(filename);
                sprintf(filename, "%s/PT_SDDB", diagdir);
                unlink(filename);
                sprintf(filename, "%s/PT_HIST", diagdir);
                unlink(filename);
        }
        diag_asl_clear_screen();
        genexit(6);
}

/*  */
/*
 * NAME: genexit
 *
 * FUNCTION: Perform general clean up, and then exit with the status code
 *
 * RETURNS: NONE
 */

void genexit(int exitcode)
{
        if(!no_menus)        /* If -e specified, then ASL wasn't started */
                diag_asl_quit();
        term_dgodm();
        catclose(fdes);
        exit(exitcode);
}

/*  */
/*
 * NAME: usage_exit
 *
 * FUNCTION: Perform general clean up, and then exit with the status code
 *
 * RETURNS: NONE
 */

void usage_exit(int exitcode)
{
        term_dgodm();
        catclose(fdes);
        exit(exitcode);
}

/*  */
/*
 * NAME: disp_and_sleep
 *
 * FUNCTION: displays the processing data message and sleeps for 1 second
 *           to give the apperance that a lot of processing is going on
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */

void disp_and_sleep(void)
{
        /* display screen indicating something is working */
        diag_msg_nw(PROCESS_DATA, fdes, MSET, PROCESS_MSG );
        sleep(1); /* it works too fast. make it look slower */
}
