static char sccsid[] = "@(#)11  1.14  src/bos/diag/tu/lega/dd_interface.c, tu_lega, bos411, 9428A410j 6/9/94 17:43:02";
/*
 * COMPONENT_NAME: (tu_lega) Low End Graphics Adapter Test Units
 *
 * FUNCTIONS: setup(), sretract(), sgrant(), enter_mom(),
 *            leave_mom(), set_mc_speed(), pos_test(), vpd_test()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 * MODULE NAME: dd_interface.c
 *
 * STATUS: Release 1, EC 00, EVT Version 1
 *
 * DEPENDENCIES:  None.
 *
 * RESTRICTIONS:  None.
 *
 * EXTERNAL REFERENCES
 *
 *     OTHER ROUTINES:  fopen, fread, fseek, fclose
 *
 *     DATA AREAS:  Global Variable - dd in file exectu.c
 *
 *     TABLES:  None.
 *
 *     MACROS:  None.
 *
 * COMPILER / ASSEMBLER
 *
 *     TYPE, VERSION: AIX C Compiler, version 3
 *
 *     OPTIONS:
 *
 * NOTES:  None.
 *
 */
#include   <stdio.h>
#include   <fcntl.h>
#include   <sys/rcmioctl.h>
#include   <sys/rcm_win.h>
#include   <sys/aixgsc.h>
#include   <mid/mid.h>

#include <sys/types.h>
#include <sys/sysconfig.h>
#include <sys/device.h>
#include <errno.h>
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/mdio.h>
#include <signal.h>

#include "bim_defs.h"
#include "host_debug.h"

/* ERROR codes returned by this module */
#define GET_HANDLE_ERROR         0x20  /* error in getting dd handle */
#define MAKE_GP_ERROR            0x21  /* error in making graphics process */
#define SET_MC_SPEED_IOCTL_ERROR 0x22  /* ioctl to change MC speed failed */

#define RETRACT_RECEIVED         0x99  /* SIGRETRACT (HOT KEY) received  */

#define POS_TEST_FAILED          0x100 /* POS register test failed  */
#define MDD_IOCTL_FAILED         0x200 /* VPD TEST system call failed */
#define VPD_STRING_COMP_ERROR    0x201 /* VPD String Compare error */
#define VPD_CRC_ERROR            0x202 /* VPD CRC value error */
#define UNMAKE_GP_ERROR          0x3D /* error in un-making graphics process */


#define OPEN_RCM_ERROR           0x23  /* error in opening /dev/rcm file */
#define DEVICE_BUSY_ERROR        0x24  /* device to be tested is being used */
#define OPEN_MDD_ERROR           0x25  /* error in opening /dev/bus0 */
#define ODM_INIT_FAILED          0x26  /* error initializing ODM */
#define ODM_OBJECT_SEARCH_FAILED 0x27  /* logical dev name not found in ODM */
#define ODM_GET_OBJECT_FAILED    0x28  /* error while trying to get object */



/********************************************************/
/*           Macro Definitions                          */
/********************************************************/
/* used to assign an integer into an array */
/* pass in the address of the first byte   */
/* in the array to be assigned into and the*/
/* integer your are assigning to it.       */
#define INT_TO_ARRAY(a,i)  *((char *) (a))     = (i) >> 24;\
                           *((char *) (a) + 1) = (i) >> 16;\
                           *((char *) (a) + 2) = (i) >> 8; \
                           *((char *) (a) + 3) = (i);

/* used to assign a short  into  an  array  */
/* pass in the address of the first byte    */
/* in the array to be assigned into and the */
/* short your are assigning to it.          */
#define SHORT_TO_ARRAY(a,i)  *((char *) (a))     = (i) >> 8;\
                             *((char *) (a) + 1) = (i);


/* external functions */
extern void event_handler();

/* function prototypes for this file */
int tu_open(char *logical_device_name);
int tu_close(void);
int set_mc_speed(int speed);
int vpd_test(int *secondary_ptr);
int read_pos(int mdd_fd, int slot, int pos_reg, char *data_ptr);
int write_pos(int mdd_fd, int slot, int pos_reg, char *data_ptr);
void acc_crc(char data);

   /* external variables */
   extern errno;

   /* LFT and RCM interfaces */
   int           rcm_fdes;
   gsc_handle    our_gsc_handle;
   make_gp       makemap;
   unmake_gp     unmake;

   /* DEVICE DRIVER INTERFACE                */
   /* Note : data types are defined in mid.h */
   mid_make_gp_t middat;

   /* GLOBAL data structures used in this file */
   int     grant = 0;

   volatile unsigned long *bim_base_addr;

   union {              /* data structure used to accumulate CRC */
      ushort  whole;
      struct {
         char msb;
         char lsb;
      } bite;
   } avalue, dvalue;

   int lega_slot = 0; /* slot where card being tested resides */

