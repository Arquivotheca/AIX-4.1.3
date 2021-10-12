/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: ATOI
 *		add_to_copy_section
 *		copy_match
 *		copyrights_from_file
 *		copyrights_from_string
 *		copyrights_ok
 *		default_copyright
 *		free_copy_section
 *		legal_copyright
 *		normalize_copy_section
 *		normalize_text
 *		read_legal_copyrights
 *		read_legal_copyrights2
 *		reduce_copy_section
 *		remove_leading_whitespace
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * COPYRIGHT NOTICE
 * Copyright (c) 1993, 1992, 1991, 1990 Open Software Foundation, Inc.
 * Permission is hereby granted to use, copy, modify and freely distribute
 * the software in this file and its documentation for any purpose without
 * fee, provided that the above copyright notice appears in all copies and
 * that both the copyright notice and this permission notice appear in
 * supporting documentation.  Further, provided that the name of Open
 * Software Foundation, Inc. ("OSF") not be used in advertising or
 * publicity pertaining to distribution of the software without prior
 * written permission from OSF.  OSF makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */
/*
 * HISTORY
 * $Log: copyrights.c,v $
 * Revision 1.1.9.2  1993/11/08  17:58:37  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/08  16:08:27  damon]
 *
 * Revision 1.1.9.1  1993/11/03  20:40:21  damon
 * 	CR 463. More pedantic
 * 	[1993/11/03  20:38:01  damon]
 * 
 * Revision 1.1.5.7  1993/09/24  17:23:10  damon
 * 	CR 687. Fixed COPYRIGHT NOTICE checking
 * 	[1993/09/24  17:12:59  damon]
 * 
 * Revision 1.1.5.6  1993/09/24  16:27:05  damon
 * 	CR 670. Added forwd decl for normalize_text
 * 	[1993/09/24  16:26:58  damon]
 * 
 * Revision 1.1.5.5  1993/09/24  16:22:45  damon
 * 	CR 670. Added forwd decl for normalize_text
 * 	[1993/09/24  16:22:34  damon]
 * 
 * Revision 1.1.5.4  1993/09/23  14:29:14  damon
 * 	CR 656. Use raw copyrights instead of valid ones
 * 	[1993/09/23  14:28:36  damon]
 * 
 * Revision 1.1.5.3  1993/09/21  22:11:15  marty
 * 	CR # 670 - Add \n to valid_copyrights.
 * 	[1993/09/21  22:11:02  marty]
 * 
 * Revision 1.1.5.2  1993/09/17  16:14:00  damon
 * 	CR 377. Removed copy_len debugging code
 * 	[1993/09/17  16:13:48  damon]
 * 
 * Revision 1.1.5.1  1993/09/16  17:19:07  damon
 * 	Added COPYRIGHT NOTICE handling
 * 	[1993/09/16  17:17:55  damon]
 * 
 * Revision 1.1.2.7  1993/04/28  14:36:07  damon
 * 	CR 463. More pedantic changes
 * 	[1993/04/28  14:34:50  damon]
 * 
 * Revision 1.1.2.6  1993/04/27  20:06:55  damon
 * 	CR 463. Made pedantic changes
 * 	[1993/04/27  20:06:50  damon]
 * 
 * Revision 1.1.2.5  1993/04/09  17:15:45  damon
 * 	CR 446. Remove warnings with added includes
 * 	[1993/04/09  17:15:01  damon]
 * 
 * Revision 1.1.2.4  1993/04/08  16:30:00  damon
 * 	CR 446. Clean up include files
 * 	[1993/04/08  16:28:05  damon]
 * 
 * Revision 1.1.2.3  1993/01/22  18:42:14  damon
 * 	CR 396. Now uses ode errnos
 * 	[1993/01/22  18:41:54  damon]
 * 
 * Revision 1.1.2.2  1993/01/15  16:10:29  damon
 * 	CR 376. Moved code from sci_rcs.c
 * 	[1993/01/15  15:46:47  damon]
 * 
 * $EndLog$
 */

