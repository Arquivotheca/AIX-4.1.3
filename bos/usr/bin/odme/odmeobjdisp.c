static char sccsid[] = "@(#)71	1.15  src/bos/usr/bin/odme/odmeobjdisp.c, cmdodm, bos411, 9435A411c 8/30/94 20:45:21";
/*
 * COMPONENT_NAME: (ODME) ODMEOBJDISP - display and edit object class data
 *
 * FUNCTIONS: object_display
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
/*   FILE NAME :    odmeobjdisp.c                                     */
/*                                                                    */
/*   DESCRIPTION :  This file contains Object Data Manager            */
/*                  Editor object class display routines.             */
/*                                                                    */
/*   CONTENTS :                                                       */
/*      void object_display ();  display the objects in the class     */
/*                                                                    */
/**********************************************************************/

#include "odme.h"
#include <mbstr.h>

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


/****************************************************************/
/*  object display                                              */
/*       This function displays the object class searched on.   */
/*       This is a recursive function called for the display    */
/*       objects.                                               */
/*                                                              */
/*    Parameters :                                              */
/*       objhandle            - the object class handle         */
/*       objs                 - the object search string        */
/*       head_descriptor_data - a ptr to the head descriptor    */
/*                              for the object class            */
/****************************************************************/
void object_display (objhandle, objs, head_descriptor_data)

struct objtoken         *objhandle;
char *objs;
struct descriptor_data  *head_descriptor_data;
{
    void  copy_data           ();
    int   find_field          ();
    void  free_object_storage ();
    void  get_descriptors     ();
    int   odm_get_objects         ();
    void  input_field         ();
    void  input_object        ();
    void  odm_get_object_storage  ();
    int   print_objects       ();

    WINDOW *box_window        = NULL,           /* outer border window   */
           *key_window        = NULL,           /* function keys window  */
           *descriptor_window = NULL,           /* descriptor window     */
           *descriptor_view   = NULL,           /* view of descriptors   */
           *object_window     = NULL,           /* objects window        */
           *object_view       = NULL,           /* view of objects       */
           *stat_window       = NULL;           /* stat window           */

    struct descriptor_data *ptr_descriptor_data = NULL;

    struct object_data *head_object_data = NULL,
                       *ptr_object_data  = NULL,
                       *temp_object_data = NULL;
	char **fptr,
		  **fptr1;

    int c,                          /* input character                        */
        cursor_col    = 0,          /* current cursor column                  */
        cursor_line   = 0,          /* current cursor line                    */
        delete_count  = 0,          /* number of deleted objects              */
        delete_length = DELETE_OBJECTS,
        field         = 0,          /* current highlighted field              */
        *head_delete  = NULL,       /* object id's of objects to delete       */
        length        = 0,          /* length of screen heading               */
        maxhorizontal = 0,          /* maximum horizontal pages               */
        more          = TRUE,       /* are more objects in the object class?  */
        object_count  = 0,          /* number of objects currently gotten     */
        object_number = 0,          /* current object number cursor is on     */
        temp,                       /* temp variable for simple computations  */
        whichone      = TRUE,      /* object to return from ObjectGet        */
        width;                      /* width of screen image                  */

    short int
        delta         = FALSE,      /* have changes occurred ?                */
        endflag       = FALSE,      /* has user selected end ?                */
        hscreen       = 0,          /* current horizontal screen number       */
        srefresh      = FALSE;      /* refresh statistics window ?            */

    register i, j, k;               /* for loop variables                     */
	char *data_start,
		*data_ptr;
	char temp_name[16];	    /* temporary holder for long names        */

    /*--------------------------------------------------------------*/
    /* Determine startcolumn for each object descriptor and         */
    /* the total width of the object.                               */
    /*                                                              */
    /*   Start on first column;                                     */
    /*   while (still more descriptors) do                          */
    /*       calculate number of columns used in current screen;    */
    /*       If ((current width + descriptor width) >               */
    /*           number of columns in a screen)                     */
    /*              add columns left in screen to width;            */
    /*       save startcolumn;                                      */
    /*       add largest descriptor width to width size;            */
    /*                                                              */
    /*--------------------------------------------------------------*/
    width = 0;
    for (i = 0, ptr_descriptor_data = head_descriptor_data;
         i < objhandle->number_desc; i++, ptr_descriptor_data++)
     {
        temp = width % MAX_DISPLAY_COLS;
        if ((temp + ptr_descriptor_data->header_width) > MAX_DISPLAY_COLS)
            width += MAX_DISPLAY_COLS - temp;

        ptr_descriptor_data->start_column = width;

        if (ptr_descriptor_data->header_width > ptr_descriptor_data->data_width)
           width += ptr_descriptor_data->header_width + GUTTER;
        else
           width += ptr_descriptor_data->data_width + GUTTER;
     }
    width -= GUTTER;
    if (( width % MAX_DISPLAY_COLS) != 0)
        width += MAX_DISPLAY_COLS - (width % MAX_DISPLAY_COLS );

    /*---------------------------------------------------------------*/
    /* Curses windowing begins :                                     */
    /*     b ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤ   TEXT  ฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฟ  */
    /*     o ณ stat window                                        ณ  */
    /*     x ฬออออออออออออออออออออออออออออออออออออออออออออออออออออน  */
    /*       บ descriptor window                                  บ  */
    /*     w ฬออออออออออออออออออออออออออออออออออออออออออออออออออออน  */
    /*     i บ object window with view                            บ  */
    /*     n บ                                                    บ  */
    /*     d บ                                                    บ  */
    /*     o ศออออออออออออออออออออออออออออออออออออออออออออออออออออผ  */
    /*     w   function key window                                   */
    /*---------------------------------------------------------------*/
    /* create the outer box window for a border                      */
    /*---------------------------------------------------------------*/
    box_window = newwin (LINES - 2, 0, 0, 0);
    box_window->_csbp = NORMAL;
    wmove     (box_window, 0, 0);

    /* wcolorout (box_window, white); */
    werase    (box_window);

    wcolorout(box_window, Bxa);
    drawbox (box_window, 0, 0, 3, COLS);
    drawbox (box_window, 2, 0, 4, COLS);
    drawbox (box_window, 5, 0, LINES - 7, COLS);
    wcolorend(box_window);

