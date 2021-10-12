/* @(#)48       1.3  src/bldenv/make/nonints.h, bldprocess, bos412, GOLDA411a 1/19/94 16:31:04
 *
 *   COMPONENT_NAME: BLDPROCESS
 *
 *   FUNCTIONS: _args_
 *
 *   ORIGINS: 27,71
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * @OSF_FREE_COPYRIGHT@
 * COPYRIGHT NOTICE
 * Copyright (c) 1992, 1991, 1990  
 * Open Software Foundation, Inc. 
 *  
 * Permission is hereby granted to use, copy, modify and freely distribute 
 * the software in this file and its documentation for any purpose without 
 * fee, provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation.  Further, provided that the name of Open 
 * Software Foundation, Inc. ("OSF") not be used in advertising or 
 * publicity pertaining to distribution of the software without prior 
 * written permission from OSF.  OSF makes no representations about the 
 * suitability of this software for any purpose.  It is provided "as is" 
 * without express or implied warranty. 
 */
/*
 * HISTORY
 * $Log: nonints.h,v $
 * Revision 1.2.4.3  1993/04/30  15:19:57  damon
 * 	CR 464. Oops, really fix return problem
 * 	[1993/04/30  15:19:35  damon]
 *
 * Revision 1.2.4.2  1993/04/26  20:27:28  damon
 * 	CR 424. make now handles error returns better
 * 	[1993/04/26  20:27:00  damon]
 * 
 * Revision 1.2.2.4  1992/12/10  18:15:12  damon
 * 	CR 329. Removed decl of ar_hdr
 * 	[1992/12/10  18:14:42  damon]
 * 
 * Revision 1.2.2.3  1992/12/03  19:07:07  damon
 * 	ODE 2.2 CR 346. Expanded copyright
 * 	[1992/12/03  18:36:22  damon]
 * 
 * Revision 1.2.2.2  1992/09/24  19:26:52  gm
 * 	CR286: Major improvements to make internals.
 * 	[1992/09/24  17:55:12  gm]
 * 
 * Revision 1.2  1991/12/05  20:44:38  devrcs
 * 	Changes for Reno make
 * 	[91/03/22  16:08:07  mckeen]
 * 
 * $EndLog$
 */

#include <stdio.h>

#ifdef __STDC__
#define _args_(args)	args
#else /* __STDC__ */
#define _args_(args)	()
#endif /* __STDC__ */

char **brk_string _args_((char *, int *));
char *emalloc _args_((u_int));
void enomem _args_((void));
char *strndup _args_((const char *, int));
int   Str_Match _args_((const char *, const char *));

struct Path;			/* internal to Dir module */

struct ar_hdr *ArchStatMember(string_t, string_t, Boolean);
FILE *	ArchFindMember(string_t, string_t, struct ar_hdr *, const char *);
void	ArchFixMembName(string_t *);
int	ArchReadHdr(FILE *, void **);
int	ArchReadMember(FILE *, string_t *, struct ar_hdr *, void *);
void	ArchToNextMember(FILE *, struct ar_hdr *, void *);
void	ArchTouchTOC(GNode *);
void	ArchTOCTime(GNode *, Boolean *);
ReturnStatus	Arch_ParseArchive _args_((char **, Lst, GNode *));
void	Arch_Touch _args_((GNode *));
void	Arch_TouchLib _args_((GNode *));
int	Arch_MTime _args_((GNode *));
int	Arch_MemMTime _args_((GNode *));
void	Arch_FindLib _args_((GNode *, Lst));
Boolean	Arch_LibOODate _args_((GNode *));
void	Arch_Init _args_((void));
void	Compat_Run _args_((Lst));
void	Cond_Setup _args_((void));
int	Cond_Eval _args_((char *));
void	Cond_End _args_((void));
void	Cond_Init _args_((char *(*)(const char *, Boolean *, int *),
			  char *(*)(const char *),
			  void (*)(const char *, ...), Boolean));
int	Cond_GetArg _args_((char **, char **, const char *, Boolean));
void	Cond_AddKeyword _args_((const char *,
				Boolean (*)(char **, int *, char **, Boolean),
				Boolean (*)(int, char *)));
