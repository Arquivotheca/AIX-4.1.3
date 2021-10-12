static char sccsid[] = "@(#)79	1.2  src/bos/usr/bin/bterm/debug.c, libbidi, bos411, 9428A410j 10/5/93 17:59:39";
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: debug_print
 *		debug_setlocation
 *		show
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <pwd.h>
#include <sys/param.h>
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>

#include "trace.h"

int	debug=0;

static char  *debug_file = "unknown file";
static int  debug_line = -1;

debug_setlocation(file, line)
    char  *file;
    int  line;
{
    debug_file = file;
    debug_line = line;
}

debug_print( f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9) /* limit of ten args */
    char *f;
    char *s0, *s1, *s2, *s3, *s4, *s5, *s6, *s7, *s8, *s9;
{
    static char  filename[128];
    static char  buff[256];
    int  pid = 0;
    int  fd = -1;
    static volatile  hits = 0;
    static volatile  busy = 0;
    
    if( busy < 0 )
	busy = 0;

    if( busy ){
	hits++;
	return;
    }

    busy++;

    if( busy > 1 ){
	hits++;
	busy--;
	return;
    }

    pid = getpid();

    sprintf( filename, "trace");
    fd = open( filename, O_WRONLY|O_APPEND|O_CREAT, 0666);
    if( fd < 0 )
	return;
     buff[0] = '\0';
/*
    if( hits ){
	sprintf(buff, "[%d HITS]\n", hits);
	hits = 0;
	write( fd, buff, strlen(buff));
    }

    if( debug_line < 0 ){
	buff[0] = '\0';
    }
    else{
	sprintf(buff, "	[ %d:[ \"%s\":%d ] ] ", pid, debug_file, debug_line);
	debug_line = -1;
    }
*/
    sprintf(/*&(buff[strlen(buff)])*/buff, f, s0, s1, s2, s3, s4, s5, s6, s7, s8, s9);

    write( fd, buff, strlen(buff));

    close( fd);

    show(buff);	/* put it on /dev/console */


    /*
    sync();
    */
    busy--;
}

extern int errno;

show(s)
    char  *s;
{
    int  showfd;
    static char  buffer[256];
    int  waserrno;

    waserrno=errno;

    if( (showfd = open("/dev/console", O_RDWR, 0777)) < 0 ){
   return;
    }
    sprintf(buffer,"%s", s);
    write(showfd, buffer, strlen(buffer));
    close(showfd);
}

