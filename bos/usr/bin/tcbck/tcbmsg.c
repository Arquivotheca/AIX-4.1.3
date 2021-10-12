static char sccsid[] = "@(#)62  1.3.1.6  src/bos/usr/bin/tcbck/tcbmsg.c, cmdsadm, bos411, 9439B411a 9/27/94 14:31:24";
/*
 * COMPONENT_NAME: (CMDSADM) tcbck - trusted computing base checker
 *
 * FUNCTIONS: msg, fmsg, msg1, fmsg1, msg2, fmsg2, fatal, usage, init_msg
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <errno.h>
#include <stdio.h>

#include "tcbmsg.h"

/*
 * Language Support
 */

#include	"tcbck_msg.h"
#include	<nl_types.h>

/*
 * External variables and flags
 */

extern	int	verbose;

extern	char	*Usage[];
extern	char	*Attributes[];

#ifdef	_NO_PROTO
extern	char	*xstrdup();
#else
extern	char	*xstrdup (char *);
#endif

/*
 * The built-in message strings appear below.  These value will be used
 * if the message catalog can't be opened or if some message is missing
 * from the catalog.
 */

/*
 * Error messages for command line arguments
 */

char	*Needs_A_Value =
"3001-003 A value must be specified for %s.\n";

char	*Needs_An_Attribute =
"3001-091 The attribute %s must be specified for\n\
	file %s.\n";

char	*All_or_Tree =
"3001-004 Specify only one of the options ALL or tree.\n";

char	*No_More_Args =
"3001-005 Do not specify filenames and classes\n\
         with the ALL or tree options.\n";

char	*P_N_or_Y =
"3001-006 Specify only one of the options -p, -n, -y or -t.\n";

char	*Duplicate_Stanza =
"3001-007 The entry for %s appears more than once in the database.\n";

char	*Duplicate_Name =
"3001-008 %s appears in the entries for files %s\n\
         and %s.\n";

char	*Duplicate_Object =
"3001-009 The database entries for %s\n\
         and %s refer to the same file.\n";

char	*Linked_Directory =
"3001-010 The entry for the directory %s may not\n\
         specify hard links.\n";

char	*Illegal_Attribute =
"3001-055 The attribute %s is not valid for the file %s.\n";

char	*Invalid_Value =
"3001-077 The attribute %s has a value %s which is not valid.\n";

char	*Invalid_Attribute =
"3001-083 The attribute %s for the file %s\n\
         has a value which is not valid.\n";

/*
 * Error messages for program execution errors
 */

char	*No_Program =
"3001-011 The program %s cannot be executed.\n";

char	*Program_Error =
"3001-012 The program %s encountered an error during execution.\n";

char	*Verify_Failed =
"3001-013 The program %s was not successful\n\
         verifying the file %s.\n";

char	*Illegal_Entry =
"3001-015 The entry for %s is not valid.\n";

char	*Illegal_Type =
"3001-056 The entry for %s has a type %s\n\
         which is not valid.\n";

char	*No_Permission =
"3001-076 Permission was denied for the requested operation.\n";

char	*Update_Failed =
"3001-078 An error was encountered updating the attribute %s.\n";

char	*Database_Error =
"3001-084 An error was encountered reading the database.\n";

char	*Input_File_Error =
"3001-093 An error was encountered reading the input file.\n";

char	*Last_Stanza =
"3001-085 The last valid stanza read was %s.\n";

char	*No_Last_Stanza =
"3001-086 No valid stanzas were read.\n";

char    *Register_Device =
"3001-103 Use the -l option to register devices on the Trusted\n\
\t Computing Base.\n";

char 	*Open_Temp_File_Error =
"3001-104 Error opening temporary file\n";

char 	*Update_Temp_File_Error =
"3001-105 Error updating temporary file\n";

char    *Register_Trusted =
"3001-106 If the unregistered device is trusted, use the -l option\n\
\t to register the device as part of the Trusted Computing Base.\n";

/*
 * Error messages for file related errors
 */

char	*Unknown_Type =
"3001-016 The file %s is of an unknown type.\n";

char	*Unknown_SUID_File =
"3001-017 The file %s is an unregistered set-UID program.\n";

char	*Unknown_SGID_File =
"3001-054 The file %s is an unregistered set-GID program.\n";

