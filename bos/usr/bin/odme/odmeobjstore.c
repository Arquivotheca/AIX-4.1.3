static char sccsid[] = "@(#)72  1.12  src/bos/usr/bin/odme/odmeobjstore.c, cmdodm, bos411, 9428A410j 4/18/91 18:51:34";
/*
 * COMPONENT_NAME: (ODME) ODMEOBJSTORE - support routines for display
 *
 * FUNCTIONS:
 *     void get_descriptors ();      get descriptors for the class
 *     int  odm_get_objects ();          get objects from object class
 *     void free_object_storage ();  free object struct storage
 *     void odm_get_object_storage ();   get object struct storage
 *     int  print_objects ();        print a screen of objects
 *     int  find_field ();           find the previous object field
 *     void input_object ();         copy the current screen fields
 *                                   to the object storage
 *     int  open_object_class ();    open the object class
 *     void delete_object_class ();  delete an entire object class
 *     void odm_initialize ();       initialize odm environment
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
/*   FILE NAME :    odmeobjstore.c                                    */
/*                                                                    */
/*   DESCRIPTION :  This file contains Object Data Manager            */
/*                  Editor object storage functions.                  */
/*                                                                    */
/*   CONTENTS :                                                       */
/*     void get_descriptors ();      get descriptors for the class    */
/*     int  odm_get_objects ();          get objects from object class    */
/*     void free_object_storage ();  free object struct storage       */
/*     void odm_get_object_storage ();   get object struct storage        */
/*     int  print_objects ();        print a screen of objects        */
/*     int  find_field ();           find the previous object field   */
/*     void input_object ();         copy the current screen fields   */
/*                                   to the object storage            */
/*     int  open_object_class ();    open the object class            */
/*     void delete_object_class ();  delete an entire object class    */
/*     void odm_initialize ();       initialize odm environment       */
/*                                                                    */
/**********************************************************************/

#include "odme.h"

extern int READONLY;
extern int INSERTON;
extern int TRM;
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



/*******************************************************************/
/*  get_descriptors : return the descriptors for the current       */
/*                    object class.                                */
/*       parameters : objhandle - current object handle,           */
/*                    head_descriptor_data - points to head of     */
/*                               malloc'd descriptor data storage. */
/*******************************************************************/
void get_descriptors (objhandle, head_descriptor_data)

struct objtoken        *objhandle;
struct descriptor_data **head_descriptor_data;
{

   struct descriptor_data  *ptr_descriptor_data;

   int  length,               /* used for holding string length  */
        whichone = TRUE;     /* which descriptor do I return ?  */
   register i;                /* for loop variable               */
        struct Class *classp;

        classp = objhandle->objptr;
   /*------------------------------------------------------------------*/
   /* SYNOPSIS :                                                       */
   /*    Malloc storage for descriptor data computed from descriptors. */
   /*                                                                  */
   /*    while (still more descriptors) do                             */
   /*     {                                                            */
   /*         odmgetdescp (get descriptor);                            */
   /*         compute column data for current descriptor;              */
  /*     }                                                            */
   /*    return;                                                       */
   /*------------------------------------------------------------------*/

   if ((*head_descriptor_data = (struct descriptor_data *) malloc
       (objhandle->number_desc * sizeof (struct descriptor_data))) == NULL)
           prerr (MSGS(ER,OS_DESC_MALLOC,DF_OS_DESC_MALLOC), 0, TRUE);

