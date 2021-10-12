static char sccsid[] = "@(#)66  1.31  src/bos/usr/bin/odme/odme.c, cmdodm, bos411, 9428A410j 12/7/93 11:47:05";
/*
 * COMPONENT_NAME: (ODME) Main for Object Data Manager Editor
 *
 * FUNCTIONS: Main, Object_Name_Edit
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
#include "odme.h"
#include <locale.h>

extern int READONLY;
extern int INSERTON;
extern int TRM;
extern char *Def_term;
extern char ODME_MASTER_PATH[];

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

/***************************************************************/
/*                         MAIN                                */
/* DESCRIPTION : This routine is the main driver for the ODME. */
/*               It displays the initial menu and is the point */
/*               from which the editor is normally  exited.    */
/* INPUT :       object class name may be passed in.           */
/***************************************************************/
main (argc, argv)
int argc;
char *argv[];

{
   	void 	setup_curses    ();
   	void 	remwin          ();
   	void 	search_criteria ();
   	void object_display   	();
   	int  object_name_edit 	();
   	void quit_curses      	();
   	void rel_graph        	();
   	void control          	();
	void set_master_path  	();
	void save_char 	 	();
	int get_char	 	();

	struct objtoken         objhandle;
   	struct descriptor_data  *head_descriptor_data;
	char objs[256];

	WINDOW 	*box_window,        /* box window for the border   */
           	*key_window,        /* function key window         */
          	*menu_window;       /* function menu window        */

	char 	*mptr;

	int 	c,                  /* user input character        */
		cursor_line = 1,    /* cursor line position        */
		length      = 0,    /* storage for string lengths  */
		data_width  = 0,    /* length of hilite bar        */
		name_given  = FALSE; /* has an object class name been entered */
	int 	i;
	setlocale(LC_ALL, "");
	catd = catopen("odme.cat", NL_CAT_LOCALE);     /* Defect 116034 */
   	/*--------------------------------------------------------------*/
   	/* Initially set read only to false                             */
   	/* Initially set the insert on key to off so keys are replaced  */
   	/*    this is more optimum for curses performance on the object */
   	/*    display screen.                                           */
   	/* Initialize the curses environment                            */
   	/* Initialize the object data manager                           */
   	/*--------------------------------------------------------------*/
   	TRAC = 0;
   	REPLAY = 0;
   	DLY = 0;
   	strcpy (objhandle.objname, "                          ");
	strcpy (objhandle.objrep_path, getenv("ODMDIR"));
   	INSERTON = FALSE;

   	setup_curses ();

   	odm_initialize();

	strcpy (ODME_MASTER_PATH, "/etc/objrepos");

	/*--------------------------------------------------------------*/
	/* create a box window which contains the object editor name    */
	/* and a border.                                                */
	/*  ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿                 */
   	/*  ³    box window                           ³                 */
   	/*  ³ ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ» ³                 */
   	/*  ³ º            ODME Text                º ³                 */
   	/*  ³ ÌÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¹ ³                 */
   	/*  ³ º        ÚÄÄÄÄÄÄÄÄÄÄÄÄ¿               º ³                 */
   	/*  ³ º        ³menu window ³               º ³                 */
   	/*  ³ º        ÀÄÄÄÄÄÄÄÄÄÄÄÄÙ               º ³                 */
   	/*  ³ ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼ ³                 */
   	/*  ³   function keys window                  ³                 */
   	/*  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ                 */
   	/* ODME name text in green                                      */
   	/*--------------------------------------------------------------*/

   	box_window = newwin (LINES - 1, 0, 0, 0);
   	wcolorout(box_window, Bxa);
   	drawbox (box_window, 0, 0, 3, COLS);
   	drawbox (box_window, 2, 0, LINES -3, COLS);
   	wcolorend(box_window);

   	length = strlen (MSGS(MN,EDITOR_NAME,DF_EDITOR_NAME));
   	wmove    (box_window, 1, CENTER - (length/2));
   	waddstr  (box_window, MSGS(MN,EDITOR_NAME,DF_EDITOR_NAME));

	touchwin (box_window);


   	/*----------------------------------------------------------*/
   	/* Create a bottom key window 1 line deep and on the last   */
   	/* line of the screen.                                      */
   	/*----------------------------------------------------------*/
	key_window = newwin (1, 0, LINES - 1, 0);

	wmove     (key_window, 0, 0);
	waddstr   (key_window, MSGS(KS,MAIN_KEYS,DF_MAIN_KEYS));

	touchwin  (key_window);
	key_window->_csbp = NORMAL;
	wrefresh  (key_window);

	/*---------------------------------------------------------*/
	/* put up copyright legend for five seconds and continue   */
	/*---------------------------------------------------------*/
	wrefresh (box_window);
/*
	wmove (box_window, 3, 15);
	waddstr (box_window, 
		"(C) Copyright International Business Machines Corp. 1990");
	box_window->_csbp = NORMAL;
	wrefresh (box_window);
	wmove (box_window, 3, 15);
	waddstr (box_window,
		"                                                       ");
*/

	/*---------------------------------------------------------*/
	/* Create a center screen for the menu items               */
	/*---------------------------------------------------------*/
	menu_window = newwin (MAX_MAIN_ITEMS + 4, 30,
                         (LINES/2) - (MAX_MAIN_ITEMS/2), CENTER - (26/2));
	menu_window->_csbp = NORMAL;

	/*---------------------------------------------------------*/
	/* Draw box                                                */
	/*---------------------------------------------------------*/
	wcolorout(menu_window, Bxa);
	cbox (menu_window);
	wcolorend(menu_window);

	/*---------------------------------------------------------*/
	/* Display the main menu items                             */
	/*---------------------------------------------------------*/
   	wmove   (menu_window, 1, 2);
   	waddstr (menu_window, MSGS(MN,menu1," Set default object class") );
   	wmove   (menu_window, 2, 2);
   	waddstr (menu_window, MSGS(MN,menu2," Display relational graphs") );
   	wmove   (menu_window, 3, 2);
   	waddstr (menu_window, MSGS(MN,menu3," Create an object class") );
   	wmove   (menu_window, 4, 2);
   	waddstr (menu_window, MSGS(MN,menu4," Selective search") );
   	wmove   (menu_window, 5, 2);
   	waddstr (menu_window, MSGS(MN,menu5," Retrieve/Edit objects") );
   	wmove   (menu_window, 6, 2);
   	waddstr (menu_window, MSGS(MN,menu6," Object Class Management") );
   	wmove   (menu_window, 7, 2);
   	waddstr (menu_window, MSGS(MN,menu8," Delete an object class") );

	touchwin (menu_window);
	menu_window->_csbp = NORMAL;
	wrefresh (menu_window);

	/*-------------------------------------------------*/
	/* If a name has been passed in save it, set flag  */
	/*-------------------------------------------------*/
	if (argc > 1)
		if ((strcmp(argv[1], "trace")) == 0)
		{
			RD = fopen(argv[2], "w+");
			TRAC = 1;	
			wmove    (box_window, LINES - 4, 7);
			waddstr  (box_window, "TRACING FILE - ");
			wmove    (box_window, LINES - 4, 20);
			waddstr  (box_window, argv[2]);
			touchwin (box_window);
			box_window->_csbp = NORMAL;
			wrefresh (box_window);
		}
	else
		if ((strcmp(argv[1], "replay")) == 0)
		{
			RD = fopen(argv[2], "r");
			REPLAY = 1;	
			wmove    (box_window, LINES - 4, 5);
			waddstr  (box_window, "EXECUTING TEST - ");
			wmove    (box_window, LINES - 4, 20);
			waddstr  (box_window, argv[2]);
			touchwin (box_window);
			box_window->_csbp = NORMAL;
			wrefresh (box_window);
		}
	else
	{
                if (strstr( DISALLOWED, argv[1]) == NULL ) {
                        strcpy (objhandle.objname, argv[1]);
                        objhandle.lock = OBJ_LOCK_EX;
                        name_given = TRUE;
                }
                else {
                        help(MSGS(MN,disallowed_class,
"Class disallowed in odme.\n\
The selected class is disallowed within odme.\n\
The following classes cannot be edited with odme:\n\
        product           Config_Rules        CuVPD\n\
        sm_cmd_opt        sm_name_hdr\n\
        sm_cmd_hdr        sm_menu_opt\n") );

                        touchwin (box_window);
                        box_window->_csbp = NORMAL;
                        wrefresh (box_window);

                        touchwin (menu_window);
                        menu_window->_csbp = NORMAL;
                        wrefresh (menu_window);

                        touchwin (key_window);
                        key_window->_csbp = NORMAL;
                        wrefresh (key_window);

                }

	}

	if (argc > 3)
		DLY = 1;


	/*-------------------------------------------------*/
	/* flush the hft input buffer                      */
	/*-------------------------------------------------*/
	flushinp();
/*
	nodelay (TRUE);
	for (i = 0; i < 100; i++) 
		c = ms_input (menu_window);
	nodelay (FALSE);
*/
	
   	for (;;)
    	{
      	/*-----------------------------------------------------------------*/
      	/* set the highlight bars data width to match the text length of   */
      	/* of the functions. example "Global search" has a width of 13     */
      	/*-----------------------------------------------------------------*/
      	hilite (menu_window, menu_window, cursor_line, 2, 27,
              cursor_line, 2, rwhite, white);

	if (REPLAY)
	{
		c = get_char();
	}
	else
	{
		c = ms_input(menu_window);
		if (TRAC) save_char(c);
	}

      	/*------------------------------------------------------*/
      	/* The following if statements take input in two forms  */
      	/* The user can either select the first letter of the   */
      	/* function or if the cursor is sitting on the function */
      	/* he wants to use he can push return.                  */
      	/*------------------------------------------------------*/
      	/* if object class parameters are to be specified       */
      	/*------------------------------------------------------*/
      	if (  (((c == KEY_NEWL) || (c == KEY_NWL)) && (cursor_line == 1))   )
       	{

          	name_given = object_name_edit (&objhandle);

		box_window->_csbp = NORMAL;
          	touchwin (box_window);
          	wrefresh (box_window);

		menu_window->_csbp = NORMAL;
          	touchwin (menu_window);
          	wrefresh (menu_window);

          	touchwin (key_window);
		key_window->_csbp = NORMAL;
          	wrefresh (key_window);

        }

     	/*------------------------------------------------------*/
      	/* if Display relational graphs selected                */
      	/*------------------------------------------------------*/
      	else if ( (((c == KEY_NEWL) || (c == KEY_NWL)) && (cursor_line == 2))  )
       	{
	  	if (name_given == FALSE)
		{
 			name_given = object_name_edit (&objhandle);
		}
		if (name_given == FALSE)
		{
			box_window->_csbp = NORMAL;
          		touchwin (box_window);
          		wrefresh (box_window);

			menu_window->_csbp = NORMAL;
          		touchwin (menu_window);
          		wrefresh (menu_window);

          		touchwin (key_window);
			key_window->_csbp = NORMAL;
          		wrefresh (key_window);

			continue;
		}

          	rel_graph (&objhandle);
	  
          	touchwin (box_window);
		box_window->_csbp = NORMAL;
          	wrefresh (box_window);

          	touchwin (menu_window);
		menu_window->_csbp = NORMAL;
          	wrefresh (menu_window);

          	touchwin (key_window);
		key_window->_csbp = NORMAL;
          	wrefresh (key_window);
       	}

      	/*-------------------------------------------*/
      	/* else if create an object class selected   */
      	/*-------------------------------------------*/
      	else if ( (((c == KEY_NEWL) || (c == KEY_NWL)) && (cursor_line == 3)) )
      	{

		if (name_given == FALSE)
		{
 			name_given = object_name_edit (&objhandle);
		}

		if (name_given == FALSE)
		{
			box_window->_csbp = NORMAL;
          		touchwin (box_window);
          		wrefresh (box_window);

			menu_window->_csbp = NORMAL;
          		touchwin (menu_window);
          		wrefresh (menu_window);

          		touchwin (key_window);
			key_window->_csbp = NORMAL;
          		wrefresh (key_window);

			continue;
		}

		if ((int)odm_mount_class(objhandle.objname) > 0)
		{
			prerr (MSGS(ER,OBJECT_EXISTS,DF_OBJECT_EXISTS),0,FALSE);
		}
		else
		{
	     		class_create (&objhandle);
		}

		touchwin (box_window);
		box_window->_csbp = NORMAL;
		wrefresh (box_window);

		touchwin (menu_window);
		menu_window->_csbp = NORMAL;
		wrefresh (menu_window);

		touchwin (key_window);
		key_window->_csbp = NORMAL;
		wrefresh (key_window);
       	}

      	/*-----------------------------------------*/
      	/* else if search by criteria pressed      */
      	/*-----------------------------------------*/
      	else if ( (((c == KEY_NEWL) || (c == KEY_NWL)) && (cursor_line == 4))  )
       	{
	  	/*------------------------------------------------*/
	  	/* check for an object class name. if not present */
	  	/* prompt for one                                 */
	  	/*------------------------------------------------*/
	  	if (name_given == FALSE)
			name_given = object_name_edit (&objhandle);

		if (name_given == FALSE)
		{
			box_window->_csbp = NORMAL;
          		touchwin (box_window);
          		wrefresh (box_window);

			menu_window->_csbp = NORMAL;
          		touchwin (menu_window);
          		wrefresh (menu_window);

          		touchwin (key_window);
			key_window->_csbp = NORMAL;
          		wrefresh (key_window);

			continue;
		}

          	if (open_object_class (&objhandle) > 0)
           	{
              	/*-------------------------------------------------*/
              	/* Get the object class descriptors                */
              	/* Go to search criteria entry; from there we will */
              	/*    go to display object class                   */
              	/* Free up the descriptors                         */
              	/*-------------------------------------------------*/
              	get_descriptors (&objhandle, &head_descriptor_data);
              	search_criteria (&objhandle, head_descriptor_data);

              	free ((struct descriptor_data *) head_descriptor_data);
           	}

 	     	touchwin (box_window);
		box_window->_csbp = NORMAL;
     	     	wrefresh (box_window);

 	     	touchwin (menu_window);
		menu_window->_csbp = NORMAL;
     	     	wrefresh (menu_window);

 	     	touchwin (key_window);
		key_window->_csbp = NORMAL;
     	     	wrefresh (key_window);
       	}
      	/*---------------------------------------------*/
      	/* else if Retrieve/Edit is selected           */
      	/*---------------------------------------------*/
      	else if ( (((c == KEY_NEWL) || (c == KEY_NWL)) && (cursor_line == 5))  )
       	{ 
	  	/*------------------------------------------------*/
 	  	/* check for an object class name. if not present */
	  	/* prompt for one.                                */
	  	/*------------------------------------------------*/
	  	if (name_given == FALSE)
			name_given = object_name_edit (&objhandle);

		if (name_given == FALSE)
		{
			box_window->_csbp = NORMAL;
          		touchwin (box_window);
          		wrefresh (box_window);

			menu_window->_csbp = NORMAL;
          		touchwin (menu_window);
          		wrefresh (menu_window);

          		touchwin (key_window);
			key_window->_csbp = NORMAL;
          		wrefresh (key_window);

			continue;
		}

          	if (open_object_class (&objhandle) > 0)
           	{
             		/*--------------------------------------------*/
             		/*  Get the descriptors and descriptor data   */
             		/*  Set the search criteria string to NULL    */
             		/*  Display all the objects                   */
             		/*  Free up the descriptors                   */
             		/*--------------------------------------------*/
             		get_descriptors (&objhandle, &head_descriptor_data);
             		objs[0] = '\0';
             		object_display (&objhandle, objs, head_descriptor_data);

             		free ((struct descriptor_data *) head_descriptor_data);
           	}

 	     	touchwin (box_window);
		box_window->_csbp = NORMAL;
     	     	wrefresh (box_window);

 	     	touchwin (menu_window);
		menu_window->_csbp = NORMAL;
     	     	wrefresh (menu_window);

 	     	touchwin (key_window);
		key_window->_csbp = NORMAL;
     	     	wrefresh (key_window);

       	}
      	/*--------------------------------------------*/
      	/*  else if help selected                     */
      	/*--------------------------------------------*/
      	else if (  (c == ESC1) || (c == KEY_F(1))  )
       	{

		help(MSGS(MN,main_help, 
           "Main menu help\n\
 The Main menu screen allows you to perform functions by\n\
 high-lighting the function using the arrow keys and    \n\
 pressing RETURN.                                       \n\
 - Display relational graphs gives a graphical overview \n\
   of how each object class relates to another.         \n\
 - Create allows you to create an object class.         \n\
 - Selective search allows you to enter search criteria \n\
   for retrieving only the objects you want.            \n\
 - Retrieve/Edit returns all the objects in the class.  \n\
 - Object Class Management allows you to specify list of\n\
   object classes.                                      \n\
 - Delete allows entire object classes to be removed.   \n\
") );


 	     	touchwin (box_window);
		box_window->_csbp = NORMAL;
     	     	wrefresh (box_window);

 	     	touchwin (menu_window);
		menu_window->_csbp = NORMAL;
     	     	wrefresh (menu_window);

 	     	touchwin (key_window);
		key_window->_csbp = NORMAL;
     	     	wrefresh (key_window);
       	}
       	/*---------------------------------------------*/
       	/*  else if delete an object class is selected */
       	/*---------------------------------------------*/
       	else if ( (((c == KEY_NEWL) || (c == KEY_NWL)) && (cursor_line == 7)) )
       	{

		name_given = object_name_edit (&objhandle);

		if (name_given == FALSE)
		{
 	     		touchwin (box_window);
			box_window->_csbp = NORMAL;
     	     		wrefresh (box_window);

 	     		touchwin (menu_window);
			menu_window->_csbp = NORMAL;
     	     		wrefresh (menu_window);

 	     		touchwin (key_window);
			key_window->_csbp = NORMAL;
     	     		wrefresh (key_window);

			continue;
		}

		if (question (MSGS(MN,delete_warning,
		"WARNING - all objects will be lost. Continue?") ) == TRUE)
		{

 	     		touchwin (box_window);
			box_window->_csbp = NORMAL;
     	     		wrefresh (box_window);

 	     		touchwin (menu_window);
			menu_window->_csbp = NORMAL;
     	     		wrefresh (menu_window);

 	     		touchwin (key_window);
			key_window->_csbp = NORMAL;
     	     		wrefresh (key_window);
			objhandle.objptr =
			 (struct Class *) odm_mount_class(objhandle.objname);
		            
			if (odm_rm_class(objhandle.objptr) == 0)
			{
				help (MSGS(MN,delete_ok,
"OBJECT CLASS DELETE \n\
*** SUCCESSFUL *** \n") );
			}
			else
			{
				prerr (MSGS(ER,DELETE_FAILED,
				  "OBJECT CLASS DELETE - FAILED -"),0, FALSE);
			}

		}

 	     	touchwin (box_window);
		box_window->_csbp = NORMAL;
     	     	wrefresh (box_window);

 	     	touchwin (menu_window);
		menu_window->_csbp = NORMAL;
     	     	wrefresh (menu_window);

 	     	touchwin (key_window);
		key_window->_csbp = NORMAL;
     	     	wrefresh (key_window);
	}
 
      	/*--------------------------------*/
      	/* else if exit selected          */
      	/*--------------------------------*/
      	else if (  (c == ESC3) || (c == KEY_F(3))  )
       	{
         	remwin (menu_window, white);
         	remwin (box_window, white);
         	remwin (key_window, white);

         	quit_curses ();
       	}

      	/*--------------------------------*/
      	/* cursor key up selected         */
      	/*--------------------------------*/
      	else if (  (c == KEY_UP) || (c == CTRL_P)  )
       	{
         	if (cursor_line == 1)
            		cursor_line = MAX_MAIN_ITEMS;
         	else
            		cursor_line--;
       	}

	/*--------------CONTROL--------------*/
	else if ( (((c == KEY_NEWL) || (c == KEY_NWL)) && (cursor_line == 6)) )
	{
		control ();

 	     	touchwin (box_window);
		box_window->_csbp = NORMAL;
     	     	wrefresh (box_window);

 	     	touchwin (menu_window);
		menu_window->_csbp = NORMAL;
     	     	wrefresh (menu_window);

 	     	touchwin (key_window);
		key_window->_csbp = NORMAL;
     	     	wrefresh (key_window);
	}
 
      	/*--------------------------------*/
      	/* cursor key down selected       */
      	/*--------------------------------*/
      	else if ( (c == KEY_DOWN) || (c == CTRL_N) )
       	{
         	if (cursor_line == MAX_MAIN_ITEMS)
             		cursor_line = 1;
         	else
             		cursor_line++;
       	}

      	/*--------------------------------*/
      	/* beep on some other key         */
      	/*--------------------------------*/
      	else
         	beep ();
 		
			
    } /* end for */
} /* end main */



