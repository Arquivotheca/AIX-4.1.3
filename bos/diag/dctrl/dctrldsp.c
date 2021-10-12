static char sccsid[] = "@(#)10	1.35.2.23  src/bos/diag/dctrl/dctrldsp.c, dctrl, bos41J, 9517A_all 4/24/95 08:56:25";
/*
 * COMPONENT_NAME: (CMDDIAG) Diagnostic Controller
 *
 * FUNCTIONS:   disp_more_tests
 *              disp_da_output
 *              disp_fru
 *              disp_frub
 *              disp_mgoal
 *              disp_ntf
 *              disp_ds_menu
 *              disp_tm_menu
 *              disp_dm_menu
 *              disp_dc_error
 *              prompt_for_diskette
 *              miss_fb
 *              ela_fb
 *              oco_fb
 *              sco_fb
 *              add_to_problem_menu
 *              disp_ela_pd_menu
 *              disp_no_test_printf
 *		wraptitle
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <nl_types.h>
#include <sys/sysprobe.h>
#include <sys/retids.h>
#include <sys/probeids.h>
#include <sys/types.h>
#include "diag/diago.h"
#include "diag/diag.h"
#include "diag/diagvpd.h"
#include "diag/da.h"
#include "diag/tmdefs.h"
#include "diag/diag_exit.h"
#include "diag/class_def.h"
#include "diag/diag_errids.h"
#include "dctrl_msg.h"
#include "dctrl.h"

/* GLOBAL VARIABLES */
struct frukey {
                char dname[NAMESIZE];   /* device name  */
                int   ftype;            /* fru bucket type  */
} frutab[100];
int     dmode;

/* EXTERNALLY DEFINED VARIABLES */
extern int advancedflg;
extern int unattendedflg;
extern int moretesting;
extern int lerror;              /* number of errors during run          */
extern int exenvflg;
extern int diag_mode;
extern int diag_ipl_source;
extern int next_diskette;
extern int systestflg;
extern int basetestflg;
extern int missingflg;
extern int elaflg;              /* error log analysis                  */
extern int num_DSM_devices;     /* number of devices in selection menu */
extern int num_diskettes;       /* number of diskettes in package      */
extern int num_All;             /* number of devices supported         */
extern int num_dev_to_test;     /* number of devices to test/tested    */
extern int srn_generated;       /* was gen_rpt called with a srn       */
extern char current_volume[];   /* the last diskette read              */
extern char startdate[];
extern char enddate[];
extern int diag_report;

extern nl_catd                  fdes;
extern diag_dev_info_ptr_t     *All;
extern diag_dev_info_ptr_t     *DSMenu;
extern diag_dev_info_ptr_t     *test_array;
extern diag_dev_info_ptr_t     *missing_tested;
extern diag_dev_info_ptr_t     *ela_tested;

/* CALLED FUNCTIONS */
void	log_symptom_strings(char *, char *, char *);
short	convert_ffc(short);
char                    *malloc();
char                    *diag_cat_gets();
diag_dev_info_t         *pop_device();
diag_dev_info_t         *find_dev();
extern int		error_log_get(int, char *, struct errdata *);
extern nl_catd diag_catopen(char *, int);
/*   */
/*
 * NAME: disp_more_tests
 *
 * FUNCTION: Display menu indicating 'Additional Resources needed'
 *
 * NOTES: If device open error - display device busy open error menu
 *        If state good - display NTF menu
 *        If state is bad - display problem found menu
 *
 * RETURNS:
 *      DIAG_ASL_COMMIT
 *      DIAG_ASL_CANCEL
 *      DIAG_ASL_EXIT
 */

disp_more_tests(dev_ptr)
diag_dev_info_t *dev_ptr;
{
        int     rc;
        int     length;
        char    *tmp;
        char    *buffer;
        char    *opstring;

        static struct  msglist prob[] = {
                        { SET_ADDPR, MSG_ADDPR_T, },
                        { SET_ADDPR, MSG_ADDPR_2, },
                        { SET_ADDPR, MSG_ADDPR_3, },
                        { SET_ADDPR, MSG_ADDPR_E, },
                        (int )NULL
        };
        static ASL_SCR_INFO     probinfo[ DIAG_NUM_ENTRIES(prob) ];

        static struct  msglist open[] = {
                        { SET_ADDDD, MSG_ADDDD_T, },
                        { SET_ADDPR, MSG_ADDPR_2, },
                        { SET_ADDPR, MSG_ADDPR_3, },
                        { SET_ADDPR, MSG_ADDPR_E, },
                        (int)NULL
        };
        static ASL_SCR_INFO     openinfo[ DIAG_NUM_ENTRIES(open) ];

        static struct  msglist ntf[] = {
                        { SET_ADDNTF, MSG_ADDNTF_T, },
                        { SET_ADDPR, MSG_ADDPR_2, },
                        { SET_ADDPR, MSG_ADDPR_3, },
                        { SET_ADDPR, MSG_ADDPR_E, },
                        (int)NULL
        };
        static ASL_SCR_INFO     ntfinfo[ DIAG_NUM_ENTRIES(ntf) ];
        static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;

        /* get the device name and description and place in 'buffer' */
        buffer = tmp = malloc(2048);
        opstring = malloc(2048);
        resource_desc(tmp,NTF_TYPE,dev_ptr);
        length = strlen(tmp);
        tmp[length] = '\n';
        tmp[length+1] = '\0';

        /* Open Error on Device Driver */
        if ( dev_ptr->flags.device_driver_err == DIAG_TRUE ) {
                dev_ptr->flags.device_driver_err = DIAG_FALSE;
                rc = diag_display(NULL, fdes, open, DIAG_MSGONLY, NULL,
                                        &menutype, openinfo);
                sprintf(opstring, openinfo[0].text, buffer);
		free(openinfo[0].text);
                openinfo[0].text = opstring;
                rc = diag_display(ADD_RESOURCE_1, fdes, NULL, DIAG_IO,
                                        ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                        &menutype, openinfo);
        }
        /* Problem Detected            */
        else if ( dev_ptr->T_CDiagDev->State == STATE_BAD ) {
                rc = diag_display(NULL, fdes, prob, DIAG_MSGONLY, NULL,
                                        &menutype, probinfo);
                sprintf(opstring, probinfo[0].text, buffer);
		free(probinfo[0].text);
                probinfo[0].text = opstring;
                rc = diag_display(ADD_RESOURCE_2, fdes, NULL, DIAG_IO,
                                        ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                        &menutype, probinfo);
        }
        /* No Trouble Found            */
        else  {
                rc = diag_display(NULL, fdes, ntf, DIAG_MSGONLY, NULL,
                                        &menutype, ntfinfo);
                sprintf(opstring, ntfinfo[0].text, buffer);
		free(ntfinfo[0].text);
                ntfinfo[0].text = opstring;
                rc = diag_display(ADD_RESOURCE_3, fdes, NULL, DIAG_IO,
                                        ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                        &menutype, ntfinfo);
        }

        if ( rc == DIAG_ASL_COMMIT )
                switch(DIAG_ITEM_SELECTED(menutype))  {
                        case 1:         /* Testing should stop  */
                                rc = DIAG_ASL_CANCEL;
                                break;
                        case 2:         /* Testing can continue*/
                                break;
                }
        free(tmp);
        free(opstring);
        return(rc);
}
/*   */
/*
 * NAME: disp_da_output
 *
 * FUNCTION: Determine output to display from Diagnostic Application.
 *
 * NOTES:
 *
 * RETURNS:
 *      0 - no error
 *     -1 - error occurred
 */

