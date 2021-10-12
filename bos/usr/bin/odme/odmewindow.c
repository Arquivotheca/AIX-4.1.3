static char sccsid[] = "@(#)74	1.26  src/bos/usr/bin/odme/odmewindow.c, cmdodm, bos411, 9428A410j 9/3/91 09:59:18";
/*
 * COMPONENT_NAME: (ODME) ODMEWINDOW - graphics support routines
 *
 * FUNCTIONS: 
 *       void   new_panel ();      create a new panel                 
 *       void   question ();       ask a yes or no question           
 *       void   setup_curses ();   Initial setup of curses & signals  
 *       void   signal_error ();   Signal error handler               
 *       void   quit_curses ();    Closeout curses and exit           
 *       void   prerr ();          Program error handler              
 *       void   hilite ();         hilight the curses field           
 *       void   remwin ();         remove a window from the screen    
 *       int    std_edit_keys ();  standard editing keys              
 *       void   input_field ();    copy the field into storage        
 *       void   null_term ();      terminate the string with a NULL   
 *       void   copy_data ();      copy data from old line to new     
 *       void   superline ();      draw a line on the screen          
 *       void   help ();           display a help box on the screen   
 * 	 void   sbox ();           shadow the window                  
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/**********************************************************************/
/*                                                                    */
/*   FILE NAME :    odmewindow.c                                      */
/*                                                                    */
/*   DESCRIPTION :  This file contains Object Data Manager            */
/*                  Editor window routines.                           */
/*                                                                    */
/*   CONTENTS :                                                       */
/*       void   new_panel ();      create a new panel                 */
/*       void   question ();       ask a yes or no question           */
/*       void   setup_curses ();   Initial setup of curses & signals  */
/*       void   signal_error ();   Signal error handler               */
/*       void   quit_curses ();    Closeout curses and exit           */
/*       void   prerr ();          Program error handler              */
/*       void   hilite ();         hilight the curses field           */
/*       void   remwin ();         remove a window from the screen    */
/*       int    std_edit_keys ();  standard editing keys              */
/*       void   input_field ();    copy the field into storage        */
/*       void   null_term ();      terminate the string with a NULL   */
/*       void   copy_data ();      copy data from old line to new     */
/*       void   superline ();      draw a line on the screen          */
/*       void   help ();           display a help box on the screen   */
/* 	 void   sbox ();           shadow the window                  */
/*                                                                    */
/**********************************************************************/


#include <stdlib.h>
#include "odme.h"
#include <ctype.h>

extern int READONLY;   /* Was this table opened as read only ?       */
extern int INSERTON;   /* Is insert currently on ?                   */

/*------------------------------------------------*/
/*  Curses color coordinates used                 */
/*------------------------------------------------*/
extern int white;
extern int rwhite;
extern int cyan;
extern int rcyan;
extern int magenta;
extern int rmagenta;
extern int red;
extern int green;
extern int TRM;



/**********************************************************************/
/*  NEW PANEL                                                         */
/*      setup new panel and return the panel pointer.                */
/*      numlines = number of items that will be placed in the panel   */
/*                 excluding the header.                              */
/*      numcols  = width of the widest item.                          */
/*      firstline, firstcol = relative position to place the panel    */
/*      header   = the header item for this panel                     */
/*      outer    = the whole window                                   */
/*      inner    = panel data area                                    */
/**********************************************************************/
void new_panel (numlines, numcols, firstline, firstcol, header, outer, inner)

int    numlines,
       numcols,
       firstline,
       firstcol;
