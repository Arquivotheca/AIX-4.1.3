/* @(#)74	1.4  src/bos/kernext/inputdd/inc/sys/inputdd.h, inputdd, bos411, 9434B411a 8/25/94 05:41:22  */
/*
 *   COMPONENT_NAME: INPUTDD
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_INPUTDD
#define _H_INPUTDD

#include <sys/types.h>

/*****************************************************************************/
/* ioctl commands for the keyboard/sound special file                        */
/*****************************************************************************/

/* list of commands                                                          */
#define KSIOC           ('K'<<8)

#define KSQUERYID       (KSIOC | 1)    /* query keyboard device identifier   */
#define KSQUERYSV       (KSIOC | 2)    /* query keyboard service vector      */
#define KSREGRING       (KSIOC | 3)    /* register input ring                */
#define KSRFLUSH        (KSIOC | 4)    /* flush input ring                   */
#define KSLED           (KSIOC | 5)    /* set/reset keyboard LEDs            */
#define KSCFGCLICK      (KSIOC | 6)    /* configure clicker                  */
#define KSVOLUME        (KSIOC | 7)    /* set alarm volume                   */
#define KSALARM         (KSIOC | 8)    /* sound alarm                        */
#define KSTRATE         (KSIOC | 9)    /* set keyboard typematic rate        */
#define KSTDELAY        (KSIOC | 10)   /* set keyboard typematic delay       */
#define KSKAP           (KSIOC | 11)   /* enable/disable keep alive poll     */
#define KSKAPACK        (KSIOC | 12)   /* keep alive poll acknowledge        */
#define KSDIAGMODE      (KSIOC | 13)   /* enable/disable diagnostics mode    */

/* Keyboard ID's                                                             */
#define KS101           0x01           /* 101 key keyboard                   */
#define KS102           0x02           /* 102 key keyboard                   */
#define KS106           0x03           /* 106 key keyboard                   */
#define KSPS2           0x04           /* ps2 keyboard                       */

/* Set/reset keyboard LED's                                                  */
#define KSCROLLLOCK     0x01           /* illuminate Scroll Lock LED         */
#define KSNUMLOCK       0x02           /* illuminate Num Lock LED            */
#define KSCAPLOCK       0x04           /* illuminate Caps Lock LED           */

/* Clicker configurations                                                    */
#define KSCLICKOFF      0              /* disable clicker                    */
#define KSCLICKLOW      1              /* enable clicker at low volume       */
#define KSCLICKMED      2              /* enable clicker at medium volume    */
#define KSCLICKHI       3              /* enable clicker at high volume      */

/* Alarm volumes                                                             */
#define KSAVOLOFF       0              /* off                                */
#define KSAVOLLOW       1              /* low volume                         */
#define KSAVOLMED       2              /* medium volume                      */
#define KSAVOLHI        3              /* high volume                        */

/* Keyboard typematic delays                                                 */
#define KSTDLY250       1              /* 250 millisecond delay              */
#define KSTDLY500       2              /* 500 millisecond delay              */
#define KSTDLY750       3              /* 750 millisecond delay              */
#define KSTDLY1000      4              /* 1000 millisecond delay             */

/* Keep alive poll                                                           */
#define KSPDISABLE      0              /* disable keep alive poll            */
#define KSPENABLE       1              /* enable keep alive poll             */

/* Diagnostics mode                                                          */
#define KSDDISABLE      0              /* disable diagnostics mode           */
#define KSDENABLE       1              /* enable diagnostics mode            */

/* Functions available via keyboard service vector                           */
#define KSVALARM        0              /* sound alarm                        */
#define KSVSAK          1              /* enable/disable secure attention key*/
#define KSVRFLUSH       2              /* flush input ring                   */

/* Secure attention key detection                                            */
#define KSSAKDISABLE    0              /* disable SAK detection              */
#define KSSAKENABLE     1              /* enable SAK detection               */