/**********************************************************************/
/* object_name_edit : allows the user to specify object name, node    */
/*                    and update purposes                             */
/*       PARAMETERS : objhandle - the object class structure          */
/**********************************************************************/
int object_name_edit (objhandle)

struct objtoken *objhandle;
{
   void new_panel ();
   void remwin    ();

   WINDOW 	*inner_panel,
          	*key_window,
          	*outer_panel,
		*name_window,
		*name_view,
		*path_window,
		*path_view,
		*window_ptr;

   int 		c,
       		i,
       		cursor_line   = 0,
       		cursor_column = 0,
       		tmp_cur_line = 0,
       		data_width = MAX_CLASS_NAME,
       		name_given = FALSE,
       		length,
    		name_width = 20,
		path_width = 20,
		name_mark = 20,
		path_mark = 20,
		name_size = MAX_CLASS_NAME,
		path_size = MAX_CLASS_PATH;

   char tmpclassname[MAX_CLASS_NAME];

   strcpy (objhandle->node, "          ");

   length = 20;

   new_panel (4, length + 20, LINES - 20, CENTER - ((length + 20)/2),
		MSGS(MN,objclpar,
              "Object Class Parameters"), &outer_panel, &inner_panel);
	outer_panel->_csbp = NORMAL;
	inner_panel->_csbp = NORMAL;

