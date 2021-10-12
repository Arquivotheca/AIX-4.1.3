/* @(#)09	1.1  src/bldenv/pkgtools/mkodmextractmsg.c, pkgtools, bos412, GOLDA411a  6/30/93  11:52:44 */
/*
 *   COMPONENT_NAME: PKGTOOLS
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

char    *Missing_Opt =
"\nThe -d and -f options are required.\n\n";

char	*File_Open_Failed =
"\tCould not open file %s for read access\n\
	errno = %d\n";

char	*Usage =
"USAGE:  \n\t%s -d <objclassdb> -f <stanzaFile> \n";

char	*Noupdate_Object_Class =
"\n\t%s DO NOT UPDATE THE OBJECT CLASS %s \n\
         errno = %d\n";

char	*Unrecognized_Object_Class =
"\n\t%s  UNRECOGNIZED OBJECT CLASS: %s in stanza \n %s \n\
         errno = %d\n";

char	*Invalid_Field =
"\n\t%s  INVALID FIELD : %s in stanza \n %s \n\
         errno = %d\n";

char	*Descriptor_Not_Found =
"\n\t%s Could not find descriptor in stanza \n %s \n";

char	*Keyname_Not_Found =
"\n\t%s Could not find key in stanza \n %s \n";
