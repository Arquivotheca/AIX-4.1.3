static char sccsid[] = "@(#)30  1.5  src/bos/usr/bin/lslpp/lslpp_utils.c, cmdswvpd, bos411, 9428A410j 6/6/94 18:21:00";

/*
 *   COMPONENT_NAME: CMDSWVPD
 *
 *   FUNCTIONS: 
 *              ckwild
 *              cmpkmch
 *              Lines
 *              LPP_State
 *              WrapText
 *              get_description
 *              get_type
 *              fl_plus
 *              fl_minus
 *              free_unused_memory
 *              free_done_memory
 *              get_name
 *              locate_fix_local
 *              history_swap
 *              print_legend
 *              print_description
 *              product_swap
 *              remove_blanks
 *              sort_inputs
 *              sort_history
 *              sort_product
 *              sort_u_product
 *              usage
 * 
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#include <lslpp.h>

static int history_swap (hist_t * h[],
                         int      i,
                         int      j);

static int sort_product (int left,
                         int right);

static int product_swap (int i,
                         int j);


/*--------------------------------------------------------------------*
 *
 *  NAME:       get_type 
 *
 *  FUNCTION:   Determines an alphabetic code for the type of the fix_info
 *              or prod_t argument.  Only one structure will be initialized
 *              depending upon who called us.  
 *
 *  RETURNS:    A char which signifies the type.
 *              ' ' -- install image (base level) returned for -L processing
 *                  -- returns 'I' if called for -l processing
 *              'M' -- Maintenance level update
 *              'E' -- Enhancement
 *              'F' -- Fix
 *
 *--------------------------------------------------------------------*/

char get_type (fix_info_type * fix, prod_t * prod_rec)
{

   /*
    * fix will be non-NIL when called by process_L_flag.  Otherwise,
    * prod_t will be non-NIL (called for -l processing.) 
    */
   if (fix != NIL (fix_info_type))
   { 
      if (IF_INSTALL (fix->op_type))
         return (' ');  /* Blank assumed to be base level by vsm */

      if ( IS_MAINT_LEV (fix))
         return ('M');

      if ( IF_LPP_PKG_PTF_TYPE_E  (fix -> cp_flag) )
         return ('E');
   }
   else
   {
      if (prod_rec->cp_flag & LPP_INSTAL)
         return ('I');

      if (IF_LPP_PKG_PTF_TYPE_C (prod_rec->cp_flag)  ||
          IF_LPP_PKG_PTF_TYPE_ML (prod_rec->cp_flag))
         return ('M');

      if (IF_LPP_PKG_PTF_TYPE_E (prod_rec->cp_flag))
         return ('E');
   }
   return ('F');

}  /* get_type  */

/*---------------------------------------------------------------------------- 
 * NAME:      Lines
 *
 * FUNCTION:  Returns a string 'fld_width' long of dashes.
 *
 * EXECUTION ENVIRONMENT:
 *
 * CALLED BY: do_name_list do_hist do_prereqs do_pid do_files do_APARs
 *
 * RETURNS:   Address of a string 'fld_width' long of dashes.
 *
 *---------------------------------------------------------------------------*/

char * Lines (int fld_width)

/* fld_width = INPUT: length of the line of dashes */

{
   static char     buf[MAX_USE_AS_ARG][MAX_LINE_WIDTH];
   static int      w = 0;
   int             i;

   /* We use a circular array of buffers here to allow the routine to be
    * used as a parameter call without each subsequent invocation blowing
    * out the buffer */

   if (w >= MAX_USE_AS_ARG)
      w = 0;
   for (i = 0; i < fld_width && i < MAX_LINE_WIDTH; i++)
      buf[w][i] = '-';

   buf[w][i] = '\0';
   w++;
   return buf[w - 1];

} /* end Lines */

/*----------------------------------------------------------------------------- 
 * NAME:      WrapText
 *
 * FUNCTION:  Wraps a text string at word boundary based on specified margins.
 *
 * EXECUTION ENVIRONMENT:
 *
 * CALLED BY: do_name_list do_prereqs do_APARs
 *
 * RETURNS:   Substring of the original string
 *
 *----------------------------------------------------------------------------*/

char * WrapText (char * source,
                 int    fld_width,
                 int  * offset)

/* s         = INPUT: input character string
 * fld_width = INPUT: width of the field the string must fit in .
 * offset    = I/O:   current processing offset into s */

