static char sccsid[] = "@(#)68  1.13  src/bos/usr/bin/odme/odmecreate.c, cmdodm, bos411, 9428A410j 1/14/93 17:42:11";
/*
 * COMPONENT_NAME: (ODME) ODMECREATE - create and edit object classes
 *
 * FUNCTIONS: class_create, permissions, edit_descriptor, edit_by_type
 *            delete_descriptor_link, add_descriptor_link, print_descriptors
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
/***************************************************************************/
/*                                                                         */
/*   FILE NAME :    odmecreate.c                                           */
/*                                                                         */
/*   DESCRIPTION :  This file contains Object Data Manager                 */
/*                  Editor create object class routines.                   */
/*                                                                         */
/*   CONTENTS :                                                            */
/*     void odm_create_class ();       create an ODM object class          */
/*     void permissions ();            enter permissions for object class  */
/*     int  edit_descriptor ();        edit the descriptor                 */
/*     int  edit_by_type ();           edit the descriptor by type         */
/*     void delete_descriptor_link (); delete a descriptor from the list   */
/*     void add_descriptor_link ();    add a descriptor to the list        */
/*     void print_descriptors ();      print a screen of descriptors       */
/*                                                                         */
/***************************************************************************/

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


/**********************************************************************/
/*  create class :                                                    */
/*      create a new object class from the descriptors prompted for.  */
/**********************************************************************/
void class_create (objhandle)

struct objtoken *objhandle;
{
    	int  	edit_descriptor ();
    	void 	delete_descriptor_link ();
    	void 	add_descriptor_link ();
    	void 	new_panel ();
    	void 	print_descriptors ();
    	int 	permissions ();
    	int  	question ();

    	struct 	descriptor_data_old 	*head_desc_data = NULL,
                           		*ptr_desc_data  = NULL,
                           		*prev_desc_data = NULL;

    	WINDOW 	*box_window,
           	*descriptor_window,
           	*key_window;

    int c;                             /* user input character                */

    int cursor_line        = -1,       /* current cursor line; initially none */
        delta              = FALSE,    /* have changes occurred ?             */
        length             = 0,        /* storage for string lengths          */
        number_descriptors = 0;        /* number of descriptors created       */

    	char 	heading[50];           /* storage for screen heading          */
    	register 	i;             /* for loop traversal                  */
    	char 		*p;

	struct Class 	*data_start,
			*data_ptr;
	char 		*namelist[Desc_Count];
	struct Class 	*linklist[Desc_Count];
	char 		*collist[Desc_Count];
	struct StringClxn *stngclxn[Desc_Count];
	int 		j;
	char 		*clname;
	struct ClassElem *elemp,
			*elemp1;

    	/*-----------------------------------------------*/
    	/* setup the curses windows on this screen       */
    	/*                                               */
    	/*   ÚÄÄÄÄÄÄÄÄÄÄÄTEXTÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿             */
    	/*   ³  TEXT                       ³ b           */
    	/*   ÌÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¹ o           */
    	/*   ºÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿º x           */
    	/*   º³                           ³º             */
    	/*   º³  Descriptors window       ³º w           */
    	/*   º³                           ³º i           */
    	/*   º³                           ³º n           */
    	/*   º³                           ³º d           */
    	/*   ºÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙº o           */
    	/*   ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼ w           */
    	/*     key window                                */
    	/*-----------------------------------------------*/
    	/*  Setup the border box window.                 */
    	/*-----------------------------------------------*/
    	box_window = newwin (LINES - 1, 0, 0, 0);
    	wmove     (box_window, 0, 0);
    	wcolorout (box_window, white);
    	werase    (box_window);

        wcolorout(box_window, Bxa);
	drawbox (box_window, 0, 0, 3, COLS);
	drawbox (box_window, 2, 0, LINES - 3, COLS);
	wcolorend(box_window);

    	length =  strlen (MSGS(CR,CREATE_NAME,DF_CREATE_NAME) );
    	wmove     (box_window, 0, CENTER - (length/2));
    	waddstr   (box_window, MSGS(CR,CREATE_NAME,DF_CREATE_NAME) );
    	sprintf   (heading, MSGS(CR, cr_objclass, "Object Class : %s"), 			   objhandle->objname);
    	wmove     (box_window, 1, GUTTER);
    	waddstr   (box_window, heading);

    	touchwin  (box_window);
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
    	waddstr   (key_window, MSGS(KS,CREATE_KEYS,DF_CREATE_KEYS) );

    	touchwin  (key_window);
    	wrefresh  (key_window);

    	/*-----------------------------------------------------------*/
    	/* Create a center screen window to display descriptors      */
    	/*-----------------------------------------------------------*/
    	descriptor_window = newwin (MAX_CREATE_ITEMS, COLS - (2*GUTTER), 						3, GUTTER);

    	/*-----------------------------------------------------------*/
    	/* allocate memory for 50 descriptors                        */
    	/*-----------------------------------------------------------*/
	if ((data_ptr = data_start = 
	   (struct Class *) malloc (sizeof(struct Class))) == NULL)
           prerr (MSGS(ER,CR_DMALLOC_FAILED,DF_CR_DMALLOC_FAILED), 0, TRUE);

	/*-------------------------------------------------------------*/
	/* set the collection pointer to NULL. Used as flag later      */
	/*-------------------------------------------------------------*/
	data_start->clxnp = NULL;

	if ((data_start->elem =
		(struct ClassElem *) malloc (Desc_Count *                                        sizeof(struct ClassElem))) == NULL)
	{
		free (data_start->clxnp->clxnname);
		free (data_start->clxnp);
		free (data_start);
      		prerr (MSGS(ER,CR_DMALLOC_FAILED,DF_CR_DMALLOC_FAILED),
                      0, TRUE);
	}
    /*-----------------------------------------------------------*/
    /*   malloc memory for the classname                         */
    /*-----------------------------------------------------------*/
	if ( (clname = (char *) malloc(255)) == NULL)
	{
		free (data_start->clxnp->clxnname);
		free (data_start->clxnp);
		free(data_start->elem);
		free(data_start);
  		prerr (MSGS(ER,CR_DMALLOC_FAILED,DF_CR_DMALLOC_FAILED),
			0, TRUE);
	}

	objhandle->objptr = data_start;
	data_start->classname = clname;	

	/*------------------------------------*/
	/* malloc storage for objects         */
	/*------------------------------------*/
	for (i = 0; i < Desc_Count; i++)
		{
		if (((namelist[i] = (char *) malloc (100)) == NULL) ||
		     ((linklist[i] = (struct Class *) 							malloc (sizeof(struct Class))) == NULL) ||
		      	((collist[i] = (char *) malloc (100)) == NULL))
			{
			for (j = 0; j < i; j++)
				{
				free (namelist[j]);
				free (linklist[j]);
				free (collist[j]);
				free (data_start->clxnp->clxnname);
				free (data_start->clxnp);
				free(data_start->elem);
				free(data_start);
  				prerr (MSGS(ER,CR_DMALLOC_FAILED,							DF_CR_DMALLOC_FAILED), 0, TRUE);
				}
			}
		} /* end for */
	
	for (i = 0; i < Desc_Count; i++)
		{
		data_start->elem[i].elemname = namelist[i];  
		data_start->elem[i].link = linklist[i];
		linklist[i]->classname = NULL;
		data_start->elem[i].col = collist[i];
		}

