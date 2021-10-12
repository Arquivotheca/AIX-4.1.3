static char sccsid[] = "@(#)04  1.2  src/bos/kernext/lft/streams/lftvi.c, lftdd, bos411, 9428A410j 5/23/94 11:25:31";
/*
 *   COMPONENT_NAME: LFTDD
 *
 *   FUNCTIONS: ascii_index
 *		clear_all_ht
 *		clear_rect
 *		copy_part
 *		cursor_absolute
 *		cursor_back_tab
 *		cursor_down
 *		cursor_ht
 *		cursor_left
 *		cursor_right
 *		cursor_up
 *		delete_char
 *		delete_line
 *		erase_char
 *		erase_display
 *		erase_l
 *		find_next_tab
 *		find_prior_tab
 *		insert_char
 *		insert_line
 *		screen_updat
 *		scroll_down
 *		scroll_up
 *		set_attributes
 *		set_clear_tab
 *		sound_beep
 *		upd_cursor
 *		update_ds_modes
 *		update_ht_stop
 *
 *   ORIGINS: 27, 83
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*------------

  This file contains the virtual display interface routines :
      cursor_up		- Move cursor up
      cursor_down	- Move cursor down
      cursor_left	- Move cursor left
      cursor_right	- Move cursor right
      cursor_absolute	- set cursor to row/column
      delete_char	- delete characters
      delete_line	- delete lines
      erase_l		- erase a line
      erase_display	- erase all or part of the display
      screen_updat	- display update
      copy_part		- set up the call to the vdd to copy 
			  part of a line to the presentation space
      clear_rect	- set up the call to the vdd routine 
      sound_beep	- call the keyboard driver service vect. to beep
      set_clear_tab	- set / clear H/V tabulations.
      update_ht_stop	- set or clear a horiz tab checking TSM
      clear_all_ht	- set or clear all horiz tab checking TSM
      cursor_back_tab	- move to previous horizontal tab
      cursor_ht		- move to next horizontal tab
      find_prior_tab	- find previous horizontal tab
      find_next_tab	- find next horizontal tab
      scroll_down	- scroll down (lines at bottom lost)
      scroll_up		- scroll up (lines at top lost)
      erase_char	- erase characters from all or part of line
      insert_line	- insert lines
      insert_char	- insert characters
      upd_cursor	- update the cursor
      ascii_index	- line feed
  ------------*/
/***********************************************************
Include Files
***********************************************************/
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/malloc.h>
#include <sys/lockl.h>
#include <sys/device.h>
#include <sys/intr.h>
#include <sys/tty.h>
#include <sys/termio.h>
#include <sys/syspest.h>

/* Private includes
   =============
*/
#include <lft.h>	
#include <sys/inputdd.h>
#include <sys/lft_ioctl.h>
#include <sys/display.h>
#include <lftcode.h>
#include <graphics/gs_trace.h>
#include <lft_debug.h>

/* Streams and tty includes
   ========================
*/
#include <sys/stream.h>
#include <lftsi.h>              /* LFT streams information */

GS_MODULE(lftvi);
BUGVDEF(db_lftvi, 0);                  /* define initial debug level */

/*-------------------------------------------------------------------
 *
 *  Index Subroutine (Line Feed)
 *
 *  This routine moves the cursor down 1 line.  If the cursor was
 *  already on the last line, lines are scrolled up 1.
 *
 *-------------------------------------------------------------------------*/
ascii_index(struct vtmstruc *vp,struct down_stream *down)
{
   int i;

   if (vp->mparms.cursor.y == SCR_HEIGHT) {
/* LFTERR TRACE "scroll DSR" */
         (*vp->display->vttscr)(vp, 1, vp->mparms.attributes,
                                vp->mparms.cursor, 0);

      for (i = 1; i < SCR_HEIGHT; ++i)
         down->line_data[i].line_length = down->line_data[i+1].line_length;

      down->line_data[SCR_HEIGHT].line_length = 0;
   }
   else
      vp->mparms.cursor.y++;

   return(0);
}


/*-------------------------------------------------------------------
 *       screen_updat
 *       ------------
 *
 *       Screen Update.
 *
 *       Process A Graphic String.
 *
 *       Update the display. Chop the output string, if necessary
 *       into lines and call the DD (vtt routines in the Display
 *       Driver).
 *
 *       Input:  The string to be output is pointed to by the variable
 *       graphic_info and the length of the output string in variable
 *       graphic_len.  The vtmstruc for the VT is pointed to by vp.
 *
 *       Output: Graphic string placed in text shape.
 *
 *-------------------------------------------------------------------------*/
