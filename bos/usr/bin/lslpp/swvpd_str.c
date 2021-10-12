static char sccsid[] = "@(#)41  1.10.1.5  src/bos/usr/bin/lslpp/swvpd_str.c, cmdswvpd, bos411, 9428A410j 3/18/94 11:06:52";

/*
 *    COMPONENT_NAME: CMDSWVPD
 *
 *  FUNCTIONS: vpd_perr, index_string_array, init_vpd_strings,
 *             init_vpd_strings2, vpd_catgets
 *
 *  ORIGINS: 27
 *
 *    (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *    All Rights Reserved
 *    Licensed Materials - Property of IBM
 *    US Government Users Restricted Rights - Use, duplication or
 *    disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include        <odmi.h>
#include        <stdio.h>
#include        <swvpd.h>
#include        <nl_types.h>
#define         STRING_DEFNS
#include        <swvpd_str.h>


 /* If these messages are changed, the same changes must be made to */
 /* swvpd.msg */
#define TEXT_SWVPD_OK           "%1$s:  Success.\n"
#define TEXT_SWVPD_SYS          \
"%1$s:  ODM error %2$d occurred trying to read SWVPD data.\n"
#define TEXT_SWVPD_BADCOL       \
"%1$s:  An unsearchable input was specified for SWVPD data search.\n"
#define TEXT_SWVPD_SQLMAX       \
"%1$s:  The search string limit of %2$d has been exceeded.\n"
#define TEXT_SWVPD_NOMATCH      \
"%1$s:  No match was found for the SWVPD data search.\n"
#define TEXT_SWVPD_NOID         \
"%1$s:  Cannot match fileset name with fileset ID.\n"
#define TEXT_SWVPD_UNKNOWN      "%1$s:  SWVPD returned unknown error: %2$d.\n"

#define NL_ERR  (nl_catd)-1

static nl_catd  vpd_catd = NL_ERR;   /* Message catalog */
                                     /* descriptor for vpd */
                                     /* routines */
static void init_vpd_strings2 (string_info_t *string_infop);


/*
 * NAME:     vpd_catclose
 *
 * FUNCTION: this routine is a front-end to catclose.
 *
 * EXECUTION ENVIRONMENT:
 *
 *           CALLED BY: lslpp
 *
 * RETURNS:  a pointer to a string.
 */
void vpd_catclose ()
{

  catclose (vpd_catd);

} /* vpd_catclose() */



/*
 * NAME:     vpd_catgets
 *
 * FUNCTION: this routine is a front-end to catgets.  It
 *           returns a pointer to a string that will be printed out to the
 *           user.  It endeavors to use the appropriate string from the swvpd
 *           message catalog, but if that isn't available, it uses the
 *           compiled-in string that was passed in as parameter "str".
 *           This routine always uses a single vpd message catalog.
 *
 * EXECUTION ENVIRONMENT:
 *
 *           CALLED BY: lslpp
 *
 * RETURNS:  a pointer to a string.
 */
char *vpd_catgets (int section, int num, char *str)
{

  if (vpd_catd == NL_ERR) {
    vpd_catd = catopen(MF_SWVPD,NL_CAT_LOCALE);
  }

  if (vpd_catd != NL_ERR) {
    return (catgets (vpd_catd, section, num, str));
  }

  return (str);
}



/*
 * NAME:     vpd_perror
 *
 * FUNCTION: Print out an error message describing a specified VPD
 *           error value.
 *
 *           This routine endeavors to use the message catalog.  If
 *           that fails, it drops back to using a set of messages
 *           that are compiled into this file.
 *           Also, this routine can be compiled without MSG
 *           defined, in which case it won't try to use, or link
 *           in, the message routines.
 *
 * EXECUTION ENVIRONMENT:
 *
 *           CALLED BY: lslpp
 *
 * RETURNS:  none.
 */
