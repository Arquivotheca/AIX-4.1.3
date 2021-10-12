static char sccsid[] = "@(#)57	1.4  src/bos/diag/util/ubackup/ubackrest.c, dsaubackup, bos411, 9436E411a 9/8/94 11:08:37";
/*
 *   COMPONENT_NAME: dsaubackup
 *
 *   FUNCTIONS: backup_file
 *              backup_nc
 *		check_tmp
 *		clean_exit
 *		get_crc
 *		get_devices
 *		get_errors
 *		main
 *		onintr
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
#include <fcntl.h>
#include <asl.h>
#include <locale.h>
#include <stdlib.h>

#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/cfgodm.h>    /* CuDv class def */

#include <diag/da.h>
#include <diag/diago.h>
#include <diag/diag.h>
#include <diag/diag_exit.h>

#include "ubackrest_msg.h"
#include "ubackrest.h"

#define RESTORE_FILE "/tmp/CEREADME"
#define ERROR_FILE "/tmp/brerr"
#define FMT_TAR 1
#define FMT_BACKUP 2
#define FMT_CPIO 3

/* entry panel */
struct msglist entry_msg[] = { 
                         {BR_SET, MENU_BR},
                         {BR_SET, MENU_BR_ACTION},
                         {0,      0}
};

/* no-file panel */
struct msglist nofile_msg[] = { 
                         {BR_SET, MENU_NOFILE},
                         {BR_SET, MENU_EXIT_ACTION},
                         {0,      0}
};

/* overwrite /tmp confirmation panel */
struct msglist tmp_conf[] = { 
                         {BR_SET, MENU_TMP_CONFIRM},
                         {BR_SET, MENU_CONFIRM_YES}, 
                         {BR_SET, MENU_CONFIRM_NO},
                         {BR_SET, MENU_CONFIRM_ACTION},
                         {0,      0}
};
ASL_SCR_INFO tmpinfo[DIAG_NUM_ENTRIES(tmp_conf)];

/* no-devices panel */
struct msglist nodev_msg[] = { 
                         {BR_SET, MENU_NODEV},
                         {BR_SET, MENU_EXIT_ACTION},
                         {0,      0}
};

/* format selection panel */
struct msglist fmt_select[] = {
                         {BR_SET, MENU_FMT_SELECT},
                         {BR_SET, MENU_FMT_SELECT_TAR},
                         {BR_SET, MENU_FMT_SELECT_BACKUP},
                         {BR_SET, MENU_FMT_SELECT_CPIO},
                         {BR_SET, MENU_FMT_SELECT_ACTION},
                         {0,      0}
};
ASL_SCR_INFO fmtinfo[DIAG_NUM_ENTRIES(fmt_select)];

/* device selection confirmation panel */
struct msglist confirm[] = { 
                         {BR_SET, MENU_CONFIRM},
                         {BR_SET, MENU_CONFIRM_YES}, 
                         {BR_SET, MENU_CONFIRM_NO},
                         {BR_SET, MENU_CONFIRM_ACTION},
                         {0,      0}
};
ASL_SCR_INFO devinfo[DIAG_NUM_ENTRIES(confirm)];

/* configure error panel */
struct msglist cfg_error[] = { 
                         {BR_SET, MENU_CFG_ERROR},
                         {BR_SET, MENU_RETURN_ACTION},
                         {0,      0}
};
ASL_SCR_INFO cfgerrinfo[DIAG_NUM_ENTRIES(cfg_error)];

/* backup panels */
struct msglist backup1[] = { 
                         {BR_SET, MENU_BACKUP1},
                         {0,      0}
};

struct msglist backup2[] = { 
                         {BR_SET, MENU_BACKUP2},
                         {BR_SET, MENU_BACKUP2_ACTION},
                         {0,      0}
};
ASL_SCR_INFO back2info[DIAG_NUM_ENTRIES(backup2)];