screen_updat(struct vtmstruc *vp, struct down_stream *down, char *graphic_info,
	     long graphic_len)
{
   int current_str;
   int need_to_scroll;
   int move_it_right;
   int i;
   struct vtt_rc_parms mp_rc;

   mp_rc.string_index = 1;
   need_to_scroll = 0;

   if (down->ds_mode_flag & SCS_DETECTED) {
      for (i = 0; i < graphic_len; i++) {
         if (graphic_info[i] >= 0x60 && graphic_info[i] <= 0x7f)
            graphic_info[i] -= 0x60;
         else
            graphic_info[i] = 0x10;
      }
   }

    /*------------
      While there are more graphics in the string to be processed,
      chop the string into an amount to be placed on this line. If the
      string does not fill the line, one operates on the whole string.
      Otherwise the code works on a line's worth:
      ------------*/
   while (graphic_len > 0) {
      int remaining_columns;

      remaining_columns = SCR_WIDTH - vp->mparms.cursor.x + 1;
      if (graphic_len < remaining_columns) {    /* fits on this line */
         current_str = graphic_len;
         move_it_right = 1;
      }else {                                    /* more than one line */
         current_str = remaining_columns;
         move_it_right = 0;
      }

    /*------------
      If one is in insert mode and the string to be inserted does
      not fill the line, it is important to note that we must move
      right even if we think there is nothing there because VTT may be
      keeping color attributes for positions beyond our known line
      length; copy_part is called to move chars at right of cursor to
      right and alter line length:
      -------------*/
      if ((down->ds_mode_flag & _IRM) && move_it_right) { /* insert/replace mode */
         copy_part(vp, remaining_columns,
                   vp->mparms.cursor.x,               vp->mparms.cursor.y,
                   vp->mparms.cursor.x + current_str, vp->mparms.cursor.y);

         down->line_data[vp->mparms.cursor.y].line_length =
            min((down->line_data[vp->mparms.cursor.y].line_length +
            current_str), SCR_WIDTH);
      }

    /*------------
      Set parameters for draw text (vtt) that are dependent on old
      cursor position before updating cursor:
      ------------*/
      mp_rc.start_row = vp->mparms.cursor.y;
      mp_rc.start_column = vp->mparms.cursor.x;
      vp->mparms.cursor.x = vp->mparms.cursor.x + current_str;
      down->line_data[vp->mparms.cursor.y].line_length =
         max(vp->mparms.cursor.x - 1,
             down->line_data[vp->mparms.cursor.y].line_length);

    /*------------
      If the cursor x is now beyond the screen width:
      (it is important to note that if the string exactly fills the
      line, the cursor will be one greater than screen width).
      If autonl is not turned on, the cursor x is set to SCR_WIDTH.
      Otherwise, the cursor is set to col 1.  If, when establishing
      col 1, we are not on the bottom line of the presentation space,
      cursor y is merely incremented.  When the cursor is on the bottom
      line of the PS, we need to scroll.  Other params are filled in and
      the device driver called.
      ------------*/
      if (vp->mparms.cursor.x > SCR_WIDTH) {
         if (down->ds_mode_flag & _AUTONL) {   /* wrap to next line when EOL */
            vp->mparms.cursor.x = 1;
            if (vp->mparms.cursor.y != SCR_HEIGHT)
               ++vp->mparms.cursor.y;
            else
               need_to_scroll = 1;
         }else{                           /* don't wrap */
            vp->mparms.cursor.x = SCR_WIDTH;
         }
      }

      mp_rc.string_length = current_str;
    /*------------
      ask vdd to put text on screen.
      vtttext(vp,string,rc_parms,cp_parms,cursor_show)
      ------------*/
         (*vp->display->vtttext)(vp, graphic_info, &mp_rc, &(vp->mparms), 0);

    /*------------
      If we owe the display a scroll, then call the vtt and fix up the line
      lengths:
      ------------*/
      if (need_to_scroll) {
	      (*vp->display->vttscr)(vp, 1, vp->mparms.attributes,
				     vp->mparms.cursor, 0);
	      
	      need_to_scroll = 0;
	      
	      for (i = 1; i < SCR_HEIGHT; ++i) {
		      down->line_data[i].line_length =
			      down->line_data[i+1].line_length;
	      }
	      down->line_data[SCR_HEIGHT].line_length = 0;
      }
    /*------------
      Decrement the number of graphics by the amount just processed
      and increment the array index by the string just processed:
      ------------*/
      graphic_len = graphic_len - current_str;
      mp_rc.string_index = mp_rc.string_index + current_str;

   } /* End of while (graphic_len > 0)  */

   return(0);
}



/*-------------------------------------------------------------------
 *
 *      clear_rect:
 *
 *      Clear rectangle routine sets up the call to the VDD routine for
 *      the adapter that services this terminal.
 *
 *-------------------------------------------------------------------------*/

clear_rect(struct vtmstruc *vp, int xstart, int ystart, int xend, int yend)
{
   struct vtt_box_rc_parms mp_clr_rec;
   ushort attr_save;

   mp_clr_rec.row_ul    = ystart;
   mp_clr_rec.column_ul    = xstart;
   mp_clr_rec.row_lr    = yend;
   mp_clr_rec.column_lr    = xend;

                                /* vttclr(vp, lines, attr, cursor_show) */
   attr_save = vp->mparms.attributes;
   vp->mparms.attributes &= ~REV_VIDEO_MASK;
   (*vp->display->vttclr)(vp, &mp_clr_rec, vp->mparms.attributes, 0);
   vp->mparms.attributes = attr_save;
   return(0);
}



/*-------------------------------------------------------------------
 *
 *      copy_part:
 *
 *      This routine sets up the call to the virtual display driver that
 *      services this terminal to copy part of a line to the presenta-
 *      tion space.
 *
 *-------------------------------------------------------------------------*/
copy_part(struct vtmstruc *vp,int length,
	  int xstart,int ystart,int xdest,int ydest)
{
   struct vtt_rc_parms mp_rc;

   mp_rc.string_length     = length;
   mp_rc.start_row   = ystart;
   mp_rc.start_column   = xstart;
   mp_rc.dest_row    = ydest;
   mp_rc.dest_column    = xdest;

                                /* vttcpl(vp, rc_parms, cursor_show) */
   (*vp->display->vttcpl)(vp, &mp_rc, 0);
   return(0);
}



/*-------------------------------------------------------------------
 *
 *  sound_beep:
 *  This routine makes a call to the sound_driver to emit a beep.
 *
 *-------------------------------------------------------------------------*/
sound_beep(struct vtmstruc *vp)
{
   struct ksalarm sound;

   sound.duration = 7;
   sound.frequency = 880;
#ifndef KBDRQD
   if (lft_ptr->dds_ptr->kbd.devno != -1)      /* keyboard attached */
#endif
   (*lft_ptr->strlft->lft_ksvtbl[KSVALARM])(lft_ptr->dds_ptr->kbd.devno,&sound);
   return(0);
}

/*-------------------------------------------------------------------
 *
 *  upd_cursor:
 *  This routine makes a call to vttmovc to update cursor position.
 *  (Only here to extract vttmovc of lftte module)
 *
 *-------------------------------------------------------------------------*/
upd_cursor(struct vtmstruc *vp)
{
   (*vp->display->vttmovc)(vp);
   return(0);
}

                        /*******************/
                        /* CURSOR ROUTINES */
                        /*******************/


/*----------------------------------------------------------------------
 *      cursor_up:  CPL, CUU, RI
 *
 *      This routine moves the cursor upward the number of rows re-
 *      quested in the escape sequence. If the request was for a nonpositive
 *      amount, the cursor is moved up 1 row; if the amount requested
 *      keeps the cursor on the screen, a simple subtract is done;
 *      if the request would move the cursor off the screen, a check
 *      is done to see if wrap is on: if wrap is on, wrap is done. If
 *      wrap is not on, the top row of the screen is postioned to (and
 *      reflected in vp->mparms.cursor.y).
 *-------------------------------------------------------------------------*/
cursor_up(struct vtmstruc *vp, struct down_stream *down)
{
   if (down->parm[0] < 1)
      down->parm[0] = 1;

   if (vp->mparms.cursor.y > down->parm[0])      /* fits on screen */
      vp->mparms.cursor.y -= down->parm[0];

   else
      if (lft_ptr->strlft->lft_mode_flag & LFWRAP) {      /* wrap at boundary */
        /*------------
          Doesn't fit on screen.   Go to bottom and continue to move up.
          For example, if request=5 and y=3, move to bottom of
          screen and then come back up 2 (5 - 3).  Mod by the screen
          height in case a large request would wrap screen more than once.
          ------------*/
         vp->mparms.cursor.y = SCR_HEIGHT
                  - ((down->parm[0] - vp->mparms.cursor.y) % SCR_HEIGHT);
      }
      else                              /* don't wrap -- goto top of screen */
         vp->mparms.cursor.y = 1;
   return(0);
}