    for (;;)
     {

        /*----------------------------------------------------------*/
        /* if no descriptors have been created yet don't highlight  */
        /*----------------------------------------------------------*/
        if (cursor_line >= 0)
            hilite (descriptor_window, descriptor_window, cursor_line,
                    0, 50, cursor_line, 0, rwhite, white);

	if (REPLAY)
	{
		c = get_char();
	}
	else
	{
		c = ms_input(descriptor_window);
		if (TRAC) save_char(c);
	}


        switch (c)
        {
            /*---------------------HELP PRESSED----------------------*/
            case KEY_F(1) :
            case ESC1     :
		help (MSGS(CR,create_help,
                  "Create help \n\
 The Create screen allows you to specify descriptors \n\
 for the object class to be created.                 \n\
                                                    \n\
 Select Add (<Esc>4 or PF4) and enter your descriptor \n\
 name, type, and iterator.  Continue (<Esc>1 or PF1) \n\
 and enter the link object class or other specifed   \n\
 fields.                                             \n\
                                                     \n\
 Upon exit of the create screen (<Esc>3 or PF3) you  \n\
 will be asked if you wish to create this object     \n\
 class.                                              \n\
") );

                  touchwin (descriptor_window);
                  wrefresh (descriptor_window);
                  touchwin (key_window);
                  wrefresh (key_window);

                  break;

            /*-------------------EDIT DESCRIPTOR-----------------------*/
            case KEY_F(2) :
            case ESC2     :
                  /*-----------------------------------------------------*/
                  /* if I am sitting on a valid descriptor               */
                  /*  {                                                  */
                  /*    edit the descriptor;                             */
                  /*    move a descriptor pointer back to the beginning  */
                  /*         of a page;                                  */
                  /*    reprint the descriptors;                         */
                  /*    touch and refresh;                               */
                  /*  }                                                  */
                  /*-----------------------------------------------------*/
                  if (ptr_desc_data != NULL)
                   {
                     edit_descriptor (ptr_desc_data);

                     for (i = 0, prev_desc_data = ptr_desc_data;
                         ((i <= cursor_line) && (prev_desc_data->prev != NULL));
                          i++, prev_desc_data = prev_desc_data->prev);

                     print_descriptors (descriptor_window, prev_desc_data);

                     touchwin (descriptor_window);
                     wrefresh (descriptor_window);
                     touchwin (key_window);
                     wrefresh (key_window);
                   }

                  break;


            /*------------------CREATE OBJECT CLASS------------------*/
            case KEY_F(3) :
            case ESC3     :
                  /*------------------------------------------------*/
                  /*  If I have made changes to this screen and     */
                  /*     I want to create the object class          */
                  /*------------------------------------------------*/

                  if ((delta == TRUE) &&
                      (question (MSGS(CR,create_msg,"CREATE Object Class ?")) 				== TRUE))
                   {
		        /*--------------------------------------------------*/
		        /* fill in Class information                        */
		        /*--------------------------------------------------*/
			elemp = data_start->elem;         
			strcpy (data_start->classname, objhandle->objname);
			data_start->begin_magic = ODMI_MAGIC;
			data_start->end_magic = -ODMI_MAGIC;
			data_start->nelem = number_descriptors;

		        /*--------------------------------------------------*/
			/* This will have to be changed to class_path when  */
			/* ODM answers DR33 about name consistency.         */
			/*--------------------------------------------------*/

                        objhandle->number_desc = number_descriptors;
		
                        /*---------------------------------------------------*/
                        /* ObjectCreate needs consecutive memory locations   */
                        /* for descriptors passed to it, so allocate storage */
                        /* and copy descriptors from the linked list         */
                        /*---------------------------------------------------*/

                        for (i = 0,
                          ptr_desc_data = head_desc_data;
                          (i < number_descriptors) && (ptr_desc_data != NULL);
                          i++, ptr_desc_data = ptr_desc_data->next)
                        {
    			    /*-----------------------------------------------*/
    			    /* if a LINK, malloc memory for Class struct     */
    			    /*-----------------------------------------------*/
			    if (ptr_desc_data->descriptor.ODM_type == ODM_LINK)
			    {
				if ( (data_start->elem[i].link->classname =
				   (char *) malloc (100)) == NULL)
			        {
  				   prerr (MSGS(ER,CR_DMALLOC_FAILED,
					DF_CR_DMALLOC_FAILED), 0, TRUE);
				}
                         	strcpy (data_start->elem[i].link->classname,
                                ptr_desc_data->descriptor.link_class);
			    }
    			    /*-----------------------------------------------*/
    			    /* if a VCHAR, set up storage for collection     */
    			    /*-----------------------------------------------*/
			    if((ptr_desc_data->descriptor.ODM_type == ODM_VCHAR)
			       && (data_start->clxnp == NULL))
			    {
			       if((data_start->clxnp = (struct StringClxn *)
                                 malloc(sizeof(struct StringClxn))) == NULL)
			       {
			          free (data_start);
      				  prerr (MSGS(ER,CR_DMALLOC_FAILED,                                                       DF_CR_DMALLOC_FAILED), 0, TRUE);
			       }

				  if ((data_start->clxnp->clxnname = 
					(char *) malloc(255)) == NULL)
					{
						free (data_start->clxnp);
						free (data_start);
      						prerr (MSGS(ER,CR_DMALLOC_FAILED                                                      ,DF_CR_DMALLOC_FAILED),
							0, TRUE);
					}
				  strcpy(data_start->clxnp->clxnname,                                                    objhandle->objname);
				  strcat(data_start->clxnp->clxnname, ".vc");
                        }			

                        strcpy (data_start->elem[i].elemname,                                                   ptr_desc_data->descriptor.descrip_name);
                        strcpy (data_start->elem[i].col,
                                ptr_desc_data->descriptor.link_descrip);

                        data_start->elem[i].type =                                                              ptr_desc_data->descriptor.ODM_type;
                        data_start->elem[i].size =                                                              ptr_desc_data->descriptor.size;

			/*---------------------------------------*/
			/*---------------------------------------*/

                      } /* end for */

		      ptr_desc_data = head_desc_data;
		      cursor_line = 0;

		      /*----------------------------------------------------*/
		      /* call routine to fill in offsets for data in Class  */
		      /*----------------------------------------------------*/
		      get_offsets(data_start);								
                      /*----------------------------------------------------*/
                      /*  Ask user for object class permissions             */
                      /*  Create object class                               */
                      /*----------------------------------------------------*/
		      if ((permissions()) == CONTINUE)
		      {
                         if (odm_create_class(data_start) < 0)
                         {
                           prerr (MSGS(ER,CR_CREATE_FAILED,DF_CR_CREATE_FAILED),
                                  odmerrno, FALSE);

                           touchwin (descriptor_window);
                           wrefresh (descriptor_window);
                           touchwin (key_window);
                           wrefresh (key_window);

                           break;
                         }
		      }
		      else
		      {
                           touchwin (descriptor_window);
                           wrefresh (descriptor_window);
                           touchwin (key_window);
                           wrefresh (key_window);
			   break;
		      }

                   } /* end delta == TRUE */

                  delwin    (descriptor_window);
                  delwin    (box_window);

                  delwin    (key_window);
                  /*--------------------------------------------*/
                  /* free up the linked list of descriptors     */
                  /*--------------------------------------------*/
                  for (i = 0, ptr_desc_data = head_desc_data;
                       ((i < number_descriptors) && (ptr_desc_data != NULL));
                       i++, ptr_desc_data = ptr_desc_data->next)
                   {
                     free ((struct descriptor_data_old *) ptr_desc_data);
                   }
		   if (data_start->clxnp != NULL)
		   {
				free (data_start->clxnp->clxnname);
				free (data_start->clxnp);
			}
				free (data_start->elem);
				free (data_start);
				for (i = 0; i < Desc_Count; i++)
				{
					free(namelist[i]);  
					if (linklist[i]->classname != NULL)
						free (linklist[i]->classname);
					free (linklist[i]);
					free(collist[i]);
				}
                  return;

            /*--------------------ADD A DESCRIPTOR---------------------*/
            case KEY_NEWL :
	    case KEY_NWL  :
            case KEY_F(4) :
            case ESC4     :
                  /*---------------------------------------------------*/
                  /* create a new descriptor link                      */
                  /* if I successfully created a link and edited       */
                  /*    the descriptor correctly                       */
                  /*  {                                                */
                  /*     increment the current number of descriptors   */
                  /*     move to the new descriptor line               */
                  /*     move a descriptor pointer back to the begin-  */
                  /*       ning of the page                            */
                  /*     reprint the descriptors                       */
                  /*     set delta equal to TRUE                       */
                  /*  }                                                */
                  /* else get rid of the link I created                */
                  /*---------------------------------------------------*/
                  add_descriptor_link (&head_desc_data, &ptr_desc_data);

                  if ((ptr_desc_data != NULL) &&
                      (edit_descriptor (ptr_desc_data) == CONTINUE))
                   {
                      number_descriptors++;
                      cursor_line++;

                      if (cursor_line >= MAX_CREATE_ITEMS)
                       {
                         cursor_line = 0;
                         print_descriptors (descriptor_window, ptr_desc_data);
                       }
                      else
                       {
                         for (i = 0, prev_desc_data = ptr_desc_data;
                         ((i < cursor_line) && (prev_desc_data->prev != NULL));
                          i++, prev_desc_data = prev_desc_data->prev);
                         print_descriptors (descriptor_window, prev_desc_data);
                       }

                      delta = TRUE;
                   }
                  else if (ptr_desc_data != NULL)
                      delete_descriptor_link (&head_desc_data, &ptr_desc_data);

                  touchwin (descriptor_window);
                  wrefresh (descriptor_window);
                  touchwin (key_window);
                  wrefresh (key_window);

                  break;

            /*------------------DELETE A DESCRIPTOR------------------*/
            case KEY_F(5) :
            case ESC5     :
                  /*---------------------------------------------------*/
                  /*  if there is no descriptor                        */
                  /*     beep                                          */
                  /*  else                                             */
                  /*   {                                               */
                  /*     delete the link                               */
                  /*     erase the window                              */
                  /*     decrement the cursor line                     */
                  /*     move a pointer back to the first of the page  */
                  /*     reprint the page                              */
                  /*   }                                               */
                  /*---------------------------------------------------*/

                  if (ptr_desc_data == NULL)
                      beep ();
                  else
                  {
		      if (ptr_desc_data->next == NULL)
				cursor_line--;
                      delete_descriptor_link (&head_desc_data, &ptr_desc_data);
		      number_descriptors--;
		  }

		  if (cursor_line < -1)
			cursor_line = -1;
							
		  if (cursor_line == -1)
                  {
		    if (ptr_desc_data != NULL)
		    {
                    for (i = 0, prev_desc_data = ptr_desc_data;
                    ((i < MAX_CREATE_ITEMS ) && (prev_desc_data->prev != NULL));
                        i++, prev_desc_data = prev_desc_data->prev);

                        print_descriptors (descriptor_window, prev_desc_data);
			cursor_line = MAX_CREATE_ITEMS - 1; 
		   }
		   else
		   {
   			wmove  (descriptor_window, 0, 0);
   			werase (descriptor_window);
		   }
                 }
		 else
		 {
                      for (i = 0, prev_desc_data = ptr_desc_data;
                          ((i < cursor_line) && (prev_desc_data->prev != NULL));
                           i++, prev_desc_data = prev_desc_data->prev);

                      print_descriptors (descriptor_window, prev_desc_data);
                 }

                  touchwin (descriptor_window);
                  wrefresh (descriptor_window);

                  break;

            /*---------------------CURSOR DOWN-----------------------*/
            case KEY_DOWN :
            case CTRL_N   :
                  if (ptr_desc_data == NULL)
                      beep ();
                  else if (ptr_desc_data->next == NULL)
                      beep ();
                  else if (cursor_line >= (MAX_CREATE_ITEMS - 1))
                   {
                      cursor_line = 0;
                      ptr_desc_data = ptr_desc_data->next;
                      print_descriptors (descriptor_window, ptr_desc_data);
                   }
                  else
                   {
                      cursor_line++;
                      ptr_desc_data = ptr_desc_data->next;
                   }

                  touchwin (descriptor_window);
                  wrefresh (descriptor_window);

                  break;

            /*-----------------------CURSOR UP------------------------*/
            case KEY_UP :
            case CTRL_P :
                  if (ptr_desc_data == NULL)
                      beep ();
                  else if (ptr_desc_data->prev == NULL)
                      beep ();
                  else if (cursor_line == 0)
                   {
                    for (i = 0, prev_desc_data = ptr_desc_data;
                     ((i < MAX_CREATE_ITEMS) && (prev_desc_data->prev != NULL));
                     i++, prev_desc_data = prev_desc_data->prev);

                      cursor_line = --i;
                      print_descriptors (descriptor_window, prev_desc_data);

                      ptr_desc_data = ptr_desc_data->prev;
                   }
                  else
                   {
                      cursor_line--;
                      ptr_desc_data = ptr_desc_data->prev;
                   }

                  touchwin (descriptor_window);
                  wrefresh (descriptor_window);

                  break;
	    case KEY_F(12) :
		  if ((odme_prscr ()) == FALSE)
		  {
			help ();
		  }

		  break;

         }  /* end switch */
     }  /* end for */
} /* end create class */



