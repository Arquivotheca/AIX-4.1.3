static char sccsid[] = "@(#)05	1.3  src/bos/diag/util/udiskenh/udmutil.c, dsaudiskenh, bos41J, 9511A_all 2/16/95 20:36:58";
/*
 *   COMPONENT_NAME: DSAUDISKMNT
 *
 *   FUNCTIONS: cond_free
 *              dev_desc
 *		disk_path
 *		disp_err
 *		disp_read_error
 *		genexit
 *		get_and_print_sense
 *		get_sense
 *		get_block
 *		get_devices
 *		get_lvm_rec
 *		put_block
 *		read_capacity
 *              reassign_block
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
#pragma options nosource
#include "udmutil.h"
#include <sys/scsi.h>
#include <stdio.h>
#include <sys/hd_psn.h>
#pragma options source

extern nl_catd cat_fd;

extern char* get_dev_desc(char*);  /* Probably from libdiag */

/* *************************************************************** */
/*
 * NAME: cond_free()
 *
 * FUNCTION:
 * Function cond_free (conditional free) calls free(x) only if x is not     
 * null and sets x to null so it won't be freed again.  This is much easier 
 * than keeping track of which pointers have or haven't been freed when it's
 * time to clean up. 
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: None.
 *
 */

void cond_free (void **x) {
if (*x) {free(*x); (*x) = NULL;}
}

/* *************************************************************** */
/*
 * FUNCTION:
 * Function disk_path constructs the pathname of the disk special file from
 * the directory name and disk name.  An "r" is prepended to the disk name  
 * if necessary.
 *   
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: A pointer to the pathname.
 */ 

char *disk_path(char *dir, char *name) {
    char *temp = malloc(strlen(dir) + 
                        strlen(name) + 2);
    if (temp == NULL) genexit(-1);
    strcpy(temp,dir);
    if (name[0] != 'r')
        strcat(temp,"r");
    strcat(temp,name);
    return temp;
}

/* *************************************************************** */
#ifdef DBG /* This entire function is only needed for debugging. */
/*
 * FUNCTION:
 * Function get_and_print_sense requests sense data and prints it 
 * to stderr.
 *   
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: None.
 */

void get_and_print_sense(int fd) {
    struct sense_data_t sense_data;
    int i;

    if (!get_sense(fd, &sense_data)) {
      fprintf (stderr, "REQUEST_SENSE succeeded:\n");
      for (i=0; i<SENSE_LENGTH; ++i) {
        fprintf (stderr, "Byte num = %d, value = %02X.\n",
                         i, ((char*)&sense_data)[i]);
      } /* end for */
    }
} /* end get_and_print_sense */
#endif

/* *************************************************************** */
/*
 * FUNCTION: Requests sense data from the device corresponding to the
 *           file descriptor and puts it in sense_data.
 *   
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 0 - success
 *          -1 - failure
 */

int get_sense(int fd, struct sense_data_t *sense_data) {
    struct sc_iocmd scsi_cmd;  /* from scsi.h */
    struct request_sense_cmd_t *request_sense_cmd_ptr;
    int i, rc;

    memset(&scsi_cmd,0,sizeof(struct sc_iocmd));
    scsi_cmd.timeout_value = 30;
    scsi_cmd.flags = B_READ;
    scsi_cmd.command_length = 6;
    scsi_cmd.buffer = (char*)sense_data;
    scsi_cmd.data_length = SENSE_LENGTH;

    request_sense_cmd_ptr =
                        (struct request_sense_cmd_t*) scsi_cmd.scsi_cdb;
    request_sense_cmd_ptr->op_code = SCSI_REQUEST_SENSE;
    request_sense_cmd_ptr->lun = 0;
    request_sense_cmd_ptr->alloc_len = SENSE_LENGTH;
    rc = ioctl(fd,DKIOCMD,&scsi_cmd);
#ifdef DBG
    if (rc == -1)
      fprintf (stderr, "REQUEST_SENSE failed, errno = %d.\n", errno);
#endif
    return rc;
} /* end get_sense */


/* ****************************************************************** */
/*
 * FUNCTION:
 * Reassign_block issues a SCSI command to the file descriptor fd to 
 * reassign the specified logical block to a new physical location. 
 *   
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 0 - success
 *          -1 - failure
 */

