static char sccsid[] = "@(#)34  1.19  src/bos/usr/bin/lslpp/process_L_flag.c, cmdswvpd, bos411, 9432A411a 8/5/94 09:37:58";

/*
 *   COMPONENT_NAME: CMDSWVPD
 *
 * FUNCTIONS: process_L_flag, 
 *            list_all_products, 
 *            list_selected_products, 
 *            print_product_info,
 *            locate_first_occurance_of_product_name,
 *            print_base_or_latest_maint_lev, 
 *            tag_contents_of_maintenance_level,
 *            get_prereq_ptf, 
 *            print_fix, 
 *            print_superseded_info, 
 *            process_vrmf_filesets,
 *            locate_next_occurance
 *
 * ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <lslpp.h>
#include <check_prereq.h>
#include <sys/types.h>

static void list_all_products (boolean * error);
static void list_selected_products (boolean * error);
static void print_product_info (fix_info_type * product_entry,
                                boolean       * error);
static fix_info_type * locate_first_occurance_of_product_name (char * name);
static
void  print_base_or_latest_maint_lev (fix_info_type * product_entry,
                                                     boolean * error);
static void tag_contents_of_maintenance_level (fix_info_type * maintenance_ptf,
                                               boolean       * error);
static void get_prereq_ptf (char * prereq_string,
                            char * ptf);

static void print_fix (fix_info_type * fix);

static void print_superseded_info (fix_info_type * fix);

static void process_vrmf_filesets (fix_info_type * first_entry_for_prod);

static fix_info_type * locate_next_occurance(char *name,fix_info_type * fix);

boolean  print_maintenance_level;

#define IS_INSTALLED(fix)                                                 \
          ((fix -> parts_applied != 0)     ||                             \
           (fix -> apply_state == BROKEN)  ||                             \
           (fix -> flags & CKP_ST_APPLYING))
         
/*
 * NAME:      process_L_flag
 *
 * FUNCTION:  Print out VPD lpp name information.
 *
 * EXECUTION ENVIRONMENT:
 *
 *            CALLED BY: lslpp
 *
 * RETURNS:   VPD_OK
 *
 */


void process_L_flag (void)
{
   boolean error;

   /*--------------------------------------------------------------------*
    *  Read the entire contents of the product vpd, merging USR/ROOT/SHARE
    *  information. Use the check_prereq function "load_fix_info_table" to
    *  read the vpd info into a fix_info_type structure.
    *------------------------------------------------------------------- */

   error = FALSE;

   check_prereq.mode = OP_APPLY;
   check_prereq.parts_to_operate_on = LPP_USER | LPP_ROOT | LPP_SHARE;
   check_prereq.First_Option = NIL (Option_t);
   check_prereq.SOP = NIL (Option_t);
   check_prereq.keep_apar_info = FALSE;
   check_prereq.keep_description_info = TRUE;
   check_prereq.called_from_ls_programs = TRUE;
   check_prereq.called_from_lslpp = TRUE;
   check_prereq.ignore_AVAILABLE_entries = TRUE;
   ckp_errs = SCREEN_LOG;

   /*
    * True argument tells load_fix_info it's being called without a SOP.
    */
   load_fix_info_table (TRUE, & error);
   if (error)
      return;
 
   /* Now that we have all of the info at our disposal, we can print.
 
      If no product names are specified, then all installed products will be
      assumed.
 
      lslpp -L
 
      Will list the latest installed fileset for an option.  If the option
      is a 3.2 formatted pkg, will list the latest maintenance level and
      the latest delta above the maintenance level.
 
      lslpp -La

      Will list, in addition to what lslpp -L lists, all of the superseded
      PTFs for the given product(s).
   */

   /* print header if necessary */
   if (! no_header) {
      if (colons)
      {
          fprintf (stdout, "#%s:%s:%s:%s:%s:%s:%s:%s:%s\n",
                            HEADER_STRING(H_PROD_NAME),
                            HEADER_STRING(H_NAME),
                            HEADER_STRING(H_RELEASE),
                            HEADER_STRING(H_LPP_STATE),
                            HEADER_STRING(H_FIX_ID),
                            HEADER_STRING(H_FIX_STATE),
                            HEADER_STRING(H_TYPE),
                            HEADER_STRING(H_COMMENT));
      }
      else
      {
          fprintf (stdout, "  %s  %+25s  %s  %s\n",
                   HEADER_STRING(H_NAME),
                   HEADER_STRING(H_RELEASE),
                   HEADER_STRING(H_STATE),
                   HEADER_STRING(H_COMMENT));

          fprintf (stdout, "  %s\n",
                   Lines(MAX_DASH_LINE_LENGTH));
      }
   }

   if ( (input_count == 1) && (strcmp ((inputs[0]), "*") == 0 ) )
   {
      /*---------------------------------------------------------------*
       *   The user specified "all" as input.
       *---------------------------------------------------------------*/
      list_all_products (&error);
   }
   else
   {
      /*---------------------------------------------------------------*
       *   Sort names specified by user.
       *---------------------------------------------------------------*/
      sort_inputs (input_count, inputs);
      list_selected_products (&error);
   }
   if (error)
       return;
   if (!colons)
       print_legend();
   return;

} /* end do_name_list */



  /*--------------------------------------------------------------------*
   *
   *  Function:   list_all_products
   *
   *  Purpose:    List everything installed.  (ie. no filtering, search 
   *              criteria specified -- name, update id, etc.)
   *
   *  Returns:    This is a void function.
   *
   *--------------------------------------------------------------------*/


