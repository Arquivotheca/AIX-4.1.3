static char sccsid[] = "@(#)03  1.1  src/bos/kernext/lft/streams/lftte.c, lftdd, bos411, 9428A410j 10/15/93 15:11:22";
/*
 *   COMPONENT_NAME: LFTDD
 *
 *   FUNCTIONS: atoi_q
 *		lftout
 *		vtmupd
 *		vtmupd3
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

  This file contains the terminal emulator routines :
      lftout	- Entry function for terminal emulator
      vtmupd	- handles <esc>[ sequences
      vtmupd3	- handles other <esc> sequences
      atoi_q	- ascii to numerique conversion
  ------------*/
/***********************************************************
Include Files
***********************************************************/
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/malloc.h>
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

GS_MODULE(lftte);
BUGVDEF(db_lftte, 0);                  /* define initial debug level */

#define CBT     'Z'
#define CHA     'G'
#define CHT     'I'
#define CTC     'W'
#define CNL     'E'
#define CPL     'F'
#define CPR     'R'
#define CUB     'D'
#define CUD     'B'
#define CUF     'C'
#define CUP     'H'
#define CUU     'A'
#define DCH     'P'
#define DL      'M'
#define DSR     'n'

#define EA      'O'     /* Capital letter Oh. */
#define ED      'J'
#define EF      'N'
#define EL      'K'
#define ECH     'X'
#define HTS     'H'
#define HVP     'f'
#define ICH     '@'
#define IL      'L'
#define IND     'D'
#define NEL     'E'
#define KSI     'p'
#define PFK     'q'
#define RCP     'u'
#define RI      'L'
#define RIS     'c'
#define RM      'l'     /* Small letter El. */
#define SCP     's'
#define SD      'T'
#define SCR     ' '
#define SU      'S'
#define SGR     'm'
#define SM      'h'
#define TBC     'g'
#define VTA     'r'
#define VTD     'x'
#define VTL     'y'
#define VTR     'w'
#define VTS     'I'

#define  DEFPARM0  if (down->numparm == 0) {down->numparm = 1; down->parm[0] = 0;}
#define  DEFPARM1  if (down->numparm == 0) {down->numparm = 1; down->parm[0] = 1;}



