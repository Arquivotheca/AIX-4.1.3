static char sccsid[] = "@(#)34  1.20  src/bos/diag/util/ucfgdev/ucfgdev.c, dsaucfgdev, bos41J 3/9/95 12:52:05";
/*
 * COMPONENT_NAME: (DUTIL) Diagnostic Utility - UCFGDEV
 *
 * FUNCTIONS:   main
 *              Gio_run_method
 *              Gio_get_selected_NSIO
 *              Gio_Add_menu
 *              Gio_del_confirm
 *              Gio_Add_confirm
 *              Gio_NSIO_is_gio_or_rs232
 *              Gio_Add_device
 *              Gio_status
 *              Gio_child_is_cfg
 *              Gio_get_desc
 *              action
 *              Gio_update_option
 *              Gio_display
 *              do_error
 *              int_handler
 *              genexit
 *              Gio_syscons_check
 *              Gio_device_install
 *              Gio_get_loc
 *              Gio_tty_name
 *              Gio_tty_cleanup
 *              Gio_gio_name
 *              Configure_parents
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <signal.h>
#include <sys/cfgdb.h>
#include <sys/mdio.h>
#include <diag/class_def.h>
#include <diag/diago.h>
#include <diag/diag.h>
#include <diag/diag_define.h>
#include "ucfgdev_msg.h"
#include <locale.h>
#include <nl_types.h>

/* AIX system commands used by this program */

#define GIO_DSP_FMT     "  %-16.16s %-16.16s %s"
#define MAX_CHAR        ASL_DIALOGUE_TEXT_SIZE

#define UCFGGIO_MENU_TOP        0x802140
#define UCFGGIO_MENU_LIST       0x802141
#define UCFGGIO_MENU_ADD_D      0x802142
#define UCFGGIO_MENU_ADD_L      0x802143
#define UCFGGIO_MENU_DEL_D      0x802144
#define UCFGGIO_MENU_DEL_L      0x802145

/* gio_type value */
#define GIO_DIAL        1
#define GIO_LPFK        2

/* maximum levels for configuring parents */
#define MAXLEVELS       4

/* device status */
#define Available       1

/* TRUE/FALSE definitions */
#define FALSE           0
#define TRUE            1

/* EXTERNAL VARIABLES   */
extern ASL_SCR_TYPE dm_menutype;

nl_catd         fdes;                           /* catalog file descriptor  */

struct msglist menu_list[] =
    {
        {GIO_SET, GIO_TITLE},
        {GIO_SET, GIO_LIST},
        {GIO_SET, GIO_ADD_DIAL},
        {GIO_SET, GIO_ADD_LPFK},
        {GIO_SET, GIO_RM_DIAL},
        {GIO_SET, GIO_RM_LPFK},
        {GIO_SET, GIO_LAST},
        {(int )NULL, (int )NULL}
    };

/* action codes passed into action() */
#define GIO_ACT_LIST    0
#define GIO_ACT_ADD_D   1
#define GIO_ACT_ADD_L   2
#define GIO_ACT_DEL_D   3
#define GIO_ACT_DEL_L   4

#define RS232_WAS_NULL  0       /* indicate rs232 was not def. or config */
#define RS232_WAS_CFG   1       /* rs232 previously configured on NSIO */
#define RS232_CFG_BY_SA 2       /* rs232 was configured on NSIO by this SA */

struct action_ctl {
        long    menu_no;
        long    scrtype;
        unsigned short setid;
        unsigned short first_msgid;
        unsigned short last_msgid;
        unsigned short null_msgid;
        unsigned short  run_err; /* error msgid of odm_run_method */
        char    *criteria;
        char    *method;
        char    *option;
        char    *method2;
        char    *option2;
} action_tbl[5] = {
        { /* list all */
                UCFGGIO_MENU_LIST,ASL_DIAG_ENTER_SC,GIO_LST_SET,
                GIO_LST_TITLE, GIO_LST_LAST, GIO_NULL_DEVICE, 0,
                "PdDvLn like */sgio/* and status=1 and chgstatus !=3",NULL,NULL,
                NULL,NULL
        },
        { /* add dial */
                UCFGGIO_MENU_ADD_D,ASL_DIAG_LIST_CANCEL_EXIT_SC,GIO_ADD_SET,
                GIO_ADD_TITLE_D,GIO_ADD_LAST,NULL,MKDEV_ERR,
             "name like dials* and status=1 and chgstatus != 3",
             MKDEV,"-c tty -t tty -s rs232 -p '%s' -w '%s'",
             MKDEV,"-s sgio -t 'dials' -a ttydevice='%s'"
        },
        { /* add lpfk */
                UCFGGIO_MENU_ADD_L,ASL_DIAG_LIST_CANCEL_EXIT_SC,GIO_ADD_SET,
                GIO_ADD_TITLE_L,GIO_ADD_LAST,NULL,MKDEV_ERR,
             "name like lpfk* and status=1 and chgstatus != 3",
             MKDEV,"-c tty -t tty -s rs232 -p '%s' -w '%s'",
             MKDEV,"-s sgio -t 'lpfkeys' -a ttydevice='%s'"
        },
        { /* delete dial */
                UCFGGIO_MENU_DEL_D,ASL_DIAG_LIST_CANCEL_EXIT_SC,GIO_DEL_SET,
                GIO_DEL_TITLE_D,GIO_DEL_LAST,GIO_NULL_DIAL,RMDEV_ERR,
                "name like dials* and status=1 and chgstatus !=3",
                RMDEV,"-l %s -d",NULL,NULL
        },
        { /* delete lpfk */
                UCFGGIO_MENU_DEL_L,ASL_DIAG_LIST_CANCEL_EXIT_SC,GIO_DEL_SET,
                GIO_DEL_TITLE_L,GIO_DEL_LAST,GIO_NULL_LPFK,RMDEV_ERR,
                "name like lpfk* and status=1 and chgstatus !=3",
                RMDEV,"-l %s -d",NULL,NULL
        }
};

