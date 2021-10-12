/*
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: getframe
 *		oxm_check
 *		oxm_close
 *		oxm_endcmd
 *		oxm_get_all
 *		oxm_gets
 *		oxm_init
 *		oxm_open
 *		oxm_poll
 *		oxm_read
 *		oxm_runcmd
 *		oxm_stdout
 *		oxm_write
 *		pipe_init
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
 * @OSF_FREE_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1992, 1991, 1990  
 * Open Software Foundation, Inc. 
 *  
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
 * $Log: odexm_client.c,v $
 * Revision 1.1.15.4  1993/11/10  14:37:18  root
 * 	CR 463. Removed machine/endian.h include
 * 	[1993/11/10  14:36:09  root]
 *
 * Revision 1.1.15.3  1993/11/08  20:18:15  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/08  20:17:32  damon]
 * 
 * Revision 1.1.15.2  1993/11/08  17:58:51  damon
 * 	CR 463. Pedantic changes
 * 	[1993/11/08  16:08:38  damon]
 * 
 * Revision 1.1.15.1  1993/11/03  20:40:39  damon
 * 	CR 463. More pedantic
 * 	[1993/11/03  20:38:13  damon]
 * 
 * Revision 1.1.12.3  1993/09/29  14:48:06  damon
 * 	CR 705. ifdndef NO_POOLL for poll.h
 * 	[1993/09/29  14:47:51  damon]
 * 
 * Revision 1.1.12.2  1993/09/29  13:55:18  damon
 * 	CR 705. ifdef for NO_POLL for platforms without poll()
 * 	[1993/09/29  13:53:41  damon]
 * 
 * Revision 1.1.12.1  1993/09/07  19:54:43  marty
 * 	CR # 653 - Change POLLNORM to POLLIN.
 * 	[1993/09/07  19:53:23  marty]
 * 
 * Revision 1.1.10.4  1993/08/19  18:26:28  damon
 * 	CR 622. Changed if STDC to ifdef STDC
 * 	[1993/08/19  18:25:46  damon]
 * 
 * Revision 1.1.10.3  1993/07/29  20:38:54  damon
 * 	CR 615. oxm_poll works in buffer full case
 * 	[1993/07/29  20:38:40  damon]
 * 
 * Revision 1.1.10.2  1993/07/29  17:26:29  damon
 * 	CR 614 changed strncpy in oxm_write to memcpy
 * 	[1993/07/29  17:26:23  damon]
 * 
 * Revision 1.1.10.1  1993/07/21  16:01:38  damon
 * 	CR 607. Added oxm_stdout and oxm_poll for odexm_cli
 * 	[1993/07/21  16:01:11  damon]
 * 
 * Revision 1.1.6.13  1993/05/26  18:07:45  damon
 * 	CR 553. Added port field
 * 	[1993/05/26  17:17:17  damon]
 * 
 * Revision 1.1.6.12  1993/05/20  13:58:36  damon
 * 	CR 515. Added oxm_write
 * 	[1993/05/20  13:58:29  damon]
 * 
 * Revision 1.1.6.11  1993/05/05  19:40:18  damon
 * 	CR 485. oxm_read returns when buffer size exceeded
 * 	[1993/05/05  19:40:13  damon]
 * 
 * Revision 1.1.6.10  1993/05/05  14:48:25  damon
 * 	CR 485. Added oxm_read()
 * 	[1993/05/05  14:48:19  damon]
 * 
 * Revision 1.1.6.9  1993/05/04  19:46:18  damon
 * 	CR 486. Added size parameter to oxm_gets
 * 	[1993/05/04  19:45:55  damon]
 * 
 * Revision 1.1.6.8  1993/04/29  19:52:10  damon
 * 	CR 463. Changed machine/endian.h to netinet/in.h
 * 	[1993/04/29  19:52:01  damon]
 * 
 * Revision 1.1.6.7  1993/04/28  14:53:47  damon
 * 	CR 463. Pedantic changes
 * 	[1993/04/28  14:53:37  damon]
 * 
 * Revision 1.1.6.6  1993/04/08  19:53:15  damon
 * 	CR 446. Clean up include files
 * 	[1993/04/08  19:53:04  damon]
 * 
 * Revision 1.1.6.5  1993/03/29  18:52:44  damon
 * 	CR 436. Run ntohl on return status
 * 	[1993/03/29  18:52:25  damon]
 * 
 * Revision 1.1.6.4  1993/03/15  17:53:13  damon
 * 	CR 436. Now empties pipe before closing
 * 	[1993/03/15  17:53:04  damon]
 * 
 * Revision 1.1.6.3  1993/03/05  20:55:57  damon
 * 	CR 436. Removed extra fprintf
 * 	[1993/03/05  20:21:16  damon]
 * 
 * Revision 1.1.6.2  1993/03/04  21:29:52  damon
 * 	CR 436. Added ability to execute commands locally
 * 	[1993/03/04  20:00:37  damon]
 * 
 * Revision 1.1.2.12  1992/12/03  17:21:21  damon
 * 	ODE 2.2 CR 183. Added CMU notice
 * 	[1992/12/03  17:08:30  damon]
 * 
 * Revision 1.1.2.11  1992/11/23  16:59:32  damon
 * 	CR 334. Changed type of count for oxm_header to unsigned short
 * 	[1992/11/23  16:59:19  damon]
 * 
 * Revision 1.1.2.10  1992/11/12  18:28:01  damon
 * 	CR 329. Added conditional use of varargs.h
 * 	[1992/11/12  18:10:35  damon]
 * 
 * Revision 1.1.2.9  1992/11/05  22:00:54  damon
 * 	CR 328. Removed extra debugging code
 * 	[1992/11/05  22:00:44  damon]
 * 
 * Revision 1.1.2.8  1992/09/24  19:01:59  gm
 * 	CR282: Made more portable to non-BSD systems.
 * 	[1992/09/23  18:22:13  gm]
 * 
 * Revision 1.1.2.7  1992/09/17  13:29:03  damon
 * 	CR 240. Added some error handling, cleaned warnings
 * 	[1992/09/17  13:25:18  damon]
 * 
 * Revision 1.1.2.6  1992/09/15  18:59:13  damon
 * 	Fixed oxm_endcmd to wait for end of file transmission
 * 	[1992/09/15  18:55:57  damon]
 * 
 * Revision 1.1.2.5  1992/09/01  19:30:23  damon
 * 	Added oxm_get_all, debugging, better arg processing
 * 	[1992/09/01  19:26:06  damon]
 * 
 * Revision 1.1.2.4  1992/08/26  19:16:50  damon
 * 	CR 240. Fixed initialization problem
 * 	[1992/08/26  19:16:37  damon]
 * 
 * Revision 1.1.2.3  1992/08/24  20:08:02  damon
 * 	Added status return to oxm_endcmd
 * 	[1992/08/24  20:07:37  damon]
 * 
 * Revision 1.1.2.2  1992/08/20  19:39:22  damon
 * 	CR 240. Initial Version
 * 	[1992/08/20  19:39:04  damon]
 * 
 * $EndLog$
 */

