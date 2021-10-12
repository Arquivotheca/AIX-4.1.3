static char sccsid[] = "@(#)41  1.5.1.12  src/bos/usr/bin/lppchk/lppchkv.c, cmdswvpd, bos411, 9440A411d 10/4/94 16:13:38";
/*
 * COMPONENT_NAME: (CMDSWVPD) Software VPD
 *
 * FUNCTIONS: 
 *           convert_ckp_state_to_vpd_state
 *           get_state_description
 *           get_state_str_index
 *           lppchkv  
 *           print_group_members
 *           print_lppchk_requisite_subgraph
 *           print_sync_action
 *           
 *             
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

#include        <lppchk.h>      /* local and standard defines           */
#include        <check_prereq.h>
#include        <inu_string.h>

#define MAX_LINE_WIDTH  80
#define ST_UNKNOWN      11

static void add_missing_req_to_printed_list (
char          * req_name,
criteria_type * req_expr,
boolean       * error);

static void convert_ckp_state_to_vpd_state (
fix_info_type * fix,
int           * fix_state,
int           * usr_state,
int           * root_state);

static int get_state_str_index (
int             state);

static char * get_state_description (
fix_info_type * fix,
char          * out_buf); 

static void print_group_members (
fix_info_type * group_node);

static void print_lppchk_requisite_subgraph (
fix_info_type       * parent,
requisite_list_type * link_from_parent,
fix_info_type       * fix,
char                * fix_name,
boolean             * error);

static void print_sync_action (
fix_info_type * fix);

static boolean is_missing_req_printed (
char          * req_name,
criteria_type * req_expr);

static void init_missing_req_printed_list ();

/*
 * Global structures and variables for handling duplicate requisite 
 * expressions when printing the contents of a requisite containing
 * missing requisites (when verbose output is requested).
 */
typedef struct {
  char          * req_name;
  criteria_type * req_expr;
} missing_req_struct;

#define INIT_MISSING_REQ_PL_SIZE  100
int missing_req_pl_size = INIT_MISSING_REQ_PL_SIZE;

missing_req_struct * missing_req_printed_list;
int missing_req_pl_tail;

/*----------------------------------------------------------------------------
** NAME: lppchkv (verify software level)
**
** FUNCTION: verify the installation level of software packages
**
** EXECUTION ENVIRONMENT:
**
**      For each installation package that has been applied or committed
**      ensure "multiple part" installation packages are consistent
**      and that all requisites are installed.
**
** RETURNS: 0 - no errors detected
**          non-zero - processing errors found. (corresponding to number
**          of errors found).
**
**---------------------------------------------------------------------------*/