#define GIO_MAXSIO         2       /* total native serial adapters allowed */
#define GIO_DEV_NULL       1
#define GIO_DEV_RS232      2       /* tty, lp, */
#define GIO_DEV_RS232_NULL 3       /* RS232 | NULL */
#define GIO_DEV_GIO        4       /* dial or LPFK */

typedef struct {
        char    name[NAMESIZE+1]; /*  device names */
        char    location[NAMESIZE+1]; /* location */
        char    desc[MAX_CHAR+1]; /* description */
        char    cfg_child[NAMESIZE+1]; /* configured child device name */
        char    ch_location[NAMESIZE+1]; /* child dev. location */
        char    ch_desc[MAX_CHAR+1]; /* child dev. description */
        char    connwhere[NAMESIZE+1];
        short   status;
        short   ch_type; /* child device type */
        struct  restore_rs232 {
                short   org_status; /* status of rs232 before unconfigure */
                char    name[NAMESIZE+1];
        } restore_info;
} Gio_Serial ;
Gio_Serial Gio_NSIO[GIO_MAXSIO];
char *new_err,*new_out;
char temp[512],err_p[256];
int diskette = FALSE;

#define GIO_ALL_GIOS    1       /* all native serial adapters cfg. with
                                 * dial or LPFK */
#define GIO_RS232_CFGED 2       /* at least one native serial adapter is
                                 * configured with RS232 */
#define GIO_OTHERS      3

extern char *diag_cat_gets();

/* LOCAL FUNCTION PROTOTYPES */
void do_error(int, char *);
void int_handler(int);
void genexit(int);
void action(int );
void Gio_del_confirm(struct CuDv *, char *);
int Gio_Add_confirm(Gio_Serial *, int, struct action_ctl *);
void Gio_get_desc(char **,struct CuDv *);
void Gio_Add_device(int );
void Gio_Add_menu(int ,int );
int Gio_child_is_cfg(struct CuDv *,struct PdCn *,int *);
void Gio_display(struct CuDv *,struct listinfo *,struct action_ctl *);
int Gio_get_selected_NSIO(int ,int ,int *);
int Gio_run_method(char *,char *,int ,char *,int);
int Gio_status(int *);
void Gio_update_option(char *,char *);
void Gio_rm_anydef(char *);
int Gio_syscons_check(char *);
int Gio_device_install(int, int);
int get_new_msgno(struct CuDv *);
void Gio_get_loc(char **,struct CuDv *);
void Gio_tty_name(char **,struct CuDv *);
void Gio_tty_cleanup(char *);
void Gio_gio_name(char **, Gio_Serial *);
void Configure_parents(char *);
extern nl_catd diag_catopen(char *, int);


/****************************************************************
* NAME: main
*
* FUNCTION: Facilitate addition/deletion of Dials/LPFKs for the
*           native serial adapter
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*       Gio_Add_device()
*       Gio_Add_menu()
*       Gio_status()
*       diag_display()
*       action()
*       genexit()
*
* RETURNS:
*       Exits with 0    (=no problems)
****************************************************************/

main()
{
  int cfg_cnts;
  int selection;
  int status = -1;
  struct sigaction act;

        setlocale(LC_ALL,"");

        /* set up interrupt handler     */
        act.sa_handler = int_handler;
        sigaction(SIGINT, &act, (struct sigaction *)NULL);

        /* open the catalog file containing the menus */
        fdes = diag_catopen(MF_UCFGDEV, 0);

        /* initialize ASL       */
        diag_asl_init("NO_TYPE_AHEAD");

        /* initialize the ODM   */
        odm_initialize();
        /* get device name of the native serial adapter */

        /* see if we are running from standalone media */
        ipl_mode (&diskette);


        /****************************************************************
        * If the user presses CANCEL or EXIT, then stop.  Else, select  *
        ****************************************************************/

        while (status != DIAG_ASL_CANCEL && status != DIAG_ASL_EXIT)
          {
          status = diag_display(UCFGGIO_MENU_TOP, fdes, menu_list,
                                DIAG_IO, ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                NULL, NULL);
          if (status == DIAG_ASL_COMMIT)
            {
            switch (selection = DIAG_ITEM_SELECTED(dm_menutype)-1)
              {
                case GIO_ACT_ADD_D: case GIO_ACT_ADD_L:
                        Gio_Add_device(selection);

                        if ((Gio_status(&cfg_cnts)) == GIO_ALL_GIOS) {
                                /* all configured with dials or lpfkeys */
                                diag_hmsg(fdes,ERROR_SET,UNAVAILABLE,NULL);
                                break;
                        }

                        /* here, present menu for adding device */
                        Gio_Add_menu(cfg_cnts, selection);

                        break;
                case GIO_ACT_LIST:
                case GIO_ACT_DEL_D: case GIO_ACT_DEL_L:
                        action(selection);
                        break;
              }
            }
          }
        genexit(0);
}



/****************************************************************
* NAME: Gio_get_selected_NSIO
*
* FUNCTION: parse the display line to return the index of Gio_NSIO[]
*           for the chosen device
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*
* RETURNS:
*       the index of Gio_NSIO[]
****************************************************************/

int Gio_get_selected_NSIO(int sel_type,int select_disp_line,int *chosendev)
{
int i;
register Gio_Serial *sio_ptr;

        for (i= *chosendev = 0; i < GIO_MAXSIO; i++) {
                sio_ptr = &Gio_NSIO[i];
                if ((sio_ptr->ch_type & sel_type) &&
                        ++*chosendev==select_disp_line)
                        return(*chosendev = i);
        }
}