disp_da_output()
{
        int rc;

        if ( num_dev_to_test == -1 || num_dev_to_test == 0 )
                return(0);

        /* if any errors occurred from any tests previously run */
        if (lerror)
                rc = disp_fru();

        /* else display no problems */
        else
                rc = disp_ntf();

        /* display menu goal if present */
        disp_mgoal();

        return(rc);
}

/*   */
/*
 * NAME: disp_fru
 *
 * FUNCTION: This function will display a Problem Report Frame. The fru
 *              bucket data is obtained from the FRUB and FRUs Object
 *              Class.
 *
 * NOTES:
 *      Allocate space for number of possible entries to
 *              ((number of devices tested) * 8) + 5 overhead lines.
 *      Determine FRUB to use
 *      Put the title line and SRN line in the display.
 *      Fill in probable causes
 *      Add last line and display screen
 *      Call frub_write to write to error file
 *      Display screen and wait for response
 *      Clear FRUB and FRUs object classes
 *
 * RETURNS:
 *      DIAG_ASL_COMMIT
 *      DIAG_ASL_CANCEL
 *      DIAG_ASL_EXIT
 */

disp_fru()
{

        int             i;
        int             line=2;         /* first line to start with */
        int             index;
        int             rc;
        int             msg_size;
        int             setno;
        nl_catd         cfg_fdes;       /* for dcda.cat SRN's      */
        nl_catd         alt_fdes;       /* if DA has SRN in DA.cat */
        char            *string;
        char            *free_string;
        char            *tmp;
        char            title_buffer[256];
        char            dacat[64];
        char            crit[256];
        char            search[256];
        char            timestamp[80];
        struct FRUs     *T_FRUs;
        struct CuDv     *T_CuDv;
        struct listinfo c_info;
        diag_dev_info_t *fru_device;
        ASL_SCR_INFO    *resinfo;
        static ASL_SCR_TYPE restype = DM_TYPE_DEFAULTS;

        /* allocate space for enough entries */
        resinfo = (ASL_SCR_INFO *)
                calloc(1,((256*8)+5)*sizeof(ASL_SCR_INFO) );
        if ( resinfo == (ASL_SCR_INFO *) NULL)
                return(-1);

        free_string = string = malloc(100*4*132);
        if ( string == (char *) NULL)
                return(-1);

        if (systestflg || basetestflg)
                msg_size = sco_fb(string, resinfo, &line, timestamp, frutab);
        else if (missingflg)
                msg_size = miss_fb(string, resinfo, &line, timestamp, frutab);
        else if (elaflg)
                msg_size = ela_fb(string, resinfo, &line, timestamp, frutab);
        else
                msg_size = oco_fb(string, resinfo, &line, timestamp, frutab);
        if ( msg_size == -1 )
                return(-1);
	else if ( msg_size == 0 )
                return (0);

        string += msg_size;

        if ( !isatty(1) && ( exenvflg == EXENV_STD ) )
                return(report_fru_byled());

        /* set the title line in the array  */
        sprintf(title_buffer, "%s%s", diag_cat_gets(fdes, DIAG_PROB_SETID,
                        DIAG_PROB_TITLE), timestamp);
        resinfo[0].text = title_buffer;
        resinfo[1].text = diag_cat_gets(fdes, DIAG_PROB_SETID, DIAG_PROB_SRN_ERRCODE);

        /* now fill in probable causes  */
        resinfo[line++].text = diag_cat_gets(fdes, DIAG_PROB_SETID,
                                        DIAG_PROB_CUS);

        cfg_fdes = diag_catopen(PORT_CAT,0);

        for(index=0; frutab[index].dname[0] != '\0'; index++)  {
                sprintf(crit, "dname = '%s' and ftype = %d",
                        frutab[index].dname, frutab[index].ftype);
                T_FRUs = (struct FRUs *)diag_get_list(FRUs_CLASS,crit,
			&c_info, MAX_EXPECT, 1);
                if ( T_FRUs == (struct FRUs *) -1)
                        return (-1);
                for(i=0; i < c_info.num; i++)  {
                        sprintf(string, "- %3d%%  %-16.16s  %-16.16s  ",
                                        T_FRUs[i].conf,T_FRUs[i].fname,
                                        T_FRUs[i].floc);
                        tmp = string + strlen(string);
                        fru_device = find_dev(T_FRUs[i].fname);

                        /* if FRU is not in database */
                        if ( fru_device == NULL ) {
                                fru_device = find_dev(T_FRUs[i].dname);

                                /* if missing options and running DMORPS */
                                /* use message set 1                     */
                                if (missingflg &&
                                    strlen(fru_device->T_PDiagDev->EnclDaName)
				    && (frutab[index].ftype == FRUB_ENCLDA) )
                                        setno = 1;
                                else
                                        setno = fru_device->T_PDiagDev->PSet;

                                if( (fru_device->T_PDiagDev->Menu & DIAG_DA_SRN)
					&& !( missingflg  && strlen(fru_device->T_PDiagDev->EnclDaName)) ){

					/* use the DA's catalog file */

                                        sprintf(dacat, "%s.cat",
                                              fru_device->T_PDiagDev->DaName);
                                        alt_fdes = diag_catopen(dacat,0);
                                        copy_text( strlen(string), tmp,
                                                diag_cat_gets(alt_fdes, setno,
                                                   T_FRUs[i].fmsg));
                                        catclose(alt_fdes);
                                }
                                else
					/* use dcda.cat for fru message */

                                        copy_text( strlen(string), tmp,
                                                diag_cat_gets(cfg_fdes, setno,
                                                   T_FRUs[i].fmsg));
                        }
                        else  {
                                copy_text( strlen(string), tmp,
                                           fru_device->Text);
                        }
                        resinfo[line++].text = string;
                        string = string + strlen(string)+1;
                }
        }

        /* add additional testing if required and last line */
        if (moretesting == DIAG_TRUE)
                resinfo[line].text = diag_cat_gets(fdes, DIAG_PROB_SETID,
                                DIAG_PROB_ADD);
        else
                resinfo[line].text = diag_cat_gets(fdes, DIAG_PROB_SETID,
                                DIAG_PROB_RET);

        restype.max_index = line;


        /* now display screen */
        if ( line > 3 ) {
                /* write all this to an error log file */
                gen_rpt(PROB_REPORT,restype,resinfo);

                if ( !unattendedflg )
                        rc = diag_display(PROB_REPORT, fdes, NULL, DIAG_IO,
                                                ASL_DIAG_ENTER_SC,
                                                &restype, resinfo);
        }

        /* clear the object classes */
        if(clr_class("FRUB"))
                disp_dc_error(ERROR6, "FRUB");
        if(clr_class("FRUs"))
                disp_dc_error(ERROR6, "FRUs");

        /* free up allocated buffers */
        free ( free_string );
        free ( resinfo );

        catclose(cfg_fdes);
        return (rc);
}
/*   */
/*
 * NAME:  miss_fb
 *
 * FUNCTION: get fru buckets for missing options
 *
 * NOTES:
 *
 * RETURNS: size of text describing fru
 *
 */

