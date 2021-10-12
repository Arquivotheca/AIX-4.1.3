static char sccsid[] = "@(#)41  1.10  src/bos/diag/tu/siosl/kbddev/kbddev.c, tu_siosl, bos41J, 9515A_all 4/7/95 11:04:06";
/*
 * COMPONENT_NAME: TU_SIOSL (Keyboard device diagnostic test units)
 *
 * FUNCTIONS: exectu (main), tu10, tu20, tu30, tu40, tu60, 
 *            SpeakerReadWriteTest, genkeymap, updatescreen, finish,
 *            SendKbdata, SendRxKbData, maketitleadvanced, setmenunumberbase,
 *            putmsg, chk_stat, set_raw_mode, restore_kbd_mode
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define ZERO    0
#define YES     1
#define NO      2
#define RETRY   3

#include <stdio.h>
#include <cur00.h>
#include <cur01.h>
#include <cur02.h>
#include <string.h>
#include <fcntl.h> 
#include <memory.h>
#include <errno.h> 
#include <ctype.h> 
#include <sys/sem.h>
#include <sys/devinfo.h>
#include <sys/types.h>
#include <sys/sysconfig.h>
#include <sys/intr.h>
#include <sys/signal.h>
#include <sys/ioctl.h>
#include <sys/mode.h>
#include <sys/mdio.h>
#include <sys/termio.h>

#include <sys/diagex.h>    
#include <sys/inputdd.h>  
#include "diag/atu.h"
#include "salioaddr.h"
#include "misc.h"

#define RINGSIZE 200   /* Size of kbd input ring - for tu40 */

#ifdef DIAGNOSTICS
#include "kbd_tu.h"
#include "diag/diago.h"
#include "diag/modid.h"
#include "dkbd_msg.h"

ASL_SCR_TYPE    menutypes= DM_TYPE_DEFAULTS;

/*
 *      message lists for menus presented via asl during this da.
 */
struct  msglist keyboard_explain[]=
        {
                { KBRD_MSGS, TESTING },
                { KBRD_MSGS, KBD_LEDS },
                (int)NULL
        };

struct  msglist keyboard_no_enter[]=
        {
                { KBRD_MSGS, TESTING },
                { KBRD_MSGS, KBD_NOENTER },
                (int)NULL
        };

struct  msglist ledson_yes_no[]=
        {
                { KBRD_MSGS, TESTING },
                { KBRD_MSGS, SAYYES },
                { KBRD_MSGS, SAYNO },
                { KBRD_MSGS, RUNOVER },
                { KBRD_MSGS, LEDS_ON_OK },
                (int)NULL
        };

struct  msglist ledsoff_yes_no[]=
        {
                { KBRD_MSGS, TESTING },
                { KBRD_MSGS, SAYYES },
                { KBRD_MSGS, SAYNO },
                { KBRD_MSGS, LEDS_OFF_OK },
                (int)NULL
        };

struct  msglist ask_102[]=
        {
                { KBRD_MSGS, TESTING },
/*                { KBRD_MSGS, GERMAN_100 },  */
                { KBRD_MSGS, BRAZIL_104 },
                { KBRD_MSGS, OTHER_102 },
                { KBRD_MSGS, ASK_102 },
                (int)NULL
        };

ASL_SCR_INFO    menu_102[DIAG_NUM_ENTRIES(ask_102)];



struct  msglist keypad_frame[]=
        {
                { KBRD_MSGS, TESTING },
                { KBRD_MSGS, KEYPAD },
                (int)NULL
        };

struct  msglist kbrd_yes_no[]=
        {
                { KBRD_MSGS, TESTING },
                { KBRD_MSGS, SAYYES },
                { KBRD_MSGS, SAYNO },
                { KBRD_MSGS, KEYPAD_WORKED },
                (int)NULL
        };

ASL_SCR_INFO	menu_kbdyn[DIAG_NUM_ENTRIES(kbrd_yes_no)];



struct  msglist speaker_explain[]=
        {
                { KBRD_MSGS, TESTING },
                { KBRD_MSGS, SPEAKER },
                (int)NULL
        };

struct  msglist speaker_no_enter[]=
        {
                { KBRD_MSGS, TESTING },
                { KBRD_MSGS, SPENOENTER },
                (int)NULL
        };

struct  msglist speaker_yes_no[]=
        {
                { KBRD_MSGS, TESTING },
                { KBRD_MSGS, SAYYES },
                { KBRD_MSGS, SAYNO },
                { KBRD_MSGS, RUNOVER },
                { KBRD_MSGS, SPEAKER_WORK },
                (int)NULL
        };

struct  msgtable
{
        struct  msglist *mlp;
        long    msgnum;
        short   msgtype;
        long    err_rc;
};

struct  msgtable msgtab[]=
        {
        	{ keyboard_explain,	0x921014,	1,	(int)NULL },
        	{ keyboard_no_enter,	0x921015,	2,	(int)NULL },
		{ ledson_yes_no,        0x921016,       0,      0x33 },
                { ledsoff_yes_no,       0x921017,       0,      0x34 },
                { ask_102, 	        0x921018,       0,      (int)NULL },
                { keypad_frame,         0x921005,       2,      (int)NULL },
                { kbrd_yes_no,          0x921006,       0,      0x43 },
                { speaker_explain,      0x921011,       1,      (int)NULL },
                { speaker_no_enter,     0x921012,       2,      (int)NULL },
                { speaker_yes_no,       0x921013,       0,      0x63 },
        };

struct  msgtable *mtp;

ulong   menunums[][10] =
        {
                {
                        0x921014,
			0x921015,
                        0x921016,
                        0x921017,
                        0x921018,
                        0x921005,
                        0x921006,
                        0x921011,
                        0x921012,
                        0x921013
                },
                {
                        0x922014,
                        0x922015,
			0x922016,
			0x922017,
			0x922018,
                        0x922005,
                        0x922006,
                        0x922011,
                        0x922012,
                        0x922013
                },
                {
                        0x923014,
                        0x923015,
                        0x923016,
			0x923017,
			0x923018,
                        0x923005,
                        0x923006,
                        0x923011,
                        0x923012,
                        0x923013
                }
        };
#endif

/* Path of interrupt handler for testing using DIAGEX */

#define KBD_DIAG_SLIH_PATH "/usr/lpp/diagnostics/slih/kbd_slih"

uchar rx_kbd_status[5];     /* Kbd data array, used by interrupt handler */

diagex_dds_t kbd_dds;         /* Kbd device dependent structure */
diag_struc_t *kbd_diagex_handle;  /* kbd DIAGEX handle */
int int_handler_kmid = 0; /* holds the kmid for int handler */
uint diagflag;		  /* Flag for setting kbd DD to DIAGNOSTIC DISABLE or */
			  /* ENABLE mode */ 

int diagcounter = 0;     /* Keep track of when we're in kbd diagnostic mode */

/* Globals for kbd device string and kbd file descriptor */
char *kbdtu_string;
int kbdtufd;

/* Declare following variables and arrays for use in TU 40 - kbd echo test */
int keyboard, line, column, linei, columni;
int keypos, mode[133];

char *pat;

/* Array of scan codes for all key positions */
static unsigned char scancode[133] = {0x0e,0x16,0x1e,0x26,0x25,0x2e,0x36,0x3d,0x3e,0x46,
	0x45,0x4e,0x55,0x5d,0x66,0x0d,0x15,0x1d,0x24,0x2d,0x2c,0x35,0x3c,0x43,
	0x44,0x4d,0x54,0x5b,0x5c,0x14,0x1c,0x1b,0x23,0x2b,0x34,0x33,0x3b,0x42,
	0x4b,0x4c,0x52,0x53,0x5a,0x12,0x13,0x1a,0x22,0x21,0x2a,0x32,0x31,0x3a,
	0x41,0x49,0x4a,0x51,0x59,0x11,0x00,0x19,0x29,0x39,0x00,0x58,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x67,0x64,0x00,0x00,0x61,0x6e,
	0x65,0x00,0x63,0x60,0x6f,0x6d,0x00,0x00,0x6a,0x76,0x6c,0x6b,0x69,0x68,
	0x77,0x75,0x73,0x72,0x70,0x7e,0x7d,0x74,0x7a,0x71,0x84,0x7c,0x7b,0x79,
	0x78,0x08,0x00,0x07,0x0f,0x17,0x1f,0x27,0x2f,0x37,0x3f,0x47,0x4f,0x56,
	0x5e,0x57,0x5f,0x62,0x00,0x00,0x00,0x00,0x20,0x28,0x30};

/* This array defines an actual row and column for each key
   position.  The first set contains all row information and the
   second set contains all column information for the keys.  This
   information is used to display the keyboard picture using curses.
*/

