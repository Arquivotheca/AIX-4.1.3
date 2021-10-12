/* @(#)16	1.3  src/bos/usr/include/diag/assign_bit.h, cmddiag, bos411, 9428A410j 12/8/92 09:00:19 */
/*
 *   COMPONENT_NAME: CMDDIAG
 *
 *   FUNCTIONS: Diagnostic header file.
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1992
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include "tmdefs.h"
struct tm_env_da_bit
{
	int	tm_env;		/* DC selected execution mode		   */
	int	da_bit;		/* DA equivelant bit to the DC mode	   */
};

struct	tm_env_da_bit tm_da_exenv[] =
{
	EXENV_IPL,		DA_EXENV_IPL,
	EXENV_STD,		DA_EXENV_STD,
	EXENV_REGR,		DA_EXENV_REGR,
	EXENV_CONC,		DA_EXENV_CONC,
	EXENV_SYSX,		DA_EXENV_SYSX,

	0,			0,
};

struct	tm_env_da_bit tm_da_advanced[] = 
{
	ADVANCED_TRUE,		DA_ADVANCED_TRUE,
	ADVANCED_FALSE,		DA_ADVANCED_FALSE,

	0,			0,
};

struct	tm_env_da_bit tm_da_system[] = 
{
	SYSTEM_TRUE,		DA_SYSTEM_TRUE,
	SYSTEM_FALSE,		DA_SYSTEM_FALSE,

	0,			0,
};

struct	tm_env_da_bit tm_da_dmode[] = 
{
	DMODE_ELA,		DA_DMODE_ELA,
	DMODE_PD,		DA_DMODE_PD,
	DMODE_REPAIR,		DA_DMODE_REPAIR,
	DMODE_MS1,		DA_DMODE_MS1,
	DMODE_MS2,		DA_DMODE_MS2,
	DMODE_FREELANCE,	DA_DMODE_FREELANCE,

	0,			0,
};

struct	tm_env_da_bit tm_da_loopmode[] = 
{
	LOOPMODE_NOTLM,		DA_LOOPMODE_NOTLM,
	LOOPMODE_ENTERLM,	DA_LOOPMODE_ENTERLM,
	LOOPMODE_INLM,		DA_LOOPMODE_INLM,
	LOOPMODE_EXITLM,	DA_LOOPMODE_EXITLM,

	0,			0,
};

struct	tm_env_da_bit tm_da_console[] = 
{
	CONSOLE_TRUE,		DA_CONSOLE_TRUE,
	CONSOLE_FALSE,		DA_CONSOLE_FALSE,

	0,			0,
};
