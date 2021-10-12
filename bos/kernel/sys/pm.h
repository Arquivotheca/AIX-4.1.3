/* @(#)19   1.5  src/bos/kernel/sys/pm.h, sysios, bos41J, 9517A_all 4/24/95 08:24:23 */
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
#ifndef _H_PM
#define _H_PM

#include <sys/types.h>
#include <sys/lock_def.h>
#include <sys/lockname.h>
#include <sys/pmdev.h>


/*************************************************************************/
/*                  General definistions                                 */
/*************************************************************************/
#define PM_SUCCESS 0
#define PM_ERROR -1


/* PM system modes */
#define PM_NONE                 0x00000000
#define PM_SYSTEM_FULL_ON       0x00000100
#define PM_SYSTEM_ENABLE        0x00000200
#define PM_SYSTEM_STANDBY       0x00000400
#define PM_SYSTEM_SUSPEND       0x00000800
#define PM_SYSTEM_HIBERNATION   0x00001000
#define PM_SYSTEM_SHUTDOWN      0x00002000
#define PM_SYSTEM_TERMINATE     0x00004000
#define PM_TERMINATE		PM_SYSTEM_TERMINATE
/* PHASE-2 */
#define PM_TRANSITION_START     0x00010000
#define PM_TRANSITION_END       0x00020000
/* end of PHASE-2 */


/*..................................*/
/*    PM event variety              */  /*    (ENTER)       |   (EXIT)       */
/*..................................*/  /* STB SUSP HIB OFF | STB SUSP HIB   */
#define PM_EVENT_NONE               0   /*      N/A              N/A         */
#define PM_EVENT_LID_OPEN           1   /*  x   x    x   x     o   o    x    */
#define PM_EVENT_LID_CLOSE          2   /*  o   o    o   x     x   x    x    */
#define PM_EVENT_MAIN_POWER         3   /*       (phase 1 PM only)           */
#define PM_EVENT_PD_CLICK           4   /*       (phase 1 PM only)           */
#define PM_EVENT_KEYBOARD           5   /*  x   x    x   x     o   o #1 x    */
#define PM_EVENT_LOW_BATTERY        6   /*  x   o    o   x     x   x    x    */
#define PM_EVENT_CRITICAL_LOW_BATTERY 7 /*  x   x    o   x     x   o    x    */
#define PM_EVENT_RING               8   /*  x   x    x   x     x   o    o #2 */
#define PM_EVENT_RESUME_TIMER       9   /*       (reserved)                  */
#define PM_EVENT_SYSTEM_IDLE_TIMER  10  /*  o   o    o   x     x   x    x    */
#define PM_EVENT_SOFTWARE_REQUEST   11  /*       (phase 1 PM only)           */
#define PM_EVENT_SUSPPEND_TIMER     12  /*       (reserved)                  */
#define PM_EVENT_DISPLAY_MESSAGE    13  /*       (reserved)                  */
#define PM_EVENT_LOWBAT_W_PHASE1DD  14  /*       (reserved)                  */
#define PM_EVENT_AC                 15  /*      N/A               N/A        */
#define PM_EVENT_DC                 16  /*      N/A               N/A        */
#define PM_EVENT_RTC                17  /*  x   x    x   x     x   o    o #3 */
#define PM_EVENT_MOUSE              18  /*  x   x    x   x     o   o #4 x    */
#define PM_EVENT_EXTRA_INPUTDD      19  /*  x   x    x   x     o   x    x    */
#define PM_EVENT_EXTRA_BUTTON       20  /*  o   o    o   o     o   o    o    */
#define PM_EVENT_ERROR              21  /*       (reserved)                  */
#define PM_EVENT_SPECIFIED_TIME     22  /*  x   o    o   o     x   x    x    */
#define PM_EVENT_GRAPHICAL_INPUT    23  /*       (phase 1 PM only)           */
#define PM_EVENT_TERMINATE          24  /*      N/A               N/A        */
/* PHASE-2 */
#define PM_EVENT_POWER_SWITCH_ON    25  /*      N/A            o   o    o     */
#define PM_EVENT_POWER_SWITCH_OFF   26  /*  x   o    o   o        N/A         */
#define	PM_EVENT_BATTERY_STATUS_RDY 27  /* Because battery data is async job */
#define PM_EVENT_PHASE1_DD          28  /* rejected due to phase-I dd 	     */ 
#define PM_EVENT_REJECT_BY_DD       29  /* A PM aware DD rejects PM request   */
#define	PM_EVENT_REJECT_BY_HIB_VOL  30	/* Rejected due to hib volume         */
#define	PM_EVENT_NOT_SUPPORTED	    31  /* Rejected due to no capability      */
#define	PM_EVENT_GENERAL_ERROR      99  /* abnormal error related to PM       */