/*
 * NAME: tu_open
 *
 * FUNCTION: call the device driver and set up pointer to the base address
 *           of the adapter. Initialize global variable bim_base_addr.
 *
 * EXECUTION ENVIRONMENT:  AIX operating system
 *
 * NOTES: None.
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: bim_base_addr, makemap, our_gsc_handle, middat,
 *                  (local to this file) and,
 *
 * RETURNS : see below
 *
 * INPUT:
 *      None.
 * OUTPUT:
 *      returns zero if load was successful, otherwise will return
 *      non-zero.
 */
int tu_open(char *logical_device_name)
{
   int     ret_code , i;
   struct CuDv CuDv_obj, *CuDv_obj_ptr;
   char criteria[40];


   /* get the slot number where the card resides for MDD operations */
   /* First perform and verify correct ODM initialization */
   ret_code = (int)odm_initialize();
   if (ret_code) {
     return(ODM_INIT_FAILED);
   } /* endif */

   /* Find the CuDv object corresponding to logical_device_name */
   sprintf(criteria, "name = '%s'", logical_device_name);
   CuDv_obj_ptr = odm_get_first(CuDv_CLASS,criteria,&CuDv_obj);
   ret_code = (int)CuDv_obj_ptr;
   if (!ret_code) {
     ret_code = ODM_OBJECT_SEARCH_FAILED;
     return(ret_code);
   } /* endif */

   if (ret_code == -1) {
     ret_code = ODM_GET_OBJECT_FAILED;
     return(ret_code);
   } /* endif */

   lega_slot = atoi(CuDv_obj.connwhere) - 1;

   rcm_fdes = open("/dev/rcm0",O_RDWR);
   if (rcm_fdes < 0) {
     return(OPEN_RCM_ERROR);
   } /* endif */

   strcpy(our_gsc_handle.devname,logical_device_name);

   /* get handle */
   if (ioctl(rcm_fdes, GSC_HANDLE, &our_gsc_handle) < 0 ){
     if (errno == EBUSY) {
       return(DEVICE_BUSY_ERROR);
     } else {
       return(GET_HANDLE_ERROR);
     } /* endif */
   } /* endif */

   /* make graphic process */

   middat.hwconfig = 0x40;    /* TELL DD WE ARE DIAGNOSTICS  6/24/91 !KM! */
   makemap.pData = (genericPtr) &middat;
   makemap.length = sizeof(middat);
   makemap.access = EXCLUSIVE_ACCESS;

   if (aixgsc(our_gsc_handle.handle, MAKE_GP, &makemap)){
     return(MAKE_GP_ERROR);
   } /* endif */

   /* turn on event_handler for SIGMSG signal which is how we get notified */
   /* of interrupts from the adapter */
   (void)signal(SIGMSG,event_handler);

   /* set up the base address for LEGA adapter */
   bim_base_addr = (unsigned long *)
                           ((int) makemap.segment | (int) middat.bus_addr);

   /* HOLD DSP in RESET */
   *(bim_base_addr + DSP_CONTROL) = 0;

   PRINT(("\n BIM BASE ADDRESS IS %x \n", bim_base_addr));


   return(0);   /* if we got here then all is well */

}  /* end tu_open() */

/*
 * NAME: tu_close
 *
 * FUNCTION: Unmakes GP and closes the path through the kernel to the device
 *
 * EXECUTION ENVIRONMENT:  AIX operating system
 *
 * NOTES: None.
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES:
 *
 * RETURNS : Always returns zero.
 *
 * INPUT: None.
 *
 * OUTPUT: None.
 *
 */
int tu_close(void)
{

   /* unmake the graphics process */
   aixgsc(our_gsc_handle.handle, UNMAKE_GP, &unmake);
   if (unmake.error) {
      return(UNMAKE_GP_ERROR);
   } /* endif */

   close(rcm_fdes);

   return(0);

} /* end tu_close() */


