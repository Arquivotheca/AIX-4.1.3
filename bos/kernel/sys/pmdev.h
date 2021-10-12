/* @(#)22   1.4  src/bos/kernel/sys/pmdev.h, sysios, bos41J, 9511A_all 3/15/95 07:48:27 */
/*
 *   COMPONENT_NAME: SYSIOS
 *
 *   FUNCTIONS: Power Management Kernel Code
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _INC_PMDEV
#define _INC_PMDEV

/* General return code for PM operation */
#define PM_SUCCESS   0
#define PM_ERROR  -1


/*********************************************************/
/* 	PM handle structure				 */ 
/*********************************************************/
/* PM handle structure */
struct pm_handle {
   int      activity;	/* PM aware DD sets this value when access occurs. */
   int      mode;	/* PM aware DD needs to maintain this device mode  */
   int      device_idle_time;	/* idle timer value during system PM enable*/
   int      device_standby_time;/* idle timer value during system standby  */
   int      idle_counter;	/* idle timer counter 			   */
   int      (*handler)(caddr_t private, int mode); /*PM core calls this entry*/
   caddr_t  private;	/* private pointer passed to the handler subroutine */
   dev_t    devno;	/* device major number minor number 	 	    */
   int      attribute;	/* device attribute for some special device drivers */
   struct pm_handle *next1; 	/* next pointer used only by PM core 	    */ 
   struct pm_handle *next2;	/* next pointer used only by PM core        */
   int      device_idle_time1; 	/* idle timer for DPMS stadnby for display  */
   int      device_idle_time2;	/* idle timer for DPMS suspend for display  */ 
   char     *device_logical_name;/* pointer to string of device logical name*/
   char     reserve[2];		/* reserved area (must be 0) 		    */
   ushort   pm_version;		/* Refer to the description below           */ 
   int      *extension;		/* for future expandability 	            */	
};


/* PM activity flag */
#define  PM_ACTIVITY_NOT_SET  -1
#define  PM_NO_ACTIVITY    0
#define  PM_ACTIVITY_OCCURRED 1


/*********************************************************/
/* 	PM aware DD's PM support level 			 */
/*********************************************************/
/* PM version */
#define	PM_PHASE1_SUPPORT_ONLY	0x000 /*This version mustn't be used any more */
#define	PM_PHASE2_SUPPORT	0x100 


/*********************************************************/
/* 	Arguments for pm handler		         */		
/*********************************************************/
/* PM Device Modes
   Commonly used by PM Core */
#define PM_DEVICE_FULL_ON       0x00000000
#define PM_DEVICE_ENABLE        0x00000100
#define PM_DEVICE_DPMS_STANDBY  0x00000201
#define PM_DEVICE_DPMS_SUSPEND  0x00000202
#define PM_DEVICE_DPMS_OFF      0x00000200
#define PM_DEVICE_IDLE          0x00000200
#define PM_DEVICE_SUSPEND       0x00000400
#define PM_DEVICE_HIBERNATION   0x00000800
#define PM_DEVICE_RESUME	0x80000000  	/* internal use for PM core */ 

/* PM notice */
#define PM_PAGE_FREEZE_NOTICE   0x00001000
#define PM_PAGE_UNFREEZE_NOTICE 0x00002000
#define PM_RING_RESUME_ENABLE_NOTICE    0x00004000


/*********************************************************/
/* 	PM attribute flags				 */
/*********************************************************/
/* PM attribute */
#define  PM_GRAPHICAL_INPUT     0x00010000
#define  PM_GRAPHICAL_OUTPUT    0x00020000

#define	 PM_OTHER_DEVICE	0x00040000 	
/* Only for phase-1 backward compatibility. Don't use this attribute any more */

#define  PM_AUDIO_INPUT         0x00080000
#define  PM_AUDIO_OUTPUT        0x00100000
#define  PM_RING_RESUME_SUPPORT 0x00200000
#define	 PM_REMOTE_TERMINAL	0x00400000


/*********************************************************/
/* 	PM kernel service to register a PM aware driver  */
/*********************************************************/
/* Definition for PM Kernel Service */
int pm_register_handle(struct pm_handle *, int);


/* PM cmd for register/unregister */
#define PM_REGISTER  1
#define PM_UNREGISTER   2


/*********************************************************/
/* 	PM planar device control 			 */
/*********************************************************/

/* PM Planar Control */
int pm_planar_control(dev_t devno, int devid, int cmd);

/* cmd */
#define PM_PLANAR_QUERY         0x00010000 	/* query the possible variety of the features */
#define PM_PLANAR_ON            0x00020000	/* power up */
#define PM_PLANAR_OFF           0x00040000	/* power off (lowest power mode)*/
#define PM_PLANAR_LOWPOWER1     0x00080000	/* low power mode (e.g. DPMS standby )*/ 
#define PM_PLANAR_LOWPOWER2     0x00100000	/* lower power mode (e.g. DPMS suspend) */
#define	PM_PLANAR_CURRENT_LEVEL 0x80000000	/* query the current power level */