#define PM_EVENT_P1SWRQ_STANDBY     100  /* for compatibility with phase1 PM */
#define PM_EVENT_P1SWRQ_SHUTDOWN    101  /* for compatibility with phase1 PM */
/* end of PHASE-2 */



/*************************************************************************/
/*                Definition for PM base kernel data  		 	 */
/*************************************************************************/

/* PM Kernel Data */
struct _pm_kernel_data {
        Simple_lock                     lock;
        void                            (*cpu_idle)();
        void                            (*cpu_idle_pm_core)();
        void                            (*turn_power_off)();
        void                            (*turn_power_off_pm_core)();
        struct pm_handle                *handle_head;
        struct pm_handle                *graphical_input_handle_head;
        struct pm_handle                *graphical_output_handle_head;
        struct pm_handle                *other_device_handle_head;
        struct pm_planar_control_handle *planar_control_handle_head;
 	Simple_lock                     planar_lock;
};



/*************************************************************************/
/*                Definition for PM SYSPROC(170585)			 */
/*************************************************************************/
#define	PM_STOP_KOTHREADS	1
#define	PM_STOP_KTHREADS	2

/*************************************************************************/
/*                Definition for PM System Call                          */
/*************************************************************************/

/*************************************************************************/
/*  pm_control_parameter system call                                     */
/*************************************************************************/
/* ctrl */
/* reserved number 				18	*/
#define PM_CTRL_SET_PARAMETERS                  19
#define PM_CTRL_QUERY_DEVICE_NUMBER             20
#define PM_CTRL_QUERY_DEVICE_LIST               21
#define PM_CTRL_SET_DEVICE_INFO                 22
#define PM_CTRL_SET_HIBERNATION_VOLUME          23
#define PM_CTRL_QUERY_DEVICE_INFO               24

/* PM set binary */
#define PM_OFF 0
#define PM_ON  1

/* PM misc */
#define PM_DEVICE_NAME_LENGTH 16

/* For query device list */
typedef struct _pm_device_names {
	int	number;
	char *	names;
} pm_device_names_t;


/* PM Device Info Structure */
typedef struct _pm_device_info {
   char  name[PM_DEVICE_NAME_LENGTH];   
   int   mode;
   int   attribute;
   int   idle_time;
   int   standby_time;
   int   idle_time1;
   int   idle_time2;
   char  reserved[24];
} pm_device_info_t;

/* sector list */
typedef struct _pm_sector {
   long  start;   	/* RBA(Relative Block Address) in sectors(512 bytes) */
   long  size; 		/* sector size in sectors(512 bytes) */
} pm_sector_t;

/* PM logical volume information */
typedef struct _pm_hibernation {
   dev_t    devno;      /* major/minor device number */
   long     ppnum;      /* physical partition number */
   long     ppsize;     /* physical partition size */
   long     snum;       /* valid sector list item number */
   pm_sector_t *slist;  /* sector list */
} pm_hibernation_t;

/* function prototype */
int pm_control_parameter(int, caddr_t);