/*
 * NAME: set_mc_speed
 *
 * FUNCTION: Sets the the micro channel slave speed for BIM according to the
 *           input parameter, so that either BIM operates in 200ns slave mode,
 *           or 300ns slave mode on the micro channel bus.
 *
 * EXECUTION ENVIRONMENT:  AIX operating system
 *
 * NOTES: None.
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: global variable dd
 *
 * RETURNS : Returns zero if successful, non-zero if error.
 *
 * INPUT: speed - the mc slave speed, either 200 or 300 (nsec).
 *
 * OUTPUT: clears bit 3 of pos register 2 in BIM.
 *
 */
int set_mc_speed(int speed)
{
   int ret_code;
   char data;
   int mdd_fd;

   mdd_fd = open("/dev/bus0",O_RDWR);
   if (!mdd_fd) {
     return(OPEN_MDD_ERROR);
   } /* endif */

   /* read POS 2 register contents */
   ret_code = read_pos(mdd_fd, lega_slot, 2, &data);
   if (ret_code) {
     close(mdd_fd);
     return(MDD_IOCTL_FAILED);
   } /* endif */

   /* modify POS 2 register contents for speed */
   if (speed==200) {
     data |= 0x08;      /* set bit 3 on for 200 ns bus cycles */
   } else {
     data &= 0xF7;      /* reset bit 3 off for 300 ns bus cycles */
   } /* endif */

   /* write back out to POS 2 */
   ret_code = write_pos(mdd_fd, lega_slot, 2, &data);
   if (ret_code) {
     close(mdd_fd);
     return(MDD_IOCTL_FAILED);
   } /* endif */

   close(mdd_fd);
   return(0);

}  /* end set_mc_speed() */

/*
 * NAME: vpd_test()
 *
 * FUNCTION: reads the VPD by using the machine device driver, then calculates
 *           its CRC and checks it against the value in the VPD ROM.
 *
 * EXECUTION ENVIRONMENT:  AIX operating system
 *
 * NOTES: None.
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: global variable dd from exectu.c
 *
 * RETURNS : Zero if successful, non-zero if error.
 *
 * INPUT: secondary_ptr - pointer to secondary information
 *
 * OUTPUT: If return code the following secondary
 *        For MDD_IOCTL_FAILED, in secondary_ptr[0] : return code from ioctl()
 *        For VPD_STRING_ERROR, in secondary_ptr[0] : expected string packed
 *                              in secondary_ptr[1] : actual string packed
 *        For VPD_CRC_ERROR, in secondary_ptr[0] : return code from ioctl()
 *
 */
