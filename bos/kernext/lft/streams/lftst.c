static char sccsid[] = "@(#)02  1.4  src/bos/kernext/lft/streams/lftst.c, lftdd, bos411, 9428A410j 7/6/94 10:50:59";
/*
 *   COMPONENT_NAME: LFTDD
 *
 *   FUNCTIONS: check_for_dead_key
 *		data_stream_intr
 *		dead_state_process
 *		expand
 *		get_xlate_table_index
 *		handle_xlated_key
 *		key_compose
 *		lftst
 *		process_key
 *		to_char
 *		vtm_key
 *
 *   ORIGINS: 27, 83
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*------------

  This file contains the scancode translator routines :
      lftst
      get_xlate_table_index - index swkb table / keyword->status
      dead_state_process    - combine 2nd keystroke withprvious dead key
      key_compose	    - check 2nd keystroke of a diacritic valid
      check_for_dead_key    - check if keystroke is a dead key
      handle_xlated_key	    - process translated key
      vtm_key		    - look for an <esc> b (EMI) to unlock keyboard
      process_key	    - echo and lock management
      data_stream_intr	    - insert page shift codes if needed, buffer
      			      everything received so far and send it to vtmkin.
      expand		    - expand a keystroke into a programmable func key
      			      or a predefined function.
      to_char		    - return ascii char codes representing 8 bit 
			      positive integer.
  ------------*/

/***********************************************************
Include Files
***********************************************************/
#include <sys/types.h>
#include <sys/intr.h>
#include <sys/param.h>
#include <sys/syspest.h>
#include <sys/termio.h>		/* Terminal input/output */

/* Private includes
   =============
*/
#include <lft.h>	
#include <sys/inputdd.h>
#include <sys/lft_ioctl.h>
#include <sys/display.h>
#include <lftcode.h>
#include <sys/syspest.h>
#include <graphics/gs_trace.h>
#include <lft_debug.h>

/* Streams and tty includes
   ========================
*/
#include <sys/stream.h>
#include <lftsi.h>              /* LFT streams information */

GS_MODULE(lftst);
BUGVDEF(db_lftst, 0);                  /* define initial debug level */


/*------------ Procedures static to this file ----------------------*/
static uchar get_xlate_table_index();
static void dead_state_process();
static void key_compose();
static void check_for_dead_key();
static void handle_xlated_key();
static void vtm_key();
static void process_key();
static void data_stream_intr();
static void expand();
void to_char();

extern uchar  kdbase[];

/*------------
  combines code page and point into short
  ------------*/
#define KEY_WORD_CODE \
((ushort)(up->key_struc.keystroke.code_page << 8)| \
(ushort) up->key_struc.keystroke.code)

/***********************************************************
void lftst(struct ir_kbd *key_word);
LFT Scan Code Translator to process keyboard events.

Process keyboard events. Called from lftKiOffl(), 
keyboard interface offlevel interrupt service routine.

INPUT  : 	Key position, Scan code, state of shift keys

OUTPUT :	Keystrokes formatted and passed upstream or 
		placed in the read side STREAMS queue.

RETURNS: 	void

ERROR CODES LOGGED:	VT_DECODE_TRASH	key out of range
***********************************************************/