int 
lppchkv
(
char  *lppname,               /* pointer to lpp_name spec to check    */
int   updt                    /* update flag - special output format  */ 
)			
{
  boolean         error = FALSE; 
  boolean         do_all;
  boolean         found;
  boolean         passed;
  boolean         printed_hdr_msg;
  boolean         req_failures_seen;
  char            fix_name   [MAX_LPP_FULLNAME_LEN];
  char            fix_name2  [MAX_LPP_FULLNAME_LEN];
  char            lppname_substr [MAX_LPP_FULLNAME_LEN] = "";
  char            state_str [MAX_LPP_FULLNAME_LEN];
  char          * ptr;
  ENTRY         * hash_entry = NIL (ENTRY);
  fix_info_type * fix = NIL (fix_info_type);
  fix_info_type * same_name_fix;
  int             rc = 0;
  int             err_cnt = 0;
  requisite_list_type * req;

  /*-------------------------------------------------------------------*
   *  STRATEGY:
   *  Use the ckprereq graph construction routines (load_fix_info() and
   *  build_graph()) to construct the requisite relationships between
   *  installed packages.  The resultant data structure is a linear, 
   *  doubly-linked fix_info table with requisite links created between
   *  members of the fix_info table.  Scan the fix_info table several
   *  times to establish requisite inconsistencies and then to report
   *  those inconsistencies. 
   *-------------------------------------------------------------------*/

  /*-------------------------------------------------------------------*
   *  Initialize variables expected by ckprereq routines then call 
   *  load_fix_info to build the linear list of nodes representing
   *  the consolidation of the usr, root and share Vital Product Databases.
   *  Explicit supersedes relationships (sceded_by info) will be built 
   *  upon return also.
   *-------------------------------------------------------------------*/
  check_prereq.mode = OP_APPLY;                    /* fake an apply op.     */
  check_prereq.parts_to_operate_on = LPP_USER | LPP_ROOT | LPP_SHARE;
                                                   /* ALL PARTS ALWAYS      */
  check_prereq.First_Option = NIL (Option_t);      /* No toc involved       */
  check_prereq.SOP = NIL (Option_t);               /* No sop involved       */
  check_prereq.consider_ifreqs = TRUE;             /* always look at ifreqs */
  check_prereq.keep_apar_info = FALSE;             /* not needed            */
  check_prereq.keep_description_info = FALSE;      /* not needed            */
  check_prereq.called_from_ls_programs = TRUE;     /* forces certain paths  */
  check_prereq.ignore_AVAILABLE_entries = TRUE;    /* not needed            */
  ckp_errs = SCREEN_LOG;                           /* errs to scrn and log  */
  ckp_warn = LOG_MSG;                              /* warnings go to log    */
  load_fix_info_table (TRUE, &error);              /* TRUE says no SOP      */
  if (error) return(CHK_BAD_RET);
  
  /*--------------------------------------------------------------------*
   *  Build the requisite graph.
   *--------------------------------------------------------------------*/
  build_graph (&error);
  if (error) return (CHK_BAD_RET);

  /*--------------------------------------------------------------------*
   *  Call a ckprereq routine which marks nodes that can satisfy 
   *  requisites (ie. CANDIDATEs to be in the graph).  This is useful
   *  for subsequent calls to other ckprereq routines.
   *--------------------------------------------------------------------*/
  mark_initial_CANDIDATE_NODEs ();

  /*--------------------------------------------------------------------*
   *  If lppchk was called to process a particular option name,
   *  look up that name in the fix info table.  Return if not found
   *  OR if only a "dummy entry" was found.  The caller provides a
   *  "*" for lpp_name when no particulare name is requested.
   *--------------------------------------------------------------------*/
  do_all = FALSE;
  if (strcmp (lppname, "*") != 0)
  {
     found = FALSE;
     if ((hash_entry = locate_hash_entry (lppname, "")) != NIL (ENTRY))
     {
        for (same_name_fix = (fix_info_type *) (hash_entry->data);
             same_name_fix != NIL (fix_info_type);
             same_name_fix = same_name_fix->collision_list)
        {
           if (same_name_fix->origin != DUMMY_FIX &&
               same_name_fix->apply_state != AVAILABLE)
           {
              found = TRUE;
              break; 
           }
        } /* for */
     }
 
     if (!found)
     {         
        /*
         * Get the lppname substring if there's a wild-card at the end of 
         * the name specified. (don't handle wild card at beginning)
         */
        if ((ptr = strstr (lppname, "*")) != NIL (char) &&
            ((&ptr[0]) != (&lppname[0])))
        {
           strcpy (lppname_substr, lppname); 
           *(lppname_substr + (ptr-lppname)) = '\0';
           /*
            * Do a scan of fix_info table to see if the sub_string is 
            * at the beginning of the fix name.
            */
           for (fix = check_prereq.fix_info_anchor->next_fix;
                fix != NIL (fix_info_type);
                fix = fix->next_fix)
           {
              if (((ptr = strstr (fix->name, lppname_substr)) != NIL (char)) &&
                  ((&ptr[0]) == (&fix->name[0])))
              {
                 found = TRUE;
                 break;

              }
           } /* for */
        } /* if ((ptr =... */
      
        if (!found)
        {
           MSG_S(MSG_ERROR, MSG_CHK_NOLPP, DEF_CHK_NOLPP, lppname, 0, 0) ;
           return (CHK_BAD_RET);
        }
     }
  } 
  else 
  {
     do_all = TRUE;
  }

   vpd_catclose ();  /* close the lppchk msg catalogue */
   CATOPEN ();       /* open cmdinstl msg catalogue    */

  /*--------------------------------------------------------------------*
   *  SCAN #1:
   *  look for "non-dummy", "non-group", "non-available" nodes.  Take the 
   *  name into consideration if one was specified.  Perform a 
   *  part-consistency/state check, followed by a requisite check IF there 
   *  was no part/state problem. 
   *--------------------------------------------------------------------*/
  err_cnt = 0;
  req_failures_seen = FALSE;
  for (fix = check_prereq.fix_info_anchor->next_fix;
       fix != NIL (fix_info_type);
       fix = fix->next_fix)
  {
     if ((fix->origin != DUMMY_GROUP)    
                    &&                               
         (fix->origin != DUMMY_FIX)      
                    && 
         ((fix->apply_state != AVAILABLE)   ||   /* ignore avlbls unless */
          (fix->flags & CKP_ST_APPLYING)    ||   /*  "ING" flag is set */
          (fix->flags & CKP_ST_COMMITTING)  || 
          (fix->flags & CKP_ST_REJECTING)   ||
          (fix->flags & CKP_ST_DEINSTALLING))
                    &&
         ((do_all) 
             || 
          ((lppname_substr [0] == '\0') && (strcmp (fix->name, lppname) == 0))
             ||
          ((lppname_substr [0] != '\0')                       && 
           ((ptr = strstr (fix->name, lppname_substr)) != NIL (char)) &&
           ((&ptr[0]) == (&fix->name[0])))))
     {
        if ((fix->apply_state != ALL_PARTS_APPLIED) 
                             ||
            ((fix->parts_committed != 0) && 
             (fix->commit_state != ALL_PARTS_COMMITTED)))
        {
           /*
            * Flag this as an inconsistent/bad state node.
            */
           err_cnt++;
           fix->flags |= REPT_CMD_LINE_FAILURE;   
        }
        else
        {
           /*
            * This node is installed.  Do requisite check.
            */
           passed = evaluate_subgraph (NIL (requisite_list_type), fix, &error);
           if (error) return (CHK_BAD_RET);

           if (! passed) {
              req_failures_seen = TRUE;
              fix->flags |= REPT_REQUISITE_FAILURE;  /* Tag for later use. */
              err_cnt++;
           } else { 
              fix->flags |= SUCCESSFUL_NODE;         /* Tag for later use. */
           }
           unmark_SUBGRAPH (fix, VISITING_THIS_NODE); /* clear bit set in
                                                         evaluation call */
        } /* if (fix->apply... */
     } /* if ((fix->origin.... */
  }  /* for */

  if (err_cnt == 0)   
     return (0);        /* no inconsistencies */

  if (updt)  /* lppchk -uv ? */
  {
     /*--------------------------------------------------------------------*
      *  SCAN #2:
      *  look for usr-root pkgs marked above with the REPT_CMD_LINE_FAILURE 
      *  flag, which have at least one part applied.  Print the 
      *  operation required to sync-up the usr part with the root part, 
      *  followed by the name of the option.
      *--------------------------------------------------------------------*/
     for (fix = check_prereq.fix_info_anchor->next_fix;
          fix != NIL (fix_info_type);
          fix = fix->next_fix)
     {
        if (fix->flags & REPT_CMD_LINE_FAILURE         &&
            fix->parts_in_fix == (LPP_USER | LPP_ROOT) && 
            fix->parts_applied != 0)
        {
           print_sync_action (fix);
        }
     } /* end for */  
     return (err_cnt);  /* No further action required for lppchk -uv */

  } /* if (updt... */

  /*--------------------------------------------------------------------*
   *  SCAN #3:
   *  Invoke another ckprereq routine to mark successful subgraphs, detected 
   *  in SCAN #1.  (This permits use of other ckprereq code to determine
   *  requisite deficiencies on the system).
   *--------------------------------------------------------------------*/
  for (fix = check_prereq.fix_info_anchor->next_fix;
       fix != NIL (fix_info_type);
       fix = fix->next_fix)
  {
     if (fix->flags & SUCCESSFUL_NODE)
     {
        tag_subgraph_SUCCESSFUL_NODEs (NIL (requisite_list_type),
                                       fix, &error);
        if (error) return (CHK_BAD_RET);
     }
  } /* for */
  unmark_graph (VISITING_THIS_NODE);  /* clear bit set in calls to tag... */

  /*--------------------------------------------------------------------*
   *  SCAN #4:
   *  Determine which requisites in the subgraph of a node marked in 
   *  SCAN #2 are causing the requisite failure.
   *--------------------------------------------------------------------*/
  for (fix = check_prereq.fix_info_anchor->next_fix;
       fix != NIL (fix_info_type);
       fix = fix->next_fix)
  {
     if (fix->flags & REPT_REQUISITE_FAILURE)
     {
         for (req = fix->requisites;
              req != NIL (requisite_list_type);
              req = req->next_requisite)
         {
            evaluate_requisite_failure_subgraph (fix, req, &error);
            if (error) return (CHK_BAD_RET);
         } /* end for */
 
         /* Reset bits set in evaluation process */
         unmark_graph (VISITING_THIS_SUBGRAPH | SUBGRAPH_VISITED);
         unmark_requisite_visited ();
     } /* end if */
  }  /* for */
  
  /*--------------------------------------------------------------------*
   *  SCAN #5:
   *  Report all inconsistencies detected above.
   *--------------------------------------------------------------------*/
  inu_init_strings ();

  printed_hdr_msg = FALSE;
  for (fix = check_prereq.fix_info_anchor->next_fix;
       fix != NIL (fix_info_type);
       fix = fix->next_fix)
  {
     if ((fix->flags & REPT_FAILED_REQUISITE) 
                        ||
         (fix->flags & REPT_CMD_LINE_FAILURE))
     {
        if (! printed_hdr_msg)
        {
           inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, 
                              CKP_LPPCHK_INCONSIST_FAILURE_HDR_I,
                              CKP_LPPCHK_INCONSIST_FAILURE_HDR_D),
                              &inu_cmdname);
          
           printed_hdr_msg = TRUE;
        }
        if (fix->origin == DUMMY_GROUP)
        {
           /*
            * Dummy group nodes are treated differently.  We'll print out
            * any unmet members of a group requisite expression until enough 
            * are printed to satisfy the "deficient" passes_required count of
            * the group node.
            */
           print_group_members (fix);
           continue; 
        }
        if (fix->origin != DUMMY_FIX)   /* Place holder for unmet requisite? */
        {
           /*
            * Print name and level information followed by a string  
            * representing the state of the fileset.
            */
           inu_msg (ckp_errs,"  %-39s (%s)\n", 
                    get_fix_name_from_fix_info (fix, fix_name), 
                    get_state_description (fix, state_str));
        }
        else
        {
           /*
            * Print name and requisite information (which may be of the
            * form v=, r=..., p= OR v.r.m.f) followed by a string  
            * representing the state of the fileset.  Use a ckprereq
            * report routine to do so.
            */
           print_requisite_and_dependent_list (fix, 
                                       FALSE, 
                                       get_state_description (fix, state_str),
                                       &error);
           if (error) return (CHK_BAD_RET);
        }   
     } 
  } /* for */

  if (printed_hdr_msg)
     inu_msg (ckp_errs, "\n");

  /*--------------------------------------------------------------------*
   *  SCAN #6:
   *  If more detail was requested, print relationships between failures 
   *  listed in SCAN #5 and their dependent filesets.
   *--------------------------------------------------------------------*/
  if ((msg_lev > 1) && req_failures_seen)
  {
     missing_req_printed_list =
           (missing_req_struct *) calloc (missing_req_pl_size,
                                          sizeof (missing_req_struct));

     printed_hdr_msg = FALSE;
     for (fix = check_prereq.fix_info_anchor->next_fix;
          fix != NIL (fix_info_type);
          fix = fix->next_fix)
     {
        if (fix->flags & REPT_REQUISITE_FAILURE)
        {
           if (! printed_hdr_msg)
           {
              inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ,
                                 CKP_LPPCHK_DEP_FAILURE_HDR_I,
                                 CKP_LPPCHK_DEP_FAILURE_HDR_D),
                                 &inu_cmdname);
              printed_hdr_msg = TRUE;
           }
           inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ,
                             CKP_PROD_AT_LEV_REQRS_I,
                             CKP_PROD_AT_LEV_REQRS_D),
                             fix->name, get_level_from_fix_info(fix, fix_name));

           /*
            * Reset the tail index of a global list of structures used to 
            * keep track of missing requisites (to prevent msg duplication).
            */
           init_missing_req_printed_list ();
           print_lppchk_requisite_subgraph (fix, 
                                            NIL (requisite_list_type),
                                            fix,
                                            fix_name, 
                                            &error);
           if (error) return (CHK_BAD_RET);
           unmark_SUBGRAPH (fix, VISITING_THIS_NODE);
           unmark_requisite_visited ();
        } /* end if (fix... */
     } /* end for */
     inu_msg (ckp_errs, "\n");
  } /* end if (verbose... */

  CATCLOSE ();
         
  return (err_cnt);

} /* end lppchkv */