#ifndef lint
static char sccsid[] = "@(#)96  1.1  src/bldenv/sbtools/libode/odexm_client.c, bldprocess, bos412, GOLDA411a 1/19/94 17:41:36";
#endif /* not lint */

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <ode/errno.h>
#include <ode/interface.h>
#include <ode/odedefs.h>
#include <ode/odexm.h>
#include <ode/public/error.h>
#include <ode/private/odexm.h>
#include <ode/run.h>
#include <sys/ioctl.h>
#ifndef NO_POLL
#include <sys/poll.h>
#endif
#include <sys/types.h>
#include <sys/time.h>

typedef struct oxm_type * OXM_CNXTN;

struct oxm_type {
  BOOLEAN running;
  BOOLEAN eof;
  char buf [ OXMBUFSIZE ];
  int buf_filled;
  int buf_pos;
  int cmd_pid;
  int in_pipe [2];
  int out_pipe [2];
  FILE * output;
  FILE * input;
};

/*
 * GLOBALS
 */

static
OXMINIT * oxminit;

BOOLEAN oxm_local = FALSE;

ERR_LOG
oxm_init ( int initct, OXMINIT * init )
{

  oxminit = init;
  return ( OE_OK );
} /* end oxm_init */

ERR_LOG oxm_check ( OXM_CNXTN oxm_cnxtn )
{
  return ( OE_OK );
} /* end oxm_check */