/*-------------------------------------------------------------------
 *      cursor_down:  CUD, CNL
 *
 *      This routine moves the cursor down as many rows on the screen
 *      as were indicated in the escape sequence. If a 0 or neg val
 *      was supplied by the application, then 1 is used; if the request
 *      is for a row within the current screen, then a simple add is
 *      done and stored in the row coord(vp->mparms.cursor.y in this case).
 *      If the request is for a row outside of the current screen,
 *      then, if wrap is on wrap is done; if wrap is not on the last
 *      row on the screen is positioned to (and reflected in
 *      vp->mparms.cursor.y).
 *-------------------------------------------------------------------------*/
cursor_down(struct vtmstruc *vp, struct down_stream *down)
{
    int remaining_lines;

    if (down->parm[0] < 1)
       down->parm[0] = 1;

    remaining_lines = SCR_HEIGHT - vp->mparms.cursor.y;
    if (down->parm[0] <= remaining_lines) /* fits on screen */
       vp->mparms.cursor.y += down->parm[0];
    else
       if (lft_ptr->strlft->lft_mode_flag & LFWRAP) {    /* wrap at boundary */
        /*------------
          doesn't fit on screen.   Go to top and continue to move down.
          For example, if request=5 and there are 3 lines remaining on
          the screen, move to top of screen and then come down
          2 more (5 - 3).  Mod by the screen height in case a large request
          would wrap screen more than once.
          ------------*/
          vp->mparms.cursor.y = 1
                 + ((down->parm[0] - remaining_lines - 1) % SCR_HEIGHT);
       }
       else                              /* don't wrap -- goto screen bottom */
          vp->mparms.cursor.y = SCR_HEIGHT;
   return(0);
}


/*----------------------------------------------------------------------
 *      cursor_left: CUB
 *
 *      This routine moves the cursor the number of columns indicated by
 *      the escape sequence sent down by the application.
 *-------------------------------------------------------------------------*/
cursor_left(struct vtmstruc *vp,struct down_stream *down)
{
    int num_lines;

    if (down->parm[0] < 1)
       down->parm[0] = 1;
    if (vp->mparms.cursor.x > down->parm[0])      /* fits on line */
       vp->mparms.cursor.x -= down->parm[0];
    else
        /*------------
          Doesn't fit on line.   Calculate number of lines to move
          if AUTONL is on.  If lines don't fit on screen, then go to
          bottom and continue to move up.  For example, if lines=5 and y=3,
          move to bottom of screen and then come back up 2 (5 - 3).
          Mod by the screen height in case a large request would wrap
          screen more than once.
          ------------*/
       if (lft_ptr->strlft->lft_mode_flag & LFWRAP) {            /* wrap at boundary */

          if (down->ds_mode_flag & _AUTONL) {       /* go to next line */
             num_lines = 1 +
                    ((down->parm[0] - vp->mparms.cursor.x) / SCR_WIDTH);
             if (num_lines < vp->mparms.cursor.y)
                vp->mparms.cursor.y -= num_lines;
             else
                vp->mparms.cursor.y = SCR_HEIGHT -
                        ((num_lines - vp->mparms.cursor.y) % SCR_HEIGHT);
          }
          vp->mparms.cursor.x =
               SCR_WIDTH - ((down->parm[0]-vp->mparms.cursor.x)%SCR_WIDTH);
       }
       else                            /* don't wrap -- goto screen margin */
          vp->mparms.cursor.x = 1;
   return(0);
}


/*----------------------------------------------------------------------
 *      cursor_right:  CUF
 *
 *      This routine computes a new cursor position to the right of the
 *      current position; the number of columns to move right is provided
 *      by the application in an escape sequence; if the amount provided
 *      is zero or negative, then a move of one column is used. If the
 *      move is on-screen, then the new column position is computed simply,
 *      from the current position and the amount to move right from the input.
 *      If the move would result in going off-screen, then a check is
 *      made to see if wrap is on: if it is, a check is made to see if
 *      autonewline is on: if it is then if the new position goes over a
 *      line boundary, that is accounted for; if wrap is not on (and
 *      the move is off-screen), then the rightmost col is selected.
 *-------------------------------------------------------------------------*/
cursor_right(struct vtmstruc *vp,struct down_stream *down)
{
    int num_lines;
    int remaining_lines;

    if (down->parm[0] < 1)
       down->parm[0] = 1;

    remaining_lines = SCR_HEIGHT - vp->mparms.cursor.y;
    if (down->parm[0] <= (SCR_WIDTH - vp->mparms.cursor.x))
       vp->mparms.cursor.x = vp->mparms.cursor.x + down->parm[0];
    else
        /*------------
          Doesn't fit on line.   Calculate number of lines to move
          if AUTONL is on.  If lines don't fit on screen, then go to
          top and continue to move down.  For example, if lines=5 and
          there are 3 lines remaining on the screen, move to the top
          of the screen and then come down 2 more (5 - 3).
          Mod by the screen height in case a large request would wrap
          screen more than once.
          ------------*/
       if (lft_ptr->strlft->lft_mode_flag & LFWRAP) {            /* wrap at boundary */

          if (down->ds_mode_flag & _AUTONL) {       /* go to next line */
             num_lines = (down->parm[0] + vp->mparms.cursor.x)/SCR_WIDTH;
             if (num_lines <= remaining_lines)       /* fits on screen */
                vp->mparms.cursor.y += num_lines;
             else                            /* doesn't fit on screen */
                vp->mparms.cursor.y = 1
                       + ((num_lines - remaining_lines - 1) % SCR_HEIGHT);
          }

          vp->mparms.cursor.x =
                (down->parm[0] + vp->mparms.cursor.x) % SCR_WIDTH;

       }
       else                            /* don't wrap -- goto screen margin */
          vp->mparms.cursor.x = SCR_WIDTH;
   return(0);
}


/*----------------------------------------------------------------------
 *      cursor_absolute:  CPR, CUP, HVP
 *
 *      This routine sets the screens row,column (vp->mparms.cursor.y,
 *      vp->mparms.cursor.x) coordinates.  The value of the coordinates
 *      comes from the escape sequence the application has sent down.
 *-------------------------------------------------------------------------*/