/*----------------------------------------------------------------------------
** NAME: print_lppchk_requisite_subgraph
**
** FUNCTION:  Prints the members of a requisite subgraph which caused
**            the installed node to have requisite problems.  Recurses
**            on the requisite chains to list all problematic requisites
**            within a subgraph.
**
** RETURNS:  void function 
**
**---------------------------------------------------------------------------*/

static void
print_lppchk_requisite_subgraph 
(
fix_info_type       * parent,
requisite_list_type * link_from_parent,
fix_info_type       * fix,
char                * print_buf,
boolean             * error
) 
{
   requisite_list_type * req;

   /*
    * Check for recursion stopping condition.
    * (Dummy fixes may be referenced more than once, representing a 
    * different requisite, therefore recursion flag is set in the 
    * requisite node (ie. link_from_parent).
    */
   if ((fix->flags & VISITING_THIS_NODE)  
                     &&
       (fix->origin != DUMMY_FIX || link_from_parent->flags.requisite_visited))
      return;

   /*
    * Set recursion stopping bits.
    */
   fix->flags |= VISITING_THIS_NODE;
   if (link_from_parent)
      link_from_parent->flags.requisite_visited = TRUE;

   if (parent != fix && (fix->flags & REPT_FAILED_REQUISITE))
   {
      /*
       * This node was flagged as being a failed requisite of another.
       * Print the node.  If it's a group, print the members of the group.  
       */
      if (fix->origin == DUMMY_GROUP)
      {
         print_group_requisites (fix, (short) 0, 
                                 NIL (requisite_list_type), 
                                 error);
         RETURN_ON_ERROR;
         unmark_SUBGRAPH (fix, VISITING_THIS_NODE);
         return;
      }

      if (fix->origin == DUMMY_FIX)
      {  
         /*
          * Special processing for "dummy fixes":  A single dummy fix may be
          * referenced multiple times in the current subgraph.  In such
          * instances, the dummy node is pointed to by requisite nodes, which
          * may represent the same or different requisite expressions.
          * We need to prevent requisite expressions from being printed multiple
          * times in the current subgraph.
          *
          * Pictorially:
          *
          *      [A] ----(v=1 r=2 m=0 f=0)--->[B]---(v=4 r=1 m=0 f=0)-->[C]
          *       +                                                      ^
          *       |                                                      |
          *       +----------------(v=4 r=1 m=0 f=0)---------------------+
          *
          * [] = fix_info structure
          * () = requisite structure
          * A, B installed.  C missing
          *
          * When printing the missing requisites in the subgraph of A,
          * need to avoid listing C 4.1.0.0 twice.
          *
          */ 
         if (! is_missing_req_printed (fix->name, 
                                         &(link_from_parent->criteria)))
         {
            get_criteria_string (&(link_from_parent->criteria), print_buf);
            inu_msg (ckp_errs, "  %s %s\n", fix->name, print_buf);
            add_missing_req_to_printed_list (
                                         fix->name, 
                                         &(link_from_parent->criteria),
                                         error);
         }
      }
      else
         inu_msg (ckp_errs, "  %s\n", 
                  get_fix_name_from_fix_info (fix, print_buf));
   } 

   /* 
    * Recurse on the requisite list of the current node.
    * Ignore ifreq branches where the base level is not installed.
    */
   for (req = fix->requisites;
        req != NIL (requisite_list_type);
        req = req->next_requisite)
   {
      if ((req->type != AN_IFREQ || req->type != AN_IFFREQ) 
                            ||
	  ck_base_lev_list (req, CBLL_SUCC_CAND))
      {
         print_lppchk_requisite_subgraph (fix, req, req->fix_ptr, print_buf,
                                          error);
         RETURN_ON_ERROR;
      }

   } /* end for */

} /* print_lppchk_requisite_subgraph */