char	*Unknown_Device =
"3001-018 The file %s is an unregistered device.\n";

char	*Unknown_TCB_File =
"3001-019 The file %s is an unregistered TCB file.\n";

char	*Unknown_TP_File =
"3001-052 The file %s is an unregistered TP file.\n";

char	*Unknown_Priv_File =
"3001-053 The file %s is an unregistered privileged file.\n";

char	*No_Such_File =
"3001-020 The file %s was not found.\n";

char	*Donot_Know_How =
"3001-021 The file %s does not exist and the entry has\n\
         no source attribute.\n";

char	*Donot_Know_What =
"3001-057 The file %s does not exist or the entry has\n\
         no type attribute.\n";

char	*Wrong_File_Type =
"3001-022 The file %s is the wrong file type.\n";

char	*Wrong_File_Modes =
"3001-023 The file %s has the wrong file mode.\n";

char	*Wrong_File_Owner =
"3001-024 The file %s has the wrong file owner.\n";

char	*Wrong_File_Group =
"3001-025 The file %s has the wrong file group.\n";

char	*Wrong_Link_Count =
"3001-026 The file %s has the wrong number of links.\n";

char	*Wrong_TCB_Flag =
"3001-027 The file %s has the wrong TCB attribute value.\n";

char	*Wrong_TP_Flag =
"3001-051 The file %s has the wrong TP attribute value.\n";

char	*Wrong_Checksum =
"3001-028 The file %s has the wrong checksum value.\n";

char	*Wrong_Size =
"3001-049 The file %s has the wrong size.\n";

char	*Wrong_ACL =
"3001-029 The file %s has the wrong access control list.\n";

char	*Wrong_PCL =
"3001-030 The file %s has the wrong privilege control list.\n";

char	*No_Such_Link =
"3001-031 The link from the file %s\n\
         to %s does not exist.\n";

char	*No_Such_Symlink =
"3001-087 The symbolic link from the file %s\n\
         to %s does not exist.\n";

char	*No_Such_Source =
"3001-080 The file %s has a source file\n\
         %s that does not exist.\n";

char	*Illegal_Link =
"3001-032 The link from the file %s\n\
         to %s should not exist.\n";

char	*Illegal_Symlink =
"3001-089 The symbolic link from the file %s\n\
         to %s should not exist.\n";

char	*Incorrect_Link =
"3001-033 The file %s is not a link\n\
         to %s.\n";

char	*Incorrect_Symlink =
"3001-088 The file %s is not a symbolic link\n\
         to %s.\n";

char	*Absolute_File =
"3001-035 The file %s must be an absolute path name.\n";

char	*Absolute_Link =
"3001-036 The link %s must be an absolute path name.\n";

char	*Absolute_Program =
"3001-037 The program %s must be an absolute path name.\n";

char	*Absolute_Source =
"3001-079 The file %s must have an absolute path name\n\
         for the source attribute %s.\n";

char	*Copy_Failed =
"3001-059 The copy from file %s\n\
         to %s was not successful.\n";

char	*Create_Failed =
"3001-092 The creation of file %s was not successful.\n";

char	*Chmod_Failed =
"3001-060 Setting permissions on the file %s\n\
         was not successful.\n";

/*
 * Other errors
 */

char	*Not_Trusted_Machine =
    "3001-101 The Trusted Computing Base is not enabled on this machine.\n"
	"\tTo enable the Trusted Computing Base, you must reinstall and\n"
	"\tset the 'Install Trusted Computing Base' option to YES.\n"
	"\tNo checking is being performed.\n";

char	*Corrupted_Machine =
	"3001-102 The %s attribute cannot be found in the data base\n"
	"\tor the attribute has an invalid value. To correct this problem,\n"
	"\tyou must reinstall the system. No checking is being performed.\n";

char	*Unknown_User =
"3001-038 The name %s is not a known user.\n";

char	*Unknown_Group =
"3001-039 The name %s is not a known group.\n";

char	*Unknown_Mode =
"3001-040 The file %s has a file mode or flag %s\n\
         which is not valid.\n";

char	*Too_Many_Links =
"3001-041 The file %s has too many links.\n";

char	*Out_Of_Memory =
"3001-042 A request for additional memory failed.\n";