char   *header;
WINDOW **outer,
       **inner;
{
/* void sbox(); */

    /*-------------------------------------------------------*/
    /*  Setup initial window with  extra lines for the top   */
    /*  and bottom border, the header line, and the border   */
    /*  below the header.  The number of colums is also      */
    /*  increased for a side border.                         */
    /*-------------------------------------------------------*/
    *outer = newwin (4 + numlines  + 1, (2 * GUTTER) + numcols + 2, firstline, firstcol);
    /*------------------------------------------------------*/
    /*  Print the header and box in the panel               */
    /*------------------------------------------------------*/
    wmove    (*outer, 1, GUTTER);
    waddstr  (*outer, header);
    wcolorout(*outer, Bxa);
    drawbox (*outer, 0, 0, 3, (2 * GUTTER) + numcols);
    drawbox (*outer, 2, 0, numlines + 2, (2 * GUTTER) + numcols);
    wcolorend(*outer);
    *inner = newwin (numlines, numcols, firstline + 3, firstcol + GUTTER);
	
    wrefresh (*outer);
    wrefresh (*inner);
}



/*******************************************************************/
/*  question :  this function displays a boxed question with a     */
/*              continue or discontinue prompt                     */
/*******************************************************************/
int question (qstring)

char qstring[];
{
	void 	new_panel ();


	WINDOW 	*inner_panel,
		*outer_panel;
	int 	c,
		i,
		rc,
		length;

	char	*msgp, resp[80];

	length = strlen (qstring);
	new_panel (1, (2 * GUTTER) + length, 6, CENTER -  (length/2), 
			MSGS(WD,yes_or_no," Yes or No "),
               		&outer_panel, &inner_panel);

	wmove   (inner_panel, 0, 0);
	waddstr (inner_panel, qstring);

	touchwin (inner_panel);
	inner_panel->_csbp = NORMAL;
	wrefresh (inner_panel);

    	beep ();

    	for (;;)
     	{
        	flushinp ();

		msgp = catgets(catd, WD, yes_or_no, "");
		if (*msgp == '\0')
		{
			if (REPLAY)
			{
				c = get_char();
			}
			else
			{
				c = ms_input(inner_panel);
				if (TRAC) save_char(c);
			}

        		if ((c == 'Y') || (c == 'y'))
        		{
        			delwin (inner_panel);
        			delwin (outer_panel);
        			return (TRUE);
        		}
        		else if ((c == 'N') || (c == 'n'))
        		{
        			delwin (inner_panel);
        			delwin (outer_panel);
        			return (FALSE);
        		}
        		else
        			beep ();
		}
		else
		{
			noraw();
			do{
				i = 0;
				resp[i] = (char) getch();
				resp[++i] = '\0';
				rc = rpmatch(resp);
			} while(rc<0);
			raw();
	
			delwin (inner_panel);
			delwin (outer_panel);
			if (rc == 1) return (TRUE);
			else return (FALSE);
		}

     	} /* end for */
} /* end question */



/*******************************************************************/
/* setup_curses :                                                  */
/*         This function sets up the curses windowing environment. */
/*         It's secondary function is the setup of error handling  */
/*         routines used by signals.  Curses color schemes are     */
/*         handled here.                                           */
/*******************************************************************/
void setup_curses ()