/*----------------------------------------------------------------------------
** NAME: get_state_description
**
** FUNCTION: Translates the state of the fix structure passed as an
**           argument, to a VPD state, then further converts that state
**           to a descriptive state of the pkg.
**
** RETURNS:  A character pointer to the out_buf string provided as an argument.
**
**---------------------------------------------------------------------------*/

static char *
get_state_description 
(
fix_info_type * fix,
char          * out_buf
) 
{
   int  fix_state;
   int  usr_state;
   int  root_state;
   char usr_str  [MAX_LINE_WIDTH] = "";
   char root_str [MAX_LINE_WIDTH] = "";
   char state_str [MAX_LINE_WIDTH] = "";

   if ((fix->origin == DUMMY_FIX)
                     ||
       ((fix->apply_state == AVAILABLE) &&
        ! ((fix->flags & CKP_ST_APPLYING) ||
           (fix->flags & CKP_ST_COMMITTING) ||
           (fix->flags & CKP_ST_REJECTING)  ||
           (fix->flags & CKP_ST_DEINSTALLING))))
       
   {
      /*
       * Not installed.  Let's return a string indicating this.
       */
      strcpy (out_buf, MSGSTR (MS_CKPREREQ,
                               CKP_LPPCHK_NOT_INSTALLED_REQ_FILESET_I,
                               CKP_LPPCHK_NOT_INSTALLED_REQ_FILESET_D));

      return (out_buf);
   }

   convert_ckp_state_to_vpd_state (fix, &fix_state, &usr_state, &root_state);

   if (fix_state == 0)
      fix_state = ST_UNKNOWN;   /* "UNKNOWN" state */

   if (fix->parts_in_fix == (LPP_USER | LPP_ROOT))
   {
      /*
       * See if we need to talk specifically about usr and root parts.
       * ie. if we're in some part-wise inconsistent state (as opposed
       * to usr part BROKEN, root part not even installed.)
       */
      if (usr_state!=0)
      {
         /*
          * usr part has state.  we'll use it.
          */
         strcpy (usr_str, inu_unpad (string [get_state_str_index (usr_state)]));

         if (root_state != 0)
            /*
             * root part has state.  Let's use it.
             */
            strcpy (root_str, inu_unpad 
                                  (string [get_state_str_index (root_state)])); 
         else
            /*
             * Root part is either not installed or is in the state initially
             * indicated in the ckprereq, fix_info structure.  If that
             * initial state (after "translation" to vpd state) was
             * applied or committed, we know that the root part is not
             * installed.  Otherwise, the root part is in that state.
             */  
            if (fix_state == ST_APPLIED   || 
                fix_state == ST_COMMITTED)
            strcpy (root_str, MSGSTR (MS_CKPREREQ, CKP_LPPCHK_NOT_INSTALLED_I,
                               CKP_LPPCHK_NOT_INSTALLED_D));
            else
               strcpy (root_str, inu_unpad (
                                     string [get_state_str_index (fix_state)]));
      } 
      else if (root_state != 0)
      {
         /*
          * usr part has no state.  root part does.  Let's use the root part.
          */
         strcpy (root_str, inu_unpad (
                                   string [get_state_str_index (root_state)]));
         /*
          * usr part is either not installed or is in the state initially
          * indicated in the ckprereq, fix_info structure.  If that
          * initial state (after "translation" to vpd state) was
          * applied or committed, we know that the usr part is not
          * installed.  Otherwise, the usr part is in that state.
          */  
         if (fix_state == ST_APPLIED   || 
             fix_state == ST_COMMITTED || 
             fix_state == ST_AVAILABLE)
            strcpy (usr_str, MSGSTR (MS_CKPREREQ, CKP_LPPCHK_NOT_INSTALLED_I,
                               CKP_LPPCHK_NOT_INSTALLED_D));
         else
            strcpy (usr_str, inu_unpad (
                                    string [get_state_str_index (fix_state)]));
      }
      else
      {
         /*
          * No specific states for usr or root part.  Let's use
          * the state of the "whole pkg" as determined by the
          * "convert" routine above.
          */
         strcpy (state_str, inu_unpad (
                                  string [get_state_str_index (fix_state)]));
      }
   }
   else
   {
      /*
       * Not a usr-root pkg.  Use the state returned by the
       * convert routine called above.
       */
      strcpy (state_str, inu_unpad (string [get_state_str_index (fix_state)]));
   }

   /* 
    * If we stored anything in the state_str, return it.
    * Otherwise return information about the usr-root states which we
    * must have deduced instead.
    */
   if (state_str[0] != '\0')
      strcpy (out_buf, state_str);
   else
      sprintf (out_buf, "%s: %s, %s: %s", 
                        inu_unpad (string [USR2_STR]),
                        usr_str, 
                        inu_unpad (string [ROOT2_STR]),
                        root_str);

   return (out_buf);

} /* get_state_description */