static void list_all_products (boolean * error)
{
  fix_info_type * fix;

  /*---------------------------------------------------------------*
   *   Start at the beginning of the fix_info_table and list each
   *   node which meets the criteria specified by the options to lslpp
   *---------------------------------------------------------------*/

  for (fix = check_prereq.fix_info_anchor -> next_fix;
       fix != NIL (fix_info_type);
       fix = fix -> next_fix)
   {
      if ((fix -> op_type & OP_TYPE_INSTALL)
                      &&
          (IS_INSTALLED (fix)))
       {
         print_product_info (fix, error);
         if (* error)
            return;
       }
   } /* end for */

} /* end list_all_products */


  /*--------------------------------------------------------------------*
   *
   *  Function:   list_selected_products
   *
   *  Purpose:    To list all the required information about the
   *              products entered on the command line.
   *
   *  Returns:    This is a void function.
   *
   *--------------------------------------------------------------------*/


static void list_selected_products (boolean * error)
{
   int  i;
   fix_info_type  * fix;

   for (i = 0; i < input_count; i++)
   {
     fix = locate_first_occurance_of_product_name (inputs[i]);
     if (fix == NIL (fix_info_type))
     {
         fprintf (stderr,
                  vpd_catgets(MSGS_LSLPP,
                              MSG_LSLPP_PRODUCT_NOT_INSTALLED,
                              TEXT_LSLPP_PRODUCT_NOT_INSTALLED ),
                              progname, inputs[i]);
         * error = TRUE;
         return;
     }
     if ( (fix -> op_type & OP_TYPE_INSTALL)
                    &&
          (IS_INSTALLED (fix)))
     {
         print_product_info (fix, error);
         if (* error)
            return;
     }
     if (ckwild(inputs[i]))  {
        while (fix->next_fix) { /*process the rest of the list to match wild card*/
            fix=locate_next_occurance(inputs[i],fix->next_fix);
            if (fix == NIL (fix_info_type))
            {
                break;
            }
            if ( (fix -> op_type & OP_TYPE_INSTALL)
                              &&
                 (IS_INSTALLED (fix)))
            {
                print_product_info (fix, error);
                if (* error)
                    return;
            }
        }
     }
   }  /**** end for ****/
} /* end list_selected_products */


  /*--------------------------------------------------------------------*
   *
   *  Function:   print_product_info
   *
   *  Purpose:    Decide what information is to be printed based on the
   *              options given to lslpp.
   *
   *  Returns:    This is a void function.
   *
   *--------------------------------------------------------------------*/