{
	char *termtype;
	char *escdelay;
	void quit_curses();
	void help();
     	void signal_error ();

     /*----------------------------------------------------------------------*/
     /*  CURSES initialization and error processing begin.                   */
     /*                                                                      */
     /*      initialize curses environment                                   */
     /*      do not translate '\r' to '\n'                                   */
     /*      data made available before '\n'                                 */
     /*      do not echo typed characters to the screen                      */
     /*      translate sequence characters into integers defined in cur02.h  */
     /*----------------------------------------------------------------------*/
     initscr ();
     nonl ();
     crmode ();
     noecho ();
     keypad (TRUE);
  
     /*----------------------------------------------------------------------*/
     /*   Turn off curses extended mode. The default is TRUE.                */
     /*----------------------------------------------------------------------*/ 
     extended(FALSE);

     /*-------------------------------------------------*/
     /* set up signals for error processing             */
     /*-------------------------------------------------*/
     /* 
     (void) signal (SIGINT,  signal_error);
     (void) signal (SIGQUIT, signal_error);
     (void) signal (SIGIOT,  signal_error);
     (void) signal (SIGBUS,  signal_error);
     (void) signal (SIGSEGV, signal_error);
     (void) signal (SIGTERM, signal_error);
     */
     /*-------------------------------------------------------------------*/
     /* Choose a highlighting scheme based on what the terminal supports  */
     /*     If colour is not available                                    */
     /*        use reverse for highlight                                  */
     /*     else                                                          */
     /*        setup foreground and background                            */
     /*-------------------------------------------------------------------*/
     if (F_CYAN == NORMAL)
       {
          white = NORMAL;
          cyan = NORMAL;
          red = NORMAL;
          magenta = NORMAL;
          red = NORMAL;
          green = NORMAL;
          rwhite = REVERSE;
          rcyan = REVERSE;
          rmagenta = REVERSE;
       }
          rwhite = REVERSE;
     /*-----------------------------------------------------*/
     /*  Make sure initial color is correct                 */
     /*  clear standard screen                              */
     /*  refresh standard screen                            */
     /*-----------------------------------------------------*/
     /* chg_attr_mode (~NORMAL, NORMAL); */
     clear ();
     refresh ();
     TRM = TRUE;
		/*----------------------------------------------------*/
		/* Check terminal type and set the variable TRM.      */
		/*----------------------------------------------------*/
		if ((termtype = getenv("TERM")) == NULL)
			{
			printf("%s",MSGS(WD,no_term,"SET TERM TYPE\n\
 The environment variable TERM has not been set.\n\
 Please set it to the appropriate terminal type\n\
 and restart the ODME.                         \n\
") );
			resetty(TRUE);
   			endwin ();
   			odm_terminate();
			fclose (RD);

   			exit (0);
			}
		if (strcmp(termtype,"ibm6154") == 0)
			TRM = FALSE;
		if (strcmp(termtype,"ibm6155") == 0)
			TRM = FALSE;
		if (strcmp(termtype,"ibm5151") == 0)
			TRM = FALSE;
		if (strcmp(termtype,"hft") == 0)
			TRM = FALSE;  

		escdelay = getenv("ESCDELAY");
		if ((atoi(escdelay)) < 1500)
		/* set the ESCDELAY value in the environment */
			putenv ("ESCDELAY=1500");

	raw();
	meta();

}  /* end setup curses */



/******************************************************************/
/*  signal_error :                                                */
/*        This is the signal error handling function. When the    */
/*        system encounters a segment violation, bus error, etc.  */
/*        this function will be called                            */
/******************************************************************/
void signal_error (signal_code)

int signal_code;
{
   void prerr ();

   prerr (MSGS(WD,signal_msg,"Signal recieved = "), signal_code, TRUE);
   exit  (signal_code);
}


/*******************************************************************/
/*  quit curses :                                                  */
/*         clear screen and close curses windowing.                */
/*******************************************************************/
void quit_curses ()

{

	/* catclose (catd); */
   	/* chg_attr_mode (~NORMAL, NORMAL); */

   	clear  ();
   	refresh ();
	resetty(TRUE);
   	endwin ();
   	odm_terminate();
	fclose (RD);

   	exit (0);
}



/*****************************************************************/
/*   PRERR   window error code for user to see and act on.       */
/*           message     = error message to print to user        */
/*           return_code = return code to exit with              */
/*           flag        = TRUE only option is to quit           */
/*                          FALSE quit or continue               */
/*****************************************************************/
void prerr (message, return_code, flag)

