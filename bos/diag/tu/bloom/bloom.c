static char sccsid[] = "@(#)41  1.3  src/bos/diag/tu/bloom/bloom.c, tu_bloom, bos41J, 9518A_all 4/27/95 13:19:37";
/*
 * COMPONENT_NAME: TU_BLOOM
 *
 * FUNCTIONS: calls exectu
 *
 * ORIGINS: origins
 *
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/devinfo.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/diagex.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "bloomtu.h"
extern int errno;

main(int argc, char **argv) {
  tucb_t tucb;
  uint rc;
  int command;
  char ccom;
  char inp[80];

  if (argc <= 1 ){
     printf("Syntax: %s [adapter_name]\n",argv[0]);
     exit(1);
  } /* endif */

  while (1){
     printf("     Bloomer Test Unit Exerciser       \n");
     printf("0:Open  1:Diag  2:Scsi  3:Close  4:rcfg  5:Se/Diff q:Exit\n");
     printf("Enter Test Unit Number:");
     scanf("%s",inp);
     sscanf(inp,"%c",&ccom);
     if (ccom=='q') break;
     sscanf(inp,"%d",&tucb.tu);
     switch (tucb.tu) {
     case BLOOM_INIT_ATU:
       strcpy(tucb.name,argv[1]);
       break;
     case BLOOM_DIAG_ATU:
       break;
     case BLOOM_SCSI_ATU:
       break;
     case BLOOM_TERM_ATU:
       break;
     case BLOOM_RCFG_ATU:
       break;
     case BLOOM_SEDIFF_ATU:
       break;
     default:
       break;
     } /* endswitch */
     rc=exectu(&tucb); 
  } /* endwhile */
}
