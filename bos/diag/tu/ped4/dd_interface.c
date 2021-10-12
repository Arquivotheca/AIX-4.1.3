/* @(#)09	1.9  src/bos/diag/tu/ped4/dd_interface.c, tu_ped4, bos411, 9428A410j 6/9/94 17:42:54 */
/*
 * COMPONENT_NAME: (tu_ped4) Pedernales Graphics Adapter Test Units
 *
 * FUNCTIONS: sretract(), sgrant(), tu_open(), tu_close(), 
 *            set_mc_speed(), vpd_check, vpd_test()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
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
#include <sys/cfgodm.h>
#include <cf.h>
#include <sys/mdio.h>
#include <signal.h>

#include "bim_defs.h"
#include "host_debug.h"

/* ERROR codes returned by this module */
#define GET_HANDLE_ERROR         0x20  /* error in getting fd handle */
#define MAKE_GP_ERROR            0x21  /* error in making graphics process */
#define SET_MC_SPEED_IOCTL_ERROR 0x22  /* ioctl to change MC speed failed */
#define RETRACT_RECEIVED         0x99  /* SIGRETRACT (HOT KEY) received  */
#define POS_TEST_FAILED          0x100 /* POS register test failed  */
#define MDD_IOCTL_FAILED         0x200 /* VPD TEST system call failed */
#define VPD_STRING_COMP_ERROR    0x201 /* VPD String Compare error */
#define VPD_CRC_ERROR            0x202 /* VPD CRC value error */
#define UNMAKE_GP_ERROR          0x3D /* error in un-making graphics process */

#define OPEN_MDD_FAILED          0x3E /* Open MDD failed */

#define OPEN_RCM_ERROR           0x44  /* open rcm failed */
#define IOCTL_GSC_HANDLE_ERROR   0x45  /* GSC_HANDLE ioctl Failed */
#define DEVICE_BUSY_ERROR        0x46  /* Device Busy error */

#define ODM_INIT_FAILED          0x26  /* error initializing ODM */
#define ODM_OBJECT_SEARCH_FAILED 0x27  /* logical dev name not found in ODM */
#define ODM_GET_OBJECT_FAILED    0x28  /* error while trying to get object */

/* For POS_TEST_FAILED, in secondary_ptr[0] : */
#define POS_TEST_IOCTL_ERROR   0xDEADBEEF /* ioctl call to test POS error */


/* Constants used in this file */
#define DEVICE_DRIVER_NAME "middd"


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

/* used to assign a short  into  an  array */
/* pass in the address of the first byte   */
/* in the array to be assigned into and the*/
/* short your are assigning to it.         */
#define SHORT_TO_ARRAY(a,i)  *((char *) (a))     = (i) >> 8;\
                             *((char *) (a) + 1) = (i);

/* function prototypes */
extern void event_handler();
int read_pos(int mdd_fd, int slot, int pos_reg, char *data_ptr);  /*function*/
int write_pos(int mdd_fd, int slot, int pos_reg, char *data_ptr); /*prototype*/
int tu_open(char *logical_devname);
int tu_close(void);

 /* external variables */
   extern errno;


   /* LFT and RCM interfaces */
   int           rcm_fdes;
   gsc_handle    our_gsc_handle;
   make_gp       makemap;
   unmake_gp     unmake;
   mid_make_gp_t middat;

   /* GLOBAL data structures used in this file */
   volatile unsigned long *bim_base_addr;

   union {              /* data structure used to accumulate CRC */
      ushort  whole;
      struct {
         char msb;
         char lsb;
      } bite;
   } avalue, dvalue;

   int ped4_slot = 0;         /* slot where card being tested resides */
   void acc_crc(char data);