void lftst(struct ir_kbd *key_word)
{
  uchar kmap_index;
  lft_keystroke_t *kstr;

/* TRACE "Entering lftst" */
GS_ENTER_TRC1(HKWD_GS_LFT,lftst,1,lftst,key_word);

  /*------------------
    translate the keys
    ------------------*/
  /* if key position is within   */
  /* range of translate table    */
     if (key_word->kbd_keypos <= LFT_MAX_POSITIONS) {
	     /* Translate key according to  */
	     /* keyboard map.               */
	     kmap_index = get_xlate_table_index(key_word,&(LFT_UP));
	     kstr=&lft_ptr->swkbd->keystrokes[kmap_index][key_word->kbd_keypos];
	     LFT_UP.key_struc.keystroke.flag = kstr->flag;
	     LFT_UP.key_struc.keystroke.stat = kstr->stat;
	     LFT_UP.key_struc.keystroke.str_page = kstr->str_page;
	     LFT_UP.key_struc.keystroke.code_page = kstr->code_page;
	     LFT_UP.key_struc.keystroke.code = kstr->code;
	     
	     /*------------
	       Look for Kanji (doesn't support dead key function).
	       Then see if we're waiting for second char of diacritic.
	       Then see if we received the first char of a diacritic.
	       ------------*/
	     if (lft_ptr->dds_ptr->kbd.kbd_type == KS106)    /* Kanji */
		     handle_xlated_key(&(LFT_UP),key_word);
	     else if (LFT_UP.ds_state_flag & KBD_DEAD_STATE)
		     dead_state_process(key_word,&(LFT_UP));     /* 2nd char of diacritic? */
	     else
		     check_for_dead_key(&(LFT_UP),key_word);     /* 1st char of diacritic? */
     }else{                                      /* key out of range, log error */
	     /* LFTERR "Untranslatable key" */
	lfterr(NULL,"LFTDD", "lftst", NULL, 0, LFT_STR_UXPSCOD, UNIQUE_1);
     }
  GS_EXIT_TRC0(HKWD_GS_LFT,lftst,1,lftst);
}

/***********************************************************
uchar get_xlate_table_index(struct ir_kbd *key_word,
			struct up_stream *up);
	based on keyword->status, returns index into software 
	keyboard table.

	get the index for the first dimension of the software 
	keyboard map based on the state of the shift keys.

INPUT : state of shift keys

OUTPUT: index into software keyboard map

RETURNS: uchar index into sw kbd map

ERROR CODES LOGGED:
***********************************************************/
static uchar get_xlate_table_index(struct ir_kbd *key_word,
				struct up_stream *up)
{
#define KEY_POS_NUMSTART 90
#define KEY_POS_NUMEND   109
#define BIT_MASK(x)  \
    (((x) & 0x001f) == 0 ? 0x80000000L : 0x40000000L >> (((x) & 0x001f) - 1))

  uchar kana;
  uchar latin;

  kana = FALSE;
  latin = FALSE;
  /*----------
    katakana keystroke
    ----------*/
  if ((lft_ptr->strlft->lft_mode_flag & LFJKANA) && (key_word->kbd_status[0] & KBDUKATAKANA))
     kana = TRUE;
  if (up->state_flag & LATIN_LEVEL)
     latin = TRUE;
  /*----------
    check for control character, left-alt, right-alt, num-lock
    ----------*/
  if (key_word->kbd_status[0] & KBDUXCTRL)
     return(KMAP_INDEX_CNTL);
  else if (key_word->kbd_status[1] & KBDUXLALT)
     return(KMAP_INDEX_LEFT_ALT);
  else if (key_word->kbd_status[1] & KBDUXRALT)
     return(KMAP_INDEX_RIGHT_ALT);
  else if ((key_word->kbd_status[0] & KBDUXNUM) &&
           key_word->kbd_keypos >= KEY_POS_NUMSTART &&
           key_word->kbd_keypos <= KEY_POS_NUMEND) {
     if (key_word->kbd_status[0] & KBDUXSHIFT)
        if (kana || latin)
           return(KMAP_INDEX_KATA);
        else
           return(KMAP_INDEX_NORMAL);
     else                                       /* not shifted */
        if (kana || latin)
           return(KMAP_INDEX_KSHIFT);
        else
           return(KMAP_INDEX_SHIFT);
  }
  /*----------
    check for capslock.  capslock is a bit mask with 1 on if the
    char is affected by the capslock key.
    ----------*/
  else if ((key_word->kbd_status[0] & KBDUXCAPS) &&
           (lft_ptr->swkbd->capslock[key_word->kbd_keypos >> 5] &
           BIT_MASK(key_word->kbd_keypos))) {
     if (key_word->kbd_status[0] & KBDUXSHIFT) {
        if (kana)
           return(KMAP_INDEX_KSHIFT);
        else if (!latin)
           return(KMAP_INDEX_NORMAL);
        else
           return(KMAP_INDEX_KATA);
     }
     else {                                      /* not shifted */
        if (kana)
           return(KMAP_INDEX_KATA);
        else if (!latin)
           return(KMAP_INDEX_SHIFT);
        else
           return(KMAP_INDEX_KSHIFT);
     }
  }
  /*----------
    a shift with capslock on returns a small letter, except in kana
    ----------*/
  else {
     if (key_word->kbd_status[0] & KBDUXSHIFT)
        if (kana || latin)
           return(KMAP_INDEX_KSHIFT);
        else
           return(KMAP_INDEX_SHIFT);
     else                                      /* not shifted */
        if (kana || latin)
           return(KMAP_INDEX_KATA);
        else
           return(KMAP_INDEX_NORMAL);
  }
}