/*----------------------------------------------------------------------------
** NAME:    print_group_members 
**
** FUNCTION: Prints the members of a group which are causing that group
**           requisite not to be satisfied.
**
** RETURNS:  void function
**
**---------------------------------------------------------------------------*/

static void
print_group_members 
(
fix_info_type * group_node
) 
{
   char                  fix_name  [MAX_LPP_FULLNAME_LEN];
   char                  state_str [MAX_LPP_FULLNAME_LEN];
   fix_info_type       * fix;
   int                   satisfied_count = 0;
   int                   printed_count = 0;
   requisite_list_type * req;

   /*
    * Scan the requisite chain of the group to count the number
    * of unsatisfied group members.
    */
   for (req = group_node->requisites;
        req != NIL (requisite_list_type);
        req = req->next_requisite)
   {
      if (req->fix_ptr->flags & (SUCCESSFUL_NODE | CANDIDATE_NODE))
         satisfied_count++;

   } /* end for */  

   /*
    * Re-scan the requisite chain printing group members which failed,
    * until the number of unsatisfied + number of satisfied == passes required.
    */
   for (req = group_node->requisites;
        req != NIL (requisite_list_type);
        req = req->next_requisite)
   {
      if (! (req->fix_ptr->flags & (SUCCESSFUL_NODE | CANDIDATE_NODE)))
      {
         /*
          * Recurse on nested groups.
          */
         if (req->fix_ptr->origin == DUMMY_GROUP)
         {
            print_group_members (req->fix_ptr);
         }
         else
         {
            /*
             * Print the name and level information for a non-dummy fix.
             * Print the requisite information for a dummy fix.
             */
            if (req->fix_ptr->origin != DUMMY_FIX)
            {
              inu_msg (ckp_errs,"  %-39s(%s)\n", 
                       get_fix_name_from_fix_info (req->fix_ptr, fix_name), 
                       get_state_description (req->fix_ptr, state_str));
            }
            else
            {
               strcpy (fix_name, req->fix_ptr->name);
               strcat (fix_name, " ");
               get_criteria_string (&(req->criteria), state_str);
               strcat (fix_name, state_str);
               inu_msg (ckp_errs,"  %-39s(%s)\n", 
                        fix_name, 
                        get_state_description (req->fix_ptr, state_str));
            }   
         } 
         printed_count++;
      }
      if ((printed_count + satisfied_count) >= group_node->passes_required)
         break;

   } /* end for */  
   
} /* print_group_members */