cursor_absolute(struct vtmstruc *vp, struct down_stream *down)
{
   if (down->parm[0] == 0)                /* y coordinate (vertical) */
      vp->mparms.cursor.y = 1;
   else {
      if (down->parm[0] <= SCR_HEIGHT)
         vp->mparms.cursor.y = down->parm[0];
      else
         vp->mparms.cursor.y = SCR_HEIGHT;
   }

   if (down->parm[1] == 0)                /* x coordinate (horizontal) */
      vp->mparms.cursor.x = 1;
   else
      if (down->parm[1] < SCR_WIDTH)
         vp->mparms.cursor.x = down->parm[1];
      else
         vp->mparms.cursor.x = SCR_WIDTH;

   return(0);
}


                        /*************************/
                        /* DELETE, ERASE, INSERT */
                        /*************************/


/*----------------------------------------------------------------------
 * Delete chars on a line subroutine:  DCH
 *
 * This routine causes data from the cursor x position for Parm positions
 * to be lost. The data following that (if any) is moved left Parameter
 * positions and the line length is decremented accordingly.
 *-------------------------------------------------------------------------*/
delete_char(struct vtmstruc *vp, struct down_stream *down)
{
   int remaining_len;
   ushort parm0;                    /* from down->parm[0] */

   /* adjust parameter if necessary                                           */
   if (down->parm[0] < 1)
      parm0 = 1;
   else
      parm0 = down->parm[0];

   /* set len from cursor to ps_width                                         */
   remaining_len = SCR_WIDTH - vp->mparms.cursor.x + 1;

                                      /* if requested to delete fewer chars
                                       * than there are spaces in presentation
                                       * space, move remaining chars to the
                                       * left
                                       */

   if (parm0 < remaining_len) {
      copy_part(vp, (remaining_len - parm0),
            vp->mparms.cursor.x + parm0,
            vp->mparms.cursor.y,
            vp->mparms.cursor.x,
            vp->mparms.cursor.y);
   }
   else /* requested to delete more chars than there are up to PS width */
      if (parm0 > remaining_len)
         parm0 = remaining_len;

                                 /* clear newly opened space at right of line */
   clear_rect(vp, (SCR_WIDTH - parm0 + 1),
      vp->mparms.cursor.y,
      SCR_WIDTH,
      vp->mparms.cursor.y);

                                      /* decrement line length by parameter */
   down->line_data[vp->mparms.cursor.y].line_length =
           max(0, (down->line_data[vp->mparms.cursor.y].line_length - parm0));
   return(0);
}


/*----------------------------------------------------------------------
 *      delete_line:  DL
 *
 *      This routine causes the lines from the cursor line for PARAM-
 *      ETER - 1 positions to be lost, where PARAMETER is a value
 *      received from the application in the escape sequence.  The data
 *      following that (if any) is scrolled up PARAMETER positions and
 *      the line lengths are scrolled accordingly.
 *-------------------------------------------------------------------------*/
delete_line(struct vtmstruc *vp,struct down_stream *down)
{
   int remaining_lines;
   int i;
                                      /* adjust input parameter as necessary */
   if (down->parm[0] < 1)
      down->parm[0] = 1;

                  /* determine number of lines left from cursor to ps_height. */
   remaining_lines = SCR_HEIGHT - vp->mparms.cursor.y + 1;

                /* Condition:
                 *     requested to delete fewer lines than there are remaining:
                 *     move lines below deletion up and also move line length up
                 */

   if (down->parm[0] < remaining_lines) {
         (*vp->display->vttcfl)(vp, vp->mparms.cursor.y + down->parm[0],
                                vp->mparms.cursor.y,
                                remaining_lines - down->parm[0],
                                0);

      for (i = vp->mparms.cursor.y + down->parm[0]; i <= SCR_HEIGHT; ++i)
         down->line_data[i - down->parm[0]].line_length =
                                       down->line_data[i].line_length;
   }
          /* Condition:
           *   rqstd to delete more lines than there are from cursor to PS len;
           *   reset down->parm[0]; clear newly opened lines at bottom of screen;
           *   zero line lengths of cleared lines.
           */
   else
      if (down->parm[0] > remaining_lines)
         down->parm[0] = remaining_lines;

   clear_rect(vp, 1, SCR_HEIGHT - down->parm[0] + 1,
              SCR_WIDTH, SCR_HEIGHT);

   for (i = SCR_HEIGHT - down->parm[0] + 1; i <= SCR_HEIGHT; ++i)
      down->line_data[i].line_length = 0;
   return(0);
}


/*----------------------------------------------------------------------
 *      erase_l:  Erase a line, a field or an area:  EA, EL
 *      EA is treated line an EL
 *
 *      This routine deals with no more than 1 line of the screen.
 *	The value sent in the escape sequence, currently stored
 *      in the down->parm array is used to decide what to erase (0 =
 *      erase to end; 1 = erase from start; 2 = erase all).  A call to
 *      clear_rect (clear rectangle) is made, which actually does the
 *      work, after this routine sets up the call.
 *-------------------------------------------------------------------------*/
erase_l(struct vtmstruc *vp, struct down_stream *down)
{
   int xstart, xstop;

   xstart = 1;                     /* set up defaults */
   xstop  = SCR_WIDTH;

   /* select according to the parameter input: */
   switch (down->parm[0]) {
      case 0:         /* erase to end of line */
         xstart = vp->mparms.cursor.x;
         break;

      case 1:         /* erase from start of line */
         xstop = vp->mparms.cursor.x;
         break;

      case 2:         /* erase entire line */
         break;

      default:        /* illegal param: ignore it. */
         return(0);
   }

   /* if (start <= line_length and stop >= line_length), we have erased
      the end of the line.  Set the line length to the new end. */
   if (xstop >= down->line_data[vp->mparms.cursor.y].line_length)
      if (xstart <= down->line_data[vp->mparms.cursor.y].line_length)
         down->line_data[vp->mparms.cursor.y].line_length = xstart - 1;

   /* clear part of line */
   clear_rect(vp, xstart, vp->mparms.cursor.y,
              xstop,  vp->mparms.cursor.y);

   return(0);
}

/*----------------------------------------------------------------------
 *      erase_char:  ECH
 *
 *      This routine erases character(s) from a line. If the erase
 *      took place at the end of a line, the line length is altered.
 *
 *      The application sends down how many characters are to be
 *      erased. This parameter is looked at for reasonableness.
 *      This implies that if input was negative value, it is set to 1.
 *      Furthermore, if requested to erase more than from cur col posn to
 *      end of line, then only the portion from curr pos to end of
 *      line is done.
 *
 *      When erasing is at end of line, line length is adjusted.
 *-------------------------------------------------------------------------*/