struct msglist backup_err[] = { 
                         {BR_SET, MENU_BACKUP_ERR},
                         {BR_SET, MENU_RETURN_ACTION},
                         {0,      0}
};
ASL_SCR_INFO back_err_info[DIAG_NUM_ENTRIES(backup_err)];

/* restore panels */
struct msglist restore1[] = { 
                         {BR_SET, MENU_RESTORE1},
                         {0,      0}
};

struct msglist restore2[] = { 
                         {BR_SET, MENU_RESTORE2},
                         {BR_SET, MENU_RESTORE2_ACTION},
                         {0,      0}
};

struct msglist restore_err[] = { 
                         {BR_SET, MENU_RESTORE_ERR},
                         {BR_SET, MENU_RETURN_ACTION},
                         {0,      0}
};
ASL_SCR_INFO res_err_info[DIAG_NUM_ENTRIES(restore_err)];

struct msglist file_err[] = { 
                         {BR_SET, MENU_FILE_ERR},
                         {BR_SET, MENU_RETURN_ACTION},
                         {0,      0}
};
ASL_SCR_INFO file_err_info[DIAG_NUM_ENTRIES(file_err)];

/* compare panels */
struct msglist compare1[] = { 
                         {BR_SET, MENU_COMPARE1},
                         {0,      0}
};
ASL_SCR_INFO cmp1info[DIAG_NUM_ENTRIES(compare1)];

struct msglist compare2[] = { 
                         {BR_SET, MENU_COMPARE2},
                         {BR_SET, MENU_RETURN_ACTION},
                         {0,      0}
};
ASL_SCR_INFO cmp2info[DIAG_NUM_ENTRIES(compare2)];

struct msglist compare_err[] = { 
                         {BR_SET, MENU_COMPARE_ERR},
                         {BR_SET, MENU_RETURN_ACTION},
                         {0,      0}
};
ASL_SCR_INFO cmp_err_info[DIAG_NUM_ENTRIES(compare_err)];

static char *diagdir;
static char back_file[256];

static nl_catd catd;

static int init_config_state = -1;

static int return_code = 0;
static int menu_mode = 1;

static struct CuDv **devices;
static struct CuDv *device;
static struct listinfo diskette_info;
static struct listinfo tape_info;
static struct CuDv *diskette_list;
static struct CuDv *tape_list;

extern char *optarg;

/* Function Declarations */
char *get_errors();
void onintr();
void clean_exit();
void select_device();
unsigned short get_crc();
extern char *diag_cat_gets(nl_catd, int, int);
extern nl_catd diag_catopen(char *, int);


/*
 * NAME: main()
 *
 * FUNCTION: This procedure is the entry point for the Backup/Restore Media
 *      Service Aid.
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
main(argc,argv)
int argc;
char *argv[];
{
    long rc;
    struct sigaction act;
    struct stat file_stats;

    /* handle Control-C like F10 (Exit) */
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    act.sa_handler = onintr;
    sigaction(SIGINT,&act,NULL);

    /* initialize locale environment */
    setlocale(LC_ALL, "");

    /* open the message catalog file */
    catd=diag_catopen(MF_UBACKREST, 0);

    /* initialize the ODM */
    if (init_dgodm() != 0) {
        return_code = 1;
        clean_exit();
    }

    /* get diagnostics directory */
    if ((diagdir = (char *)getenv("DIAGNOSTICS")) == NULL)
        diagdir = DIAGNOSTICS;

    sprintf(back_file,"%s/CEREADME",diagdir);

    /* check for calling mode (command-line or SA menu) */
    if (argc > 1) {

        menu_mode = 0;
        backup_nc(argc,argv);

    }
    else {    /* menu mode */

        /* initialize ASL */
        diag_asl_init("DEFAULT");

        /* Put up the entry panel */
        rc = diag_display(MN_BR_SA, catd, entry_msg, DIAG_IO, 
                          ASL_DIAG_KEYS_ENTER_SC, NULL, NULL);

        if (rc == DIAG_ASL_ENTER) {

            if (stat(back_file,&file_stats) == 0) {

                if ((check_tmp()) == DIAG_ASL_COMMIT) {

                    select_device();

                }
            }
            else {
                diag_display(MN_NOF_SA, catd, nofile_msg, DIAG_IO,
                             ASL_DIAG_KEYS_ENTER_SC, NULL, NULL);
            }
        }
    }

    clean_exit();

} /* end main() */


