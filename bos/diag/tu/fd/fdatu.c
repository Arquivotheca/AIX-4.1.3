static char sccsid[] = "@(#)46  1.5  src/bos/diag/tu/fd/fdatu.c, tu_fd, bos411, 9428A410j 10/7/91 13:24:16";
/*
 * COMPONENT_NAME: (niodskt) NIO diskette tu's
 *
 * FUNCTIONS: exectu
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1990, 1991
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
#include <sys/fd.h>
#include "fdatu.h"
extern int errno;

union {
    char data_char[LAST_SECTOR * SECTOR_SIZE * 2];
    unsigned long data_long[(LAST_SECTOR * SECTOR_SIZE * 2)/4];
} data_char_long;

/*
 * NAME: exectu
 *
 * FUNCTION: executes diskette test units
 *
 * EXECUTION ENVIRONMENT:
 *
 *      These tu's are called by diagnostics or mfg design applications
 *
 * (NOTES:) tucp_ptr points to the tu to be executed and status is stored
 *      in data structure pointed to by tucb_ptr.
 *
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS: 0 if no error else error as defined in the document.
 */

int exectu (fildes,tucb_ptr)
  long fildes;
  struct diskette_tucb_t *tucb_ptr;
{
  int error,rc,i,j,no_of_heads,no_of_sectors;
  int no_of_cylinders,logical_sector,current_cylinder;
  int no_of_blocks,sectors_per_cylinder;
  int motor_speed,motor_speed_offset;
  struct fdinfo diskette_fdinfo;