static void print_product_info (fix_info_type * product_entry,
                                boolean       * error)
{
  fix_info_type * fix;

  if (base_levels_only)
  {
     print_fix (product_entry); 
     return;
  }

  /*
   * 4.1 filesets are handled differently.  Pre-4.1 filesets are handled
   * as they always were.
   */
  if (IF_4_1 (product_entry->op_type))
  {
     process_vrmf_filesets(product_entry);
     return;
  }
  /*--------------------------------------------------------------------*
   *  Need to print out the maintenance level(s) of the product. This is
   *  either the installation level, or the level of the latest/all
   *  maintenance PTFs. Once found, mark that product as being visited
   *  so that we do not print it again if we come across it again.
   *--------------------------------------------------------------------*/

   print_base_or_latest_maint_lev (product_entry, error);

  /*-----------------------------------------------------------------*
   *   Search the fix_info_table list for information about the "delta"
   *   and/or the superseded information for the current product.
   *-----------------------------------------------------------------*/

   for (fix = product_entry -> next_fix;
       fix != NIL (fix_info_type);
       fix = fix -> next_fix)
   {
      if (strcmp (fix -> name, product_entry -> name) != 0)
         return;  /* We are done with this product. */

     /*--------------------------------------------------------------*
      *  Suppress printing superseded nodes unless -La.
      *--------------------------------------------------------------*/
 
     if ((strlen (fix -> superseding_ptf) != 0) && !all_info)
        fix -> flags |= VISITING_THIS_NODE;

     /*-----------------------------------------------------------*
      *  If the current node has not been visited (or suppressed above)
      *  and its superseding ptf is nil, then print the necessary info.
      *  (NOTE: superseded ptfs will be printed in print_fix if necessary)
      *-----------------------------------------------------------*/

      if ( (! (fix -> flags & VISITING_THIS_NODE) )
                       &&
          ( strlen (fix -> superseding_ptf) == 0) )  
      {                                             
         print_fix (fix);                   
      }
   } /* end for */

} /* end print_product_info */


  /*--------------------------------------------------------------------*
   *
   *  Function:   locate_first_occurance_of_product_name
   *
   *  Purpose:    Locates the first occurance of the product in the
   *              fix_info_table_list.
   *
   *  Returns:    Returns a pointer to the first fix_info entry for a
   *              product. It will be NIL if no matching entry is found.
   *
   *--------------------------------------------------------------------*/


static fix_info_type * locate_first_occurance_of_product_name (char * name)
{
        int     reflg;
  fix_info_type * fix;

  if (ckwild(name))
        reflg=TRUE;
  else
        reflg=FALSE;
  for (fix = check_prereq.fix_info_anchor->next_fix;
       fix != NIL (fix_info_type);
       fix = fix -> next_fix)
   {
    if (reflg) {
        if (cmpkmch(name,fix->name))
                return(fix);
    }
    else {
     if ( strcmp (fix -> name, name) == 0)
      {
         return (fix);
      }
    }
   } /* end for */

  return (NIL (fix_info_type));

} /* end locate_first_occurance_of_product_name */

  /*--------------------------------------------------------------------*
   *
   *  Function:   locate_next_occurance
   *
   *  Purpose:    Locates the next occurance of the product in the
   *              fix_info_table_list.
   *
   *  Returns:    Returns a pointer to the next fix_info entry for a
   *              product. It will be NIL if no matching entry is found.
   *
   *--------------------------------------------------------------------*/


static fix_info_type * locate_next_occurance(char *name,fix_info_type *nextfix)
{
        int     reflg;
  fix_info_type * fix;

  for (fix = nextfix;
       fix != NIL (fix_info_type);
       fix = fix -> next_fix)
   {
        if (cmpkmch(name,fix->name))
                return(fix);
   } /* end for */

  return (NIL (fix_info_type));

} /*end locate_next_occurance */

  /*--------------------------------------------------------------------*
   *
   *  Function:   print_base_or_latest_maint_lev
   *
   *  Purpose:    To find the installation / latest maintenance level
   *              of a product.
   *
   *  Returns:    This is a void function.
   *
   *--------------------------------------------------------------------*/


