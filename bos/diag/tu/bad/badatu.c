static char sccsid[] = "@(#)35  1.3  src/bos/diag/tu/bad/badatu.c, tu_bad, bos411, 9428A410j 6/11/91 14:59:32";
/*
 * COMPONENT_NAME: tu_bad
 *
 * FUNCTIONS: exectu
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/devinfo.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/badisk.h>
#include "badatu.h"
extern int errno;
/*
 * NAME: exectu
 *
 * FUNCTION: executes kazusa test units
 *
 * EXECUTION ENVIRONMENT:
 *
 *      These tu's are called by diagnostics or mfg design applications
 *
 * (NOTES:) tucb_ptr points to the tu to be executed and status is stored
 *      in data structure pointed to by tucb_ptr.
 *
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS: 0 if no error else error as defined in the document.
 */

int exectu (fildes,tucb_ptr)
  int fildes;
  struct bad_tucb_t *tucb_ptr;
{
  int error,rc,i,j,bad_error;
  unsigned long left_to_read,beg_rba,rba_offset;
  unsigned long buffer[BUFF_SIZE/4];
  struct badisk_ioctl_parms bad_ioctl_parms;
  struct badisk_spec bad_spec;
  struct badisk_cstat_blk bad_cstat_blk;
  unsigned char read_buffer[RBA_OFFSET];