STATIC
void
pipe_init ( va_list ap )
{
  char * dir;
  OXM_CNXTN oxm_cnxtn;
  int status;

  dir = va_arg ( ap, char * );
  oxm_cnxtn = va_arg ( ap, OXM_CNXTN );
  status = dup2 ( oxm_cnxtn -> out_pipe [0], 0 );
  if ( status == -1 )
    fprintf ( stderr, "dup2 failed with %d\n", errno );
  status = dup2 ( oxm_cnxtn -> in_pipe [1], 1 );
  if ( status == -1 )
    ui_print ( VDEBUG, "dup2 failed with %d\n", errno );
  close ( oxm_cnxtn -> in_pipe [0] );
  close ( oxm_cnxtn -> in_pipe [1] );
  close ( oxm_cnxtn -> out_pipe [0] );
  close ( oxm_cnxtn -> out_pipe [1] );
} /* pipe_init */

ERR_LOG oxm_open ( OXM_CNXTN * oxm_cnxtn, int monitor )
{
  const char *av [16];
  int i;
  const char *relay;
  OXMINIT * init_ptr;
  ODEXM_HEADER header;

  if ( (*oxm_cnxtn = (OXM_CNXTN) malloc ( sizeof ( **oxm_cnxtn ) ) ) == NULL )
    return ( err_log ( OE_ALLOC ) );
  /* if */
  (*oxm_cnxtn) -> running = FALSE;
  (*oxm_cnxtn) -> eof = TRUE;

  if ( oxm_local )
    return ( OE_OK );
  /* if */

  init_ptr = oxminit + monitor;
  i = 0;
  av [ i++ ] = "authcover";
  av [ i++ ] = init_ptr -> host;
  av [ i++ ] = init_ptr -> ident;
  if ( init_ptr -> port != NULL ) {
    av [ i++ ] = init_ptr -> port;
  } /* if */
  av [ i++ ] = NULL;
  relay = init_ptr -> relay;

  (*oxm_cnxtn) -> buf_filled = 0;
  (*oxm_cnxtn) -> buf_pos = sizeof(header);
  if ( pipe ( (*oxm_cnxtn) -> in_pipe ) != 0 )
    return ( err_log ( OE_PIPEOPEN ) );
  /* if */
  if ( pipe ( (*oxm_cnxtn) -> out_pipe ) != 0 )
    return ( err_log ( OE_PIPEOPEN ) );
  /* if */
  (*oxm_cnxtn) -> cmd_pid = full_runcmdv ( relay, av, TRUE, pipe_init,
                                           (char *) NULL,
                                           *oxm_cnxtn );
  if ( (*oxm_cnxtn) -> cmd_pid < 0 ) {
    ui_print ( VFATAL, "full_runcmdv failed\n" );
    return ( err_log ( OE_INTERNAL ) );
  } /* if */
  if ( ((*oxm_cnxtn) -> output = fdopen ( (*oxm_cnxtn) -> out_pipe [1], "w" ) )
       == NULL )
    return ( err_log ( OE_SYSERROR ) );
  (void) close ( (*oxm_cnxtn) -> in_pipe [1] );
  (void) close ( (*oxm_cnxtn) -> out_pipe [0] );
  return ( OE_OK );
} /* end oxm_open */