/*
 * NAME: tu_open
 *
 * FUNCTION: call the device driver and set up pointer to the base address
 *           of the adapter. 
 *           Initialize global variable bim_base_addr.
 *           It also calls ODM functions to get slot number for the 
 *           given logical device name to set the global variable ped4_slot.
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
  char criteria[20];

   /* get the slot number where the card resides for MDD operation */
   /* First perform and verify correct ODM initialization */
   ret_code = (int)odm_initialize();    /* initialize ODM data base */ 
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

   ped4_slot = atoi(CuDv_obj.connwhere) - 1;  /* set the slot number */

   /* Open lft via rcm0 device to access the given adapter*/ 
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
 * DATA STRUCTURES: global variable physical_slot 
 *
 * RETURNS : Returns zero if successful, non-zero if error.
 *
 * INPUT: speed - the mc slave speed, either 200 or 300 (nsec).
 *
 * OUTPUT: clears bit 3 of pos register 2 in BIM.
 *
 */
set_mc_speed(int speed)
{
   int ret_code;
   char data;
   int  mdd_fd;

   /* Open machine device driver  */ 
   mdd_fd = open("/dev/bus0",O_RDWR);
   if (!mdd_fd) {
     return(OPEN_MDD_FAILED);
   } /* endif */

   /* read POS 2 register contents */
   ret_code = read_pos(mdd_fd, ped4_slot, 2, &data);
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
   ret_code = write_pos(mdd_fd, ped4_slot, 2, &data);
   if (ret_code) {
      close(mdd_fd);  
      return(MDD_IOCTL_FAILED);
   } /* endif */

   close(mdd_fd);     /* close the "/dev/bus0" device */ 
   return(0);

 }  /* end set_mc_speed() */

/*
 * NAME: vpd_check()
 *
 * FUNCTION: This function tests a vpd ROM.  It check for the word 'VPD'
 *           in the VPD ROM data and it calculates CRC for the vpd data in ROM 
 *           and compares it against a pre-stored CRC value in VPD ROM.
 *
 * EXECUTION ENVIRONMENT:  AIX operating system
 *
 * NOTES: None.
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: global variable physical_slot from exectu.c
 *
 * RETURNS : Zero if successful, non-zero if error.
 *
 * INPUT: mdd_fd - file descriptor for the machine device driver
 *        slot - physical slot number where the card resides in
 *        secondary_ptr - pointer to secondary information
 *
 * OUTPUT: If return code the following secondary
 *        For MDD_IOCTL_FAILED, in secondary_ptr[0] : return code from ioctl()
 *        For VPD_STRING_ERROR, in secondary_ptr[0] : expected string packed
 *                              in secondary_ptr[1] : actual string packed
 *        For VPD_CRC_ERROR, in secondary_ptr[0] : return code from ioctl()
 *
 */