/******************************************************************/
/*  permissions  : set the permissions for the owner,             */
/*                      group, and others, for this object class  */
/******************************************************************/

int permissions ()

{
    void new_panel ();
    void remwin    ();

    WINDOW *key_window,
           *inner_panel,
           *outer_panel;
    
    int    c;

    int data_width = 10,
        cursor_line = 0,
        owner = ODMOWNR | ODMOWNW,
        group = ODMGRPR | 0,
        other = ODMOTHR | 0;

    /*-------------------FUNCTION KEY WINDOW--------------------*/
    key_window = newwin (1, 0, LINES - 1, 0);
    wmove     (key_window, 0, 0);
    wcolorout (key_window, white);
    werase    (key_window);
    wmove     (key_window, 0, 0);
    waddstr   (key_window, MSGS(KS, PANEL_KEYS, DF_PANEL_KEYS));
    touchwin  (key_window);
    wrefresh  (key_window);

    /*---------------------PERMISSIONS PANEL---------------------*/
    new_panel (3, 30, 6, 20, 									MSGS(CR, cr_perm_obj, " Permissions for object class "), 			&outer_panel, &inner_panel);
    wmove   (inner_panel, 0, 0);
    waddstr (inner_panel, MSGS(CR, owner_rw, "Owner [read/write]"));
    wmove   (inner_panel, 1, 0);
    waddstr (inner_panel, MSGS(CR, group_r, "Group [read]      "));
    wmove   (inner_panel, 2, 0);
    waddstr (inner_panel, MSGS(CR, other_r, "Other [read]      "));

    touchwin (inner_panel);
    wrefresh (inner_panel);

    for (;;)
     {
        hilite (inner_panel, inner_panel, cursor_line, 7,
                data_width, cursor_line, 7, rwhite, white);

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
            /*---------------------CONTINUE--------------------------*/
            case KEY_NEWL :
	    case KEY_NWL  :
            case KEY_F(1) :
            case ESC1     :

		  odm_set_perms (owner | group | other);
                  remwin   (outer_panel, white);
                  remwin   (inner_panel, white);
                  remwin   (key_window, white);

                  return (CONTINUE);


            /*--------------------EXIT-----------------------------*/
            case KEY_F(3) :
            case ESC3     :
                  remwin   (outer_panel, white);
                  remwin   (inner_panel, white);
                  remwin   (key_window,  white);

                  return (EXIT);

            /*-----------------KEY DOWN------------------------*/
            case KEY_DOWN :
            case CTRL_N   :
                   if (cursor_line == 2)
                       cursor_line = 0;
                   else
                       cursor_line++;
                   break;

            /*-------------------KEY UP---------------------*/
            case KEY_UP :
            case CTRL_P :
                  if (cursor_line == 0)
                      cursor_line = 2;
                  else
                      cursor_line--;
                  break;
				

            /*-------------------DEFAULT----------------------*/
            default :
                  if (cursor_line == 0)
                   {
                     wmove (inner_panel, 0, 0);
                     switch (owner)
                      {
                        case ODMOWNR :
                              waddstr (inner_panel, MSGS(CR, owner_w, 							"Owner [Write]     "));
                              data_width = 5;
                              owner = ODMOWNW;
                              break;

                        case ODMOWNW :
                              waddstr (inner_panel, MSGS(CR, owner_rw, 							"Owner [Read/Write]"));
                              data_width = 10;
                              owner = ODMOWNR | ODMOWNW;
                              break;

                        case (ODMOWNR | ODMOWNW) :
                              waddstr (inner_panel, MSGS(CR, owner_n,							"Owner [None]      "));
                              data_width = 4;
                              owner = 0;
                              break;

                        case 0 :
                              waddstr (inner_panel, MSGS(CR, owner_r,							"Owner [Read]      "));
                              data_width = 4;
                              owner = ODMOWNR;
                              break;

                      } /* end inner switch */
                   }
                  else if (cursor_line == 1)
                   {
                     wmove (inner_panel, 1, 0);
                     switch (group)
                      {
                        case ODMGRPR :
                              waddstr (inner_panel, MSGS(CR, group_w,							"Group [Write]     "));
                              data_width = 5;
                              group = ODMGRPW;
                              break;

                        case ODMGRPW :
                              waddstr (inner_panel, MSGS(CR, group_rw,							"Group [Read/Write]"));
                              data_width = 10;
                              group = ODMGRPR | ODMGRPW;
                              break;

                        case (ODMGRPR | ODMGRPW) :
                              waddstr (inner_panel, MSGS(CR, group_n,							"Group [None]      "));
                              data_width = 4;
                              group = 0;
                              break;

                        case 0 :
                              waddstr (inner_panel, MSGS(CR, group_r,							"Group [Read]      "));
                              data_width = 4;
                              group = ODMGRPR;
                              break;

                      } /* end switch */
                   }
                  else
                   {
                     wmove (inner_panel, 2, 0);
                     switch (other)
                      {
                        case ODMOTHR :
                              waddstr (inner_panel, MSGS(CR, other_w,							"Other [Write]     "));
                              data_width = 5;
                              other = ODMOTHW;
                              break;

                        case ODMOTHW :
                              waddstr (inner_panel, MSGS(CR, other_rw,							"Other [Read/Write]"));
                              data_width = 10;
                              other = ODMOTHR | ODMOTHW;
                              break;

                        case (ODMOTHR | ODMOTHW) :
                              waddstr (inner_panel, MSGS(CR, other_n,							"Other [None]      "));
                              data_width = 4;
                              other = 0;
                              break;

                        case 0 :
                              waddstr (inner_panel, MSGS(CR, other_r,							"Other [Read]      "));
                              data_width = 4;
                              other = ODMOTHR;
                              break;

                      } /* end inner switch */
                   } /* end else */

                   wrefresh (inner_panel);

         } /* end switch */
     } /* end for */
} /* end permissions */