ERR_LOG oxm_runcmd ( OXM_CNXTN oxm_cnxtn, int argc, char ** argv,
                     char * dir )
{
  int i;
  char ** argptr;

  if ( oxm_cnxtn -> running )
    return ( err_log ( OE_INTERNAL ) );
  /* if */
  oxm_cnxtn -> eof = FALSE;
  oxm_cnxtn -> running = TRUE;

  i = argc; 
  argptr = argv;
  ui_print ( VDEBUG, "Remote command:\n" );
  if ( dir != NULL ) {
    ui_print ( VCONT, "run from: %s\n", dir );
  } /* if */
  while ( i > 0 ) {
    ui_print ( VCONT, "%s\n", *argptr );
    i--;
    argptr++;
  } /* while */
  if ( oxm_local ) {
    pipe ( oxm_cnxtn -> in_pipe );
    if ( ( oxm_cnxtn -> input = fdopen ( oxm_cnxtn -> in_pipe [0], "r" ) )
       == NULL )
      return ( err_log ( OE_SYSERROR ) );
    /* if */
    oxm_cnxtn -> cmd_pid = fork ();
    if ( oxm_cnxtn -> cmd_pid == 0 ) {
      dup2 ( oxm_cnxtn -> in_pipe [1], 1 );
      close ( oxm_cnxtn -> in_pipe [0] );
      close ( oxm_cnxtn -> in_pipe [1] );
      exit ( runvp ( *argv, argv ) );
    } /* if */
    close ( oxm_cnxtn -> in_pipe [1] );
    return ( OE_OK );
  } /* if */ 
  i = argc; 
  argptr = argv;
  if ( dir == NULL ) {
    putw ( 0, oxm_cnxtn -> output );
  } else {
    putw ( strlen ( dir ), oxm_cnxtn -> output );
    fprintf ( oxm_cnxtn -> output, "%s", dir );
  } /* if */
  putw ( i, oxm_cnxtn -> output );
  while ( i > 0 ) {
    putw ( strlen (*argptr ), oxm_cnxtn -> output );
    fprintf ( oxm_cnxtn -> output, "%s", *argptr );
    i--;
    argptr++;
  } /* while */
  fflush ( oxm_cnxtn -> output );
  return ( OE_OK );
} /* end oxm_runcmd */

#ifndef NO_POLL
/*
 * oxm_poll should only be called after at least one oxm_read call has
 * been made.
 */
BOOLEAN
oxm_poll ( OXM_CNXTN oxm_cnxtn )
{
  int frame_size;
  int move_size;
  ODEXM_HEADER * header;
  struct pollfd pfd[1];

  pfd[0].fd = oxm_cnxtn -> in_pipe[0];
  pfd[0].events = POLLIN;
  header = (ODEXM_HEADER *) (oxm_cnxtn -> buf);
  if ( oxm_cnxtn -> buf_filled > 0 ) {
    if ( oxm_cnxtn -> buf_pos < (*header).count + sizeof (*header) ) {
      return ( TRUE );
    } /* if */
    frame_size = (*header).count + sizeof (*header);
    move_size = oxm_cnxtn -> buf_filled - frame_size;
    if ( move_size > 0 ) {
      return ( TRUE );
    }
  } /* if */
  if ( poll ( pfd, 1, 0 ) > 0 ) {
    return ( TRUE );
  } else {
    return ( FALSE );
  } /* if */ 
} /* end oxm_poll */
#endif

int
oxm_stdout ( OXM_CNXTN oxm_cnxtn )
{
  return (oxm_cnxtn -> in_pipe [0]);
} /* end oxm */

/*
 * Get at least a full frame of data from the pipe.
 */
STATIC
ERR_LOG getframe ( OXM_CNXTN oxm_cnxtn )
{
  char tmpbuf [ OXMBUFSIZE ];
  int cc;
  ODEXM_HEADER * header;
  char * bufptr;
  int move_size;
  int frame_size;
  int bufroom;

  header = (ODEXM_HEADER *) (oxm_cnxtn -> buf);

 /*
  * Align the start of the next frame at the beginning of the buffer.
  */
  if ( oxm_cnxtn -> buf_filled > 0 ) {
    frame_size = (*header).count + sizeof (*header);
    move_size = oxm_cnxtn -> buf_filled - frame_size;
    if ( move_size > 0 ) {
      memcpy ( tmpbuf, (oxm_cnxtn -> buf ) + frame_size, move_size );
      memcpy ( oxm_cnxtn -> buf, tmpbuf, move_size );
    } /* if */
    oxm_cnxtn -> buf_filled = move_size;
    oxm_cnxtn -> buf_pos = sizeof(*header);
  } /* if */

 /*
  * Set the bufptr at the end of the filled part of the buffer.
  */
  bufptr = (char *)((oxm_cnxtn -> buf) + oxm_cnxtn -> buf_filled );

  for (;;) {
    if ( oxm_cnxtn -> buf_filled >= sizeof (*header) )
      if ( oxm_cnxtn -> buf_filled >= (*header).count + sizeof (*header) )
        break;
      /* if */
    /* if */

   /*
    * The largest frame we can have is OXMBUFSIZE - sizeof(cc) bytes of data.
    * Don't read more than that into the buffer.
    */
    bufroom = OXMBUFSIZE - oxm_cnxtn -> buf_filled;
    cc = read( oxm_cnxtn -> in_pipe [0], bufptr, bufroom );
    if (  cc < 0 ) {
      if ( errno == EWOULDBLOCK )
        continue;
      /* if */
      return ( err_log ( OE_PIPEREAD ) );
    } /* if */
    if ( cc == 0 )
      return ( err_log ( OE_PIPEREAD ) );
    else {
      bufptr += cc;
      oxm_cnxtn -> buf_filled += cc;
    } /* if */
  } /* for */
  return ( OE_OK );
} /* end getframe */

