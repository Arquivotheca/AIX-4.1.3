/* @(#)42  1.14.1.16  src/bos/usr/bin/lslpp/swvpd_str.h, cmdswvpd, bos411, 9428A410j 6/6/94 18:21:08 */

/*
 *   COMPONENT_NAME: CMDSWVPD
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _SWVPD_STR_H
#define _SWVPD_STR_H

extern  char    *index_string_array();
void vpd_catclose ();
char *vpd_catgets (int section, int num, char *str);
void vpd_perror (char *str, int errnum);
void init_vpd_strings();

#define N_ENTRIES(xx)   ((sizeof (xx)) / (sizeof (xx[0])))

#include "swvpd_msg.h"

 /* These values are used to index into the array of header strings. */
#define H_LPP_STATE     1
#define H_COMMENT       2
#define H_STATUS        3
#define H_ACTION        4
#define H_USER_NAME     5
#define H_RELEASE       6
#define H_PREREQ        7
#define H_DEPENDENTS    8
#define H_FILE          9
#define H_DATE          10
#define H_COMP_ID       11
#define H_FESN          12
#define H_PROD_NAME     13
#define H_SOURCE        14
#define H_FIX_ID        15
#define H_FIX_INFO      16
#define H_DEPEND_FIX    17
#define H_DEPEND_STATE  18
#define H_TIME          19
#define H_VENDOR        20
#define H_VENDR         21
#define H_CODE          22
#define H_PRODUCT       23
#define H_FEATURE       24
#define H_ID            25
#define H_FIX_STATE     26
#define H_STATE         27
#define H_NAME          28
#define H_PATH          29
#define H_TYPE          30

 /* the following are defines which reflect the general messages */
#define TEXT_NOMEM         "%s:  Out of memory.\n"

 /* the following are defines which reflect the lpp messages */
#define TEXT_LSLPP_USAGE \
"Usage: lslpp {-d|-f|-h|-i|-L|-l|-p} [-B | -I] [-acJq] [-O{[r][s][u]}]\n \
             [fileset ... | ptf_id ... | all]\n \
\t-a Displays additional (\"all\") information when combined with\n \
\t   other flags.  (Not valid with -f, only valid with -B when\n \
\t   combined with -h)\n \
\t-B Permits PTF ID input.  (Not valid with -L)\n \
\t-c Colon-separated output.  (Not valid with -La)\n \
\t-d Dependents (filesets for which this is a requisite).\n \
\t-f Files that belong to this fileset.\n \
\t-h History information.\n \
\t-I Limits listings to base level filesets (no updates displayed).\n \
\t-i Product Identification information (requested per fileset).\n \
\t-J Use list as the output format.  (Valid with -l and -L)\n \
\t-L Lists fileset names, latest level, states, and descriptions.\n \
\t   (Consolidates usr, root and share part information.)\n \
\t-l Lists fileset names, latest level, states, and descriptions.\n \
\t   (Separates usr, root and share part information.)\n \
\t-O Data comes from [r] root and/or [s] share and/or [u] usr.\n \
\t   (Not valid with -L)\n \
\t-p Requisites of installed filesets.\n \
\t-q Quiet (no column headers).\n"

