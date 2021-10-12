static char sccsid[] = "@(#)37  1.3  src/bos/diag/da/d7135/d7135a.c, da7135, bos41J, 9515A_all 4/7/95 09:12:05";
/*
 *   COMPONENT_NAME: DA7135
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "d7135m.h"

extern int dev_lun;

/*
 *
 * NAME: raid_array_status()
 *
 * FUNCTION: Build a linked list of the dac's, LUNS's and physical
 *           disks in the array. The linked list is refered to as
 *           "array status".
 *
 * NOTES:
 *      When called with device equal to
 *    CNTRL/ALT_CNTRL: Assumes valid MODE SENSE data (page 2C)
 *                     in in tucb.scsitu.data_buffer.
 *              HDISK: Assumes valid MODE SENSE data (page 2B)
 *                     in in tucb.scsitu.data_buffer.
 *              SPARE: Assumes valid MODE SENSE data (page 2A)
 *                     in in tucb.scsitu.data_buffer. Adds status
 *                     of all disks in the array.
 *
 * RETURNS: DM_STATUS_GOOD = Added device to array status.
 *          DM_STATUS_BAD  = Invalid device type.
 *
 */

int raid_array_status(int device) {
        int rc;
        int i, j, offset;
        unsigned int disk_bits;
        unsigned int disk_mask;
        struct lun *new_lun;
        struct pdisk *disk_ptr;
        struct pdisk *new_disk;
        struct CuAt cuat;
        struct CuDv *tmp_cudv;
        struct listinfo tmp_linfo;
        float capacity;
        long block_size;
        long num_blocks;
        float block_per_mb;
        char cuat_value[NAMESIZE];
        char *ptr;

        dt(25,"In raid_array_status()");
        disk_ptr = (struct pdisk *)NULL;
        cuat_value[0] = '\0';

        /* Create a LUN and fill in the array status. */
        new_lun = (struct lun *)calloc(1, sizeof(struct lun));

        switch (device & DEVICE_TYPE_MASK) {
        case CNTRL: /* MODE SENSE page 2C (Redundant Controller Page) */
                /* Add the controller selected to the array status.   */
                /* Use the LUN struct to fill in the information.     */
                strcpy(new_lun->name, tm_input.dname);
                strcpy(new_lun->location, tm_input.dnameloc);
                strcpy(new_lun->raid_level, " ");
		if (router.prev_state == -1) {
                	new_lun->status_byte = ACTIVE_CNTRL;
		} else if (DAC_ID_ODD(dac_scsi_id)) {
                	dt(20,1,"dac ID odd, name",new_lun->name);
                	new_lun->status_byte = (router.ioctl.state & 0x0F);
		} else {
                	new_lun->status_byte = (router.ioctl.state >> 4);
		}
                new_lun->next = (struct lun *)NULL;
                /* Add the cntrl to the top (head) of the array status. */
                raid_add_to_status(BEGINNING_OF_LIST, new_lun);

                /* Get the list of controllers for this array from CuAt. */
                sprintf(attr, "name = %s and attribute = all_controller",
                        arr.dar_name);

                dt(20,1,"attr",attr);
                if (odm_get_first(CuAt_CLASS, attr, &cuat)) {
                        strcpy(cuat_value, cuat.value);
                        dt(20,1,"CuAt value", cuat_value);
                } else {
                        rc = DM_STATUS_GOOD;
                        dt(25,"No dac's in CuAt");
                        break;
                }
                /* Get the name and location for each dac from CuDv. */
                for (i = 1;
                     ((ptr = str_parse(cuat_value,",",i)) != (char *)NULL);
                     i++) {
                        dt(20,1,"ptr",ptr);
                        /* Get the name of the alt controller. */
                        sprintf(attr, "name = %s and chgstatus != 3", ptr);
                        dt(20,1,"attr",attr);
                        tmp_cudv = get_CuDv_list(CuDv_CLASS, attr,
                                                 &tmp_linfo, 1, 2);
                        if ((tmp_cudv == (struct CuDv *)(-1)) ||
                            (tmp_cudv == (struct CuDv *)NULL)) {
                                /* Error getting device info. */
                                dt(20,1,"ERROR: CuDv name", ptr);
                                rc = DM_STATUS_GOOD;
                                break;
                        }
                        /* Dont add this controller again. */
                        if (strcmp(tm_input.dname, ptr)) {
                                /* Create a LUN, fill in the cntrl status. */
                                new_lun = (struct lun *)calloc(1,
                                                           sizeof(struct lun));
                                strcpy(new_lun->name, tmp_cudv->name);
                                strcpy(new_lun->location, tmp_cudv->location);
                                strcpy(new_lun->raid_level, " ");
				if (router.prev_state == -1) {
                                	new_lun->status_byte = ALT_CNTRL;
				} else if (DAC_ID_ODD(dac_scsi_id)) {
					/* The dac_scsi_id is for the other   */
					/* dac! If it's odd, then this dac    */
					/* is even. So use the state variable */
					/* for an even dac ID!                */
                			new_lun->status_byte = 
						(router.ioctl.state >> 4);
				} else {
                			dt(20,1,"dac ID odd, name",
					   new_lun->name);
                			new_lun->status_byte = 
						(router.ioctl.state & 0x0F);
				}
                                new_lun->next = (struct lun *)NULL;
                                /* Add the alt cntrl to the array status. */
                                raid_add_to_status(END_OF_LIST, new_lun);
                        }
                } /* end while */
                rc = DM_STATUS_GOOD;
                break;
        case LUN: /* MODE SENSE page 2B (Logical Array Page) */
                /* Get the heads/cylinders form the arr struct. */
                tmp_cudv = (arr.cudv + arr.lun_counter);
                strcpy(new_lun->name, tmp_cudv->name);
                strcpy(new_lun->location, tmp_cudv->location);
                if (device & DEAD_LUN) {
                        new_lun->status_byte = DEAD_LUN;
                        strcpy(new_lun->capacity, " ");
                        strcpy(new_lun->raid_level, " ");
                        new_lun->pdisk_count = 0;
                        new_lun->status_byte = 0xFF;
                        new_lun->next = (struct lun *)NULL;
                        raid_add_to_status(END_OF_LIST, new_lun);
                        rc = display_menugoal("9045");
                        return(DM_STATUS_GOOD);
                } else {
                        new_lun->status_byte = tucb.scsitu.data_buffer[14];
                }
                sprintf(new_lun->raid_level, "%s %d",
                        (char *)diag_cat_gets(catd, D7135_MSG_SET, RAID),
                        tucb.scsitu.data_buffer[15]);
                /* LUN Capacity */
                block_size =
                    (unsigned)((tucb.scsitu.data_buffer[17] << 24) |
                               (tucb.scsitu.data_buffer[18] << 16) |
                               (tucb.scsitu.data_buffer[19] << 8) |
                               (tucb.scsitu.data_buffer[20]));
                dt(10,1,"block size",block_size);
                num_blocks =
                    (unsigned)((tucb.scsitu.data_buffer[23] << 24) |
                               (tucb.scsitu.data_buffer[24] << 16) |
                               (tucb.scsitu.data_buffer[25] << 8) |
                               (tucb.scsitu.data_buffer[26]));
                dt(10,1,"number of blocks", num_blocks);
                /* Calculate capacity in MB's. */
                block_per_mb = (float)(0x100000 / block_size);
                capacity = (float)(num_blocks / block_per_mb);
                if (capacity >= 1000) {
                        sprintf(new_lun->capacity, "%3.2f%s",
                                (capacity / 1000),
                                (char *)diag_cat_gets(catd,
                                                      D7135_MSG_SET, GB));
                } else {
                        sprintf(new_lun->capacity, "%3.2f%s",
                                capacity,
                                (char *)diag_cat_gets(catd,
                                                      D7135_MSG_SET, MB));
                }
                /* The phsical disks need to be added to new_lun */
                /* so set rc to indicate more work to do in this */
                /* function. The status for each physical disk   */
                /* will be determined when called to add SPARE   */
                /* disks with the physical mode sense (2A) data. */
                rc = DM_MORE_CONT;
                break;
        case SPARE: /* MODE SENSE page 2A (Array Physical Page)  */
                /* Add the status for all the LUN physical disks */
                /* that are already in the array status.         */
                if (raid_hptr != (struct lun *)NULL) {
                        raid_disk_status();
                }
                /* Now add spare disks to the end of the array status. */
                raid_add_spares();
                rc = DM_STATUS_GOOD;
                break;
        default:
                dt(25,"ERROR: Invalid device type");
                rc = DM_STATUS_BAD;
                break;
        }

        if (rc != DM_MORE_CONT) {
                return(rc);
        }

        /* Use the Disk Bit Map of the MODE SENSE (2B) data to determine */
        /* what physical disks are associated with this HDISK.           */
        new_lun->pdisk_count = 0;
        for (i = 0, offset = 48; i < 16; offset += 2, i++) {
                disk_bits = (unsigned int)
                            (tucb.scsitu.data_buffer[offset] << 8) |
                             tucb.scsitu.data_buffer[offset+1];
                /* Mask each bit of the disk_bits. If a bit is set, */
                /* create a disk and fill in the channel and id.    */
                /* The disk name and disk status will be determined */
                /* after all disks have been identified.            */
                for (j = 0, disk_mask = 1; j < 16; disk_mask *= 2, j++) {
                        if (disk_bits & disk_mask) {
                                /* A disk exists, add it to the list. */
                                new_disk = (struct pdisk *)calloc(1,
                                            sizeof(struct pdisk));
                                new_lun->pdisk_count += 1;
                                new_disk->ch = j + 1;
                                new_disk->id = i;
				new_disk->lun_id = dev_lun;
				dt(16,1,"Pdisk LUN",new_disk->lun_id);
                                if (((j + 1) > MAX_SUPPORTED_CHs) ||
                                    (i > MAX_SUPPORTED_IDs)) {
                                        dt(10,1,"ERROR: Invalid disk, ch",
                                           (j+1),"id",i);
					clean_up(DM_ERROR_OTHER);
                                }
				if (!lun_owns_pdisk[new_disk->ch][new_disk->id]) {
					/* New disk, not part of a sub-lun. */
					++lun_owns_pdisk[new_disk->ch][new_disk->id];
                        		++arr.num_pdisks;
				} else {
					/* New disk, part of a sub-lun. */
					strcpy(new_disk->inq_pid,
						get_pdisk_pid(arr.dar_name,
				 		   	      new_disk->ch,
						   	      new_disk->id));
                                        new_disk->ffc = get_pdisk_ffc(new_disk->inq_pid);
					strcpy(new_disk->capacity,
						get_pdisk_capacity(arr.dar_name,
				 		   		   new_disk->ch,
						   		   new_disk->id));
					new_disk->flags |= PDISK_DIAGNOSED;
				}
                                new_disk->flags |= PDISK_BELONG_TO_LUN;
                                new_disk->next = (struct pdisk *)NULL;
                                if (new_lun->pdisk_count == 1) {
                                        disk_ptr = new_disk;
                                } else {
                                        new_disk->next = disk_ptr;
                                        disk_ptr = new_disk;
                                }
                        }
                }
        }
        dt(20,1,"Name",new_lun->name);
        dt(10,1,"Pdisk count",new_lun->pdisk_count);

        if (new_lun->pdisk_count) {
                new_lun->pdisk = disk_ptr;
        }
        new_lun->next = (struct lun *)NULL;

        raid_add_to_status(END_OF_LIST, new_lun);


        return(DM_STATUS_GOOD);

} /* end raid_array_status() */