{
   int           characters_remaining;
   static char   destination[MAX_FLD_WIDTH];
   int           new_line_offset;
   char        * new_line_position;
   int           number_of_spaces;
   int           rightmost_character;
   char        * start_of_segment;


   /* Objective: return the maximum number of 'words' from the given string
    * (source). */

   start_of_segment = source + (* offset);

   /* Are we done? */
   if (* start_of_segment == '\0')
      return (NIL (char));

   /* We have to see if there is a new-line that will force us to terminate. */

   new_line_position = strchr (start_of_segment, '\n');

   if (new_line_position != NIL (char))
   {
      new_line_offset = new_line_position - start_of_segment;
      if (new_line_offset < fld_width)
      {
         strncpy (destination, start_of_segment, (new_line_offset + 1));
         destination[new_line_offset] = '\0';   /* Clobber the new-line. */
         *offset += new_line_offset + 1;
         return (destination);
      }
   }

   /* Okay, a new line is not going to cut this segment short, copy as much as
    * we can, break on white-space (if possible). */

   /* Will our string end before we copy fld_width's worth of characters? */

   characters_remaining = strlen (start_of_segment);
   if (characters_remaining <= fld_width)
   {
      strcpy (destination, start_of_segment);
      *offset += characters_remaining;
      return (destination);
   }

   /* Starting at the fld_width + 1 character, find rightmost whitespace
    * character. */

   for (rightmost_character = fld_width;
        rightmost_character > 0;
        rightmost_character--)
   {
      if (isspace (start_of_segment[rightmost_character]))
      {
         strncpy (destination, start_of_segment, rightmost_character);
         destination[rightmost_character] = '\0';

         /* Eat trailing spaces and tabs. */

         number_of_spaces = strspn ((start_of_segment + rightmost_character),
                                    " \t");
         *offset += rightmost_character + number_of_spaces;

         /* Since we will do a new line when printing this segment, eat any
          * new line character that immediately follows this segment. */

         if (source[*offset] == '\n')
            (*offset)++;

         return (destination);
      }
   }

   /* The current 'word' must be longer than our field width.  Truncate it! */

   strncpy (destination, start_of_segment, fld_width);
   destination[fld_width] = '\0';
   *offset += fld_width;
   return (destination);

} /* end WrapText */

/*-----------------------------------------------------------------------------
 * NAME:      print_description 
 *
 * FUNCTION:  Loops on the description string passed to provide word-boundary,
 *            wrapped output.  Prints a newline if no description to print.
 *
 * EXECUTION ENVIRONMENT:
 *
 * CALLED BY: print_product_rec print_fix
 *
 * RETURNS:   void function.
 *
 *---------------------------------------------------------------------------*/


void print_description 
(
char * desc,                   /* String to be printed                      */
int    WrapText_fld_width,     /* Tells WrapText how much space to print in */
int    state_str_length,       /* Length of current state string            */
int    max_state_length,       /* Max state length to be printed            */
int    desc_start_col,         /* Starting column description               */
char * smit_hash               /* Hash character if called for SMIT listing */
) 
{
   char * s;
   int    first_line = TRUE;
   int    soff = 0;

   if (desc[0] !='\0')
   {
      while ((s = WrapText (desc, WrapText_fld_width, &soff)) != NIL(char))
      {
            if (first_line)
            {
               /*
                * Print as much of description as possible on same line as
                * level and state.
                */
               if (state_str_length < max_state_length)
                  /*
                   * right-pad shorter states so descriptions are alligned.
                   */
                  printf ("%-*s%s\n", 
                           max_state_length - state_str_length, " ", s);
               else
                  printf ("%s\n", s);
               first_line = FALSE;
            }
            else
            {
               printf ("%s%+*s%s\n",
                        smit_hash,
                        desc_start_col, " ",
                        s);
            }
      } /* end while */
   }
   else
      printf ("\n");

} /* print_description */

/*----------------------------------------------------------------------------- 
 * NAME:      free_unused_memory
 *
 * FUNCTION:  frees up memory used by fixinfo field if not needed.
 *
 * RETURNS:   success always
 *
 *----------------------------------------------------------------------------*/
int free_unused_memory (prod_t * entry)
{
   if (entry -> fixinfo != (char *) 0)
   {
      free (entry -> fixinfo);
      entry -> fixinfo = (char *) 0;
   }
   return (NO_ERROR);

} /* end free_unused_memory */


/*---------------------------------------------------------------------------- 
 * NAME:      free_done_memory
 *
 * FUNCTION:  frees up prereq and name fields in prod_t structure.
 *
 * RETURNS:   success always
 *
 *---------------------------------------------------------------------------*/