	wmove   (inner_panel, 0, 0);
	waddstr (inner_panel, MSGS(MN,objclname,
	"Object Class name                       ") );

	name_window = newwin (1, name_size - 1, LINES - 17,           
			CENTER - ((length + 20)/2) + 20 );
	name_window->_csbp = NORMAL;
	wmove   (name_window, 0, 0);
	waddstr (name_window, objhandle->objname);

	wmove (name_window, 0, 0);
	name_view = newview (name_window, 1, name_width);
	
	/* wmove   (inner_panel, 1, 0);
	wprintw (inner_panel, MSGS(MN, node_msg,
	"             Node [%-*.*s]"), MAX_OBJ_NODE - 1,
            MAX_OBJ_NODE - 1, objhandle->node);  SMU Fix for 22812 */

	wmove   (inner_panel, 2, 0);
	waddstr (inner_panel, MSGS(MN,path_msg,
	"             Path                       ") );

	path_window = newwin (1, path_size - 1, LINES - 15,           
			CENTER - ((length + 20)/2) + 20 );
	wmove   (path_window, 0, 0);
	werase(path_window);
	wmove (path_window, 0, 0);
	waddstr (path_window, objhandle->objrep_path);
	wmove (path_window, 0, 0);
	path_view = newview (path_window, 1, path_width);
	wmove (path_view, 0, 0);