/*----------------------------------------------------------------------------
** NAME:     print_sync_action
**
** FUNCTION: Prints the <installp_flags> <option_name> <level> string
**           required for the lppchk -uv function. 
**
** RETURNS:  Void function 
**
**---------------------------------------------------------------------------*/

static void 
print_sync_action
(
fix_info_type * fix
)
{
   char * opt="";
   char   p;
   int    fix_state;
   int    usr_state;
   int    root_state;  
 
  /* on entry we know that one part is either APPLIED or COMMITED       */
  /* and that rpr->state does not equal lpr->state                      */
  /* output will be the following                                       */
  /*                                                                    */
  /*     usr \ root                                                     */
  /*                 |  applied | committed| other|                     */
  /*       |---------|----------|----------|------|                     */
  /*       | applied |   none   |   ERR    |  -a  |                     */
  /*       |---------|----------|----------|------|                     */
  /*       |committed|   -c     |   none   |  -ac |                     */
  /*       |---------|----------|----------|------|                     */
  /*       | other   |   -r     |   ERR    |  none|                     */
  /*       |---------|----------|----------|------|                     */
  /*                                                                    */
 
   /* 
    * determine if dealing with an update with a ptf id.  Save print info.
    */
   if (fix->level.ptf[0] != '\0')
      p = '.';
   else
      p = ' ';

   /*
    * Translate the fix_info state to states concerning parts.
    */
   convert_ckp_state_to_vpd_state (fix, &fix_state, &usr_state, &root_state);

   if (usr_state != 0)    /* usr applied or committed? */ 
   {
      /*
       * Generate a cleanup flag if the state of the fileset was determined
       * to be in one of the ING states.
       */
      if (fix_state == ST_APPLYING   ||
          fix_state == ST_COMMITTING ||
          fix_state == ST_REJECTING  ||
          fix_state == ST_DEINSTALLING)
      {
         printf ("-C %s %02d.%02d.%04d.%04d%c%s\n", fix->name,
             fix->level.ver, fix->level.rel, fix->level.mod, fix->level.fix, 
             p, fix->level.ptf) ;
      }
      if (root_state == ST_COMMITTED)
         opt = "ERR";                         /* error state -- tells caller
                                                 nothing should be done to
                                                 sync up. */
      else if (usr_state == ST_APPLIED)
         opt = "-a";                          /* We know the root's not 
                                                 committed therfore root
                                                 must need apply. */
      else if (root_state == ST_APPLIED)
         opt = "-c";                          /* Root state's applied,
                                                 therefore commit must have
                                                 been the descrepancy. */
      else
         opt = "-ac";                         /* Root must need apply-commit */
   } 
   else
   {
      if (root_state == ST_APPLIED)          
         opt = "-r";                          /* usr not installed, root is
                                                 applied - let's reject. */
      else
         opt = IF_INSTALL (fix->op_type) ? "-u" : "ERR";

                                 	      /* usr not installed, root in 
                                                 some other state.  deinstall
                                                 if it's a base level. */
                                  
   }

   if (opt[0] == '\0')
      /*
       * All other states not detected above should be ignored.
       */
      return;
  
   /*
    * Print the sync action.
    */ 
   printf("%s %s %02d.%02d.%04d.%04d%c%s\n", opt, fix->name,
          fix->level.ver, fix->level.rel, fix->level.mod, fix->level.fix, 
          p, fix->level.ptf) ;

} /* print_sync_action */ 