int vpd_test(int *secondary_ptr)
{
   int     ret_code = 0;
   int     i,j;
   char    data,field[4],vpdid[4];
   int     vpdlen = 0;
   int     fldlen = 0;
   int     datalen = 0;
   ushort  crc = 0;
   int     mdd_fd;

   mdd_fd = open("/dev/bus0",O_RDWR);
   if (!mdd_fd) {
     return(OPEN_MDD_ERROR);
   } /* endif */

   field[3] = (char)NULL;
   vpdid[4] = (char)NULL;

   /* set up data to be written to POS 5 */
   data = POS_5_AUTO_INC_ENABLE |       /* Enable POS address auto-increment */
          POS_5_DISABLE_CHANNEL_CHECK | /* Disable channel checks            */
          POS_5_ENABLE_VPD_0;           /* Enable first VPD                  */

   /* write data to POS 5 */
   ret_code = write_pos(mdd_fd, lega_slot, 5, &data);
   if (ret_code) {
      close(mdd_fd);
      secondary_ptr[0] = ret_code;
      return(MDD_IOCTL_FAILED);
   } /* endif */

   /* set up data to be written to POS 7 */
   data = 0;                             /* Address sub register 0 */

   /* write data to POS 7 */
   ret_code = write_pos(mdd_fd, lega_slot, 7, &data);
   if (ret_code) {
      close(mdd_fd);
      secondary_ptr[0] = ret_code;
      return(MDD_IOCTL_FAILED);
   } /* endif */

   /* start reading VPD from POS 3 */
   ret_code = read_pos(mdd_fd, lega_slot, 3, &data); /* Read 00  from VPD ROM */
   if (ret_code) {
      close(mdd_fd);
      secondary_ptr[0] = ret_code;
      return(MDD_IOCTL_FAILED);
   } else {
      vpdid[0] = data;
   } /* endif */
   ret_code = read_pos(mdd_fd, lega_slot, 3, &data); /* Read 'V' from VPD ROM */
   if (ret_code) {
      close(mdd_fd);
      secondary_ptr[0] = ret_code;
      return(MDD_IOCTL_FAILED);
   } else {
      vpdid[0] = data;      /*ignore the first BYTE */
   } /* endif */
   ret_code = read_pos(mdd_fd, lega_slot, 3, &data); /* Read 'P' from VPD ROM */
   if (ret_code) {
      close(mdd_fd);
      secondary_ptr[0] = ret_code;
      return(MDD_IOCTL_FAILED);
   } else {
      vpdid[1] = data;
   } /* endif */
   ret_code = read_pos(mdd_fd, lega_slot, 3, &data); /* Read 'D' from VPD ROM */
   if (ret_code) {
      close(mdd_fd);
      secondary_ptr[0] = ret_code;
      return(MDD_IOCTL_FAILED);
   } else {
      vpdid[2] = data;
   } /* endif */

   /* check to see if the string read is correct- ignore the byte 0 */
   if (strncmp(vpdid,"VPD",3) != 0) {
      secondary_ptr[0] = ('V' << 16) | ('P' << 8) | 'D';
      secondary_ptr[1] = (vpdid[0] << 16) | (vpdid[1] << 8) | (vpdid[2]);
      return (VPD_STRING_COMP_ERROR);
   } /* endif */

   /* read the length of good data in VPD */
   ret_code = read_pos(mdd_fd, lega_slot, 3, &data); /* Get 1st byte of len  */
   if (ret_code) {
      close(mdd_fd);
      secondary_ptr[0] = ret_code;
      return(MDD_IOCTL_FAILED);
   } else {
      *( ((char *)&vpdlen) + 2) = data;
   } /* endif */
   ret_code = read_pos(mdd_fd, lega_slot, 3, &data); /* Get 2nd byte of len  */
   if (ret_code) {
      close(mdd_fd);
      secondary_ptr[0] = ret_code;
      return(MDD_IOCTL_FAILED);
   } else {
      *( ((char *)&vpdlen) + 3) = data;
   } /* endif */
   vpdlen = vpdlen << 1;               /* Byte count = 2 * len */

   /* read the CRC stored in VPD */
   ret_code = read_pos(mdd_fd, lega_slot, 3, &data); /* Get 1st byte of CRC  */
   if (ret_code) {
      close(mdd_fd);
      secondary_ptr[0] = ret_code;
      return(MDD_IOCTL_FAILED);
   } else {
      *( ((char *)&crc) + 0) = data;
   } /* endif */
   ret_code = read_pos(mdd_fd, lega_slot, 3, &data); /* Get 2nd byte of CRC  */
   if (ret_code) {
      close(mdd_fd);
      secondary_ptr[0] = ret_code;
      return(MDD_IOCTL_FAILED);
   } else {
      *( ((char *)&crc) + 1) = data;
   } /* endif */

   /* initialize our calculated CRC holder */
   dvalue.whole = 0xFFFF;

   /* Start Reading Field Data */
   for (j = 0 ; j < vpdlen ; j += 4) {
      /* Read field's starting character */
      ret_code = read_pos(mdd_fd, lega_slot, 3, &data);
      if (ret_code) {
         close(mdd_fd);
         secondary_ptr[0] = ret_code;
         return(MDD_IOCTL_FAILED);
      } else {
         acc_crc(data);   /* Accumulate CRC */
         field[0] = data;
      } /* endif */

      if (field[0] != '*') {                   /* Verify Field Start */
         secondary_ptr[0] = '*';
         secondary_ptr[1] = field[0];
         close(mdd_fd);
         return(VPD_STRING_COMP_ERROR);
      } /* endif */
      if (j < (vpdlen - 3)) {
         /* Read field ID's 1st letter */
         ret_code = read_pos(mdd_fd, lega_slot, 3, &data);
         if (ret_code) {
            secondary_ptr[0] = ret_code;
            close(mdd_fd);
            return(MDD_IOCTL_FAILED);
         } else {
            field[1] = data;
            acc_crc(data);   /* Accumulate CRC */
         } /* endif */

         /* Read field ID's 2nd letter */
         ret_code = read_pos(mdd_fd, lega_slot, 3, &data);
         if (ret_code) {
            secondary_ptr[0] = ret_code;
            close(mdd_fd);
            return(MDD_IOCTL_FAILED);
         } else {
            field[2] = data;
            acc_crc(data);   /* Accumulate CRC */
         } /* endif */

         /* Read field length */
         ret_code = read_pos(mdd_fd, lega_slot, 3, &data);
         if (ret_code) {
            secondary_ptr[0] = ret_code;
            close(mdd_fd);
            return(MDD_IOCTL_FAILED);
         } else {
            fldlen = (int)data;
            acc_crc(data);   /* Accumulate CRC */
         } /* endif */

         datalen = (fldlen << 1) - 4; /* Calculate Data Length */

         /* Read the remainder of the Field */
         for (i = 0; i < datalen ; i++,j++) {
            ret_code = read_pos(mdd_fd, lega_slot, 3, &data);
            if (ret_code) {
               secondary_ptr[0] = ret_code;
               close(mdd_fd);
               return(MDD_IOCTL_FAILED);
            } else {
               acc_crc(data);   /* Accumulate CRC */
            } /* endif */
         } /* endfor */
      } /* endif */
   } /* endfor */

   if (crc != dvalue.whole) {
     secondary_ptr[0] = (int)crc;
     secondary_ptr[1] = (int)dvalue.whole;
     close(mdd_fd);
     return (VPD_CRC_ERROR);
   } /* endif */

   close(mdd_fd);
   return(0);    /* If we got here then all is well */

}  /* end vpd_test()  */