   wmove   (inner_panel, 3, 0);
   if (objhandle->lock == OBJ_LOCK_NB)
    {
      waddstr (inner_panel, MSGS(MN,openro,
	"          Open as [Read only] ") );
      READONLY = TRUE;
    }
   else
    {
      objhandle->lock = OBJ_LOCK_EX;
      waddstr (inner_panel, MSGS(MN,openrw,
	"          Open as [Read/Write]") );
      READONLY = FALSE;
    }

   touchwin (inner_panel);
	inner_panel->_csbp = NORMAL;
   wrefresh (inner_panel);

	touchwin (name_view);
	name_view->_csbp = NORMAL;
	wrefresh (name_view);

	touchwin (path_view);
	path_view->_csbp = NORMAL;
	wrefresh (path_view);

   /*----------------------------------------------------------*/
   /* Create a bottom key window 1 line deep and on the last   */
   /* line of the screen.                                      */
   /*----------------------------------------------------------*/
   key_window = newwin (1, 0, LINES - 1, 0);
	key_window->_csbp = NORMAL;

   wmove     (key_window, 0, 0);
   waddstr   (key_window, MSGS(KS,PANEL_KEYS,DF_PANEL_KEYS) );

   touchwin  (key_window);
	key_window->_csbp = NORMAL;
   wrefresh  (key_window);