vpd_check(int card_id, int mdd_fd, int cardslot, int *secondary_ptr)
{
   int ret_code;
   int     j, k, adapter_slot;
   char    data, field[4], vpdid[5], val;
   int     vpdlen = 0;
   int     fldlen = 0;
   int     datalen = 0;
   ushort  crc = 0;
   int mdd_dd;

   mdd_dd = mdd_fd; 

   adapter_slot = cardslot; 

   field[3] = '\0';
   vpdid[4] = '\0';
   data = POS_5_AUTO_INC_ENABLE |      /* Enable POS Address Auto-Increment */
          POS_5_DISABLE_CHANNEL_CHECK | /* Disable Channel Check             */
          (card_id << 1);            /* Write to VPD "card_id"  */ 

   /* Auto increment register 7 & Disable chan check, & write to VPD "card_id"*/
   ret_code = write_pos(mdd_dd,adapter_slot,5,&data);
   if (ret_code) {
      secondary_ptr[0] = card_id;   /* Card Number */
      secondary_ptr[1] = ret_code;
      return(MDD_IOCTL_FAILED);
   } /* endif */
   data = 0;
   ret_code= write_pos(mdd_dd,adapter_slot,7,&data); /* Adrs sub register 0. */
   if (ret_code) {
      secondary_ptr[0] = card_id;   /* Card Number */
      secondary_ptr[1] = ret_code;
      return(MDD_IOCTL_FAILED);
   } /* endif */
   ret_code = read_pos(mdd_dd,adapter_slot,3,&val);
   if (ret_code) {
      secondary_ptr[0] = card_id;   /* Card Number */
      secondary_ptr[1] = ret_code;
      return(MDD_IOCTL_FAILED);
   }  /* end if */
   vpdid[0] = val;                           /* Read 00 from VPD ROM. */
   ret_code= read_pos(mdd_dd,adapter_slot,3,&val);
   if (ret_code) {
      secondary_ptr[0] = card_id;   /* Card Number */
      secondary_ptr[1] = ret_code;
      return(MDD_IOCTL_FAILED);
   } /* endif */
   vpdid[0] = val; /* ignore First Byte  - Read 'V' from VPD ROM. */
   ret_code=read_pos(mdd_dd,adapter_slot,3,&val);
   if (ret_code) {
      secondary_ptr[0] = card_id;   /* Card Number */
      secondary_ptr[1] = ret_code;
      return(MDD_IOCTL_FAILED);
   } /* endif */
   vpdid[1] = val;                           /* Read 'P' from VPD ROM */
   ret_code=read_pos(mdd_dd,adapter_slot,3,&val);
   if (ret_code) {
      secondary_ptr[0] = card_id;   /* Card Number */
      secondary_ptr[1] = ret_code;
      return(MDD_IOCTL_FAILED);
   } /* endif */
   vpdid[2] = val;                           /* Read 'D' from VPD ROM. */

   vpdid[3] = '\0';  
   if (strncmp(vpdid,"VPD",3) != 0) {
      secondary_ptr[0] = card_id;   /* Card Number */
      secondary_ptr[1] = ('V' << 16) | ('P' << 8) | 'D';
      secondary_ptr[2] = (vpdid[0] << 16) | (vpdid[1] << 8) | (vpdid[2]);
      return (VPD_STRING_COMP_ERROR);
   } /* end if */ 

   ret_code= read_pos(mdd_dd,adapter_slot,3,&val);
   if (ret_code) {
      secondary_ptr[0] = card_id;   /* Card Number */
      secondary_ptr[1] = ret_code;
      return(MDD_IOCTL_FAILED);
   } /* endif */
   (*(((char *) &vpdlen) + 2)) = val;     /* Get 1st byte of length. */
   ret_code=read_pos(mdd_dd,adapter_slot,3,&val);
   if (ret_code) {
      secondary_ptr[0] = card_id;   /* Card Number */
      secondary_ptr[1] = ret_code;
      return(MDD_IOCTL_FAILED);
   } /* endif */

   (*(((char *) &vpdlen) + 3)) = val;     /* Get 2nd byte of length. */
   vpdlen = vpdlen << 1;                   /* Byte count = 2 * length */

   ret_code= read_pos(mdd_dd,adapter_slot,3,&val);
   if (ret_code) {
      secondary_ptr[0] = card_id;   /* Card Number */
      secondary_ptr[1] = ret_code;
      return(MDD_IOCTL_FAILED);
   } /* endif */
   (* (((char *) &crc) + 0)) = val;       /* Get 1st byte of CRC. */
   ret_code= read_pos(mdd_dd,adapter_slot,3,&val);
   if (ret_code) {
      secondary_ptr[0] = card_id;   /* Card Number */
      secondary_ptr[1] = ret_code;
      return(MDD_IOCTL_FAILED);
   } /* endif */
   (* (((char *) &crc) + 1)) = val;       /* Get 2nd byte of CRC. */
   dvalue.whole = 0xFFFF;

   for (j=0; j<vpdlen; j+=4) {             /* Start Reading Field Data. */
      ret_code=read_pos(mdd_dd,adapter_slot,3,&val);  /* Read 1rst value. */
      if (ret_code) {
        secondary_ptr[0] = card_id;   /* Card Number */
        secondary_ptr[1] = ret_code;
        return(MDD_IOCTL_FAILED);
      } else {
        acc_crc(val);                   /* Accumulate CRC. */
        field[0] = val;
      } /*end if */
      if (field[0] != '*') {               /* Verify Field Start. */
         secondary_ptr[0] = card_id;   /* Card Number */
         secondary_ptr[1] = '*'; 
         secondary_ptr[2] = field[0];
         return(VPD_STRING_COMP_ERROR);
      } /* endif */
      /* Read Field ID's first letter */
      if (j < (vpdlen - 3)) {
          /* Read First field letter */
          ret_code=read_pos(mdd_dd,adapter_slot,3,&val);
          if (ret_code) {
            secondary_ptr[0] = card_id;   /* Card Number */
            secondary_ptr[1] = ret_code;
            return(MDD_IOCTL_FAILED);
          } else {
            field[1] = val;                  /* and accumulate CRC. */
            acc_crc(val);
          } /* end if */
          /* Read Field ID's second letter */
          ret_code=read_pos(mdd_dd,adapter_slot,3,&val);
         if (ret_code) {
           secondary_ptr[0] = card_id;   /*Card Numner */
           secondary_ptr[1] = ret_code;
           return(MDD_IOCTL_FAILED);
         } else { 
           field[2] = val;                  /* and accumulate CRC. */
           acc_crc(val);
         }  /*end if */
         /* read field length */
         ret_code = read_pos(mdd_dd,adapter_slot,3,&val); 
         if (ret_code) {
           secondary_ptr[0] = card_id;   /* Card Number */
           secondary_ptr[1] = ret_code;
           return(MDD_IOCTL_FAILED);
         } else {
           /*  *(((char *) &fldlen) + 3) = val; */
           fldlen = (int)val;
           acc_crc(val);                      /* accumulate CRC. */
         } /* end if */
         datalen = (fldlen<<1) - 4;        /* Calculate data length */
        /* Read the remainder of the filed */
        for (k=0; k<datalen; k++,j++) {   /* Read/print field remainder. */
          ret_code =read_pos(mdd_dd,adapter_slot,3,&val);
          if (ret_code) {
            secondary_ptr[0] = card_id;   /* Card Number */
            secondary_ptr[1] = ret_code;
            return(MDD_IOCTL_FAILED);
          } else {
              acc_crc(val);  /* Accumulate CRC */ 
          }  /* end if */
        } /* endfor */
      } /* endif */
   } /* endfor  */
   if (crc != dvalue.whole) {
     secondary_ptr[0] = card_id;   /* Card Number */
     secondary_ptr[1] = (int)crc;
     secondary_ptr[2] = (int)dvalue.whole;
     return (VPD_CRC_ERROR);
   } /* endif */

   return (0);

}  /* end vpd_check */ 

