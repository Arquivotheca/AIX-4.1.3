static char sccsid[] = "@(#)53  1.4  src/bos/diag/util/u7135/u7135a.c, dsau7135, bos41J, 9520B_all 5/18/95 09:47:37";
/*
 *   COMPONENT_NAME: DSAU7135
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "u7135m.h"

/*
 *
 * NAME: raid_array_status()
 *
 * FUNCTION: Build a linked list of the dac's, LUNS's and physical
 *	     disks in the array. The linked list is refered to as
 *	     "array status".
 *
 * NOTES:
 *	When called with device equal to
 *    CNTRL/ALT_CNTRL: Assumes valid MODE SENSE data (page 2C)
 *		       in in tucb.scsitu.data_buffer.
 *		HDISK: Assumes valid MODE SENSE data (page 2B)
 *		       in in tucb.scsitu.data_buffer.
 *		SPARE: Assumes valid MODE SENSE data (page 2A)
 *		       in in tucb.scsitu.data_buffer. Adds status
 *		       of all disks in the array.
 *
 * RETURNS: DM_STATUS_GOOD = Added device to array status.
 *	    DM_STATUS_BAD  = Invalid device type.
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
	case CNTRL_STATUS:
		/* The SA dosent require dac devides to be added. */
		rc = DM_STATUS_GOOD;
		return(rc);
	case LUN_STATUS: /* MODE SENSE page 2B (Logical Array Page) */
		tmp_cudv = (arr.cudv + arr.lun_counter);
		strcpy(new_lun->name, tmp_cudv->name);
		strcpy(new_lun->location, tmp_cudv->location);
		if (device & DEAD_LUN) {
			new_lun->status_byte = BUSY_LUN;
			strcpy(new_lun->raid_level, "?");
			strcpy(new_lun->capacity, "?");
			return(DM_STATUS_GOOD);
		} else {
			new_lun->status_byte = tucb.scsitu.data_buffer[14];
		}
		sprintf(new_lun->raid_level, "%s %d",
			(char *)diag_cat_gets(catd, U7135_MSG_SET, RAID),
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
						      U7135_MSG_SET, GB));
		} else {
			sprintf(new_lun->capacity, "%3.2f%s",
				capacity,
				(char *)diag_cat_gets(catd,
						      U7135_MSG_SET, MB));
		}
		/* The phsical disks need to be added to new_lun */
		/* so set rc to indicate more work to do in this */
		/* function. The status for each physical disk	 */
		/* will be determined when called to add SPARE	 */
		/* disks with the physical mode sense (2A) data. */
		rc = DM_MORE_CONT;
		break;
	case SPARE_STATUS: /* MODE SENSE page 2A (Array Physical Page)	*/
		/* Add the status for all the LUN physical disks */
		/* that are already in the array status.	 */
		if (raid_hptr != (struct lun *)NULL) {
			raid_disk_status();
		}
		/* Now add spare disks to the end of the array status. */
		if (sa_selected != SA_CERTIFY_LUN) {
			raid_add_spares((device & 0xF0));
		}
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
	/* what physical disks are associated with this HDISK.		 */
	new_lun->pdisk_count = 0;
	for (i = 0, offset = 48; i < 16; offset += 2, i++) {
		disk_bits = (unsigned int)
			    (tucb.scsitu.data_buffer[offset] << 8) |
			     tucb.scsitu.data_buffer[offset+1];
		/* Mask each bit of the disk_bits. If a bit is set, */
		/* create a disk and fill in the channel and id.    */
		/* The disk name and disk status will be determined */
		/* after all disks have been identified.	    */
		for (j = 0, disk_mask = 1; j < 16; disk_mask *= 2, j++) {
			if (disk_bits & disk_mask) {
				/* A disk exists, add it to the list. */
				new_lun->pdisk_count += 1;
				new_disk = (struct pdisk *)calloc(1,
					    sizeof(struct pdisk));
				new_disk->ch = j + 1;
				new_disk->id = i;
				pdisk_lun_id[new_disk->ch][new_disk->id] = 
								     dev.lun;
				dt(16,1,"Pdisk LUN",dev.lun);
				if (((j + 1) > MAX_SUPPORTED_CHs) ||
				    (i > MAX_SUPPORTED_IDs)) {
					dt(10,1,"Invalid disk, ch",
					   (j+1),"id",i);
					new_disk->ch = 0;
					new_disk->id = 0;
				}
				++lun_owns_pdisk[new_disk->ch][new_disk->id];
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
		if (!strcmp(new_lun->name, "SPARE")) {
			arr.num_spares = new_lun->pdisk_count;
		}
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
 *	     decides if the entry is added to the beginning or the end
 *	     of the array status linked list.
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
	struct lun *lun_ptr;
	struct pdisk *disk_ptr;

	dt(25,"In raid_disk_status()");
	lun_ptr = raid_hptr;
	while (lun_ptr != (struct lun *)NULL) {
		disk_ptr = lun_ptr->pdisk;
		/* dt(20,1,"Disk list, name", lun_ptr->name); */
		while (disk_ptr != (struct pdisk *)NULL) {
			/* dt(10,2,"ch",disk_ptr->ch,"id",disk_ptr->id); */
			disk_ptr->status_byte = PDISK_STATUS;
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
 *	     array status.
 *
 * NOTES: See comments.
 *
 * RETURNS: void
 *
 */

void raid_add_spares(uchar status_type) {
	int i;
	int ch = 0, id = 0;
	int status_byte;
	int valid_pdisk;
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
	strcpy(new_lun->name, "SPARE");
	new_lun->next = (struct lun *)NULL;
	new_lun->pdisk = (struct pdisk *)NULL;
	disk_ptr = (struct pdisk *)NULL;

	/* Walk through the Array Physical Page Mode Sense data.  */
	/* The disk status data starts at byte offset 14 (tucb).  */
	for (i = 14; i < ((unsigned)data_buffer[13] + 12); i++) {
		status_byte = (unsigned)data_buffer[i];
		if (ch >= MAX_CHs) {
			ch = 0;
			++id;
		}
		++ch;
		valid_pdisk = 0;
		if (VALID_PDISK_CH(ch) && VALID_PDISK_ID(id)) {
			if ((status_type == ALL_PDISKS) &&
			    ((status_byte & MAIN_STATUS_MASK) != 0x01)) {
				/* All except non-existent */
				valid_pdisk = 1;
			} else if ((sa_selected == SA_FORMAT_PDISK) &&
				   ((status_byte & 0x0F) == FAILED_PDISK) &&
				   (!lun_owns_pdisk[ch][id])) {
				/* Only format failed drives on Infiniti. */
				valid_pdisk = 1;
			} else if ((sa_selected != SA_FORMAT_PDISK) && 
				   ((status_byte == SPARE) ||
				    (status_byte == HOT_SPARE))) {
				/* Spares / Hot Spares only. */
				valid_pdisk = 1;
			}

		} /* end if valid ch id */
		if (valid_pdisk) {
			dt(16,1,"Status byte", status_byte);
			new_disk = (struct pdisk *)calloc(1,
				    sizeof(struct pdisk));
			new_disk->ch = ch;
			new_disk->id = id;
			pdisk_lun_id[new_disk->ch][new_disk->id] = dev.lun;
			dt(16,1,"pdisk LUN",dev.lun);
			new_disk->status_byte = status_byte;
			strcpy(new_disk->capacity,
				get_pdisk_capacity(arr.dar_name, ch, id));
			strcpy(new_disk->ucode_level, " ");
			if (disk_ptr == (struct pdisk *)NULL) {
				disk_ptr = new_disk;
			} else {
				new_disk->next = disk_ptr;
				disk_ptr = new_disk;
			}
			new_lun->pdisk_count++;
		} /* end if valid pdisk */
	} /* end for */

	if (new_lun->pdisk_count) {
		new_lun->pdisk = disk_ptr;
		raid_add_to_status(END_OF_LIST, new_lun);
	}

	/* Get the number of spare physical disks. */
	arr.num_spares = new_lun->pdisk_count;

	free(data_buffer);

	return;

} /* end raid_add_spares() */

/*
 *
 * NAME: raid_get_menu_item()
 *
 * FUNCTION:
 *
 * NOTES: The flag param is used to tell what action to take.
 *	  Flag = MENU_ITEM : Called for a selection menu.
 *	  Flag = MENU_ITEM : Called for a selection menu.
 *
 * RETURNS: char *
 *
 */

char *raid_get_menu_item(int item, int flag) {
	int i, rc;
	char *cat_ptr;
	struct lun *lun_ptr;
	struct pdisk *disk_ptr;
	char *buff;

	/* dt(25,"In raid_get_menu_item()"); */
	switch (sa_selected) {
	case SA_CERTIFY_LUN:
		cat_ptr = (char *)diag_cat_gets(catd, U7135_MSG_SET,
					LUN_SELECTION_ITEM);
		lun_ptr = raid_hptr;
		if (lun_ptr == (struct lun *)NULL) {
			/* No LUN's configured in the array. */
			dt(25,"ERROR: No LUNs");
			clean_up(DM_STATUS_BAD);
		}
		/* Find the LUN for this item number. */
		for (i = 0;
		     (lun_ptr->next != (struct lun *)NULL) && (i < item);
		     i++) {
			lun_ptr = lun_ptr->next;
		}
		if (i != item) {
			/* Unexpected error. */
			dt(25,"ERROR: Invalid item number (LUN).");
			clean_up(DM_ERROR_UNEXPECTED);
		}
		for (i = 0;
		     lun_ptr->status_byte !=
			      sbytes_2B[i].status_byte; i++) {
			if (sbytes_2B[i].status_byte == 0xFF) {
				/* UNKNOWN is the end, so break. */
				break;
			}
		}
		/* dt(20,1,"  lun name",lun_ptr->name); */
		if (flag == GET_DEV_INFO) {
			/* Allocate for device location. */
			buff = (char *)calloc(1, NAMESIZE + 1);
			strcpy(buff, lun_ptr->location);
			strcpy(arr.lun_name, lun_ptr->name);
			strcpy(dev.capacity, lun_ptr->capacity);
			dev.lun = lun_ptr->location[10] - '\0';
			dev.pdisk_type = lun_ptr->pdisk_type;
			break;
		}
		/* Allocate for a menu item. */
		cat_ptr = (char *)diag_cat_gets(catd, U7135_MSG_SET,
					LUN_SELECTION_ITEM);
		buff = (char *)calloc(1, strlen(cat_ptr) + 128);
		/* Fill in the message with the LUN info. */
		sprintf(buff, cat_ptr,
			lun_ptr->name,
			lun_ptr->location,
			lun_ptr->raid_level,
			lun_ptr->capacity,
			(char *)diag_cat_gets(catd, U7135_MSG_SET,
					  sbytes_2B[i].status_msg));
		break;
	case SA_CERTIFY_PDISK:
	case SA_FORMAT_PDISK:
	case SA_uCODE_PDISK:
		/* Find the lun struct with the name SPARE for	 */
		/* the physical disks. Fill in the info as well. */
		lun_ptr = raid_hptr;
		if (lun_ptr == (struct lun *)NULL) {
			/* If no lun structs, no spare disks. */
			dt(25,"NO SPARE's (or LUN's) FOUND.");
			clean_up(DM_ERROR_UNEXPECTED);
		}
		while ((lun_ptr->next != (struct lun *)NULL) &&
		       (strcmp(lun_ptr->name,"SPARE"))) {
			lun_ptr = lun_ptr->next;
		}
		if (strcmp(lun_ptr->name,"SPARE")) {
			dt(25,"No spares found.");
			clean_up(DM_ERROR_UNEXPECTED);
		}
		if (item > lun_ptr->pdisk_count) {
			dt(25,"ERROR: Invalid item number (SPARE).");
			clean_up(DM_ERROR_UNEXPECTED);
		}
		disk_ptr = lun_ptr->pdisk;
		for (i = 0;
		     (disk_ptr->next != (struct pdisk *)NULL) && (i < item);
		     i++) {
			disk_ptr = disk_ptr->next;
		}
		if (i != item) {
			dt(25,"ERROR: Item not found (SPARE).");
			clean_up(DM_ERROR_UNEXPECTED);
		}

		for (i = 0;
		     disk_ptr->status_byte !=
			      sbytes_2A[i].status_byte; i++) {
			if (sbytes_2A[i].status_byte == 0xFF) {
				/* UNKNOWN is the end, so break. */
				break;
			}
		}
		disk_ptr->status_msg = sbytes_2A[i].status_msg;

		/* Save a pointer to the selected pdisk. */
		arr.pdisk_ptr = disk_ptr;

		if (flag == GET_DEV_INFO) {
			/* Allocate for device location. */
			buff = (char *)malloc(80);
			strcpy(buff, pdisk_locs[disk_ptr->ch][disk_ptr->id]);
			strcpy(dev.capacity, disk_ptr->capacity);
			arr.pdisk_ch = disk_ptr->ch;
			arr.pdisk_id = disk_ptr->id;
			/* Break out with dev info. */
			break;
		}
		/* Allocate for a menu item. */
		cat_ptr = (char *)diag_cat_gets(catd, U7135_MSG_SET,
					PDISK_SELECTION_ITEM);
		buff = (char *)malloc(strlen(cat_ptr) + 80);
		strcpy(attr, " ");
		if (sa_selected == SA_uCODE_PDISK) {
			if (disk_ptr->flags == PDISK_DL_FAILED) {
				strcpy(attr,
				       (char *)diag_cat_gets(catd, 
							     U7135_MSG_SET,
						             FAILED));
			} else if (disk_ptr->flags == PDISK_DL_PASSED) {
				strcpy(attr,
				       (char *)diag_cat_gets(catd, 
							     U7135_MSG_SET,
						             PASSED));
			} else if (disk_ptr->flags == PDISK_DL_NO_FILE) {
				strcpy(attr,
				       (char *)diag_cat_gets(catd, 
							     U7135_MSG_SET,
						             FILE_NOT_FOUND));
			}
		}
		sprintf(buff, cat_ptr, attr, 
			diag_cat_gets(catd, U7135_MSG_SET, 
				      disk_ptr->status_msg), 
			disk_ptr->capacity,
			pdisk_locs[disk_ptr->ch][disk_ptr->id],
			disk_ptr->ucode_level);
		break;
	default:
		clean_up(DM_ERROR_UNEXPECTED);
		break;
	}

	return(buff);

} /* end raid_get_menu_item() */

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
 * NAME: raid_certify()
 *
 * FUNCTION: Issue READ (10) comands.
 *
 * NOTES:
 *
 * RETURNS: DM_STATUS_GOOD or DM_STATUS_BAD
 *
 */

int raid_certify(void) {
	int i;
	int rc = DM_STATUS_CONT;
	int rs_rc;
	int retry_count = 0;
	int blocks;
	int certify_status[MAX_SUPPORTED_CHs];

	for (i = 0; i < MAX_SUPPORTED_CHs; i++) {
		certify_status[i] = 0;
	}

	while ((rc == DM_STATUS_CONT) && (retry_count < MAX_CERTIFY_RETRY)) {
		if (dev.prev_percent_complete != dev.percent_complete) {
			if (sa_selected == SA_CERTIFY_LUN) {
				rc = display_screen("3104");
			} else {
				rc = display_screen("3103");
			}
			if ((rc == DM_USER_CANCEL) ||
			    (rc == DM_USER_EXIT)) {
				clean_up(rc);
			}
			rc = DM_STATUS_CONT;
		}
		/* Initialize the READ (10) tucb struct. */
		tucb.header.tu = 0x16;
		/* Zero out the TUCB struct. */
		rc = scsitu_init(&tucb);
		if (rc != SCATU_GOOD) {
			dt(25,"ERROR: READ (10) INIT TUCB");
			return(DM_STATUS_BAD);
		}
		tucb.header.mfg  = 0;
		tucb.header.loop = 1;
		tucb.header.r1	 = 0;
		tucb.header.r2	 = 0;
		tucb.scsitu.cmd_timeout = 60;
		tucb.scsitu.command_length = 10;
		tucb.scsitu.flags = B_READ;
		tucb.scsitu.ioctl_pass_param = DKIOCMD;
		tucb.scsitu.scsi_cmd_blk[0] = 0x28;
		tucb.scsitu.scsi_cmd_blk[1] |= 0x18;
		tucb.scsitu.scsi_cmd_blk[1] |= dev.lun << 5;
		tucb.scsitu.scsi_cmd_blk[2] =
				    (uchar)(dev.current_lba >> 24);
		tucb.scsitu.scsi_cmd_blk[3] =
				    (uchar)(dev.current_lba >> 16);
		tucb.scsitu.scsi_cmd_blk[4] =
				    (uchar)(dev.current_lba >>	8);
		tucb.scsitu.scsi_cmd_blk[5] =
					   (uchar) dev.current_lba;
		tucb.scsitu.scsi_cmd_blk[6] = 0;

		blocks = (TU_BUFFER_SIZE / dev.block_size) - 1;

		if (blocks > (dev.last_lba - dev.current_lba)) {
			blocks = dev.last_lba - dev.current_lba;
		}
		tucb.scsitu.data_length = blocks * dev.block_size;

		tucb.scsitu.scsi_cmd_blk[7] = (uchar)(blocks >> 8);
		tucb.scsitu.scsi_cmd_blk[8] = (uchar) blocks;
		tucb.scsitu.scsi_cmd_blk[9] = 0;
		tucb.scsitu.data_buffer = tu_buffer;
		tucb.header.r1 = 3;

		if (arr.spt_flag) {
			/* Call the ioctl. */
			rc = ptrs_ioctl(sa.task);
		} else {
			/* Execute the READ (10) command. */
			rc = exectu(fdes, &tucb);
		}
		if (CHECK_CONDITION(rc)) {
			dt(10,1,"Certify exectu() rc",rc);
			if (!arr.spt_flag) {
				rs_rc = req_sense();
				if (rs_rc != SCATU_GOOD) {
					dt(25,"ERROR: Certify CAC req senese");
					return(DM_STATUS_BAD);
				}
			}
			rc = certify_cac(certify_status);
			switch (rc) {
			case DM_STATUS_BAD:
				dt(25,"ERROR: Certify CAC");
				return(DM_STATUS_BAD);
				break;
			case DM_ERROR_UNEXPECTED:
				++retry_count;
				break;
			default:
				break;
			}
			rc = DM_STATUS_GOOD;
		}
		if (rc == SCATU_GOOD) {
			dev.current_lba += blocks;
			if (dev.current_lba >= dev.last_lba) {
				return(DM_STATUS_GOOD);
			}
			dev.prev_percent_complete = dev.percent_complete;
			dev.percent_complete =
				       (100L * dev.current_lba) / dev.last_lba;
			rc = DM_STATUS_CONT;
		} else if (rc != DM_STATUS_CONT) {
			dt(10,1,"ERROR: Certify rc",rc);
			return(DM_STATUS_BAD);
		}

	} /* end while */

	if (rc != DM_STATUS_GOOD) {
		rc = DM_STATUS_BAD;
	}

	return(rc);

} /* end raid_certify() */

/*
 *
 * NAME: certify_cac()
 *
 * FUNCTION: Examine the sense data from a Certify READ (10) command.
 *
 * NOTES:
 *
 * RETURNS: DM_STATUS_CONT or DM_STATUS_BAD
 *
 */

int certify_cac(int status[]) {
	int i;
	int rc;

	/* Check for Predictive Failure Analysis. */
	if (sa.scode == 0x5D00) {
		/* PFA reached. */
		if (sa_selected == SA_CERTIFY_LUN) {
			arr.pdisk_ch = (sa.sdata[14] >> 4);
			arr.pdisk_id = (sa.sdata[14] & 0x0F);
		}
		rc = display_screen("3107");
		clean_up(DM_STATUS_GOOD);
	}

	for (i = 0; i < MAX_SUPPORTED_CHs; i++) {
		dt(16,1,"status",status[i]);
	}

	switch (sa.skey) {
	case 0x01:
		rc = DM_STATUS_GOOD;
		dt(25,"Recovered SK");
		break;
	case 0x03:
	case 0x04:
		if (sa_selected == SA_CERTIFY_LUN) {
			arr.pdisk_ch = (sa.sdata[14] >> 4);
			arr.pdisk_id = (sa.sdata[14] & 0x0F);

			for (i = 0; i < MAX_SUPPORTED_CHs; i++) {
				if ((status[i] != sa.sdata[14]) &&
				    (status[i] == 0)) {
					/* First time error for this disk. */
					status[i] = sa.sdata[14];
					break;
				} else if (status[i] == sa.sdata[14]) {
					rc = display_screen("3108");
					clean_up(DM_STATUS_GOOD);
				}
			}
		} else {
			if (status[0]) {
				rc = display_screen("3108");
				clean_up(DM_STATUS_GOOD);
			}
			++status[0];
		}
		break;
		rc = DM_STATUS_CONT;
	default:
		rc = DM_ERROR_UNEXPECTED;
		dt(16,1,"SK",sa.skey);
		break;
	}

	return(rc);

} /* end certify_cac() */

/* end u7135a.c */
