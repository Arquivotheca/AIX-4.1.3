static char sccsid[] = "@(#)08	1.1  src/bos/diag/util/udiskenh/udmhexit.c, dsaudiskenh, bos411, 9435A411a 8/18/94 13:55:41";
 /*
 * COMPONENT_NAME: DSAUDISKMNT
 *
 * FUNCTIONS: hexit_edit
 *
 * ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include "udmhexit.h"
 

/*
 * FUNCTION: The main routine in the udmedit program.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 
 *                -1   ASL_FAIL
 *                0    ASL_CANCEL
 *                3    ASL_EXIT
 *                7    ASL_COMMIT
 */
int hex_edit(char *title_str, char *subtitle_str, char *sector_disk_str,
             char *instn_str, char *offset_str, char *keys_str,
             char *hexw_str,
             char *asciiw_str,char *buff)
{
  bool running, new_block, error_msg;  /* flags   */
  int i, j, k, l;
  int new_cursor_p, new_block_n, char_n,last_n;
  int c_nibble, nibble_fudge, c_fudge;
  char q;
  int  rc = 0;
  int  size = 0;
  
  if (buff == NULL || title_str == NULL || subtitle_str == NULL ||
      sector_disk_str == NULL || instn_str == NULL ||
      offset_str == NULL || keys_str == NULL || 
      hexw_str == NULL || asciiw_str == NULL)
  {
      return(ASL_FAIL);
  }

  init_global(); 


  /* initialise the display/terminal/keyboard                    */
 
  if (init_init() == FALSE)
  {
    printf("Display could not be initailised properly");
    return(ASL_FAIL);
  };

  default_msg = blanks;
  running = TRUE; new_block_n = 0; new_cursor_p = 0; error_msg = FALSE;
  mode = HEX; 
  nibble_fudge = 1; 
  c_fudge = 1;

  init_keybd();  
  current_msg = default_msg;

  if (init_display(title_str,subtitle_str,sector_disk_str,instn_str,
      offset_str, keys_str, hexw_str, asciiw_str) 
      == FALSE)
  {
     running = FALSE;
  }
  
  char_n = size= ABUFF;
  while (running == TRUE)             /* main while loop         */
  {
    block_n = new_block_n;
    i = (block_n*208)%BUF_SIZE;
     
    if (i == 1)
    {
       char_n = size - 208;
    }
    else
    if (i==MAX_BLOCK)
    {   
       char_n = ABUFF-408;
    } 

    last_n = 208; 
    if ( (char_n-i) < 208 )  
    {
      last_n = char_n - i;
    }

    set_windows(&buff[i]);
    new_block = FALSE;  
    c_nibble = (new_cursor_p%208)*2;
    set_cursor(c_nibble,mode);

    while (new_block == FALSE)            /*  unchanged block loop   */
    {
      repaint();                          /* repaint screens in the  */
      q=get_input();                      /* get a key stroke        */
      switch(h_mode)
      {
        case EDIT_MODE:                   /* we are editing the file */
          switch(mode)
          {
            case HEX:
              if ( strchr(hexchars,q) != NULL )
              {
                /* convert to upper case if lower       */
                if (q > '9') q = (char)(q&0xDF);

            /* now extract the hex digit and position it as a lower/upper */
            /* nibble for the character to be changed. Set up a mask for  */
            /* the unchanged nibble.                                      */

                i = (((int)(strchr(hexchars,q)-hexchars))&0x0f); 
                l = 0xf0;
                if ((c_nibble%2) == 0) 
                {  
                  i = i<<4;
                  l = l>>4;
                };
  
            /* extract the character to be changed from the buffer and    */
            /* isolate the UNCHANGED bits                                 */ 

                j = (((block_n*208)%BUF_SIZE)+(c_nibble/2)); 
                k = (((int)(buff[j]))&l);
  
            /* generate the new character and set it in the buffer and    */
            /* the window.                                                */

                q = (char)(i|k); 
                buff[j]=q;
                insert_a_char(q,c_nibble);
                l = (c_nibble+1)%BUF_SIZE;
                if (l < (2*last_n)) 
                {
                   c_nibble = l;  
                }
                set_cursor(c_nibble,mode);
              };
            break;

            case ASCII:
              i = (((block_n*208)%BUF_SIZE)+(c_nibble/2)); 
              buff[i] = q;
              insert_a_char(q,c_nibble);
              l = (c_nibble+2)%BUF_SIZE; 
              if (l < (2*last_n)) 
              {
                 c_nibble = l;  
              }
              set_cursor(c_nibble,mode); 
            break;
          };                              /* end of mode switch      */
        break;

        case COMMAND_MODE:                /* command-like things     */

        /* Valid commands are:-
         *     b  back one screen
         *     f  forward one screen
         *     h  help
         *     l  cursor left
         *     q  quit
         *     r  cursor right
         *     t  tab to other window
         *     u  cursor up
         */

          j = 1;                          /* a usefull one to have   */
          switch(q)
          {
            case 't':                     /* flip editting screen    */
              flip_screens();
              if (mode == ASCII)
              {
                nibble_fudge = 1;
                mode = HEX;
              }
              else
              {
                nibble_fudge = 2;
                mode = ASCII;
              }  
              c_fudge = nibble_fudge;
              c_nibble = (c_nibble/2)*2;
              set_cursor(c_nibble,mode);
            break;

            case 'u':                     /* cursor north (up)       */
              c_fudge = -32;
              goto fudgit;

            case 'd':                     /* cursor south (down)     */
              c_fudge = 32;
              goto fudgit;

            case 'l':                     /* cursor west (left)      */
              c_fudge = -nibble_fudge;
              goto fudgit;

            case 'r':                     /* cursor east (right)     */
              c_fudge = nibble_fudge;

            fudgit:
              l = (BUF_SIZE+c_nibble+(j*c_fudge))%BUF_SIZE; 
              if (l < (2*last_n)) 
              {
                c_nibble = l; 
              }
              else 
              {
                beep();
              }
              set_cursor(c_nibble,mode); 
              c_fudge = nibble_fudge;
            break;

            case 'b':                     /* page backwards          */
              j = -1;

            case 'f':                     /* page forward            */
              new_block = TRUE; new_block_n = block_n+j;
              if (new_block_n < 0) new_block_n = 0;
              if (new_block_n > MAX_BLOCK)
              {
                 new_block_n = MAX_BLOCK;
                 beep();
              }
              new_cursor_p = new_block_n*208;
            break;

            case 'q': /* F3-cancel */          /* quit .. el get out */
              new_block = TRUE; 
              running = FALSE;
              rc =  ASL_CANCEL; 
            break;

            case 'c': /* F7-commit */
              new_block = TRUE; 
              running = FALSE;
              rc =  ASL_COMMIT; 
            break;

            case 'e': /* F10-exit */  
              new_block = TRUE; 
              running = FALSE;
              rc =  ASL_EXIT; 
            break;

            case 'h':                          /* display help info  */
              display_help();
            break;

            case ':':                          /* ignore seperator   */
            break;         /*NC, if omit, beeps in every cursor move */

            default:
              beep();
            break;               
          };                                      /* end of q switch */ 
        break;
      };                                     /* end of h_mode switch */

    };                                /* end of unchanged block loop */
  
  };                                  /* end of main loop            */
  
  /* end it all                                                      */
  if (error_msg == FALSE) 
  {
    reset_display();
  }

  reset_reset();
  return(rc);  
}