/* Sound alarm request structure                                             */
struct ksalarm {
   uint duration;                      /* duration in 1/128th of a second    */
   uint frequency;                     /* frequency in hertz                 */
};

/*****************************************************************************/
/* ioctl commands for tablet special file                                    */
/*****************************************************************************/

/* list of commands                                                          */
#define TABIOC          ('T'<<8)

#define TABQUERYID      (TABIOC | 1)   /* query tablet device identifier     */
#define TABREGRING      (TABIOC | 2)   /* register input ring                */
#define TABRFLUSH       (TABIOC | 3)   /* flush input ring                   */
#define TABCONVERSION   (TABIOC | 4)   /* set conversion mode                */
#define TABRESOLUTION   (TABIOC | 5)   /* set tablet resolution              */
#define TABORIGIN       (TABIOC | 6)   /* set tablet origin                  */
#define TABSAMPLERATE   (TABIOC | 7)   /* set tablet data sample rate        */
#define TABDEADZONE     (TABIOC | 8)   /* set tablet dead zone               */

/* Tablet query ID request structure                                         */
struct tabqueryid {
   uchar model;                        /* tablet model                       */
#define TAB6093M11      0x01           /*   6093 model 11 or equivalent      */
#define TAB6093M12      0x02           /*   6093 model 12 or equivalent      */

   uchar input_device;                 /* attached input device              */
#define TABUNKNOWN      0x00           /*   unknown input device             */
#define TABSTYLUS       0x01           /*   stylus                           */
#define TABPUCK         0x02           /*   puck                             */
};

/* Tablet conversion types                                                   */
#define TABINCH         0              /* respond in English units (inch)    */
#define TABCM           1              /* respond in Metric units (cm)       */

/* Tablet origins                                                            */
#define TABORGLL        0              /* origin is lower left corner        */
#define TABORGC         1              /* origin is center                   */

/*****************************************************************************/
/* ioctl commands for mouse special file                                     */
/*****************************************************************************/

/* list of commands                                                          */
#define MIOC            ('M'<<8)

#define MQUERYID        (MIOC | 1)     /* query mouse device identifier      */
#define MREGRING        (MIOC | 2)     /* register input ring                */
#define MRFLUSH         (MIOC | 3)     /* flush input ring                   */
#define MTHRESHOLD      (MIOC | 4)     /* set mouse report threshold         */
#define MRESOLUTION     (MIOC | 5)     /* set mouse resolution               */
#define MSCALE          (MIOC | 6)     /* set mouse scale factor             */
#define MSAMPLERATE     (MIOC | 7)     /* set mouse data sample rate         */

/* Mouse device ID                                                           */
#define MOUSE3B         0x01           /* 3 button mouse                     */
#define MOUSE2B         0x02           /* 2 button mouse                     */

/* Mouse resolutions                                                         */
#define MRES1           1              /* minimum                            */
#define MRES2           2              /*                                    */
#define MRES3           3              /*                                    */
#define MRES4           4              /* maximum                            */

/* Mouse scale factors                                                       */
#define MSCALE11        1              /* 1:1 scale factor                   */
#define MSCALE21        2              /* 2:1 scale factor                   */

/* Mouse sample rates                                                        */
#define MSR10           1              /* 10 samples per second              */
#define MSR20           2              /* 20 samples per second              */
#define MSR40           3              /* 40 samples per second              */
#define MSR60           4              /* 60 samples per second              */
#define MSR80           5              /* 80 samples per second              */
#define MSR100          6              /* 100 samples per second             */
#define MSR200          7              /* 200 samples per second             */

/*****************************************************************************/
/* ioctl commands for gio special file                                       */
/*****************************************************************************/

/* list of commands                                                          */
#define GIOC            ('G'<<8)

#define GIOQUERYID      (GIOC | 1)     /* query attached devices             */

/* GIO query ID request structure                                            */
struct gioqueryid {
   uchar port0_id;                     /* ID of device on port 0             */
   uchar port1_id;                     /* ID of device on port 1             */
};