static
void  print_base_or_latest_maint_lev (fix_info_type * product_entry,
                                                     boolean * error)
{
  fix_info_type       * fix;
  fix_info_type       * latest_maintenance_level;
  supersede_list_type * supersede;

  print_maintenance_level = FALSE;
  for (fix = product_entry;
       fix != NIL (fix_info_type);
       fix = fix -> next_fix)
   {
     if (strcmp (fix -> name, product_entry -> name) != 0)
     {
        if (!( print_maintenance_level))
          print_fix (product_entry);
        product_entry -> flags |= VISITING_THIS_NODE;
        return;
     }

    /*---------------------------------------------------------------*
     *  Go to the next entry in the table if we have already visited
     *  the current entry. This fixes Defect 108786 in which the
     *  maintenance levels were being printed twice .
     *---------------------------------------------------------------*/

     if ( (fix -> flags & VISITING_THIS_NODE) )
          continue;

    /*--------------------------------------------------------------------*
     *   If it is a "C" or "ML" type update and has been applied/committed
     *--------------------------------------------------------------------*/

      if ( IS_MAINT_LEV (fix)
                      &&
          (fix -> parts_applied != 0) )
      {

        /*--------------------------------------------------------------------*
         *  Okay, we have a maintenance level that is applied.  Let's find the
         *  latest maintenance level.  This is easy, just follow the supersede
         *  chain until we find the last applied one.  The superseding_fix list
         *  is ordered in increasing supersede order.
         *-------------------------------------------------------------------*/

         latest_maintenance_level = fix;

         for (supersede = latest_maintenance_level -> superseded_by;
              supersede != NIL (supersede_list_type);
              supersede = supersede->superseding_fix->superseded_by)
         {
             if (supersede->superseding_fix->parts_applied != 0)
                latest_maintenance_level = supersede->superseding_fix;
         }
         print_maintenance_level = TRUE;
         print_fix (latest_maintenance_level);
         
        /*---------------------------------------------------------------*
         *  Tag the maintenance level as being VISITED so that we don't
         *  print it again if we come across it again.
         *---------------------------------------------------------------*/

         latest_maintenance_level -> flags |= VISITING_THIS_NODE; 

         /*-----------------------------------------------------------------*
          *  if just -L, we want to suppress printing any PTFs that are 
          *  members of the maintenance PTF.   
          *-----------------------------------------------------------------*/

         if (! all_info)
            tag_contents_of_maintenance_level (latest_maintenance_level, error);

         /*---------------------------------------------------------------*
          * Mark the base level as being visited as we do not want it to
          * be printed if we have printed a maintenance level.
          *---------------------------------------------------------------*/ 

         product_entry -> flags |= VISITING_THIS_NODE;
      }
   } /* end for */

   /*---------------------------------------------------------------------*
    *   if we did not find a "C" or "ML" type update, print the base level.
    *---------------------------------------------------------------------*/
    if (!print_maintenance_level)
    {
       print_fix (product_entry);
       product_entry -> flags |= VISITING_THIS_NODE;
    }

} /* end print_base_or_latest_maint_lev */



  /*--------------------------------------------------------------------*
   *
   *  Function:   tag_contents_of_maintenance_level
   *
   *  Purpose:    To tag the prereqs of a cumulative update as being
   *              visited. This will prevent the prereqs from being
   *              printed.
   *
   *  Returns:   This is a void function.
   *
   *--------------------------------------------------------------------*/


static void tag_contents_of_maintenance_level (fix_info_type * maintenance_ptf,
                                               boolean       * error)
{
  fix_info_type * fix;
  char            fix_name[MAX_LPP_FULLNAME_LEN];
  char            ptf[MAX_PROD_PTF];

  if ( ! IS_MAINT_LEV (maintenance_ptf) )
     return;

  /*-----------------------------------------------------------------------*
   *  We have to brute force parse the requisites of this PTF and tag every
   *  requisite.
   *-----------------------------------------------------------------------*/

  while (TRUE)
  {
     get_prereq_ptf (maintenance_ptf -> prereq_file, ptf);
     if (ptf[0] == '\0')
        return;

     fix = locate_fix_local (maintenance_ptf -> name, ptf);
     if (fix == NIL (fix_info_type))
      {
        fprintf (stderr,
                 vpd_catgets(MSGS_LSLPP,
                             MSG_LSLPP_VPD_CORRUPTED,
                             TEXT_LSLPP_VPD_CORRUPTED ),
                             progname,
                             ptf,
                             get_fix_name_from_fix_info (maintenance_ptf,
                                                         fix_name));
        * error = TRUE;
        return;
      }
     fix -> flags |= VISITING_THIS_NODE;
  } /* end while */
} /* end tag_contents_of_maintenance_level */



  /*--------------------------------------------------------------------*
   *
   *  Function:   get_prereq_ptf
   *
   *  Purpose:    Gets the information about the prereqs of a maintenance
   *              update.
   *
   *  Returns:   This is a void function.
   *
   *--------------------------------------------------------------------*/


