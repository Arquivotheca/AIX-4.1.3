static char sccsid[] = "@(#)04	1.5  src/bos/diag/util/udiskenh/udmdisp.c, dsaudiskenh, bos411, 9435A411a 8/25/94 12:54:52";
/*
 *   COMPONENT_NAME: DSAUDISKMNT
 *
 *   FUNCTIONS: disp_alt
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

#pragma options nosource
#include <odmi.h>          /* odm commands */
#include <sys/scsi.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "udmutil.h"
#include "udiskmnt_msg.h"
#include "udmhexit.h"
#pragma options source

extern nl_catd cat_fd;

/*
 * NAME: disp_alt()
 *
 * FUNCTION: This procedure manages the display/alter sector operation.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: The return code equals the return code from the last call
 *          to diag_display (ASL_EXIT, ASL_END, etc.).
 *
 */


int disp_alt() {
    struct CuDv  edit_disk, *hd_list = NULL;
    struct listinfo hd_list_info;
    int num_disks, rc, temp, i;
    int disk_fd = -1;  /* Disk file descriptor. */
    int ids = -1;      /* Initial disk state.   */
    char *edit_path = NULL;
    char dev_dir[] = "/dev/";  /* Directory where device files reside.*/
    char *edit_desc = NULL; /* Disk description to display.  */
    int lvm_disk = 0; /* 1 => source disk is in LVM. */
    int lvm_removed = 0 ;/* 1 => source disk removed from LVM. */ 
    struct read_capacity_data_t capacity;
    int cont = 1; /* A boolean indicating whether to continue or not */

/* Display info panel. */
    if ((rc = disp_err(DISP_ALTER_INFO_MENU,
                       TITLES, DISP_ALTER_TITLE,
                       DISP_ALTER_SET, DISP_ALTER_INFO,
                       NULL, NULL, NULL ,0)) != ASL_ENTER)  
	cont = 0;
    else {
        hd_list = get_devices(
       "chgstatus != 3 and PdDvLn != disk/dar/array and PdDvLn LIKE disk/*",
        &hd_list_info);
        num_disks = hd_list_info.num;

        if (num_disks == 0) {
	    /* Display "no disks found" message and return */
            cont = 0;
            rc = disp_err(DISP_ALTER_NO_DISKS,
                               TITLES, DISP_ALTER_TITLE,
                               DISP_ALTER_SET, NO_DISKS,
                               NULL, NULL, NULL ,0); 
        }
        else {
	    /* Display the disk selection menus. */
            rc = select_device(DISP_ALTER_DISK,
			      TITLES, DISP_ALTER_TITLE,
			      INSTRUCTIONS, SELECT_INSTRUCTION,
			      hd_list,
			      num_disks,
                              &temp);
            if (cont = (rc == ASL_COMMIT)) {
                edit_disk = hd_list[temp];
                edit_desc = dev_desc(&edit_disk);
                edit_path = disk_path(dev_dir,edit_disk.name);
            }
        }
    }

    if (!cont) {
        /* Free any memory that has been allocated and return the rc. */
        /* The FREE macro checks for a NULL value before calling free.*/
        FREE(edit_desc);
        FREE(edit_path);
        if (hd_list) odm_free_list(hd_list, &hd_list_info); 
        return rc;
    }

    if ((ids=configure_device(edit_disk.name)) == -1 ||
       (disk_fd=openx(edit_path, O_RDWR, 0, SC_DIAGNOSTIC)) == -1 ||
        read_capacity(disk_fd, &capacity) == -1){
        /* Edited disk should NOT be in use, etc. */
        cont = 0;
        rc = disp_err(DISP_ALTER_DISK_WONT_OPEN, 
                      TITLES, DISP_ALTER_TITLE,
                      DISP_ALTER_SET, DISK_WONT_OPEN,
                      edit_desc, NULL, NULL, 1);
    }
    else {
/* 
 * Display the sector selection menu and sector edit panel.  Use a
 * "state machine" to determine what panel to display next based on
 * errors and PF keys.  Most of the variables for these two menus are
 * declared in this block.
 * The edit screen itself is not displayed via ASL, but by an existing
 * tool which was customized for this service aid.  The interface to
 * that tool accepts the title line, PF key line, and other text which
 * it displays on the edit screen.  That text needs to come from a
 * nessage file.  The title line which contains the menu number must
 * be "built" here.  Declare the necessary variables and fetch the
 * necessary messages.  
*/
        struct sense_data_t sense_data;
        char *temp;
        int last_block = capacity.block_address;
        int cur_block;       /* The chosen block (integer form). */
        char sector_str[10]; /* The chosen block (string form).  */
        struct msglist sect_list[] = {{TITLES, DISP_ALTER_TITLE},
                                       {DISP_ALTER_SET, SECTOR_NUMBER},
                                       {DISP_ALTER_SET, ENTER_SECTOR},
                                        NULL};
        ASL_SCR_INFO sect_info[DIAG_NUM_ENTRIES(sect_list)]; 
        ASL_SCR_TYPE sect_type = DM_TYPE_DEFAULTS;
        enum state_t {quit, select_sector, edit} state;
        char *raw_edit_title = diag_cat_gets(cat_fd,
                                             TITLES,DISP_ALTER_TITLE);
        char edit_title[78];
        char *edit_subtitle = NULL;
        char *raw_edit_sector_disk = diag_cat_gets(cat_fd,
                                                   DISP_ALTER_SET,
                                                   DESC_STR);
        char edit_sector_disk[78];   
        char *edit_inst = diag_cat_gets(cat_fd,DISP_ALTER_SET,INST_STR);
        char *edit_offset = diag_cat_gets(cat_fd,
                                          DISP_ALTER_SET,OFFSET_STR);
        char *ascii_window = diag_cat_gets(cat_fd, 
                                          DISP_ALTER_SET, ASCII_WINDOW);
        char *hex_window = diag_cat_gets(cat_fd,
                                          DISP_ALTER_SET, HEX_WINDOW);
        char *edit_keys = diag_cat_gets(cat_fd,DISP_ALTER_SET,KEYS_STR);
/*
 * The hex editor is called as a separate program because when called
 * as a function, its use of curses seems to interfere with ASL's use
 * use of curses and lock up the keyboard.  The udmedit program is   
 * invoked with the diag_asl_execute function.  This function 
 * takes an array of pointers to strings (edit_args declared below).
 * since the two programs
 * have different address spaces, shared memory is used
 * to make the edited buffer available to both programs.  The
 * status returned by diag_asl_execute is placed in the status variable.
 */
        char* edit_args[11]; 
        char *io_buffer;     /* Ptr to shared memory. */
        int shm_id;          /* Integer form of shared mem id. */
        char asc_shm_id[10]; /* Ascii form of shared mem id. */
        int *edit_rc;
        #pragma options align=packed
        struct status_t {
          short pad;
          char  exit_rc;
          unsigned core_dump_bool :1;
          unsigned signal_num     :7; } status ;
        #pragma options align=reset
        int *status_ptr = (int*)&status;
/********** END OF DECLARATIONS **************************************/
/*
 * There should be two newline characters in the raw_edit_title string
 * which separate the main title from the subtitle. Split the raw title
 * into the main and subtitle strings, and append the menu number to
 * the main title so it appears in the upper right corner.
*/
        temp = strchr(raw_edit_title,'\n');
        edit_subtitle = temp + 2; /* Don't ever free this! */
        *temp = NULL;
        i = strlen(raw_edit_title);
        strcpy(edit_title, raw_edit_title);
        *temp = '\n'; /* fix it back or this won't work twice. */
        /* Assume menu number is always 6 chars. */
        /* i is now the index of the null terminator of edit_title. */
        while (i <= 71) edit_title[i++] = ' ';
        edit_title[i] = '\0'; 
        temp = malloc(7); 
        sprintf(temp,"%06X\0",DISP_ALTER_EDIT);
        strcat(edit_title,temp);
        FREE(temp);

/* Fetch the message text for the sector selection menu without  */
/* displaying it.*/

        rc = diag_display(DISP_ALTER_SECTOR,
                          cat_fd, sect_list, DIAG_MSGONLY, 
                          ASL_DIAG_DIALOGUE_SC, &sect_type, sect_info);
/*
 * Insert the selected disk's description into the menu's last line.
*/
        temp = malloc(strlen(sect_info[2].text) + 
                      strlen(edit_desc) + 1);
        if (temp == NULL) genexit(-1);
        sprintf(temp, sect_info[2].text, edit_desc);
        sect_info[2].text = temp;
/*
 * The third line needs the range of valid sector numbers inserted 
 * as a prompt for the user. 
*/
        temp = malloc(strlen(sect_info[1].text) + 11);
        if (temp == NULL) genexit(-1);
        sprintf(temp, sect_info[1].text, last_block);
        sect_info[1].text = temp;
        temp = NULL;
/*
 * Now set up the input line.
*/
        sect_info[1].required = ASL_YES;
        sect_info[1].op_type = ASL_LIST_ENTRY; 
        sect_info[1].entry_type = ASL_NUM_ENTRY;
        sect_info[1].disp_values = malloc(1);
        sect_info[1].disp_values[0] = '\0';
        sect_info[1].entry_size = 10;
        sect_info[1].data_value = malloc(11);
        if (sect_info[1].data_value == NULL) genexit(-1);
        sect_info[1].data_value[0] = '0';
        sect_info[1].data_value[1] = '\0';

        sect_info[2].disp_values = NULL; /* Do this or junk sometimes */
        sect_info[2].data_value = NULL;  /* appears on the screen.    */
/* 
 * Now set up the shared memory segment which is used to share the
 * edited disk block between this program and the udmedit program.
 */
        shm_id = shmget(IPC_PRIVATE, BUF_SIZE, 0600 | IPC_CREAT);
        if (shm_id == -1) {
#ifdef DBG
            fprintf (stderr, "Shmget failed, errno = %d.\n", errno);
#endif
            genexit(-1);
        }
        sprintf (asc_shm_id, "%d\0",shm_id);
        io_buffer = shmat(shm_id, 0, 0);
        if (io_buffer ==  (char*) -1) {
#ifdef DBG
            fprintf (stderr, "Shmat failed, errno = %d.\n", errno);
#endif
            genexit(-1);
        }
/*
 * The io_buffer is larger than the disk block_size because it is
 * padded to fill the screen when the user scrolls to the bottom
 * of the hex_edit display.  The pad area is displayed (though it
 * it is not editable), so ensure it set to nulls for the sake
 * of appearance.
 */
        memset(io_buffer, 0, BUF_SIZE);
/*
 * Set up the "static" elements of the edit_args before entering
 * the state machine (i.e. the while loop).  Edit_args[3] contains
 * the currently selected sector number and is therefore updated
 * after displaying the sector selection screen.
 */
        edit_args[0] = "udmedit";
        edit_args[1] = edit_title; 
        edit_args[2] = edit_subtitle;
	edit_args[4] = edit_inst; 
        edit_args[5] = edit_offset; 
	edit_args[6] = edit_keys;
        edit_args[7] = hex_window; 
        edit_args[8] = ascii_window;
        edit_args[9] = asc_shm_id;
        edit_args[10] = NULL;
        state = select_sector;
        while (state != quit) {
            switch(state) {
                case select_sector:
                    rc = diag_display(DISP_ALTER_SECTOR,
                                cat_fd, NULL, DIAG_IO, 
                                ASL_DIAG_DIALOGUE_SC,
                                &sect_type, sect_info);
#ifdef DBG 
                    fprintf(stderr,"RC from sector selection = %d.\n",
                            rc);
#endif
/*
 * Ignore all keys except Cancel, Exit, Enter, and Commit.  Others
 * are no-ops (state remains select_sector and panel is redisplayed).
 */
                    if (rc == ASL_CANCEL || rc == ASL_EXIT) state=quit;
                    else if (rc == ASL_ENTER || rc == ASL_COMMIT) {
                        cur_block = atoi(sect_info[1].data_value);
                        sprintf(sector_str,"%d",cur_block);
#ifdef DBG 
                        fprintf(stderr,"Selected block = %d.\n",
                                       cur_block);
#endif
                        if (get_block(disk_fd,scsi,cur_block,1,
                                      io_buffer,&sense_data)) {
                            /* Ignore recovered errors. */
                            if (sense_data.sense_key != 1) {
                                rc = disp_err(DISP_ALTER_READ_ERR,
                                              TITLES, DISP_ALTER_TITLE,
                                              DISP_ALTER_SET, 
                                              DISP_ALTER_READ_ERROR,
                                              sector_str,
                                              NULL, NULL, 1);
                                if (rc != ASL_ENTER) state = quit;
                            } /* end if recovered error */
                            else state = edit;
                        } /* end if read error */
                        else state = edit;
                    } /* end if rc */
                    break;
                case edit:
                    sprintf(edit_sector_disk,raw_edit_sector_disk,
                            cur_block, edit_desc);
                    edit_args[3] = edit_sector_disk; 
                    *status_ptr = 0;
#ifdef DBG
  fprintf (stderr, "Asc_shm_id = %s, shm_id = %d.\n",asc_shm_id,shm_id);
#endif
                    rc = diag_asl_execute("/usr/lpp/diagnostics/bin/udmedit",
                                          edit_args, status_ptr);
#ifdef DBG
  fprintf (stderr, "RC from diag_asl_execute = %d.\n", rc);
  fprintf (stderr, "Status = {pad= %d,\n", status.pad);
  fprintf (stderr, "          exit_rc = %d,\n", status.exit_rc);
  fprintf (stderr, "          core_dump_bool = %d,\n", status.core_dump_bool);
  fprintf (stderr, "          signal_num = %d}.\n", status.signal_num);
#endif
                    rc = status.exit_rc;
                    if (rc == ASL_CANCEL) state = select_sector;
                    if (rc == ASL_EXIT)   state = quit;
                    if (rc == ASL_COMMIT) {
                        /* Get write confirmation from user. */
                        ASL_SCR_INFO conf_info[2];
                        ASL_SCR_TYPE conf_type = DM_TYPE_DEFAULTS;
                        conf_type.max_index = 1;
                        conf_info[0].text = diag_cat_gets(cat_fd,
                                                     TITLES,
                                                     DISP_ALTER_TITLE); 
                        conf_info[1].text = diag_cat_gets(cat_fd,
                                                    DISP_ALTER_SET,
                                                    WRITE_CONFIRMATION);
                        temp = malloc(strlen(conf_info[1].text) + 10);
                        sprintf(temp,conf_info[1].text, cur_block);
                        conf_info[1].text = temp;
                        rc = diag_display(DISP_ALTER_CONF, cat_fd, NULL,
                                          DIAG_IO,
                                          ASL_DIAG_KEYS_ENTER_SC, 
                                          &conf_type, conf_info);
                        FREE(temp);
                        /* if rc = cancel, do nothing */
                        if (rc == ASL_EXIT) state = quit;
                        if (rc == ASL_ENTER) {
                            if (put_block(disk_fd,cur_block,1,
                                          io_buffer,NULL)) {
                                /* Display the write error panel */
                                rc = disp_err(DISP_ALTER_WRITE_ERR,
                                              TITLES, DISP_ALTER_TITLE,
                                              DISP_ALTER_SET,
                                              DISP_ALTER_WRITE_ERROR,
                                              sector_str, NULL, NULL,1);
                                if (rc == ASL_EXIT) state = quit;
                            } /* end if put block fails. */
                        } /* end if write is confirmed. */
                    } /* end if commit from edit panel. */
            } /* end switch */
        } /* end while */ 
        FREE(sect_info[1].text);
        FREE(sect_info[2].text);
        FREE(sect_info[1].data_value);
        shmctl(shm_id, IPC_RMID, NULL);
    } /* end if disk opened ok */
    if (disk_fd > 0) close(disk_fd);
    if (ids >= 0) initial_state(ids, edit_disk.name);
    FREE(edit_desc);
    FREE(edit_path);
    if (hd_list) odm_free_list(hd_list, &hd_list_info); 
    return rc;
} /* end disp_alt */
