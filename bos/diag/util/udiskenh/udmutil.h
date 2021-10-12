/* @(#)50	1.2  src/bos/diag/util/udiskenh/udmutil.h, dsaudiskenh, bos411, 9435A411a 8/24/94 16:47:43 */
/*
 *   COMPONENT_NAME: DSAUDISKMNT
 *
 *   FUNCTIONS: FREE
 *		LESSER
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
#ifndef UDMUTIL
#define UDMUTIL
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <locale.h>
#include <diag/da.h>
#include <diag/diago.h>
#include <diag/diag.h>
#include <diag/diag_exit.h>
#include <sys/signal.h>
#include <sys/errno.h>
#include <sys/cfgodm.h>    /* CuDv class def */
#include <sys/scdisk.h>
#include "udiskmnt_msg.h"
#define BLOCK_SIZE   512
#define SENSE_LENGTH sizeof(struct sense_data_t)
#define FREE(x) cond_free((void*)&x)
#define LESSER(a,b) (a) < (b) ? (a):(b) 
/* #define DBG     uncomment this to turn on debug. */

extern int errno;

#pragma options align=packed
  
  struct reassign_cmd_t {
    char op_code;
    unsigned lun       : 3;
    unsigned reserved1 : 5;
    char     reserved2[4]; } ;

  struct defect_list {
    short reserved;
    short length; /* Always set this to 4. */
    long  block_num; };
    
  struct request_sense_cmd_t {
    char op_code;
    unsigned lun : 3;
    unsigned reserved1 : 5;
    short reserved2;
    char alloc_len;
    char control; } ;

 struct write_verify_cmd_t {
   char op_code;
   unsigned lun         : 3;
   unsigned DPO         : 1; /* Part of the reserved bits in SCSI-1. */
   unsigned reserved1   : 2;
   unsigned BytChk      : 1;
   unsigned rel_adr     : 1;
   long     block_address;
   char     reserved2;
   unsigned short transfer_length;
   char     control;};

struct read_capacity_cmd_t {
   char op_code;
   unsigned lun       : 3;
   unsigned reserved1 : 4;
   unsigned rel_adr   : 1;
   long block_address;
   short reserved2;
   unsigned vendor_unique1 : 2;
   unsigned reserved3 : 5;
   unsigned pmi       : 1;
   unsigned vendor_unique2 : 2;
   unsigned reserved4 : 4;
   unsigned flag      : 1;
   unsigned link      : 1; };

 struct read_capacity_data_t {
   long block_address;
   long block_size; };

 struct sense_data_t {
   unsigned valid      : 1;
   unsigned error_code : 7;
   char     segment;
   unsigned reserved1  : 4;
   unsigned sense_key  : 4;
   long     information;
   char     additional_sense_length;
   long     cmd_specific_info;
   char     sense_code;
   char     junk[5]; };

#pragma options align=reset

enum open_mode {shared, /* use "read", "write" for I/O. */ 
                scsi};  /* use SCSI commands for I/O. */

/* The menu numbers assigned to this SA are 802320 - 802329 and      */
/* 802650 - 802699. */
    /* Using an enum automatically */
    /* numbers the ids. However,   */
    /* The number for every 10th   */
    /* menu must be specified or   */
    /* alphas will appear on screen*/

enum panel_ids {
  MAIN_MEN = 0x802320,
  DISK_COPY_INFO,
  DISK_COPY_SOURCE,
  DISK_COPY_TARGET,
  DISK_COPY_CONF,
  DISK_COPY_PROGRESS,
  DISK_COPY_COMPLETE,
  DISK_COPY_WRITE_ERROR,
  DISK_COPY_READ_ERROR,
  DISK_COPY_TARGET_WONT_OPEN,
  DISK_COPY_TARGET_IN_LVM     = 0x802650,
  DISK_COPY_SOURCE_WONT_OPEN_EACCESS,
  DISK_COPY_NOT_SAME_SIZE,
  DISK_COPY_LESS_THAN_TWO_DISKS,
  DISK_COPY_SOURCE_WONT_OPEN_OTHER,
  DISK_COPY_BBDIR,
  DISP_ALTER_INFO_MENU        = 0x802660, /* Allow more "DISK_COPY" */
  DISP_ALTER_DISK,
  DISP_ALTER_SECTOR,
  DISP_ALTER_EDIT,
  DISP_ALTER_CONF,
  DISP_ALTER_NO_DISKS,
  DISP_ALTER_DISK_WONT_OPEN,
  DISP_ALTER_READ_ERR,
  DISP_ALTER_WRITE_ERR
};

