static char sccsid[] = "@(#)73	1.13  src/bos/usr/bin/odme/odmerelgraph.c, cmdodm, bos411, 9428A410j 1/14/93 17:44:41";
/*
 *   COMPONENT_NAME: ODME
 *
 *   FUNCTIONS: disp_desc
 *		disp_links
 *		disp_my_desc
 *		rel_graph
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*******************************************************************/
/*  FILE NAME :     ODMERELGRAPH.C				                      */
/*  DESCRIPTION:    This file contains the Object Data Manager     */
/*                  Editor display relational graphs routines.     */
/*  CONTENTS :							                                  */
/*      void rel_graph    ();        main relational graph routine */
/*      void disp_links   ();        display descriptors - graphic */
/*      void disp_my_desc ();        display descriptors - text    */ 
/*													                    		    */
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

char *graphfuncs[5];
/***************************************************************/
/*  rel_graph                                                  */
/*  Parameters  : objhandle - the object class handle	         */
/***************************************************************/
rel_graph (objhandle)
struct objtoken		*objhandle;
{

        void get_descriptors ();
  	void disp_links ();
	void disp_my_desc ();
	int get_funcs ();
	int odme_prscr ();
      
	struct Class *classp;

        struct descriptor_data  *head_descriptor_data, *ptr_descriptor_data;

        char objs[256];
	struct objtoken objclass;

        WINDOW *graph_wnd,            /* main graph display window   */
	       *wind_info[MAX_LINKS], /* array of windows for object classes */
               *key_window;           /* function key window         */

        int c,                        /* user input character        */
	    i,
       	    j,
            link_names_ptr,    	      /* pointer into link names list */
	    link_ptr,                 /* ptr used for cursor movement */
            cursor_line    = 0,       /* cursor line position        */
            length         = 0,       /* storage for string lengths  */
	    offset         = 6,       /* spacing for first link window */ 
	    connector_row,
	    connector_col,
            data_width     = 0;       /* length of hilite bar        */

        int selection      = -99;     /* value returned from get_funcs */

        int screen_num 	   = 0,	      /* current active screen of links */
	    total_screens  = 0,	      /* total number of screens of links */
	    came_from;      /* flag passed to disp_links, holds up,down,dont */

	char *link_names[MAX_LINKS]; /* list of object class this class links to */
	int class_perm[MAX_LINKS];

	char	working_class[MAX_DESC_NAME]; /* current object class being viewed */

	graphfuncs[0] = "  ";
	graphfuncs[1] = MSGS(RG,gfunc1,"Link Graph");
	graphfuncs[2] = MSGS(RG,gfunc2,"Retrieve/Edit");
	graphfuncs[3] = MSGS(RG,gfunc3,"Display Descriptors");
	graphfuncs[4] = MSGS(RG,gfunc4,"Return");

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
	graph_wnd->_csbp = NORMAL;

	wcolorout(graph_wnd, Bxa);
	drawbox (graph_wnd, 0, 0, 3, COLS);
	drawbox (graph_wnd, 2, 0, LINES - 3, COLS);
	wcolorend(graph_wnd);

   	length = strlen (MSGS(RG,GRAPH_NAME,DF_GRAPH_NAME) );
   	wmove    (graph_wnd, 1, CENTER - (length/2));
   	waddstr  (graph_wnd, MSGS(RG,GRAPH_NAME,DF_GRAPH_NAME) );

 	touchwin (graph_wnd);
   	graph_wnd->_csbp = NORMAL;
   	wrefresh (graph_wnd);

   	/*----------------------------------------------------------*/
   	/* Create a bottom key window 1 line deep and on the last   */
   	/* line of the screen.                                      */
   	/*----------------------------------------------------------*/
   	key_window = newwin (1, 0, LINES - 1, 0);
   	key_window->_csbp = NORMAL;
   	wmove (key_window, 0, 5);
   	waddstr (key_window, MSGS(KS,STD_KEYS,DF_STD_KEYS) );

   	touchwin (key_window);
   	key_window->_csbp = NORMAL;
   	wrefresh (graph_wnd);
   	wrefresh (graph_wnd);
   	wrefresh (key_window);