int reassign_block(int fd, int block_num) {
    struct sc_iocmd scsi_cmd;  /* from scsi.h */
    struct reassign_cmd_t* reassign_cmd_ptr;
    struct defect_list block_list = {0, 4, block_num};
    int i, rc;

    /* Zero out the entire thing. */

    memset(&scsi_cmd,0,sizeof(struct sc_iocmd));

    /* Now set individual fields. */

    scsi_cmd.timeout_value = 120;
    scsi_cmd.data_length = 8;
    scsi_cmd.buffer = (char*)&block_list;
    scsi_cmd.command_length = 6;

    /* Now build the actual SCSI command descriptor block. */
    /* The command block is scsi_cmd.cdb.  "overlay" the   */
    /* REASSIGN_BLOCKS command on that field and fill it in. */

    reassign_cmd_ptr = 
                      (struct reassign_cmd_t*) scsi_cmd.scsi_cdb;

    reassign_cmd_ptr->op_code = SCSI_REASSIGN_BLOCK;
    reassign_cmd_ptr->lun = 0;  /* Though SCSI-1 standard       */
                                     /* recommends using the actual  */
				     /* LUN here, SCSI-2 recommends  */
				     /* using 0.  I assume the open  */
				     /* and ioctl functions do       */ 
				     /* something that makes         */
				     /* the LUN unnecessary.         */
                                     /* The disks I'm working with   */
                                     /* are all lun 0 anyway.        */

   rc = ioctl(fd,DKIOCMD,&scsi_cmd);
#ifdef DBG
   if (rc == -1) {
       fprintf(stderr,"REASSIGN_BLOCK failed, errno = %d.\n",errno);
       fprintf(stderr,"status_validity = %d, scsi_bus_status = 0x%02X.\n",
                  scsi_cmd.status_validity,scsi_cmd.scsi_bus_status);
       fprintf(stderr,"                      adapter_status = 0x%02X.\n",
                  scsi_cmd.adapter_status);
       get_and_print_sense(fd);
   }
#endif
   return rc;
} /* end reassign_block */

/* ****************************************************************** */
/*
 * FUNCTION:
 * Read_capacity reads the capacity in logical blocks and block    
 * length into the data argument. 
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 0 - success
 *          -1 - failure
 */

int read_capacity(int fd, struct read_capacity_data_t* data) {
    struct sc_iocmd scsi_cmd;  /* from scsi.h */
    struct read_capacity_cmd_t* read_capacity_cmd_ptr;
    int i, rc;

    /* Zero out the entire thing. */

    memset(&scsi_cmd,0,sizeof(struct sc_iocmd));

    /* Now set individual fields. */

    scsi_cmd.timeout_value = 30;
    scsi_cmd.flags = B_READ;      
    scsi_cmd.data_length = 8;
    scsi_cmd.buffer = (char*) data; /* data is a pointer. */
    scsi_cmd.command_length = 10;

    /* Now build the actual SCSI command descriptor block. */
    /* The command block is scsi_cmd.cdb.  "overlay" the   */
    /* READ_CAPACITY command on that field and fill it in. */

    read_capacity_cmd_ptr = 
                      (struct read_capacity_cmd_t*) scsi_cmd.scsi_cdb;

    read_capacity_cmd_ptr->op_code = SCSI_READ_CAPACITY;
    read_capacity_cmd_ptr->lun = 0;  /* Though SCSI-1 standard       */
                                     /* recommends using the actual  */
				     /* LUN here, SCSI-2 recommends  */
				     /* using 0.  I assume the open  */
				     /* and ioctl functions do       */ 
				     /* something that makes         */
				     /* the LUN unnecessary.         */
                                     /* The disks I'm working with   */
                                     /* are all lun 0 anyway.        */
    read_capacity_cmd_ptr->reserved1 = 0;
    read_capacity_cmd_ptr->rel_adr = 0;
    read_capacity_cmd_ptr->block_address = 0;
    read_capacity_cmd_ptr->reserved2 = 0;
    read_capacity_cmd_ptr->vendor_unique1 = 0;
    read_capacity_cmd_ptr->reserved3 = 0;
    read_capacity_cmd_ptr->pmi = 0;
    read_capacity_cmd_ptr->vendor_unique2 = 0;
    read_capacity_cmd_ptr->reserved4 = 0;
    read_capacity_cmd_ptr->flag = 0;
    read_capacity_cmd_ptr->link = 0;

   rc = ioctl(fd,DKIOCMD,&scsi_cmd);
#ifdef DBG
   if (rc == -1) {
       fprintf(stderr,"READ_CAPACITY failed, errno = %d.\n",errno);
       fprintf(stderr,"status_validity = %d, scsi_bus_status = 0x%02X.\n",
                  scsi_cmd.status_validity,scsi_cmd.scsi_bus_status);
       fprintf(stderr,"                      adapter_status = 0x%02X.\n",
                  scsi_cmd.adapter_status);
       get_and_print_sense(fd);
   }
#endif
   return rc;
} /* end read_capacity */