int free_done_memory (prod_t * entry)
{
  if (entry->prereq != (char *) 0)  {
         free(entry->prereq);
         entry->prereq = (char *) 0;
  }
  if (entry->name != (char *) 0)  {
         free(entry->name);
         entry->name = (char *) 0;
  }
         return (NO_ERROR);
} /* end free_done_memory */


/*----------------------------------------------------------------------------- 
 * NAME:      LPP_State
 *
 * FUNCTION:  Returns the state of the lpp.
 *
 * EXECUTION ENVIRONMENT:
 *
 * CALLED BY: do_prereqs do_dependents do_APARs
 *
 * RETURNS:   Address of a string 'fld_width' long of dashes.
 *
 *----------------------------------------------------------------------------*/

char * LPP_State (prod_t * fix_record)

/* fix_record   INPUT:  product record containing lpp state     */

{
   static char     state[STATE_FLD_WIDTH + 1];

   /* save the state */

   if (fix_record -> sceded_by[0] != '\0')
   {
      sprintf (state, "(%s)", fix_record -> sceded_by);
   }
   else
   {
      strcpy (state, LPP_STATE_STRING (fix_record -> state));
   }

   return (state);

} /* end LPP_State */


/*---------------------------------------------------------------------------- 
 * NAME:      fl_plus
 *
 * FUNCTION:  This routine allocates a new entry for the argument Product list
 *            structure
 *
 * EXECUTION ENVIRONMENT:
 *
 * INPUT:     global anchor to control structure - Fix_List
 *
 * OUTPUT:    additional entry allocated, address added to array of ptrs
 *            control block updated.  If original allocation has been exceeded,
 *            the array will be expanded.
 *
 * CALLED BY: build_fix_list retrieve_all_inputs
 *
 * RETURNS:   NO_ERROR - ok
 *            ERROR    - could not allocate storage, message has been issued.
 *
 *----------------------------------------------------------------------------*/

int fl_plus (void)
{
   if (Fix_List -> used >= Fix_List -> max)
   {    /* if all allocated slots used          */

      Fix_List = (struct prod_list_ctl *)
      realloc (Fix_List, (sizeof (int) + sizeof (int) +
                          (Fix_List -> max + 20) *sizeof (prod_t *)));

      /* expand by 20 entries                 */

      if (Fix_List != NIL (struct prod_list_ctl))
      {
         Fix_List -> max += 20;
      } /* record successful expansion          */
      else
      { /* unable to expand, error and exit     */

         fprintf (stderr,
                  vpd_catgets (MSGS_GENERAL, MSG_NOMEM, TEXT_NOMEM), progname);
         return (ERROR);
      } /* end unable to realloc ctl array      */
   }  /* end needed to realloc ctl array      */

   Fix_List -> ptrs[Fix_List -> used] = (prod_t *) malloc (sizeof (prod_t));
   if (Fix_List -> ptrs[Fix_List -> used] == NIL (prod_t))
   { /* unable to allocate - error, exit     */

      fprintf (stderr, vpd_catgets (MSGS_GENERAL, MSG_NOMEM, TEXT_NOMEM),
               progname);
      return (ERROR);
   }
   else
   {
      Fix_List -> used++;
   }    /* count newly allocated entry          */

   return (NO_ERROR);

} /* end fl_plus */


/*---------------------------------------------------------------------------- 
 * NAME:      fl_minus
 *
 * FUNCTION:  This routine frees the last entry in the product structure
 *
 * EXECUTION ENVIRONMENT:
 *
 * INPUT:     global anchor to control structure - Fix_List
 *
 * OUTPUT:    Last entry in list freed and 'used' count decremented
 *
 * CALLED BY: build_fix_list sort_u_product
 *
 * RETURNS:   NO_ERROR - ok
 *
 *---------------------------------------------------------------------------*/

int fl_minus (void)
{
   free (FL_LAST);
   Fix_List -> used--;
   return (NO_ERROR);

} /* end fl_minus */



/*----------------------------------------------------------------------------- 
 * NAME:      get_name
 *
 * FUNCTION:  This routine gets the lpp name from the product list. If the lpp
 *            name is unique, it will return just the lpp name in 'name'.  If
 *            the lpp is in the product data base multiple times, but with
 *            different version, release, mod, fix identifiers, this routine
 *            will return 'lpp_name ver.rel.mod.fix' in 'name'.
 *
 * EXECUTION ENVIRONMENT:
 *
 * CALLED BY: do_prereqs do_pid do_files do_APARs
 *
 * RETURNS:   none 
 *
 *---------------------------------------------------------------------------*/

int get_name (prod_t * fix_record,
                     char   * name)