/*
 * NAME: backup_nc()
 *
 * FUNCTION: This procedure performs the backup/restore/compare of the test
 *      file in "no-console" mode; that is, without ASL menus & panels.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: none
 */
backup_nc (argc, argv)
int argc;
char *argv[];
{
    int c;
    int nc = 0;
    char dev_string[80] = "";
    char format_string[10] = "";
    char *text;
    char criteria[80];
    struct listinfo device_info;
    int format = 0;
    char sh_command1[256];
    char sh_command2[256];
    int size1, size2;
    unsigned short crc1, crc2;
    struct stat file_stats;

    return_code = 1;

    /* parse command-line options */
    while ((c = getopt(argc,argv,"cf:F:")) != EOF) {
      switch( c ) {

        /* no-console mode */
        case 'c':
            nc = 1;
            break;

        /* test device */
        case 'f':
            strcpy(dev_string,optarg);
            break;

        /* backup format */
        case 'F':
            strcpy(format_string,optarg);
            if (strcmp(format_string, "tar") == 0)
                format = FMT_TAR;
            else if (strcmp(format_string, "backup") == 0)
                format = FMT_BACKUP;
            else if (strcmp(format_string, "cpio") == 0)
                format = FMT_CPIO;
            break;

        /* print usage message for unknown options */
        default :
            text = diag_cat_gets(catd, BR_SET, USAGE_MESSAGE);
            fprintf(stderr,"\n%s\n",text);
            return;

      }
    }

    /* print usage message for incomplete options */
    if ((dev_string[0] == 0) || (format == 0) || (nc == 0)) {

        text = diag_cat_gets(catd, BR_SET, USAGE_MESSAGE);
        fprintf(stderr,"\n%s\n",text);
        return;

    }

    /* get CuDv pointer for specified device */
    sprintf(criteria,"name = %s AND chgstatus != 3",dev_string);
    device = get_CuDv_list(CuDv_CLASS, criteria, &device_info, 1, 2);

    if ((device == NULL) || ((int) device == -1))
        goto exit2;

    /* configure parent device and selected device */
    if ((init_config_state = configure_device(device->name)) == -1)
        goto exit2;

    /* setup proper backup/restore commands */
    switch (format) {
      case FMT_TAR :
        sprintf(sh_command1,
                "tar -cvf /dev/%s -C %s CEREADME",
                device->name, diagdir);
        sprintf(sh_command2,
                "cd /tmp; tar -xvf /dev/%s CEREADME",device->name);
        break;

      case FMT_BACKUP :
        sprintf(sh_command1,
                "cd %s; ls CEREADME | backbyname -iSqvf /dev/%s",
                diagdir, device->name);
        sprintf(sh_command2,
                "cd /tmp; restbyname -xSqvf /dev/%s CEREADME",
                device->name);
        break;

      default : /* only FMT_CPIO is left */
        /* use "raw" device for diskettes (e.g. rfd0 not fd0) */
        if (strcmp(device->ddins, "fd") == 0) {
            sprintf(sh_command1,
                    "cd %s; ls CEREADME | cpio -ov >/dev/r%s",
                    diagdir, device->name);
            sprintf(sh_command2,
                    "cd /tmp; cpio -iv </dev/r%s",device->name);
        }
        else {
            sprintf(sh_command1,
                    "cd %s; ls CEREADME | cpio -ov >/dev/%s",
                    diagdir, device->name);
            sprintf(sh_command2,
                    "cd /tmp; cpio -iv </dev/%s",device->name);
        }
        break;
    }

    /* remove old file */
    remove(RESTORE_FILE);

    /* execute backup and restore commands */
    if (system(sh_command1) != 0)
        goto exit;
    if (system(sh_command2) != 0)
        goto exit;

    /* compare original and restored files */
    stat(back_file, &file_stats);
    size1 = file_stats.st_size;
    stat(RESTORE_FILE, &file_stats);
    size2 = file_stats.st_size;

    crc1 = get_crc(back_file);
    crc2 = get_crc(RESTORE_FILE);

    if ((size1 == size2) && (crc1 == crc2)) {

        /* print success message */
        text = diag_cat_gets(catd, BR_SET, SUCCESS_MESSAGE);
        fprintf(stdout,"\n%s\n",text);

        return_code = 0;
    }

exit:

    /* return selected and parent devices to their original states */
    initial_state(init_config_state, device->name);
    
exit2:

    if (return_code) {

        /* print error message */
        text = diag_cat_gets(catd, BR_SET, ERROR_MESSAGE);
        fprintf(stderr,"\n%s\n",text);
    }

} /* end backup_nc() */


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

        /* display confirm panel */
        rc = diag_display(MN_TMP_SA, catd, tmp_conf, DIAG_IO, 
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
 * FUNCTION: This procedure allow the user to choose which device to test, 
 *      then calls backup_file(), passing that device.
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
    ASL_SCR_TYPE fmttype = DM_TYPE_DEFAULTS;
    ASL_SCR_INFO *selectinfo;
    char *dev_string;
    char *format_string;
    int format;

    /* get available devices */
    num_devices = get_devices();

    /* create the device selection menu */
    selecttype.max_index = num_devices + 1;  /* include the title & action */
    selectinfo = calloc(selecttype.max_index+1, sizeof(ASL_SCR_INFO));

    /* fill in title */
    selectinfo[0].text = diag_cat_gets(catd, BR_SET, MENU_DEV_SELECT);

    /* copy devices from temp_array */
    for (i=0;i<num_devices;i++) {
        selectinfo[i+1].text = (char *) malloc(128);
        sprintf(selectinfo[i+1].text,"%s\t\t%s\t\t%s",devices[i]->name,
                devices[i]->location,
                get_dev_desc(devices[i]->name));
    }

    /* fill in action */
    selectinfo[i+1].text = diag_cat_gets(catd, BR_SET, 
                                         MENU_DEV_SELECT_ACTION);
  
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

            /* display format selection menu */
            rc = diag_display(MN_FMT_SA, catd, fmt_select, DIAG_IO,
                              ASL_DIAG_LIST_CANCEL_EXIT_SC, &fmttype, fmtinfo);

            if (rc == DIAG_ASL_COMMIT) {

                /* selecttype.cur_index is the selected format */
                format = fmttype.cur_index;
                format_string = fmtinfo[format].text;

                /* fetch the text for the confirm panel */
                rc = diag_display(MN_DEV_SA, catd, confirm, DIAG_MSGONLY,
                                  ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype,
                                  devinfo);

                /* fill in the backup device, format info */
                temp_string = (char *) malloc(512);
                sprintf(temp_string, devinfo[0].text, format_string,
                        dev_string);
                devinfo[0].text = temp_string;

                /* now that the dynamic data is inserted, display panel */
                rc = diag_display(MN_DEV_SA, catd, NULL, DIAG_IO,
                                  ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype,
                                  devinfo);

                if (rc == DIAG_ASL_COMMIT) {

                    if (DIAG_ITEM_SELECTED(menutype) == 1) {

                        /* backup, restore, and compare file */
                        rc = backup_file(format);

                    } /* end if (confirm ok) */

                } /* end if (confirm selected) */

            } /* end if (format selected) */

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

    /* NOTE: get_CuDv_list() and CuDv_Class defined in <cfgodm.h> */

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

}