/* supported device ID's                                                     */
#define giolpfkid       0x01           /* LPFK device ID                     */
#define giodialsid      0x02           /* dials device ID                    */

/*****************************************************************************/
/* ioctl commands for dials special file                                     */
/*****************************************************************************/

/* list of commands                                                          */
#define DIOC            ('D'<<8)

#define DIALREGRING     (DIOC | 1)     /* register input ring                */
#define DIALRFLUSH      (DIOC | 2)     /* flush input ring                   */
#define DIALSETGRAND    (DIOC | 3)     /* set dial granularity               */

/* Set dial granularity request structure                                    */
struct dialsetgrand {
   ulong dial_select;                  /* select mask (one bit per dial -    */
                                       /*   0 = no change, 1 = change)       */
   uchar dial_value[8];                /* new granularity of each selected   */
                                       /* dial in events reported per 360    */
                                       /* degree revolution specified as a   */
                                       /* power of 2 from 2 to 8             */
#define DIAL4RPR         2             /*   4 reports per revolution         */
#define DIAL8RPR         3             /*   8 reports per revolution         */
#define DIAL16RPR        4             /*   16 reports per revolution        */
#define DIAL32RPR        5             /*   32 reports per revolution        */
#define DIAL64RPR        6             /*   64 reports per revolution        */
#define DIAL128RPR       7             /*   128 reports per revolution       */
#define DIAL256RPR       8             /*   256 reports per revolution       */
};

/*****************************************************************************/
/* ioctl commands for LPFK special file                                      */
/*****************************************************************************/

/* list of commands                                                          */
#define LIOC            ('L'<<8)

#define LPFKREGRING     (LIOC | 1)     /* register input ring                */
#define LPFKRFLUSH      (LIOC | 2)     /* flush input ring                   */
#define LPFKLIGHT       (LIOC | 3)     /* set/reset key lights               */

/* Set/reset key lights request structure                                    */
struct lpfklight {
   ulong lpfk_select;                  /* select mask (one bit per LPFK -    */
                                       /*   0 = no change, 1 = change)       */
   ulong lpfk_value;                   /* new state of selected LPFKs (one   */
                                       /*   bit per LPFK -  0 = light off,   */
                                       /*   1 = light on)                    */
};

/*****************************************************************************/
/* Input Ring Registration Structures                                        */
/*****************************************************************************/

/* Input ring registration structure (kernel address space)                  */
struct kregring{
   caddr_t ring;                       /* address of input ring              */
   uchar report_id;                    /* report identifier for device       */
   void (*notify_callback) ();         /* event notification callback        */
   void (*sak_callback) ();            /* secure attention key callback      */
};

/* Input ring registration structure (user address space)                    */
struct uregring {
   caddr_t ring   ;                    /* address of input ring              */
   int   size;                         /* size of ring in bytes (header      */
                                       /*   plus reporting area)             */
   uchar report_id;                    /* report identifier for device       */
};

/*****************************************************************************/
/* Input Ring                                                                */
/*   The input ring is a contiguous block of storage which contains zero or  */
/*   more records of variable lengths placed nose to tail. The next report   */
/*   record to be processed is pointed to by ir_head; the next location to   */
/*   queue a report record is pointed to by ir_tail. The ring is managed as  */
/*   a FIFO circular queue.                                                  */
/*****************************************************************************/

struct inputring {
   uint ir_size;                       /* size of reporting area in bytes    */
   caddr_t ir_head;                    /* current "remove" point             */
   caddr_t ir_tail;                    /* current "insert" point             */

   uchar ir_overflow;                  /* not zero if input ring overflow    */
#define IROVERFLOW      0xff           /*   input ring overflow condition    */
#define IROFCLEAR       0x00           /*   overflow condition cleared       */

   uchar ir_notifyreq;                 /* notification  type requested       */
#define IRSIGEMPTY      0x00           /*   only notify when event placed in */
                                       /*   empty ring                       */
#define IRSIGALWAYS     0xff           /*   always notify                    */

