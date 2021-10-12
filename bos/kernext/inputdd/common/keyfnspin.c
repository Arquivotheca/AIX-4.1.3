static char sccsid[] = "@(#)56  1.8  src/bos/kernext/inputdd/common/keyfnspin.c, inputdd, bos41J, 9510A_all 3/7/95 10:05:06";
/*
 * COMPONENT_NAME: (INPUTDD) Keyboard DD - keyfnspin.c
 *
 * FUNCTIONS: keyproc, shift_status, proc_event, proc_sak, un_sak,
 *            put_key, poll_appl, appl_killer, key_stat, kbdleds,
 *            sv_proc
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "ktsm.h"
#include "common.h"
#include "kbdext.h"

#ifdef GS_DEBUG
#define  CTLALTNUM(a)                                                        \
    {                                                                        \
    int aa = a;                                                              \
    printf("ctrl-alt-num pad key %d \n", aa);                                \
    }
#else
#define  CTLALTNUM(a) ctlaltnum(a)
#endif


/*****************************************************************************/
/*                                                                           */
/* NAME:        keyproc                                                      */
/*                                                                           */
/* FUNCTION:    process scan codes                                           */
/*                                                                           */
/* INPUTS:      com = pointer to common structure                            */
/*              key = pointer to keyboard extension                          */
/*              scan_code = scan code received from keyboard                 */
/*                                                                           */
/* OUTPUTS:     none                                                         */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

void keyproc(struct common *com, struct kbdext *key, uchar scan_code)
{

   uchar pos_code;
   char  *key_state;
   ushort old_status;

   KTSMDTRACE1(keyproc, enter, scan_code);

   if (scan_code == BREAK_CODE)        /* if scan code is break code         */
      key->break_code_rcv = TRUE; /*   remember that break code received*/

   else {                              /* else                               */

                                       /* if PS/2 kbd scan code coming in    */
                                       /* convert to RS kbd's scan code      */
      switch(scan_code) {
        case 0x5F:                     /* scroll lock                        */
                                       /* if space saver keyboard            */
          if ((key->nonum) && (key->kbdstat & SHIFT))
            scan_code = 0x76;          /* change scan code to NumLock        */
          break;
        case 0x85:  scan_code = 0x20;  break;
        case 0x86:  scan_code = 0x28;  break;
        case 0x87:  scan_code = 0x30;  break;
        default:  break;
      }
                                       /*   if scan code is in the valid     */
                                       /*   range then convert to position   */
                                       /*   code                             */
      if (scan_code <= LARGEST_SCAN_CODE) {

                                       /* if space saver kbd and num lock is */
                                       /* on, then ten-key emulation         */

         if ( (key->nonum) && (key->kbdstat & NUM_LOCK)) {

           switch (scan_code) {
             case 0x3D: scan_code = 0x6C; break; /* '7'        */
             case 0x3E: scan_code = 0x75; break; /* '8'        */
             case 0x46: scan_code = 0x7D; break; /* '9'        */
             case 0x45: scan_code = 0x77; break; /* '/'        */
             case 0x3C: scan_code = 0x6B; break; /* '4'        */
             case 0x43: scan_code = 0x73; break; /* '5'        */
             case 0x44: scan_code = 0x74; break; /* '6'        */
             case 0x4D: scan_code = 0x7E; break; /* '*'        */
             case 0x3B: scan_code = 0x69; break; /* '1'        */
             case 0x42: scan_code = 0x72; break; /* '2'        */
             case 0x4B: scan_code = 0x7A; break; /* '3'        */
             case 0x4C: scan_code = 0x84; break; /* '-'        */
             case 0x3A: scan_code = 0x70; break; /* '0'        */
             case 0x49: scan_code = 0x71; break; /* '.'        */
             case 0x4A: scan_code = 0x7C; break; /* '+'        */
             case 0x54:
               if (key->special_106) {
                 scan_code = 0x79;               /* Enter      */
               }
               break;
             default: break;
           }
         }


         pos_code = scan_to_posi[scan_code];

         if (pos_code) {               /*   if position code valid           */

                                       /*     pointer to entry in state tble */
            key_state = &key->kbd_state_table[pos_code];

                                       /*     clear repeat status            */
            key->kbdstat &= ~REPEAT;

                                       /*     if key was released then       */
            if (key->break_code_rcv) {
                                       /*       clear break code rcv'ed flag */
               key->break_code_rcv = FALSE;
                                       /*       if break not to be processed */
               if (*key_state != KEY_DOWN) {
                  *key_state = KEY_UP; /*          update state table        */
                  return;              /*          exit                      */
               }
                                       /*       clear make status flag       */
               key->kbdstat &= ~M_B;
                                       /*         update key state table     */
               *key_state = KEY_UP;    /*         (key is now up)            */
            }
            else {                     /*     else (key was pressed)         */

                                       /*       if repeat not to be processed*/
               if (*key_state == KEY_IGN)
                  return;              /*          exit                      */

                                       /*       set make status flag         */
               key->kbdstat |= M_B;
                                       /*       if key already down then     */
               if (*key_state)         /*         must be repeating          */
                  key->kbdstat |= REPEAT;
               else                    /*       else                         */
                                       /*         update key status array    */
                                       /*         (key is now down)          */
                  *key_state = KEY_DOWN;
            }
                                       /*     update shift status            */
            shift_status(com, key, pos_code);

            if (key->sak_enabled) /*     process SAK if enabled         */
               proc_sak(key, pos_code, scan_code);
            else                       /*     else                           */
                                       /*       finish processing key        */
               proc_event(key, pos_code, scan_code);

         }
      }
   }
}



/*****************************************************************************/
/*                                                                           */
/* NAME:        shift_status                                                 */
/*                                                                           */
/* FUNCTION:    This module keeps track of the shift status of the           */
/*              keyboard                                                     */
/*                                                                           */
/* INPUTS:      com = pointer to common structure                            */
/*              key = pointer to keyboard extension                          */
/*              pos_code = position code of pressed key                      */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