char	*Use_Tree_Option =
"3001-043 Use the tree option to find extra links.\n";

char	*Del_Failed =
"3001-046 An error occurred removing the entry for %s.\n";

char	*No_File_Matched =
"3001-047 There is no matching file entry for %s.\n";

char	*No_Class_Matched =
"3001-048 There is no matching class entry for %s.\n";

/*
 * Messages produced during interactive mode
 *
 * These messages are questions which are being prompted for replies
 * to.  The cursor should remain on the same line as the question,
 * therefore you should not add "\n" [ newline ] characters to the
 * ends of these strings.
 */

char	*Disable_ACL =
"3001-062 Clear the access control list for %s? ";

char	*Disable_PCL =
"3001-063 Clear the privilege control list for %s? ";

char	*Disable_Mode =
"3001-064 Clear the illegal file mode for %s? ";

char	*Create_Link =
"3001-066 Create the link for %s? ";

char	*Create_File =
"3001-067 Create the file %s from the source location? ";

char	*Remove_File =
"3001-069 Remove the file %s? ";

char	*Remove_Device =
"3001-070 Remove the special file %s? ";

char	*Correct_ACL =
"3001-071 Change the access control list for %s? ";

char	*Correct_PCL =
"3001-072 Change the privilege control list for %s? ";

char	*Correct_Owner =
"3001-073 Change the file owner for %s? ";

char	*Correct_Group =
"3001-074 Change the file group for %s? ";

char	*Correct_Modes =
"3001-075 Change the file modes for %s? ";

char	*New_Entry =
"3001-094 Add the new entry for file %s? ";

char 	*New_Link =
"3001-095 Add the new link for %s? ";

char	*New_Symlink =
"3001-096 Add the new symbolic link for %s? ";

char	*New_Modes =
"3001-097 Add the new modes for %s? ";

/*
 * Program fatal errors.  Impossible conditions that occur should
 * result in this error message.
 */

char	*Open_An_Apar =
"3001-090 Depending upon where this product was acquired,\n\
         contact a service representative or\n\
         an approved supplier.\n";


/*
 * The message table is an array of pointers to character pointers.
 * This permits the init_msg procedure to change the contents of the
 * built-in messages by changing the values of those pointers.
 *
 * Deleted messages cannot be removed from this array unless the
 * messages are also deleted from the message catalog.
 */

#define DELETED ((char **)-1)
char	**Message_List[] = {
	NULL,			/* placeholder so that msgs start at 1 */
	DELETED,		/* 1 */
	&Needs_A_Value,
	&All_or_Tree,
	&No_More_Args,
	&P_N_or_Y,
	&Duplicate_Stanza,
	&Duplicate_Name,
	&Duplicate_Object,
	&Linked_Directory,
	&No_Program,		/* 10 */
	&Program_Error,
	&Verify_Failed,
	DELETED,
	&Illegal_Entry,
	&Unknown_Type,
	&Unknown_SUID_File,
	&Unknown_Device,
	&Unknown_TCB_File,
	&No_Such_File,
	&Donot_Know_How,	/* 20 */
	&Wrong_File_Type,
	&Wrong_File_Modes,
	&Wrong_File_Owner,
	&Wrong_File_Group,
	&Wrong_Link_Count,
	&Wrong_TCB_Flag,
	&Wrong_Checksum,
	&Wrong_ACL,
	&Wrong_PCL,
	&No_Such_Link,		/* 30 */
	&Illegal_Link,
	&Incorrect_Link,
	DELETED,
	&Absolute_File,
	&Absolute_Link,
	&Absolute_Program,
	&Unknown_User,
	&Unknown_Group,
	&Unknown_Mode,
	&Too_Many_Links,	/* 40 */
	&Out_Of_Memory,
	&Use_Tree_Option,
	DELETED,
	DELETED,
	&Del_Failed,
	&No_File_Matched,
	&No_Class_Matched,
	&Wrong_Size,
	DELETED,
	&Wrong_TP_Flag,		/* 50 */
	&Unknown_TP_File,
	&Unknown_Priv_File,
	&Unknown_SGID_File,
	&Illegal_Attribute,
	&Illegal_Type,
	&Donot_Know_What,
	DELETED,
	&Copy_Failed,
	&Chmod_Failed,
	DELETED,		/* 60 */
	&Disable_ACL,
	&Disable_PCL,
	&Disable_Mode,
	DELETED,
	&Create_Link,
	&Create_File,
	DELETED,
	&Remove_File,
	&Remove_Device,
	&Correct_ACL,		/* 70 */
	&Correct_PCL,
	&Correct_Owner,
	&Correct_Group,
	&Correct_Modes,
	&No_Permission,
	&Invalid_Value,
	&Update_Failed,
	&Absolute_Source,
	&No_Such_Source,
	DELETED,		/* 80 */
	DELETED,
	&Invalid_Attribute,
	&Database_Error,
	&Last_Stanza,
	&No_Last_Stanza,
	&No_Such_Symlink,
	&Incorrect_Symlink,
	&Illegal_Symlink,
	&Open_An_Apar,
	&Needs_An_Attribute,	/* 90 */
	&Create_Failed,
	&Input_File_Error,
	&New_Entry,
	&New_Link,
	&New_Symlink,
	&New_Modes,
	DELETED,
	DELETED,
	DELETED,
	DELETED,		/*  100 */
	&Not_Trusted_Machine,
	&Corrupted_Machine,
	&Register_Device,
	&Open_Temp_File_Error,
	&Update_Temp_File_Error,
	&Register_Trusted,
	NULL
};

