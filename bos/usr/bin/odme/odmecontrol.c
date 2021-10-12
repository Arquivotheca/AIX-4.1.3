static char sccsid[] = "@(#)42  1.15  src/bos/usr/bin/odme/odmecontrol.c, cmdodm, bos411, 9428A410j 1/14/93 17:40:06";
/*
 * COMPONENT_NAME: (ODME) ODMECONTROL - file managment 
 *
 * FUNCTIONS: control, disp_class, get_func
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

/*******************************************************************/
/*  FILE NAME :     ODMECONTROL.C	                           */
/*  DESCRIPTION:    This file contains the Object Data Manager     */
/*                  Editor object class control routines.          */
/*  CONTENTS :		  	                                   */
/* 		    Get_class get object class info.               */
/*******************************************************************/


#include "odme.h"


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

/***************************************************************/
/*  control                                                    */
/*  Parameters  : objhandle - the object class handle	       */
/***************************************************************/
	int links_per_screen = 6;  /* number of links displayed per screen */

	char *contfuncs[6];

/*--------------------------------------------------------------*/
/* this structure is for reading the /etc/objrepos directory    */
/*--------------------------------------------------------------*/
struct odmedir {
	ino_t ino;
	char dirdata[14];
	}odmedir, *dp;

control ()
{

   void get_descriptors();
   void disp_my_desc();
   void disp_class();
   int get_func();
   int fp;	/* file pointer */
      
   struct descriptor_data  *head_descriptor_data,
		       	   *ptr_descriptor_data;

   char objs[256];
   struct objtoken objclass;

   WINDOW *graph_wnd,            /* main graph display window           */
	  *wind_info[MAX_LINKS], /* array of windows for object classes */
          *key_window;           /* function key window                 */

   int c,                                 /* user input character       */
       i,
       j,
       link_ptr,                          /* ptr used for cursor movement  */
       cursor_line    = 0,                /* cursor line position          */
       length         = 0,                /* storage for string lengths    */
       offset         = LINK_WND_START,   /* spacing for first link window */ 
       connector_row,
       connector_col,
       data_width     = 0,                /* length of hilite bar             */
       screen_num     = 0,                /* current active screen of links   */
       total_screens,	                  /* total number of screens of links */
       came_from;            /* flag passed to disp_class, holds up,down,dont */
 
   int link_names_ptr;                  /* ptr into link_names       */
   char *path_names[MAX_LINKS];         /* list of node names        */
   int *class_perm[MAX_LINKS];          /* list of class permissions */
   char *link_names[MAX_LINKS];         /* list of object classes    */
   char working_class[MAX_DESC_NAME];   /* current object class being viewed */

   int selection = -99;
   int contfnum = 5;                    /* number of functions for control */

   contfuncs[0] = "  ";
   contfuncs[1] = MSGS(CT,cfunc1,"Link Graph");
   contfuncs[2] = MSGS(CT,cfunc2,"Selective Search");
   contfuncs[3] = MSGS(CT,cfunc3,"Retrieve/Edit");
   contfuncs[4] = MSGS(CT,cfunc4,"Delete");
   contfuncs[5] = MSGS(CT,cfunc5,"Return");

   /*--------------------------------------------------------------*/
   /* create a box window which contains the relational graph name */
   /* and a border.                                                */
   /*  ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿                 */
   /*  ³    graph_wnd                            ³                 */
   /*  ³ ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ» ³                 */
   /*  ³ º            ODME Text                º ³                 */
   /*  ³ ÌÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¹ ³                 */
   /*  ³ º                                     º ³                 */
   /*  ³ º                                     º ³                 */
   /*  ³ º                                     º ³                 */
   /*  ³ ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼ ³                 */
   /*  ³   function keys window                  ³                 */
   /*  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ                 */
   /*--------------------------------------------------------------*/

   graph_wnd = newwin (LINES - 1, 0, 0, 0);

   wcolorout(graph_wnd, Bxa);
   drawbox (graph_wnd, 0, 0, 3, COLS);
   drawbox (graph_wnd, 2, 0, LINES - 3, COLS);
   wcolorend(graph_wnd);

   length = strlen (MSGS(CT,CONT_NAME,DF_CONT_NAME) );
   wmove    (graph_wnd, 1, CENTER - (length/2));
   waddstr  (graph_wnd, MSGS(CT,CONT_NAME,DF_CONT_NAME) );

   touchwin (graph_wnd);
   graph_wnd->_csbp = NORMAL;
   wrefresh (graph_wnd);

   /*----------------------------------------------------------*/
   /* Create a bottom key window 1 line deep and on the last   */
   /* line of the screen.                                      */
   /*----------------------------------------------------------*/
   key_window = newwin (1, 0, LINES - 1, 0);
   wmove     (key_window, 0, 5);
   waddstr   (key_window, MSGS(KS,STD_KEYS,DF_STD_KEYS) );

	/*------------------------------------*/
	/* malloc storage for object class    */
	/* names path names and permissions	  */
	/*------------------------------------*/
	for (i = 0; i < MAX_LINKS; i++)
		{
		if (((link_names[i] = (char *) malloc (MAX_DESC_NAME)) == NULL) ||
		     ((path_names[i] = (char *) malloc (MAX_OBJ_NODE)) == NULL) ||
		      ((class_perm[i] = (int *) malloc (5))== NULL) )
			{
			for (j = 0; j < i; j++)
				{
				free (link_names[j]);
				free (path_names[j]);
				free (class_perm[j]);
				}
			}
		/*----------------------------------*/
		/* initialize all data areas        */
		/*----------------------------------*/
		*link_names[i] = '\0';
		*path_names[i] = '\0';
		*class_perm[i] = OBJ_LOCK_EX;

		} /* end for */
	
   /*----------------------------------------------*/
	/*----------------------------------------------*/

		/*------------------------------------*/
		/* get the list of object classes     */
		/*------------------------------------*/
		link_names_ptr = 0;
		strcpy (objclass.objname, "");
		strcpy (objclass.objrep_path, ""); /* SMU Defect 36537 */
		while ( (object_name_edit(&objclass) == TRUE) )
			{
			graph_wnd->_csbp = NORMAL;
			wrefresh (graph_wnd);
			strcpy(link_names[link_names_ptr],objclass.objname);
			strcpy(path_names[link_names_ptr],objclass.objrep_path);
	   	        *(class_perm[link_names_ptr]) = objclass.lock; 

			/*----------------------------------------------*/
			/* create a window for this object class        */
			/*----------------------------------------------*/
			wind_info[link_names_ptr] = newwin(3,MAX_DESC_NAME, offset, 10);
			offset +=3;
			if (offset == MAX_LINK_POS)
				offset = LINK_WND_START;
			link_names_ptr++;
			strcpy(objclass.objname, "");
			}	
	/*---------------------------------------------------*/
	/* if no names were entered, free memory and return  */
	/*---------------------------------------------------*/
	if (link_names_ptr == 0)
		{
			for (i = 0; i < link_names_ptr; i++)
				{
				delwin(wind_info[i]);
				free (link_names[i]);
				free (path_names[i]);
				free (class_perm[i]);
				}

			/*--------------------*/
			/* delete all windows */  
			/*--------------------*/

			delwin(graph_wnd);
			remwin(key_window, white);

			return;
		}
	
			touchwin (graph_wnd);
			graph_wnd->_csbp = NORMAL;
			wrefresh (graph_wnd);
		/*-----------------------------------------------------*/
		/* if no object class names were entered then set      */
		/* link_names_ptr to -99 to indicate no links.         */
		/*-----------------------------------------------------*/
		if (link_names_ptr == 0)
			{
			link_names_ptr = -99;
			}
		/*-----------------------------------------------------*/
		/* calculate the total number of screens.              */
		/*-----------------------------------------------------*/
		if (link_names_ptr != -99)
			{
			total_screens = (link_names_ptr - 1)/links_per_screen;
			}
			else
			{
			total_screens = 0;
			}

	/*----------------------------------------*/
	/* display the main object class and all  */
	/* of its links. 			  */
	/*----------------------------------------*/
	came_from = DONT;
	disp_class(screen_num, link_names, link_names_ptr,
					wind_info, graph_wnd,came_from);
 
	/*-----------------------------------------*/
	/* display the key_window information      */
	/*-----------------------------------------*/
   touchwin  (key_window);
	key_window->_csbp = NORMAL;
   wrefresh  (key_window);
	/*-----------------------------------------*/
	/* USER INPUT BEGINS								 */
	/*-----------------------------------------*/
	link_ptr = 0;

	/*-----------------------------------------*/
	/* flush the hft input buffer              */
	/*-----------------------------------------*/
	flushinp();

	for (;;)
	{
		for (i = screen_num * links_per_screen ;(i < (screen_num *                           links_per_screen ) + links_per_screen ); i++)
			{
			if (i == (link_names_ptr) || link_names_ptr == -99)
				break;
			touchwin(wind_info[i]);
			wind_info[i]->_csbp = NORMAL;
			wrefresh(wind_info[i]);
			}

			wmove (graph_wnd, LINES - 3, COLS - 20);
			wprintw (graph_wnd,MSGS(CT,screen_msg,
				 "Screen %-3d of  %-3d"), screen_num + 1, total_screens + 1);
			graph_wnd->_csbp = NORMAL;
			wrefresh(graph_wnd);

		/*---------------------------------------*/
		/* highlight the current window          */
		/*---------------------------------------*/
		if (link_names_ptr != -99)
			hilite (wind_info[link_ptr], wind_info[link_ptr], 1, 1,
				MAX_DESC_NAME - 2,1, 0, rwhite, white);


	if (REPLAY)
		{
		c = get_char();
		}
	else
		{
	c = ms_input(graph_wnd);
	if (TRAC) save_char(c);
		}
		switch (c)
			{
			/*-----------------HELP KEY PRESSED-------------*/
			case KEY_F(1) :
			case ESC1     :
				help(MSGS(CT,control_help,
					"Object Class Management help \n\
 This function allows you to enter the       \n\
 names of multiple object classes to         \n\
 manipulate, or retrieve them from the       \n\
 default directory.                          \n\
 After entering the names of the object      \n\
 classes you wish to work with, press Enter  \n\
 without entering an object class name to    \n\
 continue. Select the functions key          \n\
 as described at the bottom of the           \n\
 screen to select the desired function       \n\
 to perform on the highlighted object class. \n\
") );

					touchwin(graph_wnd);
					graph_wnd->_csbp = NORMAL;
					wrefresh(graph_wnd);
					touchwin(key_window);
					key_window->_csbp = NORMAL;
					wrefresh(key_window); 							
				break;
	
			/*-----------------CURSOR DOWN--------------*/

			case KEY_DOWN :
			case CTRL_N   :
				if (link_names_ptr == -99)
					break;
				link_ptr++;
				if (link_ptr == ((screen_num * links_per_screen)+links_per_screen) || 
							(link_ptr == link_names_ptr)  )
					{
					if ( (screen_num ) < total_screens)
						{
						came_from = DOWN;
						screen_num++;
						disp_class(screen_num, link_names,
							link_names_ptr, wind_info, graph_wnd,came_from);
						}
						else
						{
						link_ptr--;
						beep ();
						}
					}
				break;

			/*----------------CURSOR UP----------------*/

			case KEY_UP :
			case CTRL_P :
				if (link_names_ptr == -99) 
					break;
				link_ptr--;
				if (link_ptr < (screen_num * links_per_screen) )
					{
					if ( (screen_num ) > 0)
						{
						came_from = UP;
						screen_num--;
						disp_class(screen_num, link_names, 
							link_names_ptr, wind_info, graph_wnd,came_from);
						}
						else
						{
						link_ptr++;
						beep ();
						}
					}
				break;

			/*-----------------FOREWARD ONE SCREEN---------------*/

			case ESC8   :
			case KEY_F(8):
			case KEY_NPAGE:
			case KEY_SF:
				if (screen_num < total_screens)
					{
					came_from = DOWN;
					screen_num++;
					link_ptr = screen_num * links_per_screen;
					disp_class(screen_num, link_names, 
						link_names_ptr, wind_info, graph_wnd,came_from);
					}
					else
					{
					beep ();
					}
			break;

	 		/*---------------BACKWARD ONE SCREEN------------------*/

			case ESC7   :
			case KEY_F(7):
			case KEY_PPAGE:
			case KEY_SR:
				if (screen_num > 0)
					{
					came_from = UP;
					screen_num--;
					link_ptr = screen_num * links_per_screen;
					disp_class(screen_num, link_names, 
						link_names_ptr, wind_info, graph_wnd,came_from);
					}
					else
					{
					beep ();
					}
			break;

		/*------------------EXIT--------------------*/

	 	case KEY_F(3) :
		case ESC3     :

			for (i = 0; i < link_names_ptr; i++)
				{
				delwin(wind_info[i]);
				free (link_names[i]);
				free (path_names[i]);
				free (class_perm[i]);
				}

			/* delete all windows */  

			delwin(graph_wnd);
			remwin(key_window, white);

			return;

			/*--------------GET FUNCTION---------------*/

			case KEY_F(2) :
			case ESC2 :
				selection = get_func (contfuncs, contfnum);
				switch (selection) 
					{
					case 1 :
				/*----------------RELATIONAL GRAPHS--------------------*/
	
				   strcpy (objclass.objname, link_names[link_ptr]);
					strcpy (objclass.objrep_path, path_names[link_ptr]);
					objclass.lock = *class_perm[link_ptr];
					if (*class_perm[link_ptr] == OBJ_LOCK_NB)
						READONLY = TRUE;
	
					rel_graph(&objclass);
	
					touchwin(graph_wnd);
					graph_wnd->_csbp = NORMAL;
					wrefresh(graph_wnd);
					touchwin(key_window);
					key_window->_csbp = NORMAL;
					wrefresh(key_window);
	 				break;

					case 2 :

				/*-----------------SEARCH WITH CRITERIA-----------*/
	
	
					strcpy (objclass.objname, link_names[link_ptr]);
					strcpy (objclass.objrep_path, path_names[link_ptr]);
					objclass.lock = *class_perm[link_ptr];
					if (open_object_class (&objclass) > 0)
						{
						if 	(*class_perm[link_ptr] == OBJ_LOCK_NB)
							READONLY = TRUE;
						get_descriptors (&objclass, &head_descriptor_data);
						search_criteria (&objclass, head_descriptor_data);

						free (head_descriptor_data);

						touchwin(graph_wnd);
						graph_wnd->_csbp = NORMAL;
						wrefresh(graph_wnd);
						touchwin(key_window);
						key_window->_csbp = NORMAL;
						wrefresh(key_window);
						}
	
					break;
	
					case 3 :

	 				/*----------------RETRIEVE/EDIT OBJECTS----------------*/

		 			objs[0] = '\0';
					strcpy (objclass.objname, link_names[link_ptr]);
					strcpy (objclass.objrep_path, path_names[link_ptr]);
					objclass.lock = *class_perm[link_ptr];
					if (open_object_class (&objclass) > 0)
						{
						if (*class_perm[link_ptr] == OBJ_LOCK_NB)
							READONLY = TRUE;
						get_descriptors (&objclass, &head_descriptor_data);
						object_display (&objclass, objs, head_descriptor_data);

						free (head_descriptor_data);
	
						touchwin(graph_wnd);
						graph_wnd->_csbp = NORMAL;
						wrefresh(graph_wnd);
						touchwin(key_window);
						key_window->_csbp = NORMAL;
						wrefresh(key_window);
						}
	
					break;
	
					case 4 :

					/*--------------DELETE AN OBJECT CLASS---------------*/

					/*------------------------------------------------*/
					/* if object was opened read_only. Can not delete */
					/*------------------------------------------------*/
					strcpy (objclass.objname, link_names[link_ptr]);
					strcpy (objclass.objrep_path, path_names[link_ptr]);
					objclass.lock = *class_perm[link_ptr];
					if (*class_perm[link_ptr] == OBJ_LOCK_NB)
						{
						prerr (MSGS(ER,CT_OPENED_RO,DF_CT_OPENED_RO), 0, FALSE);
						break;
						}
					if (question (MSGS(CT,ct_warning,
						"WARNING - all objects will be lost.  continue?") )

							== TRUE)
						{
						  objclass.objptr = (struct Class *)odm_mount_class(objclass.objname);
						  if(	odm_rm_class(objclass.objptr) == 0)
							{
							help (MSGS(CT,ct_delete_ok,
								"OBJECT CLASS DELETE \n\
 *** SUCCESSFUL ***\n") );
							}
							else
							{
								prerr (MSGS(ER,DELETE_FAILED,DF_DELETE_FAILED),odmerrno, FALSE);
							}
						}

	
						touchwin(graph_wnd);
						graph_wnd->_csbp = NORMAL;
						wrefresh(graph_wnd);
						touchwin(key_window);
						key_window->_csbp = NORMAL;
						wrefresh(key_window);
					break;

					default :
						selection = -99;
						touchwin(graph_wnd);
						graph_wnd->_csbp = NORMAL;
						wrefresh(graph_wnd);
						touchwin(key_window);
						key_window->_csbp = NORMAL;
						wrefresh(key_window);
						break;
					} /* end switch */

				break;
			         
		} /* end switch */
	} /* end for */

} /* end control */





