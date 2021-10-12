static char sccsid[] = "@(#)06   1.4  src/bos/kernext/inputdd/common/tabletfnspin.c, inputdd, bos411, 9428A410j 6/10/94 09:25:15";
/*
 * COMPONENT_NAME: (INPUTDD) Tablet    DD - tabletfnspin.c
 *
 * FUNCTIONS: tablet_proc
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "ktsm.h"
#include "common.h"
#include "tabext.h"

#define ABS_VALUE(x) (ushort)(((x) < 0) ? -(x) : (x))


/*****************************************************************************/
/*                                                                           */
/* IDENTIFICATION:   tablet_proc                                             */
/*                                                                           */
/* FUNCTION:         Process tablet status report                            */
/*                                                                           */
/* INPUTS:           tab = pointer to tablet extension                       */
/*                                                                           */
/* OUTPUTS:          none                                                    */
/*                                                                           */
/* ENVIRONMENT:      Interrupt                                               */
/*                                                                           */
/* NOTES:                                                                    */
/*                                                                           */
/*****************************************************************************/

void tablet_proc(struct tabext *tab)

{
   struct  ir_tablet event;
   short   temp_tab_x_pos;
   short   temp_tab_y_pos;
   uchar   buttons;

   uchar   tab_stat_1;                 /* first status byte from tablet rpt  */
   uchar   tab_stat_2;                 /* second status byte from tablet rpt */
   uchar   x_pos_1;                    /* x position bits 12 - 6             */
   uchar   x_pos_2;                    /* x position bits 5  - 0 and bit 13  */
   uchar   y_pos_1;                    /* y position bits 13 - 7             */
   uchar   y_pos_2;                    /* y position bits 6  - 0             */


   tab_stat_1 = tab->tab_block_data[0];
   tab_stat_2 = tab->tab_block_data[1];
   x_pos_1    = tab->tab_block_data[2];
   x_pos_2    = tab->tab_block_data[3];
   y_pos_1    = tab->tab_block_data[4];
   y_pos_2    = tab->tab_block_data[5];

                                       /*  process x position                */
   temp_tab_x_pos = (short)(((ushort) x_pos_1 << 5) |
           ((ushort)x_pos_2 >> 2));

   if (x_pos_2 & X_TAB_DATA_MSB)
      temp_tab_x_pos |= 0x2000;

   if (tab_stat_2 & TAB_X_DATA_SIGN)
      temp_tab_x_pos |= 0xE000;

                                       /*  process y position                */
   temp_tab_y_pos = (short)(((ushort) y_pos_1 << 5) |
           ((ushort)y_pos_2 >> 2));

   if (y_pos_2 & Y_TAB_DATA_MSB)
      temp_tab_y_pos |= 0x2000;

   if (tab_stat_2 & TAB_Y_DATA_SIGN)
      temp_tab_y_pos |= 0xE000;

                                       /*  process button events             */
   switch (tab_stat_2 & TAB_BUTTON_PRESSED)
   {
      case TAB_BUTTON_1_DOWN:
         buttons = TABLETBUTTON1;
         break;

      case TAB_BUTTON_2_DOWN:
         buttons = TABLETBUTTON2;
         break;

      case TAB_BUTTON_3_DOWN:
         buttons = TABLETBUTTON3;
         break;

      case TAB_BUTTON_4_DOWN:
         buttons = TABLETBUTTON4;
         break;

     case TAB_BUTTON_5_DOWN:
         buttons = TABLETBUTTON5;
         break;

     case TAB_BUTTON_6_DOWN:
         buttons = TABLETBUTTON6;
         break;

      default:
         buttons = 0;
   }

                                       /* if puck in active area then        */
   if ((tab_stat_1 & TAB_PRESENCE)  &&
      (ABS_VALUE(temp_tab_x_pos) <= tab->tab_hor_active) &&
      (ABS_VALUE(temp_tab_y_pos) <= tab->tab_ver_active)) {

      buttons |= TABLETPUCK;           /* indicate puck in active area       */

                                       /* if button status has not changed   */
      if (tab->tabstat == buttons) {
                                       /* dump event if resolution is below  */
                                       /* flicker value and x,y value is     */
                                       /* same as previous report            */
         if (tab->tab_resolution < TAB_FLICKER_VALUE) {
            if ((tab->tab_x_pos == temp_tab_x_pos)  &&
                (tab->tab_y_pos == temp_tab_y_pos)) {
               return;
            }
         }
         else {                        /* dump event if resolution is equal  */
                                       /* or greater than flicker value and  */
                                       /* x,y value differ from previous     */
                                       /* event by 1 or less                 */
            if ((ABS_VALUE(tab->tab_x_pos - temp_tab_x_pos) <= 1) &&
                (ABS_VALUE(tab->tab_y_pos - temp_tab_y_pos) <= 1)) {
               return;
            }
         }
      }
                                       /* save x,y value                     */
      tab->tab_x_pos = temp_tab_x_pos;
      tab->tab_y_pos = temp_tab_y_pos;
   }
   else  {
                                       /*  puck outside of active area       */
                                       /* (x,y data is NOT valid)            */

                                       /*  dump event if no change in        */
                                       /*  button status                     */
      if (buttons == tab->tabstat) return;
   }


   tab->tabstat = buttons;             /* save button status                 */

                                       /* put event on input ring            */
   event.tablet_header.report_size = sizeof(event);
   event.tablet_x = tab->tab_x_pos;
   event.tablet_y = tab->tab_y_pos;
   event.tablet_status = buttons;
   ktsm_putring(&tab->rcb, (struct ir_report *) &event, NULL);

}

/*****************************************************************************/
/*                                                                           */
/* NAME:        tab_default_cmds                                             */
/*                                                                           */
/* FUNCTION:    commands to set default values on tablet                     */
/*                                                                           */
/*****************************************************************************/

OFRAME tab_default_cmds[] = {
                             10,       /* frame count                         */
                             RESET_WRAP_MODE_CMD,
                             DISABLE_TAB_CMD,
                             SET_TAB_RES_H,
                             DEFAULT_TAB_RESOLUTION_H,
                             SET_TAB_RES_L,
                             DEFAULT_TAB_RESOLUTION_L,
                             SET_TAB_ORG,
                             DEFAULT_TAB_ORIGIN,
                             SET_TAB_SAM_RATE,
                             DEFAULT_TAB_SAMPLE_RATE
                            };