static int key[2][133] = {
        {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,
         4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,
         6,6,6,6,6,6,7,7,7,7,7,7,7,3,3,3,3,3,3,3,3,3,3,3,4,5,6,
         7,3,4,5,6,7,3,4,5,6,7,3,4,5,6,7,3,4,5,6,7,3,4,5,6,7,
         3,4,5,6,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,3,3,3,7,7,7},
        {0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,0,3,5,7,9,11,13,15,
         17,19,21,23,25,28,0,4,6,8,10,12,14,16,18,20,22,24,26,28,0,2,
         5,7,9,11,13,15,17,19,21,23,25,28,0,3,6,13,23,25,28,0,0,0,
         0,0,0,0,0,0,0,31,31,31,31,31,32,32,32,32,32,33,33,33,33,33,
         36,36,36,36,36,37,37,37,37,37,38,38,38,38,38,39,39,39,39,39,
         0,2,4,6,8,10,13,15,17,19,22,24,26,28,31,32,33,0,0,0,0,9,17,
         20}};

/* This array gives information about which key positions are used for each
   type of keyboard.  A '1' is for when that key position is used.  The first 
   set is for the '101' keyboard, the second for the '102', and the third for
   the '106' Kanji keyboard.  The fourth set is for Brazilian Krk
   104 key '102' keyboard.  The fifth set is for German Ksk 100 key '102'
   keyboard.
*/

static char indexes[5][133] = {
        {1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
         1,1,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,0,0,0,0,0,
         0,0,0,0,1,1,0,0,1,1,1,0,1,1,1,1,0,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,
         1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,
         1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,0,0,0,0,0,
         0,0,0,0,1,1,0,0,1,1,1,0,1,1,1,1,0,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,
         1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,
         1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,0,0,0,0,0,0,
         0,0,0,0,1,1,0,0,1,1,1,0,1,1,1,1,0,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,
         1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,
         1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,1,0,0,0,0,0,0,
         0,0,0,0,1,1,0,0,1,1,1,0,1,1,1,1,0,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,
         1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,
         1,1,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,0,0,0,0,0,
         0,0,0,0,1,1,0,0,1,1,1,0,1,1,1,1,0,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,
         1,0,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0}};

  /* Following cmds for 101/102 kbds */
static unsigned char kdd_cmds[2][30] = {{0xf0,0x03,0xf3,0x2b,0xfa,0xfc,0x39,0x19
,
                             0x11,0x12,0x59,0x14,0x76,0x58,0xf4,0x00,
                             0,0,0,0,0,0,0,0,0,0,0,0,0,0},

 /* Following cmds for 106 kbd */

                           {0xf3,0x2b,0xfc,0x39,0x19,0x11,0x12,0x59,
                             0x14,0x76,0x58,0x0e,0x20,0x28,0x30,0xfa,
                             0x61, 0x6a, 0x63, 0x60, 0xf4, 0x00,
                             0,0,0,0,0,0,0,0}
                          };

int	ipl_mod;
int	cuat_mod;
int	sio_ctl_reg;

/***************************************************************************
* NOTE: This function is called by the Diagnostic application to invoke a test 
*       unit (TU).  
*                                                                         
*       The 'fd' parameter should be for the /dev/kbd# device.
*                                                                         
***************************************************************************/

int exectu(kbd_name,tucb_ptr)
   char     *kbd_name ;      /* Name of kbd device */
   struct TUCB *tucb_ptr;
   {
     short unsigned int i,j;  /* Loop Index */
     int  rc = SUCCESS;  /* return code */

#ifdef DIAGNOSTICS
     int maketitleadvanced();
     void setmenunumberbase();

     if (tucb_ptr->tuenv.ad_mode == YES)
       (void)maketitleadvanced();

     setmenunumberbase(tucb_ptr->tuenv.kbtype);
#endif

    kbdtu_string = kbd_name;

/* 7012-G30 keyboard/mouse adapter is at slot 7 instead of slot 15 */
	ipl_mod = get_cpu_model (&cuat_mod);
	if (IsPowerPC_SMP (ipl_mod))
		sio_ctl_reg = SIO_CTL_G30_REG;
	else
		sio_ctl_reg = SIO_CONTROL_REG;

    /* Set loop to 1 if preset to 0 */
    if (tucb_ptr->header.loop == 0)
        tucb_ptr->header.loop = 1;

   if (rc == SUCCESS) {

     for (i=0; i<tucb_ptr->header.loop && rc == SUCCESS; i++)
      {
       switch(tucb_ptr->header.tu)
        {  case 0x10:PRINT("Keyboard command reset test - TU 0x10\n");
                     rc = tu10(tucb_ptr->tuenv.mach_fd,tucb_ptr);
                     break;        
           case 0x20:PRINT("Keyboard command tests - TU 0x20\n");
                     rc = tu20(tucb_ptr->tuenv.mach_fd,tucb_ptr);
                     break;      
           case 0x30:PRINT("Keyboard default, indicators, & scan code set tests - TU 0x30\n");
                     rc = tu30(tucb_ptr->tuenv.mach_fd,tucb_ptr);
                     break;     
           case 0x40:PRINT("Echo, keypad tests - TU 0x40\n");
                     rc = tu40(tucb_ptr->tuenv.mach_fd,tucb_ptr);
                     break;    
           case 0x60:PRINT("External speaker frequency, volume test - TU 0x60\n");
                     rc = tu60(tucb_ptr->tuenv.mach_fd,tucb_ptr);
                     break;   
           case 0x99:PRINT("Test cleanup - TU 0x99\n");
                     rc = tu99(tucb_ptr->tuenv.mach_fd);
                     break;    
           default : rc = WRONG_TU_NUMBER;
        }  /* end case */
      }  /* i for loop */

    } /* rc == SUCCESS */

   return(rc);   /* indicate there was no error */
 }  /* End function */

int SendKbData(int fd, unsigned char, unsigned char *);
int SendRxKbData(int fd, unsigned char, unsigned char *, int);

int finish(WINDOW *);
int updatescreen(WINDOW *);
WINDOW *genkeymap();

int SpeakerReadWriteTest(int, ulong, unsigned char);

int tu10(fd, tucb_ptr)
   long     fd ;     /* File descriptor for machine device */
   struct TUCB *tucb_ptr;
{
  unsigned char data;	
  int rc = SUCCESS;
  unsigned char kbddata[4];

  /* Setup to use DIAGEX */
  rc = start_diagex(fd);

  if (rc != SUCCESS) {
      return(rc);
  }

 /* Send kbd reset command */
  if ((rc = SendRxKbData(fd, (unsigned char)KBD_RESET_CMD, kbddata, 2)) != SUCCESS) { 
   rc = 0x12;
   cleanup(fd);
   return(rc);
  }

  if (kbddata[0] != KBD_ACK)	/* get Acknowledge first  */	
   rc = 0x11;
  else
  {
   if (kbddata[1] != KBD_BAT_CC)
    rc = 0x13;
  }

 /* Set kbd back with normal conditions */
  cleanup(fd);

return(rc);
}

int tu20(fd, tucb_ptr)
   long     fd ;     /* File descriptor for machine device */
   struct TUCB *tucb_ptr;
{
  unsigned char data;	
  int rc = SUCCESS;
  unsigned char kbddata[4];

  /* Setup to use DIAGEX */
  rc = start_diagex(fd);

  if (rc != SUCCESS) {
      return(rc);
  }

  /* send 'read keyboard id' command */ 
  data = 0xF2;	 
  if ((rc = SendRxKbData(fd, data, kbddata, 3)) != SUCCESS) { 
   rc = 0x22;
   cleanup(fd);
   return(rc);
  }

 /* get Acknowledge first  */	
  if(kbddata[0] != KBD_ACK)
  {
    rc = 0x21;
    cleanup(fd);
    return(rc);
  }

 /* Get keyboard id */

  if (kbddata[1] == 0xAB && kbddata[2] == 0x83)  /* 101 or 102 keyboard */
  {
   /* Send 'layout id' command to keyboard */
    data = 0xEF;
    if ((rc = SendRxKbData(fd, data, kbddata, 3)) != SUCCESS) {
     rc = 0x22;
     cleanup(fd);
     return(rc);
    }
   /* get Acknowledge first  */	
    if(kbddata[0] != KBD_ACK)
    {
      rc = 0x21;
      cleanup(fd);
      return(rc);
    }

   /* Get keyboard layout id bytes */
    if (kbddata[2] != 0xBF) {
     rc = 0x24;
     cleanup(fd);
     return(rc);
    }
  } /* if 101 or 102 keyboard */

  else {   /* It should be a Japanese 106 keyboard */
    if (kbddata[1] != 0xBF || kbddata[2] != 0xB2) 
     rc = 0x23;
  }

 /* Set kbd back with normal conditions */
 cleanup(fd);

 return(rc);
}  

/* Kbd default, indicators and scan code TU */

int tu30(fd, tucb_ptr)
   long     fd ;     /* file descriptor for using machine DD */
   struct TUCB *tucb_ptr;
{
  unsigned char data, sdata[10];	
  int rc = SUCCESS;
  unsigned char kbddata[4];

#ifdef DIAGNOSTICS
  int  mrc; /* Message return code */
#endif

  /* Clear out kbd buffer */
  clear_buffer(fd);

/* Prompt user to watch for keyboard lamps to turn on and then turn off */