/*************************************************************************/
/*  pm_control_state system call                                         */
/*************************************************************************/
/* ctrl parameter */
#define PM_CTRL_QUERY_SYSTEM_STATE  5
#define PM_CTRL_START_SYSTEM_STATE  6

typedef struct _pm_system_state {
   int   state;         /* system PM state */
   int   event;         /* resume event */
   char  name[PM_DEVICE_NAME_LENGTH];   
   char  reserve[8];    /* reserved */
} pm_system_state_t;

/* function prototype */
int pm_control_state(int, caddr_t);



/*************************************************************************/
/* pm_system_event_query system call                                     */
/*************************************************************************/
/* function prototype */
int pm_system_event_query(int *);


/*************************************************************************/
/* pm_battery_control system call                                        */
/*************************************************************************/
/* cmd */
#define PM_BATTERY_DISCHARGE  1
#define PM_BATTERY_QUERY   2


/* PM Battery Structure */
typedef struct _pm_battery_data {
   int attribute;                  /* Various information of battery */
   int capacity;                   /* Actual capacity when fully charged:(mAh)*/
   int remain;                     /* remained capacity: (mAh) */
   int refresh_discharge_capacity; /* Capacity reduced by refresh discharge  */
   int refresh_discharge_time;  /* Duration after starting refresh discharge */
   int full_charge_count;       /* times of charging fully without exhaust   */
} pm_battery_data_t;

/* pm_battery.attribute */
#define PM_BATTERY    0x00000100      /* Set if battery is supported.      */
#define PM_BATTERY_EXIST 0x00000200   /* Set when battery is attached.     */
#define PM_NICD       0x00000400      /* This is never set in Woodfield.   */
#define PM_CHARGE     0x00000800      /* Set only when battery is charged. */
#define PM_DISCHARGE  0x00001000      /* Set only during refresh discharge.*/
#define PM_AC         0x00002000      /* Set when AC power is supplied.    */
#define PM_DC         0x00004000      /* Set when AC power is not supplied.*/
/* PHASE-2 */
#define PM_REFRESH_DISCHARGE_REQ  0x00008000 /* Set if memory effect         */
/* end of PHASE-2 */

/* function prototype */
int pm_battery_control(int, struct pm_battery *);


/*************************************************************************/
/*   Miscellaneous definitions                                           */
/*************************************************************************/

/* PM config/unconfig */
#define	PM_CONFIG	0
#define	PM_UNCONFIG	1




/*************************************************************************/
/*  PM daemon data transferred to PMMI for backward compatibility        */
/*************************************************************************/
	
typedef struct _daemon_data{
   int   system_idle_action;     /* system idle action */
   int   lid_close_action;       /* lid close action */
   int   main_switch_action;     /* main power switch action */
   int   low_battery_action;     /* low battery action */
   int   specified_time_action;  /* specified time action, sus or hiber */
   int   resume_passwd;          /* enable/disable resume password */
   int   kill_lft_session;       /* continue/kill LFT session */
   int   kill_tty_session;       /* continue/kill TTY session */
   int   permission;             /* permitted state by superuser */ 
} daemon_data_t;

/*************************************************************************/
/*  PM daemon data transferred to PMMI for PMMI internal control         */
/*************************************************************************/
/* data for PM core */
typedef struct _core_data{
    int  system_idle_time;          /* system idle time in seconds */
    int  pm_beep;                   /* enable/disable beep */
    int  ringing_resume;            /* enable/disable ringing resume */
    time_t  resume_time;            /* specified time to resume */
    time_t  specified_time;         /* specified time to sus or hiber */
    int  sus_to_hiber;              /* duration from hiber to resume */
    int  kill_syncd;		    /* kill sync daemon */
    char reserve[4];                /* reserved */
} core_data_t;

typedef struct _pm_parameters {
    Simple_lock	lock;		    /* lock data to serialize the access */
    core_data_t core_data;
    daemon_data_t daemon_data;
} pm_parameters_t;





