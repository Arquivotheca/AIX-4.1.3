static char sccsid[] = "@(#)40  1.3  src/bos/diag/tu/bad/badmain.c, tu_bad, bos411, 9428A410j 6/11/91 14:59:17";
/*
 * COMPONENT_NAME: (tu_bad) bus attach hardfile tu's
 *
 * FUNCTIONS: calls exectu
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990
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
#include <stdio.h>
#include <sys/badisk.h>
#include "badatu.h"
extern int errno;

main(argc, argv)
int argc;
char *argv[];
{
  struct bad_tucb_t bad_tucb;
  int rc;
  unsigned char drive_type[20];
  int fildes,i;

  if (argc > 1 ){
     fildes = openx(argv[1], O_RDWR,0,1);
     printf("Fildes: %d Errno No: %d\n",fildes,errno);
  } /* endif */

  while (1){
    printf("          KAZUSA TEST UNIT EXERCISER           \n");
    printf(" 0:Self Test  1:Buf Wr Rd Test   2:Rd Wr Test  \n");
    printf(" 3:Seek Test  4:Rd Verify Test   5:KAZUSA Info \n");
    printf(" 6:Mfg Rd Verify Test    7: Format the Drive   \n");
    printf("20:Open a file                   99:Exit Tests \n");
    printf("Enter Test Unit Number:");
    scanf("%d",&bad_tucb.tucb.tu);
    if (bad_tucb.tucb.tu == 99){
      break;
    } /* endif */
    if (bad_tucb.tucb.tu == 20){
      printf("Enter Drive type e.g. /dev/hdisk0 :");
      scanf("%s",drive_type);
      fildes = openx(drive_type,O_RDWR,0,1);
      printf("Fildes: %d Errno No: %d\n",fildes,errno);
      continue;
    } /* endif */
    if (bad_tucb.tucb.tu == BUFF_WR_CMP_TEST ) {
      printf("Pattern of form 4 bytes in Hex:");
      scanf("%X",&bad_tucb.ip1);
      printf("Enter Number of 512 blocks in Decimal 2 Max:");
      scanf("%d",&bad_tucb.ip2);
    } /* endif */
    if (bad_tucb.tucb.tu == MFG_RD_VERIFY_TEST ) {
      printf("Starting block address in decimal:");
      scanf("%d",&bad_tucb.ip1);
      printf("Number Of 512 byte blocks to verify in decimal:");
      scanf("%d",&bad_tucb.ip2);
    } /* endif */
    printf("Enter Loop Count Number:");
    scanf("%d",&bad_tucb.tucb.loop);
    rc = exectu((long)fildes,&bad_tucb);
    printf("Return Code: %2d  Error No: %2d \n",rc,errno);
    printf("tu: %2d ",bad_tucb.tucb.tu);
    printf("mfg: %2d ",bad_tucb.tucb.mfg);
    printf("loop: %2d ",bad_tucb.tucb.loop);
    printf("ip1: %8X ",bad_tucb.ip1);
    printf("ip2: %2d ",bad_tucb.ip2);
    printf("\n");
    printf("Sec Size: %4d ",bad_tucb.bad_devinfo.un.dk.bytpsec);
    printf("Sec/Trk: %4d ",bad_tucb.bad_devinfo.un.dk.secptrk);
    printf("Trk/Cyl: %4d ",bad_tucb.bad_devinfo.un.dk.trkpcyl);
    printf("NumBlks: %4d ",bad_tucb.bad_devinfo.un.dk.numblks);
    printf("\n");
    printf("Cmd Stat: %2X ",bad_tucb.bad_diag_stat.cmd_status);
    printf("Cmd Err: %2X ",bad_tucb.bad_diag_stat.cmd_error);
    printf("Dev Stat: %2X ",bad_tucb.bad_diag_stat.dev_status);
    printf("Dev Err: %2X ",bad_tucb.bad_diag_stat.dev_error);
    printf("Pwron Err: %2X ",bad_tucb.bad_diag_stat.pwron_errcode);
    printf("Test Err: %2X ",bad_tucb.bad_diag_stat.test_errcode);
    printf("Diag Cmd: %4X ",bad_tucb.bad_diag_stat.diag_cmd);
    printf("Soft Error: %4d ",bad_tucb.bad_rd_verify_stat.soft_error);
    printf("Seek Error: %4d ",bad_tucb.bad_rd_verify_stat.seek_error);
    printf("Equipment Check: %4d ",bad_tucb.bad_rd_verify_stat.eqp_check_error);
    printf("\n");
  } /* endwhile */
  close(fildes);
}