/****************************************************************
* NAME: Gio_rm_anydef
*
* FUNCTION: Remove any non-active device on the specified serial port
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*       None
*
* RETURNS:
*       None
****************************************************************/
void Gio_rm_anydef(char *connwhere)
{
char criteria[128];

        sprintf(criteria,"/usr/bin/odmdelete -o CuDv -q\"connwhere = %s and status !=1 and chgstatus != 3\"",
                connwhere);
        system(criteria);
}

/****************************************************************
* NAME: Gio_Add_menu
*
* FUNCTION: Present add device menu for free serial adapters
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*       Gio_Add_confirm()
*
* RETURNS:
*       None
****************************************************************/
void Gio_Add_menu(int cfg_cnts,int add_type)
{
int i;
int free_cnts = GIO_MAXSIO - cfg_cnts;
int status;
char *txtpool,*lastptr;
ASL_SCR_INFO *disp_info;
register ASL_SCR_INFO *disp_ptr;
struct action_ctl *action_ptr;
Gio_Serial *sio_ptr;
static ASL_SCR_TYPE disp_type = DM_TYPE_DEFAULTS;
int rc, chosendev;
char option[64];
char *cp;

        disp_type.max_index = free_cnts;
        disp_info = (ASL_SCR_INFO *)calloc((disp_type.max_index += 2),
                                sizeof(ASL_SCR_INFO));
        lastptr = txtpool = (char *)malloc((MAX_CHAR+1) * disp_type.max_index);
        action_ptr = &action_tbl[add_type];

        disp_info->text = diag_cat_gets(fdes,action_ptr->setid,action_ptr->first_msgid);

        /* setup menu */
        disp_ptr = &disp_info[1];
        for (i=0;  i < GIO_MAXSIO; i++) {
                sio_ptr = &Gio_NSIO[i];
                /* if port is undefined or rs232, add to parent list */
                if (sio_ptr->ch_type & GIO_DEV_RS232_NULL) {
                        sprintf(lastptr,GIO_DSP_FMT,sio_ptr->name,
                                        sio_ptr->location,sio_ptr->desc);
                        disp_ptr->text = lastptr;
                        lastptr += strlen(lastptr) +1;
                        disp_ptr++;
                }
        }
        disp_ptr->text = diag_cat_gets(fdes,action_ptr->setid,action_ptr->last_msgid);

        disp_type.max_index--;
        while ( 1 ) {
                status = -1;
                while (status != DIAG_ASL_CANCEL && status != DIAG_ASL_EXIT &&
                        status != DIAG_ASL_COMMIT && status != DIAG_ASL_ENTER) {
                        /* asking the user which serial  adapter he chooses to  */
                        status = diag_display(action_ptr->menu_no, fdes, NULL,
                                  DIAG_IO, action_ptr->scrtype,
                                  &disp_type,disp_info);
                }

                if (status == DIAG_ASL_COMMIT) {

                        Gio_get_selected_NSIO(GIO_DEV_RS232_NULL,
                                        disp_type.cur_index,&chosendev);
                        sio_ptr = &Gio_NSIO[chosendev];

                        /* Port must be configured as an AVAILABLE tty */
                        /* before an sgio device can be created for the port */
                        if (sio_ptr->ch_type == GIO_DEV_NULL) {
                                Configure_parents(sio_ptr->name);
                                sprintf(option,action_ptr->option,sio_ptr->name,
                                        sio_ptr->connwhere);
                                if(Gio_run_method(action_ptr->method,option,
                                        MKDEV_ERR,sio_ptr->name,0)==0)
                                        Gio_Add_device(add_type); /*Re-init db*/
                        }
                        /* If port is already configured as a tty, attempt to */
                        /* disable login on the tty */
                        else if(sio_ptr->ch_type == GIO_DEV_RS232) {
                                if(Gio_syscons_check(sio_ptr->cfg_child)) {
                                        cp = diag_cat_gets(fdes, ERROR_SET,
                                                SYSCONS_ERR);
                                        strcpy(temp, cp);
                                        diag_asl_msg(temp,sio_ptr->name,
                                                sio_ptr->cfg_child);
                                        break;  /* quit */
                                }
                                else {
                                   if(diskette == FALSE) {
                                        sprintf(option,
                                            "-l '%s' -a login='disable'",
                                            sio_ptr->cfg_child);
                                        if (Gio_run_method(CHDEV,option,
                                            CHDEV_ERR,
                                            sio_ptr->cfg_child,0)!=0)
                                                break;  /* quit */
                                   }
                                }
                        }

                        /* Prompt user to install GIO device on adapter */
                        if(!Gio_device_install(add_type, chosendev))
                                break;  /* quit */

                        /* Create SGIO device (dial/lpfkeys) on the port */
                        sprintf(option,action_ptr->option2,sio_ptr->cfg_child);
                        if (Gio_run_method(action_ptr->method2,option,
                                        MKDEV_ERR,sio_ptr->name,0) == 0)
                        {
                                status = Gio_Add_confirm(sio_ptr,add_type,
                                                      action_ptr);
                        }
                        break; /* quit */
                }
                else
                        break; /* quit */
        }

        free(disp_info);
        free(txtpool);
}

/****************************************************************
* NAME: Gio_del_confirm
*
* FUNCTION: Reconfirm the deletion of device pointer by cudv_ptr
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*       Gio_run_method()
*
* RETURNS:
*       Exits with 0    (=no problems)
****************************************************************/
void Gio_del_confirm(struct CuDv *cudv_ptr, char *loc)
{
char *desc;

        Gio_get_desc(&desc,cudv_ptr);
        strcpy(temp, diag_cat_gets(fdes, GIO_CONFIRM_SET,
                GIO_DEL_CONFIRM));
        diag_asl_msg(temp,cudv_ptr->name,loc, desc);
        free(desc);
}