miss_fb(buffer, text, line_num, timestamp, tab)
char *buffer;
ASL_SCR_INFO *text;
int *line_num;
char *timestamp;
struct frukey tab[];
{
        diag_dev_info_t *dev_to_report;
        diag_dev_info_t *curr_dev;
        int             i;
        int             total_length=0, size;
        int             fbtype;
        int             index=0;

        /* first search for all the devices that the controller added
           to the fru list */
        for ( i=0; i < num_All; i++ ) {
                if( All[i]->flags.defective_device == DIAG_TRUE) {
                        All[i]->flags.defective_device = DIAG_FALSE;
                        fbtype = FRUB1;
                        size = add_to_problem_menu ( &fbtype, All[i],
                                                buffer, text, line_num,
                                                timestamp);
                        strcpy(tab[index].dname, All[i]->T_CuDv->name);
                        tab[index++].ftype = fbtype;
                        buffer += size;
                        total_length += size;
                }
        }

        /* next get all those DA's that added in their own      */
        for( i=0; missing_tested[i]; ++i) {
                dev_to_report = (diag_dev_info_t *) NULL;
                curr_dev = missing_tested[i];

                /* process stack until the null entry is reached */
                while( curr_dev ) {

                        /* if the device was actually tested */
                        if (curr_dev->flags.device_tested == DIAG_TRUE) {
                                /*if curr_dev is not sibling of dev_to_report*/
                                if (sibling_check(curr_dev,dev_to_report))  {
                                        fbtype = FRUB1;
                                        if (curr_dev->T_CDiagDev->State ==
                                                                STATE_BAD)
                                                dev_to_report = curr_dev;
                                }
                                else if (curr_dev->T_CDiagDev->State ==
                                                                STATE_GOOD)
                                        fbtype = FRUB2;
                        }
                        curr_dev = missing_tested[++i];
                }
                if ( dev_to_report ) {
                        size = add_to_problem_menu ( &fbtype, dev_to_report,
                                                buffer, text, line_num,
                                                timestamp);
                        strcpy(tab[index].dname, dev_to_report->T_CuDv->name);
                        tab[index++].ftype = fbtype;
                        buffer += size;
                        total_length += size;
                }
        }

        tab[index].dname[0] = '\0';
        return(total_length);
}

/*   */
/*
 * NAME: ela_fb
 *
 * FUNCTION: get fru buckets for error log analysis
 *
 * NOTES:
 *
 * RETURNS: number of bytes in buffer describing fru
 *
 */

ela_fb(buffer, text, line_num, timestamp, tab)
char *buffer;
ASL_SCR_INFO *text;
int *line_num;
char *timestamp;
struct frukey tab[];
{
        diag_dev_info_t *dev_to_report;
        diag_dev_info_t *curr_dev;
        int             i;
        int             total_length=0, size;
        int             fbtype;
        int             index=0;

        /* process the ela_tested array for bad devices */
        for( i=0; ela_tested[i]; ++i) {
                dev_to_report = (diag_dev_info_t *) NULL;
                curr_dev = ela_tested[i];

                /* process stack until the null entry is reached */
                while( curr_dev ) {

                        /* if the device was actually tested */
                        if (curr_dev->flags.device_tested == DIAG_TRUE)  {
                                /*if curr_dev is not sibling of dev_to_report*/
                                if (sibling_check(curr_dev,dev_to_report))  {
                                        fbtype = FRUB1;
                                        if (curr_dev->T_CDiagDev->State ==
                                                                STATE_BAD)
                                                dev_to_report = curr_dev;
                                }
                                else if (curr_dev->T_CDiagDev->State ==
                                                                STATE_GOOD)
                                        fbtype = FRUB2;
                        }
                        curr_dev = ela_tested[++i];
                }
                if ( dev_to_report ) {
                        size = add_to_problem_menu ( &fbtype, dev_to_report,
                                                buffer, text, line_num,
                                                timestamp);
                        strcpy(tab[index].dname, dev_to_report->T_CuDv->name);
                        tab[index++].ftype = fbtype;
                        buffer += size;
                        total_length += size;
                }
        }

        tab[index].dname[0] = '\0';
        return(total_length);
}

/*   */
/*
 * NAME: oco_fb
 *
 * FUNCTION: get fru buckets for option checkout
 *
 * NOTES:
 *
 * RETURNS: number of bytes in buffer decribing fru
 *
 */

oco_fb(buffer, text, line_num, timestamp, tab)
char *buffer;
ASL_SCR_INFO *text;
int *line_num;
char *timestamp;
struct frukey tab[];
{
        diag_dev_info_t *dev_to_report;
        diag_dev_info_t *curr_dev;
        int             i;
        int             rc;
        int             fbtype;

        dev_to_report = (diag_dev_info_t *) NULL;

        for( i=0; i < num_dev_to_test; i++ ) {

                curr_dev = pop_device(i);

                /* if the device was actually tested */
                if (curr_dev->flags.device_tested == DIAG_TRUE)  {
                        /* if curr_dev is not sibling of dev_to_report */
                        if (sibling_check(curr_dev,dev_to_report))  {

                                fbtype = FRUB1;

                                if (curr_dev->T_CDiagDev->State == STATE_BAD)
                                        dev_to_report = curr_dev;
                        }
                        else if (curr_dev->T_CDiagDev->State == STATE_GOOD)
                                fbtype = FRUB2;
                }
        }

        rc = add_to_problem_menu ( &fbtype, dev_to_report, buffer,
                                        text, line_num, timestamp);

        strcpy(tab[0].dname, dev_to_report->T_CuDv->name);
        tab[0].ftype = fbtype;
        tab[1].dname[0] = '\0';
        return(rc);
}

/*   */
/*
 * NAME: sco_fb
 *
 * FUNCTION: get fru buckets for system checkout
 *
 * NOTES: If tested device has no children and reported BAD_STATUS, use FRUB1
 *        Else use FRUB2 for each child reporting BAD_STATUS
 *
 * RETURNS: number of bytes in buffer decribing fru
 *
 */

sco_fb(buffer, text, line_num, timestamp, tab)
char *buffer;
ASL_SCR_INFO *text;
int *line_num;
char *timestamp;
struct frukey tab[];
{
        diag_dev_info_t *curr_dev;
        diag_dev_info_t *next_dev;
        diag_dev_info_t *first_dev;
        int             i;
        int             next;
        int             fbtype;
        int             total_length=0, size;
        int             index=0;

        curr_dev = pop_device(0);