#define TEXT_LSLPP_SINGLE_ENTRY       "%1$s:  -%2$s may only be entered once.\n"
#define TEXT_LSLPP_MISSING_ARGUMENTS  "%1$s:  \
missing option-arguments with -O.\n"
#define TEXT_LSLPP_INVALID_ARGUMENTS  "%1$s:  \
invalid option-arguments with -O.\n"
#define TEXT_LSLPP_INVALID_OPTION     "%1$s:  -%2$s is an invalid option.\n"
#define TEXT_LSLPP_EXCLUSIVE_FLAGS \
"%1$s:  The following flags are mutually exclusive: d,f,h,i,L,l,p.\n"
#define TEXT_LSLPP_SINGLE_CMD         "%1$s:  -%2$s may only be entered once.\n"
#define TEXT_LSLPP_COMBINING_J        "%1$s:  -J may not be used with -%2$s.\n"
#define TEXT_LSLPP_COMBINING_A_AND_B \
"%1$s:  -a may not be used with -%2$s combined with -B.\n"
#define TEXT_LSLPP_COMBINING_A_AND_F  "%1$s:  -a may not be used with -f.\n"
#define TEXT_LSLPP_COMBINING_M        "%1$s:  -m may not be used with -%2$s.\n"
#define TEXT_LSLPP_COMBINING_I_AND_B  "%1$s:  -I may not be used with -B.\n"
#define TEXT_LSLPP_NO_FIXES_M \
"%1$s:  There is no PTF ID with microcode in %2$s\n \
\tthat matches \"%3$s\".\n"
#define TEXT_LSLPP_NO_FIXES \
"%1$s:  There is no PTF ID in %2$s that matches \n\t\"%3$s\".\n"
#define TEXT_LSLPP_NO_LPP_M \
"%1$s:  There is no fileset with microcode in %2$s\n\tthat matches \"%3$s\".\n"
#define TEXT_LSLPP_NO_LPP \
"%1$s:  There is no fileset in %2$s that matches \n\t\"%3$s\".\n"
#define TEXT_LSLPP_NO_INPUT_M \
"%1$s:  There is no fileset or PTF ID with microcode in\n \
\t%2$s that matches \"%3$s\".\n"
#define TEXT_LSLPP_NO_INPUT \
"%1$s:  There is no fileset or PTF ID in %2$s\n\tthat matches \"%3$s\".\n"
#define TEXT_LSLPP_ALL_NO_FIXES_M \
"%1$s:  There is no PTF ID with microcode in %2$s,\n \
\t%3$s, or %4$s that matches \"%5$s\".\n"
#define TEXT_LSLPP_ALL_NO_FIXES \
"%1$s:  There is no PTF ID in %2$s, %3$s,\n\tor %4$s that matches \"%5$s\".\n"
#define TEXT_LSLPP_ALL_NO_LPP_M \
"%1$s:  There is no fileset with microcode in %2$s,\n \
\t%3$s, or %4$s that matches \"%5$s\".\n"
#define TEXT_LSLPP_ALL_NO_LPP \
"%1$s:  There is no fileset in %2$s, %3$s,\n\tor %4$s that matches \"%5$s\".\n"
#define TEXT_LSLPP_ALL_NO_INPUT_M \
"%1$s:  There is no fileset or PTF ID with microcode in\n \
\t%2$s, \t%3$s, or %4$s\n\tthat matches \"%5$s\".\n"
#define TEXT_LSLPP_ALL_NO_INPUT \
"%1$s:  There is no fileset or PTF ID in %2$s,\n \
\t%3$s, or %4$s that matches \"%5$s\".\n"
 /* deletei ?
#define TEXT_LSLPP_TOO_MANY_INPUTS \
"%1$s:  This command can only handle a maximum of %2$s inputs\n \
\tyour inputs (see usage) generated too many\n \
\tmatching lppnames and/or fix Ids."
 */
#define TEXT_LSLPP_NO_INSTALL \
"%1$s:  There is no installation record for this fileset \"%2$s\".\n"
#define TEXT_LSLPP_VPD_ERR            "%1$s:  \
Error while processing fileset %2$s.\n"
#define TEXT_LSLPP_PREREQA \
"At least %d of the following\n  %-*s  %-*s  %-*s  %smust be installed {\n"
#define TEXT_LSLPP_PREREQ \
"At least %d of the following must be installed {\n"
#define TEXT_LSLPP_NONE               "NONE"
#define TEXT_LSLPP_NON_IBM            "Vendor Fileset"
#define TEXT_LSLPP_ALL                "all"
#define TEXT_LSLPP_COMBINING_FLAGS    "%1$s:  \
-%2$s may not be used with -%3$s.\n"
#define TEXT_LSLPP_OPTION_ARGS        "%1$s:  \
The option-argument of -O only accepts one occurrence of %2$s.\n"
#define TEXT_LSLPP_PREREQ_COLONS \
"At least %d of the following must be installed {"
#define TEXT_LSLPP_AVAILABLE_ON       "  (Available on %s)"
#define TEXT_LSLPP_EXPLAIN_DEPENDS  "<Fileset> is a requisite of <Dependents>\n"
#define TEXT_LSLPP_NO_PATH            "%1$s:  \
%2$s does not exist as a path on your\n \
\tsystem, this probably means nothing has been installed on that path.\n"
#define TEXT_LSLPP_SUPERSEDED   "%1$s: -%2$s used without -L.\n"
#define TEXT_LSLPP_L_ERROR  "%1$s: -L may not be used with -%2$s. \n"
#define TEXT_LSLPP_PRODUCT_NOT_INSTALLED  "%1$s: Fileset %2$s not installed.\n"
#define TEXT_LSLPP_VPD_CORRUPTED "%1$s: VPD Corrupted : Could not locate %2$s for \
%3$s.\n"
#define TEXT_LSLPP_USAGE_OF_EX_FLAGS \
"\t One of the following mutually exclusive flags: d,f,h,i,L,l,p\n \
\t must be specified.\n"
#define TEXT_LSLPP_LEGEND \
"\n\nState codes: \n\
 A -- Applied. \n\
 B -- Broken. \n\
 C -- Committed. \n\
 O -- Obsolete.  (partially migrated to newer version) \n\
 ? -- Inconsistent State...Run lppchk -v. \n"