   for (;;)
    {

      /*-----------------------------------------------------------------*/
      /* set the highlight bars data width to match the text length of   */
      /* of the functions.                                               */
      /*-----------------------------------------------------------------*/
      if (cursor_line == 0)
      {
          	data_width = name_size;
		hilite (name_window, name_view, 0, 0, data_width,
			0, cursor_column - (name_mark - name_width),                                    rwhite, white);
		name_view->_csbp = NORMAL;
		wrefresh (name_view);
      }
      else if (cursor_line == 1)
      {
          	data_width = MAX_OBJ_NODE - 1;
		hilite (inner_panel, inner_panel, cursor_line, 19,
			data_width, cursor_line, cursor_column, rwhite, 				white); 
		touchwin (name_view);
		name_view->_csbp = NORMAL;
		wrefresh (name_view);
		touchwin (path_view);
		path_view->_csbp = NORMAL;
		wrefresh (path_view);
       }
       else if (cursor_line == 2)
       {
		data_width = path_size;
		hilite (path_window, path_view, 0, 0, data_width,
			0, cursor_column - (path_mark - path_width), rwhite, 				white);
		path_view->_csbp = NORMAL;
		wrefresh (path_view);
       }
       else if (cursor_line == 3)
       { 
          	data_width = 1;
		 hilite (inner_panel, inner_panel, cursor_line, 19, data_width,
				cursor_line, cursor_column, rwhite, white);
		touchwin (name_view);
		name_view->_csbp = NORMAL;
		wrefresh (name_view);
		touchwin (path_view);
		path_view->_csbp = NORMAL;
		wrefresh (path_view);
        }


/*
      hilite (window_ptr, window_ptr, 0, 0, data_width,
              cursor_line, cursor_column, rwhite, white);
*/

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
        	case KEY_NEWL :
	  	case KEY_NWL  :
          	case ESC1     :
          	case KEY_F(1) :
                input_field (name_window, 0, 0, name_size - 1, tmpclassname);
                if ( strstr( DISALLOWED, tmpclassname ) == NULL ) {
                        strcpy(objhandle->objname, tmpclassname);
                }
                else {
                        help(MSGS(MN,disallowed_class,
"Class disallowed in odme.\n\
The selected class is disallowed within odme.\n\
The following classes cannot be edited with odme:\n\
        product           Config_Rules        CuVPD\n\
        sm_cmd_opt        sm_name_hdr\n\
        sm_cmd_hdr        sm_menu_opt\n") );

                }

		/*------------------------------------------------------*/
		/* if a name has been entered, set name_given to TRUE   */
		/* otherwise set it to false                            */
		/*------------------------------------------------------*/
			for (i=0; i < name_size - 1; i++)
			{
				wmove (name_window, 0, 0);
				if (winch (name_window) != ' ')
					name_given = TRUE;
			}
	 	
                	input_field (inner_panel, 1, 19, MAX_OBJ_NODE - 1,
                            		objhandle->node);
		 	input_field (path_window, 0, 0, path_size - 1,
					objhandle->objrep_path);
			if (objhandle->objrep_path[0] != '\0')
					odm_set_path(objhandle->objrep_path);

                	remwin (key_window,  white);
                	remwin (inner_panel, white);
                	remwin (outer_panel, white);

                	return(name_given);

          	case ESC3     :
          	case KEY_F(3) :
                	remwin (key_window,  white);
                	remwin (inner_panel, white);
                	remwin (outer_panel, white);

                	return(name_given);

          	case KEY_UP :
          	case CTRL_P :
                	if (cursor_line == 0)
                  		cursor_line = 3;
                	else
                  		cursor_line--;

			if (cursor_line == 1)
			{
				cursor_line--;
				cursor_column = 0;
				inner_panel->_csbp = NORMAL;
				wrefresh (inner_panel);
				/* cursor_column = 19; */
			}
			else
			{
				cursor_column = 0;
				inner_panel->_csbp = NORMAL;
				wrefresh (inner_panel);
			}

			vscroll (name_view, 0, -name_size);
			vscroll (path_view, 0, -path_size);
			path_mark = 20;
			name_mark = 20;
                	break;

           	case KEY_DOWN :
           	case CTRL_N   :
                 	if (cursor_line == 3)
                    		cursor_line = 0;
                 	else
                     		cursor_line++;

			if (cursor_line == 1)
			{
				cursor_line++;
				cursor_column = 0;
				inner_panel->_csbp = NORMAL;
				wrefresh (inner_panel);
				/* cursor_column = 19; */
			}
			else
			{
				cursor_column = 0;
				inner_panel->_csbp = NORMAL;
				wrefresh (inner_panel);
			}

			vscroll (name_view, 0, -name_size);
			vscroll (path_view, 0, -path_size);
			path_mark = 20;
			name_mark = 20;
                 	break;

           	default :
                 	if (cursor_line == 3)
                  	{
                    		if (objhandle->lock == OBJ_LOCK_NB)
                     		{
                        		objhandle->lock = OBJ_LOCK_EX;
                        		wmove   (inner_panel, 3, 0);
                        		waddstr (inner_panel, MSGS(MN,openrw,
					"          Open as [Read/Write]") );
                        		READONLY = FALSE;
                     		}
                    		else
                     		{
                        		objhandle->lock = OBJ_LOCK_NB;
                        		wmove (inner_panel, 3, 0);
                        		waddstr (inner_panel, MSGS(MN,openro,
					"          Open as [Read only] ") );
                        		READONLY = TRUE;
                     		}
                  	}
                 	else
			{
				switch (cursor_line)
				{
					case 0 :
                    			std_edit_keys (name_window, c, 0, 							data_width - 1,
                                   		&cursor_column, &tmp_cur_line);
					if (cursor_column > name_mark - 1)
					{
						vscroll (name_view, 0, 1);
						name_mark++;
					}
					if (cursor_column < 									name_mark - name_width)
					{
						vscroll (name_view, 0, -1);
						name_mark--;
					} 
					break;

					case 1 :
                    				std_edit_keys (inner_panel, c, 								19, data_width - 1,
                                   			&cursor_column, 								&cursor_line);
						break;

					case 2 :
                    				std_edit_keys (path_window, c, 								0, data_width - 1,
                                   			&cursor_column, 								&tmp_cur_line);

						if (cursor_column > 									path_mark - 1)
						{
							vscroll(path_view,0,1);
							path_mark++;
						}

						if (cursor_column < 									path_mark - path_width)
						{
							vscroll(path_view,0,-1);
							path_mark--;
						} 

						break;
					} /* end switch */
				break;
				}

       			} /* end switch */
    		} /* end for */
} /* end object_name_edit */