/***********************************************************
static void dead_state_process(struct ir_kbd *key_word,
				struct up_stream *up)
combine second keystroke with previous dead key.

This procedure is called ONLY if kbd_dead_state is already TRUE.
 An attempt will be made to combine this second keystroke with 
the previous dead key.  If successful, the combined key will be 
sent along for further processing.  The resetting of KBD_DEAD_STATE
 will allow the key to be displayed (if desired) and sent to the 
application.  If no combination can be made, both the previous 
dead key and this key will be sent along for separate processing.  
The only exception to this rule is if this second key is itself 
dead.  In this case, the previously saved dead key is sent for 
processing and dead key state is re-entered on account of the 
second dead key, which is held for processing until the next key 
enters.

Note: This procedure does not assume that just because a character 
is flagged as dead, it is a valid dead key.  The keyboard map is 
a replaceable module and therefore, no such assumptions about the 
data would be prudent.  If a character pretending to be dead is just 
playing possum, it is treated as a regular escaping character.

NOTE: Shift, control, alt, alt graf keys must be disregarded by 
this routine so that dead state can be maintained until the REAL 
combining key comes along.

INPUT : previous deadkey and current keystroke

OUTPUT: calls key_compose

RETURNS: void

ERROR CODES LOGGED:
***********************************************************/

static void dead_state_process(struct ir_kbd *key_word,
					struct up_stream *up)
{
  lft_keystroke_t tempkey;       	/*temp. holder of this keystroke*/
  uchar i;                              /*a counter                     */
  uchar found;                          /*loop control variable         */
  lft_kbd_t *kbd;			/*keyboard info			*/