/***********************************************************************/
/*  edit descriptor :  This function displays a panel for editing the  */
/*                     descriptor's name, type, and iterator.          */
/*       PARAMETERS :  descriptor - the descriptor to input the values */
/*                     into                                            */
/*           RETURN :  Return whether edit was cancelled/continued     */
/***********************************************************************/
int edit_descriptor (ptr_desc_data)

struct descriptor_data_old *ptr_desc_data;
{
    void new_panel     ();
    void remwin        ();
    int  edit_by_type  ();
    int  std_edit_keys ();

    WINDOW *key_window,                 /* bottom function keys         */
           *outer_panel,                /* panel including the border   */
           *inner_panel;                /* data area of the panel       */

    struct field_data *head_field;      /* data attribute storage       */

    int  c;                             /* user input character         */
    int  cursor_column,                 /* current cursor column        */
         cursor_line;                   /* current cursor line          */
    char tempstr[MAX_ATTR_NAME];        /* temporary input string       */

    /*-------------------FUNCTION KEY WINDOW--------------------*/
    key_window = newwin (1, 0, LINES - 1, 0);
    wmove     (key_window, 0, 0);
    wcolorout (key_window, white);
    werase    (key_window);
    waddstr   (key_window, MSGS(KS, PANEL_KEYS, DF_PANEL_KEYS));
    touchwin  (key_window);
    wrefresh  (key_window);

    /*-------------------------------------*/
    /*  create descriptor editing panel    */
    /*  add items to panel                 */
    /*-------------------------------------*/
    new_panel (3, COLS - 15, 4, 6,
                MSGS(CR, cr_desc_edit," Descriptor Edit"), 					&outer_panel, &inner_panel);
    /*-----------------------------------------------------------*/
    /* Setup descriptor name string according to the attrname    */
    /*-----------------------------------------------------------*/
    wmove   (inner_panel, 0, 0);
    
    /* SMU Defect 36573 */
    wprintw (inner_panel, MSGS(CR, cr_name,
            "Name [%-*.*s]"), MAX_ATTR_NAME - 1,
            MAX_ATTR_NAME - 1, ptr_desc_data->descriptor.descrip_name);


    wmove   (inner_panel, 1, 0);

    /* SMU Defect 36573 */
    wprintw (inner_panel, MSGS(CR, cr_type,
                        "Type [%s]"),ptr_desc_data->header);

    /*---------------------------------------------------------*/
    /*  Set the iterator for the descriptor if not set already */
    /*---------------------------------------------------------*/
    touchwin (inner_panel);
    wrefresh (inner_panel);