char *message;
int  return_code,
     flag;
{
   void remwin ();

   int c,
       length;
   WINDOW *inner_panel,
          *outer_panel;

   beep ();
   flushinp ();

   /*------------------------------------------------------*/
   /* calculate which is greater (the top heading) or the  */
   /* message text brought in.                             */
   /* add 6 spaces for an error number appended to the end */
   /*------------------------------------------------------*/
   length = strlen (message) > 33 ? strlen(message) : 33;
   length += 6;

   if (flag == TRUE)
     new_panel (1, length, 6, CENTER - (length/2), 
		MSGS(WD,pr_quit,"<Esc>3 QUIT odme"),
                &outer_panel, &inner_panel);
   else
     new_panel (1, length, 6, CENTER - (length/2),
                MSGS(WD,pr_keys,"<Esc>1 Return   <Esc>3 QUIT odme"),
		 &outer_panel, &inner_panel);

   /*---------------------------------------------*/
   /*  Print the error message;                   */
   /*  If return code != 0 print return code      */
   /*---------------------------------------------*/
   wmove   (inner_panel, 0, 0);
   waddstr (inner_panel, message);
   if (return_code != 0)
      wprintw (inner_panel, "%d", return_code);

   touchwin (inner_panel);
	inner_panel->_csbp = NORMAL;
   wrefresh (inner_panel);


   /*-------------------------------------*/
   /*  get user response either quit      */
   /*  or continue if possible.           */
   /*-------------------------------------*/
   for (;;)
    {

	if (REPLAY)
		{
		c = get_char();
		}
	else
		{
	c = ms_input(inner_panel);
	if (TRAC) save_char(c);
		}

       switch (c)
        {
           /*-------------------------------------------------*/
           /*  Continue key pressed.  If possible allow user  */
           /*  to continue.                                   */
           /*-------------------------------------------------*/
           case ESC1     :
           case KEY_F(1) :
                  if (flag)
                      beep ();
                  else
                   {
                      remwin (outer_panel, white);
                      remwin (inner_panel, white);

                      return;
                   }
                  break;

           case ESC3      :
           case KEY_F(3)  :
           case KEY_QUIT  :
           case KEY_F(12) :
                  remwin (outer_panel, white);
                  remwin (inner_panel, white);

                  quit_curses ();

           default:
                  beep ();
                  break;

        }    /* end switch */
    }   /* end for */
}  /* end prerr */



/************************************************************************/
/*   hilite :   highlight the current field                             */
/*       Parameters :  win   = the current window                       */
/*                     view  = the current view                         */
/*                     line  = the line to highlight                    */
/*                     col   = the column to start the highlight        */
/*                     width = the width of the highlight field         */
/*                     cursor_line = what line to leave the cursor on   */
/*                     cursor_col  = what column to leave the cursor on */
/*                     high_color  = highlight color                    */
/*                     reg_color   = regular color                      */
/************************************************************************/
void hilite (win, view, line, column, width, cursor_line, cursor_column,
             high_color, reg_color)

 WINDOW *win,
        *view;
 int line,
     column,
     width,
     cursor_line,
     cursor_column,
     high_color,
     reg_color;
{
   /*-----------------------------------------------*/
   /*  highlight the line by changing the attribute */
   /*-----------------------------------------------*/
   wmove  (win, line, column);
   wchgat (win, width, high_color);
   wmove  (view, cursor_line, cursor_column);
   touchwin (view);
	view->_csbp = NORMAL;
   wrefresh (view);
   /*-------------------------------------------*/
   /* unhighlight the line so next refresh      */
   /* will cause highlight to disappear         */
   /*-------------------------------------------*/
   wmove    (win, line, column);
   wchgat   (win, width, reg_color);
   wmove    (view, cursor_line, cursor_column);

}  /* end highlight */



/*****************************************************************/
/*      remwin : remove the window from the screen               */
/*  PARAMETERS : win - the window to remove                      */
/*               color - colorout the space with this color      */
/*****************************************************************/
void remwin (win, color)

WINDOW *win;
int    color;
{
/* the following 4 line were uncommented to touch the entire window */
   wmove     (win, 0, 0);
   wcolorout (win, color); 
   werase    (win);
   touchwin  (win);

	win->_csbp = NORMAL;
   wrefresh  (win);
   delwin   (win);
}