/*
 * NAME: raid_add_to_status()
 *
 * FUNCTION: Add an entry in the array status. The position parameter
 *           decides if the entry is added to the beginning or the end
 *           of the array status linked list.
 *
 * NOTES:
 *
 * RETURNS: void
 *
 */

void raid_add_to_status(int position, struct lun *new_lun) {
        struct lun *lun_ptr;

        dt(25,"In raid_add_to_status()");
        if (position == END_OF_LIST) {
                /* Add the hdisk to the end of the array status. */
                if (raid_hptr == (struct lun *)NULL) {
                        raid_hptr = new_lun;
                } else {
                        lun_ptr = raid_hptr;
                        while(lun_ptr->next != (struct lun *)NULL) {
                                lun_ptr = lun_ptr->next;
                        }
                        lun_ptr->next = new_lun;
                }
        } else {
                /* Add to the beginning of the list. */
                new_lun->next = raid_hptr;
                raid_hptr = new_lun;
        }

        return;

} /* end raid_add_to_status() */

/*
 *
 * NAME: raid_disk_status()
 *
 * FUNCTION:
 *
 * NOTES:
 *
 * RETURNS: SCATU_GOOD =
 *
 */

void raid_disk_status(void) {
	int i;
        struct lun *lun_ptr;
        struct pdisk *disk_ptr;

        dt(25,"In raid_disk_status()");
        lun_ptr = raid_hptr;
        while (lun_ptr != (struct lun *)NULL) {
                disk_ptr = lun_ptr->pdisk;
                /* dt(20,1,"Disk list, name", lun_ptr->name); */
                while (disk_ptr != (struct pdisk *)NULL) {
                        disk_ptr->status_byte = PDISK_STATUS(disk_ptr->ch,
                                                             disk_ptr->id);
                        /* Search the physical status bytes array */
                        /* of structs for the disk status byte.  */
                        for (i = 0;
                             disk_ptr->status_byte !=
                                              sbytes_2A[i].status_byte; i++) {
                                if (sbytes_2A[i].status_byte == 0xFF) {
                                        /* UNKNOWN is the end, so break. */
                                        break;
                                }
                        }
                        disk_ptr->srn = sbytes_2A[i].srn;
                        disk_ptr->status_msg = sbytes_2A[i].status_msg;
                        disk_ptr = disk_ptr->next;
                }
                lun_ptr = lun_ptr->next;
        }

        return;

} /* end raid_disk_status() */