  while (1) {

#ifdef SUZTESTING
   PRINT("\nWatch for all keyboard lamps to turn on and then turn off.  ");
   PRINT("Hit <Enter> when ready to start test : ");
   scanf("%c",&data);
#endif

#ifdef DIAGNOSTICS

  mtp = &msgtab[0];    

  mrc = putmsg(tucb_ptr,mtp);  
  if (mrc != 0) {
     if (mrc < 0)
       return(0x30);
     else
       return(mrc);
  }
    /* Remove F3 and F10 choices at bottom of screen if user presses
       enter key.  Otherwise, display same menu information. */

  mtp = &msgtab[1];    
  mrc = putmsg(tucb_ptr,mtp);
  if (mrc != 0) {
     if (mrc < 0)
	return(0x30);
     else
         return(mrc);
  }
#endif


  /* Setup to use DIAGEX */
  rc = start_diagex(fd);

  if (rc != SUCCESS) {
      return(rc);
  }

 /* Turn on keyboard LED's */

 /* Send 'set/reset mode indicators' command to keyboard */
  data = 0xed; 
  if ((rc = SendRxKbData(fd, data, kbddata, 1)) != SUCCESS) {
   rc = 0x32;
   cleanup(fd);
   return(rc);
  }
  if(kbddata[0] != KBD_ACK)
  {
    rc = 0x31;
    cleanup(fd);
    return(rc);
  }

 /* Send option byte '0x07' (Turn on all indicator lamps) */
  data = 0x07;
  if ((rc = SendRxKbData(fd, data, kbddata, 1)) != SUCCESS) 
   rc = 0x32;

 if (rc == SUCCESS) {
  if(kbddata[0] != KBD_ACK) {
    rc = 0x31;
  }

  sleep(3);     /* Give user a chance to view kbd LED's */
 }

 /* send 'read keyboard id' command */
  data = 0xF2;
  SendRxKbData(fd, data, kbddata, 3);

  if (kbddata[1] == 0xAB && kbddata[2] == 0x83)  /* 101 or 102 keyboard */
  {

   /* Set keyboard scan code to 1 test */

    /* Send 'select scan code' command to keyboard */
    if (rc == SUCCESS) {
     data = 0xf0;
     if ((rc = SendRxKbData(fd, data, kbddata, 1)) != SUCCESS)
      rc = 0x32;
    }

    if (rc == SUCCESS) {
    /* get keyboard acknowledge */
     if(kbddata[0] != KBD_ACK)
       rc = 0x31;
    }

    /* Send option byte '0x01' (Select scan code 1) */
    if (rc == SUCCESS) {
     data = 0x01;
     if ((rc = SendRxKbData(fd, data, kbddata, 1)) != SUCCESS)
      rc = 0x32;
    }

    /* get keyboard acknowledge */
    if (rc == SUCCESS) {
     if(kbddata[0] != KBD_ACK)
       rc = 0x31;
    }

    /* Verify current scan code set */
    if (rc == SUCCESS) {
     data = 0xf0;
     if ((rc = SendRxKbData(fd, data, kbddata, 1)) != SUCCESS)
      rc = 0x32;
    }

    /* get keyboard acknowledge */
    if (rc == SUCCESS) {
     if(kbddata[0] != KBD_ACK)
       rc = 0x31;
    }

    /* Send option byte '0x00' (scan set query) to keyboard */
    if (rc == SUCCESS) {
     data = 0x00;
     if ((rc = SendRxKbData(fd, data, kbddata, 2)) != SUCCESS)
      rc = 0x32;
    }

    /* get keyboard acknowledge */
    if (rc == SUCCESS) {
     if(kbddata[0] != KBD_ACK)
       rc = 0x31;
    }

    /* Read byte indicating current scan code */
    if (rc == SUCCESS) {
     if (kbddata[1] != 0x01)
       rc = 0x35;
    }
   }

 /* Send command to re-initialize basic default conditions - (scan code 2 and */
 /* LED's off) */

/* Reset keyboard - This seems to be the only thing that works */
 
  SendRxKbData(fd, (unsigned char)KBD_RESET_CMD, kbddata, 2);
  usleep(10 * 1000);

 /* send 'read keyboard id' command */
  data = 0xF2;
  SendRxKbData(fd, data, kbddata, 3);

 if (kbddata[1] == 0xAB && kbddata[2] == 0x83)  /* 101 or 102 keyboard */
 {
   /* Test 'scan set query' command and verify current scan code set */

   if (rc == SUCCESS) {
    data = 0xf0;
    if ((rc = SendRxKbData(fd, data, kbddata, 1)) != SUCCESS) 
     rc = 0x32;
   }

   /* get keyboard acknowledge */	
   if (rc == SUCCESS) {
    if(kbddata[0] != KBD_ACK)
      rc = 0x31;
   }

   /* Send option byte '0x00' (scan set query) to keyboard */
   if (rc == SUCCESS) {
    data = 0x00;
    if ((rc = SendRxKbData(fd, data, kbddata, 2)) != SUCCESS) 
     rc = 0x32;
   }

   /* get keyboard acknowledge */	
   if (rc == SUCCESS) {
    if(kbddata[0] != KBD_ACK)
      rc = 0x31;
   }

   /* Read byte indicating current scan code */
    if (rc == SUCCESS) {
     if (kbddata[1] != 0x02)
       rc = 0x36;
    }
 }

  /* Call cleanup to close DIAGEX and start up kbd DD so user can select DA
     menu options */

 cleanup(fd);

 /* Prompt user if the keyboard lamps worked correctly */

 if (rc == SUCCESS) {

#ifdef SUZTESTING
   PRINT("\nDid all keyboard lamps turn on? (y/n) : ");
   scanf("%s",&sdata);
   PRINT("\n\n");
   if (strcmp(sdata,"n") == 0) {
      rc = 0x33;
      return(rc);
   }

   PRINT("\nAre all keyboard lamps currently turned off? (y/n) : ");
   scanf("%s",&sdata);
   PRINT("\n\n");
   if (strcmp(sdata,"n") == 0) {
      rc = 0x34;
   }
   return(rc);
#endif

#ifdef DIAGNOSTICS
   mtp = &msgtab[2];    
   mrc = putmsg(tucb_ptr,mtp);  /* Ask if LEDs turned on properly */ 

   if (mrc != YES) {
     if (mrc == RETRY)
        continue;
     if (mrc < 0)
       return(0x30);
     else if (mrc == CANCEL_KEY_ENTERED || mrc == EXIT_KEY_ENTERED)
       return(mrc);
     else
      rc = 0x33;
   }
#endif
  }

   break;
 } /* while */


#ifdef DIAGNOSTICS

 if (rc == SUCCESS) {   /* Ask if LEDs are turned off */
   mtp = &msgtab[3];    
   mrc = putmsg(tucb_ptr,mtp);  

   if (mrc != YES) {
     if (mrc < 0)
       return(0x30);
     else if (mrc == CANCEL_KEY_ENTERED || mrc == EXIT_KEY_ENTERED)
       return(mrc);
     else
      rc = 0x34;
   }
 }
#endif

 return(rc);
} 

/* The two following functions are used by tu40 routine */

/*-------------------------------------------------------------------*/
/* inputsig - process input ring event notification                  */
/*-------------------------------------------------------------------*/

void inputsig()
{

  (int) signal (SIGMSG, inputsig);       /* re-arm signal handler    */

}

/*-------------------------------------------------------------------*/
/* gotinput - process input ring events in TU 0x40                   */
/*-------------------------------------------------------------------*/
/* Input arguments:

   *ir          : ptr to struct inputring
   *quit        : ptr to quit - tell calling function whether we have
                  quit or not
   *w           : ptr to WINDOW - for updating keyboard picture
   fd           : file descriptor for kbd device
*/

void gotinput(struct inputring *ir, char *quit, WINDOW *w, long fd)
{
   struct ir_kbd rr;
   int i;
   static int odd = 0;
   char *sce;
   char *sink;
   int rc;

   while(ir->ir_head != ir->ir_tail) {

     sce = (char *) ir->ir_head;
     sink = (char *) &rr;

     for(i=0;i<sizeof(struct ir_kbd);i++) {
        *sink = *sce;
        sink++;
        sce++;
        if (sce >= ((char *)(ir) + RINGSIZE)) {
           sce = (char *) (ir) + sizeof(struct inputring);
        }
     }

     ir->ir_head = (caddr_t) sce;

     if (odd) {  /* Since for every keystroke there are two data reports, */
		 /* we set up to just look at every other report received. */

       keypos = rr.kbd_keypos;
       updatescreen(w);   /* Update keyboard picture with new key info */

       /* If user hits ESC, quit */
       if (rr.kbd_keypos == 110) *quit = 1;  /* quit if Esc */

       odd = 0;
     } /* if odd */

     else odd = 1;

   } /* while */

   if (ir->ir_overflow == IROVERFLOW) {
      PRINT("<overflow reset > \n");
      rc = ioctl(fd, KSRFLUSH, NULL);
   }
}

/* Echo, keypad test TU */