#define TEXT_LSLPP_UPDT_PCKG  "Fileset Update"
#define TEXT_LSLPP_SUPERSEDES "Supersedes:"
#define TEXT_LSLPP_NO_MAINTENANCE_LEVEL "No Maintenance Level Applied."
/* D96834 */
#define TEXT_LSLPP_WAIT_PROCESSING "Processing.....Please Wait.\n"
#define TEXT_LSLPP_COMBINING_L_AND_CA  "%1$s: \
-a may not be used with -%2$s combined with -L.\n"
#define TEXT_LSLPP_NO_A_FLAG "\
The -A flag is not valid.  For APAR information, see the \n\
\"instfix\" command.\n"
#define TEXT_LSLPP_COMBINING_L_AND_B  "%1$s: \
-B may not be used with -L.\n"

#define TEXT_LSLPP_B_NEEDS_PTF_ID  "%1$s: \
-B requires one or more PTF IDs (ex. lslpp -B U412345).\n"

#define TEXT_LSLPP_NO_PTF_MATCH "\
%1$s: Fileset with PTF ID %2$s is not installed.\n"

struct msg_string {
  char  *compiled_string;
  char  msgcat_string[100];
};
typedef struct msg_string       msg_string_t;

struct string_info {
  msg_string_t  *strings;
  int           nstrings;       /* number of entries in the array */
  int           msg_set;        /* corresponding message set */
};
typedef struct string_info      string_info_t;


 /* Macros for translating integer values of various types into the */
 /* corresponding strings. */
#define LPP_STATE_STRING(xx) (index_string_array(&lstate_string_info,xx))
#define HIST_STATE_STRING(xx) (index_string_array(&hstate_string_info,xx))
#define EVENT_STRING(xx) (index_string_array(&event_string_info,xx))
#define INV_FTYPE_STRING(xx) (index_string_array(&inv_ftype_string_info,xx))
#define INV_FORMAT_STRING(xx) (index_string_array(&inv_format_string_info,xx))
#define HEADER_STRING(xx) (index_string_array(&header_string_info,xx))
#define MEDIA_STRING(xx) (index_string_array(&media_string_info,xx))

#ifdef  STRING_DEFNS

 /* Values for the state field in the LPP */
msg_string_t lstate_strings[] = {
  { "", "" },
  { "AVAILABLE", "" },
  { "APPLYING", "" },
  { "APPLIED", "" },
  { "COMMITTING", "" },
  { "COMMITTED", "" },
  { "REJECTING", "" },
  { "BROKEN", "" },
  { "DEINSTALLING", "" },
  { "APPLY-HOLD", "" },
  { "COMMIT-HOLD", "" },
  { "OBSOLETE", "" },
  { "UNKNOWN", "" },
};
string_info_t lstate_string_info = {
  lstate_strings,
  N_ENTRIES(lstate_strings),
  LSTATE_MSGS
};

 /* Values for the state field in the history record */