/* fix_record   INPUT:  product record containing lpp name     */
/* name         OUTPUT: lpp name to use in report output       */

{
   int             result;
   prod_t          f_e, *fix_entry;

   fix_entry = &f_e;

   /* The key field id's should be the lpp name                 */

   strcpy (fix_entry -> lpp_name, fix_record -> lpp_name);

   /* Set the UPDATE field to retrieve only the install record     */

   fix_entry -> update = 0;

   /* call vpd to get first fix record                          */

   result = vpdget (PRODUCT_TABLE, PROD_LPP_NAME | PROD_UPDATE, fix_entry);

   if (result == VPD_OK)
      free_unused_memory (fix_entry);

   /* loop, allocating memory for another product record and calling    */
   /* vpd to fill in the record                                         */

   while (result == VPD_OK)
   {
      /* If the lpp name is not unique, stop looking.           */

      if ((fix_entry -> ver != fix_record -> ver) ||
          (fix_entry -> rel != fix_record -> rel) ||
          (fix_entry -> mod != fix_record -> mod) ||
          (fix_entry -> fix != fix_record -> fix))
      {
         break;
      }

      free_done_memory(fix_entry);
      result = vpdgetnxt (PRODUCT_TABLE, fix_entry);
   } /* end while (result == VPD_OK)     */


   /* add ver.rel.mod.fix to name  */

   sprintf (name, "%s %d.%d.%d.%d",
            fix_record -> lpp_name,
            fix_record -> ver, fix_record -> rel,
            fix_record -> mod, fix_record -> fix);

} /* end get_name */

/*---------------------------------------------------------------------------
 *  NAME:      history_swap
 *
 * FUNCTION:  This routine swap two records in a history list
 *
 * EXECUTION ENVIRONMENT:
 *
 * CALLED BY: sort_history
 *
 * RETURNS:   none
 *
 *--------------------------------------------------------------------------*/

static int history_swap (hist_t * h[],
                         int      i,
                         int      j)

/* h   = I/O:   list of history records i,j = INPUT: index of the two records
 * that should be swapped */

{
   hist_t         *tmp_hist;

   tmp_hist = h[i];
   h[i] = h[j];
   h[j] = tmp_hist;

} /* end history_swap */

/*---------------------------------------------------------------------------
 * NAME:      sort_history
 *
 * FUNCTION:  This routine will sort a list of history records based on time.
 *            The output list will be in reverse chronological order, i.e. most
 *            current to least current.
 *
 * EXECUTION ENVIRONMENT:
 *
 * CALLED BY: do_hist
 *
 * RETURNS:   none
 *
 *---------------------------------------------------------------------------*/

int sort_history (hist_t * hist_list[],
                         int      left,
                         int      right)

/* hist_list = I/O:   list of history records left      = INPUT: left most
 * record to process right     = INPUT: right most record to process */

{
   int             i, last;

   /* Do nothing if there is only 1 fix */

   if (left >= right)
   {
      return;
   }

   history_swap (hist_list, left, (left + right) / 2);
   last = left;
   for (i = left + 1; i <= right; i++)
   {
      if (hist_list[i] -> time > hist_list[left] -> time)
      {
         history_swap (hist_list, ++last, i);
      }
   }

   history_swap (hist_list, left, last);
   sort_history (hist_list, left, last - 1);
   sort_history (hist_list, last + 1, right);

} /* end sort_history  */


/*---------------------------------------------------------------------------
 * NAME:      sort_u_product
 *
 * FUNCTION:  This routine will sort a list of product records based on
 *            lpp_name, ver, rel, mod, fix, and PTF id. then delete any
 *            duplicate records from the sorted list
 *
 * EXECUTION ENVIRONMENT:
 *
 * CALLED BY: lslpp
 *
 * RETURNS:   none
 *
 *---------------------------------------------------------------------------*/