  kbd=&lft_ptr->dds_ptr->kbd;
  /*------------
    If this is a make of the key and the key is not an escape or ignore key
    ------------*/
  if ((key_word->kbd_status[0] & KBDUXMAKE) &&
  !(up->key_struc.keystroke.flag == FLAG_ESC_FUNCTION &&
    KEY_WORD_CODE == KF_IGNORE)) {

                                        /* if this key is not dead */
                                        /* or there is diac table  */
     if (up->key_struc.keystroke.stat != CODE_STAT_DEAD ||
         kbd->diac == NULL)
                                /* try to compose w/previous dead keystroke */
        key_compose(up,key_word);
     else {                             /* try to validate or compose */
                            /* dead wait has been satisfied, in any case */
        up->ds_state_flag &= ~KBD_DEAD_STATE;

        for (found = FALSE,i = 0; i < kbd->diac->num_diacs; i++) {
                            /* entry in table matches this key's code pg/pt */
           if (kbd->diac->diacs[i].deadkey == up->key_struc.keystroke.code) {
                                 /* save start-stop range for this diacritic */
              found = TRUE;
              break;
           }
        }
        if (found) {            /* if this key really was dead */
                                /* we have a double-dead situation */
                                /* process saved (previous) dead key as a     */
                                /* regular key; this key becomes current dead */
                                /* key; dead state re-entered                 */
           tempkey = up->key_struc.keystroke;
           up->key_struc.keystroke = up->saved_dead_key;
           up->saved_dead_key = tempkey;
           handle_xlated_key(up,key_word);
           up->ds_state_flag |= KBD_DEAD_STATE;
                                /*at this point, latest key could be loaded   */
                                /* into FIRST_KEY and processed, but I do not */
                                /* believe this is necessary                  */
        }
        else
           key_compose(up,key_word);
     } /*end this is a dead key*/
  }/*end make of key*/
  /*------------
    Not make of the key or the key is an escape or ignore key
    ------------*/
  else
     handle_xlated_key(up,key_word);
}
/***********************************************************
static void key_compose(struct up_stream *up, struct ir_kbd *key_word);
	check that second keystroke of a diacritic is in a 
	valid range.
This procedure checks a compostion table in the range of the 
diacritic (indicated by "start, stop"). If a match is found 
for the second key in the dead key sequence, then the composite 
code page/point is written into FIRST_KEY_ASSIGN (a side-effect 
of the proc).  If the composition was not successful, the saved 
dead key is processed, and then the second key is processed.  
In either case, dead key state is RESET.

INPUT : previous deadkey and current keystroke

OUTPUT: calls handle_xlated_key

RETURNS: void

ERROR CODES LOGGED:
***********************************************************/
static void key_compose(struct up_stream *up, struct ir_kbd *key_word)
{
#define KEY_FOUND     0
#define KEY_NOT_FOUND 1
  uchar status;                 /* returned to indicate success or failure  */
  uchar i;                      /* a counter                                */
  lft_keystroke_t tempkey;     /* temp. holder of this keystroke*/
  lft_kbd_t *kbd;		/* keyboard info			*/

  kbd=&lft_ptr->dds_ptr->kbd;

  status = KEY_NOT_FOUND;
                     /* dead key wait state has been satisfied; reset state */
  up->ds_state_flag &= ~KBD_DEAD_STATE;

  /*------------
    Search the composition table to see if the keystroke received
    is a valid second character.
    ------------*/
  if (kbd->diac != NULL) {
     for (i = 0; i < kbd->diac->num_diacs; i++) {
        if (kbd->diac->diacs[i].deadkey == up->saved_dead_key.code &&
           kbd->diac->diacs[i].thiskey == up->key_struc.keystroke.code) {
           status = KEY_FOUND;
           up->key_struc.keystroke.code      = kbd->diac->diacs[i].compose;
           up->key_struc.keystroke.code_page = CHARSET_P0;
           break;
        }
     }
  }
  if (status == KEY_NOT_FOUND) {
     tempkey = up->key_struc.keystroke;
     up->key_struc.keystroke = up->saved_dead_key;
     handle_xlated_key(up,key_word);
     up->key_struc.keystroke = tempkey;
  }
  handle_xlated_key(up,key_word);      /* send the char*/
}
/***********************************************************
void check_for_dead_key(struct up_stream *up,
			struct ir_kbd *key_word);
	see if keystroke is a dead key (diacritic).

This procedure is called ONLY if kbd_dead_state is FALSE.  
Under this condition, ALL keys are sent to this procedure 
to be checked for "vitality".  All keys entering this procedure 
will continue to be processed in the usual manner.  
If a key is found to be dead here, the setting of the 
KBD_DEAD_STATE will prevent it's being sent 
to lftrint() at this time.  No processing will occur here 
to a key from a locked keyboard, but it will be sent on for 
processing by the accumulation checker.
	-------------------------------------------------------
	INPUT                PROCESS               OUTPUT STATE
	-------------------------------------------------------
	valid diacritic | 1. set dead key state.  | dead state
	                | 2. save this key as     |
	                |    dead key.            |
	                | 3. (dead key state will |
	                |    prevent further pro- |
	                |    cessing of this key. |
	-------------------------------------------------------
	invalid         | 1. process this key     | ^dead state
	diacritic       |    as usual.            |
***********************************************************/
static void check_for_dead_key(struct up_stream *up,
			struct ir_kbd *key_word)
{
  int i;
  lft_kbd_t *kbd;		/* keyboard info	*/

  kbd=&lft_ptr->dds_ptr->kbd;