/*----------------------------------------------------------
lftout:

This routine receives a write buffer from the lft,parses it and routes the sequences 
within it to routines which handle those functions. Data from the write stream is 
collected by the parser in vtmstruc for use by the sequence-handling routines.

Escape sequences which are of the form ESC [ are passed to vtmupd, others are routed 
to vtmupd3
input:
xcp	points to the write buffer passed in;
xcnt	specifies how many bytes are in the write buffer;
vp	points to the vtmstruc for the current display.

output : the display driver routines are called with the correct parameters
	 
returns:
A zero return code indicates successful operation (note: invalid sequences in the write 
stream are not considered errors - basically, the invalid sequences are ignored).
----------------------------------------------------------*/
void lftout(unsigned char *xcp, int xcnt, struct vtmstruc *vp)
{
   char *xstart;               /* original buffer start ptr */
   char *xend;                 /* original buffer end ptr */
   long xcount;                /* original buffer count */

   unsigned char c;            /* current character in buffer */
   unsigned char *orgptr;       /* points to beginning of buffer */
   int control_byte;           /* is control char if TRUE */
   int count;                  /* temporary counter */
   int i;                      /* temporary index */

   xstart=xcp;                 /* save the original buffer start pointer */
   xcount=xcnt;                /* save the original buffer count */
   xend=xcp+xcnt;              /* set the original buffer end pointer */

/* TRACE "Entering lftout" */
GS_ENTER_TRC(HKWD_GS_LFT,lftte,1,lftout,xcp,xcnt,vp,0,0);

/*------------
  Parse write buffer:  Loop thru the write buffer pointed to by xcp,
  classifying each byte and transitioning to successive states depending
  upon the type encountered.
  ------------*/
   while (xcnt-- > 0) {
      orgptr = xcp;
      c = *xcp++;
      switch (LFT_DOWN.parser_state) {
         case NORML:
         {
            control_byte = TRUE;         /* preliminary assumption */
            switch (c) {
            /*------------
              bell
              ------------*/
               case IC_BEL:
                  sound_beep(vp);
                  break;

            /*------------
              backspace
              ------------*/
               case IC_BS:
                  if (vp->mparms.cursor.x > 1)
                     vp->mparms.cursor.x = vp->mparms.cursor.x - 1;
                  break;

            /*------------
              horizontal tab
              If tab caused a wrap to next line, set x to the very
              end of the old line.
              ------------*/
               case IC_HT:
               {
                  int newx,newy;

                  find_next_tab(vp,&(LFT_DOWN), &newx, &newy);

                  if (newy != vp->mparms.cursor.y)      /* line wrap */
                     vp->mparms.cursor.x = SCR_WIDTH;/* set to rh margin */
                  else
                     vp->mparms.cursor.x = newx;        /* set to tab */
               }
               break;

            /*------------
              line feed.  If LF/NL mode is on, move cursor to front of
              next line.
              ------------*/
               case IC_LF:
                  if (LFT_DOWN.ds_mode_flag & _LNM)
                     vp->mparms.cursor.x = 1;
                  ascii_index(vp,&(LFT_DOWN));         /* move down a line */
                  break;

            /*------------
              form feed.  Got back to column 1.  Then go down 1 line.
              If we're at the bottom of the screen, ask display driver
	      to scroll up 1 line.
              ------------*/
               case IC_FF:
                  vp->mparms.cursor.x = 1;
                  ascii_index(vp,&(LFT_DOWN));         /* move down a line */
                  break;

            /*------------
              carriage return.  If CR/NL mode is on, move cursor down
              a line
              ------------*/
               case IC_CR:
                  vp->mparms.cursor.x = 1;
                  if (LFT_DOWN.ds_mode_flag & _CNM)
                     ascii_index(vp,&(LFT_DOWN));      /* move down a line */
                  break;

            /*------------
              Escape
              ------------*/
               case IC_ESC:
                  LFT_DOWN.parser_state = ESCAPED;
                  break;

            /*------------
              Anything else.
              ------------*/
               default:
                                           /* could be a bad character */
                  if (c >= 32 )
                     control_byte = FALSE;
                  break;

            }

            if (!control_byte) {
               count = 1;

            /*------------
              loop thru graphic character string, then process as a unit.
              Accept everything until a control character or a delete
              is encounted.
              ------------*/
               while (xcnt && *xcp >= 0x20 && *xcp != 0x7f) {
                  ++xcp;
                  --xcnt;
                  ++count;
               }

               screen_updat(vp, &(LFT_DOWN), orgptr, count);
            }
         }
         break;

         case ESCAPED:
         {
            switch (c) {
            /*------------
              ESC [ has been encountered:
              Clear out parse parameter buffer and go to ESCLBR state.
              ------------*/
               case '[':
                  bzero(LFT_DOWN.parse_buf, sizeof(LFT_DOWN.parse_buf));
		  LFT_DOWN.collected_bytes = 0;
		  LFT_DOWN.numparm = 0;
		  LFT_DOWN.parm[0] = 0;
		  LFT_DOWN.parm[1] = 0;
		  LFT_DOWN.parser_state = ESCLBR;
                  break;

               case '(':
                  LFT_DOWN.parser_state = ESCLPAREN;
                  break;

            /*------------
              All other ESC sequences will be handled by vtmupd3 function in
              KSR mode. If in MOM, however, only the DMI and EMI sequences
              are valid.  Ignore invalid sequences.
              ------------*/
               default:
                  LFT_DOWN.numparm = 0;
                  vtmupd3(c, vp, &(LFT_DOWN));
                  LFT_DOWN.parser_state = NORML;
                  break;
            }
         }
         break;

         case ESCLPAREN:
         {
            if (c == '0')
               LFT_DOWN.ds_mode_flag |= SCS_DETECTED;
            else if (c == 'B')
               LFT_DOWN.ds_mode_flag &= ~SCS_DETECTED;
            LFT_DOWN.parser_state = NORML;
         }
         break;

         case ESCLBR:
         {
            switch (c) {
            /*------------
              delimiter between 1st and 2nd parameters.  Convert first
              parameter to a number and save it.  Clear the parse buffer.
              ------------*/
               case ';':
                  LFT_DOWN.parm[LFT_DOWN.numparm] = atoi_q(LFT_DOWN.parse_buf);
                  LFT_DOWN.numparm++;
                  for (count = 0; count < sizeof(LFT_DOWN.parse_buf); ++count)
                     LFT_DOWN.parse_buf[count] = 0;
                  LFT_DOWN.collected_bytes = 0;
                  break;

               default:
                /*------------
                  Collecting a parameter.  Store the digit.  If get too
                  many (more than 4), we've got garbage.
                  ------------*/
                  if ( (c >= '0' && c <= '9') || c == '?') {
                     i = LFT_DOWN.collected_bytes++;
                     if (i < 4)
                        LFT_DOWN.parse_buf[i] = c;
                     else
                       LFT_DOWN.parser_state = NORML;
                  }
                /*------------
                  End of valid esc sequence.
                  If there is anything in the parse buffer, we received
                  a second parameter.  Convert it to a number.  Call
                  vtmupd to update the screen.
                  ------------*/
                  else {
                     if (LFT_DOWN.parse_buf[0]) {
                        LFT_DOWN.parm[LFT_DOWN.numparm] = atoi_q(LFT_DOWN.parse_buf);
                        LFT_DOWN.numparm++;
                     }
                     vtmupd(c, vp, &(LFT_DOWN));
                     LFT_DOWN.parser_state = NORML;
                  }
                  break;
            }
         }
         break;
      }
   }

   /*----------
     now we are updating the cursor.
     ----------*/
	   upd_cursor(vp);

/* LFTERR TRACE "leaving lftout" */
GS_EXIT_TRC0(HKWD_GS_LFT,lftte,1,lftout);
   return;
}


