static char sccsid[] = "@(#)11	1.7  src/bldenv/pkgtools/adeinvmsg.c, pkgtools, bos412, GOLDA411a 8/22/94 15:36:17";
/*
 *   COMPONENT_NAME: pkgtools
 *
 *   FUNCTIONS: none
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
extern char *commandName;

char	*Usage =
"USAGE:\n\t%s -i <inslistFile> -u <lpPathName> -s <shipPathList>\n\
		-l<lppname> [ -t <idTableFile> ] [ -r ] [ -D ] [ -L ]\n\
		[ -U <VRMFlevel>] [ -a acfPathName] [ -d ] [ -v ]\n\
	where the -i, -u, -s, -l flags are required\n\
	and the -r and -D flags are mutually exclusive.\n";

/*
 *-------------------------------------------------------------
 * Array of verbose messages
 *-------------------------------------------------------------
 */
char    *verbose_msgs[] = {
                                                /* CantOpenTableFile	*/
"Cannot open id table file\n\
	 \"%s\". System id tables will be used.\n",
                                                /* FileNotFound         */
"Cannot find file \"%s\"\n\
         in ship trees.\n",
                                                /* CantResolveUid 	*/
"user id '%d' from the inslist file \"%s\"\n\
	 could not be resolved....\n\
	 Continuing to process.\n",
                                                /* CantResolveGid 	*/
"group id '%d' from the inslist file \"%s\"\n\
	 could not be resolved....\n\
	 Continuing to process.\n",
                                                /* CantGetOption 	*/
"Could not get the lpp option name from\n\
	 \"%s\" file.\n",
                                                /* MultipleUid 		*/
"user id '%d' found in inslist file \"%s\"\n\
         is defined multiple times in the table file\n\
         \"%s\" specified with the -t option.\n",
                                                /* MultipleGid 		*/
"group id '%d' found in inslist file \"%s\"\n\
         is defined multiple times in the table file\n\
         \"%s\" specified with the -t option.\n",
                                                /* InvalidTableEnt	*/
"The following entry in the table file \"%s\"\n\
         (specified with the -t option) is invalid and will be ignored:\n\
	 %s.\n",
                                                /* StatFailed		*/
"stat failed on file \"%s\"\n\terrno = %d",
                                                /* InvalidSizeInsize	*/
"Invalid size field found in insize file \"%s\" on line:\n\
	 %s.\n",
                                                /* NoValueForOperator	*/
"Size operator is required to have a value.  The following line\n\
	 in the insize file \"%s\" will be ignored:\n\
	 %s.\n",
                                                /* ACFloadError       	*/
"Could not load the given acf file (%s)\n\
         into the acf list.\n",
                                                /* lppacfError       	*/
"Could not create the 'lpp.acf' file.\n" ,
                                                /* LibNotFound		*/
"Could not size of library '%s' in which\n\
         member '%s' is to be changed.\n\
	 Assuming a lib size of 0; pkg size requirements may be invalid.\n",
                                                /* acfAddFailure	*/
"Could not create an acf entry for;\n\
         member  = %s\n\
         library = %s\n"
} ;

/*
 *------------------------------------------------------------------
 * The following is the array containing the terse messages.
 * This array would become the default message array for adeinv
 * if we move to using the messaging facilities.
 *------------------------------------------------------------------
 */
char    *default_msgs[] = {
                                                /* CantOpenTableFile	*/
"Cannot open id table file \"%s\".\n",
                                                /* FileNotFound         */
"Can't find file \"%s\" in ship trees.\n",
                                                /* CantResolveUid 	*/
"Can't resolve uid '%d' from \"%s\".\n",
                                                /* CantResolveGid 	*/
"Can't resolve gid '%d' from \"%s\".\n",
                                                /* CantGetOption 	*/
"Could not get the lpp option name from \"%s\" file.\n",
                                                /* MultipleUid 		*/
"Multiple definitions of uid '%d' from \"%s\" in \"%s\".\n",
                                                /* MultipleGid 		*/
"Multiple definitions of gid '%d' from \"%s\" in \"%s\".\n",
                                                /* InvalidTableEnt	*/
"Table file \"%s\" entry invalid => %s.\n",
                                                /* UidNotInTable	*/
"UID '%d' (from \"%s\") not in \"%s\".\n",
                                                /* GidNotInTable	*/
"GID '%d' (from \"%s\") not in \"%s\".\n",
                                                /* StatFailed		*/
"stat failed on file \"%s\" (errno = %d).\n",
                                                /* InvalidSizeInsize	*/
"Invalid size field in \"%s\" on line: %s.\n",
                                                /* NoValueForOperator	*/
"Missing Size operator value in \"%s\" on line: %s.\n",
                                                /* ACFloadError       	*/
"Could not load the given acf file (%s) into the acf list.\n",
                                                /* lppacfError       	*/
"Could not create the 'lpp.acf' file.\n" ,
                                                /* LibNotFound		*/
"Could not get size of library '%s' for member '%s' change.\n" ,
                                                /* acfAddFailure	*/
"Could not create an acf entry for; member = '%s', lib = '%s'.\n"
} ;
