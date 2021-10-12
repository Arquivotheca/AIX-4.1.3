/* @(#)23	1.4  src/bldenv/pkgtools/ptfinsmsg.c, pkgtools, bos412, GOLDA411a  9/6/94  17:07:58 */
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

char    *Missing_Opt =
"\nThe -f, -o and -i options are required.\n\n";

char	*File_Open_Failed =
"\tCould not open file %s for read access\n\
	errno = %d\n";

char	*No_Env_Var_TOP =
"\tCould not get environment variable TOP. Set the env variable first.\n";

char	*Usage =
"USAGE:  \n\t%s -f <filenameList file> -o <lpp_option> -i <inslist file>\n";

char	*noMatch =
"\tCould not find a match for %s in the inslist file.\n\
\tContinuing processing.\n";