/*
 * NAME: backup_file()
 *
 * FUNCTION: This procedure actually exec()s the commands that backup and
 *      restore the test file, and compares the original and backed-up files.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: return code from diag_display() routine
 *
 */
long backup_file(format) 
int format;
{
    char whichdev[9];
    char *temp_string;
    long rc;
    int status;
    char path[] = "/usr/bin/bsh";
    char command[] = "bsh";
    char dev_opt[] = "-c";
    char sh_command1[256];
    char sh_command2[256];
    char *options[4];
    char *errors;
    ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;
    struct stat file_stats;
    int size1, size2;
    unsigned short crc1, crc2;

    return_code = 1;

    /* configure selected and parent devices */
    if ((init_config_state = configure_device(device->name)) == -1) {

        /* fetch the text for the configure error panel */
        rc = diag_display(MN_CER_SA, catd, cfg_error, DIAG_MSGONLY,
                          ASL_DIAG_KEYS_ENTER_SC, &menutype, cfgerrinfo);

        /* fill in the test device info */
        temp_string = (char *) malloc(512);
        sprintf(temp_string, cfgerrinfo[0].text, device->name);
        cfgerrinfo[0].text = temp_string;

        /* display the configure error panel */
        rc = diag_display(MN_CER_SA, catd, NULL, DIAG_IO, 
                          ASL_DIAG_KEYS_ENTER_SC, &menutype, cfgerrinfo);

        goto exit;

    }

    /* setup proper backup/restore commands */
    switch (format) {
      case FMT_TAR :
        sprintf(sh_command1,
               "tar -cvf /dev/%s -C %s CEREADME 2>/tmp/brerr",
                device->name, diagdir);
        sprintf(sh_command2,
                "cd /tmp; tar -xvf /dev/%s CEREADME 2>/tmp/brerr",device->name);
        break;

      case FMT_BACKUP :
        sprintf(sh_command1,
                "cd %s; ls CEREADME | backbyname -iSqvf /dev/%s 2>/tmp/brerr",
                diagdir, device->name);
        sprintf(sh_command2,
                "cd /tmp; restbyname -xSqvf /dev/%s CEREADME 2>/tmp/brerr",
                device->name);
        break;

      default : /* only FMT_CPIO is left */
        /* use "raw" device for diskettes (e.g. rfd0 not fd0) */
        if (strcmp(device->ddins, "fd") == 0) {
            sprintf(sh_command1,
                    "cd %s; ls CEREADME | cpio -ov 2>/tmp/brerr >/dev/r%s",
                    diagdir, device->name);
            sprintf(sh_command2,
                    "cd /tmp; cpio -iv 2>/tmp/brerr </dev/r%s",device->name);
        }
        else {
            sprintf(sh_command1,
                    "cd %s; ls CEREADME | cpio -ov 2>/tmp/brerr >/dev/%s",
                    diagdir, device->name);
            sprintf(sh_command2,
                    "cd /tmp; cpio -iv 2>/tmp/brerr </dev/%s",device->name);
        }
        break;
    }

    /* remove old file */
    remove(RESTORE_FILE);

    /****************************************************************
    * backup file                                                   *
    ****************************************************************/

    /* display backup panel */
    diag_display(MN_BK1_SA, catd, backup1, DIAG_IO, 
                 ASL_DIAG_OUTPUT_LEAVE_SC, NULL, NULL);

    /* execute backup command */
    options[0] = command;
    options[1] = dev_opt;
    options[2] = sh_command1;
    options[3] = NULL;

    if (((rc = diag_asl_execute(path,options,&status)) == DIAG_ASL_FAIL) ||
        (status)) {

        /* fetch the text for the backup error panel */
        diag_display(MN_BER_SA, catd, backup_err, DIAG_MSGONLY,
                     ASL_DIAG_KEYS_ENTER_SC, &menutype, back_err_info);

        errors = get_errors();

        /* fill in the backup device info */
        temp_string = (char *) malloc(512);
        sprintf(temp_string, back_err_info[0].text, device->name, errors);
        back_err_info[0].text = temp_string;

        /* display the backup error panel */
        rc = diag_display(MN_BER_SA, catd, NULL, DIAG_IO,
                          ASL_DIAG_KEYS_ENTER_SC, &menutype, back_err_info);

        goto exit;
    }

    /* fetch the text for the successful backup panel */
    diag_display(MN_BK2_SA, catd, backup2, DIAG_MSGONLY,
                 ASL_DIAG_KEYS_ENTER_SC, &menutype, back2info);

    /* fill in the backup device info */
    temp_string = (char *) malloc(512);
    sprintf(temp_string, back2info[0].text, device->name);
    back2info[0].text = temp_string;

    /* display the successful backup panel */
    rc = diag_display(MN_BK2_SA, catd, NULL, DIAG_IO,
                      ASL_DIAG_KEYS_ENTER_SC, &menutype, back2info);

    if ((rc == DIAG_ASL_EXIT) || (rc == DIAG_ASL_CANCEL)) {
        goto exit;
    }

    /****************************************************************
    * restore file                                                  *
    ****************************************************************/

    /* display restore panel */
    diag_display(MN_RS1_SA, catd, restore1, DIAG_IO, 
                 ASL_DIAG_OUTPUT_LEAVE_SC, NULL, NULL);

    /* execute restore command */
    options[2] = sh_command2;

    if (((rc = diag_asl_execute(path,options,&status)) == DIAG_ASL_FAIL) ||
        (status)) {

        /* fetch the text for the restore error panel */
        diag_display(MN_RER_SA, catd, restore_err, DIAG_MSGONLY,
                     ASL_DIAG_KEYS_ENTER_SC, &menutype, res_err_info);

        errors = get_errors();

        /* fill in the restore device info */
        temp_string = (char *) malloc(512);
        sprintf(temp_string, res_err_info[0].text, device->name, errors);
        res_err_info[0].text = temp_string;

        /* display the restore error panel */
        rc = diag_display(MN_RER_SA, catd, NULL, DIAG_IO,
                          ASL_DIAG_KEYS_ENTER_SC, &menutype, res_err_info);

        goto exit;
    }

    if (stat(RESTORE_FILE,&file_stats) != 0) {

        /* fetch the text for the restore file error panel */
        diag_display(MN_NER_SA, catd, file_err, DIAG_MSGONLY,
                     ASL_DIAG_KEYS_ENTER_SC, &menutype, file_err_info);

        /* fill in the restore device info */
        temp_string = (char *) malloc(128);
        sprintf(temp_string, file_err_info[0].text, device->name);
        file_err_info[0].text = temp_string;

        /* display the restore file error panel */
        rc = diag_display(MN_NER_SA, catd, NULL, DIAG_IO,
                          ASL_DIAG_KEYS_ENTER_SC, &menutype, file_err_info);

        goto exit;
    }

    /* display successful restore panel */
    rc = diag_display(MN_RS2_SA, catd, restore2, DIAG_IO,
                      ASL_DIAG_KEYS_ENTER_SC, NULL, NULL);

    if ((rc == DIAG_ASL_EXIT) || (rc == DIAG_ASL_CANCEL)) {
        goto exit;
    }

    /****************************************************************
    * compare files                                                 *
    ****************************************************************/

    /* display compare panel */
    diag_display(MN_CM1_SA, catd, compare1, DIAG_IO, 
                 ASL_DIAG_OUTPUT_LEAVE_SC, NULL, NULL);

    /* compare original and restored files */
    stat(back_file, &file_stats);
    size1 = file_stats.st_size;
    stat(RESTORE_FILE, &file_stats);
    size2 = file_stats.st_size;

    crc1 = get_crc(back_file);
    crc2 = get_crc(RESTORE_FILE);

    if ((size1 != size2) || (crc1 != crc2)) {

        /* fetch the text for the compare error panel */
        rc = diag_display(MN_CM2_SA, catd, compare_err, DIAG_MSGONLY,
                          ASL_DIAG_KEYS_ENTER_SC, &menutype, cmp_err_info);

        /* fill in the compare info */
        temp_string = (char *) malloc(512);
        sprintf(temp_string, cmp_err_info[0].text, size1, crc1, size2, crc2);
        cmp_err_info[0].text = temp_string;

        /* display the successful compare panel */
        rc = diag_display(MN_CM2_SA, catd, NULL, DIAG_IO,
                          ASL_DIAG_KEYS_ENTER_SC, &menutype, cmp_err_info);

        goto exit;
    }

    return_code = 0;

    /* fetch the text for the successful compare panel */
    rc = diag_display(MN_CM2_SA, catd, compare2, DIAG_MSGONLY,
                      ASL_DIAG_KEYS_ENTER_SC, &menutype, cmp2info);

    /* fill in the compare info */
    temp_string = (char *) malloc(512);
    sprintf(temp_string, cmp2info[0].text, size1, crc1, size2, crc2);
    cmp2info[0].text = temp_string;

    /* display the successful compare panel */
    rc = diag_display(MN_CM2_SA, catd, NULL, DIAG_IO,
                      ASL_DIAG_KEYS_ENTER_SC, &menutype, cmp2info);

exit:

    /* return selected and parent devices to their original states */
    initial_state(init_config_state, device->name);

    return (rc);

} /* end backup_file() */