        for( i=1; i <= num_dev_to_test; ) {

                first_dev = curr_dev;

                /* set next_dev to the next device that is not child of x */
                find_next_non_dependent(i,&next,test_array,num_dev_to_test);
                next_dev = pop_device(next);

                while( (curr_dev != next_dev) && (i <= num_dev_to_test) )  {

                        if (first_dev == curr_dev)
                                fbtype = FRUB1;
                        else
                                fbtype = FRUB2;

                        /* if its state is bad and it was actually tested */
                        /*      report it.                                */
                        if ( (curr_dev->T_CDiagDev->State == STATE_BAD)
                          && (curr_dev->flags.device_tested == DIAG_TRUE) ) {
                                size = add_to_problem_menu ( &fbtype, curr_dev,
                                                buffer, text, line_num,
                                                timestamp);
                                strcpy(tab[index].dname, curr_dev->T_CuDv->name);
                                tab[index++].ftype = fbtype;
                                buffer += size;
                                total_length += size;
                        }
                        curr_dev = pop_device(i++);
                }
        }
        tab[index].dname[0] = '\0';
        return(total_length);
}
/*   */
/*
 * NAME: add_to_problem_menu
 *
 * FUNCTION:
 *
 * NOTES: May be more than one entry
 *
 * RETURNS:
 *      >0 : number of characters in buffer
 *      -1 : error reading FRUB Object Class
 */

add_to_problem_menu ( fbtype, dev_ptr, buffer, text, line_num, timestamp )
int             *fbtype;
diag_dev_info_t *dev_ptr;
char            *buffer;
ASL_SCR_INFO    *text;
int             *line_num;
char            *timestamp;
{
        nl_catd         cfg_fdes;
        nl_catd         alt_fdes;
        int             alt_fbtype = FRUB1;
        int             current_line;
        int             index;
        int             set;
        char            crit[256];
        char            dacat[64];
	char		srn[9];
	char		errcode[32];
        char            *tmp;
        char            *tmp_buff;
	short		sn, rcode,no_rcode;
        struct FRUB     *T_FRUB;
        struct listinfo f_info;

        /* Sometimes a DA will return STATUS_BAD without a FRU */
        if ( dev_ptr == (diag_dev_info_t *) NULL )
                return (0);

        /* read entry from FRUB Object Class */
        sprintf(crit, "dname = '%s' and ftype = %d",
                dev_ptr->T_CuDv->name, *fbtype);
        T_FRUB = (struct FRUB *)diag_get_list(FRUB_CLASS, crit,
			&f_info, MAX_EXPECT, 1);
        if ( T_FRUB == (struct FRUB *) -1)
                return (-1);

        /* if nothing found, try the other fru bucket type */
        if ( f_info.num == 0 )  {
                if ( *fbtype == FRUB1 )
                        alt_fbtype = FRUB2;
                sprintf(crit, "dname = '%s' and ftype = %d",
                        dev_ptr->T_CuDv->name, alt_fbtype);
                T_FRUB = (struct FRUB *)diag_get_list(FRUB_CLASS,crit,
				&f_info, MAX_EXPECT, 1);
                if ( T_FRUB == (struct FRUB *) -1)
                        return (-1);
		if(( f_info.num == 0 ) && missingflg ){
			alt_fbtype = FRUB_ENCLDA;
                	sprintf(crit, "dname = '%s' and ftype = %d",
                       		 dev_ptr->T_CuDv->name, alt_fbtype);
	                T_FRUB = (struct FRUB *)diag_get_list(FRUB_CLASS,
				crit, &f_info, 
				MAX_EXPECT, 1);
	                if ( T_FRUB == (struct FRUB *) -1)
       	                 	return (-1);
		}
                *fbtype = alt_fbtype;
        }

        cfg_fdes = diag_catopen(PORT_CAT,0);
	no_rcode=0;
        tmp_buff = buffer;
        for ( index = 0; index < f_info.num; index++)  {

                /* if sn is a -1, treat rcode as a 4 digit hex number   */
                if ( T_FRUB[index].sn == -1 ){
			rcode=convert_ffc(T_FRUB[index].rcode);
                        sprintf(tmp_buff,"      %04X:  ",rcode & 0xFFFF);
			sprintf(srn,"%04X",rcode & 0xFFFF);

		/* if sn is -2, get 8 hex digits error code from DAVars */
		} else if ( T_FRUB[index].sn == -2 ){
			no_rcode=(T_FRUB[index].rmsg == 0);
			if(getdavar(T_FRUB[index].dname, DAVARS_ERRCODE,
					DIAG_STRING, errcode) != -1)
			{
				sprintf(tmp_buff," %s", errcode);
				sprintf(srn,"%s", errcode);
				/* clear the object class in the data base */
				sprintf(crit, "dname = '%s' and vname = '%s'",
						T_FRUB[index].dname, DAVARS_ERRCODE);
				if (((int) diag_open_class(DAVars_CLASS)) != -1)
				{
					diag_rm_obj( DAVars_CLASS, crit );
					diag_close_class(DAVars_CLASS);
				}
			}

                /* else default is 3 digits separated by a dash */
                } else {
			sn=convert_ffc(T_FRUB[index].sn);
			/* If controller generated code then rcode has */
			/* ffc, and needs to be converted.	       */

                	if ( (T_FRUB[index].sn == DC_SOURCE_NEW ) ||
                     	     (T_FRUB[index].sn == DC_SOURCE_MISS) ||
                             (T_FRUB[index].sn == DC_SOURCE_SOFT))
				rcode=convert_ffc(T_FRUB[index].rcode);
			else
				rcode=T_FRUB[index].rcode;
                        sprintf(tmp_buff,"  %03X-%03X:  ", sn, rcode);
			sprintf(srn,"%03X-%03X", sn, rcode);
		}
                tmp = tmp_buff + strlen(tmp_buff);

		/* If there is a reason message then continue to fill up the */
		/* buffer using the rmsg as the msg id into the message file */

		if(!no_rcode)
		{
	                /* if a controller generated error - set set to 1 */
       	         	/* or Missing Device DA also uses set 1 */
	                if ( (T_FRUB[index].sn == DC_SOURCE_NEW ) ||
       		              (T_FRUB[index].sn == DC_SOURCE_MISS) ||
       		              (T_FRUB[index].sn == DC_SOURCE_SOFT) ||
       		              (missingflg && strlen(dev_ptr->T_PDiagDev->EnclDaName)
				     && T_FRUB[index].ftype == FRUB_ENCLDA) )
       		                 set = 1;
       		         else
       		                 set = dev_ptr->T_PDiagDev->PSet;

			/* Use message from the da catalog if the Menu bit */
			/* is set in PDiagDev and the sn is not from the   */
			/* controller nor from MORPS.			   */

       		         if( (dev_ptr->T_PDiagDev->Menu & DIAG_DA_SRN) 
       		         	&& (T_FRUB[index].sn != DC_SOURCE_NEW ) &&
       		              	(T_FRUB[index].sn != DC_SOURCE_MISS) &&
       		              	(T_FRUB[index].sn != DC_SOURCE_SOFT) &&
				!( missingflg && ((T_FRUB[index].sn == DC_SOURCE_MORPS1)
				   || (T_FRUB[index].sn == DC_SOURCE_MORPS2)) ) )
			{
       		                 sprintf(dacat, "%s.cat", dev_ptr->T_PDiagDev->DaName);
       		                 alt_fdes = diag_catopen(dacat,0);
       		                 copy_text( strlen(tmp_buff), tmp,
       		                            diag_cat_gets(alt_fdes, set,
                                                 T_FRUB[index].rmsg));
       		                 catclose(alt_fdes);
       		         }
       		         else
       		                 copy_text( strlen(tmp_buff), tmp,
       			                    diag_cat_gets(cfg_fdes, set, T_FRUB[index].rmsg));
		}
       		strcpy(timestamp,T_FRUB[index].timestamp);
       		current_line = *line_num;
       		text[current_line].text = tmp_buff;
               	tmp_buff = tmp_buff + strlen(tmp_buff) + 1;
		(*line_num)++;
        }
        catclose(cfg_fdes);
        if ( T_FRUB )
                diag_free_list ( T_FRUB, &f_info );

	if( (  diag_report ) && (dev_ptr->T_CDiagDev->State == STATE_BAD) &&
			(dev_ptr->flags.device_tested == DIAG_TRUE) )
		log_symptom_strings(dev_ptr->T_CuDv->name, srn, buffer);

        return(tmp_buff-buffer);
}
/*   */
/*
 * NAME: disp_frub
 *
 * FUNCTION: This function will display a Problem Report Frame. The fru
 *              bucket data is obtained from the input parameter fru_bucket
 *              structure.
 *
 * NOTES:
 *      Allocate space for number of possible entries to
 *      Put the title line and SRN line in the display.
 *      Fill in probable causes
 *      Add last line and display screen
 *      Call frub_write to write to error file
 *      Display screen and wait for response
 *
 * RETURNS:
 *      DIAG_ASL_COMMIT
 *      DIAG_ASL_CANCEL
 *      DIAG_ASL_EXIT
 */

