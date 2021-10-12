static char sccsid[] = "@(#)11	1.2  src/bos/diag/util/ugmcode/ugenucodesa.c, dsaugmcode, bos411, 9432B411a 8/8/94 16:03:01";
/*
 *   COMPONENT_NAME: dsaugmcode
 *
 *   FUNCTIONS: check_tmp
 *		clean_exit
 *		get_devices
 *		main
 *		onintr
 *		restore_genucode
 *		select_device
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <asl.h>
#include <locale.h>

#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/cfgodm.h>    /* CuDv class def */

#include <diag/da.h>
#include <diag/diago.h>
#include <diag/diag.h>
#include <diag/diag_exit.h>

#include "ugenucodesa.h"
#include "ugenucodesa_msg.h"

#define RESTORE_FILE "/tmp/genucode"

/* entry panel */
struct msglist entry_msg[] = { 
                         {GENERIC_SET, MENU_GEN},
                         {GENERIC_SET, MENU_GEN_ACTION},
                         {0,           0}
};

/* overwrite /tmp confirmation panel */
struct msglist tmp_conf[] = { 
                         {GENERIC_SET, MENU_TMP_CONFIRM},
                         {GENERIC_SET, MENU_CONFIRM_YES}, 
                         {GENERIC_SET, MENU_CONFIRM_NO},
                         {GENERIC_SET, MENU_CONFIRM_ACTION},
                         {0,           0}
};
ASL_SCR_INFO tmpinfo[DIAG_NUM_ENTRIES(tmp_conf)];

/* no-devices panel */
struct msglist nodev_msg[] = { 
                         {GENERIC_SET, MENU_NODEV},
                         {GENERIC_SET, MENU_NODEV_ACTION},
                         {0,           0}
};

/* device selection confirmation panel */
struct msglist dev_conf[] = { 
                         {GENERIC_SET, MENU_DEV_CONFIRM},
                         {GENERIC_SET, MENU_CONFIRM_YES}, 
                         {GENERIC_SET, MENU_CONFIRM_NO},
                         {GENERIC_SET, MENU_CONFIRM_ACTION},
                         {0,           0}
};
ASL_SCR_INFO devinfo[DIAG_NUM_ENTRIES(dev_conf)];

/* configure error panel */
struct msglist cfg_error[] = { 
                         {GENERIC_SET, MENU_CFG_ERROR},
                         {GENERIC_SET, MENU_RETURN_ACTION},
                         {0,           0}
};
ASL_SCR_INFO cfgerrinfo[DIAG_NUM_ENTRIES(cfg_error)];

/* restore completion panel */
struct msglist complete[] = { 
                         {GENERIC_SET, MENU_COMPLETE},
                         {GENERIC_SET, MENU_RETURN_ACTION},
                         {0,           0}
};
ASL_SCR_INFO compinfo[DIAG_NUM_ENTRIES(complete)];

/* restore error panel */
struct msglist restore_error[] = { 
                         {GENERIC_SET, MENU_RES_ERROR},
                         {GENERIC_SET, MENU_RETURN_ACTION},
                         {0,           0}
};
ASL_SCR_INFO reserrinfo[DIAG_NUM_ENTRIES(restore_error)];

/* execute error panel */
struct msglist exec_error[] = { 
                         {GENERIC_SET, MENU_EXC_ERROR},
                         {GENERIC_SET, MENU_RETURN_ACTION},
                         {0,           0}
};

/* genucode error panel */
struct msglist genucode_error[] = { 
                         {GENERIC_SET, MENU_GEN_ERROR},
                         {GENERIC_SET, MENU_RETURN_ACTION},
                         {0,           0}
};
ASL_SCR_INFO generrinfo[DIAG_NUM_ENTRIES(genucode_error)];

static nl_catd catd;

static int init_config_state = -1;

static int remove_ok = 0;
static int return_code = 0;

static struct CuDv **devices;
static struct CuDv *device;
static struct listinfo diskette_info;
static struct listinfo tape_info;
static struct CuDv *diskette_list = NULL;
static struct CuDv *tape_list = NULL;


/* Function Declarations */
void onintr();
void clean_exit();
void select_device();
extern char *diag_cat_gets(nl_catd, int, int);
extern nl_catd diag_catopen(char *, int);


