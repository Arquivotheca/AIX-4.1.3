static char sccsid[] = "@(#)46  1.21.2.11  src/bos/diag/util/umlc/umlcdisp.c, dsaumlc, bos411, 9428A410j 5/25/94 08:21:50";
/*
 * COMPONENT_NAME: (DUTIL) DIAGNOSTIC UTILITY - umlcdisp.c
 *
 * FUNCTIONS:   disp_load_sys(disk_type)
 *              disp_retry_format()
 *              new_vitem(vptr)
 *              disp_sys_rec(mach)
 *              load_syskeys(buf,kws)
 *              finish_syskeys(buf,kws,extras)
 *              load_vkeys(buf)
 *              disp_terminate()
 *              invalid_format(buf)
 *              disp_popup_message(msg)
 *              chk_4_scsi_dev(vptr)
 *              load_scsi_keys(buf)
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
#include "umlc_msg.h"
#include "diag/class_def.h"
#include "diag/diago.h"
#include "diag/diag.h"
#include "diag/tmdefs.h"
#include "mlc_defs.h"


/* EXTERNAL VARIABLES   */
extern nl_catd          fdes;           /* catalog file descriptor */
extern ASL_SCR_TYPE     dm_menutype;
extern int              change_flg,new_flg;
extern int              fccnt,fbcnt,pncnt;
extern int              output;
extern MACH_REC         machine;
extern char             diagdir[];
extern struct FC_PTR    *fc_ptr;
extern VPD_REC          vprec;

/* EXTERNAL FUNCTIONS */
extern struct PN_PTR *build_newrec();
extern struct FC_PTR *last_fc();
extern void del_fcs(struct FC_PTR *);
extern void del_fbs(struct FB_PTR *);
extern void del_pts(struct PN_PTR *);
extern void write_FC_data();
extern char *diag_cat_gets(nl_catd, int, int);
extern void* calloc_mem(int, int);
extern void* malloc_mem(int);

/* LOCAL VARIABLES */

typedef struct {
                char dat[80];
        } BUF;

struct scrn {
                int     cnt;
                int     cntr;
                BUF     *lines;
        } scrn;

MACH_REC        save_mach;

/*  ANSI C FUNCTION PROTOTYPING FOLLOWS */
int disp_load_sys(char *);
int disp_retry_format(int);
int new_vitem(struct VP_PTR *);
int disp_sys_rec(MACH_REC *);
void load_syskeys(LBUFF *, char [][5]);
void finish_syskeys(LBUFF *, char [][5], int);
void load_vkeys(LBUFF *);
void disp_terminate(void);
int invalid_format(LBUFF *);
void disp_popup_message(int);
int chk_4_scsi_dev(struct VP_PTR *);
void load_scsi_keys(LBUFF *);

/*  */
/*
 * NAME: disp_load_sys
 *
 * FUNCTION: This unit displays the message to insert diskette
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 0 - no error
 *          1 - exit menu
 */

int disp_load_sys(char *disk_type)
{
        int rc;
        static struct msglist   menulist[] = {
                { MSET, MINSTALL_TITLE },
                { MSET, MSDD },
                { MSET, MINSTALL_LAST },
                { (int )NULL, (int )NULL }
        };

        rc = diag_display(LOAD_SYS, fdes, menulist, DIAG_IO,
                          ASL_DIAG_ENTER_HELP_SC, NULL, NULL);

        while (rc == DIAG_ASL_HELP)
          {
          diag_hmsg(fdes, HELP_SET, HELP_802604, NULL);

          rc = diag_display(LOAD_SYS, fdes, menulist, DIAG_IO,
                            ASL_DIAG_ENTER_HELP_SC, NULL, NULL);
          }

        return ((rc == DIAG_ASL_ENTER) ? 0 : 1);
}

/*  */
/*
 * NAME: disp_retry_format
 *
 * FUNCTION: This unit displays a message when the diskette is unwritable.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 0 - retry writing
 *          1 - exit menu
 */