    /*-----------------------------------------------------*/
    /*  Malloc storage for field description structures    */
    /*  Setup field start columns                          */
    /*  Setup field data widths                            */
    /*-----------------------------------------------------*/
    head_field = (struct field_data *) malloc (3 * sizeof (struct field_data));

    head_field->start_column = 6;
    head_field->data_width   = MAX_ATTR_NAME - 1; /* leave room for \0 */
    (head_field + 1)->start_column = 6;
    (head_field + 1)->data_width   = 1;
    (head_field + 2)->start_column = 10;
    (head_field + 2)->data_width   = 3;

    cursor_line   = 0;
    cursor_column = head_field->start_column;

    /*--------------------------*/
    /* User input begins        */
    /*--------------------------*/
    for (;;)
     {
        hilite (inner_panel, inner_panel, cursor_line,
                (head_field + cursor_line)->start_column,
                (head_field + cursor_line)->data_width,
                cursor_line, cursor_column, rwhite, white);


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
            	/*-------FUNCTION KEY 1 : Continue edit----------*/
            	case KEY_NEWL :
	    	case KEY_NWL  :
            	case KEY_F(1) :
            	case ESC1     :
                   /*------------------------------------------------------*/
                   /*  Enter the data currently given into the descriptor  */
                   /*  This includes field 1 (i.e the descriptor name)     */
                   /*  field 3 (i.e. the iterator), field 2 has already    */
                   /*  been input.                                         */
                   /*------------------------------------------------------*/
                   input_field (inner_panel, 0, head_field->start_column,
                                head_field->data_width, tempstr);
                   strcpy (ptr_desc_data->descriptor.descrip_name, tempstr);

                   input_field (inner_panel, 2, (head_field + 2)->start_column,
                                (head_field + 2)->data_width, tempstr);

                   /*--------------------------------------------------*/
                   /*  Now continue the descriptor edit armed with     */
                   /*  the current descriptor.ODM_type so you'll know what */
                   /*  to prompt the user for                          */
                   /*--------------------------------------------------*/
                   if (edit_by_type (&(ptr_desc_data->descriptor)) == CONTINUE)
                    {
                       free ((struct field_data *) head_field);

                       remwin (outer_panel, white);
                       remwin (inner_panel, white);

                       return (CONTINUE);
                    }
                   else
		    {
			touchwin (outer_panel);
			wrefresh (outer_panel);
			touchwin (inner_panel);
			wrefresh (inner_panel);
                     	break;
		    }

            /*-------FUNCTION KEY 3 : Discontinue edit----------*/
            case KEY_F(3) :
            case ESC3     :
                  free ((struct field_data *) head_field);

                  remwin (outer_panel, white);
                  remwin (inner_panel, white);

                  return (EXIT);

            /*-----------------KEY DOWN------------------------*/
            case KEY_DOWN :
            case CTRL_N   :
                   if (cursor_line == 1)
                       cursor_line = 0;
                   else
                       cursor_line++;

                   cursor_column = (head_field + cursor_line)->start_column;
                   break;

            /*-------------------KEY UP---------------------*/
            case KEY_UP :
            case CTRL_P :
                  if (cursor_line == 0)
                      cursor_line = 1;
                  else
                      cursor_line--;

                  cursor_column = (head_field + cursor_line)->start_column;
                  break;

            /*--------------------DEFAULT--------------------*/
            default :
                   /*--------------------------------------------------*/
                   /*  If cursor column is 2 provide an extended edit  */
                   /*  feature where the user hits any key and is able */
                   /*  to cycle thru the possible descriptor.ODM_types.*/
                   /*--------------------------------------------------*/
                   if (cursor_line == 1)
                    {
                      switch (ptr_desc_data->descriptor.ODM_type)
                       {
                         case SHORT :
                               ptr_desc_data->descriptor.ODM_type = LONG;
                               strcpy  (ptr_desc_data->header, "ODM_LONG");
                               wmove   (inner_panel, 1, 0);
                               waddstr (inner_panel, MSGS(CR, type1,							"Type [ODM_LONG]        "));
                               break;

                         case LONG :
                               ptr_desc_data->descriptor.ODM_type = LONGCHAR;
                               strcpy  (ptr_desc_data->header, "ODM_LONGCHAR");
                               wmove   (inner_panel, 1, 0);
                               waddstr (inner_panel, MSGS(CR, type2,							"Type [ODM_LONGCHAR]    "));
                               break;

                         case LONGCHAR :
                               ptr_desc_data->descriptor.ODM_type = VCHAR;
                               strcpy  (ptr_desc_data->header, "ODM_VCHAR");
                               wmove   (inner_panel, 1, 0);
                               waddstr (inner_panel, MSGS(CR, typev,							"Type [ODM_VCHAR]        "));
                               break;

                         case VCHAR :
                               ptr_desc_data->descriptor.ODM_type = CHAR;
                               strcpy  (ptr_desc_data->header, "ODM_CHAR");
                               wmove   (inner_panel, 1, 0);
                               waddstr (inner_panel, MSGS(CR, type3,							"Type [ODM_CHAR]     "));
                               break;

			case CHAR :
                               ptr_desc_data->descriptor.ODM_type = OBJLINK;
                               strcpy  (ptr_desc_data->header, "ODM_LINK");
                               wmove   (inner_panel, 1, 0);
                               waddstr (inner_panel, MSGS(CR, type5,							"Type [ODM_LINK]       "));
                               break;

                         case OBJLINK:
                               ptr_desc_data->descriptor.ODM_type = OBJMETHOD;
                               strcpy  (ptr_desc_data->header, "ODM_METHOD");
                               wmove   (inner_panel, 1, 0);
                               waddstr (inner_panel, MSGS(CR, type8,							"Type [ODM_METHOD]    "));
                               break;

                         case OBJMETHOD:
                               ptr_desc_data->descriptor.ODM_type = BINARY;
                               strcpy  (ptr_desc_data->header, "ODM_BINARY");
                               wmove   (inner_panel, 1, 0);
                               waddstr (inner_panel, MSGS(CR, type9,							"Type [ODM_BINARY]    "));
                               break;

                         case BINARY :
                               ptr_desc_data->descriptor.ODM_type = SHORT;
                               strcpy  (ptr_desc_data->header, "ODM_SHORT");
                               wmove   (inner_panel, 1, 0);
                               waddstr (inner_panel, MSGS(CR, type10,							"Type [ODM_SHORT]      "));
                               break;

                       } /* end switch */

                      wrefresh (inner_panel);
                    }
                   else
                    {
		/* SMU Defect 57765  */
                      std_edit_keys  (inner_panel, c,
                                      (head_field + cursor_line)->start_column,
                                      (head_field + cursor_line)->data_width-5, 
                                      &cursor_column, &cursor_line);
                     }
         } /* end switch */
     } /* end for */
} /* end edit descriptor */



/***********************************************************************/
/*  edit by type :  This function displays the panel for editing the   */
/*                  descriptors according to their type.  Prompts      */
/*                  include links, sizes and keyed values.             */
/*    PARAMETERS :  descriptor - the descriptor to input values in     */
/*                                                                     */
/***********************************************************************/
int edit_by_type (descriptor)

struct descrip_info *descriptor;
{
    void new_panel     ();
    void remwin        ();
    int  std_edit_keys ();