#ifndef lint
static char sccsid[] = "@(#)75  1.1  src/bldenv/sbtools/libode/copyrights.c, bldprocess, bos412, GOLDA411a 1/19/94 17:40:49";
#endif /* not lint */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ode/copyrights.h>
#include <ode/errno.h>
#include <ode/interface.h>
#include <ode/odedefs.h>
#include <ode/parse_rc_file.h>
#include <ode/public/error.h>
#include <ode/sci.h>  /* just for log_error() */
#include <ode/util.h>
#include <sys/param.h>

#define CR              "COPYRIGHT"
#define CR_VAR          "copyright_list"        /* rc_files variable */
#define COPYRIGHT_MARKERS "OSF_COPYRIGHT;OSF_FREE_COPYRIGHT"

char *copyright_markers;

/* linked list element for copyright markers */
struct copy_elem {
  char *cr_name;                /* copyright marker string */
  struct copy_elem *cr_next;    /* pointer to next marker */
};

struct copy_elem *copy_list;    /* head of linked list */
struct copy_elem *copy_tail;    /* tail of linked list */

int copy_size;
int copy_len;
char * copy_section;
int valid_copyrights;
char * raw_copyright_list[1000];
char * valid_copyright_list[1000];
int valid_copyright_len[1000];
char * copyright_name[1000];
int years_position[1000];
BOOLEAN check_copyrights = FALSE;

void
normalize_text(char * text);

void
normalize_copy_section(void);

#define ATOI(n, p) \
    (n) = 0; \
    if ('0' > *(p) || *(p) > '9') \
        return(1); \
    while ('0' <= *(p) && *(p) <= '9') { \
        (n) = (n) * 10 + (*(p) - '0'); \
        (p)++; \
    }

/*
 * function to check for permissible copyright marker
 */
BOOLEAN
legal_copyright ( char *str )
{
  struct copy_elem *copy_ptr;

  copy_ptr = copy_list;
  while ( copy_ptr != NULL) {
    ui_print ( VDEBUG, "checking: %s\n", copy_ptr->cr_name );
    /* go through list of valid markers checking for match */
    if ( strstr (str, copy_ptr->cr_name) != NULL ) {
      ui_print ( VDEBUG, "Found legal copyright marker: %s\n", copy_ptr->cr_name
 );
      return (TRUE);
    } /* if */
    copy_ptr = copy_ptr->cr_next;
  }
  ui_print ( VDEBUG, "Couldn't find matching copyright marker for: %s", str );
  return (FALSE);
} /* end legal_copyright */

/*
 * read copyright markers from file and add to copy_list
 * returns: ERROR - on error
 *          OK - normally
 */
ERR_LOG
copyrights_from_file ( char * copyrights_rc )
{
  char buf[MAXPATHLEN];
  FILE *copy_f;
  int has_entries;
  struct copy_elem *copy_ptr;
  int lines;

  ui_print ( VDEBUG, "Looking for copyright markers in file: '%s'.\n",
             copyrights_rc );
  lines = 0;
 /*
  * Read in list of legal copyright markers.
  */
  if ( ( copy_f = fopen(copyrights_rc, "r" ) ) == NULL ) {
    return ( err_log ( OE_MARKERFILE, copyrights_rc ) );
  }
  has_entries = FALSE;
  while ( fgets ( buf, sizeof ( buf ), copy_f ) != NULL ) {
    char tmpbuf[MAXPATHLEN];

    lines += 1;
    if (buf[0] == '#')  /* comment to end of line */
    {
      ui_print ( VDEBUG, "Skipping comment line %d\n", lines );
      continue;
    }
    ui_print ( VDEBUG, "Line %d: %s\n", lines, buf );
    copy_ptr = (struct copy_elem *)malloc((size_t)( sizeof ( struct copy_elem )
));
    has_entries = TRUE;
    if ((strlen(buf) > (unsigned)0) && (buf[strlen(buf)-1] == '\n') ) {
      buf[strlen(buf)-1] = NUL;
    }
    if ( ( buf[0] == NUL) || ( buf[0] == '\n') ) {
      return ( err_log ( OE_EMPTYMARKER, copyrights_rc ) );
    }
    strcpy(tmpbuf, buf);
    if ( strstr ( buf, CR ) == NULL) {
      return ( err_log ( OE_BADMARKER, buf ) );
    } /* while */
    concat(buf, sizeof(buf), "@", tmpbuf, "@", NULL);
    copy_ptr->cr_name = strdup(buf);
    copy_ptr->cr_next = NULL;
    if ( copy_list == NULL ) {
      copy_list = copy_ptr;
      copy_tail = copy_ptr;
    } else {
      copy_tail->cr_next = copy_ptr;
      copy_tail = copy_ptr;
    } /* if */
  }
  if ( ! has_entries) {
    return ( err_log ( OE_NOMARKERS ) );
  } /* if */
  return ( OE_OK );
} /* end copyrights_from_file */