/**********************************************************************/
/*  Standard edit keys                                                */
/*          This function provides the standard edit and cursor       */
/*          movement for curses windows.                              */
/*  Parameters :                                                      */
/*          input_window  : the current window                        */
/*          c             : the current input character               */
/*          start_column  : the current fields startcolumn            */
/*          data_width    : the width of the field                    */
/*          cursor_column : column the cursor is currently on         */
/*          cursor_line   : line the cursor is currently on           */
/**********************************************************************/
int std_edit_keys  (input_window, c, start_column, data_width,
                    cursor_column, cursor_line)

WINDOW *input_window;
int c,
    start_column,
    data_width,
    *cursor_column,
    *cursor_line;
{
    int delta = FALSE;
    int char_width;
    register i;
    wchar_t wc;

    char_width = mbtowc(&wc, mbstring, MB_LEN_MAX);

    if (char_width < 0)
	return(-1);

    switch (c)
    {
        /*-----------CURSOR KEY LEFT MOVEMENT------------*/
        case KEY_LEFT:
        case CTRL_B:
               if (*cursor_column == start_column)
                  beep ();
               else
	       {
		  wc = winch(input_window);
		  char_width = wcwidth(wc);
                  *cursor_column -= char_width;
	       }
               break;

        /*----------CURSOR KEY RIGHT MOVEMENT------------*/
        case KEY_RIGHT:
        case CTRL_F:
               if (*cursor_column >= (start_column + data_width - 1))
                  beep ();
               else
	       {
		  wc = winch(input_window);
		  char_width = wcwidth(wc);
                  *cursor_column += char_width;
	       }
               break;

        /*-------------CURSOR UP-------------------------*/
        case KEY_UP :
        case CTRL_P :
              if (*cursor_line == 0)
                  beep ();
              else
                  (*cursor_line)--;
              break;

        /*-------------BACKSPACE pressed-----------------*/
        case KEY_BACKSPACE:
               if (*cursor_column == start_column)
                  beep ();
               else
                {
		  wc = winch(input_window);
		  char_width = wcwidth(wc);
                  *cursor_column -= char_width;
                  wmove (input_window, *cursor_line, *cursor_column);
		  for (i = 0; i < char_width; i++)
		  {
			wmove (input_window, *cursor_line, *cursor_column + i);
			waddch (input_window, (wchar_t)' ');
		  }
                  delta = TRUE;
                }
               break;

        /*--------------DELETE CHAR----------------------*/
        case KEY_DC:
        case CTRL_D:
               wmove  (input_window, *cursor_line, *cursor_column);
               wdelch (input_window);
               wmove (input_window, *cursor_line, start_column + data_width-1);
               winsch (input_window, ' ');
               delta = TRUE;
               break;

        /*---------Beginning of line key pressed-----------*/
        case CTRL_A   :
        case KEY_HOME :
               *cursor_column = start_column;
               break;

        /*------------End of line key pressed---------------*/
        case KEY_EOL  :
        case CTRL_E   :
        case KEY_END  :
               *cursor_column = start_column + data_width - 1;
               break;

        /*--------------INSERT TOGGLE-----------------------*/
        case INSERT   :
              if (INSERTON)
                  INSERTON = FALSE;
              else
                  INSERTON = TRUE;
              break;

        /*-------------CTRL K : kill current field----------*/
        case CTRL_K   :
              for (i = 0; i < data_width; i++)
               {
                  wmove  (input_window, *cursor_line, start_column + i);
                  wdelch (input_window);
                  winsch (input_window, ' ');
               }
		*cursor_column = start_column;
              delta = TRUE;
              break;

        /*----------------------DEFAULT----------------------*/
        default:

               if (*cursor_column >= (start_column + data_width))
                {
                   beep ();
                   break;
                }

		if (REPLAY)
		{	
			if (isprint(c))
			{
                              	wmove  (input_window, *cursor_line,					 		*cursor_column);
				waddch (input_window, c);
				if (*cursor_column <= (start_column
					+ data_width - 1)) (*cursor_column)++;
				delta = TRUE;
				break;
			}
			else
			{
				delta = FALSE;
				break;
			}
		}

                if (iswprint ((wint_t) wc))
                { 
                   if (INSERTON)
                    {
                       wmove (input_window, *cursor_line,
                              start_column + data_width - 1);
                       wdelch (input_window);
                       wmove  (input_window, *cursor_line, *cursor_column);
                       winsch (input_window, wc);
                       if (*cursor_column < (start_column + data_width - 1))
                           *cursor_column += char_width;
                    }
                   else
		    { 
                        wmove  (input_window, *cursor_line, *cursor_column);
		       	waddstr (input_window, mbstring);
                       	if (*cursor_column < (start_column + data_width - 1))
                           *cursor_column += char_width;
                    }

                   delta = TRUE;
                   break;
                }
               else
                   beep ();

               break;

     } /* end switch */
     /*-------------------------------------------------------------*/
     /* the final return condition will tell the calling function   */
     /* if changes were made.                                       */
     /*-------------------------------------------------------------*/
     if (delta)
        return (1);
     else
        return (0);

} /* end standard keys */