/****************************************************************
* NAME: Gio_Add_confirm
*
* FUNCTION: Popup confirmation menu of adding devices
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*       Gio_run_method()
*       Gio_get_desc()
*
* RETURNS:
*       same return code of diag_asl_msg()
****************************************************************/
int Gio_Add_confirm(Gio_Serial *sio_ptr,int add_type,
                         struct action_ctl *action_ptr)
{
int status;
struct CuDv *cudv;
struct listinfo cudv_info;
char *desc;
char *gioname;


        cudv = get_CuDv_list(CuDv_CLASS,action_ptr->criteria,&cudv_info,1,2);
        if (cudv == (struct CuDv *) -1) {
                do_error(ODM_ACCESS_ERR, "CuDv");
        }

        /* Find correct device name */
        Gio_gio_name(&gioname, sio_ptr);

        /* Get device description */
        Gio_get_desc(&desc,cudv);

        /* popup confirmation menu */
        strcpy(temp, diag_cat_gets(fdes, GIO_CONFIRM_SET,GIO_ADD_CONFIRM));
        status = diag_asl_msg(temp,sio_ptr->name,sio_ptr->desc,
                gioname,sio_ptr->ch_location,desc);

        /* Remove any defined devices hanging around on the port */
        Gio_rm_anydef(sio_ptr->connwhere);

        /* Clean up */
        free(gioname);
        free(desc);

        return(status);
}

/* return 1 if the parent device pointed by cudv_ptr has
*  any active dials/LPFK or rs232 connected to
*/
/****************************************************************
* NAME: Gio_NSIO_is_gio_or_rs232
*
* FUNCTION: Determines whether the parent device pointed by cudv_ptr
*           has any active dial/LPFK or rs232 child device connected
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*
* RETURNS:
*       1       if there is dial/LPFK or rs232
*       0       if there is no active dial/LPFK or rs232
*
****************************************************************/

int Gio_NSIO_is_gio_or_rs232(struct PdCn *pdcn_ptr,struct listinfo *pdcn_info)
{
        for (;  pdcn_info->num--; pdcn_ptr++) {
                if (!strncmp("gio",pdcn_ptr->connkey,3) ||
                    !strncmp("rs232",pdcn_ptr->connkey,5))
                        return(1);
        }
        return(0);
}

/****************************************************************
* NAME: Gio_Add_device
*
* FUNCTION: Intialize Gio_NSIO[] and fill in the device name of
*           the native serial adapters by querying from CuDv matching
*           PdDvLn with adapter/sio/? and filter through PcCn by
*           matching connkey with "gio"
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*       Gio_Add_menu()
*       Gio_NSIO_is_gio_or_rs232()
*       Gio_child_is_cfg()
*
* RETURNS:
*       None
****************************************************************/
void Gio_Add_device(int dev_type)
{
int i,SIO_cfg_childs;
struct CuDv *cudv,*cudv_ptr;
struct listinfo cudv_info;
struct PdCn *pdcn,*pdcn_ptr;
struct listinfo pdcn_info;
int sz = sizeof(Gio_Serial)-sizeof(struct restore_rs232);
char criteria[128];

        cudv_ptr = cudv = get_CuDv_list(CuDv_CLASS,
                "PdDvLn like adapter/sio/* and  chgstatus !=3",
                &cudv_info,2,2);

        if (cudv == (struct CuDv *) -1) {
                do_error(ODM_ACCESS_ERR, "CuDv");
        }

        if (cudv_info.num == 0) { /* no NSIO is configured */
                diag_hmsg(fdes,ERROR_SET,NSIO_NOTCFG,NULL);
                odm_free_list(cudv,&cudv_info);
                return;
        }

        /* reinitialize Gio_NSIO[] but leave restore_rs232 field alone */
        for (i=0; i < GIO_MAXSIO; i++)
                memset(&Gio_NSIO[i],0,sz);

        /* fill in the native serial adapter device name,location,desc */
        for (i= SIO_cfg_childs = 0;
                i < cudv_info.num && SIO_cfg_childs < GIO_MAXSIO;
                i++,cudv_ptr++) {

                /* do not erase restore information */

                sprintf(criteria,"uniquetype='%s' ",
                        cudv_ptr->PdDvLn->uniquetype);
                pdcn_ptr = pdcn = get_PdCn_list(PdCn_CLASS,criteria,
                        &pdcn_info,2,1);

                if (pdcn == (struct PdCn *) -1) {
                        do_error(ODM_ACCESS_ERR, "PdCn");
                }
                if (pdcn_info.num == 0)
                        continue;

                if (Gio_NSIO_is_gio_or_rs232(pdcn_ptr,&pdcn_info))
                        Gio_child_is_cfg(cudv_ptr,pdcn_ptr,&SIO_cfg_childs);
        }
        odm_free_list(cudv,&cudv_info);
        odm_free_list(pdcn,&pdcn_info);

}

/****************************************************************
* NAME: Gio_status
*
* FUNCTION: Return status of native serial adapters
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*
* RETURNS:
*       GIO_ALL_GIOS    if all configured with dials or lpfkeys
*       GIO_RS232_CFGED if any one serial port is configured with rs232
*       GIO_OTHERS      other cases other than the previous two
****************************************************************/