/* ****************************************************************** */
/*
 * FUNCTION:
 * disp_err displays a menu with 3 message         
 * lines from the nl_cat catalog.                  
 * Last line is always "press enter to continue".  
 * Title and error message                         
 * are as specified, and up to 3 text strings may be 
 * substituted into the error message.                  
 * That is, the error message may have up to      
 * three %s's.                                     
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: The return code equals the return code from the call to
 *          diag_display and therefore indicates what pf key was pressed.
 */

int disp_err(int panel_id, int title_set_id, int title_msg_id, 
         int error_set_id,
         int error_msg_id, char *str1, char *str2, char *str3,int num) {
    char* temp;
    int rc, len = 0;
    ASL_SCR_INFO errinfo[3];
    ASL_SCR_TYPE errtype;
    errtype.max_index = 2;
    errinfo[0].text = diag_cat_gets(cat_fd,title_set_id, title_msg_id);
    if (num > 0) {
        temp = diag_cat_gets(cat_fd, error_set_id, error_msg_id);
        errinfo[1].text = malloc(strlen(temp) +
                                 (str1 ? strlen(str1) : 0) +
                                 (str2 ? strlen(str2) : 0) +
                                 (str3 ? strlen(str3) : 0) + 1);
        if (errinfo[1].text == NULL) genexit(-1);
        switch (num) {
            case 1:
                sprintf(errinfo[1].text,temp, str1);
                break;
            case 2:
                sprintf(errinfo[1].text,temp, str1, str2);
                break;
            case 3:
                sprintf(errinfo[1].text,temp, str1, str2, str3);
                break;
        }
    }
    else { 
        errinfo[1].text = diag_cat_gets(cat_fd, 
                                        error_set_id, error_msg_id);
    }
    errinfo[2].text = diag_cat_gets(cat_fd, 
                                    INSTRUCTIONS, ENTER_TO_CONTINUE);
    rc = diag_display(panel_id, cat_fd, NULL, DIAG_IO, 
                    ASL_DIAG_KEYS_ENTER_SC, &errtype, errinfo);
    if (num > 0) FREE(errinfo[1].text);
    return rc;
} /* end disp_err */

/* ****************************************************************** */
/*
 * FUNCTION:
 * get_lvm_rec reads the primary lvm_rec from the device with the open
 * file descriptor fd.  If that fails, read the backup record, if that
 * fails, return NULL. 
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NULL - failure
 *          Non-NULL - a pointer the LVM record.
 */

struct lvm_rec* get_lvm_rec(int fd, enum open_mode mode) {
    struct lvm_rec* lvm_rec_ptr = malloc (BLOCK_SIZE);
    if (lvm_rec_ptr == NULL) genexit(-1);
    if (get_block(fd,mode,PSN_LVM_REC,1,(char*)lvm_rec_ptr,NULL) == -1){
        if (get_block(fd,mode,PSN_LVM_BAK,1, 
                      (char*)lvm_rec_ptr,NULL) == -1){
            FREE(lvm_rec_ptr);
        }
    }
    return lvm_rec_ptr;
}

/* ****************************************************************** */
/*
 * FUNCTION: Clean up and exit with the specified exit code.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE.
 */

void genexit(int exitcode) {
#ifdef DBG
  fprintf (stderr, "Genexit called, exitcode = %d.\n", exitcode);
#endif
  term_dgodm();
  diag_asl_quit();
  catclose(cat_fd);
  exit(exitcode);
}
 
/* ****************************************************************** */
/*
 * FUNCTION: Mallocs a char string and fills it with the device 
 *           description.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: A pointer to the device description.
 */

char* dev_desc(struct CuDv* device) {
        char* tmp = diag_cat_gets(cat_fd, GENERAL,AT_LOCATION);
	char* device_text = get_dev_desc(device->name);
        char* description_at_location = malloc(strlen(tmp) + 1 +
                                               strlen(device_text) +
                                              strlen(device->location));
        if (tmp == NULL || device_text == NULL ||
                  description_at_location == NULL)      genexit(-1);
        sprintf(description_at_location, tmp,
                        device_text,
                        device->location);

	FREE(device_text);
        return(description_at_location);
}