    WINDOW *outer_panel,            /* panel including header            */
           *inner_panel;            /* data area of the panel            */
    WINDOW *msg_wnd;                /* input error messages              */

    struct field_data *head_field;  /* used for data attributes          */

    char   tempstr[MAX_CLASS_NAME]; /* input string storage              */

    int    c;                       /* input character                   */
    int    cursor_column,           /* current cursor column position    */
           cursor_line,             /* current cursor line postion       */
           i,                       /* for loop traversal variable       */
	   rc,			    /* return code variable              */
           key_line  = -1,          /* Panel line with KEY/NO_KEY prompt */
           last_line =  0;          /* The last line of this panel       */
    int    insdata = FALSE;	    /* flag for insufficient input       */

	 /*------------------------------------------*/
	/* establish a message window                */
	/*-------------------------------------------*/
	msg_wnd = newwin (3, 65, 15, 5);
	wcolorout(msg_wnd, Bxa);
	cbox (msg_wnd);
	wcolorend(msg_wnd);
 
    /*------------------------------------------------------------*/
    /*  Depending on the descriptor.ODM_type I will have to display   */
    /*  different prompts.                                        */
    /*     For OBJIMBED : ask for imbedded object class           */
    /*         OBJREPEAT : ask for repeat object class            */
    /*         OBJLINK   : ask for linked object class            */
    /*                     ask for link descriptor                */
    /*                     ask if this descriptor is keyed        */
    /*         OBJVLINK  : ask for link descriptor                */
    /*                     ask if this descriptor is keyed        */
    /*         CHAR      : ask for field size                     */
    /*                     ask if this descriptor is keyed        */
    /*         VCHAR      : ask for field size                    */
    /*                     ask if this descriptor is keyed        */
    /*         OTHERS    : ask if this descriptor is keyed        */
    /*------------------------------------------------------------*/
    switch (descriptor->ODM_type)
     {

       /*-------------------------OBJIMBED------------------------------*/
       /*  allocate storage for panel field data                        */
       /*  create a new panel                                           */
       /*  ask for object class to imbed                                */
       /*  set up panel field data                                      */
       /*---------------------------------------------------------------*/
/*
       case OBJIMBED :
           head_field = (struct field_data *) malloc (sizeof
                        (struct field_data));

           new_panel (1, 20 + MAX_OBJ_NAME, 14, CENTER - 30/2,
                   " Imbed Object Descriptor Edit", &outer_panel, &inner_panel);

           wmove   (inner_panel, 0, 0);
           wprintw (inner_panel,"Imbed object class [%-*.*s]", MAX_OBJ_NAME - 1,
                    MAX_OBJ_NAME - 1, descriptor->link_class);

           head_field->start_column = 20;
           head_field->data_width   = MAX_OBJ_NAME - 1;
           break;
*/
       /*--------------------------OBJREPEAT-------------------------*/
       /* allocate storage for 1 panel field                         */
       /* create a new panel                                         */
       /* ask for object class to do a repeat link on                */
       /* set up panel field data                                    */
       /*------------------------------------------------------------*/
/*
       case OBJREPEAT :
           head_field = (struct field_data *) malloc (sizeof
                        (struct field_data));

           new_panel (1, 21 + MAX_OBJ_NAME, 14, CENTER - 30/2,
                  " Repeat Object Descriptor Edit", &outer_panel, &inner_panel);

           wmove   (inner_panel, 0, 0);
           wprintw (inner_panel, "Repeat object class [%-*.*s]",
                    MAX_OBJ_NAME - 1, MAX_OBJ_NAME - 1, descriptor->link_class);

           head_field->start_column = 21;
           head_field->data_width   = MAX_OBJ_NAME - 1;
           break;

*/
       /*--------------------------OBJLINK-----------------------------*/
       /* allocate panel field storage                                 */
       /* create a new panel                                           */
       /* ask for link object class                                    */
       /* ask for link descriptor name                                 */
       /* ask if this descriptor is keyed                              */
       /* set up panel field attributes                                */
       /*--------------------------------------------------------------*/
       case OBJLINK :
           head_field = (struct field_data *) malloc (3 * sizeof
                        (struct field_data));

           new_panel (3, 6 + MAX_ATTR_NAME, 14, 2, 
                      MSGS(CR,cr_obj_desc_edit," Object Link Descriptor Edit"),
                      &outer_panel, &inner_panel);

           wmove   (inner_panel, 0, 0);
           wprintw (inner_panel, MSGS(CR, link_msg1,							"Link object class [%-*.*s]"), MAX_OBJ_NAME - 1,
                    	MAX_OBJ_NAME - 1, descriptor->link_class);

           wmove    (inner_panel, 1, 0);
           wprintw  (inner_panel, MSGS(CR, link_msg2,							"Link     [%-*.*s]"), MAX_ATTR_NAME - 1,
                     	MAX_ATTR_NAME - 1, descriptor->link_descrip);

           wmove    (inner_panel, 2, 0);
           if (descriptor->key == NO_KEY)
               waddstr (inner_panel, MSGS(CR, keymsg1,							"Key descriptor [NO_KEY]"));
           else
               waddstr (inner_panel, MSGS(CR, keymsg2,							"Key descriptor [KEY]   "));

           last_line = 2;
           key_line  = 2;
           head_field->start_column = 19;
           head_field->data_width   = MAX_OBJ_NAME - 1;
           (head_field + 1)->start_column = 6;
           (head_field + 1)->data_width   = MAX_ATTR_NAME - 1;
           (head_field + 2)->start_column = 16;
           (head_field + 2)->data_width   = 1;
           break;

       /*---------------------------OBJVLINK----------------------------*/
       /* allocate storage for 2 fields in this panel                   */
       /* create a new panel centered on the screen                     */
       /* ask for the descriptor to link on name                        */
       /* ask if the link will be a keyed field                         */
       /* set up data attributes of the panel                           */
       /*---------------------------------------------------------------*/
/*
       case OBJVLINK :
           head_field = (struct field_data *) malloc (2 * sizeof
                        (struct field_data));

           new_panel (2, 6 + MAX_ATTR_NAME, 14, 2,
                   MSGS(CR, cr_ln_desc_edit, " Link Descriptor Edit"), 					&outer_panel, &inner_panel);

           wmove     (inner_panel, 0, 0);
           wprintw   (inner_panel, MSGS(CR, link_msg2,							"Link [%-*.*s]"), MAX_ATTR_NAME - 1,
                      	MAX_ATTR_NAME - 1, descriptor->link_descrip);

           wmove     (inner_panel, 1, 0);
           if (descriptor->key == NO_KEY)
             waddstr (inner_panel, MSGS(CR, keymsg1,							"Key descriptor [NO_KEY]"));
           else
             waddstr (inner_panel, MSGS(CR, keymsg2,							"Key descriptor [KEY]   "));

           last_line = 1;
           key_line  = 1;
           head_field->start_column = 6;
           head_field->data_width   = MAX_ATTR_NAME - 1;
           (head_field + 1)->start_column = 16;
           (head_field + 1)->data_width   = 1;
           break;
*/
       /*-------------------------LONGCHAR or CHAR----------------------*/
       /* allocate storage for 2 fields in this panel                   */
       /* create the panel                                              */
       /* print the field size including previously defined ones        */
       /* print whether the field will be keyed                         */
       /* set up the panel field attributes                             */
       /*---------------------------------------------------------------*/
       case LONGCHAR :
       case CHAR :
       case VCHAR:
           head_field = (struct field_data *) malloc (2 * sizeof
                        (struct field_data));

           new_panel (2, 30, 14, CENTER - 30/2, 							MSGS(CR, cr_ch_desc_edit, " Character Descriptor Edit"),
                      	&outer_panel, &inner_panel);

           wmove (inner_panel, 0, 0);
           sprintf (tempstr, MSGS(CR, fieldsize,							"Field size [%-4d]"), descriptor->size - 1);
           wprintw (inner_panel, tempstr);

           wmove (inner_panel, 1, 0);
           if (descriptor->key == NO_KEY)
               waddstr (inner_panel, MSGS(CR, keymsg1,							"Key descriptor [NO_KEY]"));
           else
               waddstr (inner_panel, MSGS(CR, keymsg2,							"Key descriptor [KEY]   "));

           last_line = 1;
           key_line  = 1;
           head_field->start_column = 12;
           head_field->data_width   = 4;
           (head_field + 1)->start_column = 16;
           (head_field + 1)->data_width   = 1;
           break;

       /*---------------------------BINARY------------------------------*/
       /* allocate storage for 1 field in this panel                    */
       /* create the new panel                                          */
       /* ask for the field size of this descriptor                     */
       /* set up the panel field attributes                             */
       /*---------------------------------------------------------------*/
       case BINARY :
           head_field = (struct field_data *) malloc (sizeof
                        (struct field_data));

           new_panel (1, 30, 14, CENTER - 30/2, 							MSGS(CR, cr_bin_desc_edit," Binary Descriptor Edit"),
                      	&outer_panel, &inner_panel),

           wmove   (inner_panel, 0, 0);
           sprintf (tempstr, MSGS(CR, fieldsize,							"Field size [%-4d]"), descriptor->size - 1);
           waddstr (inner_panel, tempstr);

           last_line = 0;
           head_field->start_column = 12;
           head_field->data_width   = 4;
           break;

       /*--------------------DEFAULT, SHORT, LONG, etc---------------------*/
       /* create field attribute storage                                   */
       /* create a new panel                                               */
       /* ask if descriptor is keyed                                       */
       /* set up field attributes                                          */
       /*------------------------------------------------------------------*/
       default :
           head_field = (struct field_data *)malloc(sizeof(struct field_data));

           new_panel (1, 30, 14, CENTER - 30/2, 							MSGS(CR, cr_desc_edit," Descriptor Edit"),
                      	&outer_panel, &inner_panel);

           wmove (inner_panel, 0, 0);
           if (descriptor->key == NO_KEY)
               waddstr (inner_panel, 									MSGS(CR, keymsg1, "Key descriptor [NO_KEY] "));
           else
               waddstr (inner_panel, 									MSGS(CR, keymsg2, "Key descriptor [KEY]    "));

           last_line = 0;
           key_line = 0;
           head_field->start_column = 16;
           head_field->data_width   = 1;
           break;

     } /* end switch */