int Gio_status(int *cfg_cnts)
{
Gio_Serial *sio_ptr = Gio_NSIO;
int i;
int gio_cnts = 0,rs232_cnts = 0;

        for (*cfg_cnts =  i = 0; i < GIO_MAXSIO; i++,sio_ptr++) {
                if (sio_ptr->ch_type== GIO_DEV_GIO)
                        ++*cfg_cnts,gio_cnts++;
                else
                if (sio_ptr->ch_type == GIO_DEV_RS232)
                        ++rs232_cnts;
        }

        return((gio_cnts==GIO_MAXSIO)? GIO_ALL_GIOS:
                (rs232_cnts)? GIO_RS232_CFGED : GIO_OTHERS);
}

/****************************************************************
* NAME: Gio_child_is_cfg
*
* FUNCTION: Add a new entry to Gio_NSIO[] if a new adapter is found
*           otherwise see if there is a child device already configured
*           with this adapter ; if no child device is configured then
*           see whether there is a child device is configured from CuDv
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*       Gio_get_desc()
*
* RETURNS:
*       Exits with 0    (=no problems)
*       1       if there is new child device configured
*       0       otherwise
****************************************************************/

int Gio_child_is_cfg(struct CuDv *cudv_ptr,struct PdCn *pdcn_ptr,
        int *SIO_cfg_childs )
{
Gio_Serial *sio_ptr;
int free_entry;
int new_child_cfged = 0;
int i,j;
char *desc;
char criteria[128];
struct CuDv *cudv_ptr1,*cudv1;
struct listinfo cudv_info;
struct CuAt *cuat;
struct listinfo cuat_info;
struct restore_rs232 *restore;

        /* check entry exists ? */
        for (i=0,sio_ptr=Gio_NSIO,free_entry=-1;
                i < GIO_MAXSIO; i++,sio_ptr++) {
                if (!strcmp(sio_ptr->name,cudv_ptr->name))
                        break;
                if (sio_ptr->name[0] == '\0') {
                        free_entry = i;
                        break;
                }
        }
        if (free_entry != -1) { /* adding new entry */

                if (free_entry == -1) { /* impossible */
                }
                /* adds new serial adapter entry to Gio_NSIO */
                sio_ptr = &Gio_NSIO[free_entry];
                sio_ptr->status = cudv_ptr->status;
                strcpy(sio_ptr->name,cudv_ptr->name);
                strcpy(sio_ptr->location,cudv_ptr->location);
                strcpy(sio_ptr->connwhere,pdcn_ptr->connwhere);
                Gio_get_desc(&desc,cudv_ptr);
                strcpy(sio_ptr->desc,desc);
                sio_ptr->cfg_child[0] = '\0';
                sio_ptr->ch_type=GIO_DEV_NULL;
        }
        if (sio_ptr->cfg_child[0] == '\0') { /* see whether there is
                * any child device configured on this adapter */

                sprintf(criteria,
                "parent=%s and connwhere=%s and status=1 and chgstatus !=3",
                        cudv_ptr->name,pdcn_ptr->connwhere);
                cudv_ptr1 = cudv1 = get_CuDv_list(CuDv_CLASS,criteria,
                        &cudv_info,1,2);

                if (cudv1 == (struct CuDv *) -1) {
                        do_error(ODM_ACCESS_ERR, "CuDv");
                }


                if (cudv_info.num ) { /* tty device config'd */

                        strcpy(sio_ptr->cfg_child,cudv_ptr1->name);
                        strcpy(sio_ptr->ch_location,cudv_ptr1->location);
                        Gio_get_desc(&desc,cudv_ptr1);
                        strcpy(sio_ptr->ch_desc,desc);
                        strcpy(sio_ptr->connwhere,pdcn_ptr->connwhere);

                        sio_ptr->ch_type = GIO_DEV_RS232;

                        /* set once only */
                        restore = &sio_ptr->restore_info;
                        if (restore->org_status== RS232_WAS_NULL) {
                            restore->org_status= RS232_WAS_CFG;
                            strcpy(restore->name,sio_ptr->cfg_child);
                        }

                        sprintf(criteria,"attribute=ttydevice and value=%s",
                                sio_ptr->cfg_child);
                        cuat = get_CuAt_list(CuAt_CLASS,criteria,&cuat_info,1,2);
                        if (cuat == (struct CuAt *) -1) {
                                do_error(ODM_ACCESS_ERR, "CuAt");
                        }

                        if (cuat_info.num) {  /* GIO device config'd */
                                strcpy(sio_ptr->cfg_child,cudv_ptr1->name);
                                Gio_get_desc(&desc,cudv_ptr1);
                                strcpy(sio_ptr->ch_desc,desc);
                                sio_ptr->ch_type = GIO_DEV_GIO;
                        }

                        ++*SIO_cfg_childs;
                        new_child_cfged = 1;
                }
                else {  /* No tty device on the port */
                        restore = &sio_ptr->restore_info;
                        restore->org_status= RS232_CFG_BY_SA;
                }

                odm_free_list(cudv1,&cudv_info);
                odm_free_list(cuat,&cuat_info);
        }
        return(new_child_cfged);
}

/****************************************************************
* NAME: Gio_get_desc
*
* FUNCTION: Get device description into desc
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*
* RETURNS:
*       None
****************************************************************/
void Gio_get_desc(char **desc,struct CuDv *cudv_ptr)
{
int setno,msgno;
char *str;
nl_catd cudv_cat;


    cudv_cat = (nl_catd) diag_device_catopen(cudv_ptr->PdDvLn->catalog, NL_CAT_LOCALE);
    setno = cudv_ptr->PdDvLn->setno;
    msgno = cudv_ptr->PdDvLn->msgno;
    if (msgno == 0)
        msgno = get_new_msgno(cudv_ptr);

    str = diag_cat_gets(cudv_cat, setno, msgno);
    *desc = (char *)malloc(1+strlen(str));
    strcpy(*desc,str);
    catclose(cudv_cat);
}

