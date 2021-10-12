 /* @(#)87	1.1  src/bos/usr/bin/bterm/parse.l, libbidi, bos411, 9428A410j 8/26/93 13:35:16 */
 /*
 **  COMPONENT_NAME: LIBBIDI
 **
 **  FUNCTIONS: yylex
 **
 **  ORIGINS: 27
 **
 **  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 **  combined with the aggregated modules for this product)
 **                   SOURCE MATERIALS
 **
 **  (C) COPYRIGHT International Business Machines Corp. 1993
 **  All Rights Reserved
 **  US Government Users Restricted Rights - Use, duplication or
 **  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 **
 ** 	Parsing rules for the 3151 and vt220 data stream.      	
 **     yylex is used to parse both keyboard input
 **     and screen input. Functions input, output
 **     and unput are different for both cases. Which
 **     function is used is determined by the global 
 **     variable Lex_Mode.
 ** 	The screen data is parsed for escape sequences and  
 ** 	control characters. Whatever is not matched,
 ** 	is processed by function scr_output in screen.c  	
 **     as a normal ascii character.
 **     The keyboard data is parsed for special keys,
 **     any normal key is processed by function kbd_ouput
 **     in kbd.c
 **     Unsupported escapes are recognized, but passed
 **     unprocessed to the tty.
 */

%{
#include "global.h"
#include "trace.h"

#define yywrap()  1

#undef input
#undef unput
#undef output
/*===========================================================*/
/* input routine for yylex() */
int input ()
{
 if (Lex_Mode==1)
   return(scr_input());
 else return(kbd_input());
}
/*--------------------------------------------------------------------------*/
/* to put a character back into lex input stream */
int unput (ch)
int     ch;
{
 if (Lex_Mode==1)
   return(scr_unput(ch));
 else return(kbd_unput(ch));
}
/*--------------------------------------------------------------------------*/
/* This function is used for any character not identified by yylex. */
int output (ch)
int     ch;
{
 if (Lex_Mode==1)
   return(scr_output(ch));
 else return(kbd_output(ch));
}
/*--------------------------------------------------------------------------*/
%}

%Start  VT_HFT IBM3151 

ESCAPE       "\033"
SPACE        "\040"
SEVEN        "\007" 
EIGHT        "\010"
NINE         "\011"   
TEN          "\012"  
ELEVEN       "\013"  
TWELVE       "\014"  
THIRTEEN     "\015"
FOURTEEN     "\016"
FIFTEEN      "\017"
TWENTY_FOUR  "\030"  