	/*------------------------------------*/
	/* mount the object class. return if   */
	/* mount fails.                        */
	/*------------------------------------*/

	if ((int)(objhandle->objptr =                                                       (struct Class *) odm_mount_class (objhandle->objname)) == -1)
	   return;
	objhandle->number_desc = objhandle->objptr->nelem;

	/*-------------------------------------------------*/
	/* get the descriptors for this object             */
	/*-------------------------------------------------*/

	get_descriptors (objhandle, &head_descriptor_data);

	/*-------------------------------------------------*/
	/* copy the name of the selected object class into */
	/* working_class.				   */
	/*-------------------------------------------------*/

   	strcpy (working_class, objhandle->objname);

	/*-----------------------------------------------------*/
	/* malloc storage for each element in the array of     */
	/* link_names. 					       */
	/*-----------------------------------------------------*/

	for (i = 0; i < MAX_LINKS; i++)
	{	
		if ((link_names[i] = (char *) malloc (MAX_DESC_NAME)) == NULL) 
		/*---------------------------------------------------*/
		/* the malloc failed so free any space allocated     */
		/*---------------------------------------------------*/
		{
		   for (j = 0; j < i; j++)
		   {
			free (link_names[j]);
		   }
		   free ((struct descriptor_data *) head_descriptor_data);
		   prerr(MSGS(ER,RG_MALLOC_FAILED,DF_RG_MALLOC_FAILED),0,TRUE);
		}
	} /* end for */

	/*---------------------------------------------------------*/
	/* loop through the descriptors and find the link fields.  */
	/* objlink.     					   */
	/*---------------------------------------------------------*/
         
        for (i = 0, link_names_ptr = 1, ptr_descriptor_data =                                head_descriptor_data; i < objhandle->number_desc;
	     i++, ptr_descriptor_data++) 
   	{
      	switch (ptr_descriptor_data->classp->elem[i].type)
	 	{
	      	case OBJLINK :
			/*-------------------------------------------------*/
			/* if it is a link or repeat, put the name in      */
			/* link_names.					   */
			/*-------------------------------------------------*/
			strcpy (link_names[link_names_ptr],
			ptr_descriptor_data->classp->elem[i].link->classname); 
			class_perm[link_names_ptr] = OBJ_LOCK_EX;

			/*-------------------------------------------------*/
			/* create a window for this link field             */
			/*-------------------------------------------------*/
			wind_info[link_names_ptr] =       
			newwin(3,MAX_DESC_NAME, offset, 10);
			wind_info[link_names_ptr]->_csbp = NORMAL;
			offset += 3;
			class_perm[link_names_ptr] = OBJ_LOCK_EX;

			/*-------------------------------------------------*/
			/* if offset = MAX_LINK_POS, we need to start the  */
			/* next link position on the top of the next page. */
			/*-------------------------------------------------*/
			if (offset == MAX_LINK_POS)
			offset = LINK_WND_START;
	 		link_names_ptr++;
  			break;

		default:
			break;
		} /* end switch */
	} /* end for */

	/*-----------------------------------------------------*/
	/* if no link descriptors are found then set           */
	/* link_names_ptr to -99 to indicate no links.         */
	/*-----------------------------------------------------*/
	if (link_names_ptr == 1)
	{
		link_names_ptr = -99;
	}

	/*-----------------------------------------------------*/
	/* calculate the total number of screens.              */
	/*-----------------------------------------------------*/
	if (link_names_ptr != -99)
	{
		total_screens = (link_names_ptr - 1) / 6;
	}

	/*-------------------------------------------------------------*/
	/* free the descriptor memory                                  */
	/*-------------------------------------------------------------*/
	free ((struct descriptor_data *) head_descriptor_data);

	/*-------------------------------------------------------------*/
	/* set up the window for the main object class                 */
	/*-------------------------------------------------------------*/
	strcpy (link_names[0], objhandle->objname);
	class_perm[0] = objhandle->lock;
       wind_info[0]=newwin(BOX_HEIGHTH,MAX_DESC_NAME,MAIN_OBJ_ROW,MAIN_OBJ_COL);
	wind_info[0]->_csbp = NORMAL;
   	wmove(wind_info[0], 1,1);
	waddstr (wind_info[0], link_names[0]);
	touchwin (wind_info[0]);
 	
