static char sccsid[] = "@(#)03	1.7  src/bos/diag/util/udiskenh/udmcopy.c, dsaudiskenh, bos411, 9435A411a 8/25/94 12:54:39";
#pragma options nosource
#include <odmi.h>
#include <sys/scsi.h>
#include <sys/hd_psn.h>
#include <lvmrec.h>
#include <lvm.h>
#include <sys/bbdir.h>
#include <sys/bootrecord.h>
#include <sys/stat.h>
#include "udmutil.h"
#include "udiskmnt_msg.h"
#include "udmbbdir.h"
#include <sys/devinfo.h>
#include <sys/ioctl.h>
#pragma options source

extern nl_catd cat_fd;
const int max_write_errors = 2;
 
/*
 * NAME: diskcopy()
 *
 * FUNCTION: This procedure manages the disk to disk copy operation. 
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: The return code equals the return code from the last call
 *          to diag_display (ASL_EXIT, ASL_END, etc.).
 *
 */

int diskcopy() {
    /* structures for source/target disks, and a list to choose from. */
    struct CuDv  from_disk, to_disk,
                 *hd_list = NULL, *static_hd_list = NULL;
    struct listinfo hd_list_info;
    int *disk_size = NULL;  /* An array of sizes for each disk */
    int num_disks, rc, temp, i;
    int last_block; /* Last block address of source disk. */
    int last_target_block; /* Last block address of target disk. */
    int s_fd = -1, t_fd = -1;  /* Source and target file descriptors. */
    int iss = -1, its = -1; /*Initial source state, init target state.*/
    char *temp_path = NULL;
    char *source_path = NULL;
    char *target_path = NULL;
    char dev_dir[] = "/dev/";  /* Directory where device files reside.*/
    char *source_desc = NULL,
         *target_desc = NULL; /* Disk descriptions to display.  */
    struct lvm_rec* lvm_rec_ptr = NULL;
    struct ipl_rec_area* boot_rec_ptr = NULL;
    int cont = 1;  /*  "Boolean" */
    int lvm_disk = 0; /* 1 => source disk is in LVM. */
    struct read_capacity_data_t capacity;
    void *bb_buffer = NULL; /* pointer to bad block buffer */
    struct devinfo di;
    char *name;
   
    /* Pointers to structures within bb_buffer */
    struct bb_hdr *hdr_ptr = NULL;
    struct bb_entry *bblist_ptr = NULL;

    static_hd_list = get_devices(
   "chgstatus != 3 and PdDvLn != disk/dar/array and PdDvLn LIKE disk/*",
    &hd_list_info);
    num_disks = hd_list_info.num;
    if (num_disks) {
        disk_size = calloc(sizeof(int),num_disks);
        hd_list = calloc(sizeof(struct CuDv), num_disks);
        if (disk_size == NULL || hd_list == NULL) genexit(-1);
        for (i=0; i<num_disks; ++i) hd_list[i] = static_hd_list[i];
    }

    /* Open each disk in "shared" mode and read their capacities.*/
    /* If this open fails for any given disk, there is no point  */
    /* in allowing it to be selected because we won't be able to */
    /* reopen it later with exclusive access.                    */
    /* Also remove from the list any disks whose type/subtype    */
    /* indicate something other than a physical SCSI disk drive, */
    /* or whose block size is not what is expected (512).        */
 
    for (i=0; i<num_disks; ) {
        temp = -1;
        name = hd_list[i].name;
        temp_path = disk_path(dev_dir,name);
        if ((iss = configure_device(name)) != -1 &&
            (temp = open(temp_path, O_RDONLY, 0)) != -1     &&
            ioctl(temp,IOCINFO,&di) != -1                   &&
            di.devtype == 'r'                               && 
            di.devsubtype == 'p'                            &&
            di.un.scdk.blksize == BLOCK_SIZE) {
               disk_size[i++] = di.un.scdk.numblks;
        }
        else {
            /* Either the configure, open, or ioctl failed,      */
            /* or one of the devinfo parameters was unexpected.     */
            /* Remove the disk from the list of disks.              */
            memmove(&hd_list[i], &hd_list[i+1], 
                                   (--num_disks-i)*sizeof(struct CuDv));
	    #ifdef DBG
            fprintf(stderr,"Disk rejected, ");
            fprintf(stderr,"errno = %d, type = %c, subtype = %c, blksize = %d",
                    errno, di.devtype, di.devsubtype, di.un.scdk.blksize);
            fprintf(stderr," (see devinfo.h).\n");
	    #endif
        }     
        FREE(temp_path);
        if (temp >= 0) close(temp); /* Close only if opened! */
        if (iss >= 0) initial_state(iss,name);
    } /* end for */
    iss = -1;

/* Display info panel. */
    if ((rc = disp_err(DISK_COPY_INFO,
                       TITLES, DISK_TO_DISK_COPY_TITLE,
                       DISK_TO_DISK_SET, DISK_TO_DISK_INFO,
                       NULL, NULL, NULL ,0)) != ASL_ENTER)  
	cont = 0; 
    else {
        if (num_disks < 2) {
	    /* Display "less than 2 disks" message and return */
            /* The disp_err function is pretty handy for displaying any */
            /* panel with three lines that always end with "press enter,*/
            /* so use it. */
            cont = 0;
            rc = disp_err(DISK_COPY_LESS_THAN_TWO_DISKS,
                               TITLES, DISK_TO_DISK_COPY_TITLE,
                               GENERAL, LESS_THAN_TWO_DISKS,
                               NULL, NULL, NULL ,0); 
        }
        else {
	    /* Display the "from" and "to" selection menus. */
            rc = select_device(DISK_COPY_SOURCE,
			      TITLES, DISK_TO_DISK_COPY_TITLE,
			      INSTRUCTIONS, SOURCE_DISK_INSTRUCTION,
			      hd_list,
			      num_disks,
                              &temp);
            if (cont = (rc == ASL_COMMIT)) {
                from_disk = hd_list[temp];
/*
 * Set last_block to the last logical block address of the source disk.
 * Blocks are numbered 0 through (size - 1).  It is important to use
 * the size of the source disk since we are allowing the target disk
 * to be larger than the source disk and we don't to try to copy more
 * blocks than the source disk has.
 */
                last_block = disk_size[temp] - 1; 
/*
  Remove the selected source disk from the list of disks and sizes.
*/
                memmove(&hd_list[temp], &hd_list[temp+1],
                        (num_disks-temp-1)*sizeof(struct CuDv));
                memmove(&disk_size[temp], &disk_size[temp+1],
                        (num_disks-temp-1)*sizeof(int));
                --num_disks;
/* 
  Compute the maximum disk size we'll accept.  Last_block already  
  holds the minimum-1.
*/
                temp = ((float)last_block) * 1.1;
/*
  Now remove all disks from the list whose sizes are not in the      
  acceptable range.
*/
                for (i=0; i<num_disks; ) {
                    if (disk_size[i] <= last_block ||
                        disk_size[i] > temp) {
                        memmove(&hd_list[i], &hd_list[i+1],
                                (num_disks-i-1)*sizeof(struct CuDv));
                        memmove(&disk_size[i], &disk_size[i+1],
                                (num_disks-i-1)*sizeof(int));
                        --num_disks;
                    }
                    else ++i;
                } /* end for */

                /* Build the source disk description and path strings */

                source_desc = dev_desc(&from_disk);
                source_path = disk_path(dev_dir,from_disk.name);
/*
  if num_disks = 0, there are no other disks the same size as
  the from disk, so display the error panel.
*/
                if (!num_disks) { 
                    cont = 0; 
                    rc = disp_err(DISK_COPY_NOT_SAME_SIZE, 
                                  TITLES, DISK_TO_DISK_COPY_TITLE,
                                  DISK_TO_DISK_SET,NOT_SAME_SIZE,
                                  source_desc, NULL, NULL, 1);
                }
                else {
                    rc = select_device(DISK_COPY_TARGET,
			         TITLES, DISK_TO_DISK_COPY_TITLE,
			         INSTRUCTIONS, TARGET_DISK_INSTRUCTION,
			         hd_list,
			         num_disks, &temp);
                    if (cont = (rc == ASL_COMMIT)) {
                        to_disk = hd_list[temp];
                        last_target_block = disk_size[temp] - 1;
                        target_desc = dev_desc(&to_disk);
                        target_path = disk_path(dev_dir,to_disk.name);
                    } /* end if commit from "to disk" menu. */
                } /* end if no other disks same size as source. */
            } /* end if commit from "from disk" menu. */
        } /* end if no disks found. */
    } /* end if commit from info menu. */

    FREE(hd_list);  
    FREE(disk_size);
    if (!cont) {
        /* Free any memory that has been allocated and return the rc. */
        /* The FREE macro checks for a NULL value before calling free.*/
        FREE(target_desc);
        FREE(target_path);
        FREE(source_desc);
        FREE(source_path);
        if (static_hd_list) odm_free_list(static_hd_list, &hd_list_info);
        return rc;
    }

    /* If we get to this point without returning, both disks have been*/
    /* selected. Now we need to open them and check if they are*/
    /* defined to LVM.   */

    if ((its=configure_device(to_disk.name)) == -1 ||
       (t_fd = openx(target_path, O_RDWR, 0, SC_DIAGNOSTIC)) == -1) {
        /* Target disk should NOT be in use, etc. */
        cont = 0;
        rc = disp_err(DISK_COPY_TARGET_WONT_OPEN, 
                      TITLES, DISK_TO_DISK_COPY_TITLE,
                      GENERAL, TARGET_DISK_WONT_OPEN,
                      target_desc, NULL, NULL, 1);
    }
    else { /* Target is open.  Check for LVM record. */
        if ((lvm_rec_ptr = get_lvm_rec(t_fd, scsi)) != NULL) {
            if (lvm_rec_ptr -> lvm_id == LVM_LVMID) {
                /* Display message about the 
                   target disk containing LVM rec. */
                struct msglist errlist[] = { 
                           {TITLES, DISK_TO_DISK_COPY_TITLE},
                           {SCROLLABLES, DISREGARD},
			   {SCROLLABLES, TERMINATE},
			   {INSTRUCTIONS, TARGET_DISK_IN_LVM},
                           NULL
                };
                ASL_SCR_INFO errinfo[DIAG_NUM_ENTRIES(errlist)];
                ASL_SCR_TYPE errtype = DM_TYPE_DEFAULTS;
                char *temp;
                diag_display(DISK_COPY_TARGET_IN_LVM, 
                        cat_fd, errlist, DIAG_MSGONLY, 
                        ASL_DIAG_LIST_CANCEL_EXIT_SC, 
                        &errtype, errinfo);
                temp = malloc(strlen(errinfo[3].text) +
                              strlen(target_desc) + 1);
                if (temp == NULL) genexit(-1);
                sprintf(temp,errinfo[3].text,target_desc);
                errinfo[3].text = temp;
                errtype.cur_index = 2;
                switch (rc = diag_display(DISK_COPY_TARGET_IN_LVM, 
                        cat_fd, NULL, DIAG_IO, 
                        ASL_DIAG_LIST_CANCEL_EXIT_SC, 
                        &errtype, errinfo)) {
                    case DIAG_ASL_EXIT:
                    case DIAG_ASL_CANCEL:
                        cont = 0;
                        break;
                    case DIAG_ASL_COMMIT:
                        cont = errtype.cur_index != 2;/* 2 means quit.*/
                        break;
                    default:
			#ifdef DBG
                        fprintf(stderr,
                       "Unexpected return code from diag_display %d.\n",
                               rc);
			       #endif
                        genexit(-1);
                 } /* end switch */
                 FREE(temp);
            } /* end if valid lvm rec */
            FREE(lvm_rec_ptr);    
        } /* end if lvm rec successfully read. */
    } /* end if target opens. */

    if (cont) { /* target is ok, open source. */
        if ((iss=configure_device(from_disk.name)) == -1 ||
           (s_fd=openx(source_path, O_RDONLY, 0, SC_DIAGNOSTIC))==-1){
            cont = 0;
#ifdef DBG
fprintf(stderr, "Failure to open source disk, errno = %d, iss = %d.\n",
                errno, iss);
#endif
            if (errno == EACCES) 
                rc = disp_err(DISK_COPY_SOURCE_WONT_OPEN_EACCESS,
                              TITLES, DISK_TO_DISK_COPY_TITLE,
                              GENERAL, SOURCE_DISK_WONT_OPEN_EACCESS,
                              source_desc, NULL, NULL, 1);
            else 
                rc = disp_err(DISK_COPY_SOURCE_WONT_OPEN_OTHER,
                              TITLES, DISK_TO_DISK_COPY_TITLE,
                              GENERAL, SOURCE_DISK_WONT_OPEN_OTHER,
                              source_desc, NULL, NULL, 1);
        }
    }

/*
 * If no errors detected so far, 
 * read the LVM record to determine if the source disk is an LVM disk.
 * If the source disk is in LVM, read 
 * the LVM bad block directory.  
 */
    
    if (cont) {
        lvm_rec_ptr = get_lvm_rec(s_fd,scsi);
        if (!lvm_rec_ptr) {
	    #ifdef DBG
            fprintf(stderr,
                       "Unable to read LVM record from source disk.\n");
		       #endif
            cont = 0;
            rc = disp_err(DISK_COPY_BBDIR,
                          TITLES, DISK_TO_DISK_COPY_TITLE,
                          GENERAL, CANT_READ_BBDIR,
                          source_desc, NULL, NULL, 1);
        }
        else lvm_disk = (lvm_rec_ptr->lvm_id == LVM_LVMID);
    }

    if (cont && lvm_disk) {
        /* We have read a valid LVM record, which means there is a */
        /* Bad Block directory to be read (though it may be empty). */
        /* Read the Bad Block directory and set bb_buffer  */
        /* to point to it. */ 
        bb_buffer = rdbbdir(s_fd);
        /* If we could not read the bb dir, give up. */
        if (bb_buffer) {
            /* Set up the pointers to the bad block header and */
            /* list of bad block entries.                      */
            hdr_ptr = bb_buffer;
            /* The list of bad block entries comes right after the */
            /* header.                                             */
            bblist_ptr = (struct bb_entry*) (hdr_ptr + 1);
        }
        else {
	    #ifdef DBG
            fprintf(stderr,
              "Unable to read bad block directory from source disk.\n");
	      #endif
            cont = 0;
            rc = disp_err(DISK_COPY_BBDIR,
                          TITLES, DISK_TO_DISK_COPY_TITLE,
                          GENERAL, CANT_READ_BBDIR,
                          source_desc, NULL, NULL, 1);
        }
    }
 
    if (cont) {
        /* Both disks are open. Get user confirmation.       */
        struct msglist sellist[] = { 
                   {TITLES, DISK_TO_DISK_COPY_TITLE},
                   {SCROLLABLES, DONT_COPY},
                   {SCROLLABLES, DO_COPY},
                   {INSTRUCTIONS, DISK_TO_DISK_CONF},
                           NULL
        };
        ASL_SCR_INFO selinfo[DIAG_NUM_ENTRIES(sellist)];
        ASL_SCR_TYPE seltype = DM_TYPE_DEFAULTS;
        char *temp;
        diag_display(DISK_COPY_CONF, 
                cat_fd, sellist, DIAG_MSGONLY, 
                ASL_DIAG_LIST_CANCEL_EXIT_SC, 
                &seltype, selinfo);
        /* Allow 20 characters for the number of sectors on both. */
        temp = malloc(strlen(selinfo[3].text) + 
                      strlen(source_desc) + strlen(target_desc) + 20);
        if (temp == NULL) genexit(-1);
        sprintf(temp,selinfo[3].text,source_desc,target_desc,
                last_block, last_target_block);
        selinfo[3].text = temp;
        seltype.cur_index = 1;
        switch (rc = diag_display(DISK_COPY_CONF, 
                cat_fd, NULL, DIAG_IO, 
                ASL_DIAG_LIST_CANCEL_EXIT_SC, 
                &seltype, selinfo)) {
            case DIAG_ASL_EXIT:
            case DIAG_ASL_CANCEL:
                cont = 0;
                break;
            case DIAG_ASL_COMMIT:
                cont = seltype.cur_index != 1;
                break;
            default:
		#ifdef DBG
                fprintf(stderr,
               "Unexpected return code from diag_display %d.\n",
                       rc);
		       #endif
                genexit(-1);
         } /* end switch */
         FREE(temp);
    }

    if (cont) {
        /* User wants to continue with copy process.        */
        /* When copying the blocks, read and write many    */
        /* blocks per SCSI operation (for performance).     */
        /* SC_MAXREQUEST is the smallest maximum request size that */
        /* any SCSI device is required to support.                 */
        char io_buffer[SC_MAXREQUEST]; 
        int max_blocks = SC_MAXREQUEST / BLOCK_SIZE; 

        /* The source_block and target_block are usually equal,    */
        /* except when copying a relocated (i.e. bad) block.       */
        /* Num_blocks is usually max_blocks, except when copying a */
        /* relocated block or if at the end of the disk and the    */
        /* total number of blocks is not a multiple of max_blocks. */
        int source_block, target_block, num_blocks;
        int next_bad_block ; 
        struct bb_entry *cur_entry,  /* Current bad block entry. */
                        *last_entry, /* Last in bad block list.  */
                        *temp_entry;   

        /* After each block group is copied, determine the  */
        /* current percentage complete and re-display the   */
        /* progress panel if the percentage has increased.  */
        int cur_percent, prev_percent;

        int temp, write_errors = 0;
        struct sense_data_t sense_data;
        char *progress_str = diag_cat_gets(cat_fd,GENERAL,PROGRESS);
        ASL_SCR_INFO info[2];     /* For the progress reports. */
        ASL_SCR_TYPE type = DM_TYPE_DEFAULTS;

        /* Set up the static part of the progress panel. */
        info[0].text = diag_cat_gets(cat_fd,
                      TITLES,DISK_TO_DISK_COPY_TITLE);
        /* In addition to the source and target disks, the */
        /* progress screen displays the %complete.         */
        info[1].text = malloc(strlen(progress_str) +
                                 strlen(target_desc) +
                                 strlen(source_desc) + 4 );
        if (info[1].text == NULL) genexit(-1);
        type.max_index = 1;

        /* If there are no bad blocks, set next_bad_block so high that*/
        /* target_block    */
        /* will never reach it.  Otherwise set it to the    */
        /* first bad block in the list.                               */
        if (hdr_ptr) {
            cur_entry = bblist_ptr;
            last_entry = bblist_ptr + (hdr_ptr->num_entries - 1);
            next_bad_block = hdr_ptr->num_entries ? 
                                cur_entry->bb_lsn  :
                                last_block + 10;  
        }
        else next_bad_block =  last_block + 10;  

        prev_percent = -1; /* Causes the progress screen to be */
        cur_percent = 0;   /* displayed immediately with 0%.   */ 
        target_block = 0; /* Start with 0th block. */
        /* Begin copying.  This loop will run about 10,000 times  */
        /* for a 320 MB drive!                                   */
        while (cont && target_block <= last_block) {
            cur_percent = ((float)target_block/(float)last_block)*100.0;
            if (cur_percent > prev_percent) {
                /* Display the report screen with the new percentage. */
                prev_percent = cur_percent;
                sprintf(info[1].text,progress_str,
                                     source_desc,target_desc,
                                     cur_percent);
                rc = diag_display(DISK_COPY_PROGRESS,
                            cat_fd, NULL, DIAG_IO, 
                            ASL_DIAG_LEAVE_NO_KEYS_SC, &type, info);
            } /* end if time to report progress */

            if (target_block == next_bad_block) {
                /* This the case when source_block and target_block  */
                /* differ. */
                /* Handle the bad block as an individual block.      */
                num_blocks = 1;
                /* The bad block may be relocated to a block which   */
                /* which has also gone bad and been relocated and    */
                /* so on.  End_of_chain finds the end of the chain.  */
                temp_entry = end_of_chain(bblist_ptr,
                                          hdr_ptr->num_entries,
                                          cur_entry);
 
                /* It's possible that the bad block has not been     */
                /* relocated, in which case we'll try to read it     */
                /* from its last location.                           */
                source_block =
                       temp_entry->rel_lsn ? temp_entry->rel_lsn :
                                             temp_entry->bb_lsn;
 
                /* If this is the last bad block, set next_bad_block */
                /* so high that last_copied_block will never reach it.*/
                if (cur_entry == last_entry)
                    next_bad_block = last_block + 10;
                else {
                    ++cur_entry;
                    next_bad_block = cur_entry->bb_lsn;
                } /* end if last bb entry. */
            }
            else { 
                source_block = target_block;
                /* Set num_blocks to read as many blocks as possible */
                /* without exceeding the max transfer size, reading  */
                /* past the last block on the drive, or reading the  */
                /* next bad block.                                   */
                num_blocks = LESSER(max_blocks, 
                                 last_block-source_block+1);
                num_blocks = LESSER(num_blocks, 
                                 next_bad_block-source_block);
            } /* end if bad block encountered. */

            /* Read num_blocks from the source disk.    */
            if (get_block(s_fd,scsi,source_block,num_blocks,
                          io_buffer,&sense_data)) {
                /* Look in the sense_data to see what the problem is.*/
                /* Ignore recovered errors, abort otherwise.         */
                if (sense_data.sense_key != 1) {/* Recovered error. */
                    char temp[15];
                    cont = 0;
                    sprintf(temp,"%d",sense_data.information);
                    rc = disp_err(DISK_COPY_READ_ERROR,
                                  TITLES, DISK_TO_DISK_COPY_TITLE,
                                  GENERAL, UNHANDLED_DISK_READ_ERROR,
                                  temp, source_desc, NULL, 2);
                } /* end if recovered error */
            } /* end if read error */

            if (cont) {
/*
 * If put_block fails and the sense data indicates an error other than
 * a recovered error (sense_key = 0x1), increment write_errors and
 * terminate the copy process if the write_errors exceeds the max or
 * if the error is something other than a medium error or if the 
 * the logical block number to be relocated is not present in the 
 * sense data.
 */
                if (put_block(t_fd,target_block,num_blocks,io_buffer,
                              &sense_data) &&
                    (sense_data.sense_key != 0x1 ||
                     sense_data.sense_key == 0x1 &&
                     sense_data.sense_code == 0x5D00)) {
                    if (++write_errors > max_write_errors || 
                        sense_data.sense_key != 0x3 ||
                        !sense_data.valid) {
                        cont = 0;
                        rc = disp_err(DISK_COPY_WRITE_ERROR,
                                      TITLES, DISK_TO_DISK_COPY_TITLE,
                                      GENERAL, DISK_WRITE_ERROR,
                                      target_desc, NULL, NULL, 1);
                    }
                    else {
/* 
 * Relocate the bad block and try to re-write the group of blocks
 * we just got the error on.  Rewriting the entire group is simpler
 * than just rewriting the single block which had the error.  
 * The rewrite is accomplished by simply not incrementing the 
 * variable target_block, so the while loop just rewrites the 
 * current group of blocks again.
 * It is not necessary to check the return code from relocating the
 * block since if that fails, the subsequent rewrite will also fail
 * again (and again), causing the copy procedure to terminate.
 */
                        reassign_block(t_fd, sense_data.information);
                    }
                }
                else target_block += num_blocks;
            }
        } /* end while */
        FREE(info[1].text);

        /* That takes care of the copy.  Cont is still true unless   */
        /* an i/o error occurred.                                    */
        /* Since all LVM bad blocks have been recovered, update the  */
        /* primary and backup bad block headers on the target disk   */
        /* to indicate that there are no bad blocks.                 */
       
        if (cont && lvm_disk) {
            struct bb_hdr *hdr_ptr = (struct bb_hdr*) io_buffer;
            hdr_ptr->num_entries = 0;
            strcpy(hdr_ptr->id,BB_DIR_ID);
            if (put_block(t_fd, PSN_BB_DIR, 1, io_buffer, NULL) ||
                put_block(t_fd, PSN_BB_BAK, 1, io_buffer, NULL)) {
                cont = 0;
                rc = disp_err(DISK_COPY_WRITE_ERROR,
                              TITLES, DISK_TO_DISK_COPY_TITLE,
                              GENERAL, DISK_WRITE_ERROR,
                              target_desc, NULL, NULL, 1);
            } /* end if either put fails. */        
        } /* end if cont */
   
        if (cont) {
            /* Display the completion panel. */
            rc = disp_err(DISK_COPY_COMPLETE,
                          TITLES, DISK_TO_DISK_COPY_TITLE,
                          DISK_TO_DISK_SET, DISK_TO_DISK_COMPLETION,
                          source_desc, target_desc, NULL, 2);
        } /* end if cont */
    } /* end if cont */

    /* We are almost ready to return to the caller.  Free any        */
    /* pointers required to prevent memory leakage, then return.     */
    /* Also close any open disk filedescriptors.                     */
    /* Don't close a disk if it was never opened.                */

    if (s_fd >= 0) close(s_fd);
    if (t_fd >= 0) close(t_fd);
    if (iss >=0) initial_state(iss, from_disk.name);
    if (its >=0) initial_state(its, to_disk.name);
    FREE(target_desc);
    FREE(target_path);
    FREE(source_desc);
    FREE(source_path);
    FREE(lvm_rec_ptr);
    FREE(boot_rec_ptr);
    FREE(bb_buffer);
    if (static_hd_list) odm_free_list(static_hd_list, &hd_list_info);

    return rc;
} /* end diskcopy */
