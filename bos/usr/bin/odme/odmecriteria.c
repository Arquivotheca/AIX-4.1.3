static char sccsid[] = "@(#)70	1.11  src/bos/usr/bin/odme/odmecriteria.c, cmdodm, bos411, 9428A410j 4/18/91 18:51:22";
/*
 * COMPONENT_NAME: (ODME) ODMECRITERIA - selective searching of object classes
 *
 * FUNCTIONS: search_criteria
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
/*   FILE NAME :    odmecriteria.c                                    */
/*                                                                    */
/*   DESCRIPTION :  This file contains Object Data Manager            */
/*                  Editor object class search criteria routines.     */
/*                                                                    */
/*   CONTENTS :                                                       */
/*        void search_criteria ();  User entry of search criteria     */
/*                                  when getting objects from object  */
/*                                  class.                            */
/**********************************************************************/

#include "odme.h"

extern int READONLY;
extern int INSERTON;
extern int TRM;

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

/********************************************************************/
/* search_criteria :  This function presents the first screen of    */
/*                    the object class. The purpose is the display  */
/*                    of the descriptors in the object for user     */
/*                    entry of search criteria.                     */
/*                                                                  */
/* example :       DISPLAY                   USER ENTERS            */
/*            data_5 (char 5)  ===>            = bobby              */
/*            data_6 (short 5) ===>            = 2                  */
/*            data_7 (long 10) ===>                                 */
/*                                                                  */
/*                   Once search criteria is entered object_display */
/*                   is called with the user's search criteria.     */
/********************************************************************/
void search_criteria (objhandle, head_descriptor_data)

struct objtoken        *objhandle;
struct descriptor_data *head_descriptor_data;

{
   void get_descriptors ();     /* get descriptors for object class         */
   void prerr ();               /* error handling routine                   */
   void hilite ();              /* hilight the data area                    */
   void object_display ();      /* display objects for object class         */
   int  std_edit_keys ();       /* use standard edit keys for user input    */

   int  c,                             /* keyboard character input variable */
        criteria_column,               /* where the criteria begins         */
        cursor_col  = 0,               /* column to display cursor          */
        cursor_line = 0,               /* line to display cursor            */
        lastlines   = 0,               /* number of lines on final page     */
        length      = 0,               /* length of header                  */
        maxvertical = 0,               /* maximum vertical number of pages  */
        total_input = 0,               /* total number of input characters  */
        vertlines   = 0,               /* number of lines on current page   */
        vscreen     = 0;               /* screen that we are currently on   */

   char linestring[MAX_INPUT_LENGTH],          /* input string from screen  */
        searchstring[MAX_SEARCH_LENGTH],       /* temporary search string   */
        andor [7];                             /* "and (" search append     */

   WINDOW *key_window,           /* bottom keys window                      */
          *box_window,           /* border box window                       */
          *criteria_view,        /* view of criteria window                 */
          *criteria_window;      /* criteria window                         */

   register i;                   /* for loop traversal variable             */
   char objs[256];          /* object search criteria used in get      */

   struct descriptor_data
            *ptr_descriptor_data;

   /*---------------------------------------------------------------------*/
   /*   Max. vertical no. of pages = no. of descriptors / items per page  */
   /*   No. of lines on last page = no. of descriptors MOD items per page */
   /*   If (number of descriptors = items per page)                       */
   /*       no. of lines on last page = items per page;                   */
   /*       subtract 1 from page number;                                  */
   /*---------------------------------------------------------------------*/
   maxvertical = objhandle->number_desc / MAX_CRITERIA_ITEMS;
   lastlines   = objhandle->number_desc % MAX_CRITERIA_ITEMS;
   if (lastlines == 0)
    {
      lastlines = MAX_CRITERIA_ITEMS;
      maxvertical--;
    }

   /*---------------------------------------------------------------*/
   /* CURSES begin : Window opens for search criteria entry screen. */
   /*                                                               */
   /*   screen           ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿ b      */
   /*   representation : ³  Statistics window (table name) ³ o      */
   /*                    ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´ x      */
   /*                    ³  Descriptors window             ³        */
   /*                    ³     descriptor ==> criteria     ³ w      */
   /*                    ³                                 ³ i      */
   /*                    ³                                 ³ n      */
   /*                    ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´ d      */
   /*                    ³  Bottom Function keys           ³ o      */
   /*                    ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ w      */
   /*---------------------------------------------------------------*/
   box_window = newwin (LINES - 1, 0, 0, 0);
   wmove     (box_window, 0, 0);
   wcolorout (box_window, white);
   werase    (box_window);

   wcolorout(box_window, Bxa);
   drawbox (box_window, 0, 0, 3, COLS);
   drawbox (box_window, 2, 0, LINES - 3, COLS);
   wcolorend(box_window);