/* PM Register Planar Control Handle */
int pm_register_planar_control_handle(
                struct pm_planar_control_handle *ppch, int cmd);

/* Structure for PM Planar Control */
typedef int PM_PLANAR_CONTROL(dev_t devno, int devid, int cmd);
typedef PM_PLANAR_CONTROL *PMPLANARCONTROL;

struct pm_planar_control_handle {
        int                     devid;
        PMPLANARCONTROL         control;
        struct pm_planar_control_handle *next;
};


/*********************************************************/
/* 	Dump device for hibernation feature		 */
/*********************************************************/
/* devdump */
#define PM_DUMP		0x1000

/*********************************************************/
/* 	Dev id control					 */ 
/*********************************************************/
#define	PMDEV_MAJOR_NUMBER	0x80000000
#define	PMDEV_MAJOR_MASK	0x7fff0000
#define	PMDEV_MINOR_MASK	0x0000ffff

/*********************************************************/
/* 	Planar device ID				 */
/*********************************************************/
#define PMDEV_LCD       0x10000
#define PMDEV_CRT       0x10100

#define PMDEV_GCC       0x20000
#define PMDEV_DAC       0x20100
#define PMDEV_VRAM      0x20200

#define PMDEV_VCAP	0x30000
#define	PMDEV_VPLAY	0x30100
#define PMDEV_CAMERA    0x30200
#define PMDEV_AUDIO     0x30300
#define PMDEV_AUDIO_EXT_MUTE   0x30310

#define PMDEV_INTKBD    0x40000
#define PMDEV_EXTKBD    0x40100
#define PMDEV_INTMOUSE  0x40200
#define PMDEV_EXTMOUSE  0x40300

#define PMDEV_SERIAL1   0x50001
#define PMDEV_SERIAL2   0x50002
#define PMDEV_PARALLEL  0x50100

#define PMDEV_FDC       0x60000

#define PMDEV_CPU       0x90000
#define PMDEV_L2        0x90100
#define	PMDEV_LOCAL2	0x90200
	/* PMDEV_LOCALn, n=2 */

#define	PMDEV_PSUSUS	0xb0000 /* Only for backward compatibility with 4.1.1 */
#define	PMDEV_PSUMAIN	0xb0100


#define PMDEV_PCMCIA00  0xc0000
	/* PMDEV_PCMCIANn, N=0,  n=0 */
	/*	   e.g. 0xc00Nn      */

#define PMDEV_PCI0000   0xd0000
	/* PMDEV_PCINNDD,  NN=00,DD=00 */
	/*	   e.g. 0xdNNnn        */

#define PMDEV_ISA_base  0xe0000		/* Don't use this "..base" any more */
#define PMDEV_ISA00  	0xe0000		/* Use this label, PMDEV_ISA00.     */
	/* PMDEV_ISANn,    N=0,  n=0 */
	/*	   e.g. 0xe00Nn      */

#define	PMDEV_CDROM	0xf0100	/* Don't use this "..CDROM" any more 	    */
				/* PMDEV_UNKNOWN_SCSI or PMDEV_INTERNAL_SCSI*/
				/* should be used for disk power management.*/

#define	PMDEV_UNKNOWN_SCSI  0x00100000
	/* PMDEV_UNKNOWN_SCSI = 0x00100iii, SCSI ID = 0, LUN=0  */ 
	/*	iii = ((SCSI ID << 6) | LUN )			*/

#define PMDEV_UNKNOWN_IDE   0x00110000
	/* PMDEV_UNKNOWN_IDE = 0x0011000n, device number = 0    */

#define	PMDEV_UNKNOWN_OTHER 0x00120000



#define	PMDEV_INTERNAL_SCSI 0x00180000
	/* PMDEV_INTERNAL_SCSI = 0x0018xiii,			*/
	/*		x = power connector number = 0		*/
	/*		SCSI ID = 0, LUN = 0;			*/
 	/*		iii= ((SCSI ID << 6) | LUN)		*/ 

#define PMDEV_INTERNAL_IDE  0x00190000
	/* PMDEV_INTERNAL_IDE = 0x0019x00n,			*/
	/* 		x = power connector number = 0		*/
	/* 		n = device number = 0			*/ 	

#define	PMDEV_INTERNAL_OTHER 0x001a0000
	/* PMDEV_INTERNAL_OTHER = 0x001ax000,			*/
	/* 		x = power connector number = 0		*/
			

#endif /* _INC_PMDEV */