void	Dir_Init _args_((void));
void	Dir_ReInit _args_((Lst));
string_t Dir_FindFile _args_((string_t, Lst));
string_t Dir_FindFileOrLink _args_((string_t, Lst, Boolean));
int	Dir_MTime _args_((GNode *));
void	Dir_AddDir _args_((Lst, string_t));
ClientData	Dir_CopyDir _args_((ClientData));
string_t Dir_MakeFlags _args_((string_t, Lst));
void	Dir_Destroy _args_((struct Path *));
void	Dir_ClearPath _args_((Lst));
void	Dir_PrintPath _args_((Lst));
void	Dir_PrintDirectories _args_((void));
void	Dir_Concat _args_((Lst, Lst));
int	Make_TimeStamp _args_((ClientData, ClientData));
Boolean	Make_OODate _args_((GNode *));
int	Make_HandleUse _args_((ClientData, ClientData));
void	Make_Update _args_((GNode *));
void	Make_DoAllVar _args_((GNode *));
Boolean	Make_Run _args_((Lst, Boolean *));
void	Job_Touch _args_((GNode *, Boolean));
Boolean	Job_CheckCommands _args_((GNode *, void (*)(const char *, ...)));
void	Job_CatchChildren _args_((void));
Boolean	Job_Make _args_((GNode *));
void	Job_Init _args_((int, int));
Boolean	Job_Full _args_((void));
Boolean	Job_Empty _args_((void));
int	Job_End _args_((void));
void	Job_Wait _args_((void));
void	Job_AbortAll _args_((void));
void	Main_ParseArgLine _args_((char *));
void	Error _args_((const char *, ...));
void	Fatal _args_((const char *, ...));
void	Punt _args_((const char *, ...));
void	DieHorribly _args_((void));
void	Finish _args_((int));
void	Parse_Error _args_((int, const char *, ...));
int	Parse_DoVar _args_((char *, GNode *));
void	Parse_AddIncludeDir _args_((string_t));
void	Parse_File _args_((string_t, FILE *));
Lst	Parse_MainName _args_((void));
void	Parse_Init _args_((void));
void	ParseErrorCond _args_((const char *, ...));
void	Suff_ClearSuffixes _args_((void));
Boolean	Suff_IsTransform _args_((string_t));
GNode *	Suff_AddTransform _args_((string_t));
void	Suff_AddSuffix _args_((string_t));
int	Suff_EndTransform _args_((ClientData, ClientData));
Lst	Suff_GetPath _args_((string_t));
void	Suff_DoPaths _args_((void));
void	Suff_AddInclude _args_((string_t));
void	Suff_AddLib _args_((string_t));
void	Suff_FindDeps _args_((GNode *));
void	Suff_SetNull _args_((string_t));
void	Suff_Init _args_((void));
void	Suff_PrintAll _args_((void));
void	Targ_Init _args_((void));
GNode *	Targ_NewGN _args_((string_t));
GNode *	Targ_FindNode _args_((string_t, int));
Lst	Targ_FindList _args_((Lst, int));
Boolean	Targ_Ignore _args_((GNode *));
Boolean	Targ_Silent _args_((GNode *));
Boolean	Targ_Precious _args_((GNode *));
void	Targ_SetMain _args_((GNode *));
int	Targ_PrintCmd _args_((ClientData, ClientData));
char *	Targ_FmtTime _args_((time_t));
void	Targ_PrintType _args_((int));
void	Targ_PrintGraph _args_((int));
void	Var_Delete _args_((string_t, GNode *));
void	Var_Set _args_((string_t, string_t, GNode *));
void	Var_Append _args_((string_t, string_t, GNode *));
Boolean	Var_Exists _args_((string_t, GNode *));
const char *Var_Value _args_((string_t, GNode *));
string_t Var_StrValue _args_((string_t, GNode *));
char *	Var_Parse _args_((const char *, GNode *, Boolean, int *, Boolean *));
char *	Var_Skip _args_((char *, GNode *, Boolean));
char *	Var_Subst _args_((const char *, GNode *, Boolean));
void	Var_Init _args_((void));
void	Var_Dump _args_((GNode *));
Boolean	Var_HasMeta _args_((const char *));
char *	VarParseCond _args_((const char *, Boolean *, int *));
char *	VarValueCond _args_((const char *));