/*
 * parse string for valid copyright markers and add to copy_list
 * returns: OK - string is parsed successfully
 *          ERROR - otherwise
 */
ERR_LOG
copyrights_from_string( char *copyrights_str )
{
  char buf[MAXPATHLEN];
  int entries;
  int has_entries;
  struct copy_elem *copy_ptr;
  char * str_ptr;
  ERR_LOG log;

  ui_print ( VDEBUG, "Looking for copyright markers in string:\n" );
  ui_print ( VCONT, "%s \n", copyrights_str );
  has_entries = FALSE;
  entries = 0;
  while ( *(str_ptr = nxtarg ( &copyrights_str, "; " ) ) != NUL ) {
    char tmpbuf[MAXPATHLEN];


    if ( strcmp ( str_ptr, "include" ) == 0 ) {
      str_ptr = nxtarg ( &copyrights_str, ";" );
      if ( *str_ptr == NUL )
        return ( err_log ( OE_INCMARKER ) );
      else if ( ( log = copyrights_from_file ( str_ptr ) ) != OE_OK )
        return ( log );
      /* end if */
      has_entries = TRUE;
      continue;
    } /* end if */
    entries += 1;
    ui_print ( VDEBUG, "Entry %d: %s\n", entries, str_ptr );
    copy_ptr = (struct copy_elem *)malloc((size_t) sizeof ( struct copy_elem ) )
;
    has_entries = TRUE;
    strcpy(tmpbuf, str_ptr);
    if ( strstr (str_ptr, CR ) == NULL) {
      return ( err_log ( OE_BADMARKER, str_ptr ) );
    }
    concat(buf, sizeof(buf), "@", tmpbuf, "@", NULL);
    copy_ptr->cr_name = strdup(buf);
    copy_ptr->cr_next = NULL;
    if ( copy_list == NULL ) {
      copy_list = copy_ptr;
      copy_tail = copy_ptr;
    } else {
      copy_tail->cr_next = copy_ptr;
      copy_tail = copy_ptr;
    } /* if */
  } /* end while */
  if ( ! has_entries) {
    return ( err_log ( OE_NOMARKERS ) );
  } else
    return ( OE_OK);
  /* end if */
} /* end copyrights_from_string */

ERR_LOG
read_legal_copyrights ( struct rcfile * contents )
{
  char *copyrights_rc;
  ERR_LOG log;

  copy_list = NULL;
  copy_tail = NULL;
  ui_print ( VDEBUG, "Composing list of legal copyright markers.\n" );

  if ( get_rc_value (CR_VAR, &copyrights_rc, contents, FALSE ) == OK )
  {
    ui_print ( VDEBUG, "copyright rc variable: (starts on next line)\n" );
    ui_print ( VCONT, "%s \n", copyrights_rc ) ;
    log = copyrights_from_string( copyrights_rc );
  } else {
    ui_print ( VDEBUG, "Using default copyright markers.\n" );
    copyright_markers = strdup ( COPYRIGHT_MARKERS );
    log = copyrights_from_string( copyright_markers );
  } /* if */
  return ( log );
} /* end read_legal_copyrights */