   uchar ir_rsv[6];                    /* reserved                           */
                                       /* reporting area follows             */
};

/*****************************************************************************/
/* Input Ring Event Report - report header                                   */
/*   Each report record on the input ring consists of an ir_report structure */
/*   followed by device dependent data                                       */
/*****************************************************************************/

struct ir_report {
   uchar report_id;                    /* report source identifier           */
   uchar report_size;                  /* size of report in bytes            */
   uchar report_rsv[2];                /* reserved                           */
   ulong report_time;                  /* system time stamp (milliseconds)   */
                                       /* device dependent data follows      */
};

/*****************************************************************************/
/* Input Ring Event Report - keyboard                                        */
/*****************************************************************************/

struct ir_kbd{                        
   struct ir_report kbd_header;        /* header                             */
   uchar kbd_keypos;                   /* key position on keyboard           */
   uchar kbd_scancode;                 /* scan code                          */
   uchar kbd_status[3];                /* status bytes                       */

                                       /* keyboard status byte 0             */
#define KBDUXSHIFT      0x80           /*   shift                            */
#define KBDUXCTRL       0x40           /*   control                          */
#define KBDUXALT        0x20           /*   alternate                        */
#define KBDUKATAKANA    0x10           /*   katakana                         */
#define KBDUXCAPS       0x08           /*   caps lock                        */
#define KBDUXNUM        0x04           /*   num lock                         */
#define KBDUXSCROLL     0x02           /*   scroll lock                      */
#define KBDUXMAKE       0x01           /*   make (key press event)           */

                                       /* keyboard status byte 1             */
#define KBDUXRPT        0x80           /*   repeat (typematic)               */
#define KBDUXLSH        0x40           /*   left shift                       */
#define KBDUXRSH        0x20           /*   right shift                      */
#define KBDUXLALT       0x10           /*   left alternate                   */
#define KBDUXRALT       0x08           /*   right alternate                  */

                                       /* keyboard status byte 2 is reserved */
};

/*****************************************************************************/
/* Input Ring Event Report - mouse                                           */
/*****************************************************************************/

struct ir_mouse {          
   struct ir_report mouse_header;      /* header                             */
                                       /* delta's are signed accumulations   */
                                       /* of mouse movements  (2's cmplmt)   */
   short mouse_deltax;                 /*   delta x                          */
   short mouse_deltay;                 /*   delta y                          */

   uchar mouse_status;                 /* button status                      */
#define MOUSEBUTTON1    0x80           /*   left most button                 */
#define MOUSEBUTTON2    0x40           /*   middle button                    */
#define MOUSEBUTTON3    0x20           /*   right button                     */
};

/*****************************************************************************/
/* Input Ring Event Report - tablet                                          */
/*****************************************************************************/

struct ir_tablet {                    
   struct ir_report tablet_header;     /* header                             */
   short tablet_x;                     /* absolute x                         */
   short tablet_y;                     /* absolute y                         */

   uchar tablet_status;                /* button/puck status                 */
#define TABLETBUTTON1   0x80
#define TABLETBUTTON2   0x40
#define TABLETBUTTON3   0x20
#define TABLETBUTTON4   0x10
#define TABLETBUTTON5   0x08
#define TABLETBUTTON6   0x04
#define TABLETPUCK      0x01           /* puck presence (0=out,1=in)         */
};

/*****************************************************************************/
/* Input Ring Event Report - LPFK                                            */
/*****************************************************************************/

struct ir_lpfk {                      
   struct ir_report lpfk_header;       /* header                             */
   short lpfk_number;                  /* pressed LPFK number                */
};

/*****************************************************************************/
/* Input Ring Event Report - dials                                           */
/*****************************************************************************/

struct ir_dials {                   
   struct ir_report dials_header;      /* header                             */
   short dials_number;                 /* number of dial that moved          */
   short dials_value;                  /* delta change (256 points for 360   */
};                                     /* degree rotation)                   */


#endif