/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/
/*       For PHASE 1 backward compatibility         */
/*$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*/

/*************************************************************************/
/*        Definition for PM System Call (for phase-1)                    */
/*************************************************************************/

/*  pm_control_parameter system call */

/* ctrl */
#define  PM_CTRL_QUERY_SYSTEM_IDLE_TIMER        1
#define  PM_CTRL_SET_SYSTEM_IDLE_TIMER          2
#define  PM_CTRL_QUERY_DEVICE_IDLE_TIMER        3
#define  PM_CTRL_SET_DEVICE_IDLE_TIMER          4
#define  PM_CTRL_QUERY_LID_CLOSE_ACTION         5
#define  PM_CTRL_SET_LID_CLOSE_ACTION           6
#define  PM_CTRL_QUERY_SYSTEM_IDLE_ACTION       7
#define  PM_CTRL_SET_SYSTEM_IDLE_ACTION         8
#define  PM_CTRL_QUERY_MAIN_SWITCH_ACTION       9
#define  PM_CTRL_SET_MAIN_SWITCH_ACTION         10
#define  PM_CTRL_QUERY_LOW_BATTERY_ACTION       11
#define  PM_CTRL_SET_LOW_BATTERY_ACTION         12
#define  PM_CTRL_QUERY_BEEP                     13
#define  PM_CTRL_SET_BEEP                       14
#define  PM_CTRL_QUERY_PM_DD_NUMBER             15
#define  PM_CTRL_QUERY_PM_DD_LIST               16
#define  PM_CTRL_QUERY_LID_STATE                17

/* PM Device/Timer Structure */
struct pm_device_timer_struct {
   dev_t devno;
   int   mode;
   int   device_idle_time;
   int   device_standby_time;
};
#define PM_DTSIZE sizeof(struct pm_device_timer_struct)

/* PM Beep set value */
#define PM_BEEP_OFF  0
#define PM_BEEP_ON   1

/* PM LID state set value */
#define PM_LID_CLOSE 0
#define PM_LID_OPEN  1


/*  pm_control_state system call */
/* ctrl */
#define  PM_CTRL_QUERY_STATE     1
#define  PM_CTRL_REQUEST_STATE   2
#define  PM_CTRL_START_STATE     3
#define  PM_CTRL_QUERY_REQUEST   4

/* PM state structure */
struct pm_state {
   int   state;
   int   id;
   int   event;
   int   devno;
};

#define PM_MAX_REQUESTED_EVENT_NUMBER  16


/* pm_event_query system call    */
int pm_event_query(int *, int *);


/* pm_battery_control system call */
/* PM Battery Structure */
struct pm_battery {
   int attribute;
   int capacity;
   int remain;
   int discharge_remain;
   int discharge_time;
   int full_charge_count;
};

/* PM Machine Data */
struct pm_machine_data {
        unsigned int    model;
        unsigned int    lid:1;
        unsigned int    battery:1;
        unsigned int    soft_switch:1;
        unsigned int    suspend_ext_kbd:1;
        unsigned int    suspend_ext_sig:1;
        unsigned int    suspend_ri:1;
        unsigned int    suspend_rtc:1;
        unsigned int    suspend_lid:1;
        unsigned int    hibernation_ext_kbd:1;
        unsigned int    hibernation_ext_sig:1;
        unsigned int    hibernation_ri:1;
        unsigned int    hibernation_rtc:1;
        unsigned int    reseved:20;
};

/* PM Core Data */
struct _pm_core_data {
        Simple_lock     lock;
        int             system_state;
        int             system_next_state;
        int             system_event;
        int             system_idle_time;
        int             system_idle_count;
        int             system_idle_action;
        int             lid_close_action;
        int             lid_state;
        int             main_switch_action;
        int             low_battery_action;
        int             beep;
        int             pmdev_psusus;
};

#endif   /* _H_PM */