int disp_retry_format(int disk_flag)
{
        int rc;
        int selection = 0;
        static struct msglist   menulist[] = {
                { MSET, MFORMAT_UPDATE_TITLE },
                { MSET, YES_OPTION },
                { MSET, NO_OPTION },
                { MSET, MLASTLINE },
                { (int )NULL, (int )NULL}
        };

        rc = diag_display(RETRY_FORMAT, fdes, menulist, DIAG_IO,
                          ASL_DIAG_LIST_CANCEL_EXIT_HELP_SC,
                          NULL, NULL);

        while (rc == DIAG_ASL_HELP)
          {
          diag_hmsg(fdes, HELP_SET, HELP_802605, NULL);
          rc = diag_display(RETRY_FORMAT, fdes, menulist, DIAG_IO,
                            ASL_DIAG_LIST_CANCEL_EXIT_HELP_SC,
                            NULL, NULL);
          }

        selection = DIAG_ITEM_SELECTED(dm_menutype);

        if (rc == DIAG_ASL_CANCEL || rc == DIAG_ASL_EXIT || selection == 2)
                return (1);

        return (0);
}
/*  */
/*
 * NAME: new_vitem
 *
 * FUNCTION: Create VPD entry for new part.
 *
 * RETURNS:
 */

int
new_vitem( struct VP_PTR *vptr )
{
struct PN_PTR *pptr;

int     i;
int     rc = -1;
int     rval;
char    *fdat[8];
LBUFF   junk[8];


        for(i=1; i<8;i++) {
                fdat[i] = (char *) calloc_mem(1,FIELD_SIZE);
        }

/* copy all vpd fields to the menu display field, stripping off the newline */

        if(*vptr->data->pn != '$' && *vptr->data->pn != '\0')
                strncpy(fdat[1],vptr->data->pn,strlen(vptr->data->pn)-1);
        if(*vptr->data->pl != '$' && *vptr->data->pl != '\0')
                strncpy(fdat[2],vptr->data->pl,strlen(vptr->data->pl)-1);
        if(*vptr->data->ax != '$' && *vptr->data->ax != '\0')
                strncpy(fdat[3],vptr->data->ax,strlen(vptr->data->ax)-1);
        if(*vptr->data->ec != '$' && *vptr->data->ec != '\0')
                strncpy(fdat[4],vptr->data->ec,strlen(vptr->data->ec)-1);
        if(*vptr->data->sn != '$' && *vptr->data->sn != '\0')
                strncpy(fdat[5],vptr->data->sn,strlen(vptr->data->sn)-1);
        if(*vptr->data->lo != '$' && *vptr->data->lo != '\0')
                strncpy(fdat[6],vptr->data->lo,strlen(vptr->data->lo)-1);
        if(*vptr->data->ds != '$' && *vptr->data->ds != '\0')
                strncpy(fdat[7],vptr->data->ds,strlen(vptr->data->ds)-1);

        for ( i=1; i < 8; i++) {
                memset(&junk[i],'\0',FIELD_SIZE);
        }

        for (i=1; i<8; i++)
                strcpy(&junk[i], fdat[i]);

        load_vkeys(&junk[1]);
        pptr = build_newrec(vptr, &junk[1]);
        build_mes_hist(pptr);

        for(i=0; i<8;i++)
                free(fdat[i]);

        return(rc);
}
/*  */
/*
 * NAME: disp_sys_rec
 *
 * FUNCTION: Display machine record data and accept any input changes.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 0 to commit
 *          -1 to cancel
 */