    touchwin (inner_panel);
    wrefresh (inner_panel);

    cursor_line   = 0;
    cursor_column = head_field->start_column;

    /*-----------------------*/
    /* begin user input      */
    /*-----------------------*/
    for (;;)
     {

        hilite (inner_panel, inner_panel, cursor_line,
                (head_field + cursor_line)->start_column,
                (head_field + cursor_line)->data_width,
                cursor_line, cursor_column, rwhite, white);


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
            /*-------FUNCTION KEY 1 : Continue edit----------*/
            case KEY_NEWL :
	    case KEY_NWL  :
            case KEY_F(1) :
            case ESC1     :
                   /*------------------------------------------------------*/
                   /*  Enter the data currently given into the descriptor  */
                   /*                                                      */
                   /*  while still more lines in this edit panel           */
                   /*   {                                                  */
                   /*      input line into temporary storage               */
                   /*      panel line depends on descriptor.ODM_type       */
                   /*         OBJLINK :                                    */
                   /*           if on the first line of the panel          */
                   /*              store object name                       */
                   /*           else I'm on the second line (link descrip) */
                   /*              store link descriptor                   */
                   /*         CHAR :                                       */
                   /*           store field length                         */
                   /*         VCHAR :                                      */
                   /*           store field length                         */
                   /*   }                                                  */
                   /*------------------------------------------------------*/
                   for (i = 0; i <= last_line; i++)
                    {
                       input_field (inner_panel, i,
                                    (head_field + i)->start_column,
                                    (head_field + i)->data_width, tempstr);

                       switch (descriptor->ODM_type)
                        {
                          case OBJLINK :
                                if (i == 0)
                                   strcpy (descriptor->link_class, tempstr);
                                else if (i == 1)
                                   strcpy (descriptor->link_descrip, tempstr);

				if (strcmp(tempstr, "") == 0)
				{
					insdata = TRUE;
					wmove (msg_wnd, 1,1);
					waddstr (msg_wnd, MSGS(ER, cr_errmsg1,			"INPUT ERROR - Link object class and Link to fields required"));
				} 
               
	                        break;

                          case LONGCHAR :
                          case CHAR     :
                          case BINARY   : 
			  case VCHAR 	:
                                if (i == 0)
                                   descriptor->size = atoi (tempstr) + 1;
				if (strcmp(tempstr, "") == 0)
				{
					insdata = TRUE;
					wmove (msg_wnd, 1,1);
					waddstr (msg_wnd, MSGS(ER, cr_errmsg2,					"INPUT ERROR - Field size is required"));
				} 

                                break;
			  case OBJMETHOD :
			  /* a size of MAX_METHOD_LENGTH is set by ODM   */
				break;

                        } /* end switch */
                    } /* end for */
		   if (insdata == TRUE)
		      {
		      insdata = FALSE;
					wcolorout(msg_wnd, Bxa);
					cbox(msg_wnd);
					wcolorend(msg_wnd);
					touchwin(msg_wnd);
					wrefresh(msg_wnd);
					
					/*-----------------------------------*/
					/* If extended curses mode is off as */
					/* it is for odme, wgetch will       */
					/* return one byte. If a multibyte   */
					/* character is being entered,       */
					/* multiple calls to wgetch are      */
					/* required.                         */
					/*-----------------------------------*/
					for (i = 0; i < MB_LEN_MAX; i++)
					{
						c = wgetch(msg_wnd);
						mbstring[i] = c;
						rc = mblen(mbstring,MB_LEN_MAX);

						if (rc >= 0)
						{
						   mbstring[++i] = '\0';
						   break;
						}

						if (i == (MB_LEN_MAX - 1))
							return (-1);
					}

					wmove (msg_wnd, 0, 0);
					werase (msg_wnd);
					touchwin(msg_wnd);
					wrefresh(msg_wnd);
					touchwin(outer_panel);
					wrefresh(outer_panel);
					touchwin(inner_panel);  
					wrefresh(inner_panel);
		      break;
		      }

                   /*----------------------------------------*/
                   /* free up the data attribute storage     */
                   /* clear and delete the panel             */
                   /*----------------------------------------*/
                   free     ((struct field_data *) head_field);

                   remwin (outer_panel, white);
                   remwin (inner_panel, white);

                   return (CONTINUE);


            /*-------FUNCTION KEY 3 : Discontinue edit----------*/
            case KEY_F(3) :
            case ESC3     :
                   /*----------------------------------------*/
                   /* free up the data attribute storage     */
                   /* clear and delete the panel             */
                   /*----------------------------------------*/
                   free     ((struct field_data *) head_field);

                   remwin (outer_panel, white);
                   remwin (inner_panel, white);

                   return (EXIT);

             /*-----------------KEY DOWN------------------------*/
             case KEY_DOWN :
             case CTRL_N   :
                   if (cursor_line == last_line)
                       cursor_line = 0;
                   else
                       cursor_line++;

                   cursor_column = (head_field + cursor_line)->start_column;
                   break;

             /*-------------------KEY UP---------------------*/
             case KEY_UP :
             case CTRL_P :
                   if (cursor_line == 0)
                       cursor_line = last_line;
                   else
                       cursor_line--;

                   cursor_column = (head_field + cursor_line)->start_column;
                   break;

             /*--------------------DEFAULT--------------------*/
             default :
                   /*--------------------------------------------------*/
                   /*  If cursor column is key column provide an       */
                   /*  extended edit feature where the user hits any   */
                   /*  key and is able to cycle thru key and no key.   */
                   /*--------------------------------------------------*/
                   if (cursor_line == key_line)
                    {
/*
                      switch (descriptor->key)
                       {
                         case NO_KEY :
                               descriptor->key = KEY;
                               wmove   (inner_panel, key_line, 0);
                               waddstr (inner_panel, "Key descriptor [KEY]   ");
                               break;

                         case KEY :
                               descriptor->key = NO_KEY;
                               wmove   (inner_panel, key_line, 0);
                               waddstr (inner_panel, "Key descriptor [NO_KEY]");
                               break;
                       } 
*/
                    }
                   else
                    {
                   std_edit_keys  (inner_panel, c,
                               (head_field + cursor_line)->start_column,
                               (head_field + cursor_line)->data_width,
                               &cursor_column, &cursor_line);
                    }

         } /* end switch */
     } /* end for */
} /* end edit by type */