  error = 0;
  for(rc=0;(error!=-1&&rc==0&&tucb_ptr->tucb.loop>0);tucb_ptr->tucb.loop--){
/*printf("loop:%d ",tucb_ptr->tucb.loop); */

     switch (tucb_ptr->tucb.tu) {
       case DIAG_SELF_TEST:
         bad_ioctl_parms.diag_test = BADIAG_SELFTEST;
         if ((error = ioctl(fildes,BADIAGTEST,&bad_ioctl_parms)) == -1) break;
         bad_ioctl_parms.milli_secs = SELF_TEST_TIME;
         if ((error = ioctl(fildes,BAWAITCC,&bad_ioctl_parms)) == -1){
            rc=2;
            error = ioctl(fildes,BAABORT,NULL) == -1;
            break;
         } /* endif */
         break;
       case BUFF_WR_CMP_TEST:
         for (i=0; i < (BUFF_SIZE/4); i++){
             buffer[i] = tucb_ptr->ip1;
         } /* endfor */
         bad_ioctl_parms.num_blks = tucb_ptr->ip2;
         bad_ioctl_parms.buff_address=(uint)&buffer[0];
         if ((error = ioctl(fildes,BABUFFW,&bad_ioctl_parms)) == -1) break;
         for (i=0; i < (BUFF_SIZE/4); i++){
             buffer[i] = tucb_ptr->ip1 ^ 0xffffffff;
         } /* endfor */
         bad_ioctl_parms.milli_secs = WR_BUF_TIME;
         if ((error = ioctl(fildes,BAWAITCC,&bad_ioctl_parms)) == -1){
            rc=2;
            error = ioctl(fildes,BAABORT,NULL) == -1;
            break;
         } /* endif */
         bad_ioctl_parms.num_blks = tucb_ptr->ip2;
         bad_ioctl_parms.buff_address=(uint)&buffer[0];
         if ((error = ioctl(fildes,BABUFFR,&bad_ioctl_parms)) == -1) break;
         bad_ioctl_parms.milli_secs = RD_BUF_TIME;
         if ((error = ioctl(fildes,BAWAITCC,&bad_ioctl_parms)) == -1){
            rc=2;
            error = ioctl(fildes,BAABORT,NULL) == -1;
            break;
         } /* endif */
         for (i=0; i < (tucb_ptr->ip2*512/4); i++){
             if (buffer[i] != tucb_ptr->ip1) {
                rc = 80;
                break;
             } /* endif */
         }
         break;
       case DIAG_RW_TEST:
         bad_ioctl_parms.diag_test = BADIAG_RDWR;
         if ((error = ioctl(fildes,BADIAGTEST,&bad_ioctl_parms)) == -1) break;
         bad_ioctl_parms.milli_secs = RW_TIME;
         if ((error = ioctl(fildes,BAWAITCC,&bad_ioctl_parms)) == -1){
            rc=2;
            error = ioctl(fildes,BAABORT,NULL) == -1;
            break;
         } /* endif */
         break;
       case DIAG_SEEK_TEST:
         bad_ioctl_parms.diag_test = BADIAG_SEEK;
         if ((error = ioctl(fildes,BADIAGTEST,&bad_ioctl_parms)) == -1) break;
         bad_ioctl_parms.milli_secs = SEEK_TIME;
         if ((error = ioctl(fildes,BAWAITCC,&bad_ioctl_parms)) == -1){
            rc=2;
            error = ioctl(fildes,BAABORT,NULL) == -1;
            break;
         } /* endif */
         break;
       case DIAG_RD_VERIFY_TEST:
         bad_ioctl_parms.diag_test = BADIAG_READV;
         if ((error = ioctl(fildes,BADIAGTEST,&bad_ioctl_parms)) == -1) break;
         bad_ioctl_parms.milli_secs = RD_VERIFY_TIME;
         if ((error = ioctl(fildes,BAWAITCC,&bad_ioctl_parms)) == -1){
            rc=2;
            error = ioctl(fildes,BAABORT,NULL) == -1;
            break;
         } /* endif */
         break;
       case BAD_INFO_TEST:
         error = ioctl(fildes,IOCINFO,&(tucb_ptr->bad_devinfo));
         break;
       case MFG_RD_VERIFY_TEST:
         tucb_ptr->bad_rd_verify_stat.soft_error=0;
         tucb_ptr->bad_rd_verify_stat.seek_error=0;
         tucb_ptr->bad_rd_verify_stat.eqp_check_error=0;
         rba_offset=RBA_OFFSET;
         tucb_ptr->ip1 *=512;
         left_to_read = tucb_ptr->ip2 * 512;
         for (beg_rba=tucb_ptr->ip1;(left_to_read > 0);){
             if ((error = lseek (fildes,beg_rba,SEEK_SET)) == -1) break;
             bad_ioctl_parms.milli_secs = MFG_RD_VERIFY_TIME;
             if ((error = ioctl(fildes,BAWAITCC,&bad_ioctl_parms)) == -1){
                rc=2;
                error = ioctl(fildes,BAABORT,NULL) == -1;
                break;
             } /* endif */
             if (left_to_read < RBA_OFFSET) rba_offset = left_to_read;
/*printf("BEG_RBA = %d ",beg_rba);
printf("RBA_OFFSET = %d ",rba_offset);
printf("tucb_ptr->ip2 = %d ",tucb_ptr->ip2);*/
             bad_error = 0;
             error = read (fildes,read_buffer,rba_offset);
             bad_ioctl_parms.milli_secs = MFG_RD_VERIFY_TIME;
             if ((error = ioctl(fildes,BAWAITCC,&bad_ioctl_parms)) == -1){
                rc=2;
                error = ioctl(fildes,BAABORT,NULL) == -1;
                break;
             } /* endif */
             if ((error = ioctl(fildes,BACCSTAT,&bad_cstat_blk))
                        == -1) return(error);
/*printf("msw = %X ",bad_cstat_blk.msw_lastrba);
printf("lsw = %X\n",bad_cstat_blk.lsw_lastrba);*/
             if ((bad_cstat_blk.cmd_error == 6) |
                 (bad_cstat_blk.cmd_error == 8) |
                 (bad_cstat_blk.cmd_error == 9) |
                 (bad_cstat_blk.cmd_error == 10) |
                 (bad_cstat_blk.cmd_error == 15) |
                 (bad_cstat_blk.cmd_error == 16) |
                 (bad_cstat_blk.dev_status & 04) |
                 (bad_cstat_blk.dev_error == 13) |
                 (bad_cstat_blk.dev_error == 14) |
                 (bad_cstat_blk.dev_error == 15) |
                 (bad_cstat_blk.dev_error == 16) |
                 (bad_cstat_blk.dev_error == 17))  { /* Hard Equipment Check */
                rc=3;
                break;
             } /* endif */
             if (bad_cstat_blk.dev_error == 1){  /* Seek Error */
                tucb_ptr->bad_rd_verify_stat.seek_error++;
                bad_error = 1;
                if (tucb_ptr->bad_rd_verify_stat.seek_error == 2){
                   rc=5;
                   break;
                } /* endif */
             } /* endif */
             if ((bad_cstat_blk.cmd_status == 3) |
                 (bad_cstat_blk.cmd_status == 5) |
                 (bad_cstat_blk.cmd_status == 7))  { /* Soft error */
                tucb_ptr->bad_rd_verify_stat.soft_error++;
             } /* endif */
             if ((bad_cstat_blk.cmd_error == 4) |
                 (bad_cstat_blk.cmd_error == 11)) { /* Soft Equipment Check */
                tucb_ptr->bad_rd_verify_stat.eqp_check_error++;
                bad_error = 1;
                if (tucb_ptr->bad_rd_verify_stat.eqp_check_error == 2){
                   rc=6;
                   break;
                } /* endif */
             } /* endif */
             if ((bad_cstat_blk.dev_error == 3) |
                 (bad_cstat_blk.dev_error == 4) |
                 (bad_cstat_blk.dev_error == 5) |
                 (bad_cstat_blk.dev_error == 6) |
                 (bad_cstat_blk.dev_error == 20) |
                 (bad_cstat_blk.dev_error == 21) |
                 (bad_cstat_blk.dev_error == 24)) { /* Hard Read Errors */
                rc=4;
                break;
             } /* endif */
             rba_offset=(((bad_cstat_blk.msw_lastrba << 16)+bad_cstat_blk.lsw_lastrba)*(512.0))-beg_rba+512;
             beg_rba=(((bad_cstat_blk.msw_lastrba << 16)+bad_cstat_blk.lsw_lastrba)*(512.0));
             if (!bad_error) beg_rba+=512;
             left_to_read -= rba_offset;
         } /* endfor */
/*printf("Soft_error=%d\n",tucb_ptr->bad_rd_verify_stat.soft_error); */

         break;
      case FORMAT_TEST:
         if ((error = ioctl(fildes,BADEVCFIG,&bad_spec)) == -1) break;
         bad_ioctl_parms.format_options = BAISDM | BAUSDM | BAPESA | BAFUPI;
         if ((error = ioctl(fildes,BAFORMAT,&bad_ioctl_parms)) == -1) break;
         do {
            if ((error = ioctl(fildes,BAFMTPROG,&bad_ioctl_parms)) == -1) break;
            sleep(1);
            printf ("Formatting Cylinder: %d \r", bad_ioctl_parms.curr_cylinder);
         } while (bad_ioctl_parms.curr_cylinder < bad_spec.cylinders); /* enddo */
         break;
       default:
         rc = 255;
     } /* endswitch */
  } /* endfor */
  if (error == -1) return(error);
  if (rc != 0) {
/*     printf("deverror = %X ",bad_cstat_blk.dev_error);
     printf("cmderror = %X ",bad_cstat_blk.cmd_error);
     printf("cmdstatus = %X ",bad_cstat_blk.cmd_status);
     printf("devstatus = %X\n ",bad_cstat_blk.dev_status);*/
     return(rc);
  } /* endif */
  if ((error = ioctl(fildes,BADIAGSTAT,&(tucb_ptr->bad_diag_stat)))
      == -1) return(error);
  rc = 1;
  if ((tucb_ptr->bad_diag_stat.cmd_error == 0) &
      (tucb_ptr->bad_diag_stat.dev_error == 0) &
      (tucb_ptr->bad_diag_stat.pwron_errcode == 0) &
      (tucb_ptr->bad_diag_stat.test_errcode == 0) ) rc = 0;
  return(rc);
}