/*
 * NAME: main()
 *
 * FUNCTION: This procedure is the entry point for the Generic Microcode
 *      Download Service Aid.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This procedure is fork()ed and exec()ed either by the diagnostic
 *      controller or by the shell.
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * (RECOVERY OPERATION:) In the event of a software error that prevents
 *      this routine from functioning, control will be returned to the
 *      diagnostic controller after presenting screens to the user
 *      informing them of the error.
 *
 * RETURNS: NONE
 */
main()
{
    long rc;
    struct sigaction act;

    /* handle Control-C like F10 (Exit) */
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    act.sa_handler = onintr;
    sigaction(SIGINT,&act,NULL);

    /* initialize locale environment and ASL */
    setlocale(LC_ALL, "");
    diag_asl_init("DEFAULT");

    /* open the message catalog file */
    catd=diag_catopen(MF_UGENUCODESA, 0);

    /* initialize the ODM */
    if (init_dgodm() != 0) {
        return_code = 1;
        clean_exit();
    }

    /* Put up the entry panel */
    rc = diag_display(MN_GEN_SA, catd, entry_msg, DIAG_IO, 
                      ASL_DIAG_KEYS_ENTER_SC, NULL, NULL);

    if (rc == DIAG_ASL_ENTER) {

        if (check_tmp() == DIAG_ASL_COMMIT) {

            remove_ok = 1;
            select_device();

        }
    }
  
    clean_exit();

} /* end main() */


/*
 * NAME: check_tmp()
 *
 * FUNCTION: This procedure checks the /tmp directory to see if 'genucode'
 *      already exists.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: DIAG_ASL_COMMIT - file doesn't exist or can be overwritten
 *          DIAG_ASL_CANCEL - F3 was pressed
 *          DIAG_ASL_QUIT   - F10 was pressed or file can't be overwritten
 */
long check_tmp ()
{
    long rc = DIAG_ASL_COMMIT;
    ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;
    struct stat file_stats;

    /* see if file to restore already exists */
    if (stat(RESTORE_FILE,&file_stats) == 0) {

        /* fetch the text for the confirm panel */
        rc = diag_display(MN_TMP_SA, catd, tmp_conf, DIAG_MSGONLY,
                          ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, tmpinfo);

        /* set default action to No */
        menutype.cur_index = 2;

        /* display confirm panel */
        rc = diag_display(MN_TMP_SA, catd, NULL, DIAG_IO, 
                          ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, tmpinfo);

        if (rc == DIAG_ASL_COMMIT) {
            if (DIAG_ITEM_SELECTED(menutype) == 2) {
                rc = DIAG_ASL_EXIT;
            }
        }
    }

    return (rc);

} /* end check_tmp() */


/*
 * NAME: select_device()
 *
 * FUNCTION: This procedure allows the user to choose which device to restore
 *      from, then calls restore_genucode(), passing that device.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 *         
 */
void select_device ()
{
    long rc = DIAG_ASL_COMMIT;
    char *temp_string;
    int i;
    int num_devices;
    ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;
    ASL_SCR_TYPE selecttype = DM_TYPE_DEFAULTS;
    ASL_SCR_INFO *selectinfo;
    char *dev_string;

    /* get available devices */
    num_devices = get_devices();

    /* create the device selection menu */
    selecttype.max_index = num_devices + 1;  /* include the title & action */
    selectinfo = (ASL_SCR_INFO *) calloc(selecttype.max_index+1,
                                         sizeof(ASL_SCR_INFO));

    /* fill in title */
    selectinfo[0].text = diag_cat_gets(catd, GENERIC_SET, MENU_SELECT);

    /* copy devices' text from CuDv objects */
    for (i=0;i<num_devices;i++) {
        selectinfo[i+1].text = (char *) malloc(128);
        sprintf(selectinfo[i+1].text,"%s\t\t%s\t\t%s",devices[i]->name,
                devices[i]->location,
                get_dev_desc(devices[i]->name));
    }

    /* fill in action */
    selectinfo[i+1].text = diag_cat_gets(catd, GENERIC_SET, 
                                         MENU_SELECT_ACTION);
  
    while (rc != DIAG_ASL_EXIT) {

        /* display device selection menu */
        rc = diag_display(MN_SEL_SA, catd, NULL, DIAG_IO,
                          ASL_DIAG_LIST_CANCEL_EXIT_SC, &selecttype,
                          selectinfo);

        if (rc == DIAG_ASL_COMMIT) {

            /* selecttype.cur_index is the selected disk */
            /* NOTE: selections start at 1 not zero, so you could say that
               NOTE: the selection is an index to the screen info text.
             */
            dev_string = selectinfo[selecttype.cur_index].text;
            device = devices[selecttype.cur_index-1];

            /* fetch all the text for the confirm panel */
            rc = diag_display(MN_DEV_SA, catd, dev_conf, DIAG_MSGONLY,
                              ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, devinfo);

            /* fill in the restore device info */
            temp_string = (char *) malloc(512);
            sprintf(temp_string, devinfo[0].text, dev_string);
            devinfo[0].text = temp_string;

            /* now that the dynamic data is inserted, display panel */
            rc = diag_display(MN_DEV_SA, catd, NULL, DIAG_IO,
                              ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, devinfo);

            if (rc == DIAG_ASL_COMMIT) {

                if (DIAG_ITEM_SELECTED(menutype) == 1) {

                    rc = restore_genucode();

                } /* end if (confirm ok) */

            } /* end if (confirm selected) */

        } /* end if (device selected) */

        else if (rc == DIAG_ASL_CANCEL) {
            rc = DIAG_ASL_EXIT;
        }

    } /* end while (not exit) */

    return;

} /* end select_device() */