/***********************************************************************/
/*   input_field : Purpose; the input of a screen field into a string  */
/*                 variable.                                           */
/*                                                                     */
/*   Parameters : input_window - the field's window                    */
/*                line         - which line the field appears on       */
/*                start_column - column the field starts on            */
/*                data_width   - length of the field                   */
/*                storage      - character storage to place the data   */
/***********************************************************************/
void input_field (input_window, line, start_column, data_width, storage)

WINDOW 	*input_window;
int	line,
       	start_column,
       	data_width;
char 	*storage;

{
   void null_term ();

   register 	i;

   int 		newcol = 0;     /* field column index */

   wchar_t	*wcs; 		/* temp storage for winch input */

   /*-------------------------------------------------------*/
   /* Malloc space for temp storage                         */
   /*-------------------------------------------------------*/

   if ((wcs = (wchar_t *) malloc ((data_width + 1) * sizeof(wchar_t))) == NULL)
	prerr (MSGS(ER, WD_MALLOC_FAILED, DF_WD_MALLOC_FAILED), 0, TRUE);

   /*--------------------------------------------------------*/
   /*  Traverse the input field storing characters           */
   /*--------------------------------------------------------*/
   for (i = 0; newcol < data_width; i++)
    {
	wmove(input_window, line, start_column + newcol);
	wcs[i] = winch(input_window);
	newcol += wcwidth(wcs[i]);
    }

    wcs[i] = 0;
   /*--------------------------------------------------*/
   /* Convert wide character to multi byte for storage */
   /*--------------------------------------------------*/
   wcstombs(storage, wcs, data_width);

   storage[data_width] = '\0';

   null_term (storage);

   free(wcs);
}


/**********************************************************************/
/*  nullterm :                                                        */
/*        Terminate the given string with the null character          */
/**********************************************************************/
void null_term (string)

char string[];
{
   register i;

   for (i = (strlen (string)); i >= 0; i--)
    {
      if (iswspace(string[i]) || iswcntrl(string[i]) || string[i] == (char)NULL)
          continue;
      else
          break;
    }
   string [i + 1] = '\0';

} /* end nullterm */




/***************************************************************/
/*  copy_data  : copy data from old line to the new line       */
/*                                                             */
/*        Parameters :  win - the field window                 */
/*                      start_column - column to start from    */
/*                      data_width   - width of data           */
/*                      old_line     - copy data from line     */
/*                      new_line     - copy data to line       */
/***************************************************************/
void copy_data (win, start_column, data_width, old_line, new_line)