/*
 * NAME: msg, fmsg
 *                                                                    
 * FUNCTION: Print message text
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process.  Called to handle error processing.
 *                                                                   
 * NOTES:
 *	Error message processing is enabled with the verbose flag.
 *	No error messages will be printed by this routine when that
 *	flag is false.  fmsg() ignores this flag and always prints
 *	messages.
 *
 * RETURNS: NONE
 */  

void
#ifdef	_NO_PROTO
msg (buf)
char	*buf;		/* Formatted message to display                     */
#else
msg (char *buf)
#endif
{
	if (! verbose)
		return;

	fputs (buf, stderr);
	fflush (stderr);
}

void
#ifdef	_NO_PROTO
fmsg (buf)
char	*buf;		/* Formatted message to display                     */
#else
fmsg (char *buf)
#endif
{
	fputs (buf, stderr);
	fflush (stderr);
}

/*
 * NAME: msg1, fmsg1
 *
 * FUNCTION: Print message text for messages with a single argument
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process.  Called to handle error processing.
 *                                                                   
 * NOTES:
 *	Calls msg or fmsg with the formatted message.
 *
 * RETURNS: NONE
 */  

void
#ifdef	_NO_PROTO
msg1 (message, arg)
char	*message,	/* Message string to add argument to                */
	*arg;		/* Argument to add to message string                */
#else
msg1 (char *message, char *arg)
#endif
{
	char	buf[BUFSIZ];	/* Buffer to format message into            */

	sprintf (buf, message, arg);
	msg (buf);
}

void
#ifdef	_NO_PROTO
fmsg1 (message, arg)
char	*message,	/* Message string to add argument to                */
	*arg;		/* Argument to add to message string                */
#else
fmsg1 (char *message, char *arg)
#endif
{
	char	buf[BUFSIZ];	/* Buffer to format message into            */

	sprintf (buf, message, arg);
	fmsg (buf);
}

/*
 * NAME: msg2, fmsg2
 *                                                                    
 * FUNCTION: Print message text for messages with two arguments
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process.  Called to handle error processing.
 *
 * NOTES:
 *	Calls msg with the formatted message.
 *                                                                   
 * RETURNS: NONE
 */  

void
#ifdef	_NO_PROTO
msg2 (message, arg1, arg2)
char	*message,	/* Message string to add arguments to               */
	*arg1,		/* First message argument                           */
	*arg2;		/* Second message argument                          */
#else
msg2 (char *message, char *arg1, char *arg2)
#endif
{
	char	buf[BUFSIZ];	/* Buffer for formatting message into       */

	sprintf (buf, message, arg1, arg2);
	msg (buf);
}

void
#ifdef	_NO_PROTO
fmsg2 (message, arg1, arg2)
char	*message,	/* Message string to add arguments to               */
	*arg1,		/* First message argument                           */
	*arg2;		/* Second message argument                          */