disp_frub( frub )
struct fru_bucket *frub;
{
        int             rc;
        int             line=0;
        char            *string;
        char            *free_string;
        char            *tmp;
        char            title_buffer[256];
        char            timestamp[80];
        nl_catd         cfg_fdes;
	short		sn, rcode;
        ASL_SCR_INFO    *resinfo;
        static ASL_SCR_TYPE restype = DM_TYPE_DEFAULTS;

        /* allocate space for enough entries */
        resinfo = (ASL_SCR_INFO *)
                calloc(1,10*sizeof(ASL_SCR_INFO) );
        if ( resinfo == (ASL_SCR_INFO *) NULL)
                return(-1);

        if ( (string = malloc(1024)) == (char *) NULL)
                return(-1);
        free_string = string;

        cfg_fdes = diag_catopen(PORT_CAT,0);

        /* set the title line in the array  */
        getdate(timestamp, 79);
        sprintf(title_buffer, "%s%s", diag_cat_gets(fdes, DIAG_PROB_SETID,
                        DIAG_PROB_TITLE), timestamp);
        resinfo[line++].text = title_buffer;

        resinfo[line++].text = diag_cat_gets(fdes, DIAG_PROB_SETID,
                        DIAG_PROB_SRN);

	sn=convert_ffc(frub->sn);
        if ( (frub->sn == DC_SOURCE_NEW ) ||
             (frub->sn == DC_SOURCE_MISS) ||
             (frub->sn == DC_SOURCE_SOFT))
		rcode=convert_ffc(frub->rcode);
	else
		rcode=frub->rcode;
        sprintf(string,"  %3x-%3x:  ", sn, rcode);
        tmp = string + strlen(string);
        copy_text( strlen(string), tmp,
                        diag_cat_gets(cfg_fdes,1,frub->rmsg));
        resinfo[line++].text = string;

        /* now fill in probable causes  */
        if ( frub->frus[0].conf )
                resinfo[line++].text = diag_cat_gets(fdes,
                        DIAG_PROB_SETID, DIAG_PROB_CUS);

        /* add last line */
        resinfo[line].text = diag_cat_gets(fdes,
                        DIAG_PROB_SETID, DIAG_PROB_RET);

        restype.max_index = line;

        /* now display screen */
        /* write all this to an error log file */
        gen_rpt(PROB_REPORT, restype,resinfo);

        if ( !unattendedflg )
                rc = diag_display(PROB_REPORT, fdes, NULL, DIAG_IO,
                                        ASL_DIAG_ENTER_SC,
                                        &restype, resinfo);

        /* free up allocated buffers */
        free ( free_string );
        free ( resinfo );

        catclose(cfg_fdes);
        return (rc);
}
/*   */
/*
 * NAME: disp_mgoal
 *
 * FUNCTION: Display a menu goal if one is present.
 *
 * NOTES:
 *
 * RETURNS:
 *      0 - if no menu goal was displayed
 *      1 - if menu goal was displayed
 *     -1 - if error occurred
 */

disp_mgoal()
{
        int count=0,rc;
        long    menu_number;
        char    buffer[4096], *bufptr, *titleptr;
	char	title_b[1028];
	char	*line;
	char	*last_p; /* Where title ended */
        struct MenuGoal *T_MenuGoal;
        struct listinfo f_info;
        ASL_SCR_INFO    *resinfo;
        static ASL_SCR_TYPE restype = DM_TYPE_DEFAULTS;

        /* allocate space for 3 entries */
        resinfo = (ASL_SCR_INFO *)
                calloc(3,sizeof(ASL_SCR_INFO) );
        if ( resinfo == (ASL_SCR_INFO *) NULL)
                return(-1);

        /* read all entries from MenuGoal Object Class */
        T_MenuGoal = (struct MenuGoal *)diag_get_list(MenuGoal_CLASS,"",
			&f_info,MAX_EXPECT,1);
        if ( T_MenuGoal != (struct MenuGoal *) -1)
                for(count=0; count < f_info.num; count++)  {
                        restype.max_index = 2;
                        /* extract menu number from text */
                        sscanf( T_MenuGoal[count].tbuffer1, "%X", &menu_number);

                        /* next extract the title */
                        strtok(T_MenuGoal[count].tbuffer1," ");
                        titleptr = T_MenuGoal[count].tbuffer1 +
                                strlen(T_MenuGoal[count].tbuffer1) + 1;
			line = (char *) strtok(titleptr, "\n");
			last_p = line;
			wraptitle(0, title_b, line);

			resinfo[0].text = title_b;
                        /* everything else is message text */
                        bufptr = titleptr + strlen(last_p) + 1;
                        strcpy( buffer, bufptr );
                        strcat( buffer, T_MenuGoal[count].tbuffer2);
                        resinfo[1].text = buffer;
                        resinfo[2].text = "";
			
			/* If running with a console - then display */
        		if ( ! unattendedflg )
                        	rc = diag_display(menu_number, fdes, NULL, DIAG_IO,
                                                ASL_DIAG_ENTER_SC,
                                                &restype, resinfo);

                        /* Append menugoal to end of the diagnostic report file */
                        append_menugoal(menu_number, restype, resinfo);
                }

        /* Reset srn_generated so that future menugoals will be put into a new */
        /* file unless a new SRN is generated.                                 */
        srn_generated = DIAG_FALSE;

        free( resinfo );
        clr_class("MenuGoal");
        return((count == 0) ? 0 : 1);
}