/*
 *
 * NAME: raid_add_spares()
 *
 * FUNCTION: Add all spare disks (failed, hot spares, etc) to the
 *           array status.
 *
 * NOTES: See comments.
 *
 * RETURNS: void
 *
 */

void raid_add_spares(void) {
        int i, j;
        int ch = 0, id = 0;
        int status_byte;
        struct lun *new_lun;
        struct pdisk *new_disk;
        struct pdisk *disk_ptr;
        char *data_buffer;

        dt(25,"In raid_add_spares()");
        data_buffer = (char *)calloc(1, tucb.scsitu.data_length);
        memcpy(data_buffer, tucb.scsitu.data_buffer, tucb.scsitu.data_length);

        /* Add the spare disks to the array status */
        /* by creating a lun struct and the disk   */
        /* pointer will contain only spare disks.  */
        /* The lun struct will have the name SPARE */
        /* which will indicate a list of spares.   */
        /* This LUN will not be displayed, but the */
        /* physical disks associated with it will. */

        /* Create the LUN for spare disks. */
        new_lun = (struct lun *)calloc(1, sizeof(struct lun));
        strcpy(new_lun->name, " ");
        new_lun->next = (struct lun *)NULL;
        new_lun->pdisk = (struct pdisk *)NULL;
        strcpy(new_lun->raid_level, "-1");
        disk_ptr = (struct pdisk *)NULL;

        /* Walk through the Array Physical page Mode Sense data.  */
        /* Check for spare and hot spare disks.                   */
        /* The disk status data starts at byte offset 14.         */
        for (i = 14; i < ((unsigned)data_buffer[13] + 12); i++) {
                status_byte = (unsigned)data_buffer[i];
                if (ch >= MAX_CHs) {
                        ch = 0;
                        ++id;
                }
                ++ch;
                /* Check for valid CH and ID. Get only spare/hot spare. */
                if (VALID_PDISK_CH(ch) && VALID_PDISK_ID(id) &&
                    ((status_byte & MAIN_STATUS_MASK) != 0x01) && 
		    (!lun_owns_pdisk[ch][id])) {
                        new_disk = (struct pdisk *)calloc(1,
                                    sizeof(struct pdisk));
                        ++arr.num_pdisks;
                        new_disk->ch = ch;
                        new_disk->id = id;
                        new_disk->lun_id = dev_lun;
			dt(16,1,"pdisk LUN",new_disk->lun_id);
                        new_disk->status_byte = status_byte;
                        if (disk_ptr == (struct pdisk *)NULL) {
                                disk_ptr = new_disk;
                        } else {
                                new_disk->next = disk_ptr;
                                disk_ptr = new_disk;
                        }
                        /* Search the physical status bytes array */
                        /* of structs for the disk status byte.  */
                        for (j = 0;
                             disk_ptr->status_byte !=
                                              sbytes_2A[j].status_byte; j++) {
                                if (sbytes_2A[j].status_byte == 0xFF) {
                                        /* UNKNOWN is the end, so break. */
                                        break;
                                }
                        }
                        disk_ptr->srn = sbytes_2A[j].srn;
                        disk_ptr->status_msg = sbytes_2A[j].status_msg;
                        new_lun->pdisk_count++;
                } /* end if */
        } /* end for */

        if (new_lun->pdisk_count) {
                new_lun->pdisk = disk_ptr;
                raid_add_to_status(END_OF_LIST, new_lun);
        }
        dt(10,1,"Number of pdisks",arr.num_pdisks);

        return;

} /* end raid_add_spares() */