/*************************************************************/
/* Routine: save_char                                        */
/* Description: saves input characters if the trace command  */
/*              has been issued.                             */
/*************************************************************/
void save_char(c)
int c;
{
switch (c)
	{
	case KEY_NEWL	:
		c = 1;
		break;
	case KEY_UP	:
		c = 2;
		break;
	case KEY_DOWN	:
		c = 3;
		break;
	case KEY_NPAGE	:
		c = 4;
		break;
	case KEY_SF	:
		c = 5;
		break;
	case KEY_PPAGE	:
		c = 6;
		break;
	case KEY_SR	:
		c = 7;
		break;
	case KEY_BTAB	:
		c = 8;
		break;
	case KEY_TAB	:
		c = 9;
		break;
	case KEY_END	:
		c = 10;
		break;
	case KEY_HOME	:
		c = 11;
		break;
	case KEY_LEFT	:
		c = 125;
		break;
	case KEY_RIGHT	:
		c = 126;
		break;
	case KEY_BACKSPACE	:
		c = 127;
		break;
	case KEY_DC	:
		c = 128;
		break;
	case KEY_EOL	:
		c = 129;
		break;
	case CTRL_A	:
		c = 12;
		break;
	case CTRL_B	:
		c = 13;
		break;
	case CTRL_D	:
		c = 14;
		break;
	case CTRL_E	:
		c = 15;
		break;
	case CTRL_F	:
		c = 16;
		break;
	case CTRL_K	:
		c = 17;
		break;
	case CTRL_N	:
		c = 18;
		break;
	case CTRL_O	:
		c = 19;
		break;
	case CTRL_P	:
		c = 20;
		break;
	case INSERT	:
 		c = 21;
		break;
	case KEY_NWL	:
		c = 22;
		break;
	case ESC0	:
		c = 30;
		break;
	case ESC1	:
		c = 31;
		break;
	case ESC2	:
		c = 132;
		break;
	case ESC3	:
		c = 33;
		break;
	case ESC4	:
		c = 34;
		break;
	case ESC5	:
		c = 35;
		break;
	case ESC6	:
		c = 36;
		break;
	case ESC7	:
		c = 37;
		break;
	case ESC8	:
		c = 38;
		break;
	case ESC9	:
		c = 29;
		break;
	case KEY_F(1)	:
		c = 41;
		break;
	case KEY_F(2)	:
		c = 42;
		break;
	case KEY_F(3)	:
		c = 43;
		break;
	case KEY_F(4)	:
		c = 44;
		break;
	case KEY_F(5)	:
		c = 45;
		break;
	case KEY_F(6)	:
		c = 46;
		break;
	case KEY_F(7)	:
		c = 28;
		break;
	case KEY_F(8)	:
		c = 23;
		break;
	case KEY_F(9)	:
		c = 24;
		break;
	case KEY_F(10)	:
		c = 25;
		break;
	case KEY_F(11)	:
		c = 26;
		break;
	case KEY_F(12)	:
		c = 27;
		break;
	default :
		break;
	} /* end switch */

	putc (c, RD);
}	