void  shift_status(struct common *com, struct kbdext *key, uchar pos_code)
{
   ushort  kbdstat;
   char    update_leds;
   uchar   led;

   kbdstat  = key->kbdstat;

   KTSMDTRACE1(shift_status, enter, kbdstat);

                                       /* driver updates CapsLock and        */
                                       /* NumLock keyboard leds only if      */
                                       /* kernel owns channel                */
   if (key->ccb[key->act_ch].owner_pid == KERNEL_PID)
      update_leds = TRUE;
   else
      update_leds = FALSE;

                                       /* switch on keyboard type            */
   switch (key->kbd_type)
   {
      case KS106:                      /*    case: 106 keyboard              */


         switch (pos_code)             /*       switch on position code      */
         {
            case ALPHA_POS_CODE:       /*          case: alpha key           */
               if (kbdstat & M_B) {    /*             if key being pressed   */
                                       /*                if Alt and Ctrl     */
                                       /*                are not down then   */
                  if ( !(kbdstat & ALT) && !(kbdstat & CNTL))
                                       /*                   reset katakana   */
                     kbdstat &= ~KATAKANA;
                                       /*                if Alt is down      */
                                       /*                and  not Korean     */
                                       /*                keyboard then its   */
                                       /*                Caps Lock           */
                  if ((kbdstat & ALT) &&
                      (key->special_106 != KOREAN_MAP))
                  {
                                       /*                   if we are        */
                                       /*                   updating leds    */
                     if (update_leds)

                                       /*                      toggle caps   */
                                       /*                      lock status   */
                        kbdstat ^= CAPS_LOCK;
                  }
               }
               break;

            case HIRAGANA_POS_CODE:    /*          case: hiragana key        */

                                       /*             if Alt and Ctrl are not*/
                                       /*             down and key pressed   */
               if (!(kbdstat & ALT) && !(kbdstat & CNTL) && (kbdstat & M_B))
               {
                                       /*                set katakana status */
                  kbdstat |= KATAKANA;
               }

               break;


            case KATAKANA_POS_CODE:    /*          case: katakana key        */

               if (kbdstat & M_B) {    /*             if key being pressed   */
                                       /*                if Korean keyboard  */
                  if (key->special_106 == KOREAN_MAP)
                  {
                                       /*                   if we are        */
                                       /*                   updating leds    */
                     if (update_leds)
                                       /*                      toggle caps   */
                                       /*                      lock status   */
                        kbdstat ^= CAPS_LOCK;
                  }
                  else                 /*                else                */
                  {                    /*                 if Alt and Ctrl    */
                                       /*                 are down then      */
                     if (!(kbdstat & ALT) && !(kbdstat & CNTL))
                                       /*                   set kata. status */
                        kbdstat |= KATAKANA;
                  }
               }

               break;


            case L_SHIFT_POS_CODE:     /*          case: left shift          */
               if (kbdstat & M_B)      /*             if it is a make        */
               {
                                       /*                set l shift status  */
                  kbdstat |= L_SHIFT;
                  kbdstat |= SHIFT;    /*                set shift status    */
               }
               else                    /*             else (its a break)     */
               {
                                       /*                clear l shift status*/
                  kbdstat &= ~L_SHIFT;
                                       /*                if r shift not set  */
                  if ( !(kbdstat & R_SHIFT))
                                       /*                   clr shift status */
                     kbdstat &= ~SHIFT;
               }
               break;


            case R_SHIFT_POS_CODE:     /*          case: right shift         */
               if (kbdstat & M_B)      /*             if it is a make        */
               {
                                       /*                set r shift status  */
                  kbdstat |= R_SHIFT;
                  kbdstat |= SHIFT;    /*                set shift status    */
               }
               else                    /*             else (its a break)     */
               {
                                       /*                clear r shift status*/
                  kbdstat &= ~R_SHIFT;
                                       /*                if l shift not set  */
                  if ( !(kbdstat & L_SHIFT))
                                       /*                   clr shift status */
                     kbdstat &= ~SHIFT;
               }
               break;


            case R_ALT_POS_CODE:       /*          case: right alt           */
               if (kbdstat & M_B)      /*             if it is a make        */
               {
                  kbdstat |= R_ALT;    /*                set r alt status    */
                  kbdstat |= ALT;      /*                set alt status      */
               }
               else                    /*             else (its a break)     */
               {
                  kbdstat &= ~R_ALT;   /*                clear r alt status  */
                  kbdstat &= ~ALT;     /*                clear alt stat      */
               }
               break;


            case CNTL_POS_CODE:        /*          case: control             */
               if (kbdstat & M_B)      /*             if it is a make        */
                  kbdstat |= CNTL;     /*                set control status  */
               else                    /*             else (its a break)     */
                  kbdstat &= ~CNTL;    /*                clear control status*/
               break;


            case NUM_LOCK_POS_CODE:    /*          case: num lock            */
                                       /*             if its a make and we   */
                                       /*             are updating leds and  */
                                       /*             Alt & Ctrl are both up */
               if ((kbdstat & M_B) &&
                   update_leds  &&
                   !(kbdstat & ALT) && !(kbdstat & CNTL))
               {
                                       /*                toggle num lock     */
                                       /*                status              */
                  kbdstat ^= NUM_LOCK;
               }
               break;

            default:                   /*          default:                  */
               break;                  /*             do nothing             */
         }
         break;

      case KSPS2:
                                       /* if not 106 keyboard,               */
                                       /* go down to default                 */
         if (key->special_106) {
            switch (pos_code) {

              case HIRAGANA_POS_CODE:  /*          case: hiragana key        */
                                       /*             if Alt and Ctrl are not*/
                                       /*             down and key pressed   */
                if (!(kbdstat & ALT) && !(kbdstat & CNTL) && (kbdstat & M_B)) {
                                       /*                set katakana status */
                  kbdstat |= KATAKANA;
                }
                break;

              case L_SHIFT_POS_CODE:   /*          case: left shift          */
                if (kbdstat & M_B) {   /*             if it is a make        */
                                       /*                set l shift status  */
                  kbdstat |= L_SHIFT;
                  kbdstat |= SHIFT;    /*                set shift status    */
                }
                else {                 /*             else (its a break)     */
                                       /*             clear l shift status   */
                  kbdstat &= ~L_SHIFT;
                                       /*                if r shift not set  */
                  if ( !(kbdstat & R_SHIFT))
                                       /*                   clr shift status */
                    kbdstat &= ~SHIFT;
                }
                break;

              case R_SHIFT_POS_CODE:   /*          case: right shift         */
                if (kbdstat & M_B) {   /*             if it is a make        */
                                       /*                set r shift status  */
                  kbdstat |= R_SHIFT;
                  kbdstat |= SHIFT;    /*                set shift status    */
                }
                else {                 /*             else (its a break)     */
                                       /*                clear r shift status*/
                  kbdstat &= ~R_SHIFT;
                                       /*                if l shift not set  */
                  if ( !(kbdstat & L_SHIFT))
                                       /*                   clr shift status */
                    kbdstat &= ~SHIFT;
                }
                break;

              case L_ALT_POS_CODE:     /*          case: left alt            */
                if (kbdstat & M_B) {   /*             if it is a make        */
                  kbdstat |= L_ALT;    /*                set l alt status    */
                  kbdstat |= ALT;      /*                set alt status      */
                }
                else {                 /*             else (its a break)     */
                  kbdstat &= ~L_ALT;   /*                clear l alt status  */
                                       /*                if r alt not set    */
                  if ( !(kbdstat & R_ALT))
                                       /*                   clear alt status */
                    kbdstat &= ~ALT;
                }
                break;

              case R_ALT_POS_CODE:     /*          case: right alt           */
                if (kbdstat & M_B) {   /*             if it is a make        */
                  kbdstat |= R_ALT;    /*                set r alt status    */
                  kbdstat |= ALT;      /*                set alt status      */
                }
                else {                 /*             else (its a break)     */
                  kbdstat &= ~R_ALT;   /*                clear r alt status  */
                                       /*                if l alt not set    */
                  if ( !(kbdstat & L_ALT))
                                       /*                   clear alt stat   */
                    kbdstat &= ~ALT;
                }
                break;


              case CNTL_POS_CODE:      /*          case: control             */
                if (kbdstat & M_B)     /*             if it is a make        */
                  kbdstat |= CNTL;     /*                set control status  */
                else                   /*             else (its a break)     */
                  kbdstat &= ~CNTL;    /*               clear control status */
                break;

              case CAPS_LOCK_POS_CODE: /*          case: caps lock           */

                if (kbdstat & M_B) {   /*          if key being pressed      */
                                       /*          if Alt and Ctrl and Shift */
                                       /*                are not down then   */
                  if ( !(kbdstat & SHIFT) && !(kbdstat & ALT) &&
                       !(kbdstat & CNTL))
                                       /*                   reset katakana   */
                    kbdstat &= ~KATAKANA;
                                       /*                if Shift is down    */
                                       /*                then its Caps Lock  */
                  if (kbdstat & SHIFT) {
                                       /*                   if we are        */
                                       /*                   updating leds    */
                    if (update_leds)
                                       /*                      toggle caps   */
                                       /*                      lock status   */
                      kbdstat ^= CAPS_LOCK;
                  }
                }
                break;

              case NUM_LOCK_POS_CODE:  /*          case: num lock            */
                                       /*             if its a make and we   */
                                       /*             are updating leds and  */
                                       /*             Alt & Ctrl are both up */
                if ((kbdstat & M_B) &&
                    (update_leds)  && !(kbdstat & ALT) && !(kbdstat & CNTL)) {
                                       /*                toggle num lock     */
                                       /*                status              */
                  kbdstat ^= NUM_LOCK;
                }
                break;

              default:                 /*          default:                  */
                break;                 /*             do nothing             */
            }
            break;
        }


      default:                         /*    default: (101 or 102 keyboard)  */

         switch (pos_code)             /*       switch on position code      */
         {
            case L_SHIFT_POS_CODE:     /*          case: left shift          */
               if (kbdstat & M_B)      /*             if it is a make        */
               {
                                       /*                set l shift status  */
                  kbdstat |= L_SHIFT;
                  kbdstat |= SHIFT;    /*                set shift status    */
               }
               else                    /*             else (its a break)     */
               {
                                       /*                clear l shift status*/
                  kbdstat &= ~L_SHIFT;
                                       /*                if r shift not set  */
                  if ( !(kbdstat & R_SHIFT))
                                       /*                   clr shift status */
                     kbdstat &= ~SHIFT;
               }
               break;


            case R_SHIFT_POS_CODE:     /*          case: right shift         */
               if (kbdstat & M_B)      /*             if it is a make        */
               {
                                       /*                set r shift status  */
                  kbdstat |= R_SHIFT;
                  kbdstat |= SHIFT;    /*                set shift status    */
               }
               else                    /*             else (its a break)     */
               {
                                       /*                clear r shift status*/
                  kbdstat &= ~R_SHIFT;
                                       /*                if l shift not set  */
                  if ( !(kbdstat & L_SHIFT))
                                       /*                   clr shift status */
                     kbdstat &= ~SHIFT;
               }
               break;


            case L_ALT_POS_CODE:       /*          case: left alt            */
               if (kbdstat & M_B)      /*             if it is a make        */
               {
                  kbdstat |= L_ALT;    /*                set l alt status    */
                  kbdstat |= ALT;      /*                set alt status      */
               }
               else                    /*             else (its a break)     */
               {
                  kbdstat &= ~L_ALT;   /*                clear l alt status  */
                                       /*                if r alt not set    */
                  if ( !(kbdstat & R_ALT))
                                       /*                   clear alt status */
                     kbdstat &= ~ALT;
               }
               break;


            case R_ALT_POS_CODE:       /*          case: right alt           */
               if (kbdstat & M_B)      /*             if it is a make        */
               {
                  kbdstat |= R_ALT;    /*                set r alt status    */
                  kbdstat |= ALT;      /*                set alt status      */
               }
               else                    /*             else (its a break)     */
               {
                  kbdstat &= ~R_ALT;   /*                clear r alt status  */
                                       /*                if l alt not set    */
                  if ( !(kbdstat & L_ALT))
                                       /*                   clear alt stat   */
                     kbdstat &= ~ALT;
               }
               break;


            case CNTL_POS_CODE:        /*          case: control             */
               if (kbdstat & M_B)      /*             if it is a make        */
                  kbdstat |= CNTL;     /*                set control status  */
               else                    /*             else (its a break)     */
                  kbdstat &= ~CNTL;    /*                clear control status*/
               break;


            case CAPS_LOCK_POS_CODE:   /*          case: caps lock           */
                                       /*             if its a make and in   */
                                       /*             ksr mode               */
               if ((kbdstat & M_B) && update_leds)
               {
                                       /*                toggle caps lock    */
                                       /*                status              */
                  kbdstat ^= CAPS_LOCK;
               }
               break;

            case NUM_LOCK_POS_CODE:    /*          case: num lock            */
                                       /*             if its a make and we   */
                                       /*             are updating leds and  */
                                       /*             Alt & Ctrl are both up */
               if ((kbdstat & M_B) &&
                   update_leds  &&
                   !(kbdstat & ALT) && !(kbdstat & CNTL))
                                       /*                toggle num lock     */
                                       /*                status              */
                  kbdstat ^= NUM_LOCK;
               break;

            default:                   /*          default:                  */
               break;                  /*             do nothing (key was not*/
                                       /*             one of the status keys)*/
         }
   }

   key->kbdstat  = kbdstat;            /* copy register variable into struct */

   if (update_leds) {                  /* update leds as required            */
      kbdleds(com, key);
   }

   KTSMDTRACE1(shift_status, exit, kbdstat);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        proc_event                                                   */
/*                                                                           */
/* FUNCTION:    This module finishes processing key events -                 */
/*              * looks for Ctrl-Alt-numpad sequence                         */
/*              * puts keyboard event on input ring                          */
/*                                                                           */
/* INPUTS:      key = pointer to keyboard extension                          */
/*              pos_code = position code of key pressed                      */
/*              scan_code = scan code of key pressed                         */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:       This routine calls the kernel function "ctlaltnum" when      */
/*              a numpad key is pressed if both the Ctrl and left Alt        */
/*              keys are down. The make and break of the numpad key is       */
/*              not placed on the ring in this case. The order in which      */
/*              the Ctrl or Alt keys are pressed does not matter.            */
/*                                                                           */
/*****************************************************************************/

void  proc_event(struct kbdext *key, uchar pos_code, uchar scan_code)
{
   char *key_state;

   KTSMDTRACE(proc_event, enter,  key->kbdstat,
          key->kbd_state_table[pos_code], pos_code, scan_code, 0);

   if ((key->kbdstat & CNTL) &&   /* if Ctrl and Alt keys are down      */
       (key->kbdstat & ALT)  &&   /* and key being pressed              */
       (key->kbdstat & M_B)) {
                                       /*    pointer to state table          */
      key_state = &key->kbd_state_table[pos_code];

                                       /*    if repeating                    */
                                       /*       put key on ring if it was not*/
                                       /*       part of 3 key sequence       */

      if (key->kbdstat & REPEAT) {
         if (*key_state == KEY_DOWN)
            put_key(key, pos_code, scan_code, key->kbdstat);
      }
      else {                           /*    else (initial make of key)      */
         switch (pos_code)             /*      process based on pos code     */
         {
            case NUMPAD0:              /*         num pad: "0"               */
               *key_state = KEY_IGN;   /*           ignore key till released */
               CTLALTNUM(0);           /*           call kernel to process   */
               break;

            case NUMPAD1:              /*         num pad: "1"               */
               *key_state = KEY_IGN;   /*           ignore key till released */
               CTLALTNUM(1);           /*           call kernel to process   */
               break;

            case NUMPAD2:              /*         num pad: "2"               */
               *key_state = KEY_IGN;   /*           ignore key till released */
               CTLALTNUM(2);           /*           call kernel to process   */
               break;

            case NUMPAD3:              /*         num pad: "3"               */
               *key_state = KEY_IGN;   /*           ignore key till released */
               CTLALTNUM(3);           /*           call kernel to process   */
               break;

            case NUMPAD4:              /*         num pad: "4"               */
               *key_state = KEY_IGN;   /*           ignore key till released */
               CTLALTNUM(4);           /*           call kernel to process   */
               break;

            case NUMPAD5:              /*         num pad: "5"               */
               *key_state = KEY_IGN;   /*           ignore key till released */
               CTLALTNUM(5);           /*           call kernel to process   */
               break;

            case NUMPAD6:              /*         num pad: "6"               */
               *key_state = KEY_IGN;   /*           ignore key till released */
               CTLALTNUM(6);           /*           call kernel to process   */
               break;

            case NUMPAD7:              /*         num pad: "7"               */
               *key_state = KEY_IGN;   /*           ignore key till released */
               CTLALTNUM(7);           /*           call kernel to process   */
               break;

            case NUMPAD8:              /*         num pad: "8"               */
               *key_state = KEY_IGN;   /*           ignore key till released */
               CTLALTNUM(8);           /*           call kernel to process   */
               break;

            case NUMPAD9:              /*         num pad: "9"               */
               *key_state = KEY_IGN;   /*           ignore key till released */
               CTLALTNUM(9);           /*           call kernel to process   */
               break;

            default:
               put_key(key, pos_code, scan_code, key->kbdstat);
         }
      }
   }
   else                                /* put key event on ring              */
      put_key(key, pos_code, scan_code, key->kbdstat);
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        proc_sak                                                     */
/*                                                                           */
/* FUNCTION:    Handles SAK detection                                        */
/*                                                                           */
/* INPUTS:      key = pointer to keyboard extension                          */
/*              pos_code = position code of key pressed                      */
/*              scan_code = scan code of key pressed                         */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:       This routine called recusively by un_sak()                   */
/*                                                                           */
/*****************************************************************************/

void  proc_sak(struct kbdext *key, uchar pos_code, uchar scan_code)
{
   KTSMDTRACE1(proc_sak, enter, key->in_sak);

   if (key->in_sak) {                  /* SAK sequence started               */
      switch(pos_code) {               /*   process based on position code   */

         case SAK_KEY_0:               /*   ctrl key...ignore                */
            break;

         case SAK_KEY_1:               /*   "x" key                          */
                                       /*      if break save shift status    */
            if (!(key->kbdstat & M_B))
               key->save_status1 = key->kbdstat;
            else                       /*       ignore if repeating          */
               if (key->kbdstat & REPEAT) break;
               else                    /*       out of sequence if make      */
                  un_sak(key, pos_code, scan_code);
            break;

         case SAK_KEY_2:               /*   "r" key                          */
                                       /*      SAK if key make and Ctrl down */
            if ((key->kbdstat & CNTL) &&
              (key->kbdstat & M_B)) {
                                       /*         ignore "r" till break      */
               key->kbd_state_table[SAK_KEY_2] = KEY_IGN;
                                       /*         ignore "x" till break      */
               if (key->kbd_state_table[SAK_KEY_1] != KEY_UP)
                  key->kbd_state_table[SAK_KEY_1] = KEY_IGN;
                                       /*          terminate sequence        */
               key->in_sak = FALSE;
                                       /*          exec SAK callback         */
               if (key->sak_callback != (void *) NULL) {
                  KTSMDTRACE0(proc_sak, callback );
                  (*((void (*)()) key->sak_callback))();
               }
            }
            else                       /*      out of seq on any other event */
               un_sak(key, pos_code, scan_code);
            break;

         default:                      /*   out of sequence on any other     */
                                       /*   key event                        */
            un_sak(key, pos_code, scan_code);
            break;
      }
   }
   else {                              /* else (SAK not started)             */
                                       /*   if Ctrl-X make then              */
      if ((key->kbdstat & CNTL) &&
        (pos_code == SAK_KEY_1) &&
        (key->kbdstat & M_B)) {

         key->in_sak = TRUE;           /*     we are in sak sequence         */
                                       /*     save status                    */
         key->save_status0 = key->kbdstat;
         key->save_status1 = key->kbdstat;
      }
      else                             /*   else go process event            */
         proc_event(key, pos_code, scan_code);
   }
}

/*****************************************************************************/
/*                                                                           */
/* NAME:        un_sak                                                       */
/*                                                                           */
/* FUNCTION:    Exit SAK sequence and enqueue appropriate keys onto          */
/*              input ring                                                   */
/*                                                                           */
/* INPUTS:      key = pointer to keyboard extension                          */
/*              pos_code = position code of pressed key                      */
/*              scan_code = scan code of pressed key                         */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:       recurrsion (ie: calling of proc_sak) handles case of         */
/*              Ctrl-X make-X break-X make ...                               */
/*                                                                           */
/*****************************************************************************/


void  un_sak(struct kbdext *key, uchar pos_code, uchar scan_code)
{

      KTSMDTRACE0(un_sak, enter);

                                     /* output X make                        */
      put_key(key, (uchar) SAK_KEY_1, (uchar) SAK_KEY_1_SC,
              key->save_status0);

                                     /* output X break as required           */
      if (key->kbd_state_table[SAK_KEY_1] == KEY_UP) {
         if (!(key->save_status1 & CNTL))
                                     /* ... need ctrl break                  */
            put_key(key, (uchar) SAK_KEY_0, (uchar) SAK_KEY_0_SC,
                 key->save_status1);
                                     /* ... now output X break               */
         put_key(key, (uchar) SAK_KEY_1, (uchar) SAK_KEY_1_SC,
                 key->save_status1);
      }
                                     /* correct Ctrl status as required      */
      if ((key->save_status1 ^ key->kbdstat) & CNTL) {
         if (((key->save_status1 & CNTL) == CNTL) &&
             ((key->kbdstat & CNTL) != CNTL))
                                     /* ... need  ctrl make                  */
            put_key(key, (uchar) SAK_KEY_0, (uchar) SAK_KEY_0_SC,
                (ushort) (key->kbdstat & ~(REPEAT | M_B)));
         else                        /* ... need ctrl break                  */
            put_key(key, (uchar) SAK_KEY_0, (uchar) SAK_KEY_0_SC,
                 (ushort) ((key->kbdstat | M_B) & ~REPEAT));
      }


      key->in_sak = FALSE;           /* out of sak sequence                  */

                                     /* process new scan code                */
      proc_sak(key, pos_code, scan_code);

}


/*****************************************************************************/
/*                                                                           */
/* NAME:        put_key                                                      */
/*                                                                           */
/* FUNCTION:    This module enqueues a keyboard event report onto the        */
/*              input ring                                                   */
/*                                                                           */
/* INPUTS:      key = pointer to keyboard extension                          */
/*              pos_code = position code of pressed key                      */
/*              scan_code = scan code of pressed key                         */
/*              status = status at time key was pressed                      */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/


void  put_key(struct kbdext *key, uchar pos_code, uchar scan_code,
              ushort status)
{


   struct ir_kbd event;                /* structure for keyboard event       */
   struct ccb *ccb;

   KTSMDTRACE(put_key,  enter,  pos_code, scan_code, status, 0, 0);

                                       /* look for keep alive poll request   */
   poll_appl(key, pos_code, scan_code, status);

                                       /* if kernel owns ring then filter    */
                                       /* out breaks of all keys except      */
                                       /* shift keys                         */
   if ((key->ccb[key->act_ch].owner_pid != KERNEL_PID)  ||
      ((key->ccb[key->act_ch].owner_pid == KERNEL_PID)  &&
      ((status & M_B) ||
      (key->shift_keys[pos_code])))) {

                                       /* build event report                 */
      event.kbd_header.report_size = sizeof(event);
      event.kbd_keypos = pos_code;
      event.kbd_scancode = scan_code;
      event.kbd_status[0] =  *(char *)(&status);
      event.kbd_status[1] =  *((char *)(&status)+1);

                                       /* put event on queue                 */
      ktsm_putring(&key->ccb[key->act_ch].rcb, (struct ir_report *) &event,
          key->notify_callback);
   }
}


/*****************************************************************************/
/*                                                                           */
/* NAME:        poll_appl                                                    */
/*                                                                           */
/* FUNCTION:    This module sends polling signal to application and          */
/*              starts response timer                                        */
/*                                                                           */
/* INPUTS:      key = pointer to keyboard extension                          */
/*              pos_code = position code of pressed key                      */
/*              scan_code = scan code of pressed key                         */
/*              status = status at time key was pressed                      */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

void  poll_appl(struct kbdext *key, uchar pos_code, uchar scan_code,
              ushort status)

{
   struct ccb *ccb;

   KTSMDTRACE(poll_appl, enter, key->ccb[key->act_ch].kpindex,
          *(key->ccb[key->act_ch].kpseq), 
          *(key->ccb[key->act_ch].kpseq+key->ccb[key->act_ch].kpindex+1),
          *(key->ccb[key->act_ch].kpseq+key->ccb[key->act_ch].kpindex+2),
          0);

   ccb = &key->ccb[key->act_ch];       /* pointer to ccb                     */
                                       /* if keep alive poll sequence        */
                                       /* defined and timer not active       */
   if ((ccb->kpseq) && (ccb->kpindex >= 0)) {
                                       /* if next key in sequence            */
      if ((status & M_B) &&        
        (pos_code == *(ccb->kpseq + ccb->kpindex + 1)) &&
        ((*(char *)(&status) & (KBDUXSHIFT|KBDUXCTRL|KBDUXALT)) == 
         *(ccb->kpseq + ccb->kpindex + 2))) {
            ccb->kpindex+=2;           /* next index                         */
                                       /* if entire sequence keyed then      */
         if ((uchar) ccb->kpindex == *(ccb->kpseq)*2) {
            ccb->kpindex = -1;         /* indicated timer is active          */
  
                                       /* signal user                        */
            pidsig(ccb->owner_pid, SIGKAP);
                                       /* zero out timer structure           */
            bzero (ccb->kptimer, sizeof(struct trb));
                                       /* set time interval                  */
            ccb->kptimer->timeout.it_value.tv_sec = KPOLL_TO;
            ccb->kptimer->timeout.it_value.tv_nsec = 0;
            ccb->kptimer->flags |= T_INCINTERVAL;
                                       /* timer interrupt handler            */
            ccb->kptimer->func = (void (*)()) appl_killer;
                                       /* timer interrupt priority           */
            ccb->kptimer->ipri = INTCLASS3;
                                       /* pointer to ccb                     */
            ccb->kptimer->t_union.addr = (caddr_t) ccb;
                                       /* start timer                        */
            tstart(ccb->kptimer);
         }
      }
      else {                           /* wrong key, start sequence over     */
         if (!(status & REPEAT)) {     /* (repeating keys ignored)           */
            ccb->kpindex = 0;
         }
      }
   }
   KTSMDTRACE1(poll_appl, exit, ccb->kpindex);
}

/*****************************************************************************/
/*                                                                           */
/* NAME:        appl_killer                                                  */
/*                                                                           */
/* FUNCTION:    This module kills channel owner as it did not                */
/*              ACK keep alive poll within 30secs                            */
/*                                                                           */
/* INPUTS:      tb = address of pop'ed trb                                   */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

void  appl_killer(struct trb *tb)

{
   struct ccb *ccb;


   ccb = (struct ccb *) tb->t_union.addr;
   pidsig(ccb->owner_pid, SIGKILL);

   KTSMDTRACE0(appl_killer, exit);

}


/*****************************************************************************/
/*                                                                           */
/* NAME:        key_stat                                                     */
/*                                                                           */
/* FUNCTION:    Update keyboard status                                       */
/*                                                                           */
/* INPUTS:      com = pointer to common                                      */
/*              key = pointer to keyboard extension                          */
/*              select = mask to select which status bits to change          */
/*              value = new value                                            */
/*                                                                           */
/* OUTPUTS:     None                                                         */
/*                                                                           */
/* ENVIRONMENT: Process                                                      */
/*                                                                           */
/* NOTES:       This function allows the top half to update keyboard status  */
/*              without conflict with bottom half                            */
/*                                                                           */
/*****************************************************************************/

int key_stat(struct common *com, struct kbdext *key, ushort select,
             ushort value)
{
   int tmp;
   int rc;

                                       /* start of critical section          */
   tmp = i_disable(com->intr_priority);
                                       /* update keyboard status             */
   key->kbdstat = (key->kbdstat & (~select)) | (value & select);

   if (kbdleds(com, key)) {            /* update leds                        */
      rc = wait_oq(com);               /* wait on I/O as required            */
   }
   else {
      rc = 0;
   }

   i_enable(tmp);                      /* end of critical section            */

   return(rc);
}



/*****************************************************************************/
/*                                                                           */
/* NAME:        kbdleds                                                      */
/*                                                                           */
/* FUNCTION:    Update keyboard leds                                         */
/*                                                                           */
/* INPUTS:      com = pointer to common structure                            */
/*              key = pointer to keyboard extension                          */
/*                                                                           */
/* OUTPUTS:     FALSE = no I/O started                                       */
/*              TRUE = I/O started                                           */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int kbdleds(struct common *com, struct kbdext *key)
{
   int rc;
   uchar led;


   led = ALL_LEDS_OFF;                 /* clear all leds                     */

   if (key->kbdstat & SCR_LOCK)        /* turn on scroll lock as required    */
      led |= SCRLLOCK_LED;

   if (key->kbd_type == KS106) {       /* process if 106 key keyboard        */
      if (key->kbdstat & CAPS_LOCK)
            led |= CAPSLOCK_LED_106;
      if (key->kbdstat & NUM_LOCK)
            led |= NUMLOCK_LED_106;
   }
   else {                              /* process if 101/102 key keyboard    */
      if (key->kbdstat & CAPS_LOCK)
         led |= CAPSLOCK_LED;
      if (key->kbdstat & NUM_LOCK)
         led |= NUMLOCK_LED;
   }

   if (led != key->led_state) {        /*  update leds on keyboard if chg'ed */
      key->led_state = led;
      put_oq2(com, (OFRAME) SET_LED_CMD, (OFRAME)((led<<8) | WRITE_KBD_CMD));
      rc = TRUE;
   }
   else  rc = FALSE;

   return(rc);

}

/*****************************************************************************/
/*                                                                           */
/* NAME:        sv_proc                                                      */
/*                                                                           */
/* FUNCTION:    process service vector requests                              */
/*                                                                           */
/* INPUTS:      com = pointer to common structure                            */
/*              key = pointer to keyboard extension                          */
/*              cmd = function                                               */
/*              arg = pointer to arguments (as required)                     */
/*                                                                           */
/* OUTPUTS:     0 = successful                                               */
/*              EINVAL = invalid parameter                                   */
/*              EBUSY = interface busy, request ignored                      */
/*              EIO = I/O error                                              */
/*              -1 = kernel has not opened channel                           */
/*                                                                           */
/* ENVIRONMENT: Interrupt                                                    */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

int sv_proc(struct common *com, struct kbdext *key, int cmd, caddr_t arg)
{
   int i, rc;

   KTSMDTRACE0(sv_proc, enter);

   rc = -1;                            /* see if kernel channel is open      */
   for(i = 0;i<KBD_NUM_CH; i++) {
     if(key->ccb[i].inuse) {
       if (key->ccb[i].oseq != NOTOPEN) {
         if (key->ccb[i].owner_pid == KERNEL_PID) {
           rc = 0;
           break;
         }
       }
     }
   }

   if (!rc) {                          /* kernel channel is open so          */
     switch(cmd) {                     /* process request                    */

                                       /*------------------------------------*/
                                       /*  sound alarm                       */
                                       /*------------------------------------*/
       case KSVALARM:
         if (arg == (caddr_t) NULL) {  /* error if no argument               */
           rc = EINVAL;
         }
         else {                        /* only process cmd when kernel       */
                                       /* channel is active                  */
           if (key->act_ch == i) {
             rc = put_sq((struct ksalarm *) arg);
           }
         }
         break;
                                       /*------------------------------------*/
                                       /*  enable/disable SAK                */
                                       /*------------------------------------*/
       case KSVSAK:
         if (arg == (caddr_t) NULL) {  /* error if no argument               */
           rc = EINVAL;
         }
         else {                        /* disable SAK if told to             */
           if (*((int *) arg) == KSSAKDISABLE) {
             key->sak_enabled = FALSE;
           }
           else {                      /* enable SAK if told to              */
             if (*((int *) arg) == KSSAKENABLE) {
               key->in_sak = FALSE;
               key->sak_enabled = TRUE;
             }
             else {                    /* invalid argument                   */
               rc = EINVAL;
             }
           }
         }
         break;
                                       /*------------------------------------*/
                                       /*  flush input ring                  */
                                       /*------------------------------------*/
       case KSVRFLUSH:
         ktsm_rflush(com,&key->ccb[i].rcb);
         break;

                                       /*------------------------------------*/
                                       /*  invalid request                   */
                                       /*------------------------------------*/
       default:
         rc = EINVAL;
         break;

     }
   }

   KTSMTRACE(sv_proc, exit, rc, cmd, 0, 0, 0);

   return(rc);
}


/*****************************************************************************/
/*                                                                           */
/*               table to translate scan codes to position codes             */
/*                                                                           */
/* NOTES:                                                                    */
/*    The scan code to key position code translation table is a table of key */
/*    position codes arranged in the order of scan codes ranging from 0 to 84*/
/*    hex (0-132 decimal).                                                   */
/*                                                                           */
/*****************************************************************************/

uchar   scan_to_posi[133] =
        {
             0,              /*   INVALID SCAN CODE 00                       */
             0,              /*   INVALID SCAN CODE 01 -                     */
             0,              /*   INVALID SCAN CODE 02                       */
             0,              /*   INVALID SCAN CODE 03 -                     */
             0,              /*   INVALID SCAN CODE 04 -                     */
             0,              /*   INVALID SCAN CODE 05 -                     */
             0,              /*   INVALID SCAN CODE 06 -                     */
           112,              /*   KPC FOR SCAN CODE 07 - 'F1'                */
           110,              /*   KPC FOR SCAN CODE 08   'ESC'               */
             0,              /*   INVALID SCAN CODE 09 -                     */
             0,              /*   INVALID SCAN CODE 0A -                     */
             0,              /*   INVALID SCAN CODE 0B -                     */
             0,              /*   INVALID SCAN CODE 0C -                     */
            16,              /*   KPC FOR SCAN CODE 0D - TABS                */
             1,              /*   KPC FOR SCAN CODE 0E - '`'                 */
           113,              /*   KPC FOR SCAN CODE 0F - 'F2                 */
             0,              /*   INVALID SCAN CODE 10                       */
            58,              /*   KPC FOR SCAN CODE 11 - 'CNTL'              */
            44,              /*   KPC FOR SCAN CODE 12 - 'SHIFT'             */
            45,              /*   KPC FOR SCAN CODE 13 - W. T.               */
            30,              /*   KPC FOR SCAN CODE 14 - 'CAPS L'            */
            17,              /*   KPC FOR SCAN CODE 15 - 'Q'                 */
            02,              /*   KPC FOR SCAN CODE 16 - '1'                 */
           114,              /*   KPC FOR SCAN CODE 17 - 'F3'                */
             0,              /*   INVALID SCAN CODE 18                       */
            60,              /*   KPC FOR SCAN CODE 19 - 'ALT'               */
            46,              /*   KPC FOR SCAN CODE 1A - 'Z'                 */
            32,              /*   KPC FOR SCAN CODE 1B - 'S'                 */
            31,              /*   KPC FOR SCAN CODE 1C - 'A'                 */
            18,              /*   KPC FOR SCAN CODE 1D - 'W'                 */
            03,              /*   KPC FOR SCAN CODE 1E - '2'                 */
           115,              /*   KPC FOR SCAN CODE 1F - 'F4'                */
           131,              /*   106_key kbd  CODE 20 -                     */
            48,              /*   KPC FOR SCAN CODE 21 - 'C'                 */
            47,              /*   KPC FOR SCAN CODE 22 - 'X'                 */
            33,              /*   KPC FOR SCAN CODE 23 - 'D'                 */
            19,              /*   KPC FOR SCAN CODE 24 - 'E'                 */
            05,              /*   KPC FOR SCAN CODE 25 - '4'                 */
            04,              /*   KPC FOR SCAN CODE 26 - '3'                 */
           116,              /*   KPC FOR SCAN CODE 27 - 'F5'                */
           132,              /*   106_key kbd  CODE 28 - 'katakana'          */
            61,              /*   KPC FOR SCAN CODE 29 - 'SPACE'             */
            49,              /*   KPC FOR SCAN CODE 2A - 'V'                 */
            34,              /*   KPC FOR SCAN CODE 2B - 'F'                 */
            21,              /*   KPC FOR SCAN CODE 2C - 'T'                 */
            20,              /*   KPC FOR SCAN CODE 2D - 'R'                 */
            06,              /*   KPC FOR SCAN CODE 2E - '5'                 */
           117,              /*   KPC FOR SCAN CODE 2F - 'F6'                */
           133,              /*   106_key kbd  CODE 30 - 'hiragana'          */
            51,              /*   KPC FOR SCAN CODE 31 - 'N'                 */
            50,              /*   KPC FOR SCAN CODE 32 - 'B'                 */
            36,              /*   KPC FOR SCAN CODE 33 - 'H'                 */
            35,              /*   KPC FOR SCAN CODE 34 - 'G'                 */
            22,              /*   KPC FOR SCAN CODE 35 - 'Y'                 */
             7,              /*   KPC FOR SCAN CODE 36 - '6'                 */
           118,              /*   KPC FOR SCAN CODE 37 - 'F7'                */
             0,              /*   INVALID SCAN CODE 38                       */
            62,              /*   KPC FOR SCAN CODE 39 - 'ALT'               */
            52,              /*   KPC FOR SCAN CODE 3A - 'M'                 */
            37,              /*   KPC FOR SCAN CODE 3B - 'J'                 */
            23,              /*   KPC FOR SCAN CODE 3C - 'U'                 */
             8,              /*   KPC FOR SCAN CODE 3D - '7'                 */
             9,              /*   KPC FOR SCAN CODE 3E - '8'                 */
           119,              /*   KPC FOR SCAN CODE 3F - 'F8'                */
             0,              /*   INVALID SCAN CODE 40                       */
            53,              /*   KPC FOR SCAN CODE 41 - ','                 */
            38,              /*   KPC FOR SCAN CODE 42 - 'K'                 */
            24,              /*   KPC FOR SCAN CODE 43 - 'I'                 */
            25,              /*   KPC FOR SCAN CODE 44 - 'O'                 */
            11,              /*   KPC FOR SCAN CODE 45 - '0'                 */
            10,              /*   KPC FOR SCAN CODE 46 - '9'                 */
           120,              /*   KPC FOR SCAN CODE 47 - 'F9'                */
             0,              /*   INVALID SCAN CODE 48                       */
            54,              /*   KPC FOR SCAN CODE 49 - '.'                 */
            55,              /*   KPC FOR SCAN CODE 4A - '/'                 */
            39,              /*   KPC FOR SCAN CODE 4B - 'L'                 */
            40,              /*   KPC FOR SCAN CODE 4C - ';'                 */
            26,              /*   KPC FOR SCAN CODE 4D - 'P'                 */
            12,              /*   KPC FOR SCAN CODE 4E - '-'                 */
           121,              /*   KPC FOR SCAN CODE 4F - 'F10'               */
             0,              /*   INVALID SCAN CODE 50                       */
            56,              /*   106_key keyboard  51                       */
            41,              /*   KPC FOR SCAN CODE 52 - '''                 */
            42,              /*   KPC FOR SCAN CODE 53 - W. T.               */
            27,              /*   KPC FOR SCAN CODE 54 - '['                 */
            13,              /*   KPC FOR SCAN CODE 55 - '='                 */
           122,              /*   KPC FOR SCAN CODE 56 - 'F11'               */
           124,              /*   KPC FOR SCAN CODE 57 - 'PRTSC'             */
            64,              /*   KPC FOR SCAN CODE 58 - 'ACTION             */
            57,              /*   KPC FOR SCAN CODE 59 - 'SHIFT'             */
            43,              /*   KPC FOR SCAN CODE 5A - RETURN              */
            28,              /*   KPC FOR SCAN CODE 5B - ']'                 */
            29,              /*   KPC FOR SCAN CODE 5C - '\'                 */
            14,              /*   106_key keyboard  5D   '-'                 */
           123,              /*   KPC FOR SCAN CODE 5E - 'F12'               */
           125,              /*   KPC FOR SCAN CODE 5F - 'SCR L'             */
            84,              /*   KPC FOR SCAN CODE 60 - 'down arrow'        */
            79,              /*   KPC FOR SCAN CODE 61 - 'left arrow'        */
           126,              /*   KPC FOR SCAN CODE 62 - 'PAUSE'             */
            83,              /*   KPC FOR SCAN CODE 63 - 'up arrow'          */
            76,              /*   KPC FOR SCAN CODE 64 - 'DEL'               */
            81,              /*   KPC FOR SCAN CODE 65 - 'END'               */
            15,              /*   KPC FOR SCAN CODE 66 - 'BACKSP'            */
            75,              /*   KPC FOR SCAN CODE 67 - 'INS'               */
            94,              /*   KPC FOR SCAN CODE 68 - **unused**          */
            93,              /*   KPC FOR SCAN CODE 69 - '1'                 */
            89,              /*   KPC FOR SCAN CODE 6A - 'right arrow'       */
            92,              /*   KPC FOR SCAN CODE 6B - '4'                 */
            91,              /*   KPC FOR SCAN CODE 6C - '7'                 */
            86,              /*   KPC FOR SCAN CODE 6D - 'PG DN'             */
            80,              /*   KPC FOR SCAN CODE 6E - 'HOME'              */
            85,              /*   KPC FOR SCAN CODE 6F - 'PG UP'             */
            99,              /*   KPC FOR SCAN CODE 70 - '0'                 */
           104,              /*   KPC FOR SCAN CODE 71 - '.'                 */
            98,              /*   KPC FOR SCAN CODE 72 - '2'                 */
            97,              /*   KPC FOR SCAN CODE 73 - '5'                 */
           102,              /*   KPC FOR SCAN CODE 74 - '6'                 */
            96,              /*   KPC FOR SCAN CODE 75 - '8'                 */
            90,              /*   KPC FOR SCAN CODE 76 - 'NUM L'             */
            95,              /*   KPC FOR SCAN CODE 77 - '/'                 */
           109,              /*   KPC FOR SCAN CODE 78 - **unused**          */
           108,              /*   KPC FOR SCAN CODE 79 -                     */
           103,              /*   KPC FOR SCAN CODE 7A - '3'                 */
           107,              /*   KPC FOR SCAN CODE 7B - **unused**          */
           106,              /*   KPC FOR SCAN CODE 7C - '+'                 */
           101,              /*   KPC FOR SCAN CODE 7D - '9'                 */
           100,              /*   KPC FOR SCAN CODE 7E - '*'                 */
             0,              /*   INVALID SCAN CODE 7F                       */
             0,              /*   INVALID SCAN CODE 80                       */
             0,              /*   INVALID SCAN CODE 81                       */
             0,              /*   INVALID SCAN CODE 82                       */
             0,              /*   INVALID SCAN CODE 83 -                     */
           105               /*   KPC FOR SCAN CODE 84 - '-'                 */
        };


/*****************************************************************************/
/*                                                                           */
/*              Initialization commands for keyboard                         */
/*                                                                           */
/*****************************************************************************/

/* 101/102 key keyboard                                                      */
/*  WARNING: do not change this table without changing TYPAPARM, NUMLOCKPARM */
/*           in keyfns.c                                                     */
OFRAME  kbdinit_101[] = {
                             18,                     /* frame count          */
                             DEFAULT_DISABLE_CMD,
                             SET_RATE_DELAY_CMD,
                             0,                      /* typamatic rate/delay */
                             SELECT_SCAN_CODE,
                             SELECT_SCAN_CODE_3,
                             SET_ALL_TYPA_MK_BRK_CMD,
                             SET_MAKE_BREAK_CMD,
                             SET_R_ALT_SCAN_CODE,
                             SET_L_ALT_SCAN_CODE,
                             SET_CONTROL_SCAN_CODE,
                             SET_L_SHIFT_SCAN_CODE,
                             SET_R_SHIFT_SCAN_CODE,
                             SET_CAPS_LOCK_SCAN_CODE,
                             SET_NUM_LOCK_SCAN_CODE,
                             SET_ACTION_SCAN_CODE,
                             SET_LED_CMD,
                             SET_ALL_LEDS_OFF,
                             KBD_ENABLE_CMD
                        };

/* PS/2 106 key keyboard                                                     */
/*  WARNING: do not change this table without changing TYPAPARM, NUMLOCKPARM */
/*           in keyfns.c                                                     */
OFRAME  kbdinit_106ps[] = {
                             22,                     /* frame count          */
                             DEFAULT_DISABLE_CMD,
                             SET_RATE_DELAY_CMD,
                             0,                      /* typamatic rate/delay */
                             SELECT_SCAN_CODE,
                             SELECT_SCAN_CODE_3,
                             SET_ALL_TYPA_MK_BRK_CMD,
                             SET_MAKE_BREAK_CMD,
                             SET_R_ALT_SCAN_CODE,
                             SET_L_ALT_SCAN_CODE,
                             SET_CONTROL_SCAN_CODE,
                             SET_L_SHIFT_SCAN_CODE,
                             SET_R_SHIFT_SCAN_CODE,
                             SET_106K_1_SCAN_CODE,
                             SET_NUM_LOCK_SCAN_CODE,
                             SET_CAPS_LOCK_SCAN_CODE,
                             SET_ACTION_SCAN_CODE,
                             SET_106K_131_SCAN_CODE,
                             SET_106K_132_SCAN_CODE,
                             SET_HIRA_KEY_SCAN_CODE,
                             SET_LED_CMD,
                             SET_ALL_LEDS_OFF,
                             KBD_ENABLE_CMD
                        };

/* 106 key keyboard                                                          */
/*  WARNING: do not change this table without changing TYPAPARM, NUMLOCKPARM */
/*           and KOREANPARM in keyfns.c                                      */
OFRAME kbdinit_106[] =  {
                             24,                     /* frame count          */
                             DEFAULT_DISABLE_CMD,
                             SET_RATE_DELAY_CMD,
                             0,                      /* typamatic rate/delay */
                             SET_MAKE_BREAK_CMD,
                             SET_R_ALT_SCAN_CODE,
                             SET_L_ALT_SCAN_CODE,
                             SET_CONTROL_SCAN_CODE,
                             SET_L_SHIFT_SCAN_CODE,
                             SET_R_SHIFT_SCAN_CODE,
                             SET_CAPS_LOCK_SCAN_CODE,
                             SET_NUM_LOCK_SCAN_CODE,
                             SET_ACTION_SCAN_CODE,
                             SET_106K_1_SCAN_CODE,   /* disable for Korean   */
                             SET_106K_131_SCAN_CODE,
                             SET_KATA_KEY_SCAN_CODE,
                             SET_HIRA_KEY_SCAN_CODE,
                             SET_DBL_RATE_TMATIC_CMD,
                             SET_L_ARROW_SCAN_CODE,
                             SET_R_ARROW_SCAN_CODE,
                             SET_U_ARROW_SCAN_CODE,
                             SET_D_ARROW_SCAN_CODE,
                             SET_LED_CMD,
                             SET_ALL_LEDS_OFF,
                             KBD_ENABLE_CMD
                        };

