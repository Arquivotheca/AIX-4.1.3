static char sccsid[] = "@(#)95	1.7  src/bos/usr/ccs/lib/libc/basename.c, libcadm, bos411, 9428A410j 3/4/94 10:20:33";
/*
 *   COMPONENT_NAME: libcadm
 *
 *   FUNCTIONS: basename
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
#include	<stdio.h>
#include	<libgen.h>
char *basename(char *path)
{
	static char *return_dot = ".";
	char *end;

	if (!path || !*path)
		return return_dot;

	/**********
	  find the last '/', If there are none, then
	  the path is the basename
	**********/
	if ((end = strrchr(path, '/')) == NULL)
		return(path);

	/*********
	  if there is anything after end, it is the basename
	**********/
	if (end[1])
		return(end+1);

	/********
	  find the first non-/
	*********/
	while (end != path && *end == '/')
		end--;

	/*********
	  set end[1] to null.  If end == path, return
	*********/
	end[1]='\0';
	if (end == path)
		return(path);

	/********
	  find the last / again
	  if one is not found, then path is the basename
	*********/
	end = strrchr(path, '/');
	if (!end)
		return(path);
	else
		return(end+1);
}