#else
fmsg2 (char *message, char *arg1, char *arg2)
#endif
{
	char	buf[BUFSIZ];	/* Buffer for formatting message into       */

	sprintf (buf, message, arg1, arg2);
	fmsg (buf);
}

/*
 * NAME: msg3, fmsg3
 *                                                                    
 * FUNCTION: Print message text for messages with three arguments
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 *	User process.  Called to handle error processing.
 *
 * NOTES:
 *	Calls msg with the formatted message.
 *                                                                   
 * RETURNS: NONE
 */  

void
#ifdef	_NO_PROTO
msg3 (message, arg1, arg2, arg3)
char	*message,	/* Message string to add arguments to               */
	*arg1,		/* First message argument                           */
	*arg2,		/* Second message argument                          */
	*arg3;		/* Third message argument                           */
#else
msg3 (char *message, char *arg1, char *arg2, char *arg3)
#endif
{
	char	buf[BUFSIZ];	/* Buffer for formatting message into       */

	sprintf (buf, message, arg1, arg2, arg3);
	msg (buf);
}

void
#ifdef	_NO_PROTO
fmsg3 (message, arg1, arg2, arg3)
char	*message,	/* Message string to add arguments to               */
	*arg1,		/* First message argument                           */
	*arg2,		/* Second message argument                          */
	*arg3;		/* Third message argument                           */
#else
fmsg3 (char *message, char *arg1, char *arg2, char *arg3)
#endif
{
	char	buf[BUFSIZ];	/* Buffer for formatting message into       */

	sprintf (buf, message, arg1, arg2, arg3);
	fmsg (buf);
}

/*
 * NAME:	usage
 *
 * FUNCTION:	Print multi-line usage message
 *
 * NOTES:
 *	This procedure calls exit().
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURNS: NONE
 *	This routine never returns.
 */

void
#ifdef	_NO_PROTO
usage (msg)
char	**msg;
#else
usage (char **msg)
#endif
{
	while (*msg)
		fmsg1 ("%s\n", *msg++);

	exit (EINVAL);
}

/*
 * NAME:	fatal
 *
 * FUNCTION:	Print fatal error message
 *
 * NOTES:
 *	This procedure calls exit()
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * RETURNS: NONE
 */

void
#ifdef	_NO_PROTO
fatal (format, arg, error)
char	*format;
char	*arg;
int	error;
#else
fatal (char *format, char *arg, int error)
#endif
{
	if (arg)
		fmsg1 (format, arg);
	else
		fmsg (format);

	exit (error);
}

nl_catd	catd;

/*
 * NAME: init_msg
 *
 * FUNCTION: Initialize message table from NLS message catalog
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 *	User process.  Called once at initialization time.
 *                                                                   
 * NOTES:
 *	Message catalogs are optional.  The internal catalog is changed
 *	only if the optional catalog can not be found.
 *
 * RETURNS: NONE
 */  

void
init_msg ()
{
	int	i;
	char	*cp;

	/*
	 * Open the catalog for sysck.
	 */

	if ((catd = catopen (MF_TCBCK, NL_CAT_LOCALE)) == (nl_catd) -1)
		return;

	/*
	 * Loop through the list of messages and read each one in.  If
	 * the message is missing, skip it and use the hardcoded version.
	 * If a message is returned, allocate space for it and save the
	 * pointer.
	 */

	for (i = 1;Message_List[i];i++) {
		if (Message_List[i] != DELETED
		    && (cp = catgets (catd, MS_TCBCK, i, *Message_List[i]))
		    && *Message_List[i] != cp)
			*Message_List[i] = xstrdup (cp);
	}

	/*
	 * Do the same as above for the USAGE message strings.
	 */

	for (i = 1;Usage[i];i++) {
		if (! (cp = catgets (catd, MS_TCBCK_USE, i, Usage[i])))
			continue;

		if (Usage[i] == cp)
			continue;

		Usage[i] = xstrdup (cp);
	}

	/*
	 * Do the same as above for the ATTRIBUTE message strings.
	 */

	for (i = 1;Attributes[i];i++) {
		if (! (cp = catgets (catd, MS_TCBCK_ATTR, i, Attributes[i])))
			continue;

		if (Attributes[i] == cp)
			continue;

		Attributes[i] = xstrdup (cp);
	}

	/*
	 * Close the catalog and return
	 */

	catclose (catd);
}