   for (i = 0, ptr_descriptor_data = *head_descriptor_data;
        i < objhandle->number_desc; i++, ptr_descriptor_data++)
    {

        /*--------------------------------------------------------------*/
        /* set up ptr to class in head descriptor data                  */
        /*--------------------------------------------------------------*/
        (ptr_descriptor_data)->classp = objhandle->objptr;

        switch ((int) classp->elem[i].type)
         {
            case SHORT :
                  ptr_descriptor_data->header_width = 9;
                  ptr_descriptor_data->data_width   = SHORTWIDTH;
                  strcpy (ptr_descriptor_data->header, "ODM_SHORT");
                  break;

            case LONG :
                  ptr_descriptor_data->header_width = 8;
                  ptr_descriptor_data->data_width   = LONGWIDTH;
                  strcpy (ptr_descriptor_data->header, "ODM_LONG");
                  break;

            case BINARY :
                  ptr_descriptor_data->header_width = 10;
                  ptr_descriptor_data->data_width   = 6;
                  strcpy (ptr_descriptor_data->header, "ODM_BINARY");
                  break;

            case OBJLINK     :
                  ptr_descriptor_data->header_width = 8;
                  ptr_descriptor_data->data_width   =
                                  classp->elem[i].size;
                  strcpy (ptr_descriptor_data->header, "ODM_LINK");
                  break;
            case OBJMETHOD :
                  ptr_descriptor_data->header_width = 10;
                  ptr_descriptor_data->data_width   =
                                  classp->elem[i].size;
                  strcpy (ptr_descriptor_data->header, "ODM_METHOD");
                  break;

            case LONGCHAR :
                  ptr_descriptor_data->header_width = 12;
                  ptr_descriptor_data->data_width =
                                  classp->elem[i].size;
                  strcpy (ptr_descriptor_data->header, "ODM_LONGCHAR");
                  break;

            case CHAR :
                  ptr_descriptor_data->header_width = 8;
                  ptr_descriptor_data->data_width   =
                                  classp->elem[i].size - 1;
                  strcpy (ptr_descriptor_data->header, "ODM_CHAR");
                  break;

            case VCHAR :
                  ptr_descriptor_data->header_width = 9;
                  ptr_descriptor_data->data_width   =
                                  classp->elem[i].size - 1;
                  strcpy (ptr_descriptor_data->header, "ODM_VCHAR");
                  break;

            default :
                  prerr (MSGS(ER,OS_UNKNOWN_DESC,DF_OS_UNKNOWN_DESC),
                         classp->elem[i].type, FALSE);
                  break;

         }  /* end switch */

        /*-----------------------------------------------------------------*/
        /* Now I'm going to find out which is longer, the header width or  */
        /* the descriptor name.  The reason is to compute the maximum      */
        /* number of columns on the screen this descriptor will take up    */
        /*-----------------------------------------------------------------*/
        if (ptr_descriptor_data->header_width <
            (length = strlen (classp->elem[i].elemname)))
               ptr_descriptor_data->header_width = length;

    } /* end for */

   return;

}  /* end get descriptors */



/*******************************************************************/
/*  get objects : get objects from object class.                   */
/*                                                                 */
/*  PARAMETERS  : objhandle - the object class handle              */
/*                objs - the search criteria                       */
/*                ptr_object_data - where to place objects         */
/*                head_descriptor_data - beginning of object       */
/*                                       descriptors               */
/*                whichone - which object to return; FIRST/NEXT    */
/*                more  - are there more objects in the class ?    */
/*******************************************************************/
int odm_get_objects (objhandle, objs, ptr_object_data, head_descriptor_data,
                 whichone, more)

struct objtoken         *objhandle;
char *objs;
struct object_data      **ptr_object_data;
struct descriptor_data  *head_descriptor_data;
int    *whichone,
       *more;

