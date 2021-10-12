/* @(#)67       1.1.1.5  src/bos/usr/bin/tcbck/tcbmsg.h, cmdsadm, bos411, 9439B411a 9/27/94 14:31:26 */
/*
 *
 * COMPONENT_NAME: (CMDSADM) sysck - system configuration checker
 *
 * FUNCTIONS: NONE
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

/* 
 * This file contains the external declarations for all of the
 * message strings used by sysck.  It should be included in any
 * file which references messages.
 */

extern	char	*Needs_A_Value;
extern	char	*Needs_An_Attribute;
extern	char	*Mode_or_Acl;
extern	char	*All_or_Tree;
extern	char	*No_More_Args;
extern	char	*P_N_or_Y;
extern	char	*Duplicate_Name;
extern	char	*Duplicate_Object;
extern	char	*Duplicate_Stanza;
extern	char	*Linked_Directory;
extern	char	*Illegal_Attribute;
extern	char	*Invalid_Value;
extern	char	*Invalid_Attribute;

extern	char	*No_Program;
extern	char	*Program_Error;
extern	char	*Verify_Failed;
extern	char	*Illegal_Entry;
extern	char	*Illegal_Type;
extern	char	*No_Permission;
extern	char	*Update_Failed;
extern	char	*Database_Error;
extern	char	*Input_File_Error;
extern	char	*Last_Stanza;
extern	char	*No_Last_Stanza;

extern	char	*Unknown_Type;
extern	char	*Unknown_SUID_File;
extern	char	*Unknown_SGID_File;
extern	char	*Unknown_Device;
extern	char	*Unknown_TCB_File;
extern	char	*Unknown_TP_File;
extern	char	*Unknown_Priv_File;
extern	char	*No_Such_File;
extern	char	*Donot_Know_How;
extern	char	*Donot_Know_What;
extern	char	*Wrong_File_Type;
extern	char	*Wrong_File_Modes;
extern	char	*Wrong_File_Owner;
extern	char	*Wrong_File_Group;
extern	char	*Wrong_Link_Count;
extern	char	*Wrong_TCB_Flag;
extern	char	*Wrong_TP_Flag;
extern	char	*Wrong_Checksum;
extern	char	*Wrong_Size;
extern	char	*Wrong_ACL;
extern	char	*Wrong_PCL;
extern	char	*No_Such_Link;
extern	char	*No_Such_Symlink;
extern	char	*No_Such_Source;
extern	char	*Incorrect_Link;
extern	char	*Incorrect_Symlink;
extern	char	*Illegal_Link;
extern	char	*Illegal_Symlink;
extern	char	*Cannot_Mkdir;
extern	char	*Cannot_Unlink;
extern	char	*Cannot_Rmdir;
extern	char	*Cannot_Mknod;
extern	char	*File_Not_In_TCB;
extern	char	*Absolute_File;
extern	char	*Absolute_Link;
extern	char	*Absolute_Program;
extern	char	*Absolute_Source;
extern	char	*Copy_Failed;
extern	char	*Create_Failed;

extern	char	*Chmod_Failed;

extern	char	*Unknown_User;
extern	char	*Unknown_Group;
extern	char	*Unknown_Mode;
extern	char	*Too_Many_Links;
extern	char	*Out_Of_Memory;
extern	char	*Use_Tree_Option;
extern	char	*Del_Failed;
extern	char	*No_File_Matched;
extern	char	*No_Class_Matched;

extern	char	*Disable_ACL;
extern	char	*Disable_PCL;
extern	char	*Disable_Mode;
extern	char	*Create_Link;
extern	char	*Create_File;
extern	char	*Remove_File;
extern	char	*Remove_Device;
extern	char	*Correct_ACL;
extern	char	*Correct_PCL;
extern	char	*Correct_Owner;
extern	char	*Correct_Group;
extern	char	*Correct_Modes;
extern	char	*New_Entry;
extern	char 	*New_Link;
extern	char	*New_Symlink;
extern	char	*New_Modes;

extern	char	*Open_An_Apar;
extern	char	*Not_Trusted_Machine;
extern	char	*Corrupted_Machine;
extern  char	*Register_Device;
extern  char	*Open_Temp_File_Error;
extern  char	*Update_Temp_File_Error;
extern  char	*Register_Trusted;