/**********************************************************/
/* parameters : screen_num - which screen of links to     */
/* display.													          */
/*              link_names - array of linked object names */
/*                  						                      */
/**********************************************************/
void disp_class (screen_num, link_names,
						link_names_ptr, wind_info, graph_wnd,came_from)
int 	screen_num;
char 	*link_names[];
int 	link_names_ptr;
WINDOW		*wind_info[];
WINDOW 		*graph_wnd;
int   came_from;
{


int i;
	/*----------------------------------------*/
	/* clear the screen if necessary          */
	/*----------------------------------------*/
 if (came_from != DONT)
		{
		for (i = (screen_num + came_from) * links_per_screen;
			i < (((screen_num + came_from) * links_per_screen) + links_per_screen); i++)
			{
			if (i == (link_names_ptr))
				break;
			werase(wind_info[i]);
			wind_info[i]->_csbp = NORMAL;
			wrefresh(wind_info[i]);
			}
		}	

	/*----------------------------------------*/
	/* display the main object class and all  */
	/* of its links.									*/
	/*----------------------------------------*/

	for (i = screen_num * links_per_screen;(i < (screen_num * links_per_screen) + links_per_screen); i++)
		{
		if (i == (link_names_ptr) || link_names_ptr == -99)
			break;
		wcolorout(wind_info[i], Bxa);
		cbox(wind_info[i]);
		wcolorend(wind_info[i]);
		wmove(wind_info[i], 1,1);
		waddstr(wind_info[i], link_names[i]);
		touchwin(wind_info[i]);
		wind_info[i]->_csbp = NORMAL;
		wrefresh(wind_info[i]);
		}	

	clearok (graph_wnd, TRUE);
	graph_wnd->_csbp = NORMAL;
	wrefresh(graph_wnd);

} /* end disp_links */


