static char sccsid[] = "@(#)09	1.1  src/bos/diag/util/udiskenh/udmmisc.c, dsaudiskenh, bos411, 9435A411a 8/18/94 13:56:05";
/*
 * OMPONENT_NAME: DSAUDISKMNT
 *
 * FUNCTIONS: none
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
 * PURPOSE:  The file contains miscellanious routines that are called by
 *           the hex_edit function.
 *
 * HISTORY:
 * nchang     07/13/94   Modified the hexit utility program written by
 *                       Pete Hilton to display hex data and allow users
 *                       to edit it.
 */


#define MISC_C               
#define MSGS_C

#include "udmhexit.h"
 
/*
 * FUNCTION: Initialise global variables.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: none.
 */
void
init_global()  /*NC, remove input parameter, argv */
{
  msg_clear = FALSE;
  hexoff = TRUE;
  h_mode = EDIT_MODE;
  stack[0] = 0;
  mode_stack.depth = 208;
  mode_stack.pointer = 0;
};

 
/*
 * FUNCTION:
 * Generate a printable string in hex or decimal      
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: The position of the result in the output string.
 */
int
make_byte_string(int number,char *string, int hex)
{
  int i,j,k;

  if ( hex == TRUE )
  {
    j = number;
    for (i=9; i>1; i--)
    {
      k = j & 0x0000000f; 
      j = j >> 4;
      string[i] = hexchars[k];      	
    };
    if (number == 0)
    {
      i = 7;
    }
    else
    {
      for (i=2; i<10; i++) if (string[i] != '0') break;
      i = i-2;
    };
    string[i] = '0'; 
    string[i+1] = 'x';
  }
  else
  {
    sprintf(string,"%10.1u",number);
    for (i=0; i<10; i++) if (string[i] != ' ') break;
  };
  return(i);
}

 
/*
 * FUNCTION:
 *      Calculate the cursor position from the nibble position   
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: None.
 */
void
calc_cursor(int nibble)
{
  int i,j,k;

  i = nibble/32+1; 
  j = nibble%32; 
  j = 2+j+(j/8);
  k = ((nibble%32)/2)+1;
  
  ascii_row = i; 
  hex_part_row = i;
  ascii_col = k; 
  hex_part_col = j;
  return;
}

/*
 * FUNCTION:
 * Set the cursor in the correct window and the byte offset cosmetic          
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: None.
 */
void
set_cursor(int nibble,int mode)
{
  int i;
  
  calc_cursor(nibble); 
  draw_cursor(mode);
  i = (block_n*208) + (nibble/2);
  set_byte_off(i);
  return;
}


/*
 * FUNCTION:
 * Stack on integer on the stack; return FALSE if stack full
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: FALSE - stack is full
 *          TRUE  - integer was stacked.
 */
int
en_stack(int *i, L_STACK *stack)
{
  if ( stack->pointer >= stack->depth )
  {
    return(FALSE);
  }
  stack->stack[stack->pointer] = *i; 
  stack->pointer++;
  return(TRUE);
}
    
/*
 * FUNCTION:
 * Unstack on integer from the stack; return FALSE if stack empty
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: FALSE - stack is empty
 *          TRUE  - integer was unstacked.
 */
int
de_stack(int *i, L_STACK *stack)
{
  if ( stack->pointer == 0 ) 
  {
    return(FALSE);
  }
  stack->pointer--; 
  *i = stack->stack[stack->pointer];
  return(TRUE);
}
    

/*
 * FUNCTION:
 * Give input based on the character from the keyboard     
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: The character entered via keyboard.
 */
char
get_input()
{
  char q;
  int j, reading;

  reading = TRUE;
  while ( reading == TRUE )
  {
    if ( key_strokes == STACKED )
    {
      q = *key_queue_pointer++;
      if ( key_queue_pointer == key_queue_end ) 
      {
         key_strokes = NOT_STACKED;
      }    
      j = (int)q;
    }
    else
    {
      j = get_a_key(); 
      q = (char)j;
    };

    if ( j < 0x20 )
    {
      /* This is a control character ... if not special then pass through
       *  ^P = push current mode on stack
       *  ^R = pop mode off the stack
       *  ^E = change to edit mode
       *  ^F = change to command mode
       */
      switch (q)
      {
        case CTRL_P:
          en_stack(&h_mode,&mode_stack); 
        break;
      
        case CTRL_R:
          de_stack(&h_mode,&mode_stack);
        break;
      
        case CTRL_E:
          h_mode = EDIT_MODE;
        break;
      
        case CTRL_F:
          h_mode = COMMAND_MODE;
        break;
        
        default:
          reading = FALSE;
        break;

      };      
    }
    else
    {
      if ((j&0xff00) == 0)          /* not a special key             */
      {
        reading = FALSE;            /* end the reading loop          */  
      }
      else
      {
        /* special key so set up the key-stroke queue                */      	
        j = j&0xffff;
        j = j - 0x100 ;
        if ( j < NT_KEYS )
        {
          if ( k_table[j] != NULL )
          {
            key_strokes = STACKED; 
            key_queue_pointer = k_table[j];
            key_queue_end = key_queue_pointer + strlen(key_queue_pointer);
          };
        };
      };                            /* endif special key (j&)        */ 
    };                              /* endif control key (j<)        */
  };                                /* end while reading             */
  return(q);
}


