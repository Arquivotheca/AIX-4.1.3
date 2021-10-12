/* @(#)20	1.4  src/bos/usr/ccs/bin/make/nonints.h, cmdmake, bos412, 9445A165578  10/25/94  10:21:37 */
/*
 *   COMPONENT_NAME: CMDMAKE      System Build Facilities (make)
 *
 *   FUNCTIONS: Job_CheckCommands
 *		
 *
 *   ORIGINS: 27,85
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * @(#)$RCSfile: nonints.h,v $ $Revision: 1.2.2.2 $ (OSF) $Date: 1991/11/14 10:35:18 $
 */

#include <stdio.h>

extern ReturnStatus	Arch_ParseArchive (const char **, const Lst, const GNode *);
extern void	Arch_Touch (GNode *);
extern int	Arch_MTime (GNode *);
extern int	Arch_MemMTime (GNode *);
extern void	Arch_Init (void);
extern void	Compat_Run(Lst);
extern void	Dir_Init (void);
extern char *	Dir_FindFile (const char *);
extern int	Dir_MTime (GNode *);
extern void	Dir_PrintDirectories(void);
extern void	Job_Touch (GNode *, Boolean);
extern Boolean	Job_CheckCommands (GNode *, void (*)(const char *, ...));
extern void	Error (const char *, ...);
extern void	Fatal (const char *, ...);
extern void	Punt (const char *, ...);
extern void	*enomem(void);
extern int	Make_TimeStamp (GNode *, GNode *);
extern Boolean	Make_OODate (GNode *);
extern int	Make_HandleTransform (const GNode *, GNode *);
extern void	Make_DoAllVar (GNode *);
extern Boolean	Parse_IsVar (const char *);
extern void	Parse_DoVar (const char *, const GNode *);
extern void	Parse_File(const char *, const FILE *, const GNode
		  *ctxt);
extern void	Parse_Init(void);
extern Lst	Parse_MainName(void);
extern char *	Str_Concat (const char *, const char *, const int);
extern char ** Str_Break(const char *, const char *, int *);
extern char *	Str_FindSubstring(char *, char *);
extern void	Suff_ClearSuffixes (void);
extern Boolean	Suff_IsTransform (char *);
extern GNode *	Suff_AddTransform (char *);
extern int	Suff_EndTransform (GNode *);
extern void	Suff_AddSuffix (char *);
extern void	Suff_FindDeps (GNode *);
extern void	Suff_Init (void);
extern void	Suff_PrintAll(void);
extern void	Targ_Init (void);
extern GNode *	Targ_NewGN (const char *);
extern GNode *	Targ_FindNode (const char *, const int);
extern Lst	Targ_FindList (Lst, int);
extern Boolean	Targ_Ignore (GNode *);
extern Boolean	Targ_Silent (GNode *);
extern Boolean	Targ_Precious (GNode *);
extern void	Targ_SetMain (GNode *);
extern int	Targ_PrintCmd (char *);
extern char *	Targ_FmtTime (const time_t);
extern void	Targ_PrintType (const int);
extern void	Targ_PrintGraph(int);
extern void	Var_Set (const char *, const char *, const GNode *);
extern void	Var_Append (const char *, const char *, const GNode *);
extern Boolean	Var_Exists(const char *, const GNode *);
extern char *	Var_Value (const char *, const GNode *);
extern char *	Var_Parse (char *, const GNode *, const Boolean, int *, Boolean *);
extern char *	Var_Subst (const char *, const GNode *, const Boolean);
extern void	Var_Init (void);
extern void	Var_Dump(GNode *);
extern void DirAddDir(const Lst,const char *);