int
disp_sys_rec( MACH_REC *mach)
{
int i, j, k;
int rc;
int rcf;
int entry=0;
int cnt;
char *tmp;
char buf[16][50];
char kwbuf[16][5];
LBUFF *dat;
LBUFF junk[M_LINES+1];
char *fdat[M_LINES+1];
char tempstr[FIELD_SIZE];

static ASL_SCR_INFO *menuinfo;
static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;

        dat = (LBUFF *) mach->d1;
        memset(buf,0,sizeof(buf));
        memset(kwbuf,0,sizeof(kwbuf));

        menuinfo=(ASL_SCR_INFO *) calloc_mem(M_LINES+2,sizeof(ASL_SCR_INFO));

        menuinfo[entry++].text= diag_cat_gets(fdes,SSET,SYSTITLE);
        menuinfo[entry++].text= diag_cat_gets(fdes,SSET,SE_MSG);
        menuinfo[entry++].text= diag_cat_gets(fdes,SSET,TM_MSG);
        menuinfo[entry++].text= diag_cat_gets(fdes,SSET,T2_MSG);
        menuinfo[entry++].text= diag_cat_gets(fdes,SSET,L1_MSG);
        menuinfo[entry++].text= diag_cat_gets(fdes,SSET,L2_MSG);
        menuinfo[entry++].text= diag_cat_gets(fdes,SSET,L3_MSG);
        menuinfo[entry++].text= diag_cat_gets(fdes,SSET,L4_MSG);
        menuinfo[entry++].text= diag_cat_gets(fdes,SSET,L5_MSG);
        menuinfo[entry++].text= diag_cat_gets(fdes,SSET,L6_MSG);
        menuinfo[entry++].text= diag_cat_gets(fdes,SSET,SY_MSG);
        menuinfo[entry++].text= diag_cat_gets(fdes,SSET,CN_MSG);
        menuinfo[entry++].text= diag_cat_gets(fdes,SSET,EA_MSG);
        menuinfo[entry].text = diag_cat_gets(fdes,USET, MNEWLAST);
        menutype.max_index = entry;

        if(mach->serial[1] != 'S') {
                if(vprec.mr.serial[1] == 'S')
                        strcpy(mach->serial, vprec.mr.serial);
                set_install_date();
                strcpy(mach->install,machine.install);
                strcpy(mach->build,machine.install);
                mach->build[4]='B';
                mach->seq_no[4]='0';
        }

        if ((mach->type[1] != 'T') && (vprec.mr.type[1] == 'T'))
                strcpy(mach->type, vprec.mr.type);


        dat = (LBUFF *) mach;
        ++dat;
        for(i=1; i < entry;i++) {
                fdat[i] = (char *) calloc_mem(1,FIELD_SIZE);
                memset(&junk[i],0,FIELD_SIZE);
                if(strlen(dat) > 1)
                        strncpy(fdat[i],dat,strlen(dat)-1);
                if(i != 2)      /* TM separated into 2 fields */
                        dat++;
                menuinfo[i].entry_type = ASL_TEXT_ENTRY;
                menuinfo[i].changed = ASL_NO;
                menuinfo[i].cur_value_index = 0;
                menuinfo[i].entry_size = FIELD_SIZE-5;
                menuinfo[i].data_value = &junk[i].dat[4];

                if(i == 3) {      /* Get model type from TM entry */
                        fdat[2][8]='\0';
                        menuinfo[i].disp_values = &fdat[i][9];
                }
                else
                        menuinfo[i].disp_values = &fdat[i][4];
        }

        if(mach->serial[1] != 'S') {
                menuinfo[1].entry_type = ASL_TEXT_ENTRY;
                menuinfo[1].required = ASL_YES;
        }
        else menuinfo[1].entry_type = ASL_NO_ENTRY;

        rc = -1;

        while ( rc != DIAG_ASL_CANCEL && rc != DIAG_ASL_EXIT &&
                                         rc != DIAG_ASL_COMMIT ) {
                rc = diag_display(SYS_RECORD, fdes, NULL, DIAG_IO,
                          ASL_DIAG_DIALOGUE_SC, &menutype, menuinfo);

                if ( rc == DIAG_ASL_HELP)
                        diag_hmsg(fdes, HELP_SET, HELP_802601, NULL);

                if ( rc == DIAG_ASL_COMMIT ) {
                        for(i=1; i < entry ; i++) {
                                if(menuinfo[i].entry_type == ASL_NO_ENTRY) {
                                        strcpy(junk[i].dat, fdat[i]);
                                }
                        }
                        load_syskeys(&junk[1], kwbuf);

                        /* Uppercase some entries */
                        for(i=1;i < entry; i++) {
                                switch(i) {
                                    case 1:  /* Serial Number */
                                    case 2:  /* Machine Type */
                                    case 3:  /* Model Number */
                                    case 10: /* Specify Codes */
                                        for (j=1;j<(strlen(junk[i].dat));j++)
                                            junk[i].dat[j] = toupper(junk[i].dat[j]);
                                        break;
                                    default :
                                        break;
                                }
                        }

                        if(invalid_format(&junk[1])) {
                                rc=-1;   /* Re-display Sys Record */
                        }
                        else {
                                finish_syskeys(&junk[1], kwbuf, entry-13);
                                dat = (LBUFF *) mach;
                                ++dat;
                                for(i=1;i < entry;i++) {

                                        /* Make sure that the field length does not */
                                        /* exceed max field length.  If it does,    */
                                        /* truncate excess data.                    */
                                        if(strlen(junk[i].dat) > FIELD_SIZE) {
                                                memset(tempstr,'\0',FIELD_SIZE);
                                                for (k=0;k<FIELD_SIZE-2;k++) {
                                                        tempstr[k] = junk[i].dat[k];
                                                } /* endfor */
                                                tempstr[k] = '\n';
                                                strcpy(junk[i].dat, tempstr);
                                        }

                                        if(i == 2) {   /* Re-join TM fields */
                                                strncpy(&junk[0],junk[i].dat,8);
                                                memset(tempstr,'\0',FIELD_SIZE);
                                                sprintf(tempstr,"-%c%c%c\n",
                                                        junk[i+1].dat[4],
                                                        junk[i+1].dat[5],
                                                        junk[i+1].dat[6] );
                                                strcat(&junk[0],tempstr);
                                                strcpy(dat,&junk[0]);
                                                i++;   /* Skip model type */
                                        }
                                        else
                                                strcpy(dat,&junk[i]);
                                        ++dat;
                                }
                        }
                }
        }
        for(i = 1; i < entry; i++)
                free(fdat[i]);
        free(menuinfo);
        return((rc == DIAG_ASL_COMMIT) ? 0 : -1);
}