int tu40(fd, tucb_ptr)
   long     fd ;    /* file descriptor for using machine DD */
   struct TUCB *tucb_ptr;
{
  unsigned char data;
  char sdata[10];
  unsigned char kbddata[4];
  int i, rc = SUCCESS;
  WINDOW *w;
  struct inputring *ir;                   /* input ring                       */
  struct uregring reg;            /* register ring structure */
  char quit;

 /* for new menu stuff to support new kbds */
  char    msgstr[512];

    /* Part # for new German kbd */
  char    gerpart[] = "(Currently not available)";  
   /* Part # for new Brazilian kbd */  
  char    brazpart[] = "88G3936"; 


#ifdef DIAGNOSTICS
  int  mrc; /* Message return code */
#endif

  (int) signal (SIGMSG, inputsig);       /* arm signal handler    */

  /* Setup to use DIAGEX */
  rc = start_diagex(fd);

  if (rc != SUCCESS) {
      return(rc);
  }

/* Reset keyboard */
  if ((rc = SendRxKbData(fd, (unsigned char)KBD_RESET_CMD, kbddata, 2)) != SUCCESS)
  {
   rc = 0x42;
   cleanup(fd);
   return(rc);
  }
  usleep(10 * 1000);

  if(kbddata[0] != KBD_ACK) {
   rc = 0x41;
   cleanup(fd);
   return(rc);
  }

 /* send 'read keyboard id' command */
  data = 0xF2;
  if ((rc = SendRxKbData(fd, data, kbddata, 3)) != SUCCESS) {
   rc = 0x42;
   cleanup(fd);
   return(rc);
  }

 /* get Acknowledge first  */
  if(kbddata[0] != KBD_ACK)
  {
    rc = 0x41;
    cleanup(fd);
    return(rc);
  }

 /* Get keyboard id */
  if (kbddata[1] == 0xAB && kbddata[2] == 0x83)  /* 101 or 102 keyboard */
  {
   /* Send 'layout id' command to keyboard */
    data = 0xEF;
    if ((rc = SendRxKbData(fd, data, kbddata, 3)) != SUCCESS) {
     rc = 0x42;
     cleanup(fd);
     return(rc);
    }
   /* get Acknowledge first  */
    if(kbddata[0] != KBD_ACK)
    {
      rc = 0x41;
      cleanup(fd);
      return(rc);
    }

   /* Get keyboard layout id bytes */
    if (kbddata[1] == 0xB0)
     keyboard = 0;               /* 101 keyboard */

    if (kbddata[1] == 0xB1)
     keyboard = 1;               /* 102 keyboard */

  } /* if 101 or 102 keyboard */

  else if (kbddata[1] == 0xBF && kbddata[2] == 0xB2)
    keyboard = 2;           /* 106 Kanji keyboard */

  else
  {
    rc = 0x45;      /* Unknown keyboard - out of order */
    cleanup(fd);
    return(rc);
  }

 /* Send keyboard echo command */

  data = 0xee;
  if ((rc = SendRxKbData(fd, data, kbddata, 1)) != SUCCESS) {
   rc = 0x42;
   cleanup(fd);
   return(rc);
  }
 /* Check keyboard response of '0xee' */
  if(kbddata[0] != 0xee)
  {
    rc = 0x43;
    cleanup(fd);
    return(rc);
  }

 /* Clear mode array - Each item of this array corresponds to a key
    position for the keyboard.  Whenever a key is pressed, the mode for that
    key will be toggled from to '0' to '1', and from '1' back to '0'.
    This is done in the updatescreen() function.
 */

  for (i = 0; i < 133; ++i)
    mode[i] = 0;

 /* Call cleanup to close DIAGEX and restore kbd DD operation - get ready for keypad test */

  cleanup(fd);

  clear_buffer(fd);

 /* If keyboard is type '102', ask if it's a Brazilian (Krk) or German
    (Ksk) keyboard. */

  if (keyboard == 1)
  {

#ifdef DIAGNOSTICS
     memset (menu_102, 0, sizeof (menu_102));
     mtp = &msgtab[4];   /* ptr to ask_102 */

     /* Do following stuff to insert part #'s into the ask_102 msg's */

     /* Copy msg strings from ask_102 struct to menu_102 */
     diag_display(mtp->msgnum, tucb_ptr->tuenv.catd,mtp->mlp, DIAG_MSGONLY,
            ASL_DIAG_KEYS_ENTER_SC, &menutypes, menu_102);
 
/* The following is commented out for now, since there is no new German kbd.

     sprintf(msgstr, menu_102[1].text, gerpart);
     free (menu_102[1].text);
     menu_102[1].text = (char *) malloc (strlen(msgstr)+1);
     strcpy (menu_102[1].text, msgstr);
*/
     
  /* When/If the above stuff gets uncommented and added back into the ask_102
     structure, the following 4 lines should get changed to use 
     "menu_102[2].text" instead.
  */
     sprintf(msgstr, menu_102[1].text, brazpart);
     free (menu_102[1].text);
     menu_102[1].text = (char *) malloc (strlen(msgstr)+1);
     strcpy (menu_102[1].text, msgstr);

     /* Set indexes so that when menu is displayed, the last (default) choice
	will be highlighted.  Ex: if you want the 3rd and last choice to be
	highlighted, you must have cur_index = 3, max_index = # of items in
	the msg struct (including the NULL entry)  */

/*     menutypes.cur_index = 3;  */
     menutypes.cur_index = 2;  /* For now, there will only be two menu items
				since there is no new Ger. kbd */  

    /* Ask what kind 102 keyboard - Put up the menu */
     mrc = chk_stat(diag_display(mtp->msgnum,tucb_ptr->tuenv.catd,NULL,
			DIAG_IO, ASL_DIAG_LIST_CANCEL_EXIT_SC,
			&menutypes, menu_102));

     if (mrc < 0)
       return(0x40);
     if (mrc == CANCEL_KEY_ENTERED || mrc == EXIT_KEY_ENTERED)
       return(mrc);
/*     if (menutypes.cur_index == 1) */  /* German kbd */
/*       keyboard = 4; */
/*     if (menutypes.cur_index == 2) */  /* Brazil kbd */
     if (menutypes.cur_index == 1)   /* Brazil kbd - currently 1st choice */
				     /* in the menu */
       keyboard = 3;

#endif

#ifdef SUZTESTING
  PRINT("\nAre you using a Brazilian keyboard? (y/n) ");
  scanf("%s",&sdata);
  if (strcmp(sdata,"y") == 0)
     keyboard = 3;
#endif

  } /* if keyboard == 1 */


 /* Prompt user to hit keys on the keyboard */
#ifdef SUZTESTING
  PRINT("\n\nPlease READ all instructions below before continuing...\n\n\n\n");
  PRINT("\nTest keyboard echo response by randomly pressing keys on the \n");
  PRINT("keyboard.  The correct keyboard echo response for each key pressed can be confirmed \n");
  PRINT("by watching that the corresponding key is highlighted on the keyboard picture.\n");
  PRINT("Be sure to press the 'Esc' key when you want to quit this test.\n");
  PRINT("Press <Enter> when you are ready to start the test.\n");
  scanf("%c",&data);
  sdata[0] = 'O';
  pat = &sdata[0];
#endif

 /* Display keyboard menu and obtain character to display the keyboard
    keymap from the catalog file */

#ifdef DIAGNOSTICS
 mtp = &msgtab[5];
 (void)putmsg(tucb_ptr,mtp);
 pat = (char *)diag_cat_gets(tucb_ptr->tuenv.catd, KBRD_MSGS,KBPAT);
#endif

 /* Clear keyboard RX buffer register, if there is any data */

  clear_buffer(fd);

   /* Open kbd device */
   if ( (kbdtufd = open(kbdtu_string, O_RDWR)) < 0 )
   {
     PRINT("Couldn't open kbd device %s\n",kbdtu_string);
     return(0x200);
   }

  /* Set up to use input ring service of kbd DD */

   ir = (struct inputring *) malloc(RINGSIZE);
   if (!ir) {
      PRINT("kbd input ring malloc failed \n");
      return(0x200);
   }

  /* Initialize reg structure */
   reg.ring = (caddr_t) ir;
   reg.size = RINGSIZE;
   reg.report_id = 0xa5;

  /* Initialize ring_buffer */

   ir->ir_size = RINGSIZE - sizeof(struct inputring);
   ir->ir_head = (caddr_t) ir + sizeof(struct inputring);
   ir->ir_tail = ir->ir_head;
   ir->ir_notifyreq = IRSIGEMPTY;
   ir->ir_overflow = IROFCLEAR;

   rc = ioctl(kbdtufd, KSREGRING, &reg);
   if (rc != 0 )
   {
     PRINT("Register input ring failed, rc = %d, errno = %d\n",rc,errno);
     return(0x200);
   }

 /* Generate the keyboard picture */
 w = genkeymap();


  if (rc == SUCCESS)
  {

   /* Get keyboard data until user escapes */

      quit = 0;
      while (!quit) {     /* while ESC not pressed */
         ir->ir_notifyreq = IRSIGALWAYS;
         rc = sleep(10);
         if (rc > 0) {
            ir->ir_notifyreq = IRSIGEMPTY;
            gotinput(ir,&quit,w,kbdtufd);
         }
      }

  }  /* if rc == SUCCESS */