/*
 *
 * NAME: raid_build_msg()
 *
 * FUNCTION:
 *
 * NOTES:
 *
 * RETURNS: void
 *
 */

void raid_build_msg(void) {
        int i, rc;
        struct lun *lun_ptr;
        struct pdisk *disk_ptr;
        char *cat_ptr;
        char buff[1024];
        char *msg_buff;
        char tmp_buff[128];

        dt(25,"In raid_build_msg()");
        strcpy(buff, (char *)diag_cat_gets(catd, D7135_MSG_SET,
                                           ARRAY_STATUS));
        raid_add_buff(buff);

        lun_ptr = raid_hptr;
        while (lun_ptr != (struct lun *)NULL) {
                cat_ptr = (char *)diag_cat_gets(catd, D7135_MSG_SET,
                                            ARRAY_STATUS_ITEM);
                /* ARRAY_STATUS_ITEM                                */
                /* "%1$.16s  %2$.16s  %3$.6s %4$.8s  %5$.18s"       */
                /*                                                  */
                /*    %1$.16s     Logical Name    (16 chars max)    */
                /*    %2$.16s    Location         (16 chars max)    */
                /*    %3$.6s      RAID level       (6 chars max)    */
                /*    %4$.8s      Capacity         (8 chars max)    */
                /*    %5$.18s    Status           (18 chars max)    */

                /* Check for a controller device. */
                if ((strlen(lun_ptr->location) > 6) &&
                    (strlen(lun_ptr->raid_level) < 2)) {
                        /* Add controller status info. */
                        switch(lun_ptr->status_byte) {
                        case ACTIVE_CNTRL:
                                lun_ptr->status_msg = CNTRL_STATUS_ACTIVE;
                                break;
                        case PASSIVE_CNTRL:
                                lun_ptr->status_msg = CNTRL_STATUS_PASSIVE;
                                break;
                        case HIR_CNTRL:
                                lun_ptr->status_msg = CNTRL_STATUS_HIR;
                                break;
                        case FAILED_CNTRL:
                                lun_ptr->status_msg = CNTRL_STATUS_FAILED;
                                sprintf(arr.fru_info, "%X:%X", dac_ffc, 0x302);
                                rc = raid_fru(SPECIFIC_FRU_TYPE);
                                break;
                        case ALT_CNTRL:
                                lun_ptr->status_msg = CNTRL_STATUS_ALT_PATH;
                                break;
                        default:
                                lun_ptr->status_msg = UNKNOWN;
                                break;
                        }
                        sprintf(buff, cat_ptr,
                                lun_ptr->name,
                                lun_ptr->location,
                                " ", /* RAID level */
                                " ", /* Capacity */
                                (char *)diag_cat_gets(catd,
                                                      D7135_MSG_SET,
                                              lun_ptr->status_msg));
                        /* Add buff to the array status. */
                        raid_add_buff(buff);
                }
                /* Check for a LUN device. */
                if ((strlen(lun_ptr->location) > 6) &&
                    (strlen(lun_ptr->raid_level) > 2)) {
                        /* Search the logical (2B) status byte array */
                        /* of structs until for this LUN's status.   */
                        for (i = 0;
                             lun_ptr->status_byte !=
                                      sbytes_2B[i].status_byte; i++) {
                                if (sbytes_2B[i].status_byte == 0xFF) {
                                        /* UNKNOWN is the end, so break. */
                                        break;
                                }
                        }
                        /* Fill in the message with the LUN info. */
                        sprintf(buff, cat_ptr,
                                lun_ptr->name,
                                lun_ptr->location,
                                lun_ptr->raid_level,
                                lun_ptr->capacity,
                                (char *)diag_cat_gets(catd, D7135_MSG_SET,
                                                  sbytes_2B[i].status_msg));
                        if (sbytes_2B[i].srn) {
                                /* Call out an SRN for the LUN. */
                                sprintf(arr.fru_info, "%X:%X:%s:%s",
                                        dac_ffc, sbytes_2B[i].srn,
                                        lun_ptr->name,
                                        lun_ptr->location);
                                rc = raid_fru(LUN_FRU_TYPE);
                        }
                        /* Add buff to the array status. */
                        raid_add_buff(buff);
                }
                /* Add the physical disks for this LUN. */
                if (lun_ptr->pdisk_count) {
                        disk_ptr = lun_ptr->pdisk;
                } else {
                        disk_ptr = (struct pdisk *)NULL;
                }
                while (disk_ptr != (struct pdisk *)NULL) {
                        cat_ptr = (char *)diag_cat_gets(catd, D7135_MSG_SET,
                                            ARRAY_STATUS_ITEM);
                        /* Fill in the message with the disk info. */
                        /* Check for the LUN with the spares. */
                        if (!(disk_ptr->flags & PDISK_BELONG_TO_LUN)) {
                                /* Spare disk LUN, get spare disk name. */
                                if (disk_ptr->status_msg == HOT_SPARE) {
                                        strcpy(tmp_buff,
                                             (char *)diag_cat_gets(catd,
                                           D7135_MSG_SET, HOT_SPARE_DISK_NAME));
                                } else {
                                        strcpy(tmp_buff,
                                             (char *)diag_cat_gets(catd,
                                           D7135_MSG_SET, SPARE_DISK_NAME));
                                }
                        } else {
                                /* Disk belongs to a LUN. Leave name blank. */
                                strcpy(tmp_buff, " ");
                        }
                        sprintf(buff, cat_ptr,
                                tmp_buff,
                                pdisk_locs[disk_ptr->ch][disk_ptr->id],
                                disk_ptr->ucode_level, /* uCode level. */
                                disk_ptr->capacity,
                                (char *)diag_cat_gets(catd, D7135_MSG_SET,
                                                  disk_ptr->status_msg));
                        if (disk_ptr->srn) {
                                /* Call out an SRN for the disk. */
                                sprintf(arr.fru_info, "%X:%X:%d:%d:%X",
                                        0x845, disk_ptr->srn,
                                        disk_ptr->ch, disk_ptr->id,
                                        disk_ptr->ffc);
                                rc = raid_fru(PDISK_FRU_TYPE);
                        }
                        /* Add buff to the array status. */
                        raid_add_buff(buff);
                        disk_ptr = disk_ptr->next;
                } /* end while */
                lun_ptr = lun_ptr->next;
                strcpy(buff, "\n");
                raid_add_buff(buff);
        } /* end while */

        return;

} /* end raid_build_msg() */

