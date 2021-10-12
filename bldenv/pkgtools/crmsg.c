static char sccsid[] = "@(#)54  1.3  src/bldenv/pkgtools/crmsg.c, pkgtools, bos412, GOLDA411a 3/16/94 14:44:22";
/*
 *   COMPONENT_NAME: PKGTOOLS
 *
 *   FUNCTIONS: none
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

char *commandName = "adecopyright";

char *Usage =
"Usage:\t%s -f copyright_master_keyword_file [-c] -l LPP name -t compids_table_file File...\n";

char *noIDinTable =
"\tThe component ID %s specified on the command line\n\
        was not found in the compids.table.\n";

char *keyWriteErr =
"\tError writing key to file %s.\n";

char *keyPatternErr =
"\tError in key pattern text in file %s.\n\
\t%s\n\
\tKey pattern name must begin with %s.\n";

char *crReadErr =
"\tError reading cr file %s.\n";

char *noKeyCrFile =
"\tError in .cr file.\n\
\tKey %s not found in %s file.\n";

char *quoteError1 =
"\tError in substitution text in %s file.\n\
\t%s\n\
\tSubstitution text must be double-quoted at each end.\n";

char *quoteError2 =
"\tERROR: Error in substitution text in %s file.\n\
\tCheck the line containing the key %s.\n\
\tSubstitution text must be double-quoted at each end.\n";