/*  */
/*
 * NAME: load_syskeys
 *
 * FUNCTION: Insert the machine record keywords into the edited fields.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */

void load_syskeys(LBUFF *buf, char kws[][5])
{
int cnt;
int rval=0;
int fmask = 0x0;

        for(cnt=0;cnt<12; cnt++,buf++) {
                switch(cnt) {
                        case 0: if(buf->dat[4] != '\0') {
                                        buf->dat[0] = '*';
                                        buf->dat[1] = 'S';
                                        buf->dat[2] = 'E';
                                        buf->dat[3] = ' ';
                                }
                                break;
                        case 1: if(buf->dat[4] != '\0') {
                                        buf->dat[0] = '*';
                                        buf->dat[1] = 'T';
                                        buf->dat[2] = 'M';
                                        buf->dat[3] = ' ';
                                }
                                break;
                        case 2: if(buf->dat[4] != '\0') {
                                        buf->dat[0] = '*';
                                        buf->dat[1] = 'T';
                                        buf->dat[2] = 'M';
                                        buf->dat[3] = ' ';
                                }
                                break;
                        case 3: if(buf->dat[4] != '\0') {
                                        buf->dat[0] = '*';
                                        buf->dat[1] = 'L';
                                        buf->dat[2] = '1';
                                        buf->dat[3] = ' ';
                                }
                                break;
                        case 4: if(buf->dat[4] != '\0') {
                                        buf->dat[0] = '*';
                                        buf->dat[1] = 'L';
                                        buf->dat[2] = '2';
                                        buf->dat[3] = ' ';
                                }
                                break;
                        case 5: if(buf->dat[4] != '\0') {
                                        buf->dat[0] = '*';
                                        buf->dat[1] = 'L';
                                        buf->dat[2] = '3';
                                        buf->dat[3] = ' ';
                                }
                                break;
                        case 6: if(buf->dat[4] != '\0') {
                                        buf->dat[0] = '*';
                                        buf->dat[1] = 'L';
                                        buf->dat[2] = '4';
                                        buf->dat[3] = ' ';
                                }
                                break;
                        case 7: if(buf->dat[4] != '\0') {
                                        buf->dat[0] = '*';
                                        buf->dat[1] = 'L';
                                        buf->dat[2] = '5';
                                        buf->dat[3] = ' ';
                                }
                                break;
                        case 8: if(buf->dat[4] != '\0') {
                                        buf->dat[0] = '*';
                                        buf->dat[1] = 'L';
                                        buf->dat[2] = '6';
                                        buf->dat[3] = ' ';
                                }
                                break;
                        case 9: if(buf->dat[4] != '\0') {
                                        buf->dat[0] = '*';
                                        buf->dat[1] = 'S';
                                        buf->dat[2] = 'Y';
                                        buf->dat[3] = ' ';
                                }
                                break;
                        case 10: if(buf->dat[4] != '\0') {
                                        buf->dat[0] = '*';
                                        buf->dat[1] = 'C';
                                        buf->dat[2] = 'N';
                                        buf->dat[3] = ' ';
                                }
                                break;
                        case 11: if(buf->dat[4] != '\0') {
                                        buf->dat[0] = '*';
                                        buf->dat[1] = 'E';
                                        buf->dat[2] = 'A';
                                        buf->dat[3] = ' ';
                                }
                                break;
                        default: break;
                        }
                }
}
/*  */
/*
 * NAME: finish_syskeys
 *
 * FUNCTION: Insert the machine record keywords into the edited fields.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */

void finish_syskeys(LBUFF *buf, char kws[][5], int extras)
{
int cnt;
int rval=0;
int fmask = 0x0;

        for(cnt=0;cnt<12; cnt++,buf++) {
                switch(cnt) {
                        case 0: strcat(buf,"\n");
                                break;
                        case 1: strcat(buf,"\n");
                                break;
                        case 2: strcat(buf,"\n");
                                break;
                        case 3: strcat(buf,"\n");
                                break;
                        case 4: if(buf->dat[4] != '\0') {
                                        strcat(buf,"\n");
                                }
                                break;
                        case 5: if(buf->dat[4] != '\0') {
                                        strcat(buf,"\n");
                                }
                                break;
                        case 6: if(buf->dat[4] != '\0') {
                                        strcat(buf,"\n");
                                }
                                break;
                        case 7: if(buf->dat[4] != '\0') {
                                        strcat(buf,"\n");
                                }
                                break;
                        case 8: if(buf->dat[4] != '\0') {
                                        strcat(buf,"\n");
                                }
                                break;
                        case 9: if(buf->dat[4] != '\0') {
                                        strcat(buf,"\n");
                                }
                                break;
                        case 10: if(buf->dat[4] != '\0') {
                                        strcat(buf,"\n");
                                }
                                break;
                        case 11: if(buf->dat[4] != '\0') {
                                        strcat(buf,"\n");
                                }
                                break;
                        default: break;
                        }
                }
}
/*  */
/*
 * NAME: load_vkeys
 *
 * FUNCTION:  this function inserts the keyword into each nonblank field
 *
 * NOTES:
 *      PN's are mandatory so a dummy value is added if this field is null.
 *
 * RETURNS: NONE
 */

void load_vkeys( LBUFF *buf)
{
int cnt;

        for(cnt=1;cnt<8; cnt++,buf++) {
                switch(cnt) {
                        case 1: if(buf->dat[4] != '\0') {
                                        buf->dat[0] = '*';
                                        buf->dat[1] = 'P';
                                        buf->dat[2] = 'N';
                                        buf->dat[3] = ' ';
                                        strcat(buf,"\n");
                                }
                                else  strcpy(buf,"*PN 00000000\n");
                                break;
                        case 2: if(buf->dat[4] != '\0') {
                                        buf->dat[0] = '*';
                                        buf->dat[1] = 'P';
                                        buf->dat[2] = 'L';
                                        buf->dat[3] = ' ';
                                        strcat(buf,"\n");
                                }
                                break;
                        case 3: if(buf->dat[4] != '\0') {
                                        buf->dat[0] = '*';
                                        buf->dat[1] = 'A';
                                        buf->dat[2] = 'X';
                                        buf->dat[3] = ' ';
                                        strcat(buf,"\n");
                                }
                                break;
                        case 4: if(buf->dat[4] != '\0') {
                                        buf->dat[0] = '*';
                                        buf->dat[1] = 'E';
                                        buf->dat[2] = 'C';
                                        buf->dat[3] = ' ';
                                        strcat(buf,"\n");
                                }
                                break;
                        case 5: if(buf->dat[4] != '\0') {
                                        buf->dat[0] = '*';
                                        buf->dat[1] = 'S';
                                        buf->dat[2] = 'N';
                                        buf->dat[3] = ' ';
                                        strcat(buf,"\n");
                                }
                                break;
                        case 6: if(buf->dat[4] != '\0') {
                                        buf->dat[0] = '*';
                                        buf->dat[1] = 'L';
                                        buf->dat[2] = 'O';
                                        buf->dat[3] = ' ';
                                        strcat(buf,"\n");
                                }
                                break;
                        case 7: if(buf->dat[4] != '\0') {
                                        buf->dat[0] = '*';
                                        buf->dat[1] = 'D';
                                        buf->dat[2] = 'S';
                                        buf->dat[3] = ' ';
                                        strcat(buf,"\n");
                                }
                                break;
                        default:
                                break;
                        }
                }
}
/*  */
/*
 * NAME: disp_terminate
 *
 * FUNCTION: This function displays the menu whenever the user selects EXIT
 *           or CANCEL.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */

