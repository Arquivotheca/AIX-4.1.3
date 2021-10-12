static char sccsid[] = "@(#)78    1.7  src/bos/usr/ccs/bin/dbx/dpi_report.c, cmddbx, bos411, 9428A410j 6/15/90 20:38:04";
/*
 * COMPONENT_NAME: (CMDDBX) - dbx symbolic debugger
 *
 * FUNCTIONS: dpi_report_output, dpi_report_error,
 *	      dpi_report_executing, dpi_report_shell,
 *	      dpi_report_trace, dpi_report_multiprocess, 
 *	      dpi_ctx_level, xde_open_windows
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <stdio.h>

/*
 * Standard error handling routines.

dpi_report_output( buffer )
char *buffer;
{
    printf("%s", buffer);
    return;
}

dpi_report_error( buffer )
char *buffer;
{
    fprintf(stderr, "%s", buffer);
    return;
}
*/

dpi_report_executing( )
{
    return;
}

dpi_report_shell( )
{
    return;
}

dpi_report_trace( dpi_info )
void *dpi_info;
{
    return;
}

dpi_report_multiprocess( pid )
int pid;
{
    return;
}

dpi_ctx_level( level )
int level;
{
    return;
}

xde_open_windows( )
{
    return;
}