static void get_prereq_ptf (char * prereq_string,
                            char * ptf)
 {
         boolean   finished;
         int       ptf_length;
  static char    * old_prereq_string;
  static char    * requisite_ptfs;

  ptf[0] = '\0';

  if (old_prereq_string != prereq_string)
   {
     requisite_ptfs = prereq_string;
     old_prereq_string = prereq_string;
   }

  if ( * requisite_ptfs == '\0')
   {
     return;
   }
  else
   {
     /* The prereqs are of the form:

            <junk> p|P {white_space} = {white_space} <ptf_id> */

     finished = FALSE;
     while (! finished)
      {
        /* Eat until we find p|P */

        requisite_ptfs = strpbrk (requisite_ptfs, "pP");
        if (requisite_ptfs == NIL (char))
           return;

        /* Point past the 'p' */

        requisite_ptfs++;

        /* Eat any white space */

        requisite_ptfs += strspn (requisite_ptfs, " \t\n");

        if (*requisite_ptfs == '=')
           finished = TRUE;
      }

     /* Skip past the '=' */

     requisite_ptfs++;

     /* Eat more whitespace! (Its high in fiber) */

     requisite_ptfs += strspn (requisite_ptfs, " \t\n");

     /* Copy the requisite PTF somewhere else. */

     ptf_length = strcspn (requisite_ptfs, " \t\n");
     strncpy (ptf, requisite_ptfs, ptf_length);
     ptf[ptf_length] = '\0';
     requisite_ptfs += ptf_length;
   }

} /* end get_prereq_ptf */


  /*--------------------------------------------------------------------*
   *
   *  Function:   print_fix
   *
   *  Purpose:    To print the description, level, state and ptf info
   *              for a product.
   *
   *  Returns:    This is a void function.
   *
   *--------------------------------------------------------------------*/