erase_char(struct vtmstruc *vp, struct down_stream *down)
{

   if (down->parm[0] < 1)
      down->parm[0] = 1;

   /* trim if necessary (user asked that more chars be erased
      than there were on the remainder of the line) */
   if (down->parm[0] > (SCR_WIDTH - vp->mparms.cursor.x + 1))
      down->parm[0] =  SCR_WIDTH - vp->mparms.cursor.x + 1;

   /* clear out space: */
   clear_rect(vp, vp->mparms.cursor.x, vp->mparms.cursor.y,
              vp->mparms.cursor.x + down->parm[0] - 1,
              vp->mparms.cursor.y);

   /* see if line lengths need to be altered. Necessary only if we     */
   /* obliterated the tail end of the line. If only an internal erase, */
   /* don't mess with line length.                                     */

   if ((vp->mparms.cursor.x + down->parm[0] - 1) >=
       down->line_data[vp->mparms.cursor.y].line_length)
      down->line_data[vp->mparms.cursor.y].line_length = vp->mparms.cursor.x - 1;
   return(0);
}


/*----------------------------------------------------------------------
 *      erase_display:  ED
 *
 *  This routine clears all or part of the screen, depending upon
 *  the request from the application.
 *-------------------------------------------------------------------------*/
erase_display(struct vtmstruc *vp, struct down_stream *down)
{
   int xstart, ystart;
   int xstop, ystop;
   int i;

   switch (down->parm[0]) {
      case 0:                           /* erase to end of display */
         xstart = vp->mparms.cursor.x;
         ystart = vp->mparms.cursor.y;
        /*------------
          Clear to end of line.  Then reset the line line length
          to x, the new end of line.
          ------------*/
         clear_rect(vp, xstart, ystart, SCR_WIDTH, ystart);

         if (down->line_data[vp->mparms.cursor.y].line_length >
             vp->mparms.cursor.x - 1)    /* cursor is in line */
            down->line_data[vp->mparms.cursor.y].line_length =
                                               vp->mparms.cursor.x - 1;
        /*------------
          Set the line length to 0 for every line below the cursor.
          If the cursor is not on the bottom line, clear every line
          below the cursor.
          ------------*/
         for (i = vp->mparms.cursor.y + 1; i <= SCR_HEIGHT; ++i)
            down->line_data[i].line_length = 0;
         if (ystart != SCR_HEIGHT) { /* cursor not on bottom line */
            clear_rect(vp, 1, ystart+1, SCR_WIDTH, SCR_HEIGHT);
         }

         break;

      case 1:                           /* erase to start of display  */
         xstop = vp->mparms.cursor.x;
         ystop = vp->mparms.cursor.y;
        /*------------
          Clear to beginning of line.  If the cursor is beyond the
          line length, clear the whole line and set its length to zero.
          Otherwise, only part of the line got cleared.
          ------------*/
         clear_rect(vp, 1, ystop, xstop, ystop);

         if (down->line_data[vp->mparms.cursor.y].line_length
             <= vp->mparms.cursor.x)
            down->line_data[vp->mparms.cursor.y].line_length = 0;

        /*------------
          Set the line length to 0 for every line above the cursor.
          If the cursor is not on the top line, clear every line
          above the cursor.
          ------------*/
         for (i = 1; i <= vp->mparms.cursor.y - 1; ++i)
             down->line_data[i].line_length = 0;
         if (ystop != 1) {
            clear_rect(vp, 1, 1, SCR_WIDTH, ystop - 1);
         }

         break;

      case 2:                                 /* erase all display */
        /*------------
          Set each line length to 0.  Clear the entire display.
          ------------*/
         for (i = 1; i <= SCR_HEIGHT; ++i)
            down->line_data[i].line_length = 0;
         clear_rect(vp, 1, 1, SCR_WIDTH, SCR_HEIGHT);
         break;

      default:                /* illegal parameter */
         return(0);
   }

   return(0);
}


/*----------------------------------------------------------------------
 *      insert_char: ICH
 *  Inserts PN empty spaces before the character indicated by the cursor.
 *  Characters beginning at the cursor are shifted right.  Characters
 *  shifted past the right margin are lost.
 *
 *      This routine builds a string of PARAMETER length, (where PARAM-
 *      ETER is a value picked up from escape sequence sent down from
 *      the application) and sends it to the screen_updat  routine for 
 *	insertion.  The effect of this control differs however from an 
 *	insert in insert mode in that the
 *      cursor x position remains constant rather than moving with the
 *      shifted string (if any) to the right.
 *
 *-------------------------------------------------------------------------*/
insert_char(struct vtmstruc *vp, struct down_stream *down)
{
   int i;
   short old_ds_modes;                  /* save datastream mode flag */
   struct vtt_cursor old_cursor;        /* save cursor position */
   int  fill_string_length;             /* local copy of down->parm[0] */
   char fill_string[SCR_WIDTH];         /* pointer to allocated area */
   char *p;                             /* aux pointer to scan fill_string */

   /*-----------
     As we lost the characters shifted past the right margin and the screen
     length is fixed to SCR_WIDTH (80) a local buffer of 80 char is sufficient.
     So we don't have to malloc as in HFT.
     ------------*/
   fill_string_length = down->parm[0];

   if (fill_string_length < 1)
      fill_string_length = 1;

   if (fill_string_length > SCR_WIDTH)
      fill_string_length = SCR_WIDTH;

   /*-----------
     save the current modes; set insert; reset autonl.
     If string is more than can fit on this line, turning off autonl
     will prevent overflow from being inserted on next line
     ------------*/
   old_ds_modes = down->ds_mode_flag;
   down->ds_mode_flag |= _IRM;                  /* turn on insert    */
   down->ds_mode_flag &= ~_AUTONL;              /* turn off autonl   */
   old_cursor = vp->mparms.cursor;        /* save so ps_updat doesn't chg it*/

   /*-----------
     blank the fill string
     ------------*/
   for (i = 0; i < fill_string_length; ++i)
      fill_string[i] = ' ';

   /* update presentation space and adjust line length */
   screen_updat(vp, down, fill_string, fill_string_length);

   /* restore values */
   down->ds_mode_flag = old_ds_modes;
   vp->mparms.cursor = old_cursor;
   return(0);
}


/*----------------------------------------------------------------------
 *      insert_line: IL
 *
 *      This routine scrolls the cursored line and all lines following
 *      it down PARAMETER lines, where PARAMETER is a value received
 *      from the escape sequence sent down by the application, and
 *      inserts PARAMETER blank lines.  The cursor does not change
 *      absolute screen position: after the control is performed, the
 *      cursor will be on the first inserted blank line.
 *      PARAMETER is stored by now in down->parm[0] and has been sent
 *      down from the application in the escape sequence.
 *-------------------------------------------------------------------------*/