ERR_LOG oxm_get_all ( OXM_CNXTN oxm_cnxtn, int fd )
{
  ERR_LOG log;
  ODEXM_HEADER * header;
  char * bufptr;
  char buf [OXMBUFSIZE];
  int b;

  if ( oxm_local ) {
    while ( ( b = read ( oxm_cnxtn -> in_pipe[0], buf, sizeof(buf) ) ) > 0 )
      write ( fd, buf, b );
    /* while */
   return ( OE_OK );
  } /* if */

  header = (ODEXM_HEADER *) (oxm_cnxtn -> buf);
  bufptr = (char *)((oxm_cnxtn -> buf) + sizeof (*header) );

  do {
    if ( ( log = getframe ( oxm_cnxtn )) != OE_OK ) {
      return ( log );
    } /* if */
    write ( fd, bufptr, (*header).count );
  } while ( (*header).count > 0 );
  oxm_cnxtn -> eof = TRUE;
  return ( OE_OK );
} /* end oxm_get_all */

int
oxm_read ( OXM_CNXTN oxm_cnxtn, char * str, int size, ERR_LOG * log )
{
  int i;
  ODEXM_HEADER * header;
  char * bufptr;
  char * strptr;
  int len = 0;

  if ( oxm_cnxtn -> eof ) {
    *log = OE_OK;
    return ( 0 );
  } /* if */

  if ( oxm_local ) {
    *log = OE_OK;
    return ( read ( oxm_cnxtn -> in_pipe[0], str, size ) );
  } /* if */
  header = (ODEXM_HEADER *) (oxm_cnxtn -> buf);
  bufptr = (char *)((oxm_cnxtn -> buf) + sizeof (*header) );

  strptr = str;

  if ( oxm_cnxtn -> buf_filled  == 0 ||
       oxm_cnxtn -> buf_pos == (*header).count + sizeof (*header) ) {
    if ( ( *log = getframe ( oxm_cnxtn )) != OE_OK ) {
      return ( -1 );
    } /* if */
    if ( (*header).count == 0 ) {
      oxm_cnxtn -> eof = TRUE;
      *log = OE_OK;
      if ( strptr != str ) {
        return ( len );
      } else {
        return ( 0 );
      } /* if */
    } /* if */
  } /* if */

  for ( i = oxm_cnxtn -> buf_pos; i < (*header).count + sizeof (*header);
        i++ ) {
    if ( len >= size )
      return ( len );
    /* if */
    *strptr = oxm_cnxtn -> buf [i];
    oxm_cnxtn -> buf_pos++;
    strptr++;
    len++;
  } /* for */
  return ( len );
} /* end oxm_read */