/*  */
/* NAME: disp_ntf
 *
 * FUNCTION: Display No Trouble Found screen.
 *
 * NOTES:
 *
 * RETURNS:
 *      DIAG_ASL_COMMIT
 *      DIAG_ASL_CANCEL
 *      DIAG_ASL_EXIT
 */

disp_ntf()
{

        diag_dev_info_t *dev_ptr;
        int             index;
        int             line=0;
        int             rc = -2;
        int             display_flag;
        int             vol;
        char            *string;
        char            *free_string;
        char            *title;
        char            title_buffer[256];
        char            timestamp[80];
        ASL_SCR_INFO    *resinfo;
        static ASL_SCR_TYPE restype = DM_TYPE_DEFAULTS;

        /* allocate space for enough entries */
        resinfo = (ASL_SCR_INFO *)
                calloc(1,(num_dev_to_test+5)*sizeof(ASL_SCR_INFO) );
        if ( resinfo == (ASL_SCR_INFO *) NULL)
                return(-1);

        free_string = string = malloc(num_dev_to_test*132);
        if ( string == (char *) NULL)
                return(-1);

        /* set the title line in the array  */
        getdate(timestamp, 79);
        if (moretesting == DIAG_TRUE)
                sprintf(title_buffer,diag_cat_gets(fdes, SET_COMP, MSG_MOREEND),timestamp);
        else
                sprintf(title_buffer,diag_cat_gets(fdes, SET_COMP, MSG_NOMOREEND),timestamp);
        resinfo[line++].text = title_buffer;

        /* read the entries from DSMenu, get the DName, location, and
           description and put them in the array        */
        display_flag = 0;
        for(index=0; index < num_dev_to_test; index++)  {
                dev_ptr = pop_device(index);
                if ( dev_ptr->flags.device_tested == DIAG_TRUE ) {
                          display_flag = 1;
                          resource_desc(string,NTF_TYPE,dev_ptr);
                          resinfo[line++].text = string;
                          string = string + strlen(string)+1;
                }
        }

        /* finally add the last line    */
        /* if running advanced mode system verification - add MLC remark */
        if ( advancedflg == DIAG_TRUE && dmode == DMODE_REPAIR )
                resinfo[line].text = diag_cat_gets(fdes, SET_COMP, MSG_COMPMLC);
        else
                resinfo[line].text = diag_cat_gets(fdes, SET_COMP, MSG_COMPEND);

        /* now display screen */
        restype.max_index = line;
        if ( display_flag ) {
                /* write all this to an error log file */
                gen_rpt(TEST_COMPLETE, restype,resinfo);
                if ( !unattendedflg )
                        rc = diag_display(TEST_COMPLETE, fdes, NULL, DIAG_IO,
                                        ASL_DIAG_ENTER_SC,
                                                &restype, resinfo);
        }

        free ( free_string );
        free ( resinfo );

        return (rc);
}

/*  */
/* NAME: prompt_for_diskette
 *
 * FUNCTION: Prompts for the specified diskette
 *
 * NOTES:
 *
 * RETURNS:
        -1 : failure
 *      DIAG_ASL_COMMIT
 *      DIAG_ASL_CANCEL
 *      DIAG_ASL_EXIT
 */

prompt_for_diskette( fdes )
nl_catd fdes;
{
        int     rc;
        char    volume[20];
        char    path[20];
        int     index;
        struct  msglist sup_dskt_menu[]=
                {
                        { DISKETTE3, MSG_DISK_4 },
                        { DISKETTE3, MSG_DISK_5 },
                        { (int) NULL, (int) NULL }
                };
        ASL_SCR_INFO    *menuinfo;
        ASL_SCR_TYPE    menutype=DM_TYPE_DEFAULTS;
        int     bc;
        char    description[256];
        char    temp_buffer[512];
        short   disk3, samedisk;

        rc=diag_display(PROMPT_DISK, fdes, sup_dskt_menu, DIAG_IO,
		       ASL_DIAG_KEYS_ENTER_SC, NULL, NULL);
        /* setup current_volume so the proper clean up can occur */
        /* i.e if the previous supplemental diskette was read in */
        /* we need to have its files removed before reading in   */
        /* the next one.                                         */

        if( file_present("/etc/diagstartS") )
        strcpy(current_volume, "S");

        if(rc != DIAG_ASL_ENTER)
               return(rc);

	do {
          /* identify the current diskette volume */
          strcpy(volume ,chkdskt("DIAG"));
          /* Only allow supplemental diskette if running off cdrom */
          if(strcmp(volume,"S") && (volume[1] != 'S') )
                if ((rc = diag_hmsg(fdes,DISKETTE3,MSG_DISK_3))
			!= DIAG_ASL_ENTER)
                       return(rc);
                else
                       continue;

           /* if no disk in drive
              or same disk is wanted (execept S) - then just return */
           if( !strcmp(volume,"E") || (!strcmp(current_volume,volume)
                          && (strcmp(volume, "S") && (volume[1] != 'S'))))
                  return(DIAG_ASL_COMMIT);

           /* if supplemental diskette was read - put up message */
           if ( !strcmp(volume,"S") || (volume[1] == 'S') )
                  rc =diag_msg_nw(READING_DSKT, fdes, DISKETTE3, MSG_DISK_2);
           else{
           /* prompt bad diskette */
                  if ((rc = diag_hmsg(fdes,DISKETTE3,MSG_DISK_3))
			!= DIAG_ASL_ENTER)
                               return(rc);
                  else
                               continue;
           }
           if ( !read_diskette(current_volume) ){
                  strcpy(current_volume,volume);
                  return(DIAG_ASL_COMMIT);
           } else {
           /* prompt bad diskette */
                  if ((rc = diag_hmsg(fdes,DISKETTE3,MSG_DISK_1)) !=
                                           DIAG_ASL_ENTER)
                                return(rc);
           }
	} while (1);

        return(0);
}

/*  */
/* NAME: disp_ds_menu
 *
 * FUNCTION: Display diagnostic selection test menu.
 *
 * NOTES: The selection made is written into the 'selection' argument.
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *      -1              : error
 *      DIAG_ASL_COMMIT
 *      DIAG_ASL_CANCEL
 *      DIAG_ASL_EXIT
 *
 */

#define LINE_OFFSET 2