	/*----------------------------------------*/
	/* display the main object class and all  */
	/* of its links.			  */
	/*----------------------------------------*/
	came_from = DONT;
	disp_links(screen_num, link_names, link_names_ptr,
		wind_info, graph_wnd,came_from);
 
	/*-----------------------------------------*/
	/* USER INPUT BEGINS			   */
	/*-----------------------------------------*/
	link_ptr = 0;

	/*-------------------------------------------*/
	/* clear the hft input buffer                */
	/*-------------------------------------------*/
	flushinp();
	for (;;)
	{
		for (i = (screen_num * 6);(i < (screen_num * 6) + 6); i++)
		{
			if (i == (link_names_ptr) || link_names_ptr == -99)
				break;
			touchwin(wind_info[i]);
			wind_info[i]->_csbp = NORMAL;
			wrefresh(wind_info[i]);
		}

		/*----------------------------------------*/
		/* update the screen msg at the bottom    */
		/*----------------------------------------*/
		wmove (graph_wnd, LINES - 3, COLS - 20);
		wprintw (graph_wnd,MSGS(RG,rg_screen,
		   "Screen %-3d of  %-3d"), screen_num + 1, total_screens + 1);
		graph_wnd->_csbp = NORMAL;
		wrefresh(graph_wnd);

		/*---------------------------------------*/
		/* highlight the current window          */
		/*---------------------------------------*/
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
				help (MSGS(RG,relgraph_help,
					"Relational Graphs help \n\
 This function will allow you to see the object \n\
 classes which are linked to the selected       \n\
 Object Class.											  \n\
                                               \n\
 Additional functions include:                  \n\
  - Display Relational Graphs for the           \n\
      highlighted object class.                 \n\
  - Retrieve/Edit objects of the                \n\
      highlighted object class.                 \n\
  - Display the descriptors of the              \n\
      highlighted object class.                 \n\
") );

				touchwin(key_window);
				key_window->_csbp = NORMAL;
				wrefresh(key_window); 							
				wmove (graph_wnd, LINES - 3, COLS - 20);
					wprintw (graph_wnd,MSGS(RG,rg_screen,
					"Screen %-3d of  %-3d"), screen_num + 1, total_screens + 1);
				touchwin(graph_wnd);
				graph_wnd->_csbp = NORMAL;
				wrefresh(graph_wnd);
				selection = -99;

				break;
	
			/*-----------------CURSOR DOWN--------------*/

			case KEY_DOWN :
			case CTRL_N   :
				/*----------------------------------------*/
				/* if there any links continue            */
				/*----------------------------------------*/
				if (link_names_ptr == -99)
					break;

				/*----------------------------------------*/
				/* increment the link_ptr                 */
				/* if bottom of screen or last link       */
				/*   if there are more screens            */
				/*     set to erase previous screen       */
				/*     increment screen_num               */
				/*     display next screen of links       */
				/*   else                                 */
				/*     decrement the link_ptr             */
				/*     beep                               */
				/*----------------------------------------*/
				link_ptr++;
				if (link_ptr == ((screen_num * 6)+6) || 
					(link_ptr == link_names_ptr))
				{
					if ((screen_num ) < total_screens)
					{
					   came_from = DOWN;
					   screen_num++;
					   disp_links(screen_num, link_names, 
					      link_names_ptr, wind_info,                                                      graph_wnd,came_from);
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
				/*----------------------------------------*/
				/* if there are any links continue        */
				/*----------------------------------------*/
				if (link_names_ptr == -99) 
					break;

				/*----------------------------------------*/
				/* decrement the link_ptr                 */
				/* if top of screen                       */
				/*   if there are more screens            */
				/*     set to erase previous screen       */
				/*     decrement screen_num               */
				/*     display next screen of links       */
				/*   else                                 */
				/*     increment the link_ptr             */
				/*     beep                               */
				/*----------------------------------------*/
				link_ptr--;
				if (link_ptr < (screen_num * 6) )
				{
					if ( (screen_num ) > 0)
					{
					   came_from = UP;
					   screen_num--;
					   disp_links(screen_num, link_names, 
						link_names_ptr, wind_info,                                                      graph_wnd,came_from);
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
				   link_ptr = screen_num * 6;
				   disp_links(screen_num, link_names, 
					link_names_ptr, wind_info,                                                      graph_wnd,came_from);
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
					link_ptr = screen_num * 6;
					disp_links(screen_num, link_names, 
					 link_names_ptr, wind_info,                                                      graph_wnd,came_from);
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
				}

				/* delete all windows */  

				delwin(graph_wnd);
				remwin(key_window, white);

				return;

			case KEY_F(12) :
				if ((odme_prscr ()) == FALSE)
				{
				help(MSGS(ER,RG_PRINT_SCREEN_ERROR,                                                  DF_RG_PRINT_SCREEN_ERROR) );
				}
				break;
				
			/*----------------HOT KEY------------------*/
/*
			case KEY_F(12) :

				control ();

				touchwin(graph_wnd); 
				wrefresh(graph_wnd);
				touchwin(wind_info[0]);
				wrefresh(wind_info[0]);
				touchwin(key_window);
				wrefresh(key_window);
				break;
				
*/
			/*--------------GET FUNCTION---------------*/

			case KEY_F(2) :
			case ESC2 :
				selection = get_func (graphfuncs, 4);
				switch (selection) 
				{
				   case 1 :
			 	   /*-----DISPLAY LINKS FOR THIS OBJECT------*/
				   /*----------------------------------------*/
				   /* if there are no links break            */
				   /* if this is the main object class break */
				   /* get the name of the highlighted object */
                                   /* class                                  */
				   /* recursive call to rel_graph            */
				   /*----------------------------------------*/
					if (link_names_ptr == -99) 
					{
					   break;
					}
				   /*----------------------------------------*/
				   /* already in Rel_Graph for link_ptr 0    */
				   /*----------------------------------------*/
					if (link_ptr == 0)
					{
					   break;
					}
					strcpy(objclass.objname,                 						link_names[link_ptr]);
					objclass.lock = OBJ_LOCK_EX;
					objclass.objrep_path[0] = '\0';
					rel_graph(&objclass);
		
					touchwin(graph_wnd); 
					graph_wnd->_csbp = NORMAL;
					wrefresh(graph_wnd);
					touchwin(key_window);
					key_window->_csbp = NORMAL;
					wrefresh(key_window);
					break;

				   case 2 :

			        /*-----RETRIEVE/EDIT OBJECTS-----------*/
			        /*-------------------------------------*/
				/* get the name of the highlighted class */ 
				/* set the searchstring to NULL        */
				/* if the open succeeds       	       */
				/* get the descriptors for this object class */
				/*   call object_display	       */
				/*   free the descriptor storage       */
				/*-------------------------------------*/
					strcpy(objclass.objname,                                                               link_names[link_ptr]);
					objclass.lock = class_perm[link_ptr];
					objclass.objrep_path[0] = '\0';
					objs[0] = '\0';
					if(open_object_class (&objclass) > 0)
					{
					   if(class_perm[link_ptr]==OBJ_LOCK_NB)
					   READONLY = TRUE;

					   get_descriptors(&objclass,                                                                   &head_descriptor_data);

					   object_display(&objclass,objs,                                                               head_descriptor_data);

	   				   free((struct descriptor_data *)                                                              head_descriptor_data);

					   touchwin(graph_wnd); 
					   graph_wnd->_csbp = NORMAL;
					   wrefresh(graph_wnd);
					   touchwin(key_window);
					   key_window->_csbp = NORMAL;
					   wrefresh(key_window);
					}
					break;

				  case 3 :

				  /*----DISPLAY THE DESCRIPTORS------*/
		
					strcpy(objclass.objname,                                                               link_names[link_ptr]);
					objclass.lock = OBJ_LOCK_EX;
					objclass.objrep_path[0] = '\0';
					objs[0] = '\0';
					if (open_object_class (&objclass ) > 0)
					{
						get_descriptors(&objclass,                                                            &head_descriptor_data);
						disp_my_desc(&objclass,                    					      head_descriptor_data);
   						free((struct descriptor_data *)      						      head_descriptor_data);

						touchwin(graph_wnd); 
						graph_wnd->_csbp = NORMAL;
						wrefresh(graph_wnd);
						touchwin(key_window);
						key_window->_csbp = NORMAL;
						wrefresh(key_window);
					}
					break;

				default :
					break;
			     } 

			touchwin(graph_wnd); 
			graph_wnd->_csbp = NORMAL;
			wrefresh(graph_wnd);
			touchwin(key_window);
			key_window->_csbp = NORMAL;
			wrefresh(key_window);
			break;

		} /* end switch */
	} /* end for */

} /* end relational graph */
 

/************************************************************/
/* parameters : screen_num - which screen of links to       */
/* display.					            */
/*              link_names - array of linked object names   */
/* new          main_obj_window - main object class window  */
/*           			                            */
/************************************************************/
void disp_links (screen_num, link_names, 
		link_names_ptr, wind_info, graph_wnd,came_from)
int  	screen_num;
char 	*link_names[];
int 	link_names_ptr;
WINDOW	*wind_info[];
WINDOW 	*graph_wnd;
int   	came_from;
{

	int i, j;
	int connector_row, connector_col;

	/*----------------------------------------*/
	/* clear the screen if necessary          */
	/*----------------------------------------*/
 	if (came_from != DONT)
	{
		for (i = (screen_num + came_from) * 6;
			i < (((screen_num + came_from) * 6) + 6); i++)
		{
			if (i == (link_names_ptr))
				break;
			werase(wind_info[i]);
			wind_info[i]->_csbp = NORMAL;
			wrefresh(wind_info[i]);
		}	

		connector_row = 4;
		connector_col = MAIN_OBJ_COL + 2;


		wmove(graph_wnd, connector_row, connector_col);
		waddstr(graph_wnd, "   ");
		connector_row++;

		/*-------------------------------------------------*/
		/* clear the connecting arrows down the side       */
		/*-------------------------------------------------*/

		for (i = ( (screen_num + came_from) * 6) + 1;
			i < (((screen_num + came_from) * 6) + 6); i++)
		{
			if (i == (link_names_ptr))
				break;
			for (j = 0; j <  2; j++, connector_row++)
			{
				wmove(graph_wnd, connector_row, connector_col);
				waddstr(graph_wnd, " ");
			}
			wmove(graph_wnd, connector_row, connector_col);
			waddstr(graph_wnd, "   ");
			connector_row++;
		}
	}

	/*----------------------------------------*/
	/* display the main object class and all  */
	/* of its links.			  */
	/*----------------------------------------*/

	for (i = (screen_num * 6);(i < (screen_num * 6) + 6); i++)
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
	if (screen_num == 0)
	{
		wmove(wind_info[0], 1, 1);
		waddstr(wind_info[0], link_names[0]);
		wcolorout(wind_info[0], Bxa);
		cbox (wind_info[0]);
		wcolorend(wind_info[0]);
	}

 
	/*--------------------------------*/
	/* link the windows together      */
	/*--------------------------------*/
 
	/*---------------------------------*/
	/* if there is at least one window */
	/* start linking from the main     */
	/* object.			   */
	/*---------------------------------*/
	connector_row = 4; 
	connector_col = MAIN_OBJ_COL + 2;

	wmove(graph_wnd, connector_row, connector_col);
	waddstr(graph_wnd, "|->");
	connector_row++;

	for (i = (screen_num * 6) + 1;(i < (screen_num * 6) + 6); i++)
	{
		if (i == (link_names_ptr) || link_names_ptr == -99)
			break;
		for (j = 0; j <  2; j++, connector_row++)
		{
			wmove(graph_wnd, connector_row, connector_col);
			waddstr(graph_wnd, "|");
		}
		wmove(graph_wnd, connector_row, connector_col);
		waddstr(graph_wnd, "|->");
		connector_row++;
	}
	if (i == link_names_ptr && link_names_ptr != -99)
	{
		wmove(graph_wnd, --connector_row, connector_col);
		waddstr(graph_wnd, "|__");
	}

	graph_wnd->_csbp = NORMAL;
	wrefresh(graph_wnd);

} /* end disp_links */


/*****************************************************************/
/*  disp_my_desc  display the descriptors for this object class  */
/*  Parameters  : objhandle - the object class handle	          */
/*              : head_descriptor_data - head of descriptor info */
/*****************************************************************/

void disp_my_desc (objhandle, head_descriptor_data)
struct objtoken		*objhandle;
struct descriptor_data *head_descriptor_data;
{
	void disp_desc ();
      
   	struct descriptor_data  *ptr_descriptor_data;

   	WINDOW 	*desc_wnd,          /* main display window   */
	 	*desc_screen,	    /* whole screen          */
          	*key_window;        /* function key window   */

   	int 	c,                  /* user input character  */
	  	i,
		j;

	int 	screen_num = 0,	    /* current active screen of descriptors */
	 	total_screens,	    /* total number of screens of links */
	 	length;


   	/*--------------------------------------------------------------*/
   	/* create a box window which contains the display descriptors   */
   	/*  name and a border.                                          */
   	/*  ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿                 */
   	/*  ³    desc_screen                          ³                 */
   	/*  ³ ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ» ³                 */
   	/*  ³ º            ODME Text                º ³                 */
   	/*  ³ ÌÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¹ ³                 */
   	/*  ³ º                                     º ³                 */
   	/*  ³ º   desc_wnd                          º ³                 */
   	/*  ³ º                                     º ³                 */
   	/*  ³ ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼ ³                 */
   	/*  ³   function keys window                  ³                 */
   	/*  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ                 */
   	/*--------------------------------------------------------------*/

   	desc_screen = newwin (LINES - 2, 0, 0, 0);

	wcolorout(desc_screen, Bxa);
	drawbox (desc_screen, 0, 0, 3, COLS);
	drawbox (desc_screen, 2, 0, LINES - 3, COLS);
	wcolorend(desc_screen);

   	length = strlen (MSGS(RG,DISP_MY_DESC_NAME,DF_DISP_MY_DESC_NAME));
   	wmove (desc_screen, 1, CENTER - (length/2));
   	waddstr (desc_screen, MSGS(RG,DISP_MY_DESC_NAME,DF_DISP_MY_DESC_NAME));
	wmove (desc_screen, 3,1);
	waddstr (desc_screen, MSGS(RG,dd_class,"Selected Object Class ==> "));
	wmove (desc_screen, 3, 28);
	waddstr (desc_screen, objhandle->objname);

   	touchwin (desc_screen);
	desc_screen->_csbp = NORMAL;
   	wrefresh (desc_screen);

	desc_wnd = newwin (17, COLS - 2, 4, 1);
	touchwin (desc_wnd);
	desc_wnd->_csbp = NORMAL;
	wrefresh (desc_wnd);

   	/*----------------------------------------------------------*/
   	/* Create a bottom key window 1 line deep and on the last   */
   	/* line of the screen.                                      */
   	/*----------------------------------------------------------*/
   	key_window = newwin (1, 0, LINES - 1, 0);

   	wmove (key_window, 0, 0);
   	waddstr (key_window, MSGS(KS,DISP_KEY,DF_DISP_KEY) );

   	touchwin (key_window);
	key_window->_csbp = NORMAL;
   	wrefresh (key_window);

	/*-----------------------------------------------------*/
	/* calculate the total number of screens.              */
	/*-----------------------------------------------------*/
	total_screens = objhandle->number_desc / 15;
 	
	/*-----------------------------------------*/
	/* display the first screen of descriptors */
	/*-----------------------------------------*/
 
	disp_desc (screen_num, head_descriptor_data,objhandle, desc_wnd);

	/*-----------------------------------------*/
	/* USER INPUT BEGINS			   */
	/*-----------------------------------------*/

	for (;;)
	{
		/*---------------------------------------------------*/
		/* update the screen message at the bottom           */
		/*---------------------------------------------------*/
		wmove (desc_screen, LINES - 3, COLS - 20);
		wprintw(desc_screen, MSGS(RG,dd_screen,
		  "Screen %-3d of  %-3d"), screen_num + 1, total_screens + 1);
		desc_screen->_csbp = NORMAL;
		wrefresh(desc_screen);

		if (REPLAY)
		{
			c = get_char();
		}
		else
		{
			c = ms_input(desc_screen);
			if (TRAC) save_char(c);
		}

		switch (c)
		{
			/*-----------------HELP KEY PRESSED-------------*/
			case KEY_F(1) :
			case ESC1     :
				help (MSGS(RG,dispdesc_help,
					"Display Descriptors help \n\
 This function will display the descriptors for \n\
 the selected object class. The NAME field will \n\
 identify the descriptor, the TYPE field will  \n\
 show the descriptors type, and the LINKS TO   \n\
 field will show the object class that this    \n\
 descriptor links to, if any.                  \n\
 Page up or page down will allow you to display \n\
 additional pages of descriptors.              \n\
") );

				touchwin(key_window);
				key_window->_csbp = NORMAL;
				wrefresh(key_window); 							
				touchwin(desc_screen);
				desc_screen->_csbp = NORMAL;
				wrefresh(desc_screen);
				touchwin(desc_wnd);
				desc_wnd->_csbp = NORMAL;
				wrefresh(desc_wnd);

				break;

			/*-----------------FOREWARD ONE SCREEN---------------*/

			case ESC8   :
			case KEY_F(8):
			case KEY_NPAGE:
			case KEY_SF:
				if (screen_num < total_screens)
				{
					screen_num++;
					disp_desc(screen_num,                   						head_descriptor_data,								objhandle, desc_wnd);
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
					screen_num--;
					disp_desc(screen_num,									head_descriptor_data,								objhandle, desc_wnd);
				}
				else
				{
					beep ();
				}
				break;

		case KEY_F(12) :
			if ((odme_prscr ()) == FALSE)
			{
				help (MSGS(ER,RG_PRINT_SCREEN_ERROR,DF_RG_PRINT_SCREEN_ERROR) );
			}
			break;


		/*------------------EXIT--------------------*/

	 	case KEY_F(3) :
		case ESC3     :

			/* delete all windows */  

			delwin(desc_wnd);
			delwin(desc_screen);
			remwin(key_window, white);

			return;

		} /* end switch */
	} /* end for */
} /* end disp_my_desc */
 
/**************************************************************/
/* disp_desc : this procedure will display one screen of data */
/* in the form of descriptors and names for the object class  */
/* INPUT : screen_num - which screen of descriptors to print  */
/*         head_descriptor_data - pointer to start of desc.   */
/*         objhandle - object handle for this object class.   */
/*         desc_wnd - window to display data in.              */
/**************************************************************/
void disp_desc (screen_num, head_descriptor_data,objhandle, desc_wnd)
int screen_num;
struct descriptor_data *head_descriptor_data;
struct objtoken *objhandle;
WINDOW *desc_wnd;
{
	int i;

	struct descriptor_data *ptr_descriptor_data;

	werase(desc_wnd);
	wmove    (desc_wnd, 0, 0);
	waddstr  (desc_wnd, MSGS(RG,dd_heading,
		"DESCRIPTOR NAME           TYPE               LINKS TO"));
	wmove (desc_wnd, 1, 0);
	waddstr (desc_wnd, "-----------------------------------------------------------------------------");
	
	for (i = 0, ptr_descriptor_data = head_descriptor_data +
            (screen_num * 15); i < 15; ptr_descriptor_data++, i++)
	{
		if ((screen_num * 15 + i) == objhandle->number_desc)
			break;
		wmove (desc_wnd, i + 2, 0);
		waddstr(desc_wnd,ptr_descriptor_data->classp->elem[i].elemname);
		wmove (desc_wnd, i + 2, MAX_DESC_NAME + 1);
		waddstr (desc_wnd, ptr_descriptor_data->header);  
		switch (ptr_descriptor_data->classp->elem[i].type)
		{    
			case OBJLINK :
			wmove (desc_wnd, i + 2, 45);
			waddstr (desc_wnd,								 ptr_descriptor_data->classp->elem[i].link->classname);
		   	break;

			default :
				break;
		} /* end switch */
 	}

	touchwin (desc_wnd);
	desc_wnd->_csbp = NORMAL;
	wrefresh (desc_wnd);
	return;
} /* end disp_desc */