void disp_terminate(void)
{
        int rc = -1;

        static struct msglist   menulist[] = {
                { MSET, MINSTALL_TITLE },
                { MSET, EXIT_CANCEL },
                { MSET, RETURN },
                { MSET, EXIT },
                { (int )NULL, (int )NULL }
        };

        while ( (rc != DIAG_ASL_CANCEL) &&
                (rc != DIAG_ASL_EXIT) &&
                (rc != DIAG_ASL_ENTER) ) {

                rc = diag_display(TERMINATE, fdes, menulist, DIAG_IO,
                               ASL_DIAG_ENTER_SC, NULL, NULL);

        }

        if(rc == DIAG_ASL_EXIT)
                genexit(0);

}
/*  */
/*
 * NAME: invalid_format
 *
 * FUNCTION: Ensure that the System Record fields are in the correct format.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */

int invalid_format(LBUFF *buf)
{
int i,k;
int rc;
int cnt;
char *token, *tokentmp;
char tdat[FIELD_SIZE];
int problem=0;

        for(cnt=0;cnt<4; cnt++,buf++) {
                strcpy(tdat,buf->dat);
                k=strlen(buf->dat);
                tokentmp=strtok(tdat," \n");
                token=strtok(NULL," \n");  /* Get second token -- the data */
                switch(cnt) {
                        case 0:
                                if ((strlen(token) == 8) && (token[2] == '-')) {
                                        for(i=3;i<=9;i++)
                                                token[i-1] = token[i];
                                        strcpy(buf->dat,tokentmp);
                                        strcat(buf->dat," ");
                                        strcat(buf->dat,token);
                                } /* endif */
                                if ( ! ((strlen(token) == 7) &&
                                        (isdigit(token[0]) != 0) &&
                                        (isdigit(token[1]) != 0) &&
                                        (isalnum(token[2]) != 0) &&
                                        (isalnum(token[3]) != 0) &&
                                        (isalnum(token[4]) != 0) &&
                                        (isalnum(token[5]) != 0) &&
                                        (isalnum(token[6]) != 0)))  {
                                                problem=1;
                                                disp_popup_message(SE_ERR);
                                }
                                break;
                        case 1:
                                if ( ! ((strlen(token) == 4) &&
                                        (isalnum(token[0]) != 0) &&
                                        (isalnum(token[1]) != 0) &&
                                        (isalnum(token[2]) != 0) &&
                                        (isalnum(token[3]) != 0)) &&
                                        (problem == 0))  {
                                                problem=1;
                                                disp_popup_message(TYPE_ERR);
                                }
                                break;
                        case 2:
                                if ( ! ((strlen(token) == 3) &&
                                        (isalnum(token[0]) != 0) &&
                                        (isalnum(token[1]) != 0) &&
                                        (isalnum(token[2]) != 0)) &&
                                        (problem == 0))  {
                                                problem=1;
                                                disp_popup_message(MODEL_ERR);
                                }
                                break;
                        case 3:
                                if ( ! ((strlen(token) > 0)) &&
                                      (problem == 0))  {
                                                problem=1;
                                                disp_popup_message(NAME_ERR);
                                }
                                break;
                        default: break;
                }
        }
        return(problem);
}
/*  */
/*
 * NAME: disp_popup_message
 *
 * FUNCTION: Display an error popup
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */

void disp_popup_message(int message)
{
        diag_asl_msg(diag_cat_gets(fdes, ESET, message));
}
/*  */
/*
 * NAME: chk_4_scsi_dev
 *
 * FUNCTION: Checks to see if the device is a SCSI device without a *LO
 *              keyword.  If so, a menu is displayed prompting the user
 *              to enter a valid SCSI location.
 *
 * RETURNS:
 */

int
chk_4_scsi_dev( struct VP_PTR *vptr )
{

int     i;
int     rc = -1;
int     rval;
char    *tstr[7];
char    *fdat[2];
LBUFF   junk[8];
char    srch[80];
char    srch2[30];
struct  listinfo  cinfo;
struct  CuDv      *cudv;
char    *token;
char    *lv;
char    blank[3];
char    dv_name[NAMESIZE];
nl_catd dv_catd;
int     Set_num;
int     Msg_num;
char    *dv_msg;
char    msgstr[512];

static struct msglist menulist[] = {
        { USET, MNEWTITLE },
        { SSET, LO_MSG },
        { USET, MNEWLAST },
        { (int )NULL, (int )NULL}
};
static ASL_SCR_INFO menuinfo[DIAG_NUM_ENTRIES(menulist)];
static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;


        /* If lo keyword is not null and is either 'IN' or 'EX', then return */
        if( (*vptr->p_mate->data->lo != '$' &&
             *vptr->p_mate->data->lo != '\0') ) {
                if( (vptr->p_mate->data->lo[4] == 'I' &&
                     vptr->p_mate->data->lo[5] == 'N') ||
                    (vptr->p_mate->data->lo[4] == 'E' &&
                     vptr->p_mate->data->lo[5] == 'X') )
                                return(2);  /* marked w/ valid LO */
        }

        /* Get CuDv record for this device by part location */
        strcpy(srch, "chgstatus != 3 and location=");
        sprintf(srch2, "%s ", vptr->data->pl);
        token = (char *)strtok(srch2," \n\0");
        token = (char *)strtok(NULL," \n\0");
        strcat(srch,token);
        cudv = get_CuDv_list(CuDv_CLASS, srch, &cinfo, 1, 2);
        if (cudv == (struct CuDv *) -1)
                return(-1);

        if (cinfo.num >= 1)  {
                lv = cudv->PdDvLn_Lvalue;
                token = (char *)strtok(lv,"/");
                token = (char *)strtok(NULL,"/");
                if( ! (strcmp("scsi",token) == 0) ) {     /* Not SCSI */
                        odm_free_list(cudv, &cinfo);
                        return(2);
                }
        }
        else
                return(3);  /* Couldn't get CuDv entry */

        /* At this point, we know we have a SCSI device w/o valid LO keyword */

        rc = diag_display(NULL,fdes,menulist,DIAG_MSGONLY,
                                NULL,&menutype,menuinfo);

        strcpy(dv_name, cudv->PdDvLn->catalog);    /* Get the catalog name */
        dv_catd = diag_device_catopen(dv_name, 0); /* Open the device cat */
        Set_num = cudv->PdDvLn->setno;             /* Get set number */
        Msg_num = cudv->PdDvLn->msgno;             /* Get message number */

        dv_msg = (char *) malloc_mem (255*sizeof(char)); /* Get the message */
        dv_msg = (char *) diag_device_gets(dv_catd, Set_num, Msg_num, "n/a");

        for(i=1;i<7;i++) {
                tstr[i] = (char *) calloc_mem(1,FIELD_SIZE);
        }

        if(*vptr->data->pn != '$' && *vptr->data->pn != '\0')
                strncpy(tstr[1],vptr->data->pn,strlen(vptr->data->pn)-1);
        if(*vptr->data->pl != '$' && *vptr->data->pl != '\0')
                strncpy(tstr[2],vptr->data->pl,strlen(vptr->data->pl)-1);
        if(*vptr->data->ax != '$' && *vptr->data->ax != '\0')
                strncpy(tstr[3],vptr->data->ax,strlen(vptr->data->ax)-1);
        if(*vptr->data->ec != '$' && *vptr->data->ec != '\0')
                strncpy(tstr[4],vptr->data->ec,strlen(vptr->data->ec)-1);
        if(*vptr->data->sn != '$' && *vptr->data->sn != '\0')
                strncpy(tstr[5],vptr->data->sn,strlen(vptr->data->sn)-1);
        if(*vptr->data->ds != '$' && *vptr->data->ds != '\0')
                strncpy(tstr[6],vptr->data->ds,strlen(vptr->data->ds)-1);

        sprintf(msgstr, menuinfo[0].text, dv_msg, &tstr[1][4], &tstr[2][4],
                &tstr[3][4], &tstr[4][4], &tstr[5][4], &tstr[6][4]);
        free(menuinfo[0].text);
        menuinfo[0].text = (char *) malloc_mem (strlen(msgstr)+1);
        strcpy(menuinfo[0].text, msgstr);

        free(dv_msg);                             /* Free memory for dv_msg */
        catclose(dv_catd);                        /* Close the device cat */

        fdat[1] = (char *) calloc_mem(1,FIELD_SIZE);


/* The following code has been removed, as manufacturing is currently not
 * putting any *LO data into VPD for any devices.   If LO becomes used in
 * the future, this code should be uncommented.
 *
 **      if(*vptr->data->lo != '$' && *vptr->data->lo != '\0')
 **              strncpy(fdat[1],vptr->data->lo,strlen(vptr->data->lo)-1);
 *
 */


        memset(&junk[1],'\0',FIELD_SIZE);

        /* For SCSI part location, check the CuDv record returned to see */
        /* if the subclass = 'scsi'.  If so, and if the user has not     */
        /* previously defined the SCSI part location, force the user to  */
        /* choose one; otherwise, make the field non-editable.           */
        menuinfo[1].op_type = ASL_RING_ENTRY;
        menuinfo[1].required = ASL_YES;
        menuinfo[1].entry_type = ASL_NO_ENTRY;
        menuinfo[1].disp_values = "??,IN,EX";
        strcpy(blank,"??");
        menuinfo[1].data_value = blank;
        menuinfo[1].entry_size = 2;
        menuinfo[1].changed = ASL_NO;
        menuinfo[1].cur_value_index = 0;
        menuinfo[1].default_value_index = 0;

        /* Even if user CANCELS/EXITS force the new     */
        /* item to be accepted and continue on.         */
        rc = -1;
        while (rc != DIAG_ASL_COMMIT) {
                rc = diag_display(NEW_VITEM, fdes, NULL, DIAG_IO,
                                  ASL_DIAG_DIALOGUE_SC, &menutype, menuinfo);
                if (rc == DIAG_ASL_HELP)
                        diag_hmsg(fdes, HELP_SET, HELP_802603, NULL);
                else if(menuinfo[1].changed != ASL_YES) {
                        if(rc == DIAG_ASL_COMMIT) {
                                disp_popup_message(LO_ERR); /* must enter LO */
                                rc = -1;
                        }
                        else {
                                disp_terminate();
                                rc = -1;
                        }
                }
                else if((rc == DIAG_ASL_CANCEL) || (rc == DIAG_ASL_EXIT)) {
                        disp_terminate();
                        rc=-1;
                }
        }

        strcpy(fdat[1],"*LO ");
        strcat(fdat[1],menuinfo[1].data_value);
        strcpy(&junk[1],fdat[1]);

        load_scsi_keys(&junk[1]);
        strcpy(vptr->data->lo,&junk[1]);  /* Put LO into VPD */

        free(fdat[1]);
        free(menuinfo);
        odm_free_list(cudv, &cinfo);

        return(rc);
}
/*  */
/*
 * NAME: load_scsi_keys
 *
 * FUNCTION:  this function inserts the keyword into each nonblank field
 *
 * NOTES:
 *      PN's are mandatory so a dummy value is added if this field is null.
 *
 * RETURNS: NONE
 */

void load_scsi_keys( LBUFF *buf)
{
int cnt;

        buf->dat[0] = '*';
        buf->dat[1] = 'L';
        buf->dat[2] = 'O';
        buf->dat[3] = ' ';
        strcat(buf,"\n");
}