/**********************************************************/
/* GET_FUNC    : allows selection of function to perform  */
/* INPUT: selection - address of selection variable to be */
/*                    returned.                           */
/*         cnt - number of menu items + 1.                */
/**********************************************************/
int get_func(funcs,cnt)
char *funcs[];
int cnt;

{
WINDOW *sel_wnd;			/* selection window */
	int sel_ptr;			/* points to current selection */
	int c;
	int i;
	int selection = -99;            /* value returned to control */
	
	sel_wnd = newwin(10, 30, 10, 40);

	wcolorout(sel_wnd, Bxa);
	cbox (sel_wnd);
	wcolorend(sel_wnd);
	
	/*----------------------------------------*/
	/* display the list of functions          */
	/*----------------------------------------*/
	for (i = 1;i < cnt + 1; i++)
	{
		wmove (sel_wnd, i, 2);
		waddstr (sel_wnd, funcs[i]);
	}

	touchwin (sel_wnd);
	sel_wnd->_csbp = NORMAL;
	wrefresh (sel_wnd);

	sel_ptr = 1;

	/*------------------------------------------*/
	/* flush the hft input buffer               */
	/*------------------------------------------*/
	flushinp();
	for (;;)
	{
		hilite (sel_wnd, sel_wnd, sel_ptr, 2, 27, 0, 0, rwhite, white);
		

	if (REPLAY)
		{
		c = get_char();
		}
	else
		{
	c = ms_input(sel_wnd);
	if (TRAC) save_char(c);
		}
		switch (c)
			{
			case KEY_DOWN :
			case CTRL_N   :
				sel_ptr++;
				if (sel_ptr == cnt + 1)
					sel_ptr = 1;
				break;

			case KEY_UP :
			case CTRL_P :
				sel_ptr--;
				if (sel_ptr == 0)
					sel_ptr = cnt; 
				break;
			       
			case KEY_NEWL :
			case KEY_NWL  :
			case CTRL_O   :
				delwin (sel_wnd);
				return (sel_ptr);
			default :
				break;

			} /* end switch */
 	} /* end for */
} /* end get_func */