ERR_LOG
read_legal_copyrights2 ( char * copyright_file )
{
  char buf[MAXPATHLEN];
  FILE *copy_f;
  char * token;
  char * buf_ptr;
  char copyright[5000];
  char raw_copy[5000];
  char tmp_copy[5000];
  char tmp_copy2[5000];
  BOOLEAN end_of_file;
  char * pos;

  check_copyrights = TRUE;

  copy_size = 5000;
  copy_len = 0;
  copy_section = (char *)malloc ( copy_size );
  *copy_section = '\0';
  valid_copyrights = 0;
  ui_print ( VDEBUG, "Looking for legal copyrights in file: '%s'.\n",
             copyright_file );
 /*
  * Read in list of legal copyright markers.
  */
  if ( ( copy_f = fopen(copyright_file, "r" ) ) == NULL ) {
    return ( err_log ( OE_OPEN, copyright_file ) );
  }
  if ( fgets ( buf, sizeof ( buf ), copy_f ) == NULL ) {
    return ( OE_OK );
  } /* if */
  if ( strncmp ( buf, "COPYRIGHT NOTICE", 16 ) != 0 ) {
    ui_print ( VFATAL, "First line in copyrights file must be COPYRIGHT NOTICE\n"
             );
    return ( err_log ( OE_INTERNAL ) );
  } /* if */
  for (;;) {
    buf_ptr = buf+16;
    token = nxtarg ( &buf_ptr, WHITESPACE );
    copyright_name[valid_copyrights] = strdup ( token );
    copyright[0] = '\0';
    raw_copy[0] = '\0';
    end_of_file = FALSE;
    for (;;) {
      if ( fgets ( buf, sizeof ( buf ), copy_f ) == NULL ) {
        end_of_file = TRUE;
        break;
      } /* if */
      if ( strncmp ( buf, "COPYRIGHT NOTICE", 16 ) == 0 ) {
        break;
      } /* if */
      rm_newline ( buf );
      concat ( tmp_copy, sizeof(tmp_copy), copyright, " ", buf,  NULL );
      concat ( tmp_copy2, sizeof(tmp_copy2), raw_copy, buf, "\n",  NULL );
      strcpy ( copyright, tmp_copy );
      strcpy ( raw_copy, tmp_copy2 );
    } /* for */
    normalize_text (copyright);
    raw_copyright_list[valid_copyrights] = strdup ( raw_copy );
    valid_copyright_list[valid_copyrights] = strdup ( copyright );
    valid_copyright_len[valid_copyrights] = strlen ( copyright );
    if ( ( pos = strstr ( copyright, "@YEARS@" ) ) == NULL ) {
      years_position[valid_copyrights] = -1;
    } else {
      years_position[valid_copyrights] = pos-copyright;
    } /* if */
    valid_copyrights++;
    if ( end_of_file ) {
      break;
    } /* if */
  } /* for */
  return ( OE_OK );
} /* end read_legal_copyrights2 */

/*
 * function to check for permissible copyright marker
 */
char *default_copyright ( void )
{
  if (copy_list == NULL)
    return(NULL);
  else
    return (copy_list->cr_name);
} /* end legal_copyright */

void
free_copy_section(void)
{
  free(copy_section);
} /* end free_copy_section */

void
add_to_copy_section ( const char * leader, const char * buf )
{
  int leader_len;
  int len;
  int new_size;

  leader_len = strlen ( leader );
  len = strlen ( buf );
  if ( leader_len < len ) {
    new_size = copy_len+len-leader_len+2; /* +1 for " \0" */
    if ( new_size > copy_size ) {
      copy_section = (char *) realloc ( copy_section, new_size );
      copy_size = new_size;
    } /* if */
    *(copy_section+copy_len) = ' ';
    copy_len = copy_len + 1;
    strcpy ( copy_section+copy_len, buf+leader_len );
    copy_len = copy_len + len-leader_len;
  } /* if */
} /* end add_to_copy_section */