char *
oxm_gets ( OXM_CNXTN oxm_cnxtn, char * str, int size, ERR_LOG * log )
{
  int i;
  ODEXM_HEADER * header;
  char * bufptr;
  char * strptr;

  if ( oxm_cnxtn -> eof ) {
    *log = OE_OK;
    return ( NULL );
  } /* if */

  if ( oxm_local ) {
    *log = OE_OK;
    return ( fgets ( str, size, oxm_cnxtn -> input ) );
  } /* if */
  header = (ODEXM_HEADER *) (oxm_cnxtn -> buf);
  bufptr = (char *)((oxm_cnxtn -> buf) + sizeof (*header) );

  strptr = str;

  for (;;) {
    if ( oxm_cnxtn -> buf_filled  == 0 ||
         oxm_cnxtn -> buf_pos == (*header).count + sizeof (*header) ) {
      if ( ( *log = getframe ( oxm_cnxtn )) != OE_OK ) {
        return ( NULL );
      } /* if */
      if ( (*header).count == 0 ) {
        oxm_cnxtn -> eof = TRUE;
        *log = OE_OK;
        if ( strptr != str ) {
          return ( str );
        } else {
          return ( NULL );
        } /* if */
      } /* if */
    } /* if */

    for ( i = oxm_cnxtn -> buf_pos; i < (*header).count + sizeof (*header);
          i++ ) {
      *strptr = oxm_cnxtn -> buf [i];
      oxm_cnxtn -> buf_pos++;
      if ( *strptr == '\n' ) {
        strptr++; 
        *strptr = '\0';
        *log = OE_OK;
        return ( str );
      } /* if */
      strptr++; 
    } /* for */
  } /* for */
} /* end oxm_gets */

int
oxm_write ( OXM_CNXTN oxm_cnxtn, char * str, int size, ERR_LOG * log )
{
  ODEXM_HEADER * header;
  char buf [OXMBUFSIZE];
  char * bufptr;
  int count;
  int bytes_written;
  int chunk_size;
  int total_written;

 /*
  * Init
  */
  total_written = 0;
  header = (ODEXM_HEADER *) buf;
  (*header).status = 0;
  bufptr = buf + sizeof(*header);
  count = size;

  while ( count > 0 ) {
    if ( count > sizeof(buf) - sizeof(*header) ) {
      chunk_size = sizeof(buf) - sizeof(*header);
    } else {
      chunk_size = count;
    } /* if */
    (*header).count = (unsigned short) chunk_size;
    memcpy ( bufptr, str + total_written, chunk_size );
    bytes_written = write ( oxm_cnxtn -> out_pipe [1], buf,
                            chunk_size + sizeof(*header) );
    count = count - bytes_written;
    total_written = total_written + bytes_written;
  } /* while */
  return ( total_written );
} /* end oxm_write */

ERR_LOG oxm_endcmd ( OXM_CNXTN oxm_cnxtn, int * status )
{
  ERR_LOG log;
  ODEXM_HEADER * header;
  char buf [1000];

  if ( ! oxm_cnxtn -> running )
    return ( err_log ( OE_INTERNAL ) );
  /* if */

  if ( oxm_local ) {
    while ( oxm_gets ( oxm_cnxtn, buf, sizeof(buf), &log ) != NULL );
    *status = endcmd ( oxm_cnxtn -> cmd_pid );
    fclose ( oxm_cnxtn -> input );
    oxm_cnxtn -> running = FALSE;
    return ( OE_OK );
  } /* if */

  header = (ODEXM_HEADER * ) oxm_cnxtn -> buf;
  if ( ! oxm_cnxtn -> eof ) {
    do {
      if ( ( log = getframe ( oxm_cnxtn )) != OE_OK)
        return ( log );
    } while ( (*header).count != 0 );
  }  /* if */
  oxm_cnxtn -> running = FALSE;
  *status = ntohl ( (*header).status);
#ifdef notdef
  *status = (*header).status;
#endif
 /*
  * Wait for indication that file transimission (if any) has completed.
  */
  getframe ( oxm_cnxtn );

  return ( OE_OK );
} /* end oxm_endcmd */

ERR_LOG
oxm_close ( OXM_CNXTN oxm_cnxtn )
{

  if ( oxm_cnxtn -> running )
    return ( err_log ( OE_INTERNAL ) );
  /* if */
  if ( oxm_local )
    return ( OE_OK );
  /* if */
  close ( oxm_cnxtn -> out_pipe [1] );
  endcmd ( oxm_cnxtn -> cmd_pid );
  free ( oxm_cnxtn );
  return ( OE_OK );
} /* end oxm_close */