/****************************************************************
* NAME: action
*
* FUNCTION: Dispatch selected menu item from the top menu
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*       Gio_display()
*
* RETURNS:
*       None
****************************************************************/
void action(int selection)
{
struct CuDv *cudv;
struct action_ctl *action_ptr;
struct listinfo cudv_info;

        action_ptr = &action_tbl[selection];

        cudv = get_CuDv_list(CuDv_CLASS,action_ptr->criteria,&cudv_info,2,2);
        if (cudv == (struct CuDv *) -1) {
                do_error(ODM_ACCESS_ERR, "CuDv");
        }

        Gio_display(cudv,&cudv_info,action_ptr);
}

/****************************************************************
* NAME: Gio_update_option
*
* FUNCTION: Update the option string used in odm_run_method by
*           replacing the template device name with the selected
*           device name
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*
* RETURNS:
*       None
****************************************************************/
void Gio_update_option(char *option,char *selected_dev)
{
char *cptr,*dptr;
char devno;
int dlen;

        cptr =  &selected_dev[dlen=strlen(selected_dev)-1];
        devno = *cptr; /* save the device number */
        *cptr = '\0';

        for (dptr = option; *dptr != '\0'; dptr++)
        if (!strncmp(dptr,selected_dev,dlen))
                break;
        if (*dptr )
                *cptr = dptr[dlen] = devno;
        return;
}

/****************************************************************
* NAME: Gio_display
*
* FUNCTION:  Build and display the menu with menu items passed from
*            cudv argument; also invokes the corresponding method
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*       Gio_get_desc()
*       Gio_update_option()
*       Gio_run_method()
*       Gio_del_confirm()
*
* RETURNS:
*       None
****************************************************************/

void Gio_display(struct CuDv *cudv,struct listinfo *cudv_info,
        struct action_ctl *action_ptr)
{
int status;
char *txtpool,*lastptr;
register struct CuDv *cudv_ptr;
ASL_SCR_INFO *disp_info;
register ASL_SCR_INFO *disp_ptr;
static ASL_SCR_TYPE disp_type = DM_TYPE_DEFAULTS;
char  tmp_method[100];
char  tmp_option[255];
int   tmp_run_err;
char  *desc, *loc;
char *ttyname;
int sel_num;
int i;

        if (cudv_info->num == 0) { /* no NSIO is configured */
                diag_hmsg(fdes,GIO_NULL_SET,action_ptr->null_msgid,NULL);
                odm_free_list(cudv,cudv_info);
                return;
        }

        disp_type.max_index = cudv_info->num;

        disp_info = (ASL_SCR_INFO *)calloc((disp_type.max_index += 2),
                                sizeof(ASL_SCR_INFO));
        lastptr = txtpool = (char *)malloc((MAX_CHAR+1) * disp_type.max_index);

        disp_info[disp_type.max_index -1].text = (char *)-1;
        disp_info->text = diag_cat_gets(fdes,action_ptr->setid,action_ptr->first_msgid);

        /* setup menu */
        disp_ptr = &disp_info[1];
        for (cudv_ptr=cudv;disp_ptr->text != (char *)-1;disp_ptr++,cudv_ptr++) {

                Gio_get_desc(&desc,cudv_ptr); /* get description of device */
                Gio_get_loc(&loc,cudv_ptr);  /* get location of device */
                sprintf(lastptr,GIO_DSP_FMT,cudv_ptr->name,loc,desc);
                free(desc);
                free(loc);
                disp_ptr->text = lastptr;
                lastptr += strlen(lastptr) +1;
        }
        disp_ptr->text = diag_cat_gets(fdes,action_ptr->setid,action_ptr->last_msgid);

        disp_type.max_index--;

        status = -1;
        while (status != DIAG_ASL_CANCEL && status != DIAG_ASL_EXIT
                && status != DIAG_ASL_COMMIT && status != DIAG_ASL_ENTER) {
                status = diag_display(action_ptr->menu_no, fdes, NULL,
                                  DIAG_IO, action_ptr->scrtype,
                                  &disp_type,disp_info);
        }

        if ((status == DIAG_ASL_ENTER || status == DIAG_ASL_COMMIT) &&
              action_ptr->method != NULL) {
                /* reach here only for deletion device action */
                sel_num = DIAG_ITEM_SELECTED(disp_type)-1;

                if (cudv_info->num) {
                        /* Set cudv_ptr to correct cudv entry */
                        for (cudv_ptr=cudv,i=0; i<sel_num; cudv_ptr++,i++) ;

                        /* Get the name of the ttydevice*/
                        Gio_tty_name(&ttyname, cudv_ptr);
                        Gio_get_loc(&loc,cudv_ptr);

                        sprintf(tmp_option,action_ptr->option,cudv_ptr->name);
                        if (Gio_run_method(action_ptr->method,
                                tmp_option,(int)action_ptr->run_err,
                                cudv_ptr->name,0)==0)
                        {
                                /* remove from data base                */
                                strcpy ( tmp_method, "odmdelete");
                                sprintf (tmp_option,
                                       "-q \"attribute=ttydevice and value=%s\" -o CuAt",
                                       cudv_ptr->name);
                                if(Gio_run_method(tmp_method,
                                        tmp_option, tmp_run_err,
                                        cudv_ptr->name,0)==0) {
                                        /* See if we should delete tty */
                                        Gio_tty_cleanup(ttyname);

                                        /* confirm device deletion */
                                        Gio_del_confirm(cudv_ptr, loc);
                                }
                        }
                        free(ttyname);
                        free(loc);
                }
        }

        free(disp_info);
        free(txtpool);
        odm_free_list(cudv,cudv_info);
        return;
}