/* ****************************************************************** */
/*
 * FUNCTION:
 * Display a list of devices on a panel for selection by user and   
 * return the return code from the selection menu.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: The return code from the diag_display of the selection menu.
 */

int select_device(int panel_num, 
                  int title_set,
                  int title_msg,
                  int instruction_set,
                  int instruction_msg,
                  struct CuDv* devlist,
                  int num,
                  int *sel_index ){  
    /* Number of lines to display = num (number of devices)           */
    /* plus one for                                                   */
    /* the title plus one for the instruction line.                   */
    const int dispsize = num+2; /* Number of lines to disp */
    ASL_SCR_INFO* selinfo = calloc(dispsize, sizeof(ASL_SCR_INFO));
    ASL_SCR_TYPE seltype = DM_TYPE_DEFAULTS;
    int i, j, rc;

    if (selinfo == NULL) {
        genexit(-1); /* cleanup and exit with error. */
    }; 

    /* Fill in title and instruction line                         */
    /* then fill in device descriptions.                          */
    selinfo[0].text = diag_cat_gets(cat_fd, title_set, title_msg);
    selinfo[dispsize -1].text = diag_cat_gets(cat_fd,instruction_set,
                                                      instruction_msg);
    for (i=0;i<num;++i){ /* i is index into devlist */
	  selinfo[i+1].text = dev_desc(&devlist[i]);
    } /* end for */
    seltype.max_index = dispsize-1;
    rc = diag_display(panel_num, cat_fd, NULL, DIAG_IO, 
                    ASL_DIAG_LIST_CANCEL_EXIT_SC, &seltype, selinfo);
    switch (rc) {
        case DIAG_ASL_EXIT:
        case DIAG_ASL_CANCEL:
        case DIAG_ASL_COMMIT:
            break;
        default:
#ifdef DBG
            fprintf(stderr,
                 "Unexpected return code from diag_display %d.\n", rc);
#endif
            genexit(-1);
    }
    /* Free the char*'s that were not set by diag_get_cats. */
    for (i=1; i<dispsize-1; ++i) FREE(selinfo[i].text);
    FREE(selinfo); 
    /* Return the index into the device list of the device selected. */
    /* Subtract one from cur_index (the selected line)               */
    *sel_index = seltype.cur_index-1;
    return rc;
} /* end select_device */

/* ****************************************************************** */
/*
 * FUNCTION: Mallocs space and fetches a list of devices from ODM that
 *           match the specified criteria.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: A pointer to the list of devices.
 */

struct CuDv* get_devices (char* criteria , struct listinfo* cudv_info) {
		   
  struct CuDv *cudv_list;

  /* get the number of available devices, from ODM - CuDv objects */
  /* NOTE: get_CuDv_list() defined in /usr/include/sys/cfgodm.h
  ** NOTE: CuDv_Class defined in /usr/include/sys/cfgodm.h
  ** NOTE: this supports a max of 20 devices                       */

  cudv_list = get_CuDv_list(CuDv_CLASS,criteria,
                               cudv_info, 20, 2);
  return cudv_list;
} /* end get_devices */

/* ****************************************************************** */
/*
 * FUNCTION: 
 * Get_block reads the specified number of blocks (num_blocks)        
 * starting with the specified block (block_num) from the specified   
 * file descriptor into the specified buffer.  The buffer size must   
 * be num_blocks * BLOCK_SIZE. The mode parameter determines if the   
 * read is done with a SCSI command or with lseek and read (the       
 * correct value depends on how the file was opened).                 
 * In the SCSI   
 * case, a READ_SENSE is issued if sense_data is non-NULL.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 * A non-zero return code indicates an error occurred.  
 */