  /* Disable input from device to ring */

  ioctl(kbdtufd,KSREGRING, NULL);

  close(kbdtufd);

 /* Free memory for inputring */
  free(ir); 

  rc = SUCCESS;

 /* Ask user how test went */
#ifdef SUZTESTING
  PRINT("\n\n\n\n       Did this test complete with your satisfaction? (y/n) ");
  scanf("%s",&sdata);
  if (strcmp(sdata,"n") == 0)
     rc = 0x44;
#endif

#ifdef DIAGNOSTICS
  /* Call special_display to solve the menutypes global problem, since
     special_display will initialize its own local copy.  */
  mrc = special_display(tucb_ptr);

  if (mrc != YES) {
   if (mrc < 0)
     return(0x40);
   else if (mrc == CANCEL_KEY_ENTERED || mrc == EXIT_KEY_ENTERED)
     return(mrc);
   else
    rc = 0x44;
  }
#endif

 return(rc);
} 

int tu60(fd, tucb_ptr)
int fd;   /* File descriptor for machine device */
struct TUCB *tucb_ptr;
{
  unsigned short i;
  unsigned char Status, reg1, reg2, data, hi, lo;
  ushort freq;    /* frequency in Hz */
  int count = 0, rc = SUCCESS;
  int  mrc; /* Message return code */
  char sdata[10];

  /* Save contents of speaker register 0 */
  if ((rc = rd_byte(fd, &reg1, SPK_CTRL0_REG)) != SUCCESS) {
    PRINT("Unsuccessful read from SPK_CTRL0_REG\n");
    rc = 0x61;
    return(rc);
  }

  /* Save contents of speaker register 1 */
  if ((rc = rd_byte(fd, &reg2, SPK_CTRL1_REG)) != SUCCESS) {
    PRINT("Unsuccessful read from SPK_CTRL1_REG\n");
    rc = 0x61;
    return(rc);
  }
 
 /* First clear keyboard buffers before continuing */
  clear_buffer(fd);

  while (1) {

#ifdef DIAGNOSTICS
    mtp = &msgtab[7]; 
    /* Explain speaker test to user */

    mrc = putmsg(tucb_ptr,mtp);
    if (mrc != 0) {
       if (mrc < 0)
         return(0x60);
       else
         return(mrc);
    }
    
    /* Remove F3 and F10 choices at bottom of screen if user presses
       enter key.  Otherwise, display same menu information. */

    mtp = &msgtab[8]; 
    mrc = putmsg(tucb_ptr,mtp);
    if (mrc != 0) {
       if (mrc < 0)
	 return(0x60);
       else
         return(mrc);
    }
#endif

  if (rc == SUCCESS)
  {
      /* Low frequency, high volume test */
       freq = 1000000 / (100 * 4);  /* 100 Hz */
       hi = (uchar)freq;
       freq >>= 8;
       lo = reg2;
       lo &= ~0x1F;
       lo |= (uchar) ((uchar)freq & 0x1F);
       lo |= 0xe0;  /* Enable speaker with high volume */
       SpeakerReadWriteTest(fd, SPK_CTRL0_REG, hi);
       SpeakerReadWriteTest(fd, SPK_CTRL1_REG, lo);

       sleep(2);		

     /* Low frequency, low volume test */
       freq = 1000000 / (100 * 4);  /* 100 Hz */
       hi = (uchar)freq;
       freq >>= 8;
       lo &= 0xa0;  /* Set speaker volume low, bit 6 off */
       lo |= (uchar) ((uchar)freq & 0x1F);
       lo |= 0xa0;  /* Enable speaker with low volume */
       SpeakerReadWriteTest(fd, SPK_CTRL0_REG, hi);
       SpeakerReadWriteTest(fd, SPK_CTRL1_REG, lo);

       sleep(4);		

       /* High frequency, high volume test */

       freq = 1000000 / (5000 * 4);  /* 10,000 Hz */
       hi = (uchar)freq;
       freq >>= 8;
       lo &= ~0x1F;
       lo |= (uchar) ((uchar)freq & 0x1F);
       lo |= 0xe0;  /* Enable speaker with high volume */
       SpeakerReadWriteTest(fd, SPK_CTRL0_REG, hi);
       SpeakerReadWriteTest(fd, SPK_CTRL1_REG, lo);

       sleep(6);		
   }

  /* Restore contents of speaker register 0 */
   wr_byte(fd, &reg1, SPK_CTRL0_REG);

  /* Restore contents of speaker register 1 */
  wr_byte(fd, &reg2, SPK_CTRL1_REG);

  /* Query user if tones sounded correct */

#ifdef DIAGNOSTICS
   mtp = &msgtab[9]; 
   mrc = putmsg(tucb_ptr,mtp);
   if (mrc != YES) {
      if (mrc == RETRY)
	continue;
      if (mrc == NO)
	return(0x63);
      if (mrc < 0)
	return(0x60);
      else
	return(mrc);
   }
#endif

#ifdef SUZTESTING
  PRINT("\n\n\n		Did it sound OK? (y/n)  ");
  scanf("%s",&sdata);
    if (strcmp(sdata,"n") == 0)
       return(0x63);
    else
	return(SUCCESS);
#endif

  break;
 } /* while */

 return(rc);
}

/* Called by TU 60 to send data to kbd speaker registers and verify the
   correctness of data */

int SpeakerReadWriteTest(int fd, ulong addr, unsigned char data)
{
  unsigned char tmp;
  int rc = SUCCESS;
	
  rc = wr_byte(fd, &data, addr);

  if (rc == SUCCESS)
  {
   rc = rd_byte(fd, &tmp, addr);
   if (data != tmp)
     rc = SPK_LOGIC_ERROR;
  }
  else 
  {
    PRINT("Unsuccessful writing %x to %d\n",data,addr);
    rc = -1; 
  }

  return(rc);
}

/* TU 0x99 - Close DIAGEX and restore kbd DD normal operation.  Called from
   within an interrupt handler of calling application. */ 

int tu99(long machfd)
{
  int rc = SUCCESS;
  struct cfg_load cfg_ld;
  char  ddpath[512];

  sprintf(ddpath, "%s/%s", (char *)getenv("DIAGX_SLIH_DIR"), "kbd_slih");
  cfg_ld.path = ddpath;
  cfg_ld.libpath = NULL;
  cfg_ld.kmid = 0;
  errno = 0;

   /* Make sure the semaphore is released by process */
   rel_sem();

  /* Grab the semaphore before accessing POS2 */
  rc = set_sem(1);

  if (rc != SUCCESS) {
    PRINT("Couldn't capture semaphore, exit'ing without cleanup\n");
    return(rc);
  }

  if (diagcounter > 0)
  {
    disable_kbd(machfd);
    close_diagex();
    diagflag = KSDDISABLE;
    rc = ioctl(kbdtufd,KSDIAGMODE,&diagflag);
    diagcounter--;
  }

  /* Unload interrupt handler from kernel */
  sysconfig(SYS_QUERYLOAD, (void *)&cfg_ld, (int)sizeof(cfg_ld));
  if (cfg_ld.kmid)
  {
     rc = sysconfig(SYS_KULOAD, (void *)&cfg_ld, (int)sizeof(cfg_ld));
  }

  /* Now release the semaphore after accessing POS2 */
  rc = rel_sem();

  close(kbdtufd);

  return (rc);
}

/* Close DIAGEX and restore kbd DD normal operation - same as tu99() */

int cleanup(long machfd)
{
  int rc = SUCCESS;

   /* Make sure the semaphore is released by process */
   rel_sem();

  /* Grab the semaphore before accessing POS2 */
  rc = set_sem(1);

  if (rc != SUCCESS) {
    PRINT("Couldn't capture semaphore, exit'ing without cleanup\n");
    close(kbdtufd);
    return(rc);
  }

  if (diagcounter > 0)
  {
    disable_kbd(machfd);
    close_diagex();
    diagflag = KSDDISABLE;
    rc = ioctl(kbdtufd,KSDIAGMODE,&diagflag);
    diagcounter--;
  }

  /* Now release the semaphore after accessing POS2 */
  rc = rel_sem();
  close(kbdtufd);

  return (rc);
}

#define MAX_SEM_RETRIES 5
#define POS_SEMKEY      0x3141592

/***************************************************************************
 FUNCTION NAME: set_sem(wait_time)

   DESCRIPTION: This function access the POS register 2 via the IPC
                semaphore which should be used by all programs desiring
                the access. The user passes in a "wait_time" to specify
                whether or not to "wait_forever" for the semaphore or
                retry MAX_SEM_RETRIES with a wait_interval of "wait_time"

   NOTES:       This function is used basically to overcome the concurrent
                access of the POS register 2 by the Keyboard/Tablet,
                Diskette, Serial Port 1 and 2, Parallel Port and Mouse
                devices.

***************************************************************************/