/*
 *
 * NAME: raid_add_buff()
 *
 * FUNCTION:
 *
 * NOTES:
 *
 * RETURNS: void
 *
 */

void raid_add_buff(char buff[]) {
        char *ptr;

        /* dt(25,"In raid_add_buff()"); */
        dt(25,buff);

        /* Allocate for, and copy the contents of buff */
        /* to the array_status message pointer.        */
        if (array_status != (char *)NULL) {
                /* Use ptr to hold the current array_status. */
                ptr = (char *)malloc(strlen(array_status) + 1);
                strcpy(ptr, array_status);
                /* Free array_status and reallocate. */
                free(array_status);
                array_status = (char *)malloc(strlen(ptr) + 1 +
                                              strlen(buff) + 1);
                /* Copy current contents back and add buff. */
                strcpy(array_status, ptr);
                strcat(array_status, buff);
                free(ptr);
        } else {
                /* First allocation for array_status. */
                array_status = (char *)malloc(strlen(buff) + 1);
                strcpy(array_status, buff);
        }
        return;

} /* end raid_add_buff() */

/*
 *
 * NAME: raid_ela()
 *
 * FUNCTION: Perform Error Log Analysis on the 7135 RAIDiant Array.
 *
 * NOTES:
 *
 * RETURNS: SCATU_GOOD =
 *
 */