  /*------------
    If this key claims to be a dead key, search the diacritic table.
    The diacritic table contains a list of valid diacritics.
    If the keystroke matches one of the keys in the diacritic table,
    save the key and mark out state as dead.
    -------------*/
  if (up->key_struc.keystroke.stat == CODE_STAT_DEAD &&
      kbd->diac != NULL) {
     for (i = 0; i < kbd->diac->num_diacs; i++) {
                        /* entry in table matches this key's code pg/pt */
        if (kbd->diac->diacs[i].deadkey == up->key_struc.keystroke.code) {
                             /* save start-stop range for this diacritic */
           up->ds_state_flag |= KBD_DEAD_STATE;        /* set dead state */
           up->saved_dead_key  = up->key_struc.keystroke;
           break;
        }
     }
  }
                                        /* process this key, dead or not,     */
  handle_xlated_key(up,key_word);      /* it might have an effect on the     */
                                        /* accumulation state.                */
}

/****************** handle_xlated_key **********************

	void handle_xlated_key(struct up_stream *up,
			struct ir_kbd *key_word)
		process translated key.

******************* handle_xlated_key *********************/
static void handle_xlated_key(struct up_stream *up,
			struct ir_kbd *key_word)
{
  ulong char_str_start;
  uchar char_str_length;
  int i;
                                        /* if not in make state */
  if (!(key_word->kbd_status[0] & KBDUXMAKE))        /* ignore keystroke     */
     return;

  if (up->key_struc.keystroke.flag == FLAG_CHAR_STRING) {
                                        /* if key maps to a string, process  */
                                        /* each code in proper code page     */

     char_str_start = KEY_WORD_CODE;    /* Get the byte position of this key */
                                        /* within the key string mappings    */
                                        /*  Page<<8 + Point == index into    */
                                        /*  key mapping string where this    */
                                        /* string starts. (this allows for   */
                                        /*64k one byte strings, or 1 64k byte*/
                                        /* string, etc.)                     */

                                        /* The first byte of the char string */
                                        /* is the length                     */
     char_str_length = up->ichrstr[char_str_start];
                                        /* Set code page for codes in        */
                                        /* character string                  */
     up->key_struc.keystroke.code_page = CHARSET_P0;
     up->key_struc.keystroke.stat = CODE_STAT_NONE;
                                        /* Process each code of character    */
                                        /* string                            */
     for (i = 1; i <= char_str_length; i++) {
        up->key_struc.keystroke.code = up->ichrstr[char_str_start + i];

                                        /* set up the keystroke structure for */
                                        /* this code                          */
        if (up->key_struc.keystroke.code < 32)
           up->key_struc.keystroke.flag = FLAG_SINGLE_CONTROL;
        else
           up->key_struc.keystroke.flag = FLAG_GRAPHIC;

        vtm_key(up);
     }
  }
  else                                  /* Normal keystroke  */
     vtm_key(up);
}
/***********************************************************
void vtm_key(struct up_stream *up)

if key should not be ignored (i.e. not CNTL, SHIFT, ALT, etc.)
handle translated key for eventual return to application  
***********************************************************/
static void vtm_key(struct up_stream *up)
{
  if (!(up->key_struc.keystroke.flag == FLAG_ESC_FUNCTION &&
             KEY_WORD_CODE == KF_IGNORE)) {
     process_key(up);
  }
}