insert_line(struct vtmstruc *vp, struct down_stream *down)
{
   int i;
   int start_old_lines;

                        /* adjust down->parm if necessary */
   if (down->parm[0] < 1)
      down->parm[0] = 1;
   if (down->parm[0] > SCR_HEIGHT)
      down->parm[0] = SCR_HEIGHT;

                       /* set number of lines remaining
                        * after the scroll down
                        */

   start_old_lines = vp->mparms.cursor.y + down->parm[0];

   /*------------
     lines that scroll down will not be pushed off the screen:
     vttcfl copies full lines.
     ------------*/
   if (start_old_lines <= SCR_HEIGHT) {
         (*vp->display->vttcfl)(vp, vp->mparms.cursor.y, start_old_lines,
                  (SCR_HEIGHT - vp->mparms.cursor.y - down->parm[0] + 1), 0);
   }
   else
      start_old_lines = SCR_HEIGHT + 1;

   /* blank newly inserted lines */
   clear_rect(vp, 1,            vp->mparms.cursor.y,
                  SCR_WIDTH, start_old_lines - 1);

   for (i = SCR_HEIGHT; i >= start_old_lines; --i)
      /* scroll ln lengths */
      down->line_data[i].line_length = down->line_data[i-down->parm[0]].line_length;

   for (i = vp->mparms.cursor.y; i <= start_old_lines - 1; ++i)
                 /* zero new ln lngths     */
      down->line_data[i].line_length = 0;

   return(0);
}


                        /*******************/
                        /* SCROLL ROUTINES */
                        /*******************/


/*----------------------------------------------------------------------
 *      scroll_up:  SU
 *
 *      This routine moves all the presentation space lines up PARAM-
 *      ETER positions. PARAMETER lines at the top will be lost and
 *      PARAMETER blank lines will be inserted at the bottom, where PARAM-
 *      ETER refers to the number of lines indicated in the escape seq-
 *      uence sent down from the application.
 *
 *-------------------------------------------------------------------------*/
scroll_up(struct vtmstruc *vp, struct down_stream *down)
{
   int lines_to_scroll;
   int i;

   if (down->parm[0] < 1)
      lines_to_scroll = 1;
   else
      lines_to_scroll = down->parm[0];
      if (lines_to_scroll > SCR_HEIGHT)
         lines_to_scroll = SCR_HEIGHT;       /* can't scroll more lines */
                                                /* than exist */

      (*vp->display->vttscr)(vp, lines_to_scroll,
            vp->mparms.attributes, vp->mparms.cursor, 0);

   /* move the line lengths up by number of lines scrolled */
   for (i = 1; i <= SCR_HEIGHT - lines_to_scroll; ++i)
      down->line_data[i].line_length =
                            down->line_data[i + lines_to_scroll].line_length;

   /* new lines on bottom have 0 line length */
   for (i = SCR_HEIGHT - lines_to_scroll + 1; i <= SCR_HEIGHT; ++i)
      down->line_data[i].line_length = 0;

   return(0);
}


/*----------------------------------------------------------------------
 *      scroll_down: SD
 *
 *      This routine moves all the presentation space lines down PARAM-
 *      ETER positions. PARAMETER lines at the bottom will be lost and
 *      PARAMETER blank lines will be inserted at the top.
 *      PARAMETER, is, of course, the numeric value sent down from the
 *      application in the escape sequence.
 *-------------------------------------------------------------------------*/
scroll_down(struct vtmstruc *vp, struct down_stream *down)
{
   int i;
   int lines_to_scroll;

   if (down->parm[0] < 1)
      lines_to_scroll = 1;
   else
      lines_to_scroll = down->parm[0];
      if (lines_to_scroll > SCR_HEIGHT)
         lines_to_scroll = SCR_HEIGHT;

      (*vp->display->vttscr)(vp, -lines_to_scroll,
                             vp->mparms.attributes, vp->mparms.cursor, 0);

   for (i = SCR_HEIGHT; i >= lines_to_scroll + 1; --i)
      down->line_data[i].line_length =
                              down->line_data[i - lines_to_scroll].line_length;

   for (i = 1; i <= lines_to_scroll; ++i)
      down->line_data[i].line_length = 0;

   return(0);
}



                        /**************************/
                        /* MISCELLANEOUS ROUTINES */
                        /**************************/


/*----------------------------------------------------------------------
 *      set_attributes:  SGR (set graphic rendition)
 *
 *      This routine sets the graphic rendition:
 *      The attributes of reverse video, blink, etc can be set as well
 *      as the foreground and background colors.
 *
 *      The SET_... macros are in vt.h
 *-------------------------------------------------------------------------*/
set_attributes(struct vtmstruc *vp, struct down_stream *down)
{
   int i,j;

   for (i = 0; i < down->numparm; ++i) {
      switch (down->parm[i]) {
         case 0:
            vp->mparms.attributes &= ~0x1f;
            break;
         case 1:
            vp->mparms.attributes |= BRIGHT_MASK;
            break;
         case 4:
            vp->mparms.attributes |= UNDERSCORE_MASK;
            break;
         case 5:
            vp->mparms.attributes |= BLINK_MASK;
            break;
         case 7:
            vp->mparms.attributes |= REV_VIDEO_MASK;
            break;
         case 8:
            vp->mparms.attributes |= NO_DISP_MASK;
            break;

         default:
            break;
      }  /* end switch */
   } /* end for */
   return(0);
}


/*----------------------------------------------------------------------
 * update_ds_modes:  RM,SM
 *
 *      This routine sets the datastream modes. The mode set is indic-
 *      ated by the parameter supplied in the escape sequence sent down
 *      from the application. The modes honored are Autonewline, Car-
 *      riage return-Newline, Insert-replace, Send-receive, Tab stop,
 *      Line feed-new line mode.
 *-------------------------------------------------------------------------*/
update_ds_modes(struct vtmstruc *vp, struct down_stream *down, int set)
{
   int i;

   for (i = 0; i < down->numparm; ++i) {
      switch (down->parm[i]) {
         case 4:         /* insert - replace mode                    */
            if (set)
               down->ds_mode_flag |= _IRM;
            else
               down->ds_mode_flag &= ~_IRM;
            break;

         case 18:        /* Tab stop mode                            */
            if (set)
               down->ds_mode_flag |= _TSM;
            else
               down->ds_mode_flag &= ~_TSM;
            break;

         case 20:        /* Line feed, new line                      */
            if (set)
               down->ds_mode_flag |= _LNM;
            else
               down->ds_mode_flag &= ~_LNM;
            break;

         case 157:       /* Autonl = 0x9d = "?7"                     */
            if (set)
               down->ds_mode_flag |= _AUTONL;         /* go to next line ON */
            else
               down->ds_mode_flag &= ~_AUTONL;        /* go to next line OFF */
            break;

         case 1521:      /* CR/NL = 0x5f1 = "?21"                    */
            if (set)
               down->ds_mode_flag |= _CNM;
            else
               down->ds_mode_flag &= ~_CNM;
            break;
         default:
	    break;
      }
   }
   return(0);
}


                        /********/
                        /* TABS */
                        /********/


