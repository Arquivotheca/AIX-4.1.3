static char sccsid[] = "@(#)09  1.1  src/bldenv/pkgtools/addvpdmsg.c, pkgtools, bos412, GOLDA411a 2/12/93 12:20:39";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

char	*Missing_Opt =
"\nThe -d, -f and -l options are required.\n\n";

char	*File_Open_Failed =
"\tCould not open stanza file %s for read access\n\
	errno = %d\n";

char	*Stanza_Overflow =
"\tThe following stanza has exceeded the maximum character\n\
	limit of %s:\n\n%s\n";

char	*Usage =
"USAGE:  \n\t%s -d <objDir> -f <stanzaFile> -l <lppname>\n";

char	*VPD_Open_Failed =
"\tCould not open VPD.\n";

char	*No_LPP_Id =
"\tCould not get lpp id for lpp %s from VPD.\n";

char	*No_Size =
"\tstrtol failed on size field with errno=%d for the\n\
	following stanza:\n\n%s";

char	*Link_Overflow =
"\tThe link attribute for the following stanza exceeds the\n\
	maximum size of %s:\n\n%s\n\n\
	The attribute value will be truncated.\n";

char	*SymLink_Overflow =
"\tThe symlink attribute for the following stanza exceeds the\n\
	maximum size of %s:\n\n%s\n\n\
	The attribute value will be truncated.\n";

char	*No_Checksum =
"\tatoi failed on checksum field with errno=%d for the\n\
	following stanza:\n\n%s\n";

char	*VPD_Add_Failed =
"\tvpdadd failed for entry %s.\n";

char	*Entry_Exists =
"\tAn entry already exists for file %s.\n\
	The entry will not be added to the vpd.\n";

char	*Malloc_Error =
"\tmalloc failed, errno=%d.\n";

char	*No_Class =
"\tThe following stanza does not contain a class attribute:\n\n%s\n";

char	*Class_Overflow =
"\tThe class attribute for the following stanza exceeds the\n\
	maximum length of %s:\n\n%s\n";