/* Function cond_free (conditional free) calls free(x) only if x is not     */
/* null and sets x to null so it won't be freed again.  This is much easier */
/* than keeping track of which pointers have or haven't been freed when it's*/
/* time to clean up. */
void cond_free (void **x);

/* Function diag_catopen is not declared in any of the diag header files,   */
/* so it is declared here along with diag_cag_gets to avoid compiler errors.*/
nl_catd diag_catopen(char*, int);
char* diag_cat_gets(nl_catd, short, short);

/* Function dev_desc returns the description of the device, does malloc */
char* dev_desc(struct CuDv* device) ;

/* Function disk_path constructs the pathname of the disk special file from */
/* the directory name and disk name.  An "r" is prepended to the disk name  */
/* if necessary. */
char *disk_path(char *dir, char *name) ;

/* Function get_sense requests sense data */
/* It returns 0 if it succeeds, -1 if it fails. */
extern int get_sense(int fd, struct sense_data_t *sense_data) ;

#ifdef DBG
/* Function get_and_print_sense requests sense data and prints it */
/* to stderr. */
extern void get_and_print_sense(int fd) ;
#endif

/* Reassign_block issues a SCSI command to the file descriptor fd to */
/* reassign the specified logical block to a new physical location.  */
extern int reassign_block (int fd, int block_num);

/* Read_capacity reads the capacity in logical blocks and block    */
/* length into the data argument. */
extern int read_capacity (int fd, /* File desc of device. */
                   struct read_capacity_data_t* data); /* the output*/

/* disp_err displays a menu with 3 message         */
/* lines from the nl_cat catalog.                  */
/* Last line is always "press enter to continue".  */
/* Title and error message                         */
/* are as specified, and up to 3 text strings may be */
/* substituted into the error                      */
/* message up to three times.                      */
/* That is, the error message may have up to       */
/* three %s's.                                     */
extern int disp_err(int panel_id,     /* Panel id for upper right corner. */
             int title_set_id, /* Set id for title message. */
             int title_msg_id, /* Msg id for title message. */
             int error_set_id, /* Set id for error message. */
             int error_msg_id, /* Msg id for error message. */
             char* str1, /* String to substitute into error msg.*/
             char* str2,
             char* str3,
             int num); /* Number of strings to substitute, max 3. */

/* Cleanup and exit program with specified exitcode. */
extern void genexit(int exitcode);

/* Put_block writes blocks to a device. It always uses a SCSI command.*/
/* The buffer size must be num_blocks*BLOCK_SIZE.                     */
/* Sense_data may be null if no sense data is required in the event of */
/* of a failure. */
extern int put_block (int fd,
               int block_num,
               int num_blocks,
               char *buffer,
               struct sense_data_t *sense_data);


/* Display a list of devices on a panel for selection by user and   */
/* return the return code from the selection menu.                  */
extern int select_device(int panel_num, /* The panel id num in
                                    upper right corner. */
                  int title_set, /* 'Set' id for title line. */
                  int title_msg, /* Msg id for title line.   */
                  int instruction_set, /* 'Set' id for
                                          instruction line. */
                  int instruction_msg, /* Msg id for instruction line.*/
                  struct CuDv* devlist, /* Pointer to list of devices.*/
                  int num,              /* Number of devices.         */
                  int* sel_index);  /* Index of selected device.      */

/* Obtain the list of devtype devices from ODM.                     */
/* Return a pointer to the list.                                    */
extern struct CuDv* get_devices (char* criteria,  /* selection criteria. */
                          struct listinfo* cudv_info); 

/* get_lvm_rec reads the primary lvm_rec from the device with the open*/
/* file descriptor fd.  If that fails, read the backup record, if that*/
/* fails, return NULL. */
extern struct lvm_rec* get_lvm_rec(int fd, enum open_mode mode) ;

/* Get_block reads the specified number of blocks (num_blocks)        */
/* starting with the specified block (block_num) from the specified   */
/* file descriptor into the specified buffer.  The buffer size must   */
/* be num_blocks * BLOCK_SIZE. The mode parameter determines if the   */
/* read is done with a SCSI command or with lseek and read (the       */
/* correct value depends on how the file was opened).                 */
/* A non-zero return code indicates an error occurred.  In the SCSI   */
/* case, a READ_SENSE is issued if sense_data is non-NULL.            */
extern int get_block (int fd, /* The file descriptor for the device.  */
               enum open_mode mode, /* Use SCSI command or not.       */
               int block_num, /* The logical block number to read.    */
               int num_blocks,
               char *buffer,
               struct sense_data_t *sense_data); /* sense_data may be null */

#endif