static void print_fix (fix_info_type * fix)
{
  boolean       split_name_lev;
  boolean       print_name;
  int           ln;
  int           ll;
  char        * desc;
  char          level_str [MAX_LPP_FULLNAME_LEN];
  char          ptf [MAX_PROD_PTF];
  static char * previous_name;

  /*
   * Check then set (if check failed) a bit indicating we've been
   * here before, hence printed this node before.  (Also, get out if
   * this is available (ie. dummy 3.2 superseded record) and we are
   * not doing -La. Do, however, print availables marked with APPLYING
   * tag -- they're not really available).
   */
   if ((fix->flags & VISITING_THIS_NODE) 
                      ||  
       ((! all_info)                    && 
        (fix->apply_state == AVAILABLE) &&
        ! (fix->flags & CKP_ST_APPLYING)))
       return;
    fix->flags |= VISITING_THIS_NODE;

   desc = get_description (fix->cp_flag, fix->name, fix->description);

   if (colons)
   {
      /*
       * VSM, currently the only dependent application of this function, needs 
       * blank fields for missing information.
       */
      if (fix->level.ptf[0] == '\0')
         strcpy (ptf, " ");
      else
         strcpy (ptf, fix->level.ptf);
 
      fprintf (stdout, "%s:%s:%d.%d.%d.%d:%c:%s:%c:%c:%s\n", 
                        fix->product_name, 
                        fix->name,
                        fix->level.ver, fix->level.rel, 
                        fix->level.mod, fix->level.fix,
                        ' ',  /* No longer meaningful, VSM expects it. */
                        ptf,
                        get_state (fix), 
                        get_type (fix, NIL(prod_t)),
                        desc);
      return;
   }

   /*
    * convert level to string.
    */
   get_level_from_fix_info (fix, level_str);

   split_name_lev = FALSE;
   print_name = FALSE;
   if (strcmp (previous_name, fix -> name) != 0)
   {
      /*-------------------------------------------------------------------*
       *   New product.  Determine if we need to print name and level on 
       *   a separate line.
       *-------------------------------------------------------------------*/
      print_name = TRUE;
      ln = strlen (fix->name);
      ll = strlen (level_str);
      if ((ln + ll + NAME_LEV_SPACE) > MAX_NAME_LEV_FOR_1_LINE)
         split_name_lev = TRUE;
      previous_name = fix->name;
   }
  
   if (split_name_lev)
      fprintf (stdout, "  %s\n  %+*s    %c    ", 
                        fix->name,
                        MAX_NAME_LEV_FOR_1_LINE, level_str,
                        get_state(fix)); 
   else
      if (print_name)
         fprintf (stdout, "  %s  %+*s    %c    ",
                           fix->name,
                           MAX_NAME_LEV_FOR_1_LINE - 
                                       (ln + NAME_LEV_SPACE), level_str,
                           get_state(fix));
      else
         fprintf (stdout, "  %+*s    %c    ",
                           MAX_NAME_LEV_FOR_1_LINE, level_str,
                           get_state (fix));

   print_description (desc, 32, 1, 1, 45, "");
 
   /*--------------------------------------------------------------*
    *    Print the superseded info if "-La" is specified.
    *--------------------------------------------------------------*/

   if ( (all_info)
              &&
       (fix->supersedes != NIL (supersede_list_type)) )
   {
      fprintf (stdout, "%+40s\n", vpd_catgets(MSGS_LSLPP,
                                             MSG_LSLPP_SUPERSEDES,
                                             TEXT_LSLPP_SUPERSEDES));
      print_superseded_info (fix);
   }

} /* end print_fix */


  /*--------------------------------------------------------------------*
   *
   *  Function:   print_superseded_info
   *
   *  Purpose:    To print the description, name, level , state and ptf
   *              id of the superseded products.
   *
   *  Returns:    This is a void function.
   *
   *--------------------------------------------------------------------*/

static void  print_superseded_info (fix_info_type * fix)
{

   supersede_list_type   * sup;
   fix_info_type         * dup;

   if (fix->supersedes != NIL (supersede_list_type))
   {
       for ( sup = fix->supersedes;
             sup != NIL (supersede_list_type);
             sup = sup->next_supersede)
       {
             dup = sup->superseded_fix;
             if (dup == NULL)
             {
                 fprintf (stderr,
                          vpd_catgets(MSGS_LSLPP,
                                      MSG_LSLPP_VPD_CORRUPTED,
                                      TEXT_LSLPP_VPD_CORRUPTED ),
                                      progname, sup->superseding_fix->name,
                                      sup->superseding_fix->level.ptf);
                 return;
             }
             else
             {
                  fprintf (stdout, "%+38s  %c    ",
                                   dup->level.ptf,
                                   get_state (dup));
                  print_description (dup->description, 32, 1, 1, 45, "");
             }

          /* Recurse to get all fixes superseded by this fix. */

          print_superseded_info (dup);

       }  /**** end for ****/
   }  /**** end if ****/
}   /**** end print_supersede_info ****/


  /*--------------------------------------------------------------------*
   *
   *  Function:   process_vrmf_filesets
   *
   *  Purpose:    Determines which non-32 filesets to report, based on
   *              flags specified.  Calls print routines to do reporting.
   *              -L  -- show latest fileset product; regardless of type
   *                     (same -l)
   *              -La -- show all filesets for product (same as -la)
   *
   *  Returns:    Void function.
   *
   *--------------------------------------------------------------------*/

static void process_vrmf_filesets (fix_info_type * first_entry_for_prod)
{
   fix_info_type * fix;
   fix_info_type * prev_fix;

   /*
    * Loop until end of list or name change.
    */
   for (fix = first_entry_for_prod;
        (fix != NIL (fix_info_type) &&
         strcmp (fix->name, first_entry_for_prod->name) == 0);
        fix = fix -> next_fix)
   {
      if (all_info)
         print_fix (fix);
      prev_fix = fix; 
   }

   if (!all_info)
      print_fix (prev_fix);

} /* process_vrmf_filesets */