int set_sem(wait_time)
int wait_time;
{
        int             semid;
        long            long_time;
        struct sembuf   sembuf_s;
        int             retry_count = (MAX_SEM_RETRIES -1);
        int             rc;
        static          first_time = 1;

        /* Obtain semaphore ID and create it if it does not already
         *  exist */

        semid = semget( (key_t) POS_SEMKEY, 1, IPC_CREAT | IPC_EXCL | S_IRUSR |
                        S_IWUSR | S_IRGRP | S_IWGRP);

        if (semid < 0)
        {
          /* If the error from semget() reveals that we failed for any
           * reason other than the fact that the semaphore already
           * existed, indicate error and return */

           if (errno != EEXIST)
                return(0x201);

          /* Semaphore already exists. Because it already exists it is
           * possible that it was JUST created by another process so
           * let's sleep here for a few clock cycles to let the other
           * process initialize everything properly */

           if (first_time)
           {
                sleep(4);
                first_time = 0;
           }
        /* That should be enough time, so get the semaphore ID without
         * CREATion flags */

        semid = semget( (key_t) POS_SEMKEY, 1, S_IRUSR | S_IWUSR | S_IRGRP |
                        S_IWGRP);

        /* Make sure that we got a valid semaphore ID, else return error */

        if (semid < 0)
            return(0x201);
      }
        else
      {
        /* Semaphore was nearly created so we need to initialize our
         * semaphore value */

        if (semctl(semid, 0, SETVAL, 1))
        {
                return(0x201);
        }
      }

        /* At this point, we have our semaphore ID and is it was the
         * first instance, it had been created and initialized for
         * use */

        /* Indicate semaphore number */

        sembuf_s.sem_num = 0;

        /* Set op to -1 indicating that we want to grab the semaphore */

        sembuf_s.sem_op = -1;

        /* If a non-negative wait_time was passed in, then indicate that
         * we do not want the process to be blocked if the semaphore is
         * unavailable. Note the SEM_UNDO flag. By including this, the
         * semaphore will get properly released should this process be
         * terminated by a signal or something */

        if (wait_time >= 0)
                sembuf_s.sem_flg = IPC_NOWAIT | SEM_UNDO;
        else
                sembuf_s.sem_flg = SEM_UNDO;

        /* See if we can get the semaphore. If the semaphore is available
         * then it has a value of 1 which will get decremented to a value
         * of 0 since our sem_op = -1. Else the semaphore is not available
         * (thus a value of 0).*/

        while (retry_count > -1)
        {
                rc = semop(semid, &sembuf_s, 1);
                if (rc == 0)
                {
                  /* Got it, so return */

                  return(SUCCESS);
                }
            if (errno == EAGAIN)
                {
        /* Semaphore held by someone else, but we indicated not to wait
         * forever. If user specified wait_time of zero, then just
         * return unsuccessfully without retries */

                        if (wait_time == 0)
                        return(0x201);

        /* Sleep for the time specified by the user and then retry */

                        sleep(wait_time);
                        retry_count --;
                }
                else
                        return(0x201);
        }
         return(0x201);
}

/***************************************************************************
 FUNCTION NAME: rel_sem()

   DESCRIPTION: This function releases the semaphore indicating
                completion of access to POS register 2 by that particular
                device.
   NOTES:

***************************************************************************/

int rel_sem()
{
        int             semid;
        struct sembuf   sembuf_s;
        int             pid;
        int             rc;
        int             sempid;

        /* Obtain semaphore ID and create it if it does not already
         *  exist */

        semid = semget( (key_t) POS_SEMKEY, 1, S_IRUSR |  S_IWUSR | S_IRGRP |
                         S_IWGRP);

        /* Make sure that we got a valid semaphore ID, else return
         * error */

        if (semid < 0)
           return(0x201);

        /* Now, we want to make sure that we do not attempt to release
         * the semaphore if we don't already have it. This ensures that
         * the semaphore value remains < 1, therefore binary */

        /* First, get the current process ID */

        pid = getpid();

        /* Next, get the process ID of the process which currently has
         * the semaphore */

        sempid = semctl(semid, 0, GETPID, 0);
        if (sempid < 0)
           return(0x201);

        /* If the current process ID does not equal the semaphore's
         * process ID, then we are not holding it so return an error */

        if (pid != sempid)
           return(0x201);

        /* Release the semaphore by handing it a positive value which
         * get added to the semaphore value indicating that it is now
         * available. Note the SEM_UNDO flag. By including this, the
         * semaphore will get properly handled should this process be
         * terminated by a signal or something. */

        sembuf_s.sem_num = 0;
        sembuf_s.sem_op = 1;
        sembuf_s.sem_flg = SEM_UNDO;
        rc = semop(semid, &sembuf_s, 1);

        return(rc);
}


/* Use curses to generate keyboard picture and update the screen 
   in functions genkeymap(), updatescreen(), and finish() */

WINDOW *genkeymap()
{
   int j;
   WINDOW *window;

   linei = 1;
   columni = 5;
#ifdef SUZTESTING
   initscr();     /* Comment out this line when dropping code to bos */
#endif
   window = newwin(10,50,13,15);
   wcolorout(window,Bxa);
   cbox(window);
   wcolorend(window);
   for(j = 0; j < 133; j++)
   {
     if (indexes[keyboard][j] == 1)
       mvwaddch(window,linei+key[0][j], columni+key[1][j],pat[0]);
   }

/* Place cursor in lower right hand corner of the box */

   wmove(window,8,48);
   wrefresh(window);
   return(window);
}

updatescreen(WINDOW *win)
{
  int chmode;
  int x,y;

  if (keypos == 110)    /* This is 'ESC' key position 110 */
  {
    getyx(win,y,x);
    wmove(win,(linei+key[0][keypos-1]), (columni+key[1][keypos-1]));
    wchgat(win,1,STANDOUT);
    wmove(win,y,x);
    wrefresh(win);
    sleep(1);
    werase(win);
    wrefresh(win);
    delwin(win);
#ifdef SUZTESTING
    endwin();        /* Comment out this line when dropping to bos */
#endif
    return(1);
  }

  /* Make sure that the key to be filled in on the screen exists on the
     keyboard currently being tested */
  if (indexes[keyboard][keypos-1] == 1)
   {
     getyx(win,y,x);
     wmove(win,(linei+key[0][keypos-1]), (columni+key[1][keypos-1]));
     if (mode[keypos-1] == 0)
     {
       mode[keypos-1] = 1;
       chmode = STANDOUT;    /* Key is highlighted */
     }
     else {
       mode[keypos-1] = 0;
       chmode = NORMAL;     /* Highlighted removed */
     }
     wchgat(win,1,chmode);
     wmove(win,y,x);
     wrefresh(win);
   }
  return(0);
}

finish(WINDOW *win)
{
  werase(win);
  wrefresh(win);
  delwin(win);
#ifdef SUZTESTING
  endwin();         /* Comment out this line when dropping to bos */
#endif
}

/* This function sends data, which is stored in the 'value' parameter,
   to the keyboard.  It also returns the ACK '0xFA' byte from kbd via
   kbddata */

int SendKbData(int fd, unsigned char value, unsigned char *kbddata)
{
  unsigned int count, pollcount=20;
  unsigned char Status, data;
  int rc = SUCCESS;
  int inttimeout;
	
  clear_buffer(fd);

  kbddata[0] = 0;
  kbddata[1] = 0;
  kbddata[2] = 0;
  kbddata[3] = 0;


   if ((rc = clear_buffer(fd)) != SUCCESS) {   /* Make sure kbd buffer clear */
	    return(rc);
   }

  /* Set global interrupt ptr contents to 0 */
   memset(rx_kbd_status, 0, sizeof(uchar) * 5);

                                       /*       send command/data           */
   if ((rc = wr_byte(fd, &value, KBD_DATA_REG)) != SUCCESS) {
           PRINT("Unsuccessful writing %x to KBD_DATA_REG\n",value);
           return(rc);
   }

   /* Get byte of returned kbd data */

   inttimeout = 600;
   while ((rx_kbd_status[1] == 0) && --inttimeout) {
            usleep(1000);  /* sleep 1000 usec. between checks */
   }

   kbddata[0] = rx_kbd_status[1];

   if (inttimeout == 0) {
           PRINT("Timed out getting kbd interrupt data, rx_kbd_status[1] = %x\n",rx_kbd_status[1]);
   }

  return(rc);

}

/* This function sends data, provided by 'value' parameter, to the keyboard,
   and it puts received data from keyboard into the kbddata array. 
   The last parameter 'num' is for the number of bytes to poll for. */