/*----------------------------------------------------------------------------
** NAME:    convert_ckp_state_to_vpd_state
**
** FUNCTION: Interprets the state in the ckprereq fix_info structure as
**           more common vpd states (ST_APPLIED, ST_COMMITTED, etc.)
**           Returns information in usr_state and/or root_state arguments
**           if a usr-root pkg passed and if the parts contain different
**           states.  In this case, fix_state is used to reflect information
**           about the initial state of the fix_info structure, to be used
**           by the caller.  For non usr-root pkgs, the state of the whole
**           pkg is returned in fix_state.
**
** RETURNS:  void function.  (pass-by-name variables contain return values.)
**
**---------------------------------------------------------------------------*/

static void
convert_ckp_state_to_vpd_state
(
fix_info_type * fix,
int           * fix_state,
int           * usr_state,
int           * root_state
)
{
   (*fix_state) = (*usr_state) = (*root_state) = 0;
   if (fix->parts_in_fix == (LPP_USER | LPP_ROOT))
   {
      if (fix->parts_applied & LPP_USER)
      {
         if (fix->parts_committed & LPP_USER) {
            *usr_state = ST_COMMITTED;
         } else {
            *usr_state = ST_APPLIED;
         }
      } 

      if (fix->parts_applied & LPP_ROOT)
      {
         if (fix->parts_committed & LPP_ROOT) {
            *root_state = ST_COMMITTED;
         } else {
            *root_state = ST_APPLIED;
         }
      }

   } /* if (fix->parts_applied... */

   /*
    * ckprereq normally doesn't deal with ING states.  Special tag bits
    * were set for lppchk in copy_vpd_to_fix_info() from ckprereq.
    */
   if (fix->flags & CKP_ST_APPLYING)
   {
      *fix_state = ST_APPLYING;
      return;
   }
   if (fix->flags & CKP_ST_COMMITTING)
   {
      *fix_state = ST_COMMITTING;
      return;
   }
   if (fix->flags & CKP_ST_REJECTING)
   {
      *fix_state = ST_REJECTING;
      return;
   }
   if (fix->flags & CKP_ST_DEINSTALLING)
   {
      *fix_state = ST_DEINSTALLING;
      return;
   }
   /* 
    * See if pkg is applied.
    */
   switch (fix->apply_state)
   {
      case BROKEN :
         *fix_state = ST_BROKEN;
         break;
      case ALL_PARTS_APPLIED :
      case PARTIALLY_APPLIED :
         *fix_state = ST_APPLIED;
         break;
      default:
         break;
   } /* switch */

   /*
    * Override applied state if pkg is committed.
    */
   if (*fix_state != ST_BROKEN)
   {
      if ((fix->commit_state == ALL_PARTS_COMMITTED) ||  /* should never be */
          (fix->commit_state == PARTIALLY_COMMITTED))
         *fix_state = ST_COMMITTED;
   }
  
} /* convert_ckp_state_to_vpd_state */ 