/*
 * NAME: get_crc()
 *
 * FUNCTION: This procedure calculates the CRC-16 value of the file whose name
 *      is passed as the input parameter.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: calculated CRC-16 value
 *
 */
static unsigned short crc_table[256] = {
0x0,  0xc0c1,  0xc181,  0x140,  0xc301,  0x3c0,  0x280,  0xc241,
0xc601,  0x6c0,  0x780,  0xc741,  0x500,  0xc5c1,  0xc481,  0x440,
0xcc01,  0xcc0,  0xd80,  0xcd41,  0xf00,  0xcfc1,  0xce81,  0xe40,
0xa00,  0xcac1,  0xcb81,  0xb40,  0xc901,  0x9c0,  0x880,  0xc841,
0xd801,  0x18c0,  0x1980,  0xd941,  0x1b00,  0xdbc1,  0xda81,  0x1a40,
0x1e00,  0xdec1,  0xdf81,  0x1f40,  0xdd01,  0x1dc0,  0x1c80,  0xdc41,
0x1400,  0xd4c1,  0xd581,  0x1540,  0xd701,  0x17c0,  0x1680,  0xd641,
0xd201,  0x12c0,  0x1380,  0xd341,  0x1100,  0xd1c1,  0xd081,  0x1040,
0xf001,  0x30c0,  0x3180,  0xf141,  0x3300,  0xf3c1,  0xf281,  0x3240,
0x3600,  0xf6c1,  0xf781,  0x3740,  0xf501,  0x35c0,  0x3480,  0xf441,
0x3c00,  0xfcc1,  0xfd81,  0x3d40,  0xff01,  0x3fc0,  0x3e80,  0xfe41,
0xfa01,  0x3ac0,  0x3b80,  0xfb41,  0x3900,  0xf9c1,  0xf881,  0x3840,
0x2800,  0xe8c1,  0xe981,  0x2940,  0xeb01,  0x2bc0,  0x2a80,  0xea41,
0xee01,  0x2ec0,  0x2f80,  0xef41,  0x2d00,  0xedc1,  0xec81,  0x2c40,
0xe401,  0x24c0,  0x2580,  0xe541,  0x2700,  0xe7c1,  0xe681,  0x2640,
0x2200,  0xe2c1,  0xe381,  0x2340,  0xe101,  0x21c0,  0x2080,  0xe041,
0xa001,  0x60c0,  0x6180,  0xa141,  0x6300,  0xa3c1,  0xa281,  0x6240,
0x6600,  0xa6c1,  0xa781,  0x6740,  0xa501,  0x65c0,  0x6480,  0xa441,
0x6c00,  0xacc1,  0xad81,  0x6d40,  0xaf01,  0x6fc0,  0x6e80,  0xae41,
0xaa01,  0x6ac0,  0x6b80,  0xab41,  0x6900,  0xa9c1,  0xa881,  0x6840,
0x7800,  0xb8c1,  0xb981,  0x7940,  0xbb01,  0x7bc0,  0x7a80,  0xba41,
0xbe01,  0x7ec0,  0x7f80,  0xbf41,  0x7d00,  0xbdc1,  0xbc81,  0x7c40,
0xb401,  0x74c0,  0x7580,  0xb541,  0x7700,  0xb7c1,  0xb681,  0x7640,
0x7200,  0xb2c1,  0xb381,  0x7340,  0xb101,  0x71c0,  0x7080,  0xb041,
0x5000,  0x90c1,  0x9181,  0x5140,  0x9301,  0x53c0,  0x5280,  0x9241,
0x9601,  0x56c0,  0x5780,  0x9741,  0x5500,  0x95c1,  0x9481,  0x5440,
0x9c01,  0x5cc0,  0x5d80,  0x9d41,  0x5f00,  0x9fc1,  0x9e81,  0x5e40,
0x5a00,  0x9ac1,  0x9b81,  0x5b40,  0x9901,  0x59c0,  0x5880,  0x9841,
0x8801,  0x48c0,  0x4980,  0x8941,  0x4b00,  0x8bc1,  0x8a81,  0x4a40,
0x4e00,  0x8ec1,  0x8f81,  0x4f40,  0x8d01,  0x4dc0,  0x4c80,  0x8c41,
0x4400,  0x84c1,  0x8581,  0x4540,  0x8701,  0x47c0,  0x4680,  0x8641,
0x8201,  0x42c0,  0x4380,  0x8341,  0x4100,  0x81c1,  0x8081,  0x4040,
} ;
 