{
   void odm_get_object_storage ();
   void free_object_storage ();

   struct descrip_content  *ptr_deschandle,
                      *index;
   struct object_data *next_object_data = NULL,
                      *prev_object_data = NULL;
   char **fptr;

   register i,
            j,
            k;

        char *data_start,
                        *data_ptr;
   /*----------------------------------------------------------*/
   /*  get storage for first object                            */
   /*  store the pointer from the object that we entered with  */
   /*  if we entered with a previous object                    */
   /*     set it's next pointer to our new storage             */
   /*  else                                                    */
   /*     we had no previous object so set the pointer to our  */
   /*     new storage                                          */
   /*----------------------------------------------------------*/
   odm_get_object_storage (objhandle->number_desc, &next_object_data,
                       head_descriptor_data);
   prev_object_data = *ptr_object_data;

   if (*ptr_object_data != NULL)
       (*ptr_object_data)->next = next_object_data;
   else
       *ptr_object_data = next_object_data;

        /*------------------------------------------------------------*/
        /* allocate memory to hold the data from the odm_get_obj          */
        /*------------------------------------------------------------*/
        data_start = (char *) malloc (head_descriptor_data->classp->structsize);
        data_ptr = data_start;

   /*------------------------------------------------------------*/
   /*  Return FETCH_OBJECTS number of objects from the object    */
   /*  class, or until the end is encountered then break out     */
   /*------------------------------------------------------------*/
   for (i = 0; i < FETCH_OBJECTS; i++)
    {
       next_object_data->prev = prev_object_data;
       next_object_data->next = NULL;
       next_object_data->modify_type = NOCHG;
       next_object_data->update = FALSE;


       if (*whichone == TRUE)
        {
           *whichone = FALSE;
                        if ( ((data_ptr =
                         (char *) odm_get_obj(head_descriptor_data->classp, objs, data_start, TRUE)) == (char *) -1) || (data_ptr == NULL) )
            {
               *ptr_object_data = NULL;
               *more = FALSE;
               break;
            }
        }
       else
        {
                        if ( ((data_ptr =
                         (char *) odm_get_obj(head_descriptor_data->classp, objs, data_start, FALSE)) == (char *) -1 ) || (data_ptr == NULL) )
            {
               *more = FALSE;
               break;
            }
        }
        /*----------------------------------------------------------*/
        /* store the id for processing deletes                      */
        /*----------------------------------------------------------*/
        next_object_data->object_id = *(long *) data_ptr;

      /*------------------------------------------------------------*/
      /*  Now allocate ODME internal storage for holding this       */
      /*  object.  The reason for this is each change to the object */
      /*  will be have to held internally until the exit of the     */
      /*  object class.  I tried keeping everything in a curses     */
      /*  window and copying it out on exit but the screens get     */
      /*  way too large.  The regular object storage returned from  */
      /*  an object get can't be used to hold the changes.          */
      /*------------------------------------------------------------*/
       for (j = 0, fptr = next_object_data->field_array;
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
                      strcpy (*fptr, (char *)(data_ptr +
                              head_descriptor_data->classp->elem[j].offset) );
                      break;

                case OBJLINK :
                      strcpy (*fptr, (char *)(data_ptr +
                              head_descriptor_data->classp->elem[j].offset)
                              + 2 * sizeof(char *) );
                      break;

                case VCHAR :
                      strcpy (*fptr, *(char **)(data_ptr +
                              head_descriptor_data->classp->elem[j].offset) );
                      break;

                /*----------------INTEGER DATA-----------------------*/
                /* These data types are of integer and should be     */
                /* used to set their corresponding pointer locations */
                /*---------------------------------------------------*/
                case SHORT :
                      *(short *) *fptr = *(short *)(data_ptr +
                              head_descriptor_data->classp->elem[j].offset);
                      break;

                case LONG :
                      *(long *) *fptr = *(long *)(data_ptr +
                             head_descriptor_data->classp->elem[j].offset);
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
                /* these characters will have to be copied in on an */
                /* individual basis, because of the possibility of  */
                /* nulls existing in the data.                      */
                /*--------------------------------------------------*/
                case BINARY :
                     for(k=0;k<(head_descriptor_data->classp->elem[j].size);k++)
                       {
                         *(char *) (*(fptr) + k) = *(char *)
                             (head_descriptor_data->classp->elem[j].offset + k);
                       }
                      break;

             }  /* end switch */
        } /* end for */

       /*--------------------------------------------------------*/
       /*  store the object id so changes can be made            */
       /*  free up the object storage gotten by ODM              */
       /*  get storage for the next object                       */
       /*--------------------------------------------------------*/

       prev_object_data = next_object_data;

       odm_get_object_storage (objhandle->number_desc, &next_object_data,
                           head_descriptor_data);

       prev_object_data->next = next_object_data;

    }  /* end for */

   if (prev_object_data != NULL)
       prev_object_data->next = NULL;

   /*--------------------------------------------------------*/
   /*  I've gotten one too many object storage so free it up */
   /*--------------------------------------------------------*/
   free_object_storage (objhandle->number_desc, &next_object_data);
   return (i);

} /* end get objects */



/*******************************************************************/
/*  free object storage : free the malloc'd storage of the object  */
/*                        data.                                    */
/*  PARAMETERS  :   number_fields - number of fields to free       */
/*                  ptr_object_data - object data to free          */
/*******************************************************************/
void free_object_storage (number_fields, ptr_object_data)

int number_fields;
struct object_data **ptr_object_data;
{
   register i;
   char **fptr;

