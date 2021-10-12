static char sccsid[] = "@(#)10	1.1  src/bos/diag/util/udiskenh/udmwindows.c, dsaudiskenh, bos411, 9435A411a 8/18/94 13:56:35";
 /*
 * COMPONENT_NAME: DSAUDISKMNT
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
 * PURPOSE:  The file contains all of the routines that are called by
 *           the hex_edit function.
 *
 * HISTORY:
 * nchang     07/13/94   Modified the hexit utility program written by
 *                       Pete Hilton to display hex data and allow users
 *                       to edit it.
 */


#define WINDOWS_C
 
#include <cur01.h>
#include "udmhexit.h"
#include "udmutil.h"
#include "udmedit_msg.h"
extern int Bxa = 0;  /* @@ added */

/*
 * FUNCTION: Reset all curses structures.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: none.
 */
void
reset_reset()
{
    /*             reset all curses structures                       */ 
    endwin(); /* endwin calls restore_colors subroutine which restores */
               /*the screen to green  frgd color  and black as bkgd */
               /*if do_colors variable is set to true. */
    return;
}

/*
 * FUNCTION: Initialize all curses structures.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: none.
 */
int
init_init()
{
 int rc; 
  /*                  initialise for curses                          */
  do_colors = TRUE;     
  /*do_colors = FALSE;     */
  rc = (int)initscr(); 
  if (rc == NULL)  
  {
    return(FALSE);
  } 

  clear(); refresh();  
  return(TRUE);
}

/*
 * FUNCTION: 
 *  Write the cursor byte offset in the parent window  
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: none.
 */
void
set_byte_off(int number)
{
  char tmp_str[11];
  int i;

  for (i=0;i<10;i++) 
  {
    tmp_str[i]=' ';  
  }

  tmp_str[10] = '\0'; 
  mvwaddstr(ourscr,3,BYTE_OFF,tmp_str);

  i = make_byte_string(number,tmp_str,hexoff);
  
  mvwaddstr(ourscr,3,BYTE_OFF,&tmp_str[i]);
  wrefresh(ourscr);
  return;
}

/*
 * FUNCTION: 
 *  Draw the cursor in the window implied by mode; return OLD mode  
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: The old mode.
 */
int
draw_cursor(int mode)
{
  int x;

  wmove(ourscr,msg_row,msg_col);
  wmove(ascii,ascii_row,ascii_col);
  wmove(hex_part,hex_part_row,hex_part_col);
  x = d_mode; 
  d_mode = mode;
  d_ascii_row = ascii_row; 
  d_hex_part_row = hex_part_row; 
  d_msg_row = msg_row;
  d_ascii_col = ascii_col; 
  d_hex_part_col = hex_part_col;  
  d_msg_col = msg_col;
  return(x);
}
 
/*
 * FUNCTION: 
 *  Display a message centred in the message area   
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: None.
 */
void
display_msg(char *message)
{
	
  if (strlen(message) > 78) 
  {
    return;	
  }

  msg_row = MSG_LINE;  
  msg_col = ((78-strlen(message))/2) + strlen(message);
  mvwaddstr(ourscr,MSG_LINE,((78-strlen(message))/2),message);
  mvwchgat(ourscr,MSG_LINE,((78-strlen(message))/2),
           strlen(message),F_GREEN);
  wrefresh(ourscr);
  return;
}	

/*
 * FUNCTION: 
 * Initialise the cursor position;
 * set old mode to ASCII to force a complete update
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: None.
 */
void
init_cursor()
{
  d_mode = ASCII;
  d_hex_part_row = 0; 
  d_hex_part_col = 0;
  d_ascii_row = 0; 
  d_ascii_col = 0; 
  d_msg_row = MSG_LINE; 
  d_msg_col = 6;
}	

/*
 * FUNCTION: 
 * initialise the parent window ... set up all the cosmetics     
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: None.
 */
void
init_parentw(char *title_str, char *subtitle_str,
             char *sector_disk_str, 
             char *instn_str, char *offset_str)
{
  wclear(ourscr);
  cbox(ourscr);

  mvwaddstr(ourscr,1,1, title_str);
  mvwaddstr(ourscr,3,1,subtitle_str);
  mvwaddstr(ourscr,5,1,sector_disk_str);
  mvwaddstr(ourscr,3,(BYTE_OFF-strlen(offset_str)),offset_str);
  mvwaddstr(ourscr,6,1,instn_str);
  display_msg(current_msg);
  return;
}

/*
 * FUNCTION: 
 * initialise the hex window .. set up all cosmetics     
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: None.
 */