   length = strlen (MSGS(SC,CRITERIA_NAME,DF_CRITERIA_NAME) );
   wmove     (box_window, 0, CENTER - (length/2));
   waddstr   (box_window, MSGS(SC,CRITERIA_NAME,DF_CRITERIA_NAME) );
   wmove     (box_window, 1, GUTTER);
   wprintw   (box_window, 
		MSGS(SC,sc_objclass,"Object Class : %s"), objhandle->objname);

   touchwin  (box_window);
   box_window->_csbp = NORMAL;
   wrefresh  (box_window);

   /*----------------------------------------------------------*/
   /* Create a bottom key window 1 line deep and on the last   */
   /* line of the screen.                                      */
   /*----------------------------------------------------------*/
   key_window = newwin (1, 0, LINES - 1, 0);
   wmove     (key_window, 0, 0);
   wcolorout (key_window, white);
   werase    (key_window);

   wmove     (key_window, 0, 0);
   waddstr   (key_window, MSGS(KS,CRITERIA_KEYS,DF_CRITERIA_KEYS) );

   touchwin  (key_window);
	key_window->_csbp = NORMAL;
   wrefresh  (key_window);

   /*-------------------------------------------------------------*/
   /* Now set up the display window for presenting the descriptor */
   /* name, type, size, and an area for criteria entry            */
   /*-------------------------------------------------------------*/
   criteria_window = newwin  (MAX_CRITERIA_ITEMS * (maxvertical + 1),
                              COLS - (2 * GUTTER), 3, GUTTER);
   criteria_view   = newview (criteria_window, MAX_CRITERIA_ITEMS,
                              COLS - (2 * GUTTER));

   /*-------------------------------------------------------------------------*/
   /* DESCRIPTOR WINDOW display begins : Display the descriptor name          */
   /*                   descriptor data size, and prompt.                     */
   /*-------------------------------------------------------------------------*/
   /*  compute where the criteria column begins (the descriptor name length + */
   /*  1 space and 2 parenthesis + the length of the longest descriptor.ODM_type  */
   /*  + the length of " ==> "                                                */
   /*-------------------------------------------------------------------------*/
   criteria_column = MAX_DESC_NAME + 3 + MAX_HEADER_LENGTH + 5;

   for (i = 0, ptr_descriptor_data = head_descriptor_data;
        i < objhandle->number_desc; ptr_descriptor_data++, i++)
    {
       wmove   (criteria_window, i, 0);
       wprintw (criteria_window, "%-*.*s", MAX_DESC_NAME, MAX_DESC_NAME,
                ptr_descriptor_data->classp->elem[i].elemname);
       wprintw (criteria_window, " (%s)", ptr_descriptor_data->header);
       wmove   (criteria_window, i, criteria_column - 4);
       wprintw (criteria_window, "==> ");
    }

   touchwin (criteria_view);
	criteria_view->_csbp = NORMAL;
   wrefresh (criteria_view);

   cursor_col = criteria_column;