void
remove_leading_whitespace(void)
{
  int i;
  char c;

  if ( copy_len == 0 ) {
    return;
  } /* if */
  i = 0;
  for (;;) {
    c = *(copy_section+i);
    if  ( c != ' ' && c != '\t' ) {
      break;
    } /* if */
    i++;
  } /* for */
  if ( i > 0 ) {
    strncpy ( copy_section, copy_section+i, copy_len - i );
    copy_len = copy_len - i;
  } /* if */
} /* end remove_leading_whitespace */

int
copy_match ( int i )
{
  int pos;
  int years_pos;
  int remaining_size;
  int last_digit_pos=0;

  years_pos = years_position[i]; 
  if ( years_pos == -1 ) {
    if ( strncmp ( valid_copyright_list[i], copy_section,
         valid_copyright_len[i]) == 0 ) {
      return ( valid_copyright_len[i] );
    } /* if */
  } else {
    if ( strncmp ( valid_copyright_list[i], copy_section,
         years_pos ) == 0 ) {
      pos = years_pos;
      while ( pos < copy_len-1 &&
              strchr ( ", \t0123456789", *(copy_section+pos) ) != NULL) {
        if ( strchr ( "0123456789", *(copy_section+pos) ) != NULL ) {
          last_digit_pos = pos;
        } /* if */
        pos++;
      } /* while */
      if ( pos == copy_len-1 ) {
        return ( pos );
      } /* if */
      remaining_size = valid_copyright_len[i] - 7 - years_position[i];
      if ( strncmp ( valid_copyright_list[i]+years_pos+7,
                     copy_section+last_digit_pos+1, remaining_size) == 0 ) {
        return ( last_digit_pos+1+remaining_size );
      } /* if */
    } /* if */
  } /* if */
  return ( 0 );
} /* end copy_match */

BOOLEAN
reduce_copy_section(void)
{
  BOOLEAN removed_text;
  int i;
  int match_len;
  char buf[40];

  do {
    removed_text = FALSE;
    for ( i = 0; i < valid_copyrights; i++ ) {
      if ( ( match_len = copy_match ( i ) ) > 0 ) {
        strncpy ( copy_section, copy_section+match_len,
                  copy_len - match_len );
        copy_len = copy_len - match_len;
        remove_leading_whitespace ();
        removed_text = TRUE;
      } /* if */
    } /* for */
  } while ( removed_text );
  if ( copy_len != 1 ) {
    strncpy ( buf, copy_section, 39 );
    ui_print ( VWARN, "The copyright section contains an invalid copyright.\n" );
    ui_print ( VCONT, "The invalid copyright starts with:\n" );
    ui_print ( VCONT, "'%s'\n", buf );
  } /* if */
  return ( copy_len == 1 );
} /* end reduce_copy_section */

void
normalize_copy_section(void)
{
  normalize_text ( copy_section );
  copy_len = strlen(copy_section)+1;
} /* end normalize_copy_section */

void
normalize_text(char * text)
{
  int i;
  int n;
  char c = '\0';
  BOOLEAN in_whitespace;
  int first_white;

  n = 0;
  first_white=0;
  in_whitespace = (*text == ' ' || *text == '\t' );

  for ( i = 0; i < strlen(text); i++ ) {
    c = *(text+i);
    if ( c == ' ' || c == '\t' ) {
      if ( !in_whitespace ) {
        in_whitespace = TRUE;
        *(text+n) = ' ';
        first_white = n;
        n++;
      } /* if */
    } else {
      in_whitespace = FALSE;
      *(text+n) = c;
      n++;
    } /* if */
  } /* for */
  if ( c == ' ' || c == '\t' ) {
    n = first_white;
  } /* if */
  *(text+n) = '\0';
} /* end normalize_text */

BOOLEAN
copyrights_ok (void)
{
  if ( check_copyrights ) {
    normalize_copy_section();
    return ( reduce_copy_section() );
  } else {
    return ( TRUE );
  } /* if */
} /* end copyrights_ok */