int SendRxKbData(int fd, unsigned char value, unsigned char *kbddata, int num)
{  
  unsigned int count, pollcount = 20;
  unsigned char Status, data;
  int j,rc = SUCCESS;
  int inttimeout;

  clear_buffer(fd);  

  kbddata[0] = 0;
  kbddata[1] = 0;
  kbddata[2] = 0;
  kbddata[3] = 0;

   if ((rc = clear_buffer(fd)) != SUCCESS) {   /* Make sure kbd buffer clear */
	return(rc);
   }

 /* Set global interrupt ptr contents to 0 */
   memset(rx_kbd_status,0, sizeof(uchar) * 5); 


                                       /*       send command/data           */
  if ((rc = wr_byte(fd, &value, KBD_DATA_REG)) != SUCCESS) {
            PRINT("Unsuccessful writing %x to KBD_DATA_REG\n",value);
            return(rc);
  }

  for (j = 0; j < num; j++) {  /* Poll for "num" number of kbd data bytes */
	
   /* Get byte of returned kbd data */

	   inttimeout = 600;
	   while ((rx_kbd_status[j+1] == 0) && --inttimeout) {
	      usleep(1000);  /* sleep 1000 usec. between checks */
	   }

           kbddata[j] = rx_kbd_status[j+1];

           if (inttimeout == 0) {
             PRINT("Timed out getting kbd interrupt data, rx_kbd_status[%d] = %x\n" ,j+1,rx_kbd_status[j+1]);
             return(-1);
           }

  } /* for j */
	   
  return (rc);
}

/* Takes down kbd DD and calls open_diagex. */

int start_diagex(long machfd)
{
  int rc = SUCCESS;

   /* Open kbd device */
   if ( (kbdtufd = open(kbdtu_string, O_RDWR)) < 0 )
   {
     PRINT("Couldn't open kbd device %s\n",kbdtu_string);
     return(0x200);
   }

   /* Grab the semaphore before accessing POS2 */
   rc = set_sem(1);

   if (rc != SUCCESS) {
     PRINT("Couldn't capture semaphore, exit'ing test\n");
     close(kbdtufd);
     return(rc);
   }

  PRINT("Disabling kbd DD\n");
  errno = 0;
  diagflag = KSDENABLE;
  rc = ioctl(kbdtufd,KSDIAGMODE,&diagflag);

  if (rc != SUCCESS)
  {
     PRINT("Enabling diagnostic mode KSDENABLE failed, errno = %d, rc = %d\n",errno,rc);
     rc = 0x200;
     rel_sem();
     close(kbdtufd);
     return(rc);
  }
  diagcounter++;

  rc = open_diagex();
  if (rc != SUCCESS)
  {
     diagflag = KSDDISABLE;
     ioctl(kbdtufd,KSDIAGMODE,&diagflag);
     diagcounter--;
     rc = 0x200;
     rel_sem();
     close(kbdtufd);
     return(rc);
  }

  enable_kbd(machfd);

  /* Now release the semaphore after accessing POS2 */
   rc = rel_sem();

  return(rc);

}

/* Make sure kbd Tx and Rx buffers are ready before sending kbd data */
 
int clear_buffer(fd)
int fd;   /* File descriptor for machine DD */
{
   int i = 0, rc = SUCCESS;
   uchar data;
   
   if ((rc = rd_byte(fd, &data, KBD_STAT_CMD_REG)) != SUCCESS) {
      PRINT("Unsuccessful read from KBD_STAT_CMD_REG\n");
      return(rc);
   }

  /* Make sure keyboard TX & RX buffers are ready before sending more keyboard 
     data */

   while ( ( (data & KBD_RX_BUF_FULL) || (!(data & KBD_TX_BUF_EMPTY)) ) && (i < 200))
   {
      if ((rc = rd_byte(fd, &data, KBD_DATA_REG)) != SUCCESS) {
         PRINT("Unsuccessful read from KBD_DATA_REG\n");
         return(rc);
      }
      usleep(25 * 1000);

      if ((rc = rd_byte(fd, &data, KBD_STAT_CMD_REG)) != SUCCESS) {
         PRINT("Unsuccessful read from KBD_STAT_CMD_REG\n");
         return(rc);
      }
      usleep(25 * 1000);

      ++i;
   }

   if ( i == 200 ) {  /* Not able to clear kbd TX buffer */
	PRINT("Unable to clear kbd TX buffer\n");
	rc = -1;
   }

   return(rc);

}

/* Make sure kbd adapter is enabled */

int enable_kbd(fd)
int fd;   /* File descriptor for machine DD */
{
  uchar cdata;
  int rc = SUCCESS;

  /* Read the current setting of SIO control reg (POS2) */
   if ((rc = rd_pos(fd, &cdata, sio_ctl_reg)) != SUCCESS) {
    PRINT("Unsuccessful read of one byte from SIO_CONTROL_REG\n");
    return(rc);
   }

  /* Take keyboard adapter out of reset */
   cdata = cdata & ~0x04; 
   if ((rc = wr_pos(fd, &cdata, sio_ctl_reg)) != SUCCESS) {
    PRINT("Unsuccessful write of one byte to SIO_CONTROL_REG\n");
    return(rc);
   }

   usleep(500*1000);

 /* Read again the current setting of SIO control reg (POS2) */
   if ((rc = rd_pos(fd, &cdata, sio_ctl_reg)) != SUCCESS) {
    PRINT("Unsuccessful read of one byte from SIO_CONTROL_REG\n");
    return(rc);
   }

  /* If keyboard adapter still in reset state */

   if (cdata & 0x04) {
        PRINT("POS2 still in reset for keyboard, POS2 : %2x\n",cdata);
        rc = -1;
        return(rc);
   }

   return(rc);

} /* enable_kbd */

/* Make sure kbd adapter is disabled before closing DIAGEX */

int disable_kbd(fd)
int fd;   /* File descriptor for machine DD */
{
  uchar cdata;
  int rc = SUCCESS;

  /* Read the current setting of SIO control reg (POS2) */
   if ((rc = rd_pos(fd, &cdata, sio_ctl_reg)) != SUCCESS) {
    PRINT("Unsuccessful read of one byte from SIO_CONTROL_REG\n");
    return(rc);
   }

  /* Put keyboard adapter in reset */
   cdata = cdata | 0x04;
   if ((rc = wr_pos(fd, &cdata, sio_ctl_reg)) != SUCCESS) {
    PRINT("Unsuccessful write of one byte to SIO_CONTROL_REG\n");
    return(rc);
   }

  return(rc);

} /* disable_kbd */


/* This function loads up test kbd interrupt handler (if not already loaded)
   and opens DIAGEX */

int open_diagex(void)
{
  int rc = 0;
  struct cfg_load cfg_ld;
  char  ddpath[512];

  PRINT("open diagex routine\n");
  /* load interrupt handler */
  sprintf(ddpath, "%s/%s", (char *)getenv("DIAGX_SLIH_DIR"), "kbd_slih");
  cfg_ld.path = ddpath;
  cfg_ld.libpath = NULL;
  cfg_ld.kmid = 0;

  sysconfig(SYS_QUERYLOAD, (void *)&cfg_ld, (int)sizeof(cfg_ld));

  if (!(cfg_ld.kmid))
  {
    errno = 0;
    rc = sysconfig(SYS_SINGLELOAD, (void *)&cfg_ld, (int)sizeof(cfg_ld));
    if (rc != SUCCESS) {
       PRINT("\n\nLOAD ./slih FAILED, errno = %d, rc =%d\n\n",errno,rc);
       rc = 0x200;
       return(rc);
    } else {
       PRINT("\n\nLOADED ./slih \n\n");
       PRINT("\nKMID = %d \n",cfg_ld.kmid);
       int_handler_kmid = cfg_ld.kmid;
    } /* endif */
  }

  /* Clear all 5 bytes. 1st byte to store index of current byte being processed
     by interrupt handler.  Up to four bytes of data returned from kbd after a 
     command is sent. */

  memset(rx_kbd_status, 0, sizeof(uchar) * 5);

  /* initialize Device Dependent Structure info */
  rc = get_kbd_dds();
  if (rc) {
    return(rc);
  } /* endif */

  /* open DIAGEX */
  errno = 0;
  rc = diag_open( &kbd_dds, &kbd_diagex_handle );

  if (rc != SUCCESS) {
    PRINT("diag_open failed, rc = %x, errno = %d\n",rc,errno);
    rc = 0x200;
  }

  return(rc);

} /* end open_diagex() */

/* This function closes DIAGEX */

int close_diagex(void)
{
  int rc = 0;

  errno = 0;
  rc = diag_close(kbd_diagex_handle);

  if (errno != SUCCESS) {
    PRINT("diag_close failed, rc = %d, errno = %d\n",rc,errno);
    rc = 0x200;
  }

  return(rc);

} /* end close_diagex() */


/* This function puts kbd dds info into the kbd_dds structure for use with
   DIAGEX.  Some of this info is hardcoded due to not being able to locate
   in ODM. (Any ideas??) */

int get_kbd_dds(void)
{
  int rc = 0;

  /* initialize Device Dependent Structure info */

  kbd_dds.slot_num = 0;
  kbd_dds.bus_intr_lvl = 1;
  kbd_dds.intr_priority = INTCLASS3;
  kbd_dds.intr_flags = 0;
  kbd_dds.dma_lvl = 0;
  kbd_dds.bus_io_addr = 0xf0000000;
  kbd_dds.bus_io_length = 0x60;
  kbd_dds.bus_mem_addr = 0;
  kbd_dds.bus_mem_length = 0;
  kbd_dds.dma_bus_mem = 0;
  kbd_dds.dma_bus_length = 0;
  kbd_dds.dma_flags = 0;
  kbd_dds.bus_id = 0x820c0060;
  kbd_dds.bus_type = BUS_MICRO_CHANNEL;
  kbd_dds.kmid = int_handler_kmid;
  kbd_dds.data_ptr = rx_kbd_status;
  kbd_dds.d_count = 5;
  kbd_dds.maxmaster = 0;

  return(rc);

} /* end get_kbd_dds() */