    length = strlen (MSGS(OD,DISPLAY_NAME,DF_DISPLAY_NAME) );
    wmove     (box_window, 0, CENTER - (length/2));
    waddstr   (box_window, MSGS(OD,DISPLAY_NAME,DF_DISPLAY_NAME) );

    touchwin  (box_window);
    box_window->_csbp = NORMAL;
    wrefresh  (box_window);

    /*-------------------------------------------------------------*/
    /* create a table name, object #, descriptor, 1 line window    */
    /*-------------------------------------------------------------*/

    stat_window = newwin (1, MAX_DISPLAY_COLS, 1, GUTTER);
    stat_window->_csbp = NORMAL;
    mbsncpy (temp_name, objhandle->objname,(size_t)15);
    wmove   (stat_window, 0, 0);
    wprintw (stat_window, MSGS(OD,od_objclass,
             "Object Class : %-*.*s  Object: %-5d  Descriptor: %-3d of  %-3d"),
             16, MAX_OBJ_NAME - 1, temp_name,
             object_number + 1, field + 1, objhandle->number_desc);

    touchwin (stat_window);
    stat_window->_csbp = NORMAL;
    wrefresh (stat_window);

    /*----------------------------------------------------------*/
    /* Create a bottom key window 2 lines deep                  */
    /*----------------------------------------------------------*/

    key_window = newwin (2, 0, LINES - 2, 0);
    key_window->_csbp = NORMAL;
    wmove     (key_window, 0, 0);

    /* wcolorout (key_window, white); */
    werase    (key_window);

    wmove     (key_window, 0, 0);
    waddstr   (key_window, MSGS(KS,DISPLAY_KEY_1,DF_DISPLAY_KEY_1) );
    wmove     (key_window, 1, 0);
    waddstr   (key_window, MSGS(KS,DISPLAY_KEY_2,DF_DISPLAY_KEY_2) );

    touchwin  (key_window);
    key_window->_csbp = NORMAL;
    wrefresh  (key_window);

    /*----------------DESCRIPTORS WINDOW: top heading-------------*/
    descriptor_window = newwin (2, width, 3, GUTTER);
    descriptor_window->_csbp = NORMAL;
    descriptor_view  = newview (descriptor_window, 2, MAX_DISPLAY_COLS);

    /*-------------------------------------------------------*/
    /*  Build top object descriptor window by traversing     */
    /*  descriptor data and printing header information.     */
    /*-------------------------------------------------------*/
    for (i = 0, ptr_descriptor_data = head_descriptor_data;
         i < objhandle->number_desc; ptr_descriptor_data++, i++)
     {
        wmove   (descriptor_window, 0, ptr_descriptor_data->start_column);
        wprintw (descriptor_window, "%-*.*s", ptr_descriptor_data->header_width,
                 ptr_descriptor_data->header_width, ptr_descriptor_data->
                 classp->elem[i].elemname);
        wmove   (descriptor_window, 1, ptr_descriptor_data->start_column);
        wprintw (descriptor_window, "%s", ptr_descriptor_data->header);
     }

    touchwin (descriptor_view);
    descriptor_view->_csbp = NORMAL;
    wrefresh (descriptor_view);

    /*-------------------OBJECTS WINDOW---------------------------*/

    object_window = newwin  (MAX_DISPLAY_ITEMS, width, 6, GUTTER);
    object_window->_csbp = NORMAL;
    object_view   = newview (object_window, MAX_DISPLAY_ITEMS,
                             MAX_DISPLAY_COLS);
    wmove     (object_view, 0, 0);
    /* wcolorout (object_view, white); */

    werase    (object_view);
    wmove     (object_view, 3, 5);
    waddstr   (object_view, MSGS(OD,loading,"LOADING...") );

    touchwin  (object_view);
    object_view->_csbp = NORMAL;
    wrefresh  (object_view);

    wmove     (object_view, 0, 0);
    werase    (object_view);

    /*--------------------------------------------------------*/
    /*  get objects from current object class and store them  */
    /*  in head_object_data malloc'd storage                  */
    /*--------------------------------------------------------*/

    if ((object_count = odm_get_objects (objhandle, objs, &head_object_data,
         head_descriptor_data, &whichone, &more)) < 0)
     {
        remwin (stat_window, white);
        remwin (key_window,  white);
        remwin (box_window,  white);
	

        delwin (descriptor_window);
        delwin (object_window);
	delwin (descriptor_view);
	delwin (object_view);

        return;
     }
    if (object_count == 0)
     {
	object_number = -1;
	srefresh = TRUE;
        cursor_line = -1;
        prerr (MSGS(ER,OD_NO_OBJECTS,DF_OD_NO_OBJECTS), 0, FALSE);
     }

    ptr_object_data = head_object_data;

    /*-----------------------------------------------------------*/
    /*  print the current objects for one screen                 */
    /*-----------------------------------------------------------*/
    print_objects (object_window, objhandle->number_desc, ptr_object_data,
                   head_descriptor_data);

    touchwin (object_view);
	object_view->_csbp = NORMAL;
    wrefresh (object_view);

    /*------------------------------------------------------------*/
    /*  allocate storage for deleted line object ids.             */
    /*------------------------------------------------------------*/
    if ((head_delete = (int *) malloc (delete_length * sizeof (long))) == NULL)
        prerr (MSGS(ER,OD_MALLOC_FAILED,DF_OD_MALLOC_FAILED), 0, TRUE);

    maxhorizontal = width / MAX_DISPLAY_COLS - 1;

	/*----------------------------------------------*/
	/* clear the hft input buffer                   */
	/*----------------------------------------------*/
	flushinp();