void vpd_perror (char *str, int errnum)
{

   switch (errnum) {

   case (VPD_OK):

      fprintf (stderr,
               vpd_catgets (MSGS_SWVPD,
                            MSG_SWVPD_OK,
                            TEXT_SWVPD_OK),
               str);
      break;

   case (VPD_SYS):

      fprintf (stderr,
               vpd_catgets (MSGS_SWVPD,
                            MSG_SWVPD_SYS,
                            TEXT_SWVPD_SYS),
               str, odmerrno);
      break;

   case (VPD_BADCOL):

      fprintf (stderr,
               vpd_catgets (MSGS_SWVPD,
                            MSG_SWVPD_BADCOL,
                            TEXT_SWVPD_BADCOL),
               str);
      break;

   case (VPD_SQLMAX):

      fprintf (stderr,
               vpd_catgets (MSGS_SWVPD,
                            MSG_SWVPD_SQLMAX,
                            TEXT_SWVPD_SQLMAX),
               str);
      break;

   case (VPD_NOMATCH):

      fprintf (stderr,
               vpd_catgets (MSGS_SWVPD,
                            MSG_SWVPD_NOMATCH,
                            TEXT_SWVPD_NOMATCH),
               str);
      break;

   case (VPD_NOID):

      fprintf (stderr,
               vpd_catgets (MSGS_SWVPD,
                            MSG_SWVPD_NOID,
                            TEXT_SWVPD_NOID),
               str);
      break;

   default:

      fprintf (stderr,
               vpd_catgets (MSGS_SWVPD,
                            MSG_SWVPD_UNKNOWN,
                            TEXT_SWVPD_UNKNOWN),
               str, errnum);
      break;
   }
}



/*
 * NAME:     index_string_array
 *
 * FUNCTION: This routine provides the functionality of indexing into an array
 *           of strings, with the following twists:
 *            - If the array index is < 0 or >= array_size, the value
 *              array_size -1 is used instead (since the entries are numbered
 *              0 through array_size-1, this is the last entry in the array).
 *              The presumption is that the last string in the array is an
 *              "UNKNOWN XYZ VALUE" string.
 *            - This routine works both with and without message catalogs.
 *            - When message catalogs are being used, this routine protects
 *              against the fact that the low-level message catalog routines
 *              re-use a single buffer.  This is done by copying the string
 *              into the msgcat_string in the string array element.
 *
 *           NOTE:  This routine will use the message catalog versions of the
 *                  strings only if the string array being used has been
 *                  initialized in init_vpd_strings()
 *
 * EXECUTION ENVIRONMENT:
 *
 *           CALLED BY: lslpp
 *
 * RETURNS:  a pointer to a string.
 */
char *index_string_array (string_infop, index)
     string_info_t      *string_infop;
     int                index;          /* index into string_array; */
                                        /* also message number in */
                                        /* msg_cat_sec in message */
                                        /* catalog. */

{
  char                  *cp;


  /* If the index points off the beginning or end of the array, make */
  /* it point at the last entry in the array */
  if ((index < 0) || (index >= (string_infop->nstrings)))
      index = string_infop->nstrings - 1;


  /* Get a pointer to the message catalog version of the string. */
  cp = string_infop->strings[index].msgcat_string;

  /* If it isn't null, then return it. */
  if (cp[0] != '\0')
      return (cp);


  /* Otherwise, return a pointer to the compiled-in version of the */
  /* string. */
  return (string_infop->strings[index].compiled_string);

}  /* index_string_array() */



/*
 * NAME:     init_vpd_strings
 *
 * FUNCTION: This routine attempts to read the message
 *           catalog version of each string in each string set (array) known to
 *           swvpd, so that the message catalog version will be safely
 *           available in memory for later use.
 *
 * EXECUTION ENVIRONMENT:
 *
 *           CALLED BY: lslpp
 *
 * RETURNS:  none.
 */
void init_vpd_strings()
{
  init_vpd_strings2 (&lstate_string_info);
  init_vpd_strings2 (&hstate_string_info);
  init_vpd_strings2 (&event_string_info);
  init_vpd_strings2 (&inv_ftype_string_info);
  init_vpd_strings2 (&inv_format_string_info);
  init_vpd_strings2 (&header_string_info);
} /* init_vpd_strings(); */



/*
 * NAME:     init_vpd_strings2
 *
 * FUNCTION: initialize one msg_string_t structure array,
 *           by getting the message catalog version of each string in the
 *           array, and copying it into the msgcat_string field in that array
 *           entry.
 *
 * EXECUTION ENVIRONMENT:
 *
 *           CALLED BY: init_vpd_strings
 *
 * RETURNS:  none.
 */
static void init_vpd_strings2 (string_info_t *string_infop)
{
  int                   i;

  for (i = 0; i < string_infop->nstrings; i++) {
    strcpy (string_infop->strings[i].msgcat_string,
            vpd_catgets (string_infop->msg_set, i,
                         string_infop->strings[i].compiled_string));
  }
}