/*
 * NAME: vpd_test()
 *
 * FUNCTION: This function will call vpd_check routine in order to read the 
 *           VPD by using the machine device driver.  The vpd_check() will 
 *           calculate the vpd data CRC and will check it against the value 
 *           in the VPD ROM.
 *
 * EXECUTION ENVIRONMENT:  AIX operating system
 *
 * NOTES: None.
 *
 * RECOVERY OPERATION: None.
 *
 * DATA STRUCTURES: NONE.
 *
 * RETURNS : Zero if successful, non-zero if error.
 *
 * INPUT: slot - physical slot number where the card resides in
 *        secondary_ptr - pointer to secondary information
 *
 * OUTPUT: If return code the following secondary
 *        For MDD_IOCTL_FAILED, in secondary_ptr[0] : return code from ioctl()
 *        For VPD_STRING_ERROR, in secondary_ptr[0] : expected string packed
 *                              in secondary_ptr[1] : actual string packed
 *        For VPD_CRC_ERROR, in secondary_ptr[0] : return code from ioctl()
 *
 */
vpd_test(int *secondary_ptr)
{
   int ret_code=0;
   int  mdd_fd;

   mdd_fd = open("/dev/bus0",O_RDWR);  /* get descriptor for mdd_fd */
   if (!mdd_fd) {
     return(OPEN_MDD_FAILED);
   } /* endif */

   /* Check VPD on processor  card.  */
   ret_code = vpd_check(0,mdd_fd, ped4_slot,secondary_ptr); 
   if (ret_code == 0) { 
      /* Check VPD on Graphics card .*/
      ret_code = vpd_check(1,mdd_fd, ped4_slot,secondary_ptr);
   } 
   else {
     close(mdd_fd);
     return(ret_code);
   }  /* end if */ 

   /* Check VPD on option card  */
   /* ret_code = vpd_check(2,mdd_fd, ped4_slot,secondary_ptr); */

   close(mdd_fd);     /* close the "/dev/bus0" device */ 
   return(ret_code); 

}  /* end vpd_test */ 

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