/*----------------------------------------------------------------------
 *      Find Next Tab Subroutine: called by vtmout() when it finds a
 *      horizontal tab in the data stream, cursor_ht() in this module.
 *
 *      NOTE: This function should never be called when the cursor is on the
 *      character position at PS width, except for the code to implement
 *      the erase field function.
 *
 *      find_next_tab:
 *      This routine finds the next tab by examining the tab array for this
 *      terminal and returning the cursor x/y coordinates of that point.
 *      It takes wrap and autonewline into consideration.
 *-------------------------------------------------------------------------*/
find_next_tab(struct vtmstruc *vp, struct down_stream *down, int *newx, int *newy)
{
   int found;                /* will transition from FALSE to TRUE */
   uint tab_word;            /* local copy of tab rack character */
   int tab_mask;             /* mask computed based upon x position mod 8 */
   int xpos;                 /* scans columns to right of cursor, looking */
                             /* for a tab stop. */

   *newx = vp->mparms.cursor.x;
   *newy = vp->mparms.cursor.y;

   /*------------
     If the cursor is at the right margin, the next tab is in column 1
     of this line or the next line (if AUTONL is on).
     ------------*/
   if (vp->mparms.cursor.x == SCR_WIDTH) {
      if (lft_ptr->strlft->lft_mode_flag & LFWRAP) {      /* wrap at boundary ON */
         *newx = 1;                     /* wrap to beginning of same line */
         if (down->ds_mode_flag & _AUTONL)    /* go to next line */
            if (vp->mparms.cursor.y == SCR_HEIGHT)
               *newy = 1;
            else
               ++(*newy);
      }
      return(0);
   }

   /*------------
     We are looking for a tab on this line.  The tab_settings
     array is a bitmap ("tab rack") with the tab stops
     represented by 1 bits.  The order is left-to-right, i.e.,
     the leftmost bit (0x80000000) represents a tab stop at column 1.

     Local xpos is initialized to the cursor x value.  Since it will be
     used to index into the tab rack, it starts out pointing to the column
     to the right of the cursor x position.

     We will loop through the tab rack left-to-right:  for each word of the
     tab rack (tab_word), we will create a tab mask (tab_mask) based upon the
     xpos modulo 32.  If a tab has been set at the current x position,
     we will set found flag to TRUE and break out of the loops.  If no
     tabs are set, we will force the cursor x position to the rightmost
     value (ps_width).

     For example, if cursor.x = 1 (at left margin):
        xpos starts at 1.  xpos / 32 = 0, so use 0 as index into tab_settings.
        xpos % 32 = 1 so shift our mask (0x80000000) right 1 (i.e. examine the
        second bit of tab_settings[0]). increment xpos (xpos now equals 2).
        if this bit is set then we are done and xpos represents the cursor
        position of the next bit (2, or one over from the left margin).
     ------------*/
   found = FALSE;
   for (xpos = vp->mparms.cursor.x; !found && xpos < SCR_WIDTH;)  {
      tab_word = down->line_data[vp->mparms.cursor.y].tab_settings[xpos >> 5];
      do {
         tab_mask = 0x80000000 >> (xpos & 0x1F);
         ++xpos;                          /* if tab stop here, xpos will be an
                                             actual cursor coordinate */
         if (tab_mask & tab_word) {
            found = TRUE;
            break;
         }
      }  while (((xpos & 0x1F) != 0) && xpos < SCR_WIDTH);
   }
   *newx = xpos;
   return(0);
}


/*----------------------------------------------------------------------
 *      Find prior Tab Subroutine. Called by cursor_back_tab() in this module.
 *
 *      This routine finds the prev tab by examining the tab array for this
 *      terminal and returning the cursor x/y coordinates of that point.
 *      It takes wrap and autonewline into consideration.
 *-------------------------------------------------------------------------*/
find_prior_tab(struct vtmstruc *vp, struct down_stream *down, int *newx, int *newy)
{
   int found;                /* will transition from FALSE to TRUE */
   uint tab_word;            /* local copy of tab rack character */
   uint tab_mask;            /* mask computed based upon x position mod 8 */
   int xpos;                 /* scans columns to left of cursor, looking */
                             /* for a tab stop. */


   *newy = vp->mparms.cursor.y;
   *newx = vp->mparms.cursor.x;
   /*------------
     If the cursor is at the left margin, the next tab is at the right
     margin of this line or the previous line (if AUTONL is on).
     ------------*/
   if (vp->mparms.cursor.x == 1) {
      if (lft_ptr->strlft->lft_mode_flag & LFWRAP) {      /* wrap to beginning of same line */
         *newx = SCR_WIDTH;
         if (down->ds_mode_flag & _AUTONL)    /* go to next line */
            if (vp->mparms.cursor.y == 1)
               *newy = SCR_HEIGHT;
            else
               --(*newy);
      }
      return(0);
   }
   /*------------
     We are looking for a tab on this line.  The tab_settings
     array is a bitmap ("tab rack") with the tab stops
     represented by 1 bits.  The order is left-to-right, i.e.,
     the leftmost bit (0x80000000) represents a tab stop at column 1.

     Local xpos is initialized to the cursor x value - 2.  Since it will be
     used to index into the tab rack, it starts out pointing to the column
     to the left of the cursor x position.

     We will loop through the tab rack right-to-left:  for each word of the
     tab rack (tab_word), we will create a tab mask (tab_mask) based upon
     shifting a leftmost bit in a word (0x80000000) right by xpos modulo 32.
     If a tab has been set at the current x position,
     we will set found flag to TRUE and break out of both loops.  If no
     tabs are set, we will force the cursor x position to the leftmost
     column (1).
     ------------*/
   found = FALSE;
   for (xpos = vp->mparms.cursor.x - 2; !found && xpos >= 0;)  {
                                /* xpos / 32 = index */
      tab_word = down->line_data[vp->mparms.cursor.y].tab_settings[xpos >> 5];
      do {
                                /* xpos % 32 = bit position in word */
         tab_mask = 0x80000000 >> (xpos & 0x1F);
         if (tab_mask & tab_word) {     /* tab is set at this position */
            found = TRUE;
            break;
         }
      }  while ((--xpos & 0x1F) != 0x1F); /* xpos % 32 going to 0 ==> quit */
   }
   if (xpos < 0)
      xpos = 0;
   *newx = xpos + 1;             /* convert to 1-relative coord system */
   return(0);
}