int sort_u_product (void)
{
   int             i;   /* index during search for duplicates */

   sort_product (0, FL_USED - 1);       /* initial sort                 */

   /* If the command_flag is -h, -d, or -f, then sort to get a unique     */
   /* record for each lpp_name ver.rel.mod.fix.ptf.  If the command_flag  */
   /* is -f, them do not use 'ptf' as an identifier to avoid getting      */
   /* multiple lists of product files.  If the command_flag is not -h,    */
   /* -d, or -f, include 'state' as an identifier of a unique record.     */

   if ((command_flag == FLAG_HISTORY) ||
       (command_flag == FLAG_DEPENDENTS) ||
       (command_flag == FLAG_FILES))
   {
      for (i = FL_USED - 1; i > 0; i--)
      {
         /* if the entries are duplicates */

         if ((strcmp (FL (i) -> lpp_name, FL (i - 1) -> lpp_name) == 0) &&
             (FL (i) -> ver == FL (i - 1) -> ver) &&
             (FL (i) -> rel == FL (i - 1) -> rel) &&
             (FL (i) -> mod == FL (i - 1) -> mod) &&
             (FL (i) -> fix == FL (i - 1) -> fix) &&
             ((strcmp (FL (i) -> ptf, FL (i - 1) -> ptf) == 0) ||
              (command_flag == FLAG_FILES)))
         {
            product_swap (i, FL_USED - 1);      /* move duplicate to end of
                                                 * list */
            fl_minus ();                  /* remove the duplicate from list */
         }
      } /* end of loop lookin for dups   */
   }
   else
   {    /* command_flag is not -h, -d, -p, or -f     */
      for (i = FL_USED - 1; i > 0; i--)
      {
         /* if the entries are duplicates */

         if ((strcmp (FL (i) -> lpp_name, FL (i - 1) -> lpp_name) == 0) &&
             (FL (i) -> ver == FL (i - 1) -> ver) &&
             (FL (i) -> rel == FL (i - 1) -> rel) &&
             (FL (i) -> mod == FL (i - 1) -> mod) &&
             (FL (i) -> fix == FL (i - 1) -> fix) &&
             (strcmp (FL (i) -> ptf, FL (i - 1) -> ptf) == 0) &&
             (FL (i) -> state == FL (i - 1) -> state))
         {
            product_swap (i, FL_USED - 1);      /* move duplicate to end of
                                                 * list */
            fl_minus ();        /* remove the duplicate from list */
         }

      } /* end of loop lookin for dups           */
   } /* end of handling -h, -d, -p, or -f     */

   sort_product (0, FL_USED - 1);       /* re-sort the list              */

} /* end sort_u_product */



/*--------------------------------------------------------------------------
 * NAME:      sort_product
 *
 * FUNCTION:  This routine will sort a list of product records based on
 *            lpp_name, ver, rel, mod, fix, and PTF id.
 *
 * EXECUTION ENVIRONMENT:
 *
 * CALLED BY: sort_u_product
 *
 * RETURNS:   none
 *
 *--------------------------------------------------------------------------*/

static int sort_product (int left,
                         int right)

/* prod_list = I/O:   list of product records left      = INPUT: left most
 * record to process right     = INPUT: right most record to process */

{
   int             i, j, last;

   /* Do nothing if there is only 1 fix */

   if (left >= right)
   {
      return;
   }

   product_swap (left, (left + right) / 2);

   last = left;
   for (i = left + 1; i <= right; i++)
   {
      j = strcmp (FL (i) -> lpp_name, FL (left) -> lpp_name);

      /* put the lpps in alphabetical order */

      if (j < 0)
      {
         product_swap (++last, i);
      }

      /* sort the ptfs within the lpps */

      else
         if (j == 0)
         {
            if (FL (i) -> ver < FL (left) -> ver)
            {
               product_swap (++last, i);
            }

            /* ver[i] >= ver[left] */

            else
               if (FL (i) -> ver == FL (left) -> ver)
               {
                  if (FL (i) -> rel < FL (left) -> rel)
                  {
                     product_swap (++last, i);
                  }

                  /* rel[i] >= rel[left] */

                  else
                     if (FL (i) -> rel == FL (left) -> rel)
                     {
                        if (FL (i) -> mod < FL (left) -> mod)
                        {
                           product_swap (++last, i);
                        }

                        /* mod[i] >= mod[left] */

                        else
                           if (FL (i) -> mod == FL (left) -> mod)
                           {
                              if (FL (i) -> fix < FL (left) -> fix)
                              {
                                 product_swap (++last, i);
                              }

                              /* fix[i] >= fix[left] */

                              else
                                 if (FL (i) -> fix == FL (left) -> fix)
                                 {
                                    j = strcmp (FL (i) -> ptf,
							FL (left) -> ptf);
                                    if (j < 0)
                                    {
                                       product_swap (++last, i);
                                    }

                                    /* ptf[i] >= ptf[left] */

                                    else
                                       if (j == 0)
                                       {
                                          if (FL (i) -> state <
                                                            FL (left) -> state)
                                          {
                                             product_swap (++last, i);
                                          }

                                       } /* if ptfs equal */
                                 } /* if fixes equal */
                           } /* if mods equal  */
                     } /* if rels equal  */
               } /* if vers equal  */
         } /* if lpp names equal */
   } /* for (i=left+1; i<=right; i++) */

   product_swap (left, last);
   sort_product (left, last - 1);
   sort_product (last + 1, right);

} /* end sort_product */