   /*------------------------------------------*/
   /* free individual object field storage     */
   /*------------------------------------------*/
    fptr = (*ptr_object_data)->field_array;

    if (*fptr != (char *) NULL)
           free (*fptr);

   /*-----------------------------------------*/
   /* free the field array pointer            */
   /* then free the object structure itself   */
   /*-----------------------------------------*/
   free ((char *) (*ptr_object_data)->field_array);
/*
   free (*ptr_object_data);
   *ptr_object_data = NULL;
*/

}


/*************************************************************************/
/*									 */
/* PARAMETERS : number_fields - number of fields in this object          */
/*              ptr_object_data - object data to return as storage       */
/*              ptr_descriptor_data - descriptors for this object        */
/*************************************************************************/
void odm_get_object_storage (number_fields, ptr_object_data, ptr_descriptor_data)

int     number_fields;
struct  object_data     **ptr_object_data;
struct descriptor_data *ptr_descriptor_data;

{
        register        i;
        char    **fptr;
        char *memdata = NULL;           /* malloced mem address for
                                        this objects descriptors */
        int memsize = 0;                /* how much memory to malloc
                                        for this objects descriptors */
        int numchr = 0;                 /* keeps track of the number of
                                        char fields for calculating
                                        amount of extra storage to malloc
                                        for word alignment */
        int rmdr =0;                    /* also used for word alignment */

        /*------------------------------------------------------*/
        /* malloc the object data structure itself              */
        /* then malloc a field array for pointing to individual */
        /* fields in the object.                                */
        /*------------------------------------------------------*/
        if ((*ptr_object_data = (struct object_data *)
                malloc (sizeof (struct object_data))) == NULL)
                  prerr (MSGS(ER,OS_STORAGE_MALLOC,DF_OS_STORAGE_MALLOC),
                        0, TRUE);

        (*ptr_object_data)->field_array = (char **) malloc
                                (number_fields * sizeof(char *));

        /*-------------------------------------------------------------*/
        /* loop through the descriptors for this object and total the  */
        /* amount of storage required by each in memsize.  then malloc */
        /* all memory for this objectors descriptor data.              */
        /*-------------------------------------------------------------*/

        for (i=0; i < number_fields; i++)
        {
                switch (ptr_descriptor_data->classp->elem[i].type)
                {
                        case SHORT:
                        case LONG:
                                memsize += (sizeof(long));
                                break;
                        case CHAR:
                        case VCHAR :
                        case OBJMETHOD:
                        case OBJLINK :
                        case LONGCHAR :
                        case BINARY :
                                memsize +=
                                 (ptr_descriptor_data->classp->elem[i].size+1);
                                numchr += 1;
                                break;
                } /* end switch */
        } /* end for */
        /*----------------------------------------------------------------*/
        /* allocate enough memory to hold all descriptors for this object */
        /*----------------------------------------------------------------*/
        numchr *= sizeof(long);
        if ((memdata = (char *) malloc (numchr + memsize)) == NULL)
                   prerr (MSGS(ER,OS_DESC_MALLOC,DF_OS_DESC_MALLOC),
                        0, TRUE);
        /*-------------------------------------------------------------*/
        /* now set field_array pointers to correct positions in memdata */
        /*-------------------------------------------------------------*/

        for (i = 0, fptr = (*ptr_object_data)->field_array, *fptr = memdata;
                i < number_fields; i++, fptr++)
        {
                /*--------------------------------------------------------*/
                /* for short, long, float, first adjust memdata to the    */
                /* next word boundry.                                     */
                /*--------------------------------------------------------*/
                switch (ptr_descriptor_data->classp->elem[i].type)
                {
                        case SHORT:
                        case LONG:
                                if ((rmdr = (int) memdata % sizeof(long)) != 0)
                                {
                                        memdata += sizeof(long) - rmdr;
                                }
                                *fptr = memdata;
                                /*
                                memdata += (sizeof(long) + rmdr);
                                */
                                memdata += (sizeof(long));
                                break;
                        case CHAR:
                        case VCHAR:
                        case LONGCHAR:
                        case OBJMETHOD:
                        case OBJLINK:
                        case BINARY:
                                *fptr =
                                  memdata;
                                memdata += ptr_descriptor_data->
                                  classp->elem[i].size + 1;
                                break;
/*
                        case OBJREPEAT:
                                *fptr = (char *) NULL;
                                memdata += (sizeof(char *));
                                break;
*/
                        default:
                                prerr (MSGS(ER,OS_INVALID_TYPE,
                                        DF_OS_INVALID_TYPE), 0, FALSE);
                                break;
                } /* end switch */

        } /* end for */
} /* end object get storage */