disp_ds_menu(selection)
int             *selection;     /* user selection of device to test */
{

        int             index;
        int             rc;
        int             i;
        int             line = 0;
        int             dchosen;
        static int      selection_entry;
        char            *string;
        static  ASL_SCR_INFO *resinfo;
        static ASL_SCR_TYPE restype = DM_TYPE_DEFAULTS;

        if(!resinfo){
        /* allocate space for enough entries */
                if((diag_ipl_source!=IPLSOURCE_DISK) && (diag_ipl_source!=IPLSOURCE_LAN)){
                        resinfo = (ASL_SCR_INFO *)
                                calloc(1,(num_DSM_devices+6)*sizeof(ASL_SCR_INFO) );
                } else
                        resinfo = (ASL_SCR_INFO *)
                                   calloc(1,(num_DSM_devices+5)*sizeof(ASL_SCR_INFO) );
                if ( resinfo == (ASL_SCR_INFO *) NULL)
                        return(-1);

                 string = malloc(num_DSM_devices*132);
                if ( (string == (char *) NULL) && (num_DSM_devices != 0))
                        return(-1);

                /* set the title line in the array   */
                if (advancedflg)
                        resinfo[line++].text = diag_cat_gets(fdes,
                                SET_DSMENU, MSG_DSMENU_A);
                else
                        resinfo[line++].text = diag_cat_gets(fdes,
                                SET_DSMENU, MSG_DSMENU_C);

                /* and the system checkout line if not in concurrent mode*/
                if(exenvflg != EXENV_CONC && diag_ipl_source == IPLSOURCE_DISK){
                        resinfo[line].text =
                                diag_cat_gets(fdes, SET_DSMENU, MSG_DSMENU_1);
                        resinfo[line++].non_select = ASL_NO;
                }

                /* and the base system test */
		if ( has_isa_capability() )
                	resinfo[line].text=diag_cat_gets(fdes,SET_DSMENU,MSG_DSMENU_ISA);
		else
                	resinfo[line].text=diag_cat_gets(fdes,SET_DSMENU,MSG_DSMENU_2);
                resinfo[line++].non_select = ASL_NO;
                selection_entry = line;
                /* read all the entries from DSMenu,get the DName,
                   location, and description and put them in the array*/
                for(index=0; index < num_DSM_devices; index++)  {
                        if(DSMenu[index] == NULL)
                                break;
                        resource_desc(string,DIAG_SEL_TYPE, DSMenu[index]);
                        resinfo[line].text = string;
                        resinfo[line].non_select = ASL_NO;
                        string = string + strlen(string)+1;
                        line++;
                }
                /* Add line to allow reading in Supplemental diskette */
		/* Check to see if there are more than 8MB of memory  */

                if((diag_ipl_source != IPLSOURCE_DISK) && (diag_ipl_source != IPLSOURCE_LAN)){
                        resinfo[line].text=diag_cat_gets(fdes,
                                        SET_DSMENU , MSG_DSMENU_4);
                        resinfo[line].non_select = ASL_NO;
                        line++;
                }

                /* finally add the last line */
                resinfo[line].text=diag_cat_gets(fdes,SET_DSMENU,MSG_DSMENU_E);
                restype.max_index = line;
                /* add in the asterisk field if device has been tested */
        }
        for(index=0,i=selection_entry;index < num_DSM_devices;i++,index++) {
                if(DSMenu[index] == NULL)
                        break;
                if( DSMenu[index]->Asterisk == 'T' )
                        resinfo[i].item_flag = '*';
        }

        /* now display menu */
        if (advancedflg)
                rc = diag_display(DIAG_SEL_ADV, fdes, NULL, DIAG_IO,
                                ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                &restype, resinfo);
        else
                rc = diag_display(DIAG_SEL_CUST, fdes, NULL, DIAG_IO,
                                ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                &restype, resinfo);

        if ( rc == DIAG_ASL_COMMIT) {
                dchosen = DIAG_ITEM_SELECTED(restype);
                if ((exenvflg != EXENV_CONC && diag_ipl_source != IPLSOURCE_DISK
                      && dchosen == 1) ||
                     (exenvflg != EXENV_CONC && dchosen == 2 && diag_ipl_source
                      == IPLSOURCE_DISK) ||
                     (exenvflg == EXENV_CONC && dchosen == 1) ) {
                        basetestflg++;
                        resinfo[dchosen].item_flag = '*';
                }
                else if ( (exenvflg != EXENV_CONC && dchosen == 1) &&
                          (diag_ipl_source == IPLSOURCE_DISK) ){
                        systestflg = SYSTEM_TRUE;
                        resinfo[dchosen].item_flag = '*';
                }
                else if( ((diag_ipl_source != IPLSOURCE_DISK)&&(diag_ipl_source != IPLSOURCE_LAN)) &&
                                dchosen==(restype.max_index-1)){
                        /* prompt for another diskette */
                        next_diskette = DIAG_TRUE;
                        resinfo[dchosen].item_flag = '*';
                } else {
                        if (exenvflg == EXENV_CONC || diag_ipl_source != IPLSOURCE_DISK)
                                dchosen -=2;
                        else
                                dchosen -=3;

                }

                *selection = dchosen;
        }
        return (rc);
}

/*  */
/* NAME: disp_tm_menu
 *
 * FUNCTION: Display the Test Method Selection menu. Return the
 *              item selected.
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *      DIAG_ASL_COMMIT
 *      DIAG_ASL_CANCEL
 *      DIAG_ASL_EXIT
 *
 */

extern ASL_SCR_TYPE dm_menutype;

disp_tm_menu(selection)
int *selection;
{
        int     rc;
        static struct msglist menuTMETHOD[] = {
                {SET_TMETHOD, MSG_TMETHOD_0, },
                {SET_TMETHOD, MSG_TMETHOD_1, },
                {SET_TMETHOD, MSG_TMETHOD_2, },
                {SET_TMETHOD, MSG_TMETHOD_E, },
                (int)NULL,
        };

        *selection = LOOPMODE_NOTLM;

        if (unattendedflg)
                return(DIAG_ASL_COMMIT);

        rc = diag_display(TEST_METHOD, fdes, menuTMETHOD, DIAG_IO,
                                ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                NULL, NULL);

        if( rc == DIAG_ASL_COMMIT )
                if (DIAG_ITEM_SELECTED(dm_menutype) == 2)
                        *selection = LOOPMODE_ENTERLM;

        return(rc);
}

/*  */
/* NAME: disp_dm_menu
 *
 * FUNCTION: Display the Diagnostic Mode Selection menu. Return the
 *              item selected.
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *      DIAG_ASL_COMMIT
 *      DIAG_ASL_CANCEL
 *      DIAG_ASL_EXIT
 *
 */

disp_dm_menu(selection)
int *selection;
{
        int     rc;
        static struct msglist menuDMODE[] = {
                {SET_DMODE, MSG_DMODE_0, },
                {SET_DMODE, MSG_DMODE_2, },
                {SET_DMODE, MSG_DMODE_1, },
                {SET_DMODE, MSG_DMODE_E, },
                (int)NULL,
        };

        if (unattendedflg) {
                *selection = DMODE_REPAIR;
                return(DIAG_ASL_COMMIT);
        }

        if ( (diag_ipl_source != IPLSOURCE_DISK) && (diag_ipl_source != IPLSOURCE_LAN )) {
                menuDMODE[1].msgid = MSG_DMODE_S;
                menuDMODE[2].msgid = MSG_DMODE_PD;
        }

        rc = diag_display(DIAG_MODE_MENU, fdes, menuDMODE, DIAG_IO,
                                ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                NULL, NULL);

        if( rc == DIAG_ASL_COMMIT )
                switch ( DIAG_ITEM_SELECTED(dm_menutype)) {
                        case 1:
                                *selection = DMODE_REPAIR;
                                dmode = DMODE_REPAIR;
                                break;
                        case 2:
                                *selection = DMODE_PD;
                                dmode = DMODE_PD;
                                break;
                }
        return(rc);
}

/*  */
/* NAME: disp_dc_error
 *
 * FUNCTION: Display a diagnostic error
 *
 * NOTES: Error will be displayed only if unattendedflg not set
 *
 * DATA STRUCTURES:
 *
 * RETURNS: NONE
 *
 */