/*---------------------------------------------------------------------------
 * NAME:      product_swap
 *
 * FUNCTION:  This routine swap two records in a product list
 *
 * EXECUTION ENVIRONMENT:
 *
 * CALLED BY: sort_product
 *
 * RETURNS:   none
 *
 *---------------------------------------------------------------------------*/

static int product_swap (int i,
                         int j)

/* i,j = INPUT: index of the two records that should be swapped */

{
   prod_t         *tmp_prod;

   tmp_prod = FL (i);
   FL (i) = FL (j);
   FL (j) = tmp_prod;

} /* end product_swap */

/*---------------------------------------------------------------------------
 * NAME:      usage
 *
 * FUNCTION:  Describes the correct usage of the 'lslpp' command
 *
 * EXECUTION ENVIRONMENT:
 *
 * CALLED BY: lslpp
 *
 * RETURNS:   None 
 *---------------------------------------------------------------------------*/

int usage (void)
{

   fprintf (stderr, vpd_catgets (MSGS_LSLPP, MSG_LSLPP_USAGE,
                                 TEXT_LSLPP_USAGE),
            progname);

   fprintf (stderr, "\n");
   fprintf (stderr, vpd_catgets (MSGS_LSLPP, MSG_LSLPP_USAGE_OF_EX_FLAGS,
                                 TEXT_LSLPP_USAGE_OF_EX_FLAGS),
            progname);

} /* end usage */

/*----------------------------------------------------------------------------
 * NAME:      remove_blanks
 *
 * FUNCTION:  This routine removes the blanks and tabs and new lines from the
 *            beginning and end of a string.
 *
 * EXECUTION ENVIRONMENT:
 *
 * CALLED BY: do_prereqs
 *
 * RETURNS:   The string without the leading and trailing blanks, along with
 *            the new string length.
 *
 *---------------------------------------------------------------------------*/

int remove_blanks (char * str)

/* str  I/O:  character string               */

{
   int    i;

   /* remove the blanks and tabs and new lines from the front of the string */

   while (str[0] == ' ' || str[0] == '\t' || str[0] == '\n')
   {
      for (i = 0; i < strlen (str); i++)
      {
         str[i] = str[i + 1];
      }
      str[i] = '\0';
   }

   /* remove the blanks and tabs and new lines from the back of the string */

   i = strlen (str) - 1;
   while (i > 0 && (str[i] == ' ' || str[i] == '\t' || str[i] == '\n'))
   {
      str[i] = '\0';
      i = strlen (str) - 1;
   }

} /* end remove_blanks */

  /*--------------------------------------------------------------------*
   *
   *  Function:   get_state
   *
   *  Purpose:    To get the state of a particular product from the
   *              corresponding entry in the fix_info_table.
   *
   *  Returns:    A char which signifies the state.
   *
   *--------------------------------------------------------------------*/

char get_state(fix_info_type * fix)
{

  char  state;

  if (MIGRATING (fix->cp_flag))
  {
     return ('O');  /* obsolete */
  }  
  else if ( (fix->apply_state == BROKEN)
                           ||
            (fix->commit_state == BROKEN) )
       {
             state = 'B';                  /* broken state */
       }
  else if ( (fix->apply_state == PARTIALLY_APPLIED)
                               ||
            (fix->commit_state == PARTIALLY_COMMITTED) )
       {
            state = '?';                  /* inconsistent state */
       }

  else if ( (fix->apply_state == ALL_PARTS_APPLIED)
                           &&
            (fix->commit_state == ALL_PARTS_COMMITTED) )
       {
            state = 'C';            /* committed state */
       }

  else if ( (fix->apply_state == ALL_PARTS_APPLIED) )
       {
           state = 'A';             /* applied state */
       }

  else if ( (fix->apply_state == AVAILABLE) &&
            !(fix->flags & CKP_ST_APPLYING) )
       {
           if (fix->superseded_by != NIL(supersede_list_type))
               state = '-';
           else
               state = 'N';             /* available state */
       }

  else
        state = '?';

  return (state);

}  /**** end get_state  ****/

  /*--------------------------------------------------------------------*
   *
   *  Function:   locate_fix_local
   *
   *  Purpose:    To locate a particular fix in the fix_info_table list
   *              given the product name and ptf id.
   *
   *  Returns:    Returns a pointer to the particular fix entr in the
   *              fix_info_table list. Returns NIL if no entry is
   *              found.
   *
   *--------------------------------------------------------------------*/