int raid_ela(char *task) {
        int ch, id;
        int rc;
        int ela_rc;
        int op_flag = INIT;
        struct  errdata err_data;
        int dev_counter = 0;
        int exit_flag = FALSE;
        int init_flag = TRUE;
        char *dd; /* pointer for error log detail_data. */
        char *ptr;
	char ela_criteria[128];

        dt(25,"In raid_ela()");
        dt(20,1,"device name",tm_input.dname);

        /* Get the ERP data for this task. */
        ptr = get_data(MISC_TASK_DATA, atoi(task));
        da.erp_str = cat_data(ERP_DATA, " ", ";", ptr);

        if (ELA_NOT_SUPPORTED) {
                dt(25,"Test mode does not support ELA.");
                return(DM_STATUS_GOOD);
        }

        if (ELA_MODE_ONLY) {
                /* Only do ELA for this device. */
                dev_counter = arr.linfo.num = 0;
        }

        while (op_flag != TERMI) {
                dt(10,1,"  device counter",dev_counter);
                if (dev_counter < arr.linfo.num) {
                        /* Get the errors for an hdisk in the array. */
                        sprintf(ela_criteria, "%s -N %s", tm_input.date,
                                (arr.cudv + dev_counter)->name);
                } else if (dev_counter == arr.linfo.num) {
                        /* Get the errors for this device. */
                        sprintf(ela_criteria, "%s -N %s", tm_input.date,
                                tm_input.dname);
                } else {
                        /* No more device names, break out of the loop. */
                        op_flag = TERMI;
                        break;
                }
                init_flag = TRUE;
                while(op_flag != TERMI) {
                        if (init_flag == TRUE) {
                                op_flag = INIT;
                                init_flag = FALSE;
                        } else {
                                op_flag = SUBSEQ;
                        }
                        ela_rc = error_log_get(op_flag, 
					       ela_criteria, &err_data );
                        dt(20,1,"  ela_criteria",ela_criteria);
                        dt(10,2,"  ela_rc",ela_rc,"  op_flag",op_flag);
                        if( ela_rc < 1 ) {
                                /* Terminate the error log routine and */
                                /* go back for another device name.    */
                                rc = error_log_get(TERMI, ela_criteria, &err_data);
                                break;
                        }
                        dd = err_data.detail_data;
                        dt(16,1,"  ELA error ID",err_data.err_id);

                        switch(err_data.err_id) {
                        case ERRID_DISK_ARRAY_ERR4:
                                /* Check for Predictive Failure Analysis */
                                /* (ASC/ASCQ 5D/00) in the detail data.  */
                                if (ELA_PFA(dd)) {
					ch = (ELA_FRUC(dd) >> 4);
					id = (ELA_FRUC(dd) & 0x0F);
					ptr = get_pdisk_pid(arr.dar_name, ch, id);
                                        rc = get_pdisk_ffc(ptr);
                                        sprintf(arr.fru_info,
                                                "%X:%X:%d:%d:%X", 0x845, 0x199, 
						ch, id, rc);
                                        rc = raid_fru(ELA_PDISK_PFA_FRU_TYPE);
					exit_flag = TRUE;
                                }
                                break;
                        case ERRID_DISK_ARRAY_ERR6:
                        case ERRID_DISK_ARRAY_ERR14:
                                if ((ELA_MODE_ONLY) && (ELA_FRUC(dd) == 0x06)){
                                        sprintf(arr.fru_info,
                                                "%X:%X", dac_ffc, 0x215);
                                        rc = raid_fru(SPECIFIC_FRU_TYPE);
					exit_flag = TRUE;
                                } else if (ELA_FRUC(dd) >= 0x10) {
					ch = (ELA_FRUC(dd) >> 4);
					id = (ELA_FRUC(dd) & 0x0F);
					ptr = get_pdisk_pid(arr.dar_name, ch, id);
                                        rc = get_pdisk_ffc(ptr);
                                        sprintf(arr.fru_info,
                                                "%X:%X:%d:%d:%X", 0x845, 0x130,
						ch, id, rc);
                                        rc = raid_fru(ELA_PDISK_FRUC_FRU_TYPE);
					exit_flag = TRUE;
                                }
                                break;
                        case ERRID_DISK_ARRAY_ERR7:
                                /* Controller Health Check failed. */
                                sprintf(arr.fru_info, "%X:%X:%s",
                                        dac_ffc, 0x406, err_data.location);
                                rc = raid_fru(ELA_CNTRL_FRU_TYPE);
				exit_flag = TRUE;
                                break;
                        case ERRID_DISK_ARRAY_ERR8:
                                /* Controller failure. */
                                sprintf(arr.fru_info, "%X:%X:%s",
                                        dac_ffc, 0x401, err_data.location);
                                rc = raid_fru(ELA_CNTRL_FRU_TYPE);
				exit_flag = TRUE;
                                break;
                        case ERRID_DISK_ARRAY_ERR9:
                                /* Controller failure. */
                                sprintf(arr.fru_info, "%X:%X:%s",
                                        dac_ffc, 0x406, err_data.location);
                                rc = raid_fru(ELA_CNTRL_FRU_TYPE);
				exit_flag = TRUE;
                                break;
                        case ERRID_DISK_ARRAY_ERR10:
                                /* Controller failure. */
                                sprintf(arr.fru_info, "%X:%X:%s",
                                        dac_ffc, 0x405, err_data.location);
                                rc = raid_fru(ELA_CNTRL_FRU_TYPE);
				exit_flag = TRUE;
                                break;
                        case ERRID_DISK_ARRAY_ERR12:
				ch = (ELA_FRUC(dd) >> 4);
				id = (ELA_FRUC(dd) & 0x0F);
				if (!VALID_PDISK_CH(ch) || 
				    !VALID_PDISK_ID(id)) {
					dt(25,"Invalid CH/ID.");
					break;
				}
				ptr = get_pdisk_pid(arr.dar_name, ch, id);
                                rc = get_pdisk_ffc(ptr);
                                sprintf(arr.fru_info,
                                        "%X:%X:%d:%d:%X", 0x845, 0x403,
					ch, id, rc);
                                rc = raid_fru(ELA_PDISK_FRUC_FRU_TYPE);
				exit_flag = TRUE;
                                break;
                        case ERRID_DISK_ARRAY_ERR13:
                                /* Controller failure. */
                                sprintf(arr.fru_info, "%X:%X:%s",
                                        dac_ffc, 0x404, err_data.location);
                                rc = raid_fru(ELA_CNTRL_FRU_TYPE);
				exit_flag = TRUE;
                                break;
                        default:
                                break;
                        }
                }
                ++dev_counter;
        } /* end while */

        if ((exit_flag == TRUE) || (rc != DM_STATUS_GOOD)) {
                clean_up(DM_STATUS_BAD);
        }

        return(DM_STATUS_GOOD);

} /* end raid_ela() */