FUNCTION_KEYS [a-l]
PA_KEYS   [m,n,o]
/* y value between 0x20 and 0x37 */
Y_VALUE   [" "!\"#$%&'()*+,\-./0-7]
/* x value between 0x20 and 0x6f */
X_VALUE   [" "!\"#$%&'()*+,\-./0-9:;<=>?@A-Z\[\\\]\^_`a-o]

%%
{ESCAPE}{SPACE}9@           {TRACE(("starting 3151\n"));
                             BEGIN 0 ; BEGIN IBM3151;} /* select mode */
{ESCAPE}{SPACE}9<e          {TRACE(("starting vt220\n"));
                             BEGIN 0 ; BEGIN VT_HFT;} 
{ESCAPE}{SPACE}9<i          {TRACE(("starting hft\n"));
                             BEGIN 0 ; BEGIN VT_HFT; } 

{SEVEN}                     do_bell();                  /* common escapes */
{EIGHT}                     do_backspace();
{NINE}                      do_tab();
{TEN}                       do_line_feed();
{THIRTEEN}                  do_carriage_return();
{TWENTY_FOUR}               do_bidi_command_mode();

<IBM3151>{ESCAPE}>A                  do_enter_alt_charset_mode(); /* ibm3151 */
<IBM3151>{ESCAPE}>B                  do_exit_alt_charset_mode();
<IBM3151>{ESCAPE}>@                  do_exit_alt_charset_mode();
<IBM3151>{ESCAPE}<A                  do_enter_alt2_charset_mode();
<IBM3151>{ESCAPE}<B                  do_exit_alt2_charset_mode();
<IBM3151>{ESCAPE}<@                  do_exit_alt2_charset_mode();
<IBM3151>{ESCAPE}4.                  do_3151_set_attributes();
<IBM3151>{ESCAPE}4@                  do_exit_attribute_mode();
<IBM3151>{ESCAPE}{SPACE}S            do_power_on_reset();
<IBM3151>{ESCAPE}A                   do_cursor_up(1);
<IBM3151>{ESCAPE}B                   do_cursor_down(1);
<IBM3151>{ESCAPE}C                   do_cursor_right(1);
<IBM3151>{ESCAPE}D                   do_cursor_left(1);
<IBM3151>{ESCAPE}H                   do_cursor_home();
<IBM3151>{ESCAPE}I                   do_clr_eol();
<IBM3151>{ESCAPE}J                   do_clr_eos();
<IBM3151>{ESCAPE}K                   do_erase_input();
<IBM3151>{ESCAPE}L                   do_clear_screen();
<IBM3151>{ESCAPE}!L                  do_clear_screen();
<IBM3151>{ESCAPE}N                   do_insert_line(1);
<IBM3151>{ESCAPE}O                   do_delete_line(1);
<IBM3151>{ESCAPE}P{SPACE}{TEN}       do_insert_character(1);
<IBM3151>{ESCAPE}Q                   do_delete_character(1);
<IBM3151>{ESCAPE}X{Y_VALUE}{X_VALUE}  do_buffer_address(
                                               buffer[buffer_index-2]-32,
                                               buffer[buffer_index-1]-32);
<IBM3151>{ESCAPE}Y{Y_VALUE}{X_VALUE}  do_cursor_address(
                                               buffer[buffer_index-2]-32,
                                               buffer[buffer_index-1]-32);
<IBM3151>{ESCAPE}Z                   do_insert_cursor();
<IBM3151>{ESCAPE}{SPACE}Z            do_reset_buffer_address();
<IBM3151>{ESCAPE}{SPACE}9            do_3151_set_control1();
<IBM3151>{ESCAPE}!9                  do_3151_set_control2();
<IBM3151>{ESCAPE}\"9                 do_3151_set_control3();
<IBM3151>{ESCAPE}#9                  do_3151_set_control4();
<IBM3151>{ESCAPE}$9                  do_3151_set_control5();
<IBM3151>{ESCAPE}%9                  do_3151_set_control6();
<IBM3151>{ESCAPE}&9                  do_3151_set_control7();
<IBM3151>{ESCAPE}{SPACE}t            do_3151_reset_function_keys();
<IBM3151>{ESCAPE}t                   do_3151_reset_function_key();
<IBM3151>{ESCAPE}!=                  do_3151_set_function_key();


<IBM3151>{ESCAPE}{FUNCTION_KEYS}{THIRTEEN}     do_function_keys(
                                                  buffer[buffer_index-2]-0x61);
<IBM3151>{ESCAPE}!{FUNCTION_KEYS}{THIRTEEN}    do_function_keys(
                                                buffer[buffer_index-2]-0x61+12);
<IBM3151>{ESCAPE}\"{FUNCTION_KEYS}{THIRTEEN}   do_function_keys(
                                                buffer[buffer_index-2]-0x61+24);
<IBM3151>{ESCAPE}!{PA_KEYS}{THIRTEEN}          do_3151_pa_keys();
<IBM3151>{ESCAPE}\"A                           do_jump();


<VT_HFT>{ESCAPE}O[P,Q,R,S]          do_vt220_pa_keys();    /* vt220 escapes */
<VT_HFT>{ESCAPE}\[[0-9]*~           do_function_keys(get_vt220_param());


<VT_HFT>{ESCAPE}\[0[0-3][0-9]q      do_function_keys(get_vt220_param()); /*hft*/


<VT_HFT>{ELEVEN}                    do_line_feed(); /* vt220+hft escapes */
<VT_HFT>{TWELVE}                    do_line_feed();
<VT_HFT>{FOURTEEN}                  {if (Lex_Mode==2)
                                        string_to_pty("\016");
                                        else do_enter_alt_charset_mode();}
<VT_HFT>{FIFTEEN}                   {if (Lex_Mode==2)
                                          string_to_pty("\017");
                                          else do_exit_alt_charset_mode();}
<VT_HFT>{ESCAPE}\[[0-9]*@           do_insert_character(get_vt220_param());
<VT_HFT>{ESCAPE}\(0                 do_enter_alt_charset_mode(); 
<VT_HFT>{ESCAPE}\(B                 do_exit_alt_charset_mode();
<VT_HFT>{ESCAPE}H                   do_set_horizontal_tab();
<VT_HFT>{ESCAPE}I                   do_set_vertical_tab();
<VT_HFT>{ESCAPE}\[[0-9]*A           do_cursor_up(get_vt220_param());
<VT_HFT>{ESCAPE}\[[0-9]*B           do_cursor_down(get_vt220_param());
<VT_HFT>{ESCAPE}\[[0-9]*C           do_cursor_right(get_vt220_param());
<VT_HFT>{ESCAPE}\[[0-9]*D           do_cursor_left(get_vt220_param());
<VT_HFT>{ESCAPE}\[E                 do_next_line();
<VT_HFT>{ESCAPE}\[F                 do_previous_line();
<VT_HFT>{ESCAPE}\[[0-9]*G           do_cursor_address(SCR->_logy,
                                                     get_vt220_param()-1);
<VT_HFT>{ESCAPE}\[H                 do_cursor_home();
<VT_HFT>{ESCAPE}\[[0-9]+;[0-9]+H    do_vt220_cursor_address();
<VT_HFT>{ESCAPE}\[I                 do_tab();
<VT_HFT>{ESCAPE}\[[0,1,2]?J         do_vt220_clr_scr();
<VT_HFT>{ESCAPE}\[[0,1,2]?K         do_vt220_clr_lin();
<VT_HFT>{ESCAPE}\[[0-9]*L           do_insert_line(get_vt220_param());
<VT_HFT>{ESCAPE}\[[0-9]*M           do_delete_line(get_vt220_param());
<VT_HFT>{ESCAPE}\[[0,1,2]?O         do_vt220_clr_lin();
<VT_HFT>{ESCAPE}\[[0-9]*P           do_delete_character(get_vt220_param());
<VT_HFT>{ESCAPE}\[[0-9]+;[0-9]+R    do_vt220_cursor_address();
<VT_HFT>{ESCAPE}\[[0-9]*S           do_scroll_up(get_vt220_param());
<VT_HFT>{ESCAPE}\[[0-9]*T           do_scroll_down(get_vt220_param());
<VT_HFT>{ESCAPE}\[-0-6]?W           do_cursor_tab_control(get_vt220_param());
<VT_HFT>{ESCAPE}\[[0-9]*X           do_clr_lin(SCR->_logx,
                                              SCR->_logx+get_vt220_param());
<VT_HFT>{ESCAPE}\[Y                 do_cursor_vertical_tab();
<VT_HFT>{ESCAPE}\[Z                 do_cursor_back_tab();
<VT_HFT>{ESCAPE}\[[0-9]+;[0-9]+f    do_vt220_cursor_address();
<VT_HFT>{ESCAPE}\[[0-3]?g           do_cursor_clear_tab(get_vt220_param());
<VT_HFT>{ESCAPE}\[2l                string_to_hft("\033[2l"); /* not handled */
<VT_HFT>{ESCAPE}\[2h                string_to_hft("\033[2h"); /* not handled */
<VT_HFT>{ESCAPE}\[4l                INSERT=FALSE;
<VT_HFT>{ESCAPE}\[4h                INSERT=TRUE;
<VT_HFT>{ESCAPE}\[12l               string_to_hft("\033[12l"); /* not handled */
<VT_HFT>{ESCAPE}\[12h               string_to_hft("\033[12h"); /* not handled */
<VT_HFT>{ESCAPE}\[20h               AUTOLF=TRUE;
<VT_HFT>{ESCAPE}\[20l               AUTOLF=FALSE;
<VT_HFT>{ESCAPE}\[\?1l              string_to_hft("\033[?1l"); /* not handled */
<VT_HFT>{ESCAPE}\[\?1h              string_to_hft("\033[?1h"); /* not handled */
<VT_HFT>{ESCAPE}\[\?2l              string_to_hft("\033[?2l"); /* not handled */
<VT_HFT>{ESCAPE}\[\?2h              string_to_hft("\033[?2h"); /* not handled */
<VT_HFT>{ESCAPE}\[\?3l              string_to_hft("\033[?3l"); /* not handled */
<VT_HFT>{ESCAPE}\[\?3h              string_to_hft("\033[?3h"); /* not handled */
<VT_HFT>{ESCAPE}\[\?4l              string_to_hft("\033[?4l"); /* not handled */
<VT_HFT>{ESCAPE}\[\?4h              string_to_hft("\033[?4h"); /* not handled */
<VT_HFT>{ESCAPE}\[\?5l              string_to_hft("\033[?5l"); /* not handled */
<VT_HFT>{ESCAPE}\[\?5h              string_to_hft("\033[?5h"); /* not handled */
<VT_HFT>{ESCAPE}\[\?6l              string_to_hft("\033[?6l"); /* not handled */
<VT_HFT>{ESCAPE}\[\?6h              string_to_hft("\033[?6h"); /* not handled */
<VT_HFT>{ESCAPE}\[\?7h              {SCROLL=1;WRAP=1;} /* mode settings */
<VT_HFT>{ESCAPE}\[\?7l              {SCROLL=0;WRAP=0;}
<VT_HFT>{ESCAPE}\[\?8l              string_to_hft("\033[?8l"); /* not handled */
<VT_HFT>{ESCAPE}\[\?8h              string_to_hft("\033[?8h"); /* not handled */
<VT_HFT>{ESCAPE}\[\?18l             string_to_hft("\033[?18l"); /* not handled */
<VT_HFT>{ESCAPE}\[\?18h             string_to_hft("\033[?18h");/* not handled */
<VT_HFT>{ESCAPE}\[\?19l             string_to_hft("\033[?19l");/* not handled */
<VT_HFT>{ESCAPE}\[\?19h             string_to_hft("\033[?19h");/* not handled */
<VT_HFT>{ESCAPE}\[\?25l             string_to_hft("\033[?25l");/* not handled */
<VT_HFT>{ESCAPE}\[\?25h             string_to_hft("\033[?25h");/* not handled */
<VT_HFT>{ESCAPE}\[\?34h             do_bidi_set_RTL_national(); 
<VT_HFT>{ESCAPE}\[\?34l             do_bidi_set_LTR_Latin();
<VT_HFT>{ESCAPE}\[\?35h             do_bidi_set_national();
<VT_HFT>{ESCAPE}\[\?35l             do_bidi_set_Latin();
<VT_HFT>{ESCAPE}\[m                 do_exit_attribute_mode();
<VT_HFT>{ESCAPE}\[[0-9]*(;[0-9]+)*m   do_vt220_set_attributes();
<VT_HFT>{ESCAPE}\[!p                do_vt220_soft_reset();

{ESCAPE}\[\?81h             do_bidi_screen_reverse();      /* bidi escapes */
{ESCAPE}\[\?82h             do_bidi_disable_keys();
{ESCAPE}\[\?82l             do_bidi_enable_keys();
{ESCAPE}\[\?83h             do_bidi_save_environment();
{ESCAPE}\[\?83l             do_bidi_restore_environment();
{ESCAPE}\[\?84h             do_bidi_set_RTL_national();
{ESCAPE}\[\?84l             do_bidi_set_LTR_Latin();
{ESCAPE}\[\?85h             do_bidi_set_national();
{ESCAPE}\[\?85l             do_bidi_set_Latin();
{ESCAPE}\[\?86h             do_bidi_seton();
{ESCAPE}\[\?86l             do_bidi_setoff();
{ESCAPE}\[\?87h             do_bidi_toggle_autopush();
{ESCAPE}\[\?88h             do_bidi_autopush_on();
{ESCAPE}\[\?88l             do_bidi_autopush_off();
{ESCAPE}\[\?90h             do_bidi_symmetric_on();
{ESCAPE}\[\?90l             do_bidi_symmetric_off();
{ESCAPE}\[8l                do_bidi_set_mode_implicit();
{ESCAPE}\[8h                do_bidi_reset_mode_visual();
{ESCAPE}\[(0|3);1{SPACE}S   do_bidi_SPD();
{ESCAPE}\[(0|1)\[           do_bidi_SRS();
{ESCAPE}\[[0-9][0-9]?(;[0-9][0-9]?)*{SPACE}\]    do_bidi_SAPV();
%%