fix_info_type * locate_fix_local (char * name,
                                  char * ptf)
{
   fix_info_type * fix;
   ENTRY         * hash_entry;

   hash_entry = locate_hash_entry (name, ptf);
   if (hash_entry == NIL (ENTRY) )
      return (NIL (fix_info_type) );

   for (fix = (fix_info_type *) (hash_entry -> data);
        fix != NIL (fix_info_type);
        fix = fix -> collision_list)
    {
      /*
       * The hash loook-up returns a list of one or more fix nodes with either
       * the same name or the same ptf id.  Since we're ALWAYS passed a ptf
       * we will ALWAYS hash according to ptf, so our collision list may
       * contain different names with the same ptfid.  Let's get the name
       * we were asked for.
       */
      if (strcmp (fix -> name, name) == 0)
      {
         return (fix);
      } /* end if */
    }

   return (NIL (fix_info_type) );

} /* end locate_fix_local */

  /*--------------------------------------------------------------------*
   *
   *  Function:   print_legend
   *
   *  Purpose:    To print the legend at the end of the output.
   *
   *  Returns:    This is a void function.
   *
   *--------------------------------------------------------------------*/


void print_legend (void)
{

    fprintf (stdout, vpd_catgets (MSGS_LSLPP, MSG_LSLPP_LEGEND,
                                  TEXT_LSLPP_LEGEND));

}   /**** end of print_legend ****/

  /*--------------------------------------------------------------------*
   *
   *  Function:   sort_inputs
   *
   *  Purpose:    To sort the inputs given on the command line
   *
   *  Returns:    This is a void function.
   *
   *--------------------------------------------------------------------*/

void sort_inputs (int   input_count,
                  char  *inputs[])
{
  int    input;
  int    other_input;
  char   temp[MAX_LPP_FULLNAME_LEN];

  for (input = 0;
       input < (input_count - 1);
       input++)
   {
     for (other_input = (input + 1);
          other_input < input_count;
          other_input++)
      {
        if (strcmp (inputs[input], inputs[other_input]) > 0)
         {
           /* Swap */

           strcpy (temp, inputs[input]);
           strcpy (inputs[input], inputs[other_input]);
           strcpy (inputs[other_input], temp);
         } /* end if */
      } /* end for */
   } /* end for */

} /* end sort_inputs */

 /*----------------------------------------------------------------------*
  *
  * Function:    get_description
  *
  * Purpose:     To retrieve a description to print for a fileset.
  *              If fix doesn't have a description, check the lpp
  *              record for the base level.  If still no description,
  *              return "Fileset Update" if the fileset is an update.
  *              Else return empty string.
  *
  * Returns:     A character pointer to the description of the product.
  *
  *----------------------------------------------------------------------*/


char * get_description (long cp_flag, char * name, char * in_desc)
{
    lpp_t       lpp_e, *lpp;
    int         result;
    static char desc [MAX_LPP_FULLNAME_LEN];

    if (in_desc [0] != '\0')
       return (in_desc);

    lpp = &lpp_e;
    desc [0] = '\0';
    strcpy (lpp->name, name);

    /*-------------------------------------------------------------*
     *   If the product has a user part then search the USR_VPD for
     *   that product.
     *-------------------------------------------------------------*/

    if (cp_flag & LPP_USER)
    {
         vpdremotepath (USR_VPD_PATH);
         vpdremote();
    }

    /*------------------------------------------------------------------*
     *   If the product has a share part then search the SHARE_VPD for
     *   that product.
     *------------------------------------------------------------------*/

    else if (cp_flag & LPP_SHARE)
    {
         vpdremotepath (SHARE_VPD_PATH);
         vpdremote();
    }

    result = vpdget (LPP_TABLE,
                     LPP_NAME,
                     lpp);

    if (result==VPD_OK)
        return (lpp->description);

    if (cp_flag & LPP_UPDATE)
       /* Return "Fileset Update" */
       strcpy (desc, vpd_catgets (MSGS_LSLPP, MSG_LSLPP_UPDT_PCKG,
                                  TEXT_LSLPP_UPDT_PCKG));

    return (desc);

} /* end get_description */


  /*--------------------------------------------------------------------*
   *
   *  Function:   cmpkmch (adapted from odmlike.c)
   *
   *  Purpose:    match the pattern to the string
   *
   *  Returns:    This is a void function.
   *
   *--------------------------------------------------------------------*/

int  cmpkmch ( pattern, string )

register char   *pattern;            /* WILDCARD PATTERN TO BE USED */
register char   *string;             /* STRING TO BE MATCHED        */