/*
 * NAME: get_devices()
 *
 * FUNCTION: This procedure gets a list of installed diskette and tape devices
 *      from ODM.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: number of available diskette and tape devices
 */
int get_devices ()
{
    int i;
    int num_devices = 0;

    /* NOTE: get_CuDv_list() and CuDv_CLASS defined in <cfgodm.h> */

    /* get diskette devices */
    diskette_list = get_CuDv_list(CuDv_CLASS,
                                  "PdDvLn like diskette/* AND chgstatus != 3",
                                  &diskette_info, 20, 2);

    if ((diskette_list != NULL) && ((int) diskette_list != -1)) {
        num_devices = diskette_info.num;
    }

    /* get tape devices */
    tape_list = get_CuDv_list(CuDv_CLASS,
                              "PdDvLn like tape/* AND chgstatus != 3",
                              &tape_info, 20, 2);

    if ((tape_list != NULL) && ((int) tape_list != -1)) {
        num_devices += tape_info.num;
    }

    /* see if any devices are available */
    if (num_devices == 0) {

        return_code = 1;

        /* Put up the no-devices panel */
        diag_display(MN_NOD_SA, catd, nodev_msg, DIAG_IO, 
                     ASL_DIAG_KEYS_ENTER_SC, NULL, NULL);

        clean_exit();
    }

    devices = (struct CuDv **) calloc(num_devices, sizeof(struct CuDv *));

    for (i=0; i<diskette_info.num; i++) {
        devices[i] = &diskette_list[i];
    }

    for (i=0; i<tape_info.num; i++) {
        devices[diskette_info.num + i] = &tape_list[i];
    }

    return(num_devices);

} /* end get_devices() */


/*
 * NAME: restore_genucode()
 *
 * FUNCTION: This procedure actually exec()s the commands that restore 
 *      genucode to /tmp and execute genucode.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: return code from diag_display() routine
 *         
 */