/*-------------------------------------------------------------------------
 *
 *      atoi_q - This routine converts an ascii string to integer.
 *      It permits characters other than the digits 0-9, specifically
 *      a question mark, to be converted to an int consistent with its
 *      collating sequence.
 *
 *-------------------------------------------------------------------------*/
atoi_q(s)
uchar *s;
{
   int n;

   n = 0;
   while (*s >= '0' && *s <= '?')
      n = 10 * n + *s++ - '0';
   return(n);
}

/*----------------------------------------------------------------------
  vtmupd   -    Escape sequence processor

                The vtmupd routine is called when an escape sequence
                of the form ESC [ has been found in the input buffer.
                The parameters within the sequence have already been
                collected by lftout and saved in the parser data
                structures in down_stream (*down).

                The last character of the sequence determines what work
                is to be done.

  Input parameters:
     c          The final character of the escape sequence

     vp         pointer to the vtmstruc.
     down	pointer to the parser info (collected bytes ...)
---------------------------------------------------------------------------*/

vtmupd(char c, struct vtmstruc *vp, struct down_stream *down)
{

/* LFTERR TRACE "Entering vtmupd" */
GS_ENTER_TRC1(HKWD_GS_LFT,lftte,1,vtmupd,c);
   switch (c)
   {
      case CBT:       /* CBT = Z; Cursor Back Tab                       */
         DEFPARM1;
         cursor_back_tab(vp,down);
         break;

      case CHA:       /* CHA = G; Cursor Horiz Absolute                 */
         DEFPARM1;
         if (down->parm[0] == 0) {
            vp->mparms.cursor.x = 1;
            break;
         }

         if (down->parm[0] < SCR_WIDTH) {
            vp->mparms.cursor.x = down->parm[0];
            break;
         }
         else
            vp->mparms.cursor.x = SCR_WIDTH;
         break;

      case CHT:       /* CHT = I; Cursor Horiz Tab                      */
         DEFPARM1;
         cursor_ht(vp,down);
         break;

      case CTC:         /* CTC = W; Cursor Tab Stop Control */
         DEFPARM0;
         set_clear_tab(vp,down,0);
         break;

      case CNL:       /* CNL = E; Cursor Next Line                      */
         DEFPARM1;
         vp->mparms.cursor.x = 1;
         cursor_down(vp,down);
         break;

      case CPL:       /* CPL = F; Cursor Preceding Line                 */
         DEFPARM1;
         vp->mparms.cursor.x = 1;
         cursor_up(vp,down);
         break;

      case CUB:       /* CUB = D; Cursor Backward                       */
         DEFPARM1;
         cursor_left(vp,down);
         break;

      case CUD:       /* CUD = B; Cursor Down                           */
         DEFPARM1;
         cursor_down(vp,down);
         break;

      case CUF:       /* CUF = C; Cursor Forward                        */
         DEFPARM1;
         cursor_right(vp,down);
         break;

      case CUP:       /* CUP = H; Cursor Position                       */
      case HVP:       /* HVP = f; Horiz and Vertical Posn.              */
         if (down->numparm != 2){	    /* home pos. (\E[H <==>\E[1;1H) */
		 down->numparm = 2;
		 down->parm[0] = 1;         /* line */
		 down->parm[1] = 1;         /* column */
	 }
         if (down->parm[0] == 0) down->parm[0] = 1;         /* line */
         if (down->parm[1] == 0) down->parm[1] = 1;         /* column */
         cursor_absolute(vp,down);
         break;

      case CUU:       /* CUU = A; Cursor Up                             */
         DEFPARM1;
         cursor_up(vp,down);
         break;

      case DCH:       /* DCH = P; Delete Character                      */
         DEFPARM1;
         delete_char(vp,down);
         break;

      case DL:        /* DL  = M; Delete Line                           */
         DEFPARM1;
         delete_line(vp,down);
         break;

      case ED:        /* ED  =  J; Erase to end of Display              */
         DEFPARM0;
         erase_display(vp,down);
         break;

      case EL:        /* EL  = K         ; Erase to end of Line */
         DEFPARM0;
         erase_l(vp,down);
         break;

      case ECH:       /* ECH = X; Erase Character                       */
         DEFPARM1;
         erase_char(vp,down);
         break;

      case ICH:       /* ICH = @; Insert Character                      */
         DEFPARM1;
         insert_char(vp,down);
         break;

      case IL:        /* IL  = L; Insert Line                           */
         DEFPARM1;
         insert_line(vp,down);
         break;

      case PFK:       /* PFK = q; PF Key Report                         */
         /* we do no work here: private cntrls not displayed       */
         break;

      case RM:        /* RM  = l (lower-case L); Reset Mode */
         update_ds_modes(vp,down, 0);
         break;

      case SD:        /* SD  = T; Scroll Down                          */
         DEFPARM1;
         scroll_down(vp,down);
         break;

      case SU:        /* SU  = S; Scroll Up                            */
         DEFPARM1;
         scroll_up(vp,down);
         break;

      case SGR:       /* SGR = m; Set Graphic Rendition                */
         DEFPARM0;
         set_attributes(vp,down);
         break;

      case SM:        /* SM  = h; Set Mode                             */
         update_ds_modes(vp,down, 1);
         break;

      case TBC:         /* TBC = g; Tab Clear */
         DEFPARM0;
         set_clear_tab(vp,down,2);
         break;

      case VTD:         /* VTD = x; stuff done in the old VTD sequ.    */
	 switch(down->parm[0]){
	 case 0:	/* set the wrap mode "ESC [ 0 x"  */
		    lft_ptr->strlft->lft_mode_flag |= LFWRAP;
		 break;
	 case 1:	/* reset the wrap mode "ESC [ 1 x" */
		    lft_ptr->strlft->lft_mode_flag &= ~LFWRAP;
		 break;
	 case 2:	/* set the kana keyboard "ESC [ 2 x" */
		    lft_ptr->strlft->lft_mode_flag |= LFJKANA;
		 break;
	 case 3:	/* reset the kana keyboard "ESC [ 3 x" */
		    lft_ptr->strlft->lft_mode_flag &= ~LFJKANA;
		 break;
	 }
         break;

      default:
/* LFTERR TRACE "Ignored Multibyte control code" */
	lfterr(NULL,"LFTDD", "vtmupd", NULL, 0, LFT_STR_UXPESCS, UNIQUE_1);
	 
         break;
   }

/* LFTERR TRACE "Leaving vtmupd" */
GS_EXIT_TRC0(HKWD_GS_LFT,lftte,1,vtmupd);
   return;
}
/*----------------------------------------------------------------------
 *     vtmupd3
 *
 *     FUNCTION: To manage miscellaneous escape sequences
 *
 *     SYNOPSIS:
 *
 *        This component of the mode processor handles the escape
 *        sequences that do not have the '[' char in the sequence.
 *        Among this set are the controls to disable and enable man-
 *        ual input, index and reverse index, move the cursor to the
 *        next line, reset to initial state, set vertical and horiz-
 *        ontal tab stops.
 *
 *     INPUT:
 *
 *        c    - the final character of the escape sequence sent
 *               down by the application.
 *
 *        vp   - pointer to terminal data structure.
 *	  down - pointer to the parser info (collected bytes ...)
 *-------------------------------------------------------------------------*/
vtmupd3(char c, struct vtmstruc *vp, struct down_stream *down)
{

/* TRACE "Entering vtmupd3" */
GS_ENTER_TRC1(HKWD_GS_LFT,lftte,1,vtmupd3,c);
  switch (c) {
     case HTS:       /* HTS = H; Horiz Tab Stop                        */
        update_ht_stop(vp,down, 1);
        break;

     case IND:       /* IND = D; Index                                 */
        ascii_index(vp,down);           /* move down 1 line, maybe scroll */
        break;

     case NEL:       /* ENL = E; Cursor Next Line                      */
        vp->mparms.cursor.x = 1;
        ascii_index(vp,down);           /* move down 1 line, maybe scroll */
        break;

     case RI :       /* RI  = L; Reverse Index                         */
        down->parm[0] = 1;
        cursor_up(vp,down);
        break;

     default:        /* default; not one of the above                 */
/* LFTERR TRACE "Ignored multibyte control code" */
	lfterr(NULL,"LFTDD", "vtmupd3", NULL, 0, LFT_STR_UXPESCS, UNIQUE_2);
        break;
   }
/* TRACE "Leaving vtmupd3" */
GS_EXIT_TRC0(HKWD_GS_LFT,lftte,1,vtmupd3);
   return;
}