/***********************************************************************/
/*  print objects : display the next screen of objects.                */
/*                                                                     */
/*     PARAMETERS : obj_window - window to display objects in          */
/*                  number_fields - number of fields in object         */
/*                  ptr_object_data - object struct to start print at  */
/*                  head_descriptor_data - descriptor storage          */
/***********************************************************************/
int print_objects (obj_window, number_fields, ptr_object_data,
                   head_descriptor_data)

WINDOW                  *obj_window;
int                     number_fields;
struct object_data      *ptr_object_data;
struct descriptor_data  *head_descriptor_data;
{
    struct descriptor_data  *ptr_descriptor_data;
    char   **fptr;
    register i,
             j,
             k;

    wmove  (obj_window, 0, 0);
    werase (obj_window);

    /*----------------------------------------------------------------*/
    /*  Traverse each object till no more objects or screen is full   */
    /*       Traverse the descriptors and the corresponding field     */
    /*           move to the next character position;                 */
    /*           print out the field according to the type;           */
    /*----------------------------------------------------------------*/
    for (i = 0; (i < MAX_DISPLAY_ITEMS) && (ptr_object_data != NULL);
         i++, ptr_object_data = ptr_object_data->next)
     {
        for (j = 0, fptr = ptr_object_data->field_array,
             ptr_descriptor_data = head_descriptor_data;
             j < number_fields; j++, fptr++, ptr_descriptor_data++)
         {
            wmove (obj_window, i, ptr_descriptor_data->start_column);
            switch (head_descriptor_data->classp->elem[j].type)
             {

                /*-------------------CHARACTER DATA----------------------*/
                /* Add the string terminated by a null to the window     */
                /*-------------------------------------------------------*/
                case CHAR        :
                case OBJLINK    :
                case VCHAR       :
                case LONGCHAR    :
                case OBJMETHOD   :
                      waddstr (obj_window, *fptr);
                      break;

                /*--------------------UNKNOWN DATA-------------------------*/
                /* Since this type can hold anything, show all printable   */
                /* characters and output a default character for non-      */
                /* printable                                               */
                /*---------------------------------------------------------*/
                case BINARY      :
                        waddstr (obj_window, "BINARY");
                      break;

                /*---------------------INTEGER DATA-------------------------*/
                /* Print all integer data types on the screen according to  */
                /* their data widths                                        */
                /*----------------------------------------------------------*/
                case LONG :
                      wprintw (obj_window, "%-*ld", ptr_descriptor_data->
                               data_width, *(long *) *fptr);
                      break;

                case SHORT :
                      wprintw (obj_window, "%-*d", ptr_descriptor_data->
                               data_width, *(short *) *fptr);
                      break;

                default :
                       prerr (MSGS(ER,OS_UNKNOWN_DESC,DF_OS_UNKNOWN_DESC),
                            head_descriptor_data->classp->elem[j].type, FALSE);
                       return (-1);

             }  /* end switch */
         }  /* end for */
     }  /* end for */

    return (i);    /* return the number printed */
}



/********************************************************************/
/* find_field : find the previous descriptor.  Use this function    */
/*              when moving backward thru the object fields         */
/*                                                                  */
/* PARAMETERS : column - column the cursor is now on                */
/*              number_desc - number of descriptors in the object   */
/*              head_descriptor_data - descriptor storage           */
/********************************************************************/
int find_field (column, number_desc, head_descriptor_data)

int    column,
       number_desc;
struct descriptor_data *head_descriptor_data;
{

  int i;

  /*---------------------------------------------------------------*/
  /*  Traverse descriptors from the beginning til you find         */
  /*  the descriptor that the current cursor position belongs      */
  /*  to and return the descriptor number before this descriptor   */
  /*---------------------------------------------------------------*/
  for (i = 0; i < number_desc; i++)
    if (((head_descriptor_data + i)->start_column +
         (head_descriptor_data + i)->data_width) >= column) break;

  return i;
}