   /*-----------------------------*/
   /*   USER INPUT BEGINS         */
   /*-----------------------------*/
   for (;;)
    {
       /*------------------------------------------------*/
       /* set the number of vertical lines for this page */
       /*------------------------------------------------*/
       if (vscreen == maxvertical)
         vertlines = lastlines;
       else
         vertlines = MAX_CRITERIA_ITEMS;

       hilite (criteria_view, criteria_view, cursor_line, 0,
               criteria_column - 1, cursor_line, cursor_col, rwhite, white);


	if (REPLAY)
		{
		c = get_char();
		}
	else
		{
	c = ms_input(criteria_view);
	if (TRAC) save_char(c);
		}

       switch (c)
        {

          /*------------------------HELP PRESSED-------------------------*/
          case KEY_F(1) :
          case ESC1     :
		help (MSGS(SC,criteria_help,
                "Criteria help \n\
 The Criteria screen allows you to enter SQL like  \n\
 syntax for searching the object class.            \n\
                                                   \n\
      What ODME displays        What you type      \n\
   ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿    \n\
   ³ Programmer  (char 23) ==>   like 'HAL'   ³    \n\
   ³ Age         (short 7) ==>   = 23         ³    \n\
   ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ    \n\
 Pressing Search (<Esc>2 or PF2) will return all   \n\
 objects where the descriptors satisfy the criteria \n\
 (Programmer like 'HAL') and (Age = 23).        \n\
                                                   \n\
 Entering no search string returns all objects.    \n\
") );
		touchwin (box_window);
		box_window->_csbp = NORMAL;
		wrefresh (box_window);

      touchwin (criteria_view);
		criteria_view->_csbp = NORMAL;
      wrefresh (criteria_view);
		
		touchwin (key_window);
		key_window->_csbp = NORMAL;
		wrefresh (key_window);

                break;

          /*--------SEARCH key pressed : Get search criteria-------------*/
          case KEY_F(2) :
          case KEY_DO   :
          case ESC2 :
                /*---------------------------------------------------------*/
                /*  while (more descriptors) do                            */
                /*   {                                                     */
                /*      input user string into linestring                  */
                /*      if (valid characters were found)                   */
                /*          store descriptor name in searchstring;         */
                /*          append user string to searchstring             */
                /*   }                                                     */
                /*---------------------------------------------------------*/

                searchstring[0] = '\0';
                strcpy (andor," ");

                for (i = 0; i < objhandle->number_desc; i++)
                 {
                   linestring[0] = '\0';

                   input_field (criteria_window, i, criteria_column,
                               COLS - GUTTER - criteria_column - 2, linestring);

                   if (linestring[0] != '\0')
                    {
			strcat (searchstring, andor);
                        strcat (searchstring,
                               head_descriptor_data->classp->elem[i].elemname);
                        strcat (searchstring, " ");
                        strcat (searchstring, linestring);
                        strcpy (andor, " and ");
                    }
                 } /* end for */

                /*-----------------------------------------------------*/
                /* If a search string was not entered use the default  */
                /* (NULL).  Maximum 200 characters.                    */
                /*-----------------------------------------------------*/
                total_input = strlen (searchstring);

                if (total_input == 0)
                   objs[0] = '\0';
                else if (total_input >= MAX_SEARCH_LENGTH)
                   prerr (MSGS(ER,SC_SEARCH_STRING,DF_SC_SEARCH_STRING),0,TRUE);
                else
                   strcpy (objs, searchstring);

                /***********************************************/
                /*  IMPORTANT CALL OBJECT DISPLAY              */
                /*     use current descriptors and search      */
                /*     string built.                           */
                /***********************************************/


                object_display (objhandle, objs, head_descriptor_data);

                touchwin (box_window);
		box_window->_csbp = NORMAL;
                wrefresh (box_window);

                touchwin (criteria_view);
		criteria_view->_csbp = NORMAL;
                wrefresh (criteria_view);

                touchwin (key_window);
		key_window->_csbp = NORMAL;
                wrefresh (key_window);

                break;

          /*------------EXIT key pressed : leave search criteria-----------*/
          case KEY_F(3) :
          case KEY_QUIT :
          case ESC3     :
                remwin (key_window, white);
                remwin (box_window, white);
                delwin (criteria_window);

                return;

          /*-----------------CURSOR KEY UP-----------------------*/
          case KEY_UP:
          case CTRL_P:
                cursor_line--;
                if (cursor_line < 0)
                 {
                    if (vscreen == 0)
                     {
                        cursor_line = 0;
                        beep ();
                     }
                    else
                     {
                        vscreen--;
                        vertlines   = MAX_CRITERIA_ITEMS;
                        cursor_line = vertlines - 1;
                        vscroll (criteria_view, -MAX_CRITERIA_ITEMS, 0);
                     }
                 }
                break;

          /*-----------------RETURN KEY-----------------------*/
          case KEY_NEWL:
	  case KEY_NWL :
                cursor_col = criteria_column;

          /*----------------CURSOR KEY DOWN---------------------*/
          case KEY_DOWN:
          case CTRL_N:
                cursor_line++;
                if (cursor_line == vertlines)
                 {
                    if (vscreen == maxvertical)
                     {
                        cursor_line--;
                        beep ();
                     }
                   else
                     {
                        vscreen++;
                        cursor_line = 0;
                        vscroll (criteria_view, MAX_CRITERIA_ITEMS, 0);
                     }
                 }
                break;

          /*-----------PREVIOUS PAGE key pressed------------------*/
          case KEY_F(7)  :
          case KEY_PPAGE :
          case KEY_SR    :
          case ESC7      :
                if (vscreen == 0)
                    beep ();
                else
                 {
                    vscreen--;
                    cursor_line = 0;
                    vscroll (criteria_view, -MAX_CRITERIA_ITEMS, 0);
                 }
                break;

          /*------------NEXT PAGE key pressed---------------------*/
          case KEY_F(8)  :
          case KEY_NPAGE :
          case KEY_SF    :
          case ESC8      :
                if (vscreen == maxvertical)
                    beep ();
                else
                 {
                    vscreen++;
                    cursor_line = 0;
                    vscroll (criteria_view, MAX_CRITERIA_ITEMS, 0);
                 }
                break;

			case KEY_F(12) :
				if ((odme_prscr ()) == FALSE)
					{
					help (MSGS(ER,PRINT_SCREEN_ERROR,DF_PRINT_SCREEN_ERROR) );
					}
				break;

          /*---------------------DEFAULT-------------------------*/
          default:
                std_edit_keys (criteria_view, c, criteria_column,
                               COLS - GUTTER - criteria_column - 2, &cursor_col,
                               &cursor_line);
                break;

        } /* end switch */
    } /* end for */
} /* end search criteria */