long restore_genucode() 
{
    char whichdev[9];
    char *temp_string;
    long rc;
    int status;
    char path[] = "/usr/bin/bsh";
    char command[] = "bsh";
    char command2[] = "genucode";
    char dev_opt[] = "-c";
    char sh_command[80];
    char *options[4];
    ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;
    struct stat file_stats;

    return_code = 1;

    /* configure selected and parent devices */
    if ((init_config_state = configure_device(device->name)) == -1) {

        /* fetch the text for the configure error panel */
        rc = diag_display(MN_CER_SA, catd, cfg_error, DIAG_MSGONLY,
                          ASL_DIAG_KEYS_ENTER_SC, &menutype, cfgerrinfo);

        /* fill in the backup device info */
        temp_string = (char *) malloc(512);
        sprintf(temp_string, cfgerrinfo[0].text, device->name);
        cfgerrinfo[0].text = temp_string;

        /* display the configure error panel */
        rc = diag_display(MN_CER_SA, catd, NULL, DIAG_IO, 
                          ASL_DIAG_KEYS_ENTER_SC, &menutype, cfgerrinfo);

        return (rc);
    }

    /* remove old file */
    remove(RESTORE_FILE);

    /* execute restore command */
    sprintf(sh_command,"cd /tmp; tar -xvf /dev/%s genucode",device->name);
    options[0] = command;
    options[1] = dev_opt;
    options[2] = sh_command;
    options[3] = NULL;

    if ((diag_asl_execute(path,options,&status) == DIAG_ASL_FAIL) ||
        (status) || (stat(RESTORE_FILE,&file_stats) != 0)) {

        /* fetch the text for the restore error panel */
        rc = diag_display(MN_ER1_SA, catd, restore_error, DIAG_MSGONLY,
                          ASL_DIAG_KEYS_ENTER_SC, &menutype, reserrinfo);

        /* fill in the restore device info */
        temp_string = (char *) malloc(512);
        sprintf(temp_string, reserrinfo[0].text, device->name);
        reserrinfo[0].text = temp_string;

        /* display the restore error panel */
        rc = diag_display(MN_ER1_SA, catd, NULL, DIAG_IO, 
                          ASL_DIAG_KEYS_ENTER_SC, &menutype, reserrinfo);

        goto exit;
    }

    /* execute restored genucode */
    options[0] = command2;
    options[1] = NULL;

    if (diag_asl_execute(RESTORE_FILE,options,&status) == DIAG_ASL_FAIL) {

        /* display the execute error panel */
        rc = diag_display(MN_ER2_SA, catd, exec_error, DIAG_IO, 
                          ASL_DIAG_KEYS_ENTER_SC, NULL, NULL);

        goto exit;
    }

    /* check genucode's exit code */
    if (status) {

        /* fetch the text for the genucode error panel */
        rc = diag_display(MN_ER3_SA, catd, genucode_error, DIAG_MSGONLY, 
                          ASL_DIAG_KEYS_ENTER_SC, &menutype, generrinfo);

        /* fill in genucode's exit code */
        temp_string = (char *) malloc(256);
        sprintf(temp_string, generrinfo[0].text, ((status >>8) & 0xff));
        generrinfo[0].text = temp_string;

        /* display the genucode error panel */
        rc = diag_display(MN_ER3_SA, catd, NULL, DIAG_IO, 
                          ASL_DIAG_KEYS_ENTER_SC, &menutype, generrinfo);

        goto exit;
    }

    return_code = 0;

    /* fetch the text for the completion panel */
    rc = diag_display(MN_COM_SA, catd, complete, DIAG_MSGONLY,
                      ASL_DIAG_KEYS_ENTER_SC, &menutype, compinfo);

    /* fill in the restore device info */
    temp_string = (char *) malloc(512);
    sprintf(temp_string, compinfo[0].text, device->name);
    compinfo[0].text = temp_string;

    /* display the completion panel */
    rc = diag_display(MN_COM_SA, catd, NULL, DIAG_IO, 
                      ASL_DIAG_KEYS_ENTER_SC, &menutype, compinfo);

exit:

    /* return selected and parent devices to their original states */
    initial_state(init_config_state, device->name);

    return (rc);

} /* end restore_genucode() */


/*
 * NAME: onintr()
 *
 * FUNCTION: This procedure handles the Control-C signal
 *
 * EXECUTION ENVIRONMENT: This procedure executes as a signal handler.
 *
 * RETURNS: NONE
 */
void onintr() {

    /* return selected and parent devices to their original states */
    if (init_config_state >= 0)
        initial_state(init_config_state, device->name);

    clean_exit();
 
} /* end onintr() */

/*
 * NAME: clean_exit()
 *
 * FUNCTION: Closes all open files, terminates ASL and ODM before
 * returning to the Diagnostic controller.
 *
 * EXECUTION ENVIRONMENT:  Diagnostic process or signal handler.
 *
 * RETURNS: 0 - last requested download was successful
 *         -1 - last requested download failed
 */
void clean_exit()
{

    /* cleanup /tmp */
    if (remove_ok)
        remove(RESTORE_FILE);

    /* free up ODM CuDv objects */
    if ((diskette_list != NULL) && ((int) diskette_list != -1))
        odm_free_list(diskette_list, &diskette_info);
    if ((tape_list != NULL) && ((int) tape_list != -1))
        odm_free_list(tape_list, &tape_info);

    /* quit ASL, close the catalog file, and terminate ODM */
    diag_asl_quit(NULL);
    catclose(catd);
    term_dgodm();

    /* return to SERVICE AIDS SELECTION MENU */
    exit(return_code);

} /* end clean_exit(); */