void
init_hexw(char *hexw_str)
{
  fullbox(hex_part,(char)(0xb3),(char)(0xc4),'o','o','o','o'); 
  mvwaddstr(hex_part,0,2,Hhead);  
  mvwaddstr(hex_part,14,13,hexw_str); 
  return;
}

/*
 * FUNCTION: 
 * initialise the ascii window .. set up all cosmetics   
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: None.
 */
void
init_asciiw(char *asciiw_str)
{
  fullbox(ascii,(char)(0xb3),(char)(0xc4),'o','o','o','o');
  mvwaddstr(ascii,14,2,asciiw_str);
  return;
}

/*
 * FUNCTION: 
 *  General display initialisation                              
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: TRUE.
 */
int
init_display(char *title_str, char *subtitle_str, 
             char *sector_disk_str, char *instn_str, 
             char *offset_str, char *keys_str, 
             char *hexw_str, char *asciiw_str) 
{
  ourscr = newwin(24,80,0,0);
  hex_part = subwin(ourscr,15,39,7,8);
  ascii = subwin(ourscr,15,18,7,52);
  actscr = hex_part; passcr = ascii;
  help_scr = newwin(24,44,0,18);
  cbox(help_scr);
  passcr = ascii; 
  actscr = hex_part; 
  mode = HEX; 
    
  init_cursor();
  init_parentw(title_str,subtitle_str,sector_disk_str,instn_str,
               offset_str);
  init_hexw(hexw_str);
  init_asciiw(asciiw_str);
  mvwaddstr(ourscr,22,1,keys_str); 
  return(TRUE);
}

/*
 * FUNCTION: 
 *          General display reset                           
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: None.
 */
void
reset_display()
{
  clear();
  move(1,0);
  refresh();
  return;
}

/*
 * FUNCTION: 
 *        set keyboard to raw and rare meat    
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: None.
 */
void
init_keybd()   
{
  nonl(); 
  noecho(); 
  raw();
  keypad(TRUE);
  return;
}

/*
 * FUNCTION: 
 * initialise the help text strings and set up page and line vectors 
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: None.
 */
void
init_help_strings()
{
  char *a, **b, ***c, *d, *e, *f;
  int j,k,l,m,n;
  long i,p;
  nl_catd fd = diag_catopen(MF_UDMEDIT, 0);
  

  if ( (int)fd == -1 )
  {
    help_text = no_help_pages; 
    return;
  }                    

  /* Get the help headings. */
  help_head[0] = diag_cat_gets(fd,HELP,HEAD0);
  help_head[1] = diag_cat_gets(fd,HELP,HEAD1);
  help_head[2] = diag_cat_gets(fd,HELP,HEAD2);
  help_head[3] = blanks;
  help_head[4] = diag_cat_gets(fd,HELP,HEAD4);
  help_head[5] = blanks;
  help_head[6] = diag_cat_gets(fd,HELP,HEAD6);

  a = diag_cat_gets(fd,HELP,TEXT);
  i = strlen(a);

  /* scan the block read, replacing "0x0a" with "0x00", counting the 
   * lines we discover and any page throws.
   */  

  k=0; l=0;
  for (j=0;j<=i;j++)
  {
    if ( a[j] == '\n' )
    {
      a[j] = '\0'; k++;
      if ( strncmp(&a[j+1],page_throw_string,3) == 0 )
      {
        l++; 
        a[j+1] = '\001'; 
        k--;
      };           
    };
  };

  /* We dont care about empty lines at the end 
   * so scan back to exclude them
   */
  j = i-1;
  while ( a[j] == '\0' )
  {
    j--;  
    k--;   	
  };

  if ( k == 0 )
  {
    /* no text to put up                                             */	
    help_text = no_help_pages; 
    return;
  };
                    
  /* we get enough space for the lines vector, which will never be
   * more than (lines in help file/16)+number of page throws)+lines
   */

  m = k+(k/16)+l+1;  /*@@ 16-> 13 */
  j = (sizeof(char *))*m;
  if ( (b = (char **)malloc(j)) == NULL )
  {
    help_text = no_help_pages; 
    return;
  };
                    

  /* now get enough space for the padded (or truncated) lines
   */
                      
  m = k*(strlen(help_head[0])+1);
  if ( (d = (char *)malloc(m)) == NULL )
  {
    free(b);  	
    help_text = no_help_pages; 
    return;
  };
                    
  /* Here we go, building the help text proper.  We have
   * a series of strings, some of which are page throw
   * indicators.  If the next string is a page throw, then
   * we insert a NULL into the line vector.  Otherwise, we
   * copy the string to the padded buffer, and insert it's
   * address into the vector.  If this the last line on the
   * on the last page then we insert it, otherwise if this
   * is the seventeenth line, then we insert a NULL
   * before it to signify this starts a new page.  Multiple 
   * page throws are condensed to a single page throw.
   */
  l = 0; m = 0; n = 0; p = 0; 
  j = strlen(help_head[3]); 
  e = a; 
  f = d;

  while ( l != k )
  { 
    i = strlen(e);
    if ( e[0] == '\001' )
    {
      /* page throw indicator                                        */
      if ( m != 0 )
      {
        if ( b[m-1] == NULL )
        {
          m--;  
          p--;
        }
        b[m] = NULL; 
        p++; 
        m++; 
        n = 0;
      }
    }
    else
    {
      strcpy(f,help_head[3]); strncpy(f,e,( (i<j)?i:j) );
      l++;
      if ( l != k )
      {
        if ( n == 16 )           	
        {
          b[m] = NULL; 
          m++; 
          n = 0; 
          p++;
        }
      }
      b[m] = f; 
      m++; 
      n++;
      f = &f[j+1];
    }
    e = &e[i+1];      	
  }
  if ( b[m-1] != NULL )
  {
    b[m] = NULL; 
    p++;
  }

  /* The line vector is complete.  We now set up the page vector
   */
  
  m = sizeof(char **)*(p+1); 
  if ( (c = (char ***)malloc(j)) == NULL )
  {
    free(b); 
    free(d);  	
    help_text = no_help_pages; 
    return;
  }

  help_text = c;
  c[0] = b; 
  m = 1; j = 1;
  while ( m < p )
  {
    if ( b[j] == NULL )
    {
      c[m] = &b[j+1]; 
      m++;
    }
    j++;
  }
  c[m] = NULL;
  return;
}