  error = 0;
  if (tucb_ptr->tucb.tu == STEP_TEST){
     if ((error = ioctl(fildes,FDIOCREADID,tucb_ptr->ip2)) == 0) {
        if((error=ioctl(fildes,FDIOCSTATUS,&(tucb_ptr->diskette_status)))==0){
            current_cylinder = tucb_ptr->diskette_status.cylinder_num;
        }
     }
  } /* endif */
  for(rc=0;(error!=-1&&rc==0&&tucb_ptr->tucb.loop>0);tucb_ptr->tucb.loop--){
     switch (tucb_ptr->tucb.tu) {
       case ADAPTER_TEST:
         error = ioctl(fildes,FDIOCRESET);
         break;
       case SELECT_TEST:
         error = ioctl(fildes,FDIOCSELDRV);
         break;
       case DESELECT_TEST:
         error = ioctl(fildes,FDIOCDSELDRV);
         break;
       case RECALIB_TEST:
         if ((error = ioctl(fildes,FDIOCRECAL)) == -1) break;
         if ((error = ioctl(fildes,FDIOCSTATUS,&(tucb_ptr->diskette_status)))
             == -1) break;
         if (((tucb_ptr->diskette_status.result3)&TRACK00) == 0) {
            rc=2;
            break;
         }
         if ((error = ioctl(fildes,FDIOCSEEK,1)) == -1) break;
         if ((error = ioctl(fildes,FDIOCSTATUS,&(tucb_ptr->diskette_status)))
             == -1) break;
         if (((tucb_ptr->diskette_status.result3)&TRACK00) != 0) rc=80;
         break;
       case DISK_CHANGE_TEST:
         if ((error = ioctl(fildes,FDIOCSELDRV)) == -1) break;
         if ((error = ioctl(fildes,FDIOCSTATUS,&(tucb_ptr->diskette_status)))
             == -1) break;
         if (!((tucb_ptr->diskette_status.dsktchng) & DISK_CHANGED)){
             rc=88;
             break;
         } /* endif */
         if ((error = ioctl(fildes,FDIOCRECAL)) == -1) break;
         if ((error = ioctl(fildes,FDIOCSEEK,1)) == -1) break;
         if ((error = ioctl(fildes,FDIOCSTATUS,&(tucb_ptr->diskette_status)))
             == -1) break;
         if ((tucb_ptr->diskette_status.dsktchng) & DISK_CHANGED) rc=81;
         break;
       case DISK_WR_PROT_TEST:
         if ((error = ioctl(fildes,FDIOCSELDRV)) == -1) break;
         if ((error = ioctl(fildes,FDIOCSTATUS,&(tucb_ptr->diskette_status)))
             == -1) break;
         if ((tucb_ptr->diskette_status.result3) & WRITE_PROTECT) rc=82;
         break;
       case INDEX_TEST:
         for (i =0; i < (LAST_SECTOR); i++) {
             data_char_long.data_char[i*4] = tucb_ptr->ip1;
             data_char_long.data_char[i*4+1]= 0;
             data_char_long.data_char[i*4+2]= i+1;
             data_char_long.data_char[i*4+3]= SIZE;
         } /* endfor */
         error = ioctl(fildes,FDIOCFORMAT,(data_char_long.data_char));
         break;
       case STEP_TEST:
         if ((error = ioctl(fildes,FDIOCSEEK,current_cylinder)) == -1) break;
         if ((error = ioctl(fildes,FDIOCREADID,tucb_ptr->ip2))
             == -1) break;
         if ((error = ioctl(fildes,FDIOCSTATUS,&(tucb_ptr->diskette_status)))
             == -1) break;
         if (tucb_ptr->diskette_status.cylinder_num != current_cylinder)rc=83;
         if ((error = ioctl(fildes,FDIOCSEEK,tucb_ptr->ip1)) == -1) break;
         if ((error = ioctl(fildes,FDIOCREADID,tucb_ptr->ip2))
             == -1) break;
         if ((error = ioctl(fildes,FDIOCSTATUS,&(tucb_ptr->diskette_status)))
             == -1) break;
         if (tucb_ptr->diskette_status.cylinder_num != tucb_ptr->ip1) rc = 83;
         break;
       case READ_TEST:
         if ((error = ioctl(fildes,FDIOCSELDRV)) == -1) break;
         if ((error = ioctl(fildes,FDIOCSTATUS,&(tucb_ptr->diskette_status)))
              == -1) break;
         no_of_heads = 1;
         if ((tucb_ptr->diskette_status.status3)&FDDOUBLE) no_of_heads = 2;
         no_of_sectors = 36;
         if ((tucb_ptr->diskette_status.status3)&FD8PRTRCK) no_of_sectors=8;
         if ((tucb_ptr->diskette_status.status3)&FD9PRTRCK) no_of_sectors=9;
         if ((tucb_ptr->diskette_status.status3)&FD15PRTRCK) no_of_sectors=15;
         if ((tucb_ptr->diskette_status.status3)&FD18PRTRCK) no_of_sectors=18;
         logical_sector=(((tucb_ptr->ip1)*no_of_sectors*no_of_heads)+
                         ((tucb_ptr->ip2)*no_of_sectors)+
                         ((tucb_ptr->ip3)-1))*SECTOR_SIZE;
         if ((error = lseek (fildes,logical_sector,SEEK_SET)) == -1) break;
         if ((error = read (fildes,(tucb_ptr->datacl.datac),SECTOR_SIZE))
             == -1) break;
         break;
       case WRITE_TEST:
         if ((error = ioctl(fildes,FDIOCSELDRV)) == -1) break;
         if ((error = ioctl(fildes,FDIOCSTATUS,&(tucb_ptr->diskette_status)))
              == -1) break;
         no_of_heads = 1;
         if ((tucb_ptr->diskette_status.status3)&FDDOUBLE) no_of_heads = 2;
         no_of_sectors = 36;
         if ((tucb_ptr->diskette_status.status3)&FD8PRTRCK) no_of_sectors=8;
         if ((tucb_ptr->diskette_status.status3)&FD9PRTRCK) no_of_sectors=9;
         if ((tucb_ptr->diskette_status.status3)&FD15PRTRCK) no_of_sectors=15;
         if ((tucb_ptr->diskette_status.status3)&FD18PRTRCK) no_of_sectors=18;
         logical_sector=(((tucb_ptr->ip1)*no_of_sectors*no_of_heads)+
                         ((tucb_ptr->ip2)*no_of_sectors)+
                         ((tucb_ptr->ip3)-1))*SECTOR_SIZE;
         if ((error = lseek (fildes,logical_sector,SEEK_SET)) == -1) break;
         for (i=0; i < SECTOR_SIZE/4; i++){
             data_char_long.data_long[i] = tucb_ptr->ip4;
         } /* endfor */
         if ((error = write (fildes,(data_char_long.data_char),SECTOR_SIZE))
             == -1) break;
         break;
       case WR_READ_CMP_TEST:
         if ((error = ioctl(fildes,FDIOCSELDRV)) == -1) break;
         if ((error = ioctl(fildes,FDIOCSTATUS,&(tucb_ptr->diskette_status)))
              == -1) break;
         no_of_heads = 1;
         if ((tucb_ptr->diskette_status.status3)&FDDOUBLE) no_of_heads = 2;
         no_of_sectors = 36;
         if ((tucb_ptr->diskette_status.status3)&FD8PRTRCK) no_of_sectors=8;
         if ((tucb_ptr->diskette_status.status3)&FD9PRTRCK) no_of_sectors=9;
         if ((tucb_ptr->diskette_status.status3)&FD15PRTRCK) no_of_sectors=15;
         if ((tucb_ptr->diskette_status.status3)&FD18PRTRCK) no_of_sectors=18;
         logical_sector=((tucb_ptr->ip1)*no_of_sectors*no_of_heads)*SECTOR_SIZE;
         if ((error = lseek (fildes,logical_sector,SEEK_SET)) == -1) break;
         for (i=0; i < (no_of_sectors * 2 * SECTOR_SIZE/4); i++){
             data_char_long.data_long[i] = tucb_ptr->ip2;
         } /* endfor */
         if ((error=write(fildes,(data_char_long.data_char),
                          (no_of_sectors*SECTOR_SIZE*2))) == -1) break;
         for (j=0;j < tucb_ptr->ip3;j++){
            if ((error = lseek (fildes,logical_sector,SEEK_SET)) == -1) break;
            for (i=0; i < (no_of_sectors * 2 * SECTOR_SIZE/4); i++){
                data_char_long.data_long[i] = tucb_ptr->ip2 ^ 0xffffffff;
            } /* endfor */
            if ((error=read(fildes,(data_char_long.data_char),
                            (no_of_sectors*SECTOR_SIZE*2))) == -1) break;
            for (i=0; i < (no_of_sectors * 2 * SECTOR_SIZE/4); i++){
                if (data_char_long.data_long[i] != tucb_ptr->ip2) {
                   rc = 84;
                   break;
                } /* endif */
            } /* endfor */
            if (rc == 84){
               break;
            } /* endif */
         } /* endfor */
         break;
       case LDSTY_1MB_TEST:
         if ((error = ioctl(fildes,FDIOCRECAL)) == -1) break;
         if ((error = ioctl(fildes,FDIOCGETPARMS,&(tucb_ptr->diskette_parms)))
              == -1) break;
         /*
          * The value used next is just an arbitrary value larger than the
          * real value.
          */
         no_of_sectors= tucb_ptr->diskette_parms.sectors_per_track;
         no_of_heads=tucb_ptr->diskette_parms.sectors_per_cylinder/
                     tucb_ptr->diskette_parms.sectors_per_track;
         no_of_blocks=tucb_ptr->diskette_parms.number_of_blocks;
         tucb_ptr->diskette_parms.sectors_per_track = 10;
         tucb_ptr->diskette_parms.sectors_per_cylinder = tucb_ptr->
            diskette_parms.sectors_per_track * no_of_heads;
         tucb_ptr->diskette_parms.number_of_blocks = tucb_ptr->diskette_parms.
            cylinders_per_disk * tucb_ptr->diskette_parms.sectors_per_cylinder;
         if ((error = ioctl(fildes,FDIOCSETPARMS,&(tucb_ptr->diskette_parms)))
              == -1) break;
         logical_sector=((tucb_ptr->ip1)*(tucb_ptr->diskette_parms.
             sectors_per_track)*no_of_heads+9)*SECTOR_SIZE;
         if ((error = lseek (fildes,logical_sector,SEEK_SET)) == -1) break;
         rc = 85 ;
         error = read (fildes,(tucb_ptr->datacl.datac),SECTOR_SIZE);
         if (error == -1) {
            rc = 86;
            if ((error = ioctl(fildes,FDIOCREADID, 0)) == -1) break;
            if ((error=ioctl(fildes,FDIOCSTATUS,&(tucb_ptr->diskette_status)))
                == -1) break;
            rc = 0;
            if ((tucb_ptr->diskette_status.cylinder_num != tucb_ptr->ip1) ||
               (tucb_ptr->diskette_status.result0 & 0xc0))
               rc = 87;
         }
         tucb_ptr->diskette_parms.sectors_per_track=no_of_sectors;
         tucb_ptr->diskette_parms.sectors_per_cylinder=no_of_sectors*no_of_heads;
         tucb_ptr->diskette_parms.number_of_blocks=no_of_blocks;
         if ((error = ioctl(fildes,FDIOCSETPARMS,&(tucb_ptr->diskette_parms)))
              == -1) break;
         break;
       case LDSTY_2MB_TEST:
         if ((error = ioctl(fildes,FDIOCRECAL)) == -1) break;
         if ((error = ioctl(fildes,FDIOCGETPARMS,&(tucb_ptr->diskette_parms)))
              == -1) break;
         /*
          * The value used next is just an arbitrary value larger than the
          * real value.
          */
         no_of_sectors= tucb_ptr->diskette_parms.sectors_per_track;
         no_of_heads=tucb_ptr->diskette_parms.sectors_per_cylinder/
                     tucb_ptr->diskette_parms.sectors_per_track;
         no_of_blocks=tucb_ptr->diskette_parms.number_of_blocks;
         tucb_ptr->diskette_parms.sectors_per_track = 19;
         tucb_ptr->diskette_parms.sectors_per_cylinder = tucb_ptr->
            diskette_parms.sectors_per_track * no_of_heads;
         tucb_ptr->diskette_parms.number_of_blocks = tucb_ptr->diskette_parms.
            cylinders_per_disk * tucb_ptr->diskette_parms.sectors_per_cylinder;
         if ((error = ioctl(fildes,FDIOCSETPARMS,&(tucb_ptr->diskette_parms)))
              == -1) break;
         logical_sector=((tucb_ptr->ip1)*(tucb_ptr->diskette_parms.
             sectors_per_track)*no_of_heads+18)*SECTOR_SIZE;
         if ((error = lseek (fildes,logical_sector,SEEK_SET)) == -1) break;
         rc = 91 ;
         error = read (fildes,(tucb_ptr->datacl.datac),SECTOR_SIZE);
         if (error == -1) {
            rc = 92;
            if ((error = ioctl(fildes,FDIOCREADID, 0)) == -1) break;
            if ((error=ioctl(fildes,FDIOCSTATUS,&(tucb_ptr->diskette_status)))
                == -1) break;
            rc = 0;
            if ((tucb_ptr->diskette_status.cylinder_num != tucb_ptr->ip1) ||
               (tucb_ptr->diskette_status.result0 & 0xc0))
               rc = 93;
         }
         tucb_ptr->diskette_parms.sectors_per_track=no_of_sectors;
         tucb_ptr->diskette_parms.sectors_per_cylinder=no_of_sectors*no_of_heads;
         tucb_ptr->diskette_parms.number_of_blocks=no_of_blocks;
         if ((error = ioctl(fildes,FDIOCSETPARMS,&(tucb_ptr->diskette_parms)))
              == -1) break;
         break;
       case VERIFY_TEST:
         if ((error = ioctl(fildes,FDIOCSELDRV)) == -1) break;
         if ((error=ioctl(fildes,FDIOCSTATUS,&(tucb_ptr->diskette_status)))
             == -1) break;
         no_of_heads = 1;
         if ((tucb_ptr->diskette_status.status3)&FDDOUBLE) no_of_heads = 2;
         no_of_sectors = 36;
         if ((tucb_ptr->diskette_status.status3)&FD8PRTRCK) no_of_sectors=8;
         if ((tucb_ptr->diskette_status.status3)&FD9PRTRCK) no_of_sectors=9;
         if ((tucb_ptr->diskette_status.status3)&FD15PRTRCK) no_of_sectors=15;
         if ((tucb_ptr->diskette_status.status3)&FD18PRTRCK) no_of_sectors=18;
         no_of_cylinders = 40;
         if ((tucb_ptr->diskette_status.status3)&FD80CYLS) no_of_cylinders=80;
         for (i = 0 ; i < no_of_cylinders; i++){
           if ((error = lseek(fildes,i*no_of_sectors*no_of_heads*SECTOR_SIZE,
                              SEEK_SET)) == -1) break;
           if ((error=read(fildes,(data_char_long.data_char),
                           (no_of_sectors*SECTOR_SIZE*2))) == -1) break;
         } /* endfor */
         break;
       case SPEED_TEST:
         if ((error = ioctl(fildes,FDIOCSPEED)) == -1) break;
         if ((error=ioctl(fildes,FDIOCSTATUS,&(tucb_ptr->diskette_status)))
             == -1) break;
         motor_speed=300;
         if ((tucb_ptr->diskette_status.status2)&FD5INCHHIGH) motor_speed=360;
         motor_speed_offset = abs(motor_speed - tucb_ptr->diskette_status.motor_speed);
         rc = 0;
         if (motor_speed_offset > (1.5*motor_speed/100)) rc = 89;
         break;
       case HEAD_SETTLE_TEST:
         rc = 90;
         if ((error = ioctl(fildes,FDIOCSETTLE,tucb_ptr->ip1)) == -1) break;
         rc = 0;
         break;
       case ENABLE_RETRY:
         error = ioctl(fildes,FDIOCRETRY);
         break;
       case DISABLE_RETRY:
         error = ioctl(fildes,FDIOCNORETRY);
         break;
       default:
         rc = 255;
     } /* endswitch */
  } /* endfor */
  if (error == -1) {
     if ((error = ioctl(fildes,FDIOCSELDRV)) == -1) return(error);
     if ((error = ioctl(fildes,FDIOCSTATUS,&(tucb_ptr->diskette_status)))
         == -1) return(error);
     rc=15;
     if ((tucb_ptr->diskette_status.dsktchng) & DISK_CHANGED)      return(rc);
     rc = 254;
     if ((tucb_ptr->diskette_status.status2)&FDTIMEOUT)                rc = 1;
     if ((tucb_ptr->diskette_status.result0)<STATUS_ERROR)         return(rc);
     if ((tucb_ptr->diskette_status.result0)&EQUIP_CHECK)              rc = 2;
     if (((tucb_ptr->diskette_status.result0)&INV_COM_MASK)==INV_CMND) rc = 3;
     if ((tucb_ptr->diskette_status.result1)&EOT)                      rc = 4;
     if ((tucb_ptr->diskette_status.result1)&CRC_ERR) {
                                                                       rc = 5;
        if ((tucb_ptr->diskette_status.result2)&CRC_ERR_DATA)          rc = 6;
     }
     if ((tucb_ptr->diskette_status.result1)&OVR_RUN_ERR)              rc = 7;
     if ((tucb_ptr->diskette_status.result1)&NO_DATA)                  rc = 8;
     if ((tucb_ptr->diskette_status.result1)&WRITE_PROTECTED)          rc = 9;
     if ((tucb_ptr->diskette_status.result1)&NO_ADDRESS_MARK) {
                                                                       rc = 10;
        if ((tucb_ptr->diskette_status.result2)&NO_ADDR_MARK_DATA)     rc = 11;
     }
     if ((tucb_ptr->diskette_status.result2)&CONTROL_MARK)             rc = 12;
     if ((tucb_ptr->diskette_status.result2)&WRONG_TRACK)              rc = 13;
     if ((tucb_ptr->diskette_status.result2)&BAD_TRACK)                rc = 14;
  }
  return(rc);
}