/*************************************************************/
/* Routine: get_char                                         */
/* Description: gets  input characters if the trace command  */
/*              has been issued.                             */
/*************************************************************/
int get_char ()
{
	long 	i,
		k,
		j;
	int 	c;

	c = getc (RD);

switch (c)
	{
		case 1	:
			c = KEY_NEWL;
			break;
		case 2	:
			c = KEY_UP	 ;
			break;
		case 3	:
			c = KEY_DOWN	 ;
			break;
		case 4	:
			c = KEY_NPAGE	 ;
			break;
		case 5	:
			c = KEY_SF	 ;
			break;
		case 6	:
			c = KEY_PPAGE	 ;
			break;
		case 7	:
			c = KEY_SR	 ;
			break;
		case 8	:
			c = KEY_BTAB	 ;
			break;
		case 9	:
			c = KEY_TAB	 ;
			break;
		case 10	:
			c = KEY_END	 ;
			break;
		case 11	:
			c = KEY_HOME	 ;
			break;
		case 125:
			c = KEY_LEFT;
			break;
		case 126:
			c = KEY_RIGHT;
			break;
		case 127:
			c = KEY_BACKSPACE;
			break;
		case 128:
			c = KEY_DC;
			break;
		case 129:
			c = KEY_EOL;
			break;
		case 12	:
			c = CTRL_A	 ;
			break;
		case 13	:
			c = CTRL_B	 ;
			break;
		case 14	:
			c = CTRL_D	 ;
			break;
		case 15	:
			c = CTRL_E	 ;
			break;
		case 16	:
			c = CTRL_F	 ;
			break;
		case 17	:
			c = CTRL_K	 ;
			break;
		case 18	:
			c = CTRL_N	 ;
			break;
		case 19	:
			c = CTRL_O	 ;
			break;
		case 20	:
			c = CTRL_P	 ;
			break;
		case 21	:
			c = INSERT	 ;
			break;
		case 22 :
			c = KEY_NWL      ;
			break;
		case 30	:
			c = ESC0	 ;
			break;
		case 31	:
			c = ESC1	 ;
			break;
		case 132	:
			c = ESC2	 ;
			break;
		case 33	:
			c = ESC3	 ;
			break;
		case 34	:
			c = ESC4	 ;
			break;
		case 35	:
			c = ESC5	 ;
			break;
		case 36	:
			c = ESC6	 ;
			break;
		case 37	:
			c = ESC7	 ;
			break;
		case 38	:
			c = ESC8	 ;
			break;
		case 29	:
			c = ESC9	 ;
			break;
		case 41	:
			c = KEY_F(1)	 ;
			break;
		case 42	:
			c = KEY_F(2)	 ;
			break;
		case 43	:
			c = KEY_F(3)	 ;
			break;
		case 44	:
			c = KEY_F(4)	 ;
			break;
		case 45	:
			c = KEY_F(5)	 ;
			break;
		case 46	:
			c = KEY_F(6)	 ;
			break;
		case 28	:
			c = KEY_F(7)	 ;
			break;
		case 23	:
			c = KEY_F(8)	 ;
			break;
		case 24	:
			c = KEY_F(9)	 ;
			break;
		case 25	:
			c = KEY_F(10)	 ;
			break;
		case 26	:
			c = KEY_F(11)	 ;
			break;
		case 27	:
			c = KEY_F(12)	 ;
			break;
		default  :
			break;
	} /* end switch */
	if (DLY)
		{
		for (j = 0; j < 3500000; j++)
			{
			}
		}

/*	if (DLY) sleep(1); */ 
	return (c);
}	