/* Generate CRC-16 value of specified file */
unsigned short get_crc(filename)
char *filename;
{
    int fd;
    int nr;
    char buf[4096];
    char *buffptr;
    int i, index;
    unsigned short crc = 0;

    /* open file to be CRCed */
    fd = open(filename, O_RDONLY);
 
    /* loop, computing CRC on 4k at a time */
    while ((nr = read(fd, buf, 4096)) > 0) {
    
        buffptr = buf;
        for (i=0; i<nr; i++, buffptr++) {

            index = ((crc ^ *buffptr) & 0x0ff);
            crc = ((crc >> 8) & 0x00ff) ^ crc_table[index];
        }

    }

    close(fd);
    return(crc);

} /* end get_crc() */


/*
 * NAME: get_errors()
 *
 * FUNCTION: This procedure retrieves stderr messages written to a temporary
 *      file.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: pointer to error string
 */
char *get_errors() {
  
    char *string_ptr = NULL;
    int fd;
    struct stat file_stats;

    if (stat(ERROR_FILE,&file_stats) == 0) {

        if (file_stats.st_size > 0) {

            fd = open(ERROR_FILE,O_RDONLY,0);
            string_ptr = (char *) malloc(file_stats.st_size);
            read(fd,string_ptr,file_stats.st_size);
            string_ptr[file_stats.st_size-1] = 0;

        }

    }

    return(string_ptr);

} /* end get_errors() */


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
 * EXECUTION ENVIRONMENT: Diagnostic process or signal handler.
 *
 * RETURNS: 0 - last requested backup/restore/compare was successful
 *         -1 - last requested backup/restore/compare failed
 */
void clean_exit()
{
    int standalone;

    /* cleanup /tmp */
    ipl_mode(&standalone);
    if (standalone == DIAG_TRUE)
        remove(RESTORE_FILE);

    /* free up ODM CuDv objects */
    if ((diskette_list != NULL) && ((int) diskette_list != -1))
        odm_free_list((char *)diskette_list, &diskette_info);
    if ((tape_list != NULL) && ((int) tape_list != -1))
        odm_free_list((char *)tape_list, &tape_info);

    /* quit ASL, close the catalog file, and terminate ODM */
    if (menu_mode) {
        remove(ERROR_FILE);
        diag_asl_quit(NULL);
    }
    catclose(catd);
    term_dgodm();

    /* return to SERVICE AIDS SELECTION MENU when done */
    exit(return_code);

} /* end clean_exit() */