int get_block (int fd,    
               enum open_mode mode,
               int block_num,
               int num_blocks,
               char *buffer,
               struct sense_data_t *sense_data) {
  /* This procedure (and put_block) gets called about 10,000 times  */
  /* for a 320MB disk, so performance is important here.           */
  static int rc;
  static struct sc_rdwrt control_block;      /* From scsi.h */
  static int first_call = 1;
  if (mode == scsi) {
      if (first_call) { /* initialize control_block */
          memset(&control_block,0,sizeof(struct sc_rdwrt));
          control_block.timeout_value = 30;
          first_call = 0;
      }
      control_block.req_sense_length = sense_data ? SENSE_LENGTH : 0 ;
      control_block.request_sense_ptr = (char*)sense_data;
      control_block.data_length = num_blocks * BLOCK_SIZE;
      control_block.buffer = buffer;
      control_block.logical_blk_addr = block_num;
      if (sense_data) sense_data->sense_key = 0; /* init to NO_SENSE. */
      /* ioctl automatically does the READ_SENSE if needed. */
      rc = ioctl(fd,DKIORDSE,&control_block);
#ifdef DBG
      if (rc == -1) {
        int i;
        fprintf(stderr, "Error reading drive, errno = %d.\n", errno);
        fprintf(stderr, "  Status_validity = %d.\n", 
                        control_block.status_validity);
        fprintf(stderr, "  SCSI_bus_status = %d.\n", 
                        control_block.scsi_bus_status);
        fprintf(stderr, "  Adapter_status = %d.\n", 
                        control_block.adapter_status);
        if (sense_data) fprintf (stderr, "SENSE data:\n");
        for (i=0; sense_data && (i<SENSE_LENGTH); ++i) {
          fprintf (stderr, "Byte num = %d, value = %02X.\n",
                           i, ((char*)sense_data)[i]);
        } /* end for */
      } /* endif */
#endif
  }
  else {
      if ((rc=lseek(fd, BLOCK_SIZE*block_num, SEEK_SET)) != -1) { 
          if (read(fd, buffer, BLOCK_SIZE*num_blocks) !=
             BLOCK_SIZE*num_blocks)
              rc = -1;
      }
  } /* end if mode */
  return rc;
} /* end get_block */


/* ****************************************************************** */
/*
 * FUNCTION: 
 * Put_block writes blocks to a device. It always uses a SCSI command.
 * The buffer size must be num_blocks*BLOCK_SIZE.                     
 * Sense_data may be null if no sense data is required in the event of 
 * of a failure. 
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 0 - success
 *          -1 - failure
 */

int put_block (int fd,    
               int block_num,
               int num_blocks,
               char *buffer,
               struct sense_data_t *sense_data) {

    static struct sc_iocmd scsi_cmd;
    static struct write_verify_cmd_t *write_verify_cmd_ptr;
    static int i, rc, first_call = 1;

    if (first_call) {
        /* Zero out the whole thing. */
        memset(&scsi_cmd,0,sizeof(struct sc_iocmd));
        /* Now set individual fields. */
        scsi_cmd.timeout_value = 30;
        scsi_cmd.command_length = 10;
        /* Now build the actual SCSI command descriptor block. */
        /* The command block is scsi_cmd.cdb.  "overlay" the   */
        /* WRITE_VERIFY command on that field and fill it in. */

        write_verify_cmd_ptr = 
                      (struct write_verify_cmd_t*) scsi_cmd.scsi_cdb;

        write_verify_cmd_ptr->op_code = SCSI_WRITE_AND_VERIFY;
    }
    scsi_cmd.data_length = BLOCK_SIZE*num_blocks;
    scsi_cmd.buffer = buffer;
    write_verify_cmd_ptr->block_address = block_num;
    write_verify_cmd_ptr->transfer_length = num_blocks;

    rc =  ioctl(fd,DKIOCMD,&scsi_cmd);
    if (rc == -1) {
#ifdef DBG
        fprintf(stderr,"WRITE_AND_VERIFY failed, errno = %d.\n",errno);
        fprintf(stderr,"status_validity = %d, scsi_bus_status = 0x%02X.\n",
                   scsi_cmd.status_validity,scsi_cmd.scsi_bus_status);
        fprintf(stderr,"                      adapter_status = 0x%02X.\n",
                   scsi_cmd.adapter_status);
        if (scsi_cmd.status_validity && 
           (scsi_cmd.scsi_bus_status ==  SC_CHECK_CONDITION)) 
               fprintf (stderr,"  Check_condition received.\n ");
#endif
/*
 * If the sense_data buffer is not null, get sense.
 * If get_sense fails, set the sense_key to 0 so the caller won't
 * try to use the rest of the sense data (0 means "no sense").
 */
        if (sense_data)
            if (get_sense(fd, sense_data)) sense_data->sense_key = 0;
    }
    return rc;
} /* end put_block */