  /**************************************************/
  /*  IMPORTANT :  USER INPUT begins.               */
  /**************************************************/
  for (;;)
   {
     /*---------------------------------------------------------------*/
     /*  Check and see if we need to get some more objects from ODM   */
     /*      Move to the last object that we have.                    */
     /*      get the next group of objects;                           */
     /*---------------------------------------------------------------*/
     if (((object_number + (2 * MAX_DISPLAY_ITEMS)) >= object_count) && more)
      {
         for(temp_object_data = ptr_object_data; temp_object_data->next != NULL;
              temp_object_data = temp_object_data->next);

         if((object_count += odm_get_objects(objhandle, objs, &temp_object_data,
              head_descriptor_data, &whichone, &more)) < 0)
                 prerr(MSGS(ER,OD_BAD_RETURN,DF_OD_BAD_RETURN), odmerrno, TRUE);
      }
     /*-------------------------------------------------------------*/
     /*  Refresh the statistics window if we have moved to a new    */
     /*  descriptor or object                                       */
     /*-------------------------------------------------------------*/
     if (srefresh)
      {
         wmove   (stat_window, 0, 0);
         wprintw (stat_window, MSGS(OD,od_objclass,
             "Object Class : %-*.*s  Object: %-5d  Descriptor: %-3d of  %-3d"),
             16, MAX_OBJ_NAME - 1, temp_name,
             object_number + 1, field + 1, objhandle->number_desc);
         touchwin (stat_window);
			stat_window->_csbp = NORMAL;
         wrefresh (stat_window);
         srefresh = FALSE;
      }

     if (cursor_line >= 0)
	{
         hilite (object_window, object_view, cursor_line,
                 (head_descriptor_data + field)->start_column,
                 (head_descriptor_data + field)->data_width,
                 cursor_line, cursor_col % MAX_DISPLAY_COLS, rwhite, white);
	}
	else
	{
		object_view->_csbp = NORMAL;
		wrefresh (object_view);
	}

     /*-----------------------------------------------------*/
     /*  These routines are rather slow, so to prevent too  */
     /*  much type ahead flush the keyboard input           */
     /*-----------------------------------------------------*/
     flushinp (); 

	if (REPLAY)
		{
		c = get_char();
		}
	else
		{
	c = ms_input(object_view);
	if (TRAC) save_char(c);
		}

     switch (c)
      {

        /*---------------------HELP PRESSED----------------------*/
        case KEY_F(1) :
        case ESC1     :
		help (MSGS(OD,display_help,
              "Display help \n\
                                                 \n\
 The Display screen allows the user to browse and edit objects  \n\
 in the object class.                                           \n\
 If the object class has been opened in read/write mode the user \n\
 can add objects (<Esc>4 or PF4), copy objects (<Esc>6 or PF6), \n\
 delete objects (<Esc>5 or PF5), and edit the current object.   \n\
 On exit (<Esc>3 or PF3) you will be asked if you want to commit \n\
 any changes you may have made.                                 \n\
 NOTE*  ODM always makes changes at the end of the object class, \n\
        so relative positions of objects may change.            \n\
") );

		touchwin (box_window);
		box_window->_csbp = NORMAL;
		wrefresh (box_window);
		touchwin (stat_window);
		stat_window->_csbp = NORMAL;
		wrefresh (stat_window);
		touchwin (key_window);
		key_window->_csbp = NORMAL;
		wrefresh (key_window);
   		touchwin (descriptor_view);
		descriptor_view->_csbp = NORMAL;
		wrefresh (descriptor_view);
		touchwin (object_view);
		object_view->_csbp = NORMAL;
		wrefresh (object_view);

              break;

        /*-----------------EXPAND to next object class-------------------*/
        /* This function key will cause a recursive call to the current  */
        /* function.  The purpose of this call is the display of the     */
        /* next object linked on by OBJLINK, OBJVLINK and OBJVLINKCOL,   */
        /* or OBJREPEAT.  The next object is opened, a search string is  */
        /* built, the descriptors are returned and a call is made.       */
        /*---------------------------------------------------------------*/
        case ESC2:
        case KEY_F(2):
              /*------------------------------------------------*/
              /*  if we have no objects produce beep and break  */
              /*------------------------------------------------*/

              if (ptr_object_data == NULL)
               {
                  beep ();
                  break;
               }

              /*------------------------------------------------------------*/
              /*  Update the current object line if changes have occurred.  */
              /*------------------------------------------------------------*/

              if (ptr_object_data->update == TRUE)
               {
                 input_object (object_window,cursor_line,objhandle->number_desc,
                               ptr_object_data, head_descriptor_data);
                 ptr_object_data->update = FALSE;
               }


               /***********OBJECT EXPAND BEGINS*************/

               {
                 struct objtoken         nexthandle; 
                 char  *nextsrch[255];   
                 struct descriptor_data  *next_descriptor_data;

                 int  error = FALSE;
                 char searchdata[MAX_LINK_LENGTH];

                 /*-------------------------------------------------*/
                 /* Check current field to see if it has a link to  */
                 /* another object.                                 */
                 /* Then copy the link names from the fields        */
                 /*-------------------------------------------------*/
                 switch (head_descriptor_data->classp->elem[field].type)
                  {

                    case OBJLINK :
			strcpy (nexthandle.objname, 
				head_descriptor_data->classp->elem[field].link->classname);
			nextsrch[0] = '\0';
                        break;
 

                    default :
                        beep ();
                        prerr (MSGS(ER,OD_NOT_A_LINK,DF_OD_NOT_A_LINK),0,FALSE);
                        error = TRUE;
                        break;

                  }   

                 if (error) break;


                 strcpy (nexthandle.objrep_path, objhandle->objrep_path);
                 strcpy (nexthandle.node, objhandle->node);
		 nexthandle.lock = objhandle->lock;

                 /*----------------------------------------------------*/
                 /*  OPEN of object linked on                          */
                 /*----------------------------------------------------*/

		if(open_object_class(&nexthandle) < 0)
                {
                     prerr (MSGS(ER,OD_EXPAND_OPEN,DF_OD_EXPAND_OPEN),
                            odmerrno, FALSE);
                }
                else
                {

                     /*-------------------------------------------------------*/
                     /* Return all object classes with ObjectGdescp.  The     */
                     /* purpose is to match the current descriptor name       */
                     /* against the descriptor in the next object.  Once      */
                     /* there is a match we can tell if this descriptor is    */
                     /* of type char (meaning the search string should have   */
                     /* quotes around it as in  a = 'two') or this descriptor */
                     /* is a numeral (meaning the search string should be     */
                     /* written as in  a = 2)                                 */
                     /*-------------------------------------------------------*/

                     get_descriptors (&nexthandle, &next_descriptor_data);

                     /*----------------------------------------------------*/
                     /*  RECURSIVE call to current routine to display next */
                     /*            object.                                 */
                     /*----------------------------------------------------*/

                     object_display (&nexthandle,nextsrch,next_descriptor_data);

		     touchwin (box_window);
		     box_window->_csbp = NORMAL;
		     wrefresh (box_window);

		     touchwin (stat_window);
		     stat_window->_csbp = NORMAL;
		     wrefresh (stat_window);

		     touchwin (key_window);
		     key_window->_csbp = NORMAL;
		     wrefresh (key_window);

   		     touchwin (descriptor_view);
		     descriptor_view->_csbp = NORMAL;
		     wrefresh (descriptor_view);

		     touchwin (object_view);
		     object_view->_csbp = NORMAL;
		     wrefresh (object_view);

                     if (odm_close_class (nexthandle.objptr) != 0)
                         prerr (MSGS(ER,OD_CLOSE_FAILED,DF_OD_CLOSE_FAILED),
                                 odmerrno, FALSE);
                     free (next_descriptor_data);

                  } 
               }
             break;



       /*---------------EXIT DISPLAY OBJECTS------------------*/
       case ESC3:
       case KEY_F(3):
              /*------------------------------------------------------------*/
              /*  Update the current object line if changes have occurred.  */
              /*------------------------------------------------------------*/
              if((ptr_object_data != NULL) && (ptr_object_data->update == TRUE))
               {
                 input_object (object_window,cursor_line,objhandle->number_desc,
                               ptr_object_data, head_descriptor_data);
                 ptr_object_data->update = FALSE;
               }


              if ((delta == TRUE) && (question (
		MSGS(OD,commit,"COMMIT changes ?") ) == TRUE))
               {
                  char *commitsearch;
                  char   intochar[LONGWIDTH + 1];

                  touchwin (object_view);
	          object_view->_csbp = NORMAL;
                  wrefresh (object_view);


                  /*----------------------------------------------*/
                  /*  process deletes from object class           */
                  /*----------------------------------------------*/
                  for (i = 0; i < delete_count; i++)
                   {

			if ( (odm_rm_by_id(head_descriptor_data->classp,
				*(long *)(head_delete + i))) < 0)
                           prerr (MSGS(ER,OD_DELETE_FAILED,DF_OD_DELETE_FAILED),
                                   odmerrno, FALSE);

                   }   /* end for {process deletes } */


		/*---------------------------------------------------------*/
		/* allocate memory to hold the data from the odm_get_obj   */
		/*---------------------------------------------------------*/
		data_start = data_ptr = (char *) malloc (head_descriptor_data->classp->structsize);

                  /*-----------------------------------------------*/
                  /* process ADDS, CHANGES                         */
                  /*-----------------------------------------------*/
                  for (i = 0, temp_object_data = head_object_data;
                       temp_object_data !=NULL;
                       temp_object_data = temp_object_data->next, i++)
                   {
                     if (temp_object_data->modify_type == NOCHG) continue;

                     for (j = 0, fptr = temp_object_data->field_array;
                     j < head_descriptor_data->classp->nelem; j++, fptr++)
                     {
                       switch (head_descriptor_data->classp->elem[j].type)
                       {

                /*----------------CHARACTER DATA------------------*/
                /* These data types should be held in character   */
                /* format with null endings.                      */
                /*------------------------------------------------*/
                case CHAR :
                case LONGCHAR :
                case OBJMETHOD : 
                      strcpy ((char *)(data_ptr +
			head_descriptor_data->classp->elem[j].offset), *fptr);
			break;

                case OBJLINK :
                      strcpy ((char *)(data_ptr + 
			head_descriptor_data->classp->elem[j].offset
			 + 2 * sizeof(char *)), *fptr);
			break;
                case VCHAR :
                      *(char **)(data_ptr + head_descriptor_data->classp->elem[j].offset) = *fptr;
/*
                      strcpy (*(char **)(data_ptr + 
			head_descriptor_data->classp->elem[j].offset), *fptr);
*/
			break;


                /*----------------INTEGER DATA-----------------------*/
                /* These data types are of integer and should be     */
                /* used to set their corresponding pointer locations */
                /*---------------------------------------------------*/
                case SHORT :
                      *(short *)(data_ptr +
			head_descriptor_data->classp->elem[j].offset) = *(short *) *fptr;
                      break;

                case LONG :
                      *(long *)(data_ptr +
			head_descriptor_data->classp->elem[j].offset)= *(long *) *fptr;
                      break;

                /*------------------NULL DATA------------------------*/
                /* will never hold anything from the screen          */
                /*---------------------------------------------------*/
/*
                case OBJREPEAT :
                      *fptr = (char *) NULL;
                      break;
*/
                /*----------------------BINARY----------------------*/

             }  /* end switch */
        } /* end for */

                     switch (temp_object_data->modify_type)
                      {
                       case ADD :
                         if ((temp_object_data->object_id =
			     odm_add_obj(head_descriptor_data->classp,                                          data_start)) < 0)
                         prerr (MSGS(ER,OD_ADD_FAILED,DF_OD_ADD_FAILED),
                                     odmerrno, FALSE);
                         break;

                       case CHANGE :
			/*--------------------------------------*/
			/* store the id for the change          */
			/*--------------------------------------*/
			*(long *) data_start = temp_object_data->object_id;
			if ((odm_change_obj(head_descriptor_data->classp,                                    data_start)) < 0)
                        prerr (MSGS(ER,OD_CHANGE_FAILED,DF_OD_CHANGE_FAILED),
                             odmerrno, FALSE);
                        break;

                      } /* end switch {add or change}      */
                   } /* end for {process adds, changes} */

              /*--------------------------------------------*/
              /*  Free up object storage and proceed to     */
              /*  exit object.                              */
              /*--------------------------------------------*/
/*
             for (ptr_object_data = head_object_data;
                  ptr_object_data != NULL;
                  ptr_object_data = ptr_object_data->next)
              {
                  free_object_storage (objhandle->number_desc,&ptr_object_data);
              }
*/
	     free (data_start);

 	     free (head_object_data);
	     head_object_data = NULL;

            }  /* end delta == true */

             remwin (box_window,  white);
             remwin (stat_window, white);
             remwin (key_window,  white);

             delwin   (descriptor_window);

             delwin   (object_window);

             return;



       /*-------------ADD AN OBJECT----------------*/
       case ESC4:
       case CTRL_O:
       case KEY_F(4):
             /*------------------------------------------------------------*/
             /*  Update the current object line if changes have occurred.  */
             /*------------------------------------------------------------*/
             if ((ptr_object_data != NULL) && (ptr_object_data->update == TRUE))
              {
                input_object(object_window, cursor_line, objhandle->number_desc,
                              ptr_object_data, head_descriptor_data);
                ptr_object_data->update = FALSE;
              }

             /*-------------check for read only opened objects----------*/
             if (READONLY)
              {
                 beep ();
                 prerr (MSGS(ER,OD_READ_ONLY,DF_OD_READ_ONLY),0, FALSE);
                 break;
              }


             cursor_col = head_descriptor_data->start_column;
             field = 0;

             for (;(cursor_col / MAX_DISPLAY_COLS) != hscreen;)
              {
                hscreen--;
                vscroll (object_view, 0, -MAX_DISPLAY_COLS);
                vscroll (descriptor_view, 0, -MAX_DISPLAY_COLS);
              }
					descriptor_view->_csbp = NORMAL;
             wrefresh (descriptor_view);

             /*-----------------------------------------------------------*/
             /*  Create a new object                                      */
             /*                                                           */
             /*    allocate storage for new object;                       */
             /*                                                           */
             /*    link new object to previous object;                    */
             /*    link new object to the next object;                    */
             /*    if an object follows the newly created one             */
             /*       link the object that follows to the new object;     */
             /*    link the previous object to the new object;            */
             /*                                                           */
             /*    modify type = ADD;                                     */
             /*    this row currently does not need updating;             */
             /*    set a dummy object id type;                            */
             /*                                                           */
             /*-----------------------------------------------------------*/

             odm_get_object_storage (objhandle->number_desc, &temp_object_data,
                                 head_descriptor_data);
             if (ptr_object_data == NULL)
              {
                 ptr_object_data = temp_object_data;
                 head_object_data = temp_object_data;
                 temp_object_data->prev = NULL;
                 temp_object_data->next = NULL;
              }
             else
              {
                 temp_object_data->prev = ptr_object_data;
                 temp_object_data->next = ptr_object_data->next;
                 if (ptr_object_data->next != NULL)
                     (ptr_object_data->next)->prev = temp_object_data;

                 ptr_object_data->next = temp_object_data;
              }

             temp_object_data->modify_type = ADD;
             temp_object_data->update = FALSE;
             temp_object_data->object_id = 0;

             ptr_object_data = temp_object_data;

             /*----------------------------------------------------------*/
             /*  increment object count;                                 */
             /*  increment object number;                                */
             /*  increment window line;                                  */
             /*                                                          */
             /*  if the last line on the page;                           */
             /*     window line = first line;                            */
             /*     redisplay page starting with the object copied from; */
             /*  else                                                    */
             /*     insert a line;                                       */
             /*                                                          */
             /*  point to the newly created object;                      */
             /*----------------------------------------------------------*/
             object_count++;
             object_number++;
             cursor_line++;

             if (cursor_line >= MAX_DISPLAY_ITEMS)
              {
                 cursor_line = 0;
                 print_objects (object_window, objhandle->number_desc,
                                ptr_object_data, head_descriptor_data);
              }
             else
              {
                 wmove (object_window, cursor_line, cursor_col);
                 winsertln (object_window);
              }

             srefresh = TRUE;
             delta = TRUE;
             break;



       /*--------------DELETE AN OBJECT------------------*/
       case ESC5:
       case KEY_F(5):
             /*------------------------------------------------*/
             /*  if we have no objects produce beep and break  */
             /*------------------------------------------------*/
             if (ptr_object_data == NULL)
              {
                 beep ();
                 break;
              }

             /*-------------check for read only opened objects----------*/
             if (READONLY)
              {
                 beep ();
                 prerr (MSGS(ER,OD_READ_ONLY,DF_OD_READ_ONLY), 0, FALSE);
                 break;
              }

             /*-------------------------------------------------------------*/
             /*  if delete storage is full                                  */
             /*   {                                                         */
             /*     increment delete space length;                          */
             /*     realloc delete storage with new length;                 */
             /*   }                                                         */
             /*  if this object exists in the object class                  */
             /*  (i.e. is not one that the user has added in this session)  */
             /*   {                                                         */
             /*      store the object id in delete storage;                 */
             /*      increment the deleted objects count;                   */
             /*    }                                                        */
             /*-------------------------------------------------------------*/
             if (delete_count >= delete_length)
              {
                delete_length += DELETE_OBJECTS;
                if ((head_delete = (int *) realloc (head_delete,
                   delete_length * sizeof (long))) == NULL)
                 {
                    prerr(MSGS(ER,OD_DELETE_MALLOC,DF_OD_DELETE_MALLOC),0,TRUE);
                 }
              }

             if (ptr_object_data->modify_type != ADD)
              {
                 *(head_delete + delete_count) = ptr_object_data->object_id;
                 delete_count++;
              }

             /*------------------------------------------------------*/
             /*  if an object precedes this deleted object           */
             /*     point the previous object's next pointer to the  */
             /*        next object                                   */
             /*                                                      */
             /*  if an object follows this deleted object            */
             /*   {                                                  */
             /*     temp object = delete object;                     */
             /*     current object moves to next object;             */
             /*     if the delete object had an object before it     */
             /*        point the previous pointer to the object      */
             /*        after the object to delete                    */
             /*     point the next object to the previous object     */
             /*   }                                                  */
             /*  else nothing follows                                */
             /*   {                                                  */
             /*     temp object = delete object;                     */
             /*     current object moves back one object             */
             /*     if this is a valid object                        */
             /*        point the next pointer to NULL                */
             /*                                                      */
             /*     delete this last line                            */
             /*     decrement the window line                        */
             /*   }                                                  */
             /*                                                      */
             /*  free the object storage for delete object;          */
             /*------------------------------------------------------*/
             if (ptr_object_data->next != NULL)
              {
                temp_object_data = ptr_object_data;
                ptr_object_data = ptr_object_data->next;
                if (temp_object_data->prev != NULL)
                   (temp_object_data->prev)->next = temp_object_data->next;

                (temp_object_data->next)->prev = temp_object_data->prev;
              }
             else
              {
                temp_object_data = ptr_object_data;
                ptr_object_data = ptr_object_data->prev;

                if (ptr_object_data != NULL)
                    ptr_object_data->next = NULL;

                wdeleteln (object_window);
		cursor_line--;
                object_number--;
              }

             free_object_storage (objhandle->number_desc, &temp_object_data);

		if (ptr_object_data != NULL)
			{
             if (cursor_line == -1)
              {
                 for (i = 0, temp_object_data = ptr_object_data;
                      ((i < MAX_DISPLAY_ITEMS - 1) &&
                      (temp_object_data->prev != NULL));
                      i++, temp_object_data = temp_object_data->prev);

		 if (object_number >= 0)
		 {
	     	    cursor_line = i;
		 }
			
                 print_objects (object_window, objhandle->number_desc,
                                temp_object_data, head_descriptor_data);

	       }
             else
	       { 
                for (i = 0, temp_object_data = ptr_object_data;
                  ((i < cursor_line) && (temp_object_data->prev != NULL));
                  i++, temp_object_data = temp_object_data->prev);

                 print_objects (object_window, objhandle->number_desc,
                           temp_object_data, head_descriptor_data);
	       }
		} /* end ptr_object_data = NULL */
             object_count--;
             srefresh = TRUE;
             delta = TRUE;

             break;



       /*---------------COPY THE OBJECT------------------*/
       case ESC6:
       case KEY_F(6):
             /*------------------------------------------------*/
             /*  if we have no objects produce beep and break  */
             /*------------------------------------------------*/
             if (ptr_object_data == NULL)
              {
                 beep ();
                 break;
              }

             /*-------------check for read only opened objects----------*/
             if (READONLY)
              {
                 beep ();
                 prerr (MSGS(ER,OD_READ_ONLY,DF_OD_READ_ONLY), 0, FALSE);
                 break;
              }

             /*------------------------------------------------------------*/
             /*  Update the current object line if changes have occurred.  */
             /*------------------------------------------------------------*/
             if (ptr_object_data->update == TRUE)
              {
                input_object(object_window, cursor_line, objhandle->number_desc,
                              ptr_object_data, head_descriptor_data);
                ptr_object_data->update = FALSE;
              }

             /*-----------------------------------------------------------*/
             /*  Create a new object and copy the data from the current   */
             /*  window line to the object line and the next window line. */
             /*                                                           */
             /*    allocate storage for new object;                       */
             /*                                                           */
             /*    link new object to previous object;                    */
             /*    link new object to the next object;                    */
             /*    if an object follows the newly created one             */
             /*       link the object that follows to the new object;     */
             /*    link the previous object to the new object;            */
             /*                                                           */
             /*    modify type = ADD;                                     */
             /*    this row currently does not need updating;             */
             /*    set a dummy object id type;                            */
             /*                                                           */
             /*    copy the current window line to the new object;        */
             /*-----------------------------------------------------------*/

             odm_get_object_storage (objhandle->number_desc, &temp_object_data,
                                 head_descriptor_data);

             temp_object_data->prev = ptr_object_data;
             temp_object_data->next = ptr_object_data->next;
             if (ptr_object_data->next != NULL)
                  (ptr_object_data->next)->prev = temp_object_data;
             ptr_object_data->next = temp_object_data;

             temp_object_data->modify_type = ADD;
             temp_object_data->update = FALSE;
             temp_object_data->object_id = 0;

		/*-------------------------------------------------------*/
		/* loop through the descriptors and copy the binary fields */
		/* input_object will not copy binary data.                 */
		/*---------------------------------------------------------*/
	    for (i = 0, fptr = ptr_object_data->field_array,
 		fptr1 = temp_object_data->field_array,
		ptr_descriptor_data = head_descriptor_data;
		i < objhandle->number_desc;
		i++, ptr_descriptor_data++, fptr++, fptr1++)
		{
			switch (ptr_descriptor_data->classp->elem[i].type)
			{
			case CHAR :
			case VCHAR :
			case LONGCHAR :
			case OBJMETHOD :
			case OBJLINK :
			case SHORT :
			case LONG :
/*
			case OBJREPEAT :
*/
				break;
			case BINARY :
			  for (k = 0;                                                                       k < ptr_descriptor_data->classp->elem[i].size; k++)
			  {
			   *(char *) (*(fptr1) + k) = *(char *) (*(fptr) + k);
			  }
			  break;
			} /* end switch */
		} /* end for */


               input_object (object_window, cursor_line, objhandle->number_desc,
                           temp_object_data, head_descriptor_data);

               ptr_object_data = temp_object_data;

             /*----------------------------------------------------------*/
             /*  increment object count;                                 */
             /*  increment object number;                                */
             /*  increment window line;                                  */
             /*                                                          */
             /*  if the last line on the page;                           */
             /*     window line = first line;                            */
             /*     redisplay page starting with the object copied from; */
             /*  else                                                    */
             /*     insert a line;                                       */
             /*     copy the previous line to the new line;              */
             /*                                                          */
             /*----------------------------------------------------------*/
             object_count++;
             object_number++;
             cursor_line++;

             if (cursor_line >= MAX_DISPLAY_ITEMS)
              {
                 cursor_line = 0;
                 print_objects (object_window, objhandle->number_desc,
                                ptr_object_data, head_descriptor_data);
              }
             else
              {
                 wmove (object_window, cursor_line, cursor_col);
                 winsertln (object_window);

                 copy_data (object_window, 0, width, cursor_line - 1,
                            cursor_line);
              }
             srefresh = TRUE;
             delta = TRUE;
             break;



       /*-------------MOVE BACK ONE PAGE-----------------*/
       case ESC7:
       case KEY_F(7):
       case KEY_PPAGE:
       case KEY_SR:
             /*------------------------------------------------*/
             /*  if we have no objects produce beep and break  */
             /*------------------------------------------------*/
             if (ptr_object_data == NULL)
              {
                 beep ();
                 break;
              }

             /*------------------------------------------------------------*/
             /*  Update the current object line if changes have occurred.  */
             /*------------------------------------------------------------*/
             if (ptr_object_data->update == TRUE)
              {
                input_object(object_window, cursor_line, objhandle->number_desc,
                              ptr_object_data, head_descriptor_data);
                ptr_object_data->update = FALSE;
              }

             /*------------------------------------------------------*/
             /*  If there are no previous objects                    */
             /*     produce error sound                              */
             /*  else                                                */
             /*   {                                                  */
             /*     Move object pointer back one screen;             */
             /*     window line = 0;                                 */
             /*     compute new object number;                       */
             /*     print the objects on the screen;                 */
             /*   }                                                  */
             /*------------------------------------------------------*/
             if (ptr_object_data->prev == NULL)
              {
                 beep ();
              }
             else
              {
                 for (i = 0; (ptr_object_data->prev != NULL) &&
                      (i < cursor_line);
                      i++, ptr_object_data = ptr_object_data->prev);

                 for (i = 0; (ptr_object_data->prev != NULL) &&
                      (i < MAX_DISPLAY_ITEMS);
                      i++, ptr_object_data = ptr_object_data->prev);

                 cursor_line = 0;
                 object_number -= i;
                 print_objects (object_window, objhandle->number_desc,
                                ptr_object_data, head_descriptor_data);
                 srefresh = TRUE;
              }

             break;



       /*-------------------MOVE FORWARD ONE PAGE-------------------*/
       case ESC8:
       case KEY_F(8):
       case KEY_NPAGE:
       case KEY_SF:
             /*------------------------------------------------*/
             /*  if we have no objects produce beep and break  */
             /*------------------------------------------------*/
             if (ptr_object_data == NULL)
              {
                 beep ();
                 break;
              }

             /*------------------------------------------------------------*/
             /*  Update the current object line if changes have occurred.  */
             /*------------------------------------------------------------*/
             if (ptr_object_data->update == TRUE)
              {
                input_object(object_window, cursor_line, objhandle->number_desc,
                              ptr_object_data, head_descriptor_data);
                ptr_object_data->update = FALSE;
              }

             /*----------------------------------------------------*/
             /*  If we are one the last object                     */
             /*     produce error sound                            */
             /*  else                                              */
             /*   {                                                */
             /*     move to the last object or one full screen     */
             /*     set the window line to the first line          */
             /*     increment object number by the number moved    */
             /*     print this screen of objects                   */
             /*   }                                                */
             /*----------------------------------------------------*/
             if (ptr_object_data->next == NULL)
              {
                 beep ();
              }
             else
              {
                  for (i = 0; (i < ((MAX_DISPLAY_ITEMS - cursor_line))) &&
                       (ptr_object_data->next != NULL);
                       i++, ptr_object_data = ptr_object_data->next);

                   cursor_line = 0;
                   object_number += i;
                   print_objects (object_window, objhandle->number_desc,
                                  ptr_object_data, head_descriptor_data);

                   srefresh = TRUE;
              }

             break;



       /*--------------MOVE LEFT A SCREEN----------------*/
       case ESC9:
       case KEY_F(9):
             if (hscreen == 0) beep ();
             else
              {
                hscreen--;
                vscroll (object_view, 0, -MAX_DISPLAY_COLS);
                vscroll (descriptor_view, 0, -MAX_DISPLAY_COLS);
                cursor_col = hscreen * MAX_DISPLAY_COLS;
                field = find_field (cursor_col, objhandle->number_desc,
                                    head_descriptor_data);
                srefresh = TRUE;

		descriptor_view->_csbp = NORMAL;
                wrefresh (descriptor_view);
              }
             break;


       /*------------------MOVE RIGHT A SCREEN-------------------*/
       case ESC0:
       case KEY_F(10):
             if (hscreen == maxhorizontal) beep ();
             else
              {
                 hscreen++;
                 vscroll (object_view, 0, MAX_DISPLAY_COLS);
                 vscroll (descriptor_view, 0, MAX_DISPLAY_COLS);
                 cursor_col = hscreen * MAX_DISPLAY_COLS;
                 field = find_field (cursor_col, objhandle->number_desc,
                                     head_descriptor_data);

		 descriptor_view->_csbp = NORMAL;
                 wrefresh (descriptor_view);
                 srefresh = TRUE;
              }
             break;



       /*-----------------BACKWARD TAB-------------------*/
       case KEY_BTAB:
             if (field == 0)
              {
                beep ();
                break;
              }
             field--;
             cursor_col = (head_descriptor_data + field)->start_column;

             for (;(cursor_col / MAX_DISPLAY_COLS) != hscreen;)
              {
                 hscreen--;
                 vscroll (object_view, 0, -MAX_DISPLAY_COLS);
                 vscroll (descriptor_view, 0, -MAX_DISPLAY_COLS);
              }
	     descriptor_view->_csbp = NORMAL;
             wrefresh (descriptor_view);
             srefresh = TRUE;
             break;


       /*-------------------FORWARD TAB-------------------*/
       case KEY_TAB:
             if (field == (objhandle->number_desc - 1))
              {
                 beep ();
                 break;
              }
             field++;
             cursor_col = (head_descriptor_data + field)->start_column;

             for (;(cursor_col / MAX_DISPLAY_COLS) != hscreen;)
              {
                 hscreen++;
                 vscroll (object_view, 0, MAX_DISPLAY_COLS);
                 vscroll (descriptor_view, 0, MAX_DISPLAY_COLS);
              }
				descriptor_view->_csbp = NORMAL;
             wrefresh (descriptor_view);
             srefresh = TRUE;
             break;


       /*------------------MOVE TO END OF FIELD----------------------*/
       case CTRL_E:
             temp = (head_descriptor_data + field)->start_column +
                    (head_descriptor_data + field)->data_width - 1;
             if (cursor_col == temp)
              {
                 beep ();
                 break;
              }
             cursor_col = temp;

             for (;(cursor_col / MAX_DISPLAY_COLS) != hscreen;)
              {
                 hscreen++;
                 vscroll (object_view, 0, MAX_DISPLAY_COLS);
                 vscroll (descriptor_view, 0, MAX_DISPLAY_COLS);
              }
				descriptor_view->_csbp = NORMAL;
             wrefresh (descriptor_view);
             break;



       /*-----------------MOVE TO END OF OBJECT---------------------*/
       case KEY_END:
             field = objhandle->number_desc - 1;

             cursor_col = (head_descriptor_data + field)->start_column;

             for (;(cursor_col / MAX_DISPLAY_COLS) != hscreen;)
              {
                 hscreen++;
                 vscroll (object_view, 0, MAX_DISPLAY_COLS);
                 vscroll (descriptor_view, 0, MAX_DISPLAY_COLS);
              }
				descriptor_view->_csbp = NORMAL;
             wrefresh (descriptor_view);
             srefresh = TRUE;
             break;


       /*---------------MOVE TO BEGINNING OF OBJECT------------------*/
       case KEY_HOME:
             cursor_col = head_descriptor_data->start_column;
             field = 0;
             for (;(cursor_col / MAX_DISPLAY_COLS) != hscreen;)
              {
                 hscreen--;
                 vscroll (object_view, 0, -MAX_DISPLAY_COLS);
                 vscroll (descriptor_view, 0, -MAX_DISPLAY_COLS);
              }
	     descriptor_view->_csbp = NORMAL;
             wrefresh (descriptor_view);
             srefresh = TRUE;
             break;



       /*---------------CURSOR DOWN-------------------------*/
       case KEY_DOWN:
       case CTRL_N:
             /*------------------------------------------------*/
             /*  if we have no objects produce beep and break  */
             /*------------------------------------------------*/
             if (ptr_object_data == NULL)
              {
                 beep ();
                 break;
              }

             /*------------------------------------------------------------*/
             /*  Update the current object line if changes have occurred.  */
             /*------------------------------------------------------------*/
             if (ptr_object_data->update == TRUE)
              {
                input_object(object_window, cursor_line, objhandle->number_desc,
                              ptr_object_data, head_descriptor_data);
                ptr_object_data->update = FALSE;
              }

             /*---------------------------------------------------------*/
             /*  if we are on the last object   */
             /*      produce an error sound                             */
             /*  else if on the last line of the screen                 */
             /*   {                                                     */
             /*      set window line to first line of screen            */
             /*      move current pointer to next object                */
             /*      increment object number                            */
             /*      print the next screen                              */
             /*   }                                                     */
             /*  else just move to next object                          */
             /*   {                                                     */
             /*      increment window pointer                           */
             /*      move current pointer to next pointer               */
             /*      increment object pointer                           */
             /*   }                                                     */
             /*---------------------------------------------------------*/
             if (ptr_object_data->next == NULL)
              {
                 beep ();
              }
             else if (cursor_line == (MAX_DISPLAY_ITEMS - 1))
              {
                 cursor_line = 0;
                 ptr_object_data = ptr_object_data->next;
                 object_number++;

                 print_objects (object_window, objhandle->number_desc,
                                ptr_object_data, head_descriptor_data);
              }
             else
              {
                 cursor_line++;
                 ptr_object_data = ptr_object_data->next;
                 object_number++;
              }

             srefresh = TRUE;
             break;



       /*-------------------CURSOR UP---------------------*/
       case KEY_UP:
       case CTRL_P:
             /*------------------------------------------------*/
             /*  if we have no objects produce beep and break  */
             /*------------------------------------------------*/
             if (ptr_object_data == NULL)
              {
                 beep ();
                 break;
              }

             /*------------------------------------------------------------*/
             /*  Update the current object line if changes have occurred.  */
             /*------------------------------------------------------------*/
             if (ptr_object_data->update == TRUE)
              {
                input_object(object_window, cursor_line, objhandle->number_desc,
                              ptr_object_data, head_descriptor_data);
                ptr_object_data->update = FALSE;
              }

             /*-------------------------------------------------------*/
             /*  IF there is no previous object                       */
             /*      produce error sound                              */
             /*  else if I am on the first line                       */
             /*   {                                                   */
             /*      move back to the first object or a full page     */
             /*      print the page of objects moved back to          */
             /*      move current pointer back one object             */
             /*      decrement object number                          */
             /*   }                                                   */
             /*  else just move back one object                       */
             /*   {                                                   */
             /*      decrement window line                            */
             /*      move current pointer back one object             */
             /*      decrement object number                          */
             /*   }                                                   */
             /*-------------------------------------------------------*/
             if (ptr_object_data->prev == NULL)
              {
                beep ();
              }
             else if (cursor_line == 0)
              {
                 for (i = 0, temp_object_data = ptr_object_data;
                      ((i < MAX_DISPLAY_ITEMS) &&
                      (temp_object_data->prev != NULL));
                      i++, temp_object_data = temp_object_data->prev);

                 cursor_line = --i;
                 print_objects (object_window, objhandle->number_desc,
                                temp_object_data, head_descriptor_data);

                 ptr_object_data = ptr_object_data->prev;
                 object_number--;
              }
             else
              {
                 cursor_line--;
                 ptr_object_data = ptr_object_data->prev;
                 object_number--;
              }

             srefresh = TRUE;
             break;


       /*-----------------DEFAULT SOME OTHER KEY----------------*/
       default:
             /*------------------------------------------------*/
             /*  if we have no objects produce beep and break  */
             /*------------------------------------------------*/
             if (ptr_object_data == NULL)
              {
                 beep ();
                 break;
              }

             /*-------------check for read only opened objects----------*/
             if (READONLY)
              {
                 beep ();
                 prerr (MSGS(ER,OD_READ_ONLY,DF_OD_READ_ONLY), 0, FALSE);
                 break;
              }
	     if (head_descriptor_data->classp->elem[field].type == BINARY)
	      {
		 beep ();
		 break;
	      }

             /*------------------------------------------------------*/
             /*  Call standard editing keys for processing rest of   */
             /*  character types.  A return of 1 means something on  */
             /*  the screen changed.                                 */
             /*------------------------------------------------------*/
             if (std_edit_keys (object_window, c, (head_descriptor_data + field)
                 ->start_column, (head_descriptor_data + field)->data_width,
                 &cursor_col, &cursor_line) == 1)
              {
                 if (ptr_object_data->modify_type != ADD)
                     ptr_object_data->modify_type = CHANGE;

                 ptr_object_data->update = TRUE;
                 delta = TRUE;
              }

             /*-----------------------------------------------------*/
             /*  If the user has moved between screens on the last  */
             /*  character; scroll the screen.                      */
             /*-----------------------------------------------------*/
             if ((cursor_col / MAX_DISPLAY_COLS) > hscreen)
              {
                 hscreen++;
                 vscroll (object_view, 0, MAX_DISPLAY_COLS);
                 vscroll (descriptor_view, 0, MAX_DISPLAY_COLS);
	 	 descriptor_view->_csbp = NORMAL;
                 wrefresh (descriptor_view);
              }
             else if ((cursor_col / MAX_DISPLAY_COLS) < hscreen)
              {
                 hscreen--;
                 vscroll (object_view, 0, -MAX_DISPLAY_COLS);
                 vscroll (descriptor_view, 0, -MAX_DISPLAY_COLS);
					descriptor_view->_csbp = NORMAL;
                 wrefresh (descriptor_view);
              }
	     break;

	case KEY_F(12) :
		 if ((odme_prscr ()) == FALSE)
		  {
			help (MSGS(ER,PRINT_SCREEN_ERROR,
				DF_PRINT_SCREEN_ERROR) );
		  }

		 break;

      }  /* end switch */
  } /* end for */
}  /* end object display */