/****************************************************************/
/*  delete descriptor link : delete this descriptor storage     */
/*                         from the linked list and setup all   */
/*                         the pointers.                        */
/*     PARAMETERS : head_desc_data - pointer to head of linked  */
/*                                    list.                     */
/*                  ptr_desc_data  - pointer to delete storage  */
/****************************************************************/
void delete_descriptor_link (head_desc_data, ptr_desc_data)

struct descriptor_data_old **head_desc_data,
                       **ptr_desc_data;

{
    struct descriptor_data_old  *temp_desc_data;

    temp_desc_data = *ptr_desc_data;

    /*--------------------------------------------------*/
    /* move the descriptor pointer up to the next       */
    /* descriptor if possible otherwise move it back    */
    /*--------------------------------------------------*/
    if (temp_desc_data->next == NULL)
       *ptr_desc_data = temp_desc_data->prev;
    else
       *ptr_desc_data = temp_desc_data->next;

    /*--------------------------------------------------------------------*/
    /*  move the head descriptor up one if I am deleting it               */
    /*  If there is a descriptor before this, link to next descriptor     */
    /*  If there is a descriptor after this, link to previous descriptor  */
    /*--------------------------------------------------------------------*/

    if (*head_desc_data == temp_desc_data)
       *head_desc_data = temp_desc_data->next;

    if (temp_desc_data->next != NULL)
        (temp_desc_data->next)->prev = temp_desc_data->prev;
    if (temp_desc_data->prev != NULL)
        (temp_desc_data->prev)->next = temp_desc_data->next;

    free ((struct descriptor_data_old *) temp_desc_data);

} /* end delete descriptor link */



/****************************************************************/
/*  add descriptor link :  add a descriptor to the linked list  */
/*                         setup all the pointers               */
/*     PARAMETERS : head_desc_data - pointer to head of linked  */
/*                                    list.                     */
/*                  ptr_desc_data  - pointer to descriptor      */
/*                         preceding addition; on return it     */
/*                         points to the new descriptor.        */
/****************************************************************/
void add_descriptor_link (head_desc_data, ptr_desc_data)

struct descriptor_data_old **head_desc_data,
                       **ptr_desc_data;
{
   struct descriptor_data_old *prev_desc_data;

   prev_desc_data = *ptr_desc_data;

   *ptr_desc_data = (struct descriptor_data_old *) malloc
                     (sizeof (struct descriptor_data_old));

    (*ptr_desc_data)->descriptor.iterator    = 1;
    (*ptr_desc_data)->descriptor.size        = 2;
    (*ptr_desc_data)->descriptor.key         = NO_KEY;
    (*ptr_desc_data)->descriptor.descrip_name[0] = '\0';
    (*ptr_desc_data)->descriptor.link_class[0]  = '\0';
    (*ptr_desc_data)->descriptor.ODM_type        = CHAR;

    strcpy ((*ptr_desc_data)->header, "ODM_CHAR");

    /*-------------------------------------------------------------------*/
    /*  if a descriptor link precedes the new descriptor                 */
    /*   {                                                               */
    /*     if there is to be a link after the new descriptor             */
    /*        link the descriptor after the new, to the new descriptor   */
    /*     link the new descriptor to the descriptor after               */
    /*     link the previous descriptor to the new descriptor            */
    /*     link the new descriptor to the previous descriptor            */
    /*   }                                                               */
    /*  else we are starting at the head descriptor link                 */
    /*   {                                                               */
    /*     point the head descriptor to the new descriptor               */
    /*     setup NULL links                                              */
    /*   }                                                               */
    /*-------------------------------------------------------------------*/
    if (prev_desc_data != NULL)
     {
        if (prev_desc_data->next != NULL)
            (prev_desc_data->next)->prev = *ptr_desc_data;

        (*ptr_desc_data)->next = prev_desc_data->next;
        prev_desc_data->next = *ptr_desc_data;
        (*ptr_desc_data)->prev = prev_desc_data;
     }
    else
     {
        *head_desc_data = *ptr_desc_data;
        (*ptr_desc_data)->next = NULL;
        (*ptr_desc_data)->prev = NULL;
     }

} /* end add descriptor link */



/**********************************************************************/
/* print descriptors  :  print a screenful of descriptors.            */
/*                                                                    */
/*         PARAMETERS :  descriptor_window - the window to print in   */
/*                       ptr_desc_data - the descriptor to start at   */
/**********************************************************************/
void print_descriptors (descriptor_window, ptr_desc_data)

WINDOW *descriptor_window;
struct descriptor_data_old *ptr_desc_data;
{
   register i;

   wmove  (descriptor_window, 0, 0);
   werase (descriptor_window);

   /*----------------------------------------------------------------*/
   /* DESCRIPTOR WINDOW display begins : Display the descriptor name */
   /*                   and descriptor.ODM_type                      */
   /*----------------------------------------------------------------*/
   for (i = 0; ((i < MAX_CREATE_ITEMS) && (ptr_desc_data != NULL));
        ptr_desc_data = ptr_desc_data->next, i++)
    {
       wmove   (descriptor_window, i, 0);
       wprintw (descriptor_window, "%-*.*s  (%s)", MAX_DESC_NAME,
                MAX_DESC_NAME, ptr_desc_data->descriptor.descrip_name,
                ptr_desc_data->header);
    }  /* end for */

} /* end print descriptors */