WINDOW 	*win;
int 	start_column,
    	data_width,
    	old_line,
    	new_line;
{
  	wchar_t	temp;
  	register i;

  	for (i = 0; i < data_width; i++)
   	{
      		wmove (win, old_line, start_column + i);
      		if ((temp = winch (win)) == ' ') continue;

      		wmove (win, new_line, start_column + i);
      		waddch (win, temp);
   	}
}  /* end copy field */



/**********************************************************************/
/*  superline  :  this function draws a line between points.          */
/*  PARAMETERS :  win - the current user window, x1 - the beginning   */
/*                line parameter, y1 - the beginning column parameter */
/*                x2 - the ending line parameter, y2 - the ending     */
/*                column parameter.                                   */
/**********************************************************************/
void superline (win, x1, y1, x2, y2)

WINDOW  *win;
int     x1, y1, x2, y2;
{

     	if (x1 == x2)
      	{
        	for (;y1 <= y2; y1++)
         	{
            		wmove  (win, x1, y1);
            		waddch (win, 'Ä');
         	}
      	}
     	else if (y1 == y2)
      	{
        	for (;x1 <= x2; x1++)
         	{
           	 	wmove  (win, x1, y1);
            		waddch (win, '³');
         	}
      	}
} /* end superline */



/**************************************************************/
/*  help  -  odm_run_methodd normally by pressing PF1 or Esc 1.  This */
/*           function displays a help text for the current    */
/*           screen.                                          */
/* PARAMETERS : This function can take a variable number of   */
/*              character strings as its parameters.  Always  */
/*              include (char *) NULL as the last parameter   */
/*              as this is the (end of parameters) condition  */
/**************************************************************/
void help (help_data)
char help_data[];

{
   	WINDOW 	*inner_panel,
          	*outer_panel;
   	va_list ap;
   	char 	*arguments[MAXARGS];
	char 	*ptr;	
	char 	tmpmsg[2000];
   	int  	arg_number = 0,
        	c,
        	i,
        	j,
        	length     = 0,
        	max        = 0;


	strcpy (tmpmsg, help_data);

	for (i=0; i < MAXARGS; i++)
	{
		if ( (arguments[i] = (char *) malloc(COLS - 15) ) == NULL)
		{
			for (j = 0; j < i; j++)
				free (arguments[j]);
			prerr (MSGS(ER,WD_MALLOC_FAILED,DF_WD_MALLOC_FAILED),
				 0, TRUE);
		}
	}

	ptr = strtok(tmpmsg, "\n");
	strcpy (arguments[arg_number], ptr);
	
	length = strlen (arguments[arg_number]);
	arg_number++;

	while ((ptr = strtok((char *) NULL, "\n")) != NULL)
	{
		strcpy (arguments[arg_number], ptr);
		length = strlen (arguments[arg_number]);
		if (length > max) max = length;
		arg_number++;
	}
	
	/*
   	for (va_start (ap);
        ((arguments[arg_number] = va_arg (ap, char *)) != (char *) NULL);
        arg_number++)
    	{
       		length = strlen (arguments[arg_number]);
       		if (length > max) max = length;
    	}

   	arg_number--;

   	va_end (ap);
	*/
/*
   new_panel (arg_number, (2 * GUTTER) + max, (LINES / 2) - (arg_number / 2),
          CENTER -  (max/2) - GUTTER, arguments[0], &outer_panel, &inner_panel);
*/

   	new_panel (arg_number, (2 * GUTTER) + max, 5,
          5, arguments[0], &outer_panel, &inner_panel);

   	for (i = 1, j = 0; i <= arg_number; i++, j++)
    	{
      		wmove   (inner_panel, j, 0);
      		waddstr (inner_panel, arguments[i]);
    	}

	inner_panel->_csbp = NORMAL;
   	wrefresh (inner_panel);

	if (REPLAY)
	{
		c = get_char();
	}
	else
	{
		c = ms_input(inner_panel);
		if (TRAC) save_char(c);
	}

	for (i=0; i <= arg_number; i++)
		free(arguments[i]);

   	delwin (inner_panel);
   	delwin (outer_panel);

}  /* end of help */