/****************************************************************
* NAME: do_error
*
* FUNCTION: Generate a menu containing the error message.
*           Call the generic exit routine.
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*       genexit()
*
* RETURNS:
*       None
****************************************************************/
void do_error(int emsg_num, char *msg_str)
{
  int  rc;

/*      Perror(fdes, ERROR_SET, emsg_num, msg_str);             */
        strcpy(temp, diag_cat_gets(fdes, ERROR_SET, emsg_num));
        diag_asl_msg(temp, msg_str);
        genexit(-1);
}

/****************************************************************
* NAME: int_handler
*
* FUNCTION: In case of an interrupt, this routine is called.
*           (NOTE:  It was unclear whether this routine would
*           be called with 1 argument, as indicated by the
*           sigaction structure, or 3 arguments, as indicated
*           in the documentation of system call sigaction().)
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*       genexit()
*
* RETURNS:
*       None
****************************************************************/
/* void int_handler(int sig, int code, struct sigcontext *scp)  */
void int_handler(int sig)
{
        diag_asl_clear_screen();
        genexit(1);
}

/*  */
/****************************************************************
* NAME: genexit
*
* FUNCTION: Exit the ASL menu mode.  Relinquish all acquired
*           ODM data.  Close the NL catalog "ucfggio.cat".
*           Exit to process that invoked ucfggio.
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*       None
*
* RETURNS:
*       None
****************************************************************/
void genexit(int exitcode)
{
        diag_asl_quit();
        odm_terminate();
        catclose(fdes);
        exit(exitcode);
}

/****************************************************************
* NAME: Gio_run_method
*
* FUNCTION: Call odm_run_method to execute the command passed by
*           method argument and display a popup menu if there is
*           any error returned from odm_run_method
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RETURNS:
*       return code of odm_run_method
****************************************************************/

Gio_run_method(char *method,char *option,int run_err,
        char *device,int defer_err)
{
char *cp;
int status;

        if ((status = odm_run_method(method,option,&new_out,&new_err))) {
                if (!defer_err) {
                        cp =    diag_cat_gets(fdes, ERROR_SET,
                                        (unsigned short)run_err);
                        strcpy(temp, cp);
                        strcpy(err_p,new_err);
                        diag_asl_msg(temp,device,err_p);
                        free(new_err);
                }
                free(new_out);
        }
        return(status);
}

/****************************************************************
* NAME: get_new_msgno
*
* FUNCTION: Looks for a valid message number in the CuAt
*           class or the PdAt class.  This routine is modelled
*           after routine get_device_text() in libdiag.a
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*         do_error()
*
* RETURNS:
*       None
****************************************************************/
int get_new_msgno(struct CuDv *cudv)
{
  int msgno = 0;
  struct CuAt *cuat = (struct CuAt *) NULL;
  struct PdAt *pdat = (struct PdAt *) NULL;
  struct listinfo c_info;
  char crit[100];

        /* check for type 'T' attribute in customized attributes */
        sprintf(crit, "name = %s AND type = T", cudv->name);
        cuat = (struct CuAt *)get_CuAt_list(CuAt_CLASS, crit, &c_info, 1, 1);
        if (cuat == (struct CuAt *) -1) do_error(ODM_ACCESS_ERR, "CuAt");

        /* if no customized attribute, then get default from PdAt */
        if (c_info.num == 0) {
                sprintf(crit, "uniquetype = %s AND type = T",
                        cudv->PdDvLn_Lvalue);
                pdat = (struct PdAt *)get_PdAt_list(PdAt_CLASS, crit,
                        &c_info, 1, 1);
                if (pdat == (struct PdAt *) -1)
                        do_error(ODM_ACCESS_ERR, "PdAt");
                else if (c_info.num == 1)
                        msgno = atoi(pdat->deflt);
        }
        else
                msgno = atoi(cuat->value);

        if (cuat != (struct CuAt *)NULL)
          odm_free_list(cuat, &c_info);
        return(msgno);
}


/****************************************************************
* NAME: Gio_syscons_check
*
* FUNCTION: Checks if the specified tty is the system console.
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*         do_error()
*
* RETURNS:
*       0 - Device is not the system console
*       1 - Device is the system console
****************************************************************/
int Gio_syscons_check(char *device)
{
        char    buffer[128];
        char    dvname[128];
        struct  CuAt *cuat;
        struct  listinfo p_info;

        strcpy(buffer, "attribute = syscons");
        cuat = get_CuAt_list(CuAt_CLASS, buffer, &p_info, 1, 2);
        if(cuat == (struct CuAt *) -1)
                do_error(ODM_ACCESS_ERR, "CuAt");
        sprintf(dvname,"/dev/%s", device);
        if (!strcmp(cuat->value ,dvname))
                return (1);
        else
                return 0;
}


/****************************************************************
* NAME: Gio_device_install
*
* FUNCTION: Prompts user to connect device to Native Serial Port
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RETURNS:
*       0 - User requested not to install device on port
*       1 - User responded that device was installed on port.
****************************************************************/
int Gio_device_install(int add_type, int chosendev)
{
        char *cp;
        Gio_Serial *sio_ptr;
        int rc;

        sio_ptr = &Gio_NSIO[chosendev];

        if(add_type == GIO_ACT_ADD_D)
           cp = diag_cat_gets(fdes, GIO_INSTALL_SET, GIO_INSTALL_DIAL);
        else
           cp = diag_cat_gets(fdes, GIO_INSTALL_SET, GIO_INSTALL_LPFK);
        strcpy(temp, cp);
        rc=diag_asl_msg(temp, sio_ptr->name);
        if(rc == DIAG_ASL_ENTER)
           return 1;
        else
           return 0;
}