{

    register int  strcur;                /* CURRENT STRING CHARACTER      */
    register int  patcur;                /* CURRENT WILDCARD PATTERN CHAR */
    register int  retcode = FALSE;
    register int  notflag;
    int  lowchar;                        /* LOW CHAR IN [.-.] SEARCH      */
    int  highchar;                       /* HIGH CHAR IN [.-.] SEARCH     */
    int  looping;


    if (pattern == NULL || string == NULL)
      {
        return(-1);
      } /* endif */

    /* DETERMINE WHAT TYPE OF CHARACTER WE ARE MATCHING AGAINST */
    switch (*pattern)   {

        /* CHECK FOR AN OPENING BRACKET */
    case '[':

        /* GET THE CURRENT STRING CHARACTER TO MATCH. MASK TO 8 BITS */
        strcur = *string;
        strcur &= 0377;

        /* GET OUT IF TRYING TO MATCH AGAINST THE NULL-TERMINATOR */
        if (strcur == 0)
            break;

        pattern++;
        string++;

        /* THE LOWEST CHARACTER IS INITIALLY SET LOWER THAN ANY POSSIBLE CHAR */
        lowchar = -1;

        /* CHECK FOR ! */
        if (*pattern == '!')   {
            notflag = TRUE;
            pattern++;
          }
        else
            notflag = FALSE;

        looping = TRUE;

        /* LOOP UNTIL A ] IS FOUND */
        while (looping)   {
            patcur = *pattern++;
            patcur &= 0377;

            switch (patcur)   {
            case ']':

                /* IF THE ! WAS USED, SWAP THE RESULT OF THE MATCH. */
                if (notflag)   {
                    if (retcode)
                        retcode = FALSE;
                    else
                        retcode = TRUE;
                  }

                /* TRY TO MATCH THE REST OF THE STRING AFTER THE [...]       */
                /* IF WE HAVE SUCCESSFULLY MATCHED THE PART IN THE BRACKETS. */
                if (retcode)
                    retcode = cmpkmch (pattern, string);
                looping = FALSE;
                break;

            case '-':
                highchar = *pattern++;
                highchar &= 0377;

                /* CHECK TO SEE IF THE CHARACTER IS WITHIN THE RANGE */
                if ((strcur >= lowchar) && (strcur <= highchar))
                    retcode = TRUE;
                break;

            case 0:

                /* IF THE END OF THE PATTERN IS ENCOUNTERED BEFORE ], THEN ERROR */
                retcode = FALSE;
                looping = FALSE;
                break;

            default:

                /* GET THE LOW CHARACTER FOR FUTURE USE */
                lowchar = patcur;
                if (strcur == patcur)
                    retcode = TRUE;
                break;
              }
          }
        break;

    default:

        /* PATTERN AND CURRENT STRING CHARACTER MUST MATCH EXACTLY */
        if (*pattern != *string)
            break;

        /* IF THE CURRENT PATTERN CHARACTER MATCHES THE CURRENT STRING */
        /* CHARACTER, FALL THROUGH AS IF A MATCH ON ANY                */
        /* SINGLE CHARACTER HAD OCCURRED.                              */

    case '?':
        if (*string != '\0')   {
            pattern++;
            string++;
            retcode = cmpkmch (pattern, string);
          }
        break;

    case '*':

        /* AN * MATCHES ZERO OR MORE CHARACTERS.                  */
        /* IF THERE ARE MULTIPLE ASTERISKS, REMOVE THE DUPLICATES */
        while (*pattern == '*')
            pattern++;

        /* IF THERE ARE NO MORE CHARACTERS IN THE PATTERN, THEN SUCCESSFUL. */
        if (*pattern == '\0')
            retcode = TRUE;
        else   {
            while (*string != '\0')   {
                if (retcode = cmpkmch (pattern, string))
                    break;
                string++;
              }
          }
        break;

    case '\0':

        /* THE PATTERN HAS TERMINATED. */
        if (*string == '\0')
            retcode = TRUE;
        break;
      }


        return(retcode);

}
  /*--------------------------------------------------------------------*
   *
   *  Function:   ckwild
   *
   *  Purpose:    to check if wild char exists in the input string
   *
   *  Returns:    TRUE if shell wildchar. exists else FALSE
   *
   *--------------------------------------------------------------------*/

int ckwild(cptr)
char *cptr;
{
                while (*cptr) {
                        if (*cptr == '?' || *cptr == '[' || *cptr == '*' )
                                return(TRUE);
                        cptr++;
                }
                return(FALSE);
}