msg_string_t hstate_strings[] = {
  { "", "" },
  { "COMPLETE", "" },
  { "PENDING", "" },
  { "BROKEN", "" },
  { "CANCELLED", "" },
  { "UNKNOWN", "" },
};
string_info_t hstate_string_info = {
  hstate_strings,
  N_ENTRIES(hstate_strings),
  HSTATE_MSGS
};

 /* Values for the event field in the history record */
msg_string_t event_strings[] = {
  { "", "" },
  { "APPLY", "" },
  { "COMMIT", "" },
  { "REJECT", "" },
  { "CLEANUP", "" },
  { "DEINSTALL", "" },
  { "UNKNOWN", "" },
};

string_info_t event_string_info = {
  event_strings,
  N_ENTRIES(event_strings),
  EVENT_MSGS
};

 /* Values for the file type field in the inventory record */
msg_string_t inv_ftype_strings[] = {
  { "", "" },
  { "CONFIGURATION", "" },
  { "SOURCE", "" },
  { "ARCHIVE", "" },
  { "UNKNOWN", "" },
};
string_info_t inv_ftype_string_info = {
  inv_ftype_strings,
  N_ENTRIES(inv_ftype_strings),
  INV_FTYPE_MSGS
};

 /* Values for the format field in the inventory record */
msg_string_t inv_format_strings[] = {
  { "", "" },
  { "file", "" },
  { "archive entry", "" },
  { "Object Data Manager", "" },
  { "Data Management Services", "" },
  { "UNKNOWN", "" },
};
string_info_t inv_format_string_info = {
  inv_format_strings,
  N_ENTRIES(inv_format_strings),
  INV_FORMAT_MSGS
};

 /* If this set of strings is changed, the corresponding strings in */
 /* swvpd.msg must be changed as well. */
 /* Also, the corresponding defined constants, above, must be changed. */
msg_string_t header_strings[] = {
  { "", "" },
  { "State", "" },
  { "Description", "" },
  { "Status", "" },
  { "Action", "" },
  { "User Name", "" },
  { "Level", "" },
  { "Requisites", "" },
  { "Dependents", "" },
  { "File", "" },
  { "Date", "" },
  { "Product Id", "" },
  { "Feature Id", "" },
  { "Package Name", "" },
  { "Source", "" },
  { "PTF Id", "" },
  { "Fix Information", "" },
  { "Dependents Fix", "" },
  { "Dependents State", "" },
  { "Time", "" },
  { "Vendor", "" },
  { "Vendr", "" },
  { "Code", "" },
  { "Package", "" },
  { "Feature", "" },
  { "Id", "" },
  { "Fix State", "" },
  { "State", "" },
  { "Fileset", "" },
  { "Path", "" },
  { "Type", "" },
  { "", "" },
};
string_info_t header_string_info = {
  header_strings,
  N_ENTRIES(header_strings),
  HEADER_MSGS
};


 /* If this set of strings is changed, the corresponding strings in */
 /* swvpd.msg must be changed as well. */
 /* Also, the corresponding defined constants, above, must be changed. */
msg_string_t media_strings[] = {
  { "", "" },
  { "diskette", "" },
  { "tape", "" },
  { "file", "" },
};
string_info_t media_string_info = {
  media_strings,
  N_ENTRIES(media_strings),
  MEDIA_MSGS
};


#else   /* STRING_DEFNS */
extern msg_string_t     cs_type_strings[];
extern string_info_t    cs_type_string_info;
extern msg_string_t     lstate_strings[];
extern string_info_t    lstate_string_info;
extern msg_string_t     hstate_strings[];
extern string_info_t    hstate_string_info;
extern msg_string_t     event_strings[];
extern string_info_t    event_string_info;
extern msg_string_t     inv_ftype_strings[];
extern string_info_t    inv_ftype_string_info;
extern msg_string_t     inv_format_strings[];
extern string_info_t    inv_format_string_info;
extern msg_string_t     header_strings[];
extern string_info_t    header_string_info;
extern msg_string_t     media_strings[];
extern string_info_t    media_string_info;
#endif  /* ! STRING_DEFNS */

#endif  /* ! _SWVPD_STR_H */