/*
 *
 * NAME: raid_fru()
 *
 * FUNCTION: Populate a FRU Bucket struct by calling insert_frub() and
 *           addfrub(). The Diag Controller will display the FRU after
 *           the DA exits.
 *
 * NOTES:
 *
 * RETURNS:
 *
 */

int raid_fru(int fru_type) {
        int i, rc;
        int conf = 0;
        int orig_ffc, pdisk_ffc, ffc = 0, rcode = 0;
	int changed_dac_ffc = FALSE;
        int r_code_val;
        int ch;
        int id;
        int exit_flag = FALSE;
        int fruc; /* FRU Code */
        int ifrub;
        int ifrus = 0;
        char *ptr;

        check_asl();

        dt(10,1,"In raid_fru(), FRU type",fru_type);
        dt(20,1,"   FRU info",arr.fru_info);

        /* The first two items in the FRU info are the SRN and reason code. */
        ffc = strtol(str_parse(arr.fru_info, ":", 1), ep, 16);
        rcode = strtol(str_parse(arr.fru_info, ":", 2), ep, 16);

        if (fru_type == FRUC_FRU_TYPE) {
                fruc = strtol(str_parse(arr.fru_info, ":", 2), ep, 16);
                /* Look at fruc (Sense data byte 14, FRUCode) to */
                /* determine the correct FFC and reason code.    */
                switch (fruc) {
                case 0x00: /* No FRU Code. */
                        /* No FRU found. */
                        ffc = 0;
                        rcode = 0;
                        break;
                case 0x01: /* Adapter/Controller */
                        ffc = dac_ffc;
                        rcode = 0x101;
                        break;
                case 0x02:
                case 0x03:
                case 0x04:
                case 0x05: /* Controller. */
                        ffc = dac_ffc;
                        rcode = 0x102;
                        break;
                case 0x06: /* Check the MODE SENSE (2D) data. */
                        ffc = dac_ffc;
                        r_code_val = atoi(str_parse(arr.fru_info, ":", 3));
                        rcode = raid_check_fru_2D(r_code_val);
                        break;
                default:
                        ffc = 0;
                        rcode = 0;
                        break;
                } /* end switch */
        } else if (fru_type == TASK_FRU_TYPE) {
                /* The rcode will hold the task number. */
                /* Convert it to a reason code value.   */
                switch(rcode) {
                default:
                        rcode = 0x101;
                        break;
                }
        } else if (fru_type >= PDISK_FRU_TYPE) {
		pdisk_ffc = strtol(str_parse(arr.fru_info, ":", 5), ep, 16);
		if (pdisk_ffc == 0) {
			/* PDisk could not be identified. */
        		fru_type = UNKNOWN_PDISK_FRU_TYPE;
			ffc = dac_ffc;
			rcode = 0x310;
		}
        } /* end if */

        /* No FRU to callout. */
        if ((ffc == 0) || (rcode == 0)) {
                dt(25,"  No failing FRU's found.");
                return(DM_STATUS_GOOD);
        }

	if ((ffc == dac_ffc) && (dac_ffc != INF_DAC_FFC)) {
		/* dac is a 9302, change FFC so the */
		/* fru bucket will be found.        */
		ffc = INF_DAC_FFC;
		changed_dac_ffc = TRUE;
	}

        /* Search the DA FRU Bucket array (raid_frub) for an FFC */
        /* and rcode value matching the reason code passed in. */
        for (ifrub = 0; !((raid_frub[ifrub].sn == ffc) &&
                          (raid_frub[ifrub].rcode == rcode)); ifrub++) {
                if (raid_frub[ifrub].rcode == 0) {
                        /* Reached the end of the DA FRU array. */
                        dt(16,1,"ERROR: Invalid SRN", rcode);
                        return(DM_ERROR_OTHER);
                }
        }
	if (changed_dac_ffc == TRUE) {
		raid_frub[ifrub].sn = DAC_FFC_9302;
	} else {
		orig_ffc = raid_frub[ifrub].sn;
	}
        strcpy(raid_frub[ifrub].dname,tm_input.dname);

        switch(fru_type) {
        case ELA_PDISK_PFA_FRU_TYPE:
        case ELA_PDISK_FRUC_FRU_TYPE:
        case PDISK_FRUC_FRU_TYPE:
        case UNKNOWN_PDISK_FRU_TYPE:
        case PDISK_FRU_TYPE:
                ch = atoi(str_parse(arr.fru_info, ":", 3));
                id = atoi(str_parse(arr.fru_info, ":", 4));
                if (!VALID_PDISK_CH(ch) || !VALID_PDISK_ID(id)) {
                        /* Invalid CH or ID in FRUC. */
                        dt(16,1,"ERROR: Invalid CH or ID in FRUC",fruc);
                        /* Set them to zero for UNKNOWN location. */
                        ch = 0;
                        id = 0;
                }
                /* Fill in the physical disk location. */
                strcpy(raid_frub[ifrub].frus[0].floc,
                       pdisk_locs[ch][id]);
                if (fru_type != UNKNOWN_PDISK_FRU_TYPE) {
                        raid_frub[ifrub].sn = pdisk_ffc;
                }
		strcpy(raid_frub[ifrub].frus[0].fname, 
				get_pdisk_capacity(arr.dar_name, ch, id));
                break;
        case LUN_FRU_TYPE:
                /* Get the LUN name and location. */
                ptr = str_parse(arr.fru_info, ":", 3);
                strcpy(raid_frub[ifrub].frus[0].fname, ptr);
                ptr = str_parse(arr.fru_info, ":", 4);
                strcpy(raid_frub[ifrub].frus[0].floc, ptr);
                break;
        default:
                break;
        }

        ffc = raid_frub[ifrub].sn;
        rc = insert_frub(&tm_input, &raid_frub[ifrub]);
        if (rc != 0) {
                dt(16,1,"ERROR: Insert FRUB", rcode);
                return(DM_ERROR_OTHER);
        }
        raid_frub[ifrub].sn = ffc;
        rc = addfrub(&raid_frub[ifrub]);
        if (rc != 0) {
                dt(16,1,"ERROR: Add FRUB", rcode);
                return(DM_ERROR_OTHER);
        }
        raid_frub[ifrub].sn = orig_ffc;

        /* For certain SRN's, display the "run READI ..." menu goal. */
        switch (rcode) {
        case 0x135:
        case 0x160:
        case 0x162:
        case 0x165:
        case 0x166:
                rc = display_menugoal("9046");
                break;
        default:
                break;
        }

        /* If the FFC is 845 or 846 (Disk FRU), display the "Check */
        /* cables and terminators ..." menu goal.                  */
        if (ffc != dac_ffc) {
                rc = display_menugoal("9044");
        }

        if (exit_flag) {
                clean_up(DM_STATUS_BAD);
        } else {
                DA_SETRC_STATUS(DA_STATUS_BAD);
        }

        return(DM_STATUS_BAD);

} /* end raid_fru() */