/*
 * FUNCTION: 
 *  Initialise help window .. set up cosmetics                  
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: None.
 */
void
init_helpw()
{
  int i;
  char ***a;
  
  if ( help_text == NULL )
  {
    init_help_strings();
    a = help_text; 
    i = 0;
    while ( a[i] != NULL ) 
    { 
      i++;
    }
    total_help_pages = i;
  };
  

  help_page_number = strrchr(help_head[0],'?')-help_head[0];

  mvwaddstr(help_scr,0,2,help_head[0]);
  mvwaddstr(help_scr,1,2,help_head[5]);
  mvwaddstr(help_scr,19,2,help_head[5]);
  mvwaddstr(help_scr,20,2,help_head[1]);
  mvwaddstr(help_scr,21,2,help_head[2]);
  mvwaddstr(help_scr,22,2,help_head[3]);
  mvwaddstr(help_scr,23,2,help_head[4]);
  mvwchgat(help_scr,0,2,40,BOLD | F_RED);
  mvwchgat(help_scr,20,2,40,F_GREEN);
  mvwchgat(help_scr,21,2,40,F_GREEN);
  mvwchgat(help_scr,22,2,40,F_GREEN);
  mvwchgat(help_scr,23,2,40,BOLD | F_RED);
  return;
}

/*
 * FUNCTION: 
 *  fill the help window with text based on page number       
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: None.
 */
void
fill_help()
{
  int i,j,k;
  char ***a, **b, *c;
  char tmp_str[11];
  
  /* do the page number thing ... make_byte_string expects a 
   * 10 byte buffer
   */ 

  for (i=0;i<10;i++) 
  {
    tmp_str[i]=' ';  
  } 

  tmp_str[10] = '\0'; 
  mvwaddstr(help_scr,0,help_page_number+2,&tmp_str[8]);

  i = make_byte_string((help_page+1),tmp_str,FALSE);
  
  mvwaddstr(help_scr,0,help_page_number+2,&tmp_str[i]);
  mvwchgat(help_scr,0,2,40,BOLD | F_RED);
  wrefresh(ourscr);
  
  a = help_text; 
  b = a[help_page]; 
  k = 0;
  for (j = 2; j < 19; j++)
  {
    if ( b[k] != NULL )
    {
      c = b[k]; 
      k++;
    }
    else
    {
      c = help_head[5];
    };       	
    mvwaddstr(help_scr,j,2,c);
  };

  if ( (help_page+1) != total_help_pages )
  {
    c = help_head[6];  
    mvwaddstr(help_scr,18,2,c);
  };
  touchwin(help_scr);
  wrefresh(help_scr);
  return;
}