/****************************************************************
* NAME: Gio_get_loc
*
* FUNCTION: Get device location into loc
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*
* RETURNS:
*       None
****************************************************************/
void Gio_get_loc(char **loc, struct CuDv *cudv_ptr)
{
struct CuAt *cuat;
struct listinfo cuat_info;
struct CuDv *cudv1;
struct listinfo cudv_info1;
char criteria[128];


    /* Get the related tty for the specified dial/lpfkeys */
    sprintf(criteria,"name=%s and attribute='ttydevice'",cudv_ptr->name);
    cuat = get_CuAt_list(CuAt_CLASS,criteria,&cuat_info,1,2);
    if (cuat == (struct CuAt *) -1) {
            do_error(ODM_ACCESS_ERR, "CuAt");
    }

    /* Get the location for the related tty */
    sprintf(criteria,"name=%s and status=1 and chgstatus!=3",cuat->value);
    cudv1 = get_CuDv_list(CuDv_CLASS,criteria,&cudv_info1,1,2);
    if (cudv1 == (struct CuDv *) -1) {
            do_error(ODM_ACCESS_ERR, "CuDv");
    }


    *loc = (char *)malloc(1+strlen(cudv1->location));
    strcpy(*loc,cudv1->location);
    odm_free_list(cuat,&cuat_info);
    odm_free_list(cudv1,&cudv_info1);
}

/****************************************************************
* NAME: Gio_tty_name
*
* FUNCTION: Get the name of the tty associated with the dial/lpfk
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*
* RETURNS:
*       None
****************************************************************/
void Gio_tty_name(char **ttyname, struct CuDv *cudv_ptr)
{
struct CuAt *cuat;
struct listinfo cuat_info;
char criteria[128];


    /* Get the related tty for the specified dial/lpfkeys */
    sprintf(criteria,"name=%s and attribute='ttydevice'",cudv_ptr->name);
    cuat = get_CuAt_list(CuAt_CLASS,criteria,&cuat_info,1,2);
    if (cuat == (struct CuAt *) -1) {
            do_error(ODM_ACCESS_ERR, "CuAt");
    }

    *ttyname = (char *)malloc(1+strlen(cuat->value));
    strcpy(*ttyname,cuat->value);
    odm_free_list(cuat,&cuat_info);
}

/****************************************************************
* NAME: Gio_tty_cleanup
*
* FUNCTION: If we created a tty in order to create a dial/lpfk, we need to
*           delete it.
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*
* RETURNS:
*       None
****************************************************************/
void Gio_tty_cleanup(char *ttyname)
{
Gio_Serial *sio_ptr;
int i;
char criteria[128];

   for(i=0; i < GIO_MAXSIO; i++) {
      sio_ptr=&Gio_NSIO[i];
      if(!strncmp(sio_ptr->cfg_child,ttyname,strlen(ttyname))) {
         /* If we created the tty, kill it */
         if(sio_ptr->restore_info.org_status == RS232_CFG_BY_SA) {
            sprintf(criteria,"-l %s -d",ttyname);
            Gio_run_method(RMDEV,criteria,RMDEV_ERR,ttyname,0);
            break;
         }
         else
            break;
      }
   }

}

/****************************************************************
* NAME: Gio_gio_name
*
* FUNCTION: Get the name of the dial/lpfk associated with the tty
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*
* RETURNS:
*       None
****************************************************************/
void Gio_gio_name(char **gioname, Gio_Serial *sio_ptr)
{
struct CuAt *cuat;
struct listinfo cuat_info;
char criteria[128];


    /* Get the related tty for the specified dial/lpfkeys */
    sprintf(criteria,"value=%s and attribute='ttydevice'",sio_ptr->cfg_child);
    cuat = get_CuAt_list(CuAt_CLASS,criteria,&cuat_info,1,2);
    if (cuat == (struct CuAt *) -1) {
            do_error(ODM_ACCESS_ERR, "CuAt");
    }

    *gioname = (char *)malloc(1+strlen(cuat->name));
    strcpy(*gioname,cuat->name);
    odm_free_list(cuat,&cuat_info);
}

/****************************************************************
* NAME: Configure_parents
*
* FUNCTION: Configure the parent adapters of the chosen tty port.
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*
* RETURNS:
*       None
****************************************************************/
void Configure_parents(char *name)
{
   int   i, rc;
   char  args[255];
   struct CuDv *p_cudv;
   struct listinfo p_info;
   char  *obuf;
   char  parents[MAXLEVELS][NAMESIZE+1] = { "", "", "" };
   char  config[MAXLEVELS][NAMESIZE+1] = { "", "", "" };

   /* Build array "parents".  Identifies the devices that need    */
   /* to be configured.  Stop processing if a device in the path  */
   /* is configured or does not have a config method.             */

   sprintf(args, "name = %s", name);
   for (i=0; i<MAXLEVELS+1; i++) {
      p_cudv = get_CuDv_list(CuDv_CLASS, args, &p_info, 1, 2);
      if(p_cudv == (struct CuDv *) -1 || p_info.num == 0) {
         genexit(3);
      } /* endif */
      if((p_cudv->status != Available) && strlen(p_cudv->PdDvLn->Configure)) {
         strcpy(parents[i], p_cudv->name);
         strcpy(config[i], p_cudv->PdDvLn->Configure);
      }
      else
         break;
      sprintf(args, "name = %s", p_cudv->parent);
      odm_free_list(p_cudv, &p_info);
   } /* endfor */
   for (; i>=0; i--) {
      if(strlen(parents[i])) {
         sprintf(args, " -l %s", parents[i]);
         rc = invoke_method (config[i], args, &obuf);
         if (rc != 0) {
            genexit(5);
         } /* endif */
      } /* endif */
   } /* endfor */
}