/*
 *
 * NAME: raid_check_fru_2D()
 *
 * FUNCTION: Check Mode Sense data (2D) for failing FRUS'
 *
 * NOTES: see array.h for FRU_2D_XXX macros.
 *
 * RETURNS: 0 if no failing FRU, otherwise
 *          reason code of failing FRU.
 */

int raid_check_fru_2D(int r_code_param) {
        int reason_code = 0;

        dt(25,"In raid_check_fru_2D()");
        dt(10,1," rcode", r_code_param);

        /* Check for failing FRU's (refer to array.h for MACRO's */
        switch (r_code_param) {
        case 200:
                if (FRU_2D_200)
                        reason_code = 0x200;
                break;
        case 201:
                if (FRU_2D_201)
                        reason_code = 0x201;
                break;
        case 202:
                if (FRU_2D_202)
                        reason_code = 0x202;
                break;
        case 203:
                if (FRU_2D_203)
                        reason_code = 0x203;
                break;
        case 204:
                if (FRU_2D_204)
                        reason_code = 0x204;
                break;
        case 205:
                if (FRU_2D_205)
                        reason_code = 0x205;
                break;
        case 206:
                if (FRU_2D_206)
                        reason_code = 0x206;
                break;
        case 207:
                if (FRU_2D_207)
                        reason_code = 0x207;
                break;
        case 208:
                if (FRU_2D_208)
                        reason_code = 0x208;
                break;
        case 209:
                if (FRU_2D_209)
                        reason_code = 0x209;
                break;
        case 210:
                if (FRU_2D_210)
                        reason_code = 0x210;
                break;
        case 211:
                if (FRU_2D_211)
                        reason_code = 0x211;
                break;
        case 212:
                if (FRU_2D_212)
                        reason_code = 0x212;
                break;
        case 213:
                if (FRU_2D_213)
                        reason_code = 0x213;
                break;
        case 214:
                if (FRU_2D_214)
                        reason_code = 0x214;
                break;
        default:
                dt(25,"Invalid rcode!");
                break;
        }
        return(reason_code);

} /* end check raid_check_fru_2D() */

/* end d7135a.c */