/*----------------------------------------------------------------------------
** NAME:    get_state_str_index
**
** FUNCTION: given vpd state (ST_APPLIED, ST_COMMITTED, etc.) ....
**
** RETURNS:  integer representing index of string in "string" array (for
**           msg conversion).
**
**---------------------------------------------------------------------------*/

static int
get_state_str_index
(
int           state
)
{
   switch (state)
   {
      case ST_APPLYING:     return (APPLYING_STR);     break;
      case ST_APPLIED:      return (APPLIED_STR);      break;
      case ST_COMMITTING:   return (COMMITTING_STR);   break;
      case ST_COMMITTED:    return (COMMITTED_STR);    break;
      case ST_REJECTING:    return (REJECTING_STR);    break;
      case ST_DEINSTALLING: return (DEINSTALLING_STR); break;
      case ST_AVAILABLE:    return (AVAILABLE_STR);    break;
      case ST_BROKEN:       return (BROKEN_STR);       break;
          default:          return (UNKNOWN_STR);      break;

   } /* switch */

} /* get_state_str_index */

/*----------------------------------------------------------------------------
** NAME:    is_missing_req_printed
**
** FUNCTION:  Checks to see if the arguments passed are in the list of 
**            missing requisites for which a requisite expression has
**            been printed. 
**
** RETURNS:   TRUE if the req_expr and req_name arguments already have a
**                 structure in the global missing_req_printed_list
**            FALSE otherwise
**
**---------------------------------------------------------------------------*/

static boolean
is_missing_req_printed
(
char          * req_name,
criteria_type * req_expr
)
{
   int i;
   /*
    * Brute force scan of array.  Performance should not be adversely affected
    * since array only contains entries for current subgraph (ie. a single
    * node with missing requisites).
    */
   for (i = 0; i <= missing_req_pl_tail; i ++)
   {
      if (same_criteria (req_expr, missing_req_printed_list[i].req_expr) &&
          (strcmp (req_name, missing_req_printed_list[i].req_name) == 0))
         return (TRUE);
   }
   return (FALSE);

} /* is_missing_req_printed */

/*----------------------------------------------------------------------------
** NAME:    add_missing_req_to_printed_list
**
** FUNCTION: Sets ptrs in the next available element in the global
**           missing_req_printed_list to the arguments passed to this 
**           routine (a name in a requisite expression and a pointer to 
**           the criteria structure containing requisite information).
**
** RETURNS: void function. 
**
**---------------------------------------------------------------------------*/

static void
add_missing_req_to_printed_list
(
char          * req_name,
criteria_type * req_expr,
boolean       * error
)
{
   /*
    * increment our pointer to tail of list and realloc the global array if 
    * necessary.
    */
   if ((++missing_req_pl_tail) >= missing_req_pl_size)
   {
      missing_req_pl_size += INIT_MISSING_REQ_PL_SIZE;
      if ((missing_req_printed_list = (missing_req_struct *) realloc 
              (missing_req_printed_list, missing_req_pl_size * 
                                          sizeof (missing_req_struct))) == NULL)
      {
         inu_msg (ckp_errs, MSGSTR (MS_INUCOMMON, CMN_REALLOC_FAILED_E,
                  CMN_REALLOC_FAILED_D), inu_cmdname);
         *error = TRUE;
         return;
      }
   }

   /*
    * Make next free structure point to our arguments.
    */
   missing_req_printed_list[missing_req_pl_tail].req_name = req_name;
   missing_req_printed_list[missing_req_pl_tail].req_expr = req_expr;

} /* add_missing_req_to_printed_list */

/*----------------------------------------------------------------------------
** NAME:     init_missing_req_printed_list
**
** FUNCTION: Resets/initializes the global missing_req_pl_tail index.
**
** RETURNS:  void function. 
**
**---------------------------------------------------------------------------*/

static void
init_missing_req_printed_list 
(
)
{
  missing_req_pl_tail = -1;

} /* init_missing_req_printed_list */