/*************************************************************************/
/*  input_object : copy the current window line to the object storage.   */
/*                                                                       */
/*    PARAMETERS : obj_window - the curren object window                 */
/*                 line - the line to copy into the object               */
/*                 number_fields - the number of fields in the object    */
/*                 ptr_object_data - the object to fill with screen data */
/*                 ptr_descriptor_data - the descriptors for the object  */
/*************************************************************************/
void input_object (obj_window, line, number_fields, ptr_object_data,
                   ptr_descriptor_data)

WINDOW *obj_window;
int    line,
       number_fields;
struct object_data      *ptr_object_data;
struct descriptor_data  *ptr_descriptor_data;
{

   void input_field ();

   char **fptr,
/* CHECK THE ACTUAL LENGTH OF MAX_CHAR_LENGTH */
        tempstr[MAX_LONGCHAR_LENGTH + 1];
   register i,
            j,
            k;

  /*-----------------------------------------------------------*/
  /* traverse each descriptor for the object                   */
  /*    input that field into a temporary string               */
  /*    do a cast on the appropriate data type                 */
  /*-----------------------------------------------------------*/
   for (i = 0, fptr = ptr_object_data->field_array;
        i < number_fields;
        i++, ptr_descriptor_data++, fptr++)

    {
        input_field (obj_window, line, ptr_descriptor_data->start_column,
                     ptr_descriptor_data->data_width, tempstr);

        switch (ptr_descriptor_data->classp->elem[i].type)
         {
            case CHAR :
            case VCHAR :
            case LONGCHAR :
            case OBJMETHOD :
            case OBJLINK :
                  strcpy (*fptr, tempstr);
                  break;

            /*----------------------BINARY----------------------*/
            /* these characters will have to be copied in on an */
            /* individual basis, because of the possibility of  */
            /* nulls existing in the data.                      */
            /*--------------------------------------------------*/
            case BINARY :
                  break;

            case SHORT :
                  *(short *) *fptr = atoi(tempstr);
                  break;

            case LONG :
                  *(long *) *fptr = atol(tempstr);
                  break;
/*
            case OBJREPEAT :
                  *fptr = (char *) NULL;
                  break;
*/

         }  /* end switch */
    } /* end for */
} /* end input object */



/*************************************************************************/
/*  open object class : open the ODM object class                        */
/*                                                                       */
/*         PARAMETERS : object_handle - the object handle to pass back   */
/*************************************************************************/
int open_object_class (objhandle)

struct objtoken *objhandle;
{

        if ((int)(objhandle->objptr = (struct Class *) odm_mount_class(objhandle->objname))
                ==  -1)
                {
                prerr (MSGS(ER,OS_OPEN_FAILED,DF_OS_OPEN_FAILED), 0, FALSE);
                objhandle->number_desc = 0;
                return (-1);
                }
        else
                {
                objhandle->number_desc = objhandle->objptr->nelem;
                }

        if ((int)odm_open_class(objhandle->objptr) == -1)
                {
                prerr (MSGS(ER,OS_OPEN_FAILED,DF_OS_OPEN_FAILED), 0, FALSE);
                 objhandle->number_desc = 0;
                return (-1);
                }
   return (objhandle->number_desc);

}

/*********************************************************************/
/*  void delete_object_class : delete an object class.               */
/*********************************************************************/
/*
void delete_object_class (objhandle)

struct objtoken *objhandle;
{
        struct class_info       object;

        strcpy (object.class_rep_path, objhandle->objrep_path);
        strcpy (object.class_name,     objhandle->objname);
        strcpy (object.node,      objhandle->node);

        object.expand_flag = NOEXPAND;

        if ( (odmdrop (&object, FORCE)) < 0)
        {
                prerr (MSGS(ER,OS_DELETE_FAILED,DF_OS_DELETE_FAILED),
                    odmerrno, FALSE);
        }
        else
        {
                prerr (MSGS(OS,os_delete_ok,"OBJECT CLASS DELETE SUCCESSFUL"),
                 0, FALSE);
        }

        return;
}
*/
/*********************************************************************/
/*  void odm_initialize : initialize the ODM operating environment.  */
/*                        lock option and the odm user               */
/*********************************************************************/
/*
void odm_initialize ()

{
        odm_initialize();
}
*/