/*
 * FUNCTION: 
 *       display simple help                                  
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: None.
 */
void
display_help()
{
  int k;
  char q;
  
  init_helpw();
  q = 'f';  
  help_page = -1;
  k = total_help_pages-1;

  while (q != 'q')
  {
    if (q == 'f') 
    {
      help_page++;
    }
    if (q == 'b') 
    {
      help_page--;
    }
    if (help_page < 0) 
    {
      help_page = 0;
    }
    if (help_page > k) 
    {
      help_page = k;
    }

    if ( (q == 'f') || (q == 'b') )
    {
      fill_help();
    }
    q = (char)(get_input());
  };  

  touchwin(ourscr); 
  touchwin(hex_part); 
  touchwin(ascii);
  wrefresh(ourscr);
  return;
}	

/*
 * FUNCTION: 
 *  repaint the windows if they have changed.  Curses only     
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: None.
 */
void
repaint()
{
  wrefresh((WINDOW *)passcr);
  wrefresh((WINDOW *)actscr);
  return;
}

/*
 * FUNCTION: 
 * flip active/passive screens ... currently only used by curses  
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: None.
 */
void
flip_screens()
{
  tmpscr = passcr; 
  passcr = actscr; 
  actscr = tmpscr;
}  

/*
 * FUNCTION: 
 *  fill the hex window with data from buffer             
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 0.
 */
int
fill_with_hex(char *buff)
{
  int i,j,k,l,m,n;
  char  stringy[36];            /* length of stringy is 2*16+(16-1)/4+1 */

  for (i=0; i<=34; i++) 
  {
    stringy[i] = ' '; 
  }
  stringy[35] = '\0'; 

  for (i=1; i<=13; i++)  /*@@ 16-> 13 */
  {
    for (j=1; j<=16; j=j+1)
    {
      k = (int)buff[(((i-1)*16)+j-1)];
      l = k&0x0F; 
      m = (k>>4)&0x0F;
      n = (2*j)+((j-1)/4); 
      n = n-2;
      stringy[n] = hexchars[m]; 
      n = n+1;
      stringy[n] = hexchars[l];
    };
      wstandend(hex_part);      	 
      mvwaddstr(hex_part,i,2,stringy);
      mvwchgat(hex_part,i,1,37, BOLD | F_BROWN);
      wcolorout(hex_part,F_GREEN);
  };  
  return(0);
}

/*
 * FUNCTION: 
 *  fill the ascii window with data from buffer          
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 0.
 */
int
fill_with_char(char *buff)
{
  int i,j,k,mode;
  char q;
  char stringy[209];
  
  for (i=0; i<=13; i++) 
  {
    stringy[i] = ' '; 
  }
  stringy[16] = '\0';

  for (i=1; i<=13; i++) /*@@ 16-> 13 */
  {
    k = 0;  	
    for (j=1; j<=16; j++)   
    {
      q = buff[(((i-1)*16)+j-1)]; 
      mode = F_BROWN;
      if (q < ' ')
      {
         q ='.'; 
         mode = F_WHITE;
      }
      stringy[k] = q; k = k+1;
  
      wstandend(ascii);
      mvwaddch(ascii,i,j,q);
      mvwchgat(ascii,i,j,1,mode);
      wcolorout(ascii,F_GREEN);
    }
  }  
  return(0);
}

/*
 * FUNCTION: 
 *  Set data in the windows from the buffer       
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: None.
 */
void
set_windows(char *buff) 
{
  fill_with_char(buff); 
  fill_with_hex(buff);
  return;
}

/*
 * FUNCTION: 
 * insert a char q at position indicated by c_nibble nibble address 
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: None.
 */
void
insert_a_char(char q,int nibble)
{
  char s;
  int j,k,l;

  if (q < ' ') 
  {
    s = '.';
  }
  else 
  {
    s = q;
  }

  k = (int)q;
  j = k&0x0F; 
  l = (k>>4)&0x0F;
  set_cursor(((nibble)/2)*2,HEX);
  waddch(ascii,s);
  waddch(hex_part,hexchars[l]);
  waddch(hex_part,hexchars[j]);
  return;
}

  
 
/*
 * FUNCTION: 
 * Get a key-stroke from the keyboard 
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: The key.
 */
int
get_a_key() 
{
  int i;
  
    i = getch();
    if ( msg_clear == TRUE )
    {            
      display_msg(blanks);
      display_msg(default_msg);
      msg_clear = FALSE;
    };             
    return(i);
}