/********************************************************/
/* SBOX	  This routine will place shaded box around     */
/*        the window passed. 				*/
/* ulx 	upper left x coordinate in wnd			*/
/* uly	upper left y coordinate in wnd			*/
/* lrx	lower right x coordinate in wnd			*/
/* lry	lower rignt y coordinate in wnd			*/
/* wnd	window to be shaded				*/
/********************************************************/
/*
void sbox(ulx, uly, lrx, lry, wnd)
int 	ulx,
    	uly,
    	lrx,
    	lry;

WINDOW 	*wnd;

{
	int 	i;

	wmove (wnd, ulx, uly);
	for ( i = 0; i < (lry - uly); i++)
		waddstr (wnd, "Ü");

	wrefresh (wnd);

	for (i = 1; i < (lrx - ulx + 1); i++)  
	{
		wmove (wnd, ulx + i, uly - 1);
		waddstr (wnd, "±Û");
		wmove (wnd, ulx + i, lry - 1);
		waddstr (wnd, "Û");
	}

	wrefresh (wnd);
	wmove (wnd, lrx, uly + 1);

	for ( i = 0; i < (lry - uly - 2); i++)
		waddstr (wnd, "Ü");
	wrefresh (wnd);
	wmove (wnd, lrx + 1, uly  - 1);

	for ( i = 0; i < (lry - uly); i++)
		waddstr (wnd, "±");

	wnd->_csbp = NORMAL;
	wrefresh (wnd);
}
*/

int fncnt = 1;
char scrf[] = "SCRNA";

/************************************************************************/
/*	ODME_PRSR     This routine will print the contents of           */
/*                    the current screen.				*/
/*		      Screen images are stored in alphabetized files	*/
/*		      beginning with SCRNA then SCRNB, SCRNC etc.       */
/*		      Users can print these files after exiting ODME	*/
/*		      A beep will signal a successful print operation.	*/
/*		      Future enhancements may be to use a method to    	*/
/*		      print the file real time, and allow printer       */
/*		      selection.					*/
/*	INPUT :    none							*/
/*	OUTPUT :   TRUE if file opened successfully and screen was     	*/
/*		   copied.						*/
/*		   FALSE if file open failed.				*/
/************************************************************************/
int odme_prscr ()
{

	char 	line[200];
	long 	x, y;
	int	c;
	FILE 	*fp;

	/*--------------------------------------------------------------*/
	/* open the file for writing. If the open fails, return FALSE. 	*/
	/*--------------------------------------------------------------*/
	if ((fp = fopen (scrf, "w")) == NULL)
		return (FALSE);  

	/*--------------------------------------------------------------*/
	/* do a form feed and new line. Print the heading.              */
	/* Loop through the current screen image which is contained in  */
	/* curscr and save the data a line at a time at in the file     */
	/*--------------------------------------------------------------*/
	fprintf (fp, "\n");
	fprintf (fp, MSGS(WD,pr_screen_hdng,
		"\n------Object Data Manager Editor Print------\n\n") );

	for (y = 0 ; y < LINES ; y++)
	{
    		for (x = 0 ; x < COLS ; x++)
		{
			wmove(curscr, y, x);
			c = winch(curscr);
			line[x] = c;
		} 

		/*------------------------------------------*/
		/* terminate the line with a NULL           */
		/*------------------------------------------*/
    		line[x] = '\0';
    		fprintf(fp,"%s\n",line);
	}

	fflush(fp);

	/*-------------------------------------------------------------*/
	/* increment the the fifth letter of the file name to the next */
	/* letter of the alphabet so the next file will be uniquely    */
	/* named.	               				       */
	/*-------------------------------------------------------------*/
	scrf[4]++;
	beep ();

	/*-----------------------*/
	/* close the file        */
	/*-----------------------*/
	fclose (fp);
	return (TRUE);

} /* end odme_prscr */