/***********************************************************

void process_key(struct up_stream *up)
	determine keyboard lock state, check character echo
	and enque to screen manager if necessary, call 
	data_stream_intr.

This subroutine determines the echoablility of the keystroke,
calls the appropriate update routine, and issues data stream 
interrupts.

***********************************************************/
static void process_key(struct up_stream *up)
{
  int rc;
  uchar saved_key;
  uchar code;
                                /* dont go any further with dead keys */
  if (up->ds_state_flag & KBD_DEAD_STATE)
     return;
                        /* set flag indicating that long interrupt should NOT */
                        /* be forced this will be set in VTMKIN if SRM mode */

  code = up->key_struc.keystroke.code;
                                           /*********************/
                                           /* Key is a graphic. */
                                           /*********************/
  if (up->key_struc.keystroke.flag == FLAG_GRAPHIC) {
     data_stream_intr(up);
  }
                                        /*********************************/
                                        /* Key is a single byte control. */
                                        /*********************************/
  else if (up->key_struc.keystroke.flag == FLAG_SINGLE_CONTROL) {
     data_stream_intr(up);
  }
                      /********************************************************/
                      /* Key is a predefined function (and wasn't decoded     */
                      /* from the keyboard as an individually keyed sequence).*/
                      /********************************************************/
  else if (up->key_struc.keystroke.stat != CODE_STAT_DECODED  &&
           (up->key_struc.keystroke.flag == FLAG_ESC_FUNCTION ||
            up->key_struc.keystroke.flag == FLAG_CNTL_FUNCTION)) {

     expand(up);              /* Call routine (EXPAND) to expand the function */
                              /* into its control sequence representation.    */
                              /* Note that the sequence needs to be expanded  */
                              /* even if not echoed because of the data stream*/
                              /* interrupts.                                  */

     code = up->key_struc.keystroke.code;
                               /*if not a key to ignore (unsupported function)*/
     if (code != IC_IGNORE) {
        data_stream_intr(up);
     }
  }
                                        /******************************/
                                        /* Key is a decoded sequence. */
                                        /******************************/
  else if (up->key_struc.keystroke.stat == CODE_STAT_DECODED &&
           (up->key_struc.keystroke.flag == FLAG_ESC_FUNCTION ||
            up->key_struc.keystroke.flag == FLAG_CNTL_FUNCTION)) {
     data_stream_intr(up);
  }
}
/***********************************************************
void data_stream_intr(struct up_stream *up)
	insert page shift codes if needed, buffer everything
	received so far and returned to lftOffl.

PROCEDURE : Send any necessary data stream interrupts
***********************************************************/
static void data_stream_intr(struct up_stream *up)
{
  long  i;
  long rc;

                                        /* if this is a dead key */
  if (up->ds_state_flag & KBD_DEAD_STATE)
     return;
                               /* Single character codes (graphics and        */
                               /* single byte controls) need to have the      */
                               /* code placed in the key buffer so that the   */
                               /*data stream interrupt routine will process it*/
  if (up->key_struc.keystroke.flag == FLAG_GRAPHIC ||
      up->key_struc.keystroke.flag == FLAG_SINGLE_CONTROL) {

     up->key_struc.key_buffer[0]  = up->key_struc.keystroke.code;
     up->key_struc.key_seq_length = 1;
  }
                                /*********************************************/
                                /* Control and escape sequences need to have */
                                /* the sequence prefixed with the escape     */
                                /* single byte control (which was never      */
                                /* removed when it was originally received). */
                                /*********************************************/
  if ((up->key_struc.keystroke.flag == FLAG_ESC_FUNCTION ||
       up->key_struc.keystroke.flag == FLAG_CNTL_FUNCTION) &&
      up->key_struc.keystroke.code != IC_IGNORE) {
     up->key_struc.ascii_buffer[up->key_struc.key_buff_cnt++] = IC_ESC;
  }
                                /*********************************************/
                                /* Now place anything that's waiting to      */
                                /* be returned to the host into the buffer   */
                                /* that the lftOffl routine examines.         */
                                /*********************************************/
  for (i = 0; i < up->key_struc.key_seq_length; i++) {
     up->key_struc.ascii_buffer[up->key_struc.key_buff_cnt++] = up->key_struc.key_buffer[i];
  }
}
/***********************************************************

void expand(struct up_stream *up)
	expand a raw keystroke into a programmable function key
 	or a predefined function.

     EXPAND FLAG & CODE TO ASCII CONTROL CODE SEQUENCE CHAR STRING
This subroutine takes the Code_Flags and Code values and if they 
represent a keyed sequence, the sequence of ASCII codes for that 
control is put in Code_SEQ, and the Code value is changed to the 
base sequence ID value.
***********************************************************/
static void expand(struct up_stream *up)
{
#define START_OF_STATE  0xa0
                                      /**********************/
                                      /* if PF Key sequence */
                                      /**********************/
  if (up->key_struc.keystroke.stat == CODE_STAT_PF_KEY) {
     up->key_struc.key_buffer[0]  = FE_CSI;
     to_char((ushort) (up->key_struc.keystroke.code - KF_PF1 + 1),
             &up->key_struc.key_buffer[1]);
     up->key_struc.key_buffer[4]  = CSEQ_F_PFK;
     up->key_struc.key_seq_length = 5;
     up->key_struc.keystroke.code   = IC_PFK;
  }                                   /*************************/
                                      /* if state key sequence */
                                      /*************************/
  else if (up->key_struc.keystroke.stat == CODE_STAT_STATE) {
     if (lft_ptr->strlft->lft_mode_flag & LFJKANA) {

        up->state_flag &= ~(ALPHANUM_STATE|HIRAGANA_STATE|KATAKANA_STATE);
        switch (KEY_WORD_CODE) {
           case KF_ALN: up->state_flag |= ALPHANUM_STATE; break;
           case KF_HIR: up->state_flag |= HIRAGANA_STATE; break;
           case KF_KAT: up->state_flag |= KATAKANA_STATE;
        }
                                        /* send the VTK */
        up->key_struc.key_buffer[0] = FE_CSI;
        to_char((ushort) (up->key_struc.keystroke.code - START_OF_STATE),
                &up->key_struc.key_buffer[1]);
        up->key_struc.key_buffer[4] = CSEQ_F_VTK;
        up->key_struc.key_seq_length = 5;
        up->key_struc.keystroke.code = IC_VTK;
     }
        /* if this is the greek or czrillc keyboard, then switch layers   */
 else if ((strcmp(lft_ptr->swkbd->kbdname,GREEK_MAP) == 0) ||
              (strcmp(lft_ptr->swkbd->kbdname,"bg_BG") == 0) ||
              (strncmp(lft_ptr->swkbd->kbdname,"bg_BGalt",8) == 0) ||
              (strcmp(lft_ptr->swkbd->kbdname,"mk_MK") == 0) ||
              (strcmp(lft_ptr->swkbd->kbdname,"ru_RU") == 0) ||
              (strncmp(lft_ptr->swkbd->kbdname,"ru_RUalt",8) == 0) ||
              (strcmp(lft_ptr->swkbd->kbdname,"sr_SP") == 0)) {
        if (up->key_struc.keystroke.code == KF_PF2) {
           up->state_flag &= ~LATIN_LEVEL;
           lft_ptr->swkbd->capslock[0] = 0x07fe1;
        }
        else if (up->key_struc.keystroke.code == KF_PF1) {
           up->state_flag |= LATIN_LEVEL;
           lft_ptr->swkbd->capslock[0] = 0x07fe1;
        }
        up->key_struc.keystroke.code = IC_IGNORE;
     }
     else {
        up->key_struc.key_seq_length = 0;
        up->key_struc.keystroke.code   = IC_IGNORE;
     }
  }                                   /**********************/
                                      /* if Escape sequence */
                                      /**********************/
  else if (up->key_struc.keystroke.flag == FLAG_ESC_FUNCTION) {
    switch (KEY_WORD_CODE) {
                                        /* Reverse index */
       case KF_RI:   up->key_struc.key_buffer[0] = FE_RI; break;

                                        /* Init - clear PS and reset tabs */
       case KF_INIT: up->key_struc.key_buffer[0] = FS_RIS; break;

                                        /* Index */
       case KF_IND:  up->key_struc.key_buffer[0] = FE_IND; break;

       default:      up->key_struc.keystroke.code = IC_IGNORE;
    }

    if (up->key_struc.keystroke.code == IC_IGNORE)
                                        /* if unsupported key definition then */
                                        /* no data stream interrupt returned  */
       up->key_struc.key_seq_length = 0;
    else {
       up->key_struc.key_seq_length = 1;
       up->key_struc.keystroke.code = kdbase[(KEY_WORD_CODE) - 0x101];
    }
  }                                   /************************/
                                      /* if control sequences */
                                      /************************/
  else if (up->key_struc.keystroke.flag == FLAG_CNTL_FUNCTION) {
     up->key_struc.key_buffer[0] = FE_CSI;
     up->key_struc.key_seq_length = 2;
     switch (KEY_WORD_CODE) {
        case KF_CUU:                  /* Cursor up 1 */
             up->key_struc.key_buffer[1] = CSEQ_F_CUU; break;
        case KF_CUD:                  /* Cursor down 1 */
             up->key_struc.key_buffer[1] = CSEQ_F_CUD; break;
        case KF_CUF:                  /* Cursor forward 1 */
             up->key_struc.key_buffer[1] = CSEQ_F_CUF; break;
        case KF_CUB:                  /* Cursor back 1 */
             up->key_struc.key_buffer[1] = CSEQ_F_CUB; break;
        case KF_CBT:                  /* Move cursor back to prev hor tab */
             up->key_struc.key_buffer[1] = CSEQ_F_CBT; break;
        case KF_CHT:                  /* Move cursor forward to next hor tab */
             up->key_struc.key_buffer[1] = CSEQ_F_CHT; break;
        case KF_CVT:                  /* Move cursor down 1 ver tab stop */
             up->key_struc.key_buffer[1] = CSEQ_F_CVT; break;
        case KF_HOM:                  /* Cursor to home position */
             up->key_struc.key_buffer[1] = CSEQ_F_CUP; break;
        case KF_LL:                   /* cursor to 1st char, last line in PS */
             to_char(SCR_HEIGHT,&up->key_struc.key_buffer[1]);
             up->key_struc.key_buffer[4] = IC_SEMI;
             up->key_struc.key_buffer[5] = IC_1;
             up->key_struc.key_buffer[6] = CSEQ_F_CUP;
             up->key_struc.key_seq_length = 7;
             break;
        case KF_END:                  /* cursor to last char, last line in PS */
             to_char(SCR_HEIGHT,&up->key_struc.key_buffer[1]);
             up->key_struc.key_buffer[4] = IC_SEMI;
             to_char(SCR_WIDTH,&up->key_struc.key_buffer[5]);
             up->key_struc.key_buffer[8] = CSEQ_F_CUP;
             up->key_struc.key_seq_length = 9;
             break;
        case KF_CPL:                  /* cursor to 1st char of preceding line */
             up->key_struc.key_buffer[1] = CSEQ_F_CPL; break;
        case KF_CNL:                  /* cursor to 1st char of next line */
             up->key_struc.key_buffer[1] = CSEQ_F_CNL; break;
        case KF_DCH:                  /* Delete cursored char */
             up->key_struc.key_buffer[1] = CSEQ_F_DCH; break;
        case KF_IL:                   /* Insert line */
             up->key_struc.key_buffer[1] = CSEQ_F_IL;  break;
        case KF_DL:                   /* Delete line */
             up->key_struc.key_buffer[1] = CSEQ_F_DL;  break;
        case KF_EEOL:                 /* Erase to end of line */
             up->key_struc.key_buffer[1]  = IC_0;
             up->key_struc.key_buffer[2]  = CSEQ_F_EL;
             up->key_struc.key_seq_length = 3;
             break;
        case KF_EEOF:         /* Erase to end of field (hor tab stop for KSR) */
             up->key_struc.key_buffer[1]  = IC_0;
             up->key_struc.key_buffer[2]  = CSEQ_F_EF;
             up->key_struc.key_seq_length = 3;
             break;
        case KF_CLR:                  /* Erase entire PS */
             up->key_struc.key_buffer[1]  = IC_2;
             up->key_struc.key_buffer[2]  = CSEQ_F_ED;
             up->key_struc.key_seq_length = 3;
             break;
     }
                             /* translate to the representative internal code */
    up->key_struc.keystroke.code = kdbase[(KEY_WORD_CODE) - 0x101];
                                /* if dead key then nothing is returned in the*/
                                /* data stream interrupt                      */
    if (up->key_struc.keystroke.code == IC_IGNORE)
       up->key_struc.key_seq_length = 0;
  }
}
/***********************************************************
void to_char(ushort pos, char *result)
	return ascii char codes representing 8 bit positive
 	integer.

RETURN ASCII CHAR CODES REPRESENTING 8 BIT POSITIVE INTEGER
***********************************************************/
static void to_char(ushort pos, char *result)
{
  long i;
  long j;

  j = 0;
  for (i = 100; i >= 1; i /= 10) {
     result[j] = (pos / i) + IC_0;
     pos %= i;
     j++;
  }
}