/*
 * NAME: read_pos()
 *
 * FUNCTION: reads a POS register by using the machine device driver.
 *
 * EXECUTION ENVIRONMENT:  AIX operating system
 *
 * NOTES: None.
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES:
 *
 * RETURNS : Zero if successful, non-zero if error.
 *
 * INPUT: mdd_fd - file descriptor for the machine device driver
 *        slot - physical slot number where the card resides in
 *        pos_reg - POS register number to be read
 *        data_ptr - after exit points to data read
 *
 * OUTPUT: None.
 *
 */
int read_pos(int mdd_fd, int slot, int pos_reg, char *data_ptr)
{
   MACH_DD_IO arg;

   arg.md_incr = MV_BYTE;
   arg.md_data = data_ptr;
   arg.md_size = 1;
   arg.md_addr = POSREG(pos_reg,slot);

   return(ioctl(mdd_fd,MIOCCGET,&arg));

} /* end read_pos() */

/*
 * NAME: write_pos()
 *
 * FUNCTION: writes a POS register by using the machine device driver.
 *
 * EXECUTION ENVIRONMENT:  AIX operating system
 *
 * NOTES: None.
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES:
 *
 * RETURNS : Zero if successful, non-zero if error.
 *
 * INPUT: mdd_fd - file descriptor for the machine device driver
 *        slot - physical slot number where the card resides in
 *        pos_reg - POS register number to be read
 *        data_ptr - upon entry points to data to be written
 *
 * OUTPUT: None.
 *
 */
int write_pos(int mdd_fd, int slot, int pos_reg, char *data_ptr)
{
   MACH_DD_IO arg;

   arg.md_incr = MV_BYTE;
   arg.md_data = data_ptr;
   arg.md_size = 1;
   arg.md_addr = POSREG(pos_reg,slot);

   return(ioctl(mdd_fd,MIOCCPUT,&arg));

} /* end write_pos() */


/*
 * NAME: acc_crc()
 *
 * FUNCTION: accumulates CRC into global variable "dvalue.whole".
 *
 * EXECUTION ENVIRONMENT:  AIX operating system
 *
 * NOTES: None.
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: global variables "avalue" and "dvalue" in this file
 *
 * RETURNS : N/A.
 *
 * INPUT: data - byte to be added to the CRC
 *
 * OUTPUT: None.
 *
 */
void acc_crc(char data)
{

   avalue.bite.lsb = (data ^ dvalue.bite.lsb);
   dvalue.bite.lsb = avalue.bite.lsb;
   avalue.whole    = ((avalue.whole << 4) ^ dvalue.bite.lsb);
   dvalue.bite.lsb = avalue.bite.lsb;
   avalue.whole   <<= 8;

   avalue.whole   >>= 1;
   avalue.bite.lsb ^= dvalue.bite.lsb;
   avalue.whole   >>= 4;

   avalue.whole     = (avalue.bite.lsb << 8) | avalue.bite.msb;
   avalue.whole     = ((avalue.whole & 0xFF07) ^ dvalue.bite.lsb);
   avalue.whole     = (avalue.bite.lsb << 8) | avalue.bite.msb;
   avalue.bite.lsb ^= dvalue.bite.msb;
   dvalue.whole     = avalue.whole;

} /* end acc_crc() */