/* This function uses the machine device driver to read one byte from
   the specified address, returning the information to pdata */

int rd_byte(fd, pdata, addr)
int fd;
unsigned char *pdata;
unsigned int addr;
{
  MACH_DD_IO iob;
  int rc;

  iob.md_data = pdata;
  iob.md_incr = MV_BYTE;
  iob.md_size = sizeof(*pdata);
  iob.md_addr = addr;

  rc = ioctl(fd, MIOBUSGET, &iob);
/*
  PRINT("Read byte = %2X\n",*pdata);
  PRINT("Read addr = %4X\n",addr);
*/

  return (rc);
}
/* This function uses the machine device driver to write one byte from
   pdata to the specified address */

int wr_byte(fd, pdata, addr)
int fd;
unsigned char *pdata;
unsigned int addr;
{
  MACH_DD_IO iob;
  int rc;

  iob.md_data = pdata;
  iob.md_incr = MV_BYTE;
  iob.md_size = sizeof(*pdata);
  iob.md_addr = addr;

  rc = ioctl(fd, MIOBUSPUT, &iob);
/*
  PRINT("Write byte = %2X\n",*pdata);
  PRINT("Write addr = %4X\n",addr);
*/

  return (rc);
}

/* This function uses the machine device driver to write one byte from
   pdata to the specified pos register address */

int wr_pos(fd, pdata, addr)
int fd;
unsigned char *pdata;
unsigned int addr;
{
  MACH_DD_IO iob;
  int rc;

  iob.md_data = pdata;
  iob.md_incr = MV_BYTE;
  iob.md_size = sizeof(*pdata);
  iob.md_addr = addr;

  rc = ioctl(fd, MIOCCPUT, &iob);
/*  printf("Write byte = %2X\n",*pdata);
  printf("Write addr = %4X\n",addr);
*/
  return (rc);
}

/* This function uses the machine device driver to read one byte from
   the specified pos register, returning the information to pdata */

int rd_pos(fd, pdata, addr)
int fd;
unsigned char *pdata;
unsigned int addr;
{
  MACH_DD_IO iob;
  int rc;

  iob.md_data = pdata;
  iob.md_incr = MV_BYTE;
  iob.md_size = sizeof(*pdata);
  iob.md_addr = addr;

  rc = ioctl(fd, MIOCCGET, &iob);
/*  printf("Read byte = %2X\n",*pdata);
  printf("Read addr = %4X\n",addr);
*/
  return (rc);
}


#ifdef DIAGNOSTICS
/*
 * NAME:
 *       maketitleadvanced
 *
 * FUNCTION:
 *      This function alters the message id values for the global
 *      message lists when running in advanced mode.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is called as a function by main.
 *
 * (NOTES:)
 *      This function:
 *      Sets the msglist msgids to the value for TESTAM, which will
 *      cause the title Testing in advanced mode to be displayed instead
 *      of just Testing.
 *
 * (RECOVERY OPERATION:)
 *      None needed.
 *
 * (DATA STRUCTURES:)
 *      This function modifies the following global structures and variables:
 *      All structures of type msglist.
 *
 * RETURNS:
 *      None.
 */ 

maketitleadvanced()
{
        keyboard_explain[0].msgid = TESTAM;
	ledson_yes_no[0].msgid = TESTAM;
        ledsoff_yes_no[0].msgid = TESTAM;
        keypad_frame[0].msgid = TESTAM;
        kbrd_yes_no[0].msgid = TESTAM;
        speaker_explain[0].msgid = TESTAM;
        speaker_no_enter[0].msgid = TESTAM;
        speaker_yes_no[0].msgid = TESTAM;
}

/*
 * NAME:
 *       setmenunumberbase
 *
 * FUNCTION:
 *      This function alters the menu number values for the global
 *      message lists when running with different keyboards.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is called as a function by main.
 *
 * (NOTES:)
 *      This function:
 *      Sets the msgtab msgnum to the value for the type of keyboard
 *      being tested.
 *
 * (RECOVERY OPERATION:)
 *      None needed.
 *
 * (DATA STRUCTURES:)
 *      This function modifies the following global structures and variables:
 *      All msgtab msgnum values.
 *
 * RETURNS:
 *      None.
 */ 

void    setmenunumberbase(kbtype)
int     kbtype;
{
        int     i;
        ulong   modifier;

        for ( i=0; i< (sizeof msgtab / sizeof(struct msgtable)); ++i )
                msgtab[i].msgnum = menunums[kbtype][i];
}
/*
 * NAME:
 *       putmsg
 *
 * FUNCTION:
 *      This function displays a menu of some type dependent on an input
 *      parameter.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is called as a function by main.
 *
 * (NOTES:)
 *      This function:
 *      Displays the requested menu, waiting for user response if it is
 *      required.
 *
 * (RECOVERY OPERATION:)
 *      Software errors are handled by the call to the chk_stat function.
 *
 * (DATA STRUCTURES:)
 *      None altered.
 *
 * RETURNS:
 *      0 if everything is okay.
 *      -1 in the event that the menu was not able to display.
 *      EXIT_KEY_ENTERED if user presses exit key.
 *      CANCEL_KEY_ENTERED if user presses cancel key.
 */ 

putmsg( tucb_ptr,mtp )
struct  TUCB    *tucb_ptr;
struct  msgtable *mtp;
{
        int     rc;

        menutypes.cur_index=YES;
        switch ( mtp->msgtype )
        {
                case 0:

                        rc = chk_stat(diag_display(mtp->msgnum,
				tucb_ptr->tuenv.catd,mtp->mlp,DIAG_IO,
                                ASL_DIAG_LIST_CANCEL_EXIT_SC,
				&menutypes, NULL));
                        if ( rc != 0 )
                                return(rc);
                        else
                        if (TRUE)   /* added to fool lint */
                                return(menutypes.cur_index);
                    else
                                break;

                case 1:

                        rc = chk_stat(diag_display(mtp->msgnum,
				tucb_ptr->tuenv.catd,mtp->mlp,DIAG_IO,
				ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, NULL));
                        if ( rc != 0 )
                                return(rc);

                        break;

                case 2:

                        rc = chk_stat(diag_display(mtp->msgnum,
				tucb_ptr->tuenv.catd,mtp->mlp,DIAG_IO,
				ASL_DIAG_LEAVE_NO_KEYS_SC,
                                NULL, NULL));
                        if ( rc != 0 )
                                return(rc);

                        break;

                default:
                        break;
        }
        return(0);
}

/*
 * NAME:
 *      chk_stat
 *
 * FUNCTION:
 *      Handles the return code from an asl or diag_asl procedure call.
 *
 * EXECUTION ENVIRONMENT:
 *      This function is called by the putmsg procedure which is called as
 *      a function from main.
 *
 * (NOTES:)
 *      This function acts on the return code from an asl or diag_asl
 *      procedure call.
 *
 * (RECOVERY OPERATION:)
 *      None.
 *
 * (DATA STRUCTURES:)
 *      None.
 *
 * RETURNS:
 *      0 if okay.
 *      -1 if menu fails to display.
 *      EXIT_KEY_ENTERED if user presses exit key.
 *      CANCEL_KEY_ENTERED if user presses cancel key.
 */

chk_stat(returned_code)
long    returned_code;
{
        if ( returned_code == DIAG_ASL_CANCEL )
                return( CANCEL_KEY_ENTERED );
        else
                if ( returned_code == DIAG_ASL_EXIT )
                        return( EXIT_KEY_ENTERED );
                else
                        if ( returned_code == DIAG_MALLOCFAILED )
                                return(-1);
                        else
                                return(0);
}

/* Special function used by tu40 */

int special_display(tucb_ptr)
  struct TUCB *tucb_ptr;
{

  ASL_SCR_TYPE    menutypes2= DM_TYPE_DEFAULTS;
  int mrc;

  mtp = &msgtab[6];

  /* Copy msg strings from kbrd_yes_no struct to menu_kbdyn */
  /* This should cause the menu's default highlight to be on the 'YES' choice */

  diag_display(mtp->msgnum, tucb_ptr->tuenv.catd,mtp->mlp, DIAG_MSGONLY,
               ASL_DIAG_KEYS_ENTER_SC, &menutypes2, menu_kbdyn);


  /* Ask if keyboard keypad test ran OK - Put up the menu */
  mrc = chk_stat(diag_display(mtp->msgnum,tucb_ptr->tuenv.catd,NULL,
                 DIAG_IO, ASL_DIAG_LIST_CANCEL_EXIT_SC,
		 &menutypes2, menu_kbdyn));

  if (mrc != 0) {
     return(mrc);
  }

  return(menutypes2.cur_index);

}
#endif