/*----------------------------------------------------------------------
 *      cursor_ht:  CHT
 *
 *      cursor horizontal tab:
 *      This routine finds the next horizontal tab and places the
 *      cursor there.
 *-------------------------------------------------------------------------*/
cursor_ht(struct vtmstruc *vp, struct down_stream *down)
{
   int i;
   int newx,newy;                                  /* next tab positions  */

                                          /* adjust for user as necessary */
   if (down->parm[0] < 1) down->parm[0] = 1;

   for (i = 0; i < down->parm[0]; ++i) {
      find_next_tab(vp,down, &newx, &newy);
      vp->mparms.cursor.y = newy;
      vp->mparms.cursor.x = newx;
   }
   return(0);
}


/*----------------------------------------------------------------------
 *      cursor_back_tab:
 *
 *      This routine moves the cursor back to the previous tabstop.
 *-------------------------------------------------------------------------*/
cursor_back_tab(struct vtmstruc *vp, struct down_stream *down)
{

                           /* adjust for user input as necessary */
   if (down->numparm < 1) {
      down->numparm = 1;
      down->parm[0] = 1;
   }
   if (down->parm[0] < 1)
      down->parm[0] = 1;

   while (down->parm[0]-- > 0)
      find_prior_tab(vp,down, &vp->mparms.cursor.x, &vp->mparms.cursor.y);
   return(0);
}


/*----------------------------------------------------------------------
 *      clear_all_ht:  Called by set_clear_tab()
 *
 *      clear all horizontal tabs on a line
 *-------------------------------------------------------------------------*/
clear_all_ht(struct vtmstruc *vp, struct down_stream *down)
{
    int i;
    int begin, end;

    /*------------
      Clear all bits in the tab_settings array
      ------------*/
    if (! (down->ds_mode_flag & _TSM)) {      /* tabulation stop mode OFF */
                                        /* ==> whole screen */
       begin = 1;
       end = SCR_HEIGHT;
    }
    else {                              /* one line only */
       begin = end = vp->mparms.cursor.y;
    }

    /*------------
      Set all bits in the entire tab_settings array to zero.  Then
      reset tabs at both margins, since these cannot be cleared.
      ------------*/
    for (i = begin; i <= end; ++i) {
       bzero(down->line_data[i].tab_settings,
             sizeof(down->line_data[i].tab_settings));

       down->line_data[i].tab_settings[0] = 0x80000000;
       down->line_data[i].tab_settings[SCR_WIDTH >> 5] |=
                        (SCR_WIDTH & 0x1f) == 0 ?
                        0x80000000 : 0x40000000 >> ((SCR_WIDTH & 0x1f) - 1);
    }

    return(0);
}


/*----------------------------------------------------------------------
 *      update_htstop:  HTS, set_clear_tab()
 *
 *      update horizontal tab stops:
 *      This routine either sets or clears horizontal tabs, depending
 *      upon the value of the input arg, 'set'; it either  works in the
 *      mode of applying the set/clear to the whole screen or one line.
 *-------------------------------------------------------------------------*/
update_ht_stop(struct vtmstruc *vp, struct down_stream *down, unsigned char set)
{
   unsigned j;
   ulong cur_set;
   int which;
   int begin,end;
   unsigned mask;

   /* cursor starts at 1, but array starts at 0, so subtract one */
   which = (vp->mparms.cursor.x - 1) >> 5;                   /* divide by 32 */
   mask = 0x80000000 >> ((vp->mparms.cursor.x - 1) & 0x1F);  /* mod by 32 */

   /*------------
     This code decides if we are dealing with tabbing specs for a screen's
     worth or for a line.  TSM (tabulation stop mode) determines whether
     horizontal tabs apply identically to all lines (RM, reset mode ==> off)
     or uniquely to each line on which they are set (SM, set mode --> on).
     ------------*/
   if (! (down->ds_mode_flag & _TSM)) {       /* tabulation stop mode OFF */
                                        /* ==> whole screen */
      begin = 1;
      end = SCR_HEIGHT;
   }
   else {                               /* one line only */
      begin = end = vp->mparms.cursor.y;
   }

   /* This code selects the int which holds the tab info of interest */
   for (j = begin; j <= end; ++j) {
      cur_set = down->line_data[j].tab_settings[which];

      if (set == TRUE)
         cur_set |= mask;
      else {                             /* clear */
         cur_set &= ~mask;
      }

      down->line_data[j].tab_settings[which] = cur_set;

     /*------------
       cursor stops are set at 1 and ps_width.  Cannot clear them!
       ------------*/

      down->line_data[j].tab_settings[0] |= 0x80000000;
      down->line_data[j].tab_settings[SCR_WIDTH >> 5] |=
                         0x80000000 >> (SCR_WIDTH & 0x1f);
   }
   return(0);
}


/*----------------------------------------------------------------------
 *      set_clear_tab:  CTC, TBC
 *
 *      This routine either sets or clears the tabs depending upon the
 *      info in the esc sequence.  It operates upon either a line or
 *      screen model.
 *
 *   CTC:                                  TBC:
 *      0 = set   horiz tab at cursor
 *      2 = clear horiz tab at cursor           0 = clear horiz tab at cursor
 *      4 = clear all horiz tab on line         2 = clear horiz tabs on line
 *      5 = clear all horiz tabs                3 = clear all horiz tabs
 *-------------------------------------------------------------------------*/
set_clear_tab(struct vtmstruc *vp, struct down_stream *down,unsigned int parm)
{
    int i,j;
    ushort old_ds_modes;

    i = 0;

    while (down->numparm --) {
        switch (down->parm[i++] + parm) {

        /*------------
          update_ht_stop checks _TSM to see if stop applies to one
          line or all lines
          ------------*/
        case 0:                 /* Set horizontal tab at cursor */
            update_ht_stop(vp,down, 1);
            break;

         case 2:                /* Clear horizontal tab at cursor */
            update_ht_stop(vp,down, 0);
            break;

        /*------------
          clear_all_ht checks _TSM to see if stop applies to one
          line or all lines
          ------------*/
         case 4:                /* Clear all horizontal tabs on line */
            old_ds_modes = down->ds_mode_flag;
            down->ds_mode_flag |= _TSM;
            clear_all_ht(vp,down);
            down->ds_mode_flag = old_ds_modes;
            break;

         case 5:                /* Clear all horizontal tabs */
            old_ds_modes = down->ds_mode_flag;
            down->ds_mode_flag &= ~_TSM;
            clear_all_ht(vp,down);
            down->ds_mode_flag = old_ds_modes;
            break;

        /*------------
          Documentation says to just ignore invalid parameters.
          ------------*/
         default:
            break;
      }                         /* end switch */
    }                           /* end while */
    return(0);
}