disp_dc_error( error_id, str )
int     error_id;
char    *str;
{
        if ( ! unattendedflg )
                Perror(fdes, ESET, error_id, str );
}
/*  */
/* NAME: disp_ela_pd_menu
 *
 * FUNCTION: Display the Device not supported in Conc mode menu.
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *      DIAG_ASL_COMMIT
 *      DIAG_ASL_CANCEL
 *      DIAG_ASL_EXIT
 *
 */

disp_ela_pd_menu(devptr)
diag_dev_info_t *devptr;
{
        int     rc;
        char    *opstring;
        static struct msglist elamenu[] = {
                {SET_EMODE, MSG_ELAPD_0, },
                {SET_EMODE, MSG_ELAPD_1, },
                {SET_EMODE, MSG_ELAPD_2, },
                {SET_EMODE, MSG_ELAPD_E, },
                (int)NULL,
        };
        static ASL_SCR_INFO     elainfo[ DIAG_NUM_ENTRIES(elamenu) ];
        static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;

        opstring = malloc(2048);
        rc = diag_display(NULL, fdes, elamenu, DIAG_MSGONLY, NULL,
                                        &menutype, elainfo);
        sprintf(opstring, elainfo[0].text, devptr->T_CuDv->name);
        free(elainfo[0].text);
        elainfo[0].text = opstring;
        rc = diag_display(0x801008, fdes, NULL, DIAG_IO,
                                        ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                        &menutype, elainfo);

        if( rc == DIAG_ASL_COMMIT )
                switch ( DIAG_ITEM_SELECTED(menutype)) {
                        case 1:
                                diag_mode = DMODE_PD;
                                break;
                        case 2:
                                rc = DIAG_ASL_CANCEL;
                                break;
                }
        free(opstring);
        return(rc);
}

/*  */
/* NAME: disp_no_test_printf
 *
 * FUNCTION: Display the Device not tested in X environment message.
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: NONE
 *
 */
disp_no_test_printf(int error_id, char *str)
{
       printf("%s %s\n", diag_cat_gets(fdes, INFOSET, TMSGT), str);
       printf("  %s %s device.\n", diag_cat_gets(fdes, ESET, error_id), str);
}

/*  */
/* NAME: log_symptom_strings
 *
 * FUNCTION: log symptom strings.
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: NONE
 *
 */
void log_symptom_strings(char *device_name, char *srn, char *detail_data)
{
	char		crit[255];
	struct CuVPD	*cuvpd_user;
	struct	listinfo c_info;
	VPDBUF	vbuf;
	char	*vptr;
	char	*serialnumber=(char *)NULL;
	char	*modelno=(char *)NULL;
        register int             i,j,op_flg;
        int             rc;
	struct  errdata err_data;

	/* Check to see if user VPD has been entered for the system unit */

	sprintf(crit, "name=%s and vpd_type=%d", SYSUNIT, USER_VPD);
	cuvpd_user = (struct CuVPD *)diag_get_list(CuVPD_CLASS, crit,
			&c_info, 1, 1);
	if(c_info.num == 0)
		return;

	parse_vpd(cuvpd_user->vpd, &vbuf,0);
	for(i=0; i < vbuf.entries; i++){
		vptr=vbuf.vdat[i];
		if(!strncmp(vptr, "*SN", 3)){
			serialnumber = (char *) calloc(1, SN_LEN);
			strncpy(serialnumber, (vptr+4), SN_LEN);
			/* Make sure string is null terminated */
			j=0;
			while ((serialnumber[j] != ' ') &&
			       (j < SN_LEN))
				j++;
			serialnumber[j] = '\0';
		}
		if(!strncmp(vptr, "*TM", 3)){
			modelno = (char *) calloc(1, DEVS_LEN);
			strncpy(modelno, (vptr+4), DEVS_LEN);
			/* Make sure string is null terminated */
			j=0;
			while ((modelno[j] != ' ') &&
			       (j < DEVS_LEN))
				j++;
			modelno[j] = '\0';
		}
	}

	/* Now make sure there is no entry logged  */
	/* against this device within the last     */
	/* 24 hour period.			   */
	sprintf(crit, "-s %s -e %s -N %s -j %X", startdate, enddate,
		     device_name, ERRID_DIAG_REPORT);
	op_flg=INIT;
	rc=error_log_get(op_flg, crit, &err_data);
	if (rc <= 0){ /* No entry found, log this one */
		PROBE_DATA(p, 6);
		p.sskwds[0].sskwd_id = SSKWD_PIDS;
		p.sskwds[0].SSKWD_PTR = BOS_COMP;
		p.sskwds[1].sskwd_id = SSKWD_LVLS;
		p.sskwds[1].SSKWD_PTR = BOS_VERS;
		p.sskwds[2].sskwd_id = SSKWD_PCSS;
		p.sskwds[2].SSKWD_PTR = PCSS_DIAGRPT;
		p.sskwds[3].sskwd_id = SSKWD_SN;
		p.sskwds[3].SSKWD_PTR = serialnumber;
		p.sskwds[4].sskwd_id = SSKWD_DEVS;
		p.sskwds[4].SSKWD_PTR = modelno;
		p.sskwds[5].sskwd_id = SSKWD_SRN;
		p.sskwds[5].SSKWD_PTR = srn;
                p.erecp=(struct err_rec *)calloc(1,sizeof(ERR_REC(ERR_REC_MAX)));
		strcpy(p.erecp->resource_name, device_name);
		p.erecp->error_id = ERRID_DIAG_REPORT;
		strncpy(p.erecp->detail_data, detail_data,ERR_REC_MAX);
		p.erecl=sizeof(ERR_REC(ERR_REC_MAX));
		rc=probe(&p);
		return; /* log one entry, then return*/
	}
	else
		return; /* Do not log this entry */

}
/*   */
/*
 * NAME: wraptitle
 *
 * FUNCTION: Adjust wraparound of text on screen
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 0
 */

/* Title length is 80 chars less 6 digits menu numbers and 1 space */

#define	TITLE_LENGTH	73

wraptitle( string_length, buffer, text )
int     string_length;  /* current length of text in buffer     */
char    *buffer;        /* buffer to append text to             */
char    *text;          /* text to be appended                  */
{
        int     i;
        int     char_positions;

        /* determine if length of text string will fit on one line */
       	char_positions = TITLE_LENGTH - string_length;
        if ( char_positions < strlen(text))  {

        /* dont break the line in the middle of a word */
      	       if( text[char_positions] != ' ')
                     while ( --char_positions )
                           if( text[char_positions] == ' ')
                                   break;
               if ( char_positions == 0 )
                     char_positions = TITLE_LENGTH - string_length;

               for ( i = 0; i < char_positions; i++, buffer++, text++ )
                     *buffer = ( *text == '\n' ) ? ' ' : *text ;
               *buffer++ = '\n';
               while ( *text == ' ' )   /* remove any leading blanks */
                     text++;
               wraptitle( string_length, buffer, text);
         }
         else
              	sprintf(buffer, "%s", text);

         return(0);
}
